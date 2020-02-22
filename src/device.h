#include "message.h"
#include "deviceDescriptor.h"

class Device
{
public:
	Device(const DeviceDescriptor& devDes) {this->devDes = devDes;}

	virtual void connect2Device(const DeviceDescriptor& dev) = 0;
	virtual void sendReqWait4Resp(const Message& msg, Message& resp) = 0;
	virtual void initServer() = 0;
	virtual void listen4Req(DeviceDescriptor& client) = 0;
	virtual void fetchRequestData(Message& req) = 0;
	virtual void sendResponse(const Message& resp) = 0;
	virtual void endComm() = 0;

	string getDevAddr() { return this->devDes.addr; }
	int getDevID() { return this->devDes.devID; }
	string getDevName() { return this->devDes.name; }

protected:
	DeviceDescriptor devDes;
};
