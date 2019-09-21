#include <string>
#include <fstream>
#include "utils.h"
#include "torrent.h"
#include "chunkReq.h"

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

void ChunkReq::retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk){
	Torrent torrent {torrentName};

	if (!torrent.getFilename().empty()){
		torrent.serialize(true);
		chunk = torrent.RetrieveChunk(chunkNum);
	}
}