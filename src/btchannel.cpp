#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <iostream>

#include "btchannel.h"

#define DEBUG 0

const string BTChannel::zaddr = "00:00:00:00:00:00";
constexpr bdaddr_t BTChannel::bdAddrAny;

BTChannel::BTChannel()
{
	this->addr.rc_family = AF_BLUETOOTH;
    this->addr.rc_channel = 1;
    bacpy(&this->addr.rc_bdaddr, &bdAddrAny);
    this->clientAddr.rc_family = AF_BLUETOOTH;
    this->clientAddr.rc_channel = 1;
    str2ba(&this->zaddr[0], &this->clientAddr.rc_bdaddr);
}

BTChannel::BTChannel(const struct sockaddr_rc& addr)
{
    this->addr.rc_family = addr.rc_family;
    this->addr.rc_channel = addr.rc_channel;
    memcpy(&this->addr.rc_bdaddr, &addr.rc_bdaddr, sizeof(bdaddr_t));
    this->clientAddr.rc_family = AF_BLUETOOTH;
    this->clientAddr.rc_channel = 1;
    str2ba(&this->zaddr[0], &this->clientAddr.rc_bdaddr);
}

BTChannel::BTChannel(const string& addr)
{
    this->addr.rc_family = AF_BLUETOOTH;
    this->addr.rc_channel = 1;
    str2ba(&addr[0], &this->addr.rc_bdaddr);
    this->clientAddr.rc_family = AF_BLUETOOTH;
    this->clientAddr.rc_channel = 1;
    str2ba(&this->zaddr[0], &this->clientAddr.rc_bdaddr);
}

BTChannel::~BTChannel()
{
    //closeClient();
    //closeServer();
}

void BTChannel::setAdr(const struct sockaddr_rc& addr)
{
    this->addr.rc_family = addr.rc_family;
    this->addr.rc_channel = addr.rc_channel;
    memcpy(&this->addr.rc_bdaddr, &addr.rc_bdaddr, sizeof(bdaddr_t));
}

void BTChannel::setAdr(const string& addr)
{
    this->addr.rc_family = AF_BLUETOOTH;
    this->addr.rc_channel = 1;
    str2ba(&addr[0], &this->addr.rc_bdaddr);
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
	int status = ::connect(this->sock, reinterpret_cast<struct sockaddr*>(&this->addr), sizeof(this->addr));
    if (status == -1)
        cout << "Channel Error: Cannot connect to socket. " << errno << endl;
    return errno;
}

int BTChannel::writeToClient(const Message& msg)
{
    return write(this->clientSock, msg);
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
    return read(this->clientSock, msg);
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
    dev.sock = this->clientSock;
    vector<char> cAddr(18, 0);
    ba2str(&this->clientAddr.rc_bdaddr, &cAddr[0]);
    transform(cAddr.begin(), cAddr.end(), std::back_inserter(dev.addr),
               [](char c) {
                   return c;
                });
    return status;
}

int BTChannel::accept()
{
	socklen_t size = sizeof(this->clientAddr);
	this->clientSock = ::accept(this->sock, reinterpret_cast<struct sockaddr*>(&this->clientAddr), &size);
    if (this->clientSock == -1)
        cout << "Channel Error: Failed to accept message. " << errno << endl;
    return errno;
}

int BTChannel::closeClient()
{
    return close(this->clientSock);
}

int BTChannel::closeServer()
{
    return close(this->sock);
}

int BTChannel::close(int sock)
{
    int status = ::close(sock);
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
