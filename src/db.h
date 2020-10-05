#include <stdlib.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <mutex>
#include <sys/mman.h>
#include <string.h>
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
	//struct Credentials;
	struct Column;
	struct Table;

public:
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

	DatabaseConnector();
	DatabaseConnector(const Address& addr, const Credentials& cred, const string& schema);
	~DatabaseConnector();

	static void firstTimeInit(const Address& addr, const Credentials& privUser, const Credentials& newUser, 
		const string& schema, const vector<DatabaseConnector::Table>& tables);
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

	struct SafeCredentials
	{
		SafeCredentials()
		{
			pwd = (char*)malloc(sizeof(char)* pwdSize);
			int ret = mlock(pwd, pwdSize);
			if (ret == -1)
				cout << "db mlock error: " << errno << endl;
			memset(pwd, 0x00, pwdSize);
		}
		
		SafeCredentials(const string& user, const char* const pwd)
		: user{user}
		{
			this->user = user;
			size_t len = strlen(pwd);
			memcpy(this->pwd, pwd, len > pwdSize ? pwdSize : len);
		}

		bool isValid()
		{
			if (pwd == NULL)
				return false;
			return true;
		}

		~SafeCredentials()
		{
			int ret = munlock(pwd, pwdSize);
			if (ret == -1)
				cout << "db munlock error: " << errno << endl;
			if (pwd) free(pwd);
		}
	private:
		string user;
		char* pwd = NULL;
		static const unsigned int pwdSize = 33;
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

	static recursive_mutex mutex;

	static const string tcp;
	static const string createUserStatment;
	static const string grantAllStatment;
	static const string createSchemaStatment;
	static const string createTableStatment;
	static const string ifNotExists;

	static void connect();
	static void connectIfNeeded();
	static void reconnectIfNeeded();
	static void disconnect();
	static void disconnectIfNeeded();
	static bool createUser(bool checkExists, const Credentials& user);
	static bool createSchema(bool checkExists);
	static bool createSchema(const string& schema, bool checkExists);
	static void setSchema();
	static void setSchema(const string& schema);
	static bool createTables();
	static bool createTable(const DatabaseConnector::Table& table, bool checkExists);
	static bool grantAllUser(const Credentials& user);

private:
	static Address addr;
	static Credentials cred;
	static string schema;
	static vector<Table> tables;

	static unsigned int connectionCounter;


	static void initDriver();
};
