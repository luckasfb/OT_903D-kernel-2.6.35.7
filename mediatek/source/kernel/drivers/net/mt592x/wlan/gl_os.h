





#ifndef _GL_OS_H
#define _GL_OS_H

#define CFG_MAX_WLAN_DEVICES                1 /* number of wlan card will coexist*/

#define CFG_USE_SPIN_LOCK_BOTTOM_HALF       1 /* 1: Enable use of SPIN LOCK Bottom Half for LINUX
                                                 0: Disable - use SPIN LOCK IRQ SAVE instead */

#define CFG_TX_PADDING_SMALL_ETH_PACKET     0 /* 1: Enable - Drop ethernet packet if it < 14 bytes.
                                                             And pad ethernet packet with dummy 0 if it < 60 bytes.
                                                 0: Disable */

#define CFG_TX_STOP_NETIF_QUEUE_THRESHOLD   2048 /* packets */


#define ETH_P_1X                            0x888E
#define IPTOS_PREC_OFFSET                   5
#define USER_PRIORITY_DEFAULT               0

//#if SUPPORT_WAPI
#define ETH_WPI_1X                         0x88B4
//#endif

#define DRV_NAME    "[MT5921]: "

#include <linux/version.h>      /* constant of kernel version */

#include <linux/kernel.h>       /* bitops.h */

#include <linux/timer.h>        /* struct timer_list */
#include <linux/jiffies.h>      /* jiffies */
#include <linux/delay.h>        /* udelay and mdelay macro */

#include <linux/irq.h>          /* IRQT_FALLING */

#include <linux/netdevice.h>    /* struct net_device, struct net_device_stats */
#include <linux/etherdevice.h>  /* for eth_type_trans() function */
#include <linux/wireless.h>     /* struct iw_statistics */
#include <linux/if_arp.h>

#include <linux/ip.h>           /* struct iphdr */

#include <linux/string.h>       /* for memcpy()/memset() function */
#include <linux/stddef.h>       /* for offsetof() macro */

#include <linux/proc_fs.h>      /* The proc filesystem constants/structures */

#include <linux/rtnetlink.h>    /* for rtnl_lock() and rtnl_unlock() */
#include <linux/kthread.h>      /* kthread_should_stop(), kthread_run() */
#include <linux/vmalloc.h>

#include <linux/inetdevice.h>

#include <asm/uaccess.h>        /* for copy_from_user() */

#include <asm/io.h>             /* readw and writew */

#include <asm/div64.h>           /* for division */

extern void panic(const char * fmt, ...);                       /* See kernel/panic.c */
extern int printk(const char * fmt, ...);                       /* See kernel/printk.c */
extern int sprintf(char *buf, const char *fmt, ...);            /* See lib/vsprintf.c */
extern int sscanf(const char * buf, const char * fmt, ...);     /* See lib/vsprintf.c */

#if WIRELESS_EXT > 12
#include <net/iw_handler.h>
#endif

#include "version.h"

#include "config.h"

#include "gl_typedef.h"
#include "typedef.h"
#include "queue.h"
#include "gl_kal.h"
#include "hif.h"


#include "debug.h"

#include "wlan_lib.h"
#include "wlan_oid.h"
#include "wlan_oid_rftest.h"

#include "../platform.h"

extern BOOLEAN fgIsBusAccessFailed;


typedef struct _GL_WPA_INFO_T {
    UINT_32 u4WpaVersion;
    UINT_32 u4KeyMgmt;
    UINT_32 u4CipherGroup;
    UINT_32 u4CipherPairwise;
    UINT_32 u4AuthAlg;
    BOOLEAN fgPrivacyInvoke;
} GL_WPA_INFO_T, *P_GL_WPA_INFO_T;


struct _GLUE_INFO_T {
    /* Device handle */
    struct net_device *prDevHandler;

    /* Device Index(index of arWlanDevInfo[]) */
    INT_32 i4DevIdx;

    /* Device statistics */
    struct net_device_stats rNetDevStats;

    /* Tasklet struct used for deferred task */
    struct tasklet_struct tasklet;

    /* The master timer event (only one) */
    struct timer_list rMasterTimer;


    /* Wireless statistics struct net_device */
    struct iw_statistics rIwStats;

