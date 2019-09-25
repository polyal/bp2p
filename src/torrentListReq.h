#ifndef TORRENTLISTREQ_H
#define TORRENTLISTREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


class TorrentListReq : public RRPacket {
	public:
		TorrentListReq();
		TorrentListReq(const vector<char>& req);

		void processRequest();
		void createRequest();

	private:
		void getTorrentList(vector<string>& torrentNames);
		void serializeTorrentList(const vector<string>& torrentNames, string& serializedList);
};

#endif
