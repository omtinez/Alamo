#ifndef _COMMON_SWSPI_H
#define _COMMON_SWSPI_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "nRF24L01.h"

#define SWSPI_MODE SPI_MODE0
#define SWSPI_DIV SPI_CLOCK_DIV2

#define SPI_CLOCK_DIV4 4
#define SPI_CLOCK_DIV16 16
#define SPI_CLOCK_DIV64 64
#define SPI_CLOCK_DIV128 128
#define SPI_CLOCK_DIV2 2
#define SPI_CLOCK_DIV8 8
#define SPI_CLOCK_DIV32 32
#define SPI_CLOCK_DIV64_2 64

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

uint8_t spi_transfer(uint8_t data);
uint8_t spi_ntransfer(const uint8_t *req, uint8_t *resp, int resplen);
#define spi_interrupt_on()
#define spi_interrupt_off()

// for nCS (nSEL) select of device before sending
#define BEGIN_SPI_SENDING(PORT, PIN) do { \
        (PORT) &= ~_BV(PIN); \
        _delay_loop_1(4); \
} while (0)

// for nCS (nSEL) unselect of device after sending
#define END_SPI_SENDING(PORT, PIN) do { \
        _delay_loop_1(4); \
        (PORT) |= _BV(PIN); \
} while (0)

#endif /* !_COMMON_SWSPI_H*/