#include "torrentDB.h"

const string TorrentDB::schemaName = "bp2p";
const string TorrentDB::dbip = "127.0.0.1";
const string TorrentDB::dbport = "3306";
const string TorrentDB::dbuser = "bpuser";
const string TorrentDB::dbpwd = "pwd";

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
	createSchema(TorrentDB::schemaName, true);

	return false;
}

