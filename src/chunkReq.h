#ifndef CHUNKREQ_H
#define CHUNKREQ_H

#include <vector>
#include "torrent.h"
#include "rrpacket.h"

using namespace std;

class ChunkReq : public RRPacket {
	public:
		ChunkReq();
		ChunkReq(const vector<char>& req);

		void createRequest(const string& torrentName, const int& chunkNum);
		void processRequest();
		void processRespose(const vector<char>& chunk, const int& size);

	private:
		string torrentName;
		vector<char> chunk;
		int chunkNum = -1;
		int size = -1;

		void createRequest();

		void getTorrentNameFromReq(string& torrentName);
		int getChunkNumFromReq(const string& req);
		void retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk, int& size);

		void processRespose();
};

#endif