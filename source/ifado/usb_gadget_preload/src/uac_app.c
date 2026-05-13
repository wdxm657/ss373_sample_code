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

#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
#include <string.h>

#include "usb_ac_fu_range.h"
#include "uac_app.h"

#include "mi_sys.h"

#include "cam_os_wrapper.h"
#include "sys_sys_isw_cli.h"
#include "mi_ai.h"

#include "mi_ao.h"
#include "mi_ai.h"
#include "mi_ai_impl.h"
#include "mi_ao_impl.h"

#include "uac.h"
#include "st_uac.h"


#include "AudioProcess.h"

#include "uac_audio.h"
#include "pCam_handler_audio.h"

#include "usb_app_dbg.h"
#include "st_common.h"

#define AUDIO_APC_ENABLE 0

#define MAX_AUD_DEV_NUM (1)

#define AI_DEV_ID_MAX   (4)
#define AO_DEV_ID_MAX   (1)

#define AI_DEV_AMIC     (0)
#define AI_DEV_DMIC     (1)
#define AI_DEV_I2S_RX   (2)
#define AI_DEV_LineIn   (3)
#define AI_DEV_I2S_RX_AND_SRC   (4)

#define AO_DEV_LineOut  (0)
#define AO_DEV_I2S_TX   (1)

#define AI_VOLUME_AMIC_MIN      (0)
#define AI_VOLUME_AMIC_MAX      (21)
#define AI_VOLUME_DMIC_MIN      (-60)
#define AI_VOLUME_DMIC_MAX      (3)
#define AI_VOLUME_LINEIN_MIN    (0)
#define AI_VOLUME_LINEIN_MAX    (7)

#define AIO_LPLL_SOURCE_CLK (48000) //Unit: K. 48000 is 48000*1000=48MHz.

typedef struct uac_app_para
{
    MI_U32 uac_idx;

#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    MI_U8 uac_init;
    MI_U8 StartCapture_UAC;
    MI_BOOL bEnableAI;

    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_U32 u32AiSampleRate;

    MI_BOOL bAiEnableVqe;
    MI_BOOL bAiEnableNr;
    MI_BOOL bAiEnableAec;

    CamOsThread uacThread;
#endif

#if defined(CONFIG_USB_GADGET_UAC_SPK_SUPPORT)
    MI_BOOL bEnableAO;
    MI_U8 ao_init;

    MI_AUDIO_DEV AoDevId;
    MI_AO_CHN AoChn;
    MI_U32 u32AoSampleRate;

    MI_BOOL bAoEnableVqe;
    MI_BOOL bAoEnableNr;
#endif
}ST_UAC_ARGS;

#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
extern MI_S32 MI_AI_Init(void);
extern MI_S32 MI_AI_DeInit(void);
#endif
#if defined(CONFIG_USB_GADGET_UAC_SPK_SUPPORT)
extern MI_S32 MI_AO_Init(void);
extern MI_S32 MI_AO_DeInit(void);
#endif

static struct uac_app_para guac_app_para[MAX_AUD_DEV_NUM];
static int uac_running = 0;

#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
static int aio_fine_tune_sample_rate_cur = 0, aio_clk_2_lpll = 0;
static char aio_fine_tune_name[28] = {"aio_proc lpll setkhz 48000"};
#endif

