#ifndef _my_SPI_H_
#define _my_SPI_H_
#include "Arduino.h"
#define PIN_SS    10
#define PIN_MOSI  11
#define PIN_MISO  12
#define PIN_SCK  13

#define CS_ENABLE()     digitalWrite(PIN_SS, LOW)
#define CS_DISABLE()    digitalWrite(PIN_SS, HIGH)

void SPI_init();
uint8_t SPI_transfer(uint8_t data);


#endif
