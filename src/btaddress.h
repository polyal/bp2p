#ifndef BTADDRESS_H
#define BTADDRESS_H

#include "address.h"

class BTAddress : public Address
{
public:
	BTAddress(const BTAddress& addr) : Address{addr}
	{
		this->channel = addr.channel;
		this->btAddr = addr.btAddr;
	}

	BTAddress(const string& addr, unsigned int channel) : Address{addr}
	{
		this->channel = channel;
		this->btAddr.rc_family = AF_BLUETOOTH;
    	this->btAddr.rc_channel = channel;
    	str2ba(&addr[0], &this->btAddr.rc_bdaddr);
    	this->addr = &btAddr;
	}

	string getStrAddr() const;
	void* get();
	unsigned int getSize() const {return sizeof(btAddr);}

private:
	int channel = 0;
	struct sockaddr_rc btAddr;
};

#endif
