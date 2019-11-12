#include <iostream>
#include <algorithm>
#include "torrentFileReq.h"
#include "torrentListReq.h"
#include "chunkReq.h"
#include "rrfactory.h"

RRPacket::requestType RRFactory::getReqTypeFromReq(const Message& req)
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string strReqType;
	int iReqType;
	RRPacket::requestType reqType;

	vector<char> reqData = req.data;

	if (reqData.size() <= prefix.length()){
		cout << "Bad Request" << endl;
		return RRPacket::badReq;
	}

	auto it = std::search(reqData.begin(), reqData.end(), prefix.begin(), prefix.end());
	if (std::distance(reqData.begin(), it) != 0){
		cout << "Bad Request" << endl;
		return RRPacket::badReq;
	}

	try {
		iReqType = stoi(&reqData[prefix.length()]);
	}
	catch (...) {
		cout << "Bad Request" << endl;
	    return RRPacket::badReq;
	}
	
	reqType = static_cast<RRPacket::requestType>(iReqType);

	return reqType;
}

unique_ptr<RRPacket> RRFactory::create(const Message& req) 
{
	RRPacket::requestType reqType = getReqTypeFromReq(req);

	switch(reqType) {
	    case RRPacket::torrentFile:
	    	return make_unique<TorrentFileReq>(req);
	    case RRPacket::chunk:
	    	return make_unique<ChunkReq>(req);
	    case RRPacket::torrentList:
	    	return make_unique<TorrentListReq>(req);
	    default:
	    	cout << "Bad Request" << endl;
	    	return nullptr;
	}
}