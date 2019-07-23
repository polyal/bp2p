#include "torrent.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

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

const string Torrent::torrentDir = "torrents/";

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


Torrent::Torrent(const string& torrentName, const vector<string>& files){
	numPieces = 0;
	filename = "";
	torrentLocation = "";
	serializedObj = "";

	if (torrentName.empty() || files.empty()){
		cout << "Error: Torrent Name or Files array empty" << endl;
		return;
	}

	createTorrent (torrentName, files);
}

Torrent::Torrent(const string& torrentName){
	readTorrentFromFile(torrentName);
	deserialize(serializedObj);
	depackage(filename.c_str());
}

Torrent::Torrent(){
	numPieces = 0;
	filename[0] = '\0';
	torrentLocation[0] = '\0';
}

int Torrent::createTorrent (const string& torrentName, const vector<string>& files){
	if (torrentName.empty() || files.empty()){
		cout << "Create Torrent Error: invalid input" << endl;
		return -1;
	}

	createPackage(torrentName, files);
	generateChunks();
	serialize();
	dumpToTorrentFile();

	return 0;
}

int Torrent::createPackage(const string& torrentName, const vector<string>& files){
	int ret = 0;
	const char* cstrTorrent = torrentName.c_str();
	vector<const char*> cstrFiles;
    cstrFiles.reserve(files.size());
	
	for(auto& file: files)
    	cstrFiles.push_back(file.c_str());
	cstrFiles.push_back(NULL); // last entry needs to be NULL

	ret = package(cstrTorrent, cstrFiles.data());

	if (ret != 0){
		cout << "Packaging Error" << endl;
		return ret;
	}

	// set filename instance
	filename = torrentName;
	
	return ret;
}


int Torrent::generateChunks(){
	int ret = 0;
	char** digest = NULL;
	int length = 0;

	// need to call createPackage() first
	if (filename.empty())
		return -1;

	// generate chunk hashes
	ret = computeSha256FileChunks(filename.c_str(), &digest, &length);

	//convert hash chunks to hex strings
	int i = 0;
	for (i = 0; i < length; i++){
		string strdigest = bytesToHex(digest[0], 32);
		cout << strdigest << endl;

		auto keyValPair = make_tuple(strdigest, true);

		chunks.push_back(keyValPair);
	}

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
    	cout << "Chunking Error" << endl;
		return ret;
    }

    // object initializations
    numPieces = length;

    return 0;
}

void Torrent::serialize(){
	if (filename.empty() || numPieces == 0 || chunks.empty()){
		cout << "Serialize Input Error: invalid input" << endl;
		return;
	}

	jobj["filename"] = filename;
	jobj["numPieces"] = numPieces;

	std::vector<tuple<string, bool>>::size_type i = 0;
	for(i = 0; i != chunks.size(); i++) {
    	jobj[to_string(i)] = { get<0>(chunks[i]), true };
	}

	serializedObj = jobj.dump();
	cout << "json ::\n" << serializedObj << endl;
}

void Torrent::deserialize(string& serializedObj){
	this->serializedObj = serializedObj;
	jobj = nlohmann::json::parse(serializedObj);

	filename = jobj["filename"].get<std::string>();
	numPieces = jobj["numPieces"];

	cout << "filename " << filename << " " << numPieces << endl;

	int i = 0;
	for (i = 0; i < numPieces; i++) {
		auto hashpair = jobj[to_string(i)];
  		std::cout << hashpair << endl;

  		make_tuple(hashpair[0], hashpair[1]);
  		auto keyValPair = make_tuple(hashpair[0], hashpair[1]);
		chunks.push_back(keyValPair);
	}

	cout << "\n ventor \n";

	for(auto const& value: chunks) {
		cout << get<0>(value) << " " << get<1>(value) << endl;
	}
}

void Torrent::dumpToTorrentFile (){
	string fullpath;

	if (torrentDir.empty() || filename.empty() || serializedObj.empty()){
		cout << "Dump Error: input error" << endl;
		return;
	}

	fullpath = torrentDir + filename;

	cout << "Write Torrent: " << fullpath << endl;

	ofstream fTorrent {fullpath};

	if (fTorrent.is_open()){
		fTorrent << serializedObj;
 		fTorrent.close();
	}
	else{
		cout << "Dump Error: error opening output torrent file" << endl;
	}
}

void Torrent::readTorrentFromFile(const string& torrentName){
	string fullpath;

	if (torrentDir.empty() || torrentName.empty()){
		cout << "Read Torrent File: input error" << endl;
		return;
	}
	fullpath = torrentDir + torrentName;

	cout << "Read torrent: " << fullpath << endl;

	string data;
 	ifstream fTorrent{fullpath};

 	if (fTorrent.is_open()){
 		while(getline(fTorrent, data))
	        cout << data;
		fTorrent.close();
 	}
 	else
 		cout << "Read Torrent Error: cant open file" << endl;
    
	serializedObj = data;
}



int main(int argc, char *argv[]){
	string archive { argv[1] };

	cout << argc << archive << endl;

	if (argc == 2){
		Torrent newTorrent(archive);
	}
	else{
		vector<string> files {argv+2, argv+argc};

		for (auto& file : files)
			cout << file << endl;
		cout << endl;

		Torrent newTorrent(archive, files);
	}

    cout << "helloworld\n";

    return 0;

}