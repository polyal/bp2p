#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include "utils.h"
#include "btdevice.h"
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "chunkReq.h"
#include "rrfactory.h"
#include "node.h"

using namespace std;


Peer::Peer(){

}

void Peer::processRequest(const vector<char>& req, vector<char>& resp){
	unique_ptr<RRPacket> packet = RRFactory::create(req);

	if (packet){
		packet->processRequest();
		resp = packet->getResp();
	}
}


void Peer::createRequest(){
	/*TorrentListReq req1;
	req1.createRequest();
	req1.processRequest();
	req1.processResponse();*/

	/*string torrentName{"torrent"};
	TorrentFileReq req2;
	req2.createRequest(torrentName);
	req2.processRequest();
	req2.processResponse();*/

	string torrentName{"large"};
	for (int i = 0; i < 2; i++){
		ChunkReq req1;
		req1.createRequest(torrentName, i);
		req1.processRequest();
		req1.processResponse(req1.resp, req1.size);
	}
	

	/*string strreq1{req1.req.begin(), req1.req.end()};
	string strreq2{req2.req.begin(), req2.req.end()};
	string strreq3{req3.req.begin(), req3.req.end()};
	cout << strreq1 << endl << strreq2 << endl << strreq3 << endl;*/
}

int main(int argc, char *argv[]){
	/*string torrentName {"newPackage"};
	string file1 {"test/test1"};
	string file2 {"test/test2"};
	string file3 {"test/test3"};
	string file4 {"test/large"};
	vector<string> files{file1, file2, file3, file4};
	Torrent t {torrentName, files};
	t.create();*/
	//Peer me{};
	//me.createRequest();

	vector<DeviceDescriptor> localDevs;
	vector<DeviceDescriptor> nearbyDevs;
	BTDevice::findLocalDevs(localDevs);

	vector<BTDevice> devices;
	for (auto& dd : localDevs){
		BTDevice dev{dd};
		dev.enableScan();
		devices.push_back(dev);
	}

	for (auto& dev : devices){
		dev.findNearbyDevs(nearbyDevs);
	}

    return 0;
}