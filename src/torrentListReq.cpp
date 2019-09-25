#include <string>
#include "utils.h"
#include "torrent.h"
#include "torrentListReq.h"

TorrentListReq::TorrentListReq(){
}

TorrentListReq::TorrentListReq(const vector<char>& req):RRPacket(req){
}

void TorrentListReq::createRequest(){
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::torrentList));

	std::copy(request.begin(), request.end(), std::back_inserter(req));
}

void TorrentListReq::processRequest(){
	vector<string> torrentNames;
	string serializedList = "";

	getTorrentList(torrentNames);
	serializeTorrentList(torrentNames, serializedList);

	std::copy(serializedList.begin(), serializedList.end(), std::back_inserter(this->resp));
}

void TorrentListReq::getTorrentList(vector<string>& torrentNames){
	vector<string> torrentFiles;
	torrentFiles = Torrent::getTorrentNames();

	for(auto const& filename: torrentFiles) {
		Torrent tor {filename};
		string torrentName = tor.getFilename();
		if (!torrentName.empty())
			torrentNames.push_back(torrentName);
	}
}

void TorrentListReq::serializeTorrentList(const vector<string>& torrentNames, string& serializedList){
	for(auto const& torrentName: torrentNames) {
		serializedList += torrentName;
		serializedList += commSeparator;
	}

	if (serializedList.size() > 2){
		serializedList.pop_back();
		serializedList.pop_back();
	}
}

void TorrentListReq::processRespose(){
	vector<string> torrentList;
	string resp{this->resp.begin(), this->resp.end()};
	parseTorrentList(resp, torrentList);
	this->torrentList = torrentList;
}

void TorrentListReq::parseTorrentList(const string& resp, vector<string>& torrentList){
	Utils::tokenize(resp, commSeparator, torrentList);
}