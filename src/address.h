#ifndef ADDRESS_H
#define ADDRESS_H

class Address
{
public:
	Address(){}
	Address(const Address& addr)
	{
		this->strAddr = addr.strAddr;
		this->addr = addr.addr;
	}

	Address(const string& addr){this->strAddr = addr;}

	string getStrAddr() const {return strAddr;}
	void* get() {return addr;};
	virtual unsigned int getSize() const = 0;

protected:
	string strAddr;
	void* addr = nullptr;
};

#endif
