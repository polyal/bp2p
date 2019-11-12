#include <atomic>
#include <memory>

using namespace std;

class Node{
private:
	static inline string cli = "bp2p> ";

	enum DevStatus
	{
		READY,
		BUSY,
		INACTIVE
	};
	
	map<DeviceDescriptor, vector<DeviceDescriptor>> local2remote;
	map<DeviceDescriptor, DevStatus> remoteStatus;

public:
	struct Server
	{
		Server(unique_ptr<thread> t, shared_ptr<atomic<bool>> active) 
		{
			this->t = move(t);
			this->active = active;
		}

		void close()
		{
			*this->active = false;
			this->t->join();
		}

		unique_ptr<thread> t;
		shared_ptr<atomic<bool>> active;
	};

	struct Job
	{
		unique_ptr<RRPacket> req = nullptr;
		RRPacket::requestType type;
	};

	vector<DeviceDescriptor> localDevs;
	map<DeviceDescriptor, Server> servers;

	Node();

	static void printcli();

	void findLocalDevs();
	void scanForDevs();

	void requestTorrent(const string& torrentName);

	void requestTorrentList(const DeviceDescriptor& client, const DeviceDescriptor& server, Message& rsp);
	void requestTorrentFile(const DeviceDescriptor& client, const DeviceDescriptor& server, 
		const string& torrentName, Message& rsp);
	void requestChunk(const DeviceDescriptor& client, const DeviceDescriptor& server, 
		const string& torrentName, const int chunkNum, Message& rsp);

	static void processRequest(const Message& req, Message& rsp);

	unique_ptr<Server> createServerThread(DeviceDescriptor servDev);
	static void server(DeviceDescriptor dev, shared_ptr<atomic<bool>>);

private:
	void sendRequestWait4Response(RRPacket& req, Message& rsp, 
		const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes);

};

