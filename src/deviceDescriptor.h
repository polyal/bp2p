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

	bool operator==(const DeviceDescriptor& dev) const
	{
		bool isEqual = false;
		if (this->addr.compare(dev.addr) == 0) isEqual = true;
	 	return isEqual;
	}

	bool operator<(const DeviceDescriptor& dev) const
	{
		return this->addr < dev.addr;
	}

	string addr;
	string name;
	int devID = -1;

	static const unsigned short addrLen = 18;
	static const unsigned short maxNameLen = 256;
	inline static const string uknownName = "[unknown]";
};

// needed so we can hash DeviceDescriptors
// eg. so we can store DeviceDescriptors in an unordered_set
namespace std
{
	template<>
	struct hash<DeviceDescriptor>
	{
		size_t operator()(const DeviceDescriptor& dev) const
		{
			return hash<string>()(dev.addr);
		}
	};
}

#endif