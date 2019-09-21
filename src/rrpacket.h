// request reponse packet class
#include <vector>
#include <string>
#include <memory>

using namespace std;

class RRPacket {
	public:
		static const int chunkSize = 32768;  //256 kilobyte chunk size
		static const string commString;
		static const string commSeparator;
		vector<char> req;
		vector<char> resp;

		typedef enum _requestType {
			badReq,
			torrentFile, 
			chunk,
			torrentList 
		} requestType;

		RRPacket();
		RRPacket(const vector<char>& req);

		//static unique_ptr<RRPacket> create(const vector<char>& req);

		virtual void processRequest() = 0;

	private:
		static requestType getReqTypeFromReq(const vector<char>& req);
		template<typename T, typename... Args> unique_ptr<T> make_unique(Args&&... args);
};

/*class TorrentListReq : public RRpacket {
	public:
		TorrentListReq();
		TorrentListReq(const vector<char>& req);

	private:
		void getTorrentList(vector<string>& torrentNames);
		void serializeTorrentList(const vector<string>& torrentNames, string& serializedList);
};

class ChunkReq : public RRpacket {
	public:
		ChunkReq();
		ChunkReq(const vector<char>& req);

	private:
		void getTorrentNameFromReq(string& torrentName);
		int getChunkNumFromReq(const string& req);
		void retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk);
};*/