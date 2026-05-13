/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
 * All rights reserved.
 *
 * Unless otherwise stipulated in writing, any and all information contained
 * herein regardless in any format shall remain the sole proprietary of
 * SigmaStar and be kept in strict confidence
 * (SigmaStar Confidential Information) by the recipient.
 * Any unauthorized act including without limitation unauthorized disclosure,
 * copying, use, reproduction, sale, distribution, modification, disassembling,
 * reverse engineering and compiling of the contents of SigmaStar Confidential
 * Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
 * rights to any and all damages, losses, costs and expenses resulting therefrom.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include "AudioSRCProcess.h"
#include "st_common.h"

typedef struct
{
    char  chunkID[4];
    int   chunkSize;
    char  format[4];
    char  subchunk1ID[4];
    int   subchunk1Size;
    short audioFormat;
    short numChannels;
    int   sampleRate;
    int   byteRate;
    short blockAlign;
    short bitsPerSample;
    char  subchunk2ID[4];
    int   subchunk2Size;
} WaveHeaderStruct;

WaveHeaderStruct Header_struct;

void SetWaveHeader(int newSampleRate, char newBitDepth, char newNumChans)
{
    // set all the ASCII literals
    memcpy(Header_struct.chunkID, "RIFF", 4);
    memcpy(Header_struct.format, "WAVE", 4);
    memcpy(Header_struct.subchunk1ID, "fmt ", 4);
    memcpy(Header_struct.subchunk2ID, "data", 4);

    // set all the known numeric values
    Header_struct.audioFormat   = 1;
    Header_struct.chunkSize     = 36;
    Header_struct.subchunk1Size = 16;
    Header_struct.subchunk2Size = 0;

    // set the passed-in numeric values
    Header_struct.sampleRate    = newSampleRate * 1000;
    Header_struct.bitsPerSample = newBitDepth;
    Header_struct.numChannels   = newNumChans;

    // calculate the remaining dependent values
    Header_struct.blockAlign = Header_struct.numChannels * Header_struct.bitsPerSample / 8;
    Header_struct.byteRate   = Header_struct.sampleRate * Header_struct.blockAlign;
}

void WaveHeaderUpdate(int numBytes)
{
    Header_struct.subchunk2Size = numBytes;
    Header_struct.chunkSize     = Header_struct.subchunk2Size + 36;
}

void WaveHeaderInfo(int *dataArray)
{
    memcpy(dataArray, &Header_struct, (char)sizeof(Header_struct));
}

int GetHeaderResult(int numBytes, int *dataArray)
{
    WaveHeaderUpdate(numBytes);
    WaveHeaderInfo(dataArray);
    return 0;
}

int GetSrcOutSampleRate(SrcConversionMode mode)
{
    int outsamplerate = -1;
    switch (mode)
    {
        case SRC_8k_to_16k:
            outsamplerate = SRATE_16K;
            break;
        case SRC_8k_to_32k:
            outsamplerate = SRATE_32K;
            break;
        case SRC_8k_to_48k:
            outsamplerate = SRATE_48K;
            break;
        case SRC_16k_to_8k:
            outsamplerate = SRATE_8K;
            break;
        case SRC_16k_to_32k:
            outsamplerate = SRATE_32K;
            break;
        case SRC_16k_to_48k:
            outsamplerate = SRATE_48K;
            break;
        case SRC_32k_to_8k:
            outsamplerate = SRATE_8K;
            break;
        case SRC_32k_to_16k:
            outsamplerate = SRATE_16K;
            break;
        case SRC_32k_to_48k:
            outsamplerate = SRATE_48K;
            break;
        case SRC_48k_to_8k:
            outsamplerate = SRATE_8K;
            break;
        case SRC_48k_to_16k:
            outsamplerate = SRATE_16K;
            break;
        case SRC_48k_to_32k:
            outsamplerate = SRATE_32K;
            break;
    }
    return outsamplerate;
}

int main(int argc, char *argv[])
{
    char         new_header[100];
    FILE *       fin, *fout;
    int          total_size    = 0;
    int          WaveOut_srate = 0;
    ALGO_SRC_RET ret           = 0;
    unsigned int frame_size    = (256); // Support 256, 512, 1024 and 2880

    char *input_file  = "resource/input/audio/src_original.wav";
    char *output_file = "out/audio/src_processed.wav";

    printf("input_file  = %s\n", input_file);
    printf("output_file = %s\n\n", output_file);

    // For Lib usage
    char *           working_buf_ptr;
    short            audio_input[frame_size * 2], audio_output[frame_size * 12];
    int              OutputSampleNum;
    unsigned int     buf_size;
    SRCStructProcess src_struct;

    fin = fopen(input_file, "rb");
    if (!fin)
    {
        fprintf(stderr, "Error opening file: %s\n", input_file);
        return -1;
    }

    /* SRC parameter setting */
    src_struct.WaveIn_srate = SRATE_48K;
    src_struct.channel      = 2;
    src_struct.mode         = SRC_48k_to_8k;
    src_struct.order        = ORDER_HIGH;
    src_struct.point_number = frame_size;

    // Set output file header
    WaveOut_srate = GetSrcOutSampleRate(src_struct.mode);
    if (WaveOut_srate == -1)
    {
        fprintf(stderr, "GetSrcOutSampleRate failed\n");
        return -1;
    }
    SetWaveHeader(WaveOut_srate, 16, src_struct.channel);

    ST_Common_CheckMkdirOutFile(output_file);
    fout = fopen(output_file, "wb");
    if (!fout)
    {
        fprintf(stderr, "Error opening file: %s\n", output_file);
        return -1;
    }

    /* SRC init */
    buf_size          = IaaSrc_GetBufferSize(src_struct.mode);
    working_buf_ptr   = malloc(buf_size);
    SRC_HANDLE handle = IaaSrc_Init(working_buf_ptr, &src_struct);

    // main process

    fread(audio_input, sizeof(char), 44, fin);
    while (fread(audio_input, sizeof(short), frame_size * src_struct.channel, fin))
    {
        /* Run SRC process */
        ret = IaaSrc_Run(handle, audio_input, audio_output, &OutputSampleNum);
        if (ret != 0)
        {
            printf("SE run error !\n");
            return -1;
        }

        // write output file information
        total_size += src_struct.channel * OutputSampleNum * sizeof(short);
        fwrite(audio_output, sizeof(short), src_struct.channel * OutputSampleNum, fout);
    }
    GetHeaderResult(total_size, (int *)new_header);
    fseek(fout, 0, SEEK_SET);
    fwrite(new_header, sizeof(char), 44, fout);
    fclose(fin);
    fclose(fout);
    IaaSrc_Release(handle);
    free(working_buf_ptr);
    return 0;
}
