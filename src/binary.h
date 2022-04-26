#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

#ifdef _WIN64
    #ifdef WORDS_BIGENDIAN
        #define putle16(buf, x) (*(uint16_t*)(buf) = _byteswap_ushort((uint16_t)(x)))
        #define putle32(buf, x) (*(uint32_t*)(buf) = _byteswap_ulong((uint32_t)(x)))
        #define putle64(buf, x) (*(uint64_t*)(buf) = _byteswap_uint64((uint64_t)(x)))
    #else
        #define putle16(buf, x) (*(uint16_t*)(buf) = (uint16_t)(x))
        #define putle32(buf, x) (*(uint32_t*)(buf) = (uint32_t)(x))
        #define putle64(buf, x) (*(uint64_t*)(buf) = (uint64_t)(x))
    #endif
#else
    #ifdef WORDS_BIGENDIAN
        #define putle16(buf, x) (*(uint16_t*)(buf) = __builtin_bswap16((uint16_t)(x)))
        #define putle32(buf, x) (*(uint32_t*)(buf) = __builtin_bswap32((uint32_t)(x)))
        #define putle64(buf, x) (*(uint64_t*)(buf) = __builtin_bswap64((uint64_t)(x)))
    #else
        #define putle16(buf, x) (*(uint16_t*)(buf) = (uint16_t)(x))
        #define putle32(buf, x) (*(uint32_t*)(buf) = (uint32_t)(x))
        #define putle64(buf, x) (*(uint64_t*)(buf) = (uint64_t)(x))
    #endif
#endif
