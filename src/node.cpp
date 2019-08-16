#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "node.h"

// include c libs
extern "C" {
#include "blue.h"
}

using namespace std;

Peer::Peer(){

}

void Peer::findNearbyDevices(){
	int status = 0;
	devInf* devs = NULL;
	int numDevs;
	vector<Device> devices;

	status = findDevices(&devs, &numDevs);

	if (status == 0 && devs && numDevs > 0){
		devices.reserve(numDevs);

		int i;
		for (i = 0; i < numDevs; i++){
			Device dev{devs[i].addr, devs[i].name};
			devices.push_back(dev);
		}

		free(devs);
	}

	nodes.insert( nodes.end(), devices.begin(), devices.end() );
}

void Peer::findLocalDevices(){
	int status = 0;
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

	localDevices = devices;
}

void Peer::connect2Node(Peer::Device& dev){
	int err = 0;
	int sock = -1;

	sock = ::connect2Server(dev.getChannel(), dev.getAddr().c_str(), &err);

	if (err > 0){
		cout << "CreateSock2Client Error: " << err << endl;
	}

	dev.setSendSock(sock);
}

void Peer::sendReqWait4Resp(const Peer::Device& dev, const string req, string resp){
	int err = 0;
	auto bResp = unique_ptr<char[]>{new char[255]};
	
	err = ::sendReqWait4Resp(dev.getSendSock(), req.c_str(), req.length(), bResp.get());

	if (err > 0){
		cout << "sendReqWait4Resp Error: " << err << endl;
		return;
	}

	resp = bResp.get();
}

void Peer::initServer(Peer::Device& dev){
	int err = 0;
	int sock = -1;

	sock = ::initServer(&err);

	if (err > 0){
		cout << "initServer Error: " << err << endl;
		return;
	}

	dev.setRecSock(sock);
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

void Peer::Device::setSendSock(int sock){
	this->sendSock = sock;
}

void Peer::Device::setRecSock(int sock){
	this->recSock = sock;
}


int main(int argc, char *argv[]){
	Peer me{};

	me.findLocalDevices();
	me.findNearbyDevices();

    return 0;
}