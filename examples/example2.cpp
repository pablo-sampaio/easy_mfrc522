
#include <ctype.h>

#include "EasyMFRC522.h"

/**
 * Obs.: see PIN wiring for Arduino or ESP8266 in example 1.
 */


EasyMFRC522 rfidReader(D4, D3); //the Mifare sensor, with the SDA and RST pins given
                                //the default (factory) keys A and B are used (or used setKeys to change)

// this struct represents an entry in the access history 
// with the time and gate where a RFID tag was used
struct AccessRecord {
  unsigned long time;
  char gate;
};

#define HISTORY_MAX_SIZE 20

AccessRecord history[HISTORY_MAX_SIZE];
int historySize;           // occupied positions

#define BLOCK_ADDR  12     // writes from this block on, in the tag (block #12 is in sector 3)

/**
 * Initialize.
 */
void setup() {
  Serial.begin(9600);
  
  // does nothing, waiting for serial port to be ready
  while (!Serial)
    ;

  rfidReader.init();
}


/**
 * Main code.
 */
void loop() {
  Serial.println("========================="); Serial.println();
  Serial.println("TYPE a LETTER to represent the gate, to be stored in a new access record");
  Serial.println("TYPE 0 to reset the access history stored in the tag");
  Serial.println("TYPE 1 (or anything else) to list the full access history stored in the tag");

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

  Serial.println("=> TAG DETECTED (Mifare 1KB)\n");

  int result;

  //load history to the global variables
  if (rfidReader.existsFile("history", BLOCK_ADDR)) {
    
    result = rfidReader.readFile("history", (byte*)history, sizeof(AccessRecord)*HISTORY_MAX_SIZE, BLOCK_ADDR);
    if (result >= 0) {
      historySize = result / sizeof(AccessRecord);
      Serial.printf("LOADED access history with success: %d entries, %d bytes\n\n", historySize, result);
    
    } else {
      Serial.printf("ERROR reading tag: %d\n\n", result);
      option = -1; // so it won't do any operation below
    }
  
  } else {
    Serial.printf("TAG HAS NO history recorded.\n\n");
    historySize = 0;

  }

  if (isAlpha(option)) {
    Serial.printf("[%c] Storing new access record...\n", option);
    AccessRecord access;
    access.gate = toupper(option);
    access.time = millis();
    Serial.printf("=> New access record: time=%ld, gate=%c\n", access.time, access.gate);

    if (historySize >= HISTORY_MAX_SIZE) {
      //discards the first record, by shifting all the records one position to the left
      for (int i = 0; i < historySize-1; i++) {
        history[i] = history[i+1];
      }
      historySize -= 1;
    }

    history[historySize] = access;
    historySize ++;
    
    //writes onlye the useful positions
    result = rfidReader.writeFile("history", (byte*)history, sizeof(AccessRecord)*historySize, BLOCK_ADDR);

    if (result > 0) {
      Serial.printf("=> Stored in the tag with success: %d entries in total\n", historySize); 
    } else {
      Serial.printf("=> ERROR writing tag: %d\n", result);
    }
  
  } else if (option == '0') {
    Serial.println("[0] Resetting the access history stored in the tag...");
    
    //writes an empty history
    result = rfidReader.writeFile("history", (byte*)history, 0, BLOCK_ADDR);

    if (result > 0) {
      Serial.printf("=> Reseted the tag with success: 0 entries\n"); 
      historySize = 0;
    } else {
      Serial.printf("=> ERROR writing tag: %d\n", result);
    }

  } else if (option != -1) {
    Serial.println("[*] Listing the access history loaded from the tag...");
    for (int i = 0; i < historySize; i ++) {
      Serial.printf("  %02d: time %ld, gate %c\n", i, history[i].time, history[i].gate);
    }
  
  }

  // clear "garbage" input from serial
  while (Serial.available() > 0) { 
    Serial.read();
  }

  rfidReader.unselectMifareTag(true);
  
  Serial.println();
  Serial.println("Finished operation!");
  Serial.println();
  delay(1000);
}
