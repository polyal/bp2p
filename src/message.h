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

	Message(string m)
	{
		copy(m.begin(), m.end(), std::back_inserter(data));
		size = data.size();
	}

	void create(vector<char> m, unsigned int s)
	{
		size = s;
		data = m;
	}

	void create(string m)
	{
		size = m.size();
		copy(m.begin(), m.end(), std::back_inserter(data));
	}

	void clear()
	{
		this->size = 0;
		this->data.clear();
	}

	Message& operator=(const Message& msg)
	{
		this->size = msg.size;
		this->data = msg.data;
		return *this;
	}

	unsigned int size = 0;
	vector<char> data;
};

#endif
