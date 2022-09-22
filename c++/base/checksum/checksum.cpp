#include "checksum.h"
#include <zlib.h>

using namespace muduo::base;

size_t SimpleCheckSum::flush() { return 0; }

size_t SimpleCheckSum::getCheckSum() const { return sum_; }

size_t SimpleCheckSum::write(const char *lpBuf, size_t nSize) {
    const tbyte *tp    = reinterpret_cast<const tbyte *>(lpBuf);
    const tbyte *tpEnd = tp + nSize;
    for (; tp != tpEnd; ++tp) {
        tbyte cipher = (*tp ^ (r_ >> 8));
        r_           = (cipher + r_) * c1_ + c2_;
        sum_ += cipher;
    }
    return nSize;
}

void SimpleCheckSum::reset() {
    sum_ = 0;
    r_   = 55665;
    c1_  = 52845;
    c2_  = 22719;
}
size_t SimpleCheckSum::getResultSize() const { return sizeof(sum_); }

static const uint32_t crc_32_tab[] = {
    0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL, 0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL,
    0x0edb8832UL, 0x79dcb8a4UL, 0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL, 0x90bf1d91UL,
    0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL, 0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL,
    0x136c9856UL, 0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL, 0xfa0f3d63UL, 0x8d080df5UL,
    0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL, 0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
    0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL, 0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL,
    0x26d930acUL, 0x51de003aUL, 0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL, 0xb8bda50fUL,
    0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL, 0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL,
    0x76dc4190UL, 0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL, 0x9fbfe4a5UL, 0xe8b8d433UL,
    0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL, 0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
    0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL, 0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL,
    0x65b0d9c6UL, 0x12b7e950UL, 0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL, 0xfbd44c65UL,
    0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL, 0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL,
    0x4369e96aUL, 0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL, 0xaa0a4c5fUL, 0xdd0d7cc9UL,
    0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL, 0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
    0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL, 0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL,
    0xedb88320UL, 0x9abfb3b6UL, 0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL, 0x73dc1683UL,
    0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL, 0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL,
    0xf00f9344UL, 0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL, 0x196c3671UL, 0x6e6b06e7UL,
    0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL, 0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
    0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL, 0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL,
    0xd80d2bdaUL, 0xaf0a1b4cUL, 0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL, 0x4669be79UL,
    0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL, 0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL,
    0xc5ba3bbeUL, 0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL, 0x2cd99e8bUL, 0x5bdeae1dUL,
    0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL, 0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
    0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL, 0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL,
    0x86d3d2d4UL, 0xf1d4e242UL, 0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL, 0x18b74777UL,
    0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL, 0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL,
    0xa00ae278UL, 0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL, 0x4969474dUL, 0x3e6e77dbUL,
    0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL, 0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
    0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL, 0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL,
    0xb3667a2eUL, 0xc4614ab8UL, 0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL, 0x2d02ef8dL};

size_t CRC32::write(const void *lpBuf, size_t nSize) {
    const tbyte *tp    = reinterpret_cast<const tbyte *>(lpBuf);
    const tbyte *tpEnd = tp + nSize;
    for (; tp != tpEnd; ++tp) {
        sum_ = crc_32_tab[ (tbyte)(sum_) ^ *tp ] ^ ((sum_) >> 8);
    }
    return static_cast<int>(nSize);
}

size_t CRC32::flush() { return 0; }

uint32_t CRC32::getChecksum() const { return sum_ ^ UINT32_MAX; }

size_t CRC32::getResultSize() const { return sizeof(sum_); }

void CRC32::reset() { sum_ = UINT32_MAX; }

//-------------------- MD5

enum {
    S11 = 7,
    S12 = 12,
    S13 = 17,
    S14 = 22,
    S21 = 5,
    S22 = 9,
    S23 = 14,
    S24 = 20,
    S31 = 4,
    S32 = 11,
    S33 = 16,
    S34 = 23,
    S41 = 6,
    S42 = 10,
    S43 = 15,
    S44 = 21
};

static uint8_t PADDING[ 64 ] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// F, G, H and I are basic MD5 functions.
inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (((x) & (y)) | ((~x) & (z))); }
inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (((x) & (z)) | ((y) & (~z))); }
inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return ((x) ^ (y) ^ (z)); }
inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return ((y) ^ ((x) | (~z))); }

// ROTATE_LEFT rotates x left n bits.
inline uint32_t ROTATE_LEFT(uint32_t x, uint32_t n) { return (((x) << (n)) | ((x) >> (32 - (n)))); }

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
#define FF(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }
#define GG(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }
#define HH(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }
#define II(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }

MD5::MD5()
    : CheckSumBase() {
    reset();
}

MD5::~MD5() {}

