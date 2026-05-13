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

#include "st_common_mailbox.h"
#include "st_common.h"

static MI_U8 *g_map_base = NULL;
static MI_S32            msys_fd    = -1;
static MI_S32            mem_fd     = -1;


const char * class_to_str(Mbx_Class_e class)
{
    switch (class)
    {
        case E_MBX_CLASS_ACK:
            return "ack";
        case E_MBX_CLASS_JPEG:
            return "jpg";
        case E_MBX_CLASS_STATEMCHN:
            return "stat";
        case E_MBX_CLASS_AED:
            return "aed";
        case E_MBX_CLASS_USB:
            return "usb";
        case E_MBX_CLASS_RAW_IMG:
            return "raw-img";
        case E_MBX_CLASS_IMG_QUERY:
            return "raw-img-query";
        case E_MBX_CLASS_PSRAM_ACTIVE:
            return "psram-active";
        case E_MBX_CLASS_BIN_DATA:
            return "bindata";
        case E_MBX_CLASS_HEART_BEAT:
            return "hb";
        default:
            ST_ERR("unknown class:%d.\n", class);
            return "unknown";
    }
}

const char * state_to_str(enum en_mailbox_statemachine state)
{
    switch (state)
    {
        case E_MBX_STATEMCHN_DEAD:
            return "dead";
        case E_MBX_STATEMCHN_ALIVE:
            return "alive";
        case E_MBX_STATEMCHN_KILLME:
            return "killme";
        case E_MBX_STATEMCHN_SLEEP:
            return "sleep";
        case E_MBX_STATEMCHN_IDLE:
            return "idle";
        default:
            ST_ERR("unknown state:%d.\n", state);
            return "unknown";
    }
}

void ST_Common_Mailbox_Show_Msg(const char* dire, SS_Mbx_Msg_t *pmsg)
{
    MI_S32 i = 0;
    ST_DBG("%s msg:class:%s para_cnt:%d para:", dire, class_to_str(pmsg->u8MsgClass), pmsg->u8ParameterCount);
    for(i = 0; i < pmsg->u8ParameterCount; i++)
    {
        ST_DBG("%u \n", pmsg->u16Parameters[i]);
    }
    ST_DBG("\n");
}

MI_S32 riu_init(void)
{
    MI_U8 *map_base = NULL;
    MSYS_MMIO_INFO info;

    /*  RIU Base mapping */
    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);

    msys_fd = open("/dev/msys", O_RDWR | O_SYNC);
    if (-1 == msys_fd)
    {
        printf("can't open /dev/msys\n");
        return -1;
    }
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (-1 == mem_fd)
    {
        printf("can't open /dev/mem\n");
        close(msys_fd);
        return -1;
    }

    if (0 != ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("DMEM request failed!!\n");
        close(msys_fd);
        msys_fd = -1;
        close(mem_fd);
        mem_fd = -1;
        return -1;
    }

    map_base = mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, info.addr);
    if (map_base == 0)
    {
        printf("mmap failed. NULL pointer!\n");

        close(msys_fd);
        msys_fd = -1;
        close(mem_fd);
        mem_fd = -1;
        return -1;
    }
    g_map_base = map_base;
    return 0;
}

MI_S32 riu_deinit(void)
{
    /* Close application */
    if (g_map_base != NULL)
    {
        munmap(g_map_base, 0xff);
        g_map_base = NULL;
    }
    if (msys_fd != -1)
    {
        close(msys_fd);
        msys_fd = -1;
    }
    if (mem_fd != -1)
    {
        close(mem_fd);
        mem_fd = -1;
    }
    return 0;
}

void riu_write(MI_U32 g_bank, MI_U32 offset, MI_U16 val)
{
    MI_VIRT addr;
    addr                    = (MI_VIRT)(g_map_base + g_bank * 0x200 + offset * 4);
    *(MI_U16 *)addr = val;
    printf("riu_w 0x%x 0x%x 0x%x \n", g_bank, offset, val);
}

MI_U32 riu_read(MI_U32 g_bank, MI_U32 offset)
{
    MI_VIRT addr;
    MI_U32  val;
    addr = (MI_VIRT)(g_map_base + g_bank * 0x200 + offset * 4);
    val  = *(MI_U16 *)addr;
    // printf("riu_r 0x%x 0x%x 0x%x \n", g_bank, offset, val);
    return val;
}


void riu_write_mask(MI_U32 g_bank, MI_U32 offset, MI_U16 mask, MI_U16 val)
{
    MI_VIRT addr;
    MI_U16 tmp;
    addr            = (MI_VIRT)(g_map_base + g_bank * 0x200 + offset * 4);
    tmp = *(MI_U16 *)addr;
    tmp = tmp & (~mask);
    tmp |= val & mask;
    *(MI_U16 *)addr = tmp;
    printf("riu_w 0x%x 0x%x 0x%x 0x%x \n", g_bank, offset, val, mask);
}


MI_U32 ST_Common_set_NonPM_StateMachine(enum en_mailbox_statemachine state)
{
    MI_U32 val;
    val = riu_read(0x31, 0x19);
    val &= 0xfff8;
    val |= state; // alive
    riu_write(0x31, 0x19, val);
    return 0;
}

enum en_mailbox_statemachine ST_Common_CkeckPM_State(void)
{
    MI_U32 val;
    val = riu_read(0x31, 0x18);
    val &= 0x0001;
    if (val)
    {
        return E_MBX_STATEMCHN_ALIVE;
    }
    return E_MBX_STATEMCHN_DEAD;
}


