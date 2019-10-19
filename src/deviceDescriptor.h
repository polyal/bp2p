#ifndef DEVICEDESCRIPTOR_H
#define DEVICEDESCRIPTOR_H

#include <string>

using namespace std;

struct DeviceDescriptor
{
	DeviceDescriptor()
	{
	}

	DeviceDescriptor(const string& addr, int sock = -1)
	{
		this->addr = addr;
		this->name = "";
		this->sock = sock;
	}

	DeviceDescriptor(const string& addr, const string& name, int sock = -1)
	{
		this->addr = addr;
		this->name = name;
		this->sock = sock;
	}

	string addr;
	string name;
	int sock = -1;
};

#endif