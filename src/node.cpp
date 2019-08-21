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

int Peer::connect2Node(Peer::Device& dev){
	int err = 0;
	int sock = -1;

	sock = ::connect2Server(1, dev.getAddr().c_str(), &err);

	if (err > 0){
		cout << "CreateSock2Client Error: " << err << endl;
		return err;
	}

	dev.setSendSock(sock);

	return 0;
}

int Peer::sendReqWait4Resp(const Peer::Device& dev, const string req, string& resp){
	int err = 0;
	auto bResp = unique_ptr<char[]>{new char[255]};
	
	err = ::sendReqWait4Resp(dev.getSendSock(), req.c_str(), req.length(), bResp.get());

	if (err > 0){
		cout << "sendReqWait4Resp Error: " << err << endl;
		return err;
	}

	resp = bResp.get();
	return 0;
}

int Peer::initServer(Peer::Device& dev){
	int err = 0;
	int sock = -1;

	sock = ::initServer(&err);

	if (err > 0){
		cout << "initServer Error: " << err << endl;
		return err;
	}
	cout << "initServer Socket: " << sock << endl;

	dev.setRecSock(sock);
	return 0;
}

int Peer::listen4Req(Peer::Device& dev, Peer::Device& client, string& req){
	int err = 0;
	int clientSock = 0;
	char* data = NULL;
	int size = 0;
	char clientAddr[ADDR_SIZE];

	clientSock = ::listen4Req(dev.getRecSock(), &data, &size, clientAddr, &err);

	if (err > 0){
		cout << "listen4Req Error: " << err << endl;
		return err;
	}

	Device caller{clientAddr, ""};
	client = caller;
	client.setSendSock(clientSock);
	req = data;
	
	if (data) free (data);

	return 0;
}

int Peer::sendResponse(Peer::Device& dev, const string data){
	int err = 0;

	err = ::sendResponse(dev.getSendSock(), data.c_str(), data.size());

	if (err > 0){
		cout << "sendResponse Error: " << err << endl;
		return err;
	}

	return 0;
}

void Peer::endComm(Peer::Device& dev){
	int err = 0;

	err = closeSocket(dev.getSendSock());

	if (err > 0){
		cout << "endComm Error: " << err << endl;
		return;
	}
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

void Peer::Client(){
	int err = 0;

	this->findNearbyDevices();

	if (nodes.size() == 0){
		cout << "Client Error: No Nodes Available " << endl;
		return;
	}

	err = this->connect2Node(nodes[0]);

	if (err > 0){
		cout << "Client Error: connect2Node Failed with " << err << endl;
		return;
	}

	string resp;
	err = this->sendReqWait4Resp(nodes[0], "Send This Data\n", resp);
	if (err > 0){
		cout << "Client Error: sendReqWait4Resp Failed with " << err << endl;
		return;
	}

	cout << "Received " << resp << " from " << nodes[0].getAddr() << endl;

	this->endComm(localDevices[0]);
}

void Peer::Server(){
	int err = 0;

	this->findLocalDevices();
	if (localDevices.size() == 0){
		cout << "Server Error: No Local Devices Available " << endl;
		return;
	}

	err = this->initServer(localDevices[0]);
	if (err > 0){
		cout << "Server Error: initServer Failed with " << err << endl;
		return;
	}
	cout << "Server Notice: Sock " << localDevices[0].getRecSock() << endl;

	Peer::Device client;
	string req;
	err = this->listen4Req(localDevices[0], client, req);
	if (err > 0){
		cout << "Server Error: listen4Req Failed with " << err << endl;
		return;
	}
	cout << localDevices[0].getAddr() << " received " << req << " from " << client.getAddr() << " PORT " << client.getSendSock() << endl;

	err = this->sendResponse(client, "Send Back This Data\n");
	if (err > 0){
		cout << "Server Error: sendResponse Failed with " << err << endl;
		return;
	}

	this->endComm(localDevices[0]);
	this->endComm(client);
}

int main(int argc, char *argv[]){
	Peer me{};

	me.Server();

    return 0;
}