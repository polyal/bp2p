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
		this->data = m;
		this->size = s;
	}

	Message(string m)
	{
		copy(m.begin(), m.end(), std::back_inserter(this->data));
		this->size = data.size();
	}

	void create(vector<char> m, unsigned int s)
	{
		this->data = m;
		this->size = s;
	}

	void create(string m)
	{
		copy(m.begin(), m.end(), std::back_inserter(this->data));
		this->size = m.size();
	}

	void clear()
	{
		this->data.clear();
		this->size = 0;
	}

	Message& operator=(const Message& msg)
	{
		this->data = msg.data;
		this->size = msg.size;
		return *this;
	}

	unsigned int size = 0;
	vector<char> data;
};

#endif
