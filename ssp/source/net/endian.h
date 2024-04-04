//
// Created by 侯鑫 on 2024/1/2.
//

#ifndef SSP_TOOLS_ENDIAN_H
#define SSP_TOOLS_ENDIAN_H

#if defined(LINUX)
#include <endian.h>
#elif defined(MAC_OS)
#include <machine/endian.h>
#endif

#include <cstdint>

namespace ssp::net::sockets {

inline uint64_t HostToNetwork64(uint64_t host64)
{
#if defined(LINUX)
    return htobe64(host64);
#elif defined(MAC_OS)
    return htonll(host64);
#else
    return 0;
#endif
}

inline uint32_t HostToNetwork32(uint32_t host32)
{
#if defined(LINUX)
    return htobe32(host32);
#elif defined(MAC_OS)
    return htonl(host32);
#else
    return 0;
#endif
}

inline uint16_t HostToNetwork16(uint16_t host16)
{
#if defined(LINUX)
    return htobe16(host16);
#elif defined(MAC_OS)
    return htons(host16);
#else
    return 0;
#endif
}

inline uint64_t NetworkToHost64(uint64_t net64)
{
#if defined(LINUX)
    return be64toh(net64);
#elif defined(MAC_OS)
    return ntohll(net64);
#else
    return 0;
#endif
}

inline uint32_t NetworkToHost32(uint32_t net32)
{
#if defined(LINUX)
    return be32toh(net32);
#elif defined(MAC_OS)
    return ntohl(net32);
#else
    return 0;
#endif
}

inline uint16_t NetworkToHost16(uint16_t net16)
{
#if defined(LINUX)
    return be16toh(net16);
#elif defined(MAC_OS)
    return ntohs(net16);
#else
    return 0;
#endif
}
}

#endif //SSP_TOOLS_ENDIAN_H
