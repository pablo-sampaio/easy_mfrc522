
#ifndef __RFID_DICTIONARY___
#define __RFID_DICTIONARY___

#include <EasyMFRC522.h>

/**
 * Allows to access and write each RFID tag as a dictionary (associative array) with 
 * arbitrary number of entries. Each entry is given as a "key-value" pair of strings. 
 * The "key" string must be unique. This class is similar to Python dictionaries or
 * Java HashMaps, but allowing only strings, and reading/writing to each RFID.
 * 
 * Each "key" may also be understood as an attribute or field, for which you may assign 
 * a single value. (Also, the term "key" here must no be confused with the authentication 
 * "keys" A and B used to access RFID tags). 
 * 
 * The main operations are available: 
 * 
 *   .set(key, value) - adds or replaces an entry to the dictionary, for the given key; 
 *                      the data is immediatelly written to the RFID tag
 *   .get(key)        - retrieves the value associated to the (unique) key.
 *   .remove(key)     - removes the key-value entry; the entry is immediatelly removed 
 *                      from the RFID tag
 * 
 * There's no intialization function (the MFRC522 is initialized automatically). 
 * Before using the operations above, you must stablish connection to a RFID tag in
 * the range of the reader. Use:
 * 
 *   detectTag()     - stablishes connection and loads all key-value entries
 *   disconnectTag() - disconnets and unloads the data
 * 
 * Assumptions for using this class:
 * 1 - The same for EasyMFRC522 (i.e using the same authentication key A in all blocks, 
 *     existence of single tag in the range)
 * 2 - The data (you want to store/read) is composed of pairs of strings key -> value
 *     (or attribute -> value) as explained above
 */
class RfidDictionaryView { 
private:
    EasyMFRC522* device;
    int startBlock;      // Number of block where the dictionary starts 
    bool deleteDevice;   // Indicates whether the EasyMFRC522 must be deallocated by this class

    String *dictionary;  // Array used as a dictionary, alternating keys (in even indices) and values (odd indices)
    int capacity;        // Maximum size of the array (extended as needed)
    int size;            // Used size of the dictionary
    bool loaded;         // Indicates if the dictionary was already loaded from the currently selected RFID tag
    byte tag_uid[4];     // UID of the tag from where the data were loaded

public:

    RfidDictionaryView(byte sdaPin, byte resetPin, int startBlock = 1);
    RfidDictionaryView(EasyMFRC522* rfidDevice, int startBlock = 1, bool autoDeallocateDevice = false);
    virtual ~RfidDictionaryView();

    bool detectTag(byte outputTagId[4] = NULL);
    void disconnectTag(bool allowRedetection = false);

    void set(const String& keyString, const String& valueString);
    String get(const String& keyString);

    void remove(const String& keyString);
    bool hasKey(const String& keyString);
    
    String getKey(int entryIndex);
    int getNumEntries();
    inline int getNumKeys() {
        return this->getNumEntries();
    }

    int getMaxSpaceInTag();

private:
    // auxiliary functions

    void _ensure_loaded();

    void _read_dictionary();
    void _write_dictionary() ;
    int _dict_find(const String& key);
    inline bool _dict_has_key(const String& key) {
        return _dict_find(key) >= 0;
    }
    void _grow_dict();

};

#endif