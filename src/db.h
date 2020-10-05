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
	struct Column;
	struct Table;

public:
	struct SafeCredentials
	{
		static const unsigned int userSize = 33;
		static const unsigned int pwdSize = 33;

		SafeCredentials()
		{
			initPwd();
		}
		
		SafeCredentials(const string& user, const char* const pwd)
		: user{user}
		{
			this->user = user;
			initPwd();
			size_t len = strlen(pwd);
			len > pwdSize ? this->len = pwdSize : this->len = len;
			memcpy(this->pwd, pwd, this->len);
		}

		~SafeCredentials()
		{
			int ret = munlock(this->pwd, pwdSize);
			if (ret == -1)
				cout << "db munlock error: " << errno << endl;
			delete[] this->pwd;
		}

		bool isValid()
		{
			if (pwd == NULL)
				return false;
			return true;
		}

		SafeCredentials& operator=(const SafeCredentials& cred)
		{
			this->user = cred.getUser();
			size_t len = strlen(cred.getPwd());
			memcpy(this->pwd, cred.getPwd(), len > pwdSize ? pwdSize : len);
		    return *this;
		}

		string getUser() const
		{
			return this->user;
		}

		char* getPwd() const
		{
			return this->pwd;
		}
		size_t getPwdLen() const
		{
			return this->len;
		}
		
	private:
		string user;
		char* pwd = nullptr;
		size_t len = 0;

		void initPwd()
		{
			this->pwd = new char[pwdSize];
			int ret = mlock(this->pwd, pwdSize);
			if (ret == -1)
				cout << "db mlock error: " << errno << endl;
			memset(this->pwd, 0x00, pwdSize);
		}
	};

	DatabaseConnector();
	DatabaseConnector(const Address& addr, const SafeCredentials& cred, const string& schema);
	~DatabaseConnector();

	static void firstTimeInit(const Address& addr, const SafeCredentials& privUser, const SafeCredentials& newUser, 
		const string& schema, const vector<DatabaseConnector::Table>& tables);
	static void init(const Address& addr, const SafeCredentials& cred, const string& schema, 
		const vector<DatabaseConnector::Table>& tables);
	static sql::ResultSet* createStatementAndExecuteQuery(const string& query);
	static bool createStatementAndExecute(const string& query);
	static bool createStatementAndExecute(const char* const query);
	static sql::Statement* createStatement();
	static sql::ResultSet* executeQuery(sql::Statement* stmt, const string& query);
	static sql::ResultSet* executeQuery(sql::PreparedStatement* stmt);
	static sql::ResultSet* executeQuery(sql::PreparedStatement* stmt, const string& query);
	static bool execute(sql::Statement* stmt, const string& query);
	static bool execute(sql::Statement* stmt, const char* const query);
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
	static const string identifiedBy;

	static void connect();
	static void connectIfNeeded();
	static void reconnectIfNeeded();
	static void disconnect();
	static void disconnectIfNeeded();
	static bool createUser(bool checkExists, const SafeCredentials& user);
	static bool createSchema(bool checkExists);
	static bool createSchema(const string& schema, bool checkExists);
	static void setSchema();
	static void setSchema(const string& schema);
	static bool createTables();
	static bool createTable(const DatabaseConnector::Table& table, bool checkExists);
	static bool grantAllUser(const SafeCredentials& user);

private:
	static Address addr;
	static SafeCredentials cred;
	static string schema;
	static vector<Table> tables;

	static unsigned int connectionCounter;


	static void initDriver();
};
