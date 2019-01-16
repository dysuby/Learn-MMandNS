#include "x509.h"

#define GET(len, val) \
    for (int i = 0; i < len; ++i) val += (int)fgetc(cer) << ((len - i - 1) * 8);

#define BYTE_LEN 4096
#define STR_LEN 100

FILE *cer;

int getLen() {
    unsigned char c = fgetc(cer);
    int ret = 0;
    if (c & 0x80) {
        c -= 0x80;
        GET(c, ret);
    } else {
        ret = c;
    }
    return ret;
}

void getOID(unsigned char *oid, int len, char *out) {
    int index = 0;
    int b = oid[0] % 40;
    int a = (oid[0] - b) / 40;
    sprintf(out, "%d.%d", a, b);
    for (int i = 1; i < len; ++i) {
        if (oid[i] < 128) {
            sprintf(out + strlen(out), ".%d", oid[i]);
        } else {
            int res = 0;
            while (oid[i] >= 128)
                res = res * 128 + oid[i++] - 128;
            res = res * 128 + oid[i];
            sprintf(out + strlen(out), ".%d", res);
        }
    }
}

void getAlgorithm(char *oid) {
    // RFC5698
    if (!strcmp("1.2.840.10040.4.1", oid))
        sprintf(oid, "dsa");
    else if (!strcmp("1.3.14.3.2.26", oid))
        sprintf(oid, "sha-1");
    else if (!strcmp("2.16.840.1.101.3.4.2.4", oid))
        sprintf(oid, "sha-224");
    else if (!strcmp("2.16.840.1.101.3.4.2.1", oid))
        sprintf(oid, "sha-256");
    else if (!strcmp("2.16.840.1.101.3.4.2.2", oid))
        sprintf(oid, "sha-384");
    else if (!strcmp("2.16.840.1.101.3.4.2.3", oid))
        sprintf(oid, "sha-512");
    else if (!strcmp("1.2.840.113549.1.1.1", oid))
        sprintf(oid, "rsa");
    else if (!strcmp("1.2.840.113549.2.2", oid))
        sprintf(oid, "md2");
    else if (!strcmp("1.2.840.113549.2.5", oid))
        sprintf(oid, "md5");
    else if (!strcmp("1.2.840.113549.1.1.2", oid))
        sprintf(oid, "md2WithRSAEncryption");
    else if (!strcmp("1.2.840.113549.1.1.4", oid))
        sprintf(oid, "md5WithRSAEncryption");
    else if (!strcmp("1.2.840.113549.1.1.5", oid))
        sprintf(oid, "sha1WithRSAEncryption");
    else if (!strcmp("1.2.840.113549.1.1.11", oid))
        sprintf(oid, "sha256WithRSAEncryption");
    else if (!strcmp("1.2.840.113549.1.1.12", oid))
        sprintf(oid, " sha384WithRSAEncryption");
    else if (!strcmp("1.2.840.113549.1.1.13", oid))
        sprintf(oid, "sha512WithRSAEncryption");
    else if (!strcmp("1.2.840.10040.4.3", oid))
        sprintf(oid, "sha1WithDSA");
    else
        sprintf(oid, "<Unknown>");
}

void getIssuer(char *issuer) {
    // https://www.cryptosys.net/pki/manpki/pki_distnames.html
    if (!strcmp("2.5.4.6", issuer))
        sprintf(issuer, "countryName");
    else if (!strcmp("2.5.4.10", issuer))
        sprintf(issuer, "organizationName");
    else if (!strcmp("2.5.4.3", issuer))
        sprintf(issuer, "commonName");
    else if (!strcmp("2.5.4.11", issuer))
        sprintf(issuer, "organizationalUnitName");
    else if (!strcmp("2.5.4.7", issuer))
        sprintf(issuer, "localityName");
    else if (!strcmp("2.5.4.8", issuer))
        sprintf(issuer, "stateOrProvinceName");
    else
        sprintf(issuer, "<Unknown>");
}

void parseName() {
    fgetc(cer);
    int len = getLen() + ftell(cer);
    unsigned char bytes[BYTE_LEN] = {0};
    char str[STR_LEN];
    while (ftell(cer) < len) {
        fgetc(cer);  // set
        getLen();
        fgetc(cer);  // sequence
        getLen();

        // AttributeType
        fgetc(cer);
        int l = getLen();
        fread(bytes, 1, l, cer);
        getOID(bytes, l, str);
        printf("\t%-10s ", str);
        getIssuer(str);
        printf("%-25s", str);
        memset(str, 0, STR_LEN);

        // AttributeValue
        fgetc(cer);
        l = getLen();
        fread(str, 1, l, cer);
        printf("%s\n", str);
    }
}

