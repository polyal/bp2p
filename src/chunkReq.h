#ifndef CHUNKREQ_H
#define CHUNKREQ_H

#include <vector>
#include "torrent.h"
#include "rrpacket.h"

using namespace std;

class ChunkReq : public RRPacket 
{
public:
	ChunkReq();
	ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr);
	ChunkReq(const Message& req);
	ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req);
	ChunkReq(const string& torrentName, const int index);
	ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, 
		const string& torrentName, const int index);

	void createRequest();
	void extractNameAndIndex(string& name, int& index);
	void processRequest(const vector<char>& chunk);
	void processResponse(const Message& msg);
	
	string getTorrentName() const;
	int getIndex() const;
	vector<char> getChunk() const;

private:
	string strreq;
	string torrentName;
	int index = -1;
	void extractTorrentName(string& name);
	void extractIndexFromReq(int& index);
	void processRequest();
	RequestType type = Chunk;
};

#endif