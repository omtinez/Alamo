#include "txrx.h"
#include "nRF24L01.h"
#include "swspi.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// tmp
#include "usart.h"


// maintain a copy of the config register
volatile uint8_t config;

// Initialize all the default settings of the RF module and I/O pins of AVR
void txrx_init() {

    // Make sure that interrupts are disabled
    cli();

    // Shut down during config
    txrx_st(CONFIG, 0);
    
    // Set CSN and CE to default
    CHIP_ENABLE_LO();
    CHIP_SELECT_HI();
    
    // Enable auto-acknowledgement
    txrx_st(EN_AA, 0x01);
    
    // Configure the retry wait time and number of attempts
    txrx_st(SETUP_RETR, 0x3F); // 3 -> 250 + 250 * X us, F -> 15 attempts
    
    // Choose number of enabled data pipes
    txrx_st(EN_RXADDR, 0x01);
    
    // RF address width setup
    txrx_st(SETUP_AW, 0x03); // 0b0000 0011 -> 5 bytes RF address
    
    // RF channel setup
    txrx_st(RF_CH, 0x01); // 0b0000 0001 -> 2,400-2,527GHz
    
    // Power mode and data speed
    txrx_st(RF_SETUP, 0x26); // 0b0010 0110 -> 250kbps 0dBm (last bit means nothing)
    
    // Set length of payload 
    txrx_st(RX_PW_P0, 8);
    
    // Clear all status flags
    txrx_st(STATUS, _BV(RX_DR));
    txrx_st(STATUS, _BV(TX_DS));
    txrx_st(STATUS, _BV(MAX_RT));

    // initialize config
    config = 0x00;
    
    // Enable CRC (forced if EN_AA is enabled)
    config |= _BV(EN_CRC);
    
    // Double byte CRC encoding scheme
    config |= _BV(CRCO);
    
    // Receiver mode by default

    // Start up the module
    txrx_st(CONFIG, config | _BV(PWR_UP));
}

// Load value from the specified register
uint8_t txrx_ld(uint8_t reg) {
    uint8_t value;
    CHIP_SELECT_LO();
    spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    value = spi_transfer(NOP);
    CHIP_SELECT_HI();
    return value;
}

// Stores the given value to a specified register
uint8_t txrx_st(uint8_t reg, uint8_t value) {
    uint8_t status;
    CHIP_SELECT_LO();
    status = spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    spi_transfer(value);
    CHIP_SELECT_HI();
    return status;
}

// Loads multiple values from the given start position in the specified register
uint8_t txrx_ldm(uint8_t reg, uint8_t * value, uint8_t len) {
    uint8_t status;
    CHIP_SELECT_LO();
    status = spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    _delay_us(10);
    spi_ntransfer(value, value, len);
    CHIP_SELECT_HI();
    return status;
}

// Stores multiple values at the given start position of the specified register
uint8_t txrx_stm(uint8_t reg, uint8_t* values, uint8_t len) {
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

void txrx_listen(uint8_t* addr) {

    // Set receiver address
    txrx_stm(RX_ADDR_P0, addr, 5); // on P0 to match EN_RXADDR setting

    // Power up the radio as a receiver
    config |= _BV(PRIM_RX);
    txrx_st(CONFIG, config | _BV(PWR_UP));
    
    // Enter RX mode
    CHIP_ENABLE_HI();
    
    // Enable interrupts
    sei();
}

// Returns true if there is data waiting at incoming queue
uint8_t txrx_ready() {
    uint8_t status;
    CHIP_SELECT_LO();
    status = spi_transfer(NOP);
    CHIP_SELECT_HI();
    return status & _BV(RX_DR);
}

// Reads len bytes from incoming queue into data array
void txrx_read(uint8_t * data, uint8_t len) {

    // Read packet from FIFO queue
    CHIP_SELECT_LO();
    spi_transfer(R_RX_PAYLOAD);
    _delay_us(10);
    spi_ntransfer(data, data, len);
    CHIP_SELECT_HI();
    
    // Clear the incoming package flag
    txrx_st(STATUS, _BV(RX_DR));
    
    // Flush the FIFO queue
    CHIP_SELECT_LO();
    spi_transfer(FLUSH_TX);
    CHIP_SELECT_HI();
}

// Send data from a buffer to the pre-configured receiver address
uint8_t txrx_write(uint8_t* addr, uint8_t* data, uint8_t len) {

    // Flush data from TX queue
    CHIP_SELECT_LO();
    spi_transfer(FLUSH_TX);
    CHIP_SELECT_HI();

    // Flush data from RX queue
    CHIP_SELECT_LO();
    spi_transfer(FLUSH_RX);
    CHIP_SELECT_HI();
    
    // Clear flags from any previous transmission
    txrx_st(STATUS, _BV(TX_DS) | _BV(MAX_RT));
    
    // Enable auto-acknowledgement
    txrx_st(EN_AA, 0x01);
    
    // Set transmitter address (where we are transmitting to)
    txrx_stm(TX_ADDR, addr, 5);

    // Set receiver address (same as TX_ADDR since EN_AA is set)
    txrx_stm(RX_ADDR_P0, addr, 5);
    
    // Send payload to RF module
    CHIP_SELECT_LO();
    spi_transfer(W_TX_PAYLOAD);
    _delay_us(10);
    spi_ntransfer(data, data, len);
    CHIP_SELECT_HI();

    // Power up the radio as a transmitter
    config &= ~_BV(PRIM_RX);
    txrx_st(CONFIG, config | _BV(PWR_UP));
    _delay_ms(100);
        
    // Set ready flag
    CHIP_ENABLE_HI();
    
    // Wait for transmission to finish
    int timeout = 500;
    uint8_t status, ret = RESULT_TIMEOUT;
    while (timeout > 0) {
        status = txrx_ld(STATUS);
        if (status & _BV(MAX_RT)) {
            txrx_st(STATUS, _BV(MAX_RT));
            ret = RESULT_FAILED;
            break;
        } else if (status & _BV(TX_DS)) {
            txrx_st(STATUS, _BV(TX_DS));
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