void parseAlgorithmIdentifier() {
    int len;
    unsigned char bytes[BYTE_LEN] = {0};
    char str[STR_LEN];
    fgetc(cer);
    len = getLen();
    fread(bytes, 1, len, cer);
    getOID(bytes, len, str);
    printf("\tAlgorithm: %s", str);
    getAlgorithm(str);
    printf(" %s\n", str);

    // param
    if (fgetc(cer) != 0x05) {
        fseek(cer, getLen(), SEEK_CUR);
        printf("\tParameter: ...\n");
    } else {
        fgetc(cer);
        printf("\tParameter: NULL\n");
    }
}

void getTime(char *str) {
    memset(str, 0, STR_LEN);
    char c = fgetc(cer);
    int l = getLen();
    fread(str, 1, l, cer);
    if (c == 0x17)
        sprintf(str, "%s (UTCTime)", str);
    else
        sprintf(str, "%s (GeneralizedTime)", str);
}

void parse(const char *filename) {
    cer = fopen(filename, "rb");
    if (!cer) {
        printf("File doesn't exit!\n");
        exit(1);
    }

    unsigned char single, bytes[BYTE_LEN] = {0};
    char str[STR_LEN];
    int len = 0;
    fgetc(cer);  // Certificate
    getLen();

    fgetc(cer);  // TBSCertificate    
    getLen();

    // version
    single = fgetc(cer);
    if (single >= 0xA0) {
        getLen();
        fgetc(cer);
        len = getLen();
        int version = 1;
        GET(len, version);
        printf("Version: %d\n", version);

        single = fgetc(cer);
    } else {
        printf("Version: 1\n");
    }

    // serialNumber
    len = getLen();
    fread(bytes, 1, len, cer);
    printf("Serial Number: 0x");
    for (int i = 0; i < len; ++i) printf("%02x", bytes[i]);
    putchar('\n');

    // signature
    printf("Signature:\n");
    fgetc(cer);
    getLen();
    // algorithm
    parseAlgorithmIdentifier();

    // issuer
    printf("Issuer:\n");
    parseName();

    // Validity
    printf("Validity:\n");
    fgetc(cer);
    getLen();
    getTime(str);
    printf("\tStartTime: %s\n", str);
    getTime(str);
    printf("\tEndTime: %s\n", str);

    // Subject
    printf("Subject:\n");
    parseName();

    // SubjectPublicKeyInfo
    printf("SubjectPublicKeyInfo:\n");
    fgetc(cer);
    getLen();
    fgetc(cer);
    getLen();

    fgetc(cer);
    len = getLen();
    fread(bytes, 1, len, cer);
    getOID(bytes, len, str);
    printf("\tecPublicKey %s\n", str);

    fgetc(cer);
    len = getLen();
    fread(bytes, 1, len, cer);
    getOID(bytes, len, str);
    printf("\tprime256v1 %s\n", str);

    // subjectPublicKey
    fgetc(cer);
    len = getLen() - 1;
    fgetc(cer);
    fread(bytes, 1, len, cer);
    printf("\tSubjectPublicKey: 0x");
    for (int i = 0; i < len; ++i) printf("%02x", bytes[i]);
    putchar('\n');

    // skip optional
    while ((single = fgetc(cer)) > 0xA0) {
        switch (single)  {
            case 0xA1:
                printf("IssuerUniqueID: ...\n");
                break;
            case 0xA2:
                printf("SubjectUniqueID: ...\n");
                break;
            case 0xA3:
                printf("Extensions: ...\n");
                break;
            default:
                break;
        }
        fseek(cer, getLen(), SEEK_CUR);
    }

    // signatureAlgorithm
    printf("SignatureAlgorithm: \n", ftell(cer));
    getLen();
    parseAlgorithmIdentifier();

    // signatureValue
    fgetc(cer);
    len = getLen() - 1;
    fgetc(cer);
    fread(bytes, 1, len, cer);
    printf("SignatureValue: 0x");
    for (int i = 0; i < len; ++i) printf("%02x", bytes[i]);
    putchar('\n');

    fclose(cer);
}  
