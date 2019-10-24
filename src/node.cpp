#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include "utils.h"
#include "btdevice.h"
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "chunkReq.h"
#include "rrfactory.h"
#include "node.h"

using namespace std;

const string Peer::commString = "bp2p";
const string Peer::commSeparator = "||";

Peer::Peer(){

}

void Peer::findNearbyDevices(){
	/*int status = 0;
	devInf* devs = NULL;
	int numDevs;
	vector<Device> devices;

	status = ::findDevices(&devs, &numDevs);

	if (status == 0 && devs && numDevs > 0){
		devices.reserve(numDevs);

		int i;
		for (i = 0; i < numDevs; i++){
			Device dev{devs[i].addr, devs[i].name};
			devices.push_back(dev);
		}

		free(devs);
	}

	nodes.insert( nodes.end(), devices.begin(), devices.end() );*/
}

void Peer::findLocalDevices(){
	/*int status = 0;
	devInf* devs = NULL;
	int numDevs;
	vector<Device> devices;

	status = ::findLocalDevices(&devs, &numDevs);

	if (status == 0 && devs && numDevs > 0){
		devices.reserve(numDevs);

		int i;
		for (i = 0; i < numDevs; i++){
			Device dev{devs[i].devId, devs[i].addr, devs[i].name};
			devices.push_back(dev);
		}

		free(devs);
	}

	localDevices = devices;*/
}

void Peer::processRequest(const vector<char>& req, vector<char>& resp){
	unique_ptr<RRPacket> packet = RRFactory::create(req);

	if (packet){
		packet->processRequest();
		resp = packet->getResp();
	}
}


void Peer::createRequest(){
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

///////////////////////////////////////////////////////////
// defintions for device class

Peer::Device::Device(){
	this->id = -1;
	this->addr = "";	
	this->name = "";
}

Peer::Device::Device(string addr, string name){
	this->addr = addr;
	this->name = name;
}

Peer::Device::Device(int id, string addr, string name){
	this->id = id;
	this->addr = addr;
	this->name = name;
}

string Peer::Device::getAddr() const{
	return this->addr;
}

string Peer::Device::getName() const{
	return this->name;
}

int Peer::Device::getChannel() const{
	return this->channel;
}

int Peer::Device::getSendSock() const{
	return this->sendSock;
}

int Peer::Device::getRecSock() const{
	return this->recSock;
}

void Peer::Device::setSendSock(int sock){
	this->sendSock = sock;
}

void Peer::Device::setRecSock(int sock){
	this->recSock = sock;
}

void Peer::Device::addTorrentNames(vector<string> torrentNames){
	this->torrentNames = torrentNames;
}

int main(int argc, char *argv[]){
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

	vector<DeviceDescriptor> localDevs;
	vector<DeviceDescriptor> nearbyDevs;
	BTDevice::findLocalDevs(localDevs);
	BTDevice dev{localDevs[0]};
	dev.findNearbyDevs(nearbyDevs);

    return 0;
}