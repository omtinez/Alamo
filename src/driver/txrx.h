#ifndef _txrx_H_
#define _txrx_H_

#include <avr/io.h>

// txrx settings
#define txrx_TX_ADDR { 0xF2, 0xF2, 0xF2, 0xF2, 0xF2 }
#define txrx_RX_ADDR { 0xF2, 0xF2, 0xF2, 0xF2, 0xF2 }

// CSN high
#define CHIP_SELECT_HI() do { \
    _delay_us(10); \
    PORTC |= _BV(CSN_PIN); \
    _delay_us(10); \
} while(0);

// CSN low
#define CHIP_SELECT_LO() do { \
    _delay_us(10); \
    PORTC &= ~_BV(CSN_PIN); \
    _delay_us(10); \
} while(0);

// CE high
#define CHIP_ENABLE_HI() do { \
    _delay_us(10); \
    CE_PORT |= _BV(CE_PIN); \
    _delay_us(10); \
} while(0)

// CE low
#define CHIP_ENABLE_LO() do { \
    _delay_us(10); \
    CE_PORT &= ~_BV(CE_PIN); \
    _delay_us(10); \
} while(0)

// Definitions used for result codes
#define RESULT_SUCCESS  0
#define RESULT_FAILED   1
#define RESULT_TIMEOUT  2

// Init and I/O functions
void txrx_init();
uint8_t txrx_ready();
void txrx_listen(uint8_t* addr);
uint8_t txrx_write(uint8_t* addr, uint8_t* data, uint8_t len);
void txrx_read(uint8_t* data, uint8_t len);

// Register related functions
uint8_t txrx_ld(uint8_t reg);
uint8_t txrx_st(uint8_t reg, uint8_t value);
uint8_t txrx_ldm(uint8_t reg, uint8_t* data, uint8_t len);
uint8_t txrx_stm(uint8_t reg, uint8_t* data, uint8_t len);

#endif /* _txrx_H_ */
