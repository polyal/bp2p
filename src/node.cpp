#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <utility>
#include <unordered_set>
#include "utils.h"
#include "btdevice.h"
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "chunkReq.h"
#include "rrfactory.h"
#include "node.h"

using namespace std;


Node::Node(){

}

void Node::printcli()
{
	cout << Node::cli;
}

void Node::findLocalDevs()
{
	vector<DeviceDescriptor> devs;
	BTDevice::findLocalDevs(devs);
	this->localDevs.assign(devs.begin(), devs.end());

	for (auto dev : this->localDevs){
		cout << "Local Devs: " << dev.addr << " " << dev.devID << " " << dev.name << endl;
	}
}

void Node::scanForDevs()
{
	this->local2remote.clear();
	this->remote2local.clear();
	this->remoteStatus.clear();
	vector<DeviceDescriptor> remoteDevs;
	for (auto const& local : this->localDevs){
		BTDevice dev{local};
		dev.findNearbyDevs(remoteDevs);
		this->local2remote[local] = remoteDevs;
		// remove local devs from scan
		for (auto const& locDev : this->localDevs){
			if (find(remoteDevs.begin(), remoteDevs.end(), locDev) != remoteDevs.end())
		   		remoteDevs.erase(remove(remoteDevs.begin(), remoteDevs.end(), locDev), remoteDevs.end());
		}
		for (auto const& remote : remoteDevs){
			if (remote2local.find(remote) == remote2local.end()){
				vector<DeviceDescriptor> locals{local};
				remote2local[remote] = locals;
			}
			else{
				remote2local[remote].push_back(local);
			}
			this->remoteStatus[remote] = READY;
		}
		remoteDevs.clear();
	}

	for (auto const& [local, remotes] : this->local2remote){
		cout << "--Local Dev: " << local.addr << " " << local.devID << " " << local.name << endl;
		for (auto const& remote :  remotes)
			cout << "\t --Remote Devs: " << remote.addr << " " << remote.devID << " " << remote.name << endl;
	}
	for (auto const& [remote, locals] : this->remote2local){
		cout << "++Remote Dev: " << remote.addr << " " << remote.devID << " " << remote.name << endl;
		for (auto const& local :  locals)
			cout << "\t ++Local Devs: " << local.addr << " " << local.devID << " " << local.name << endl;
	}
}

/*void Node::requestTorrentList(const DeviceDescriptor& client, const DeviceDescriptor& server, Message& rsp)
{
	TorrentListReq req;
	req.createRequest();
	sendRequestWait4Response(req, rsp, client, server);
	req.processResponse(rsp);
}

void Node::requestTorrentFile(const DeviceDescriptor& client, const DeviceDescriptor& server, 
	const string& torrentName, Message& rsp)
{
	TorrentFileReq req{torrentName};
	req.createRequest();
	sendRequestWait4Response(req, rsp, client, server);
	req.processResponse(rsp);
}

void Node::requestChunk(const DeviceDescriptor& client, const DeviceDescriptor& server, 
	const string& torrentName, const int chunkNum, Message& rsp)
{
	ChunkReq req{torrentName, chunkNum};
	req.createRequest();
	sendRequestWait4Response(req, rsp, client, server);
	req.processResponse(rsp);
}*/

void Node::carryOutRequest(RRPacket& req)
{
	Message rsp;
	req.createRequest();
	sendRequestWait4Response(req.getReq(), rsp, req.getLocalAddr(), req.getRemoteAddr());
	req.processResponse(rsp);
}

void Node::sendRequestWait4Response(const Message& req, Message& rsp, 
	const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes)
{
	BTDevice client{clientDes};
	try{
		client.connect2Device(serverDes);
		client.sendReqWait4Resp(req, rsp);
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}
	//string strresp{rsp.data.begin(), rsp.data.end()};
	//cout << "CLIENT RSP: " << rsp.data.size() << " " << rsp.size << endl;
	try{
		client.endComm();
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}
}

void Node::processRequest(const Message& req, Message& rsp)
{
	unique_ptr<RRPacket> packet = RRFactory::create(req);

	if (packet){
		packet->processRequest();
		rsp = packet->getRsp();
	}
}

void Node::createServers()
{
	for (auto const& dev : this->localDevs){
		this->servers[dev] = createServerThread(dev);	
	}
}

