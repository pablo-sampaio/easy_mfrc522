
#include "EasyMFRC522.h"

/**
 * ----------------------------------------------------------------------------
 * Easy MFRC522 library - Labeled Data - Example #1
 * (Further information: https://github.com/pablo-sampaio/easy_mfrc522)
 * 
 * -----------------------------------------
 * Minimal example of reading/writing data chunks of arbitrary length (possibly 
 * spanning multiple sectors) using labeled read/write operations. 
 * 
 * Labeled operations allows reading data without knowing previously its exact size
 * (in the tag). Also allows one to query if the data chunk exists or its size.
 * 
 * Hardware: you need an Arduino or Esp8266 connected to a MFRC522 reader, and
 * at least one Mifare Classic card/tag.
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

#define MAX_STRING_SIZE 100  // size of the char array that will be written to the tag
#define BLOCK 1              // initial block, from where the data will be stored in the tag

EasyMFRC522 rfidReader(D4, D3); //the Mifare sensor, with the SDA and RST pins given
                                //the default (factory) keys A and B are used (or used setKeys to change)

// printf-style function for serial output
void printfSerial(const char *fmt, ...);


/** 
 * In the Arduino framework, this function is called once to initialize whatever you need.
 */
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // to wait for up to 20s in "read" functions
  
  while (!Serial)
    ;

  // you must call this initialization function!
  rfidReader.init(); 
}


/** 
 * In the Arduino framework, this function is called repeatedly.
 */
void loop() {
  Serial.println("========================="); Serial.println();
  Serial.println("CHOOSE an operation: ");
  Serial.println("  'w' to write a string (to a RFID tag)");
  Serial.println("  's' to read (only) the string size");
  Serial.println("  'r' to read the string");

  while (Serial.available() == 0) // waits for incoming data
    ;

  char option = Serial.read();

  Serial.println();
  Serial.println("APPROACH a Mifare tag. Waiting...");

  bool success;
  do {
    // returns true if a Mifare tag is detected
    success = rfidReader.detectTag();    
    delay(50); //0.05s
  } while (!success);

  Serial.println("--> PICC DETECTED!");

  int result;
  char stringBuffer[MAX_STRING_SIZE];

  if (option == 'w') {
    strcpy(stringBuffer, "Hello Tag! Just a random text!");  // you may try a different string here, with LESS than MAX_STRING_SIZE characters
    int stringSize = strlen(stringBuffer);
    
    // starting from tag's block #1, writes a data chunk labeled "mylabel", with its content given by stringBuffer, of stringSize+1 bytes (because of the trailing 0 in strings) 
    result = rfidReader.writeFile(BLOCK, "mylabel", (byte*)stringBuffer, stringSize+1);

    if (result >= 0) {
      printfSerial("--> Successfully written \"%s\" to the tag, ending in block %d\n", stringBuffer, result);
    } else {
      printfSerial("--> Error writing to the tag: %d\n", result);
    }
  
  } else if (option == 's') {
    // queries if a data chunk labeled "mylabel" is stored in block #1 (as initial block)
    result = rfidReader.readFileSize(BLOCK, "mylabel");

    if (result >= 0) {
      printfSerial("--> Size of data chunk found in the tag: %d bytes\n", result);
    } else { 
      printfSerial("--> Error reading the tag (%d)! Probably there is no data with the given label in the given block.\n", result);
    }
  
  } else if (option == 'r') {
    // starting from block #1, reads the data chunk labeled "mylabel", filling in the given buffer with the data
    result = rfidReader.readFile(BLOCK, "mylabel", (byte*)stringBuffer, MAX_STRING_SIZE);

    stringBuffer[MAX_STRING_SIZE-1] = 0;   // for safety; in case the string was stored without a \0 in the end 
                                           // (would not happen in this example, but it is a good practice when reading strings) 

    if (result >= 0) { // non-negative values indicate success, while negative ones indicate error
      printfSerial("--> String data retrieved: \"%s\" (bytes: %d)\n", stringBuffer, result);
    } else { 
      printfSerial("--> Error reading the tag (%d)! Probably there is no data labeled \"mylabel\".\n", result);
    }

  }

  while (Serial.available() > 0) {  // clear "garbage" input from serial
    Serial.read();
  }

  // call this after doing all desired operations in the tag
  rfidReader.unselectMifareTag();
  
  Serial.println();
  Serial.println("Finished operation!");
  Serial.println();
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
