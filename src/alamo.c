#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "nRF24L01.h"
#include "swspi.h"
#include "usart.h"
#include "mirf.h"

#define NANO 1
#define MASTER 0

char OUTPUT_BUFFER[64];
uint8_t buffersize = 8;
uint8_t buffer[8];

uint8_t testdata[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

ISR (INT0_vect) {
    serial_write_str("THERE IS DATA!\n");
    memset(buffer, 0, sizeof(uint8_t) * buffersize);
    mirf_read(buffer, buffersize);
    sprintf(OUTPUT_BUFFER, "DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n", 
            buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    serial_write_str (OUTPUT_BUFFER);
}

void blink1(void) {
    LED_PORT |= _BV(LED_PIN);
    _delay_ms(100);
    LED_PORT &= ~_BV(LED_PIN);
    _delay_ms(100);
}

void blink3(void) {
    LED_PORT |= _BV(LED_PIN);
    _delay_ms(50);
    LED_PORT &= ~_BV(LED_PIN);
    _delay_ms(50);
    LED_PORT |= _BV(LED_PIN);
    _delay_ms(50);
    LED_PORT &= ~_BV(LED_PIN);
    _delay_ms(50);
    LED_PORT |= _BV(LED_PIN);
    _delay_ms(50);
    LED_PORT &= ~_BV(LED_PIN);
    _delay_ms(50);
}

void blink2(void) {
    LED_PORT &= ~_BV(LED_PIN);
    _delay_ms(50);
    LED_PORT |= _BV(LED_PIN);
    _delay_ms(50);
    LED_PORT &= ~_BV(LED_PIN);
}

void printSettings() {

    uint8_t reg;
    uint8_t regbuffer[16];
    
    reg = mirf_ld(EN_AA);
    sprintf(OUTPUT_BUFFER, "EN_AA: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    reg = mirf_ld(EN_RXADDR);
    sprintf(OUTPUT_BUFFER, "EN_RXADDR: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    reg = mirf_ld(SETUP_AW);
    sprintf(OUTPUT_BUFFER, "SETUP_AW: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    reg = mirf_ld(SETUP_RETR);
    sprintf(OUTPUT_BUFFER, "SETUP_RETR: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    reg = mirf_ld(RF_CH);
    sprintf(OUTPUT_BUFFER, "RF_CH: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    reg = mirf_ld(RF_SETUP);
    sprintf(OUTPUT_BUFFER, "RF_SETUP: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    reg = mirf_ld(STATUS);
    sprintf(OUTPUT_BUFFER, "Status: 0x%02x\n", reg);
    serial_write_str(OUTPUT_BUFFER);
    
    memset(regbuffer, 0, 16 * sizeof(uint8_t));
    mirf_ldm(TX_ADDR, regbuffer, 5);
    sprintf(OUTPUT_BUFFER, "TX_ADDR: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", 
            regbuffer[0], regbuffer[1], regbuffer[2], regbuffer[3], regbuffer[4]);
    serial_write_str(OUTPUT_BUFFER);

    memset(regbuffer, 0, 16 * sizeof(uint8_t));
    mirf_ldm(RX_ADDR_P0, regbuffer, 5);
    sprintf(OUTPUT_BUFFER, "RX_ADDR_P0: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", 
            regbuffer[0], regbuffer[1], regbuffer[2], regbuffer[3], regbuffer[4]);
    serial_write_str(OUTPUT_BUFFER);
    
    memset(regbuffer, 0, 16 * sizeof(uint8_t));
    mirf_ldm(RX_ADDR_P1, regbuffer, 5);
    sprintf(OUTPUT_BUFFER, "RX_ADDR_P1: %02x %02x %02x %02x %02x\n", 
            regbuffer[0], regbuffer[1], regbuffer[2], regbuffer[3], regbuffer[4]);
    serial_write_str(OUTPUT_BUFFER);
}

void init(void) {

    // Safe sleep and signal init
    _delay_ms(3000);
    blink1();
    _delay_ms(100);
    blink1();
    _delay_ms(100);
    blink1();

    // Set up I/O
    MISO_DDR &= ~_BV(MISO_PIN);
    IRQ_DDR &= ~_BV(IRQ_PIN);
    MOSI_DDR |= _BV(MOSI_PIN);
    SCLK_DDR |= _BV(SCLK_PIN);
    CE_DDR |= _BV(CE_PIN);
    CSN_DDR |= _BV(CSN_PIN);
    
    // Set up interrupts
    EICRA |= _BV(ISC10);
    EIMSK |= _BV(INT0);
    
    // initialize USART
    USART_Init((F_CPU / (USART_BAUDRATE * 16UL)) - 1);
    serial_write_str("Begin\n");

    // Initialize AVR for use with mirf
    mirf_init();
    
    printSettings();
}

int main(void) {

    // Initialization
    init();

    
    if (!MASTER) {
    
        uint8_t addr[5] = mirf_RX_ADDR;
        mirf_listen(addr);

        while(!MASTER) {
        
            serial_write_str("RECEIVING DATA...\n");
            blink2();
            
            _delay_ms(1000);
        }
    }
    
    while(MASTER) {
        
        memcpy(buffer, testdata, 8 * sizeof(uint8_t));

        serial_write_str("SENDING DATA...\n");
        sprintf(OUTPUT_BUFFER, "DATA: %02x %02x %02x %02x %02x %02x %02x %02x\n", 
            buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
        serial_write_str (OUTPUT_BUFFER);
 
        blink2();
        uint8_t addr[5] = mirf_RX_ADDR;
        uint8_t res = mirf_write(addr, buffer, buffersize);
        
        switch (res) {
        case RESULT_SUCCESS:
            serial_write_str("DATA SENT SUCCESS!\n");
            int i = 10;
            while(i--) {
                blink2();
                _delay_ms(50);
            }
            break;

        case RESULT_FAILED:
            serial_write_str("SENDING DATA FAILED\n");
            break;

        case RESULT_TIMEOUT:
            serial_write_str("SENDING DATA TIMED OUT\n");
            break;
        }
        
        _delay_ms(1000);
    }
    
    return 0;
}
