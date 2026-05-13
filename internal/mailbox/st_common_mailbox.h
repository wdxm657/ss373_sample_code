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
#ifndef _ST_COMMON_MAILBOX_H_
#define _ST_COMMON_MAILBOX_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mdrv_msys_io.h"
#include "mdrv_msys_io_st.h"
#include "mdrv_verchk.h"
#include "mi_sys.h"
#include <sys/ioctl.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include "ss_mbx.h"
#include "cam_os_wrapper.h"


#define CMR_MBX_CLASS_BASE 0

////////////////////////////////
#define PM_RTOS_IMAGE_HEADER_OFFSET       0
#define PM_RTOS_IMAGE_HEADER_IVT          0
#define PM_RTOS_IMAGE_HEADER_MAKER        1376
#define PM_RTOS_IMAGE_HEADER_SIZE         1380
#define PM_RTOS_IMAGE_HEADER_CHECKSUM     1384
#define PM_RTOS_IMAGE_HEADER_ROM_SECTION1 1388
#define PM_RTOS_IMAGE_HEADER_ROM_SECTION2 1392

#define GET_PM_RTOS_IMAGE_MAKER(buf)    (*((volatile MI_U32 *)(buf + PM_RTOS_IMAGE_HEADER_OFFSET + PM_RTOS_IMAGE_HEADER_MAKER)))
#define GET_PM_RTOS_IMAGE_HEADER_SIZE()     (sizeof(pm_rtos_image_header_t))
#define GET_PM_RTOS_IMAGE_SIZE(buf)         (*((volatile MI_U32 *)(buf + PM_RTOS_IMAGE_HEADER_OFFSET + PM_RTOS_IMAGE_HEADER_SIZE)))
#define GET_PM_RTOS_IMAGE_LOAD_ADDR()       (0x27C08000)

#define REG_ADDR_BASE_PM_SLEEP (0x0e)
#define REG_ADDR_BASE_CM4      (0x40)

#define REG_ADDR_BASE_PM_SLEEP  (0x0e)
#define REG_ADDR_BASE_PM_TOP    (0x1e)
#define REG_ADDR_BASE_PMPADTOP  (0x3f)
#define REG_ADDR_BASE_CM4       (0x40)
#define REG_ADDR_BASE_HPB       (0x55)
#define REG_ADDR_BASE_CHIPTOP   (0x101e)

struct mma_buf_info
{
    void       *pVirAddr;
    MI_PHY      phyAddr;
    MI_U32      u32Size;
};

struct image_info
{
    struct mma_buf_info stImageBuf;
    MI_U32 u32HeaderSize;
    MI_U32 u32ImageSize;
    MI_U32 u32DataSize;
    MI_U32 u32DataOffset;
    MI_U32 u32DataCompType;
};

typedef struct pm_rtos_image_header
{
    MI_U8  u8Ivt[PM_RTOS_IMAGE_HEADER_MAKER]; // Interrupt Vector Table, 32byte
    MI_U32 u32Marker;                          // RTK header, 0x5F4B5452 ( RTK_ )
    MI_U32 u32Size;                            // Image size (include header)
    MI_U32 u32Checksum;                        // Checksum (exclude header)
    MI_U32 u32EndOfRomSection1;             // for ipl bdma move rtos image to pm imi use
    MI_U32 u32EndOfRomSection2;             // for ipl bdma move rtos image to pm psram use
} __attribute__((packed)) pm_rtos_image_header_t;
////////////////////////////////

// BDMA image descriptor
typedef struct bdma_img_desc {
    u32 u32MagicNum;
    u32 u32Addr;
    u32 u32Size;
    u8  u8MemType;
    u16 u16Width;
    u16 u16Height;
    u8  u8PixelFmt; //0:RAW_PACK8BIT 2:RAW_PACK16BIT
} bdma_img_desc_t;


typedef enum
{
    E_MBX_CLASS_ACK                 = 0,
    E_MBX_CLASS_JPEG                = 1,
    E_MBX_CLASS_STATEMCHN           = 2,
    E_MBX_CLASS_AED                 = 3,
    E_MBX_CLASS_USB                 = 4,

    E_MBX_CLASS_RAW_IMG             = 5,
    E_MBX_CLASS_RAW_IMG_OK          = E_MBX_CLASS_RAW_IMG,
    E_MBX_CLASS_IMG_QUERY           = 6,
    E_MBX_CLASS_IMG_QUERY_RESP      = E_MBX_CLASS_IMG_QUERY,
    E_MBX_CLASS_PSRAM_ACTIVE        = 7,
    E_MBX_CLASS_PSRAM_ACTIVE_RESP   = E_MBX_CLASS_PSRAM_ACTIVE,
    E_MBX_CLASS_BIN_DATA            = 8,
    E_MBX_CLASS_BIN_DATA_RESP       = E_MBX_CLASS_BIN_DATA,
    E_MBX_CLASS_BIN_DATA_OK         = E_MBX_CLASS_BIN_DATA,
    E_MBX_CLASS_HEART_BEAT          = 9,

    E_MBX_CLASS_MAX                 = 10,
    E_MBX_CLASS_NULL
}Mbx_Class_e;

typedef enum
{
    E_MBX_MEMTYPE_IMI   = 0,
    E_MBX_MEMTYPE_PSRAM = 1,
    E_MBX_MEMTYPE_MAX
}MBX_MEMTYPE_ID;

enum en_mailbox_jpeg_ack_type
{
    /*E_MBX_JPEG_ENABLE         = 0,
    E_MBX_JPEG_DISABLE        = 1,
    E_MBX_JPEG_SNAPSHOT       = 2,
    E_MBX_JPEG_RECV_DONE      = 3,
    E_MBX_JPEG_MAX            = 0xFF*/
    E_MBX_ACK_SUCCESS,
    E_MBX_ACK_TIMEOUT,
    E_MBX_ACK_ERROR,
};

enum en_mailbox_statemachine
{
    E_MBX_STATEMCHN_DEAD,
    E_MBX_STATEMCHN_ALIVE,
    E_MBX_STATEMCHN_KILLME,
    E_MBX_STATEMCHN_SLEEP,
    E_MBX_STATEMCHN_IDLE,
    E_MBX_STATEMCHN_MAX,
};

MI_S32 ST_Common_Mailbox_Enable(void);
MI_S32 ST_Common_Mailbox_Disable(void);
MI_S32 ST_Common_Mailbox_SendMsg(SS_Mbx_Msg_t *stMsg,MI_U32 TimeoutInMs);
MI_U32 ST_Common_set_NonPM_StateMachine(enum en_mailbox_statemachine state);
MI_S32 ST_Common_Mailbox_ReceiveMsg(Mbx_Class_e msgClass, SS_Mbx_Msg_t *pmsg, s32 s32WaitMs);
MI_S32 ST_Common_Mailbox_KillMe(void);
void riu_write(MI_U32 g_bank, MI_U32 offset, MI_U16 val);
MI_U32 riu_read(MI_U32 g_bank, MI_U32 offset);
MI_S32 RunPmrtos(char* rtos_path);
enum en_mailbox_statemachine ST_Common_CkeckPM_State(void);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_MAILBOX_H_



