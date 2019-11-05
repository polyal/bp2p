#include "message.h"
#include "deviceDescriptor.h"

class Device
{
protected:
	DeviceDescriptor des;

public:
	virtual void connect2Device(const DeviceDescriptor& dev) = 0;
	virtual void sendReqWait4Resp(const Message& msg, Message& resp) = 0;
	virtual void initServer() = 0;
	virtual void listen4Req(DeviceDescriptor& client) = 0;
	virtual void fetchRequestData(Message& req) = 0;
	virtual void sendResponse(const Message& resp) = 0;
	virtual void endComm() = 0;
};