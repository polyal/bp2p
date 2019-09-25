#ifndef TORRENTFILEREQ_H
#define TORRENTFILEREQ_H

#include <vector>
#include "rrpacket.h"
#include "torrent.h"

using namespace std;


//class RRpacket;
class TorrentFileReq : public RRPacket {
	public:
		TorrentFileReq();
		TorrentFileReq(const vector<char>& req);

		void createRequest(const string& torrentName);
		void processRequest();
		void processRespose();

	private:
		Torrent torrent;
		string torrentName;

		void createRequest();

		void getTorrentNameFromReq(string& torrentName);
		void getSerialzedTorrent(const string& torrentName, string& serializedTorrent);
};

#endif