MI_S32 ST_Common_Mailbox_Enable(void)
{
    MI_S32 s32Ret = 0;
    MI_U8 class = 0;
    s32Ret=riu_init();
    if (s32Ret != MI_SUCCESS)
    {
        printf("riu_init failed\n");
        return s32Ret;
    }

    s32Ret = SS_Mailbox_Init();
    if (s32Ret)
    {
        ST_ERR("SS_Mailbox_Init failed,s32Ret: %d\n",s32Ret);
        return s32Ret;
    }


    for (class = E_MBX_CLASS_ACK; class < E_MBX_CLASS_MAX; class++)
    {
        s32Ret |= SS_Mailbox_Enable(class);
        if (s32Ret)
        {
            SS_Mailbox_Deinit();
            ST_ERR("SS_Mailbox_Enable failed,class: %d\n",class);
            return s32Ret;
        }
    }

    return s32Ret;
}

MI_S32 ST_Common_Mailbox_Disable(void)
{
    MI_S32 s32Ret = 0;
    MI_U8 class = 0;

    for (class = E_MBX_CLASS_ACK; class < E_MBX_CLASS_MAX; class++)
    {
        s32Ret |= SS_Mailbox_Disable(class);
        if (s32Ret)
        {
            ST_ERR("SS_Mailbox_Disable failed,class: %d\n",class);
            return s32Ret;
        }
    }

    s32Ret |= SS_Mailbox_Deinit();
    s32Ret |= riu_deinit();
    return s32Ret;
}

MI_S32 ST_Common_Mailbox_SendMsg(SS_Mbx_Msg_t *stMsg,MI_U32 TimeoutInMs)
{
    MI_S32 s32Ret = 0;
    s32Ret |= SS_Mailbox_SendMsg(stMsg, TimeoutInMs);
    if (s32Ret)
    {
        ST_ERR("SS_Mailbox_SendMsg failed,ret:%d\n",s32Ret);
        return s32Ret;
    }
    ST_Common_Mailbox_Show_Msg("send", stMsg);
    return s32Ret;
}

MI_S32 ST_Common_Mailbox_ReceiveMsg(Mbx_Class_e msgClass, SS_Mbx_Msg_t *pmsg, s32 s32WaitMs)
{
    MI_S32 s32Ret = 0;

    pmsg->u8MsgClass = msgClass;
    pmsg->eDirect = E_SS_MBX_DIRECT_ARM_TO_CM4;

    s32Ret = SS_Mailbox_RecvMsg(msgClass, pmsg, s32WaitMs);
    if (s32Ret)
    {
        ST_ERR("SS_Mailbox_RecvMsg failed,ret:%d\n",s32Ret);
        return s32Ret;
    }
    ST_Common_Mailbox_Show_Msg("recv", pmsg);
    return s32Ret;
}


MI_S32 ST_Common_Mailbox_KillMe(void)
{
    SS_Mbx_Msg_t stMbxMsg;
    stMbxMsg.eDirect          = E_SS_MBX_DIRECT_ARM_TO_CM4;
    stMbxMsg.u8Ctrl           = 1;
    stMbxMsg.u8Status         = 0x0;
    stMbxMsg.u8MsgClass       = E_MBX_CLASS_STATEMCHN;
    stMbxMsg.u8ParameterCount = 1;
    stMbxMsg.u16Parameters[0] = E_MBX_STATEMCHN_KILLME;
    ST_Common_set_NonPM_StateMachine(E_MBX_STATEMCHN_DEAD);
    return ST_Common_Mailbox_SendMsg(&stMbxMsg,1000);
}

/* run pmrtos  */
MI_BOOL ST_NONPM_CheckPmRtosImageMagic(void *pImageBuf)
{
    return (GET_PM_RTOS_IMAGE_MAKER(pImageBuf) == 0x5F4B5452);
}

void ST_NONPM_GetImageHeaderInfo(struct mma_buf_info *pImageBuf, struct image_info *pImageInfo)
{
    pImageInfo->u32HeaderSize     = GET_PM_RTOS_IMAGE_HEADER_SIZE();
    pImageInfo->u32DataSize       = GET_PM_RTOS_IMAGE_SIZE(pImageBuf->pVirAddr);
    pImageInfo->u32ImageSize      = GET_PM_RTOS_IMAGE_SIZE(pImageBuf->pVirAddr);
    pImageInfo->u32DataOffset     = 0;
    pImageInfo->u32DataCompType  = 0;
    pImageInfo->stImageBuf = *pImageBuf;
}

MI_S32 ST_NONPM_GetImageInfo(struct mma_buf_info *pstImageBuf, struct image_info *pstImageInfo)
{
    if (FALSE == ST_NONPM_CheckPmRtosImageMagic(pstImageBuf->pVirAddr))
    {
        printf("image is not pm rtos.\n");
        return -1;
    }
    printf("check image magic.\n");
    ST_NONPM_GetImageHeaderInfo(pstImageBuf, pstImageInfo);
    printf("get image header info.\n");

/*
* For common images format:
*
*                   -------> data offset
*                   |
* |<--header size-->|
* ----------------------------------------------
* |     header      |           binary         |
* ----------------------------------------------
*                   |<---------data_size------>|
* |<------------------image_size-------------->|
*
*/
    return MI_SUCCESS;
}

