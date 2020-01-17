#include "db.h"


const string DatabaseConnector::tcp = "tcp://";
const string DatabaseConnector::createSchemaStatment = "create schema ";
const string DatabaseConnector::createTableStatment = "create table ";
const string DatabaseConnector::ifNotExists = "if not exists ";

DatabaseConnector::DatabaseConnector()
{
	this->driver = nullptr;
	this->con = nullptr;
}

DatabaseConnector::DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd)
{
	this->driver = nullptr;
	this->con = nullptr;
	this->ip = ip;
	this->port = port;
	this->user = user;
	this->pwd = pwd;
}

DatabaseConnector::DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd, 
	const string& schema)
{
	this->driver = nullptr;
	this->con = nullptr;
	this->ip = ip;
	this->port = port;
	this->user = user;
	this->pwd = pwd;
	this->schema = schema;
}

DatabaseConnector::~DatabaseConnector()
{
	if (this->con) delete this->con;
}

void DatabaseConnector::connect()
{
	try{
		this->driver = get_driver_instance();
		string url = this->tcp + this->ip + ":" + this->port;
		this->con = this->driver->connect(url, this->user, this->pwd);
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

void DatabaseConnector::connect(const string& ip, const string& port, const string& user, const string& pwd)
{
	this->ip = ip;
	this->port = port;
	this->user = user;
	this->pwd = pwd;
	this->schema = schema;
	try{
		connect();
	}
	catch(...){
		throw;
	}
}

void DatabaseConnector::setSchema()
{
	try{
		this->con->setSchema(this->schema);
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

void DatabaseConnector::setSchema(const string& schema)
{
	this->schema = schema;
	try{
		setSchema();
	}
	catch(...){
		throw;
	}
}

sql::Statement* DatabaseConnector::createStatement()
{
	sql::Statement* stmt = nullptr;
	try{
		stmt = this->con->createStatement();
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
	query += this->schema;
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

bool DatabaseConnector::createTable(const string& table, const vector<string>& columns, bool checkExists)
{
	bool res = false;
	string query = DatabaseConnector::createTableStatment;
	if (checkExists)
		query += DatabaseConnector::ifNotExists;
	query += table;
	query += " (";
	for (auto column : columns){
		query += column + ",";
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
