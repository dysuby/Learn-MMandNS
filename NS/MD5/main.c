#include "md5.h"

int main() {
    unsigned char out[16];
    char input[1025] = {0};
    printf("MD5: Please input the plain text:\n");

    // 允许输入空串或带空格的字符串
    int i;
    for (i = 0; i < 1024 && (input[i] = getchar()) != '\n'; ++i) {}
    input[i] = '\0';
    
    md5(input, out);
    
    printf("Cipher text:\n");
    for (int i = 0; i < 16; ++i) printf("%02x", out[i]);
    putchar('\n');
    return 0;
}