#if defined(CONFIG_USB_GADGET_UAC_SPK_SUPPORT)
#if 0
static MI_S32 ST_AO_Init(MI_U32 u32AoSampleRate, MI_U8 chn)
{
    MI_AUDIO_DEV AoDevId     = 0;
    MI_AO_Attr_t stAoSetAttr = {0};
    MI_AO_If_e enAoIf        = E_MI_AO_IF_DAC_AB;
    MI_S8 s8LeftVolume       = 0;
    MI_S8 s8RightVolume      = 0;
    MI_AO_GainFading_e eGainFading         = E_MI_AO_GAIN_FADING_OFF;
    MI_AUDIO_SampleRate_e eAOSetSamplerate = u32AoSampleRate;

    struct uac_app_para *puac_para = &(guac_app_para[chn]);
    if (puac_para->AoDevId == AO_DEV_I2S_TX)
    {
        CamOsPrintf("Func %s Not verified %d\n", __FUNCTION__, puac_para->AoDevId);
    }
    memset(&stAoSetAttr, 0, (size_t)sizeof(MI_AO_Attr_t));
    stAoSetAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_DOUBLE_MONO;
    stAoSetAttr.enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAoSetAttr.enSampleRate  = eAOSetSamplerate;
    stAoSetAttr.enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    stAoSetAttr.u32PeriodSize = AO_PERIOD_SIZE(eAOSetSamplerate);

    /* open ao device */
    STCHECKRESULT(MI_AO_Open(AoDevId, &stAoSetAttr));

    /* attach ao interface */
    STCHECKRESULT(MI_AO_AttachIf(AoDevId, enAoIf, 0));

    /* set ao device volume */
    STCHECKRESULT(MI_AO_SetVolume(AoDevId, s8LeftVolume, s8RightVolume, eGainFading));

    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return MI_SUCCESS;
}

static MI_S32 ST_AO_Deinit(void)
{
    ExecFunc(MI_AO_DetachIf(0, E_MI_AO_IF_DAC_AB), MI_SUCCESS);
    ExecFunc(MI_AO_Close(0), MI_SUCCESS);
    return MI_SUCCESS;
}
#endif
void uac_spk_init(u8 spk_idx, u32 sample_rate)
{
    //MI_S32 s32Ret = -1;
    struct uac_app_para *puac_para;

    RET_ON(spk_idx >= MAX_AUD_DEV_NUM);
    puac_para = &(guac_app_para[spk_idx]);
    CamOsPrintf(KERN_INFO"%s, sample_rate:%d \n", __func__,sample_rate);
    if(puac_para->bEnableAO)
    {
        if(sample_rate == 8000 || sample_rate == 16000 || sample_rate == 32000 || sample_rate == 48000)
            puac_para->u32AoSampleRate = sample_rate;

        //MI_AO_Init();

        //s32Ret = ST_AO_Init(puac_para->u32AoSampleRate, spk_idx);
        //RET_ON(MI_SUCCESS != s32Ret);
        puac_para->ao_init = 1;
    }
}

void uac_spk_stop(u8 spk_idx)
{
    struct uac_app_para *puac_para;

    RET_ON(spk_idx >= MAX_AUD_DEV_NUM);
    puac_para = &(guac_app_para[spk_idx]);
    if (puac_para->bEnableAO && puac_para->ao_init)
    {
        //ST_AO_Deinit();
        //MI_AO_DeInit();
        puac_para->ao_init = 0;
    }
}

void uac_spk_send_frame(u8 spk_idx, void *frame, u32 size)
{
    MI_S32 s32Ret = -1;
    struct uac_app_para *puac_para;

    RET_ON(spk_idx >= MAX_AUD_DEV_NUM);
    puac_para = &(guac_app_para[spk_idx]);
    if (frame == NULL)
        return;

    if (puac_para->bEnableAO && puac_para->ao_init)
    {
        ST_UAC_Frame_t stUacFrame;

        stUacFrame.data = frame;
        stUacFrame.length = size;

        do {
            s32Ret = MI_AO_Write(0, stUacFrame.data, stUacFrame.length, 0, -1);
        } while(MI_AO_ERR_NOBUF == s32Ret);
    }
}
#endif

