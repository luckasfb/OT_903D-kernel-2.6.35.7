









#include "gl_os.h"
#include "gl_typedef.h"
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>
//#include <linux/mmc/protocol.h>

#include <linux/mmc/sdio_func.h> /* sdio_readl(), etc */
#include <linux/mmc/sdio_ids.h> /* SDIO_CLASS_WLAN */

#include <asm/memory.h>
#include <linux/mm.h>

#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/sched.h>
static const int MAX_IOREQ_NUM = 60;


PVOID pPreAllocedBSSCached = NULL;

static struct MT5921_ops mt5921_ops;
static struct SDIOBUS_ops sdiobus_ops;
static UINT_32 gu4Dummy = 0;

#define PRE_ALLOC_MEM_SIZE (32*1024)
extern void
sdioHardStartXmit (
    struct sk_buff *prSkb,
    struct net_device *prDev
    );

extern int
hifRegisterBus (
    struct MT5921_ops * mtops,
    struct SDIOBUS_ops * busops
    );


extern VOID
hifUnregisterBus(
    remove_card pfRemove
    );


extern BOOL
hifBusInit (
    PVOID pvData
    );
    
extern VOID
hifBusRelease (
    PVOID pvData
    );
    
extern int
hifBusSetIrq (
    void * pvData,
    void * pfnIsr
    );

extern VOID
hifBusFreeIrq (
    void * pvData
    );    

#if 0
extern BOOL
hifDevRegRead (
    IN  void *          pHif,
    IN  unsigned long         u4Register,
    OUT unsigned long *       pu4Value
    );

extern BOOL
hifDevRegWrite (
    IN  void *          pHif,
    IN unsigned long        u4Register,
    IN unsigned long        u4Value
    );
    
                                
extern BOOL
hifDevPortRead (
    IN  void *          pHif,
    IN  unsigned short         u2Port,
    IN  unsigned short         u2Len,
    OUT unsigned char *        pucBuf,
    IN  unsigned short         u2ValidOutBufSize
    );

extern BOOL
hifDevPortWrite (
    IN void *         pHif,
    IN unsigned short        u2Port,
    IN unsigned short        u2Len,
    IN unsigned char *       pucBuf,
    IN unsigned short        u2ValidInBufSize
    );
#endif
    
extern void sdio_irq_func(void * pHif);

extern void hifSetInfo(void * pHif, void *prGlueInfo);
//static void mt5921_irq_func(struct sdio_func *func)
static int mt5921_irq_func(struct sdio_func *func)
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    prGlueInfo = sdio_get_drvdata(func);
    ASSERT(prGlueInfo);
    ASSERT(prGlueInfo->prAdapter);
	
    //printk(KERN_INFO DRV_NAME"++ mt5921_irq_func ++ \n");
	
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_INT); // GeorgeKuo_modify:
    prGlueInfo->rHifInfo.u4ReqFlag |= REQ_FLAG_INT;
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_INT); // GeorgeKuo_modify:

    wake_up_interruptible(&prGlueInfo->rHifInfo.waitq);
    //printk(KERN_INFO DRV_NAME"-- mt5921_irq_func -- \n");
    return 0;
}


