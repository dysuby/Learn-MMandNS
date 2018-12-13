#include "md5.h"

uint32_t s[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                  5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

uint32_t T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

uint32_t g[64] = {0, 1, 2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                  1, 6, 11, 0,  5,  10, 15, 4,  9,  14, 3,  8,  13, 2,  7,  12,
                  5, 8, 11, 14, 1,  4,  7,  10, 13, 0,  3,  6,  9,  12, 15, 2,
                  0, 7, 14, 5,  12, 3,  10, 1,  8,  15, 6,  13, 4,  11, 2,  9};

uint64_t final_len = 0;

/**
 * 补齐
*/
uint32_t *preprocess(const char *input) {
    uint64_t len = strlen(input);
    final_len = ((len + 8) / 64 + 1) * 64;                  // 计算总字节数
    uint32_t *ms = (uint32_t *)malloc(final_len);
    memset(ms, 0, final_len);                               // 置 0
    memcpy(ms, input, len);
    ms[len / 4] |= 0x80 << (len % 4) * 8;                   // append 1
    final_len /= 4;                                         // 计算 uint32_t 数组大小
    len *= 8;                                               // 计算 K
    memcpy(ms + final_len - 2, &len, 8);                    // 将后 64 位置 K
    return ms;
}

/**
 * 循环左移
*/
uint32_t leftrotate(uint32_t in, uint32_t offset) {
    return (in << offset) | (in >> (32 - offset));
}

/**
 * 主流程
*/
void md5(const char *input, uint8_t out[16]) {
    uint32_t *ms = preprocess(input);
    uint32_t h[4] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};
    uint32_t chunk[16];
    uint32_t a, b, c, d, f;
    for (int i = 0; i < final_len; i += 16) {
        memcpy(chunk, ms + i, 64);
        a = h[0];
        b = h[1];
        c = h[2];
        d = h[3];

        for (int i = 0; i < 64; ++i) {
            if (i < 16)
                f = (b & c) | (~b & d);     // F
            else if (i < 32)
                f = (d & b) | (~d & c);     // G
            else if (i < 48)
                f = b ^ c ^ d;              // H
            else
                f = c ^ (b | ~d);           // I
            f = a + f + T[i] + chunk[g[i]];
            a = d;
            d = c;
            c = b;
            b += leftrotate(f, s[i]);
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
    }
    memcpy(out, h, 16);
    free(ms);
}
