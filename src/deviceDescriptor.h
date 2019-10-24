#ifndef DEVICEDESCRIPTOR_H
#define DEVICEDESCRIPTOR_H

#include <string>

using namespace std;


struct DeviceDescriptor
{
	DeviceDescriptor()
	{
	}

	DeviceDescriptor(const string& addr, int devID = -1, int sock = -1)
	{
		this->addr = addr;
		this->name = "";
		this->devID = devID;
		this->sock = sock;
	}

	DeviceDescriptor(const string& addr, const string& name, int devID = -1, int sock = -1)
	{
		this->addr = addr;
		this->name = name;
		this->devID = devID;
		this->sock = sock;
	}

	void create(const string& addr, int devID = -1, int sock = -1)
	{
		this->addr = addr;
		this->name = "";
		this->devID = devID;
		this->sock = sock;
	}

	void create(const string& addr, const string& name, int devID = -1, int sock = -1)
	{
		this->addr = addr;
		this->name = name;
		this->devID = devID;
		this->sock = sock;
	}

	DeviceDescriptor& operator=(const DeviceDescriptor& dev)
	{
		this->addr = dev.addr;
		this->name = dev.name;
		this->devID = dev.devID;
		this->sock = dev.sock;
		return *this;
	}

	string addr;
	string name;
	int devID = -1;
	int sock = -1;

	static const unsigned short addrLen = 18;
	static const unsigned short maxNameLen = 256;
	//static const string uknownName;
};

//const string DeviceDescriptor::uknownName = "[unknown]";

#endif