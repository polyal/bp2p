#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include "deviceDescriptor.h"
#include "socketChannel.h"

using namespace std;

class BTChannel : public SocketChannel
{
public:
	BTChannel();
	BTChannel(const struct sockaddr_rc& addr);
	BTChannel(const string& addr, unsigned int ch = 1);
	~BTChannel();

	void setCh(const struct sockaddr_rc& addr);
	void setCh(const string& addr, unsigned int ch = 1);
	void setCh(unsigned int ch);

	void setRemoteCh(const struct sockaddr_rc& addr);
	void setRemoteCh(const string& addr, unsigned int ch = 1);
	void setRemoteCh(unsigned int ch);
	
	void salloc();
	void connect();
	
	void write(const Message& msg);
	void read(Message& msg);

	void bind();
	void listen();
	void accept(DeviceDescriptor& dev);

	void close();

	static int getTimeout();

protected:
	void write(int socket, const Message& msg);
	void read(int socket, Message& msg);
	void write(int socket);
	void read(int socket);

	void accept();

	void closeRemote();
	void closeLocal();
	void close(int& sock);

private:
	static const unsigned int chunkSize = 32768;
	static const unsigned int btChunk = 1008;  // bluetooth transfers data in chuncks of 1008 bytes
	static const string zaddr;
	static constexpr bdaddr_t bdAddrAny = {{0, 0, 0, 0, 0, 0}}; // replaces BDADDR_ANY from bluetooth.h

	Message omsg;  // output
	Message imsg;  // input
	struct sockaddr_rc addr;
	struct sockaddr_rc remoteAddr;

	static const int timeout = 2;  // in seconds
	void setReusePort();
	void setTimeout();
};
