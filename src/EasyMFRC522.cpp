
#include <SPI.h>
#include <MFRC522.h>

#include "EasyMFRC522.h"

/*** 
 * To understand this code, it is necessary to understand the internal architecture of 
 * Mifare Classic tags (or PICCs - Proximity Integrated Circuit Cards). This code supports 
 * all Mifare Classic tags (mini, 1K and 4K), but was tested only on Mifare Classic 1k!
 * 
 * Each tag is divided into sectors and blocks. Most sectors have 4 blocks, but some sectors
 * in Mifare 4k have 16 blocks. The blocks are numbered sequentially starting in 0 (in this
 * library). Each block has 16 bytes. (In Mifare 1k, there are 64 blocks). In block #0
 * (first block in the first sector) contains the ID of the tag, normally a unique number 
 * written from factory.
 * 
 * To access the blocks of each sector, it is necessary to authenticate using two 
 * sector-specific keys, called Key A and Key B. The sector may be configured to use only
 * one of the keys. These setttings, as well as other sector-specific flags are stored in
 * the fourth block of each sector. 
 * 
 * This library makes it easy to read and write arbitrary-length data, that may require 
 * multiple blocks and sectors. 
 * 
 * This library assumes that:
 * - all sectors are authenticated with the same key A, without the need for key B,
 * - at most one tag/PICC is in the range of the MFRC522 antenna, in anytime
 * - the user data is identified by a label (like a file name)
 */

#define READ_WRITE_TRIALS 5

//#define SHOW_DEBUG_MESSAGES 1  // uncomment this line to enable messages output to the serial

#ifdef SHOW_DEBUG_MESSAGES
  #define dbgPrintln(str) Serial.println(str)
  #define dbgPrint(str)   Serial.print(str)
#else
  #define dbgPrintln(str)  ;  //like a "no-op", alt.: {}
  #define dbgPrint(str)    ;
#endif


////////////////////////////////////////////////////////
//////// INITIALIZATION/CONFIGURATION FUNCTIONS ////////

EasyMFRC522::EasyMFRC522(byte sda_pin, byte reset_pin) 
  : device(sda_pin, reset_pin) // calls constructor of class MFRC522
{
  // for keys A and B, we use the default from the factory: FFFFFFFFFFFF (hex) 
  for (int i = 0; i < 6; i ++) {
    this->key.keyByte[i] = 0xFF;
  }
}

EasyMFRC522::~EasyMFRC522() {
  // there is nothing to free/deallocate
}

void EasyMFRC522::init() {
  SPI.begin();
  this->device.PCD_Init();

  byte v = this->device.PCD_ReadRegister(this->device.VersionReg); //get the MFRC522 software version
  Serial.print(F("MFRC522 sw version: "));
  if (v == 0x91) {
    Serial.print(F("v1.0"));
  } else if (v == 0x92) {
    Serial.print(F("v2.0"));
  } else if ((v == 0x00) || (v == 0xFF)) {
    Serial.print(F("communication failure")); //check physical connections and initialization parameters
  } else {
    Serial.print(F("unknown "));
    Serial.print(v, HEX);
  }
  Serial.println();
}

void EasyMFRC522::setKeyA(byte keyA[6]) {
  for (int i = 0; i < 6; i ++) {
    this->key.keyByte[i] = keyA[i];
  }
}

bool EasyMFRC522::detectTag(byte outputTagId[4]) {
  if (! device.PICC_IsNewCardPresent())
    return false;

  if (! device.PICC_ReadCardSerial())  //selects one of the cards/tags
      return false;

  MFRC522::PICC_Type piccType = device.PICC_GetType(device.uid.sak);
  
  // accepts only Mifare Classic protocol
  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI
		    || piccType == MFRC522::PICC_TYPE_MIFARE_1K
		    || piccType == MFRC522::PICC_TYPE_MIFARE_4K) {

    // copies the tag's ID to the variable provided
    if (outputTagId != NULL) {
      for (int i = 0; i < 4; i ++) {
        outputTagId[i] = this->device.uid.uidByte[i];
      }
    }

    return true;
  }

  return false;
}

