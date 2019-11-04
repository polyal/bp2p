#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/ioctl.h>
#include "btdevice.h"

#define DEBUG 1

BTDevice::BTDevice()
{

}

BTDevice::BTDevice(const DeviceDescriptor& dev)
{
	this->des = dev;
}

BTDevice::BTDevice(const string& devAddr)
{
	this->des.addr = devAddr;
	this->des.devID = hci_devid(devAddr.c_str());

	vector<char> cName(249, 0);
	int sock2dev = hci_open_dev(this->des.devID);
    hci_read_local_name(sock2dev, 249, cName.data(), 0);
    if (sock2dev >= 0) ::close(sock2dev);
    transform(cName.begin(), cName.end(), back_inserter(this->des.name),
               [](char c) {
                   return c;
                });

    cout << "New Device " << this->des.addr << " " << this->des.devID << " " << this->des.name << endl;
}

int BTDevice::connect2Device(const DeviceDescriptor& dev)
{
	int status = 0;
	status = channel.salloc();
	channel.setCh(this->des.addr, 0);
	channel.bind();
	channel.setRemoteCh(dev.addr, 15);
	status = channel.connect();
	return status;
}

int BTDevice::sendReqWait4Resp(const Message& req, Message& resp)
{
	int status = 0;
	status = channel.writeToServer(req);
	status = channel.readFromServer(resp);
	return status;
}

int BTDevice::initServer()
{
	int status = 0;
	channel.setCh(serverCh);
	status = channel.salloc();
	status = channel.bind();
	return status;
}

int BTDevice::listen4Req(DeviceDescriptor& client)
{
	int status = 0;
	status = channel.listen();
	status = channel.accept(client);
	return status;
}

int BTDevice::fetchRequestData(Message& req)
{
	int status = 0;
	status = channel.readFromClient(req);
	return status;
}

int BTDevice::sendResponse(const Message& resp)
{
	int status = 0;
	status = channel.writeToClient(resp);
	return status;
}

int BTDevice::endClientComm()
{
	int status = 0;
	status = channel.closeRemote();
	return status;
}

int BTDevice::endServerComm()
{
	int status = 0;
	status = channel.close();
	return status;
}

int BTDevice::endComm()
{
	int status = 0;
	status = endClientComm();
    status = endServerComm();
    return status;
}

int BTDevice::findLocalDevs(vector<DeviceDescriptor>& devs)
{
	struct hci_dev_list_req* devList = NULL;
    int numDevs = 0, status = 0;

    status = getHCIDevList(devList, numDevs);
    status = HCIDevList2DevDesList(devs, devList, numDevs);

    if (devList) free(devList);

    return status;
}

int BTDevice::getHCIDevList(struct hci_dev_list_req*& devList, int& numDevs)
{
    int status = -1;
    int sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (sock >= 0){
        devList = reinterpret_cast<struct hci_dev_list_req*>
        		  (malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t)));
	    
	    if (devList){
	        memset(devList, 0, HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t));
    		devList->dev_num = HCI_MAX_DEV;

    		// request list of devices from microcontroller
		    if (ioctl(sock, HCIGETDEVLIST, (void *) devList) >= 0){
		    	if (devList->dev_num > 0){
		  			status = 0;
		  			numDevs = devList->dev_num;
		    	}
		    
		    }

	    }
    }

    if (status == -1){
    	status = errno;
    	if (devList) free(devList);
    	numDevs = 0;
    	devList = NULL;
    }
    if (sock >= 0) ::close(sock);

    return status;
}

int BTDevice::HCIDevList2DevDesList(vector<DeviceDescriptor>& devs, 
	const struct hci_dev_list_req* const devList, int nDevs)
{
	if (!devList || nDevs < 1) return -1;

	const struct hci_dev_req* devReq = devList->dev_req;
    for (int i = 0; i < nDevs; i++) {
    	DeviceDescriptor dev;
        if (HCIDev2DevDes(dev, devReq[i]))
        	devs.push_back(dev);
    }

    return 0;
}

bool BTDevice::HCIDev2DevDes(DeviceDescriptor& dev, const struct hci_dev_req& devReq)
{
	int decOpt = devReq.dev_opt;
	if (!hci_test_bit(HCI_UP, &decOpt)) return false;

	bdaddr_t bdaddr = {0};
    vector<char> cAddr(18, 0);
    vector<char> cName(249, 0);
    
    hci_devba(devReq.dev_id, &bdaddr);
    ba2str(&bdaddr, cAddr.data());

    int sock2dev = hci_open_dev( devReq.dev_id );
    hci_read_local_name(sock2dev, 249, cName.data(), 0);
    if (sock2dev >= 0) ::close(sock2dev);

    string addr{cAddr.begin(), cAddr.end()};
    string name{cName.begin(), cName.end()};
    dev.create(addr, name, devReq.dev_id);

    cout << "Find Local Dev: " << dev.devID << " " << dev.addr << " " << dev.name << " " << endl;

    return true;
}

int BTDevice::findNearbyDevs(vector<DeviceDescriptor>& devs)
{
	inquiry_info* inqInf = NULL;
	int numDevs = 0, status = -1;
    
    status = getInqInfo(inqInf, numDevs);
    status = inqInfList2DevDesList(devs, inqInf, numDevs);

    if (inqInf) free( inqInf );
    return status;
}

