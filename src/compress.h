#include <string>
#include <zlib.h>

#define CHUNK 32768 // 256K

using namespace std;

/* 
  Compress from file source to file dest until EOF on source.
  def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
  allocated for processing, Z_STREAM_ERROR if an invalid compression
  level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
  version of the library linked do not match, or Z_ERRNO if there is
  an error reading or writing the files.
*/
int compressFile(FILE* const source, FILE* const dest, int level);

/*
  Decompress from file source to file dest until stream ends or EOF.
  inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
  allocated for processing, Z_DATA_ERROR if the deflate data is
  invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
  the version of the library linked do not match, or Z_ERRNO if there
  is an error reading or writing the files. 
*/
int decompressFile(FILE* const source, FILE* const dest);

void zerr(int ret);


class Ezlib
{
private:
    static const string postFix; 
    static const int chunkSize = 32768;
    int level = 9;  // highest compression level
    z_stream strm;
    string source;
    string dest;

public:
    Ezlib();
    Ezlib(const string& source, const int level = 9);
    Ezlib(const string& source, const string& dest, const int level = 9);

    void setup(const string& filename, const int level = 9);
    void setup(const string& source, const string& dest, const int level = 9);
    int compress();
    int decompress();

private:
    int initCompress();
    void initCompressStreamState();
    int initDecompress();
    void initDecompressStreamState();
};