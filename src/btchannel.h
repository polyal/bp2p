#include <string>
#include "message.h"
#include "channel.h"

using namespace std;

class BTChannel : public Channel
{
private:
	static const unsigned int chunkSize = 32768;
	static const string zaddr;

	Message omsg;  // output
	Message imsg;  // input
	int sock = -1;
	struct sockaddr_rc addr;
	struct sockaddr_rc clientAddr;

public:
	BTChannel();
	BTChannel(struct sockaddr_rc addr, const Message& msg = Message{});
	BTChannel(const string& addr, const Message& msg = Message{});
	~BTChannel();
	
	void salloc();
	void connect();
	
	void write();
	void read();

	void bind();
	void listen();
	void accept();

	void end();
};