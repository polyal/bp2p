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
	static const string zaddr;
	static constexpr bdaddr_t bdAddrAny = {{0, 0, 0, 0, 0, 0}}; // replaces BDADDR_ANY from bluetooth.h

	Message omsg;  // output
	Message imsg;  // input
	int sock = -1;
	int clientSock = -1;
	struct sockaddr_rc addr;
	struct sockaddr_rc clientAddr;

public:
	BTChannel();
	BTChannel(const struct sockaddr_rc& addr);
	BTChannel(const string& addr);
	~BTChannel();

	void setAdr(const struct sockaddr_rc& addr);
	void setAdr(const string& addr);
	
	int salloc();
	int connect();
	
	int writeToClient(const Message& msg);
	int writeToServer(const Message& msg);
	int readFromClient(Message& msg);
	int readFromServer(Message& msg);

	int bind();
	int listen();
	int accept(DeviceDescriptor& dev);

	int closeClient();
	int closeServer();

protected:
	int write(int sock);
	int read(int sock);
	int write(int sock, const Message& msg);
	int read(int sock, Message& msg);

	int accept();

	int close(int sock);
};