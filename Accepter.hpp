#pragma once

#include "Reactor.hpp"
#include "Sock.hpp"
#include "Service.hpp"
#include "Util.hpp"

int Accepter(Event *evp)
{
    std::cout << "A new sock has arrived, sock: " << evp->sock << std::endl;
    while(true)
    {
        int sock = Sock::Accept(evp->sock);
        if(sock < 0)
        {
            std::cout << "Accept Done! " << std::endl;
            break;
        }
        std::cout << "Accept success: " << sock << std::endl;
        SetNonBlock(sock);

        Event *other_ev = new Event();
        other_ev->sock = sock;
        other_ev->R = evp->R;
        other_ev->RegisterCallback(Recver, Sender, Errorer);

        evp->R->InsertEvent(other_ev, EPOLLIN|EPOLLET);
    }
}