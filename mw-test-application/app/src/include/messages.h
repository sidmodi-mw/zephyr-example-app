/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

struct version_msg
{
    uint8_t  major;
    uint8_t  minor;
    uint16_t build;
};

struct acc_msg
{
    int x;
    int y;
    int z;
};

enum button_press
{
    BUTTON_PRESS,
    BUTTON_RELEASE,
    BUTTON_INVALID
};

#define UART_MESSAGE_BUFFER_SIZE 256
struct uart_msg
{
    unsigned int len;
    uint8_t      data[UART_MESSAGE_BUFFER_SIZE];
};