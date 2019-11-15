#ifndef TORRENTLISTREQ_H
#define TORRENTLISTREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


class TorrentListReq : public RRPacket 
{
public:
	vector<string> torrentList;

	TorrentListReq();
	TorrentListReq(const Message& req);
	TorrentListReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr);
	TorrentListReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req);

	void createRequest();
	void processRequest();
	void processResponse(const Message& msg);

private:
	void processResponse();

	void getTorrentList(vector<string>& torrentNames);
	void serializeTorrentList(const vector<string>& torrentNames, string& serializedList);

	void parseTorrentList(const string& resp, vector<string>& torrentList);
};

#endif
