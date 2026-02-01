#ifndef DEVICEDESCRIPTOR_H
#define DEVICEDESCRIPTOR_H

#include <string>

using namespace std;


struct DeviceDescriptor
{
	DeviceDescriptor();
	DeviceDescriptor(const string& addr, int devID = -1);
	DeviceDescriptor(const string& addr, const string& name, int devID = -1);

	void create(const string& addr, int devID = -1);
	void create(const string& addr, const string& name, int devID = -1);

	DeviceDescriptor& operator=(const DeviceDescriptor& dev);
	bool operator==(const DeviceDescriptor& dev) const;
	bool operator<(const DeviceDescriptor& dev) const;

	string addr;
	string name;
	int devID = -1;

	static const unsigned short addrLen = 17;
	static const unsigned short addrLenWithNull = 18;
	static const unsigned short maxNameLen = 256;
	static const string uknownName;
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