unique_ptr<Node::WorkerThread> Node::createServerThread(const DeviceDescriptor& servDev)
{
	auto status = make_shared<atomic<WorkerThread::Status>>(WorkerThread::Status::ACTIVE);
	auto event = make_shared<SyncEvent>();
	auto tServer = make_unique<thread>(
		[this, servDev, status, event]
		{
			serverThread(servDev, status, event);
		});
	return make_unique<WorkerThread>(move(tServer), status, event);
}


void Node::serverThread(DeviceDescriptor devDes, 
	shared_ptr<atomic<Node::WorkerThread::Status>> status, shared_ptr<SyncEvent> event)
{
	Message req;
	Message rsp;
	DeviceDescriptor client;

	BTDevice dev{devDes};
	cout << "Server Dev: " << dev.getDevAddr() << " " << dev.getDevID() << " " << dev.getDevName() << endl;

	do{
		unique_lock<std::mutex> lock(event->m);
		event->cv.wait(lock, 
			[status]
			{
				return *status != Node::WorkerThread::PAUSE;
			});
		lock.unlock();
		try{
			dev.initServer();
			dev.listen4Req(client);
			dev.fetchRequestData(req);
			string strreq{req.data.begin(), req.data.end()};
			cout << "SERVER --Request: " << strreq << endl;
			processRequest(req, rsp);
			//string strrsp{rsp.data.begin(), rsp.data.end()};
			//cout << "Server Rsp " << strrsp << " " << rsp.size << endl;
			dev.sendResponse(rsp);
		}
		catch(int e){
			cout << "Caught Exception " << e << endl;
		}
		try{
			dev.endComm();
		}
		catch(int e){
			cout << "Caught Exception " << e << endl;
		}

		req.clear();
		rsp.clear();
	} while (*status != Node::WorkerThread::Status::KILL);
}

void Node::createJobManager()
{
	this->jobManager = createJobManagerThread();
}

unique_ptr<Node::WorkerThread> Node::createJobManagerThread()
{
	auto status = make_shared<atomic<WorkerThread::Status>>(WorkerThread::ACTIVE);
	auto event = make_shared<SyncEvent>();
	auto tJobManager = make_unique<thread>(
		[this, status, event]
		{
			jobManagerThread(status, event);
		});
	return make_unique<WorkerThread>(move(tJobManager), status, event);
}

void Node::jobManagerThread(shared_ptr<atomic<Node::WorkerThread::Status>> status, shared_ptr<SyncEvent> event)
{
	do{
		unique_lock<std::mutex> lock(event->m);
		event->cv.wait(lock, 
			[this, status]
			{
				return (!this->jobs.empty() && *status != Node::WorkerThread::PAUSE) || *status == Node::WorkerThread::KILL;
			});
		if (this->jobs.size() > 0){
			cout << "Job manager: has items " << this->jobs.size() << endl;
			shared_ptr<RRPacket> req = jobs.front();
			if (req){
				cout << "Job Manager: not NULL" << endl;
				carryOutRequest(*req);
				jobs.pop_front();
			}
		}
		lock.unlock();
		this_thread::sleep_for (std::chrono::milliseconds(20));
	} while(*status != Node::WorkerThread::Status::KILL);
}

void Node::activateWorkerThreads()
{
	activateServerThreads();
	activateJobManager();
}

void Node::activateServerThreads()
{
	for(auto const& [key, val] : this->servers)
	{
		cout << "Activate Server: " << key.addr << " " << key.devID << " " << key.name << endl;
	    val->activate();
	}
}

void Node::activateJobManager()
{
	if (this->jobManager)
		this->jobManager->activate();
}

void Node::pauseWorkerThreads()
{
	pauseServerThreads();
	pauseJobManager();
	this_thread::sleep_for (std::chrono::seconds(BTChannel::getTimeout())); 
}

void Node::pauseServerThreads()
{
	for(auto const& [key, val] : this->servers)
	{
		cout << "Pause Server: " << key.addr << " " << key.devID << " " << key.name << endl;
	    val->pause();
	}
}

void Node::pauseJobManager()
{
	if (this->jobManager)
		this->jobManager->pause();
}


void Node::killWorkerThreads()
{
	killServers();
	killJobManager();

}