#if AUDIO_APC_ENABLE
char *apc_workingBuffer = NULL;
APC_HANDLE apc_handle;
AudioProcessInit apc_init;
int ST_APC_Init(void)
{
    unsigned int workingBufferSize;
    AudioApcBufferConfig apc_switch;
    AudioAnrConfig anr_config;
    AudioEqConfig eq_config;
    AudioHpfConfig hpf_config;
    AudioAgcConfig agc_config;
    /****************************User change section start**********************************/
    int intensity_band[6] = {3,24,40,64,80,128};
    int intensity[7] = {30,30,30,30,30,30,30};
    short eq_table[129];
    memset(eq_table, 0, sizeof(eq_table));
    short compression_ratio_input[_AGC_CR_NUM] = {-70,-40,-35,-20,-5};
    short compression_ratio_output[_AGC_CR_NUM]= {-70,-30,-24,-24,-24};

    apc_switch.anr_enable = 1;
    apc_switch.eq_enable = 0;
    apc_switch.agc_enable = 1;

    apc_init.point_number = 128;
    apc_init.channel = 2;
    apc_init.sample_rate = IAA_APC_SAMPLE_RATE_48000;

    /******ANR Config*******/
    anr_config.anr_enable = apc_switch.anr_enable;
    anr_config.user_mode = 2;
    memcpy(anr_config.anr_intensity_band, intensity_band, sizeof(intensity_band));
    memcpy(anr_config.anr_intensity, intensity, sizeof(intensity));
    anr_config.anr_smooth_level = 10;
    anr_config.anr_converge_speed = 0;
    /******EQ Config********/
    eq_config.eq_enable = apc_switch.eq_enable;
    eq_config.user_mode = 1;
    memcpy(eq_config.eq_gain_db, eq_table, sizeof(eq_table));
    /******HPF Config********/
    hpf_config.hpf_enable = apc_switch.eq_enable;
    hpf_config.user_mode = 1;
    hpf_config.cutoff_frequency = AUDIO_HPF_FREQ_150;
    /******AGC Config********/
    agc_config.agc_enable = apc_switch.agc_enable;
    agc_config.user_mode = 1;
    agc_config.gain_info.gain_max  = 40;
    agc_config.gain_info.gain_min  = -10;
    agc_config.gain_info.gain_init = 0;
    agc_config.drop_gain_max = 36;
    agc_config.gain_step = 1;
    agc_config.attack_time = 1;
    agc_config.release_time = 1;
    agc_config.noise_gate_db = -70;
    agc_config.noise_gate_attenuation_db = 0;
    agc_config.drop_gain_threshold = -4;
    memcpy(agc_config.compression_ratio_input, compression_ratio_input, sizeof(compression_ratio_input));
    memcpy(agc_config.compression_ratio_output, compression_ratio_output, sizeof(compression_ratio_output));

    /****************************User change section end***********************************/
    //(1)IaaApc_GetBufferSize
    workingBufferSize = IaaApc_GetBufferSize(&apc_switch);
    apc_workingBuffer = (char*)malloc(workingBufferSize);
    if(NULL == apc_workingBuffer)
    {
        CamOsPrintf(KERN_ERR"malloc workingBuffer failed !\n");
        return -1;
    }
    //(2)IaaApc_Init
    apc_handle = IaaApc_Init(apc_workingBuffer, &apc_init, &apc_switch);
    if(NULL == apc_handle)
    {
        CamOsPrintf(KERN_ERR"IaaApc_Init failed !\n");
        return -1;
    }
    //(3)IaaApc_Config
    if(IaaApc_Config(apc_handle, &anr_config, &eq_config, &hpf_config, NULL, NULL, &agc_config))
    {
        CamOsPrintf(KERN_ERR"IaaApc_Config failed !\n");
        return -1;
    }

    return 0;
}

int ST_APC_Run(void *v_addr, u32 size)
{
    int tempSize;
    int ret=0;
    tempSize = apc_init.point_number * apc_init.channel;
    int iCount = size/tempSize;
    for ( int i=0; i<iCount; i++ )
    {
        //(5)IaaAnr_Run
        ret = IaaApc_Run(apc_handle, v_addr + 2 * i * tempSize);
        if(ret)
        {
            CamOsPrintf(KERN_ERR"IaaAPC_Run failed !\n");
            return -1;
        }
    }

    return 0;
}

