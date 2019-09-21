#include <string>
#include "torrent.h"
#include "torrentListReq.h"

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