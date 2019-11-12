#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <utility>
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
	this->remoteStatus.clear();
	vector<DeviceDescriptor> remoteDevs;
	for (auto devDes : this->localDevs){
		BTDevice dev{devDes};
		dev.findNearbyDevs(remoteDevs);
		this->local2remote[devDes] = remoteDevs;
		for (auto remote : remoteDevs){
			this->remoteStatus[remote] = READY;
		}
		remoteDevs.clear();
	}

	// TODO: remove local devs from nearby devs
	for (auto it = this->local2remote.begin(); it != this->local2remote.end(); it++){
		cout << "Local Dev: " << it->first.addr << " " << it->first.devID << " " << it->first.name << endl;
		for (auto remote :  it->second)
			cout << "\t Remote Devs: " << remote.addr << " " << remote.devID << " " << remote.name << endl;
	}
}

void Node::requestTorrent(const string& torrentName)
{

}

void Node::requestTorrentList(const DeviceDescriptor& client, const DeviceDescriptor& server, Message& rsp)
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
}

void Node::sendRequestWait4Response(RRPacket& req, Message& rsp, 
	const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes)
{
	BTDevice client{clientDes};
	Message msg{req.getReq()};

	try{
		client.connect2Device(serverDes);
		client.sendReqWait4Resp(msg, rsp);
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

unique_ptr<Node::Server> Node::createServerThread(DeviceDescriptor servDev)
{
	auto active = make_shared<atomic<bool>>(true);
	auto tServer = make_unique<thread>(Node::server, servDev, active);
	return make_unique<Server>(move(tServer), active);
}


void Node::server(DeviceDescriptor devDes, shared_ptr<atomic<bool>> active)
{
	Message req;
	Message rsp;
	DeviceDescriptor client;

	BTDevice dev{devDes};
	cout << "Server Dev: " << dev.getDevAddr() << " " << dev.getDevID() << " " << dev.getDevName() << endl;

	do{
		try{
			dev.initServer();
			dev.listen4Req(client);
			dev.fetchRequestData(req);
			string strreq{req.data.begin(), req.data.end()};
			cout << "SERVER --Request: " << strreq << endl;
			processRequest(req, rsp);
			string strrsp{rsp.data.begin(), rsp.data.end()};
			cout << "Server Rsp " << strrsp << " " << rsp.size << endl;
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

		if (!(*active))
			break;

		req.clear();
		rsp.clear();
	} while (1);
}

int main(int argc, char *argv[]){
	// creating a new torrent
	/*string torrentName {"larger"};
	string file1 {"test/test1"};
	string file2 {"test/test2"};
	string file3 {"test/test3"};
	string file4 {"test/large"};
	vector<string> files{file1, file2, file3, file4};
	Torrent t {torrentName, files};
	t.create();*/

	cout << "hello!" << endl;
	cout << "scanning for devices..." << endl;
	Node myNode;
	myNode.findLocalDevs();
	myNode.scanForDevs();
	cout << "done. " << endl << endl;

	// create server
	//DeviceDescriptor serverDes = myNode.localDevs[0];
	//unique_ptr<Node::Server> server = myNode.createServerThread(myNode.localDevs[0]);
	//pair<DeviceDescriptor, Node::Server> item = make_pair(serverDes, server);
	//myNode.servers = move(serv);

	/*string in;
	vector<string> args;
	do
	{
		Node::printcli();
		
		getline(cin, in);
		Utils::tokenize(in, " ", args);

		for (auto arg : args)
			cout << "!" << arg << "! ";
		cout << endl;

		if (in.compare("q") == 0){
			break;
		}

		args.clear();
	} while (1);*/

	unique_ptr<Node::Server> server = myNode.createServerThread(myNode.localDevs[0]);
	for (int i = 0; i < 1; i++){
		Message rsp;
		this_thread::sleep_for (std::chrono::milliseconds(10));
		myNode.requestTorrentList(myNode.localDevs[1], myNode.localDevs[0], rsp);
		string rspd{rsp.data.begin(), rsp.data.end()};
		cout << "BACK: " << rspd << endl;
		cout << "LOOP " << i << endl;
	}
	server->close();

    return 0;
}