
#include "EasyMFRC522.h"

/**
 * ----------------------------------------------------------------------------
 * Easy MFRC522 library - Unlabeled Data - Example #1
 * (Further information: https://github.com/pablo-sampaio/easy_mfrc522)
 * 
 * -----------------------------------------
 * Simple example of reading/writing data chunks using the unlabeled operations. 
 * You must provide the exact number of bytes both to write and to read. 
 * (Useful if your data size is fixed). 
 * 
 * In this example, we store and retrieve the content of a "struct" variable.
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

#define BLOCK  1    //the initial block for all operations

// this struct represents credentials that may be granted for someone
// to access a specific gate until a certain time (in the same day)
struct GateAccessCredentials {
  char personName[80];
  char gateId[20];
  int expireHour;   //0-23h
  int expireMinute;
};

void printGateAccessCredentials(GateAccessCredentials *credentials); //auxiliary function, defined in the end of the file


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20000); // to wait for up to 20s in "read" functions
  
  while (!Serial)
    ;

  // you must call this initialization function!
  rfidReader.init(); 
}


void loop() {
  Serial.println("========================="); Serial.println();
  Serial.println("CHOOSE an operation: ");
  Serial.println("  'g' to grant access (write credentials)");
  Serial.println("  'r' to read the credentials");

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

  Serial.println("--> TAG DETECTED!\n");
  int result;

  if (option == 'g') {
    Serial.println("GRANTING access:");
    // to simulate that access was granted to Gate H2, with a random expire time
    GateAccessCredentials credentials = {"Pablo Sampaio", "Gate H2", 00, 00}; 
    credentials.expireHour = random(0, 24);
    credentials.expireMinute = random(0, 60);
    
    // starting from tag's block #1, writes the contents of variable "credentials", whose size is given (and independs of its content)
    result = rfidReader.writeRaw(BLOCK, (byte*)&credentials, sizeof(GateAccessCredentials));

    if (result >= 0) {
      Serial.print  ("--> Credentials written to the tag, ending in block ");
      Serial.println(result);
    } else {
      Serial.print  ("--> Error writing to the tag: ");
      Serial.println(result);
    }
  
  } else if (option == 'r') {
    Serial.println("LOADING CREDENTIALS from the tag:");
    GateAccessCredentials credentials;

    // starting from the given block, reads the data from the tag (for the amount of bytes given), loading to the variable "credentials"
    // attention: if you didn't write the credentials before, you will get "garbage" here
    result = rfidReader.readRaw(BLOCK, (byte*)&credentials, sizeof(GateAccessCredentials));

    if (result >= 0) {
      printGateAccessCredentials(&credentials);
    } else { 
      Serial.print("--> Error reading the tag, got");
      Serial.println(result);
    }

  } else if (option == 'd') {
    // extra: if you want to use the original Balboa's class, just call this
    MFRC522* device = rfidReader.getMFRC522();
    device->PICC_DumpMifareClassicSectorToSerial(&(device->uid), rfidReader.getKey(), 0); // dump the whole sector #0
    Serial.println();

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

// auxiliary function to print the contents of a GateAccessCredentials struct
void printGateAccessCredentials(GateAccessCredentials *credentials) {
  Serial.print("   - Name: "); 
  Serial.println(credentials->personName);

  Serial.print("   - Gate: "); 
  Serial.println(credentials->gateId);

  Serial.print("   - Expiration time: ");
  Serial.print(credentials->expireHour);
  if (credentials->expireMinute < 10) {
    Serial.print(":0");
  } else {
    Serial.print(":");
  }
  Serial.println(credentials->expireMinute);
}
