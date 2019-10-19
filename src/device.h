#include "message.h"
#include "deviceDescriptor.h"

class Device
{
protected:
	DeviceDescriptor ddes;

public:
	virtual int connect2Device(const DeviceDescriptor& dev) = 0;
	virtual int sendReqWait4Resp(const Message& msg, Message& resp) = 0;
	virtual int initServer() = 0;
	virtual int listen4Req(DeviceDescriptor& client) = 0;
	virtual int fetchRequestData(Message& req) = 0;
	virtual int sendResponse(const Message& resp) = 0;
	virtual int endComm() = 0;
};