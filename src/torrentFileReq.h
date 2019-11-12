#ifndef TORRENTFILEREQ_H
#define TORRENTFILEREQ_H

#include <vector>
#include "torrent.h"
#include "rrpacket.h"

using namespace std;

class TorrentFileReq : public RRPacket 
{
public:
	TorrentFileReq();
	TorrentFileReq(const Message& req);
	TorrentFileReq(const string& torrentName);

	void createRequest();
	void processRequest();
	void processResponse(const Message& msg);

private:
	Torrent torrent;
	string torrentName;

	void processResponse();

	void getTorrentNameFromReq(string& torrentName);
	void getSerialzedTorrent(const string& torrentName, string& serializedTorrent);
};

#endif