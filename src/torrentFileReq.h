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
	void processRequest();
	void processResponse(const Message& msg);

	RequestType getType();

	Torrent getTorrent() const;

private:
	Torrent torrent;
	string torrentName;
	void processResponse();
	void getTorrentNameFromReq(string& torrentName);
	void getSerialzedTorrent(const string& torrentName, string& serializedTorrent);
	RequestType type = TorrentFile;
};

#endif