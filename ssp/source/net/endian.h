//
// Created by 侯鑫 on 2024/1/2.
//

#ifndef SSP_TOOLS_ENDIAN_H
#define SSP_TOOLS_ENDIAN_H

#ifdef LINUX
#include <endian.h>
#endif
#include <cstdint>

namespace ssp::net::sockets {

inline uint64_t HostToNetwork64(uint64_t host64)
{
#ifdef LINUX
    return htobe64(host64);
#else
    return 0;
#endif
}

inline uint32_t HostToNetwork32(uint32_t host32)
{
#ifdef LINUX
    return htobe32(host32);
#else
    return 0;
#endif
}

inline uint16_t HostToNetwork16(uint16_t host16)
{
#ifdef LINUX
    return htobe16(host16);
#else
    return 0;
#endif
}

inline uint64_t NetworkToHost64(uint64_t net64)
{
#ifdef LINUX
    return be64toh(net64);
#else
    return 0;
#endif
}

inline uint32_t NetworkToHost32(uint32_t net32)
{
#ifdef LINUX
    return be32toh(net32);
#else
    return 0;
#endif
}

inline uint16_t NetworkToHost16(uint16_t net16)
{
#ifdef LINUX
    return be16toh(net16);
#else
    return 0;
#endif
}
}

#endif //SSP_TOOLS_ENDIAN_H
