#include <iostream>
#include <algorithm>
#include "rrpacket.h"
//#include "torrent.h"
//#include "utils.h"

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

RRPacket::requestType RRPacket::getReqTypeFromReq(const vector<char>& req){
	string prefix = commString + commSeparator;
	string strReqType;
	int iReqType;
	requestType reqType;

	if (req.size() <= prefix.length()){
		cout << "Bad Request" << endl;
		return badReq;
	}

	auto it = std::search(req.begin(), req.end(), prefix.begin(), prefix.end());
	if (std::distance(req.begin(), it) != 0){
		cout << "Bad Request" << endl;
		return badReq;
	}

	try {
		iReqType = stoi(&req[prefix.length()]);
	}
	catch (...) {
		cout << "Bad Request" << endl;
	    return badReq;
	}
	
	reqType = static_cast<requestType>(iReqType);

	return reqType;
}


// have to implement this because my compiler version doesn't
// support make_unnique
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/*unique_ptr<RRPacket> RRPacket::create(const vector<char>& req) {
	requestType reqType = getReqTypeFromReq(req);

	switch(reqType) {
	    case torrentFile:
	        cout << "Torrent File" << endl;
	        return make_unique<SubClass>(param,param2);
	    case chunk:
	        cout << "Chunk" << endl;
	        return make_unique<SubClass>(param,param2);
	    case torrentList:
	    	cout << "Torrent List" << endl;
	    	return make_unique<SubClass>(param,param2);
	    default:
	    	cout << "Bad Request" << endl;
	    	return nullptr;
	}

	return reqType;
}*/


/*TorrentFileReq::TorrentFileReq(){
}

TorrentFileReq::TorrentFileReq(const vector<char>& req) : RRPacket(req){
}

void TorrentFileReq::processRequest (){
	string torrentName = "";
	String strResp;

	getTorrentNameFromReq(req, torrentName);
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
	string serializedTorrent = "";

	if (!torrent.getFilename().empty()){
		torrent.serialize(true);
		serializedTorrent = torrent.getSerializedTorrent();
	}
}

TorrentListReq::TorrentListReq(){
}

TorrentListReq::TorrentListReq(const vector<char>& req):RRPacket(req){
}

void TorrentListReq::processRequest(){
	vector<string> torrentNames;
	string serializedList = "";

	getTorrentList(torrentNames);
	serializeTorrentList(torrentNames, serializedList);

	std::copy(serializedList.begin(), serializedList.end(), std::back_inserter(this->resp));
	cout << "resp: " << resp << endl;
}

void TorrentListReq::getTorrentList(vector<string>& torrentNames){
	vector<string> torrentFiles;
	torrentFiles = Torrent::getTorrentNames();

	for(auto const& filename: torrentFiles) {
		Torrent tor {filename};
		string torrentName = tor.getFilename();
		if (!torrentName.empty())
			torrentNames.push_back(torrentName);
		cout << "file: " << filename << " torrent: " << torrentName << endl;
	}
}

void TorrentListReq::serializeTorrentList(const vector<string>& torrentNames, string& serializedList){
	string serializedList = "";

	for(auto const& torrentName: torrentNames) {
		serializedList += torrentName;
		serializedList += commSeparator;
	}

	if (serializedList.size() > 2){
		serializedList.pop_back();
		serializedList.pop_back();
	}
}

ChunkReq::ChunkReq(){
}

ChunkReq::ChunkReq(const vector<char>& req):RRPacket(req){
}

void ChunkReq::processRequest(){
	string torrentName = "";
	int chunkNum = -1;
	vector<char> chunk;
	string strReq {this->req.begin(), this->req.end()};

	getTorrentNameFromReq(strReq);
	chunkNum = getChunkNumFromReq(strReq);
	retrieveChunk(torrentName, chunkNum, chunk);

	resp = chunk;

	// testing purposes
	string filename = Torrent::getTorrentDataPath() + "download";
	ofstream fTorrent {filename};

	if (fTorrent.is_open()){
		fTorrent.write(chunk.data(), chunk.size());
 		fTorrent.close();
 		Torrent dowload{};
 		dowload.unpackage(filename);
	}
	else{
		cout << "Dump Error: error opening output torrent file" << endl;
	}
}

void ChunkReq::getTorrentNameFromReq(string& torrentName){
	vector<string> tokens;
	torrentName = "";
	string strReq {this->req.begin(), this->req.end()};

	Utils::tokenize(strReq, commSeparator, tokens);

	if (tokens.size() > 2){
		torrentName = tokens[2];
	}
}

int ChunkReq::getChunkNumFromReq(const string& req){
	vector<string> tokens;
	int chunkNum = -1;

	Utils::tokenize(req, commSeparator, tokens);

	if (tokens.size() > 3){
		try {
    		chunkNum = stoi(tokens[3]);
		}
		catch (...) {
			cout << "Bad Chunk Num" << endl;
		    chunkNum = -1;
		}
	}

	return chunkNum;
}

void Peer::retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk){
	Torrent torrent {torrentName};

	if (!torrent.getFilename().empty()){
		torrent.serialize(true);
		chunk = torrent.RetrieveChunk(chunkNum);
	}
}*/
