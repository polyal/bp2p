#include "torrentFileReq.h"
#include "utils.h"


TorrentFileReq::TorrentFileReq()
{
	this->torrentName = "";
}

TorrentFileReq::TorrentFileReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr)
	: RRPacket(remoteAddr, localAddr)
{
	this->torrentName = "";
}

TorrentFileReq::TorrentFileReq(const Message& req) : RRPacket(req)
{
	this->torrentName = "";
}

TorrentFileReq::TorrentFileReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr,
	const Message& req) : RRPacket(remoteAddr, localAddr, req)
{
	this->torrentName = "";
}

TorrentFileReq::TorrentFileReq(const string& torrentName)
{
	this->torrentName = torrentName;
}

TorrentFileReq::TorrentFileReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr,
	const string& torrentName) : RRPacket(remoteAddr, localAddr) 
{
	this->torrentName = torrentName;
}

void TorrentFileReq::create(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const string& torrentName)
{
	this->localAddr = localAddr;
	this->remoteAddr = remoteAddr;
	this->torrentName = torrentName;	
}

void TorrentFileReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::TorrentFile));
	request += RRPacket::commSeparator + this->torrentName;

	std::copy(request.begin(), request.end(), std::back_inserter(this->req.data));
	this->req.size = request.size();
}

void TorrentFileReq::processRequest(const string& serializedTorrent)
{
	this->serializedTorrent = serializedTorrent;
	processRequest();
}

void TorrentFileReq::processRequest()
{
	std::copy(this->serializedTorrent.begin(), this->serializedTorrent.end(), std::back_inserter(this->rsp.data));
	this->rsp.size = this->serializedTorrent.size();
}

void TorrentFileReq::extractTorrentName(string& torrentName)
{
	vector<string> tokens;
	torrentName = "";
	string strReq {this->req.data.begin(), this->req.data.end()};
	Utils::tokenize(strReq, commSeparator, tokens);
	if (tokens.size() > 2){
		torrentName = tokens[2];
	}
}

void TorrentFileReq::processResponse(const Message& msg)
{
	this->rsp = msg;
	processResponse();
}

void TorrentFileReq::processResponse()
{
	this->serializedTorrent.assign(this->rsp.data.begin(), this->rsp.data.end());
}

string TorrentFileReq::getSerializedTorrent() const
{
	return this->serializedTorrent;
}

string TorrentFileReq::getTorrentName() const
{
	return this->torrentName;
}
