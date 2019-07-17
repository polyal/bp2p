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


// include c libs
extern "C" {
#include "package.h"
#include "hash.h"
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


Torrent::Torrent(const char* archive, const char** files){
	createTorrent (archive, files);
}

Torrent::Torrent(const char* filename){
	depackage(filename);
}

Torrent::Torrent(){
	numPieces = 0;
	filename[0] = '\0';
	torrentLocation[0] = '\0';
}

int Torrent::createTorrent (const char* archive, const char** files){
	cout << "createTorrent\n";

	createPackage(archive, files);
	generateChunks();
	createJson();

	return 0;
}

int Torrent::createPackage(const char* archive, const char** files){
	int ret = 0;

	ret = package(archive, files);
	generateChunks();

	if (ret != 0){
		cout << "packaging error" << endl;
		return ret;
	}

	// set filename instance
	strcpy(filename, archive);
	
	return 0;
}


int Torrent::generateChunks(){
	int ret = 0;
	char** digest = NULL;
	int length = 0;

	// need to call cfreate package first
	if (filename[0] == '\0')
		return -1;

	// generate chunk hashes
	ret = computeSha256FileChunks(filename, &digest, &length);

	//convert hash chunks to hex strings
	int i = 0;
	for (i = 0; i < length; i++){
		string strdigest = bytesToHex(digest[0], 32);
		cout << strdigest << endl;

		auto keyValPair = make_tuple(strdigest, true);

		chunks.push_back(keyValPair);
	}

	string strdigest = bytesToHex(digest[0], 32);

	// free up digest mem
	i = 0;
    if (digest){
        for (i = 0; i < length; i++){
            if (digest[i])
                free(digest[i]);
            else
            	break; // if NULL, malloc failed to allocate memory
        }
        free(digest);
    }

    cout << "num chunks " << length << endl;
    if (ret != 0){
    	cout << "chunking error" << endl;
		return ret;
    }

    // object initializations
    numPieces = length;

    return 0;
}

void Torrent::createJson(){

	std::vector<tuple<string, bool>>::size_type i = 0;
	for(i = 0; i != chunks.size(); i++) {
    	jobj[to_string(i)] = { get<0>(chunks[i]), true };
	}

	serializedObj = jobj.dump();
	cout << "json\n\n" << serializedObj << endl;
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