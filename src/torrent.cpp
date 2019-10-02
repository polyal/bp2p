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

Torrent::Torrent()
{
	this->numPieces = 0;
	this->name = "";
	this->packagePath = "";
	this->torrentPath = "";
	this->uid = "";
	this->serializedObj = "";
	this->size = 0;
}

Torrent::Torrent(const string& name, const vector<string>& files)
{
	this->numPieces = 0;
	this->name = name;
	this->files = files;
	this->packagePath = "";
	this->torrentPath = "";
	this->uid = "";
	this->size = 0;
	this->serializedObj = "";
}

Torrent::Torrent(const string& name)
{
	this->numPieces = 0;
	this->name = name;
	this->packagePath = "";
	this->torrentPath = "";
	this->uid = "";
	this->size = 0;
	this->serializedObj = "";
}

Torrent::Torrent(const Torrent& torrent)
{
	this->numPieces = torrent.numPieces;
	this->name = torrent.name;
	this->files = torrent.files;
	this->packagePath = torrent.packagePath;
	this->torrentPath = torrent.torrentPath;
	this->uid = torrent.uid;
	copy(torrent.chunks.begin(), torrent.chunks.end(), back_inserter(this->chunks));
	this->jobj = torrent.jobj;
	this->serializedObj = torrent.serializedObj;
	this->size = torrent.size;
}

bool Torrent::create(const string& name, const vector<string>& files){
	this->name = name;
	this->files = files;
	return create();
}

bool Torrent::create()
{
	package();
	generateChunks();
	generateFileHash();
	serialize();
	dumpToTorrentFile();
	return isValid();
}

bool Torrent::open(const string& name){
	this->name = name;
	return open();
}

bool Torrent::open()
{
	if (this->name.empty())
	{
		cout << "Error: Torrent Name Invalid" << endl;
		return false;
	}

	string torrentPath = getTorrentsPath() + this->name;
	if (Utils::doesFileExist(torrentPath))
	{
		readTorrentFromFile(torrentPath);
		deserialize(false);
	}
	return isValid();
}

bool Torrent::createTorrentFromSerializedObj(const string& serializedObj)
{
	if (serializedObj.empty())
	{
		cout << "Create Torrent From Serialized Obj Error: invalid input" << endl;
		return false;
	}
	this->serializedObj = serializedObj;
	deserialize(true);
	dumpToTorrentFile();
	return isValid();
}

int Torrent::package()
{
	int ret = 0;
	string packagePath;
	const char* cstrPackagePath;
	vector<const char*> cstrFiles;

    if (this->name.empty() || this->files.empty())
    	return -1;

    // prepare package location
	packagePath = getTorrentDataPath() + this->name;
    cstrPackagePath = packagePath.c_str();

    // prepare list of files included in torrent
	cstrFiles.reserve(files.size());	
	for(auto& file: files)
    	cstrFiles.push_back(file.c_str());
	cstrFiles.push_back(NULL); // last entry must to be NULL

	ret = ::package(cstrPackagePath, cstrFiles.data());
	if (ret != 0)
	{
		cout << "Create Package Error: " << ret << endl;
		return ret;
	}

	this->packagePath = packagePath;
	this->size = Utils::filesize(cstrPackagePath);
	return ret;
}

int Torrent::unpackage()
{
	string packagePath = getTorrentDataPath() + this->name;
	if (packagePath.empty())
	{
		cout << "Depackage Error: invalid input" << endl;
		return -1;
	}
	return ::unpackage(packagePath.c_str());
}


int Torrent::generateChunks()
{
	int ret = 0;
	char** digest = NULL;
	unsigned int length = 0;

	// need to call createPackage() first
	if (this->packagePath.empty())
		return -1;

	// generate chunk hashes and convert them to hex strings
	ret = computeSha256FileChunks(packagePath.c_str(), &digest, &length);
	for (unsigned int i = 0; i < length; i++)
	{
		string strdigest = Utils::bytesToHex(digest[i], SHA256_DIGEST_LENGTH);
		this->chunks.push_back(Chunk{i, strdigest, true});
	}

	// free up digest mem
    if (digest)
    {
        for (unsigned int i = 0; i < length; i++)
        {
            if (digest[i])
                free(digest[i]);
            else
            	break; // if NULL, malloc failed to allocate memory
        }
        free(digest);
    }

    if (ret != 0)
    {
    	cout << "Chunking Error" << endl;
		return ret;
    }

    // object initializations
    this->numPieces = length;
    return 0;
}

