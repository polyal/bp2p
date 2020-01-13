#include <vector>
#include <tuple>
#include "db.h"

class TorrentDB : public DatabaseConnector
{
public:
	TorrentDB();
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd);
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);

	bool init();

protected:
	bool createTables();
	bool createTorrentTable();
	bool createFilesTable();
	bool createChunksTable();
	bool insertIntoTorrents(size_t uid, const string& name, unsigned int numPieces, unsigned int size);
	bool insertIntoFiles(size_t uid, const vector<string>& files);
	bool insertIntoChunks(size_t uid, unsigned int index, size_t hash, bool exists);

	static const string dbip;
	static const string dbport;
	static const string dbuser;
	static const string dbpwd;
	static const string schemaName;

	static const string torrentTable;
	static const string filesTable;
	static const string chunksTable;
};
