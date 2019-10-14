#include <vector>
#include <string>
#include "channel.h"

using namespace std;


class BTChannel : public Channel
{
public:
	struct Message
	{
		Message()
		{
			size = chunkSize;
			data.resize(size);
		}

		Message(vector<char> m, unsigned int s)
		{
			if (m.size() > chunkSize || s > chunkSize || s > m.size()) return; // error
			size = s;
			data = m;
		}

		void create(vector<char> m, unsigned int s)
		{
			if (m.size() > chunkSize || s > chunkSize || s > m.size()) return; // error
			size = s;
			data = m;
		}

		Message& operator=(const Message& msg)
		{
			this->size = msg.size;
			this->data = data;
			return *this;
		}

		unsigned int size = 0;
		vector<char> data;
	};

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
	
	void salloc();
	void connect();
	
	void write();
	void read();

	void bind();
	void listen();
	void accept();
};