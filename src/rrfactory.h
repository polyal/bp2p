#include "rrpacket.h"
#include "message.h"
#include <vector>

class RRFactory 
{
public:
	static unique_ptr<RRPacket> create(const Message& req);

private:
	static RRPacket::requestType getReqTypeFromReq(const Message& req);
};