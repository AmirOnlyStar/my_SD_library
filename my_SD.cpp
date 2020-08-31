
#include "my_SPI.h"
#include "my_SD.h"
/*############################################################################*/
void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{ 
    uint8_t  res;
    // transmit command to sd card
    res = SPI_transfer(cmd|0x40);
    // transmit argument
    res = SPI_transfer((uint8_t)(arg >> 24));
    res = SPI_transfer((uint8_t)(arg >> 16));
    res = SPI_transfer((uint8_t)(arg >> 8));
    res = SPI_transfer((uint8_t)(arg));
    // transmit crc
    res = SPI_transfer(crc|0x01);
}
/*############################################################################*/
void SD_powerUpSeq()
{
    // make sure card is deselected
    CS_DISABLE();
    // give SD card time to power up
    _delay_ms(1);
    // send 80 clock cycles to synchronize
    for(uint8_t i = 0; i < 10; i++)
        SPI_transfer(0xFF);
    // deselect SD card
    CS_DISABLE();
    SPI_transfer(0xFF);
}

void SD_sendIfCond(uint8_t *res)
{
    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);
    // send CMD8
    SD_command(CMD8, CMD8_ARG, CMD8_CRC);
    // read response
    SD_readRes3_7(res);
    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);
}

uint8_t SD_goIdleState()
{
    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);
    // send CMD0
    SD_command(CMD0, CMD0_ARG, CMD0_CRC);
    // read response
    uint8_t res1 = SD_readRes1();
    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);
    return res1;
}

void SD_readOCR(uint8_t *res)
{
    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);

    // send CMD58
    SD_command(CMD58, CMD58_ARG, CMD58_CRC);

    // read response
    SD_readRes3_7(res);

    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);
}

uint8_t SD_sendApp()
{
    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);

    // send CMD0
    SD_command(CMD55, CMD55_ARG, CMD55_CRC);

    // read response
    uint8_t res1 = SD_readRes1();

    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);

    return res1;
}

uint8_t SD_sendOpCond()
{
    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);

    // send CMD0
    SD_command(ACMD41, ACMD41_ARG, ACMD41_CRC);

    // read response
    uint8_t res1 = SD_readRes1();
//    SD_printR1(res1[0]);

    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);

    return res1;
}

uint8_t SD_init(void)
{
    uint8_t res[5], cmdAttempts = 0;

    SD_powerUpSeq();

    // command card to idle
    while((res[0] = SD_goIdleState()) != 0x01)
    {
        cmdAttempts++;
        if(cmdAttempts > 10) return SD_ERROR;
    }

    // send interface conditions
    SD_sendIfCond(res);
    if(res[0] != 0x01)
    {
        return SD_ERROR;
    }

    // check echo pattern
    if(res[4] != 0xAA)
    {
        return SD_ERROR;
    }

    // attempt to initialize card
    cmdAttempts = 0;
    do
    {
        if(cmdAttempts > 100) return SD_ERROR;

        // send app cmd
        res[0] = SD_sendApp();

        // if no error in response
        if(res[0] < 2)
        {
            res[0] = SD_sendOpCond();
        }

        // wait
        _delay_ms(10);

        cmdAttempts++;
    }
    while(res[0] != SD_READY);

    // read OCR
    SD_readOCR(res);

    // check card is ready
    if(!(res[1] & 0x80)) return SD_ERROR;

    return SD_READY;
}

uint8_t SD_readSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token)
{
    uint8_t res1, read;
    uint16_t readAttempts;

    // set token to none
    *token = 0xFF;

    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);

    // send CMD17
    SD_command(CMD17, addr, CMD17_CRC);

    // read R1
    res1 = SD_readRes1();

    // if response received from card
    if(res1 != 0xFF)
    {
        // wait for a response token (timeout = 100ms)
        readAttempts = 0;
        while(++readAttempts != SD_MAX_READ_ATTEMPTS)
            if((read = SPI_transfer(0xFF)) != 0xFF) break;

        // if response token is 0xFE
        if(read == 0xFE)
        {
            // read 512 byte block
            for(uint16_t i = 0; i < 512; i++) *buf++ = SPI_transfer(0xFF);

            // read 16-bit CRC
            SPI_transfer(0xFF);
            SPI_transfer(0xFF);
        }

        // set token to card response
        *token = read;
    }

    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);

    return res1;
}

uint8_t SD_writeSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token)
{
    uint8_t readAttempts, read;
    uint8_t res[5];
    // set token to none
    *token = 0xFF;

    // assert chip select
    SPI_transfer(0xFF);
    CS_ENABLE();
    SPI_transfer(0xFF);

    // send CMD24
    SD_command(CMD24, addr, CMD24_CRC);

    // read response
    res[0] = SD_readRes1();
              
    // if no error
    if(res[0] == SD_READY)
    {
        // send start token
        SPI_transfer(SD_START_TOKEN);

        // write buffer to card
        for(uint16_t i = 0; i < SD_BLOCK_LEN; i++) SPI_transfer(buf[i]);

        // wait for a response (timeout = 250ms)
        readAttempts = 0;
        while(++readAttempts != SD_MAX_WRITE_ATTEMPTS)
            if((read = SPI_transfer(0xFF)) != 0xFF) { *token = 0xFF; break; }

        // if data accepted
        if((read & 0x1F) == 0x05)
        {
            // set token to data accepted
            *token = 0x05;

            // wait for write to finish (timeout = 250ms)
            readAttempts = 0;
            while(SPI_transfer(0xFF) == 0x00)
                if(++readAttempts == SD_MAX_WRITE_ATTEMPTS) { *token = 0x00; break; }
        }
    }

    // deassert chip select
    SPI_transfer(0xFF);
    CS_DISABLE();
    SPI_transfer(0xFF);

    return res[0];
}

