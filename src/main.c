#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "ad5940.h"
#include "BodyImpedance.h" // ¡Asegúrate de descargar este archivo de ADI!

LOG_MODULE_REGISTER(main_biometria, LOG_LEVEL_INF);

#define APPBUFF_SIZE 512
uint32_t AppBuff[APPBUFF_SIZE];

/**
 * @brief Procesa y muestra los resultados obtenidos por la interrupción
 */
static void BIAShowResult(uint32_t *pData, uint32_t DataCount)
{
    float freq;
    fImpPol_Type *pImp = (fImpPol_Type*)pData;
    
    AppBIACtrl(BIACTRL_GETFREQ, &freq);
    LOG_INF("Frecuencia de prueba: %.2f Hz", freq);

    for(int i = 0; i < DataCount; i++) {
        LOG_INF("Magnitud: %f Ohm | Fase: %f grados", 
                pImp[i].Magnitude, 
                pImp[i].Phase * 180 / MATH_PI);
    }
}

/**
 * @brief Configuración inicial del hardware y los relojes
 */
static int32_t AD5940PlatformCfg(void)
{
    CLKCfg_Type clk_cfg;
    FIFOCfg_Type fifo_cfg;

    AD5940_HWReset();
    AD5940_Initialize();

    /* 1. Configurar reloj a 16MHz */
    clk_cfg.ADCClkDiv = ADCCLKDIV_1;
    clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
    clk_cfg.SysClkDiv = SYSCLKDIV_1;
    clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
    clk_cfg.HfOSC32MHzMode = bFALSE;
    clk_cfg.HFOSCEn = bTRUE;
    clk_cfg.HFXTALEn = bFALSE;
    clk_cfg.LFOSCEn = bTRUE;
    AD5940_CLKCfg(&clk_cfg);

    /* 2. Configurar el FIFO para recibir los resultados de la Transformada de Fourier (DFT) */
    fifo_cfg.FIFOEn = bFALSE;
    fifo_cfg.FIFOMode = FIFOMODE_FIFO;
    fifo_cfg.FIFOSize = FIFOSIZE_4KB; 
    fifo_cfg.FIFOSrc = FIFOSRC_DFT;
    fifo_cfg.FIFOThresh = 4;
    AD5940_FIFOCfg(&fifo_cfg); // Apagar para limpiar
    fifo_cfg.FIFOEn = bTRUE;  
    AD5940_FIFOCfg(&fifo_cfg); // Encender

    /* 3. Habilitar interrupciones */
    AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);
    AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH, bTRUE);
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);
    return 0;
}

/**
 * @brief Configuración de los parámetros del barrido de bioimpedancia
 */
static void AD5940BIAStructInit(void)
{
    AppBIACfg_Type *pBIACfg;
    AppBIAGetCfg(&pBIACfg);
  
    pBIACfg->SeqStartAddr = 0;
    pBIACfg->MaxSeqLen = 512;
    pBIACfg->RcalVal = 10000.0;     /* Resistencia de calibración (10k Ohm típicos) */
    pBIACfg->DftNum = DFTNUM_8192;  /* Precisión de la DFT */
    pBIACfg->NumOfData = -1;        /* Bucle infinito */
    pBIACfg->BiaODR = 20;           /* Tasa de muestreo (20 Hz) */
    pBIACfg->FifoThresh = 4;
    pBIACfg->ADCSinc3Osr = ADCSINC3OSR_2;
}

int main(void)
{
    uint32_t temp_size;
    k_msleep(2000); 

    printk("\r\n===========================================\r\n");
    printk("HOLA! El programa entro a la funcion MAIN.\r\n");
    printk("===========================================\r\n");

    LOG_INF("Iniciando medición BIA...");

    /* 1. Inicializar recursos del nRF5340 (pines e interrupciones) */
    if (AD5940_MCUResourceInit(NULL) != 0) {
        LOG_ERR("Fallo en MCUResourceInit");
        return -1;
    }

    /* 2. Reset por hardware para despertar al sensor */
    AD5940_HWReset();
    k_msleep(100); /* Damos 100ms para que el chip respire */

    /* 3. LA PRUEBA DE FUEGO: Leer el CHIP ID */
    uint32_t chip_id = AD5940_GetChipID();
    LOG_INF(">>> CHIP ID Leido: 0x%04x <<<", chip_id);

    if (chip_id != 0x5501 && chip_id != 0x5502) {
        LOG_ERR("ERROR FATAL: El SPI no funciona. Revisa los 6 cables.");
        LOG_ERR("El programa se detiene aqui para que no salgan ceros.");
        while(1) { k_msleep(1000); } /* Bucle infinito para bloquear la placa */
    }

    LOG_INF("¡EXITO! El sensor responde perfectamente. Configuracion BIA...");

    /* Si llegamos aquí, el SPI va perfecto. Seguimos con el programa */
    AD5940PlatformCfg();
    AD5940BIAStructInit();

    AppBIAInit(AppBuff, APPBUFF_SIZE);
    AppBIACtrl(BIACTRL_START, 0);
 
    while (1) {
        if (AD5940_GetMCUIntFlag()) {
            AD5940_ClrMCUIntFlag();
            temp_size = APPBUFF_SIZE;
            AppBIAISR(AppBuff, &temp_size); 
            
            if (temp_size > 0) {
                BIAShowResult(AppBuff, temp_size);
            }
        }
        k_yield(); 
    }
    return 0;
}