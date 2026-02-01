#include <iostream>
#include <algorithm>
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "torrentAvailReq.h"
#include "chunkReq.h"
#include "rrfactory.h"

RRPacket::RequestType RRFactory::getReqTypeFromReq(const Message& req)
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string strReqType;
	int iReqType;
	RRPacket::RequestType reqType;
	vector<char> reqData = req.data;

	if (reqData.size() <= prefix.length()){
		cout << "Bad Request" << endl;
		return RRPacket::BadReq;
	}

	auto it = std::search(reqData.begin(), reqData.end(), prefix.begin(), prefix.end());
	if (std::distance(reqData.begin(), it) != 0){
		cout << "Bad Request" << endl;
		return RRPacket::BadReq;
	}

	try {
		iReqType = stoi(&reqData[prefix.length()]);
	}
	catch (...) {
		cout << "Bad Request" << endl;
	    return RRPacket::BadReq;
	}

	reqType = static_cast<RRPacket::RequestType>(iReqType);
	return reqType;
}

unique_ptr<RRPacket> RRFactory::create(const Message& req) 
{
	RRPacket::RequestType reqType = getReqTypeFromReq(req);

	switch(reqType){
	    case RRPacket::TorrentFile:
	    	return make_unique<TorrentFileReq>(req);
	    case RRPacket::Chunk:
	    	return make_unique<ChunkReq>(req);
	    case RRPacket::TorrentList:
	    	return make_unique<TorrentListReq>(req);
	    case RRPacket::TorrentAvailability:
	     	return make_unique<TorrentAvailReq>(req);
	    default:
	    	cout << "Bad Request" << endl;
	    	return nullptr;
	}
}
