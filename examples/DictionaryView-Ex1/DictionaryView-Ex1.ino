
#include "RfidDictionaryView.h"

 /**
  * ----------------------------------------------------------------------------
  * Easy MFRC522 library - Dictionary View - Example #1
  * (Further information: https://github.com/pablo-sampaio/easy_mfrc522)
  * 
  * -----------------------------------------
  * Minimal example of reading/writing data as key-value pairs, using DictionaryView.
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
  * --------------------------------------------------------------------------
  */

int startBlock = 10;  // you may choose any block; choose #1 to have maximum storage availabel for the dictionary
RfidDictionaryView rfidDict(D4, D3, startBlock); // parameters: ports for SDA and RST pins, and initial block in the RFID tag


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // waits for up to 20s in "read" functions

  // não faz nada enquanto porta serial não estiver aberta
  while (!Serial)
    ;

delay(1000); //to wait the serial monitor to start (in PlatformIO)
}


void loop() {
  bool tagSelected;
  
  Serial.println();
  Serial.println("STEP 0: Approach a Mifare tag. Waiting...");

  do {
    // returns true if a Mifare tag is detected
    tagSelected = rfidDict.detectTag();
    delay(5);
  } while (!tagSelected);

  Serial.println(" --> PICC DETECTED (Mifare Classic)");
  Serial.print  (" --> Space available for dictionary (in bytes): ");
  Serial.println(rfidDict.getMaxSpaceInTag());
  Serial.println();
  delay(1000);

  String key, value;
  int numKeys;

  Serial.println("STEP 1: Reading value associated with key \"PERSON\"");

  value = rfidDict.get( "PERSON" ); // at first time (when key is not present), returns empty string
  
  Serial.println(" --> PAIR: \"PERSON\" => \"" + value + "\"\n");
  delay(1000);

  Serial.println("STEP 2: Setting values for keys \"PERSON\", \"ADDRESS\" and \"AGE\"\n"); 
  key = "PERSON";
  value = "Joe van Paul";
  rfidDict.set( key, value );   //using String variables 

  rfidDict.set( "ADDRESS", "Up above Street, 0" );  //using string literals
  rfidDict.set( "AGE", "30" );
  delay(1000);

  Serial.println("STEP 3 - Full content of the DICTIONARY");

  numKeys = rfidDict.getNumKeys();

  for (int i = 0; i < numKeys; i ++) {
    key = rfidDict.getKey(i);
    value = rfidDict.get(key);
    Serial.println("  | " + key + " \t=> " + value);
  }
  Serial.println("  --------------------------------\n");
  delay(1000);
  
  Serial.println("STEP 4: Setting new value for key \"PERSON\"\n"); 
  
  rfidDict.set( "PERSON", "Antonie, the Ant" );
  delay(1000);

  Serial.println("STEP 5 - Full content of the DICTIONARY");
  numKeys = rfidDict.getNumKeys();

  for (int i = 0; i < numKeys; i ++) {
    key = rfidDict.getKey(i);
    value = rfidDict.get(key);
    Serial.println("  | " + key + " \t=> " + value);
  }
  Serial.println("  --------------------------------\n");
  delay(1000);

  Serial.println("---> Example finished. Restarting in 10 secs. <---\n");
 
  // remember to do this 
  rfidDict.disconnectTag();

  delay(9000); //9s
}

