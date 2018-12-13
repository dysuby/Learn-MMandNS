#include "des.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 位运算 */
#define SetBit(A, k) ((A)[((k) / CHAR_BIT)] |= (1 << (7 - (k) % CHAR_BIT)))
#define ClearBit(A, k) ((A)[((k) / CHAR_BIT)] &= ~(1 << (7 - (k) % CHAR_BIT)))
#define TestBit(A, k) \
    (((A)[((k) / CHAR_BIT)] & (1 << (7 - (k) % CHAR_BIT))) != 0)

/* dst[i] = src[j] */
#define Equal(dst, i, src, j) \
    if (TestBit(src, j))  \
        SetBit(dst, i);   \
    else                      \
        ClearBit(dst, i);

/* 置换用，ptable为置换表 */
#define permutation(dst, src, ptable, size) \
    for (int _i_ = 0; _i_ < size; ++_i_) Equal(dst, _i_, src, ptable[_i_] - 1);

uchar temp[MAX_LEN + 9];  // 算法实际操作的对象，因为要补全字节所以 +9
uchar subkey[16][6];  // 子密钥


// 各种表以及初始密钥
const char IP[64] = {58, 50, 42, 34, 26, 18, 10, 2,  60, 52, 44, 36, 28,
                     20, 12, 4,  62, 54, 46, 38, 30, 22, 14, 6,  64, 56,
                     48, 40, 32, 24, 16, 8,  57, 49, 41, 33, 25, 17, 9,
                     1,  59, 51, 43, 35, 27, 19, 11, 3,  61, 53, 45, 37,
                     29, 21, 13, 5,  63, 55, 47, 39, 31, 23, 15, 7};

const char E[48] = {32, 1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
                    8,  9,  10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
                    16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
                    24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1};

const char PC1[56] = {57, 49, 41, 33, 25, 17, 9,  1,  58, 50, 42, 34, 26, 18,
                      10, 2,  59, 51, 43, 35, 27, 19, 11, 3,  60, 52, 44, 36,
                      63, 55, 47, 39, 31, 23, 15, 7,  62, 54, 46, 38, 30, 22,
                      14, 6,  61, 53, 45, 37, 29, 21, 13, 5,  28, 20, 12, 4};

const char PC2[48] = {14, 17, 11, 24, 1,  5,  3,  28, 15, 6,  21, 10,
                      23, 19, 12, 4,  26, 8,  16, 7,  27, 20, 13, 2,
                      41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
                      44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32};

const char S[8][64] = {
    {14, 4,  13, 1, 2,  15, 11, 8,  3,  10, 6,  12, 5,  9,  0, 7,
     0,  15, 7,  4, 15, 2,  13, 1,  10, 6,  12, 11, 9,  5,  3, 8,
     4,  1,  14, 8, 13, 6,  2,  11, 15, 12, 9,  7,  3,  10, 5, 0,
     15, 12, 8,  2, 4,  9,  1,  7,  5,  11, 3,  14, 10, 0,  6, 13},

    {15, 1,  8,  14, 6,  11, 3,  4,  9,  7, 2,  13, 12, 0, 5,  10,
     3,  13, 4,  7,  15, 2,  8,  14, 12, 0, 1,  10, 6,  9, 11, 5,
     0,  14, 7,  11, 10, 4,  13, 1,  5,  8, 12, 6,  9,  3, 2,  15,
     13, 8,  10, 1,  3,  15, 4,  2,  11, 6, 7,  12, 0,  5, 14, 9},

    {10, 0,  9,  14, 6, 3,  15, 5,  1,  13, 12, 7,  11, 4,  2,  8,
     13, 7,  0,  9,  3, 4,  6,  10, 2,  8,  5,  14, 12, 11, 15, 1,
     13, 6,  4,  9,  8, 15, 3,  0,  11, 1,  2,  12, 5,  10, 14, 7,
     1,  10, 13, 0,  6, 9,  8,  7,  4,  15, 14, 3,  11, 5,  2,  12},

    {7,  13, 14, 3, 0,  6,  9,  10, 1,  2, 8, 5,  11, 12, 4,  15,
     12, 8,  11, 5, 6,  15, 0,  3,  4,  7, 2, 12, 1,  10, 14, 9,
     10, 6,  9,  0, 12, 11, 7,  13, 15, 1, 3, 14, 5,  2,  8,  4,
     3,  15, 0,  6, 10, 1,  13, 8,  9,  4, 5, 11, 12, 7,  2,  14},

    {2,  12, 4,  1,  7,  10, 11, 6,  8,  5,  3,  15, 13, 0, 14, 9,
     14, 11, 2,  12, 4,  7,  13, 1,  5,  0,  15, 10, 3,  9, 8,  6,
     4,  2,  1,  11, 10, 13, 7,  8,  15, 9,  12, 5,  6,  3, 0,  14,
     11, 8,  12, 7,  1,  14, 2,  13, 6,  15, 0,  9,  10, 4, 5,  3},

    {12, 1,  10, 15, 9, 2,  6,  8,  0,  13, 3,  4,  14, 7,  5,  11,
     10, 15, 4,  2,  7, 12, 9,  5,  6,  1,  13, 14, 0,  11, 3,  8,
     9,  14, 15, 5,  2, 8,  12, 3,  7,  0,  4,  10, 1,  13, 11, 6,
     4,  3,  2,  12, 9, 5,  15, 10, 11, 14, 1,  7,  6,  0,  8,  13},

    {4,  11, 2,  14, 15, 0, 8,  13, 3,  12, 9, 7,  5,  10, 6, 1,
     13, 0,  11, 7,  4,  9, 1,  10, 14, 3,  5, 12, 2,  15, 8, 6,
     1,  4,  11, 13, 12, 3, 7,  14, 10, 15, 6, 8,  0,  5,  9, 2,
     6,  11, 13, 8,  1,  4, 10, 7,  9,  5,  0, 15, 14, 2,  3, 12},

    {13, 2,  8,  4, 6,  15, 11, 1,  10, 9,  3,  14, 5,  0,  12, 7,
     1,  15, 13, 8, 10, 3,  7,  4,  12, 5,  6,  11, 0,  14, 9,  2,
     7,  11, 4,  1, 9,  12, 14, 2,  0,  6,  10, 13, 15, 3,  5,  8,
     2,  1,  14, 7, 4,  10, 8,  13, 15, 12, 9,  0,  3,  5,  6,  11}};

