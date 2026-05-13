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
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "mi_sys.h"
#include "st_common.h"
#include "st_common_crypto.h"

int ST_Common_CryptoAes_Init(struct cryptodev_ctx* ctx, int cfd, const uint8_t* key, unsigned int key_size,
                             enum cryptodev_crypto_op_t crypto_op)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(key);

#ifdef CIOCGSESSINFO
    struct session_info_op siop;
#endif

    memset(ctx, 0, sizeof(*ctx));
    ctx->cfd = cfd;

    ctx->sess.cipher = crypto_op;
    ctx->sess.keylen = key_size;
    ctx->sess.key    = (void*)key;
    if (ioctl(ctx->cfd, CIOCGSESSION, &ctx->sess))
    {
        perror("ioctl(CIOCGSESSION)");
        return -1;
    }

#ifdef CIOCGSESSINFO
    memset(&siop, 0, sizeof(siop));

    siop.ses = ctx->sess.ses;
    if (ioctl(ctx->cfd, CIOCGSESSINFO, &siop))
    {
        perror("ioctl(CIOCGSESSINFO)");
        return -1;
    }
    printf("Got %s with driver %s\n", siop.cipher_info.cra_name, siop.cipher_info.cra_driver_name);

    /*printf("Alignmask is %x\n", (unsigned int)siop.alignmask); */
    ctx->alignmask = siop.alignmask;
#endif
    return 0;
}

int ST_Common_CryptoAes_DeInit(struct cryptodev_ctx* ctx)
{
    ST_CHECK_POINTER(ctx);

    if (ioctl(ctx->cfd, CIOCFSESSION, &ctx->sess.ses))
    {
        perror("ioctl(CIOCFSESSION)");
    }
    return 0;
}

int ST_Common_CryptoAes_Encrypt(struct cryptodev_ctx* ctx, const void* iv, const void* plaintext, void* ciphertext,
                                size_t size, enum cryptodev_crypto_op_t crypto_op)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(plaintext);
    ST_CHECK_POINTER(ciphertext);

    struct crypt_op cryp;

    memset(&cryp, 0, sizeof(cryp));

    /* Encrypt data.in to data.encrypted */
    cryp.ses = ctx->sess.ses;
    cryp.len = size;
    cryp.src = (void*)plaintext;
    cryp.dst = ciphertext;
    if (CRYPTO_AES_CBC == crypto_op)
    {
        cryp.iv = (void*)iv;
    }
    cryp.op = COP_ENCRYPT;
    if (ioctl(ctx->cfd, CIOCCRYPT, &cryp))
    {
        perror("ioctl(CIOCCRYPT)");
        return -1;
    }

    return 0;
}

int ST_Common_CryptoAes_Decrypt(struct cryptodev_ctx* ctx, const void* iv, const void* ciphertext, void* plaintext,
                                size_t size, enum cryptodev_crypto_op_t crypto_op)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(plaintext);
    ST_CHECK_POINTER(ciphertext);

    struct crypt_op cryp;

    memset(&cryp, 0, sizeof(cryp));

    /* Encrypt data.in to data.encrypted */
    cryp.ses = ctx->sess.ses;
    cryp.len = size;
    cryp.src = (void*)ciphertext;
    cryp.dst = plaintext;
    if (CRYPTO_AES_CBC == crypto_op)
    {
        cryp.iv = (void*)iv;
    }
    cryp.op = COP_DECRYPT;
    if (ioctl(ctx->cfd, CIOCCRYPT, &cryp))
    {
        perror("ioctl(CIOCCRYPT)");
        return -1;
    }

    return 0;
}

int ST_Common_CryptoSha_Init(struct cryptodev_ctx* ctx, int cfd)
{
    ST_CHECK_POINTER(ctx);

    struct session_info_op siop;

    memset(ctx, 0, sizeof(*ctx));
    ctx->cfd      = cfd;
    ctx->sess.mac = CRYPTO_SHA2_256;

    if (ioctl(ctx->cfd, CIOCGSESSION, &ctx->sess))
    {
        perror("ioctl(CIOCGSESSION)");
        return -1;
    }
#ifdef DEBUG
    fprintf(stderr, "ST_Common_CryptoSha_Init:   cfd=%d, ses=%04x\n", ctx->cfd, ctx->sess.ses);
#endif

    siop.ses = ctx->sess.ses;
    if (ioctl(ctx->cfd, CIOCGSESSINFO, &siop))
    {
        perror("ioctl(CIOCGSESSINFO)");
        return -1;
    }
    printf("Got %s with driver %s\n", siop.hash_info.cra_name, siop.hash_info.cra_driver_name);
    return 0;
}