int BTDevice::getInqInfo(inquiry_info*& inqInf, int& numDevs)
{
	int status = -1;
    
    if (this->des.devID >= 0) {

        inqInf = reinterpret_cast<inquiry_info*>(malloc(sizeof(inquiry_info)* BTDevice::maxDevs));
	    if (inqInf){
	    	// perform bluetooth discovery, clear previously discovered devices from cache
		    numDevs = hci_inquiry(this->des.devID, discUnit, BTDevice::maxDevs, NULL, &inqInf, IREQ_CACHE_FLUSH);
		    status = 0;
	    }
    }

    return status;
}

int BTDevice::inqInfList2DevDesList(vector<DeviceDescriptor>& devs, const inquiry_info* const inqInf, int numDevs)
{
    for (int i = 0; i < numDevs; i++) {
    	DeviceDescriptor dev;
    	inqInf2DevDes(dev, inqInf[i]);
        devs.push_back(dev);
    }

    return 0;
}

int BTDevice::inqInf2DevDes(DeviceDescriptor& dev, const inquiry_info& inqInf)
{
	vector<char> cAddr(DeviceDescriptor::addrLen, 0);
	vector<char> cName(DeviceDescriptor::maxNameLen, 0);

    ba2str(&inqInf.bdaddr, cAddr.data());
    int sock = hci_open_dev(this->des.devID);
    if (hci_read_remote_name(sock, &inqInf.bdaddr, cName.size(), cName.data(), 0) < 0)
        std::copy(DeviceDescriptor::uknownName.begin(), DeviceDescriptor::uknownName.end(),
         back_inserter(cName));
    if (sock >= 0) ::close(sock);

    string addr{cAddr.begin(), cAddr.end()};
    string name{cName.begin(), cName.end()};
    dev.create(addr, name);
    cout << "Devices: " << dev.addr << " " << dev.name << endl;

    return 0;
}

int BTDevice::enableScan()
{
	int status = -1;
    struct hci_dev_req dr;

    int sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (sock >= 0){
		dr.dev_id = this->des.devID;
		dr.dev_opt = SCAN_PAGE | SCAN_INQUIRY;
		if (ioctl(sock, HCISETSCAN, (unsigned long) &dr) >= 0)
			status = 0;
	}

	if (status == -1) status = errno;
	cout << "enable scan err: " << status << " dev: " << this->des.devID << endl;
	if (sock >= 0) ::close(sock);
	return status;
}

#if DEBUG == 1
int main (int argc, char *argv[])
{
	vector<string> args {argv+1, argv+argc};
	vector<DeviceDescriptor> devs;

	if (args.size() > 0){
		if (args[0].compare("-l") == 0)
			BTDevice::findLocalDevs(devs);
		else if (args[0].compare("-f") == 0 && args.size() > 1){
			string addr{args[1].begin(), args[1].end()};
			BTDevice dev{addr};
			dev.findNearbyDevs(devs);
		}
		else if (args[0].compare("-s") == 0 && args.size() > 2){
			string addr{args[1].begin(), args[1].end()};
			int channel = stoi(args[2]);

			string data{"++Server to Client."};
			Message req;
			Message resp{data};
			DeviceDescriptor client;

			BTDevice myDev{addr};
			myDev.initServer();
			myDev.listen4Req(client);
			myDev.fetchRequestData(req);
			string strreq{req.data.begin(), req.data.end()};
			cout << "Message: " << strreq << endl;
			myDev.sendResponse(resp);
			myDev.endComm();

		}
		else if (args[0].compare("-c") == 0 && args.size() > 4){
			string addr{args[1].begin(), args[1].end()};
			int channel = stoi(args[2]);
			DeviceDescriptor dev{addr};

			string data{"--Client to Server."};
			Message req{data};
			Message resp;

			BTDevice myDev;
			myDev.connect2Device(dev);
			myDev.sendReqWait4Resp(req, resp);
			string strresp{resp.data.begin(), resp.data.end()};
			cout << "Message: " << strresp << endl;
			myDev.endComm();
		}
	}
	else{
		cout << "Usage: " << endl;
	}
	
/*
#if 1  // client
	string addr = "34:DE:1A:1D:F4:0B";
	DeviceDescriptor dev{addr};

	string data{"--Client to Server."};
	Message req{data};
	Message resp;

	BTDevice myDev;
	myDev.connect2Device(dev);
	myDev.sendReqWait4Resp(req, resp);
	string strresp{resp.data.begin(), resp.data.end()};
	cout << "Message: " << strresp << endl;
	myDev.endComm();
#else  // server
	string data{"++Server to Client."};
	Message req;
	Message resp{data};
	DeviceDescriptor client;

	BTDevice myDev;
	myDev.initServer();
	myDev.listen4Req(client);
	myDev.fetchRequestData(req);
	string strreq{req.data.begin(), req.data.end()};
	cout << "Message: " << strreq << endl;
	myDev.sendResponse(resp);
	myDev.endComm();
#endif
*/

	return 0;
}
#endif