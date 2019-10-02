#include "torrentFileReq.h"
#include "utils.h"


TorrentFileReq::TorrentFileReq(){
	torrentName = "";
}

TorrentFileReq::TorrentFileReq(const vector<char>& req) : RRPacket(req){
	torrentName = "";
}

void TorrentFileReq::createRequest(const string& torrentName){
	this->torrentName = torrentName;
	createRequest();
}

void TorrentFileReq::createRequest(){
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::torrentFile));
	request += RRPacket::commSeparator + this->torrentName;

	std::copy(request.begin(), request.end(), std::back_inserter(req));
	cout << "create: " << request << endl;
}

void TorrentFileReq::processRequest (){
	string torrentName = "";
	string strResp;

	getTorrentNameFromReq(torrentName);
	getSerialzedTorrent(torrentName, strResp);

	std::copy(strResp.begin(), strResp.end(), std::back_inserter(resp));
	cout << "resp: " << strResp << endl;
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
	if (torrent.open())
		serializedTorrent = torrent.getSerializedTorrent();
	// rewrite this with public functions
	/*if (!torrent.getFilename().empty()){
		torrent.serialize(true);
		serializedTorrent = torrent.getSerializedTorrent();
	}*/
}

void TorrentFileReq::processResponse(){
	string strresp{resp.begin(), resp.end()};
	cout << "start respoProess: " << endl;
	Torrent torrent;
	torrent.createTorrentFromSerializedObj(strresp);
	cout << "1 respoProess: " << endl;
	if (torrent.isValid()){
		this->torrent = torrent;
		//this->torrent.name += "!!!";
		//cout << "2 respoProess: " << endl;
		//this->torrent.dumpToTorrentFile();
	}
}