MI_BOOL ST_NONPM_HpbCheckPmInitDone(void)
{
    if ((riu_read(REG_ADDR_BASE_PM_TOP, 0x48)) & (1 << 5))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void ST_NONPM_HpbSetFactory(void)
{
    switch (riu_read(REG_ADDR_BASE_CHIPTOP, 0x48) & 0x00FF)
    {
        case 0x16: // SSI6D55Q_2nd PSRAM AP 32Mbit
        case 0x1E: // SSI6D55Q_4th PSRAM AP 32Mbit
        case 0x17: // SSI6D60Q_2nd PSRAM AP 32Mbit
        case 0x1F: // SSI6D60Q_4th PSRAM AP 32Mbit
        case 0x14: // SSI6D50D_2nd PSRAM AP 32Mbit
        case 0x1C: // SSI6D50D_4th PSRAM AP 32Mbit
        case 0x15: // SSI6D50Q_2nd PSRAM AP 32Mbit
        case 0x1D: // SSI6D50Q_4th PSRAM AP 32Mbit
            riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, 0x000F, 0x0001);
            break;
        case 0x12: // SSI6D55Q     PSRAM WB 32Mbit
        case 0x1A: // SSI6D55Q_3rd PSRAM WB 32Mbit
        case 0x13: // SSI6D60Q     PSRAM WB 32Mbit
        case 0x1B: // SSI6D60Q_3rd PSRAM WB 32Mbit
        case 0x10: // SSI6D50D     PSRAM WB 32Mbit
        case 0x18: // SSI6D50D_3rd PSRAM WB 32Mbit
        case 0x11: // SSI6D50Q     PSRAM WB 32Mbit
        case 0x19: // SSI6D50Q_3rd PSRAM WB 32Mbit
            riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, 0x000F, 0x0002);
            break;
        case 0x56: // SSI6D55Q_6th PSRAM AP 64Mbit
        case 0x5E: // SSI6D55Q_8th PSRAM AP 64Mbit
        case 0x57: // SSI6D60Q_6th PSRAM AP 64Mbit
        case 0x5F: // SSI6D60Q_8th PSRAM AP 64Mbit
        case 0x54: // SSI6D50D_6th PSRAM AP 64Mbit
        case 0x5C: // SSI6D50D_8th PSRAM AP 64Mbit
        case 0x55: // SSI6D50Q_6th PSRAM AP 64Mbit
        case 0x5D: // SSI6D50Q_8th PSRAM AP 64Mbit
            riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, 0x000F, 0x0003);
            break;
        case 0x52: // SSI6D55Q_5th PSRAM WB 64Mbit
        case 0x5A: // SSI6D55Q_7th PSRAM WB 64Mbit
        case 0x53: // SSI6D60Q_5th PSRAM WB 64Mbit
        case 0x5B: // SSI6D60Q_7th PSRAM WB 64Mbit
        case 0x50: // SSI6D50D_5th PSRAM WB 64Mbit
        case 0x58: // SSI6D50D_7th PSRAM WB 64Mbit
        case 0x51: // SSI6D50Q_5th PSRAM WB 64Mbit
        case 0x59: // SSI6D50Q_7th PSRAM WB 64Mbit
            riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, 0x000F, 0x0004);
            break;
        default:
            return;
    }
}

MI_U16 ST_NONPM_HpbGetFactory(void)
{
    return (riu_read(REG_ADDR_BASE_PM_TOP, 0x48) & 0x000f);
}

void ST_NONPM_HpbInitAp4m(void)
{
    // PSRAM ICG
    riu_write_mask(REG_ADDR_BASE_PM_SLEEP, 0x47, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3));

    // --- DLY CHAIN [3:0] clkph_rdqs
    riu_write(REG_ADDR_BASE_HPB, 0x26, 0x0000);
    // MCG ON
    riu_write(REG_ADDR_BASE_HPB, 0x70, 0x0000);
    //[0] sw reset
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0000);
    //[0] wb oen
    riu_write(REG_ADDR_BASE_HPB, 0x25, 0x0000);
    //[11:0] PSRAM size
    riu_write(REG_ADDR_BASE_HPB, 0x40, 0x0004);
    // [4] init mode, [1:0] pkg
    riu_write(REG_ADDR_BASE_HPB, 0x01, 0x0001);
    //// [0] ck enable
    riu_write(REG_ADDR_BASE_HPB, 0x02, 0x0000);
    // [0] cmd_oenz, [5:4] tCSS, [8:11] rwds_cnt, [14:12] BL
    riu_write(REG_ADDR_BASE_HPB, 0x03, 0x0020);
    // [2:0] wrap BL
    riu_write(REG_ADDR_BASE_HPB, 0x07, 0x0000);
    // MR0
    riu_write(REG_ADDR_BASE_HPB, 0x04, 0x000d);
    // [11:8] WL, [3:0] RL
    riu_write(REG_ADDR_BASE_HPB, 0x05, 0x0002);
    // [7:0] tRP
    riu_write(REG_ADDR_BASE_HPB, 0x10, 0x0a06);
    // [11:8] tCSHIW, [3:0] tCSHIR
    riu_write(REG_ADDR_BASE_HPB, 0x11, 0x0502);
    // [7:0] tXPHS
    riu_write(REG_ADDR_BASE_HPB, 0x12, 0x0064);

    // sw insert delay for tVCS 150 us
    // [4] init mode, [1:0] pkg
    riu_write(REG_ADDR_BASE_HPB, 0x01, 0x0011);
    usleep(1);

    // [4] init start
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0010);
    usleep(1);

    // unmask
    // [7:0] garb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x24, 0x0000);
    // [1:0] parb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x31, 0x0000);
    // assign client id 6 to BDMA
    riu_write(REG_ADDR_BASE_HPB, 0x38, 0x0000);
    // arb tog
    riu_write(REG_ADDR_BASE_HPB, 0x20, 0x0001);
    riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, (1 << 5), (1 << 5));
}

