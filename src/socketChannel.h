#ifndef SOCKETCHANNEL_H
#define SOCKETCHANNEL_H

#include "channel.h"

class SocketChannel : public Channel 
{
public:
	SocketChannel(){};
	virtual void connect() = 0;
	virtual void write(const Message& msg) = 0;
	virtual void read(Message& msg) = 0;
	virtual void bind() = 0;
	virtual void listen() = 0;
	virtual void accept(DeviceDescriptor& dev) = 0;
	virtual void close() = 0;

	virtual void setLocalAddress(unique_ptr<Address> localAddr) = 0;
	virtual void setRemoteAddress(unique_ptr<Address> remoteAddr) = 0;

protected:
	virtual void salloc() = 0;
	virtual void write(int socket, const Message& msg) = 0;
	virtual void read(int socket, Message& msg) = 0;
	virtual void close(int& socket) = 0;

	int localSocket = -1;
	int remoteSocket = -1;
	int activeSocket = -1;
};

#endif