/*############################################################################*/
uint8_t SD_readRes1()
{
    uint8_t i = 0, res1;
    // keep polling until actual data received
    while((res1 = SPI_transfer(0xFF)) == 0xFF)
    {
        i++;
        // if no data received for 8 bytes, break
        if(i > 8) break;
    }
    return res1;
}

void SD_readRes3_7(uint8_t *res)
{
    // read R1
    res[0] = SD_readRes1();
    // if error reading R1, return
    if(res[0] > 1) return;
    // read remaining bytes
    res[1] = SPI_transfer(0xFF);
    res[2] = SPI_transfer(0xFF);
    res[3] = SPI_transfer(0xFF);
    res[4] = SPI_transfer(0xFF);
}


void SD_printR1(uint8_t res)
{
    if(res & 0b10000000)
        { Serial.print("\tError: MSB = 1\r\n"); return; }
    if(res == 0)
        { Serial.print("\tCard Ready\r\n"); return; }
    if(PARAM_ERROR(res))
        Serial.print("\tParameter Error\r\n");
    if(ADDR_ERROR(res))
        Serial.print("\tAddress Error\r\n");
    if(ERASE_SEQ_ERROR(res))
        Serial.print("\tErase Sequence Error\r\n");
    if(CRC_ERROR(res))
        Serial.print("\tCRC Error\r\n");
    if(ILLEGAL_CMD(res))
        Serial.print("\tIllegal Command\r\n");
    if(ERASE_RESET(res))
        Serial.print("\tErase Reset Error\r\n");
    if(IN_IDLE(res))
        Serial.print("\tIn Idle State\r\n");
}

void SD_printR3(uint8_t *res)
{
    SD_printR1(res[0]);

    if(res[0] > 1) return;

    Serial.print("\tCard Power Up Status: ");
    if(POWER_UP_STATUS(res[1]))
    {
        Serial.print("READY\r\n");
        Serial.print("\tCCS Status: ");
        if(CCS_VAL(res[1])){ Serial.print("1\r\n"); }
        else Serial.print("0\r\n");
    }
    else
    {
        Serial.print("BUSY\r\n");
    }

    Serial.print("\tVDD Window: ");
    if(VDD_2728(res[3])) Serial.print("2.7-2.8, ");
    if(VDD_2829(res[2])) Serial.print("2.8-2.9, ");
    if(VDD_2930(res[2])) Serial.print("2.9-3.0, ");
    if(VDD_3031(res[2])) Serial.print("3.0-3.1, ");
    if(VDD_3132(res[2])) Serial.print("3.1-3.2, ");
    if(VDD_3233(res[2])) Serial.print("3.2-3.3, ");
    if(VDD_3334(res[2])) Serial.print("3.3-3.4, ");
    if(VDD_3435(res[2])) Serial.print("3.4-3.5, ");
    if(VDD_3536(res[2])) Serial.print("3.5-3.6");
    Serial.print("\r\n");
}


void SD_printR7(uint8_t *res)
{
    SD_printR1(res[0]);

    if(res[0] > 1) return;
    Serial.print("\tCommand Version: ");
    Serial.print(CMD_VER(res[1]),HEX);
    Serial.print("\r\n");
    Serial.print("\tVoltage Accepted: ");
    if(VOL_ACC(res[3]) == VOLTAGE_ACC_27_33)
        Serial.print("2.7-3.6V\r\n");
    else if(VOL_ACC(res[3]) == VOLTAGE_ACC_LOW)
        Serial.print("LOW VOLTAGE\r\n");
    else if(VOL_ACC(res[3]) == VOLTAGE_ACC_RES1)
        Serial.print("RESERVED\r\n");
    else if(VOL_ACC(res[3]) == VOLTAGE_ACC_RES2)
        Serial.print("RESERVED\r\n");
    else
        Serial.print("NOT DEFINED\r\n");
    Serial.print("\tEcho: ");
    Serial.print(res[4],HEX);
    Serial.print("\r\n");
}

void SD_printDataErrToken(uint8_t token)
{
    if(SD_TOKEN_OOR(token))
        Serial.print("\tData out of range\r\n");
    if(SD_TOKEN_CECC(token))
        Serial.print("\tCard ECC failed\r\n");
    if(SD_TOKEN_CC(token))
        Serial.print("\tCC Error\r\n");
    if(SD_TOKEN_ERROR(token))
        Serial.print("\tError\r\n");
}