void ST_NONPM_HpbInitWb4m(void)
{
    // PSRAM ICG
    riu_write_mask(REG_ADDR_BASE_PM_SLEEP, 0x47, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3));

    // --- DLY CHAIN [3:0] clkph_rdqs
    riu_write(REG_ADDR_BASE_HPB, 0x26, 0x0000);

    // MCG ON
    riu_write(REG_ADDR_BASE_HPB, 0x70, 0x0000);
    //[0] sw reset
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0000);
    //[0] wb oen
    riu_write(REG_ADDR_BASE_HPB, 0x25, 0x0001);
    //[11:0] PSRAM size
    riu_write(REG_ADDR_BASE_HPB, 0x40, 0x0004);
    //// [0] ck enable
    riu_write(REG_ADDR_BASE_HPB, 0x02, 0x0000);
    // [0] cmd_oenz, [5:4] tCSS, [8:11] rwds_cnt, [14:12] BL
    riu_write(REG_ADDR_BASE_HPB, 0x03, 0x0310);
    // [2:0] wrap BL
    riu_write(REG_ADDR_BASE_HPB, 0x07, 0x0000);
    // MR0
    riu_write(REG_ADDR_BASE_HPB, 0x04, 0x8f04);
    // [11:8] WL, [3:0] RL
    riu_write(REG_ADDR_BASE_HPB, 0x05, 0x0503);
    // [7:0] tRP
    riu_write(REG_ADDR_BASE_HPB, 0x10, 0x000a);
    // [11:8] tCSHIW, [3:0] tCSHIR
    riu_write(REG_ADDR_BASE_HPB, 0x11, 0x0101);

    // sw insert delay for tVCS 150 us
    // [4] init mode, [1:0] pkg
    riu_write(REG_ADDR_BASE_HPB, 0x01, 0x0000);
    usleep(1);

    // [4] init start
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0010);
    usleep(1);

    // unmask
    // [7:0] garb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x24, 0x0000);
    // [1:0] parb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x31, 0x0000);
    // assign client id 6 to BDMA
    riu_write(REG_ADDR_BASE_HPB, 0x38, 0x0000);
    // arb tog
    riu_write(REG_ADDR_BASE_HPB, 0x20, 0x0001);

    riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, (1 << 5), (1 << 5));
}

void ST_NONPM_HpbInitAp8m(void)
{
    // PSRAM ICG
    riu_write_mask(REG_ADDR_BASE_PM_SLEEP, 0x47, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3));

    // --- DLY CHAIN [3:0] clkph_rdqs
    riu_write(REG_ADDR_BASE_HPB, 0x26, 0x0000);

    // MCG ON
    riu_write(REG_ADDR_BASE_HPB, 0x70, 0x0000);
    //[0] sw reset
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0000);
    //[0] wb oen
    riu_write(REG_ADDR_BASE_HPB, 0x25, 0x0000);
    //[11:0] PSRAM size
    riu_write(REG_ADDR_BASE_HPB, 0x40, 0x0008);
    // [4] init mode, [1:0] pkg
    riu_write(REG_ADDR_BASE_HPB, 0x01, 0x0001);
    //// [0] ck enable
    riu_write(REG_ADDR_BASE_HPB, 0x02, 0x0000);
    // [0] cmd_oenz, [5:4] tCSS, [8:11] rwds_cnt, [14:12] BL
    riu_write(REG_ADDR_BASE_HPB, 0x03, 0x0020);
    // [2:0] wrap BL
    riu_write(REG_ADDR_BASE_HPB, 0x07, 0x0000);
    // MR0
    riu_write(REG_ADDR_BASE_HPB, 0x04, 0x000d);
    // [11:8] WL, [3:0] RL
    riu_write(REG_ADDR_BASE_HPB, 0x05, 0x0002);
    // [7:0] tRP
    riu_write(REG_ADDR_BASE_HPB, 0x10, 0x0a06);
    // [11:8] tCSHIW, [3:0] tCSHIR
    riu_write(REG_ADDR_BASE_HPB, 0x11, 0x0502);
    // [7:0] tXPHS
    riu_write(REG_ADDR_BASE_HPB, 0x12, 0x0064);

    // sw insert delay for tVCS 150 us
    // [4] init mode, [1:0] pkg
    riu_write(REG_ADDR_BASE_HPB, 0x01, 0x0231);
    usleep(1);

    // [4] init start
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0010);
    usleep(1);

    // unmask
    // [7:0] garb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x24, 0x0000);
    // [1:0] parb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x31, 0x0000);
    // assign client id 6 to BDMA
    riu_write(REG_ADDR_BASE_HPB, 0x38, 0x0000);
    // arb tog
    riu_write(REG_ADDR_BASE_HPB, 0x20, 0x0001);
    riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, (1 << 5), (1 << 5));
}

void ST_NONPM_HpbInitWb8m(void)
{
    MI_U32 u32TimeoutCnt = 10;

    // PSRAM ICG
    riu_write_mask(REG_ADDR_BASE_PM_SLEEP, 0x47, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3));

    // --- DLY CHAIN [3:0] clkph_rdqs
    riu_write(REG_ADDR_BASE_HPB, 0x26, 0x0000);

    // MCG ON
    riu_write(REG_ADDR_BASE_HPB, 0x70, 0x0000);
    //[0] sw reset
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0000);
    //[0] wb oen
    riu_write(REG_ADDR_BASE_HPB, 0x25, 0x0001);
    //[11:0] PSRAM size
    riu_write(REG_ADDR_BASE_HPB, 0x40, 0x0008);
    //// [0] ck enable
    riu_write(REG_ADDR_BASE_HPB, 0x02, 0x0000);
    // [0] cmd_oenz, [5:4] tCSS, [8:11] rwds_cnt, [14:12] BL
    riu_write(REG_ADDR_BASE_HPB, 0x03, 0x6310);
    // [2:0] wrap BL
    riu_write(REG_ADDR_BASE_HPB, 0x07, 0x0003);
    // MR0
    riu_write(REG_ADDR_BASE_HPB, 0x04, 0x8f07);
    // [11:8] WL, [3:0] RL
    riu_write(REG_ADDR_BASE_HPB, 0x05, 0x0503);
    // [7:0] tRP
    riu_write(REG_ADDR_BASE_HPB, 0x10, 0x000a);
    // [11:8] tCSHIW, [3:0] tCSHIR
    riu_write(REG_ADDR_BASE_HPB, 0x11, 0x0101);

    // sw insert delay for tVCS 150 us
    // [4] init mode, [1:0] pkg
    riu_write(REG_ADDR_BASE_HPB, 0x01, 0x0000);
    usleep(1);

    // [4] init start
    riu_write(REG_ADDR_BASE_HPB, 0x00, 0x0010);
    usleep(1);

    // polling done
    while (u32TimeoutCnt--)
    {
        if (riu_read(REG_ADDR_BASE_HPB, 0X0F) == 0x71)
        {
            printf("psram init done!\n");
            break;
        }
        usleep(1);
    }
    if (!u32TimeoutCnt)
    {
        printf("psram init fail!\n");
        return;
    }

    // unmask
    // [7:0] garb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x24, 0x0000);
    // [1:0] parb rq mask
    riu_write(REG_ADDR_BASE_HPB, 0x31, 0x0000);
    // assign client id 6 to BDMA
    riu_write(REG_ADDR_BASE_HPB, 0x38, 0x0000);
    // arb tog
    riu_write(REG_ADDR_BASE_HPB, 0x20, 0x0001);

    riu_write_mask(REG_ADDR_BASE_PM_TOP, 0x48, (1 << 5), (1 << 5));
}

