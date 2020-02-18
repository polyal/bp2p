#include <stdlib.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <mutex>
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
	struct Address;
	struct Credentials;
	struct Column;
	struct Table;

public:
	DatabaseConnector();
	DatabaseConnector(const Address& addr, const Credentials& cred, const string& schema);
	~DatabaseConnector();

	static void init(const Address& addr, const Credentials& cred, const string& schema, 
		const vector<DatabaseConnector::Table>& tables);
	static sql::ResultSet* createStatementAndExecuteQuery(const string& query);
	static bool createStatementAndExecute(const string& query);
	static sql::Statement* createStatement();
	static sql::ResultSet* executeQuery(sql::Statement* stmt, const string& query);
	static sql::ResultSet* executeQuery(sql::PreparedStatement* stmt);
	static sql::ResultSet* executeQuery(sql::PreparedStatement* stmt, const string& query);
	static bool execute(sql::Statement* stmt, const string& query);
	static bool execute(sql::PreparedStatement* stmt);

protected:
	struct Address
	{
		Address()
		{
		}

		Address(string ip, string port)
		: ip{ip}, port{port} {}
		string ip;
		string port;
	};

	struct Credentials
	{
		Credentials()
		{
		}
		
		Credentials(string user, string pwd)
		: user{user}, pwd{pwd} {}
		string user;
		string pwd;
	};

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

	static void connect();
	static void reconnectIfNeeded();
	static void disconnect();
	static bool createSchema(bool checkExists);
	static bool createSchema(const string& schema, bool checkExists);
	static void setSchema();
	static bool createTables();
	static bool createTable(const DatabaseConnector::Table& table, bool checkExists);

private:
	static Address addr;
	static Credentials cred;
	static string schema;
	static vector<Table> tables;

	static recursive_mutex mutex;
	static unique_lock<recursive_mutex> lock;
	static unsigned int connectionCounter;


	static void initDriver();
};
