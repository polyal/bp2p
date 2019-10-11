/**********************************************************
**
**   Author: Lorant Polya
**
**   This file archives a folder and
**   compresses it using compress.h lib.
**   After the file is compressed it is 
**   ready to be transported.
**   This file is also resposible for
**   decompressing the transported file
**   and unarchiving it.
**
**   https://github.com/libarchive/libarchive/wiki/Examples
**
**********************************************************/

#include <archive.h>
#include <archive_entry.h>
#include <string.h>
#include <iostream>
#include <string>
#include "archiver.h"
#include "compress.h"

#define DEBUG 1

int package(const string& package, const vector<string> filenames){
    int ret;
    string src{"temp"};
    
    Archiver archiver{src, filenames};
    archiver.archive();
    
    Ezlib compressor{src, package};
    ret = compressor.compress();
    if (ret != Z_OK)
        zerr(ret);

    remove("temp");
    return ret;
}

int unpackage(const string& packageName){
    int ret;
    
    string dest{"temp"};
    Ezlib decompressor{packageName, dest};
    ret = decompressor.decompress();
    if (ret != Z_OK)
        zerr(ret);

    Archiver archiver{dest};
    archiver.extract();

    remove("temp");
    return ret;
}


/****************************************
*
*  Usage: -c compression
*         -e extraction
*
*  for comepression first argument after
*  flag is out file, next arguments are
*  files to compress
*
*  extraction, only takes one argument
*  after flag and thats compressed file
*
****************************************/
#if DEBUG == 1
int main(int argc, const char **argv)
{
    const char *outname;
    const char *flag;
    const char** files;

    if (argc < 3){
        cout << "Usage: ./a.out [-c|-e] [package_name] [files_to_package] ..." << endl;
        return 0;
    }

    argv++;
    flag = *argv++;
    outname = *argv++;
    files = argv;

    string packageName{outname};
    vector<string> filenames;
    while(*files)
    {
        string file{*files};
        filenames.push_back(file);
        files++;
    }

    if (strcmp ("-c", flag) == 0){
        package(packageName, filenames);
    }
    else if (strcmp ("-e", flag) == 0){
        unpackage(packageName);
    }
    else{
        printf("Usage: ./a.out [-c|-e] [package_name] [files_to_package] ...\n");
    }

    return 0;
}
#endif