void ST_NONPM_HpbBist(void)
{
    MI_U32 u32LoopCnt = 0;
    // --- bist
    // wriu -w 0x005570 0x0001
    riu_write(REG_ADDR_BASE_HPB, 0x38, 0x0001);
    // wriu -w 0x005540 0x0001
    riu_write(REG_ADDR_BASE_HPB, 0x20, 0x0001);
    // wriu -w 0x0055a8 0x2000
    riu_write(REG_ADDR_BASE_HPB, 0x54, 0x2000);
    // wriu -w 0x0055aa 0x0000
    riu_write(REG_ADDR_BASE_HPB, 0x55, 0x0000);
    // wriu -w 0x0055ae 0x1000
    riu_write(REG_ADDR_BASE_HPB, 0x57, 0x1000);
    // wriu -w 0x0055b0 0x0000
    riu_write(REG_ADDR_BASE_HPB, 0x58, 0x0000);
    // wriu -w 0x0055a0 0x0001
    riu_write(REG_ADDR_BASE_HPB, 0x50, 0x0001);
    // read addr : 0x0055b2, check data : 0x0014
    printf("psram bist.\n");
    while ((riu_read(REG_ADDR_BASE_HPB, 0x59) & 0x00FF) != 0x14)
    {
        if (u32LoopCnt++ > 100)
        {
            printf("NG(0x%x)\n", riu_read(REG_ADDR_BASE_HPB, 0x59));
            printf("psram bist fail\n");
        }
        usleep(5 * 1000);
    }
    printf("psram bist ok\n");

    // disable BIST - reg_test_en: 0
    riu_write(REG_ADDR_BASE_HPB, 0x50, 0x0000);
    // assign client id 6 to BDMA
    riu_write(REG_ADDR_BASE_HPB, 0x38, 0x0000);
    // arb trigger
    riu_write(REG_ADDR_BASE_HPB, 0x20, 0x0001);
}

void ST_NONPM_HpbInit(void)
{
    MI_U16 test=0;
    if (ST_NONPM_HpbCheckPmInitDone())
    {
        return;
    }

    // pad mux
    riu_write(REG_ADDR_BASE_PMPADTOP, 0x36, 0x001);

    if (ST_NONPM_HpbGetFactory() == 0)
    {
        ST_NONPM_HpbSetFactory();
    }
    test=ST_NONPM_HpbGetFactory();
    printf("++++++++++++++++++++++test:%u\n",test);
    switch (ST_NONPM_HpbGetFactory())
    {
        case 1:
            ST_NONPM_HpbInitAp4m();
            break;

        case 2:
            ST_NONPM_HpbInitWb4m();
            break;

        case 3:
            ST_NONPM_HpbInitAp8m();
            break;

        case 4:
            ST_NONPM_HpbInitWb8m();
            break;
    }
    ST_NONPM_HpbBist();
}

