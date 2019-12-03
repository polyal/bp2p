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
#include "package.h"
#include "torrent.h"

#define DEBUG 0

using namespace std;


Torrent::Torrent()
{
	this->numPieces = 0;
	this->name = "";
	this->packagePath = "";
	this->torrentPath = "";
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
	this->size = 0;
	this->serializedObj = "";
}

Torrent::Torrent(const string& name)
{
	this->numPieces = 0;
	this->name = name;
	this->packagePath = "";
	this->torrentPath = "";
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
	this->chunks.clear();
	copy(torrent.chunks.begin(), torrent.chunks.end(), back_inserter(this->chunks));
	this->jobj = torrent.jobj;
	this->serializedObj = torrent.serializedObj;
	this->size = torrent.size;
}

bool Torrent::create(const string& name, const vector<string>& files)
{
	this->name = name;
	this->files = files;
	return create();
}

bool Torrent::create()
{
	package();
	generateChunks();
	//generateFileHash();
	serialize();
	dumpToTorrentFile();
	return isValid();
}

bool Torrent::open(const string& name)
{
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
	serialize();
	dumpToTorrentFile();
	return isValid();
}

int Torrent::package()
{
	int ret = 0;
	string packagePath;

    if (this->name.empty() || this->files.empty())
    	return -1;

    // prepare package location
	packagePath = getTorrentDataPath() + this->name;

	Package package{packagePath, this->files};
	ret = package.package();
	if (ret != 0)
	{
		cout << "Create Package Error: " << ret << endl;
		return ret;
	}

	this->packagePath = packagePath;
	this->size = Utils::filesize(packagePath.c_str());
	return ret;
}

int Torrent::unpackage()
{
	string packagePath = getTorrentDataPath() + this->name;
	Package package{packagePath, this->files};
	int ret = package.unpackage();
	if (ret != 0)
	{
		cout << "Depackage Error: invalid input" << endl;
		return ret;
	}
	return 0;
}


int Torrent::generateChunks()
{
	this->chunks.clear();
	if (this->packagePath.empty())
		return -1;
    vector<char> fileChunk (this->chunkSize, 0);  // stores a chunk of the file for hashing
    string strChunk;
    unsigned int i = 0;
    ifstream file {this->packagePath, ifstream::binary};
    while(!file.eof()){
        file.read(fileChunk.data(), this->chunkSize);
        strChunk.assign(fileChunk.begin(), fileChunk.end());
        size_t chunkHash = hash<string>()(strChunk);
        this->chunks.push_back(Chunk{i, chunkHash, true});
        this->uid = (this->uid ^ (chunkHash << 1)) >> 1;
        fileChunk.clear();
        strChunk.clear();
        i++;
    }
    this->numPieces = this->chunks.size();
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
	this->chunks.clear();
	try{
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
	  		size_t hash = hashpair[0];
	  		bool exists = false;
	  		if (!create)
				exists = hashpair[1];
			this->chunks.push_back(Chunk{index, hash, exists});
		}
	}
	catch(...){
		this->chunks.clear();
	}
	sort(this->chunks.begin(), this->chunks.end(), 
		[](const Chunk& first, const Chunk& second)->bool
		{
			return first.index < second.index;
		});
}

