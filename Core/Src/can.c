/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef MSG_BOOT = {0, 0, 0, 0, 0, (FunctionalState)0};
CAN_TxHeaderTypeDef MSG_SIZE = {0, 0, 0, 0, 0, (FunctionalState)0};
CAN_TxHeaderTypeDef MSG_DATA = {0, 0, 0, 0, 0, (FunctionalState)0};
CAN_TxHeaderTypeDef MSG_NEXT = {0, 0, 0, 0, 0, (FunctionalState)0};

int8_t sending;
int8_t received;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void) {

    /* USER CODE BEGIN CAN1_Init 0 */
    // Can ID
    // Device Mask: 0x00
    // Command ID : 0x12
    MSG_BOOT.StdId = 0x123;
    MSG_BOOT.ExtId = MSG_BOOT.StdId;
    MSG_BOOT.DLC   = 0;
    MSG_BOOT.RTR   = CAN_RTR_REMOTE;
    MSG_BOOT.IDE   = CAN_ID_STD;

    MSG_SIZE.StdId = 0x124;
    MSG_SIZE.ExtId = MSG_SIZE.StdId;
    MSG_SIZE.DLC   = 4;
    MSG_SIZE.RTR   = CAN_RTR_DATA;
    MSG_SIZE.IDE   = CAN_ID_STD;

    MSG_DATA.StdId = 0x125;
    MSG_DATA.ExtId = MSG_DATA.StdId;
    MSG_DATA.DLC   = 8;
    MSG_DATA.RTR   = CAN_RTR_DATA;
    MSG_DATA.IDE   = CAN_ID_STD;

    MSG_NEXT.StdId = 0x126;
    MSG_NEXT.ExtId = MSG_NEXT.StdId;
    MSG_NEXT.DLC   = 0;
    MSG_NEXT.RTR   = CAN_RTR_REMOTE;
    MSG_NEXT.IDE   = CAN_ID_STD;
    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance                  = CAN1;
    hcan1.Init.Prescaler            = 10;
    hcan1.Init.Mode                 = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth        = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1             = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2             = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode    = DISABLE;
    hcan1.Init.AutoBusOff           = DISABLE;
    hcan1.Init.AutoWakeUp           = ENABLE;
    hcan1.Init.AutoRetransmission   = ENABLE;
    hcan1.Init.ReceiveFifoLocked    = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */
    CAN_FilterTypeDef filter = {0, 0, 0, 0, 0, (FunctionalState)0};

    filter.FilterIdHigh         = 0x12F << 21;
    filter.FilterIdLow          = 0;
    filter.FilterMaskIdHigh     = 0x7FF << 21;
    filter.FilterMaskIdLow      = 0;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation     = CAN_FILTER_ENABLE;
    filter.SlaveStartFilterBank = 0;

    if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE END CAN1_Init 2 */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspInit 0 */

        /* USER CODE END CAN1_MspInit 0 */
        /* CAN1 clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**CAN1 GPIO Configuration
        PB5     ------> CAN1_RX
        PB6     ------> CAN1_TX
        */
        GPIO_InitStruct.Pin       = GPIO_PIN_5;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF3_CAN1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_6;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_CAN1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* CAN1 interrupt Init */
        HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
        /* USER CODE BEGIN CAN1_MspInit 1 */

        /* USER CODE END CAN1_MspInit 1 */
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle) {

    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN1 GPIO Configuration
        PB5     ------> CAN1_RX
        PB6     ------> CAN1_TX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_6);

        /* CAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
void MX_CAN1_Send(const CAN_TxHeaderTypeDef* header, const uint8_t* data) {
    uint32_t mailbox;
    HAL_CAN_AddTxMessage(&hcan1, (CAN_TxHeaderTypeDef*)header, (uint8_t*)data, &mailbox);
    while (HAL_CAN_IsTxMessagePending(&hcan1, mailbox))
        ;
}

unsigned char MX_CAN1_Available() {
    return (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) != 0 ||
            HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO1) != 0);
}

uint32_t MX_CAN1_Recv(uint8_t* data) {
    CAN_RxHeaderTypeDef header;
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) != 0) {
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &header, data);
    } else {
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO1, &header, data);
    }
    return header.StdId;
}

void MX_CAN1_DeInit() {
    HAL_CAN_Stop(&hcan1);
    HAL_CAN_DeInit(&hcan1);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
