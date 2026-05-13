#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "mi_sys.h"
#include "st_common_venc.h"
#include "st_common.h"

typedef struct ST_VencInputParam_s
{
    MI_BOOL bCheckResult;
    MI_U8   u8CmdIndex;
} ST_VencInputParam_t;

static MI_U8 gu8CmdIndex;

typedef struct ST_VencInfo_s
{
    int                         EncodedType;
    ST_Common_InputFile_Attr_t  InputInfo;
    ST_Common_OutputFile_Attr_t OutputInfo;
} ST_VencInfo_t;

static ST_VencInfo_t       VencInfo;

static MI_S32 ST_MoudleInit()
{
    MI_VENC_DEV                 s32DevId = 0;
    MI_VENC_CHN                 s32ChnId = 0;
    MI_VENC_ModType_e           eType    = (MI_VENC_ModType_e)VencInfo.EncodedType;
    MI_VENC_InitParam_t         VencParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    stVencChnAttr.stVeAttr.eType = (MI_VENC_ModType_e)VencInfo.EncodedType;

    VencInfo.InputInfo.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    VencInfo.InputInfo.u32Width            = 1920;
    VencInfo.InputInfo.u32Height           = 1080;
    VencInfo.InputInfo.ePixelFormat        = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    VencInfo.OutputInfo.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;

    STCHECKRESULT(ST_Common_Sys_Init());
    ST_Common_GetVencDefaultDevAttr(&VencParam);
    STCHECKRESULT(ST_Common_VencCreateDev(s32DevId, &VencParam));
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    if(E_MI_VENC_MODTYPE_H264E == eType)
    {
        stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 12;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 12;
    }
    else if (E_MI_VENC_MODTYPE_H265E == eType)
    {
        stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 12;
        stVencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 12;
    }
    STCHECKRESULT(ST_Common_VencStartChn(s32DevId, s32ChnId, &stVencChnAttr));
    return MI_SUCCESS;
}

static MI_S32 ST_ModuleDeInit()
{
    MI_VENC_DEV s32DevId = 0;
    MI_VENC_CHN s32ChnId = 0;
    STCHECKRESULT(ST_Common_VencStopChn(s32DevId, s32ChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(s32DevId));
    STCHECKRESULT(ST_Common_Sys_Exit());
    return MI_SUCCESS;
}

static MI_S32 ST_Pipeline_Preview()
{
    strcpy(VencInfo.InputInfo.InputFilePath,"./resource/input/1920_1080_nv12.yuv");
    sprintf(VencInfo.OutputInfo.FilePath, "./out/venc/venc_demo_case%d", gu8CmdIndex);
    VencInfo.InputInfo.u32Fps = 5;
    VencInfo.OutputInfo.u16DumpBuffNum = 3; //how much yuv frame will be write to venc
    STCHECKRESULT(ST_MoudleInit());
    STCHECKRESULT(pthread_create(&VencInfo.InputInfo.pPutDatathread, NULL, ST_Common_PutInputDataThread, (void *)&VencInfo.InputInfo));
    ST_Common_CheckResult(&VencInfo.OutputInfo);

    ST_Common_WaitDumpFinsh(&VencInfo.OutputInfo);

    VencInfo.InputInfo.bThreadExit = TRUE;
    STCHECKRESULT(pthread_join(VencInfo.InputInfo.pPutDatathread, NULL));
    STCHECKRESULT(ST_ModuleDeInit());
    return MI_SUCCESS;
}

void ST_Venc_Usage(void)
{
    printf("Usage: ./proc_venc_venc_demo 0 work in H264\n");
    printf("Usage: ./proc_venc_venc_demo 1 work in H265\n");
}

MI_S32 main(int argc, char **argv)
{
    if (argc < 2)
    {
        ST_Venc_Usage();
        return -1;
    }

    gu8CmdIndex = atoi(argv[1]);

    switch (gu8CmdIndex)
    {
        case 0:
            VencInfo.EncodedType = E_MI_VENC_MODTYPE_H264E;
            break;
        case 1:
            VencInfo.EncodedType = E_MI_VENC_MODTYPE_H265E;
            break;
        default:
            printf("the index is invaild!\n");
            ST_Venc_Usage();
            return -1;
    }
    STCHECKRESULT(ST_Pipeline_Preview());

    return 0;
}
