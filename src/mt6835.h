/*******************************************************************************
* @file           : mt6835.h
* @author         : Hotakus (hotakus@foxmail.com)
* @brief          : None
* @date           : 2025/2/10
*
* SPDX-License-Identifier: MPL-2.0
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at https://mozilla.org/MPL/2.0/.
* Copyright (c) 2025 Hotakus. All rights reserved.
*****************************************************************************/

#ifndef MT6835_H
#define MT6835_H

#include <stdbool.h>
#include "stdint.h"
#include "stdlib.h"

#define MT6835_USE_DEBUG        (1u)    // enable debug or not
#define MT6835_USE_CRC          (1u)    // enable CRC or not
#define MT6835_MALLOC(x)        malloc(x)
#define MT6835_FREE(x)          free(x)

#if MT6835_USE_DEBUG == 1
#include "stdio.h"
#define MT6835_DEBUG(...)       printf(__VA_ARGS__)
#else
#define MT6835_DEBUG(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MT6835_ZERO_REG_STEP        (0.088f)
#define MT6835_ANGLE_RESOLUTION     (1 << 21)

typedef enum mt6835_cmd_enum_t {
    MT6835_CMD_RD = (0b0011),               /**< user read register. */
    MT6835_CMD_WR = (0b0110),               /**< user write register. */
    MT6835_CMD_EEPROM = (0b1100),           /**< user erase and program EEPROM. */
    MT6835_CMD_ZERO = (0b0101),             /**< AUTO setting zero. */
    MT6835_CMD_BURST = (0b1010),            /**< burst mode. */
} mt6835_cmd_enum_t;

typedef enum mt6835_reg_enum_t {
    MT6835_REG_ID = (0x001),                 // ID
    MT6835_REG_ANGLE3 = (0x003),             // angle 3
    MT6835_REG_ANGLE2 = (0x004),             // angle 2
    MT6835_REG_ANGLE1 = (0x005),             // angle 1 and state
    MT6835_REG_CRC = (0x006),                // CRC
    MT6835_REG_ABZ_RES2 = (0x007),           // ABZ res 2
    MT6835_REG_ABZ_RES1 = (0x008),           // ABZ res 1
    MT6835_REG_ZERO2 = (0x009),              // zero 2
    MT6835_REG_ZERO1 = (0x00A),              // zero 1
    MT6835_REG_UVW = (0x00B),                // UVW
    MT6835_REG_PWM = (0x00C),                // PWM
    MT6835_REG_HYST = (0x00D),               // HYST
    MT6835_REG_AUTOCAL = (0x00E),            // AUTO cal
} mt6835_reg_enum_t;

typedef enum mt6835_cs_state_enum_t {
    MT6835_CS_LOW = 0,
    MT6835_CS_HIGH = 1
} mt6835_cs_state_enum_t;

/**
 *  mt6835 state
 */
typedef enum mt6835_state_enum_t {
    MT6835_STATE_OVER_SPEED = 0x01,            // over speed
    MT6835_STATE_MAG_FIELD_WEAK = 0x02,        // mag field weak
    MT6835_STATE_VCC_UNDERVOLTAGE = 0x04       // vcc under voltage
} mt6835_state_enum_t;

/**
 *  mt6835 crc check enable
 */
typedef enum mt6835_crc_check_enum_t {
    MT6835_CRC_CHECK_DISABLE = 0,
    MT6835_CRC_CHECK_ENABLE = 1
} mt6835_crc_check_enum_t;

typedef enum mt6835_read_angle_method_enum_t {
    MT6835_READ_ANGLE_METHOD_NORMAL = 0,
    MT6835_READ_ANGLE_METHOD_BURST = 1,
} mt6835_read_angle_method_enum_t;

/**
 *  mt6835 data frame
 */
typedef struct mt6835_data_frame_t {
    union {
        uint32_t pack;
        struct {
            uint8_t reserved: 4;
            mt6835_cmd_enum_t cmd: 4;
            mt6835_reg_enum_t reg: 8;
            uint8_t normal_byte: 8;
            uint8_t empty_byte: 8;
        };
    };
} mt6835_data_frame_t;

/**
 *  mt6835 object
 */
typedef struct mt6835_t {
    struct {
        void (*spi_cs_control)(mt6835_cs_state_enum_t state);
        void (*spi_send)(uint8_t *tx_buf, uint8_t len);
        void (*spi_recv)(uint8_t *rx_buf, uint8_t len);
        void (*spi_send_recv)(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len);
    } func;

    uint8_t crc;
    bool crc_res;

    uint32_t raw_angle; // raw angle, 21 bits
    mt6835_data_frame_t data_frame;
    mt6835_state_enum_t state;
    mt6835_crc_check_enum_t crc_check;
} mt6835_t;


/* mt6835 create and destroy functions */
mt6835_t *mt6835_create();
void mt6835_destroy(mt6835_t *mt6835);

#if MT6835_USE_CRC == 1
void mt6835_enable_crc_check(mt6835_t *mt6835);
void mt6835_disable_crc_check(mt6835_t *mt6835);
#endif

/* link functions, link to your spi interface */
void mt6835_link_spi_cs_control(mt6835_t *mt6835, void (*spi_cs_control)(mt6835_cs_state_enum_t state));
void mt6835_link_spi_send(mt6835_t *mt6835, void (*spi_send)(uint8_t *tx_buf, uint8_t len));
void mt6835_link_spi_recv(mt6835_t *mt6835, void (*spi_recv)(uint8_t *rx_buf, uint8_t len));
void mt6835_link_spi_send_recv(mt6835_t *mt6835, void (*spi_send_recv)(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len));

/* mt6835 get and set functions*/
uint8_t mt6835_get_id(mt6835_t *mt6835);
void mt6835_set_id(mt6835_t *mt6835, uint8_t custom_id);
uint32_t mt6835_get_raw_angle(mt6835_t *mt6835, mt6835_read_angle_method_enum_t method);
uint16_t mt6835_get_raw_zero_angle(mt6835_t *mt6835);
float mt6835_get_angle(mt6835_t *mt6835, mt6835_read_angle_method_enum_t method);
float mt6835_get_zero_angle(mt6835_t *mt6835);
bool mt6835_set_zero_angle(mt6835_t *mt6835, float rad);

/* mt6835 base functions */
uint8_t mt6835_read_reg(mt6835_t *mt6835, mt6835_reg_enum_t reg);
void mt6835_write_reg(mt6835_t *mt6835, mt6835_reg_enum_t reg, uint8_t data);
bool mt6835_write_eeprom(mt6835_t *mt6835);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //MT6835_H
