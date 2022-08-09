#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

// utilizers

// make a socket non-block
void SetNonBlock(int sock)
{
    int fl = fcntl(sock,F_GETFL);
    if(fl < 0)
    {
        std::cerr << "fcntl failed" << std::endl;
        return;
    }
    fcntl(sock, F_SETFL, fl | O_NONBLOCK);
}

// 3+4X5+6X
void SplitSegment(std::string &inbuffer, std::vector<std::string> *tokens, std::string sep)
{
    while(true)
    {
        std::cout << "inbuffer: " << inbuffer << std::endl;
        auto pos inbuffer.find(sep);
        if(pos == std::string::npos)
        {
            break;
        }
        std::string sub = inbuffer.substr(0,pos);
        tokens->push_back(sub);
        inbuffer.erase(0, pos+sep.size());
    }
}

bool Deserialize(std::string &seg, std::string *out1, std::striing *out2)
{
    // 1 + 2
    std::string op = "+";
    auto pos = seg.find(op);
    if(pos == std::string::npos)
        return false;
    *out1 = seg.substr(0,pos);
    *out2 = seg.substr(pos+op.size());
    return true;
}