MI_S32 ST_NONPM_RunPmRtos(struct mma_buf_info *pstImageBuf)
{
    MI_U8                   u8Index, u8i;
    MI_U32                  u32TcmImageSize, u32ImiImageSize, u32PsramImageSize;
    pm_rtos_image_header_t *pstPmRtosImageheader = NULL;
    MI_S32                  s32Ret = MI_SUCCESS;
    struct mma_buf_info     stCheckBuf = {0};
    char fileName[256];

    struct image_info stImageInfo = {0};
    printf("before get image info.\n");
    s32Ret = ST_NONPM_GetImageInfo(pstImageBuf, &stImageInfo);
    if (s32Ret)
    {
        printf("can not get image info from image buf.\n");
        return -1;
    }

#if 1
    // add check mma
    s32Ret = ST_Common_AllocateMemory(pstImageBuf->u32Size , &stCheckBuf.phyAddr, &stCheckBuf.pVirAddr);
    if (s32Ret)
    {
        printf("failed to alloc mma buf for check.\n");
        return -1;
    }

    stCheckBuf.u32Size = pstImageBuf->u32Size;
    memset(stCheckBuf.pVirAddr, 0x0, stCheckBuf.u32Size);
#endif
    printf("get image info.\n");

    pstPmRtosImageheader = (pm_rtos_image_header_t *)(stImageInfo.stImageBuf.pVirAddr);
    printf("marker:%d\r\n", pstPmRtosImageheader->u32Marker);
    printf("size:%d\r\n", pstPmRtosImageheader->u32Size);
    printf("checksum:%d\r\n", pstPmRtosImageheader->u32Checksum);
    printf("end_of_rom_section1:%d\r\n", pstPmRtosImageheader->u32EndOfRomSection1);
    printf("end_of_rom_section2:%d\r\n", pstPmRtosImageheader->u32EndOfRomSection2);

    // set cm4 reset
    riu_write(REG_ADDR_BASE_PM_SLEEP, 0x29, riu_read(REG_ADDR_BASE_PM_SLEEP, 0x29) & (~0x1000));

    printf("reset cm4.\n");

    // reg_tcm_en set 0
    riu_write(REG_ADDR_BASE_CM4, 0x56, 0x0000);

    printf("tcm en 0.\n");

    if (pstPmRtosImageheader->u32EndOfRomSection1 == 0 && pstPmRtosImageheader->u32EndOfRomSection2 == 0)
    {
        // TCM only
        printf("TCM only\n");
        u32TcmImageSize   = pstPmRtosImageheader->u32Size;
        u32ImiImageSize   = 0;
        u32PsramImageSize = 0;
    }
    else if (pstPmRtosImageheader->u32EndOfRomSection2 == 0)
    {
        // TCM + IMI
        printf("TCM + IMI\n");
        u32TcmImageSize   = pstPmRtosImageheader->u32EndOfRomSection1;
        u32ImiImageSize   = pstPmRtosImageheader->u32Size - pstPmRtosImageheader->u32EndOfRomSection1;
        u32PsramImageSize = 0;
    }
    else if (pstPmRtosImageheader->u32EndOfRomSection1 == 0)
    {
        // TCM + PSRAM
        printf("TCM + PSRAM\n");
        u32TcmImageSize   = pstPmRtosImageheader->u32EndOfRomSection2;
        u32ImiImageSize   = 0;
        u32PsramImageSize = pstPmRtosImageheader->u32Size - pstPmRtosImageheader->u32EndOfRomSection2;
    }
    else
    {
        // TCM + IMI + PSRAM
        printf("TCM + IMI + PSRAM\n");
        u32TcmImageSize   = pstPmRtosImageheader->u32EndOfRomSection1;
        u32ImiImageSize   = pstPmRtosImageheader->u32EndOfRomSection2 - pstPmRtosImageheader->u32EndOfRomSection1;
        u32PsramImageSize = pstPmRtosImageheader->u32Size - pstPmRtosImageheader->u32EndOfRomSection2;
    }

    printf("cal tcm/imi/psram.\n");

    u8Index = u32TcmImageSize >> 14;
    u8Index += ((u32TcmImageSize & 0x3FFF) ? 1 : 0);
    printf("u8Index = %d.\n", u8Index);
    for (u8i = 0; u8i < u8Index; u8i++)
    {
        //drv_pm_bdma_miu_to_pmimi(0, load_addr + (0x4000 * i), 0x74000 - (0x4000 * i), MIN(tcm_image_size, 0x4000));
        s32Ret = MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_DRAM_TO_PM_SRAM, 0x74000 - (0x4000 * u8i),
            stImageInfo.stImageBuf.phyAddr + (0x4000 * u8i), MIN(u32TcmImageSize, 0x4000));
        if (s32Ret)
        {
            printf("failed to copy tcm image %d.\n", u8i);
        }
#if 0
        // check data
        s32Ret = MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM, stCheckBuf.phyAddr + (0x4000 * u8i),
            0x74000 - (0x4000 * u8i), MIN(u32TcmImageSize, 0x4000));
        if (s32Ret)
        {
            printf("failed to copy tcm image %d to dram.\n", u8i);
        }

        if (memcmp(stCheckBuf.pVirAddr + (0x4000 * u8i),
            stImageInfo.stImageBuf.pVirAddr + (0x4000 * u8i),
            MIN(u32TcmImageSize, 0x4000)))
        {
            printf("tcm image %d is not same.\n", u8i);

            memset(fileName, 0x0, sizeof(fileName));
            sprintf(fileName, "TcmImageFromDram%d", u8i);
            // ST_Common_WriteFile(fileName, stImageInfo.stImageBuf.pVirAddr + (0x4000 * u8i), MIN(u32TcmImageSize, 0x4000));

            memset(fileName, 0x0, sizeof(fileName));
            sprintf(fileName, "TcmImageFromPmSram%d", u8i);
            //ST_Common_WriteFile(fileName, stCheckBuf.pVirAddr + (0x4000 * u8i), MIN(u32TcmImageSize, 0x4000));
/*
            printf("tcm image from dram:\n");
            for (u32j = 0; u32j < MIN(u32TcmImageSize, 0x4000); u32j++)
            {
                printf("%d ", *(MI_U8 *)(stImageInfo.stImageBuf.pVirAddr + (0x4000 * u8i) + u32j));
                if ((u32j != 0) && (u32j % 10 == 0))
                {
                    printf("\n");
                }
            }
            printf("\n");

            printf("tcm image from pmsram:\n");
            for (u32j = 0; u32j < MIN(u32TcmImageSize, 0x4000); u32j++)
            {
                printf("%d ", *(MI_U8 *)(stCheckBuf.pVirAddr + (0x4000 * u8i) + u32j));
                if ((u32j != 0) && (u32j % 10 == 0))
                {
                    printf("\n");
                }
            }
            printf("\n");
*/
        }
#endif
        u32TcmImageSize -= 0x4000;

#if 0
        // check bdma
        strcpy((char *)stImageInfo.stImageBuf.pVirAddr, "0123456789");
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_DRAM_TO_PM_SRAM, 0,
            stImageInfo.stImageBuf.phyAddr, strlen("0123456789") + 1);
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM,
            stCheckBuf.phyAddr, 0, strlen("0123456789") + 1);
        printf("copy string:%s.\n", (char *)stCheckBuf.pVirAddr);
#endif
    }

    printf("bdma copy tcm.\n");

    // check TCM section only
    if (u32ImiImageSize == 0 && u32PsramImageSize == 0)
    {
        goto boot_pm_rtos;
    }

    // reg_tcm_en set 1
    riu_write(REG_ADDR_BASE_CM4, 0x56, 0x0001);
    printf("tcm en 1.\n");


    if (u32ImiImageSize)
    {

        //drv_pm_bdma_miu_to_pmimi(0, load_addr + header->end_of_rom_section1, 0, imi_image_size);
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_DRAM_TO_PM_SRAM, 0,
            stImageInfo.stImageBuf.phyAddr + pstPmRtosImageheader->u32EndOfRomSection1, u32ImiImageSize);
        if (s32Ret)
        {
            printf("failed to copy imi image.\n");
        }

