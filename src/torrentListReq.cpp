#include <string>
#include "utils.h"
#include "torrent.h"
#include "torrentListReq.h"

TorrentListReq::TorrentListReq()
{
}

TorrentListReq::TorrentListReq(const Message& req) : RRPacket(req)
{
}

TorrentListReq::TorrentListReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr) 
	: RRPacket(remoteAddr, localAddr)
{
}

TorrentListReq::TorrentListReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req) 
	: RRPacket(remoteAddr, localAddr, req)
{
}

void TorrentListReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::TorrentList));

	std::copy(request.begin(), request.end(), std::back_inserter(this->req.data));
	this->req.size = request.size();
}

void TorrentListReq::processRequest(const vector<string>& torrentList)
{
	this->torrentList = torrentList;
	processRequest();
}

void TorrentListReq::processRequest()
{
	string serializedList = "";
	serializeTorrentList(this->torrentList, serializedList);
	std::copy(serializedList.begin(), serializedList.end(), std::back_inserter(this->rsp.data));
	this->rsp.size = serializedList.size();
}

void TorrentListReq::serializeTorrentList(const vector<string>& torrentNames, string& serializedList)
{
	for(auto const& torrentName: torrentNames) {
		serializedList += torrentName;
		serializedList += commSeparator;
	}
	// remove the last commSeparator from end of string
	if (serializedList.size() > 2){
		serializedList.pop_back();
		serializedList.pop_back();
	}
}

void TorrentListReq::processResponse(const Message& msg)
{
	this->rsp = msg;
	processResponse();
}

void TorrentListReq::processResponse()
{
	vector<string> torrentList;
	string rsp{this->rsp.data.begin(), this->rsp.data.end()};
	parseTorrentList(rsp, torrentList);
	this->torrentList = torrentList;
}

void TorrentListReq::parseTorrentList(const string& rsp, vector<string>& torrentList)
{
	Utils::tokenize(rsp, commSeparator, torrentList);
}

vector<string> TorrentListReq::getTorrentList() const
{
	return torrentList;
}

RRPacket::RequestType TorrentListReq::getType()
{
	return type;
}
