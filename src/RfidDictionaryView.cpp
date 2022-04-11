
#include "RfidDictionaryView.h"

#define INITIAL_CAPACITY    30   // must be a non-negative even number; the number of pairs key/value is half this value
#define CAPACITY_INCREMENT  30   // must be a non-negative even number; when it is necessary to grow, increment by this value


RfidDictionaryView::RfidDictionaryView(EasyMFRC522* rfidDevice, int startBlock, bool autoDeallocateDevice) {
  this->device = rfidDevice;
  this->startBlock = startBlock;
  this->deleteDevice = autoDeallocateDevice;
  this->capacity = INITIAL_CAPACITY;
  this->dictionary = new String[INITIAL_CAPACITY];
  this->size = 0;
  this->loaded = false;
  for (int i = 0; i < 4; i ++) {
    this->tag_uid[i] = 0x00;
  }
}

RfidDictionaryView::RfidDictionaryView(byte sdaPin, byte resetPin, int startBlock)
  : RfidDictionaryView(new EasyMFRC522(sdaPin, resetPin), startBlock, true) 
{
  // calls the other constructor above, creating the EasyMFRC522 instance and
  // enabling the flag to auto delete it in the destructor
  this->device->init(); 
}

RfidDictionaryView::~RfidDictionaryView() {
  this->loaded = false;
  delete[] this->dictionary;
  if (this->deleteDevice) {
    delete this->device;
  }
}

/** 
 * Returns the space in bytes available exclusively for storing the dictionary entries
 * (not counting space for metadata).
 */
int RfidDictionaryView::getMaxSpaceInTag() {
  // subtracts 16 bytes (1 block), which is the space used by EasyMFRC522 to store file metadata
  return this->device->getUserDataSpace(this->startBlock) - 16;
}

//---- RFID **INTERNAL** FUNCTIONS --------------------------------------//

/**
 * This function takes the data stored in the buffer and breaks it into an
 * internal array of strings, representing a dictionary. 
 * In the array, a "key" string is followed by its corresponding "value" 
 * string, in this order. 
 * Ex.: to represent a dictionary with one single entry <"Age" : 20>,
 *      set dictionary[0] = "Age" and dictionary[1] = "20".
 */
void RfidDictionaryView::_read_dictionary() {
  // reset the dictionary
  for (int i = 0; i < this->capacity; i++) {
    this->dictionary[i] = "";
  }
  this->size = 0;

  // subtracts 16 (1 block), which is the space used by EasyMFRC522 to store file information
  int spaceForDictInTag = getMaxSpaceInTag();

  if (! this->device->existsFile(this->startBlock, "_rfiddict_")) {
    // in this case, it is considered loaded as an empty dictionary
    this->loaded = true; 
    // copies the uid of the current tag
    MFRC522* mfrc522 = this->device->getMFRC522();
    for (int i = 0; i < 4; i ++) {
      this->tag_uid[i] = mfrc522->uid.uidByte[i];
    }
    return;
  }
 
  byte *buffer = new byte[spaceForDictInTag];

  // Lê e coloca as informações da tag no buffer
  int result = this->device->readFile(this->startBlock, "_rfiddict_", buffer, spaceForDictInTag);
  
  if (result < 0) {
    Serial.print  ("Error: when reading the RFID tag, got ");
    Serial.println(result);
    this->loaded = false;
    delete[] buffer;
    return;
  }

  // Creates the dicionary from the data that was read from the tag
  // Obs.: result contains the number of bytes that were actually read
  int pos = 0;
  
  for (pos = 0; pos < result; pos ++) {
    if ((char)buffer[pos] == '\n') {  //mudar para \0
      this->size++;
      if (this->size >= this->capacity) {
        //grows to be able to add more strings
        _grow_dict();
      }
    
    } else {
      //isso pode ser mais eficiente... ideia: setar \0 e passar o buffer+contador
      this->dictionary[this->size].concat((char)buffer[pos]);
    }
  }

  // copies the uid of the current tag
  MFRC522* mfrc522 = this->device->getMFRC522();
  for (int i = 0; i < 4; i ++) {
    this->tag_uid[i] = mfrc522->uid.uidByte[i];
  }

  this->loaded = true;
  delete[] buffer;
}

// Finds the index of a key; assumes the dictionary is loaded.
int RfidDictionaryView::_dict_find(const String& key) {
  for (int i = 0; i < this->size; i = i + 2) {
    if (this->dictionary[i].equals(key)) {
      return i;
    }
  }
  return -1;
}

// Writes the dictionary to the tag; assumes the dictionary is loaded.
void RfidDictionaryView::_write_dictionary() {
  String dictString = "";
  int nextEntrySize;
  int spaceForDictInTag = getMaxSpaceInTag();

  for (int i = 0; i < this->size; i += 2) {
    nextEntrySize =  this->dictionary[i].length() + this->dictionary[i+1].length() + 2;
    if ( (int(dictString.length()) + nextEntrySize) <= spaceForDictInTag ) {
      dictString.concat(dictionary[i]);
      dictString.concat("\n");
      dictString.concat(dictionary[i+1]);
      dictString.concat("\n");
    } else {
      this->loaded = false;
      Serial.println("Error: Dictionary is too big! Some entries were not written!");
      break;
    }
  }

  int stringSize = dictString.length();    // should be the same as strlen(buffer) 
  char *buffer = new char[stringSize + 1]; // adds +1 space for the '\0' that terminates the string (written by toCharArray)
  dictString.toCharArray(buffer, stringSize+1);

  int result = this->device->writeFile(this->startBlock, "_rfiddict_", (byte*)buffer, stringSize);
  delete[] buffer;

  if (result <= 0) {
    this->loaded = false;
    Serial.print  ("Error: Could not write to the tag, got ");
    Serial.println(result);
  }

}

