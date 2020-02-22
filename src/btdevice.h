#include "btchannel.h"
#include "device.h"
#include <string>

using namespace std;

class BTDevice : public Device
{
public:
	BTDevice(const DeviceDescriptor& dev);
	BTDevice(const string& devAddr);

	void connect2Device(const DeviceDescriptor& dev);
	void sendReqWait4Resp(const Message& req, Message& resp);
	void initServer();
	void listen4Req(DeviceDescriptor& client);
	void fetchRequestData(Message& req);
	void sendResponse(const Message& resp);
	void endComm();

	int findNearbyDevs(vector<DeviceDescriptor>& devs);
	static int findLocalDevs(vector<DeviceDescriptor>& devs);

	int enableScan();

protected:
	static const unsigned short maxDevs = 255;  // max devs we can search for
	static const unsigned short discUnit = 2;   // discovery unit * 1.28sec = time spent searching for devices
	unsigned int serverCh = 15;  // server channel
	unsigned int clientCh = 0;   // client connects to first available channel

	BTChannel channel;

private:
	int getInqInfo(inquiry_info*& inqInf, int& numDevs);
	int inqInfList2DevDesList(vector<DeviceDescriptor>& devs, const inquiry_info* const inqInf, int numDevs);
	int inqInf2DevDes(DeviceDescriptor& dev, const inquiry_info& inqInf);
	void readRemoteName(string& name, bdaddr_t bdaddr);

	static int getHCIDevList(struct hci_dev_list_req*& devList, int& numDevs);
	static int HCIDevList2DevDesList(vector<DeviceDescriptor>& devs, const struct hci_dev_list_req* const devList, int nDevs);
	static bool HCIDev2DevDes(DeviceDescriptor& dev, const struct hci_dev_req& devReq);
	static void readLocalName(string& name, int devID);
};
