# DSMR Driver Test

## Introduction
This test checks if the Dutch Smart Meter (DSMR) device driver is working
properly.

## Test setup
To test this, wire a RJ-12 connector as follows:

1. Power Out (5V)
2. Request-to-send (RTS)
3. Data GND
4. Not connected
5. Data (TX)
6. Power GND

Note that the signals are 5V and the data line is inverted. The driver does not
account for this. The RTS can be connected to 5V as well.

DSMR 4.0 or older does not have pin 1 and 6.

## Compilation
Override (using `CFLAGS`) any of the following parameters for your setup:

* `DSMR_PARAM_UART_DEV = UART_DEV(1)`
* `DSMR_PARAM_RTS_PIN = GPIO_UNDEF`
* `DSMR_PARAM_BAUDRATE = 115200`
* `DSMR_PARAM_VERSION = DSMR_VERSION_5_0`
* `DSMR_PARAM_CHECKSUM = DSMR_CHECKSUM_REQUIRED`

## Expected result
After initialization, the driver will read telegrams from the DSMR. If the RTS
is specified, it will pull the pin high before reading, and low after reading.

If a telegram is parsed successfully, its length will be printed.
