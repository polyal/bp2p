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
protected:
	struct Column;
	struct Table;

public:
	DatabaseConnector();
	DatabaseConnector(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);

	static void init(const string& ip, const string& port, const string& user, const string& pwd, 
		const string& schema, const vector<DatabaseConnector::Table> tables);
	static sql::ResultSet* createStatementAndExecuteQuery(const string& query);
	static bool createStatementAndExecute(const string& query);
	static sql::Statement* createStatement();
	static sql::ResultSet* executeQuery(sql::Statement* stmt, const string& query);
	static sql::ResultSet* executeQuery(sql::PreparedStatement* stmt);
	static sql::ResultSet* executeQuery(sql::PreparedStatement* stmt, const string& query);
	static bool execute(sql::Statement* stmt, const string& query);
	static bool execute(sql::PreparedStatement* stmt);

protected:
	struct Column
	{
		string def;
	};

	struct Table
	{
		string name;
		vector<Column> columns;
	};

	static sql::Driver* driver;
	static sql::Connection* con;

	static const string tcp;
	static const string createSchemaStatment;
	static const string createTableStatment;
	static const string ifNotExists;

	static vector<Table> tables;

	static bool connect();
	static void reconnectIfNeeded();
	static void disconnect();
	static bool createSchema(bool checkExists);
	static bool createSchema(const string& schema, bool checkExists);
	static void setSchema();
	static bool createTables();
	static bool createTable(const DatabaseConnector::Table& table, bool checkExists);

private:
	static string ip;
	static string port;
	static string user;
	static string pwd;
	static string schema;

	static void initDriver();
};
