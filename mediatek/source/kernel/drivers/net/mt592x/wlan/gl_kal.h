






#ifndef _GL_KAL_H
#define _GL_KAL_H



#include "config.h"
#include "gl_typedef.h"
#include "gl_os.h"
#include "wlan_lib.h"

#if DBG
extern int allocatedMemSize;
#endif


typedef enum _ENUM_SPIN_LOCK_CATEGORY_E {
    SPIN_LOCK_FSM = 0,
    SPIN_LOCK_TX,
    SPIN_LOCK_IO_REQ,
    SPIN_LOCK_INT,
    SPIN_LOCK_NUM
} ENUM_SPIN_LOCK_CATEGORY_E;



/*----------------------------------------------------------------------------*/
/* Macros of SPIN LOCK operations for using in Driver Layer                   */
/*----------------------------------------------------------------------------*/
#define KAL_SPIN_LOCK_DECLARATION()             UINT_32 __u4Flags

#define KAL_ACQUIRE_SPIN_LOCK(_prAdapter, _rLockCategory)   \
            kalAcquireSpinLock(((P_ADAPTER_T)_prAdapter)->prGlueInfo, _rLockCategory, &__u4Flags)

#define KAL_RELEASE_SPIN_LOCK(_prAdapter, _rLockCategory)   \
            kalReleaseSpinLock(((P_ADAPTER_T)_prAdapter)->prGlueInfo, _rLockCategory, __u4Flags)

/*----------------------------------------------------------------------------*/
/* Macros for accessing Reserved Fields of native packet                      */
/*----------------------------------------------------------------------------*/
#define KAL_GET_PKT_QUEUE_ENTRY(_p)             GLUE_GET_PKT_QUEUE_ENTRY(_p)
#define KAL_GET_PKT_DESCRIPTOR(_prQueueEntry)   GLUE_GET_PKT_DESCRIPTOR(_prQueueEntry)
#define KAL_GET_PKT_TID(_p)                     GLUE_GET_PKT_TID(_p)
#define KAL_GET_PKT_IS1X(_p)                    GLUE_GET_PKT_IS1X(_p)
#define KAL_GET_PKT_HEADER_LEN(_p)              GLUE_GET_PKT_HEADER_LEN(_p)
#define KAL_GET_PKT_PAYLOAD_LEN(_p)             GLUE_GET_PKT_PAYLOAD_LEN(_p)
#define KAL_GET_PKT_ARRIVAL_TIME(_p)            GLUE_GET_PKT_ARRIVAL_TIME(_p)

#define kalUpdateReAssocRspInfo(prGlueInfo, pucFrameBody, u4FrameBodyLen)

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#if DBG
#define kalMemAlloc(u4Size) ({    \
    void *pvAddr = kmalloc(u4Size, GFP_NOWAIT);   \
    if (pvAddr) {   \
        allocatedMemSize += u4Size;   \
        printk(KERN_INFO DRV_NAME "0x%p(%ld) allocated (%s:%s)\n", \
            pvAddr, (UINT_32)u4Size, __FILE__, __FUNCTION__);  \
    }   \
    pvAddr; \
    })
#else
#define kalMemAlloc(u4Size)                         kmalloc(u4Size, GFP_NOWAIT) //kmalloc(u4Size, GFP_ATOMIC)
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#if DBG
#define kalMemFree(pvAddr, u4Size)  \
    {   \
        if (pvAddr) {   \
            allocatedMemSize -= u4Size; \
            printk(KERN_INFO DRV_NAME "0x%p(%ld) freed (%s:%s)\n", \
                pvAddr, (UINT_32)u4Size, __FILE__, __FUNCTION__);  \
        }   \
        kfree(pvAddr);  \
    }
#else
#define kalMemFree(pvAddr, u4Size)                  kfree(pvAddr)
#endif

#define kalUdelay(u4USec)                           udelay(u4USec)

#define kalMdelay(u4MSec)                           mdelay(u4MSec)

/* Copy memory from user space to kernel space */
#define kalMemCopyFromUser(_pvTo, _pvFrom, _u4N)    copy_from_user(_pvTo, _pvFrom, _u4N)

/* Copy memory from kernel space to user space */
#define kalMemCopyToUser(_pvTo, _pvFrom, _u4N)      copy_to_user(_pvTo, _pvFrom, _u4N)

/* Copy memory block with specific size */
#define kalMemCopy(pvDst, pvSrc, u4Size)            memcpy(pvDst, pvSrc, u4Size)

/* Set memory block with specific pattern */
#define kalMemSet(pvAddr, ucPattern, u4Size)        memset(pvAddr, ucPattern, u4Size)

#define kalMemCmp(pvAddr1, pvAddr2, u4Size)         memcmp(pvAddr1, pvAddr2, u4Size)

/* Zero specific memory block */
#define kalMemZero(pvAddr, u4Size)                  memset(pvAddr, 0, u4Size)

