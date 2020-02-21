#include "torrentDB.h"

#define DEBUG 0

const string TorrentDB::schema = "bp2p";
const string TorrentDB::ip = "127.0.0.1";
const string TorrentDB::port = "3306";
const string TorrentDB::user = "bpuser";
const string TorrentDB::pwd = "bppwd";

DatabaseConnector::Address TorrentDB::addr{ip, port};
DatabaseConnector::Credentials TorrentDB::cred{user, pwd};

const string TorrentDB::torrentTableName = "torrents";
const string TorrentDB::filesTableName = "files";
const string TorrentDB::chunksTableName = "chunks";

TorrentDB::TorrentDB() : DatabaseConnector(addr, cred, schema)
{
	this->setSchema();
}

bool TorrentDB::init()
{
	vector<DatabaseConnector::Table> tables = createTableDefs();
	try{
		DatabaseConnector::init(TorrentDB::addr, TorrentDB::cred, TorrentDB::schema, tables);
	}
	catch(...){
		throw;
	}
	return false;
}

vector<DatabaseConnector::Table> TorrentDB::createTableDefs()
{
	vector<DatabaseConnector::Table> tables;	
	DatabaseConnector::Table torrentTable = createTorrentTableDef();
	DatabaseConnector::Table filesTable = createFilesTableDef();
	DatabaseConnector::Table chunksTable = createChunksTableDef();
	tables.push_back(torrentTable);
	tables.push_back(filesTable);
	tables.push_back(chunksTable);
	return tables;
}

DatabaseConnector::Table TorrentDB::createTorrentTableDef()
{
	DatabaseConnector::Table table;
	vector<DatabaseConnector::Column> columns;
	DatabaseConnector::Column column;
	column.def = "uid INT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "name VARCHAR(255) NOT NULL";
	columns.push_back(column);
	column.def = "num_pieces INT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "size BIGINT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "PRIMARY KEY (uid)";
	columns.push_back(column);
	table.name = TorrentDB::torrentTableName;
	table.columns = columns;
	return table;
}

DatabaseConnector::Table TorrentDB::createFilesTableDef()
{
	DatabaseConnector::Table table;
	vector<DatabaseConnector::Column> columns;
	DatabaseConnector::Column column;
	column.def = "uid INT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "name VARCHAR(255) NOT NULL";
	columns.push_back(column);
	column.def = "PRIMARY KEY (uid, name)";
	columns.push_back(column);
	column.def = "INDEX file_ind (uid, name)";
	columns.push_back(column);
	column.def = "FOREIGN KEY (uid) REFERENCES torrents (uid) ON UPDATE CASCADE ON DELETE CASCADE";
	columns.push_back(column);
	table.name = TorrentDB::filesTableName;
	table.columns = columns;
	return table;
}

DatabaseConnector::Table TorrentDB::createChunksTableDef()
{
	DatabaseConnector::Table table;
	vector<DatabaseConnector::Column> columns;
	DatabaseConnector::Column column;
	column.def = "uid INT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "chunk_index INT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "chunk_hash INT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "chunk_exists TINYINT UNSIGNED NOT NULL";
	columns.push_back(column);
	column.def = "PRIMARY KEY (uid, chunk_index)";
	columns.push_back(column);
	column.def = "INDEX chunk_ind (uid, chunk_index)";
	columns.push_back(column);
	column.def = "FOREIGN KEY (uid) REFERENCES torrents (uid) ON UPDATE CASCADE ON DELETE CASCADE";
	columns.push_back(column);
	table.name = TorrentDB::chunksTableName;
	table.columns = columns;
	return table;
}

