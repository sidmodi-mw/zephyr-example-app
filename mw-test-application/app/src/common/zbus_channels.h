#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/zbus/zbus.h>

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
#define MSG_TO_VERSION_MSG( _msg ) ( (struct version_msg *)_msg )

enum button_press
{
    BUTTON_PRESS,
    BUTTON_RELEASE
};

enum button_name
{
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4
};

struct button_msg
{
    enum button_name  button;
    enum button_press press;
};
#define MSG_TO_BUTTON_MSG( _msg ) ( (struct button_msg *)_msg )

#define UART_MESSAGE_BUFFER_SIZE 256
struct uart_msg
{
    unsigned int len;
    uint8_t      data[UART_MESSAGE_BUFFER_SIZE];
};

#define MSG_TO_UART_MSG( _msg ) ( (struct uart_msg *)_msg )

ZBUS_CHAN_DECLARE( BUTTON_CHAN, UART_CHAN );
