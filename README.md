# Easy MFRC522 Library

This library allows you to read/write data to RFID cards in a simple fashion. The objective for its creation was the **ease of use**, (subjectively) defined by these features:
1. small client code
1. few function calls 
1. abstract away protocolar operations

It supports Arduino and NodeMCU boards. Avaiable on *PlatformIO*. 

**For beginners**: MFRC522 is a module to read/write contactless cards/tags based on RFID tecnology (also called contactless smartcards, proximiy cards, PICCs, etc). These cards have an intricate internal memory organization, accessed with non-trivial protocols. This library was created to simplify things as much as possible.


## Functionalities

It offers different ways to read/write data chunks of arbitrary length (possibly spanning multiple sectors of a tag), in a single function call.

Two classes are provided:

1. Class **EasyMFRC522** allows one to read/write **binary data chunks** spanning multiple sectors in a single operation. You provide the *start block* and the operation reads/writes all blocks needed. You may read/write in two modes:
  * **unlabeled**, where you (in your code) don't provide a label for the data chunk (the start block is the only identification)
  * **labeled**, where you provide a string to identify (together with the start block) your data chunk; this mode allows some facilities: you may query the data size without reading all the data chunk, simpler read operation, etc.
  
 2. Class **RfidDictionaryView** allows one to access the tag's memory as a *dictionary* (or *map*) data structure; that is: 
   * your data is kept as **key-value** pairs (both must be strings)
   * a **read** operation requires only the key 
   * a **write** operation requires both, and may either update the *value* for the given *key* (if the *key* already exists), or add the whole pair (if the *key* is not present).
 
 **Attention**: *The "keys" mentioned above have no relation to the "authentication keys (A and B)" used in Mifare tags*.

## Potential Target Users

* People learning how to use Arduino/NodeMCU/etc to read/write RFID tags
* Anyone that wants to use RFID tags/cards to store and read (relatively) large data chunks

## Instalation

On PlatformIO...

On Arduino IDE...

## Restrictions

The simplifications adopted and the lack of time, made this library restricted in these ways:

* It only supports RFID cards of [Mifare Classic](https://en.wikipedia.org/wiki/MIFARE) family (mini, 1K and 4K)
* It offers only basic read/write operations (but you may access the wrapped Balboa's class to do other operations)
* All blocks are accessed only in transport mode, and using only Key A for read/write operations
* The same key A must be used by all blocks on which you do a read/write operation

**Attention 1**: *This is not a comprehensive library for using MFRC522*. If you need just that, check  [Balboa's library](https://github.com/miguelbalboa/rfid). This library is currently a wrapper of Balboa's.

**Attention 2**: *Tested only with Mifare 1k tags!*

 ## Licensing

 Currently licensed under LGPGL.
