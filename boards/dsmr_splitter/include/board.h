/*
 * Copyright (C) 2015-2022 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_dsmr_splitter
 * @{
 *
 * @file
 * @brief       Board specific definitions for the DSMR Splitter board
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "periph_conf.h"
#include "periph/adc.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Xtimer configuration
 *
 * The timer runs at 1000 kHz to increase accuracy, or at 32.768 kHz if
 * LETIMER is used.
 * @{
 */
#if IS_ACTIVE(CONFIG_EFM32_XTIMER_USE_LETIMER)
#define XTIMER_DEV          (TIMER_DEV(2))
#define XTIMER_HZ           (32768UL)
#define XTIMER_WIDTH        (16)
#else
#define XTIMER_DEV          (TIMER_DEV(0))
#define XTIMER_HZ           (1000000UL)
#define XTIMER_WIDTH        (32)
#endif
#define XTIMER_CHAN         (0)
/** @} */

/**
 * @name    Push button pin definitions
 * @{
 */
#define PB0_PIN             GPIO_UNDEF
#define PB1_PIN             GPIO_UNDEF
/** @} */

/**
 * @name    LED pin definitions
 * @{
 */
#define LED0_PIN            GPIO_PIN(PC, 11)
#define LED1_PIN            GPIO_UNDEF
/** @} */

/**
 * @name    Macros for controlling the on-board LEDs
 * @{
 */
#define LED0_ON             gpio_set(LED0_PIN)
#define LED0_OFF            gpio_clear(LED0_PIN)
#define LED0_TOGGLE         gpio_toggle(LED0_PIN)
#define LED1_ON             gpio_set(LED1_PIN)
#define LED1_OFF            gpio_clear(LED1_PIN)
#define LED1_TOGGLE         gpio_toggle(LED1_PIN)
/** @} */

/**
 * @name    Core temperature sensor configuration
 *
 * Connection to the on-chip temperature sensor.
 * @{
 */
#define CORETEMP_ADC        ADC_LINE(0)
/** @} */

/**
 * @brief   Initialize the board (GPIO, sensors, clocks).
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