int ST_APC_DeInit(void)
{
    IaaApc_Free(apc_handle);
    free(apc_workingBuffer);
    return 0;
}
#endif

#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
static int usb_audio_aio_fine_tune_sample_rate(u8 mic_idx, u32 cur_frm_cnt, u32 low_bound, u32 high_bound)
{
    int aio_set_new_lpll_clk = 0;
    u32 aio_sample_rate_target = 0;
    struct uac_app_para *puac_para;

    RET_VAL_ON(mic_idx >= MAX_AUD_DEV_NUM, 0);
    puac_para = &(guac_app_para[mic_idx]);
    if ((cur_frm_cnt < low_bound && (aio_fine_tune_sample_rate_cur != 1)))
    {
        aio_sample_rate_target = puac_para->u32AiSampleRate + (puac_para->u32AiSampleRate >> 5);
        aio_fine_tune_sample_rate_cur = 1;
        aio_set_new_lpll_clk = 1;
        CamOsPrintf(KERN_INFO"$+");
    }
    else if ((cur_frm_cnt > high_bound && (aio_fine_tune_sample_rate_cur != -1)))
    {
        aio_sample_rate_target = puac_para->u32AiSampleRate - (puac_para->u32AiSampleRate >> 5);
        aio_fine_tune_sample_rate_cur = -1;
        aio_set_new_lpll_clk = 1;
        CamOsPrintf(KERN_INFO"$-");
    }
    else if(((aio_fine_tune_sample_rate_cur == 1) && (cur_frm_cnt > low_bound))
            || ((aio_fine_tune_sample_rate_cur == -1) && (cur_frm_cnt < high_bound)))
    {
        aio_sample_rate_target = puac_para->u32AiSampleRate;
        aio_fine_tune_sample_rate_cur = 0;
        aio_set_new_lpll_clk = 1;
        CamOsPrintf(KERN_INFO"$=");
    }

    if(aio_set_new_lpll_clk)
    {
        u32 new_lpll_clk = 0;
        new_lpll_clk = (u32)(((u64)aio_sample_rate_target * AIO_LPLL_SOURCE_CLK) / (u64)puac_para->u32AiSampleRate);
        aio_fine_tune_name[21] = '0' + ((new_lpll_clk / 10000) % 10); //ten thousand digits
        aio_fine_tune_name[22] = '0' + ((new_lpll_clk / 1000) % 10); //thousands digit
        aio_fine_tune_name[23] = '0' + ((new_lpll_clk / 100) % 10); //hundreds digit
        aio_fine_tune_name[24] = '0' + ((new_lpll_clk / 10) % 10); //tens digit
        aio_fine_tune_name[25] = '0' + ((new_lpll_clk / 1) % 10); //units digit

        if(aio_clk_2_lpll == 0)
        {
            //run_command("aio_proc lpll init");
            aio_clk_2_lpll = 1;
        }
        //run_command(aio_fine_tune_name);
    }

    return aio_fine_tune_sample_rate_cur;
}

