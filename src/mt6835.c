/*******************************************************************************
* @file           : mt6835.c
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

#include "mt6835.h"
#include <math.h>
#include <string.h>

static char *TAG = "MT6835";

#if MT6835_USE_CRC == 1
static uint8_t crc8_table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};


/**
 * @brief  CRC校验：X8+X2+X+1
 * @param  data  数据指针
 * @param  len   数据长度
 * @return CRC校验值
 */
static uint8_t crc_table(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00; // 初始CRC值

    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i]; // 与数据异或
        crc = crc8_table[crc]; // 查表更新CRC
    }

    return crc;
}


/**
 * @brief enable crc check
 * @param mt6835 mt6835 object
 */
void mt6835_enable_crc_check(mt6835_t *mt6835) {
    mt6835->crc_check = true;
}


/**
 * @brief disable crc check
 * @param mt6835 mt6835 object
 */
void mt6835_disable_crc_check(mt6835_t *mt6835) {
    mt6835->crc_check = false;
}
#endif


/**
 * @brief spi cs control, this function is weak, you can override it
 * @param state MT6835_CS_HIGH or MT6835_CS_LOW
 */
__attribute__((weak)) void mt6835_cs_control(mt6835_cs_state_enum_t state) {
    (void)state;
}


/**
 * @brief spi send, this function is weak, you can override it
 * @param tx_buf tx buffer
 * @param len length
 */
__attribute__((weak)) void mt6835_spi_send(uint8_t *tx_buf, uint8_t len) {
    (void)tx_buf;
    (void)len;
}


/**
 * @brief spi receive, this function is weak, you can override it
 * @param rx_buf rx buffer
 * @param len length
 * @return uint8_t rx data
 */
__attribute__((weak)) void mt6835_spi_recv(uint8_t *rx_buf, uint8_t len) {
    (void)rx_buf;
    (void)len;
}


/**
 * @brief spi send and receive, this function is weak, you can override it
 * @param tx_buf tx buffer
 * @param rx_buf rx buffer
 * @param len length
 */
__attribute__((weak)) void mt6835_spi_send_recv(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len) {
    (void)tx_buf;
    (void)rx_buf;
    (void)len;
}


/**
 * @brief create a mt6835 object
 * @return mt6835 object
 */
mt6835_t *mt6835_create() {
    mt6835_t *mt6835 = (mt6835_t *)MT6835_MALLOC(sizeof(mt6835_t));
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s malloc failed", TAG);
        return NULL;
    }
    memset(mt6835, 0, sizeof(mt6835_t));
    mt6835->crc_check = false;
    return mt6835;
}


/**
 * @brief destroy a mt6835 object
 * @param mt6835 mt6835 object
 */
void mt6835_destroy(mt6835_t *mt6835) {
    if (mt6835 != NULL) {
        MT6835_FREE(mt6835);
    }
}


/**
 * @brief link spi cs control function to mt6835 object
 * @param mt6835 mt6835 object
 * @param spi_cs_control spi cs control function
 */
void mt6835_link_spi_cs_control(mt6835_t *mt6835, void (*spi_cs_control)(mt6835_cs_state_enum_t state)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_cs_control == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_cs_control(null)", TAG);
        mt6835->func.spi_cs_control = mt6835_cs_control;
    } else {
        mt6835->func.spi_cs_control = spi_cs_control;
    }
}


/**
 * @brief link spi send function to mt6835 object
 * @param mt6835 mt6835 object
 * @param spi_send spi send function
 */
void mt6835_link_spi_send(mt6835_t *mt6835, void (*spi_send)(uint8_t *tx_buf, uint8_t len)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_send == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_send(null)", TAG);
        mt6835->func.spi_send = mt6835_spi_send;
    }
    mt6835->func.spi_send = spi_send;
}


/**
 * @brief link spi receive function to mt6835 object
 * @param mt6835 mt6835 object
 * @param spi_recv spi receive function
 */
void mt6835_link_spi_recv(mt6835_t *mt6835, void (*spi_recv)(uint8_t *rx_buf, uint8_t len)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_recv == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_recv(null)", TAG);
        mt6835->func.spi_recv = mt6835_spi_recv;
    }
    mt6835->func.spi_recv = spi_recv;
}


