/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example for demonstrating SAUL and the SAUL registry
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "dsmr.h"
#include "dsmr_params.h"

static uint8_t telegram[DSMR_TELEGRAM_SIZE];
static dsmr_t dev;

int main(void)
{
    puts("DSMR test application");

    /* initialize the driver */
    puts("Initializing driver.\n");

    int result = dsmr_init(&dev, &dsmr_params[0]);

    if (result != DSMR_OK) {
        printf("Initialize failed with error code %d.\n", result);
        return 1;
    }

    /* continuously perform a read */
    while (1) {
        puts("Reading data.");

        result = dsmr_read(&dev, telegram, sizeof(telegram));

        if (result < 0) {
            printf("Read failed with error code %d.\n", result);
            continue;
        }

        printf("Read completed, telegram is %d bytes.\n", result);
    }

    return 0;
}
