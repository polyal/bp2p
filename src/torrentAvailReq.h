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
	void processRequest();
	void processResponse(const Message& msg);

	vector<int> getTorrentAvail() const;

private:
	string torrentName;
	vector<int> torrentAvail;
	void getTorrentNameFromReq(string& torrentName);
	void getTorrentAvailFromTorrent(const string torrentName, vector<int>& torrentAvail);
	void processResponse();
};

#endif
