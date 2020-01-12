#include "torrentDB.h"

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
	columns.push_back("`index` INT UNSIGNED NOT NULL");
	columns.push_back("`hash` INT UNSIGNED NOT NULL");
	columns.push_back("`exists` TINYINT UNSIGNED NOT NULL");
	columns.push_back("PRIMARY KEY (uid, `index`)");
	columns.push_back("INDEX chunk_ind (uid, `index`)");
	columns.push_back("FOREIGN KEY (uid) REFERENCES torrents (uid) ON UPDATE CASCADE ON DELETE CASCADE");
	try{
		res = createTable(TorrentDB::chunksTable, columns, true);
	}
	catch(...){
		throw;
	}
	return res;
}


int main(void)
{
	cout << endl;
	cout << "Running 'SELECT * FROM testTable;'..." << endl;

	try {
		TorrentDB test;
		test.init();
	    /*sql::Driver *driver;
	 	sql::Connection *con;
	 	sql::Statement *stmt;
	 	sql::ResultSet *res;

	  	// Create a connection
	  	driver = get_driver_instance();
	  	con = driver->connect("tcp://127.0.0.1:3306", "testUser", "pwd");
	  	// Connect to the MySQL test database
	  	con->setSchema("test");

	  	stmt = con->createStatement();
	  	res = stmt->executeQuery("SELECT * FROM testTable;");
	  	while (res->next()) {
	    	cout << "\t... MySQL replies: ";
	    	// Access column data by alias or column name
	    	cout << res->getString("text") << endl;
	    	cout << "\t... MySQL says it again: ";
	    	// Access column fata by numeric offset, 1 is the first column
	    	cout << res->getString(1) << endl;
	  	}
	  	delete res;
	  	delete stmt;
	  	delete con;*/
	} 
	catch (sql::SQLException &e) {
	  	cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
}

	cout << endl;

	return EXIT_SUCCESS;
}
