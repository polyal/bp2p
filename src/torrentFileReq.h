#include <vector>
#include "rrpacket.h"

using namespace std;


//class RRpacket;
class TorrentFileReq : public RRPacket {
	public:
		TorrentFileReq();
		TorrentFileReq(const vector<char>& req);

		void processRequest();

	private:
		void getTorrentNameFromReq(string& torrentName);
		void getSerialzedTorrent(const string& torrentName, string& serializedTorrent);
};