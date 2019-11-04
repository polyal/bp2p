#ifndef DEVICEDESCRIPTOR_H
#define DEVICEDESCRIPTOR_H

#include <string>

using namespace std;


struct DeviceDescriptor
{
	DeviceDescriptor()
	{
	}

	DeviceDescriptor(const string& addr, int devID = -1)
	{
		this->addr = addr;
		this->name = "";
		this->devID = devID;
	}

	DeviceDescriptor(const string& addr, const string& name, int devID = -1)
	{
		this->addr = addr;
		this->name = name;
		this->devID = devID;
	}

	void create(const string& addr, int devID = -1)
	{
		this->addr = addr;
		this->name = "";
		this->devID = devID;
	}

	void create(const string& addr, const string& name, int devID = -1)
	{
		this->addr = addr;
		this->name = name;
		this->devID = devID;
	}

	DeviceDescriptor& operator=(const DeviceDescriptor& dev)
	{
		this->addr = dev.addr;
		this->name = dev.name;
		this->devID = dev.devID;
		return *this;
	}

	string addr;
	string name;
	int devID = -1;

	static const unsigned short addrLen = 18;
	static const unsigned short maxNameLen = 256;
	inline static const string uknownName = "[unknown]";
};

#endif