/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

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

using namespace std;

class DatabaseConnector
{
public:
	DatabaseConnector();
	DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd);
	DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);

	void connect();
	void connect(const string& ip, const string& port, const string& user, const string& pwd);
	void setSchema();
	void setSchema(const string& schema);
	sql::Statement* createStatement();
	sql::ResultSet* executeQuery(sql::Statement* stmt, const string& query);
	bool execute(sql::Statement* stmt, const string& query);
	sql::ResultSet* createStatementAndExecuteQuery(const string& query);
	bool createStatementAndExecute(const string& query);
	bool createSchema(const string& schema, bool checkExists);

private:
	static string tcp;
	static string createStatment;
	static string ifNotExists;

	string ip;
	string port;
	string user;
	string pwd;
	string schema;

	sql::Driver* driver;
	sql::Connection* con;
};
