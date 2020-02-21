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


const string Node::createTorCmd = "-tc";
const string Node::listNearbyTorsCmd = "-lnt";
const string Node::requestTorCmd = "-tr";
const string Node::requestTorDataCmd = "-td";
const string Node::quitCmd = "-q";

const string Node::requestChunkAvailCmd = "-rc";

const string Node::cli = "bp2p> ";

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

	for (auto const& local2remotePair : this->local2remote){
		auto local = local2remotePair.first;
		auto remotes = local2remotePair.second;
		cout << "--Local Dev: " << local.addr << " " << local.devID << " " << local.name << endl;
		for (auto const& remote :  remotes)
			cout << "\t --Remote Devs: " << remote.addr << " " << remote.devID << " " << remote.name << endl;
	}
	for (auto const& remote2localPair : this->remote2local){
		auto remote = remote2localPair.first;
		auto locals = remote2localPair.second;
		cout << "++Remote Dev: " << remote.addr << " " << remote.devID << " " << remote.name << endl;
		for (auto const& local :  locals)
			cout << "\t ++Local Devs: " << local.addr << " " << local.devID << " " << local.name << endl;
	}
}

void Node::carryOutRequest(RRPacket& req)
{
	Message rsp;
	req.createRequest();
	try{
		sendRequestWait4Response(req.getReq(), rsp, req.getLocalAddr(), req.getRemoteAddr());
	}
	catch(...){
		throw;
	}
	req.processResponse(rsp);
}

void Node::sendRequestWait4Response(const Message& req, Message& rsp, 
	const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes)
{
	int error = 0;
	BTDevice client{clientDes};
	try{
		client.connect2Device(serverDes);
		client.sendReqWait4Resp(req, rsp);
	}
	catch(int e){
		// data still might be valid in this case
		// bluetooth sends data in chunks of 1008 bytes
		// if data%1008=0, we will catch this
		if (e != ECONNRESET) 
			error = e;
		cout << "Caught Exception " << e << endl;
	}
	//string strresp{rsp.data.begin(), rsp.data.end()};
	//cout << "CLIENT RSP: " << rsp.data.size() << " " << rsp.size << " " << strresp << endl;
	cout << "CLIENT RSP: " << rsp.data.size() << " " << rsp.size << endl;
	try{
		client.endComm();
	}
	catch(int e){
		if (error == 0)
			error = e;
		cout << "Caught CLOSE Exception " << e << endl;
	}
	if (error > 0)
		throw error;
}

void Node::completeRequest(const RRPacket& packet)
{
	const RRPacket* packetptr = &packet;
	auto torListReq = dynamic_cast<const TorrentListReq*>(packetptr);
	if (torListReq)
		completeRequest(*torListReq);
	auto torFileReq = dynamic_cast<const TorrentFileReq*>(packetptr);
	if (torFileReq)
		completeRequest(*torFileReq);
	auto torAvailReq = dynamic_cast<const TorrentAvailReq*>(packetptr);
	if (torAvailReq)
		completeRequest(*torAvailReq);
	auto chunkReq = dynamic_cast<const ChunkReq*>(packetptr);
	if (chunkReq)
		completeRequest(*chunkReq);
}

void Node::completeRequest(const TorrentListReq& req)
{	
	DeviceDescriptor dev = req.getRemoteAddr();
	vector<string> torrentNames = req.getTorrentList();
	this->dev2tor[dev] = torrentNames;
}

void Node::completeRequest(const TorrentFileReq& req)
{
	string name = req.getTorrentName();
	string serializedTorrent = req.getSerializedTorrent();
	Torrent tor;
	tor.createTorrentFromSerializedObj(serializedTorrent);
	if (tor.open())
		this->name2torrent[name] = tor;
}

void Node::completeRequest(const TorrentAvailReq& req)
{
	DeviceDescriptor dev = req.getRemoteAddr();
	string name = req.getTorrentName();
	vector<int> torAvail = req.getTorrentAvail();
	for (int chunkAvail : torAvail){
		if (this->torName2Avail[name].find(chunkAvail) == this->torName2Avail[name].end())
			this->torName2Avail[name].insert(pair<int, vector<DeviceDescriptor>>(chunkAvail, vector<DeviceDescriptor>()));
		this->torName2Avail[name][chunkAvail].push_back(dev);
	}
}

void Node::completeRequest(const ChunkReq& req)
{
	string name = req.getTorrentName();
	int index = req.getIndex();
	vector<char> chunk = req.getChunk();
	Torrent tor = this->name2torrent[name];
	if (!tor.torrentDataExists())
		tor.createTorrentDataFile();
	tor.putChunk(chunk, index);
	this->name2torrent[name] = tor;
}

