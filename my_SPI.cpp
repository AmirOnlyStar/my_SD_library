#include "my_SPI.h"
void SPI_init()
{
    pinMode(PIN_SS, OUTPUT);
      digitalWrite(PIN_SS, HIGH);
    pinMode(PIN_MOSI, OUTPUT);
      digitalWrite(PIN_MOSI, HIGH);
    pinMode(PIN_MISO, INPUT_PULLUP);
    pinMode(PIN_SCK, OUTPUT);
    // enable SPI, set as master, and clock to fosc/4
    SPCR = (1 << SPE) | (1 << MSTR) | (0 << SPR1) | (0 << SPR0);
}

uint8_t SPI_transfer(uint8_t data)
{
    // load data into register
    SPDR = data;

    // Wait for transmission complete
    while(!(SPSR & (1 << SPIF)));

    // return SPDR
    return SPDR;
}
