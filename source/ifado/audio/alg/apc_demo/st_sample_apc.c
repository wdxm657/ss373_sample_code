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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "AudioProcess.h"
#include "st_common.h"

unsigned int WorkingBuffer2[1] = {0};

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

int main(int argc, char *argv[])
{
    short input[1024];
    int   counter           = 0;
    int   intensity_band[6] = {3, 24, 40, 64, 80, 128};
    int   intensity[7]      = {0, 30, 0, 30, 0, 30, 0};
    short eq_table[129]     = {1,  1,  -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,
                           -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,
                           -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,
                           -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,
                           -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,
                           -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5};
    memset(eq_table, 0, sizeof(short) * 129);
    short compression_ratio_input[_AGC_CR_NUM]         = {-65, -55, -48, -25, -18, -12, 0};
    short compression_ratio_output[_AGC_CR_NUM]        = {-65, -50, -27, -12, -1, -1, -1};
    int   compressionRatioArrayLowInput[_AGC_CR_NUM]   = {-80, -60, -40, -20, 0, 0, 0};
    int   compressionRatioArrayLowOutput[_AGC_CR_NUM]  = {-5, -5, -5, -5, -5, -5, -5};
    int   compressionRatioArrayMidInput[_AGC_CR_NUM]   = {-80, -60, -40, -20, 0, 0, 0};
    int   compressionRatioArrayMidOutput[_AGC_CR_NUM]  = {-80, -60, -40, -20, 0, 0, 0};
    int   compressionRatioArrayHighInput[_AGC_CR_NUM]  = {-80, -60, -40, -20, 0, 0, 0};
    int   compressionRatioArrayHighOutput[_AGC_CR_NUM] = {-80, -60, -40, -20, 0, 0, 0};

    AudioApcBufferConfig apc_switch;
    apc_switch.anr_enable = 1;
    apc_switch.eq_enable  = 1;
    apc_switch.dr_enable  = 0;
    apc_switch.vad_enable = 0;
    apc_switch.agc_enable = 1;
    int   Buffer_size     = IaaApc_GetBufferSize(&apc_switch);
    char *working_buf_ptr = (char *)malloc(Buffer_size);

    FILE *           fin, *fout;
    int              ret1;
    AudioProcessInit apc_init;
    AudioAnrConfig   anr_config;
    AudioEqConfig    eq_config;
    AudioHpfConfig   hpf_config;
    AudioAgcConfig   agc_config;
    APC_HANDLE       handle;
    int              PN   = 128;
    apc_init.point_number = PN;
    apc_init.channel      = 1;
    apc_init.sample_rate  = IAA_APC_SAMPLE_RATE_16000;

    char *input_file  = "resource/input/audio/apc_original.wav";
    char *output_file = "out/audio/apc_processed.wav";

    printf("input_file  = %s\n", input_file);
    printf("output_file = %s\n\n", output_file);

    handle = IaaApc_Init((char *)working_buf_ptr, &apc_init, &apc_switch);
    if (handle == NULL)
    {
        printf("APC init error\r\n");
        return -1;
    }

    /******ANR Config*******/
    anr_config.anr_enable      = apc_switch.anr_enable;
    anr_config.user_mode       = 1;
    anr_config.anr_filter_mode = 0;
    memcpy(anr_config.anr_intensity_band, intensity_band, 6 * sizeof(int));
    memcpy(anr_config.anr_intensity, intensity, 7 * sizeof(int));
    anr_config.anr_smooth_level   = 10;
    anr_config.anr_converge_speed = 2;
    /******EQ Config********/
    eq_config.eq_enable = apc_switch.eq_enable;
    eq_config.user_mode = 1;
    memcpy(eq_config.eq_gain_db, eq_table, _EQ_BAND_NUM * sizeof(short));
    /******HPF Config********/
    hpf_config.hpf_enable       = apc_switch.eq_enable;
    hpf_config.user_mode        = 1;
    hpf_config.cutoff_frequency = AUDIO_HPF_FREQ_150;
    /******AGC Config********/
    agc_config.agc_enable          = apc_switch.agc_enable;
    agc_config.user_mode           = 2;
    agc_config.gain_info.gain_max  = 40;
    agc_config.gain_info.gain_min  = -10;
    agc_config.gain_info.gain_init = 12;
    agc_config.drop_gain_max       = 36;
    agc_config.gain_step           = 1;
    agc_config.attack_time         = 1;
    agc_config.release_time        = 1;
    agc_config.noise_gate_db       = -80;
    memcpy(agc_config.compression_ratio_input, compression_ratio_input, _AGC_CR_NUM * sizeof(short));
    memcpy(agc_config.compression_ratio_output, compression_ratio_output, _AGC_CR_NUM * sizeof(short));
    agc_config.noise_gate_attenuation_db = 0;
    agc_config.drop_gain_threshold       = -5;

    if (IaaApc_Config(handle, &anr_config, &eq_config, &hpf_config, NULL, NULL, &agc_config) == -1)
    {
        printf("Config Error!");
        return -1;
    }

    IaaApc_SetLowFreqCompressionRatioCurve(handle, compressionRatioArrayLowInput, compressionRatioArrayLowOutput);
    IaaApc_SetMidFreqCompressionRatioCurve(handle, compressionRatioArrayMidInput, compressionRatioArrayMidOutput);
    IaaApc_SetHighFreqCompressionRatioCurve(handle, compressionRatioArrayHighInput, compressionRatioArrayHighOutput);

    fin = fopen(input_file, "rb");
    if (!fin)
    {
        printf("the input file %s could not be open\n", input_file);
        return -1;
    }

    ST_Common_CheckMkdirOutFile(output_file);
    fout = fopen(output_file, "wb");
    if (!fout)
    {
        printf("the output file could not be open\n");
        return -1;
    }

    fread(input, sizeof(char), 44, fin);   // read header 44 bytes
    fwrite(input, sizeof(char), 44, fout); // write 44 bytes output

    while (fread(input, sizeof(short), apc_init.point_number * apc_init.channel, fin))
    {
        counter++;
        ret1 = IaaApc_Run(handle, input);
        if (ret1 < 0)
        {
            printf("Error occured in NoiseReduct\n");
            break;
        }

        fwrite(input, sizeof(short), apc_init.point_number * apc_init.channel, fout);
    }

    IaaApc_Free(handle);
    free(working_buf_ptr);
    fclose(fin);
    fclose(fout);

    return 0;
}
