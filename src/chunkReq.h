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
	ChunkReq(const string& torrentName, const int chunkNum);
	ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, 
		const string& torrentName, const int chunkNum);

	void createRequest();
	void processRequest();
	void processResponse(const Message& msg);

	string getTorrentName() const;
	int getChunkNum() const;

private:
	string torrentName;
	int chunkNum = -1;

	void getTorrentNameFromReq(string& torrentName);
	int getChunkNumFromReq(const string& req);
	void retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk, int& size);

	void processResponse();
};

#endif