static MI_S32 ST_AI_Init(MI_U32 u32AiSampleRate, MI_U8 chn)
{
    MI_AUDIO_DEV AiDevId     = 0;
    MI_AI_Attr_t stAiSetAttr = {0};
    MI_AI_Attr_t stAiGetAttr = {0};
    MI_AI_If_e enAiIf[]      = {E_MI_AI_IF_ADC_AB, E_MI_AI_IF_ECHO_A};
    MI_U8 u8ChnGrpId         = 0;
    MI_S16 s8DpgaGain[]      = {-10};
    MI_U32 u32FrameCnt       = 5;
    MI_AUDIO_SampleRate_e eAISetSamplerate = u32AiSampleRate;
    //set output port buffer depth
    MI_SYS_ChnPort_t stAiChnOutputPort;
    struct uac_app_para *puac_para = &(guac_app_para[chn]);

    if (puac_para->AiDevId == AI_DEV_AMIC)
    {
        enAiIf[0]      = E_MI_AI_IF_ADC_AB;
        enAiIf[1]      = E_MI_AI_IF_ECHO_A;
    }
    else if (puac_para->AiDevId == AI_DEV_DMIC)
    {
        enAiIf[0]      = E_MI_AI_IF_DMIC_A_01;
        enAiIf[1]      = E_MI_AI_IF_ECHO_A;
        CamOsPrintf("[%s]L%d DevId%d\n", __FUNCTION__, __LINE__, puac_para->AiDevId);
    }
    else {
        CamOsPrintf("Func %s Not verified %d\n", __FUNCTION__, puac_para->AiDevId);
    }
    CamOsPrintf("Func %s start\n", __FUNCTION__);
    memset(&stAiSetAttr, 0, (size_t)sizeof(MI_AI_Attr_t));
    memset(&stAiGetAttr, 0, (size_t)sizeof(MI_AI_Attr_t));

    stAiSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAiSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiSetAttr.enSampleRate = eAISetSamplerate;
    stAiSetAttr.u32PeriodSize = AI_PERIOD_SIZE(eAISetSamplerate);
    stAiSetAttr.bInterleaved = TRUE;
    /* open ai device */
    STCHECKRESULT(MI_AI_Open(AiDevId, &stAiSetAttr));

    /* get ai device */
    STCHECKRESULT(MI_AI_GetAttr(AiDevId, &stAiGetAttr));

    /* attach ai interface */
    STCHECKRESULT(MI_AI_AttachIf(AiDevId, enAiIf, sizeof(enAiIf) / sizeof(enAiIf[0])));

    /* set ai interface gain */
    if (puac_para->AiDevId == AI_DEV_AMIC)
    {
        STCHECKRESULT(MI_AI_SetIfGain(E_MI_AI_IF_ADC_AB, 18, 18));  //0~19
    }
    else if (puac_para->AiDevId == AI_DEV_DMIC)
    {
        STCHECKRESULT(MI_AI_SetIfGain(E_MI_AI_IF_DMIC_A_01, 4, 4)); //0~5
    }

    /* set output depth */
    memset(&stAiChnOutputPort, 0, (size_t)sizeof(stAiChnOutputPort));
    stAiChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stAiChnOutputPort.u32DevId = AiDevId;
    stAiChnOutputPort.u32ChnId = u8ChnGrpId;
    stAiChnOutputPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stAiChnOutputPort, u32FrameCnt, u32FrameCnt));

    /* set ai dpga gain */
    STCHECKRESULT(MI_AI_SetGain(AiDevId, u8ChnGrpId, s8DpgaGain, sizeof(s8DpgaGain) / sizeof(s8DpgaGain[0])));

    /* enable ai device channel */
    STCHECKRESULT(MI_AI_EnableChnGroup(AiDevId, u8ChnGrpId));

    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return MI_SUCCESS;
}

static MI_S32 ST_AI_Deinit(void)
{
    ExecFunc(MI_AI_DisableChnGroup(0, 0), MI_SUCCESS);
    ExecFunc(MI_AI_Close(0), MI_SUCCESS);
    return MI_SUCCESS;
}

MI_S32 ST_AudioModuleInit(u8 mic_idx)
{
    struct uac_app_para *puac_para;

    RET_VAL_ON(mic_idx >= MAX_AUD_DEV_NUM, -1);
    puac_para = &(guac_app_para[mic_idx]);

    if(puac_para->bEnableAI)
    {
        CamOsPrintf(KERN_ERR"enable ai sample_rate:%d \n", puac_para->u32AiSampleRate);
        MI_U32 ret;
        //MI_AI_Init();
        ret = ST_AI_Init(puac_para->u32AiSampleRate, mic_idx);
        RET_VAL_ON(MI_SUCCESS != ret, ret);
    }
#if AUDIO_APC_ENABLE
    ST_APC_Init();
#endif

    return MI_SUCCESS;
}

