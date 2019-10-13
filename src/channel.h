

class Channel
{
public:
	virtual void salloc() = 0;
	virtual void connect() = 0;
	virtual void write() = 0;
	virtual void read() = 0;
	virtual void bind() = 0;
	virtual void listen() = 0;
	virtual void accept() = 0;
};