const char P[32] = {16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23,
                    26, 5, 18, 31, 10, 2,  8,  24, 14, 32, 27,
                    3,  9, 19, 13, 30, 6,  22, 11, 4,  25};

const char IP_1[64] = {40, 8,  48, 16, 56, 24, 64, 32, 39, 7,  47, 15, 55,
                       23, 63, 31, 38, 6,  46, 14, 54, 22, 62, 30, 37, 5,
                       45, 13, 53, 21, 61, 29, 36, 4,  44, 12, 52, 20, 60,
                       28, 35, 3,  43, 11, 51, 19, 59, 27, 34, 2,  42, 10,
                       50, 18, 58, 26, 33, 1,  41, 9,  49, 17, 57, 25};

const char key[8] = {'v', 'e', 'g', '_', 'c', 'h', 'i', 'c'};

/* 补齐分块 */
int fixLength() {
    int l = strlen(temp);
    int s = 8 - l % 8, index;
    for (index = l; index - l < s; ++index) temp[index] = s;
    temp[index] = 0;
    return l + s;
}

/* IP-置换 */
void IP_permutation(uchar *block) {
    uchar after[8];
    memcpy(after, block, 8);
    permutation(after, block, IP, 64);
    memcpy(block, after, 8);
}

/* 将 str 循环左移 bit 位，因为只在子密钥生成中使用，所以 str 长度为 28 位 */
void LS(uchar *str, int bit) {
    uchar tc[1];

    // 保存每个字节的高两位
    for (int i = 0; i < 4; ++i) {
        Equal(tc, 2 * i, str + i, 0);
        Equal(tc, 2 * i + 1, str + i, 1);
    }

    // 对每个字节进行循环左移
    for (int i = 0; i < 4; ++i) {
        str[i] <<= bit;
        if (bit == 2) {
            if (i < 3) {
                Equal(str + i, 6, tc, (i + 1) * 2);
                Equal(str + i, 7, tc, (i + 1) * 2 + 1);
            } else {
                Equal(str + i, 2, tc, 0);
                Equal(str + i, 3, tc, 1);
            }
        } else {
            if (i < 3) {
                Equal(str + i, 7, tc, (i + 1) * 2);
            } else {
                Equal(str + i, 3, tc, 0);
            }
        }
    }
}

/* 生成子密钥 */
void generateKey() {
    uchar CD[7], K[6];

    // PC-1 置换
    permutation(CD, key, PC1, 56);

    // 分别将 CD 的前后 28 位存在 C，D 中
    uchar C[4] = {0}, D[4] = {0};
    for (int i = 0; i < 27; ++i) Equal(C, i, CD, i);
    for (int i = 0; i < 27; ++i) Equal(D, i, CD, i + 28);

    for (int i = 1; i <= 16; ++i) {
        // LS(A_i)
        if (i == 1 || i == 2 || i == 9 || i == 16) {
            LS(C, 1);
            LS(D, 1);
        } else {
            LS(C, 2);
            LS(D, 2);
        }

        // 将结果存回 CD中
        for (int j = 0; j < 27; ++j) Equal(CD, j, C, j);
        for (int j = 0; j < 27; ++j) Equal(CD, j, D, j + 28);

        // PC-2 置换
        permutation(K, CD, PC2, 48);

        // 存回 subkey[i - 1]
        memcpy(subkey + i - 1, K, 6);
    }
}