WLAN_STATUS
sdio_io_ctrl (IN P_GLUE_INFO_T        prGlueInfo,
    IN PFN_OID_HANDLER_FUNC pfnOidHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    IN BOOL                 fgRead,
    IN BOOL                 fgWaitResp,
    OUT PUINT_32            pu4QryInfoLen
    )
{
    P_GL_IO_REQ_T prIoReq = NULL;
    P_GL_HIF_INFO_T prHifInfo = NULL;
    P_QUE_T prIOReqQueue = NULL;
    P_QUE_T prFreeIOReqQueue = NULL;
    WLAN_STATUS ret = WLAN_STATUS_SUCCESS;

    /* Sanity check */
    ASSERT(prGlueInfo);
    ASSERT(pfnOidHandler);
    if (!prGlueInfo || !pfnOidHandler) {
        printk(KERN_WARNING DRV_NAME"sdio_io_ctrl invalid parameters: 0x%p, 0x%p\n",
            prGlueInfo, pfnOidHandler);
        return WLAN_STATUS_INVALID_DATA;
    }

    if (FALSE != fgRead) { /* Sanity check for read */
        ASSERT(pvInfoBuf);
        ASSERT(u4InfoBufLen);
        ASSERT(pu4QryInfoLen);
        ASSERT(FALSE != fgWaitResp); /* Caller must wait for read opeartion */
        if (!pvInfoBuf || !u4InfoBufLen || (FALSE == fgWaitResp) || !pu4QryInfoLen) {
            printk(KERN_WARNING DRV_NAME"sdio_io_ctrl read with invalid parameters: 0x%p, 0x%lx, 0x%x, 0x%p\n",
                pvInfoBuf, u4InfoBufLen, fgWaitResp, pu4QryInfoLen);
            return WLAN_STATUS_INVALID_DATA;
        }
    }

    if (FALSE == fgWaitResp) { /* Sanity check for NO-wait */
        /* If Caller does NOT wait for results, input pu4QryInfoLen must be NULL
         * or it WILL cause problem when being updated by thread sdiod.
         */
        ASSERT(NULL == pu4QryInfoLen);
        if (NULL != pu4QryInfoLen) {
            printk(KERN_WARNING DRV_NAME"pu4QryInfoLen shall be NULL if fgWaitResp if FALSE \n");
            return WLAN_STATUS_INVALID_DATA;
        }
    }

    prHifInfo = &prGlueInfo->rHifInfo;
    prIOReqQueue = &prHifInfo->rIOReqQueue;
    prFreeIOReqQueue = &prHifInfo->rFreeIOReqQueue;

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
    QUEUE_REMOVE_HEAD(prFreeIOReqQueue, prIoReq, P_GL_IO_REQ_T);
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);

    ASSERT(prIoReq);
    if (NULL == prIoReq) {
        printk(KERN_WARNING DRV_NAME"prIoReq allocate fail from rFreeIOReqQueue(%ld %p %p)\n",
            prFreeIOReqQueue->u4NumElem,
            prFreeIOReqQueue->prHead,
            prFreeIOReqQueue->prTail);
        return WLAN_STATUS_RESOURCES;
    }

    /* TODO: if fgWaitResp is false, pvInfoBuf may be invalid after this
    ** function return.
    */
    prIoReq->prAdapter = prGlueInfo->prAdapter;
    prIoReq->pfnOidHandler = pfnOidHandler;
    prIoReq->pvInfoBuf = pvInfoBuf;
    prIoReq->u4InfoBufLen = u4InfoBufLen;
    prIoReq->pu4QryInfoLen = (FALSE != fgWaitResp) ? pu4QryInfoLen : &gu4Dummy;
    prIoReq->fgRead = fgRead;
    prIoReq->fgWaitResp= fgWaitResp;
    prIoReq->u4Flag = 0;

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
    QUEUE_INSERT_TAIL(prIOReqQueue, &prIoReq->rQueEntry);
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);

    wake_up_interruptible(&prHifInfo->waitq);

    if (fgWaitResp) {
        might_sleep();
        BUG_ON(in_interrupt()); //ASSERT(!in_interrupt());
        wait_event_interruptible(prIoReq->cmdwait_q, (prIoReq->u4Flag != 0));
        ret = prIoReq->rStatus;
        GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
        QUEUE_INSERT_TAIL(prFreeIOReqQueue, &prIoReq->rQueEntry);
        GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
    }

    return ret;
}


