#pragma once

#include "Reactor.hpp"
#include "Util.hpp"
#include <cerrno>
#include <string>
#include <vector>

#define ONCE_SIZE 128

// 1: finish this round's read
//-1: meet mistakes
// 0: other side shut socket off
int RecverCore(int sock, std::string &inbuffer)
{
    while(true)
    {
        char buffer[ONCE_SIZE];
        ssize_t s = recv(sock, buffer, ONCE_SIZE-1, 0);
        if(s > 0)
        {
            buffer[s] = '\0';
            inbuffer += buffer;
        }
        else if(s < 0)
        {
            if(errno == EINTR)
            {
                // low chance, interrupted by signal
                continue;
            }
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 1. finish reading, no data left in system's read buffer
                return 1;
            }
            // 2. really went wrong
            return -1;
        }
        else
        {
            // s == 0
            return 0;
        }
    }
}

int Recver(Event *evp)
{
    std::cout << "Recver be called" << std::endl;

    // 1. real recv() func
    int result = RecverCore(evp->sock, evp->inbuffer);
    if(result <= 0)
    {
        if(evp->errorer)
        {
            evp->errorer(evp);
        }
        return -1;
    }

    //        1+2X3+4X5+6X
    // 2.deal with TCP sticky packets
    std::vector<std::string> tokens;
    std::string sep = "X";
    SplitSegment(evp->inbuffer, &tokens, sep);

    // 3.deserialize
    for(auto &seg: tokens)
    {
        std::string data1, data2;
        if(Deserialize(seg, &data1, data2))
        {
            // 4. get the result
            int x = atoi(data1.c_str());
            int y = atoi(data2.c_str());
            int z = x + y;

            // 5.make response -- put into evp->outbuffer
            // 2+3X -> 2+3=5X
            std::string res = data1;
            res += "+";
            res += data2;
            res += "=";
            res += std::to_string(z);
            res += sep;

            evp->outbuffer += res;  
        }
    }

    // 6.let Reactor to decide when should we actually send it
    if(!(evp->buffer).empty())
    {
        evp->R->EnableRW(evp->sock, true, true);
    }

    return 0;
}

// 1: sent all data successfully
// 0: didn't send all data but need to stop
//-1: sending failed
int SendCore(int sock, std::string &outbuffer)
{
    while(true)
    {
        int total = 0;  //total bytes be sent in one round
        const char* start = outbuffer.c_str();
        int size = outbuffer.size();
        ssize_t curr = send(sock, start + total, size - total, 0);
        if(curr > 0)
        {
            total += curr;
            if(total == size)
            {
                // sent all data
                outbuffer.clear();
                return 1;
            }
        }
        else
        {
            if(errno == EINTR)
            {
                continue;
            }
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // still has data remain but other side's write buffer is full
                outbuffer.erase(0, total);
                return 0;
            }

            return -1;
        }
    }
}

int Sender(Event *evp)
{
    std::cout << "Sender be called" << std::endl;

    int result = SenderCore(evp->sock, evp->outbuffer);
    if(result == 1)
    {
        evp->R->EnableRW(evp->sock, true, false);
    }
    else if(result == 0)
    {
        // can do nothing here
        evp->R->EnableRW(evp->sock, true, true);
    }
    else
    {
        if(evp->errorer)
        {
            evp->errorer(evp);
        }
    }
}

int Errorer(Event *evp)
{
    std::cout << "Errorer be called" << std::endl;
    evp->R->DeleteEvent(evp);
}