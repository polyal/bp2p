#include <iostream>
#include <fstream>
#include <utility>
#include <unordered_set>
#include "btdevice.h"
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "torrentAvailReq.h"
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
			if (this->remote2local.find(remote) == this->remote2local.end()){
				vector<DeviceDescriptor> locals{local};
				this->remote2local[remote] = locals;
			}
			else{
				this->remote2local[remote].push_back(local);
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

void Node::carryOutRequest(RRPacket& req)
{
	carryOutRequest(&req);
}

void Node::carryOutRequest(RRPacket* const req)
{
	if (req){
		Message rsp;
		req->createRequest();
		sendRequestWait4Response(req->getReq(), rsp, req->getLocalAddr(), req->getRemoteAddr());
		processResponse(req, rsp);
	}
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
	string strresp{rsp.data.begin(), rsp.data.end()};
	cout << "CLIENT RSP: " << rsp.data.size() << " " << rsp.size << " " << strresp << endl;
	try{
		client.endComm();
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}
}

void Node::processResponse(RRPacket* packet, const Message& rsp)
{
	packet->processResponse(rsp);
	auto torListReq = dynamic_cast<TorrentListReq*>(packet);
	processResponse(torListReq);
	auto torFileReq = dynamic_cast<TorrentFileReq*>(packet);
	processResponse(torFileReq);
	auto torAvailReq = dynamic_cast<TorrentAvailReq*>(packet);
	processResponse(torAvailReq);
	auto chunkReq = dynamic_cast<ChunkReq*>(packet);
	processResponse(chunkReq);
}

void Node::processResponse(const TorrentListReq* const req)
{	
	if (req){
		DeviceDescriptor dev = req->getRemoteAddr();
		vector<string> torrentNames = req->getTorrentList();
		this->dev2tor[dev] = torrentNames;
	}
}

void Node::processResponse(const TorrentFileReq* const req)
{
	if (req){
		string name = req->getTorrentName();
		string serializedTorrent = req->getSerializedTorrent();
		Torrent tor;
		tor.createTorrentFromSerializedObj(serializedTorrent);
		if (tor.open())
			this->name2torrent[name] = tor;
	}
}

void Node::processResponse(const TorrentAvailReq* const req)
{
	if (req){
		DeviceDescriptor dev = req->getRemoteAddr();
		string name = req->getTorrentName();
		vector<int> torAvail = req->getTorrentAvail();
		for (int chunkAvail : torAvail){
			if (this->torName2Avail[name].find(chunkAvail) == this->torName2Avail[name].end())
				this->torName2Avail[name].insert(pair<int, vector<DeviceDescriptor>>(chunkAvail, vector<DeviceDescriptor>()));
			this->torName2Avail[name][chunkAvail].push_back(dev);
		}
	}
}

void Node::processResponse(const ChunkReq* const req)
{
	if (req){
		string name = req->getTorrentName();
		int index = req->getIndex();
		vector<char> chunk = req->getChunk();
		Torrent tor = this->name2torrent[name];
		if (tor.open()){
			if (!tor.torrentDataExists())
				tor.createTorrentDataFile();
			tor.putChunk(chunk, index);
		}
	}
}

void Node::processRequest(const Message& req, Message& rsp)
{
	unique_ptr<RRPacket> packet = RRFactory::create(req);
	if (packet){
		processRequest(packet.get());
		rsp = packet->getRsp();
	}
}

void Node::processRequest(RRPacket* packet)
{
	auto torListReq = dynamic_cast<TorrentListReq*>(packet);
	if (torListReq){
		vector<string> torrentList;
		getTorrentNameList(torrentList);
		torListReq->processRequest(torrentList);
		return;
	}
	auto torFileReq = dynamic_cast<TorrentFileReq*>(packet);
	if (torFileReq){
		string name, serializedTorrent;
		torFileReq->extractTorrentName(name);
		getSerializedTorrent(serializedTorrent, name);
		torFileReq->processRequest(serializedTorrent);
		return;
	}
	auto torAvailReq = dynamic_cast<TorrentAvailReq*>(packet);
	if (torAvailReq){
		string name;
		vector<int> torAvail;
		torAvailReq->extractTorrentName(name);
		getTorrentAvailFromTorrent(torAvail, name);
		torAvailReq->processRequest(torAvail);
		return;
	}
	auto chunkReq = dynamic_cast<ChunkReq*>(packet);
	if (chunkReq){
		string name;
		int index;
		chunkReq->extractNameAndIndex(name, index);
		vector<char> chunk;
		retrieveChunk(chunk, name, index);
		chunkReq->processRequest(chunk);
		return;
	}
}

void Node::getTorrentNameList(vector<string>& torrentList)
{
	for (const auto& pair :  this->name2torrent)
		torrentList.push_back(pair.first);
}

void Node::getSerializedTorrent(string& serializedTorrent, const string& name)
{
	serializedTorrent.clear();
	auto nameTorPair = this->name2torrent.find(name);
	if (nameTorPair != this->name2torrent.end()){
		Torrent tor = nameTorPair->second;
		if (tor.open())
			serializedTorrent = tor.getSerializedTorrent();
	}
}

void Node::getTorrentAvailFromTorrent(vector<int>& torrentAvail, const string& name)
{
	torrentAvail.clear();
	auto nameTorPair = this->name2torrent.find(name);
	if (nameTorPair != this->name2torrent.end()){
		Torrent tor = nameTorPair->second;
		if (tor.open())
			torrentAvail = tor.getChunkAvail();
	}
}

void Node::retrieveChunk(vector<char>& chunk, const string& name, const int& index)
{
	auto nameTorPair = this->name2torrent.find(name);
	if (nameTorPair != this->name2torrent.end()){
		Torrent tor = nameTorPair->second;
		if (tor.open())
			chunk = tor.getChunk(index);
	}
}

void Node::createServers()
{
	for (auto const& dev : this->localDevs){
		this->servers[dev] = createServerThread(dev);	
	}
}

unique_ptr<WorkerThread> Node::createServerThread(const DeviceDescriptor& servDev)
{
	return make_unique<WorkerThread>(
		[this, &servDev]
		{
			serverThread(servDev);
		});
}

void Node::serverThread(DeviceDescriptor devDes)
{
	Message req;
	Message rsp;
	DeviceDescriptor client;

	BTDevice dev{devDes};
	cout << "Server Dev: " << dev.getDevAddr() << " " << dev.getDevID() << " " << dev.getDevName() << endl;

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
}

void Node::createJobManager()
{
	this->jobManager = createJobManagerThread();
}

unique_ptr<WorkerThread> Node::createJobManagerThread()
{
	return make_unique<WorkerThread>(
		[this]
		{
			jobManagerThread();
		},
		[this]
		{
			return this->jobs.empty();
		});
}

void Node::jobManagerThread()
{
	if (this->jobs.size() > 0){
		cout << "Job manager: has items " << this->jobs.size() << endl;
		shared_ptr<RRPacket> req = jobs.front();
		auto chunkReq = dynamic_pointer_cast<ChunkReq>(req);
		if (chunkReq){
			cout << "Job Manager: not NULL" << endl;
			carryOutRequest(*chunkReq);
			// mark chunk as received
			jobs.pop_front();
		}
	}
	this_thread::sleep_for (std::chrono::milliseconds(20));
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

void Node::insertJob(const shared_ptr<RRPacket> job)
{
	if (this->jobManager){
		this->jobManager->modify(
			[this, job]
			{
				this->jobs.push_back(job);
			});
	}
}

bool Node::createTorrent(const string& name, const vector<string>& files)
{
	Torrent t{name, files};
	if (t.create()){
		this->name2torrent[name] = t;
		return true;
	}
	return false;
}

int Node::listNearbyTorrents(const vector<string>& addrs)
{
	auto addrSet = Utils::filterDuplicates<string>(addrs);
	vector<DeviceDescriptor> devs;
	for (auto const& addr : addrSet){
		DeviceDescriptor dev{addr};
		devs.push_back(dev);
	}
	if (devs.empty()){
		requestAllNearbyTorrents();
	}
	else{
		requestNearbyTorrents(devs);
	}
	return 0;
}

void Node::requestAllNearbyTorrents()
{
	map<DeviceDescriptor, vector<string>> nearbyTorrents;
	pauseWorkerThreads();
	for(auto const& [key, val] : this->remote2local)
	{
		int index = Utils::grnd(0, val.size()-1);
		TorrentListReq req{key, val[index]};
		carryOutRequest(req);
		nearbyTorrents[key] = req.getTorrentList();
	}
	activateWorkerThreads();
	this->dev2tor = nearbyTorrents;
}

void Node::requestNearbyTorrents(const vector<DeviceDescriptor>& devs)
{
	map<DeviceDescriptor, vector<string>> nearbyTorrents;
	pauseWorkerThreads();
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
	activateWorkerThreads();
	for (const auto& [dev, tors] : nearbyTorrents){
		this->dev2tor[dev] = tors;
	}
}

int Node::requestTorrentFile(const string& name, const string& addr)
{
	int status = 0;
	DeviceDescriptor dev{addr};
	status = requestTorrentFile(name, dev);
	return status;
}

int Node::requestTorrentFile(const string& name, const DeviceDescriptor& dev)
{
	int status = -1;
	Torrent torrent;
	pauseWorkerThreads();
	auto remoteLocalPair = this->remote2local.find(dev);
	if (remoteLocalPair != this->remote2local.end()){
		auto remote = remoteLocalPair->first;
		auto locals = remoteLocalPair->second;
		int index = Utils::grnd(0, locals.size()-1);
		TorrentFileReq req{remote, locals[index], name};
		carryOutRequest(req);
	}
	activateWorkerThreads();
	return status;
}

int Node::requestTorrentFileIfMissing(const string& name, Torrent& torrent)
{
	int status = -1;
	auto TorNameDevPair = this->torName2dev.find(name);
	if (TorNameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs =  TorNameDevPair->second;
		auto nameTorPair = this->name2torrent.find(name);
		if (nameTorPair == this->name2torrent.end()){
			int index = Utils::grnd(0, devs.size()-1);
			DeviceDescriptor dev{devs[index]};	
			status = requestTorrentFile(name, dev);
			if (status == 0)
				torrent = this->name2torrent[name];
		}
		else{
			torrent = this->name2torrent[name];
		}
	}
	return status;
}

int Node::getMissingChunkIndex(const Torrent& torrent)
{
	return torrent.getMissingChunkIndex();	
}

int Node::requestTorrentData(const string& name)
{
	int status = 0;
	if (this->dev2tor.empty() || this->torName2dev.find(name) == this->torName2dev.end()){
		requestAllNearbyTorrents();
	}
	this->torName2dev.clear();
	Utils::swapKeyVal(this->torName2dev, this->dev2tor);
	auto TorNameDevPair = this->torName2dev.find(name);
	if (TorNameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs =  TorNameDevPair->second;
		vector<int> avail;
		for (const auto&  dev : devs){
			requestTorrentAvail(name, dev, avail);
			for (int chunkAvail : avail){
				if (this->torName2Avail[name].find(chunkAvail) == this->torName2Avail[name].end())
					this->torName2Avail[name].insert(pair<int, vector<DeviceDescriptor>>(chunkAvail, vector<DeviceDescriptor>()));
				torName2Avail[name][chunkAvail].push_back(dev);
			}
			avail.clear();
		}
		Torrent torrent;
		status = requestTorrentFileIfMissing(name, torrent);		
		status = requestChunk(torrent);
	}
	else{
		status = -1;
	}
	return status;
}

int Node::requestTorrentAvail(const string& name, const string& addr)
{
	DeviceDescriptor dev{addr};
	vector<int> avail;
	return requestTorrentAvail(name, dev, avail);
}

int Node::requestTorrentAvail(const string& name, const DeviceDescriptor& dev, vector<int>& avail)
{
	
	pauseWorkerThreads();
	auto remoteLocalPair = this->remote2local.find(dev);
	if (remoteLocalPair != this->remote2local.end()){
		auto remote = remoteLocalPair->first;
		auto locals = remoteLocalPair->second;
		int index = Utils::grnd(0, locals.size()-1);
		TorrentAvailReq req{remote, locals[index], name};
		carryOutRequest(req);
		avail = req.getTorrentAvail();
	}
	activateWorkerThreads();
	return 0;
}

int Node::requestChunk(const Torrent& torrent)
{
	int missingChunkIndex = getMissingChunkIndex(torrent);
	string name = torrent.getFilename();
	return requestChunk(name, missingChunkIndex);
}

int Node::requestChunk(const string& name, int index)
{
	int status = 0;
	auto nameDevPair = this->torName2dev.find(name);
	if (nameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs = nameDevPair->second;
		int i = Utils::grnd(0, devs.size()-1);
		DeviceDescriptor dev = devs[i];
		status = requestChunk(name, index, dev);
	}
	else{
		status = -1;
	}
	return status;
}

int Node::requestChunk(const string& name, int index, const DeviceDescriptor& dev)
{
	int status = -1;
	auto remoteLocalPair = this->remote2local.find(dev);
	if (remoteLocalPair != this->remote2local.end()){
		auto remote = remoteLocalPair->first;
		auto locals = remoteLocalPair->second;
		int i = Utils::grnd(0, locals.size()-1);
		auto req = createChunkRequest(name, index, remote, locals[i]);
		insertJob(req);
		status = 0;
	}
	return status;
}

shared_ptr<ChunkReq> Node::createChunkRequest(const string& name, int index, 
	const DeviceDescriptor& remote, const DeviceDescriptor& local)
{
	return make_shared<ChunkReq>(remote, local, name, index);
}

void Node::populateLocalTorrents()
{
	vector<Torrent> torList;
	getTorrentList(torList);
	this->name2torrent.clear();
	for (const auto& tor : torList)
		this->name2torrent[tor.getFilename()] = tor;
}

void Node::getTorrentList(vector<Torrent>& torrentList)
{
	vector<string> torrentFiles;
	torrentFiles = Torrent::getTorrentNames();

	for(auto const& filename: torrentFiles) {
		Torrent tor {filename};
		if (tor.open())
			torrentList.push_back(filename);
	}
}

int main(int argc, char *argv[]){
	cout << "hello!" << endl;
	cout << "scanning for devices..." << endl;
	Node myNode;
	myNode.findLocalDevs();
	myNode.scanForDevs();
	cout << "done. " << endl << endl;

	myNode.populateLocalTorrents();
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
			if (args[0].compare(Node::createTorCmd) == 0 && args.size() > 3){
				string filename = args[1];
				vector<string> files{args.begin() + 2, args.end()};
				if (!myNode.createTorrent(filename, files))
					cout << "Create Torrent Failed" << endl;
			}
			else if (args[0].compare(Node::listNearbyTorsCmd) == 0){
				vector<string> addr;
				if (args.size() > 1)
					copy(args.begin()+1, args.end(), back_inserter(addr));
				myNode.listNearbyTorrents(addr);
			}
			else if (args[0].compare(Node::requestTorCmd) == 0){
				string name, addr;
				if (args.size() > 2){
					name = args[1];
					addr = args[2];
					myNode.requestTorrentFile(name, addr);
				}
				else{
					cout << "Usage: ./bp2p.exe -tr [name] [addr]" << endl;
				}
			}
			else if (args[0].compare(Node::quitCmd) == 0)
				break;
			else if (args[0].compare(Node::requestChunkAvailCmd) == 0)
				myNode.requestTorrentAvail(args[1], args[2]);
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