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
#include "package.h"

#define DEBUG 0

const string Package::tempName = "temp";

Package::Package()
{
    this->name = "";
}

Package::Package(const string& name)
{
    this->name = name;
}

Package::Package(const string& name, const vector<string> filenames)
{
    this->name = name;
    this->files = filenames;
}

void Package::setup(const string& name)
{
    this->name = name;
}

void Package::setup(const string& name, const vector<string> filenames)
{
    this->name = name;
    this->files = filenames;
}


int Package::package()
{
    archive();
    int ret = compress();
    remove("temp");
    return ret;
}

int Package::unpackage()
{
    int ret = decompress();
    extract();
    remove("temp");
    return ret;
}

int Package::archive()
{
    Archiver archiver{this->tempName, this->files};
    archiver.archive();
    return 0;
}

int Package::extract()
{
    Archiver archiver{this->tempName};
    archiver.extract();
    return 0;
}


int Package::compress()
{
    Ezlib compressor{this->tempName, this->name};
    int ret = compressor.compress();
    if (ret != Z_OK)
        zerr(ret);
    return ret;
}

int Package::decompress()
{
    Ezlib decompressor{this->name, this->tempName};
    int ret = decompressor.decompress();
    if (ret != Z_OK)
        zerr(ret);
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