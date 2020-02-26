#ifndef CHANNEL_H
#define CHANNEL_H

#include <memory>
#include "message.h"
#include "address.h"
#include "deviceDescriptor.h"

class Channel
{
public:
	Channel(){};
	virtual void salloc() = 0;
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
	unique_ptr<Address> localAddr;
	unique_ptr<Address> remoteAddr;
};

#endif
