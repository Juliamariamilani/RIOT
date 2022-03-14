/*
 * Copyright (C) 2022 Bas Stottelaar <basstottelaar@gmail.com>
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
 * @brief       DSMR splitter application.
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "periph/gpio.h"
#include "periph/uart.h"

#include "board.h"
#include "mutex.h"
#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "shell.h"
#include "shell_commands.h"

#include "dsmr.h"
#include "dsmr_params.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief Buffer definition for storing telegrams.
 */
typedef struct {
    mutex_t lock;
    uint8_t data[DSMR_TELEGRAM_SIZE];
    size_t len;
} buffer_t;

/**
 * @brief LED thread definition
 */
typedef struct {
    char stack[THREAD_STACKSIZE_LARGE];
    kernel_pid_t pid;

    gpio_t led_pin;
} led_thread_t;

/**
 * @brief Receiver thread definition
 */
typedef struct {
    char stack[THREAD_STACKSIZE_LARGE];
    kernel_pid_t pid;

    bool enabled;
    uint32_t interval;

    buffer_t buffer;

    struct {
        uint32_t failed_timeout;
        uint32_t failed_checksum;
        uint32_t failed_other;
        uint32_t read;
    } stats;
} receiver_thread_t;

/**
 * @brief Sender thread definition
 */
typedef struct {
    char stack[THREAD_STACKSIZE_LARGE];
    char name[12];
    kernel_pid_t pid;

    bool enabled;
    uint32_t interval;

    uart_t uart_dev;
    gpio_t rts_pin;

    int rts;

    buffer_t buffer[3];
    uint8_t index;

    struct {
        uint32_t copied;
        uint32_t requested;
        uint32_t aborted;
        uint32_t written;
    } stats;
} sender_thread_t;

static dsmr_t dev;

static led_thread_t led = {
    .led_pin = LED0_PIN
};
static receiver_thread_t receiver = {
    .enabled = true,
    .interval = 1000,
    .buffer = {
        .lock = MUTEX_INIT
    }
};
static sender_thread_t senders[3] = {
    {
        .enabled = true,
        .interval = 1000,
        .uart_dev = UART_DEV(2),
        .rts_pin = GPIO_PIN(PA, 1),
        .buffer = {
            {
                .lock = MUTEX_INIT
            },
            {
                .lock = MUTEX_INIT
            },
            {
                .lock = MUTEX_INIT
            }
        }
    },
    {
        .enabled = true,
        .interval = 1000,
        .uart_dev = UART_DEV(3),
        .rts_pin = GPIO_PIN(PB, 11),
        .buffer = {
            {
                .lock = MUTEX_INIT
            },
            {
                .lock = MUTEX_INIT
            },
            {
                .lock = MUTEX_INIT
            }
        }
    },
    {
        .enabled = true,
        .interval = 1000,
        .uart_dev = UART_DEV(4),
        .rts_pin = GPIO_PIN(PD, 13),
        .buffer = {
            {
                .lock = MUTEX_INIT
            },
            {
                .lock = MUTEX_INIT
            },
            {
                .lock = MUTEX_INIT
            }
        }
    }
};

static int cmd_disable(int argc, char **argv);
static int cmd_dump(int argc, char **argv);
static int cmd_enable(int argc, char **argv);
static int cmd_interval(int argc, char **argv);
static int cmd_stats(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "disable", "Disable port", cmd_disable },
    { "dump", "Dump port buffer", cmd_dump },
    { "enable", "Enable port", cmd_enable },
    { "interval", "Set update interval", cmd_interval },
    { "stats", "DSMR statistics", cmd_stats },
    { NULL, NULL, NULL }
};

static void _cb_gpio(void *arg)
{
    sender_thread_t *sender = (sender_thread_t *)arg;

    int state = gpio_read(sender->rts_pin);

    if (state == 1 && sender->rts == 0) {
        sender->stats.requested++;
    }

    sender->rts = state;
}

static int cmd_disable(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <port 0-%d>\n", argv[0], ARRAY_SIZE(senders));
        return 1;
    }

    unsigned port = (unsigned)atoi(argv[1]);

    if (port >= ARRAY_SIZE(senders) + 1) {
        printf("Error: port out of range.\n");
        return 1;
    }

    if (port == 0) {
        receiver.enabled = false;
    }
    else {
        senders[port - 1].enabled = false;
    }

    return 0;
}

