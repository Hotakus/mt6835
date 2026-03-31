/*******************************************************************************
* @file           : mt6835_stm32_spi_port.c
* @author         : Hotakus (hotakus@foxmail.com)
* @brief          : FOC main header file
* @date           : 2025/2/10
*
* SPDX-License-Identifier: MPL-2.0
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at https://mozilla.org/MPL/2.0/.
* Copyright (c) 2025 Hotakus. All rights reserved.
*****************************************************************************/


#include "mt6835_stm32_spi_port.h"

#if MT6835_STM32_SPI_PORT_ENABLE == 1

#include "spi.h"
#define SPI_INSTANCE    hspi1
#define SPI_CS          SPI1_CS_Pin
#define SPI_CS_PORT     SPI1_CS_GPIO_Port

#if MT6835_STM32_SPI_PORT_ENABLE == 1
static mt6835_t *mt6835 = NULL;
#endif

static void mt6835_cs_control(mt6835_cs_state_enum_t state) {
    if (state == MT6835_CS_HIGH) {
        HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS, GPIO_PIN_RESET);
    }
}

static void mt6835_spi_send_recv(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len) {
    HAL_SPI_TransmitReceive(&SPI_INSTANCE, tx_buf, rx_buf, len, 1000);
}

#if MT6835_USE_DMA == 1
static void mt6835_spi_dma_send_recv(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len) {
    HAL_SPI_TransmitReceive_DMA(&SPI_INSTANCE, tx_buf, rx_buf, len);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1)
    {
#if MT6835_USE_DMA == 1
        mt6835_dma_transmit_done(mt6835);
#endif
    }
}
#endif

#endif


/**
 * @brief mt6835 stm32 spi port init
 * @return mt6835 object
 */
mt6835_t *mt6835_stm32_spi_port_init(void) {
#if MT6835_STM32_SPI_PORT_ENABLE == 1
    mt6835 = mt6835_create();
    // HAL_Delay(2000);
    mt6835_link_spi_cs_control(mt6835, mt6835_cs_control);
    mt6835_link_spi_send_recv(mt6835, mt6835_spi_send_recv);
#if MT6835_USE_DMA == 1
    mt6835_link_spi_dma_send_recv(mt6835, mt6835_spi_dma_send_recv);
#endif
    return mt6835;
#else
    return NULL;
#endif
}
