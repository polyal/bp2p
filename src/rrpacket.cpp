#include "rrpacket.h"
//#include "torrent.h"
//#include "utils.h"

class TorrentFileReq;

const string RRPacket::commString = "bp2p";
const string RRPacket::commSeparator = "||";

RRPacket::RRPacket(){
	req.reserve(chunkSize);
	resp.reserve(chunkSize);
}

RRPacket::RRPacket(const vector<char>& req){
	this->req = req;
	resp.reserve(chunkSize);
}

vector<char> RRPacket::getResp(){
	return this->resp;
}
