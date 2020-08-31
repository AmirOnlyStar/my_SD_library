#ifndef _my_SD_H_
#define _my_SD_H_

#include "my_SPI.h"
#include "Arduino.h"

#define SD_READY  0
#define SD_ERROR    1

#define SD_START_TOKEN          0xFE
#define SD_ERROR_TOKEN          0x00

#define SD_BLOCK_LEN            512

#define SD_DATA_ACCEPTED        0x05
#define SD_DATA_REJECTED_CRC    0x0B
#define SD_DATA_REJECTED_WRITE  0x0D

#define SD_IN_IDLE_STATE    0x01
#define SD_READY            0x00
#define SD_R1_NO_ERROR(X)   X < 0x02

//CMD0
//01000000 00000000 00000000 00000000 00000000 10010101
//0x40     0x00     0x00     0x00     0x00     0x95
#define CMD0        0
#define CMD0_ARG    0x00000000
#define CMD0_CRC    0x94

//CMD8
#define CMD8        8
#define CMD8_ARG    0x0000001AA
#define CMD8_CRC    0x86 //(1000011 << 1)

//CMD58
#define CMD58       58
#define CMD58_ARG   0x00000000
#define CMD58_CRC   0x00

//CMD55
#define CMD55       55
#define CMD55_ARG   0x00000000
#define CMD55_CRC   0x00

//ACMD41  
#define ACMD41      41
#define ACMD41_ARG  0x40000000
#define ACMD41_CRC  0x00

//CMD17
#define CMD17                   17
#define CMD17_CRC               0x00
#define SD_MAX_READ_ATTEMPTS    1563
/*******************************************************************************
 Read single 512 byte block
 token = 0xFE - Successful read
 token = 0x0X - Data error
 token = 0xFF - Timeout
*******************************************************************************/

#define CMD24                   24
#define CMD24_CRC               0x00
#define SD_MAX_WRITE_ATTEMPTS   3907
/*******************************************************************************
 Write single 512 byte block
 token = 0x00 - busy timeout
 token = 0x05 - data accepted
 token = 0xFF - response timeout
*******************************************************************************/

//R1
#define PARAM_ERROR(X)      X & 0b01000000
#define ADDR_ERROR(X)       X & 0b00100000
#define ERASE_SEQ_ERROR(X)  X & 0b00010000
#define CRC_ERROR(X)        X & 0b00001000
#define ILLEGAL_CMD(X)      X & 0b00000100
#define ERASE_RESET(X)      X & 0b00000010
#define IN_IDLE(X)          X & 0b00000001

//R7
#define CMD_VER(X)          ((X >> 4) & 0xF0)
#define VOL_ACC(X)          (X & 0x1F)
#define VOLTAGE_ACC_27_33   0b00000001
#define VOLTAGE_ACC_LOW     0b00000010
#define VOLTAGE_ACC_RES1    0b00000100
#define VOLTAGE_ACC_RES2    0b00001000

//OCR
#define POWER_UP_STATUS(X)  X & 0x40
#define CCS_VAL(X)          X & 0x40
#define VDD_2728(X)         X & 0b10000000
#define VDD_2829(X)         X & 0b00000001
#define VDD_2930(X)         X & 0b00000010
#define VDD_3031(X)         X & 0b00000100
#define VDD_3132(X)         X & 0b00001000
#define VDD_3233(X)         X & 0b00010000
#define VDD_3334(X)         X & 0b00100000
#define VDD_3435(X)         X & 0b01000000
#define VDD_3536(X)         X & 0b10000000

//Token ERR
#define SD_TOKEN_OOR(X)     X & 0b00001000
#define SD_TOKEN_CECC(X)    X & 0b00000100
#define SD_TOKEN_CC(X)      X & 0b00000010
#define SD_TOKEN_ERROR(X)   X & 0b00000001

void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc);

void SD_powerUpSeq();
void SD_sendIfCond(uint8_t *res);
uint8_t SD_goIdleState();
void SD_readOCR(uint8_t *res);
uint8_t SD_sendApp();
uint8_t SD_sendOpCond();
uint8_t SD_init();
uint8_t SD_readSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token);
uint8_t SD_writeSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token);

uint8_t SD_readRes1();
void SD_readRes3_7(uint8_t *res);

void SD_printR1(uint8_t res);
void SD_printR3(uint8_t *res);
void SD_printR7(uint8_t *res);
void SD_printDataErrToken(uint8_t token);





#endif
