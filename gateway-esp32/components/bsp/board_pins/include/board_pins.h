/**
 * @file board_pins.h
 * @brief Hardware Pin Definitions for ESMU Gateway
 */

#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include "driver/uart.h"

/* CAN Hardware Configuration */
#define CAN_TX_PIN           12
#define CAN_RX_PIN           13

/* I2C Hardware Configuration */
#define I2C_SDA_PIN          21
#define I2C_SCL_PIN          22

/* SIM7600/A7680C UART Configuration */
#define SIM_TX_PIN           17
#define SIM_RX_PIN           16
#define SIM_RST_PIN          5
#define SIM_UART_PORT        UART_NUM_2

/* UI/Status Hardware */
#define CONFIG_PIN_PROVISION_BUTTON 15
#define CONFIG_PIN_STATUS_LED      2
#define CONFIG_PIN_CONFIG_LED      4

/* ST7789 Display Configuration (SPI) */
#define ST7789_SCL_PIN             18   // SCLK
#define ST7789_SDA_PIN             23   // MOSI
#define ST7789_CS_PIN              26
#define ST7789_DC_PIN              27
#define ST7789_RST_PIN             33
#define ST7789_BL_PIN              32

#endif // BOARD_PINS_H
