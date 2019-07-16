///////////////////////////////////////////////////////////
//
//  Hashes a file in chunks of size 32768 bytes into a
//  256 bit hash.
//
//  char* filename:  filename of file to hash
//  char* digest:    function allocated pointer to 2d array
//  int* length:     number of chucks the file splits into
//                    
int computeSha256FileChunks(const char* path, char*** digest, int* length);