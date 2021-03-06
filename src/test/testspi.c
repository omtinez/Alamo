#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#include "swspi.h"

#define setbit(P, B) ((P) |= (_BV(B)))
#define clibit(P, B) ((P) &= ~(_BV(B)))
#define getbit(P, B) (((P) & (_BV(B)))? 1:0)

#define SETMOSI() setbit(SWSPI_MOSI_PORT, SWSPI_MOSI_PIN)
#define CLIMOSI() clibit(SWSPI_MOSI_PORT, SWSPI_MOSI_PIN)
#define NOMOSI() setbit(SWSPI_MOSI_PORT, SWSPI_MOSI_PIN)
#define READMISO() getbit(SWSPI_MISO_PORT, SWSPI_MISO_PIN)

void blink(void) {
    PORTB &= ~_BV(PB5);
    _delay_ms(100);
    PORTB |= _BV(PB5);
    _delay_ms(100);
    PORTB &= ~_BV(PB5);
}

int main(void) {

    // Configure input/output pins
    //DDRB = (1<<PB5); // LED
    DDRC &= ~_BV(SWSPI_MISO_PIN);
    DDRC |= _BV(SWSPI_MOSI_PIN)|_BV(SWSPI_SCLK_PIN);
    USART_Init(F_CPU/16/BAUD-1);
    
    int i;
    char output[32];
    for (i = 0; i < 10; i++) {
        _delay_ms(1000);
        uint8_t out = i;
        uint8_t in = 0;
        in = spi_transfer(out);
        
        if (in == out) {
            sprintf(output, "SPI echo test %d pass\n", i);
            serial_write_str (output);
            blink();
        } else {
            sprintf(output, "SPI echo test %d fail\n", i);
            serial_write_str (output);
            blink();
            blink();
        }
    }
    /*
    // Activate interrupts
    sei();
    
    // Configure mirf
    mirf_config();
     
    while(1) {
        blink();
        mirf_send(buffer,buffersize);
        _delay_ms(1000);
        
        // If maximum retries were reached, reset MAX_RT
        //if (mirf_max_rt_reached()) {
            mirf_config_register(STATUS, 1<<MAX_RT);
        //}
    }

/*
    // Test communication
    mirf_send(buffer,buffersize);
    while (!mirf_data_ready());
    mirf_get_data(buffer);
*/
    return 0;
}
