#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
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
	TorrentFileReq req;
	req.createRequest(torrentName);
	sendRequestWait4Response(req, rsp, client, server);
	req.processResponse(rsp);
}

void Node::requestChunk(const DeviceDescriptor& client, const DeviceDescriptor& server, 
	const string& torrentName, const int chunkNum, Message& rsp)
{
	ChunkReq req;
	req.createRequest(torrentName, chunkNum);
	sendRequestWait4Response(req, rsp, client, server);
	req.processResponse(rsp);
}

void Node::sendRequestWait4Response(RRPacket& req, Message& rsp, 
	const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes)
{
	BTDevice client{clientDes};
	Message msg{req.getReq(), (unsigned int)req.getReq().size()};

	try{
		client.connect2Device(serverDes);
		client.sendReqWait4Resp(msg, rsp);
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}
	//string strresp{rsp.data.begin(), rsp.data.end()};
	cout << "CLIENT RSP: " << rsp.data.size() << " " << rsp.size << endl;
	try{
		client.endComm();
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}
}

void Node::processRequest(const Message& req, Message& rsp)
{
	vector<char> c_rsp;
	processRequest(req.data, c_rsp);
	rsp.create(c_rsp, (unsigned int)c_rsp.size());
}
void Node::processRequest(const vector<char>& req, vector<char>& rsp)
{
	unique_ptr<RRPacket> packet = RRFactory::create(req);

	if (packet){
		packet->processRequest();
		rsp = packet->getResp();
	}
}

void Node::createRequest(){
	/*TorrentListReq req1;
	req1.createRequest();
	req1.processRequest();
	req1.processResponse();*/

	/*string torrentName{"torrent"};
	TorrentFileReq req2;
	req2.createRequest(torrentName);
	req2.processRequest();
	req2.processResponse();*/

	/*string torrentName{"large"};
	for (int i = 0; i < 2; i++){
		ChunkReq req1;
		req1.createRequest(torrentName, i);
		req1.processRequest();
		req1.processResponse(req1.resp, req1.size);
	}*/
	

	/*string strreq1{req1.req.begin(), req1.req.end()};
	string strreq2{req2.req.begin(), req2.req.end()};
	string strreq3{req3.req.begin(), req3.req.end()};
	cout << strreq1 << endl << strreq2 << endl << strreq3 << endl;*/
}

thread Node::createServerThread(DeviceDescriptor servDev)
{
	thread tServer{Node::server, servDev};
	return tServer;
}


void Node::server(DeviceDescriptor devDes)
{
	string data{"++Server to Client."};
	Message req;
	Message rsp{data};
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
		string strrsp{rsp.data.begin(), rsp.data.end()};
		//cout << "SERVER --Response: " << strrsp << endl;
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

	Node myNode;
	myNode.findLocalDevs();
	myNode.scanForDevs();


	for (int i = 0; i < 2; i++){
		Message rsp;
		thread tServer = myNode.createServerThread(myNode.localDevs[0]);
		this_thread::sleep_for (std::chrono::seconds(5));
		myNode.requestChunk(myNode.localDevs[1], myNode.localDevs[0], "large", i, rsp);
		tServer.join();
		cout << "LOOP " << i << endl;
	}
	

    return 0;
}