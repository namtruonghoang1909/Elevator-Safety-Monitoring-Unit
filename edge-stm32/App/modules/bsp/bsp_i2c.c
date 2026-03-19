/**
 * @file bsp_i2c.c
 * @brief Thread-safe I2C Board Support Package implementation for STM32
 */

#include "bsp_i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────

static SemaphoreHandle_t i2c_mutex = NULL;
static StaticSemaphore_t i2c_mutex_buffer;

#define I2C_TIMEOUT_MS  500

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

static bsp_i2c_status_t map_hal_status(HAL_StatusTypeDef status) {
    switch (status) {
        case HAL_OK:      return BSP_I2C_OK;
        case HAL_BUSY:    return BSP_I2C_BUSY;
        case HAL_TIMEOUT: return BSP_I2C_TIMEOUT;
        case HAL_ERROR:
        default:          return BSP_I2C_ERROR;
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bsp_i2c_status_t bsp_i2c_init(void) {
    // Force a software reset of the I2C peripheral to clear the BUSY flag
    if (i2c_mutex == NULL) {
        i2c_mutex = xSemaphoreCreateMutexStatic(&i2c_mutex_buffer);
    }
    return (i2c_mutex != NULL) ? BSP_I2C_OK : BSP_I2C_ERROR;
}

// ─────────────────────────────────────────────
// Register-level Operations
// ─────────────────────────────────────────────

bsp_i2c_status_t bsp_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val) {
    if (i2c_mutex == NULL) return BSP_I2C_ERROR;

    bsp_i2c_status_t status = BSP_I2C_ERROR;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) == pdTRUE) {
        uint16_t stm_addr = (uint16_t)(dev_addr << 1);
        HAL_StatusTypeDef hal_ret = HAL_I2C_Mem_Write(&hi2c1, stm_addr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, I2C_TIMEOUT_MS);
        status = map_hal_status(hal_ret);
        xSemaphoreGive(i2c_mutex);
    } else {
        status = BSP_I2C_TIMEOUT;
    }
    return status;
}

bsp_i2c_status_t bsp_i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *val) {
    if (i2c_mutex == NULL) return BSP_I2C_ERROR;

    bsp_i2c_status_t status = BSP_I2C_ERROR;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) == pdTRUE) {
        uint16_t stm_addr = (uint16_t)(dev_addr << 1);
        HAL_StatusTypeDef hal_ret = HAL_I2C_Mem_Read(&hi2c1, stm_addr, reg, I2C_MEMADD_SIZE_8BIT, val, 1, I2C_TIMEOUT_MS);
        status = map_hal_status(hal_ret);
        xSemaphoreGive(i2c_mutex);
    } else {
        status = BSP_I2C_TIMEOUT;
    }
    return status;
}

bsp_i2c_status_t bsp_i2c_read_consecutive_regs(uint8_t dev_addr, uint8_t start_reg, uint8_t *data_buf, uint16_t len) {
    if (i2c_mutex == NULL) return BSP_I2C_ERROR;

    bsp_i2c_status_t status = BSP_I2C_ERROR;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) == pdTRUE) {
        uint16_t stm_addr = (uint16_t)(dev_addr << 1);
        HAL_StatusTypeDef hal_ret = HAL_I2C_Mem_Read(&hi2c1, stm_addr, start_reg, I2C_MEMADD_SIZE_8BIT, data_buf, len, I2C_TIMEOUT_MS);
        status = map_hal_status(hal_ret);
        xSemaphoreGive(i2c_mutex);
    } else {
        status = BSP_I2C_TIMEOUT;
    }
    return status;
}

// ─────────────────────────────────────────────
// Low-level Byte / Bytes Operations 
// ─────────────────────────────────────────────

bsp_i2c_status_t bsp_i2c_write_byte(uint8_t dev_addr, uint8_t value) {
    return bsp_i2c_write_bytes(dev_addr, &value, 1);
}

bsp_i2c_status_t bsp_i2c_write_bytes(uint8_t dev_addr, const uint8_t *data, uint16_t len) {
    if (i2c_mutex == NULL) return BSP_I2C_ERROR;

    bsp_i2c_status_t status = BSP_I2C_ERROR;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) == pdTRUE) {
        uint16_t stm_addr = (uint16_t)(dev_addr << 1);
        HAL_StatusTypeDef hal_ret = HAL_I2C_Master_Transmit(&hi2c1, stm_addr, (uint8_t*)data, len, I2C_TIMEOUT_MS);
        status = map_hal_status(hal_ret);
        xSemaphoreGive(i2c_mutex);
    } else {
        status = BSP_I2C_TIMEOUT;
    }
    return status;
}

bsp_i2c_status_t bsp_i2c_read_byte(uint8_t dev_addr, uint8_t *out_val) {
    if (i2c_mutex == NULL) return BSP_I2C_ERROR;

    bsp_i2c_status_t status = BSP_I2C_ERROR;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) == pdTRUE) {
        uint16_t stm_addr = (uint16_t)(dev_addr << 1);
        HAL_StatusTypeDef hal_ret = HAL_I2C_Master_Receive(&hi2c1, stm_addr, out_val, 1, I2C_TIMEOUT_MS);
        status = map_hal_status(hal_ret);
        xSemaphoreGive(i2c_mutex);
    } else {
        status = BSP_I2C_TIMEOUT;
    }
    return status;
}

bool bsp_i2c_is_ready(uint8_t dev_addr) {
    if (i2c_mutex == NULL) return false;

    bool ready = false;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) == pdTRUE) {
        uint16_t stm_addr = (uint16_t)(dev_addr << 1);
        if (HAL_I2C_IsDeviceReady(&hi2c1, stm_addr, 3, I2C_TIMEOUT_MS) == HAL_OK) {
            ready = true;
        }
        xSemaphoreGive(i2c_mutex);
    }
    return ready;
}