static int cmd_dump(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        printf("Usage: %s <port 0-%d> [<index 0-%d>]\n", argv[0], ARRAY_SIZE(senders), ARRAY_SIZE(senders[0].buffer));
        return 1;
    }

    unsigned port = (unsigned)atoi(argv[1]);

    if (port >= ARRAY_SIZE(senders) + 1) {
        printf("Error: port out of range.\n");
        return 1;
    }

    unsigned index = 0;

    if (argc == 3) {
        index = (unsigned)atoi(argv[2]);

        if (index >= ARRAY_SIZE(senders[0].buffer)) {
            printf("Error: index out of rnage.\n");
            return 1;
        }
    }

    if (port == 0) {
        mutex_lock(&(receiver.buffer.lock));

        for (size_t pos = 0; pos < receiver.buffer.len; pos++) {
            putchar((int)receiver.buffer.data[pos]);
        }

        mutex_unlock(&(receiver.buffer.lock));
    }
    else {
        mutex_lock(&(senders[port - 1].buffer[index].lock));

        for (size_t pos = 0; pos < senders[port - 1].buffer[index].len; pos++) {
            putchar((int)senders[port - 1].buffer[index].data[pos]);
        }

        mutex_unlock(&(senders[port - 1].buffer[index].lock));
    }

    return 0;
}

static int cmd_enable(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <port 0-%d>\n", argv[0], ARRAY_SIZE(senders));
        return 1;
    }

    unsigned port = (unsigned)atoi(argv[1]);

    if (port > ARRAY_SIZE(senders)) {
        printf("Error: port out of range.\n");
        return 1;
    }

    if (port == 0) {
        receiver.enabled = true;
    }
    else {
        senders[port - 1].enabled = true;
    }

    return 0;
}

static int cmd_interval(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <port 0-%d> <interval>\n", argv[0], ARRAY_SIZE(senders));
        return 1;
    }

    unsigned port = (unsigned)atoi(argv[1]);

    if (port >= ARRAY_SIZE(senders) + 1) {
        printf("Error: port out of range.\n");
        return 1;
    }

    unsigned interval = (unsigned)atoi(argv[2]);

    if (port == 0) {
        receiver.interval = interval;
    }
    else {
        senders[port - 1].interval = interval;
    }

    return 0;
}

static int cmd_stats(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("Port 0 (receiver):\n");
    printf("- Enabled: %s\n", receiver.enabled ? "Y" : "N");
    printf("- Interval: %ld ms\n", receiver.interval);
    printf("- Buffer: %d bytes\n", receiver.buffer.len);
    printf("- Telegrams reads: %ld\n", receiver.stats.read);
    printf("- Telegrams reads failed (timeout): %ld\n", receiver.stats.failed_timeout);
    printf("- Telegrams reads failed (checksum): %ld\n", receiver.stats.failed_checksum);
    printf("- Telegrams reads failed (other): %ld\n", receiver.stats.failed_other);

    for (unsigned i = 0; i < ARRAY_SIZE(senders); i++) {
        printf("\n");

        printf("Port %d (sender):\n", i + 1);
        printf("- Enabled: %s\n", senders[i].enabled ? "Y" : "N");
        printf("- Interval: %ld ms\n", senders[i].interval);
        printf("- Request to send: %s\n", senders[i].rts ? "Y" : "N");

        for (unsigned j = 0; j < ARRAY_SIZE(senders[i].buffer); j++) {
            printf("- Buffer %d: %d bytes", j, senders[i].buffer[j].len);

            if (senders[i].index == j) {
                printf(" (current)");
            }

            printf("\n");
        }

        printf("- Telegrams copied: %ld\n", senders[i].stats.copied);
        printf("- Telegrams requested: %ld\n", senders[i].stats.requested);
        printf("- Telegrams writes aborted: %ld\n", senders[i].stats.aborted);
        printf("- Telegrams written: %ld\n", senders[i].stats.written);
    }

    return 0;
}

static void* led_thread(void *arg)
{
    (void)arg;

    msg_t msg;
    msg_t msg_queue[4];

    gpio_init(led.led_pin, GPIO_OUT);
    gpio_set(led.led_pin);

    /* wait for LED messages */
    msg_init_queue(msg_queue, ARRAY_SIZE(msg_queue));

    while (1) {
        msg_receive(&msg);

        gpio_clear(led.led_pin);
        xtimer_msleep(100);
        gpio_set(led.led_pin);
        xtimer_msleep(100);
    }

    return NULL;
}

