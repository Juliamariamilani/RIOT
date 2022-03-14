/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_dsmr
 *
 * @{
 * @file
 * @brief       Default configuration for DSMR device driver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef DSMR_PARAMS_H
#define DSMR_PARAMS_H

#include "board.h"
#include "dsmr.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the DSMR device driver
 *
 * The default values assume a DSMR 5.0 meter (115200 baud) that has the RTS
 * line pulled up.
 *
 * @{
 */
#ifndef DSMR_PARAM_UART_DEV
#define DSMR_PARAM_UART_DEV     UART_DEV(1)
#endif
#ifndef DSMR_PARAM_RTS_PIN
#define DSMR_PARAM_RTS_PIN      GPIO_PIN(PD, 15)
#endif
#ifndef DSMR_PARAM_BAUDRATE
#define DSMR_PARAM_BAUDRATE     115200
#endif
#ifndef DSMR_PARAM_VERSION
#define DSMR_PARAM_VERSION      DSMR_VERSION_5_0
#endif
#ifndef DSMR_PARAM_CHECKSUM
#define DSMR_PARAM_CHECKSUM     DSMR_CHECKSUM_REQUIRED
#endif

#ifndef DSMR_PARAMS
#define DSMR_PARAMS             { .uart_dev = DSMR_PARAM_UART_DEV, \
                                  .rts_pin = DSMR_PARAM_RTS_PIN, \
                                  .baudrate = DSMR_PARAM_BAUDRATE, \
                                  .version = DSMR_PARAM_VERSION, \
                                  .checksum = DSMR_PARAM_CHECKSUM }
#endif
/**@}*/

/**
 * @brief   Parameter struct for DSMR device driver
 */
static const dsmr_params_t dsmr_params[] =
{
    DSMR_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* DSMR_PARAMS_H */
/** @} */
