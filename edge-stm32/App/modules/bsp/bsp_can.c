/**
 * @file bsp_can.c
 * @brief CAN Board Support Package implementation for STM32
 */

#include "bsp_can.h"
#include <stdio.h>
#include <string.h>

extern CAN_HandleTypeDef hcan;

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bsp_can_status_t bsp_can_init(void) {
    CAN_FilterTypeDef sFilterConfig;

    // Filter 0: Accept ALL Standard IDs (Filter Mask Mode)
    // For ESMU, we want IDs like 0x010, 0x100, 0x200
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
        return BSP_CAN_ERROR;
    }

    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        return BSP_CAN_ERROR;
    }

    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return BSP_CAN_ERROR;
    }

    return BSP_CAN_OK;
}

bsp_can_status_t bsp_can_send(uint32_t std_id, const uint8_t *data, uint8_t len) {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;

    if (len > 8) return BSP_CAN_ERROR;

    TxHeader.StdId = std_id;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = len;
    TxHeader.TransmitGlobalTime = DISABLE;

    // Wait for a free mailbox (Simple blocking for BSP)
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0);

    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, (uint8_t*)data, &TxMailbox) != HAL_OK) {
        return BSP_CAN_ERROR;
    }

    return BSP_CAN_OK;
}

bsp_can_status_t bsp_can_read(uint32_t *std_id, uint8_t *data, uint8_t *len) {
    CAN_RxHeaderTypeDef RxHeader;

    if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, data) != HAL_OK) {
        return BSP_CAN_ERROR;
    }

    *std_id = RxHeader.StdId;
    *len = RxHeader.DLC;

    return BSP_CAN_OK;
}