bool TorrentDB::insertIntoTorrents(const string& name, unsigned int numPieces, unsigned long long size)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "INSERT INTO " + TorrentDB::torrentTableName + " VALUES (?, ?, ?, ?);";
	try{
		stmt = this->con->prepareStatement(query);
		stmt->setUInt(1, this->uid);
		stmt->setString(2, name);
		stmt->setUInt(3, numPieces);
		stmt->setUInt64(4, size);
		res = execute(stmt);
	}
	catch(...){
		if (stmt) delete stmt;
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool TorrentDB::insertIntoFiles(const vector<string>& files)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "INSERT INTO " + TorrentDB::filesTableName + " VALUES (?, ?);";
	try{
		stmt = this->con->prepareStatement(query);
		for (const auto& file : files){
			stmt->setUInt(1, this->uid);
			stmt->setString(2, file);
			execute(stmt);
		}
	}
	catch(...){
		if (stmt) delete stmt;
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool TorrentDB::insertIntoChunks(unsigned int index, size_t hash, bool exists)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "INSERT INTO " + TorrentDB::chunksTableName + " VALUES (?, ?, ?, ?);";
	try{
		stmt = this->con->prepareStatement(query);
		stmt->setUInt(1, this->uid);
		stmt->setUInt(2, index);
		stmt->setUInt(3, hash);
		exists ? stmt->setUInt(4, 1) : stmt->setUInt(4, 0);
		execute(stmt);
	}
	catch(...){
		if (stmt) delete stmt;
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool TorrentDB::updateChunks(const vector<ChunkRow>& chunks)
{
	bool res = false;
	sql::PreparedStatement* stmt = nullptr;
	string query = "UPDATE " + TorrentDB::chunksTableName + " SET chunk_exists=? WHERE uid=? AND chunk_index=?;";
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
		if (stmt) delete stmt;
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

TorrentDB::TorrentJoined TorrentDB::getJoinedTorrent()
{
	TorrentInfoRow torInfo = getTorrentInfo();		
	vector<TorrentDB::FileRow> files = getTorrentFiles();
	vector<TorrentDB::ChunkRow> chunks = getTorrentChunks();
	TorrentJoined torrent{torInfo, files, chunks};
	return torrent; 
}

TorrentDB::TorrentInfoRow TorrentDB::getTorrentInfo()
{
	sql::PreparedStatement* stmt = nullptr;
	sql::ResultSet* res = nullptr;
	TorrentInfoRow torInfo;
	string query = "SELECT name, num_pieces, size FROM " + TorrentDB::torrentTableName + " WHERE uid=?;";
	try{
		stmt = this->con->prepareStatement(query);
		stmt->setUInt(1, this->uid);
		res = executeQuery(stmt);
		if (res->next()){
			torInfo.name = res->getString("name");
			torInfo.numPieces = res->getUInt("num_pieces");
			torInfo.size = res->getUInt64("size");
		}
	}
	catch(...){
		if (stmt) delete stmt;
		if (res) delete res;
		throw;
	}
	if (stmt) delete stmt;
	if (res) delete res;
	return torInfo;
}

vector<TorrentDB::FileRow> TorrentDB::getTorrentFiles()
{
	return getTorrentFiles(this->con, this->uid);	
}

vector<TorrentDB::FileRow> TorrentDB::getTorrentFiles(sql::Connection* const con, size_t uid)
{
	sql::PreparedStatement* stmt = nullptr;
	sql::ResultSet* res = nullptr;
	vector<FileRow> files;
	string query = "SELECT name FROM " + TorrentDB::filesTableName + " WHERE uid=?;";
	try{
		stmt = con->prepareStatement(query);
		stmt->setUInt(1, uid);
		res = executeQuery(stmt);
		while (res->next()){
		    string name = res->getString("name");
		    FileRow file{name};
		    files.push_back(file);
		}
	}
	catch(...){
		if (stmt) delete stmt;
		if (res) delete res;
		throw;
	}
	if (stmt) delete stmt;
	if (res) delete res;
	return files;
}

vector<TorrentDB::ChunkRow> TorrentDB::getTorrentChunks()
{
	return getTorrentChunks(this->con, this->uid);
}

vector<TorrentDB::ChunkRow> TorrentDB::getTorrentChunks(sql::Connection* const con, size_t uid)
{
	sql::PreparedStatement* stmt = nullptr;
	sql::ResultSet* res = nullptr;
	vector<ChunkRow> chunks;
	string query = "SELECT chunk_index, chunk_hash, chunk_exists FROM " + TorrentDB::chunksTableName + " WHERE uid=?;";
	try{
		stmt = con->prepareStatement(query);
		stmt->setUInt(1, uid);
		res = executeQuery(stmt);
		while (res->next()){
		    unsigned int index = res->getUInt("chunk_index");
		    unsigned int hash = res->getUInt("chunk_hash");
		    bool exists = false;
		    if (res->getUInt("chunk_exists")) exists = true;
		    ChunkRow chunk{index, hash, exists};
		    chunks.push_back(chunk);
		}
	}
	catch(...){
		if (stmt) delete stmt;
		if (res) delete res;
		throw;
	}
	if (stmt) delete stmt;
	if (res) delete res;
	return chunks;
}

vector<TorrentDB::TorrentJoined> TorrentDB::getAllTorrentRows()
{
	vector<TorrentDB::TorrentJoined> joinedTorRows;
	try{
		connectIfNeeded();
		setSchema(TorrentDB::schema);
		vector<TorrentInfoRow> torInfoRows;
		torInfoRows = getAllTorrentInfoRows();
		for (const auto& torInfo : torInfoRows){
			vector<TorrentDB::FileRow> fileRows = getTorrentFiles(con, torInfo.uid);
			vector<TorrentDB::ChunkRow> chunkRows = getTorrentChunks(con, torInfo.uid);
			TorrentJoined joinedTor{torInfo, fileRows, chunkRows};
			joinedTorRows.push_back(joinedTor);
		}
		disconnectIfNeeded();
	}
	catch(...){
		disconnectIfNeeded();
		throw;
	}
	return joinedTorRows;
}

vector<TorrentDB::TorrentInfoRow> TorrentDB::getAllTorrentInfoRows()
{
	sql::Statement* stmt = nullptr;
	sql::ResultSet* res = nullptr;
	vector<TorrentInfoRow> torInfoRows;
	string query = "SELECT uid, name, num_pieces, size FROM " + TorrentDB::torrentTableName;
	try{
		stmt = con->createStatement();
		res = executeQuery(stmt, query);
		TorrentInfoRow torInfo;
		while (res->next()){
			torInfo.uid = res->getUInt("uid");
			torInfo.name = res->getString("name");
			torInfo.numPieces = res->getUInt("num_pieces");
			torInfo.size = res->getUInt64("size");
			torInfoRows.push_back(torInfo);
		}
	}
	catch(...){
		if (stmt) delete stmt;
		if (res) delete res;
		throw;
	}
	if (stmt) delete stmt;
	if (res) delete res;
	return torInfoRows;
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
