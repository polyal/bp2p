#include <iostream>
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

void Peer::connectToClient(Peer::Device dev){
	int err = 0;

	::connectToClient(dev.getAddr().c_str(), &err);

	if (err > 0){
		cout << "CreateSock2Client Error: " << err << endl;
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

string Peer::Device::getAddr(){
	return this->addr;
}

string Peer::Device::getName(){
	return this->name;
}

int Peer::Device::getSock(){
	return this->sock;
}


int main(int argc, char *argv[]){
	Peer me{};

	me.findLocalDevices();
	me.findNearbyDevices();

    return 0;
}