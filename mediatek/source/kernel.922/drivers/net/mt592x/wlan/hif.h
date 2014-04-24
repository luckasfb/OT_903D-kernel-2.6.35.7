





#ifndef _HIF_H
#define _HIF_H



#if defined(_PF_COLIBRI) && _PF_COLIBRI
#include "colibri.h"
#endif

#if defined(_PF_S3C2440) && _PF_S3C2440
#include "s3c2440a.h"
#endif

#if defined(_PF_S3C6410) && _PF_S3C6410
#include "s3c6410.h"
#endif

#if defined(_PF_S3C2416) && _PF_S3C2416
#include "s3c2416.h"
#endif

#if defined(_PF_MT6516) && _PF_MT6516
#include "mt6516.h"
#endif


#define REQ_FLAG_HALT               (0x01)
#define REQ_FLAG_INT                (0x02)
#define REQ_FLAG_OID                (0x04)
#define REQ_FLAG_TIMER              (0x08)
#define REQ_FLAG_RESET              (0x10)



typedef struct _GL_IO_REQ_T {
    QUE_ENTRY_T             rQueEntry;
    wait_queue_head_t   cmdwait_q;
    BOOL                fgRead;
    BOOL                fgWaitResp;
    P_ADAPTER_T          prAdapter;
    PFN_OID_HANDLER_FUNC pfnOidHandler;
    PVOID                pvInfoBuf;
    UINT_32              u4InfoBufLen;
    PUINT_32            pu4QryInfoLen;
    WLAN_STATUS         rStatus;
    UINT_32              u4Flag;
} GL_IO_REQ_T, *P_GL_IO_REQ_T;


typedef struct _GL_HIF_INFO_T {
    struct sdio_func	*func;

    /* mtk_sdiod thread information */
    struct task_struct  *main_thread;
    wait_queue_head_t   waitq;
    UINT_32             u4ReqFlag; /* REQ_FLAG_XXX */
    BOOL                fgStopThread;

    /* Packet Queue */
    QUE_T                   rSendQueue;

    /* OID related */
    PUINT_8                 pucIOReqBuff;
    QUE_T                   rIOReqQueue;
    QUE_T                   rFreeIOReqQueue;

    //spinlock_t		lock;
} GL_HIF_INFO_T, *P_GL_HIF_INFO_T;




WLAN_STATUS
sdio_io_ctrl(
    IN P_GLUE_INFO_T        prGlueInfo,
    IN PFN_OID_HANDLER_FUNC pfnOidQryHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    IN BOOL                 fgRead,
    IN BOOL                 fgWaitResp,
    OUT PUINT_32            pu4QryInfoLen
    );

//static int sdio_thread(void *data);

WLAN_STATUS
glRegisterBus(
    probe_card pfProbe,
    remove_card pfRemove
    );

VOID
glUnregisterBus(
    remove_card pfRemove
    );

int
hifRegisterPlatformDriver (
	suspend_callback	wlanSuspend,
	resume_callback		wlanResume
    );


int hifUnregisterPlatformDriver (
    );


VOID
glSetHifInfo (
    P_GLUE_INFO_T prGlueInfo,
    UINT_32 u4Cookie
    );

VOID
glClearHifInfo (
    P_GLUE_INFO_T prGlueInfo
    );

BOOL
glBusInit (
    PVOID pvData
    );

VOID
glBusRelease (
    PVOID pData
    );

INT_32
glBusSetIrq (
    PVOID pvData,
    PVOID pfnIsr,
    PVOID pvCookie
    );

VOID
glBusFreeIrq (
    PVOID pvData,
    PVOID pvCookie
    );

VOID
glSetPowerState (
    IN P_GLUE_INFO_T  prGlueInfo,
    IN UINT_32 ePowerMode
    );

#endif /* _HIF_H */



