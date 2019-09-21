#ifndef RRPACKET_H
#define RRPACKET_H

// request reponse packet class
#include <vector>
#include <string>
#include <memory>

using namespace std;

class RRPacket {
	public:
		static const int chunkSize = 32768;  //256 kilobyte chunk size
		static const string commString;
		static const string commSeparator;
		vector<char> req;
		vector<char> resp;

		typedef enum _requestType {
			badReq,
			torrentList,
			torrentFile, 
			chunk
		} requestType;

		RRPacket();
		RRPacket(const vector<char>& req);

		vector<char> getResp();

		virtual void processRequest() = 0;
};

#endif