void EasyMFRC522::unselectMifareTag(bool allowRedetection) {
  device.PICC_HaltA();
  device.PCD_StopCrypto1();
  if (allowRedetection) {
    device.PCD_AntennaOff();
    device.PCD_AntennaOn();
  }
}

/**
 * Gives the net storage capacity of the currently selected tag, from the given start 
 * block on (i.e. the start block is pottentially considered as user space if it does
 * not viollate the rules below). 
 * 
 * Assuming these blocks are reserved (user cannot write arbitrary data on them): 
 * block 0 (used for the tag ID), and the trailling block of each sector (used for
 * authentication and access data).
 */
int EasyMFRC522::getUserDataSpace(int startBlock) {
  MFRC522::PICC_Type piccType = device.PICC_GetType(device.uid.sak);

  if (startBlock == 0 || startBlock == 1) {
    if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI) {
      return 224;  //320 bytes in 5 sectors, with 6 blocks (of 16 bytes) reserved
    } else if (piccType == MFRC522::PICC_TYPE_MIFARE_1K) {
      return 752;  //1024 bytes in 16 sectors, with 17 blocks reserved
    } else if (piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
      return 3440; //4096 bytes in 40 sectors, with 41 blocks reserved
    }
    return 0;
  }
  
  int fullSectors4blocks = 0;
  int fullSectors16blocks = 0;
  int remainingBlocks = 0;

  // the code below code calculates the PREVIOUS space for user data (before the start block)

  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI 
      || piccType == MFRC522::PICC_TYPE_MIFARE_1K
      || (piccType == MFRC522::PICC_TYPE_MIFARE_4K && startBlock <= 128) )  {

    fullSectors4blocks = startBlock / 4;
    fullSectors16blocks = 0;
    remainingBlocks = startBlock % 4;

  } else if (piccType == MFRC522::PICC_TYPE_MIFARE_4K && startBlock > 128) {
    fullSectors4blocks = 32;                        // there are 32 sectors of 4 blocks before this block
    fullSectors16blocks = (startBlock - 128) / 16;  // number of full sectors with 16 blocks before this block
    remainingBlocks = (startBlock - 128) % 16;
  }

  // summing up all previous usable blocks: 
  // there are 3 or 15 per sector (depeding on the sector), discards block 0, and adds the other blocks of the "last" (incomplete) sector
  int previousBlocks = fullSectors4blocks*3 + fullSectors16blocks*15 - 1 + remainingBlocks;
  
  int previousSpace = previousBlocks * 16; // 16 bytes per block
  
  int availableSpace = getUserDataSpace(0) - previousSpace;
  
  return (availableSpace < 0)? 0 : availableSpace;
}

///////////////////////////////////////////////////
////////// READ/WRITE RAW (multisector) ///////////

/**
 * Writes the given "data" array (with "dataSize" bytes) in the Mifare 1K tag, starting in "initialBlock" (or in the next one, if it is a 
 * trailer block or the block #0) and successively writing to the next non-trailer block, until all data is written. 
 * 
 * The tag must have been detected and selected by the MFRC522 sensor (e.g. using "detectAndSelectMifareTag()").
 * There is no need to authenticate in the tag prior to calling this function.
 * This function does not stop authentication (so, in case of success, the tag is still authenticated in the sector of the last block written). 
 * 
 * Returns: negative number -- error
 *          positive number -- last block written
 */
