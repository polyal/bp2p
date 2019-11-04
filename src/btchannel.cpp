#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <iostream>

#include "btchannel.h"

#define DEBUG 0


BTChannel::BTChannel()
{
	this->addr.rc_family = AF_BLUETOOTH;
    this->addr.rc_channel = 1;
    bacpy(&this->addr.rc_bdaddr, &bdAddrAny);
    this->remoteAddr.rc_family = AF_BLUETOOTH;
    this->remoteAddr.rc_channel = 1;
    bacpy(&this->remoteAddr.rc_bdaddr, &bdAddrAny);
}

BTChannel::BTChannel(const struct sockaddr_rc& addr)
{
    this->addr.rc_family = addr.rc_family;
    this->addr.rc_channel = addr.rc_channel;
    bacpy(&this->addr.rc_bdaddr, &addr.rc_bdaddr);
    this->remoteAddr.rc_family = AF_BLUETOOTH;
    this->remoteAddr.rc_channel = 1;
    bacpy(&this->remoteAddr.rc_bdaddr, &bdAddrAny);
}

BTChannel::BTChannel(const string& addr, unsigned int ch)
{
    this->addr.rc_family = AF_BLUETOOTH;
    this->addr.rc_channel = ch;
    str2ba(&addr[0], &this->addr.rc_bdaddr);
    this->remoteAddr.rc_family = AF_BLUETOOTH;
    this->remoteAddr.rc_channel = 0;
    bacpy(&this->remoteAddr.rc_bdaddr, &bdAddrAny);
}

BTChannel::~BTChannel()
{
    //closeClient();
    //closeServer();
}

void BTChannel::setCh(const struct sockaddr_rc& addr)
{
    this->addr.rc_family = addr.rc_family;
    this->addr.rc_channel = addr.rc_channel;
    bacpy(&this->addr.rc_bdaddr, &addr.rc_bdaddr);
}

void BTChannel::setCh(const string& addr, unsigned int ch)
{
    this->addr.rc_family = AF_BLUETOOTH;
    this->addr.rc_channel = ch;
    str2ba(&addr[0], &this->addr.rc_bdaddr);
}

void BTChannel::setCh(unsigned int ch)
{
    this->addr.rc_channel = ch;
}

void BTChannel::setRemoteCh(const struct sockaddr_rc& addr)
{
    this->remoteAddr.rc_family = addr.rc_family;
    this->remoteAddr.rc_channel = addr.rc_channel;
    bacpy(&this->remoteAddr.rc_bdaddr, &addr.rc_bdaddr);
}

void BTChannel::setRemoteCh(const string& addr, unsigned int ch)
{
    this->remoteAddr.rc_family = AF_BLUETOOTH;
    this->remoteAddr.rc_channel = ch;
    str2ba(&addr[0], &this->remoteAddr.rc_bdaddr);
}

void BTChannel::setRemoteCh(unsigned int ch)
{
    this->remoteAddr.rc_channel = ch;
}

int BTChannel::salloc()
{
    this->sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (this->sock == -1)
        cout << "Channel Error: Cannot allocate socket. " << errno << endl;
    return errno;
}

int BTChannel::connect()
{
	int status = ::connect(this->sock, reinterpret_cast<struct sockaddr*>(&this->remoteAddr), sizeof(this->remoteAddr));
    if (status == -1)
        cout << "Channel Error: Cannot connect to socket. " << errno << endl;
    return errno;
}

int BTChannel::writeToClient(const Message& msg)
{
    return write(this->remoteSock, msg);
}

int BTChannel::writeToServer(const Message& msg)
{
    return write(this->sock, msg);
}

int BTChannel::write(int sock, const Message& msg)
{
    this->omsg = msg;
    int status = write(sock);
    return status;
}

int BTChannel::write(int sock)
{
	int status = ::write(sock, this->omsg.data.data(), this->omsg.size);
    if( status == -1 )
        cout << "Channel Error: Write error. " << errno << endl;
    return errno;
}

int BTChannel::readFromClient(Message& msg)
{
    return read(this->remoteSock, msg);
}

int BTChannel::readFromServer(Message& msg)
{
    return read(this->sock, msg);
}

int BTChannel::read(int sock, Message& msg)
{
    int status = read(sock);
    msg = this->imsg;
    return status;
}

int BTChannel::read(int sock)
{
    vector<char> tmpMsg(this->chunkSize);
	int bytesRead = ::read(sock,tmpMsg.data(), this->chunkSize);
    if( bytesRead == -1 )
        cout << "Channel Error: Failed to read message. " << errno << endl;
    this->imsg.data = tmpMsg;
    this->imsg.size = bytesRead;
    return errno;
}

int BTChannel::bind()
{
	int status = ::bind(this->sock, reinterpret_cast<struct sockaddr*>(&this->addr), sizeof(this->addr));
    if (status == -1)
        cout << "Channel Error: Cannot bind name to socket. " << errno << endl;
    return errno;
}

int BTChannel::listen()
{
	int status = ::listen(this->sock, 1);
    if (status == -1)
        cout << "Channel Error: Cannot listen for connections on socket. " << errno << endl;
    return errno;
}

int BTChannel::accept(DeviceDescriptor& dev)
{
    int status = accept();
    vector<char> cAddr(18, 0);
    ba2str(&this->remoteAddr.rc_bdaddr, &cAddr[0]);
    transform(cAddr.begin(), cAddr.end(), std::back_inserter(dev.addr),
               [](char c) {
                   return c;
                });
    return status;
}

int BTChannel::accept()
{
	socklen_t size = sizeof(this->remoteAddr);
	this->remoteSock = ::accept(this->sock, reinterpret_cast<struct sockaddr*>(&this->remoteAddr), &size);
    if (this->remoteSock == -1)
        cout << "Channel Error: Failed to accept message. " << errno << endl;
    return errno;
}

int BTChannel::closeRemote()
{
    return ::close(this->remoteSock);
}

int BTChannel::close()
{
    return ::close(this->sock);
}

int BTChannel::close(int sock)
{
    int status = sock >= 0 ? ::close(sock) : 0;
    if (status == -1)
        cout << "Channel Error: Something went wring closing the socket. " << errno << endl;
    return errno;
}


#if DEBUG == 1
int main()
{
	return 0;
}
#endif
