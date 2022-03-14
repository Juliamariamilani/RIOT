/*
 * Copyright (C) 2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_dsmr Dutch Smart Meter (DSMR) device driver
 * @ingroup     drivers_sensors
 * @brief       Driver to read telegrams from a DSMR device driver.
 *
 * The Dutch Smart Meter (DSMR, Slimme Meter) is a utility meter installed in
 * buildings in The Netherlands and Belgium. These meters have a dedicated port
 * for (home) owners to read energy consumption, a so-called P1 port. Besides
 * energy, the meters can also report gas or water consumption, depending on
 * the connected sub-devices via M-Bus.
 *
 * The telegrams are formatted according to IEC 62056-21.
 *
 * This driver only reads and validates telegrams. It does not parse the
 * telegrams. It is up to the end-user to choose a parser, which can be a
 * trade-off between parsing features, robustness and memory/flash usage.
 *
 * Note that this device driver does not invert the data line. You need to
 * invert the data line using an inverter.
 *
 * @{
 *
 * @file
 * @brief       Interface definition of the DSMR device driver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef DSMR_H
#define DSMR_H

#include <stddef.h>
#include <stdint.h>

#include "mutex.h"
#include "xtimer.h"

#include "periph/gpio.h"
#include "periph/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DSMR telegram size
 *
 * A typical DSMR 5 telegram is about 1-2 KiB, but it depends on the properties
 * reported by the meter and connected sub-devices.
 */
#ifndef DSMR_TELEGRAM_SIZE
#define DSMR_TELEGRAM_SIZE 2048
#endif

/**
 * @brief DSMR read timeout
 */
#define DSMR_READ_TIMEOUT (5 * US_PER_SEC)

/**
 * @brief DSMR checksum polynomial for CRC calculation
 */
#define DSMR_CHECKSUM_POLY 0xa001

/**
 * @brief DSMR checksum seed for CRC calculation
 */
#define DSMR_CHECKSUM_SEED 0x0000

/**
 * @brief Driver return codes
 */
enum {
    DSMR_OK             = 0,    /**< All OK */
    DSMR_ERR_INIT       = -1,   /**< Initialization error */
    DSMR_ERR_TIMEOUT    = -2,   /**< Reader timeout error */
    DSMR_ERR_CHECKSUM   = -3,   /**< Checksum error */
};

/**
 * @brief Telegram reader states
 */
typedef enum {
    DSMR_STATE_IDLE,                    /**< Nothing to do */
    DSMR_STATE_SYNCHRONIZING,           /**< Syncing to first byte */
    DSMR_STATE_WAITING_FOR_TELEGRAM,    /**< Waiting for telegram */
    DSMR_STATE_WAITING_FOR_CHECKSUM,    /**< Waiting for checksum */
    DSMR_STATE_COMPLETE,                /**< Telegram complete */
} dsmr_state_t;

/**
 * @brief Telegram checksum expectations
 */
typedef enum {
    DSMR_CHECKSUM_OPTIONAL,     /**< Telegram without a checksum is valid */
    DSMR_CHECKSUM_REQUIRED,     /**< Telegram must have a checksum */
} dsmr_checksum_t;

/**
 * @brief DSMR protocol versions
 */
typedef enum {
    DSMR_VERSION_2_0,           /**< DSMR version 2.0 */
    DSMR_VERSION_4_0,           /**< DSMR version 4.0 */
    DSMR_VERSION_4_2,           /**< DSMR version 4.2 */
    DSMR_VERSION_5_0,           /**< DSMR version 5.0 */
} dsmr_version_t;

/**
 * @brief Device initialization parameters.
 */
typedef struct {
    uart_t uart_dev;            /**< I2C bus the sensor is connected to */
    gpio_t rts_pin;             /**< GPIO pin for toggeling RTS */
    uint32_t baudrate;          /**< DSMR baudrate */
    dsmr_version_t version;     /**< DSMR protocol version to use */
    dsmr_checksum_t checksum;   /**< DSMR checksum expectations */
} dsmr_params_t;

/**
 * @brief Device structure.
 */
typedef struct {
    dsmr_params_t params;       /**< Device parameters */

    dsmr_state_t state;         /**< Telegram reader state */

    uint8_t *buf;               /**< Buffer to write telegram to */
    size_t len;                 /**< Buffer length */
    size_t idx;                 /**< Current buffer index */

    mutex_t lock;               /**< Read lock */
    mutex_t complete;           /**< Read complete lock */
} dsmr_t;

/**
 * @brief   Initialize and reset the DSMR device driver
 *
 * @param[in] dev           device descriptor
 * @param[in] params        initialization parameters
 *
 * @return                  DSMR_OK on successful initialization
 * @return                  DSMR_ERR_INIT on initialization error
 */
int dsmr_init(dsmr_t *dev, const dsmr_params_t *params);

/**
 * @brief   Read a telegram from the DSMR device into a user-supplied buffer
 *
 * The driver will toggle the RTS pin to signal the DSMR device to output a
 * telegram. It will then synchronize with the start of the telegram, and read
 * the telegram into @p out. The RTS pin is optional, and can be kept high.
 * However, the driver will only copy bytes when invoking this method.
 *
 * The buffer must be non-null, and sufficient large to contain a complete
 * telegam. @c DSMR_TELEGRAM_SIZE can be used for allocating a byte buffer. If
 * the buffer is too small, the telegram will never complete and result in
 * time-out errors.
 *
 * The buffer might be modified in case of an error.
 *
 * @param[in] dev           device descriptor
 * @param[out] out          non-null buffer to copy telegram into
 * @param[in] len           length of the buffer
 *
 * @return                  number of bytes read into buffer
 * @return                  DSMR_ERR_TIMEOUT on timeout
 * @return                  DSMR_ERR_CHECKSUM on checksum error
 */
ssize_t dsmr_read(dsmr_t *dev, uint8_t *out, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* DSMR_H */
/** @} */
