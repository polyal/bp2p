#ifndef RRPACKET_H
#define RRPACKET_H

// request reponse packet class
#include <vector>
#include <string>
#include <memory>

using namespace std;

class RRPacket 
{
protected:
	static const int chunkSize = 32768;  //256 kilobyte chunk size
	vector<char> req;
	vector<char> resp;

public:
	inline static const string commString = "bp2p";
	inline static const string commSeparator = "||";

	typedef enum _requestType {
		badReq,
		torrentList,
		torrentFile, 
		chunk
	} requestType;

	RRPacket();
	RRPacket(const vector<char>& req);

	vector<char> getReq();
	vector<char> getResp();

	virtual void createRequest() = 0;
	virtual void processRequest() = 0;
	virtual void processResponse() = 0;
};

#endif
