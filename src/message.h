#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>

using namespace std;

struct Message
{	
	Message()
	{
	}

	Message(vector<char> m, unsigned int s)
	{
		size = s;
		data = m;
	}

	void create(vector<char> m, unsigned int s)
	{
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

#endif