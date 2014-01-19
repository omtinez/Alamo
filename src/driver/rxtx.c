#include "rxtx.h"
#include "nRF24L01.h"
#include "swspi.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define CONFIG_REG (0 | _BV(EN_CRC) | _BV(CRCO))

// maintain a copy of the config register
volatile uint8_t config;

// Initialize all the default settings of the RF module and I/O pins of AVR
void rxtx_init() {

    // Make sure that interrupts are disabled
    cli();

    // Shut down during config
    rxtx_st(CONFIG, 0);
    
    // Set CSN and CE to default
    CHIP_ENABLE_LO();
    CHIP_SELECT_HI();
    
    // Enable auto-acknowledgement
    rxtx_st(EN_AA, 0x01);
    
    // Configure the retry wait time and number of attempts
    rxtx_st(SETUP_RETR, 0x3F); // 3 -> 250 + 250 * X us, F -> 15 attempts
    
    // Choose number of enabled data pipes
    rxtx_st(EN_RXADDR, 0x01);
    
    // RF address width setup
    rxtx_st(SETUP_AW, 0x03); // 0b0000 0011 -> 5 bytes RF address
    
    // RF channel setup
    rxtx_st(RF_CH, 0x01); // 0b0000 0001 -> 2,400-2,527GHz
    
    // Power mode and data speed
    rxtx_st(RF_SETUP, 0x26); // 0b0010 0110 -> 250kbps 0dBm (last bit means nothing)
    
    // Set length of payload 
    rxtx_st(RX_PW_P0, 8);
    
    // Clear all status flags
    rxtx_st(STATUS, _BV(RX_DR));
    rxtx_st(STATUS, _BV(TX_DS));
    rxtx_st(STATUS, _BV(MAX_RT));

    // initialize config
    config = 0x00;
    
    // Enable CRC (forced if EN_AA is enabled)
    config |= _BV(EN_CRC);
    
    // Double byte CRC encoding scheme
    config |= _BV(CRCO);

    // Start up the module
    rxtx_st(CONFIG, config | _BV(PWR_UP));
    //rxtx_st(CONFIG, CONFIG_REG | _BV(PWR_UP));
}

// Load value from the specified register
uint8_t rxtx_ld(uint8_t reg) {
    uint8_t value;
    CHIP_SELECT_LO();
    spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    value = spi_transfer(NOP);
    CHIP_SELECT_HI();
    return value;
}

// Stores the given value to a specified register
uint8_t rxtx_st(uint8_t reg, uint8_t value) {
    uint8_t status;
    CHIP_SELECT_LO();
    status = spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    spi_transfer(value);
    CHIP_SELECT_HI();
    return status;
}

// Loads multiple values from the given start position in the specified register
uint8_t rxtx_ldm(uint8_t reg, uint8_t * value, uint8_t len) {
    uint8_t status;
    CHIP_SELECT_LO();
    status = spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    spi_ntransfer(value, value, len);
    CHIP_SELECT_HI();
    return status;
}

// Stores multiple values at the given start position of the specified register
uint8_t rxtx_stm(uint8_t reg, uint8_t* values, uint8_t len) {
    uint8_t status;
    uint8_t data[len];
    memcpy(data, values, len * sizeof(uint8_t));
    CHIP_SELECT_LO();
    status = spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    spi_ntransfer(data, data, len);
    CHIP_SELECT_HI();
    return status;
}

void rxtx_listen(uint8_t* addr) {

    // Set receiver address
    rxtx_stm(RX_ADDR_P0, addr, 5); // on P0 to match EN_RXADDR setting

    // Power up the radio as a receiver
    config |= _BV(PRIM_RX);
    rxtx_st(CONFIG, config | _BV(PWR_UP));
    //rxtx_st(CONFIG, CONFIG_REG | _BV(PRIM_RX) | _BV(PWR_UP));
    
    // Enter RX mode
    CHIP_ENABLE_HI();
    
    // Enable interrupts if settings allow
    #ifdef USE_INTERRUPTS
    sei();
    #endif
}

// Returns true if there is data waiting at incoming queue
uint8_t rxtx_ready() {

    // Read queue status from pin if settings allow
    #ifdef USE_IRQ
    return IRQ_PORT & _BV(IRQ_PIN);
    #endif

    // Otherwise query status register
    uint8_t status;
    CHIP_SELECT_LO();
    status = spi_transfer(NOP);
    CHIP_SELECT_HI();
    return status & _BV(RX_DR);
}

// Reads len bytes from incoming queue into data array
void rxtx_read(uint8_t * data, uint8_t len) {

    // Read packet from FIFO queue
    CHIP_SELECT_LO();
    spi_transfer(R_RX_PAYLOAD);
    _delay_us(10);
    spi_ntransfer(data, data, len);
    CHIP_SELECT_HI();
    
    // Clear the incoming package flag
    rxtx_st(STATUS, _BV(RX_DR));
    
    // Flush the FIFO queue
    CHIP_SELECT_LO();
    spi_transfer(FLUSH_TX);
    CHIP_SELECT_HI();
}

// Send data from a buffer to the pre-configured receiver address
uint8_t rxtx_write(uint8_t* addr, uint8_t* data, uint8_t len) {

    // Flush data from TX queue
    CHIP_SELECT_LO();
    spi_transfer(FLUSH_TX);
    CHIP_SELECT_HI();

    // Flush data from RX queue
    CHIP_SELECT_LO();
    spi_transfer(FLUSH_RX);
    CHIP_SELECT_HI();
    
    // Clear flags from any previous transmission
    rxtx_st(STATUS, _BV(TX_DS) | _BV(MAX_RT));
    
    // Enable auto-acknowledgement
    rxtx_st(EN_AA, 0x01);
    
    // Set transmitter address (where we are transmitting to)
    rxtx_stm(TX_ADDR, addr, 5);

    // Set receiver address (same as TX_ADDR since EN_AA is set)
    rxtx_stm(RX_ADDR_P0, addr, 5);
    
    // Send payload to RF module
    CHIP_SELECT_LO();
    spi_transfer(W_TX_PAYLOAD);
    _delay_us(10);
    spi_ntransfer(data, data, len);
    CHIP_SELECT_HI();

    // Power up the radio as a transmitter
    config &= ~_BV(PRIM_RX);
    rxtx_st(CONFIG, config | _BV(PWR_UP));
    //rxtx_st(CONFIG, (CONFIG_REG | _BV(PWR_UP)) & ~_BV(PRIM_RX));
    _delay_ms(100);
        
    // Set ready flag
    CHIP_ENABLE_HI();
    
    // Wait for transmission to finish
    int timeout = 500;
    uint8_t status, ret = RESULT_TIMEOUT;
    while (timeout > 0) {
        status = rxtx_ld(STATUS);
        if (status & _BV(MAX_RT)) {
            rxtx_st(STATUS, _BV(MAX_RT));
            ret = RESULT_FAILED;
            break;
        } else if (status & _BV(TX_DS)) {
            rxtx_st(STATUS, _BV(TX_DS));
            ret = RESULT_SUCCESS;
            break;
        }
        
        timeout -= 10;
        _delay_ms(10);
    }

    // Finish transmission
    CHIP_ENABLE_LO();
    return ret;
}
