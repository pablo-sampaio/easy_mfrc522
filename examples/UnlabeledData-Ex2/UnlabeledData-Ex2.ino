
#include "EasyMFRC522.h"

/**
 * ----------------------------------------------------------------------------
 * Easy MFRC522 library - Unlabeled Data - Example #1
 * (Further information: https://github.com/pablo-sampaio/easy_mfrc522)
 * 
 * -----------------------------------------
 * We use unlabeled operations to read and print the contents of each sector
 * in Mifare 1k tags. Each sector has 4 blocks (of 16 bytes each), but we only 
 * print the first 3 blocks, which are the ones available for user data (except
 * for block #0, which holds the ID). (The 4th is for sector configurations).
 * 
 * It also detects blocks that may (potentially) contain the start of a chunk
 * of labelled data. (May be used as an utility to find such data).
 * 
 * -----------------------------------------
 * Pin layout used (where * indicates configurable pin):
 * -----------------------------------------
 * MFRC522      Arduino       NodeMCU
 * Reader       Uno           Esp8266
 * Pin          Pin           Pin    
 * -----------------------------------------
 * SDA(SS)      4*            D4*
 * SCK          13            D5   
 * MOSI         11            D7
 * MISO         12            D6
 * RST          3*            D3*
 * NC(IRQ)      not used      not used
 * 3.3V         3.3V          3V
 * GND          GND           GND
 * --------------------------------------------------------------------------
 * Obs.: This code may not work if you use different types of boards to alternately
 * read/write, because of the memory size and alignment of the struct.
 */

EasyMFRC522 rfidReader(D4, D3); //the MFRC522 reader, with the SDA and RST pins given
                                //the default (factory) keys A and B are used (or used setKeys to change)

// printf-style function for serial output
void printfSerial(const char *fmt, ...);


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // to wait for up to 20s in "read" functions
  
  while (!Serial)
    ;

  rfidReader.init(); // initialization
  delay(1000);
}


void loop() {
  Serial.println();
  Serial.println("APPROACH a Mifare tag. Waiting...");

  bool success;
  byte tagId[4];
  do {
    // if a Mifare tag is detected, returns true and sets the tagId
    success = rfidReader.detectTag(tagId);
    delay(50); //0.05s
  } while (!success);
  printfSerial("--> TAG DETECTED, ID = %02X %02X %02X %02X \n\n", tagId[0], tagId[1], tagId[2], tagId[3]);

  Serial.println("====== TAG'S CONTENT (user space )======\n");
  
  int result;
  byte buffer[3*16];

  for (int block = 0; block < 64; block +=4) {
    delay(1000); //1s
    printfSerial("SECTOR %02d:\n", block/4);

    // reads the next three blocks (corresponds to all space for user data in the sector, except for block #0)
    result = rfidReader.readRaw(block, buffer, 3*16);
    
    if (result < 0) {
      printfSerial("--> Error: %d.\n\n", result);
      continue;
    }

    int bufferIndex, b;
    char dataLabel[13];
    dataLabel[12] = 0;
    
    // loop to read/print the 3 blocks
    bufferIndex = 0;

    for (b = 0; b < 3; b++) {
      printfSerial(" [%02d]  ", block+b);
      
      // test if a labeled data may be present: blocks that begin with "0x1C + twelve-length string"
      if (buffer[bufferIndex] == 0x1C) {
        for (int k = 0; k < 12; k++) {
          dataLabel[k] = buffer[bufferIndex+k+1];
        }          
        printfSerial("may start a data chunk labeled \"%s\" \n", dataLabel);
        printfSerial("       ");
      }

      // loop the 16 bytes of the block, printing them
      for (int i = 0; i < 16; i++) {
        printfSerial("%02X ", buffer[bufferIndex]);
        bufferIndex ++;
      }
      Serial.println(); //end of block
    }
    printfSerial(" [%02d]  reserved\n\n", block+b);
    //Serial.println(); //end of sector
  } 

  // Code below shows how to access Balboa's MFRC522 object, to use its functions
  // I used its dumping operation below, for debugging
  //MFRC522* device = rfidReader.getMFRC522();
  //device->PICC_DumpMifareClassicSectorToSerial(&(device->uid), rfidReader.getKey(), 0); // dump sector #0
  //device->PICC_DumpMifareClassicSectorToSerial(&(device->uid), rfidReader.getKey(), 1); // dump sector #1

  rfidReader.unselectMifareTag(false);
  
  Serial.println("========================================\n");
  Serial.println("Finished!");
  Serial.println();
  delay(5000);
}


/**
 * this function is a substitute  toSerial.printf() function, which was used in the 
 * first versions of this library, but seems to be unavailable for some operating systems.
 */
void printfSerial(const char *fmt, ...) {
  char buf[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Serial.print(buf);
}
