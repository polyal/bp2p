#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "hash.h"

#define CHUNK 32768
#define DEBUG 0


void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65])
{
    int i = 0;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    outputBuffer[64] = 0;
}

int computeSha256File(const char * const path, char digest[65])
{
    FILE *file = fopen(path, "rb");
    if(!file) return 1;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    unsigned char *buffer = malloc(CHUNK);
    if(!buffer) return 1;

    int bytesRead = 0;
    while((bytesRead = fread(buffer, 1, CHUNK, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final(hash, &sha256);

    sha256_hash_string(hash, digest);
    fclose(file);
    free(buffer);
    return 0;
}


void computeSha256(char *buffer, char* digest, int size)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buffer, size);
    SHA256_Final(hash, &sha256);

    memcpy(digest, hash, SHA256_DIGEST_LENGTH);
}


///////////////////////////////////////////////////////////
//
//  Hashes a file in chunks of size 32768 bytes into a
//  256 bit hash.
//
//  char* filename:  filename of file to hash
//  char* digest:    function allocated pointer to 2d array
//                    
int computeSha256FileChunks(const char* const path, char*** const digest, int* const length){
    FILE *fp = NULL;
    long fileSize = 0;
    *length = 0;

    fp = fopen(path, "rb");
    if(!fp) return -534;

    // get file size
    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp); 
    fseek(fp, 0L, SEEK_SET);

    if (fileSize == 0) return ENOENT;
    
    int i = 0;
    long numChunks = (fileSize % CHUNK) ? (fileSize / CHUNK) + 1 : fileSize / CHUNK;
    *length = numChunks;

    *digest = malloc(sizeof(char*) * numChunks);
    if (*digest == NULL) return ENOMEM;
    for (i = 0; i < numChunks; i++){
        (*digest)[i] = malloc(sizeof(char) * SHA256_DIGEST_LENGTH);
        if ((*digest)[i] == NULL) return ENOMEM;
    }

    i = 0;
    int bytesRead = 0;
    char *buffer = malloc(CHUNK);
    while((bytesRead = fread(buffer, 1, CHUNK, fp)))
    {
        computeSha256(buffer, (*digest)[i], bytesRead);
        memset(buffer, 0, CHUNK);
        //printf("%d %lx \n", i, (*digest)[i]);
        i++;
    }
    free(buffer);

    return 0;
}

#if DEBUG == 1
int main (int argc, char **argv)
{
    argv++;
    char* filename = *argv;
    printf("%s \n", filename);

    char** digest = NULL;
    int length = 0;
    computeSha256FileChunks(filename, &digest, &length);

    int i = 0;
    if (digest){
        for (i = 0; i < length; i++){
            if (digest[i])
                free(digest[i]);
        }
        free(digest);
    }
    
    return 0;
}
#endif