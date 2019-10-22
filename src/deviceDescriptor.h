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

	string addr;
	string name;
	int devID = -1;
	int sock = -1;
};

#endif