    /* spinlock to sync power save mechanism */
    spinlock_t rSpinLock[SPIN_LOCK_NUM];

    /* Number of pending frames, also used for debuging if any frame is
     * missing during the process of unloading Driver.
     *
     * NOTE(Kevin): In Linux, we also use this variable as the threshold
     * for manipulating the netif_stop(wake)_queue() func.
     */
    UINT_32         u4TxPendingFrameNum;

    /* Host interface related information */
    /* defined in related hif header file */
    GL_HIF_INFO_T   rHifInfo;

    /*! \brief wext wpa related information */
    GL_WPA_INFO_T   rWpaInfo;

    /* Pointer to ADAPTER_T - main data structure of internal protocol stack */
    P_ADAPTER_T prAdapter;

#ifdef WLAN_INCLUDE_PROC
    struct proc_dir_entry *pProcRoot;
#endif /* WLAN_INCLUDE_PROC */

    /* Indicated media state */
    ENUM_PARAM_MEDIA_STATE_T eParamMediaStateIndicated;

    /* Device power state D0~D3 */
    PARAM_DEVICE_POWER_STATE ePowerState;

#if CFG_SUPPORT_EXT_CONFIG
    UINT_16                 au2ExtCfg[256];  /* NVRAM data buffer */
    UINT_32                 u4ExtCfgLength;  /* 0 means data is NOT valid */
    ENUM_CFG_SRC_TYPE_T     eCfgSrcType;
#endif

#if SUPPORT_WAPI
    /* Should be large than the PARAM_WAPI_ASSOC_INFO_T */
    UINT_8                  aucWapiAssocInfoIEs[42];
    UINT_16                 u2WapiAssocInfoIESz;
#endif
    BOOL    fgRadioOn; /* driver radio on/off state */
   
    /* LED Support Usage */
    UINT_16            u2LedBlinkMode;
    UINT_32            u4LedBlinkOnTime;
    UINT_32            u4LedBlinkOffTime;
    UINT_32            u4LedSetting;

};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
    /* linux 2.4 */
    typedef void (*PFN_WLANISR)(int irq, void *dev_id, struct pt_regs *regs);
#else
    typedef irqreturn_t (*PFN_WLANISR)(int irq, void *dev_id, struct pt_regs *regs);
#endif

typedef void (*PFN_LINUX_TIMER_FUNC)(unsigned long);



/*----------------------------------------------------------------------------*/
/* Macros of SPIN LOCK operations for using in Glue Layer                     */
/*----------------------------------------------------------------------------*/
#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
    #define GLUE_SPIN_LOCK_DECLARATION()
    #define GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
            { \
                if (rLockCategory < SPIN_LOCK_NUM) \
                spin_lock_bh(&(prGlueInfo->rSpinLock[rLockCategory])); \
            }
    #define GLUE_RELEASE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
            { \
                if (rLockCategory < SPIN_LOCK_NUM) \
                spin_unlock_bh(&(prGlueInfo->rSpinLock[rLockCategory])); \
            }
#else /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
    #define GLUE_SPIN_LOCK_DECLARATION()                        UINT_32 __u4Flags = 0
    #define GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
            { \
                if (rLockCategory < SPIN_LOCK_NUM) \
                spin_lock_irqsave(&(prGlueInfo)->rSpinLock[rLockCategory], __u4Flags); \
            }
    #define GLUE_RELEASE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
            { \
                if (rLockCategory < SPIN_LOCK_NUM) \
                spin_unlock_irqrestore(&(prGlueInfo->rSpinLock[rLockCategory]), __u4Flags); \
            }
#endif /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */


/*----------------------------------------------------------------------------*/
/* Macros for accessing Reserved Fields of native packet                      */
/*----------------------------------------------------------------------------*/
#define GLUE_GET_PKT_QUEUE_ENTRY(_p)    \
            (&( ((struct sk_buff *)(_p))->cb[0] ))

#define GLUE_GET_PKT_DESCRIPTOR(_prQueueEntry)  \
            ((P_NATIVE_PACKET) ((UINT_32)_prQueueEntry - offsetof(struct sk_buff, cb[0])) )


