#include <string>
#include <fstream>
#include "utils.h"
#include "torrent.h"
#include "chunkReq.h"

ChunkReq::ChunkReq()
{
}

ChunkReq::ChunkReq(const vector<char>& req):RRPacket(req)
{
}

void ChunkReq::createRequest(const string& torrentName, const int chunkNum)
{
	this->torrentName = torrentName;
	this->chunkNum = chunkNum;
	createRequest();
}

void ChunkReq::createRequest()
{
	string prefix = RRPacket::commString + RRPacket::commSeparator;
	string request = prefix + to_string(static_cast<int>(RRPacket::chunk));
	request += RRPacket::commSeparator + this->torrentName;
	request += RRPacket::commSeparator + to_string(this->chunkNum);

	std::copy(request.begin(), request.end(), std::back_inserter(req));
}

void ChunkReq::processRequest()
{
	string torrentName = "";
	int chunkNum = -1, size = 0;
	vector<char> chunk;
	string strReq {this->req.begin(), this->req.end()};

	getTorrentNameFromReq(torrentName);
	chunkNum = getChunkNumFromReq(strReq);
	retrieveChunk(torrentName, chunkNum, chunk, size);

	this->resp = chunk;
	this->size = size;

	cout << "processRequest " << this->size << " " << this->resp.size() << " " << chunkNum << endl;

	// testing purposes
	/*string filename = Torrent::getTorrentDataPath() + "download";
	ofstream fTorrent {filename};

	if (fTorrent.is_open()){
		fTorrent.write(chunk.data(), size);
 		fTorrent.close();
 		Torrent dowload{};
 		dowload.unpackage(filename);
	}
	else{
		cout << "Dump Error: error opening output torrent file" << endl;
	}*/
}

void ChunkReq::getTorrentNameFromReq(string& torrentName)
{
	vector<string> tokens;
	torrentName = "";
	string strReq {this->req.begin(), this->req.end()};

	Utils::tokenize(strReq, commSeparator, tokens);

	if (tokens.size() > 2){
		torrentName = tokens[2];
	}
}

int ChunkReq::getChunkNumFromReq(const string& req)
{
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

void ChunkReq::retrieveChunk(const string& torrentName, const int& chunkNum, vector<char>& chunk, int& size)
{
	Torrent torrent {torrentName};
	if (torrent.open())
		chunk = torrent.getChunk(chunkNum, size);
	else
		cout << "retreive chunk failed" << endl;
}

void ChunkReq::processResponse(const Message& msg)
{
	this->chunk = msg.data;
	this->size = msg.size;
	processResponse();
}

void ChunkReq::processResponse()
{
	Torrent torrent{torrentName};
	if (torrent.open())
	{
		torrent.name = torrentName + "!!!"; // for testing only
		if (!torrent.torrentDataExists())
			torrent.createTorrentDataFile();
		torrent.putChunk(this->chunk, this->size, this->chunkNum);
	}
	cout << "put " << this->chunk.size() << " " << this->size << " " << this->chunkNum << endl;
}