#if 0
        // check data
        s32Ret = MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM,
            stCheckBuf.phyAddr + pstPmRtosImageheader->u32EndOfRomSection1,
            0, u32ImiImageSize);
        if (s32Ret)
        {
            printf("failed to copy imi image to dram.\n");
        }

        if (memcmp(stCheckBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection1,
            stImageInfo.stImageBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection1,
            u32ImiImageSize))
        {
            printf("imi image is not same.\n");

            memset(fileName, 0x0, sizeof(fileName));
            sprintf(fileName, "ImiImageFromDram");
            // ST_Common_WriteFile(fileName, stImageInfo.stImageBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection1, u32ImiImageSize);

            memset(fileName, 0x0, sizeof(fileName));
            sprintf(fileName, "ImiImageFromPmSram");
            // ST_Common_WriteFile(fileName, stCheckBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection1, u32ImiImageSize);
        }
        printf("bdma copy imi.\n");
#endif

#if 0
        // check bdma
        strcpy((char *)stImageInfo.stImageBuf.pVirAddr, "0123456789");
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_DRAM_TO_PM_SRAM, 0,
            stImageInfo.stImageBuf.phyAddr, strlen("0123456789") + 1);
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM,
            stCheckBuf.phyAddr, 0, strlen("0123456789") + 1);
        printf("copy string:%s.\n", (char *)stCheckBuf.pVirAddr);
#endif
   }
    printf("1111111111111111111.\n");

    if (u32PsramImageSize)
    {
        printf("222222222222222222.\n");
        //drv_pm_bdma_miu_to_psram(0, load_addr + header->end_of_rom_section2, 0, psram_image_size);
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_DRAM_TO_PM_PSRAM, 0,
            stImageInfo.stImageBuf.phyAddr + pstPmRtosImageheader->u32EndOfRomSection2, u32PsramImageSize);
        if (s32Ret)
        {
            printf("failed to copy psram image.\n");
        }

#if 0
        // check data
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_PM_PSRAM_TO_DRAM,
            stCheckBuf.phyAddr + pstPmRtosImageheader->u32EndOfRomSection2, 0, u32PsramImageSize);
        if (s32Ret)
        {
            printf("failed to copy psram image to dram.\n");
        }

        if (memcmp(stCheckBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection2,
            stImageInfo.stImageBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection2,
            u32PsramImageSize))
        {
            printf("psram image is not same.\n");
            memset(fileName, 0x0, sizeof(fileName));
            sprintf(fileName, "PsramImageFromDram");
            // ST_Common_WriteFile(fileName, stImageInfo.stImageBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection2, u32PsramImageSize);

            memset(fileName, 0x0, sizeof(fileName));
            sprintf(fileName, "PsramImageFromPmSram");
            // ST_Common_WriteFile(fileName, stCheckBuf.pVirAddr + pstPmRtosImageheader->u32EndOfRomSection2, u32PsramImageSize);
        }
#endif

#if 0
        // check bdma
        strcpy((char *)stImageInfo.stImageBuf.pVirAddr, "0123456789");
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_DRAM_TO_PM_PSRAM, 0,
            stImageInfo.stImageBuf.phyAddr, strlen("0123456789") + 1);
        MI_SYS_MemcpyPaEx(0, E_MI_SYS_MEMCPY_PM_PSRAM_TO_DRAM,
            stCheckBuf.phyAddr, 0, strlen("0123456789") + 1);
        printf("copy string:%s.\n", (char *)stCheckBuf.pVirAddr);
        //printf("copy from psram, src %lld, dst:%lld.\n", 0ULL, stCheckBuf.phyAddr + pstPmRtosImageheader->u32EndOfRomSection2);
#endif

        printf("bdma copy psram.\n");
    }

boot_pm_rtos:

    printf("run pm rtos\n");

    // release cm4 reset
    riu_write(REG_ADDR_BASE_PM_SLEEP, 0x29, riu_read(REG_ADDR_BASE_PM_SLEEP, 0x29) | (0x1000));
    printf("release cm4 reset.\n");

    return MI_SUCCESS;
}


MI_S32 RunPmrtos(char* rtos_path)
{
    MI_S32 s32Ret = MI_SUCCESS;
    FILE * pfdPmRtos = NULL;
    MI_U32 u32FileSize = 0;
    struct mma_buf_info stBufInfo = {0};
    MI_U32 u32ScanCnt = 0;
    SS_Mbx_Msg_t stMbxMsg = {0};
    enum en_mailbox_statemachine pm_state=E_MBX_STATEMCHN_DEAD;
    /************************************************
    step1 :check cm4 alive
    *************************************************/
    pm_state=ST_Common_CkeckPM_State();
    // cm4 has not receive my scan message
    // it may not be activated yet or it may be dead
    // load cm4 image
    s32Ret = ST_Common_OpenSourceFile(rtos_path, &pfdPmRtos);
    if (s32Ret)
    {
        printf("can not open pm rtos path %s.\n", rtos_path);
        return -1;
    }
    s32Ret = ST_Common_GetSourceFileSize(pfdPmRtos, &u32FileSize);
    if (s32Ret)
    {
        printf("can not get file size.\n");
        return -1;
    }

    stBufInfo.u32Size = u32FileSize;
    // s32Ret = ST_Common_AllocateMemory(u32FileSize, &stBufInfo.phyAddr, &stBufInfo.pVirAddr);
    s32Ret = ST_Common_AllocateMemory(u32FileSize, &stBufInfo.phyAddr, (MI_U8 **)&stBufInfo.pVirAddr);
    if (s32Ret)
    {
        printf("can not alloc memory for pm rtos.\n");
        return -1;
    }

    s32Ret = fread(stBufInfo.pVirAddr, stBufInfo.u32Size, 1, pfdPmRtos);
    if (s32Ret!=1)
    {
        printf("can not read data from file.\n");
        return -1;
    }

    // riu_init();
    ST_NONPM_HpbInit();
    s32Ret = ST_NONPM_RunPmRtos(&stBufInfo);
    if (s32Ret)
    {
        printf("can not run pm rtos.\n");
        return -1;
    }
    // riu_deinit();
    stMbxMsg.u8MsgClass = E_MBX_CLASS_STATEMCHN;
    stMbxMsg.eDirect = E_SS_MBX_DIRECT_ARM_TO_CM4;
    stMbxMsg.u8ParameterCount = 1;
    stMbxMsg.u16Parameters[0] = E_MBX_STATEMCHN_ALIVE;

    do{
        s32Ret = SS_Mailbox_SendMsg(&stMbxMsg, 100);
        u32ScanCnt++;
    }while((s32Ret != MI_SUCCESS) && (u32ScanCnt < 20));

    if (u32ScanCnt >= 20)
    {
        printf("pm rtos did not start successfully.\n");
        return -1;
    }
    printf("pm rtos start successfully.\n");
    return MI_SUCCESS;
}