void MD5::transform(uint32_t *state, const uint8_t *block) {
    uint32_t a = state[ 0 ], b = state[ 1 ], c = state[ 2 ], d = state[ 3 ], x[ 16 ];
    decode(x, block, 64);
    // Round 1
    FF(a, b, c, d, x[ 0 ], S11, 0xd76aa478)  /* 1 */
    FF(d, a, b, c, x[ 1 ], S12, 0xe8c7b756)  /* 2 */
    FF(c, d, a, b, x[ 2 ], S13, 0x242070db)  /* 3 */
    FF(b, c, d, a, x[ 3 ], S14, 0xc1bdceee)  /* 4 */
    FF(a, b, c, d, x[ 4 ], S11, 0xf57c0faf)  /* 5 */
    FF(d, a, b, c, x[ 5 ], S12, 0x4787c62a)  /* 6 */
    FF(c, d, a, b, x[ 6 ], S13, 0xa8304613)  /* 7 */
    FF(b, c, d, a, x[ 7 ], S14, 0xfd469501)  /* 8 */
    FF(a, b, c, d, x[ 8 ], S11, 0x698098d8)  /* 9 */
    FF(d, a, b, c, x[ 9 ], S12, 0x8b44f7af)  /* 10 */
    FF(c, d, a, b, x[ 10 ], S13, 0xffff5bb1) /* 11 */
    FF(b, c, d, a, x[ 11 ], S14, 0x895cd7be) /* 12 */
    FF(a, b, c, d, x[ 12 ], S11, 0x6b901122) /* 13 */
    FF(d, a, b, c, x[ 13 ], S12, 0xfd987193) /* 14 */
    FF(c, d, a, b, x[ 14 ], S13, 0xa679438e) /* 15 */
    FF(b, c, d, a, x[ 15 ], S14, 0x49b40821) /* 16 */
    // Round 2
    GG(a, b, c, d, x[ 1 ], S21, 0xf61e2562)  /* 17 */
    GG(d, a, b, c, x[ 6 ], S22, 0xc040b340)  /* 18 */
    GG(c, d, a, b, x[ 11 ], S23, 0x265e5a51) /* 19 */
    GG(b, c, d, a, x[ 0 ], S24, 0xe9b6c7aa)  /* 20 */
    GG(a, b, c, d, x[ 5 ], S21, 0xd62f105d)  /* 21 */
    GG(d, a, b, c, x[ 10 ], S22, 0x2441453)  /* 22 */
    GG(c, d, a, b, x[ 15 ], S23, 0xd8a1e681) /* 23 */
    GG(b, c, d, a, x[ 4 ], S24, 0xe7d3fbc8)  /* 24 */
    GG(a, b, c, d, x[ 9 ], S21, 0x21e1cde6)  /* 25 */
    GG(d, a, b, c, x[ 14 ], S22, 0xc33707d6) /* 26 */
    GG(c, d, a, b, x[ 3 ], S23, 0xf4d50d87)  /* 27 */
    GG(b, c, d, a, x[ 8 ], S24, 0x455a14ed)  /* 28 */
    GG(a, b, c, d, x[ 13 ], S21, 0xa9e3e905) /* 29 */
    GG(d, a, b, c, x[ 2 ], S22, 0xfcefa3f8)  /* 30 */
    GG(c, d, a, b, x[ 7 ], S23, 0x676f02d9)  /* 31 */
    GG(b, c, d, a, x[ 12 ], S24, 0x8d2a4c8a) /* 32 */
    // Round 3
    HH(a, b, c, d, x[ 5 ], S31, 0xfffa3942)  /* 33 */
    HH(d, a, b, c, x[ 8 ], S32, 0x8771f681)  /* 34 */
    HH(c, d, a, b, x[ 11 ], S33, 0x6d9d6122) /* 35 */
    HH(b, c, d, a, x[ 14 ], S34, 0xfde5380c) /* 36 */
    HH(a, b, c, d, x[ 1 ], S31, 0xa4beea44)  /* 37 */
    HH(d, a, b, c, x[ 4 ], S32, 0x4bdecfa9)  /* 38 */
    HH(c, d, a, b, x[ 7 ], S33, 0xf6bb4b60)  /* 39 */
    HH(b, c, d, a, x[ 10 ], S34, 0xbebfbc70) /* 40 */
    HH(a, b, c, d, x[ 13 ], S31, 0x289b7ec6) /* 41 */
    HH(d, a, b, c, x[ 0 ], S32, 0xeaa127fa)  /* 42 */
    HH(c, d, a, b, x[ 3 ], S33, 0xd4ef3085)  /* 43 */
    HH(b, c, d, a, x[ 6 ], S34, 0x4881d05)   /* 44 */
    HH(a, b, c, d, x[ 9 ], S31, 0xd9d4d039)  /* 45 */
    HH(d, a, b, c, x[ 12 ], S32, 0xe6db99e5) /* 46 */
    HH(c, d, a, b, x[ 15 ], S33, 0x1fa27cf8) /* 47 */
    HH(b, c, d, a, x[ 2 ], S34, 0xc4ac5665)  /* 48 */
    // Round 4
    II(a, b, c, d, x[ 0 ], S41, 0xf4292244)  /* 49 */
    II(d, a, b, c, x[ 7 ], S42, 0x432aff97)  /* 50 */
    II(c, d, a, b, x[ 14 ], S43, 0xab9423a7) /* 51 */
    II(b, c, d, a, x[ 5 ], S44, 0xfc93a039)  /* 52 */
    II(a, b, c, d, x[ 12 ], S41, 0x655b59c3) /* 53 */
    II(d, a, b, c, x[ 3 ], S42, 0x8f0ccc92)  /* 54 */
    II(c, d, a, b, x[ 10 ], S43, 0xffeff47d) /* 55 */
    II(b, c, d, a, x[ 1 ], S44, 0x85845dd1)  /* 56 */
    II(a, b, c, d, x[ 8 ], S41, 0x6fa87e4f)  /* 57 */
    II(d, a, b, c, x[ 15 ], S42, 0xfe2ce6e0) /* 58 */
    II(c, d, a, b, x[ 6 ], S43, 0xa3014314)  /* 59 */
    II(b, c, d, a, x[ 13 ], S44, 0x4e0811a1) /* 60 */
    II(a, b, c, d, x[ 4 ], S41, 0xf7537e82)  /* 61 */
    II(d, a, b, c, x[ 11 ], S42, 0xbd3af235) /* 62 */
    II(c, d, a, b, x[ 2 ], S43, 0x2ad7d2bb)  /* 63 */
    II(b, c, d, a, x[ 9 ], S44, 0xeb86d391)  /* 64 */

    state[ 0 ] += a;
    state[ 1 ] += b;
    state[ 2 ] += c;
    state[ 3 ] += d;

    // Zero sensitive information.
    memset(x, 0, sizeof(x));
}

