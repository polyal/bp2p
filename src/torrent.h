/**********************************************************
**
**    Torrent.h
**    Lorant Polya
**    June 30, 2019
**
**    This file defines all the needed functionality
**    for creating and using torrent files.
**
**    The general idea is to take in a filename or
**    folder and compress it.  Then take the compressed
**    file and split it up into chunks.  The chunks will
**	  be kept track of using this torrent file.
**
**********************************************************/

#ifndef TORRENT_H
#define TORRENT_H

#include <iostream>
#include <linux/limits.h>
#include <vector>
#include <tuple>
#include <functional>
#include "nlohmann/json.hpp"
#include "torrentDB.h"

using namespace std;

class Torrent
{
public:
	Torrent();

	///////////////////////////////////////////////////////////
	//  Ititializes invariant for creating a new torrent.
	//
	//  name:  the name of the new torrent
	//  files: files which are going to be included in the new torrent
	Torrent(const string& name, const vector<string>& files);

	///////////////////////////////////////////////////////////
	//  Ititializes invariant for opening an existing torrent.
	//
	//  name:  the name of the new torrent
	//  files: files which are going to be included in the new torrent
	Torrent(const string& name);

	///////////////////////////////////////////////////////////
	//  Copy constructor
	//
	//  torrent: the torrent to be duplicated
	Torrent(const Torrent& torrent);
		


	///////////////////////////////////////////////////////////
	//  Creates a torrent.
	//
	//  name:  the name of the new torrent
	//  files: files which are going to be included in the new torrent
	//
	//  returns true on success
	bool create(const string& name, const vector<string>& files);

	///////////////////////////////////////////////////////////
	//  Creates a torrent using the name provided and the list
	//  files.  Used in conjuction with the
	//	Torrent(const string& name, const vector<string>& files)
	//  constructor.
	//
	//  returns true on success
	bool create();

	///////////////////////////////////////////////////////////
	//  Opens an existing torrent and populates torrent object.
	//
	//  name:  name of the torrent to be opened
	//
	//  returns true on success
	bool open(const string& name);

	///////////////////////////////////////////////////////////
	//  Opens an existing torrent and populates torrent object.
	//  Used in conjuction with the constructor that takes the 
	//  torrent name.
	//
	//  returns true on success
	bool open();
		
	///////////////////////////////////////////////////////////
	//  Creates a torrent from a serialized json object stored
	//  as a string that represents the torrent. 
	//  This is used when a torrent is received as a string 
	//  across a channel and needs to be reconsteructed.
	//
	//  returns true on success
	bool createTorrentFromSerializedObj(const string& serializedObj);


	///////////////////////////////////////////////////////////
	//  Retreives a chunk from a torrent package
	//
	//  index:  The piece of the file to be returned
	//  size:   The size of the piece
	//
	//  returns the requested chunk or empty list of invalid
	vector<char> getChunk(const unsigned int index);

	///////////////////////////////////////////////////////////
	//  Puts a chunk of data into a torrent package
	//
	//  chunk:  The piece of the package to be inserted
	//  index:  The chunk index
	//
	//  returns true if chunk hash matches
	bool putChunk(const vector<char>& chunk, const unsigned int index);

	///////////////////////////////////////////////////////////
	//  Creates an empty package of the appropriate size
	void createTorrentDataFile();

	////////////////////////////////////////////////////////////
	//  Unpacked a package.
	int unpackage ();

	bool isComplete() const;
	bool isValid();
	bool torrentDataExists();

	Torrent& operator=(const Torrent& torrent);
	bool operator==(const Torrent& torrent) const;

	string getFilename() const;
	string getSerializedTorrent();
	size_t getUid() const;
	vector<int> getChunkAvail() const;
	int getMissingChunkIndex() const;

	static string getTorrentsPath();
	static string getTorrentDataPath();
	static vector<string> getTorrentNames();

private:
	static const int chunkSize = 32768;  //256 kilobyte chunk size
	static const string torrentFileDir;
	static const string torrentDataDir;

	struct Chunk
	{
		unsigned int index;
		size_t hash;
		bool exists;
		Chunk(unsigned int index, size_t hash, bool exists) 
		: index{index}, hash{hash}, exists{exists} {}
		Chunk(const Chunk& chunk)
		: index{chunk.index} , hash{chunk.hash}, exists{chunk.exists} {}
		Chunk()
		: index{0} , hash{0}, exists{false} {}

		Chunk& operator=(const Chunk& chunk)
		{
			this->index = chunk.index;
			this->hash = chunk.hash;
			this->exists = chunk.exists;
			return *this;
		}
	};
	
	string name;
	vector<string> files;                // files in torrent
	string packagePath;                  // path to the package
	string torrentPath;                  // path to the torrent
	size_t uid = 0;                      // hash of the package used as UID
	unsigned int numPieces;              // total num chunks needed to construct package
	vector<Chunk> chunks;  				 // chunks that the [ackage is made up of
	unsigned long long size;             // size of the package

	nlohmann::json jobj;                 // json object representing the torrent
	string serializedObj;                // serialzed version of the json object

	TorrentDB db;

	////////////////////////////////////////////////////////////
	//  Creates a package containing the files that were 
	//  specified in a call to the constructor or
	//  a call to createTorrent.
	int package();

	////////////////////////////////////////////////////////////
	//  divides the package into chunks that can be transfered
	//  accross a channel and checked against a hash
	int generateChunks();

	////////////////////////////////////////////////////////////
	//  verifies the hash of the chunk
	bool validateChunk(const vector<char>& chunk, const unsigned int index);

	////////////////////////////////////////////////////////////
	//  Serializes the contents of the torrent object to be
	//  stored in a file or transported across a channel
	void serialize();

	////////////////////////////////////////////////////////////
	//  Deserializes a string into a torrent object
	//
	//  create:  set to true to indicate tge torrent is being
	//           created for the first time.  All chunks will
	//           be set to false, indicating the user doesn't
	//           own them
	void deserialize(const bool create);

	////////////////////////////////////////////////////////////
	//  Writes the serialized torrent into a file, therefore,
	//  creating a torrent file
	void dumpToTorrentFile();

	////////////////////////////////////////////////////////////
	//  Reads a torrent from a torrent file and populates the
	//  serialzied torrent object
	//
	//  torrentPath:  full path to the torrent file to be read in
	void readTorrentFromFile(const string& torrentPath);

	void setDBuid();
	bool insertTorrentToDB();
};

namespace std
{
	template<>
	struct hash<Torrent>
	{
		size_t operator()(const Torrent& t) const
		{
			return t.getUid();
		}
	};
}

#endif
