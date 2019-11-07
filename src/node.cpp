#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <set>
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
	set<DeviceDescriptor> devSet;
	vector<DeviceDescriptor> nearbyDevs;
	for (auto devDes : this->localDevs){
		BTDevice dev{devDes};
		dev.findNearbyDevs(nearbyDevs);
		devSet.insert(nearbyDevs.begin(), nearbyDevs.end());
		nearbyDevs.clear();
	}
	this->remoteDevs.assign(devSet.begin(), devSet.end());

	// TODO: remove local devs from nearby devs
	for (auto dev : this->remoteDevs){
		cout << "Remote Devs: " << dev.addr << " " << dev.devID << " " << dev.name << endl;
	}
}

void Node::processRequest(const vector<char>& req, vector<char>& resp){
	unique_ptr<RRPacket> packet = RRFactory::create(req);

	if (packet){
		packet->processRequest();
		resp = packet->getResp();
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

	string torrentName{"large"};
	for (int i = 0; i < 2; i++){
		ChunkReq req1;
		req1.createRequest(torrentName, i);
		req1.processRequest();
		req1.processResponse(req1.resp, req1.size);
	}
	

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
	Message resp{data};
	DeviceDescriptor client;

	BTDevice dev{devDes};
	cout << "Server Dev: " << dev.getDevAddr() << " " << dev.getDevID() << " " << dev.getDevName() << endl;
	try{
		dev.initServer();
		dev.listen4Req(client);
		dev.fetchRequestData(req);
		string strreq{req.data.begin(), req.data.end()};
		cout << "Message: " << strreq << endl;
		dev.sendResponse(resp);
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
	/*string torrentName {"newPackage"};
	string file1 {"test/test1"};
	string file2 {"test/test2"};
	string file3 {"test/test3"};
	string file4 {"test/large"};
	vector<string> files{file1, file2, file3, file4};
	Torrent t {torrentName, files};
	t.create();*/
	//Peer me{};
	//me.createRequest();

	// searching for local devices
	// and every nearby device
	// for each local
	/*vector<DeviceDescriptor> localDevs;
	vector<DeviceDescriptor> nearbyDevs;
	BTDevice::findLocalDevs(localDevs);

	vector<BTDevice> devices;
	for (auto& dd : localDevs){
		BTDevice dev{dd};
		dev.enableScan();
		devices.push_back(dev);
	}

	for (auto& dev : devices){
		dev.findNearbyDevs(nearbyDevs);
	}*/


	vector<DeviceDescriptor> localDevs;
	BTDevice::findLocalDevs(localDevs);

	Node myNode;
	myNode.findLocalDevs();
	myNode.scanForDevs();

	/*thread tServer = myNode.createServerThread(localDevs[0]);

	this_thread::sleep_for (std::chrono::seconds(5));

	cout << "Main thread" << endl;
	string data{"--Client to Server."};
	Message req{data};
	Message resp;

	BTDevice clientDev{localDevs[1]};
	try{
		clientDev.connect2Device(localDevs[0]);
		clientDev.sendReqWait4Resp(req, resp);
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}
	string strresp{resp.data.begin(), resp.data.end()};
	cout << "Message: " << strresp << endl;
	try{
		clientDev.endComm();
	}
	catch(int e){
		cout << "Caught Exception " << e << endl;
	}

	tServer.join();*/

    return 0;
}