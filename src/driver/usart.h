#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
 
#define USART_BAUDRATE 9600

void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
void serial_write_str(const char* str);

#endif /* _USART_H_ */
