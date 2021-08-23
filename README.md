# Easy MFRC522 Library

This library allows you to read/write data to RFID cards in a simple fashion. The objective for its creation was the **ease of use**, (subjectively) defined by these features:
1. small client code
1. few function calls 
1. abstract away protocolar operations

It supports Arduino and NodeMCU boards. Avaiable on *PlatformIO*. 

**For beginners**: MFRC522 is a module to read/write contactless cards/tags based on RFID tecnology (also called contactless smartcards, proximiy cards, PICCs, etc). These cards have an intricate internal memory organization, accessed with non-trivial protocols. This library was created to simplify things as much as possible. 

## Functionalities

*Easy MFRC522 library* offers different alternatives to read/write data chunks of arbitrary length (possibly spanning multiple sectors of a tag), in a single function call.

Two classes are provided:

1. Class **EasyMFRC522** allows one to read/write **binary data chunks**. You provide the *start block* and the operation reads/writes all blocks needed. You may read/write in two ways:
  * **unlabeled data**, where you (in your code) don't provide a label for the data chunk (so the start block is the only identification); you must provide the exact size of the data chunk when reading it (and obviously when writing it too).
  * **labeled data (file)**, where you provide a string to identify (together with the start block) your data chunk; this mode allows some facilities: you may query if the file is present in the card, or may query the data size (without actually reading the whole data chunk), and your read operation don't require the exact size of the data.
  
 2. Class **RfidDictionaryView** allows one to access the memory of the tag as a *dictionary* (or *associative array*) data structure; that is: 
   * your data is kept as **key-value** pairs (both must be strings)
   * a **read** operation requires only the key 
   * a **write** operation requires both, and may either update the *value* for the given *key* (if the *key* already exists), or add the whole pair (if the *key* is not present).

This library is currently a wrapper of [Balboa's library](https://github.com/miguelbalboa/rfid). My aim was just to add functionality, not to replace it. If you want, you can access (the wrapped) Balboa's class to use its functions.

 **Attention**: *The "keys" mentioned in the second class have no relation to the "authentication keys (A and B)" used in Mifare tags*. They are "keys" in the sense used in dictionaries or hash maps or associative arrays.

## Potential Target Users

* People learning how to use Arduino/NodeMCU/etc to read/write RFID tags
* Anyone that wants to use RFID tags/cards to store and read (relatively) large data chunks

## Instalation

On *PlatformIO (PIO)*: To install in your PIO project: 
  1. Open your platformio.ini file
  1. After the field "lib_deps =", create a new line and add (with some identation) "pablo-sampaio/Easy MFRC522 @ ~0.2.0"
  1. The library will be automatically downloaded your project, and you may include "Easy MFRC522.h" in any file you want 

(Alternatively: click on the PIO sidebar icon, click on "Libraries", search for "Easy MFRC522", click on "Easy MFRC522 by Pablo Sampaio", click on "Add to Project" and then follow the instructions.)

On *Arduino IDE*: 
  1. Click on "Sketch" menu, then choose "Include Library" -> "Manage Libraries". 
  1. The "Library Manager" will open. Then search for "Easy MFRC522".
  1. Then click on "Install".

## Limitations

Due to the simplifications adopted (and to the lack of time and resources), this library has some limitations:

* It only supports RFID cards of [Mifare Classic](https://en.wikipedia.org/wiki/MIFARE) family (mini, 1K and 4K)
* It offers only basic read/write operations (but you may access the wrapped Balboa's class to do other operations)
* All blocks are accessed only in transport mode, and using only Key A for read/write operations
* The same key A must be used by all blocks on which you do a read/write operation

**Attention 1**: *This is not a library with comprehensive functionality for using MFRC522*. If you need just that, check out [Balboa's library](https://github.com/miguelbalboa/rfid). This library is currently a wrapper of Balboa's, so you can access Balboa's class and use all its functionality.

**Attention 2**: *Tested only with Mifare 1k tags!*

 ## Licensing

 Currently licensed under LGPGL.

## Acknowledgements

Thanks to _Vinicius Barbosa Polito_ who contributed to an earlier project that would become the RfidDictionaryView class.
