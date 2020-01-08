#include "db.h"

class TorrentDB : public DatabaseConnector
{
public:
	TorrentDB();
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd);
	TorrentDB(const string& ip, const string& port, const string& user, const string& pwd, const string& schema);

	bool init();

private:
	static const string schemaName;
	static const string dbip;
	static const string dbport;
	static const string dbuser;
	static const string dbpwd;
};