int EasyMFRC522::writeRaw(int initialBlock, byte* data, int dataSize) {
  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;
  int statusCode;

  int bytesWritten = 0;  
  int currBlock = initialBlock;
  int currSector = currBlock / 4;
  bool sectorAuthenticated = false;
  
  while (bytesWritten < dataSize) {
    //TODO: this code is for Mifare 1k; adapt to other types
    if (currBlock % 4 == 3 || currBlock == 0) {
      dbgPrint("   - Ignored block: "); dbgPrintln(currBlock);
      currBlock ++;
      currSector ++;
      sectorAuthenticated = false; //because the sector changed
    }

    //TODO: adapt to other types of tag
    if (currBlock > 63) {
      dbgPrint("Error writeRaw(): not enough space");
      return -220;
    }

    if (! sectorAuthenticated) {
      dbgPrint("   - Authenticating sector: "); dbgPrintln(currSector); 
      for (int i = 0; i < READ_WRITE_TRIALS; i ++) {
        status = device.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, currBlock, &key, &(device.uid));
        if (status == MFRC522::STATUS_OK) {
          break;
        }
        dbgPrintln(F("    na"));
      }
      if (status != MFRC522::STATUS_OK) {
        dbgPrint("Error writeRaw(): could not authenticate, block "); dbgPrintln(currBlock);
        return -221;
      }
      sectorAuthenticated = true;
    }

    int bytes = dataSize - bytesWritten;
    bytes = (bytes < 16)? bytes : 16;
    for (int i = 0; i < READ_WRITE_TRIALS; i ++) {
      statusCode = _writeBlockAndVerify(currBlock, data, bytesWritten, bytes);
      if (statusCode >= 0) {
        bytesWritten += bytes;
        break;
      }
    }
    if (statusCode < 0) {
      return -200 + statusCode;
    }

    currBlock ++;
  }

  return currBlock - 1;
}

int EasyMFRC522::_writeBlockAndVerify(int blockAddr, byte* data, int startIndex, int bytesToWrite) {
  MFRC522::StatusCode status;
  if (bytesToWrite == 16) {
    status = device.MIFARE_Write(blockAddr, data + startIndex, 16);
    if (status != MFRC522::STATUS_OK) {
      dbgPrint  ("Error _writeBlockAndVerify(): could not write block ");
      dbgPrintln(blockAddr);
      return -5;
    }

    return _verifyBlock(blockAddr, data, startIndex, 16); //verifies block written in the tag against the data array (positions 0-16)
    
  } else if (0 < bytesToWrite && bytesToWrite < 16) {
    //copies to the buffer before writing
    for (int i = 0; i < bytesToWrite; i ++) {
      blockBuffer[i] = data[startIndex + i];
    }
    for (int i = bytesToWrite; i < 16; i ++) { //the buffer is completed with 0s
      blockBuffer[i] = 0;
    }
    
    status = device.MIFARE_Write(blockAddr, blockBuffer, 16);
    if (status != MFRC522::STATUS_OK) {
      dbgPrint  ("Error _writeBlockAndVerify(): could not write block ");
      dbgPrintln(blockAddr);
      return -6;
    }

    return _verifyBlock(blockAddr, blockBuffer, 0, bytesToWrite); //verifies block written in the tag against the buffer (positions 0-bytesToWrite)
    
  } else {
    dbgPrintln("Error _writeBlockAndVerify(): invalid data size");
    return -7;
    
  }
}

int EasyMFRC522::_verifyBlock(int blockAddr, byte* refData, byte startByte, byte bytesToCheck) {
  byte bufferSize = 18;
  MFRC522::StatusCode status = device.MIFARE_Read(blockAddr, blockBuffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
      dbgPrintln("Error _verifyBlock(): could not read");
      return -3;
  }

  for (byte i = 0; i < bytesToCheck; i++) {
      if (blockBuffer[i] != refData[startByte + i]) {
        dbgPrint("Error _verifyBlock(): verification error in byte: ");
        dbgPrintln(i);
        return -4;
      }
  }

  return 0;
} 

/**
 * Reads the data starting in the given initial block, and with the given dataSize, then copies the data to "dataOutput". 
 * Returns the number of bytes read. 
 * 
 * See more details in the comments to writeRaw().
 * 
 * The "dataOutput" must be previously allocated, with enough room ("dataOutputMaxSize") for the data read from the tag.
 * The tag must have been detected and selected by the MFRC522 sensor (e.g. using "detectAndSelectMifareTag()").
 * There is no need to authenticate in the tag prior to calling this function.
 * This function does not stop authentication (so, in case of success, the tag is still authenticated in the sector of the last block read). 
 * 
 * Returns: negative number -- error
 *          positive number -- number of bytes read (from the tag to the output array)
 */
