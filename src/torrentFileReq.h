#ifndef TORRENTFILEREQ_H
#define TORRENTFILEREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


//class RRpacket;
class TorrentFileReq : public RRPacket {
	public:
		TorrentFileReq();
		TorrentFileReq(const vector<char>& req);

		void processRequest();
		void createRequest(const string& torrentName);

	private:
		string torrentName;

		void createRequest();

		void getTorrentNameFromReq(string& torrentName);
		void getSerialzedTorrent(const string& torrentName, string& serializedTorrent);
};

#endif