void MD5::update(const uint8_t *input, uint32_t len) {
    unsigned int i, index, partLen;
    // Compute number of bytes mod 64
    index = static_cast<unsigned int>((context_.count[ 0 ] >> 3) & 0x3F);
    // Update number of bits
    if ((context_.count[ 0 ] += ((uint32_t)len << 3)) < ((uint32_t)len << 3))
        context_.count[ 1 ]++;
    context_.count[ 1 ] += ((uint32_t)len >> 29);
    partLen = 64 - index;
    // Transform as many times as possible.
    if (len >= partLen) {
        memcpy(&context_.buffer[ index ], input, partLen);
        transform(context_.state, context_.buffer);
        for (i = partLen; i + 63 < len; i += 64) {
            transform(context_.state, &input[ i ]);
        }
        index = 0;
    } else {
        i = 0;
    }
    // Buffer remaining input
    memcpy(&context_.buffer[ index ], &input[ i ], len - i);
}

void MD5::final() {
    uint8_t      bits[ 8 ];
    unsigned int index, padLen;
    // Save number of bits
    encode(bits, context_.count, 8);
    // Pad out to 56 mod 64.
    index  = (unsigned int)((context_.count[ 0 ] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    update(PADDING, padLen);
    // Append length (before padding)
    update(bits, 8);
    // Store state in digest
    encode(md5digest_, context_.state, 16);
    // Zeroize sensitive information.
    memset(&context_, 0, sizeof(context_));
}

void MD5::encode(uint8_t *output, uint32_t *input, uint32_t len) {
    uint32_t i, j;
    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[ j ]     = (uint8_t)(input[ i ] & 0xff);
        output[ j + 1 ] = (uint8_t)((input[ i ] >> 8) & 0xff);
        output[ j + 2 ] = (uint8_t)((input[ i ] >> 16) & 0xff);
        output[ j + 3 ] = (uint8_t)((input[ i ] >> 24) & 0xff);
    }
}

void MD5::decode(uint32_t *output, const uint8_t *input, uint32_t len) {
    unsigned int i, j;
    for (i = 0, j = 0; j < len; i++, j += 4)
        output[ i ] = ((uint32_t)input[ j ]) | (((uint32_t)input[ j + 1 ]) << 8) | (((uint32_t)input[ j + 2 ]) << 16) |
                      (((uint32_t)input[ j + 3 ]) << 24);
}

void MD5::reset() {
    md5str_.clear();
    context_.count[ 0 ] = context_.count[ 1 ] = 0;
    context_.state[ 0 ]                       = 0x67452301;
    context_.state[ 1 ]                       = 0xefcdab89;
    context_.state[ 2 ]                       = 0x98badcfe;
    context_.state[ 3 ]                       = 0x10325476;
    hasFlush_                                 = false;
}

size_t MD5::flush() {
    if (hasFlush_) {
        return 0;
    }

    hasFlush_ = true;
    final();
    int   i;
    char *r;

    char md5str[ 33 ] = {0};
    for (i = 0, r = md5str; i < 16; i++, r += 2) {
        snprintf(r, 3, "%02x", md5digest_[ i ]);
    }
    *r = '\0';

    md5str_.assign(md5str);
    return 0;
}

size_t MD5::write(const void *lpBuf, size_t nSize) {
    update(static_cast<const unsigned char *>(lpBuf), static_cast<unsigned int>(nSize));
    return nSize;
}

const char *MD5::getMD5String() const { return md5str_.c_str(); }

const tbyte *MD5::getMD5Digest() const { return md5digest_; }

size_t MD5::getResultSize() const { return 16; }