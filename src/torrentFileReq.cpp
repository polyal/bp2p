#include "torrentFileReq.h"
#include "utils.h"


TorrentFileReq::TorrentFileReq()
{
	torrentName = "";
}

TorrentFileReq::TorrentFileReq(const vector<char>& req) : RRPacket(req)
{
	torrentName = "";
}

void TorrentFileReq::createRequest(const string& torrentName)
{
	this->torrentName = torrentName;
	createRequest();
}

void TorrentFileReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::torrentFile));
	request += RRPacket::commSeparator + this->torrentName;

	std::copy(request.begin(), request.end(), std::back_inserter(this->req));
}

void TorrentFileReq::processRequest()
{
	string torrentName = "";
	string strResp;

	getTorrentNameFromReq(torrentName);
	getSerialzedTorrent(torrentName, strResp);

	std::copy(strResp.begin(), strResp.end(), std::back_inserter(this->resp));
}

void TorrentFileReq::getTorrentNameFromReq(string& torrentName)
{
	vector<string> tokens;
	torrentName = "";
	string strReq {this->req.begin(), this->req.end()};

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
	this->resp = msg.data;
	processResponse();
}

void TorrentFileReq::processResponse()
{
	string strresp{this->resp.begin(), this->resp.end()};
	Torrent torrent;
	torrent.createTorrentFromSerializedObj(strresp);
}