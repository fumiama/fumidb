#ifndef _BINARY_H_
#define _BINARY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef __null
#  define __null 0
#endif

#ifdef __linux__
#  include <endian.h>
#endif
#ifdef __FreeBSD__
#  include <sys/endian.h>
#endif
#ifdef __NetBSD__
#  include <sys/endian.h>
#endif
#ifdef __OpenBSD__
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#endif
#ifdef __MAC_10_0
#  define be16toh(x) ntohs(x)
#  define be32toh(x) ntohl(x)
#  define be64toh(x) ntohll(x)
#  define htobe16(x) ntohs(x)
#  define htobe32(x) htonl(x)
#  define htobe64(x) htonll(x)
#endif
#ifdef _WIN64
	#ifdef WORDS_BIGENDIAN
		#  define be16toh(x) (x)
		#  define be32toh(x) (x)
		#  define be64toh(x) (x)
		#  define htobe16(x) (x)
		#  define htobe32(x) (x)
		#  define htobe64(x) (x)
	#else
		#  define be16toh(x) _byteswap_ushort(x)
		#  define be32toh(x) _byteswap_ulong(x)
		#  define be64toh(x) _byteswap_uint64(x)
		#  define htobe16(x) _byteswap_ushort(x)
		#  define htobe32(x) _byteswap_ulong(x)
		#  define htobe64(x) _byteswap_uint64(x)
	#endif
#endif

#ifdef WORDS_BIGENDIAN
    #ifdef _WIN64
        #  define le16toh(x) _byteswap_ushort(x)
        #  define le32toh(x) _byteswap_ulong(x)
        #  define le64toh(x) _byteswap_uint64(x)
        #  define htole16(x) _byteswap_ushort(x)
        #  define htole32(x) _byteswap_ulong(x)
        #  define htole64(x) _byteswap_uint64(x)

        #  define putle16(buf, x) (*(uint16_t*)(buf) = _byteswap_ushort((uint16_t)(x)))
        #  define putle32(buf, x) (*(uint32_t*)(buf) = _byteswap_ulong((uint32_t)(x)))
        #  define putle64(buf, x) (*(uint64_t*)(buf) = _byteswap_uint64((uint64_t)(x)))

        #  define readle16(fd, num) (read((fd), &(num), 2),((num)=_byteswap_ushort((uint16_t)(num))))
        #  define readle32(fd, num) (read((fd), &(num), 4),((num)=_byteswap_ulong((uint32_t)(num))))
        #  define readle64(fd, num) (read((fd), &(num), 8),((num)=_byteswap_uint64((uint64_t)(num))))

        #  define le16(buf) _byteswap_ushort(*(uint16_t*)(buf))
        #  define le32(buf) _byteswap_ulong(*(uint32_t*)(buf))
        #  define le64(buf) _byteswap_uint64(*(uint64_t*)(buf))
    #else
        #  define le16toh(x) __builtin_bswap16(x)
        #  define le32toh(x) __builtin_bswap32(x)
        #  define le64toh(x) __builtin_bswap64(x)
        #  define htole16(x) __builtin_bswap16(x)
        #  define htole32(x) __builtin_bswap32(x)
        #  define htole64(x) __builtin_bswap64(x)

        #  define putle16(buf, x) (*(uint16_t*)(buf) = __builtin_bswap16((uint16_t)(x)))
        #  define putle32(buf, x) (*(uint32_t*)(buf) = __builtin_bswap32((uint32_t)(x)))
        #  define putle64(buf, x) (*(uint64_t*)(buf) = __builtin_bswap64((uint64_t)(x)))

        #  define readle16(fd, num) (read((fd), &(num), 2),((num)=__builtin_bswap16((uint16_t)(num))))
        #  define readle32(fd, num) (read((fd), &(num), 4),((num)=__builtin_bswap32((uint32_t)(num))))
        #  define readle64(fd, num) (read((fd), &(num), 8),((num)=__builtin_bswap64((uint64_t)(num))))

        #  define le16(buf) __builtin_bswap16(*(uint16_t*)(buf))
        #  define le32(buf) __builtin_bswap32(*(uint32_t*)(buf))
        #  define le64(buf) __builtin_bswap64(*(uint64_t*)(buf))
    #endif
#else
    #  define le16toh(x) (x)
    #  define le32toh(x) (x)
    #  define le64toh(x) (x)
    #  define htole16(x) (x)
    #  define htole32(x) (x)
    #  define htole64(x) (x)

    #  define putle16(buf, x) (*(uint16_t*)(buf) = (uint16_t)(x))
    #  define putle32(buf, x) (*(uint32_t*)(buf) = (uint32_t)(x))
    #  define putle64(buf, x) (*(uint64_t*)(buf) = (uint64_t)(x))

    #  define readle16(fd, num) read((fd), &(num), 2)
    #  define readle32(fd, num) read((fd), &(num), 4)
    #  define readle64(fd, num) read((fd), &(num), 8)

    #  define le16(buf) (*(uint16_t*)(buf))
    #  define le32(buf) (*(uint32_t*)(buf))
    #  define le64(buf) (*(uint64_t*)(buf))
#endif

#  define likely(x)      __builtin_expect(!!(x), 1)
#  define unlikely(x)    __builtin_expect(!!(x), 0)

#endif