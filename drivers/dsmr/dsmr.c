#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dsmr.h"

#include "checksum/ucrc16.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief UART callback to read the telegram and advance state.
 */
static void _cb_uart(void *arg, uint8_t byte)
{
    dsmr_t *dev = arg;

    /* do not process data if not requested */
    if (dev->state == DSMR_STATE_IDLE || dev->state == DSMR_STATE_COMPLETE) {
        return;
    }

    /* a new telegram starts with a forward slash, so synchronize here */
    if (dev->state == DSMR_STATE_SYNCHRONIZING) {
        if (byte == '/') {
            DEBUG("A\n");
            dev->idx = 0;
            dev->state = DSMR_STATE_WAITING_FOR_TELEGRAM;
        }
    }

    /* a telegram ends with a !, after which an optional checksum follows */
    else if (dev->state == DSMR_STATE_WAITING_FOR_TELEGRAM) {
        if (byte == '!') {
            DEBUG("B\n");
            dev->state = DSMR_STATE_WAITING_FOR_CHECKSUM;
        }
    }

    /* the telegram is completely received after the last \r\n is received */
    else if (dev->state == DSMR_STATE_WAITING_FOR_CHECKSUM) {
        if (byte == '\n') {
            DEBUG("C\n");
            dev->state = DSMR_STATE_COMPLETE;
        }
    }

    /* store data in buffer (overflow check for safety) */
    if (dev->idx < dev->len) {
        dev->buf[dev->idx++] = byte;
    }

    /* unlock reader if telegram is complete */
    if (dev->state == DSMR_STATE_COMPLETE) {
        mutex_unlock(&(dev->complete));
    }
}

/**
 * @brief Parse telegram checksum
 */
static bool _parse_checksum(dsmr_t *dev, uint16_t *out)
{
    assert(dev->state == DSMR_STATE_COMPLETE);

    char tmp[5] = { 0 };

    memcpy(&tmp, &(dev->buf[dev->idx - 6]), 4);

    return sscanf((const char *)&(dev->buf[dev->idx - 6]), "%4hX", out) == 1;
}

/**
 * @brief Verify the actual checksum with the expected checksum
 */
static bool _verify_checksum(dsmr_t *dev)
{
    assert(dev->state == DSMR_STATE_COMPLETE);

    /* checksums are optional */
    if (dev->buf[dev->idx - 3] == '!') {
        if (dev->params.checksum == DSMR_CHECKSUM_REQUIRED) {
            DEBUG("[dsmr] _verify_checksum: telegram does not have a checksum, but it is required\n");
            return false;
        }

        DEBUG("[dsmr] _verify_checksum: telegram does not have a checksum, assuming it is valid\n");
        return true;
    }

    /* compare checksums */
    uint16_t expected = 0;
    uint16_t actual = ucrc16_calc_le(dev->buf,
                                     dev->idx - 6,
                                     DSMR_CHECKSUM_POLY,
                                     DSMR_CHECKSUM_SEED);

    if (!_parse_checksum(dev, &expected)) {
        DEBUG("[dsmr] _verify_checksum: parsing checksum failed\n");
        return false;
    }

    return expected == actual;
}

/**
 * @brief Wait for telegram to be completed, or for a read timeout
 */
static void _wait_completion(dsmr_t *dev)
{
    mutex_lock(&(dev->complete));

    xtimer_mutex_lock_timeout(&(dev->complete), DSMR_READ_TIMEOUT);

    mutex_unlock(&(dev->complete));
}

int dsmr_init(dsmr_t *dev, const dsmr_params_t *params)
{
    /* setup descriptor */
    dev->params = *params;

    dev->buf = NULL;
    dev->idx = 0;
    dev->len = 0;

    dev->state = DSMR_STATE_IDLE;

    /* setup locks */
    mutex_init(&(dev->lock));
    mutex_init(&(dev->complete));

    /* initialize RTS pin */
    if (gpio_is_valid(dev->params.rts_pin)) {
        if (gpio_init(dev->params.rts_pin, GPIO_OUT) != 0) {
            DEBUG("[dsmr] dsmr_init: error initializing GPIO\n");
            return DSMR_ERR_INIT;
        }
    }

    /* initialize UART */
    uart_poweron(dev->params.uart_dev);

    if (uart_init(dev->params.uart_dev, dev->params.baudrate, _cb_uart, dev) != UART_OK) {
        DEBUG("[dsmr] dsmr_init: error initializing UART\n");
        return DSMR_ERR_INIT;
    }

    return DSMR_OK;
}

int dsmr_read(dsmr_t *dev, uint8_t *out, size_t len)
{
    assert(out != NULL);

    mutex_lock(&(dev->lock));

    dev->buf = out;
    dev->len = len;

    dev->state = DSMR_STATE_SYNCHRONIZING;

    /* toggle RTS to instruct the meter to start sending telegrams */
    DEBUG("[dsmr] dsmr_read: start reading\n");

    if (gpio_is_valid(dev->params.rts_pin)) {
        gpio_set(dev->params.rts_pin);
    }

    _wait_completion(dev);

    if (gpio_is_valid(dev->params.rts_pin)) {
        gpio_clear(dev->params.rts_pin);
    }

    /* check if telegram was completed, or if a timeout occurred */
    if (dev->state != DSMR_STATE_COMPLETE) {
        dev->state = DSMR_STATE_IDLE;

        mutex_unlock(&(dev->lock));

        return DSMR_ERR_TIMEOUT;
    }

    DEBUG("[dsmr] dsmr_read: telegram completed\n");

    /* validate checksum */
    if (!_verify_checksum(dev)) {
        dev->state = DSMR_STATE_IDLE;

        mutex_unlock(&(dev->lock));

        return DSMR_ERR_CHECKSUM;
    }

    DEBUG("[dsmr] dsmr_read: telegram checksum valid\n");

    int result = dev->idx;

    dev->state = DSMR_STATE_IDLE;

    mutex_unlock(&(dev->lock));

    return result;
}