int Torrent::generateFileHash()
{
	int err = 0;
	const char* cFilename;
	char digest[65];

	if (this->packagePath.empty())
	{
		cout << "File Hash Input Error: invalid input" << endl;
		return -1;
	}

	cFilename = this->packagePath.c_str();
	err = computeSha256File(cFilename, digest);
	if (err > 0)
	{
		cout << "File Hash Error: couldn't hash file." << endl;
		return -1;
	}

	//convert hash to hex strings
	this->uid = Utils::bytesToHex(digest, 32);
	return 0;
}

void Torrent::serialize()
{
	if (this->name.empty() || this->numPieces == 0 || this->chunks.empty())
	{
		cout << "Serialize Error" << endl;
		return;
	}

	jobj["name"] = this->name;
	jobj["uid"] = this->uid;
	jobj["numPieces"] = this->numPieces;
	jobj["size"] = this->size;
	for(unsigned int i = 0; i != this->chunks.size(); i++)
    		jobj[to_string(i)] = {chunks[i].hash, chunks[i].exists};
	this->serializedObj = jobj.dump();
}

void Torrent::deserialize(const bool create)
{
	if (this->serializedObj.empty())
	{
		cout << "Deserialize error" << endl;
		return;
	}

	this->jobj = nlohmann::json::parse(this->serializedObj);
	this->name = this->jobj["name"].get<std::string>();
	this->numPieces = this->jobj["numPieces"];
	this->uid = this->jobj["uid"];
	this->size = this->jobj["size"];
	this->packagePath = Torrent::getTorrentDataPath() + this->name;
	for (unsigned int i = 0; i < this->numPieces; i++)
	{
		unsigned int index = i;
		auto hashpair = this->jobj[to_string(i)];
  		string hash = hashpair[0];
  		bool exists = false;
  		if (!create)
			exists = hashpair[1];
		this->chunks.push_back(Chunk{index, hash, exists});
	}
	sort(chunks.begin(), chunks.end(), Chunk::cmp);
}

void Torrent::dumpToTorrentFile()
{
	if (this->serializedObj.empty()){
		cout << "Dump Error: input error" << endl;
		return;
	}

	string packagePath = Torrent::getTorrentsPath() + this->name;
	ofstream fTorrent {packagePath};
	if (fTorrent.is_open())
		fTorrent << serializedObj;	
}

void Torrent::readTorrentFromFile(const string& torrentPath)
{
	if (torrentPath.empty())
	{
		cout << "Read Torrent File: input error" << endl;
		return;
	}

	string data = "";
 	ifstream fTorrent{torrentPath};
 	if (fTorrent.is_open())
 	{
		std::string str((std::istreambuf_iterator<char>(fTorrent)), std::istreambuf_iterator<char>());
		data = str;
 	}
    this->torrentPath = torrentPath;
	this->serializedObj = data;
}

vector<char> Torrent::getChunk(const int& chunkNum, int& size){
	vector<char> chunk(chunkSize);
	ifstream fTorrent {this->packagePath, ifstream::binary};
	if (fTorrent.is_open()){
		fTorrent.seekg (chunkNum * this->chunkSize);
		fTorrent.read (&chunk[0], this->chunkSize);
	}
    size = fTorrent.gcount();
	return chunk;
}

void Torrent::putChunk(const vector<char>& chunk, const int size, const int chunkNum){
	string fullpath = getTorrentDataPath() + this->name;
	std::fstream ofs(fullpath, std::ios::binary | std::ios_base::out | std::ios_base::in);
	if (ofs.is_open()){
    	ofs.seekp(this->chunkSize * chunkNum, std::ios_base::beg);
    	ofs.write(chunk.data(), size);
	}
}

void Torrent::createTorrentDataFile(){
	string packagePath = getTorrentDataPath() + this->name;
	std::ofstream ofs(packagePath, std::ios::binary | std::ios::out);
    ofs.seekp(this->size-1);
    ofs.write("", 1);
}

bool Torrent::isComplete(){
	bool complete = true;
	if (chunks.empty())
		return false;

	for(auto it = this->chunks.begin(); it != this->chunks.end(); it++)
	{
    	if (it->exists == false)
    	{
    		complete = false;
    		break;
    	}
	}
	return complete;
}

bool Torrent::isValid(){
	return (numPieces > 0
			&& !name.empty()
			&& !packagePath.empty()
			&& !uid.empty()
			&& !chunks.empty()
			&& !serializedObj.empty());
}

bool Torrent::torrentDataExists(){
	string packagePath = getTorrentDataPath() + this->name;
	return Utils::doesFileExist(packagePath);
}

Torrent& Torrent::operator=(const Torrent& torrent){
	this->numPieces = torrent.numPieces;
	this->name = torrent.name;
	this->packagePath = torrent.packagePath;
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
	if (ret > 0)
		cout << "getTorrentNames error: " << ret << endl;
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