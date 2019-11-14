#include <atomic>
#include <memory>
#include <mutex>
#include <list>
#include <condition_variable>

using namespace std;

class Node
{
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
	struct Peer
	{
		Peer(unique_ptr<thread> t, shared_ptr<atomic<bool>> kill) 
		{
			this->t = move(t);
			this->kill = kill;
		}

		void setKill()
		{
			*this->kill = true;
		}

		void close()
		{
			*this->kill = true;
			if (this->t) this->t->join();
		}

		unique_ptr<thread> t = nullptr;
		shared_ptr<atomic<bool>> kill;
	};


	vector<DeviceDescriptor> localDevs;
	map<DeviceDescriptor, Peer> servers;

	mutex jmMutex;
	condition_variable jmEvent;
	list<shared_ptr<RRPacket>> jobs;

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

	void processRequest(const Message& req, Message& rsp);

	unique_ptr<Peer> createServerThread(DeviceDescriptor servDev);
	unique_ptr<Peer> createJobManagerThread();

private:
	void sendRequestWait4Response(RRPacket& req, Message& rsp, 
		const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes);

	void serverThread(DeviceDescriptor dev, shared_ptr<atomic<bool>>);
	void jobManagerThread(shared_ptr<atomic<bool>> kill);

};

