#include "db.h"

sql::Driver* DatabaseConnector::driver = nullptr;
sql::Connection* DatabaseConnector::con = nullptr;
vector<DatabaseConnector::Table> DatabaseConnector::tables;

const string DatabaseConnector::tcp = "tcp://";
const string DatabaseConnector::createSchemaStatment = "create schema ";
const string DatabaseConnector::createTableStatment = "create table ";
const string DatabaseConnector::ifNotExists = "if not exists ";

string DatabaseConnector::ip = "";
string DatabaseConnector::port = "";
string DatabaseConnector::user = "";
string DatabaseConnector::pwd = "";
string DatabaseConnector::schema = "";

DatabaseConnector::DatabaseConnector()
{
}

DatabaseConnector::DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd,
	const string& schema)
{
	this->con = nullptr;
	this->ip = ip;
	this->port = port;
	this->user = user;
	this->pwd = pwd;
	this->schema = schema;
}

void DatabaseConnector::init(const string& ip, const string& port, const string& user, const string& pwd,
	const string& schema, const vector<DatabaseConnector::Table> tables)
{
	DatabaseConnector::ip = ip;
	DatabaseConnector::port = port;
	DatabaseConnector::user = user;
	DatabaseConnector::pwd = pwd;
	DatabaseConnector::schema = schema;
	DatabaseConnector::tables = tables;
	initDriver();
	connect();
	createSchema(true);
	setSchema();
	createTables();
	disconnect();
}

void DatabaseConnector::initDriver()
{
	try{
		driver = get_driver_instance();
	}
	catch (sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	  	throw;
	}	
}

void DatabaseConnector::connect()
{
	try{
		string url = DatabaseConnector::tcp + DatabaseConnector::ip + ":" + DatabaseConnector::port;
		DatabaseConnector::con = driver->connect(url, DatabaseConnector::user, DatabaseConnector::pwd);
	}
	catch (sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	  	throw;
	}	
}

void DatabaseConnector::reconnectIfNeeded()
{
	try{
		if (DatabaseConnector::con)
			if (!DatabaseConnector::con->isValid())
				DatabaseConnector::con->reconnect();
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
}

void DatabaseConnector::disconnect()
{
	if (DatabaseConnector::con)
		delete DatabaseConnector::con;
	DatabaseConnector::con = nullptr;
}

void DatabaseConnector::setSchema()
{
	try{
		DatabaseConnector::con->setSchema(DatabaseConnector::schema);
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
}

sql::Statement* DatabaseConnector::createStatement()
{
	sql::Statement* stmt = nullptr;
	try{
		stmt = DatabaseConnector::con->createStatement();
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
	return stmt;
}

sql::ResultSet* DatabaseConnector::executeQuery(sql::Statement* stmt, const string& query)
{
	sql::ResultSet* res = nullptr;
	try{
		res = stmt->executeQuery(query);
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
	return res;
}

sql::ResultSet* DatabaseConnector::executeQuery(sql::PreparedStatement* stmt)
{
	sql::ResultSet* res = nullptr;
	try{
		res = stmt->executeQuery();
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
	return res;
}

sql::ResultSet* DatabaseConnector::executeQuery(sql::PreparedStatement* stmt, const string& query)
{
	sql::ResultSet* res = nullptr;
	try{
		res = stmt->executeQuery(query);
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
	return res;
}

bool DatabaseConnector::execute(sql::Statement* stmt, const string& query)
{
	bool res = false;
	try{
		res = stmt->execute(query);
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
	return res;	
}

bool DatabaseConnector::execute(sql::PreparedStatement* stmt)
{
	bool res = false;
	try{
		res = stmt->execute();
	}
	catch(sql::SQLException& e){
		cout << "# ERR: SQLException in " << __FILE__;
	  	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  	cout << "# ERR: " << e.what();
	  	cout << " (MySQL error code: " << e.getErrorCode();
	  	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		throw;
	}
	return res;	
}

sql::ResultSet* DatabaseConnector::createStatementAndExecuteQuery(const string& query)
{
	sql::ResultSet* res = nullptr;
	sql::Statement* stmt = nullptr;
	try{
		stmt = createStatement();
		res = executeQuery(stmt, query);
	}
	catch(...){
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool DatabaseConnector::createStatementAndExecute(const string& query)
{
	bool res = false;
	sql::Statement* stmt = nullptr;
	try{
		stmt = createStatement();
		res = execute(stmt, query);
	}
	catch(...){
		throw;
	}
	if (stmt) delete stmt;
	return res;
}

bool DatabaseConnector::createSchema(bool checkExists)
{
	bool res = false;
	string query = DatabaseConnector::createSchemaStatment;
	if (checkExists)
		query += DatabaseConnector::ifNotExists;
	query += DatabaseConnector::schema;
	try{
		res = createStatementAndExecute(query);
	}
	catch(...){
		throw;
	}
	return res;
}

bool DatabaseConnector::createSchema(const string& schema, bool checkExists)
{
	bool res = false;
	string query = DatabaseConnector::createSchemaStatment;
	if (checkExists)
		query += DatabaseConnector::ifNotExists;
	query += schema;
	try{
		res = createStatementAndExecute(query);
	}
	catch(...){
		throw;
	}
	return res;
}

bool DatabaseConnector::createTables()
{
	bool res = true;
	try{
		for (const auto& table : DatabaseConnector::tables)
			res = (createTable(table, true) && res);
	}
	catch(...){
		throw;
	}
	return res;
}

bool DatabaseConnector::createTable(const DatabaseConnector::Table& table, bool checkExists)
{
	bool res = false;
	string query = DatabaseConnector::createTableStatment;
	if (checkExists)
		query += DatabaseConnector::ifNotExists;
	query += table.name;
	query += " (";
	for (const auto& column : table.columns){
		query += column.def + ",";
	}
	query.pop_back(); // remove last comma
	query += " );";
	try{
		res = createStatementAndExecute(query);
	}
	catch(...){
		throw;
	}
	return res;
}
