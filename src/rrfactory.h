#include <vector>
#include "message.h"
#include "rrpacket.h"

class RRFactory 
{
public:
	static unique_ptr<RRPacket> create(const Message& req);

private:
	static RRPacket::RequestType getReqTypeFromReq(const Message& req);
};
