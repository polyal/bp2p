#ifndef TORRENTLISTREQ_H
#define TORRENTLISTREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


class TorrentListReq : public RRPacket {
	public:
		TorrentListReq();
		TorrentListReq(const vector<char>& req);

		void createRequest();
		void processRequest();
		void processRespose();

		vector<string> torrentList;

	private:
		void getTorrentList(vector<string>& torrentNames);
		void serializeTorrentList(const vector<string>& torrentNames, string& serializedList);

		void parseTorrentList(const string& resp, vector<string>& torrentList);
};

#endif
