#include "rrpacket.h"

class TorrentFileReq;


RRPacket::RRPacket()
{
}

RRPacket::RRPacket(const Message& req)
{
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
