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

	static bool init();
	bool insertIntoTorrents(const string& name, unsigned int numPieces, unsigned long long size);
	bool insertIntoFiles(const vector<string>& files);
	bool insertIntoChunks(unsigned int index, size_t hash, bool exists);
	bool updateChunks(const vector<ChunkRow>& chunks);
	TorrentJoined getJoinedTorrent();

	static vector<TorrentDB::TorrentJoined> getAllTorrentRows();


protected:
	static const string torrentTableName;
	static const string filesTableName;
	static const string chunksTableName;

	static const string ip;
	static const string port;
	static const string user;
	static const string pwd;
	static const string schema;

	static DatabaseConnector::Address addr;
	static DatabaseConnector::Credentials cred;

	static vector<DatabaseConnector::Table> createTableDefs();
	static DatabaseConnector::Table createTorrentTableDef();
	static DatabaseConnector::Table createFilesTableDef();
	static DatabaseConnector::Table createChunksTableDef();

	TorrentInfoRow getTorrentInfo();
	vector<FileRow> getTorrentFiles();
	vector<ChunkRow> getTorrentChunks();

	static vector<TorrentInfoRow> getAllTorrentInfoRows();
	static vector<FileRow> getTorrentFiles(sql::Connection* const con, size_t uid);
	static vector<ChunkRow> getTorrentChunks(sql::Connection* const con, size_t uid);

	size_t uid = 0;

	struct TorrentInfoRow
	{
		size_t uid;
		string name;
		unsigned int numPieces;
		unsigned long long size;
		TorrentInfoRow()
		: uid{0}, numPieces{0}, size{0} {}
		TorrentInfoRow(string name, unsigned int numPieces, unsigned long long size)
		: uid{0}, name{name}, numPieces{numPieces}, size{size} {}
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