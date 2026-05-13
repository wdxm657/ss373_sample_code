/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "g711.h"
#include "st_common.h"

void func_linear2alaw(void);
void func_linear2ulaw(void);
void func_alaw2linear(void);
void func_ulaw2linear(void);
void func_alaw2ulaw(void);
void func_ulaw2alaw(void);

FILE* fpIn;  // input file
FILE* fpOut; // output file

int main(int argc, char* argv[])
{
    char        src_file[128] = {0};
    char        dst_file[128] = {0};
    char        option[128]   = {0}; // Select operation mode: encode, decode, convert
    const char* operation[6]  = {"en_linear2alaw", "en_linear2ulaw", "de_alaw2linear",
                                "de_ulaw2linear", "con_alaw2ulaw",  "con_ulaw2alaw"};

    /*
    *analyse parameter
    *argv[1]:Enter files and paths
    *argv[2]:Output file and path
    *argv[3]:Select operation mode: de_linear2alaw,de_linear2ulaw,\
    en_alaw2linear,en_ulaw2linear,con_alaw2ulaw,con_ulaw2alaw
    */
    if (4 != argc)
    {
        strcpy(src_file, "resource/input/audio/g711_8K_alaw_8bit_MONO_30s.pcm");
        strcpy(dst_file, "out/audio/g711_8K_ulaw_8bit_MONO_30s.pcm");
        strcpy(option, "con_alaw2ulaw");
    }
    else
    {
        sscanf(argv[1], "%s", src_file);
        sscanf(argv[2], "%s", dst_file);
        sscanf(argv[3], "%s", option);
    }

    printf("option      = %s\n", option);
    fpIn = fopen(src_file, "rb");
    if (NULL == fpIn)
    {
        printf("fopen in_file failed !\n");
        return -1;
    }
    printf("input_file  = %s\n", src_file);

    ST_Common_CheckMkdirOutFile(dst_file);
    fpOut = fopen(dst_file, "wb");
    if (NULL == fpOut)
    {
        printf("fopen out_file failed !\n");
        return -1;
    }
    printf("output_file = %s\n\n", dst_file);

    int ret;
    if ((ret = strcmp(option, operation[0])) == 0)
    {
        // linear2alaw
        func_linear2alaw();
    }
    else if ((ret = strcmp(option, operation[1])) == 0)
    {
        // linear2ulaw
        func_linear2ulaw();
    }
    else if ((ret = strcmp(option, operation[2])) == 0)
    {
        // alaw2linear
        func_alaw2linear();
    }
    else if ((ret = strcmp(option, operation[3])) == 0)
    {
        // ulaw2linear
        func_ulaw2linear();
    }
    else if ((ret = strcmp(option, operation[4])) == 0)
    {
        // alaw2ulaw
        func_alaw2ulaw();
    }
    else if ((ret = strcmp(option, operation[5])) == 0)
    {
        // ulaw2alaw
        func_ulaw2alaw();
    }
    else
    {
        printf("argv[3]:Please enter the correct parameters!\n");
    }
    fclose(fpIn);
    fclose(fpOut);

    return 0;
}

/*linear2alaw*/
void func_linear2alaw(void)
{
    short         pcm_data = 0;
    unsigned char law_data = 0;
    int           tmp;
    int           gBytesRead;

    while (1)
    {
        gBytesRead = fread(&pcm_data, 1, sizeof(short), fpIn);
        if (gBytesRead != sizeof(short))
        {
            break;
        }

        tmp      = pcm_data;
        law_data = linear2alaw(tmp);
        fwrite(&law_data, 1, sizeof(unsigned char), fpOut);
    }
    printf("encode to alaw is success !!!\n");
}

/*linear2ulaw*/
void func_linear2ulaw(void)
{
    short         pcm_data = 0;
    unsigned char law_data = 0;
    int           tmp;
    int           gBytesRead;

    while (1)
    {
        gBytesRead = fread(&pcm_data, 1, sizeof(short), fpIn);
        if (gBytesRead != sizeof(short))
        {
            break;
        }

        tmp      = pcm_data;
        law_data = linear2ulaw(tmp);
        fwrite(&law_data, 1, sizeof(unsigned char), fpOut);
    }
    printf("encode to ulaw is success !!!\n");
}

/*alaw2linear*/
void func_alaw2linear(void)
{
    short         pcm_data = 0;
    unsigned char law_data = 0;
    int           tmp;
    int           gBytesRead;

    while (1)
    {
        gBytesRead = fread(&law_data, 1, sizeof(unsigned char), fpIn);
        if (gBytesRead != sizeof(unsigned char))
        {
            break;
        }

        tmp      = law_data;
        pcm_data = alaw2linear(tmp);
        fwrite(&pcm_data, 1, sizeof(short), fpOut);
    }
    printf("decode from alaw is success !!!\n");
}

/*ulaw2linear*/
void func_ulaw2linear(void)
{
    short         pcm_data = 0;
    unsigned char law_data = 0;
    int           tmp;
    int           gBytesRead;

    while (1)
    {
        gBytesRead = fread(&law_data, 1, sizeof(unsigned char), fpIn);
        if (gBytesRead != sizeof(unsigned char))
        {
            break;
        }

        tmp      = law_data;
        pcm_data = ulaw2linear(tmp);
        fwrite(&pcm_data, 1, sizeof(short), fpOut);
    }
    printf("decode from ulaw is success !!!\n");
}

/*alaw2ulaw*/
void func_alaw2ulaw(void)
{
    unsigned char alaw_data = 0;
    unsigned char ulaw_data = 0;
    int           gBytesRead;

    while (1)
    {
        gBytesRead = fread(&alaw_data, 1, sizeof(unsigned char), fpIn);
        if (gBytesRead != sizeof(unsigned char))
        {
            break;
        }
        ulaw_data = alaw2ulaw(alaw_data);
        fwrite(&ulaw_data, 1, sizeof(unsigned char), fpOut);
    }
    printf("alaw to ulaw is success !!!\n");
}

/*ulaw2alaw*/
void func_ulaw2alaw(void)
{
    unsigned char alaw_data = 0;
    unsigned char ulaw_data = 0;
    int           gBytesRead;

    while (1)
    {
        gBytesRead = fread(&ulaw_data, 1, sizeof(unsigned char), fpIn);
        if (gBytesRead != sizeof(unsigned char))
        {
            break;
        }
        alaw_data = ulaw2alaw(ulaw_data);
        fwrite(&alaw_data, 1, sizeof(unsigned char), fpOut);
    }
    printf("ulaw to alaw is success !!!\n");
}
