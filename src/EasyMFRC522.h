
#ifndef __EASY_MFRC522_H__
#define __EASY_MFRC522_H__

#include <MFRC522.h>

/**
 * This library is a wrapper for <MFRC522.h> that provides two classes to easily read 
 * from and write to Mifare Classic tags (PICC). (Attention: tested only on Mifare 1k tags!)
 * 
 * Class EasyMFRC522 allows reading/writting data chunks spanning multiple sectors in 
 * a single call. The data may be unlabelled with a known fixed size; or may be 
 * labelled (with a string) with arbitrary size. Multiple data chunks may be written to
 * a single tag.
 * 
 * Class RfidDictionaryView allows reading/writting string data as pairs "key-value".
 * (It works like a "dictionary" of "map" data structure).
 * 
 * Pablo A. Sampaio, 2018, 2021
 */


/**
 * Allows to read (or write) data chunks from (or to) multiple sequential blocks 
 * and sectors of the tag (PICC), in a single operation (function call), from a 
 * given starting block. 
 * 
 * You can write data of arbitrary length (that fits the tag). This class will
 * write sequentially to the blocks, but skipping special blocks.
 *
 * You can read/write labelled or unlabelled data chunks starting on any block.
 * You may write multiple of them, maybe mixing unlabelled/labelled, in a single 
 * tag, but you must prevent overlapping of data chunks yourself.
 * 
 * When writting a labelled data, you provide a string (parameter "fileName")
 * to identify the data chunk. The advantage of writting labelled data is data: 
 * (1) you may test if the data chunk is or not present on a tag, by its label;
 * (2) you don't need to know the exact size of the data chunk in a read operation;
 * (3) you may even query its size (before reading the whole data chunk).
 *  
 * Assumes:
 * 1 - All sectors use the same key A to authenticate. 
 * 2 - And that all blocks are in "transport configuration" (therefore, read and write
 *     operations can be done using anyone of the keys, but we only use key A).
 * 3 - There is always a single tag in the detection range
 */
class EasyMFRC522 {
private:
    MFRC522 device;
    MFRC522::MIFARE_Key key;

    byte blockBuffer[18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int _writeBlockAndVerify(int blockAddr, byte* data, int startIndex, int bytesToWrite);
    int _readBlock(int blockAddr, byte* destiny, byte firstIndex, byte bytesToRead);
    int _verifyBlock(int blockAddr, byte* refData, byte startByte, byte bytesToCheck);

public:

	EasyMFRC522(byte sdaPin, byte resetPin);
    virtual ~EasyMFRC522();

    // configurations
    void init();
    void setKeyA(byte keyA[6]);

    inline MFRC522* getMFRC522() {
        return &this->device;
    }
    inline MFRC522::MIFARE_Key* getKey() {
        return &this->key;
    }

    // detection of tags
    bool detectTag(byte outputTagId[4] = NULL);

    // parameter indicates if the same tag should be detectable immediately
    // again (withou having to move away and back again)
    void unselectMifareTag(bool allowRedetection = true);

    int getUserDataSpace(int startBlock = 0);

    /* The functions below read/write labeled labeled (name) data, that are somewhat 
     * similar to "files", where the file name (data label) AND the start block
     * must be provided either to read the size of the data (with readSize()) or 
     * to read/write the whole data.
     * 
     * Duplicate names are allowed on a tag, in different blocks. The pair 
     * "start block" + "label" uniquely identifies your data chunk in a tag.
     * 
     * Each data name is a null(zero)-terminated string, with maximum size 12. 
     * In longer names, only the first 12 chars are considered.
     * 
     * The data is read/write from/to a contiguous series of blocks starting in the 
     * given initial block, possibly spanning multiple sectors, but skipping the 
     * trailling block of each sector.
     * 
     * Therefore, when reading the name AND the initial block must be the exact names 
     * and initial blocks used when the data were written.
     */

    int writeFile(byte initialBlock, const char fileName[13], byte* data, int dataSize);
    inline int writeFile(byte initialBlock, String fileName, byte* data, int dataSize) {
        char buffer[13];
        fileName.toCharArray(buffer, 13);
        return writeFile(initialBlock, buffer, data, dataSize);
    }

    int readFile(byte initialBlock, const char fileName[13], byte* dataOut, int dataOutCapacity);
    inline int readFile(byte initialBlock, String fileName, byte* dataOut, int dataOutCapacity) {
        char buffer[13];
        fileName.toCharArray(buffer, 13);
        return readFile(initialBlock, buffer, dataOut, dataOutCapacity);
    }

    int readFileSize(int initialBlock, const char fileName[13]);
    inline int readFileSize(int initialBlock, String fileName) {
        char buffer[13];
        fileName.toCharArray(buffer, 13);
        return readFileSize(initialBlock, buffer);
    }

    inline bool existsFile(int initialBlock, const char fileName[13]) {
        return readFileSize(initialBlock, fileName) >= 0;
    }
    inline bool existsFile(int initialBlock, String fileName) {
        return readFileSize(initialBlock, fileName) >= 0;
    }

    /* These member functions don't assign labels to the data, so the size of 
     * the data cannot be properly retrieved by this class (with readSize). 
     * 
     * The data is also read/write from/to a contiguous series of blocks starting 
     * in the given initial block, possibly spanning multiple sectors, but 
     * skipping the trailling block of each sector.
     * 
     * These functions may be useful to implement a different organization of the
     * data (in the tag's memory).
     */

    int writeRaw(int initialBlock, byte* data, int dataSize);
    
    int readRaw(int initialBlock, byte* dataOutput, int dataSize);

};


// Just to make ir easier for the user. 
// He/she will only need to include one header to use any of the classes.
#include "RfidDictionaryView.h"


#endif
