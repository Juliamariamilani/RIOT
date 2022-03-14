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
 * @brief       Configuration of CPU peripherals for the DSMR Splitter board
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "cpu.h"
#include "periph_cpu.h"
#include "em_cmu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Clock configuration
 * @{
 */
#ifndef CLOCK_HF
#define CLOCK_HF            cmuSelect_HFXO
#endif
#ifndef CLOCK_CORE_DIV
#define CLOCK_CORE_DIV      cmuClkDiv_1
#endif
#ifndef CLOCK_LFA
#define CLOCK_LFA           cmuSelect_LFRCO
#endif
#ifndef CLOCK_LFB
#define CLOCK_LFB           cmuSelect_HFCLKLE
#endif
#ifndef CLOCK_LFE
#define CLOCK_LFE           cmuSelect_LFRCO
#endif
/** @} */

/**
 * @name    DC-DC configuration
 * @{
 */
#define EMU_DCDCINIT_OFF
/** @} */

/**
 * @name    ADC configuration
 * @{
 */
static const adc_conf_t adc_config[] = {
    {
        .dev = ADC0,
        .cmu = cmuClock_ADC0,
    }
};

static const adc_chan_conf_t adc_channel_config[] = {
    {
        .dev = 0,
        .input = adcPosSelTEMP,
        .reference = adcRef1V25,
        .acq_time = adcAcqTime8
    },
    {
        .dev = 0,
        .input = adcPosSelAVDD,
        .reference = adcRef5V,
        .acq_time = adcAcqTime8
    }
};

#define ADC_DEV_NUMOF       ARRAY_SIZE(adc_config)
#define ADC_NUMOF           ARRAY_SIZE(adc_channel_config)
/** @} */

/**
 * @name    Timer configuration
 *
 * The implementation uses two timers in cascade mode.
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        .prescaler = {
            .dev = WTIMER0,
            .cmu = cmuClock_WTIMER0
        },
        .timer = {
            .dev = WTIMER1,
            .cmu = cmuClock_WTIMER1
        },
        .irq = WTIMER1_IRQn,
        .channel_numof = 3
    },
    {
        .prescaler = {
            .dev = TIMER0,
            .cmu = cmuClock_TIMER0
        },
        .timer = {
            .dev = TIMER1,
            .cmu = cmuClock_TIMER1
        },
        .irq = TIMER1_IRQn,
        .channel_numof = 3
    },
    {
        .prescaler = {
            .dev = NULL,
            .cmu = cmuClock_LETIMER0
        },
        .timer = {
            .dev = LETIMER0,
            .cmu = cmuClock_LETIMER0
        },
        .irq = LETIMER0_IRQn,
        .channel_numof = 2
    }
};

#define TIMER_NUMOF         ARRAY_SIZE(timer_config)
#define TIMER_0_ISR         isr_wtimer1
#define TIMER_1_ISR         isr_timer1
#define TIMER_2_ISR         isr_letimer0
/** @} */

/**
 * @name    UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        .dev = LEUART0,
        .rx_pin = GPIO_PIN(PA, 3),
        .tx_pin = GPIO_PIN(PA, 2),
        .loc = LEUART_ROUTELOC0_RXLOC_LOC2 |
               LEUART_ROUTELOC0_TXLOC_LOC2,
        .cmu = cmuClock_LEUART0,
        .irq = LEUART0_IRQn
    },
    {
        .dev = USART0,
        .rx_pin = GPIO_PIN(PD, 14),
        .tx_pin = GPIO_PIN(PC, 6),
        .loc = USART_ROUTELOC0_RXLOC_LOC21 |
               USART_ROUTELOC0_TXLOC_LOC10,
        .cmu = cmuClock_USART0,
        .irq = USART0_RX_IRQn
    },
    {
        .dev = USART1,
        .rx_pin = GPIO_PIN(PC, 7),
        .tx_pin = GPIO_PIN(PA, 0),
        .loc = USART_ROUTELOC0_RXLOC_LOC11 |
               USART_ROUTELOC0_TXLOC_LOC0,
        .cmu = cmuClock_USART1,
        .irq = USART1_RX_IRQn
    },
    {
        .dev = USART2,
        .rx_pin = GPIO_PIN(PF, 7),
        .tx_pin = GPIO_PIN(PA, 5),
        .loc = USART_ROUTELOC0_RXLOC_LOC19 |
               USART_ROUTELOC0_TXLOC_LOC0,
        .cmu = cmuClock_USART2,
        .irq = USART2_RX_IRQn
    },
    {
        .dev = USART3,
        .rx_pin = GPIO_PIN(PD, 11),
        .tx_pin = GPIO_PIN(PD, 12),
        .loc = USART_ROUTELOC0_RXLOC_LOC2 |
               USART_ROUTELOC0_TXLOC_LOC4,
        .cmu = cmuClock_USART3,
        .irq = USART3_RX_IRQn
    },
};

#define UART_NUMOF          ARRAY_SIZE(uart_config)
#define UART_0_ISR_RX       isr_leuart0
#define UART_1_ISR_RX       isr_usart0_rx
#define UART_2_ISR_RX       isr_usart1_rx
#define UART_3_ISR_RX       isr_usart2_rx
#define UART_4_ISR_RX       isr_usart3_rx
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
