# Easy MFRC522 Library

This library allows you to read/write data to RFID cards in a simple fashion, using a **MFRC522**\* module (properly connected to a development board, like Arduino). 

It was developed to be *easy to use*, (subjectively) defined by these requirements:
1. small client code
1. few function calls 
1. read/write data with arbitrary length seamlessly
1. encapsulate repeated/protocolar operations (required by the RFID tags)

The library supports these boards: *Arduino*, *NodeMCU* and similar boards. 

Avaiable on *PlatformIO* and *Arduino IDE*. 

\* *MFRC522* is a module to read/write contactless cards/tags based on RFID tecnology (also called contactless smartcards, proximiy cards, PICCs, etc). These cards have a somewhat intricate internal memory organization accessed with non-trivial protocols. This library was created to simplify things as much as possible. 

---

## Functionalities

*Easy MFRC522 library* offers different alternatives to read/write data chunks of arbitrary length (possibly spanning multiple sectors of a tag), in a single function call.

Two classes are provided (they should become *three* in the next major release).

### 1. Class **EasyMFRC522** 

This class allows one to read/write **binary data chunks**. You provide the *start block* and the operation reads/writes from/to all blocks needed. You may read/write in two ways:

  * **unlabeled data**, where you (in your code) don't provide a label for the data chunk (so the start block is the only identification); you must provide the exact size of the data chunk when reading it (and obviously when writing it too).
  * **labeled data (file)**, where you provide a string to identify (together with the start block) your data chunk; this mode gives you some facilities: (1) you may query if the file is present in the card, (2) or may query the data size (without actually reading its content), and (3) the read operation doesn't require the data size.
 
 ### 2. Class **RfidDictionaryView** 
 
 This class allows one to access the memory of the tag as a *dictionary* (or *associative array*) data structure. In other words: 
 
   * your data is kept as pairs of strings **key-value**, where the key is unique
   * a **read()** operation receives only the *key* (a string), and returns the associated *value* (also a string)
   * a **write()** operation receives both, and may either update the *value* for the given *key* (if the *key* already exists), or add the whole pair (if the *key* is not present)
   * the data is automatically updated on the RFID tag seamlessly.

 **Attention**: *The "keys" mentioned in the class RfidDictionaryView is not related to the "authentication keys (A and B)" used in Mifare tags*. They are "keys" in the sense used in associative arrays (like Python's dictionary, or Java's HashMap or TreeMap).
 
### 3. Other Operations

This library currently wraps up [Balboa's library](https://github.com/miguelbalboa/rfid). My aim was just to add functionality, not to replace it. 

So, if you want to do other operations supported by MFRC522, you can access (the wrapped instance of) Balboa's class and use its functionality. Just call getMFRC522().

---

## Potential Target Users

You may probably find this library useful if:

* You are learning Arduino/NodeMCU/etc and want to try read data from (or write to) RFID tags
* You are developing a project to use RFID tags/cards to store and read (relatively) large data chunks

## Installation

On **PlatformIO (PIO)**, to install in your PIO project: 
  1. Open the file named **platformio.ini** inside your project (there should be one). After the field "lib_deps =", create a new line and add (with some identation) "pablo-sampaio/Easy MFRC522 @ ~0.2.1".
  1. The library will automatically be downloaded to your project, and you may include *"EasyMFRC522.h"* in any C/C++ file you want.
  1. (Alternatively: click on the PIO sidebar icon, click on "Libraries", search for "Easy MFRC522", click on "Easy MFRC522 by Pablo Sampaio", click on "Add to Project" and then follow the instructions.)

On **Arduino IDE**: 
  1. Click on "Sketch" menu, then choose "Include Library" -> "Manage Libraries". 
  1. The "Library Manager" will open. Then search for "Easy MFRC522".
  1. Then click on "Install". (You do it once, and it will be available for *all* projects).

## Limitations

Due to the simplifications adopted or due to the lack of time and resources, this library has some limitations:

* It only supports RFID cards of [Mifare Classic](https://en.wikipedia.org/wiki/MIFARE) family (mini, 1K and 4K)
* All blocks are accessed only in transport mode, and using only Key A for read/write operations
* The same authentication *key A* must be used in all blocks on which you do a read/write operation
* It doesn't (directly) provide comprehensive functionality for using MFRC522, but it depends on [Balboa's library](https://github.com/miguelbalboa/rfid), so you can access Balboa's MFRC522 class and use many other basic functionalities.
* Currently, it is tested only on Mifare 1k tags (the model that is most widely available on the market)

 ## Licensing

 Currently licensed under LGPGL v3.

## Acknowledgements

Thanks to _Vinicius Barbosa Polito_ who contributed to an earlier C project that evoluted to the RfidDictionaryView class.
