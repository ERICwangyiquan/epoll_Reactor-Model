#pragma once

#include <iostream>
#include <string
#include <unordered_map>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>

#define SIZE 128
#define NUM 64

class Event;
class Reactor;

typedef int (*callback_t)(Event *ev);

// Node managed by Reactor
class Event
{
public:
    // file descriptor
    int sock;
    // sock's inbuffer
    std::string inbuffer;
    // sock's outbuffer
    std::string outbuffer;
    // callbacks
    callback_t recver;
    callback_t sender;
    callback_t errorer;

    // point back to Reactor to use it's member funcs
    Reactor *R;

public:
    Event()
    {
        sock = -1;
        recver = nullptr;
        sender = nullptr;
        errorer = nullptr;
        R = nullptr;
    }

    void RegisterCallback(callback_t _recver, callback_t _sender, callback_t _errorer)
    {
        recver = _recver;
        sender = _sender;
        errorer = _errorer;
    }

    ~Event()
    {
    }
};

// Reactor : Event = 1 : n;
class Reactor
{
private:
    int epfd;
    std::unordered_map<int, Event *> events;    //a set for all events

public:
    Reactor() : epfd(-1)
    {
    }

    void InitRreactor() // create epoll model
    {
        epfd = epoll_create(SIZE);
        if(epfd < 0)
        {
            std::cerrr << "epoll_create error" << std::endl;
            exit(2);
        }
        std::cout << "InitReactor success" << std::endl;
    }

    bool InsertEvent(Event *evp, uint32_t evs)
    {
        // 1.insert sock into epoll model
        struct epoll_event ev;
        ev.events = evs;
        ev.data.fd = evp->sock;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, evp->sock, &ev) < 0)
        {
            std::cerr << "epoll_ctl add event failed" << std::endl;
            return false;
        }
        // 2.insert sock into unordered_map
        events.inserts({evp->sock, evp});
    }

    void DeleteEvent(Event *evp)
    {
        int sock = evp->sock;
        auto iter = events.find(sock);
        if(iter != events.end())
        {
            // 1.delete from epoll
            epoll_ctl(epfd, EPOLL_CTL_DEL, sock);

            // 2.delete from unordered_map
            events.erase(iter);

            // 3.close socket
            close(sock);

            // 4.delete Event Node
            delete evp;
        }
    }

    // Modify  (EPOLL_CTL_MOD)
    bool EnableRW(int sock, bool enable_read, bool enable_write)
    {
        struct epoll_event ev;
        ev.events = EPOLLET | (enable_read? EPOLLIN : 0) | (enable_write? EPOLLOUT : 0);
        ev.data.fd = sock;

        if(epoll_ctl(epfd, EPOLL_CTL_MOD, sokc, &ev) < 0)
        {
            std::cerr << "epoll_ctl mod event failed" << std::endl;
            return false;
        }
    }

    bool IsSockOK(int sock)
    {
        auto iter = events.find(sock);
        return iter != events.end();
    }

    // dispatch ready sock
    void Dispatcher(int timeout)
    {
        struct epoll_event revs[NUM];
        int n = epoll_wait(epfd, revs, NUM, timeout);
        for(int i=0; i<n; i++)
        {
            int sock = revs[i].data.fd;
            uint32_t revents = revs[i].events;

            // let IO-funcs to deal with the errors
            if(revents & EPOLLERR)
            {
                revents |= (EPOLLIN | EPOLLOUT);
            }
            if(revents & EPOLLHUP)
            {
                revents |= (EPOLLIN | EPOLLOUT);
            }

            // call callback func
            if(revents & EPOLLIN)
            {
                if(IsSockOK(sock) && events[sock]->recver)
                    events[sock]->recver(events[sock]);
            }
            if(revents & EPOLLOUT)
            {
                if(IsSockOK(sock) && events[sock]->sender)
                    events[sock]->sender(events[sock]);
            }
        }
    }

    ~Reactor()
    {}
};