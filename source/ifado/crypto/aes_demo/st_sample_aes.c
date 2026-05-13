/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "st_common_crypto.h"

#define AES_BLOCK_SIZE  16
#define KEY_SIZE        16

static void PrintValue(char* pbuf, int len, char* description)
{
    printf("\n%s\n", description);
    for (int i = 0; i < len; i++)
    {
        printf("0x%02x, ", pbuf[i]);
    }
    printf("\n");
}

static int ST_DoAes(int cfd)
{
    char                 plaintext_raw[AES_BLOCK_SIZE + 63];
    char                 ciphertext[AES_BLOCK_SIZE + 63];
    char                 plaintext[AES_BLOCK_SIZE + 63];
    char                 ciphertext_expect[AES_BLOCK_SIZE]  = {0xdf, 0x55, 0x6a, 0x33, 0x43, 0x8d, 0xb8, 0x7b,0xc4, 0x1b, 0x17, 0x52, 0xc5, 0x5e, 0x5e, 0x49};
    char                 iv[AES_BLOCK_SIZE];
    uint8_t              key[KEY_SIZE]                      = {0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct               cryptodev_ctx ctx;

    ST_Common_CryptoAes_Init(&ctx, cfd, key, sizeof(key), CRYPTO_AES_CBC);

    // Encrypto 0x0 data with 0x0 iv
    memset(plaintext_raw, 0x0, AES_BLOCK_SIZE);
    memset(iv, 0x0, sizeof(iv));

    // Do crypto
    ST_Common_CryptoAes_Encrypt(&ctx, iv, plaintext_raw, ciphertext, AES_BLOCK_SIZE, CRYPTO_AES_CBC);
    ST_Common_CryptoAes_Decrypt(&ctx, iv, ciphertext, plaintext, AES_BLOCK_SIZE, CRYPTO_AES_CBC);

    // Print result
    PrintValue(plaintext_raw, AES_BLOCK_SIZE, "plaintext_raw:");
    PrintValue(ciphertext, AES_BLOCK_SIZE, "ciphertext:");
    PrintValue(ciphertext_expect, AES_BLOCK_SIZE, "ciphertext_expect:");
    PrintValue(plaintext, AES_BLOCK_SIZE, "plaintext(de):");

    // Verify the encrypto result
    if (memcmp(ciphertext, ciphertext_expect, AES_BLOCK_SIZE) || memcmp(plaintext_raw, plaintext, AES_BLOCK_SIZE))
    {
        fprintf(stderr, "FAIL: Decrypted data are different from the input data.\n");
        ST_Common_CryptoAes_DeInit(&ctx);
        return -1;
    }

    ST_Common_CryptoAes_DeInit(&ctx);

    printf("AES Test passed\n");

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
    ret = ST_DoAes(cfd);
    if (ret != 0)
    {
        printf("ST_DoAes Fail\n");
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
