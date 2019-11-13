#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include "message.h"
#include "deviceDescriptor.h"
#include "channel.h"

using namespace std;

class BTChannel : public Channel
{
private:
	static const unsigned int chunkSize = 32768;
	static const unsigned int btChunk = 1008;  // bluetooth transfers data in chuncks of 1008 bytes
	inline static const string zaddr = "00:00:00:00:00:00";
	inline static constexpr bdaddr_t bdAddrAny = {{0, 0, 0, 0, 0, 0}}; // replaces BDADDR_ANY from bluetooth.h

	Message omsg;  // output
	Message imsg;  // input
	int sock = -1;
	int remoteSock = -1;
	struct sockaddr_rc addr;
	struct sockaddr_rc remoteAddr;

	static const int timeout = 2;  // in seconds
	void setReusePort();
	void setTimeout();

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
	
	void writeToClient(const Message& msg);
	void writeToServer(const Message& msg);
	void readFromClient(Message& msg);
	void readFromServer(Message& msg);

	void bind();
	void listen();
	void accept(DeviceDescriptor& dev);

	void closeRemote();
	void close();

protected:
	void write(int sock);
	void read(int sock);
	void write(int sock, const Message& msg);
	void read(int sock, Message& msg);

	void accept();

	void close(int& sock);
};