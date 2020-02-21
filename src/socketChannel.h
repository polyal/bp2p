#ifndef SOCKETCHANNEL_H
#define SOCKETCHANNEL_H

#include "channel.h"

class SocketChannel : public Channel 
{
public:
	virtual void connect() = 0;
	virtual void write(const Message& msg) = 0;
	virtual void read(Message& msg) = 0;
	virtual void bind() = 0;
	virtual void listen() = 0;
	virtual void accept() = 0;
	virtual void close() = 0;
protected:
	virtual void salloc() = 0;
	virtual void write(int socket, const Message& msg) = 0;
	virtual void read(int socket, Message& msg) = 0;
	virtual void close(int& socket) = 0;

	int localSocket;
	int remoteSocket;
	int activeSocket;
};

#endif
