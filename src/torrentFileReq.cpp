#include "torrentFileReq.h"
#include "torrent.h"
#include "utils.h"


TorrentFileReq::TorrentFileReq(){
}

TorrentFileReq::TorrentFileReq(const vector<char>& req) : RRPacket(req){
}

void TorrentFileReq::processRequest (){
	string torrentName = "";
	string strResp;

	getTorrentNameFromReq(torrentName);
	getSerialzedTorrent(torrentName, strResp);

	std::copy(strResp.begin(), strResp.end(), std::back_inserter(resp));
}

void TorrentFileReq::getTorrentNameFromReq(string& torrentName){
	vector<string> tokens;
	torrentName = "";
	string strReq {this->req.begin(), this->req.end()};

	Utils::tokenize(strReq, commSeparator, tokens);

	if (tokens.size() > 2){
		torrentName = tokens[2];
	}
}

void TorrentFileReq::getSerialzedTorrent(const string& torrentName, string& serializedTorrent){
	Torrent torrent {torrentName};

	if (!torrent.getFilename().empty()){
		torrent.serialize(true);
		serializedTorrent = torrent.getSerializedTorrent();
	}
}