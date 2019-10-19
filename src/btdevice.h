#include "btchannel.h"
#include "device.h"

using namespace std;

class BTDevice : public Device
{
protected:
	DeviceDescriptor dev;
	BTChannel channel;

public:
	BTDevice();

	int connect2Device(const DeviceDescriptor& dev);
	int sendReqWait4Resp(const Message& req, Message& resp);
	int initServer();
	int listen4Req(DeviceDescriptor& client);
	int fetchRequestData(Message& req);
	int sendResponse(const Message& resp);
	int endComm();
	int endClientComm();
	int endServerComm();
};