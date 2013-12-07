#include "swspi.h"
#include <util/delay_basic.h>

#define setbit(P, B) ((P) |= (_BV(B)))
#define clibit(P, B) ((P) &= ~(_BV(B)))
#define getbit(P, B) (((P) & (_BV(B)))? 1:0)

#ifdef SWSPI_DIV
#  define SPIDELAYTIME ((SWSPI_DIV)/2)
#else
#  define SPIDELAYTIME 1
#endif
#define SPIHALFDELAY() _delay_loop_1(SPIDELAYTIME) // half of tact (bit transfer) - min 1 CPU tact

#define SETMOSI() setbit(MOSI_PORT, MOSI_PIN)
#define CLIMOSI() clibit(MOSI_PORT, MOSI_PIN)
#define NOMOSI() setbit(MOSI_PORT, MOSI_PIN)
#define READMISO() getbit(MISO_PORT, MISO_PIN)

#if (2 & SWSPI_MODE)
#  define ONSCLK() clibit(SCLK_PORT, SCLK_PIN)
#  define OFFSCLK() setbit(SCLK_PORT, SCLK_PIN)
#else
#  define ONSCLK() setbit(SCLK_PORT, SCLK_PIN)
#  define OFFSCLK() clibit(SCLK_PORT, SCLK_PIN)
#endif

#if (1 & SWSPI_MODE)
#  define SHIFTBIT(outbyte, inbyte) do { \
    (outbyte) & 0x80 ? (SETMOSI()) : (CLIMOSI()); \
    (outbyte) <<= 1; \
    ONSCLK(); \
    SPIHALFDELAY(); \
    (inbyte) <<= 1; \
    (inbyte) |= READMISO(); \
    OFFSCLK(); \
    SPIHALFDELAY(); \
} while (0)

#else
#  define SHIFTBIT(outbyte, inbyte) do { \
    (outbyte) & 0x80 ? (SETMOSI()) : (CLIMOSI()); \
    (outbyte) <<= 1; \
    SPIHALFDELAY(); \
    ONSCLK(); \
    SPIHALFDELAY(); \
    (inbyte) <<= 1; \
    (inbyte) |= READMISO(); \
    OFFSCLK(); \
} while (0)
#endif

uint8_t spi_transfer(uint8_t data) {
    int nbit;
    uint8_t res = 0;
    for (nbit=0; nbit<8; nbit++) {
        SHIFTBIT(data, res);
    }
    NOMOSI();
    return (res);
}

/* resp - responce from slave device; resplen - max. expected resp. len (buf. size);
 * if resp is NULL, not saved. Returns always the last byte of the response.
 */
uint8_t spi_ntransfer(const uint8_t *req, uint8_t *resp, int len) {
    int nbit;
    int nbyte;
    register uint8_t outbyte;
    uint8_t inbyte = 0;
    for (nbyte=0; nbyte<len; nbyte++) {
        outbyte = req[nbyte];
        inbyte = 0;
        for (nbit=0; nbit<8; nbit++) {
            SHIFTBIT(outbyte, inbyte);
        }
        if (resp) {
            resp[nbyte] = inbyte;
        }
    }
    NOMOSI();
    return (inbyte);
}

#undef ONSCLK
#undef OFFSCLK
#undef SETMOSI
#undef CLIMOSI
#undef NOMOSI
#undef READMISO
#undef SPIHALFDELAY
#undef SPIDELAYTIME
#undef SHIFTBIT