/**
 * @brief link spi send and receive function to mt6835 object
 * @param mt6835 mt6835 object
 * @param spi_send_recv spi send and receive function
 */
void mt6835_link_spi_send_recv(mt6835_t *mt6835, void (*spi_send_recv)(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_send_recv == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_send_recv(null)", TAG);
        mt6835->func.spi_send_recv = mt6835_spi_send_recv;
    }
    mt6835->func.spi_send_recv = spi_send_recv;
}


/**
 * @brief get mt6835 id, the id can be programmed by user
 * @param mt6835 mt6835 object
 * @return id
 */
uint8_t mt6835_get_id(mt6835_t *mt6835) {
    return mt6835_read_reg(mt6835, MT6835_REG_ID);
}

/**
 * @brief set mt6835 id, the id can be programmed by user
 * @param mt6835 mt6835 object
 * @return id
 */
void mt6835_set_id(mt6835_t *mt6835, uint8_t custom_id) {
    mt6835_write_reg(mt6835, MT6835_REG_ID, custom_id);
}


/**
 * @brief get mt6835 raw angle
 * @param mt6835 mt6835 object
 * @param method read angle method, MT6835_READ_ANGLE_METHOD_NORMAL or MT6835_READ_ANGLE_METHOD_BURST
 * @return raw angle data in raw
 */
uint32_t mt6835_get_raw_angle(mt6835_t *mt6835, mt6835_read_angle_method_enum_t method) {
    uint8_t rx_buf[6] = {0};
    uint8_t tx_buf[6] = {0};

    switch (method) {
        case MT6835_READ_ANGLE_METHOD_NORMAL: {
            rx_buf[0] = mt6835_read_reg(mt6835, MT6835_REG_ANGLE3);
            rx_buf[1] = mt6835_read_reg(mt6835, MT6835_REG_ANGLE2);
            rx_buf[2] = mt6835_read_reg(mt6835, MT6835_REG_ANGLE1);
            if (mt6835->crc_check) {
                rx_buf[3] = mt6835_read_reg(mt6835, MT6835_REG_CRC);
            }
            break;
        }
        case MT6835_READ_ANGLE_METHOD_BURST: {
            const uint8_t rd = mt6835->crc_check ? 6 : 5;

            mt6835->func.spi_cs_control(MT6835_CS_HIGH);
            mt6835->data_frame.cmd = MT6835_CMD_BURST;
            mt6835->data_frame.reg = MT6835_REG_ANGLE3;
            tx_buf[0] = mt6835->data_frame.pack & 0xFF;
            tx_buf[1] = (mt6835->data_frame.pack >> 8) & 0xFF;

            mt6835->func.spi_cs_control(MT6835_CS_LOW);
            if (mt6835->func.spi_send_recv) {
                mt6835->func.spi_send_recv(tx_buf, rx_buf, rd);
            } else {
                mt6835->func.spi_send(tx_buf, rd);
                mt6835->func.spi_recv(rx_buf, rd);
            }
            mt6835->func.spi_cs_control(MT6835_CS_HIGH);

            memmove(rx_buf, &rx_buf[2], 3);
            if (mt6835->crc_check) {
                rx_buf[3] = rx_buf[5];
            }
            break;
        }
    }

    if (mt6835->crc_check) {
        mt6835->crc = rx_buf[3];
#if MT6835_USE_CRC == 1
        if (crc_table(rx_buf, 3) != rx_buf[3]) {
            MT6835_DEBUG("%s crc check failed\r\n", TAG);
            mt6835->crc_res = false;
            return 0;
        }
        mt6835->crc_res = true;
#endif
    }

    mt6835->state = rx_buf[2] & 0x07;
    return (rx_buf[0] << 13) | (rx_buf[1] << 5) | (rx_buf[2] >> 3);
}


/**
 * @brief get mt6835 angle in rad
 * @param mt6835 mt6835 object
 * @param method read angle method, MT6835_READ_ANGLE_METHOD_NORMAL or MT6835_READ_ANGLE_METHOD_BURST
 * @return angle data in rad
 */
