/**********************************************************
**
**    compress.h
**    Lorant Polya
**    Oct 9, 2019
**
**    This file declares the Ezlib class.  This class is
**    a compression class that wraps the zlib
**    compression library.  It makes it easy to compress
**    and uncompress files while hiding all the details
**    from the user.
**
**    eg.
**    https://zlib.net/zlib_how.html
**
**********************************************************/

#include <string>
#include <zlib.h>

using namespace std;


void zerr(int ret);


class Ezlib
{
public:
    ///////////////////////////////////////////////////////////
    //  Initilizes source and destination filenames and
    //  compression level.  
    //
    //  source:  The filename of the file to be compressed
    //  des:     The out name of the comressed file.  If no 
    //           dest is provided, the ezlib postifx will be
    //           appended to the source filename
    //  level:   The level of compresseion.  By default, 
    //           compression level is set to 9, the highest 
    //           possible level
    Ezlib();
    Ezlib(const string& source, const int level = 9);
    Ezlib(const string& source, const string& dest, const int level = 9);

    ///////////////////////////////////////////////////////////
    //  Initilizes source and destination filenames and
    //  compression level.  Used in conjuntion with with the
    //  no argument constructor.
    //
    //  source:  The filename of the file to be compressed
    //  des:     The out name of the comressed file.  If no 
    //           dest is provided, the ezlib postifx will be
    //           appended to the source filename
    //  level:   The level of compresseion.  By default, 
    //           compression level is set to 9, the highest 
    //           possible level
    void setup(const string& filename, const int level = 9);
    void setup(const string& source, const string& dest, const int level = 9);

    ///////////////////////////////////////////////////////////
    //  Compresses a file
    int compress();

    ///////////////////////////////////////////////////////////
    //  Decompresses a file
    int decompress();

private:
    static const string ext; 
    static const int chunkSize = 32768;

    int level = 9;  // highest compression level
    z_stream strm;
    string source;
    string dest;

    ///////////////////////////////////////////////////////////
    //  Initializes the state of the z_stream and prepares for
    //  compression or decompression
    int initCompress();
    void initCompressStreamState();
    int initDecompress();
    void initDecompressStreamState();
};
