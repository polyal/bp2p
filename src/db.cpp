#include "db.h"


string DatabaseConnector::tcp = "tcp://";
string DatabaseConnector::createStatment = "create schema ";
string DatabaseConnector::ifNotExists = "if not exists ";

DatabaseConnector::DatabaseConnector()
{	
}

DatabaseConnector::DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd)
{
	this->ip = ip;
	this->port = port;
	this->user = user;
	this->pwd = pwd;
}

DatabaseConnector::DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd, 
	const string& schema)
{
	this->ip = ip;
	this->port = port;
	this->user = user;
	this->pwd = pwd;
	this->schema = schema;
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

sql::ResultSet* DatabaseConnector::createStatementAndExecuteQuery(const string& query)
{
	sql::ResultSet* res;
	try{
		sql::Statement* stmt = createStatement();
		res = executeQuery(stmt, query);
	}
	catch(...){
		throw;
	}
	return res;
}

bool DatabaseConnector::createStatementAndExecute(const string& query)
{
	bool res = false;
	try{
		sql::Statement* stmt = createStatement();
		res = executeQuery(stmt, query);
	}
	catch(...){
		throw;
	}
	return res;
}

sql::ResultSet* DatabaseConnector::createSchema(const string& schema, bool checkExists)
{
	sql::ResultSet* res = nullptr;
	string query = DatabaseConnector::createStatment;
	if (checkExists)
		query += DatabaseConnector::ifNotExists;
	query += schema;
	try{
		res = createStatementAndExecuteQuery(query);
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
	 	sql::Driver *driver;
	 	sql::Connection *con;
	 	sql::Statement *stmt;
	 	sql::ResultSet *res;

	  	/* Create a connection */
	  	driver = get_driver_instance();
	  	con = driver->connect("tcp://127.0.0.1:3306", "testUser", "pwd");
	  	/* Connect to the MySQL test database */
	  	con->setSchema("test");

	  	stmt = con->createStatement();
	  	res = stmt->executeQuery("SELECT * FROM testTable;");
	  	while (res->next()) {
	    	cout << "\t... MySQL replies: ";
	    	/* Access column data by alias or column name */
	    	cout << res->getString("text") << endl;
	    	cout << "\t... MySQL says it again: ";
	    	/* Access column fata by numeric offset, 1 is the first column */
	    	cout << res->getString(1) << endl;
	  	}
	  	delete res;
	  	delete stmt;
	  	delete con;

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
