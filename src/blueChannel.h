#include <vector>
#include <string>
#include "channel.h"

using namespace std;

struct Message
{
	int size = 0;
	vector<char> data;
};


class BTChannel : public Channel
{
private:
	static const int chunkSize = 32768;
	static const string zaddr;

	Message omsg;  // output
	Message imsg;  // input
	int sock = -1;
	struct sockaddr_rc addr;
	struct sockaddr_rc clientAddr;

public:
	BTChannel();
	
	void salloc();
	void connect();
	
	void write();
	void read();

	void bind();
	void listen();
	void accept();
};