#if 0
MI_S32 ST_NONPM_PipeInit()
{
    MI_S32 s32Ret = MI_SUCCESS;
    //SS_InterCoreMgr_ScanInfo_t stScanInfo = {0};
    FILE * pfdPmRtos = NULL;
    MI_U32 u32FileSize = 0;
    struct mma_buf_info stBufInfo = {0};
    MI_U32 u32ScanCnt = 0;
    u8 u8MsgClass;
    SS_Mbx_Msg_t stMbxMsg = {0};
    enum en_mailbox_statemachine pm_state=E_MBX_STATEMCHN_DEAD;
    /************************************************
    step1 :check cm4 alive？
    *************************************************/
    STCHECKRESULT(MI_SYS_Init(0));
    STCHECKRESULT(SS_Mailbox_Init());
    for(u8MsgClass = E_MBX_CLASS_ACK; u8MsgClass < E_MBX_CLASS_MAX; u8MsgClass++)
    {
        STCHECKRESULT(SS_Mailbox_Enable(u8MsgClass));
    }
    stMbxMsg.u8MsgClass = E_MBX_CLASS_STATEMCHN;
    stMbxMsg.eDirect = E_SS_MBX_DIRECT_ARM_TO_CM4;
    stMbxMsg.u8ParameterCount = 1;
    stMbxMsg.u16Parameters[0] = E_MBX_STATEMCHN_ALIVE;
    s32Ret = SS_Mailbox_SendMsg(&stMbxMsg, 1000);
    if (MI_SUCCESS != s32Ret)
    {
        // cm4 has not receive my scan message
        // it may not be activated yet or it may be dead
        // load cm4 image
        s32Ret = ST_Common_OpenSourceFile(g_aPmRtosPath, &pfdPmRtos);
        if (s32Ret)
        {
            printf("can not open pm rtos path %s.\n", g_aPmRtosPath);
            return -1;
        }
        s32Ret = ST_Common_GetSourceFileSize(pfdPmRtos, &u32FileSize);
        if (s32Ret)
        {
            printf("can not get file size.\n");
            return -1;
        }

        printf("======1=====\n");

        stBufInfo.u32Size = u32FileSize;
        s32Ret = ST_Common_AllocateMemory(u32FileSize, &stBufInfo.phyAddr, &stBufInfo.pVirAddr);
        if (s32Ret)
        {
            printf("can not alloc memory for pm rtos.\n");
            return -1;
        }

        printf("======2=====\n");

        s32Ret = fread(stBufInfo.pVirAddr, stBufInfo.u32Size, 1, pfdPmRtos);
        if (s32Ret != 1)
        {
            printf("can not read data from file.\n");
            return -1;
        }

        printf("======3=====\n");

        riu_init();
        printf("======3-1=====\n");
        ST_NONPM_HpbInit();
        printf("======3-1-1=====\n");
        s32Ret = ST_NONPM_RunPmRtos(&stBufInfo);
        if (s32Ret)
        {
            printf("can not run pm rtos.\n");
            return -1;
        }
        printf("======3-2=====\n");
        riu_deinit();
        printf("======3-3=====\n");

        printf("======4=====\n");

        do{
            s32Ret = SS_Mailbox_SendMsg(&stMbxMsg, 100);
            u32ScanCnt++;
        }while((s32Ret != MI_SUCCESS) && (u32ScanCnt < 20));

        if (u32ScanCnt >= 20)
        {
            printf("pm rtos did not start successfully.\n");
            return -1;
        }

        printf("======5=====\n");

        // kill me
        stMbxMsg.u8MsgClass = E_MBX_CLASS_STATEMCHN;
        stMbxMsg.eDirect = E_SS_MBX_DIRECT_ARM_TO_CM4;
        stMbxMsg.u8ParameterCount = 1;
        stMbxMsg.u16Parameters[0] = E_MBX_STATEMCHN_KILLME;
        s32Ret = SS_Mailbox_SendMsg(&stMbxMsg, 1000);
        if (s32Ret)
        {
            printf("failed to send mbx msg.\n");
            return -1;
        }
        else
        {
            printf("wait for kill me.\n");

            for(u8MsgClass = E_MBX_CLASS_ACK; u8MsgClass < E_MBX_CLASS_MAX; u8MsgClass++)
            {
                STCHECKRESULT(SS_Mailbox_Disable(u8MsgClass));
            }
            STCHECKRESULT(SS_Mailbox_Deinit());
        }
    }
    return MI_SUCCESS;
}

#endif