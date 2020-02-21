#include "message.h"

class Channel
{
public:
	virtual void salloc() = 0;
	virtual void connect() = 0;
	virtual void write(const Message& msg) = 0;
	virtual void read(Message& msg) = 0;
	virtual void bind() = 0;
	virtual void listen() = 0;
	virtual void accept() = 0;
	virtual void close() = 0;
};