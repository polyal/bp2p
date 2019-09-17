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

#include <iostream>
#include <linux/limits.h>
#include <vector>
#include <tuple>

#include "nlohmann/json.hpp"

using namespace std;

class Torrent {
	private:
		static const int chunkSize = 32768;  //256 kilobyte chunk size
		static const string torrentFileDir;
		static const string torrentDataDir;

		int numPieces;
		string name;
		string fullpath;
		string uid;
		vector<tuple<string, bool>> chunks;

		nlohmann::json jobj;
		string serializedObj;

 	public:
 		Torrent();
 		Torrent(const string& torrentName); // takes the filename of an already existing torrent
 		Torrent(const string& torrentName, const vector<string>& files); // takes a list of files to be packaged


 		// takes the filename of a file to be converted into a torrent
		int createTorrent (const string& archive, const vector<string>& files);

		// takes the name of the archive to be created and a list of files to be archived abd compressed
		int createPackage(const string& archive, const vector<string>& files);

		int unpackage (const string& packageName);
		
 		// uses filename instance variable to generate chunks from and fills the chunks vector instance variable
		int generateChunks();

		// uses uses filename to hash the whole file.  This is used as a uid for the torrent
		int generateFileHash();

		// if create is true, all the chunks are marked as existing
		void serialize(bool create);

		// if create is true, all chunks are marked as nonexisting, meaning it is a nre torrent
		void deserialize(string& serializedObj, bool create);

		// uses the serialized instance varaible to create a torrent file
		void dumpToTorrentFile ();

		void readTorrentFromFile(const string& torrentName);

		bool isTorrentComplete();

		string getFilename();
		string getSerializedTorrent();
		vector<char> RetrieveChunk(const int chunkNum);

		static string getTorrentsPath();
		static string getTorrentDataPath();
		static vector<string> getTorrentNames();

};