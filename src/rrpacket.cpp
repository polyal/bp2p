#include "rrpacket.h"

class TorrentFileReq;


RRPacket::RRPacket()
{
}

RRPacket::RRPacket(const Message& req)
{
	this->req = req;
}

RRPacket::RRPacket(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr)
{
	this->remoteAddr = remoteAddr;
	this->localAddr = localAddr;
}

RRPacket::RRPacket(const DeviceDescriptor& remoteAddr, const DeviceDescriptor& localAddr, const Message& req)
{
	this->remoteAddr = remoteAddr;
	this->localAddr = localAddr;
	this->req = req;
}

Message RRPacket::getReq()
{
	return this->req;
}

Message RRPacket::getRsp()
{
	return this->rsp;
}
