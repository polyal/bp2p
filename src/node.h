#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>
#include <memory>
#include "deviceDescriptor.h"
#include "rrpacket.h"
#include "utils.h"
#include "workerThread.h"
#include "torrent.h"

using namespace std;

class Node
{
public:
	static const string createTorCmd;         // create torrent
	static const string listNearbyTorsCmd;    // list nearby torrents
	static const string requestTorCmd;        // request torrent file command
	static const string requestTorDataCmd;    // request torrent data command
	static const string quitCmd;              // quit

	static const string requestChunkAvailCmd; // testing request

	Node();

	static void printcli();

	void init();

	void activateWorkerThreads();
	void pauseWorkerThreads();
	void killWorkerThreads();

	bool createTorrent(const string& name, const vector<string>& files);
	int listNearbyTorrents(const vector<string>& addrs);
	bool syncRequestTorrentFile(const string& name, const string& addr);
	bool requestTorrentData(const string& name);

	int requestTorrentAvail(const string& name, const string& addr);  // testing

	void populateLocalTorrents();

private:
	static const string cli;

	enum DevStatus
	{
		READY,
		BUSY,
		INACTIVE
	};
	vector<DeviceDescriptor> localDevs;	
	map<DeviceDescriptor, vector<DeviceDescriptor>> local2remote;
	map<DeviceDescriptor, vector<DeviceDescriptor>> remote2local;
	map<DeviceDescriptor, DevStatus> remoteStatus;

	enum chunkStatus
	{
		HAVE,
		NEED,
		REQUESTING
	};
	map<DeviceDescriptor, vector<string>> dev2tor;                  // remote devices torrents
	map<string, vector<DeviceDescriptor>> torName2dev;              // torrents available from remote devices
	map<string, map<int, vector<DeviceDescriptor>>> torName2Avail;  // torrents available chunks
	//unordered_set<Torrent> torrents;
	map<string, Torrent> name2torrent;

	map<DeviceDescriptor, unique_ptr<WorkerThread>> servers;
	unique_ptr<WorkerThread> jobManager = nullptr;
	list<shared_ptr<RRPacket>> jobs;

	// initializations
	void initDB();
	void findLocalDevs();
	void scanForDevs();
	void createServers();
	void createJobManager();

	// request/response
	void carryOutRequest(RRPacket& req);
	void sendRequestWait4Response(const Message& req, Message& rsp, 
		const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes);
	void completeRequest(const RRPacket& packet);
	void completeRequest(const TorrentListReq& req);
	void completeRequest(const TorrentFileReq& req);
	void completeRequest(const TorrentAvailReq& req);
	void completeRequest(const ChunkReq& req);
	void processRequest(const Message& req, Message& rsp);
	void processRequest(RRPacket& req);
	void processRequest(TorrentListReq& req);
	void processRequest(TorrentFileReq& req);
	void processRequest(TorrentAvailReq& req);
	void processRequest(ChunkReq& req);

	void getTorrentNameList(vector<string>& torrentList);
	void getSerializedTorrent(string& serializedTorrent, const string& name);
	void getTorrentAvailFromTorrent(vector<int>& torrentAvail, const string& name);
	void retrieveChunk(vector<char>& chunk, const string& name, const int& index);

	// server/client init
	unique_ptr<WorkerThread> createServerThread(const DeviceDescriptor& servDev);
	void serverThread(DeviceDescriptor devDes);
	unique_ptr<WorkerThread> createJobManagerThread();
	void jobManagerThread();

	int request(RRPacket& req);
	int syncRequest(RRPacket& req);
	void retryRequest();
	void retryRequest(shared_ptr<RRPacket> req);
	void insertJob(const shared_ptr<RRPacket> job);

	bool syncRequestAllNearbyTorrents();
	bool syncRequestNearbyTorrents(const vector<DeviceDescriptor>& devs);
	bool syncRequestNearbyTorrents(const DeviceDescriptor& dev);
	bool syncRequestAllTorrentAvail(const string& name);
	bool syncRequestTorrentAvail(const string& name, const vector<DeviceDescriptor>& devs);
	bool syncRequestTorrentAvail(const string& name, const DeviceDescriptor& dev);
	bool syncRequestTorrentFile(const string& name);

	void requestAllNearbyTorrents();
	void requestNearbyTorrents(const vector<DeviceDescriptor>& devs);
	void requestNearbyTorrents(const DeviceDescriptor& remote);
	bool requestAllTorrentAvail(const string& name);
	bool requestTorrentAvail(const string& name);
	bool requestTorrentAvail(const string& name, const vector<DeviceDescriptor>& devs);
	bool requestTorrentAvail(const string& name, const DeviceDescriptor& dev);
	bool requestTorrentFile(const string& name);
	bool requestTorrentFileIfMissing(const string& name);
	int requestChunk(const string& torrent);
	int requestChunk(const string& name, int index);
	int requestChunk(const string& name, int index, const DeviceDescriptor& dev);
	int getMissingChunkIndex(const string& torrent);
	shared_ptr<ChunkReq> createChunkRequest(const string& name, int index, 
		const DeviceDescriptor& remote, const DeviceDescriptor& local);

	bool createNearbyTorrentsRequest(TorrentListReq& req, const DeviceDescriptor& remote);
	void createNearbyTorrentsRequest(TorrentListReq& req, const DeviceDescriptor& remote, const DeviceDescriptor& local);
	bool createTorrentAvailRequest(TorrentAvailReq& req, const string& name, const DeviceDescriptor& dev);
	bool createTorrentFileRequest(TorrentFileReq& req, const string& name);
	bool createTorrentFileRequest(TorrentFileReq& req, const string& name, const string& addr);
	bool createTorrentFileRequest(TorrentFileReq& req, const string& name, const DeviceDescriptor& dev);

	// server/client control
	void activateServerThreads();
	void activateJobManager();
	void pauseServerThreads();
	void pauseJobManager();
	void killServers();
	void killJobManager();

	void getTorrentList(vector<Torrent>& torrentList);

};

class ArgParser
{
public:
	ArgParser()
	{
	}

	ArgParser(const string& cmd, vector<string>& args)
	{
		parse(cmd, args);
	}

	void parse(const string& cmd, vector<string>& args)
	{
		Utils::tokenize(cmd, " ", args);
		args.erase(remove(args.begin(), args.end(), ""), args.end());
		args.erase(remove(args.begin(), args.end(), "\n"), args.end());
		this->cmd  = cmd;
		this->args = args;
	}

	string cmd;
	vector <string> args;
};
