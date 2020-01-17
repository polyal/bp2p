#include "torrentDB.h"

#define DEBUG 0

const string TorrentDB::schemaName = "bp2p";
const string TorrentDB::dbip = "127.0.0.1";
const string TorrentDB::dbport = "3306";
const string TorrentDB::dbuser = "bpuser";
const string TorrentDB::dbpwd = "bppwd";

const string TorrentDB::torrentTable = "torrents";
const string TorrentDB::filesTable = "files";
const string TorrentDB::chunksTable = "chunks";

TorrentDB::TorrentDB()
{
	this->ip = dbip;
	this->port = dbport;
	this->user = dbuser;
	this->pwd = dbpwd;
	this->schema = schemaName;
}

TorrentDB::TorrentDB(const string& ip, const string& port, const string& user, const string& pwd)
	: DatabaseConnector(ip, port, user, pwd)
{
	this->schema = schemaName;	
}

TorrentDB::TorrentDB(const string& ip, const string& port, const string& user, const string& pwd, const string& schema)
	: DatabaseConnector(ip, port, user, pwd, schema)
{
}

bool TorrentDB::init()
{
	try{
		connect();
		createSchema(TorrentDB::schemaName, true);
		setSchema();
		createTables();	
	}
	catch(...){
		throw;
	}
	

	return false;
}

bool TorrentDB::createTables()
{
	bool res = false;
	try{
		createTorrentTable();
		createFilesTable();
		createChunksTable();	
	}
	catch(...){
		throw;
	}
	return res;  
}

bool TorrentDB::createTorrentTable()
{
	bool res = false;
	vector<string> columns;
	columns.push_back("uid INT UNSIGNED NOT NULL");
	columns.push_back("name VARCHAR(255) NOT NULL");
	columns.push_back("num_pieces INT UNSIGNED NOT NULL");
	columns.push_back("size BIGINT UNSIGNED NOT NULL");
	columns.push_back("PRIMARY KEY (uid)");
	try{
		res = createTable(TorrentDB::torrentTable, columns, true);
	}
	catch(...){
		throw;
	}
	return res;
}

bool TorrentDB::createFilesTable()
{
	bool res = false;
	vector<string> columns;
	columns.push_back("uid INT UNSIGNED NOT NULL");
	columns.push_back("file VARCHAR(255) NOT NULL");
	columns.push_back("PRIMARY KEY (uid, file)");
	columns.push_back("INDEX file_ind (uid, file)");
	columns.push_back("FOREIGN KEY (uid) REFERENCES torrents (uid) ON UPDATE CASCADE ON DELETE CASCADE");
	try{
		res = createTable(TorrentDB::filesTable, columns, true);
	}
	catch(...){
		throw;
	}
	return res;
}

bool TorrentDB::createChunksTable()
{
	bool res = false;
	vector<string> columns;
	columns.push_back("uid INT UNSIGNED NOT NULL");
	columns.push_back("chunk_index INT UNSIGNED NOT NULL");
	columns.push_back("chunk_hash INT UNSIGNED NOT NULL");
	columns.push_back("chunk_exists TINYINT UNSIGNED NOT NULL");
	columns.push_back("PRIMARY KEY (uid, chunk_index)");
	columns.push_back("INDEX chunk_ind (uid, chunk_index)");
	columns.push_back("FOREIGN KEY (uid) REFERENCES torrents (uid) ON UPDATE CASCADE ON DELETE CASCADE");
	try{
		res = createTable(TorrentDB::chunksTable, columns, true);
	}
	catch(...){
		throw;
	}
	return res;
}

bool TorrentDB::insertIntoTorrents(const string& name, unsigned int numPieces, unsigned long long size)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "INSERT INTO " + TorrentDB::torrentTable + " VALUES (?, ?, ?, ?);";
	try{
		stmt = this->con->prepareStatement(query);
		stmt->setUInt(1, this->uid);
		stmt->setString(2, name);
		stmt->setUInt(3, numPieces);
		stmt->setUInt64(4, size);
		res = execute(stmt);
	}
	catch(...){
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool TorrentDB::insertIntoFiles(const vector<string>& files)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "INSERT INTO " + TorrentDB::filesTable + " VALUES (?, ?);";
	try{
		stmt = this->con->prepareStatement(query);
		for (const auto& file : files){
			stmt->setUInt(1, this->uid);
			stmt->setString(2, file);
			execute(stmt);
		}
	}
	catch(...){
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool TorrentDB::insertIntoChunks(unsigned int index, size_t hash, bool exists)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "INSERT INTO " + TorrentDB::chunksTable + " VALUES (?, ?, ?, ?);";
	try{
		stmt = this->con->prepareStatement(query);
		stmt->setUInt(1, this->uid);
		stmt->setUInt(2, index);
		stmt->setUInt(3, hash);
		exists ? stmt->setUInt(4, 1) : stmt->setUInt(4, 0);
		execute(stmt);
	}
	catch(...){
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool TorrentDB::updateChunks(const vector<ChunkRow>& chunks)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "UPDATE " + TorrentDB::chunksTable + " SET chunk_exists=? WHERE uid=? AND chunk_index=?;";
	try{
		stmt = this->con->prepareStatement(query);
		for (const auto& chunk : chunks){
			chunk.exists ? stmt->setUInt(1, 1) : stmt->setUInt(1, 0);
			stmt->setUInt(2, this->uid);
			stmt->setUInt(3, chunk.index);
			execute(stmt);
		}
	}
	catch(...){
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

#if DEBUG == 1
int main(void)
{
	try {
		TorrentDB test;
		test.init();
		/*test.insertIntoTorrents(123, "torrent_name45", 2, 52046);
		vector<string> files;
		files.push_back("file_1");
		files.push_back("file_2");
		files.push_back("file_3");
		test.insertIntoFiles(123, files);
		test.insertIntoChunks(123, 0, 4294967295, true);
		test.insertIntoChunks(123, 1, 123498765, false);*/
		test.updateChunk(1, true);
	} 
	catch (sql::SQLException &e) {
	  	cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	return EXIT_SUCCESS;
}
#endif