/* defined for wince sdio driver only */
#if defined(_HIF_SDIO)
#define kalDevSetPowerState(prGlueInfo, ePowerMode) glSetPowerState(prGlueInfo, ePowerMode)
#else
#define kalDevSetPowerState(prGlueInfo, ePowerMode)
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define kalSendComplete(prGlueInfo, pvPacket, status)   \
            kalSendCompleteAndAwakeQueue(prGlueInfo, pvPacket)


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define kalQueryBufferPointer(prGlueInfo, pvPacket)     \
            ((PUINT_8)((struct sk_buff *)pvPacket)->data)


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define kalQueryValidBufferLength(prGlueInfo, pvPacket)     \
            ((UINT_32)((struct sk_buff *)pvPacket)->end -  \
             (UINT_32)((struct sk_buff *)pvPacket)->data)

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define kalCopyFrame(prGlueInfo, pvPacket, pucDestBuffer)   \
            {struct sk_buff *skb = (struct sk_buff *)pvPacket; \
             memcpy(pucDestBuffer, skb->data, skb->len);}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define kalGetTimeTick()                            jiffies

#define kalPrint                                    printk

#define kalBreakPoint() \
    do { \
        BUG(); \
        panic("Oops"); \
    } while(0)

#define PRINTF_ARG(...)                             __VA_ARGS__
#define SPRINTF(buf, arg)                           {buf += sprintf((char *)(buf), PRINTF_ARG arg);}

/*----------------------------------------------------------------------------*/
/* Routines in gl_kal.c                                                       */
/*----------------------------------------------------------------------------*/
VOID
kalAcquireSpinLock(
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_SPIN_LOCK_CATEGORY_E    rLockCategory,
    OUT PUINT_32                    pu4Flags
    );

VOID
kalReleaseSpinLock(
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_SPIN_LOCK_CATEGORY_E    rLockCategory,
    IN UINT_32                      u4Flags
    );

VOID
kalUpdateMACAddress(
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PUINT_8          pucMacAddr
    );

VOID
kalPacketFree(
    IN  P_GLUE_INFO_T    prGlueInfo,
    IN PVOID             pvPacket
    );

PVOID
kalPacketAlloc(
    IN P_GLUE_INFO_T     prGlueInfo,
    IN UINT_32           u4Size,
    OUT PUINT_8          *ppucData
    );

VOID
kalOsTimerInitialize(
    IN P_GLUE_INFO_T     prGlueInfo,
    IN PVOID             prTimerHandler
    );

BOOL
kalSetTimer(
    IN P_GLUE_INFO_T      prGlueInfo,
    IN OS_SYSTIME         rInterval
    );

WLAN_STATUS
kalProcessRxPacket(
    IN P_GLUE_INFO_T      prGlueInfo,
    IN PVOID              pvPacket,
    IN PUINT_8            pucPacketStart,
    IN UINT_32            u4PacketLen,
    IN PBOOLEAN           pfgIsRetain,
    IN ENUM_CSUM_RESULT_T aeCSUM[]
    );

WLAN_STATUS
kalRxIndicatePkts(
    IN P_GLUE_INFO_T prGlueInfo,
    IN PVOID         apvPkts[],
    IN UINT_8        ucPktNum
    );

VOID
kalIndicateStatusAndComplete(
    IN P_GLUE_INFO_T prGlueInfo,
    IN WLAN_STATUS   eStatus,
    IN PVOID         pvBuf,
    IN UINT_32       u4BufLen
    );

VOID
kalUpdateReAssocReqInfo(
    IN P_GLUE_INFO_T prGlueInfo,
    IN PUINT_8       pucFrameBody,
    IN UINT_32       u4FrameBodyLen,
    IN BOOLEAN       fgReassocRequest
    );

#if CFG_TX_FRAGMENT
BOOLEAN
kalQueryTxPacketHeader(
    IN P_GLUE_INFO_T prGlueInfo,
    IN PVOID         pvPacket,
    OUT PUINT_16     pu2EtherTypeLen,
    OUT PUINT_8      pucEthDestAddr
    );
#endif /* CFG_TX_FRAGMENT */

VOID
kalSendCompleteAndAwakeQueue(
    IN P_GLUE_INFO_T  prGlueInfo,
    IN PVOID          pvPacket
    );

#if CFG_TCP_IP_CHKSUM_OFFLOAD
VOID
kalQueryTxChksumOffloadParam(
    IN  PVOID     pvPacket,
    OUT PUINT_8   pucFlag);

VOID
kalUpdateRxCSUMOffloadParam(
    IN PVOID               pvPacket,
    IN ENUM_CSUM_RESULT_T  eCSUM[]
    );
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */


/*----------------------------------------------------------------------------*/
/* Routines in interface - ehpi/sdio.c                                                       */
/*----------------------------------------------------------------------------*/
BOOL
kalDevRegRead(
    IN  P_GLUE_INFO_T  prGlueInfo,
    IN  UINT_32        u4Register,
    OUT PUINT_32       pu4Value
    );

BOOL
kalDevRegWrite(
    P_GLUE_INFO_T  prGlueInfo,
    IN UINT_32     u4Register,
    IN UINT_32     u4Value
    );

BOOL
kalDevPortRead(
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_16         u2Port,
    IN  UINT_16         u2Len,
    OUT PUINT_8         pucBuf,
    IN  UINT_16         u2ValidOutBufSize
    );

BOOL
kalDevPortWrite(
    P_GLUE_INFO_T  prGlueInfo,
    IN UINT_16     u2Port,
    IN UINT_16     u2Len,
    IN PUINT_8     pucBuf,
    IN UINT_16     u2ValidInBufSize
    );

void 
kalDevEnableIrq();

void 
kalDevDisableIrq();

    #if CFG_SUPPORT_EXT_CONFIG
UINT_32
kalReadExtCfg (
    IN P_GLUE_INFO_T prGlueInfo
    );
    #endif


#endif /* _GL_KAL_H */

