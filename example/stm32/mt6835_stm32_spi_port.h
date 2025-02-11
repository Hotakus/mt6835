/*******************************************************************************
* @file           : mt6835_stm32_spi_port.h
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

#ifndef MT6835_STM32_SPI_PORT_H
#define MT6835_STM32_SPI_PORT_H

#define MT6835_STM32_SPI_PORT_ENABLE  (0u)

#ifdef __cplusplus
extern "C" {
#endif

#include "mt6835.h"

mt6835_t *mt6835_stm32_spi_port_init(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //MT6835_STM32_SPI_PORT_H
