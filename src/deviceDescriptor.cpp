#include "deviceDescriptor.h"


const string DeviceDescriptor::uknownName = "[unknown]";

DeviceDescriptor::DeviceDescriptor()
{
}

DeviceDescriptor::DeviceDescriptor(const string& addr, int devID)
{
	this->addr = addr;
	this->name = "";
	this->devID = devID;
	this->addr.resize(this->addrLen);
}

DeviceDescriptor::DeviceDescriptor(const string& addr, const string& name, int devID)
{
	this->addr = addr;
	this->name = name;
	this->devID = devID;
	this->addr.resize(this->addrLen);
}

void DeviceDescriptor::create(const string& addr, int devID)
{
	this->addr = addr;
	this->name = "";
	this->devID = devID;
	this->addr.resize(this->addrLen);
}

void DeviceDescriptor::create(const string& addr, const string& name, int devID)
{
	this->addr = addr;
	this->name = name;
	this->devID = devID;
	this->addr.resize(this->addrLen);
}

DeviceDescriptor& DeviceDescriptor::operator=(const DeviceDescriptor& dev)
{
	this->addr = dev.addr;
	this->name = dev.name;
	this->devID = dev.devID;
	return *this;
}

bool DeviceDescriptor::operator==(const DeviceDescriptor& dev) const
{
	return this->addr == dev.addr;
}

bool DeviceDescriptor::operator<(const DeviceDescriptor& dev) const
{
	return this->addr < dev.addr;
}
