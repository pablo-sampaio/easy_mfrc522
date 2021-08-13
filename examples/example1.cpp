
#include <SPI.h>
#include "EasyMFRC522.h"

/******************************************
 * Connections MFRC522 pins -> Arduino ports:
 *  - SDA -> (configurable)
 *  - SCK -> 13
 *  - MOSI -> 11
 *  - MISO -> 12
 *  - NC (IRQ) -> not connected
 *  - GND -> GND
 *  - RST -> (configurable)
 *  - Pino 3.3V -> 3.3V
 ******************************************
 * Connections MFRC522 pins -> NODEMCU ESP8266 ports:
 *  - SDA -> (configurable, using D4)
 *  - SCK -> D5
 *  - MOSI -> D7
 *  - MISO -> D6
 *  - NC (IRQ) -> not connected
 *  - GND -> GND
 *  - RST -> (configurable, using D3)
 *  - Pino 3.3V -> 3V
 ******************************************
 * Other boards: connect the non-configurable pins to the corresponding 
 * SPI-related pins (MISO, MOSI, etc).
 ******************************************/

EasyMFRC522 rfidReader(D4, D3); //the Mifare sensor, with the SDA and RST pins given
                                //the default (factory) keys A and B are used (or used setKeys to change)

/**
 * Initialize.
 */
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // waits for up to 20s in "read" functions
  
  // não faz nada enquanto porta serial não estiver aberta
  while (!Serial)
    ;

  rfidReader.init();
}


/**
 * Main code.
 */
void loop() {
  Serial.println("========================="); Serial.println();
  Serial.println("CHOOSE operation: 'r' to read string from tag / 'w' to write \"Hello Tag!\" / 'd' to dump sector 0" );

  // waits for data
  while (Serial.available() == 0) 
    ;

  char option = Serial.read();
  Serial.println(option);

  Serial.println();
  Serial.println("APPROACH a Mifare tag. Waiting...");

  // se detectou a tag, sai do loop; senão, fica tentando detectar
  // bug: fails, if a tag is not presented in some time
  bool success;
  do {
    success = rfidReader.detectAndSelectMifareTag();
    delay(5);
  } while (!success);

  Serial.println("--> PICC DETECTED (Mifare Classic)");

  int result;
  char name[30];

  if (option == 'r') {
    Serial.println("[R] Retrieving the name stored in the card...");
    int maxSize = 30; // i.e. can only write in indexes 0 to 29
 
    result = rfidReader.readFile("myfile", (byte*)name, maxSize);
    name[29] = 0;     // for safety, in case the string was stored without a 0 in the end (would not happen with strings written by this program, see the 'w' option) 

    if (result > 0) { // success
      Serial.printf("--> Name: \"%s\" (bytes: %d)\n", name, result);
    } else {
      Serial.printf("--> ERROR reading tag: %d\n", result);
    }

  } else if (option == 'w') {
    strcpy(name, "Hello Tag!");        // attention: must have less than 30 characters because of the size of array "name"
    int nameSize = strlen(name);
    
    Serial.printf("[W] Writing \"%s\" (%d bytes)\n", name, nameSize+1); // adds 1 because of the trailing \0 that will also be written to the tag
   
    result = rfidReader.writeFile("myfile", (byte*)name, nameSize+1);

    if (result > 0) {
      Serial.printf("--> Written with success until block %d\n", result);
    } else {
      Serial.printf("--> Error writing tag: %d\n", result);
    }
  
  } else if (option == 'd') {
    Serial.println(F("- [D] Current data in the whole sector:"));
    int sector = 0;
    MFRC522* device = rfidReader.getMFRC522();
    device->PICC_DumpMifareClassicSectorToSerial(&(device->uid), rfidReader.getKey(), sector);
    Serial.println();

  }

  // clear "garbage" input from serial
  while (Serial.available() > 0) { 
    Serial.read();
  }

  rfidReader.unselectMifareTag();
  
  Serial.println();
  Serial.println("Finished operation!");
  Serial.println();
  delay(1000);
}

