#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "ad5940.h"
#include "BodyImpedance.h"

LOG_MODULE_REGISTER(bia_app, LOG_LEVEL_INF);

#define APPBUFF_SIZE 512
static uint32_t AppBuff[APPBUFF_SIZE];

/* Externs del port y la app */
extern uint32_t AD5940_MCUResourceInit(void *pCfg);

/* --- Configuración de la plataforma con Trazas de Control --- */
static int32_t AD5940PlatformCfg(void)
{
    CLKCfg_Type clk_cfg;
    FIFOCfg_Type fifo_cfg;
    AGPIOCfg_Type gpio_cfg;

    LOG_INF("  [Punto A] Ejecutando HWReset...");
    AD5940_HWReset();
    
    LOG_INF("  [Punto B] Ejecutando AD5940_Initialize...");
    AD5940_Initialize();

    LOG_INF("  [Punto C] Configurando Reloj...");
    clk_cfg.ADCClkDiv = ADCCLKDIV_1;
    clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
    clk_cfg.SysClkDiv = SYSCLKDIV_1;
    clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
    clk_cfg.HfOSC32MHzMode = bFALSE;
    clk_cfg.HFOSCEn = bTRUE;
    clk_cfg.HFXTALEn = bFALSE;
    clk_cfg.LFOSCEn = bTRUE;
    AD5940_CLKCfg(&clk_cfg);

    LOG_INF("  [Punto D] Configurando FIFO...");
    fifo_cfg.FIFOEn = bFALSE;
    fifo_cfg.FIFOMode = FIFOMODE_FIFO;
    fifo_cfg.FIFOSize = FIFOSIZE_4KB;
    fifo_cfg.FIFOSrc = FIFOSRC_DFT;
    fifo_cfg.FIFOThresh = 4; 
    AD5940_FIFOCfg(&fifo_cfg);
    fifo_cfg.FIFOEn = bTRUE;  
    AD5940_FIFOCfg(&fifo_cfg);
    
    LOG_INF("  [Punto E] Configurando Interrupt Controller...");
    AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);
    AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH, bTRUE);
    AD5940_INTCClrFlag(AFEINTSRC_ALLINT);

    LOG_INF("  [Punto F] Configurando AGPIOs...");
    gpio_cfg.FuncSet = GP6_SYNC|GP5_SYNC|GP4_SYNC|GP2_TRIG|GP1_SYNC|GP0_INT;
    gpio_cfg.InputEnSet = AGPIO_Pin2;
    gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin4|AGPIO_Pin5|AGPIO_Pin6;
    gpio_cfg.OutVal = 0;
    gpio_cfg.PullEnSet = 0;
    AD5940_AGPIOCfg(&gpio_cfg);

    AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK); 
    return 0;
}

void main(void)
{
    uint32_t temp;
    AppBIACfg_Type *pBIACfg;

    LOG_INF("=== PASO 1: Inicializando MCU Resource ===");
    if (AD5940_MCUResourceInit(NULL) != 0) {
        LOG_ERR("Fallo crítico en MCUResourceInit");
        return;
    }

    LOG_INF("=== PASO 2: Entrando en AD5940PlatformCfg ===");
    AD5940PlatformCfg();
    LOG_INF("=== PASO 3: Configuración de plataforma OK ===");

    LOG_INF("=== PASO 4: Obteniendo Cursors de BIA ===");
    AppBIAGetCfg(&pBIACfg);
    pBIACfg->SeqStartAddr = 0;
    pBIACfg->MaxSeqLen = 512;
    pBIACfg->RcalVal = 10000.0;
    pBIACfg->DftNum = DFTNUM_8192;
    pBIACfg->NumOfData = -1;
    pBIACfg->BiaODR = 20;
    pBIACfg->FifoThresh = 4;
    pBIACfg->ADCSinc3Osr = ADCSINC3OSR_2;

    LOG_INF("=== PASO 5: Entrando en AppBIAInit (Escritura de Sequencer) ===");
    AppBIAInit(AppBuff, APPBUFF_SIZE);
    LOG_INF("=== PASO 6: AppBIAInit OK ===");

    LOG_INF("=== PASO 7: Ejecutando AppBIACtrl START ===");
    AppBIACtrl(BIACTRL_START, 0);
    LOG_INF("=== PASO 8: Todo el sistema arrancado ===");

    while(1) {

        //LOG_INF("AD5940_GetMCUIntFlag() VALUE: %d", AD5940_GetMCUIntFlag());
        
        if(AD5940_GetMCUIntFlag()) {
            AD5940_ClrMCUIntFlag();
            temp = APPBUFF_SIZE;
            AppBIAISR(AppBuff, &temp); 
            
            fImpPol_Type *pImp = (fImpPol_Type*)AppBuff;
            for(int i=0; i<temp; i++) {
                LOG_INF("Impedancia: %.2f Ohm | Fase: %.2f deg", 
                        pImp[i].Magnitude, 
                        pImp[i].Phase * 180 / 3.14159); //Revisar
            }
        }

        static uint32_t diag_tick = 0;
        if(++diag_tick % 100 == 0) {
            uint32_t fifo_cnt = AD5940_FIFOGetCnt();
            uint32_t isr_status = AD5940_ReadReg(REG_INTC_INTCFLAG0); 
            uint32_t afe_stat = AD5940_ReadReg(REG_AFE_PSWSTA);        
            
            LOG_INF("--- DIAGNOSTICO BIA ---");
            LOG_INF("FIFO Count: %d", fifo_cnt);
            LOG_INF("Interrupt Flag (INTC0): 0x%08x", isr_status);
            LOG_INF("Switch Matrix (PSWSTA): 0x%08x", afe_stat);
        }
        k_sleep(K_MSEC(10));
    }
}