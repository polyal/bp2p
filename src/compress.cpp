/**********************************************************
**
**  Author: Lorant Polya
**
**  This file exposes zlib inflate and deflate api
**  used for compressiona de compression
**
**  https://zlib.net/zlib_how.html
**
**********************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "compress.h"

#include <vector>
#include <fstream>


int compressFile(FILE* const source, FILE* const dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate deflate state 
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    // compress until end of file
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);

        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }

        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        // run deflate() on input until output buffer not full, finish
        // compression if all of source has been read in
        do {

            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = deflate(&strm, flush);    // no bad return value
            assert(ret != Z_STREAM_ERROR);  // state not clobbered

            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }

        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

    /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}


int decompressFile(FILE* const source, FILE* const dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    // decompress until deflate stream ends or end of file
    do {

        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        // run inflate() on input until output buffer not full
        do {

            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  // state not clobbered
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     // and fall through
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }

            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }

        } while (strm.avail_out == 0);

    // done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // clean up and return
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}


/**********************************************************
**********************************************************/

Ezlib::Ezlib()
{
    this->source = "";
    this->dest = "";
}

Ezlib::Ezlib(const string& source, const int level)
{
    this->source = source;
    this->dest = source + postFix;
    this->level = level;
}

Ezlib::Ezlib(const string& source, const string& dest, const int level)
{
    this->source = source;
    this->dest = dest;
    this->level = level;
}

void Ezlib::setup(const string& source, const int level)
{
    this->source = source;
    this->dest = source + postFix;
    this->level = level;
}

void Ezlib::setup(const string& source, const string& dest, const int level)
{
    this->source = source;
    this->dest = dest;
    this->level = level;
}

int Ezlib::compress()
{
    int ret, flush;
    unsigned int have;
    vector<char> in(this->chunkSize);
    vector<char> out(this->chunkSize);

    init();
    ifstream fSource {this->source, ifstream::binary};
    ofstream fDest {this->dest, ifstream::binary};
    if (!fSource.is_open() || !fDest.is_open()) return Z_ERRNO;

    do
    {
        !fSource.read(&in[0], in.size());
        this->strm.avail_in = fSource.gcount();
        if (fSource.fail() && !fSource.eof())
        {
            (void)deflateEnd(&this->strm);
            return Z_ERRNO;
        }
        flush = fSource.eof() ?  Z_FINISH : Z_NO_FLUSH;
        this->strm.next_in = reinterpret_cast<unsigned char*>(&in[0]);

        do
        {
            this->strm.avail_out = this->chunkSize;
            this->strm.next_out = reinterpret_cast<unsigned char*>(&out[0]);
            ret = deflate(&this->strm, flush); // no bad return value
            assert(ret != Z_STREAM_ERROR);     // state not clobbered

            have = this->chunkSize - this->strm.avail_out;
            unsigned long before = fDest.tellp();
            fDest.write(&out[0], have);
            if (fDest.fail() || (unsigned long)fDest.tellp() - before != have)
            {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */
    } while(flush != Z_FINISH);

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

const string Ezlib::postFix = ".ez"; 

int Ezlib::init()
{
    initStreamState();
    return deflateInit(&this->strm, this->level);
}

void Ezlib::initStreamState()
{
    // allocate deflate state 
    this->strm.zalloc = Z_NULL;
    this->strm.zfree = Z_NULL;
    this->strm.opaque = Z_NULL;
}
