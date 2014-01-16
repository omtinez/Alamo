#ifndef _COMMON_SWSPI_H
#define _COMMON_SWSPI_H

#include <avr/io.h>
#include <util/delay.h>
#include "nRF24L01.h"

uint8_t spi_transfer(uint8_t data);
void spi_ntransfer(uint8_t *dataout, uint8_t *datain, int len);

#endif /* !_COMMON_SWSPI_H*/
