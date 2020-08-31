#include "util/delay.h"
#include "my_SPI.h"
#include "my_SD.h"

void setup() {
  // put your setup code here, to run once:
      // initialize SPI
   uint8_t res1;
   uint8_t res[5];
   uint8_t sdBuf[512];
   uint8_t token;
    Serial.begin(1000000);
    Serial.println("Serial Ready >>>");
    SPI_init();
    Serial.println("SPI init OK.");
    res1 = SD_init();
    if(res1==0)
    {
      Serial.println("SD init OK.");
    }else
    {
      Serial.println("SD init ERR.");
    }
    
    // start power up sequence
//    SD_powerUpSeq();
    // command card to idle
//    res1 = SD_goIdleState();
//    Serial.print("resposnse:"); Serial.println(res1);
//
//    Serial.print("Sending CMD0...\r\n");
//    res[0] = SD_goIdleState();
//    Serial.print("Response:\r\n");
//    SD_printR1(res[0]);
//
//    // send if conditions
//    Serial.print("Sending CMD8...\r\n");
//    SD_sendIfCond(res);
//    Serial.print("Response:\r\n");
//    SD_printR7(res);
//      
//    Serial.print("Sending CMD58...\r\n");
//    SD_readOCR(res);
//    Serial.print("Response:\r\n");
//    SD_printR3(res);
    
//    // command card to idle
//    Serial.print("Sending CMD55...\r\n");
//    res[0] = SD_sendApp();
//    Serial.print("Response:\r\n");
//    SD_printR1(res[0]);
//
//    Serial.print("Sending ACMD41...\r\n");
//    res[0] = SD_sendOpCond();
//    Serial.print("Response:\r\n");
//    SD_printR1(res[0]);


// Read single block 
    res[0] = SD_readSingleBlock(0x00000000, sdBuf, &token);
    Serial.print("Token:");Serial.print(token);Serial.print("\r\n");
//    print response
    if(SD_R1_NO_ERROR(res[0]) && (token == 0xFE))
    {
        for(uint16_t i = 0; i < 512; i++) {Serial.print(sdBuf[i],HEX);Serial.print(" ");}
        Serial.print("\r\n");
    }
    else
    {
        Serial.print("Error reading sector\r\n");
    }
    
    uint8_t buf[512];
    uint32_t addr = 0x00000000;
    // update address to 0x00000100
    addr = 0x00000000;
    // fill buffer with 0x55
    for(uint16_t i = 0; i < 512; i++) buf[i] = 0x55;
    // write data to sector
    res[0] = SD_writeSingleBlock(addr, buf, &token);
    Serial.print("\r\nResponse:\r\n");
    SD_printR1(res[0]); 
    // if no errors writing
    if(res[0] == 0x00)
    {
      if(token == SD_DATA_ACCEPTED)
          Serial.print("Write successful\r\n");
    }
    Serial.print("Response 1:\r\n");
    SD_printR1(res);

    // if error token received
    if(!(token & 0xF0))
    {
        Serial.print("Error token:\r\n");
        SD_printDataErrToken(token);
    }
    else if(token == 0xFF)
    {
        Serial.print("Timeout\r\n");
    }
    
 }
void loop() {
  // put your main code here, to run repeatedly:

}
