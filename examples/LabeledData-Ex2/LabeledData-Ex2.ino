
#include <ctype.h>

#include "EasyMFRC522.h"

/**
 * ----------------------------------------------------------------------------
 * Easy MFRC522 library - Labeled Data - Example #2
 * (Further information: https://github.com/pablo-sampaio/easy_mfrc522)
 * 
 * --------------------------------------------------------------------------
 * In this toy example, we imagine an application were RFID tags are used to 
 * access/open gates or doors in a building. (See also "Ulabeled Data - Ex1").
 * 
 * This programs shows how you can keep a history, in each RFID tag, of the times 
 * and gates were the tag was used. To simulate the access to a gate, you just
 * type a single letter representing the gate id. The time is represented with
 * the relative timestamp given by millis().
 * 
 * We use labeled data to be able to query if the history data is present or not
 * in the tag, and to be able to handle a history (represented as an array of 
 * structs) with variable number of records.
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
 * -----------------------------------------
 * Other boards: connect the non-configurable pins to the corresponding 
 * SPI-related pins (MISO, MOSI). Connect the configurable pins to any
 * general-purpose IO digital ports and adjust the declaration below.
 * --------------------------------------------------------------------------
 */


EasyMFRC522 rfidReader(D4, D3); //the Mifare sensor, with the SDA and RST pins given
                                //the default (factory) keys A and B are used (or used setKeys to change)

// printf-style function for serial output
void printfSerial(const char *fmt, ...);

// this struct represents an entry in the access history 
// with the time and gate where a RFID tag was used
struct AccessRecord {
  unsigned long time;
  char gate;
};

#define HISTORY_MAX_SIZE 20

AccessRecord history[HISTORY_MAX_SIZE];
int historySize;      // counts occupied/useful positions

#define BLOCK  16     // writes from this block on, in the tag


void setup() {
  Serial.begin(9600);
  
  while (!Serial)
    ;

  rfidReader.init();

  delay(1000);
}


void loop() {
  Serial.println("========================="); Serial.println();
  Serial.println("TYPE any LETTER to represent the gate, to be stored in a new access record");
  Serial.println("TYPE 0 to reset the access history stored in the tag");
  Serial.println("TYPE 1 to list the full access history stored in the tag");

  while (Serial.available() == 0) // waits for incoming data
    ;

  char option = Serial.read();
  Serial.println(option);

  Serial.println();
  Serial.println("APPROACH a Mifare tag. Waiting...");

  bool success;
  do {
    // returns true if a Mifare tag is detected
    success = rfidReader.detectTag();
    delay(5);
  } while (!success);

  Serial.println("--> RFID tag detected!\n");

  int result;

  // below, we load the "history" to the global variables, if the proper labeled data is found
  Serial.println("LOADING access history:");

  if (rfidReader.existsFile(BLOCK, "history")) {
    
    result = rfidReader.readFile(BLOCK, "history", (byte*)history, sizeof(AccessRecord)*HISTORY_MAX_SIZE);
    if (result >= 0) {
      historySize = result / sizeof(AccessRecord);
      printfSerial("--> Success: %d entries, %d bytes\n\n", historySize, result);
    
    } else {
      printfSerial("--> Error: %d\n\n", result);
      historySize = -1;  // history was not loaded

    }
  
  } else {
    printfSerial("--> Tag seems to have no access history data. Try resetting the history.\n\n");
    historySize = -1;  // history was not loaded

  }

  // below, we execute the operation chosen
  // two of them require the history properly loaded (so historySize != -1)

  if (isAlpha(option) && historySize != -1) {
    printfSerial("STORING (in the tag) an access record to gate %c:\n", toupper(option));
    AccessRecord access;
    access.gate = toupper(option);
    access.time = millis();
    printfSerial("--> New access record: time=%ld, gate=%c\n", access.time, access.gate);

    if (historySize >= HISTORY_MAX_SIZE) {
      // discards the first record, by shifting all the records one position to the left
      for (int i = 0; i < historySize-1; i++) {
        history[i] = history[i+1];
      }
      historySize -= 1;
    }

    history[historySize] = access;
    historySize ++;
    
    // writes only the useful positions
    result = rfidReader.writeFile(BLOCK, "history", (byte*)history, sizeof(AccessRecord)*historySize);

    if (result > 0) {
      printfSerial("--> Stored with success: %d entries in total\n\n", historySize); 
    } else {
      printfSerial("--> Error: %d\n\n", result);
    }
  
  } else if (option == '0') {
    Serial.println("RESETTING the access history stored in the tag...");
    
    // writes an empty history
    result = rfidReader.writeFile(BLOCK, "history", (byte*)history, 0);

    if (result > 0) {
      printfSerial("--> Reset done with success: 0 entries\n\n"); 
      historySize = 0;
    } else {
      printfSerial("--> Error: %d\n\n", result);
    }

  } else if (option == '1' && historySize != -1) {
    Serial.println("ACCESS HISTORY (all records):");
    for (int i = 0; i < historySize; i ++) {
      printfSerial(" | %02d: time %ld, gate %c\n", i, history[i].time, history[i].gate);
    }
    Serial.println(" -------\n");
  }

  // clear "garbage" input from serial
  while (Serial.available() > 0) { 
    Serial.read();
  }

  rfidReader.unselectMifareTag(true);
  
  Serial.println("Finished operation!\n");
  delay(3000);
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