void Node::killServers()
{
	for(auto const& [key, val] : this->servers)
	{
		cout << "Kill Server: " << key.addr << " " << key.devID << " " << key.name << endl;
	    val->close();
	}
}

void Node::killJobManager()
{
	if (this->jobManager)
		this->jobManager->close();
}

bool Node::createTorrent(const string& name, const vector<string>& files)
{
	Torrent t{name, files};
	return t.create();
}

int Node::listNearbyTorrents(const vector<string>& addrs)
{
	unordered_set<DeviceDescriptor> devs;
	for (auto addr : addrs){
		DeviceDescriptor dev{addr};
		devs.insert(dev);
	}
	pauseWorkerThreads();
	map<DeviceDescriptor, vector<string>> nearbyTorrents;
	if (devs.empty()){
		for(auto const& [key, val] : this->remote2local)
		{
			int index = Utils::grnd(0, val.size()-1);
			TorrentListReq req{key, val[index]};
			carryOutRequest(req);
			nearbyTorrents[key] = req.getTorrentList();
		}
	}
	else{
		for (auto dev : devs){
			auto keyVal = this->remote2local.find(dev);
			if (keyVal != this->remote2local.end()){
				auto remote = keyVal->first;
				auto locals = keyVal->second;
				int index = Utils::grnd(0, locals.size()-1);
				TorrentListReq req{remote, locals[index]};
				carryOutRequest(req);
				nearbyTorrents[remote] = req.getTorrentList();
			}
		}
	}
	activateWorkerThreads();
	for (auto const&  [key, val] : nearbyTorrents){
		cout << key.addr << "\n\t";
		for (auto const& tor : val)
			cout << tor << " ";
		cout << endl;
	}
	return 0;
}

int main(int argc, char *argv[]){
	cout << "hello!" << endl;
	cout << "scanning for devices..." << endl;
	Node myNode;
	myNode.findLocalDevs();
	myNode.scanForDevs();
	cout << "done. " << endl << endl;

	// create server
	myNode.createServers();	
	myNode.createJobManager();

	string in;
	vector<string> args;
	do
	{
		Node::printcli();
		
		getline(cin, in);
		ArgParser argparser{in, args};

		if (!args.empty()){
			if (args[0].compare(Node::createTorCmd) == 0 && args.size() > 2){
				string filename = args[0];
				vector<string> files{args.begin() + 1, args.end()};
				if (!myNode.createTorrent(filename, files))
					cout << "Create Torrent Failed" << endl;
			}
			else if (args[0].compare(Node::listNearbyTorsCmd) == 0){
				vector<string> addr;
				if (args.size() > 1)
					copy(args.begin()+1, args.end(), back_inserter(addr));
				myNode.listNearbyTorrents(addr);
			}
			else if (args[0].compare(Node::quitCmd) == 0)
				break;
			else if(args[0].compare("-p") == 0){
				myNode.pauseWorkerThreads();
			}
			else if(args[0].compare("-a") == 0){
				myNode.activateWorkerThreads();
			}
			args.clear();
		}
	} while (1);

	myNode.killWorkerThreads();

	/*unique_ptr<Node::Peer> server = myNode.createServerThread(myNode.localDevs[0]);
	this_thread::sleep_for (std::chrono::milliseconds(10));
	unique_ptr<Node::Peer> jobMan = myNode.createJobManagerThread();
	{
		std::unique_lock<std::mutex> lock(myNode.jmMutex);
		for (int i = 0; i < 2; i++){
			//Message rsp;
			//this_thread::sleep_for (std::chrono::milliseconds(10));
			//myNode.jobManagerMutex.lock();
			auto chunk = make_shared<ChunkReq>("largerNew", i);
			myNode.jobs.push_back(chunk);
			//myNode.jobManagerMutex.unlock();
			//myNode.requestChunk(myNode.localDevs[1], myNode.localDevs[0], "largerNew", i, rsp);

			cout << "LOOP " << i << endl;
		}
		lock.unlock();
		myNode.jmEvent.notify_one();
	}

	this_thread::sleep_for (std::chrono::seconds(10));
	server->close();
	// notify again just to wake up the thread so it quits
	{
		std::unique_lock<std::mutex> lock(myNode.jmMutex);
		jobMan->setKill();
		lock.unlock();
		myNode.jmEvent.notify_one();
	}
	jobMan->close();*/

    return 0;
}