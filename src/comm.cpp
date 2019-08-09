#include <string>
#include <vector>
#include "comm.h"

// include c libs
extern "C" {
#include "blue.h"
}

using namespace std;




connection::connection(){


}


vector<connection::device> connection::findLocalDevices(){
	int status = 0;
	devInf* devs = NULL;
	int numDevs;
	vector<connection::device> devices;

	status = findDevices(&devs, &numDevs);

	if (status == 0 && devs && numDevs > 0){
		devices.reserve(numDevs);

		int i;
		for (i = 0; i < numDevs; i++){
			connection::device dev{devs[i].addr, devs[i].name};
			devices.push_back(dev);
		}

		free(devs);
	}

	return devices;
}




///////////////////////////////////////////////////////////
// defintions for device class

connection::device::device(){
	this->id = -1;
	this->addr = "";	
	this->name = "";	
}

connection::device::device(string addr, string name){
	this->addr = addr;
	this->name = name;
}

connection::device::device(int id, string addr, string name){
	this->id = id;
	this->addr = addr;
	this->name = name;
}