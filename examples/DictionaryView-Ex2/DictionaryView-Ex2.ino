
#include "RfidDictionaryView.h"

/**
  * ----------------------------------------------------------------------------
  * Easy MFRC522 library - Dictionary View - Example #2
  * (Further information: https://github.com/pablo-sampaio/easy_mfrc522)
  * 
  * -----------------------------------------
  * A more elaborate example of reading/writing data as key-value pairs, using 
  * DictionaryView. Allows one to test any operation, chosen by sending data 
  * (as input to this program) through the terminal.
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
  */

#define START_BLOCK   18  // you may choose any block; choose '1' for maximum storage for the dictionary

RfidDictionaryView rfidDict(D4, D3, START_BLOCK); // parameters: ports for SDA and RST pins, and initial block in the RFID tag
bool tagSelected = false;

// printf-style function for serial output
void printfSerial(const char *fmt, ...);


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // waits for up to 20s in "read" functions

  // não faz nada enquanto porta serial não estiver aberta
  while (!Serial)
    ;

  delay(1000); //to wait the serial monitor to start (in PlatformIO)
}


void loop() {
  byte tagId[4] = {0, 0, 0, 0};

  if (!tagSelected) {
    Serial.println();
    Serial.println("APPROACH a Mifare tag. Waiting...");

    do {
      // returns true if a Mifare tag is detected
      tagSelected = rfidDict.detectTag(tagId);
      delay(5);
    } while (!tagSelected);

    printfSerial("- TAG DETECTED, ID = %02X %02X %02X %02X \n", tagId[0], tagId[1], tagId[2], tagId[3]);
    printfSerial("  space available for dictionary: %d bytes.\n\n", rfidDict.getMaxSpaceInTag());
  }

  Serial.println("========================="); 
  Serial.println("CHOOSE operation: ");
  Serial.println(" g<key> to get/read the value associated to a key" );
  Serial.println(" s<key>:<value> to set/put the pair (key-->value)" );
  Serial.println(" r<key> to remove a key" );
  Serial.println(" j to remove all keys" );
  Serial.println(" f to finish (disconnect) the tag" );
  Serial.println(" d or * (anything else) to print the dictionary" );
  Serial.println("--");

  while (Serial.available() == 0) 
    ;

  String line = Serial.readStringUntil('\n');
  line.trim();           //removes leading and trailing spaces, \n, \r, etc.
  Serial.println(line);  //outputs the line, to show in the serial monitor (because in VS PlataformIO it doesn't show as you type)
  Serial.println("--\n");

  char option = line[0]; // first character of the line

  if (option == 'g' || option == 'G') {
    String key = line.substring(1);
    String value;

    key.trim();
    value = rfidDict.get( key );

    Serial.println(" - ENTRY: (\"" + key + "\" => \"" + value + "\")\n");

  } else if (option == 's' || option == 'S') {
    int indexSeparation = line.indexOf(':');
    if (indexSeparation == -1) {
      Serial.println(" - Error: inform key and value separated by colon (:)\n");
    
    } else {
      String key = line.substring(1, indexSeparation);
      String value = line.substring(indexSeparation+1);

      key.trim();
      value.trim();

      if (rfidDict.hasKey(key)) {
        Serial.println(" - Replacing entry: (\"" + key + "\" => \"" + rfidDict.get(key) + "\")");
      }
    
      rfidDict.set( key , value );

      Serial.println(" - SET (\"" + key + "\" => \"" + value + "\")\n");
    }
  
  } else if (option == 'r' || option == 'R') {
    String key = line.substring(1);
    key.trim();

    rfidDict.remove( key );

    Serial.println(" - REMOVE \"" + key + "\"\n");

  } else if (option == 'j' || option == 'J') {
    String key, value;
    int numKeys = rfidDict.getNumKeys();

    Serial.println(" - REMOVING ALL...");
    for (int i = numKeys-1; i >= 0; i --) {   // removes from the last to the first, to avoid errors
      key = rfidDict.getKey(i);
      rfidDict.remove(key);
      Serial.println("   \tkey \"" + key + "\"");
    }
    Serial.println("   DONE.\n");

  } else if (option == 'f' || option == 'F') {
    rfidDict.disconnectTag(true);
    tagSelected = false;
    Serial.println("Please move the tag away in 4 secs"); //2s below, and 2s in the end of the function
    delay(2000);

  } else {
    String key, value;
    int numKeys = rfidDict.getNumKeys();

    Serial.println("-- DICTIONARY ------------------");
    for (int i = 0; i < numKeys; i ++) {
      key = rfidDict.getKey(i);
      value = rfidDict.get(key);
      Serial.println("| \"" + key + "\" \t=> \"" + value + "\"");
    }
    Serial.println("--------------------------------\n");

  }

  // clear "garbage" input from serial
  while (Serial.available() > 0) { 
    Serial.read();
  }

  Serial.println("Finished operation!");
  Serial.println();
  delay(2000);
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
