#include <iostream>
#include <algorithm>
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "chunkReq.h"
#include "rrfactory.h"

RRPacket::requestType RRFactory::getReqTypeFromReq(const vector<char>& req)
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string strReqType;
	int iReqType;
	RRPacket::requestType reqType;

	if (req.size() <= prefix.length()){
		cout << "Bad Request" << endl;
		return RRPacket::badReq;
	}

	auto it = std::search(req.begin(), req.end(), prefix.begin(), prefix.end());
	if (std::distance(req.begin(), it) != 0){
		cout << "Bad Request" << endl;
		return RRPacket::badReq;
	}

	try {
		iReqType = stoi(&req[prefix.length()]);
	}
	catch (...) {
		cout << "Bad Request" << endl;
	    return RRPacket::badReq;
	}
	
	reqType = static_cast<RRPacket::requestType>(iReqType);

	return reqType;
}

unique_ptr<RRPacket> RRFactory::create(const vector<char>& req) 
{
	RRPacket::requestType reqType = getReqTypeFromReq(req);

	switch(reqType) {
	    case RRPacket::torrentFile:
	        return unique_ptr<TorrentFileReq>(new TorrentFileReq(req));
	    case RRPacket::chunk:
	        cout << "Chunk" << endl;
	        return unique_ptr<ChunkReq>(new ChunkReq(req));
	    case RRPacket::torrentList:
	    	cout << "Torrent List" << endl;
	    	return unique_ptr<TorrentListReq>(new TorrentListReq(req));
	    default:
	    	cout << "Bad Request" << endl;
	    	return nullptr;
	}
}