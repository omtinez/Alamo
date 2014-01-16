#include "usart.h"

void USART_Init(unsigned int ubrr)
{
    // Set baud rate
    UBRR0H = (unsigned char) (ubrr >> 8);
    UBRR0L = (unsigned char) ubrr;
    
    // Enable receiver and transmitter
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    // Set frame format: 8data, 1 stop bit
    UCSR0C = (1<<UCSZ00) | (1 << UCSZ01);
}

void USART_Transmit(unsigned char data )
{
    // Wait for empty transmit buffer
    while (!( UCSR0A & (1<<UDRE0)));
    
    // Put data into buffer, sends the data
    UDR0 = data;
}

// Send a null terminated string to the serial port
void serial_write_str(const char* str)
{
    int len = strlen(str);
    int i;
    for (i = 0; i < len; i++) {
        USART_Transmit(str[i]);
    }
}
