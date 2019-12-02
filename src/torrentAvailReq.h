#ifndef TORRENTAVAILREQ_H
#define TORRENTAVAILREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


class TorrentAvailReq : public RRPacket 
{
public:
	TorrentAvailReq();
	TorrentAvailReq(const Message& req);
	TorrentAvailReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr);
	TorrentAvailReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req);
	TorrentAvailReq(const string& torrentName);
	TorrentAvailReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const string& torrentName);

	void createRequest();
	void extractTorrentName(string& torrentName);
	void processRequest(const vector<int>& torrentAvail);
	void processResponse(const Message& msg);
	
	string getTorrentName() const;
	vector<int> getTorrentAvail() const;

private:
	string torrentName;
	vector<int> torrentAvail;
	void processRequest();
	void processResponse();
	RequestType type = TorrentAvailability;
};

#endif