void feistel(uchar *R, uchar *K) {
    // E-扩展
    uchar estr[6];
    permutation(estr, R, E, 48);

    // 按位异或
    for (int i = 0; i < 48; ++i) {
        if (TestBit(estr, i) ^ TestBit(K, i))
            SetBit(estr, i);
        else
            ClearBit(estr, i);
    }

    // S-box
    for (int i = 0; i < 8; ++i) {
        // 分组，每组 6 位
        uchar group[1];
        for (int j = 0; j < 6; ++j) {
            Equal(group, j, estr, (i * 6) + j);
        }

        // 计算 row = b1b6
        int row = TestBit(group, 0) << 1 + TestBit(group, 5);
        // 计算 col = b2b3b4b5
        int col = 0;
        for (int j = 1; j < 5; ++j) {
            col += TestBit(group, j) << (4 - j);
        }

        // 将 4 位输出写回 R 中
        int index = row * 16 + col;
        for (int j = 4; j < 8; ++j) {
            Equal(R, (i * 4) + j - 4, S[i] + index, j);
        }
    }

    // P-置换
    uchar old[4];
    memcpy(old, R, 4);
    permutation(R, old, P, 32);
}

void T(uchar *block, int r) {
    uchar L[4], R[4];
    // L_{i-1} = block 左 32 位
    memcpy(L, block, 4);

    // R_{i-1} = block 右 32 位
    memcpy(R, block + 4, 4);

    // L_i = R_{i-1}
    memcpy(block, R, 4);

    // R_i = L_{i-1} ^ f(R_{i-1}, K_i)
    feistel(R, subkey[r]);
    uchar Ri[4];
    for (int i = 0; i < 32; ++i) {
        if (TestBit(L, i) ^ TestBit(R, i))
            SetBit(Ri, i);
        else
            ClearBit(Ri, i);
    }

    // 存回 block 中
    memcpy(block + 4, Ri, 4);
}

/* IP-1 置换*/
void IPreverse(uchar *block) {
    uchar old[8];
    memcpy(old, block, 8);
    permutation(block, old, IP_1, 64);
}

/* 加密过程 */
int encrypt(uchar *unencrypted, uchar *encrypted) {
    if (strlen(unencrypted) > MAX_LEN) {
        printf("Out of lenghth!\n");
        exit(1);
    }
    memset(temp, 0, MAX_LEN + 9);
    memcpy(temp, unencrypted, strlen(unencrypted));

    // 生成子密钥
    generateKey();

    // 补全分块，num 为块数
    int len = fixLength(), num = len / 8;

    for (int i = 0; i < num; ++i) {
        uchar *block = temp + i * 8;

        // IP 置换
        IP_permutation(block);

        // 16 轮迭代-T
        for (int r = 0; r < 16; ++r) {
            T(block, r);
        }

        // 前后 32 位互换
        char old[4];
        memcpy(old, block, 4);
        memcpy(block, block + 4, 4);
        memcpy(block + 4, old, 4);

        // IP-1 逆置换
        IPreverse(block);
    }

    // 输出
    memcpy(encrypted, temp, len);

    return len;
}

/* 解密过程 */
void decrypt(uchar *encrypted, uchar *unencrypted, int len) {
    if (len > MAX_LEN) {
        printf("Out of lenghth!\n");
        exit(1);
    }
    memset(temp, 0, MAX_LEN + 9);
    memcpy(temp, encrypted, len);

    // 生成子密钥
    generateKey();

    int num = len / 8;
    for (int i = 0; i < num; ++i) {
        uchar *block = temp + i * 8;

        // IP 置换
        IP_permutation(block);

        // 迭代 T，反序调用子密钥
        for (int r = 15; r >= 0; --r) {
            T(block, r);
        }

        // 前后 32 位互换
        char old[4];
        memcpy(old, block, 4);
        memcpy(block, block + 4, 4);
        memcpy(block + 4, old, 4);

        // IP-1 逆置换
        IPreverse(block);
    }

    // 删掉加密时补全的字节
    uchar c = temp[len - 1];
    memset(temp + len - c, 0, c);

    // 输出
    memcpy(unencrypted, temp, len);
}
