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

void BTChannel::salloc()
{
    this->sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (this->sock == -1){
        cout << "Channel Error: Cannot allocate socket. " << errno << endl;
        throw errno;
    }
}

void BTChannel::connect()
{
	int status = ::connect(this->sock, reinterpret_cast<struct sockaddr*>(&this->remoteAddr), sizeof(this->remoteAddr));
    if (status == -1){
        cout << "Channel Error: Cannot connect to socket. " << errno << endl;
        throw errno;
    }
}

void BTChannel::writeToClient(const Message& msg)
{
    try{
        write(this->remoteSock, msg);
    }
    catch(...){
        throw;
    }
}

void BTChannel::writeToServer(const Message& msg)
{
    try{
        write(this->sock, msg);
    }
    catch(...){
        throw;
    }
}

void BTChannel::write(int sock, const Message& msg)
{
    this->omsg = msg;
    try{
        write(sock);
    }
    catch(...){
        throw;
    }
}

void BTChannel::write(int sock)
{
	int status = ::write(sock, this->omsg.data.data(), this->omsg.size);
    if( status == -1 ){
        cout << "Channel Error: Write error. " << errno << endl;
        throw errno;
    }
}

void BTChannel::readFromClient(Message& msg)
{
    try{
        read(this->remoteSock, msg);
    }
    catch(...){
        throw;
    }
}

void BTChannel::readFromServer(Message& msg)
{
    try{
        read(this->sock, msg);
    }
    catch(...){
        throw;
    }
}

void BTChannel::read(int sock, Message& msg)
{
    try{
        read(sock);
    }
    catch(...){
        throw;
    }
    msg = this->imsg;
}

void BTChannel::read(int sock)
{
    vector<char> tmpMsg(this->btChunk);
    int totalSize = 0;
    int bytesRead = 0;
    this->imsg.clear();
	while ((bytesRead = ::read(sock, tmpMsg.data(), this->btChunk)) > 0){
        this->imsg.data.insert(this->imsg.data.end(), tmpMsg.begin(), tmpMsg.begin() + bytesRead);
        totalSize += bytesRead;
        if (bytesRead < (int)this->btChunk || (errno != 0 && errno != ENOENT))
            break;
    }
    if( bytesRead == -1 ){
        cout << "Channel Error: Failed to read message. " << errno << endl;
        if (errno != ECONNRESET)
            throw errno;
    }
    //string rspp{this->imsg.data.begin(), this->imsg.data.end()};
    //cout << "Read Msg: " << rspp << endl;
    this->imsg.size = totalSize;
    this->imsg.data.resize(totalSize);
}

void BTChannel::setReusePort()
{
    int enable = 1;
    if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0){
        cout << "Channel Error: Couldn't set reuse ort" << endl;
    }
}

void BTChannel::bind()
{
    setReusePort();
	int status = ::bind(this->sock, reinterpret_cast<struct sockaddr*>(&this->addr), sizeof(this->addr));
    if (status == -1){
        cout << "Channel Error: Cannot bind name to socket. " << errno << endl;
        throw errno;
    }
}

void BTChannel::setTimeout()
{
    struct timeval timeout;      
    timeout.tv_sec = BTChannel::timeout;
    timeout.tv_usec = 0;

    if (setsockopt (this->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        cout << "Channel Error: Couldn't set timeout" << endl;
    }
}

void BTChannel::listen()
{
    setTimeout();
	int status = ::listen(this->sock, 1);
    if (status == -1){
        cout << "Channel Error: Cannot listen for connections on socket. " << errno << endl;
        throw errno;
    }
}

void BTChannel::accept(DeviceDescriptor& dev)
{
    try{
        accept();
    }
    catch(...){
        throw;
    }
    vector<char> cAddr(18, 0);
    ba2str(&this->remoteAddr.rc_bdaddr, &cAddr[0]);
    transform(cAddr.begin(), cAddr.end(), std::back_inserter(dev.addr),
               [](char c) {
                   return c;
                });
}

void BTChannel::accept()
{
	socklen_t size = sizeof(this->remoteAddr);
	this->remoteSock = ::accept(this->sock, reinterpret_cast<struct sockaddr*>(&this->remoteAddr), &size);
    if (this->remoteSock == -1){
        cout << "Channel Error: Failed to accept message. " << errno << endl;
        throw errno;
    }
}

void BTChannel::closeRemote()
{
    try{
        close(this->remoteSock);
    }
    catch(...){
        throw;
    }
}

void BTChannel::close()
{
    try{
        close(this->sock);
    }
    catch(...){
        throw;
    }
}

void BTChannel::close(int& sock)
{
    int status = sock >= 0 ? ::close(sock) : 0;
    sock = -1;
    if (status == -1){
        cout << "Channel Error: Something went wring closing the socket. " << errno << endl;
        throw errno;
    }
}


#if DEBUG == 1
int main()
{
	return 0;
}
#endif
