/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <tuple>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

class DatabaseConnector
{
public:
	DatabaseConnector();
	DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd);
	DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);
	~DatabaseConnector();

	void connect();
	void connect(const string& ip, const string& port, const string& user, const string& pwd);
	void setSchema();
	void setSchema(const string& schema);
	sql::Statement* createStatement();
	sql::ResultSet* executeQuery(sql::Statement* stmt, const string& query);
	sql::ResultSet* executeQuery(sql::PreparedStatement* stmt);
	sql::ResultSet* executeQuery(sql::PreparedStatement* stmt, const string& query);
	bool execute(sql::Statement* stmt, const string& query);
	bool execute(sql::PreparedStatement* stmt);
	sql::ResultSet* createStatementAndExecuteQuery(const string& query);
	bool createStatementAndExecute(const string& query);
	bool createSchema(bool checkExists);
	bool createSchema(const string& schema, bool checkExists);
	bool createTable(const string& table, const vector<string>& columns, bool checkExists);

protected:
	static const string tcp;
	static const string createSchemaStatment;
	static const string createTableStatment;
	static const string ifNotExists;

	string ip;
	string port;
	string user;
	string pwd;
	string schema;

	sql::Driver* driver;
	sql::Connection* con;
};