void RfidDictionaryView::_grow_dict() {
  this->capacity += CAPACITY_INCREMENT;
  String* new_dict = new String[this->capacity];

  for (int i = 0; i < this->size; i ++) {
    new_dict[i] = this->dictionary[i];
  }

  delete[] this->dictionary;
  this->dictionary = new_dict;
}

void RfidDictionaryView::_ensure_loaded() {
  if (loaded) {
    // checks the uid of the tag
    MFRC522 *mfrc522 = this->device->getMFRC522();
    for (int i = 0; i < 4; i ++) {
      if (this->tag_uid[i] != mfrc522->uid.uidByte[i]) {
        // the current tag (connected/selected by the device) is different
        loaded = false;
        break;
      }
    }
  }

  if (!loaded) { //don't refactor this block as an else!
    _read_dictionary();
  }
}

//---- RFID **PUBLIC** FUNCTIONS ----------------------------------------//

/**
 * This function selects and stablishes connection to an RFID tag. 
 * If you do not connect first, the other functions will not work.
 * 
 * If you connect directly with the corresponding functions of classes
 * EasyMFRC522 or MFRC522, some operations of this class may not work.
 * But, once connected with this function, you may do any operations
 * of those classes.
 */
bool RfidDictionaryView::detectTag(byte outputTagId[4]) {
  // try at most twice
  bool tag_detected = this->device->detectTag(outputTagId);
  if (!tag_detected) {
    tag_detected = this->device->detectTag(outputTagId);
  }

  if (tag_detected) {
    this->loaded = false;
    //obs.: this class' field "tag_uid" will be loaded in _read_dict(), which will be
    //called in the first call to a public operation (because loaded is false)
  }

  return tag_detected;
}

/**
 * This function deselects the tag. After this, the other rfid functions will 
 * not work properly.
 * Known issue: you will need to physically move away (from the RFID reader), 
 * then back again, before connecting another time.
 */
void RfidDictionaryView::disconnectTag(bool allowRedetection) {
  this->device->unselectMifareTag(allowRedetection);
  this->loaded = false;
  this->size = 0;
}

int RfidDictionaryView::getNumEntries() {
  _ensure_loaded();
  if (! this->loaded) {
    return -1;
  }

  return this->size/2;
}

String RfidDictionaryView::getKey(int key_index) {
  _ensure_loaded();
  if (! this->loaded) {
    return "";
  }

  if (2*key_index >= this->size) {
    Serial.print   ("Error: invalid index ");
    Serial.println(key_index);
    return "";
  }
  return dictionary[2*key_index];
}

bool RfidDictionaryView::hasKey(const String& dict_key) {
  _ensure_loaded();
  if (! this->loaded) {
    return false;
  }
  return _dict_has_key(dict_key);
}

/**
 * In this function you give the key that you want to remove from the dictionary.
 * It will remove the key and its value.
 */
void RfidDictionaryView::remove(const String& key) {
  _ensure_loaded();
  if (! this->loaded) {
    return ;
  }

  int key_index = _dict_find(key);

  if (key_index < 0) {
    Serial.println("Error: key not found!");
    return;
  }

  //Remove the key and its value from the array
  for (int i = key_index; i < this->size; i++) {
    this->dictionary[i] = this->dictionary[i+2];
  }
  this->size -= 2;

  _write_dictionary();
}

/**
 * In this function you give the key and it returns the value associated to that key.
 * If the key does not exist, it will return the empty string "".
 */
String RfidDictionaryView::get(const String& key) {
  _ensure_loaded();
  if (! this->loaded) {
    return "";
  }

  for (int i = 0; i < this->size; i = i + 2) {
    if (this->dictionary[i] == key) {
      return this->dictionary[i + 1];
    }
  }

  return String("");
}

/**
 * This function is given a key and a value.
 * If the key exists, it overwrites the old value with the new one.
 * If it does not exist, the pair <key : value> is added.
 */
void RfidDictionaryView::set(const String& key, const String& value) {
  _ensure_loaded();
  if (! this->loaded) {
    return ;
  }

  int key_index = _dict_find(key);

  if (key_index >= 0) {
    //if the key exists, updates only the value (kept in the next index)
    this->dictionary[key_index+1] = value;
    _write_dictionary();
  
  } else {
    //if the key does not exist and there is no room, allocates more space
    if (this->size+2 > this->capacity) {
      _grow_dict();
    }

    this->dictionary[this->size] = key;
    this->dictionary[this->size+1] = value;
    this->size += 2;
    _write_dictionary();
  }

}
