#include "ad5940.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ad5940_port, LOG_LEVEL_INF);

/* 1. Obtenemos las referencias del Device Tree (app.overlay) */
#define SPI_NODE DT_NODELABEL(ad5940)
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

static const struct spi_dt_spec spi_dev = SPI_DT_SPEC_GET(SPI_NODE, SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0);
static const struct gpio_dt_spec reset_pin = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, reset_gpios);
static const struct gpio_dt_spec int_pin = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, int_gpios);

/* NUEVO: Pin de Chip Select manual para evitar que Zephyr corte la señal */
static const struct gpio_dt_spec cs_pin = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, cs_gpios);

/* 2. Variables para la interrupción */
static struct gpio_callback ad5940_int_cb_data;
volatile uint32_t ucInterrupted = 0; /* Bandera o flag de interrupción */

/* =========================================================================
 * Manejador de Interrupciones (Equivalente a Ext_Int0_Handler)
 * ========================================================================= */
void ad5940_int_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    /* El AD5940 nos avisa que hay datos listos */
    ucInterrupted = 1;
}

/* =========================================================================
 * Implementación de las funciones "Port" exigidas por ad5940.h
 * ========================================================================= */

uint32_t AD5940_MCUResourceInit(void *pCfg)
{
    int ret;
    LOG_INF("Inicializando recursos de hardware para AD5940...");

    /* Configurar SPI */
    if (!spi_is_ready_dt(&spi_dev)) {
        LOG_ERR("Error: El bus SPI no está listo.");
        return 1; 
    }

    /* Configurar pin de Chip Select (CS) manual */
    if (!gpio_is_ready_dt(&cs_pin)) {
        LOG_ERR("Error: El pin de CS no está listo.");
        return 1;
    }
    gpio_pin_configure_dt(&cs_pin, GPIO_OUTPUT_INACTIVE);

    /* Configurar pin de Reset */
    if (!gpio_is_ready_dt(&reset_pin)) {
        LOG_ERR("Error: El pin de Reset no está listo.");
        return 1;
    }
    gpio_pin_configure_dt(&reset_pin, GPIO_OUTPUT_INACTIVE);

    /* Configurar pin de Interrupción (INTC) */
    if (!gpio_is_ready_dt(&int_pin)) {
        LOG_ERR("Error: El pin de Interrupción no está listo.");
        return 1;
    }
    gpio_pin_configure_dt(&int_pin, GPIO_INPUT);
    
    /* Configurar la interrupción para que salte en el flanco activo */
    ret = gpio_pin_interrupt_configure_dt(&int_pin, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Error configurando la interrupción: %d", ret);
        return 1;
    }

    /* Enganchar la función de callback al pin */
    gpio_init_callback(&ad5940_int_cb_data, ad5940_int_callback, BIT(int_pin.pin));
    gpio_add_callback(int_pin.port, &ad5940_int_cb_data);

    /* Estado inicial: CS y Reset en alto para que el sensor no se vuelva loco al arrancar */
    AD5940_CsSet();
    AD5940_RstSet();

    LOG_INF("Recursos de Hardware (SPI, Reset, CS Manual e Interrupciones) OK.");
    return 0; 
}

void AD5940_CsClr(void) {
    /* 1 = Activa el pin (lo pone a 0V porque lo definimos como ACTIVE_LOW) */
    gpio_pin_set_dt(&cs_pin, 1); 
}

void AD5940_CsSet(void) {
    /* 0 = Desactiva el pin (lo pone a 3.3V) */
    gpio_pin_set_dt(&cs_pin, 0); 
}

void AD5940_RstClr(void) {
    gpio_pin_set_dt(&reset_pin, 1); /* Activa el Reset */
}

void AD5940_RstSet(void) {
    gpio_pin_set_dt(&reset_pin, 0); /* Desactiva el Reset */
}

void AD5940_Delay10us(uint32_t time) {
    k_busy_wait(time * 10);
}

uint32_t AD5940_GetMCUIntFlag(void) {
    return ucInterrupted;
}

uint32_t AD5940_ClrMCUIntFlag(void) {
    ucInterrupted = 0;
    return 1;
}

void AD5940_ReadWriteNBytes(unsigned char *pSendBuffer, unsigned char *pRecvBuff, unsigned long length)
{
    struct spi_buf tx_buf = { .buf = pSendBuffer, .len = length };
    struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };

    struct spi_buf rx_buf = { .buf = pRecvBuff, .len = length };
    struct spi_buf_set rx = { .buffers = &rx_buf, .count = 1 };

    /* Al quitar el CS del Device Tree del bus SPI, esta función se limita 
       estrictamente a mover los datos sin tocar el pin de habilitación */
    int ret = spi_transceive_dt(&spi_dev, &tx, &rx);
    if (ret < 0) {
        LOG_ERR("Error SPI: %d", ret);
    }
}