static int sha_call_crypt(struct cryptodev_ctx* ctx, const void* text, size_t size, void* digest, unsigned int flags)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(text);

    struct crypt_op cryp;

    memset(&cryp, 0, sizeof(cryp));

    /* Fill out the fields with text, size, digest result and flags */
    cryp.ses   = ctx->sess.ses;
    cryp.len   = size;
    cryp.src   = (void*)text;
    cryp.mac   = digest;
    cryp.flags = flags;
#ifdef DEBUG
    fprintf(stderr, "sha_call_crypt: cfd=%d, ses=%04x, CIOCCRYPT(len=%d, src='%s', flags=%04x)\n", ctx->cfd,
            ctx->sess.ses, cryp.len, (char*)cryp.src, cryp.flags);
#endif
    return ioctl(ctx->cfd, CIOCCRYPT, &cryp);
}

int ST_Common_CryptoSha_Hash(struct cryptodev_ctx* ctx, const void* text, size_t size, void* digest)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(text);
    ST_CHECK_POINTER(digest);

#ifdef DEBUG
    fprintf(stderr, "ST_Common_CryptoSha_Hash:       cfd=%d, ses=%04x, text='%s', size=%ld\n", ctx->cfd, ctx->sess.ses,
            (char*)text, size);
#endif
    if (sha_call_crypt(ctx, text, size, digest, 0))
    {
        perror("ST_Common_CryptoSha_Hash: ioctl(CIOCCRYPT)");
        return -1;
    }

    return 0;
}

int ST_Common_CryptoSha_Update(struct cryptodev_ctx* ctx, const void* text, size_t size)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(text);

#ifdef DEBUG
    fprintf(stderr, "ST_Common_CryptoSha_Update:     cfd=%d, ses=%04x, text='%s', size=%ld\n", ctx->cfd, ctx->sess.ses,
            (char*)text, size);
#endif
    if (sha_call_crypt(ctx, text, size, NULL, COP_FLAG_UPDATE))
    {
        perror("ST_Common_CryptoSha_Update: ioctl(CIOCCRYPT)");
        return -1;
    }

    return 0;
}

int ST_Common_CryptoSha_Copy(struct cryptodev_ctx* to_ctx, const struct cryptodev_ctx* from_ctx)
{
    ST_CHECK_POINTER(to_ctx);
    ST_CHECK_POINTER(from_ctx);

    struct cphash_op cphash;

#ifdef DEBUG
    fprintf(stderr,
            "ST_Common_CryptoSha_Copy: from= cfd=%d, ses=%04x\n"
            "            to= cfd=%d, ses=%04x\n",
            from_ctx->cfd, from_ctx->sess.ses, to_ctx->cfd, to_ctx->sess.ses);
#endif
    memset(&cphash, 0, sizeof(cphash));

    cphash.src_ses = from_ctx->sess.ses;
    cphash.dst_ses = to_ctx->sess.ses;
    if (ioctl(to_ctx->cfd, CIOCCPHASH, &cphash))
    {
        perror("ioctl(CIOCCPHASH)");
        return -1;
    }

    return 0;
}

int ST_Common_CryptoSha_Final(struct cryptodev_ctx* ctx, const void* text, size_t size, void* digest)
{
    ST_CHECK_POINTER(ctx);
    ST_CHECK_POINTER(text);
    ST_CHECK_POINTER(digest);

#ifdef DEBUG
    fprintf(stderr, "ST_Common_CryptoSha_Final:      cfd=%d, ses=%04x, text='%s', size=%ld\n", ctx->cfd, ctx->sess.ses,
            (char*)text, size);
#endif
    if (sha_call_crypt(ctx, text, size, digest, COP_FLAG_FINAL))
    {
        perror("ST_Common_CryptoSha_Final: ioctl(CIOCCRYPT)");
        return -1;
    }

    return 0;
}

int ST_Common_CryptoSha_DeInit(struct cryptodev_ctx* ctx)
{
    ST_CHECK_POINTER(ctx);

#ifdef DEBUG
    fprintf(stderr, "ST_Common_CryptoSha_DeInit: cfd=%d, ses=%04x\n", ctx->cfd, ctx->sess.ses);
#endif
    if (ioctl(ctx->cfd, CIOCFSESSION, &ctx->sess.ses))
    {
        perror("ioctl(CIOCFSESSION)");
    }

    return 0;
}

int ST_Common_CryptoRsa_Run(struct rsa_config* prsa_config, int cfd)
{
    ST_CHECK_POINTER(prsa_config);

    // RSA calculate
    if (ioctl(cfd, MDrv_RSA_Calculate, prsa_config))
    {
        perror("ioctl(MDrv_RSA_Calculate)");
        return -1;
    }

    return 0;
}
