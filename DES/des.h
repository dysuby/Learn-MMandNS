#ifndef DES
#define DES

#define uchar unsigned char

#define MAX_LEN 1024

/* 返回加密后的长度 */ 
int encrypt(uchar *unencrypted, uchar *encrypted);

/* len 为密文长度 */
void decrypt(uchar *encrypted, uchar *unencrypted, int len);

#endif
