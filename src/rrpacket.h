#ifndef RRPACKET_H
#define RRPACKET_H

#include <vector>
#include <string>
#include <memory>
#include "message.h"
#include "deviceDescriptor.h"

using namespace std;


class RRPacket 
{
public:
	inline static const string commString = "bp2p";
	inline static const string commSeparator = "||";

	enum RequestType
	{
		BadReq,
		TorrentList,
		TorrentFile,
		TorrentAvailability,
		Chunk
	};

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

protected:
	Message req;
	Message rsp;
	DeviceDescriptor localAddr;
	DeviceDescriptor remoteAddr;
	RequestType type;
};

#endif
