/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized diSclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <crypto/cryptodev.h>
#include "st_common_crypto.h"


static void PrintValue(uint8_t* pbuf, int len, char* description)
{
    printf("\n%s\n", description);
    for (int i = 0; i < len; i++)
    {
        printf("0x%02x, ", pbuf[i]);
    }
    printf("\n");
}


static int ST_DoSha(int cfd)
{
    struct cryptodev_ctx ctx1;
    uint8_t              digest[32];
    char                 text[]  = "The quick brown fox jumps over the lazy dog";
    char                 text1[] = "The quick brown fox";
    char                 text2[] = " jumps over the lazy dog";
    int                  check_OneOperation = 0, check_UpdataFinal = 0;
    // uint8_t expected[] = "\x2f\xd4\xe1\xc6\x7a\x2d\x28\xfc\xed\x84\x9e\xe1\xbb\x76\xe7\x39\x1b\x93\xeb\x12";
    uint8_t expected[] = {0xd7, 0xa8, 0xfb, 0xb3, 0x07, 0xd7, 0x80, 0x94, 0x69, 0xca, 0x9a,
                          0xbc, 0xb0, 0x08, 0x2e, 0x4f, 0x8d, 0x56, 0x51, 0xe4, 0x6d, 0x3c,
                          0xdb, 0x76, 0x2d, 0x02, 0xd0, 0xbf, 0x37, 0xc9, 0xe5, 0x92};


    printf("\n\nComputing digest in one operation\n");
    ST_Common_CryptoSha_Init(&ctx1, cfd);
    ST_Common_CryptoSha_Hash(&ctx1, text, strlen(text), digest);
    ST_Common_CryptoSha_DeInit(&ctx1);
    PrintValue(digest, 32, "The digest:");
    PrintValue(expected, 32, "The expected:");
    check_OneOperation = memcmp(digest, expected, 32);


    printf("\n\nComputing digest using update/final\n");
    ST_Common_CryptoSha_Init(&ctx1, cfd);
    ST_Common_CryptoSha_Update(&ctx1, text1, strlen(text1));
    ST_Common_CryptoSha_Final(&ctx1, text2, strlen(text2), digest);
    ST_Common_CryptoSha_DeInit(&ctx1);
    PrintValue(digest, 32, "The digest:");
    PrintValue(expected, 32, "The expected:");
    check_UpdataFinal = memcmp(digest, expected, 32);


    if ((0 != check_OneOperation) || (0 != check_UpdataFinal))
    {
        printf("SHA Test failed\n");
        return -1;
    }

    printf("SHA Test passed\n");
    return 0;
}


int main()
{
    int cfd = -1;
    int ret = -1;
    /* Open the crypto device */
    cfd = open("/dev/crypto", O_RDWR, 0);
    if (cfd < 0)
    {
        perror("open(/dev/crypto)");
        return -1;
    }

    /* Run the test itself */
    ret = ST_DoSha(cfd);
    if (0 != ret)
    {
        printf("ST_DoSha Fail\n");
        close(cfd);
        return -1;
    }

    /* Close the original descriptor */
    if (close(cfd))
    {
        perror("close(cfd)");
        return -1;
    }

    return 0;
}
