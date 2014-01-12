#ifndef _MIRF_H_
#define _MIRF_H_

#include <avr/io.h>

// Mirf settings
#define mirf_TX_ADDR { 0xF2, 0xF2, 0xF2, 0xF2, 0xF2 }
#define mirf_RX_ADDR { 0xF2, 0xF2, 0xF2, 0xF2, 0xF2 }

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

// Reboot RF module and enter TX mode
#define TRANSMIT_MODE_ON() do { \
        config &= ~_BV(PRIM_RX); \
        mirf_st(CONFIG, config); \
        _delay_us(10); \
        mirf_st(CONFIG, config | _BV(PWR_UP)); \
        _delay_ms(100); \
} while (0)

// Exit TX mode and reboot RF module
#define TRANSMIT_MODE_OFF() do { \
        config |= _BV(PRIM_RX); \
        mirf_st(CONFIG, config & ~_BV(PWR_UP)); \
        _delay_us(10); \
        mirf_st(CONFIG, config | _BV(PWR_UP)); \
        _delay_ms(100); \
} while (0)

// Definitions used for result codes
#define RESULT_SUCCESS  0
#define RESULT_FAILED   1
#define RESULT_TIMEOUT  2

// Init and I/O functions
void mirf_init();
uint8_t mirf_ready();
void mirf_listen(uint8_t* addr);
uint8_t mirf_write(uint8_t* addr, uint8_t* data, uint8_t len);
void mirf_read(uint8_t* data, uint8_t len);

// Register related functions
uint8_t mirf_ld(uint8_t reg);
uint8_t mirf_st(uint8_t reg, uint8_t value);
uint8_t mirf_ldm(uint8_t reg, uint8_t* value, uint8_t len);
uint8_t mirf_stm(uint8_t reg, uint8_t* value, uint8_t len);

#endif /* _MIRF_H_ */