static void* receiver_thread(void *arg)
{
    (void)arg;

    msg_t msg;

    xtimer_ticks32_t last_wakeup = xtimer_now();

    while (1) {
        if (receiver.enabled) {
            mutex_lock(&(receiver.buffer.lock));

            /* read a new telegram */
            ssize_t res = dsmr_read(&dev, receiver.buffer.data, sizeof(receiver.buffer.data));

            if (res <= 0) {
                DEBUG("[main] receiver_thread: read failed with result %d\n", res);

                if (res == DSMR_ERR_CHECKSUM) {
                    receiver.stats.failed_checksum++;
                }
                else if (res == DSMR_ERR_TIMEOUT) {
                    receiver.stats.failed_timeout++;
                }
                else {
                    receiver.stats.failed_other++;
                }

                mutex_unlock(&(receiver.buffer.lock));
                continue;
            }

            receiver.buffer.len = res;

            DEBUG("[main] receiver_thread: read OK, %d bytes\n", res);
            receiver.stats.read++;

            /* copy to all senders that are currently not busy */
            for (unsigned i = 0; i < ARRAY_SIZE(senders); i++) {
                uint8_t j = (senders[i].index + 1) % ARRAY_SIZE(senders[i].buffer);

                if (mutex_trylock(&(senders[i].buffer[j].lock))) {
                    DEBUG("[main] receiver_thread: copy to sender %d in buffer index %d.\n", i, j);

                    memcpy(senders[i].buffer[j].data, receiver.buffer.data, receiver.buffer.len);

                    senders[i].buffer[j].len = receiver.buffer.len;
                    senders[i].index = j;
                    senders[i].stats.copied++;

                    mutex_unlock(&(senders[i].buffer[j].lock));
                }
            }

            msg_try_send(&msg, led.pid);

            mutex_unlock(&(receiver.buffer.lock));
        }

        /* receive at most 1 telegram read per interval */
        xtimer_periodic_wakeup(&last_wakeup, receiver.interval * US_PER_MS);
    }

    return NULL;
}

static void* sender_thread(void *arg)
{
    sender_thread_t *sender = (sender_thread_t *)arg;

    msg_t msg;

    xtimer_ticks32_t last_wakeup = xtimer_now();

    while (1) {
        uint8_t i = sender->index;

        if (sender->enabled && sender->rts && sender->buffer[i].len > 0) {
            DEBUG("[main] sender_thread: telegram requested.\n");

            mutex_lock(&(sender->buffer[i].lock));

            size_t pos = 0;

            while (sender->rts && pos < sender->buffer[i].len) {
                uart_write(sender->uart_dev, &(sender->buffer[i].data[pos++]), 1);
            }

            if (pos == sender->buffer[i].len) {
                sender->stats.written++;

                msg_try_send(&msg, led.pid);
                msg_try_send(&msg, led.pid);

                DEBUG("[main] sender_thread: write succeeded.\n");
            }
            else {
                sender->stats.aborted++;

                msg_try_send(&msg, led.pid);

                DEBUG("[main] sender_thread: write aborted, %d bytes remaining.\n", sender->buffer[i].len - pos);
            }

            /* ensure this telegram is not sent twice */
            sender->buffer[i].len = 0;

            mutex_unlock(&(sender->buffer[i].lock));
        }

        /* send at most 1 telegram per interval */
        xtimer_periodic_wakeup(&last_wakeup, sender->interval * US_PER_MS);
    }

    return NULL;
}

int main(void)
{
    /* initialize led thread */
    led.pid = thread_create(led.stack,
                            sizeof(led.stack),
                            THREAD_PRIORITY_MAIN,
                            THREAD_CREATE_STACKTEST,
                            led_thread,
                            NULL,
                            "led");

    /* initialize receiver */
    int result = dsmr_init(&dev, &dsmr_params[0]);

    if (result != DSMR_OK) {
        printf("DSMR initialization failed with error code %d.\n", result);
        return 1;
    }

    receiver.pid = thread_create(receiver.stack,
                                 sizeof(receiver.stack),
                                 THREAD_PRIORITY_MAIN,
                                 THREAD_CREATE_STACKTEST,
                                 receiver_thread,
                                 NULL,
                                 "receiver");

    /* initialize senders */
    for (unsigned i = 0; i < ARRAY_SIZE(senders); i++) {
        if (gpio_init_int(senders[i].rts_pin, GPIO_IN_PD, GPIO_BOTH, _cb_gpio, &senders[i]) != 0) {
            printf("GPIO initialization for sender %d failed.\n", i);
            return 1;
        }

        senders[i].rts = gpio_read(senders[i].rts_pin);

        uart_poweron(senders[i].uart_dev);

        if (uart_init(senders[i].uart_dev, dsmr_params[0].baudrate, NULL, NULL) != UART_OK) {
            printf("UART initialization for sender %d failed.\n", i);
            return 1;
        }

        sprintf(senders[i].name, "sender %d", i);

        senders[i].pid = thread_create(senders[i].stack,
                                       sizeof(senders[i].stack),
                                       THREAD_PRIORITY_MAIN,
                                       THREAD_CREATE_STACKTEST,
                                       sender_thread,
                                       &senders[i],
                                       senders[i].name);
    }

    /* run shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
