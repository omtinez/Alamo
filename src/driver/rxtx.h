#ifndef _rxtx_H_
#define _rxtx_H_

#include <avr/io.h>

// rxtx settings
#define rxtx_TX_ADDR { 0xF2, 0xF2, 0xF2, 0xF2, 0xF2 }
#define rxtx_RX_ADDR { 0xF2, 0xF2, 0xF2, 0xF2, 0xF2 }

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
void rxtx_init();
uint8_t rxtx_ready();
void rxtx_listen(uint8_t* addr);
uint8_t rxtx_write(uint8_t* addr, uint8_t* data, uint8_t len);
void rxtx_read(uint8_t* data, uint8_t len);

// Register related functions
uint8_t rxtx_ld(uint8_t reg);
uint8_t rxtx_st(uint8_t reg, uint8_t value);
uint8_t rxtx_ldm(uint8_t reg, uint8_t* data, uint8_t len);
uint8_t rxtx_stm(uint8_t reg, uint8_t* data, uint8_t len);

#endif /* _rxtx_H_ */