static int mtk_sdiod_thread(void *data)
{
    struct net_device *dev = data;
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)netdev_priv(dev);
    P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;

    P_QUE_ENTRY_T       prQueueEntry;
    P_GL_IO_REQ_T       prIoReq;
    P_QUE_T             prSendQueue;
    P_QUE_T             prIOReqQueue;
    struct sk_buff      *prSkb;

    int ret = 0;

    /* GeorgeKuo(20090731): set to no freeze. mtk_sdiod will be stopped before
    ** entering suspend state.
    */
    current->flags |= PF_NOFREEZE;

    printk(KERN_INFO DRV_NAME"mtk_sdiod starts running... \n");

    set_user_nice(current, -10);
  	
    for (;;) {
#if 1
        ret = wait_event_interruptible(prHifInfo->waitq,
            (kthread_should_stop() ||
                QUEUE_IS_NOT_EMPTY(&prGlueInfo->rHifInfo.rIOReqQueue) ||
                QUEUE_IS_NOT_EMPTY(&prGlueInfo->rHifInfo.rSendQueue) ||
                (prGlueInfo->rHifInfo.u4ReqFlag != 0)));
#else
        add_wait_queue(&prHifInfo->waitq, &wait);
        printk("after add_wait_queue\n");
        set_current_state(TASK_INTERRUPTIBLE);
        printk("after set_current_state\n");
        schedule();
        printk("after schedule\n");
        set_current_state(TASK_RUNNING);
        printk("after set_current_state\n");
        remove_wait_queue(&prHifInfo->waitq, &wait);
        printk("after remove_wait_queue\n");
#endif

#if 0 /* GeorgeKuo(20090731): freeze here will block kthread_stop() call. */
        try_to_freeze();
#endif

        if (kthread_should_stop()) {
            printk(KERN_INFO DRV_NAME"mtk_sdiod should stop now...\n");
            prHifInfo->fgStopThread = TRUE;
        }

        /* Handle I/O Request */
        while (TRUE) {
            prIOReqQueue = &prGlueInfo->rHifInfo.rIOReqQueue;
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
            QUEUE_REMOVE_HEAD(prIOReqQueue, prIoReq, P_GL_IO_REQ_T);
            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);

            if (NULL == prIoReq) {
                //printk(KERN_INFO DRV_NAME"No IO Req\n");
                break;
            }

            if (FALSE == prHifInfo->fgStopThread) {
                if (FALSE == prIoReq->fgRead) {
                    prIoReq->rStatus = wlanSetInformation(
                        prIoReq->prAdapter,
                        prIoReq->pfnOidHandler,
                        prIoReq->pvInfoBuf,
                        prIoReq->u4InfoBufLen,
                        prIoReq->pu4QryInfoLen);
                }
                else {
                    prIoReq->rStatus = wlanQueryInformation(
                        prIoReq->prAdapter,
                        prIoReq->pfnOidHandler,
                        prIoReq->pvInfoBuf,
                        prIoReq->u4InfoBufLen,
                        prIoReq->pu4QryInfoLen);
                }
            }
            else {
                /* Should stop now... return which status? */
                prIoReq->rStatus = WLAN_STATUS_ADAPTER_NOT_READY;
                printk(KERN_INFO DRV_NAME"clean pending IO_REQ\n");
            }

            if (prIoReq->fgWaitResp) {
                prIoReq->u4Flag = 1;
                wake_up_interruptible(&prIoReq->cmdwait_q);
            }
            else {
                GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
                QUEUE_INSERT_TAIL(&prHifInfo->rFreeIOReqQueue, &prIoReq->rQueEntry);
                GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_IO_REQ);
            }
        }

		    //printk(KERN_INFO DRV_NAME"%ld, %d\n", prGlueInfo->rHifInfo.u4ReqFlag, prHifInfo->fgStopThread);
        /* Handle Interrupt */
        GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_INT);
        if (prGlueInfo->rHifInfo.u4ReqFlag & REQ_FLAG_INT) {
            prGlueInfo->rHifInfo.u4ReqFlag &= ~REQ_FLAG_INT;
            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_INT);

            if (FALSE == prHifInfo->fgStopThread) {
                wlanISR(prGlueInfo->prAdapter, TRUE);
                wlanIST(prGlueInfo->prAdapter);
		#ifdef CFG_EINT_HANDLED_IN_WLAN
		sdiobus_ops.DevEnableEint();	
		#endif
            }
            else {
                /* Should stop now... skip pending interrupt */
                printk(KERN_INFO DRV_NAME"ignore pending interrupt\n");
            }
        }
        else {
            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_INT);
        }

        /* Handle Packet Tx */
        while (TRUE) {
            prSendQueue = &prGlueInfo->rHifInfo.rSendQueue;
            GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX);
            QUEUE_REMOVE_HEAD(prSendQueue, prQueueEntry, P_QUE_ENTRY_T);
            GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX);

            if (NULL == prQueueEntry) {
                //printk(KERN_INFO DRV_NAME"No Tx Packet\n");
                break;
            }

            prSkb = (struct sk_buff *)GLUE_GET_PKT_DESCRIPTOR(prQueueEntry);
            ASSERT(prSkb);
            if (NULL == prSkb) {
                printk(KERN_WARNING DRV_NAME"prSkb == NULL in tx\n");
            }
            else {
                if (FALSE == prHifInfo->fgStopThread) {
                    //printk(KERN_INFO DRV_NAME"Process Tx\n");
                    sdioHardStartXmit(prSkb, prGlueInfo->prDevHandler);
                }
                else {
                    /* Should stop now... skip pending tx packets */
                    printk(KERN_INFO DRV_NAME"clean pending tx skb\n");
                    dev_kfree_skb(prSkb);
                }
            }
        }

        if (FALSE != prHifInfo->fgStopThread) {
            break;
        }
    }

    printk(KERN_INFO DRV_NAME"mtk_sdiod stops\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
glRegisterBus (
    probe_card pfProbe,
    remove_card pfRemove
    )
{
	mt5921_ops.inthandler	= mt5921_irq_func;
	mt5921_ops.probe		= pfProbe;
	mt5921_ops.remove		= pfRemove;
	  /* Renbang 20100702 allocate memory when module insert, 
    * in case of that there is no availiable 32k physical memory when initialization
    */
    pPreAllocedBSSCached = kalMemAlloc(PRE_ALLOC_MEM_SIZE);
    if (pPreAllocedBSSCached == NULL) {
        printk(KERN_NOTICE "[wifi] no memory for pPreAllocedBSSCached\n");
    }else
        printk(KERN_NOTICE "[wifi] pPreAllocedBSSCached 0x%lx, size %d\n",pPreAllocedBSSCached, PRE_ALLOC_MEM_SIZE);
    
    return hifRegisterBus(&mt5921_ops, &sdiobus_ops);
} /* end of glRegisterBus() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glUnregisterBus(
    remove_card pfRemove
    )
{
    //printk(KERN_INFO DRV_NAME"++glUnregisterBus++\n");
    ASSERT(pfRemove);
    
    if(pPreAllocedBSSCached){
        kalMemFree(pPreAllocedBSSCached, PRE_ALLOC_MEM_SIZE);
        pPreAllocedBSSCached = NULL;
    }
    hifUnregisterBus(pfRemove);

    //printk(KERN_INFO DRV_NAME"--glUnregisterBus--\n");
    return;
} /* end of glUnregisterBus() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glSetHifInfo (
    P_GLUE_INFO_T prGlueInfo,
    UINT_32 u4Cookie
    )
{
    P_GL_HIF_INFO_T prHif = NULL;
    PUINT_8         pucMemHandle = NULL;
    P_GL_IO_REQ_T prIOReq = NULL;
    int i = 0;

    //printk(KERN_INFO DRV_NAME"++glSetHifInfo++\n");
    prHif = &prGlueInfo->rHifInfo;
    prHif->func = (struct sdio_func*)u4Cookie;

    init_waitqueue_head(&prHif->waitq);

    printk(KERN_INFO DRV_NAME"prHif->func->dev = 0x%p\n", &prHif->func->dev);
    printk(KERN_INFO DRV_NAME"prHif->func->vendor = 0x%04X\n", prHif->func->vendor);
    printk(KERN_INFO DRV_NAME"prHif->func->device = 0x%04X\n", prHif->func->device);

    hifSetInfo(prHif->func, prGlueInfo);
    //sdio_set_drvdata(prHif->func, prGlueInfo);

    /* GeorgeKuo: all drivers registering to a bus driver have to set this
     * linking.
     */
    SET_NETDEV_DEV(prGlueInfo->prDevHandler, &prHif->func->dev);

    QUEUE_INITIALIZE(&prHif->rSendQueue);
    QUEUE_INITIALIZE(&prHif->rIOReqQueue);
    QUEUE_INITIALIZE(&prHif->rFreeIOReqQueue);

    prHif->pucIOReqBuff = kalMemAlloc(sizeof(GL_IO_REQ_T)*MAX_IOREQ_NUM);

    pucMemHandle = prHif->pucIOReqBuff;
    for (i = 0; i < MAX_IOREQ_NUM; i++) {
        prIOReq = (P_GL_IO_REQ_T)pucMemHandle;
        init_waitqueue_head(&prIOReq->cmdwait_q);
        QUEUE_INSERT_TAIL(&prHif->rFreeIOReqQueue, &prIOReq->rQueEntry);
        pucMemHandle += ALIGN_4(sizeof(GL_IO_REQ_T));
    }

    prHif->fgStopThread = FALSE;

    /* Start mtk_sdiod thread */
    prHif->main_thread = kthread_run(mtk_sdiod_thread, prGlueInfo->prDevHandler, "mtk_sdiod");

    return;
} /* end of glSetHifInfo() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glClearHifInfo (
    P_GLUE_INFO_T prGlueInfo
    )
{
    P_GL_HIF_INFO_T prHif = NULL;

    ASSERT(prGlueInfo);
    prHif = &prGlueInfo->rHifInfo;

    printk(KERN_INFO DRV_NAME"mtk_sdiod (0x%p, 0x%p)\n", prHif, prHif->main_thread);
    kthread_stop(prHif->main_thread);
    printk(KERN_INFO DRV_NAME"mtk_sdiod stopped\n");

    kalMemFree(prHif->pucIOReqBuff, sizeof(GL_IO_REQ_T)*MAX_IOREQ_NUM);

    return;
} /* end of glClearHifInfo() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
glBusInit (
    PVOID pvData
    )
{
    struct sdio_func *func = NULL;

    printk(KERN_INFO DRV_NAME"++glBusInit ++ pvData = 0x%p\n", pvData);
    ASSERT(pvData);

    func = (struct sdio_func *)pvData;
    printk(KERN_INFO DRV_NAME"--glBusInit -- \n");
    return hifBusInit(func);
} /* end of glBusInit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glBusRelease (
    PVOID pvData
    )
{

    return;
} /* end of glBusRelease() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
INT_32
glBusSetIrq (
    PVOID pvData,
    PVOID pfnIsr,
    PVOID pvCookie
    )
{
    struct net_device *prNetDevice = NULL;
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_GL_HIF_INFO_T prHifInfo = NULL;
    int ret = 0;

    ASSERT(pvData);
    if (!pvData) {
        return -1;
    }
    prNetDevice = (struct net_device *)pvData;

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDevice);
    ASSERT(prGlueInfo);
    if (!prGlueInfo) {
        return -1;
    }

    prHifInfo = &prGlueInfo->rHifInfo;

    ret = hifBusSetIrq(prHifInfo->func, sdiobus_ops.DevIrq);
    printk(KERN_INFO DRV_NAME"%s hifBusSetIrq ret=%d \n", __FUNCTION__, ret);
    return ret;
} /* end of glBusSetIrq() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glBusFreeIrq (
    PVOID pvData,
    PVOID pvCookie
    )
{
    struct net_device *prNetDevice = NULL;
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_GL_HIF_INFO_T prHifInfo = NULL;

    ASSERT(pvData);
    if (!pvData) {
        printk(KERN_INFO DRV_NAME"%s null pvData\n", __FUNCTION__);
        return;
    }
    prNetDevice = (struct net_device *)pvData;

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDevice);
    ASSERT(prGlueInfo);
    if (!prGlueInfo) {
        printk(KERN_INFO DRV_NAME"%s no glue info\n", __FUNCTION__);
        return;
    }

    prHifInfo = &prGlueInfo->rHifInfo;

    hifBusFreeIrq(prHifInfo->func);

    return;
} /* end of glBusreeIrq() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
kalDevRegRead (
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_32         u4Register,
    OUT PUINT_32        pu4Value
    )
{

    //printk(KERN_INFO DRV_NAME"++kalDevRegRead++ buf:0x%p, reg:0x%x\n", pu4Value, u4Register);
    ASSERT(prGlueInfo);
    ASSERT(pu4Value);

    //return hifDevRegRead(prGlueInfo->rHifInfo.func, u4Register, pu4Value);
    return sdiobus_ops.DevRegRead(prGlueInfo->rHifInfo.func, u4Register, pu4Value);

} /* end of kalDevRegRead() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
kalDevRegWrite (
    IN P_GLUE_INFO_T  prGlueInfo,
    IN UINT_32        u4Register,
    IN UINT_32        u4Value
    )
{
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"--kalDevRegWrite-- reg:0x%x, val:0x%x\n", u4Register, u4Value);
    ASSERT(prGlueInfo);

    sdiobus_ops.DevRegWrite(prGlueInfo->rHifInfo.func, u4Register, u4Value);

    //printk(KERN_INFO DRV_NAME"kalDevRegWrite MCR(%#x) = %#x, %d\n", u4Register, u4Value, ret);

    //printk(KERN_INFO DRV_NAME"--kalDevRegWrite--\n");

    return (ret) ? FALSE : TRUE;
} /* end of kalDevRegWrite() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
kalDevPortRead (
    IN  P_GLUE_INFO_T   prGlueInfo,
    IN  UINT_16         u2Port,
    IN  UINT_16         u2Len,
    OUT PUINT_8         pucBuf,
    IN  UINT_16         u2ValidOutBufSize
    )
{
    P_GL_HIF_INFO_T prHifInfo = NULL;
    struct sdio_func *prSdioFunc = NULL;
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"++kalDevPortRead++ buf:0x%p, port:0x%x, length:%d\n", pucBuf, u2Port, u2Len);
    ASSERT(prGlueInfo);
    prHifInfo = &prGlueInfo->rHifInfo;
    prSdioFunc = prHifInfo->func;

    ret = sdiobus_ops.DevPortRead(prSdioFunc, u2Port, u2Len, pucBuf, u2ValidOutBufSize);

    if (ret == FALSE) {
        ERRORLOG(("sdio_readsb() reports error: %x", ret));
    }

    //printk(KERN_INFO DRV_NAME"-- kalDevPortRead-- ret=%d\n", ret);
    return ret;
} /* end of kalDevPortRead() */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
kalDevPortWrite (
    IN P_GLUE_INFO_T  prGlueInfo,
    IN UINT_16        u2Port,
    IN UINT_16        u2Len,
    IN PUINT_8        pucBuf,
    IN UINT_16        u2ValidInBufSize
    )
{
    P_GL_HIF_INFO_T prHifInfo = NULL;
    struct sdio_func *prSdioFunc = NULL;
    int ret = 0;

    //printk(KERN_INFO DRV_NAME"++kalDevPortWrite++ buf:0x%p, port:0x%x, length:%d\n", pucBuf, u2Port, u2Len);
    ASSERT(prGlueInfo);
    prHifInfo = &prGlueInfo->rHifInfo;
    prSdioFunc = prHifInfo->func;
	
	ASSERT(pucBuf);
	ASSERT(u2Len <= u2ValidInBufSize);
	ASSERT(prSdioFunc->cur_blksize > 0);

    ret = sdiobus_ops.DevPortWrite(prSdioFunc, u2Port, u2Len, pucBuf, u2ValidInBufSize);

    if (ret == FALSE) {
        ERRORLOG(("hifDevPortWrite() reports error: %x", ret));
    }

    //printk(KERN_INFO DRV_NAME"-- kalDevPortWrite-- ret=%d\n", ret);
    return ret;
} /* end of kalDevPortWrite() */


