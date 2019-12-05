#ifndef TORRENTFILEREQ_H
#define TORRENTFILEREQ_H

#include <vector>
#include "rrpacket.h"

using namespace std;


class TorrentFileReq : public RRPacket 
{
public:
	TorrentFileReq();
	TorrentFileReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr);
	TorrentFileReq(const Message& req);
	TorrentFileReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req);
	TorrentFileReq(const string& torrentName);
	TorrentFileReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const string& torrentName);

	void create(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const string& torrentName);
	
	void createRequest();
	void extractTorrentName(string& torrentName);
	void processRequest(const string& serializedTorrent);
	void processResponse(const Message& msg);
	
	string getSerializedTorrent() const;
	string getTorrentName() const;

private:
	string torrentName;
	string serializedTorrent;
	void processRequest();
	void processResponse();
	RequestType type = TorrentFile;
};

#endif
