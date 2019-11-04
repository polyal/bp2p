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
	inline static const string zaddr = "00:00:00:00:00:00";
	inline static constexpr bdaddr_t bdAddrAny = {{0, 0, 0, 0, 0, 0}}; // replaces BDADDR_ANY from bluetooth.h

	Message omsg;  // output
	Message imsg;  // input
	int sock = -1;
	int remoteSock = -1;
	struct sockaddr_rc addr;
	struct sockaddr_rc remoteAddr;

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
	
	int salloc();
	int connect();
	
	int writeToClient(const Message& msg);
	int writeToServer(const Message& msg);
	int readFromClient(Message& msg);
	int readFromServer(Message& msg);

	int bind();
	int listen();
	int accept(DeviceDescriptor& dev);

	int closeRemote();
	int close();

protected:
	int write(int sock);
	int read(int sock);
	int write(int sock, const Message& msg);
	int read(int sock, Message& msg);

	int accept();

	int close(int sock);
};