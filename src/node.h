#include <atomic>
#include <memory>
#include <mutex>
#include <list>
#include <condition_variable>
#include "eventSync.h"

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
	map<DeviceDescriptor, vector<DeviceDescriptor>> remote2local;
	map<DeviceDescriptor, DevStatus> remoteStatus;
	

public:
	inline static const string createTorCmd = "-ct";    // create torrent
	inline static const string listNearbyTorsCmd = "-lnt"; // list nearby torrents
	inline static const string quitCmd = "-q";

	struct WorkerThread
	{
		enum Status
		{
			ACTIVE,
			PAUSE,
			KILL
		};

		WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status) 
		{
			this->t = move(t);
			this->status = status;
		}

		WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status, shared_ptr<SyncEvent> event) 
		{
			this->t = move(t);
			this->status = status;
			this->event = event;
		}

		void activate()
		{
			if (this->event){
				std::unique_lock<std::mutex> lock(this->event->m);
				setStatus(ACTIVE);
				lock.unlock();
				this->event->cv.notify_one();
			}
			else
				setStatus(ACTIVE);
		}

		void pause()
		{
			if (this->event){
				std::unique_lock<std::mutex> lock(this->event->m);
				setStatus(PAUSE);
				lock.unlock();
				this->event->cv.notify_one();
			}
			else
				setStatus(PAUSE);
		}

		void kill()
		{
			if (this->event){
				std::unique_lock<std::mutex> lock(this->event->m);
				setStatus(KILL);
				lock.unlock();
				this->event->cv.notify_one();
			}
			else
				setStatus(KILL);
		}

		void setStatus(Status status)
		{
			*this->status = status;
		}

		void close()
		{
			kill();
			if (this->t) this->t->join();
		}

		unique_ptr<thread> t = nullptr;
		shared_ptr<atomic<Status>> status = nullptr;
		shared_ptr<SyncEvent> event = nullptr;
	};


	vector<DeviceDescriptor> localDevs;
	map<DeviceDescriptor, unique_ptr<WorkerThread>> servers;

	unique_ptr<WorkerThread> jobManager = nullptr;
	list<shared_ptr<RRPacket>> jobs;

	Node();

	static void printcli();

	void findLocalDevs();
	void scanForDevs();

	/*void requestTorrentList(const DeviceDescriptor& client, const DeviceDescriptor& server, Message& rsp);
	void requestTorrentFile(const DeviceDescriptor& client, const DeviceDescriptor& server, 
		const string& torrentName, Message& rsp);
	void requestChunk(const DeviceDescriptor& client, const DeviceDescriptor& server, 
		const string& torrentName, const int chunkNum, Message& rsp);*/

	unique_ptr<WorkerThread> createServerThread(DeviceDescriptor servDev);
	void createJobManager();

	void activateWorkerThreads();
	void pauseWorkerThreads();
	void killWorkerThreads();

	bool createTorrent(const string& name, const vector<string>& files);
	int listNearbyTorrents(const vector<string>& addrs);

private:
	void carryOutRequest(RRPacket& req);
	void sendRequestWait4Response(const Message& req, Message& rsp, 
		const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes);
	void processRequest(const Message& req, Message& rsp);

	void serverThread(DeviceDescriptor devDes, 
		shared_ptr<atomic<Node::WorkerThread::Status>> status, shared_ptr<SyncEvent> event);
	unique_ptr<WorkerThread> createJobManagerThread();
	void jobManagerThread(shared_ptr<atomic<WorkerThread::Status>> status, shared_ptr<SyncEvent> event);

	void activateServerThreads();
	void activateJobManager();
	void pauseServerThreads();
	void pauseJobManager();
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