#include <atomic>
#include <memory>
#include <mutex>
#include <list>
#include <condition_variable>

using namespace std;

class Node
{
private:
	inline static const string cli = "bp2p> ";

	enum DevStatus
	{
		READY,
		BUSY,
		INACTIVE
	};
	
	map<DeviceDescriptor, vector<DeviceDescriptor>> local2remote;
	map<DeviceDescriptor, DevStatus> remoteStatus;
	

public:
	inline static const string createTorCmd = "-ct";
	inline static const string quitCmd = "-q";

	struct WorkerThread
	{
		WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<bool>> kill) 
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
	map<DeviceDescriptor, unique_ptr<WorkerThread>> servers;

	mutex jmMutex;
	condition_variable jmEvent;
	unique_ptr<WorkerThread> jobManager = nullptr;
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

	unique_ptr<WorkerThread> createServerThread(DeviceDescriptor servDev);
	void createJobManager();

	void killWorkerThreads();

	bool createTorrent(const string& name, const vector<string>& files);

private:
	void sendRequestWait4Response(RRPacket& req, Message& rsp, 
		const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes);

	void serverThread(DeviceDescriptor dev, shared_ptr<atomic<bool>>);
	unique_ptr<WorkerThread> createJobManagerThread();
	void jobManagerThread(shared_ptr<atomic<bool>> kill);

	void killServers();
	void killJobManager();

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
		args.erase(std::remove(args.begin(), args.end(), ""), args.end());
		args.erase(std::remove(args.begin(), args.end(), "\n"), args.end());
		this->cmd  = cmd;
		this->args = args;
	}

	string cmd;
	vector <string> args;
};