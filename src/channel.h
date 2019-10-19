

class Channel
{
public:
	virtual int salloc() = 0;
	virtual int connect() = 0;
	virtual int write(int sock) = 0;
	virtual int read(int sock) = 0;
	virtual int bind() = 0;
	virtual int listen() = 0;
	virtual int accept() = 0;
	virtual int close(int sock) = 0;
};