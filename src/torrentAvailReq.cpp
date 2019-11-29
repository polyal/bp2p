#include <string>
#include <sstream>
#include <iterator>
#include "utils.h"
#include "torrent.h"
#include "torrentAvailReq.h"

TorrentAvailReq::TorrentAvailReq()
{
}

TorrentAvailReq::TorrentAvailReq(const Message& req) : RRPacket(req)
{
}

TorrentAvailReq::TorrentAvailReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr) 
	: RRPacket(remoteAddr, localAddr)
{
}

TorrentAvailReq::TorrentAvailReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req) 
	: RRPacket(remoteAddr, localAddr, req)
{
}

TorrentAvailReq::TorrentAvailReq(const string& torrentName)
{
	this->torrentName = torrentName;
}

TorrentAvailReq::TorrentAvailReq(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr,
	const string& torrentName) : RRPacket(remoteAddr, localAddr) 
{
	this->torrentName = torrentName;
}

void TorrentAvailReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::TorrentAvailability));
	request += RRPacket::commSeparator + this->torrentName;

	std::copy(request.begin(), request.end(), std::back_inserter(this->req.data));
	this->req.size = request.size();
}

void TorrentAvailReq::processRequest(const vector<int>& torrentAvail)
{
	this->torrentAvail = torrentAvail;
	processRequest();
}

void TorrentAvailReq::processRequest()
{
	string strRsp;
	if (!this->torrentAvail.empty()){
  		ostringstream oss;
    	copy(this->torrentAvail.begin(), this->torrentAvail.end()-1, 
    		ostream_iterator<int>(oss, RRPacket::commSeparator.c_str()));
    	oss << this->torrentAvail.back();
    	strRsp = oss.str();
  	}
  	std::copy(strRsp.begin(), strRsp.end(), std::back_inserter(this->rsp.data));
	this->rsp.size = strRsp.size();
}

void TorrentAvailReq::extractTorrentName(string& torrentName)
{
	vector<string> tokens;
	this->torrentName = "";
	string strReq {this->req.data.begin(), this->req.data.end()};
	Utils::tokenize(strReq, commSeparator, tokens);
	if (tokens.size() > 2){
		torrentName = tokens[2];
	}
}

void TorrentAvailReq::processResponse(const Message& msg)
{
	this->rsp = msg;
	processResponse();
}

void TorrentAvailReq::processResponse()
{
	string strRsp{this->rsp.data.begin(), this->rsp.data.end()};
	vector<string> strAvailChunks;
	Utils::tokenize(strRsp, RRPacket::commSeparator, strAvailChunks);
	for (const auto& strAvailChunk : strAvailChunks){
		int availChunk = -1;
		try {
		 	availChunk = stoi(strAvailChunk);
		}
		catch(invalid_argument& e){
		 	this->torrentAvail.clear();
		 	break;
		}
		this->torrentAvail.push_back(availChunk);
	}
}

vector<int> TorrentAvailReq::getTorrentAvail() const
{
	return this->torrentAvail;
}

RRPacket::RequestType TorrentAvailReq::getType()
{
	return type;
}
