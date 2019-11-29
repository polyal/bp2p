#include <string>
#include <fstream>
#include "utils.h"
#include "torrent.h"
#include "chunkReq.h"

ChunkReq::ChunkReq()
{
}

ChunkReq::ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr)
	: RRPacket(remoteAddr, localAddr)
{
	this->torrentName = "";
	this->index = 0;
}

ChunkReq::ChunkReq(const Message& req) : RRPacket(req)
{
}

ChunkReq::ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr,
	const Message& req) : RRPacket(remoteAddr, localAddr, req)
{
	this->torrentName = "";
	this->index = 0;
}

ChunkReq::ChunkReq(const string& torrentName, const int index)
{
	this->torrentName = torrentName;
	this->index = index;
}

ChunkReq::ChunkReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr,
	const string& torrentName, const int index) : RRPacket(remoteAddr, localAddr) 
{
	this->torrentName = torrentName;
	this->index = index;
}

void ChunkReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::Chunk));
	request += RRPacket::commSeparator + this->torrentName;
	request += RRPacket::commSeparator + to_string(this->index);

	std::copy(request.begin(), request.end(), std::back_inserter(this->req.data));
	this->req.size = request.size();
}

void ChunkReq::processRequest(const vector<char>& chunk)
{
	this->rsp.data = chunk;
	this->rsp.size = chunk.size();
}

void ChunkReq::processRequest()
{
}

void ChunkReq::extractNameAndIndex(string& name, int& index)
{
	this->strreq.assign(this->req.data.begin(), this->req.data.end());
	extractTorrentName(name);
	extractIndexFromReq(index);
}

void ChunkReq::extractTorrentName(string& name)
{
	name.clear();
	vector<string> tokens;
	Utils::tokenize(strreq, commSeparator, tokens);
	if (tokens.size() > 2){
		name = tokens[2];
	}
}

void ChunkReq::extractIndexFromReq(int& index)
{
	index = -1;
	vector<string> tokens;
	Utils::tokenize(strreq, commSeparator, tokens);

	if (tokens.size() > 3){
		try {
    		index = stoi(tokens[3]);
		}
		catch (...) {
			cout << "Bad Index" << endl;
		}
	}
}

void ChunkReq::processResponse(const Message& msg)
{
	this->rsp = msg;
	processResponse();
}

void ChunkReq::processResponse()
{
	Torrent torrent{torrentName};
	if (torrent.open())
	{
		if (!torrent.torrentDataExists())
			torrent.createTorrentDataFile();
		torrent.putChunk(this->rsp.data, this->rsp.size, this->index);
	}
	cout << "put " << this->rsp.data.size() << " " << this->rsp.size << " " << this->index << endl;
}

string ChunkReq::getTorrentName() const
{
	return this->torrentName;
}

int ChunkReq::getIndex() const
{
	return this->index;
}

RRPacket::RequestType ChunkReq::getType()
{
	return type;
}
