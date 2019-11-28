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

void TorrentFileReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::TorrentFile));
	request += RRPacket::commSeparator + this->torrentName;

	std::copy(request.begin(), request.end(), std::back_inserter(this->req.data));
	this->req.size = request.size();
}

void TorrentFileReq::processRequest()
{
	string torrentName = "";
	string strResp;

	getTorrentNameFromReq(torrentName);
	getSerialzedTorrent(torrentName, strResp);

	std::copy(strResp.begin(), strResp.end(), std::back_inserter(this->rsp.data));
	this->rsp.size = strResp.size();
}

void TorrentFileReq::getTorrentNameFromReq(string& torrentName)
{
	vector<string> tokens;
	torrentName = "";
	string strReq {this->req.data.begin(), this->req.data.end()};
	Utils::tokenize(strReq, commSeparator, tokens);
	if (tokens.size() > 2){
		torrentName = tokens[2];
	}
}

void TorrentFileReq::getSerialzedTorrent(const string& torrentName, string& serializedTorrent)
{
	Torrent torrent {torrentName};
	if (torrent.open())
		serializedTorrent = torrent.getSerializedTorrent();
}

void TorrentFileReq::processResponse(const Message& msg)
{
	this->rsp = msg;
	processResponse();
}

void TorrentFileReq::processResponse()
{
	string strresp{this->rsp.data.begin(), this->rsp.data.end()};
	Torrent torrent;
	torrent.createTorrentFromSerializedObj(strresp);
	this->torrent = torrent;
}

Torrent TorrentFileReq::getTorrent() const
{
	return this->torrent;
}

RRPacket::RequestType TorrentFileReq::getType()
{
	return type;
}