int EasyMFRC522::readRaw(int initialBlock, byte* dataOutput, int dataSize) {
  MFRC522::StatusCode status;
  int bytesRead = 0;
 
  int currBlock = initialBlock;
  int currSector = currBlock / 4;
  bool sectorAuthenticated = false; 
  
  while (bytesRead < dataSize) {      
    //TODO: support other Mifare Classic tags
    if (currBlock % 4 == 3) {  //attention: don't exclude block #0 here; excluded only in write operations
      dbgPrint("   - Ignored block: "); dbgPrintln(currBlock);
      currBlock ++;
      currSector ++;
      sectorAuthenticated = false; //because the sector changed
    }

    // TODO: adapt to other types of tags - this is specific of Mifare 1k !
    if (currBlock > 63) {
      dbgPrint("Error readRaw(): end of tag's memory reached");
      return -120;
    }

    if (! sectorAuthenticated) {
      dbgPrint("   - Authenticating sector: "); dbgPrintln(currSector); 

      for (int i = 0; i < READ_WRITE_TRIALS; i ++) {
        status = device.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, currBlock, &key, &(device.uid));
        if (status == MFRC522::STATUS_OK) {
          break;
        }
        dbgPrintln(F("    na"));
      }
      if (status != MFRC522::STATUS_OK) {
        dbgPrint("Error readRaw(): could not authenticate block "); dbgPrintln(currBlock);
        return -121;
      }
      sectorAuthenticated = true;
    }

    int code;
    int bytes = dataSize - bytesRead;
    bytes = (bytes < 16)? bytes : 16;
    
    for (int i = 0; i < READ_WRITE_TRIALS; i ++) {
      code = _readBlock(currBlock, dataOutput, bytesRead, bytes);
      if (code >= 0) { // success
        bytesRead += bytes;
        break;
      }
    }
    if (code < 0) {
      //in this point, a message should have been printed by _readBlock()
      return -100 + code;
    }

    currBlock ++;
  }

  return dataSize;
}

int EasyMFRC522::_readBlock(int block, byte* destiny, byte firstIndex, byte bytesToRead) {
  MFRC522::StatusCode status;
  byte bufferSize = 18;

  status = device.MIFARE_Read(block, blockBuffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
    dbgPrint("Error readBlock(): could not read block ");  dbgPrintln(block);
    return -1;
  }

  if (bytesToRead < 0 || bytesToRead > 16) {
    dbgPrint("Error readBlock(): invalid size");
    return -2;
  }

  for (int i = 0; i < bytesToRead; i ++) {
    destiny[firstIndex + i] = blockBuffer[i];
  }

  return 0;
}


//////////////////////////////////////////////////////
////////// READ/WRITE LABELED DATA (files) ///////////

int EasyMFRC522::writeFile(byte initialBlock, const char dataLabel[12], byte* data, int dataSize) {
  // prepares the buffer with the content for the initial block  
  blockBuffer[0] = 0x1C; // ASCII FILE SEPARATOR (=28 decimal)
  for (int i = 0; i < 12; i ++) {
    blockBuffer[i+1] = dataLabel[i];
    if (dataLabel[i] == '\0') {
      break;
    }
  }
  //blockBuffer[13] = flags; //for future use

  if (dataSize < 0) {
    dataSize = 0;
  }
  blockBuffer[14] = byte(dataSize);
  blockBuffer[15] = byte(dataSize >> 8);

  int lastBlockUsed = this->writeRaw(initialBlock, blockBuffer, 16); // attention: NEVER write less than 16 bytes using the blockBuffer as source (to prevent name aliasing inside writeRaw())
  if (lastBlockUsed < 0) {
    //the message should already have been printed by writeRaw, so just return the error code
    return -2000 + lastBlockUsed; // error code (see comment in the end of this file)
  }

  int status = this->writeRaw(lastBlockUsed+1, data, dataSize);
  if (status < 0) {
    return -2500 + status;        // error code (see comment in the end of this file)
  }

  return status;
}

/**
 * Reads the identification block, checking if it is properly identified with the given 
 * data label, then parses and returns the size of the data stored in the remaining blocks.
 * 
 * Returns: negative number -- error
 *          positive number -- the size of the data stored (starting from the next block, not counting trailling blocks)
 */
