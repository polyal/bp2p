#include <vector>
#include "rrpacket.h"

using namespace std;

class ChunkReq : public RRPacket {
	public:
		ChunkReq();
		ChunkReq(const vector<char>& req);

		void processRequest();

	private:
		void getTorrentNameFromReq(string& torrentName);
		int getChunkNumFromReq(const string& req);
		void retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk);
};