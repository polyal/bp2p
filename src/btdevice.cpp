#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/ioctl.h>
#include "btdevice.h"

#define DEBUG 0

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
	readLocalName(this->des.name, this->des.devID);

    cout << "New Device " << this->des.addr << " " << this->des.devID << " " << this->des.name << endl;
}

void BTDevice::connect2Device(const DeviceDescriptor& dev)
{
	try{
		channel.salloc();
		channel.setCh(this->des.addr, 0);
		channel.bind();
		channel.setRemoteCh(dev.addr, 15);
		channel.connect();
	}
	catch(...){
		throw;
	}
}

void BTDevice::sendReqWait4Resp(const Message& req, Message& resp)
{
	try{
		channel.writeToServer(req);
		channel.readFromServer(resp);
	}
	catch(...){
		throw;
	}
}

void BTDevice::initServer()
{
	channel.setCh(this->des.addr ,serverCh);
	try{
		channel.salloc();
		channel.bind();
	}
	catch(...){
		throw;
	}
}

void BTDevice::listen4Req(DeviceDescriptor& client)
{
	try{
		channel.listen();
		channel.accept(client);
	}
	catch(...){
		throw;
	}
}

void BTDevice::fetchRequestData(Message& req)
{
	try{
		channel.readFromClient(req);
	}
	catch(...){
		throw;
	}
}

void BTDevice::sendResponse(const Message& resp)
{
	try{
		channel.writeToClient(resp);
	}
	catch(...){
		throw;
	}
}

void BTDevice::endClientComm()
{
	try{
		channel.closeRemote();
	}
	catch(...){
		throw;
	}
}

void BTDevice::endServerComm()
{
	try{
		channel.close();
	}
	catch(...){
		throw;
	}
}

void BTDevice::endComm()
{
	try{
		endClientComm();
    }
    catch(int e){
    	cout << "Close client sock Error: " << e << endl;
    }
    try{
    	endServerComm();
    }
    catch(int e){
    	cout << "Close server sock Error: " << e << endl;
    }
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
    vector<char> cAddr(DeviceDescriptor::addrLenWithNull, 0);
    
    hci_devba(devReq.dev_id, &bdaddr);
    ba2str(&bdaddr, cAddr.data());

    string name;
    readLocalName(name, devReq.dev_id);

    string addr{cAddr.begin(), cAddr.end()};
    dev.create(addr, name, devReq.dev_id);

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
	string name;
    readRemoteName(name, inqInf.bdaddr);

    vector<char> cAddr(DeviceDescriptor::addrLenWithNull, 0);
    ba2str(&inqInf.bdaddr, cAddr.data());
    string addr{cAddr.begin(), cAddr.end()};
    
    dev.create(addr, name);
    return 0;
}

void BTDevice::readLocalName(string& name, int devID)
{
	vector<char> cName(DeviceDescriptor::maxNameLen, 0);
	int sock2dev = hci_open_dev(devID);
    hci_read_local_name(sock2dev, cName.size(), cName.data(), 0);
    if (sock2dev >= 0) ::close(sock2dev);
    transform(cName.begin(), cName.end(), back_inserter(name),
               [](char c) {
                   return c;
                });
}

void BTDevice::readRemoteName(string& name, bdaddr_t bdaddr)
{
	vector<char> cName(DeviceDescriptor::maxNameLen, 0);
	int sock = hci_open_dev(this->des.devID);
    if (hci_read_remote_name(sock, &bdaddr, cName.size(), cName.data(), 0) < 0)
        copy(DeviceDescriptor::uknownName.begin(), DeviceDescriptor::uknownName.end(),
         back_inserter(cName));
    if (sock >= 0) ::close(sock);
	transform(cName.begin(), cName.end(), back_inserter(name),
	   [](char c) {
	       return c;
	    });
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
		else if (args[0].compare("-s") == 0 && args.size() > 1){
			string addr{args[1].begin(), args[1].end()};

			string data{"++Server to Client."};
			Message req;
			Message resp{data};
			DeviceDescriptor client;

			BTDevice myDev{addr};
			try{
				myDev.initServer();
				myDev.listen4Req(client);
				myDev.fetchRequestData(req);
				string strreq{req.data.begin(), req.data.end()};
				cout << "Message: " << strreq << endl;
				myDev.sendResponse(resp);
			}
			catch(int e){
				cout << "Caught Exception " << e << endl;
			}
			try{
				myDev.endComm();
			}
			catch(int e){
				cout << "Caught Exception " << e << endl;
			}

		}
		else if (args[0].compare("-c") == 0 && args.size() > 2){
			string addr{args[1].begin(), args[1].end()};
			DeviceDescriptor dev{addr};

			string data{"--Client to Server."};
			Message req{data};
			Message resp;

			BTDevice myDev;
			try{
				myDev.connect2Device(dev);
				myDev.sendReqWait4Resp(req, resp);
			}
			catch(int e){
				cout << "Caught Exception " << e << endl;
			}
			string strresp{resp.data.begin(), resp.data.end()};
			cout << "Message: " << strresp << endl;
			try{
				myDev.endComm();
			}
			catch(int e){
				cout << "Caught Exception " << e << endl;
			}
		}
	}
	else{
		cout << "Usage: " << endl;
	}
	
	return 0;
}
#endif