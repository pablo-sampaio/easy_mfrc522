
#include "RfidDictionaryView.h"

/**
 * Obs.: see PIN wiring for Arduino or ESP8266 in example 1.
 */

int startBlock = 1;
RfidDictionaryView rfidDict(D4, D3, startBlock); // parameters: ports for SDA and RST pins, and initial block in the RFID tag
bool tagSelected = false;


/**
 * Initializations.
 */
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // waits for up to 20s in "read" functions

  // não faz nada enquanto porta serial não estiver aberta
  while (!Serial)
    ;

}


/**
 * Main code.
 */
void loop() {
  if (!tagSelected) {
    Serial.println();
    Serial.println("APPROACH a Mifare tag. Waiting...");

    // se detectou a tag, sai do loop; senão, fica tentando detectar
    // bug: fails, if a tag is not presented in some time
    do {
      tagSelected = rfidDict.detectTag();
      //tagSelected = mfrc522.detectAndSelectMifareTag(); //not recommended!
      delay(5);
    } while (!tagSelected);

    Serial.println("- PICC DETECTED (Mifare Classic)");
    Serial.print  ("- space available for dictionary: ");
    Serial.println(rfidDict.getMaxSpaceInTag());
  }

  Serial.println("========================="); 
  Serial.println("CHOOSE operation: ");
  Serial.println(" g<key> to get/read the value from a key" );
  Serial.println(" s<key>:<value> to set/put the pair (key-->value)" );
  Serial.println(" r<key> to remove a key" );
  Serial.println(" j to remove all keys" );
  Serial.println(" f to finish and unselect the tag" );
  Serial.println(" d or * (anything else) to print the dictionary" );
  Serial.println("--");

  // waits for data
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

    Serial.println(" - GET(" + key + ") => " + value + "\n");

  } else if (option == 's' || option == 'S') {
    int indexSeparation = line.indexOf(':');

    String key = line.substring(1, indexSeparation);
    String value = line.substring(indexSeparation+1);

    key.trim();
    value.trim();

    if (rfidDict.hasKey(key)) {
      Serial.println(" - Old entry (to replace): (" + key + " => " + rfidDict.get(key) + ")\n");
    }
    
    rfidDict.set( key , value );

    Serial.println(" - SET (" + key + " => " + value + ")\n");
  
  } else if (option == 'r' || option == 'R') {
    String key = line.substring(1);
    key.trim();

    rfidDict.remove( key );

    Serial.println(" - REMOVE (" + key + ")\n");

  } else if (option == 'j' || option == 'J') {
    String key, value;
    int numKeys = rfidDict.getNumKeys();

    Serial.println(" - REMOVING ALL...");
    for (int i = numKeys-1; i >= 0; i --) {   // removes from the last to the first, to avoid errors
      key = rfidDict.getKey(i);
      rfidDict.remove(key);
      Serial.println(" - remove (" + key + ")\n");
    }
    Serial.println(" - DONE.");

  } else if (option == 'f' || option == 'F') {
    rfidDict.disconnectTag(true);
    tagSelected = false;
    Serial.println("Please move away the tag in 2 secs");
    delay(2000);

  } else {
    String key, value;
    int numKeys = rfidDict.getNumKeys();

    Serial.println("-- DICTIONARY ------------------");
    for (int i = 0; i < numKeys; i ++) {
      key = rfidDict.getKey(i);
      value = rfidDict.get(key);
      Serial.println("| " + key + " \t=> " + value);
    }
    Serial.println("--------------------------------\n");

  }

  // clear "garbage" input from serial
  while (Serial.available() > 0) { 
    Serial.read();
  }

  Serial.println("Finished operation!");
  Serial.println();
  delay(1000);
}

