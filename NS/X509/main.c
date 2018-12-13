#include "x509.h"

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Please input the cer path in args\n");
        exit(1);
    }
    parse(argv[1]);
    return 0;
}