void Node::processRequest(const Message& req, Message& rsp)
{
	unique_ptr<RRPacket> packet = RRFactory::create(req);
	if (packet){
		processRequest(*packet);
		rsp = packet->getRsp();
	}
}

void Node::processRequest(RRPacket& packet)
{
	auto packetptr = &packet;
	auto torListReq = dynamic_cast<TorrentListReq*>(packetptr);
	if (torListReq)
		processRequest(*torListReq);
	auto torFileReq = dynamic_cast<TorrentFileReq*>(packetptr);
	if (torFileReq)
		processRequest(*torFileReq);
	auto torAvailReq = dynamic_cast<TorrentAvailReq*>(packetptr);
	if (torAvailReq)
		processRequest(*torAvailReq);
	auto chunkReq = dynamic_cast<ChunkReq*>(packetptr);
	if (chunkReq)
		processRequest(*chunkReq);
}

void Node::processRequest(TorrentListReq& req)
{
	vector<string> torrentList;
	getTorrentNameList(torrentList);
	req.processRequest(torrentList);
}

void Node::processRequest(TorrentFileReq& req)
{
	string name, serializedTorrent;
	req.extractTorrentName(name);
	getSerializedTorrent(serializedTorrent, name);
	req.processRequest(serializedTorrent);
}

void Node::processRequest(TorrentAvailReq& req)
{
	string name;
	vector<int> torAvail;
	req.extractTorrentName(name);
	getTorrentAvailFromTorrent(torAvail, name);
	req.processRequest(torAvail);
}

void Node::processRequest(ChunkReq& req)
{
	string name;
	int index;
	req.extractNameAndIndex(name, index);
	vector<char> chunk;
	retrieveChunk(chunk, name, index);
	req.processRequest(chunk);
}