#define GLUE_SET_PKT_TID_IS1X(_p, _tid, _is1x)  \
            (*((PUINT_8) &( ((struct sk_buff *)(_p))->cb[4] )) = (((UINT_8)((_tid) & (BITS(0,6)))) | ((UINT_8)((_is1x) << 7))))

#define GLUE_GET_PKT_TID(_p)        \
            ((*((PUINT_8) &( ((struct sk_buff *)(_p))->cb[4] )) ) & (BITS(0,6)))

#define GLUE_GET_PKT_IS1X(_p)        \
            (((*((PUINT_8) &( ((struct sk_buff *)(_p))->cb[4] ))) & (BIT(7))) >> 7 )

#define GLUE_SET_PKT_HEADER_LEN(_p, _ucMacHeaderLen)    \
            (*((PUINT_8) &( ((struct sk_buff *)(_p))->cb[5] )) = (UINT_8)(_ucMacHeaderLen))

#define GLUE_GET_PKT_HEADER_LEN(_p) \
            (*((PUINT_8) &( ((struct sk_buff *)(_p))->cb[5] )) )


#define GLUE_SET_PKT_PAYLOAD_LEN(_p, _u2PayloadLen) \
            (*((PUINT_16) &( ((struct sk_buff *)(_p))->cb[6] )) = (UINT_16)(_u2PayloadLen))

#define GLUE_GET_PKT_PAYLOAD_LEN(_p)    \
            (*((PUINT_16) &( ((struct sk_buff *)(_p))->cb[6] )) )


#define GLUE_SET_PKT_ARRIVAL_TIME(_p, _rSysTime) \
            (*((POS_SYSTIME) &( ((struct sk_buff *)(_p))->cb[8] )) = (OS_SYSTIME)(_rSysTime))

#define GLUE_GET_PKT_ARRIVAL_TIME(_p)    \
            (*((POS_SYSTIME) &( ((struct sk_buff *)(_p))->cb[8] )) )

/* Check validity of prDev, private data, and pointers */
#define GLUE_CHK_DEV(prDev) \
    ((prDev && netdev_priv(prDev)) ? TRUE : FALSE)

#define GLUE_CHK_PR2(prDev, pr2) \
    ((GLUE_CHK_DEV(prDev) && pr2) ? TRUE : FALSE)

#define GLUE_CHK_PR3(prDev, pr2, pr3) \
    ((GLUE_CHK_PR2(prDev, pr2) && pr3) ? TRUE : FALSE)

#define GLUE_CHK_PR4(prDev, pr2, pr3, pr4) \
    ((GLUE_CHK_PR3(prDev, pr2, pr3) && pr4) ? TRUE : FALSE)


#ifdef WLAN_INCLUDE_PROC
INT_32
procRemoveProcfs (
    struct net_device *prDev,
    char *pucDevName
    );

INT_32
procInitProcfs (
    struct net_device *prDev,
    char *pucDevName
    );
#endif /* WLAN_INCLUDE_PROC */

#if defined(_HIF_SDIO)
extern WLAN_STATUS
sdio_io_ctrl (IN P_GLUE_INFO_T        prGlueInfo,
    IN PFN_OID_HANDLER_FUNC pfnOidHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    IN BOOL                 fgRead,
    IN BOOL                 fgWaitResp,
    OUT PUINT_32            pu4QryInfoLen
    );
#endif
extern WIFI_CFG_DATA gPlatformCfg;


#if !BUILD_USE_EEPROM
extern bool
cfgDataRead16( void * prAdapter,unsigned char ucWordOffset, unsigned short * pu2Data );
extern bool
cfgDataWrite16( void * prAdapter,unsigned char ucWordOffset,unsigned short u2Data );

#if 0
extern bool
platformNvramRead16( void * prAdapter,unsigned char ucWordOffset, unsigned short * pu2Data );

extern bool
platformNvramWrite16( void * prAdapter,unsigned char ucWordOffset,unsigned short u2Data );
#endif
#endif

extern bool
customDataRead8( void * prAdapter,unsigned char ucByteOffset, unsigned char * pucData );

extern bool
customDataWrite8( void * prAdapter,unsigned char ucByteOffset,unsigned char ucData );

extern int platform_init(void);
extern void platform_deinit(void);




#endif /* _GL_OS_H */

