#ifndef TORRENTFILEREQ_H
#define TORRENTFILEREQ_H

#include <vector>
#include "torrent.h"
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

	void createRequest();
	void extractTorrentName(string& torrentName);
	void processRequest(const string& serializedTorrent);
	void processResponse(const Message& msg);

	RequestType getType();

	Torrent getTorrent() const;

private:
	Torrent torrent;
	string torrentName;
	string serializedTorrent;
	void processRequest();
	void processResponse();
	RequestType type = TorrentFile;
};

#endif