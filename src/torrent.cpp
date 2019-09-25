#include "torrent.h"
#include <stdio.h>
#include <iostream>
#include <string>
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

const string Torrent::torrentFileDir = "torrents/";
const string Torrent::torrentDataDir = "torrentData/";


Torrent::Torrent(const string& torrentName, const vector<string>& files){
	this->numPieces = 0;
	this->name = "";
	this->serializedObj = "";

	if (torrentName.empty() || files.empty()){
		cout << "Error: Torrent Name or Files array empty" << endl;
		return;
	}

	createTorrent (torrentName, files);
}

Torrent::Torrent(const string& torrentName){
	string fullpath;

	if (torrentName.empty()){
		cout << "Error: Torrent Name Invalid" << endl;
		return;
	}

	fullpath = getTorrentsPath() + torrentName;
	cout << torrentName << " " << fullpath << endl;

	if (Utils::doesFileExist(fullpath)){
		cout << "ecists" << endl;
		readTorrentFromFile(fullpath);
		deserialize(serializedObj, false);
		//unpackage (fullpath);
	}
}

Torrent::Torrent(){
	this->numPieces = 0;
	this->name = "";
	this->fullpath = "";
	this->uid = "";
	this->serializedObj = "";
}

Torrent::Torrent(const Torrent& torrent){
	this->numPieces = torrent.numPieces;
	this->name = torrent.name;
	this->fullpath = torrent.fullpath;
	this->uid = torrent.uid;
	copy(torrent.chunks.begin(), torrent.chunks.end(), back_inserter(this->chunks));
	this->jobj = torrent.jobj;
	this->serializedObj = torrent.serializedObj;
}

void Torrent::createTorrentFromSerializedObj(const string& serializedObj){
	if (serializedObj.empty()){
		cout << "Create Torrent From Serialized Obj Error: invalid input" << endl;
		return;
	}

	deserialize(serializedObj, false);
	dumpToTorrentFile();
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
	string torrentDataFullPath;
	const char* cstrTorrentDataFullPath;
	vector<const char*> cstrFiles;
    cstrFiles.reserve(files.size());

    torrentDataFullPath = getTorrentDataPath() + torrentName;
    cstrTorrentDataFullPath = torrentDataFullPath.c_str();

    cout << cstrTorrentDataFullPath << endl;
	
	for(auto& file: files)
    	cstrFiles.push_back(file.c_str());
	cstrFiles.push_back(NULL); // last entry needs to be NULL

	ret = package(cstrTorrentDataFullPath, cstrFiles.data());

	if (ret != 0){
		cout << "Packaging Error" << endl;
		return ret;
	}

	// set filename instance
	this->name = torrentName;
	this->fullpath = torrentDataFullPath;
	
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
	if (this->fullpath.empty())
		return -1;

	// generate chunk hashes
	ret = computeSha256FileChunks(fullpath.c_str(), &digest, &length);

	//convert hash chunks to hex strings
	int i = 0;
	for (i = 0; i < length; i++){
		string strdigest = Utils::bytesToHex(digest[0], 32);
		cout << strdigest << endl;

		auto keyValPair = make_tuple(strdigest, true);

		this->chunks.push_back(keyValPair);
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
    this->numPieces = length;

    return 0;
}

int Torrent::generateFileHash(){
	int err = 0;
	const char* cFilename;
	char digest[65];

	if (this->fullpath.empty()){
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
	this->uid = Utils::bytesToHex(digest, 32);
	cout << uid << endl;

	return 0;
}

void Torrent::serialize(bool create){
	if (this->name.empty() || this->numPieces == 0 || this->chunks.empty()){
		cout << "Serialize Input Error: invalid input" << endl;
		return;
	}

	jobj["name"] = this->name;
	jobj["uid"] = this->uid;
	jobj["numPieces"] = this->numPieces;

	std::vector<tuple<string, bool>>::size_type i = 0;
	for(i = 0; i != this->chunks.size(); i++) {
		if (create)
    		jobj[to_string(i)] = {get<0>(chunks[i]), true};
    	else
    		jobj[to_string(i)] = {get<0>(chunks[i]), get<1>(chunks[i])};
	}

	this->serializedObj = jobj.dump();
	cout << "json ::\n" << serializedObj << endl;
}

void Torrent::deserialize(const string& serializedObj, const bool create){
	this->serializedObj = serializedObj;
	this->jobj = nlohmann::json::parse(serializedObj);

	this->name = this->jobj["name"].get<std::string>();
	this->numPieces = this->jobj["numPieces"];
	this->uid = this->jobj["uid"];
	this->fullpath = Torrent::getTorrentDataPath() + this->name;

	cout << "name " << this->name << ", num Pieces " << numPieces << endl;

	int i = 0;
	for (i = 0; i < this->numPieces; i++) {
		auto hashpair = this->jobj[to_string(i)];
  		std::cout << hashpair << endl;

  		tuple<string, bool> keyValPair;
  		if (create)
  			keyValPair = make_tuple(hashpair[0], false);
		else
			keyValPair = make_tuple(hashpair[0], hashpair[1]);
		this->chunks.push_back(keyValPair);
	}

	for(auto const& value: this->chunks) {
		cout << get<0>(value) << " " << get<1>(value) << endl;
	}
}

void Torrent::dumpToTorrentFile (){
	string fullpath;

	if (this->name.empty() || this->serializedObj.empty()){
		cout << "Dump Error: input error" << endl;
		return;
	}

	fullpath = Torrent::getTorrentsPath() + this->name;

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
	if (torrentName.empty()){
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
    
	this->serializedObj = data;
}

vector<char> Torrent::RetrieveChunk(const int& chunkNum, int& size){
	vector<char> chunk(chunkSize);

	ifstream fTorrent {this->fullpath, ifstream::binary};
	if (fTorrent.is_open()){
		fTorrent.seekg (chunkNum * this->chunkSize);
		fTorrent.read (&chunk[0], this->chunkSize);
		
	}

	if (fTorrent)
      std::cout << "all characters read successfully.";
    else{
      std::cout << "error: only " << fTorrent.gcount() << " could be read" << strerror(errno) << endl;
      size = fTorrent.gcount();
    }
	fTorrent.close();

	return chunk;
}

bool Torrent::isComplete(){
	bool complete = true;
	if (chunks.empty())
		return false;

	for(auto it = this->chunks.begin(); it != this->chunks.end(); it++) {
    	if (get<1>(*it) == false){
    		complete = false;
    		break;
    	}
	}

	return complete;
}

bool Torrent::isValid(){
	return (numPieces > 0
			&& !name.empty()
			&& !fullpath.empty()
			&& !uid.empty()
			&& !chunks.empty()
			&& !serializedObj.empty());
}

Torrent& Torrent::operator=(const Torrent& torrent){
	this->numPieces = torrent.numPieces;
	this->name = torrent.name;
	this->fullpath = torrent.fullpath;
	this->uid = torrent.uid;
	copy(torrent.chunks.begin(), torrent.chunks.end(), back_inserter(this->chunks));
	this->jobj = torrent.jobj;
	this->serializedObj = torrent.serializedObj;

    return *this;  // Return a reference to myself.
}

string Torrent::getFilename(){
	return this->name;
}

string Torrent::getSerializedTorrent(){
	return this->serializedObj;
}

string Torrent::getTorrentsPath(){
	string appPath = Utils::getApplicationPath();
	return appPath + torrentFileDir;
}

string Torrent::getTorrentDataPath(){
	string appPath = Utils::getApplicationPath();
	return appPath + torrentDataDir;
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