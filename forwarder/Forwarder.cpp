#include"Forwarder.h"

Forwarder::Forwarder(key_t key,SocketAddress &destAddr)
    :msgQue_(key),sock_()
{
    sock_.connect(destAddr);
}

Forwarder::~Forwarder(){
    try{
        msgQue_.deleteQueue();
        sock_.close();
    }catch(...){
    }
}

void Forwarder::forward(){
    while(true){
        Message<char,RAW_MSG_SIZE> msg;
        int msglen = msgQue_.pop<RAW_MSG_SIZE>(msg);
        SocketBuffer sockBuf(msglen+8);
        sockBuf.copyFrom(static_cast<void *>&msg,msglen);
    }
}
