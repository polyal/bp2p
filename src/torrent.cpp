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

#include "utils.h"

// include c libs
extern "C" {
#include "package.h"
#include "hash.h"
}

#define DEBUG 0

using namespace std;

const string Torrent::torrentDir = "torrents/";
const string Torrent::torrentFileDir = "torrentData/";

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
	string torrentsPath, fullpath;

	if (torrentName.empty()){
		cout << "Error: Torrent Name Invalid" << endl;
		return;
	}

	torrentsPath = getTorrentsPath();
	fullpath =  torrentsPath + torrentName;
	cout << torrentName << " " << fullpath << endl;

	if (Utils::doesFileExist(fullpath)){
		cout << "ecists" << endl;
		readTorrentFromFile(fullpath);
		deserialize(serializedObj, false);
		//unpackage (fullpath);
	}
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
	generateFileHash();
	serialize(true);
	dumpToTorrentFile();

	return 0;
}

int Torrent::createPackage(const string& torrentName, const vector<string>& files){
	int ret = 0;
	string torrentPath;
	const char* cstrTorrentData;
	vector<const char*> cstrFiles;
    cstrFiles.reserve(files.size());

    torrentPath = torrentFileDir + torrentName;
    cstrTorrentData = torrentPath.c_str();

    cout << cstrTorrentData << endl;
	
	for(auto& file: files)
    	cstrFiles.push_back(file.c_str());
	cstrFiles.push_back(NULL); // last entry needs to be NULL

	ret = package(cstrTorrentData, cstrFiles.data());

	if (ret != 0){
		cout << "Packaging Error" << endl;
		return ret;
	}

	// set filename instance
	filename = torrentName;
	fullpath = torrentPath;
	
	return ret;
}

int Torrent::unpackage (const string& packageName){
	if (packageName.empty()){
		cout << "Depackage Error: invalid input" << endl;
	}

	cout << packageName << endl;

	return depackage(packageName.c_str());
}


int Torrent::generateChunks(){
	int ret = 0;
	char** digest = NULL;
	int length = 0;

	// need to call createPackage() first
	if (fullpath.empty())
		return -1;

	// generate chunk hashes
	ret = computeSha256FileChunks(fullpath.c_str(), &digest, &length);

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

int Torrent::generateFileHash(){
	int err = 0;
	const char* cFilename;
	char digest[65];

	if (fullpath.empty()){
		cout << "File Hash Input Error: invalid input" << endl;
		return -1;
	}

	cFilename = this->fullpath.c_str();
	err = computeSha256File(cFilename, digest);

	if (err > 0){
		cout << "File Hash Error: couldn't hash file." << endl;
		return -1;
	}

	//convert hash to hex strings
	uid = bytesToHex(digest, 32);
	cout << uid << endl;

	return 0;
}

void Torrent::serialize(bool create){
	if (filename.empty() || numPieces == 0 || chunks.empty()){
		cout << "Serialize Input Error: invalid input" << endl;
		return;
	}

	jobj["filename"] = filename;
	jobj["uid"] = uid;
	jobj["numPieces"] = numPieces;

	std::vector<tuple<string, bool>>::size_type i = 0;
	for(i = 0; i != chunks.size(); i++) {
		if (create)
    		jobj[to_string(i)] = {get<0>(chunks[i]), true};
    	else
    		jobj[to_string(i)] = {get<0>(chunks[i]), get<1>(chunks[i])};
	}

	serializedObj = jobj.dump();
	cout << "json ::\n" << serializedObj << endl;
}

void Torrent::deserialize(string& serializedObj, bool create){
	this->serializedObj = serializedObj;
	jobj = nlohmann::json::parse(serializedObj);

	filename = jobj["filename"].get<std::string>();
	numPieces = jobj["numPieces"];
	uid = jobj["uid"];
	fullpath = torrentFileDir + filename;

	cout << "filename " << filename << ", num Pieces " << numPieces << endl;

	int i = 0;
	for (i = 0; i < numPieces; i++) {
		auto hashpair = jobj[to_string(i)];
  		std::cout << hashpair << endl;

  		tuple<string, bool> keyValPair;
  		if (create)
  			keyValPair = make_tuple(hashpair[0], false);
		else
			keyValPair = make_tuple(hashpair[0], hashpair[1]);
		chunks.push_back(keyValPair);
	}

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
	if (torrentDir.empty() || torrentName.empty()){
		cout << "Read Torrent File: input error" << endl;
		return;
	}

	cout << "Read torrent: " << torrentName << endl;

	string data;
 	ifstream fTorrent{torrentName};

 	if (fTorrent.is_open()){
 		// this should always only be one line
 		while(getline(fTorrent, data))
	        cout << data << endl;
		fTorrent.close();
 	}
 	else
 		cout << "Read Torrent Error: cant open file" << endl;
    
	serializedObj = data;
}

bool Torrent::isTorrentComplete(){
	bool complete = true;
	if (chunks.empty())
		return false;

	for(auto it = chunks.begin(); it != chunks.end(); it++) {
    	if (get<1>(*it) == false){
    		complete = false;
    		break;
    	}
	}

	return complete;
}

string Torrent::getFilename(){
	return this->filename;
}

string Torrent::getSerializedTorrent(){
	return this->serializedObj;
}

string Torrent::getTorrentsPath(){
	string appPath = Utils::getApplicationPath();
	return appPath + torrentDir;
}

vector<string> Torrent::getTorrentNames(){
	int ret;
	string torrentsPath = getTorrentsPath();
	vector<string> torrentNames;

	ret = Utils::listFileInDir(torrentsPath, torrentNames);
	if (ret > 0){
		cout << "getTorrentNames error: " << ret << endl;
	}

	/*for(auto const& value: torrentNames) {
		cout << value << endl;
	}
	cout << "done" << endl;*/

    return torrentNames;
}

#if DEBUG == 1
int main(int argc, char *argv[]){
	if (argc < 2){
		cout << "Usage: IMPORT: ./a.out [torrent]\n       EXPORT: ./a.out [torrent] [file1 | file2 | ...]" << endl;
		return 0;
	}

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

    return 0;
}
#endif