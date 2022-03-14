/*
    File: p1-parser.h

    Prototypes and structs to help parse Dutch Smart Meter P1-telegrams.

    (c) 2017, Levien van Zon (levien at zonnetjes.net, https://github.com/lvzon)
*/

#ifndef DSMR_PARSER_H
#define DSMR_PARSER_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Data structure to hold meter data

#include "dsmr_parser_data.h"

// Default meter timezone is CET (The Netherlands and most of mainland Western Europe)

#define METER_TIMEZONE	"CET-1CEST,M3.5.0/2,M10.5.0/3"

// Parser buffer used to store strings

#define PARSER_BUFLEN 4096

// Parser stack length (maximum number of string/int capture-elements per line)

#define PARSER_MAXARGS 12

// Data structure used by the Ragel parser

typedef struct {
    int cs;                             // Variables needed by Ragel parsers/scanners
    const char *pe;
    char *ts;
    char *te;
    int act;

    char buffer[PARSER_BUFLEN+1];       // String capture buffers
    int buflen;

    int argc;                           // Integer capture stack
    long long arg[PARSER_MAXARGS];
    int multiplier;
    int bitcount;
    int decimalpos;

    int strargc;                        // String capture stack
    char *strarg[PARSER_MAXARGS];

    // Variables specific to the P1-parser

    uint16_t crc16;
    char *meter_timezone;
    int	parse_errors,
        pfaileventcount;

    unsigned int devcount,
                 timeseries_period_minutes;
    uint32_t timeseries_time;

    // Data structure to hold meter data
    dsmr_parser_data_t	*data;
} dsmr_parser_t;


// Lookup table for long long integer powers of ten

# define MAX_DIVIDER_EXP 18

static const long long pow10[MAX_DIVIDER_EXP + 1] = {
        1, 10, 100, 1000, 10000, 100000L, 1000000L, 10000000L, 100000000L, 1000000000L,
        10000000000LL, 100000000000LL, 1000000000000LL, 10000000000000LL, 100000000000000LL,
        1000000000000000LL, 10000000000000000LL, 100000000000000000LL, 1000000000000000000LL};


// Function prototypes

void dsmr_parser_init(dsmr_parser_t *fsm );
void dsmr_parser_execute(dsmr_parser_t *fsm, const char *data, int len, int eofflag);
int dsmr_parser_finish(dsmr_parser_t *fsm);

#ifdef __cplusplus
}
#endif

#endif /* DSMR_PARSER_H */
