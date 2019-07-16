#include "torrent.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

// determine file/dir
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nlohmann/json.hpp"

// include c libs
extern "C" {
#include "../src/package.h"
#include "../src/hash.h"
}

using namespace std;

string bytesToHex(char* bytes, int len){
	std::stringstream digest;

	digest.setf(std::ios_base::hex, std::ios::basefield);
	digest.fill('0');
	for (int i = 0; i<len; i++)
	{
		digest << std::setw(2) << (unsigned int)(unsigned char)(bytes[i]);
	}

	const std::string ret = digest.str();

	return ret;
}

unsigned int value(char c)
{
	if (c >= '0' && c <= '9') { return c - '0'; }
	if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
	if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }

	return -1; // Error!
}

//we suppose the size is alwazs going to be even as it is supposed to be an encrypted value
//converts cstring to
char* hexToBytes(const string& strhex, int* size){
	const char* str = strhex.c_str();
	int bufSize = strhex.length() / 2;

	char* buf = new char[bufSize];



	for (int i = 0; i < bufSize; i++){
		buf[i] = value(str[2 * i]) * 16 + value(str[2 * i + 1]);
	}

	*size = bufSize;

	return  buf;
}



int Torrent::createTorrent (char* filename){
	cout << "createTorrent\n";

	return 0;
}

Torrent::Torrent(const char* archive, const char** files){
	int ret = 0;

	ret = package(archive, files);

	if (ret != 0){
		cout << "packaging error" << endl;
		return;
	}

	char** digest = NULL;
	int length = 0;
	ret = computeSha256FileChunks(archive, &digest, &length);

	string strdigest = bytesToHex(digest[0], 32);

	cout << strdigest << endl;

	// free up digest mem
	int i = 0;
    if (digest){
        for (i = 0; i < length; i++){
            if (digest[i])
                free(digest[i]);
        }
        free(digest);
    }

    cout << "num chuncks " << length << endl;
    if (ret != 0){
    	cout << "hashing error" << endl;
		return;
    }
}

Torrent::Torrent(const char* filename){
	depackage(filename);
}

Torrent::Torrent(){
	numPieces = 0;
	filename[0] = '\0';
	torrentLocation[0] = '\0';
}



int main(int argc, char *argv[]){
	const char* archive = argv[1];

	cout << argc << endl;

	if (argc == 2){
		Torrent newTorrent(archive);
	}
	else{
		const char** files = (const char**)&argv[2];
		Torrent newTorrent(archive, files);
	}

    cout << "helloworld\n";

    return 0;

}