void Torrent::dumpToTorrentFile()
{
	if (this->serializedObj.empty())
	{
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

vector<char> Torrent::getChunk(int index)
{
	vector<char> chunk(chunkSize);
	ifstream fTorrent {this->packagePath, ifstream::binary};
	if (fTorrent.is_open())
	{
		fTorrent.seekg (index * this->chunkSize);
		fTorrent.read (&chunk[0], this->chunkSize);
	}
    int size = fTorrent.gcount();
    chunk.resize(size);
	return chunk;
}

void Torrent::putChunk(const vector<char>& chunk, const int index)
{
	if (index < 0)
		return;
	string fullpath = getTorrentDataPath() + this->name;
	std::fstream ofs(fullpath, std::ios::binary | std::ios_base::out | std::ios_base::in);
	if (ofs.is_open())
	{
    	ofs.seekp(this->chunkSize * index, std::ios_base::beg);
    	ofs.write(chunk.data(), chunk.size());
    	this->chunks[index].exists = true;
	}
}

void Torrent::createTorrentDataFile()
{
	string packagePath = getTorrentDataPath() + this->name;
	std::ofstream ofs(packagePath, std::ios::binary | std::ios::out);
    ofs.seekp(this->size-1);
    ofs.write("", 1);
}

bool Torrent::isComplete() const
{
	bool complete = true;
	if (!this->chunks.empty()){
		for(const auto& chunk : this->chunks){
	    	if (!chunk.exists){
	    		complete = false;
	    		break;
	    	}
		}
	}
	else
		complete = false;
	return complete;
}

bool Torrent::isValid()
{
	return (this->numPieces > 0
			&& !this->name.empty()
			&& !this->packagePath.empty()
			&& !this->chunks.empty()
			&& !this->serializedObj.empty());
}

bool Torrent::torrentDataExists()
{
	string packagePath = getTorrentDataPath() + this->name;
	return Utils::doesFileExist(packagePath);
}

Torrent& Torrent::operator=(const Torrent& torrent)
{
	this->numPieces = torrent.numPieces;
	this->name = torrent.name;
	this->files = torrent.files;
	this->packagePath = torrent.packagePath;
	this->torrentPath = torrent.torrentPath;
	this->uid = torrent.uid;
	this->chunks.clear();
	copy(torrent.chunks.begin(), torrent.chunks.end(), back_inserter(this->chunks));
	this->jobj = torrent.jobj;
	this->serializedObj = torrent.serializedObj;
	this->size = torrent.size;

    return *this;  // Return a reference to myself.
}

bool Torrent::operator==(const Torrent& torrent) const
{
	return this->numPieces == torrent.numPieces &&
		   this->name == torrent.name &&
		   this->files == torrent.files &&
		   this->packagePath == torrent.packagePath &&
		   this->torrentPath == torrent.torrentPath &&
		   this->uid == torrent.uid &&
		   this->jobj == torrent.jobj &&
		   this->serializedObj == torrent.serializedObj &&
		   this->size == torrent.size;
}

string Torrent::getFilename() const
{
	return this->name;
}

string Torrent::getSerializedTorrent()
{
	return this->serializedObj;
}

size_t Torrent::getUid() const
{
	return this->uid;
}

vector<int> Torrent::getChunkAvail() const
{
	vector<int> avail;
	for (const auto& chunk : this->chunks){
		if (chunk.exists)
			avail.push_back(chunk.index);
	}
	return avail;
}

int Torrent::getMissingChunkIndex() const
{
	int chunkIndex = -1;
	Chunk chunk;
	if (this->chunks.size() > 0 && !isComplete()){
		do{
			int index = Utils::grnd(0, this->chunks.size()-1);
			chunk = this->chunks[index];
		} while (chunk.exists);
		chunkIndex = chunk.index;
	}
	return chunkIndex;
}

string Torrent::getTorrentsPath()
{
	string appPath = Utils::getApplicationPath();
	return appPath + torrentFileDir;
}

string Torrent::getTorrentDataPath()
{
	string appPath = Utils::getApplicationPath();
	return appPath + torrentDataDir;
}

vector<string> Torrent::getTorrentNames()
{
	int ret;
	string torrentsPath = getTorrentsPath();
	vector<string> torrentNames;
	ret = Utils::listFileInDir(torrentsPath, torrentNames);
	if (ret > 0)
		cout << "getTorrentNames error: " << ret << endl;
    return torrentNames;
}

#if DEBUG == 1
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "Usage: IMPORT: ./a.out [torrent]\n       EXPORT: ./a.out [torrent] [file1 | file2 | ...]" << endl;
		return 0;
	}

	string archive { argv[1] };
	cout << argc << archive << endl;

	if (argc == 2)
		Torrent newTorrent(archive);
	else
	{
		vector<string> files {argv+2, argv+argc};

		for (auto& file : files)
			cout << file << endl;
		cout << endl;

		Torrent newTorrent(archive, files);
		newTorrent.create();
	}

    return 0;
}
#endif