float mt6835_get_angle(mt6835_t *mt6835, mt6835_read_angle_method_enum_t method) {
    uint32_t raw_angle = mt6835_get_raw_angle(mt6835, method);
    return (float)(raw_angle * 2.996056226329803e-6);
}


/**
 * @brief get mt6835 raw zero angle in raw
 * @param mt6835 mt6835 object
 * @return zero angle data in raw
 */
uint16_t mt6835_get_raw_zero_angle(mt6835_t *mt6835) {
    uint8_t rx_buf[2] = {0};
    rx_buf[1] = mt6835_read_reg(mt6835, MT6835_REG_ZERO2);
    rx_buf[0] = mt6835_read_reg(mt6835, MT6835_REG_ZERO1);
    uint16_t res = (rx_buf[1] << 4) | (rx_buf[0] >> 4);
    return res;
}


/**
 * @brief get mt6835 zero angle in deg
 * @param mt6835 mt6835 object
 * @return zero angle data in deg
 */
float mt6835_get_zero_angle(mt6835_t *mt6835) {
    return (float)mt6835_get_raw_zero_angle(mt6835) * MT6835_ZERO_REG_STEP;
}


/**
 * @brief set mt6835 zero angle
 * @param mt6835 mt6835 object
 * @param rad zero angle in rad
 * @return true: success, false: fail
 */
bool mt6835_set_zero_angle(mt6835_t *mt6835, float rad) {
    uint16_t angle = (uint16_t)roundf(rad * 57.295779513f / MT6835_ZERO_REG_STEP);
    if (angle > 0xFFF) {
        return false;
    }

    uint8_t tx_buf[2] = {0};

    tx_buf[1] = angle >> 4;
    tx_buf[0] = (angle & 0x0F) << 4;
    tx_buf[0] |= mt6835_read_reg(mt6835, MT6835_REG_ZERO1) & 0x0F;

    mt6835_write_reg(mt6835, MT6835_REG_ZERO2, tx_buf[1]);
    mt6835_write_reg(mt6835, MT6835_REG_ZERO1, tx_buf[0]);

    return true;
}


/**
 * @brief read mt6835 register
 * @param mt6835 mt6835 object
 * @param reg register address, @ref mt6835_reg_enum_t
 * @return data
 */
uint8_t mt6835_read_reg(mt6835_t *mt6835, mt6835_reg_enum_t reg) {
    uint8_t result[3] = {0, 0, 0};

    /* data frame */
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
    mt6835->data_frame.cmd = MT6835_CMD_RD; // byte read command
    mt6835->data_frame.reg = reg;

    mt6835->func.spi_cs_control(MT6835_CS_LOW);
    if (mt6835->func.spi_send_recv == NULL ? false : true) {
        mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
    } else {
        mt6835->func.spi_send((uint8_t *)&mt6835->data_frame.pack, 3);
        mt6835->func.spi_recv((uint8_t *)&result, 3);
    }
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);

    return result[2];
}

/**
 * @brief write mt6835 register
 * @param mt6835 mt6835 object
 * @param reg register address, @ref mt6835_reg_enum_t
 * @param data data to write
 */
void mt6835_write_reg(mt6835_t *mt6835, mt6835_reg_enum_t reg, uint8_t data) {
    uint8_t result[3] = {0, 0, 0};

    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
    mt6835->data_frame.cmd = MT6835_CMD_WR; // byte write command
    mt6835->data_frame.reg = reg;
    mt6835->data_frame.normal_byte = data;

    mt6835->func.spi_cs_control(MT6835_CS_LOW);
    mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
}

/**
 * @brief write mt6835 eeprom, you must to write corresponding register first, and then call this function.
 * you need wait 6 seconds after write eeprom
 * @param mt6835 mt6835 object
 * @return true if success, false otherwise
 */
bool mt6835_write_eeprom(mt6835_t *mt6835) {
    uint8_t result[3] = {0, 0, 0xFF};

    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
    mt6835->data_frame.cmd = MT6835_CMD_EEPROM; // byte read command
    mt6835->data_frame.reg = 0;
    mt6835->data_frame.normal_byte = 0;

    mt6835->func.spi_cs_control(MT6835_CS_LOW);
    mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);

    if (result[2] == 0xFF || result[2] != 0x55) {
        return false;
    }
    return true;
}
