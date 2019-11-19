#ifndef RRPACKET_H
#define RRPACKET_H

// request reponse packet class
#include <vector>
#include <string>
#include <memory>
#include "message.h"
#include "deviceDescriptor.h"

using namespace std;

class RRPacket 
{
protected:
	Message req;
	Message rsp;
	DeviceDescriptor localAddr;
	DeviceDescriptor remoteAddr;

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
	RRPacket(const Message& req);
	RRPacket(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr);
	RRPacket(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req);

	DeviceDescriptor getLocalAddr() const;
	DeviceDescriptor getRemoteAddr() const;

	Message getReq();
	Message getRsp();

	virtual void createRequest() = 0;
	virtual void processRequest() = 0;
	virtual void processResponse(const Message& rsp) = 0;
};

#endif
