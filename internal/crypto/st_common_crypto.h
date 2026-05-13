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
#ifndef _ST_COMMON_CRYPTO_H_
#define _ST_COMMON_CRYPTO_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "drv_rsa.h"
#include <crypto/cryptodev.h>

struct cryptodev_ctx
{
    int               cfd;
    struct session_op sess;
    uint16_t          alignmask;
};


int ST_Common_CryptoAes_Init(struct cryptodev_ctx* ctx, int cfd, const uint8_t* key, unsigned int key_size, enum cryptodev_crypto_op_t crypto_op);
int ST_Common_CryptoAes_Encrypt(struct cryptodev_ctx* ctx, const void* iv, const void* plaintext, void* ciphertext, size_t size, enum cryptodev_crypto_op_t crypto_op);
int ST_Common_CryptoAes_Decrypt(struct cryptodev_ctx* ctx, const void* iv, const void* ciphertext, void* plaintext, size_t size, enum cryptodev_crypto_op_t crypto_op);
int ST_Common_CryptoAes_DeInit(struct cryptodev_ctx* ctx);


int ST_Common_CryptoSha_Init(struct cryptodev_ctx* ctx, int cfd);
int ST_Common_CryptoSha_Hash(struct cryptodev_ctx* ctx, const void* text, size_t size, void* digest);
int ST_Common_CryptoSha_Update(struct cryptodev_ctx* ctx, const void* text, size_t size);
int ST_Common_CryptoSha_Copy(struct cryptodev_ctx* to_ctx, const struct cryptodev_ctx* from_ctx);
int ST_Common_CryptoSha_Final(struct cryptodev_ctx* ctx, const void* text, size_t size, void* digest);
int ST_Common_CryptoSha_DeInit(struct cryptodev_ctx* ctx);


int ST_Common_CryptoRsa_Run(struct rsa_config *prsa_config, int cfd);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_CRYPTO_H_