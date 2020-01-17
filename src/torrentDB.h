#ifndef TORRENTDB_H
#define TORRENTDB_H
#include <vector>
#include <tuple>
#include "db.h"

class Torrent;

class TorrentDB : public DatabaseConnector
{
	friend class Torrent;
public:
	struct ChunkRow;

	TorrentDB();
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd);
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);

	bool init();
	bool insertIntoTorrents(const string& name, unsigned int numPieces, unsigned long long size);
	bool insertIntoFiles(const vector<string>& files);
	bool insertIntoChunks(unsigned int index, size_t hash, bool exists);
	bool updateChunks(const vector<ChunkRow>& chunks);

	struct ChunkRow 
	{
		unsigned int index;
		bool exists;
		ChunkRow() 
		: index{0}, exists{false} {}
		ChunkRow(unsigned int index, bool exists) 
		: index{index}, exists{exists} {}
	};

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

	size_t uid = 0;
};

#endif