MI_S32 ST_AudioModuleUnInit(u8 mic_idx)
{
    struct uac_app_para *puac_para;

    RET_VAL_ON(mic_idx >= MAX_AUD_DEV_NUM, -1);
    puac_para = &(guac_app_para[mic_idx]);

#if AUDIO_APC_ENABLE
    ST_APC_DeInit();
#endif
    if(puac_para->bEnableAI)
    {
        ST_AI_Deinit();
    }
    //MI_AI_DeInit();

    return MI_SUCCESS;
}

void* UAC_Audio_Playback_Task(void* data)
{
    u32 ring_usage = 0;
    struct uac_app_para *puac_para;
    MI_AI_Data_t stAiFrame, stAiFrame2;

    puac_para = data;
    while(CAM_OS_OK != CamOsThreadShouldStop())
    {
        if(puac_para->StartCapture_UAC == TRUE)
        {
            if(puac_para->uac_init == 0)
            {
                CamOsPrintf(KERN_INFO"uac: start capture\n");
                //ST_AudioModuleInit(puac_para->uac_idx);
                PCAM_USB_SetAudioAttrEnable(0/*ch_id*/, 1);
                puac_para->uac_init = 1;
                uac_running++;
            }

            if (MI_AI_Read(0, 0, &stAiFrame, &stAiFrame2, -1) != MI_SUCCESS)
            {
                CamOsPrintf(KERN_ERR"uac: AI GetFrame failed!\n");
                continue;
            }
#if AUDIO_APC_ENABLE
            ST_APC_Run((short*)stAiFrame.apvBuffer[0], stAiFrame.u32Byte[0]);
#endif

            sstar_usbd_uac_mic_sent_packet((void *)stAiFrame.apvBuffer[0], stAiFrame.u32Byte[0]);
            MI_AI_ReleaseData(0, 0, &stAiFrame, &stAiFrame2);

            ring_usage = sstar_usbd_uac_mic_get_ring_usage(0); //0% ~ 100%
            usb_audio_aio_fine_tune_sample_rate(puac_para->uac_idx, ring_usage, 20, 80);
        }
        else
        {
            if(puac_para->uac_init == 1)
            {
                CamOsPrintf(KERN_ERR"uac: stop capture\n");
                PCAM_USB_SetAudioAttrEnable(0/*ch_id*/, 0);
                //ST_AudioModuleUnInit(puac_para->uac_idx);
                puac_para->uac_init = 0;
                uac_running--;
            }
            CamOsUsSleep(1);
        }
    }
    return NULL;
}

void UAC_StartCapture(u8 mic_idx, u32 samplerate)
{
    struct uac_app_para *puac_para;
    u32 timeout = 1000;

    RET_ON(mic_idx >= MAX_AUD_DEV_NUM);
    puac_para = &(guac_app_para[mic_idx]);
    puac_para->u32AiSampleRate = (MI_U32)samplerate;
    puac_para->StartCapture_UAC = TRUE;
    while((puac_para->uac_init == 0) && (--timeout > 0))
    {
        CamOsMsSleep(1);
    }
    RET_ON(timeout == 0);
}

void UAC_StopCapture(u8 mic_idx)
{
    struct uac_app_para *puac_para;
    u32 timeout = 1000;

    RET_ON(mic_idx >= MAX_AUD_DEV_NUM);
    puac_para = &(guac_app_para[mic_idx]);
    puac_para->StartCapture_UAC = FALSE;
    while((puac_para->uac_init != 0) && (--timeout > 0))
    {
        CamOsMsSleep(1);
    }
    RET_ON(timeout == 0);
}

#endif //#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)

