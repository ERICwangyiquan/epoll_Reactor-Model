#include "Reactor.hpp"
#include "Sock.hpp"
#include "Accepter.hpp"
#include "Util.hpp"

void Usage(std::string proc)
{
    std::cout << "Usage: " << proc << " port" << std::endl;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    // 1. create new socket, listen
    int listen_sock = Sock::Socket();
    SetNonBlock(listen_sock);
    Sock::Bind(listen_sock, (uint16_t)atoi(argv[1]));
    Sock::Listen(listen_sock);

    // 2. create Reactor instance
    Reactor *R = new Reactor();
    R->InitRreactor();

    // 3.1 insert listen_sock
    Event *evp = new Event;
    evp->sock = listen_sock;
    evp->R = R;
    evp->RegisterCallback(Accepter, nullptr, nullptr);  //Accepter is the recver here

    // 3.2 insert into Reactor
    R->InsertEvent(evp, EPOLLIN|EPOLLET);

    // 4.dispatch begin
    int timeout = 1000;
    for( ; ; )
    {
        R->Dispatcher(timeout);
    }
}