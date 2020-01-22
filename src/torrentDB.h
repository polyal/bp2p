#ifndef TORRENTDB_H
#define TORRENTDB_H
#include <vector>
#include <tuple>
#include "db.h"

class Torrent;

class TorrentDB : public DatabaseConnector
{
	friend class Torrent;
protected:
	struct TorrentJoined;
	struct ChunkRow;
	struct FileRow;
	struct TorrentInfoRow;

public:
	TorrentDB();
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd);
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);

	bool init();
	bool insertIntoTorrents(const string& name, unsigned int numPieces, unsigned long long size);
	bool insertIntoFiles(const vector<string>& files);
	bool insertIntoChunks(unsigned int index, size_t hash, bool exists);
	bool updateChunks(const vector<ChunkRow>& chunks);
	TorrentJoined getJoinedTorrent();

protected:
	static const string dbip;
	static const string dbport;
	static const string dbuser;
	static const string dbpwd;
	static const string schemaName;

	static const string torrentTable;
	static const string filesTable;
	static const string chunksTable;

	bool createTables();
	bool createTorrentTable();
	bool createFilesTable();
	bool createChunksTable();
	TorrentInfoRow getTorrentInfo();
	vector<FileRow> getTorrentFiles();
	vector<ChunkRow> getTorrentChunks();

	size_t uid = 0;

	struct TorrentInfoRow
	{
		string name;
		unsigned int numPieces;
		unsigned long long size;
		TorrentInfoRow()
		: numPieces{0}, size{0} {}
		TorrentInfoRow(string name, unsigned int numPieces, unsigned long long size)
		: name{name}, numPieces{numPieces}, size{size} {}
	};

	struct FileRow
	{
		string name;
		FileRow(string name)
		: name{name} {}
	};

	struct ChunkRow 
	{
		unsigned int index;
		unsigned int hash;
		bool exists;
		ChunkRow() 
		: index{0}, exists{false} {}
		ChunkRow(unsigned int index, bool exists) 
		: index{index}, exists{exists} {}
		ChunkRow(unsigned int index, unsigned int hash, bool exists) 
		: index{index}, hash{hash}, exists{exists} {}
	};

	struct TorrentJoined
	{
		TorrentInfoRow torrentInfo;
		vector<FileRow> files;
		vector<ChunkRow> chunks;
		TorrentJoined(TorrentInfoRow torrentInfo, vector<FileRow> files, vector<ChunkRow> chunks)
		: torrentInfo{torrentInfo}, files{files}, chunks{chunks}{}
	};
};

#endif