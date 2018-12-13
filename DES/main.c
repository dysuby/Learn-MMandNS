#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "des.h"

/* 测试 1000 次 */
void test() {
    int len, j;
    srand(time(NULL));
    uchar *unencrypted = NULL, *encrypted = NULL, *back = NULL;
    for (int i = 0; i < 1000; ++i) {
        printf("Case: %d\r", i);
        len = (rand() % MAX_LEN);

        // 初始化变量
        unencrypted = (uchar *)realloc(unencrypted, len + 1);
        encrypted = (uchar *)realloc(encrypted, len + 1);
        back = (uchar *)realloc(back, len + 1);

        // 生成随机字符串，33为第一个可打印字符
        for (j = 0; j < len; ++j) unencrypted[j] = 33 + rand() % 94;
        unencrypted[j] = 0;

        int ret = encrypt(unencrypted, encrypted);
        decrypt(encrypted, back, ret);

        if (strcmp(unencrypted, back)) {
            printf("test fail!\nunencrypted: %s\nAfter decrypted: %s\n",
                   unencrypted, back);
            free(unencrypted);
            free(encrypted);
            free(back);
            return;
        }
    }
    free(unencrypted);
    free(encrypted);
    free(back);
    printf("1000 cases pass~\n");
}

int main() {
    int mode = 0;
    printf(
        "Enter test mode?\n"
        "0 - test\n"
        "1 - normal\n");
    scanf("%d", &mode);

    if (!mode) {
        test();
        printf("Test end~\n");
        exit(0);
    }

    uchar unencrypted[MAX_LEN], encrypted[MAX_LEN];
    printf("Please input(max lenghth: 1023): ");
    scanf("%s", unencrypted);

    int len = encrypt(unencrypted, encrypted);
    printf("Encrypted(Hex): ");
    for (int i = 0; i < len; ++i) printf("%02x ", encrypted[i]);
    printf("\n");

    memset(unencrypted, 0, MAX_LEN);

    decrypt(encrypted, unencrypted, len);
    printf("Decrypted: %s\n", unencrypted);

    return 0;
}