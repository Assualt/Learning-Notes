//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_SOCKET_TYPEDEF_H
#define SSP_TOOLS_SOCKET_TYPEDEF_H

#include <cstdint>

namespace ssp::net {

struct NormalSocket {
public:
    virtual bool Close()
    {
        return true;
    }

    virtual void Init(int32_t fd, void *args)
    {
        fd_ = fd;
    }

private:
    int32_t fd_{-1};
};

struct SocketSsl : public NormalSocket {
public:
    bool Close() override
    {
        NormalSocket::Close();
        return true;
    }

    void Init(int32_t fd, void *args) override
    {
        NormalSocket::Init(fd, args);
        InitSslOptions(fd, args);
    }

private:
    void InitSslOptions(int32_t fd, void *args)
    {

    }
};

}

#endif //SSP_TOOLS_SOCKET_TYPEDEF_H
