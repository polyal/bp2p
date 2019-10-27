#include "rrpacket.h"
//#include "torrent.h"
//#include "utils.h"

class TorrentFileReq;


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
