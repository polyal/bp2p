#include <vector>
#include <fstream>
#include "compress.h"


/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret){
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

const string Ezlib::ext = ".ez";

Ezlib::Ezlib()
{
    this->source = "";
    this->dest = "";
    this->level = 9;
}

Ezlib::Ezlib(const string& source, const int level)
{
    this->source = source;
    this->dest = source + this->ext;
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
    this->dest = source + this->ext;
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

    ret = initCompress();
    if (ret != Z_OK)
        return ret;

    ifstream fSource {this->source, ifstream::binary};
    ofstream fDest {this->dest, ifstream::binary};
    if (!fSource.is_open() || !fDest.is_open()) return Z_ERRNO;

    do{
        fSource.read(&in[0], in.size());
        this->strm.avail_in = fSource.gcount();
        if (fSource.fail() && !fSource.eof()){
            (void)deflateEnd(&this->strm);
            return Z_ERRNO;
        }
        flush = fSource.eof() ?  Z_FINISH : Z_NO_FLUSH;
        this->strm.next_in = reinterpret_cast<unsigned char*>(&in[0]);

        do{
            this->strm.avail_out = this->chunkSize;
            this->strm.next_out = reinterpret_cast<unsigned char*>(&out[0]);
            ret = deflate(&this->strm, flush); // no bad return value
            //assert(ret != Z_STREAM_ERROR);     // state not clobbered

            have = this->chunkSize - this->strm.avail_out;
            unsigned long before = fDest.tellp();
            fDest.write(&out[0], have);
            if (fDest.fail() || (unsigned long)fDest.tellp() - before != have){
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        }while (strm.avail_out == 0);
        //assert(strm.avail_in == 0);     /* all input will be used */
    }while(flush != Z_FINISH);

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

int Ezlib::decompress()
{
    int ret;
    unsigned int have;
    vector<char> in(this->chunkSize);
    vector<char> out(this->chunkSize);

    ret = initDecompress();
    if (ret != Z_OK)
        return ret;

    ifstream fSource {this->source, ifstream::binary};
    ofstream fDest {this->dest, ifstream::binary};
    if (!fSource.is_open() || !fDest.is_open())
        return Z_ERRNO;

    // decompress until deflate stream ends or end of file
    do{
        fSource.read(&in[0], in.size());
        this->strm.avail_in = fSource.gcount();
        if (fSource.fail() && !fSource.eof()){
            (void)inflateEnd(&this->strm);
            return Z_ERRNO;
        }
        else if (this->strm.avail_in == 0)
            break;

        this->strm.next_in = reinterpret_cast<unsigned char*>(&in[0]);

        // run inflate() on input until output buffer not full
        do{
            this->strm.avail_out = this->chunkSize;
            this->strm.next_out = reinterpret_cast<unsigned char*>(&out[0]);

            ret = inflate(&this->strm, Z_NO_FLUSH);
            //assert(ret != Z_STREAM_ERROR);  // state not clobbered

            if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR){
                if (ret == Z_NEED_DICT)
                    ret = Z_DATA_ERROR; 
                (void)inflateEnd(&this->strm);
                return ret;
            }
            
            have = this->chunkSize - this->strm.avail_out;
            unsigned long before = fDest.tellp();
            fDest.write(&out[0], have);
            if (fDest.fail() || (unsigned long)fDest.tellp() - before != have){
                (void)inflateEnd(&this->strm);
                return Z_ERRNO;
            }
        }while (this->strm.avail_out == 0);

    // done when inflate() says it's done
    }while (ret != Z_STREAM_END);

    // clean up and return
    (void)inflateEnd(&this->strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int Ezlib::initCompress()
{
    initCompressStreamState();
    return deflateInit(&this->strm, this->level);
}

void Ezlib::initCompressStreamState()
{
    this->strm.zalloc = Z_NULL;
    this->strm.zfree = Z_NULL;
    this->strm.opaque = Z_NULL;
}

int Ezlib::initDecompress()
{
    initDecompressStreamState();
    return inflateInit(&this->strm);
}

void Ezlib::initDecompressStreamState()
{
    this->strm.zalloc = Z_NULL;
    this->strm.zfree = Z_NULL;
    this->strm.opaque = Z_NULL;
    this->strm.avail_in = 0;
    this->strm.next_in = Z_NULL;
}
