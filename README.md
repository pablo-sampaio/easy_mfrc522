# easy_mfrc522

This library controls a MFRC522 device to combine these requirements: 
1. easy of use (small client code) 
1. small time/memory footprint 
1. to allow one to check if a previous "version" of the desired data is already recorded on the tag.

This library is a wrapper for <MFRC522.h> that provides two classes to easily read 
 from and write to Mifare Classic tags (PICC). 
  
 Class EasyMFRC522 allows reading/writting data chunks spanning multiple sectors in 
 a single call. The data may be unlabelled with a known fixed size, or may be 
 labelled (with a string) with arbitrary size. Multiple data chunks may be written to
 a single PICC (tag).
  
 Class RfidDictionaryView allows one to access a tag like a "dictionary" (or "map") data structure, that is, by reading/writting data as pairs "key-value" (both must
 be strings).
  
 (Attention: tested only with Mifare 1k tags!)
 