void Node::getTorrentNameList(vector<string>& torrentList)
{
	for (const auto& pair : this->name2torrent)
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
	//cout << "Server Dev: " << dev.getDevAddr() << " " << dev.getDevID() << " " << dev.getDevName() << endl;
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
		if (e != EAGAIN)
			cout << "Server Caught Exception " << e << endl;
	}
	try{
		dev.endComm();
	}
	catch(int e){
		cout << "Server Caught CLOSE Exception " << e << endl;
	}
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
		cout << "!!!!Job manager: has items " << this->jobs.size() << endl;
		shared_ptr<RRPacket> req = this->jobs.front();
		if (req){
			try{
				carryOutRequest(*req);
			}
			catch(...){
				retryRequest();
				return;
			}
			completeRequest(*req);

			auto chunkReq = dynamic_cast<ChunkReq*>(req.get());
			if (chunkReq){
				string name = chunkReq->getTorrentName();
				Torrent tor = this->name2torrent[name];
				if (!tor.isComplete())
					requestChunk(name);
				else
					tor.unpackage();
			}
			this->jobs.pop_front();
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
	for(auto const& server : this->servers)
	{
		cout << "Activate Server: " << server.first.addr << " " << server.first.devID << " " << server.first.name << endl;
	    server.second->activate();
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
	for(auto const& server : this->servers)
	{
		cout << "Pause Server: " << server.first.addr << " " << server.first.devID << " " << server.first.name << endl;
	    server.second->pause();
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
	for(auto const& server : this->servers)
	{
		cout << "Kill Server: " << server.first.addr << " " << server.first.devID << " " << server.first.name << endl;
	    server.second->close();
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

int Node::request(RRPacket& req)
{
	try{
		carryOutRequest(req);
	}
	catch(int e){
		cout << "REQUEST " << e << endl;
		return e;
	}
	completeRequest(req);
	return 0;
}

int Node::syncRequest(RRPacket& req)
{
	pauseWorkerThreads();
	int status = request(req);
	activateWorkerThreads();
	return status;
}

void Node::retryRequest()
{
	shared_ptr<RRPacket> req = this->jobs.front();
	retryRequest(req);
	this->jobs.pop_front();	
}

void Node::retryRequest(shared_ptr<RRPacket> req)
{
	auto torListReq = dynamic_pointer_cast<TorrentListReq>(req);
	if (torListReq)
		requestNearbyTorrents(torListReq->getRemoteAddr());
	auto torFileReq = dynamic_pointer_cast<TorrentFileReq>(req);
	if (torFileReq)
		requestTorrentFile(torFileReq->getTorrentName());
	auto torAvailReq = dynamic_pointer_cast<TorrentAvailReq>(req);
	if (torAvailReq)
		requestTorrentAvail(torAvailReq->getTorrentName());
	auto chunkReq = dynamic_pointer_cast<ChunkReq>(req);
	if (chunkReq)
		requestChunk(chunkReq->getTorrentName());		
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
	if (devs.empty())
		syncRequestAllNearbyTorrents();
	else
		syncRequestNearbyTorrents(devs);
	return 0;
}

bool Node::syncRequestAllNearbyTorrents()
{
	vector<DeviceDescriptor> devs;
	for (auto it = this->remote2local.begin(); it != this->remote2local.end(); it++){
		devs.push_back(it->first);
	}
	return syncRequestNearbyTorrents(devs);
}

bool Node::syncRequestNearbyTorrents(const vector<DeviceDescriptor>& devs)
{
	bool status = true;
	for (auto dev : devs){
		syncRequestNearbyTorrents(dev);
	}
	return status;
}

bool Node::syncRequestNearbyTorrents(const DeviceDescriptor& dev)
{
	bool status = true;
	TorrentListReq req;
	if (createNearbyTorrentsRequest(req, dev)){
		status = status && syncRequest(req);
	}
	return status;
}

void Node::requestAllNearbyTorrents()
{
	vector<DeviceDescriptor> devs;
	for (auto it = this->remote2local.begin(); it != this->remote2local.end(); it++){
		devs.push_back(it->first);
	}
	requestNearbyTorrents(devs);
}

void Node::requestNearbyTorrents(const vector<DeviceDescriptor>& devs)
{
	for (auto dev : devs){
		requestNearbyTorrents(dev);
	}
}

void Node::requestNearbyTorrents(const DeviceDescriptor& remote)
{
	TorrentListReq req;
	if (createNearbyTorrentsRequest(req, remote)){
		auto reqPtr = make_shared<TorrentListReq>(req);
		insertJob(reqPtr);
	}
}

bool Node::createNearbyTorrentsRequest(TorrentListReq& req, const DeviceDescriptor& remote)
{
	bool status = false;
	auto keyVal = this->remote2local.find(remote);
	if (keyVal != this->remote2local.end()){
		auto remote = keyVal->first;
		auto locals = keyVal->second;
		int index = Utils::grnd(0, locals.size()-1);
		createNearbyTorrentsRequest(req, remote, locals[index]);
		status = true;
	}
	return status;
}

void Node::createNearbyTorrentsRequest(TorrentListReq& req, const DeviceDescriptor& remote, const DeviceDescriptor& local)
{
	req.create(remote, local);
}

bool Node::syncRequestTorrentFile(const string& name)
{
	TorrentFileReq req;
	if (createTorrentFileRequest(req, name))
		return (syncRequest(req) == 0);
	return false;
}

bool Node::syncRequestTorrentFile(const string& name, const string& addr)
{
	TorrentFileReq req;
	if (createTorrentFileRequest(req, name, addr))
		return (syncRequest(req) == 0);
	return false;
}

bool Node::requestTorrentFile(const string& name)
{
	bool status = false;
	TorrentFileReq req;
	if (createTorrentFileRequest(req, name)){
		auto reqPtr = make_shared<TorrentFileReq>(req);
		insertJob(reqPtr);
		status = true;
	}
	return status;
}

bool Node::createTorrentFileRequest(TorrentFileReq& req, const string& name, const string& addr)
{
	bool status = false;
	DeviceDescriptor dev{addr};
	status = createTorrentFileRequest(req, name, dev);
	return status;
}

bool Node::createTorrentFileRequest(TorrentFileReq& req, const string& name)
{
	bool status = false;
	auto TorNameDevPair = this->torName2dev.find(name);
	if (TorNameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs = TorNameDevPair->second;
		int index = Utils::grnd(0, devs.size()-1);
		DeviceDescriptor dev{devs[index]};	
		status = createTorrentFileRequest(req, name, dev);
	}
	return status;
}

bool Node::createTorrentFileRequest(TorrentFileReq& req, const string& name, const DeviceDescriptor& dev)
{
	int status = false;
	Torrent torrent;
	auto remoteLocalPair = this->remote2local.find(dev);
	if (remoteLocalPair != this->remote2local.end()){
		auto remote = remoteLocalPair->first;
		auto locals = remoteLocalPair->second;
		int index = Utils::grnd(0, locals.size()-1);
		req.create(remote, locals[index], name);
		status = true;
	}
	return status;
}

bool Node::requestTorrentFileIfMissing(const string& name)
{
	bool status = false;
	auto nameTorPair = this->name2torrent.find(name);
	if (nameTorPair == this->name2torrent.end()){
		status = syncRequestTorrentFile(name);
	}
	else{
		status = true;
	}
	return status;
}

int Node::getMissingChunkIndex(const string& name)
{
	int chunk = -1;
	auto name2torrentpair = this->name2torrent.find(name);
	if (name2torrentpair != this->name2torrent.end()){
		Torrent torrent = name2torrentpair->second;
		chunk = torrent.getMissingChunkIndex();
	}
	return chunk;
}

bool Node::requestTorrentData(const string& name)
{
	bool status = false;
	if (this->dev2tor.empty() || this->torName2dev.find(name) == this->torName2dev.end()){
		syncRequestAllNearbyTorrents();
	}
	this->torName2dev.clear();
	Utils::swapKeyVal(this->torName2dev, this->dev2tor);
	if (syncRequestAllTorrentAvail(name)){
		if (requestTorrentFileIfMissing(name))		
			status = requestChunk(name);
		else
			status = false;
	}
	else{
		status = false;
	}
	return status;
}

int Node::requestTorrentAvail(const string& name, const string& addr)
{
	DeviceDescriptor dev{addr};
	return requestTorrentAvail(name, dev);
}

bool Node::syncRequestAllTorrentAvail(const string& name)
{
	bool status = false;
	auto torNameDevPair = this->torName2dev.find(name);
	if (torNameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs = torNameDevPair->second;
		status = syncRequestTorrentAvail(name, devs);
	}
	return status;
}

bool Node::syncRequestTorrentAvail(const string& name, const vector<DeviceDescriptor>& devs)
{
	bool status = true;
	for (const auto& dev : devs){
		status = status && syncRequestTorrentAvail(name, dev);
	}
	return status;
}

bool Node::syncRequestTorrentAvail(const string& name, const DeviceDescriptor& dev)
{
	TorrentAvailReq req;
	if (createTorrentAvailRequest(req, name, dev))
		return (syncRequest(req) == 0);
	return false;
}

bool Node::requestAllTorrentAvail(const string& name)
{
	bool status = false;
	auto torNameDevPair = this->torName2dev.find(name);
	if (torNameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs = torNameDevPair->second;
		status = requestTorrentAvail(name, devs);
	}
	return status;
}

bool Node::requestTorrentAvail(const string& name)
{
	bool status = false;
	auto torNameDevPair = this->torName2dev.find(name);
	if (torNameDevPair != this->torName2dev.end()){
		vector<DeviceDescriptor> devs = torNameDevPair->second;
		int index = Utils::grnd(0, devs.size()-1);
		status = requestTorrentAvail(name, devs[index]);
	}
	return status;
}

bool Node::requestTorrentAvail(const string& name, const vector<DeviceDescriptor>& devs)
{
	bool status = true;
	for (const auto& dev : devs){
		status = status && requestTorrentAvail(name, dev);
	}
	return status;
}

bool Node::requestTorrentAvail(const string& name, const DeviceDescriptor& dev)
{
	bool status = true;
	TorrentAvailReq req;
	if (createTorrentAvailRequest(req, name, dev)){
		auto reqPtr = make_shared<TorrentAvailReq>(req);
		insertJob(reqPtr);
		status = true;
	}
	return status;
}

bool Node::createTorrentAvailRequest(TorrentAvailReq& req, const string& name, const DeviceDescriptor& dev)
{
	bool status = false;
	auto remoteLocalPair = this->remote2local.find(dev);
	if (remoteLocalPair != this->remote2local.end()){
		auto remote = remoteLocalPair->first;
		auto locals = remoteLocalPair->second;
		int index = Utils::grnd(0, locals.size()-1);
		req.create(name, remote, locals[index]);
		status = true;
	}
	return status;
}

int Node::requestChunk(const string& name)
{
	int index = getMissingChunkIndex(name);
	return requestChunk(name, index);
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
	torrentList = Torrent::getAllTorrents();
}

void Node::initDB()
{
	TorrentDB::init();
}

void Node::init()
{
	cout << "hello!" << endl;
	cout << "initializing database connection..." << endl;
	initDB();
	cout << "reading existing torrents..." << endl;
	populateLocalTorrents();
	cout << "scanning for devices..." << endl;
	findLocalDevs();
	scanForDevs();
	cout << "creating server(s)..." << endl;
	createServers();	
	createJobManager();
}

int main()
{
	Node myNode;
	myNode.init();

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
					myNode.syncRequestTorrentFile(name, addr);
				}
				else{
					cout << "Usage: ./bp2p.exe -tr [name] [addr]" << endl;
				}
			}
			else if (args[0].compare(Node::requestTorDataCmd) == 0){
				string name;
				if (args.size() > 1){
					name = args[1];
					myNode.requestTorrentData(name);
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

    return 0;
}