void kalDevEnableIrq()
{
	if(NULL != sdiobus_ops.DevEnableEint)
		sdiobus_ops.DevEnableEint();
	else
		ERRORLOG(("sdiobus_ops.DevEnableEint = NULL.\n"));
}

void kalDevDisableIrq()
{
	if(NULL != sdiobus_ops.DevDisableEint)
		sdiobus_ops.DevDisableEint();
	else
		ERRORLOG(("sdiobus_ops.DevDisableEint = NULL\n"));
}


VOID
glSetPowerState (
    IN P_GLUE_INFO_T  prGlueInfo,
    IN UINT_32 ePowerMode
    )
{
#if 0
    struct sdio_func *prSdioFunc = NULL;
    struct mmc_host *prMmcHost = NULL;
    struct mmc_command cmd;

    ASSERT(prGlueInfo);

    prSdioFunc = prGlueInfo->rHifInfo.func;
    ASSERT(prSdioFunc);

    prMmcHost = prSdioFunc->card->host;
    ASSERT(prMmcHost);
    if (ParamDeviceStateD0 == ePowerMode) {
        /* Test CMD5 */
        memset(&cmd, 0, sizeof(struct mmc_command));

        cmd.opcode = SD_IO_SEND_OP_COND;
        cmd.arg = 0;
        cmd.flags = MMC_RSP_SPI_R4 | MMC_RSP_R4 | MMC_CMD_BCR;

        mmc_wait_for_cmd(prMmcHost, &cmd, 3);
    }
#endif
    return;
}