int EasyMFRC522::readFileSize(int initialBlock, const char dataLabel[12]) {
  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;

  if (initialBlock % 4 == 3) { //it is a trailer block --> go to the next one
    initialBlock ++;
  }

  for (int i = 0; i < READ_WRITE_TRIALS; i ++) {
    status = this->device.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, initialBlock, &this->key, &(this->device.uid));
    if (status == MFRC522::STATUS_OK) {
      break;
    }
    dbgPrintln(F("    na"));
  }
  if (status != MFRC522::STATUS_OK) {
    dbgPrintln("Error readFileSize(): could not authenticate");
    return -8;
  }

  byte bufferSize = 18;
  for (int i = 0; i < READ_WRITE_TRIALS; i ++) {
    status = this->device.MIFARE_Read(initialBlock, blockBuffer, &bufferSize);
    if (status == MFRC522::STATUS_OK) {
      break;
    }
  }
  if (status != MFRC522::STATUS_OK) {
    dbgPrintln("Error readFileSize(): could not read");
    return -9;
  }

  if (blockBuffer[0] != 0x1C) {
    dbgPrintln("Error readFileSize(): this block does not start a file");
    return -10;
  }
  
  // checks if all characters of the data label matches, including the final \0
  // if the label has more than 12 chars, only the first 12 chars are considered
  for (int i = 0; i < 12; i++) {
    if (dataLabel[i] != blockBuffer[i+1]) {
      dbgPrintln("Error readFileSize(): data label doesn't match");
      return -11;
    }
    if (dataLabel[i] == '\0')  //in this situation, we have already confirmed that blockBuffer[i+1] == '\0' in the "if" above
      break;
  }

  //int flags = blockBuffer[13]; //for future use

  int dataSize = ((unsigned int)blockBuffer[15] << 8) | (unsigned int)blockBuffer[14];
  return dataSize;
}

int EasyMFRC522::readFile(byte initialBlock, const char dataLabel[12], byte* dataOut, int dataOutCapacity) {
  if (initialBlock % 4 == 3) { //it is a sector's trailing block --> go to the next (attention: this is done in readFileSize(), but should be kept here too, because of special cases, e.g. initialBlock = 3) and these functions are called successively)
    initialBlock ++;
  }

  int dataSize = this->readFileSize(initialBlock, dataLabel);
  if (dataSize < 0) {
    return -1000 + dataSize; // error code (see comment in the end of this file)
  } else if ((int)dataOutCapacity < dataSize) {
    dbgPrintln("Error in readFile(): not enough room in the given output buffer");
    return -1020;
  }
  dbgPrint(" -- data size: "); dbgPrintln(dataSize);

  int status = this->readRaw(initialBlock+1, dataOut, dataSize);
  if (status < 0) { 
    return -1000 + status; // error code (see comment in the end of this file)
  }

  return status;
}


/**
 * -----------
 * ERROR CODES
 * -----------
 * Error codes are negative integers returned by the functions.
 * 
 * We show below, what codes each function may return. If you get 
 * one error code, you can use the information below to identify 
 * its origin.
 * 
 * _readBlock ->
 * -1 | -2
 * 
 * _writeBlockAndVerify ->
 * -5 | -6 | -7 | _verifyBlock
 * 
 * _verifyBlock ->
 * -3 | -4
 * 
 * readFileSize ->
 * -8 | -9 | -10 | -11
 * 
 * readRaw (unlabelled) ->
 * -120 | -121 | (-100 + _readBlock)
 * 
 * writeRaw (unlabelled) ->
 * -220 | -221 | (-200 + _writeBlockAndVerify) 
 * 
 * readFile ->
 * -1020 | (-1000 + readFileSize) | readRaw
 * 
 * writeFile ->
 * (-2000 + writeRaw) | (-2500 + writeRaw)
 * 
 * Because some functions may call others, and one function may be
 * called in multiple places, we used a combination of values that
 * prevent producing the same value (when the same function is 
 * called in different places.) In other words, each value identifies
 * exactly were, in the code, the error was originated.
 */