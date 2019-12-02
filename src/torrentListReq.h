#ifndef TORRENTLISTREQ_H
#define TORRENTLISTREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


class TorrentListReq : public RRPacket 
{
public:
	TorrentListReq();
	TorrentListReq(const Message& req);
	TorrentListReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr);
	TorrentListReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req);

	void createRequest();
	void processRequest(const vector<string>& torrentList);
	void processResponse(const Message& msg);

	vector<string> getTorrentList() const;

private:
	vector<string> torrentList;
	void processRequest();
	void processResponse();
	void serializeTorrentList(const vector<string>& torrentNames, string& serializedList);
	void parseTorrentList(const string& resp, vector<string>& torrentList);
	RequestType type = TorrentList;
};

#endif
