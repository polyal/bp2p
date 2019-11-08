#include "rrpacket.h"
#include <vector>

class RRFactory 
{
public:
	static unique_ptr<RRPacket> create(const vector<char>& req);

private:
	static RRPacket::requestType getReqTypeFromReq(const vector<char>& req);
};