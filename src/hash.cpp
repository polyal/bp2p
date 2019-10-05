#include <iostream>
#include <fstream>
#include "utils.h"
#include "hash.h"

#define CHUNK 32768
#define DEBUG 0


Hash::Hash()
{
    this->hash.reserve(SHA256_DIGEST_LENGTH);
    this->strHash = "";
}

vector<char> Hash::getBytes()
{
    return this->hash;
}

string Hash::toString()
{
    this->strHash = Utils::bytesToHex(this->hash.data(), SHA256_DIGEST_LENGTH);
    return this->strHash;
}

Sha256::Sha256() : Hash()
{

}

void Sha256::init()
{
    SHA256_Init(&this->ctx);
}

void Sha256::update(const vector<char>& buff, const int size)
{
    SHA256_Update(&this->ctx, buff.data(), size);
}

vector<char> Sha256::final()
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &this->ctx);
    vector<char> temp (hash, hash + SHA256_DIGEST_LENGTH);
    this->hash = temp;
    return this->hash;
}

vector<char> Sha256::computeHash(const vector<char>& buff, const int size)
{
    init();
    update(buff, size);
    this->hash = final();
    return this->hash;
}

Sha256FileHasher::Sha256FileHasher()
{
    this->filename = "";
}

Sha256FileHasher::Sha256FileHasher(const string& filename)
{
    this->filename = filename;
}

void Sha256FileHasher::open(const string& filename)
{
    this->filename = filename;
}

vector<Sha256> Sha256FileHasher::computeFileChunkHash()
{
    if (this->filename.empty())
        return this->chunkHashs;
    vector<Sha256> chunkHashs;                  // stores hashes of chunks
    vector<char> fileChunk (chunkSize, 0);      // stores a chunk of the file for hashing
    ifstream file {this->filename, ifstream::binary};
    while(!file.eof())
    {
        file.read(fileChunk.data(), fileChunk.size());
        streamsize size = file.gcount();
        Sha256 chunkHash;
        chunkHash.computeHash(fileChunk, size);
        chunkHashs.push_back(chunkHash);
    }
    this->chunkHashs = chunkHashs;
    return this->chunkHashs;
}

Sha256 Sha256FileHasher::computeFileHash()
{
    if (this->filename.empty())
        return this->fileHash;
    Sha256 fileHash;
    fileHash.init();
    vector<char> fileChunk (chunkSize, 0);
    ifstream file {this->filename, ifstream::binary};
    while(file.read(fileChunk.data(), fileChunk.size()))
    {
        streamsize size = file.gcount();
        fileHash.update(fileChunk, size);
    }
    fileHash.final();
    this->fileHash = fileHash;
    return this->fileHash;
}

vector<string> Sha256FileHasher::chunkHashsToString()
{
    vector<string> hashs;
    for (auto& chunk : chunkHashs)
    {
        string hash = chunk.toString();
        hashs.push_back(hash);
    }
    return hashs;
}

string Sha256FileHasher::fileHashToString()
{
    return fileHash.toString();
}



#if DEBUG == 1
int main (int argc, char **argv)
{
    if (argc < 2)
        return 0;

    argv++;
    char* filename = *argv;
    printf("%s \n", filename);

    char** digest = NULL;
    unsigned int length = 0;
    computeSha256FileChunks(filename, &digest, &length);

    unsigned int i = 0;
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