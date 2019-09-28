#include <openssl/sha.h>

///////////////////////////////////////////////////////////
//
//  Hashes a file in chunks of size 32768 bytes into a
//  256 bit hash.
//
//  char* filename:  filename of file to hash
//  char* digest:    function allocated pointer to 2d array
//  int* length:     number of chucks the file splits into
//                    
int computeSha256FileChunks(const char* const path, char*** const digest, int* const length);


///////////////////////////////////////////////////////////
//
//  Hashes a file into  a 256 bit hash.
//
//  char* filename:  filename of file to hash
//  char digest[65]: function allocated pointer to 2d array
//  
int computeSha256File(const char * const path, char digest[65]);