# easy_mfrc522

To combine these requirements: (I) easy of use (small client code); 
(II) small time/memory footprint; (III) to allow one to check if a previous "version"
of the desired data is already recorded on the tag (like a file in a file system).)
 
This library is a wrapper for <MFRC522.h> that provides two classes to easily read 
 from and write to Mifare Classic tags (PICC). (Attention: tested only on Mifare 1k tags!)
  
 Class EasyMFRC522 allows reading/writting data chunks spanning multiple sectors in 
 a single call. The data may be unlabelled with a known fixed size; or may be 
 labelled (with a string) with arbitrary size. Multiple data chunks may be written to
 a single PICC (tag).
  
 Class RfidDictionaryView allows reading/writting string data as pairs "key-value".
 (It works like a "dictionary" of "map" data structure).
 