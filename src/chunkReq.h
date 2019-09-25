#ifndef CHUNKREQ_H
#define CHUNKREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;

class ChunkReq : public RRPacket {
	public:
		ChunkReq();
		ChunkReq(const vector<char>& req);

		void processRequest();
		void createRequest(const string& torrentName, const int& chunkNum);

	private:
		string torrentName;
		int chunkNum = -1;

		void createRequest();

		void getTorrentNameFromReq(string& torrentName);
		int getChunkNumFromReq(const string& req);
		void retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk, int& size);
};

#endif