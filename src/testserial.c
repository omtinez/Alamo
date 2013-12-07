#include "usart.h"

int main (void)
{
  // Put an LED on PD4 (pin 6)
// PD4 as output
//DDRD |= (1<<4);
//Set high
//PORTD |= (1<<4);
  
  // initialize USART
  USART_Init((F_CPU / (USART_BAUDRATE * 16UL)) - 1);

  // loop
  int count = 10;
  while (1) {

    _delay_ms(500);
                
    //PORTD ^= (1<<4);

    // print count
    char str[16];
    sprintf(str, "%d\n", count);
    if(count) {
      serial_write_str(str);
      count--;
    }
    else {
      serial_write_str("BOOM\n");
      count = 10;
    }

  }
 
  return 1;
}