void ST_AudioDefaultArgs(void)
{
    MI_U32 i;
    ST_UAC_ARGS *args;

    for (i = 0; i < MAX_AUD_DEV_NUM; i++)
    {
        args = &guac_app_para[i];
        args->uac_idx = i;
#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
        args->bEnableAI = TRUE;
        args->AiDevId = AI_DEV_AMIC;
        args->u32AiSampleRate = E_MI_AUDIO_SAMPLE_RATE_8000;
        CamOsPrintf(KERN_EMERG"[%s]L%d ENA.%s SPR:%dK\n", __FUNCTION__, __LINE__, (args->AiDevId==AI_DEV_AMIC)?"AMIC":(args->AiDevId==AI_DEV_DMIC)?"DMIC":"OTHER", (args->u32AiSampleRate/1000));
#endif
#if defined(CONFIG_USB_GADGET_UAC_SPK_SUPPORT)
        args->bEnableAO = TRUE;
        args->AoDevId = AO_DEV_LineOut;
        args->u32AoSampleRate = E_MI_AUDIO_SAMPLE_RATE_8000;
#endif
    }
}

struct uac_user_ops uac_app_ops =
{
    .uac_ac_process_setup_req = uac_process_req_ac,
    .uac_ac_process_setup_data = uac_process_data_ac,

    .fpuac_ac_proc_setup_init = uac_ac_process_setup_init,
    .fpuac_ac_proc_setup_deinit = uac_ac_process_setup_deinit,
    .fpuac_ac_proc_setup_speed_negotiation = uac_ac_process_setup_speed_negotiation,

#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    .uac_as_mic_process_setup_req = NULL,
    .uac_as_mic_process_setup_data = NULL,
    .uac_mic_start_stream = UAC_StartCapture,
    .uac_mic_stop_stream = UAC_StopCapture,
#endif

#if defined(CONFIG_USB_GADGET_UAC_SPK_SUPPORT)
    .uac_as_spk_process_setup_req = NULL,
    .uac_as_spk_process_setup_data = NULL,
    .uac_spk_init_stream = uac_spk_init,
    .uac_spk_start_stream = uac_spk_send_frame,
    .uac_spk_stop_stream = uac_spk_stop,
#endif
};

void uac_app_init(void)
{
#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    struct uac_app_para *puac_para;
    u32 aud_idx = 0;
#endif

    ST_AudioDefaultArgs();

#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    CamOsThreadAttrb_t threadUACAttr[MAX_AUD_DEV_NUM] =
    {
        {.nPriority = 99,.szName = "UAC_APP",.nStackSize = 6144/*3072 * 10 ???*/},
#if (MAX_AUD_DEV_NUM > 1)
        {.nPriority = 99,.szName = "UAC_APP2",.nStackSize = 6144/*3072 * 10 ???*/},
#endif
#if (MAX_AUD_DEV_NUM > 2)
        {.nPriority = 99,.szName = "UAC_APP3",.nStackSize = 6144/*3072 * 10 ???*/},
#endif
#if (MAX_AUD_DEV_NUM > 3)
        {.nPriority = 99,.szName = "UAC_APP4",.nStackSize = 6144/*3072 * 10 ???*/},
#endif
    };

    for(aud_idx = 0; aud_idx < MAX_AUD_DEV_NUM; ++aud_idx)
    {
        puac_para = &(guac_app_para[aud_idx]);
        if (CamOsThreadCreate(&puac_para->uacThread, &threadUACAttr[aud_idx], UAC_Audio_Playback_Task, puac_para) != CAM_OS_OK)
        {
            CamOsPrintf(KERN_ERR"creat audio playback task fail\n");
        }
    }
#endif
}

void uac_app_deinit(void)
{
 #if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    struct uac_app_para *puac_para;
    u32 aud_idx = 0;

    for(aud_idx = 0; aud_idx < MAX_AUD_DEV_NUM; ++aud_idx)
    {
        CamOsPrintf(KERN_INFO"CamOsThreadStop\n");
        puac_para = &(guac_app_para[aud_idx]);
        CamOsThreadStop(puac_para->uacThread);
    }
#endif
}

MI_BOOL uac_app_is_stop(void)
{
    if (!uac_running)
    {
        return 1;
    }
    return 0;
}
#endif //#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)

