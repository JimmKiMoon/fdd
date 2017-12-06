#ifndef FORWARDER_H
#define FORWARDER_H
#include"MessageQueue.h"
#include"TCPSocket.h"

class Forwarder{
private:
    MessageQueue msgQue_;
    TCPSocket sock_;
public:
    Forwarder(key_t key,SocketAddress &destAddr);
    ~Forwarder();
public:
    void forward();
};

#endif // FORWARDER_H
