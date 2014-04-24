






#include "gl_os.h"

#include "debug.h"

#include "wlan_lib.h"

#include "gl_wext.h"
#include "gl_wext_priv.h"

#if 0
extern struct spi_dev *spi_dev_get_by_minor(unsigned index);
extern int spi_master_send(struct spi_dev *,char* ,int);
extern int spi_master_recv(struct spi_dev *,char* ,int);
#endif

extern WIFI_CFG_DATA gPlatformCfg;


typedef struct _WLANDEV_INFO_T {
    struct net_device *prDev;

#if defined(_HIF_SDIO)
#else    
    struct tasklet_struct rTasklet;
#endif
    
} WLANDEV_INFO_T, *P_WLANDEV_INFO_T;

//for  black duck scan of GPL 2.0
#if  0
struct spi_dev;
struct spi_algorithm;

struct spi_msg {
 	__u16 flags;
 	__u16 len;		/* msg length				*/
 	__u8 *wbuf;		/* pointer to msg data to write	*/
 	__u8 *rbuf;		/* pointer to msg data for read */
};

struct spi_dev {
	int 			minor;
        /* vivek, 2009-04-13 16:55 Notes:  separating dmabufr and dmabufw for gspi_io*/

	dma_addr_t		dmabuf;		/* handle for DMA transfer		*/
	dma_addr_t		dmabufr;		/* handle for DMA read transfer		*/
	dma_addr_t		dmabufw;		/* handle for DMA write transfer		*/
	unsigned int 		flags;		/* flags for the SPI operation 		*/
	struct semaphore 	bus_lock;	/* semaphore for bus access 		*/

	struct spi_algorithm 	*algo;		/* the algorithm to access the bus	*/
	void 			*algo_data;	/* the bus control struct		*/

	int 			timeout;
	int 			retries;
	struct device		dev;		/* the adapter device 			*/
};

struct spi_algorithm {
	char name[32];				/* textual description 	*/
	unsigned int id;

	/* If an adapter algorithm can't to SPI-level access, set master_xfer
	   to NULL. If an adapter algorithm can do SMBus access, set
	   smbus_xfer. If set to NULL, the SMBus protocol is simulated
	   using common SPI messages */
	int (*master_xfer)(struct spi_dev *spi_dev,struct spi_msg *msgs,
	                   int num);

	/* --- ioctl like call to set div. parameters. */
	int (*algo_control)(struct spi_dev *, unsigned int, unsigned long);

	/* To determine what the adapter supports */
	u32 (*functionality) (struct spi_dev *);
	int (*close)(struct spi_dev *spi_dev);
};
#endif


MODULE_AUTHOR(NIC_AUTHOR);
MODULE_DESCRIPTION(NIC_DESC);
MODULE_SUPPORTED_DEVICE(NIC_NAME);

#ifdef MODULE_LICENSE
MODULE_LICENSE("Proprietary");
#endif /* MODULE_LICENSE */

#define NIC_INF_NAME    "wlan%d" /* interface name */

#if DBG
UINT_8  aucDebugModule[DBG_MODULE_NUM];
UINT_32 u4DebugModule = 0;
#endif /* DBG */

//4 2007/06/26, mikewu, now we don't use this, we just fix the number of wlan device to 1
static WLANDEV_INFO_T arWlanDevInfo[1] = {{0}};
//static UINT_32 u4WlanDevNum = 0; /* How many NICs coexist now */

static UINT_32 u4PacketFilter = 0;
static UINT_8 aucMCAddrList[MAX_NUM_GROUP_ADDR][ETH_ALEN];



void initCustomData(void);

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void
wlanClearDevIdx (
    struct net_device *prDev
    )
{
    ASSERT(prDev);

    if (arWlanDevInfo[0].prDev == prDev) {
#if defined(_HIF_SDIO)
#else  
        tasklet_kill(&arWlanDevInfo[0].rTasklet);
#endif
        arWlanDevInfo[0].prDev = NULL;
    }

    return;
} /* end of wlanClearDevIdx() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wlanGetDevIdx (
    struct net_device *prDev
    )
{
    ASSERT(prDev);   
    
    if (arWlanDevInfo[0].prDev == (struct net_device *)NULL) {
        arWlanDevInfo[0].prDev = prDev;
        
        return 0;
    }

    return -1;
} /* end of wlanGetDevIdx() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
wlanQoSFrameClassifier (
    IN P_GLUE_INFO_T prGlueInfo,
    IN struct sk_buff *skb,
    OUT PUINT_8 pucPriorityParam,
    OUT PBOOLEAN pfgIs1x
    )
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
    const struct iphdr *ip = ip_hdr(skb);
#else
    const struct iphdr *ip = skb->nh.iph;
#endif
    UINT_8 ucUserPriority = USER_PRIORITY_DEFAULT; /* Default */


    ASSERT(prGlueInfo);
    ASSERT(skb);

    if (skb->protocol == __constant_htons(ETH_P_IP)) {
        if (ip->version == IPVERSION) {
            ucUserPriority = IPTOS_PREC(ip->tos) >> IPTOS_PREC_OFFSET;
        }
    }
#if 1 /* SW Port Control */
    else if (skb->protocol == __constant_htons(ETH_P_1X)) {
        *pfgIs1x = TRUE;
    }
#if SUPPORT_WAPI
    else if (skb->protocol == __constant_htons(ETH_WPI_1X)) {
        *pfgIs1x = TRUE;
    }
#endif
#endif

    /* TODO(Kevin): Add TSPEC classifier here */

    *pucPriorityParam = ucUserPriority;

    return;
} /* end of wlanQoSFrameClassifier() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int
wlanDoIOCTL(
    struct net_device *prDev,
    struct ifreq *prIFReq,
    int i4Cmd
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    int ret = 0;

#if !defined(_HIF_SDIO)
    GLUE_SPIN_LOCK_DECLARATION();
#endif

    /* Verify input parameters for the following functions */
    ASSERT(prDev && prIFReq);
    if (!prDev || !prIFReq) {
        printk(KERN_WARNING DRV_NAME"%s Invalid input data\n", __FUNCTION__);
        return -EINVAL;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    ASSERT(prGlueInfo);
    if (!prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"%s No glue info\n", __FUNCTION__);
        return -EFAULT;
    }

#if !defined(_HIF_SDIO)
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    if (i4Cmd == SIOCGIWPRIV) {
        /* 0x8B0D, get private ioctl table */
        ret = wext_get_priv(prDev, prIFReq);
    }
    else if ((i4Cmd >= SIOCIWFIRST) && (i4Cmd < SIOCIWFIRSTPRIV)) {
        /* 0x8B00 ~ 0x8BDF, wireless extension region */
        ret = wext_support_ioctl(prDev, prIFReq, i4Cmd);
    }
    else if ((i4Cmd >= SIOCIWFIRSTPRIV) && (i4Cmd < SIOCIWLASTPRIV)) {
        /* 0x8BE0 ~ 0x8BFF, private ioctl region */
        ret = priv_support_ioctl(prDev, prIFReq, i4Cmd);
    }
    else {
        printk(KERN_NOTICE DRV_NAME"Unexpected ioctl command: 0x%04x\n", i4Cmd);
        /* return 0 for safe? */
    }

#if !defined(_HIF_SDIO)
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    return ret;
} /* end of wlanDoIOCTL() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void
wlanSetMulticastList (
    struct net_device *prDev
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    //UINT_32 u4PacketFilter = 0;
#if !defined(_HIF_SDIO)
    UINT_32 u4SetInfoLen;
#endif

#if !defined(_HIF_SDIO)
    GLUE_SPIN_LOCK_DECLARATION();
#endif

    prGlueInfo = (NULL != prDev) ? (P_GLUE_INFO_T)netdev_priv(prDev) : NULL;
    ASSERT(prDev);
    ASSERT(prGlueInfo);
    if (!prDev || !prGlueInfo) {
        printk(KERN_WARNING DRV_NAME"abnormal dev or skb: prDev(0x%p), prGlueInfo(0x%p)\n",
            prDev, prGlueInfo);
        return;
    }

#if !defined(_HIF_SDIO)
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    if (prDev->flags & IFF_PROMISC) {
        u4PacketFilter |= PARAM_PACKET_FILTER_PROMISCUOUS;
    }

    if (prDev->flags & IFF_BROADCAST) {
        u4PacketFilter |= PARAM_PACKET_FILTER_BROADCAST;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    if (prDev->flags & IFF_MULTICAST) {
        if ((prDev->flags & IFF_ALLMULTI) ||
            (prDev->mc_count > MAX_NUM_GROUP_ADDR)) 
#else
    if (prDev->flags & IFF_MULTICAST) {
        if ((prDev->flags & IFF_ALLMULTI) ||
            (netdev_mc_count(prDev) > MAX_NUM_GROUP_ADDR)) 
#endif
{

            u4PacketFilter |= PARAM_PACKET_FILTER_ALL_MULTICAST;
        }
        else {
            u4PacketFilter |= PARAM_PACKET_FILTER_MULTICAST;
        }
    }

#if defined(_HIF_SDIO)
    if (sdio_io_ctrl(prGlueInfo,
                wlanoidSetCurrentPacketFilter,
                &u4PacketFilter,
                sizeof(u4PacketFilter),
                FALSE,
                FALSE,
                NULL)) {
        printk(KERN_WARNING DRV_NAME"Fail to wlanoidSetCurrentPacketFilter: 0x%lx \n", u4PacketFilter);
    }
#else
    wlanSetInformation(prGlueInfo->prAdapter,
                       wlanoidSetCurrentPacketFilter,
                       &u4PacketFilter,
                       sizeof(u4PacketFilter),
                       &u4SetInfoLen);
#endif

    if (u4PacketFilter & PARAM_PACKET_FILTER_MULTICAST) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
        /* Prepare multicast address list */
        struct dev_mc_list *prMcList;
        //UINT_8 aucMCAddrList[MAX_NUM_GROUP_ADDR][ETH_ALEN];
        UINT_32 i = 0;

        for (i = 0, prMcList = prDev->mc_list;
             (prMcList) && (i <= prDev->mc_count) && (i <= MAX_NUM_GROUP_ADDR);
             i++, prMcList = prMcList->next) {

            memcpy(&aucMCAddrList[i][0], prMcList->dmi_addr, ETH_ALEN);
        }
#else
        /* Prepare multicast address list */
	struct netdev_hw_addr *ha;
	UINT_32 i = 0;

	netdev_for_each_mc_addr(ha, prDev){
		if(i <= MAX_NUM_GROUP_ADDR && i <= netdev_mc_count(prDev)){
			memcpy(&aucMCAddrList[i][0], ha->addr, ETH_ALEN);
			i++;
		}
	}
#endif
         //u4TotalMCAddrSize = i*ETH_ALEN;
#if defined(_HIF_SDIO)
         if (sdio_io_ctrl(prGlueInfo,
                            wlanoidSetMulticastList,
                            &aucMCAddrList[0][0],
                            (i * ETH_ALEN),
                            FALSE,
                            FALSE,
                            NULL)) {
             printk(KERN_WARNING DRV_NAME"Fail to wlanoidSetMulticastList i: %ld \n", i);
        }
#else
        wlanSetInformation(prGlueInfo->prAdapter,
                           wlanoidSetMulticastList,
                           &aucMCAddrList[0][0],
                           (i * ETH_ALEN),
                           &u4SetInfoLen);
#endif
    }

#if !defined(_HIF_SDIO)
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    return;
} /* end of wlanSetMulticastList() */


#if CFG_TX_DBG_PACKET_ORDER
#define MAXIMUM_NUM_OF_USER_PRIORITY    16

UINT_32 u4PacketIDIn[MAXIMUM_NUM_OF_USER_PRIORITY] = {0};
#endif /* CFG_TX_DBG_PACKET_ORDER */

#if defined(_HIF_SDIO)
void
sdioHardStartXmit(
    struct sk_buff *prSkb,
    struct net_device *prDev
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL; //private data of prDev
    P_ADAPTER_T prAdapter = NULL; //prGlueInfo->prAdapter;
    PACKET_INFO_T rPacketInfo;
    UINT_32 u4PacketLen; // = prSkb->len;

    UINT_8 ucPriorityParam;
    UINT_8 ucMacHeaderLen;
    UINT_16 u2PayloadLen;
    BOOLEAN fgIs1x = FALSE;
    WLAN_STATUS rStatus;

    struct sk_buff *prTxSkb = NULL;

#if !defined(_HIF_SDIO)
    GLUE_SPIN_LOCK_DECLARATION();
#endif

    ASSERT(prDev);
    ASSERT(prSkb);
    if (!prDev || !prSkb) {
        printk(KERN_WARNING DRV_NAME"abnormal dev or skb: prDev(0x%p), prSkb(0x%p)\n",
            prDev, prSkb);
        goto check_stop;
    }

#if defined (_HIF_SDIO)
    /* Move prSkb->data to 4-byte alignment address for some host DMA */
    prTxSkb = skb_copy_expand(prSkb, 4, 0, GFP_ATOMIC);
    ASSERT(prTxSkb);

    if (NULL == prTxSkb) {
        printk(KERN_INFO "MT5921: dev_alloc_skb(%d) fail!\n", CFG_RX_MAX_PKT_SIZE);
        dev_kfree_skb(prSkb);
        goto check_stop;
    }

    /* GeorgeKuo: some old kernel has bug in skb_copy_expand() */
    if (prTxSkb->ip_summed != prSkb->ip_summed) {
        prTxSkb->ip_summed = prSkb->ip_summed;
        prTxSkb->csum = prSkb->csum;
    }
    dev_kfree_skb(prSkb);

#else
    prTxSkb = prSkb;
#endif

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    ASSERT(prGlueInfo);
    prAdapter = (prGlueInfo) ? prGlueInfo->prAdapter : NULL;
    ASSERT(prAdapter);
    if (!prGlueInfo || !prAdapter) {
        printk(KERN_WARNING DRV_NAME"abnormal prGlueInfo or prAdapter: prGlueInfo(0x%p), prAdapter(0x%p)\n",
            prGlueInfo, prAdapter);
        dev_kfree_skb(prSkb);
        goto check_stop;
    }

    u4PacketLen = prTxSkb->len;

    /* Session of Frame Classifier */
    wlanQoSFrameClassifier(prGlueInfo, prTxSkb, &ucPriorityParam, &fgIs1x);


    /* Update time stamp for Net Device */
    prDev->trans_start = jiffies;


    /* Save the value of Priority Parameter */
    GLUE_SET_PKT_TID_IS1X(prTxSkb, ucPriorityParam, fgIs1x);

    ucMacHeaderLen = ETH_HLEN;

    /* Save the value of Header Length */
    GLUE_SET_PKT_HEADER_LEN(prTxSkb, ucMacHeaderLen);

    u2PayloadLen = (UINT_16)(u4PacketLen - ETH_HLEN);

    /* Save the value of Payload Length */
    GLUE_SET_PKT_PAYLOAD_LEN(prTxSkb, u2PayloadLen);

    /* Save the value of arrival time */
    GLUE_SET_PKT_ARRIVAL_TIME(prTxSkb, jiffies);


    PACKET_INFO_INIT(&rPacketInfo,
                     (BOOLEAN)FALSE,
                     fgIs1x,
                     (P_NATIVE_PACKET)prTxSkb,
                     ucPriorityParam,
                     ucMacHeaderLen,
                     u2PayloadLen,
                     (PUINT_8)prTxSkb->data
                     );

#if !defined(_HIF_SDIO)
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    prGlueInfo->u4TxPendingFrameNum++;

    if ((rStatus = wlanSendPacket(prAdapter, &rPacketInfo)) == WLAN_STATUS_PENDING) {

#if CFG_TX_DBG_PACKET_ORDER
        {
            ASSERT(ucPriorityParam <= 15);

            /* NOTE(Kevin): cb[0~11] have already been used. */
            *(PUINT_32)&prTxSkb->cb[12] = u4PacketIDIn[ucPriorityParam]++;

            //printk("In %ld[%d]\n", *(PUINT_32)&prSkb->cb[12], ucPriorityParam);
        }
#endif /* CFG_TX_DBG_PACKET_ORDER */
    }
    else if (rStatus == WLAN_STATUS_SUCCESS) {
        /* NOTE: When rStatus == WLAN_STATUS_SUCCESS, packet will be freed
         * by WLAN driver internal.
         */
    }
    else if (rStatus == WLAN_STATUS_FAILURE) {
        dev_kfree_skb(prTxSkb);
        prGlueInfo->u4TxPendingFrameNum--;
    }
    else {
        ASSERT(0); /* NOTE: Currently we didn't have other return status */
    }

#if !defined(_HIF_SDIO)
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

check_stop:
    if (prGlueInfo->u4TxPendingFrameNum >= CFG_TX_STOP_NETIF_QUEUE_THRESHOLD) {
        netif_stop_queue(prDev);
    }

}
#endif /* _HIF_SDIO */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int
wlanHardStartXmit(
    struct sk_buff *prSkb,
    struct net_device *prDev
    )
{
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
#ifndef _HIF_SDIO
    UINT_32 u4PacketLen = prSkb->len;
#endif
#ifdef _HIF_SDIO
    P_GL_HIF_INFO_T prHifInfo =  NULL;
    P_QUE_ENTRY_T prQueueEntry;
    P_QUE_T prSendQueue;
#else
    P_ADAPTER_T prAdapter = prGlueInfo->prAdapter;
    PACKET_INFO_T rPacketInfo;
#endif

#ifndef _HIF_SDIO
    /* Priority Parameter in 6.2.1.1.2 Semantics of the service primitive */
    UINT_8 ucPriorityParam;
    UINT_8 ucMacHeaderLen;
    UINT_16 u2PayloadLen;
    BOOLEAN fgIs1x = FALSE;
    WLAN_STATUS rStatus;
#endif

    GLUE_SPIN_LOCK_DECLARATION();


    ASSERT(prSkb);
    ASSERT(prDev);

/* Drop damaged packet and pad small packet with 0 */
#if CFG_TX_PADDING_SMALL_ETH_PACKET
    if (u4PacketLen < ETH_HLEN) { /* < 14 */
        dev_kfree_skb(prSkb);
        /* <TODO> Kevin: Update DROP Counter here. */
        return NETDEV_TX_OK;
    }

    if (u4PacketLen < ETH_ZLEN) { /* < 60 */

    #if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 28)

        if (skb_tailroom(prSkb) >= (ETH_ZLEN - u4PacketLen)) {
            memset(prSkb->data + u4PacketLen, 0, (ETH_ZLEN - u4PacketLen));
        }
        else {
            struct sk_buff *nskb;

            nskb = skb_copy_expand(prSkb,
                                   skb_headroom(prSkb),
                                   skb_tailroom(prSkb) + (ETH_ZLEN - u4PacketLen),
                                   GFP_ATOMIC);
            dev_kfree_skb(prSkb);
            if (nskb) {
                memset(nskb->data + nskb->len, 0, (ETH_ZLEN - u4PacketLen));
            }
            else {
                return NETDEV_TX_OK;
            }
        }

    #else

        if ((prSkb = skb_padto(prSkb, ETH_ZLEN)) == (struct sk_buff *)NULL) {
            return NETDEV_TX_OK;
        }

    #endif /* LINUX_VERSION_CODE */

        u4PacketLen = ETH_ZLEN;
    }
#endif /* CFG_TX_PADDING_SMALL_ETH_PACKET */

#ifdef _HIF_SDIO
    prHifInfo = &prGlueInfo->rHifInfo;
    prQueueEntry = (P_QUE_ENTRY_T)GLUE_GET_PKT_QUEUE_ENTRY(prSkb);
    prSendQueue = &prGlueInfo->rHifInfo.rSendQueue;
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX);
    QUEUE_INSERT_TAIL(prSendQueue, prQueueEntry);
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX);
    wake_up_interruptible(&prHifInfo->waitq);
#else

    /* Session of Frame Classifier */
    wlanQoSFrameClassifier(prGlueInfo, prSkb, &ucPriorityParam, &fgIs1x);


    /* Update time stamp for Net Device */
    prDev->trans_start = jiffies;


    /* Save the value of Priority Parameter */
    GLUE_SET_PKT_TID_IS1X(prSkb, ucPriorityParam, fgIs1x);

    ucMacHeaderLen = ETH_HLEN;

    /* Save the value of Header Length */
    GLUE_SET_PKT_HEADER_LEN(prSkb, ucMacHeaderLen);

    u2PayloadLen = (UINT_16)(u4PacketLen - ETH_HLEN);

    /* Save the value of Payload Length */
    GLUE_SET_PKT_PAYLOAD_LEN(prSkb, u2PayloadLen);

    /* Save the value of arrival time */
    GLUE_SET_PKT_ARRIVAL_TIME(prSkb, jiffies);


    PACKET_INFO_INIT(&rPacketInfo,
                     (BOOLEAN)FALSE,
                     fgIs1x,
                     (P_NATIVE_PACKET)prSkb,
                     ucPriorityParam,
                     ucMacHeaderLen,
                     u2PayloadLen,
                     (PUINT_8)prSkb->data
                     );

    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

    prGlueInfo->u4TxPendingFrameNum++;

    if ((rStatus = wlanSendPacket(prAdapter, &rPacketInfo)) == WLAN_STATUS_PENDING) {

#if CFG_TX_DBG_PACKET_ORDER
        {
            ASSERT(ucPriorityParam <= 15);

            /* NOTE(Kevin): cb[0~11] have already been used. */
            *(PUINT_32)&prSkb->cb[12] = u4PacketIDIn[ucPriorityParam]++;

            //printk("In %ld[%d]\n", *(PUINT_32)&prSkb->cb[12], ucPriorityParam);
        }
#endif /* CFG_TX_DBG_PACKET_ORDER */

    }
    else if (rStatus == WLAN_STATUS_SUCCESS) {
        /* NOTE: When rStatus == WLAN_STATUS_SUCCESS, packet will be freed
         * by WLAN driver internal.
         */
    }
    else if (rStatus == WLAN_STATUS_FAILURE) {
        dev_kfree_skb(prSkb);
        prGlueInfo->u4TxPendingFrameNum--;
    }
    else {
        ASSERT(0); /* NOTE: Currently we didn't have other return status */
    }

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif /* end of defined(_HIF_SDIO) */

    if (prGlueInfo->u4TxPendingFrameNum >= CFG_TX_STOP_NETIF_QUEUE_THRESHOLD) {
        netif_stop_queue(prDev);
    }

    /* For Linux, we'll always return OK FLAG, because we'll free this skb by ourself */
    return NETDEV_TX_OK;

} /* end of wlanHardStartXmit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wlanSetMACAddress (
    IN struct net_device *prDev,
    IN void *pvAddr
    )
{
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    struct sockaddr *prMacAddr = (struct sockaddr *)pvAddr;
    UINT_32 u4SetInfoLen;
    int i4Ret = 0;

#if !defined(_HIF_SDIO)
    GLUE_SPIN_LOCK_DECLARATION();
#endif


    ASSERT(prDev);
    ASSERT(pvAddr);

#if !defined(_HIF_SDIO)
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo,
                           SPIN_LOCK_FSM);
#endif

#if defined(_HIF_SDIO)
    if (sdio_io_ctrl(prGlueInfo,
                       wlanoidSetCurrentAddr,
                       prMacAddr->sa_data,
                       PARAM_MAC_ADDR_LEN,
                       FALSE,
                       TRUE,
                       &u4SetInfoLen) == WLAN_STATUS_SUCCESS)
#else
    if (wlanSetInformation(prGlueInfo->prAdapter,
                           wlanoidSetCurrentAddr,
                           prMacAddr->sa_data,
                           PARAM_MAC_ADDR_LEN,
                           &u4SetInfoLen) == WLAN_STATUS_SUCCESS)
#endif
    {
        memcpy(prDev->dev_addr, prMacAddr->sa_data, PARAM_MAC_ADDR_LEN);
    }
    else {
        i4Ret = -EFAULT;
        printk(KERN_WARNING DRV_NAME"Fail to wlanSetMACAddress\n");
    }

#if !defined(_HIF_SDIO)
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo,
                           SPIN_LOCK_FSM);
#endif

    return i4Ret;

} /* end of wlanSetMACAddress() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
struct net_device_stats *
wlanGetStats (
    IN struct net_device *prDev
    )
{
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    PARAM_LINUX_NETDEV_STATISTICS_T rCustomNetDevStats;
    UINT_32 u4BufLen;

    ASSERT(prDev);

    memset(&rCustomNetDevStats, 0, sizeof(rCustomNetDevStats));
    wlanQueryInformation(prGlueInfo->prAdapter,
                         wlanoidQueryStatisticsForLinux,
                         &rCustomNetDevStats,
                         sizeof(rCustomNetDevStats),
                         &u4BufLen);

    prGlueInfo->rNetDevStats.rx_packets = rCustomNetDevStats.u4RxPackets;
    prGlueInfo->rNetDevStats.tx_packets = rCustomNetDevStats.u4TxPackets;
    prGlueInfo->rNetDevStats.rx_bytes   = rCustomNetDevStats.u4RxBytes;
    prGlueInfo->rNetDevStats.tx_bytes   = rCustomNetDevStats.u4TxBytes;
    prGlueInfo->rNetDevStats.rx_errors  = rCustomNetDevStats.u4RxErrors;
    prGlueInfo->rNetDevStats.tx_errors  = rCustomNetDevStats.u4TxErrors;
    prGlueInfo->rNetDevStats.multicast  = rCustomNetDevStats.u4Multicast;

    return &prGlueInfo->rNetDevStats;

} /* end of wlanGetStats() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void
wlanTasklet (
    unsigned long u4Data
    )
{
#if defined(_HIF_SDIO)
#else
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)u4Data;
#endif

#if defined(_HIF_SDIO)
    #if 0
    wlanISR(prGlueInfo->prAdapter, TRUE);
    wlanIST(prGlueInfo->prAdapter);
    #endif
#else
    GLUE_SPIN_LOCK_DECLARATION();


    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo,
                           SPIN_LOCK_FSM);

    wlanIST(prGlueInfo->prAdapter);

    GLUE_RELEASE_SPIN_LOCK(prGlueInfo,
                           SPIN_LOCK_FSM);
#endif
    return;
} /* end of wlanTasklet() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wlanInit(
    struct net_device *prDev
    )
{
#if defined(_HIF_SDIO)
#else  	
    P_GLUE_INFO_T prGlueInfo = NULL;
#endif

    printk("[MT5921] wlanInit ++\n");
    if (!prDev) {
        return -ENXIO;
    }
    
#if defined(_HIF_SDIO)
#else  
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    tasklet_init(&arWlanDevInfo[0].rTasklet,
                 wlanTasklet,
                 (unsigned long)prGlueInfo);
#endif

    return 0; /* success */
} /* end of wlanInit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void
wlanUninit(
    struct net_device *prDev
    )
{
    return;
} /* end of wlanUninit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wlanOpen(
    struct net_device *prDev
    )
{
    ASSERT(prDev);

    netif_start_queue(prDev); /* To start TX Queue */

    return 0; /* success */
} /* end of wlanOpen() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wlanStop(
    struct net_device *prDev
    )
{
    ASSERT(prDev);

    netif_stop_queue(prDev); /* To stop TX Queue */
    return 0; /* success */
} /* end of wlanStop() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static INT_32
wlanNetRegister(
    struct net_device *prDev
    )
{
    INT_32 i4DevIdx = -1;

    ASSERT(prDev);


    /* GeorgeKuo: register_netdev() helps translate wlan%d to wlan0; it
     * calls rtnl_lock() and rtnl_unlock() to do protection for
     * register_netdevice(). Avoid using register_netdevice() directly.
     */
    //rtnl_lock();

    do {
        if (!prDev) {
            break;
        }

        if ((i4DevIdx = wlanGetDevIdx(prDev)) < 0) {
            printk(KERN_ALERT "wlanNetRegister: net_device number exceeds.\n");
            break;
        }

        if (register_netdev(prDev) < 0) {
            printk(KERN_ALERT "wlanNetRegister: net_device context is not registered.\n");
            wlanClearDevIdx(prDev);
            i4DevIdx = -1;
        }

    }
    while(FALSE);

    //rtnl_unlock();

    return i4DevIdx; /* success */

} /* end of wlanNetRegister() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static VOID
wlanNetUnregister(
    struct net_device *prDev
    )
{
    /* Waiting for executing tasklet completion.
     * Note: before tasklet completion, all resources should be kept.
     */

    /* GeorgeKuo: unregister_netdev() uses rtnl_lock() to protect
     * unregister_netdevice(). Avoid using unregister_netdevice() directly.
     */

    //rtnl_lock();

    if (!prDev) {
        //rtnl_unlock();
        printk(KERN_ALERT DRV_NAME "wlanNetUnregister: The device context is NULL\n");
        //ASSERT(prDev);
        return;
    }

    wlanClearDevIdx(prDev);
    //unregister_netdevice(prDev);
    unregister_netdev(prDev);

    printk(KERN_INFO DRV_NAME "unregisterdev(0x%p)\n", prDev);
    //rtnl_unlock();

    return;
} /* end of wlanNetUnregister() */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))

	struct net_device_ops wlan_device_ops = {
	    .ndo_open  = wlanOpen,
	    .ndo_stop = wlanStop,
	    .ndo_get_stats = wlanGetStats,
	    .ndo_set_mac_address = wlanSetMACAddress,
	    .ndo_set_multicast_list = wlanSetMulticastList,
	    .ndo_start_xmit   = wlanHardStartXmit,
	    .ndo_do_ioctl  = wlanDoIOCTL,
	    .ndo_init  = wlanInit,
	    .ndo_uninit = wlanUninit,
	};

#endif


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static struct net_device *
wlanNetCreate(
    PVOID pvData
    )
{
    struct net_device *prDev = NULL;
    P_GLUE_INFO_T prGlueInfo =  NULL;
    P_ADAPTER_T prAdapter = NULL;
    UINT_32 i;

    ASSERT(pvData);

    //4 <1> Create netdev
    /* GeorgeKuo: use netdev allocation function alloc_netdev(). Kernel ensures
     * 32-byte alignment and zero initialization.
     */
    prDev = alloc_netdev(sizeof(GLUE_INFO_T), NIC_INF_NAME, ether_setup);
    printk(KERN_INFO DRV_NAME "net_device prDev(0x%p) allocated\n", prDev);

    if (!prDev) {
        printk(KERN_ALERT DRV_NAME "Allocating memory to net_device context failed\n");
        return NULL;
    }

    //4 <2> Create Glue structure
    /* GeorgeKuo: private data is allocated by alloc_netdev. Using it directly
     * is discouraged.
     */
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);

    if (!prGlueInfo) {
        printk(KERN_ALERT DRV_NAME"Allocating memory to glue layer failed\n");
        /* GeorgeKuo: free glue info with free_netdev() */
        //kalMemFree(prDev, sizeof(struct net_device));
        goto netcreate_err;
    }

    //4 <3> Initial Glue structure
    prGlueInfo->prDevHandler = prDev;

    prGlueInfo->eParamMediaStateIndicated = PARAM_MEDIA_STATE_DISCONNECTED;
    prGlueInfo->ePowerState = ParamDeviceStateD0;

    prGlueInfo->fgRadioOn = FALSE;

#if CFG_SUPPORT_EXT_CONFIG
    prGlueInfo->eCfgSrcType = CFG_SRC_TYPE_NVRAM;
    prGlueInfo->u4ExtCfgLength = 0;
#endif
   prGlueInfo->u4LedSetting = 0;
   prGlueInfo->u2LedBlinkMode = gPlatformCfg.rWifiCustom.u2LedBlinkMode;
   prGlueInfo->u4LedBlinkOnTime = gPlatformCfg.rWifiCustom.u4LedBlinkOnTime; 
   prGlueInfo->u4LedBlinkOffTime = gPlatformCfg.rWifiCustom.u4LedBlinkOffTime;

    for (i = 0; i < SPIN_LOCK_NUM; i++) {
        spin_lock_init(&prGlueInfo->rSpinLock[i]);
    }

    //4 <4> Create Adapter structure
    prAdapter = (P_ADAPTER_T)wlanAdapterCreate(prGlueInfo);

    if (!prAdapter) {
        printk(KERN_ALERT DRV_NAME "Allocating memory to adapter failed\n");
        goto netcreate_err;
    }

    prGlueInfo->prAdapter = prAdapter;

    //4 <5> Set the ioaddr to HIFInfo
    glSetHifInfo(prGlueInfo, (UINT_32)pvData);

    /* MTK, Infinity, 20090819, not used and removed { */
    //prDev->irq = WLAN_STA_IRQ;
    /* MTK, Infinity, 20090819, not used and removed } */

    //4 <6> Register Callback function
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31))
    prDev->open  = wlanOpen;
    prDev->stop  = wlanStop;
    
    /*prDev->change_mtu    = wlanChangeMTU;*/
    prDev->get_stats = wlanGetStats;
    prDev->set_mac_address = wlanSetMACAddress;
    prDev->set_multicast_list = wlanSetMulticastList;
    prDev->hard_start_xmit   = wlanHardStartXmit;

    /* GeorgeKuo: wait to be removed? */
    prDev->do_ioctl  = wlanDoIOCTL;
    prDev->init  = wlanInit;
    prDev->uninit = wlanUninit;
#else
	prDev->netdev_ops = &wlan_device_ops;

#endif
    //4 <7> Use wireless extension to replace IOCTL
    prDev->wireless_handlers = &wext_handler_def;

    //4 <8> Those Callback function which are not required or TODO

    //4 <8> Use wireless extension to replace IOCTL
/*    prDev->wireless_handlers =   NULL; */

    netif_carrier_off(prDev);
    netif_stop_queue(prDev);
    goto netcreate_done;

netcreate_err:
    if (NULL != prAdapter) {
        wlanAdapterDestroy(prAdapter);
        prAdapter = NULL;
    }

    if (NULL != prDev) {
        /* Free net_device and private data, which are allocated by
         * alloc_netdev().
         */
        free_netdev(prDev);
        prDev = NULL;
    }

netcreate_done:

    return prDev;

} /* end of wlanNetCreate() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static VOID
wlanNetDestroy(
    struct net_device *prDev
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;

    ASSERT(prDev);

    if (!prDev) {
        printk(KERN_ALERT "wlanNetDestroy: The device context is NULL\n");
        return;
    }

    /* prGlueInfo is allocated with net_device */
    prGlueInfo = netdev_priv(prDev);
    ASSERT(prGlueInfo);

    glClearHifInfo(prGlueInfo);

    wlanAdapterDestroy(prGlueInfo->prAdapter);
    prGlueInfo->prAdapter = NULL;

    /* Free net_device and private data, which are allocated by alloc_netdev().
     */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
	prDev->netdev_ops = NULL;
#endif
	
    free_netdev(prDev);

    return;
} /* end of wlanNetDestroy() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
VOID
wlanHWInterrupt (
    INT_32 i4Irq,
    PVOID pvDevID,
    struct pt_regs *prRegs
    )
{
    ASSERT(pvDevID);

    return;
} /* end of wlanHWInterrupt() */

#else

irqreturn_t
wlanHWInterrupt (
    INT_32 i4Irq,
    PVOID pvDevID,
    struct pt_regs *prRegs
    )
{
#if defined(_HIF_SDIO)

#else
    P_WLANDEV_INFO_T prWlandevInfo = (P_WLANDEV_INFO_T)pvDevID;
    struct net_device *prDev = prWlandevInfo->prDev;
    struct tasklet_struct *prTasklet = &prWlandevInfo->rTasklet;
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    P_ADAPTER_T prAdapter = (P_ADAPTER_T)prGlueInfo->prAdapter;


    ASSERT(pvDevID);

    wlanISR(prAdapter, TRUE);

    tasklet_hi_schedule(prTasklet);
#endif

    return IRQ_HANDLED;
/* NOTE(Kevin): Should we use return IRQ_HANDLED; instead of IRQ_NONE */

} /* end of wlanHWInterrupt() */
#endif

struct net_device *prNetDev = NULL;
PARAM_POWER_MODE queryPower = 0;

void wlanSuspend(void)
{
	UINT_8  ip[4];
     	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	UINT_32  U32_ret;
		
	printk(KERN_INFO "*********wlanSuspend*****\n");

	if(0 == gPlatformCfg.rWifiCustom.u4L3PktFilterEn)
		return;
	
	if(!prNetDev || !(prNetDev->ip_ptr)||\
		!((struct in_device *)(prNetDev->ip_ptr))->ifa_list||\
		!(&(((struct in_device *)(prNetDev->ip_ptr))->ifa_list->ifa_local))){
		printk(KERN_INFO "ip is not avaliable.\n");		
		return;
	}

	ASSERT(prNetDev);
	kalMemCopy(ip, &(((struct in_device *)(prNetDev->ip_ptr))->ifa_list->ifa_local), sizeof(ip));

        prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);
        prAdapter = prGlueInfo->prAdapter;
	printk(KERN_INFO"ip is %d.%d.%d.%d\n", 
			ip[0],ip[1],ip[2],ip[3]);

#if defined(_HIF_SDIO)
	sdio_io_ctrl(prGlueInfo,
		wlanoidQuery802dot11PowerSaveProfile,
		&queryPower,
		sizeof(queryPower),
		TRUE,
		TRUE,
		&U32_ret);
	#else
	wlanQueryInformation(prGlueInfo->prAdapter,
		wlanoidQuery802dot11PowerSaveProfile,
		&queryPower,
		sizeof(queryPower),
		&U32_ret);
	#endif

	if(queryPower !=  Param_PowerModeFast_PSP){
		PARAM_POWER_MODE ePowerMode = Param_PowerModeFast_PSP;
	
		#if defined(_HIF_SDIO)
		sdio_io_ctrl(prGlueInfo,
			wlanoidSet802dot11PowerSaveProfile,
			&ePowerMode,
			sizeof(ePowerMode),
			FALSE,
			TRUE,
			&U32_ret);
		#else
		wlanSetInformation(prGlueInfo->prAdapter,
			wlanoidSet802dot11PowerSaveProfile,
			&ePowerMode,
			sizeof(ePowerMode),
			&U32_ret);
		#endif			
	}


	if(1 == gPlatformCfg.rWifiCustom.u4L3PktFilterEn){	
	
		#if defined(_HIF_SDIO)
			sdio_io_ctrl(prGlueInfo,
		 wlanoidSetIpAddress,
		 (PVOID)ip,
		  sizeof(ip),
		  FALSE,
		  TRUE,
		  &U32_ret);
#else
	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
	wlanSetInformation(prGlueInfo->prAdapter,
		   wlanoidSetIpAddress,
		   (PVOID)ip,
		   sizeof(ip),
		   &U32_ret);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif
	}else if(2 == gPlatformCfg.rWifiCustom.u4L3PktFilterEn){
		UINT_8 on = 0;

		#if defined(_HIF_SDIO)
		sdio_io_ctrl(prGlueInfo,
			wlanRxSetBroadcast,
			(PVOID)on,
			sizeof(on),
			FALSE,
			TRUE,
			&U32_ret);
		#else
		GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		wlanSetInformation(prGlueInfo->prAdapter,
			wlanRxSetBroadcast,
			(PVOID)on,
			sizeof(on),
			&U32_ret);
		GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		#endif		
	}
}

void wlanResume(void)
{
	UINT_8  ip[4] = {0, 0, 0, 0};
     	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	UINT_32	U32_ret;

	printk(KERN_INFO "*********wlanResume******\n");

	if(0 == gPlatformCfg.rWifiCustom.u4L3PktFilterEn)
		return;

	if(1 == gPlatformCfg.rWifiCustom.u4L3PktFilterEn){
	if(!prNetDev || !(prNetDev->ip_ptr))
		return;

	ASSERT(prNetDev);
        prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);
        prAdapter = prGlueInfo->prAdapter;

#if defined(_HIF_SDIO)
	sdio_io_ctrl(prGlueInfo,
		 wlanoidSetIpAddress,
		 (PVOID)ip,
		  sizeof(ip),
		  FALSE,
		  TRUE,
		  &U32_ret);
#else
	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
	wlanSetInformation(prGlueInfo->prAdapter,
		   wlanoidSetIpAddress,
		   (PVOID)ip,
		   sizeof(ip),
		   &U32_ret);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif
	}else if(2 == gPlatformCfg.rWifiCustom.u4L3PktFilterEn){
		if(!prNetDev || !(prNetDev->ip_ptr)||\
			!((struct in_device *)(prNetDev->ip_ptr))->ifa_list||\
			!(&(((struct in_device *)(prNetDev->ip_ptr))->ifa_list->ifa_local))){
			printk(KERN_INFO "ip is not avaliable.\n");		
			return;
}

		ASSERT(prNetDev);
		prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

		UINT_8 on = 1;
		#if defined(_HIF_SDIO)
		sdio_io_ctrl(prGlueInfo,
			wlanRxSetBroadcast,
			(PVOID)on,
			sizeof(on),
			FALSE,
			TRUE,
			&U32_ret);
		#else
		GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		wlanSetInformation(prGlueInfo->prAdapter,
			wlanRxSetBroadcast,
			(PVOID)on,
			sizeof(on),
			&U32_ret);
		GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		#endif		
	}

	{
		PARAM_POWER_MODE ePowerMode = queryPower;
	
		#if defined(_HIF_SDIO)
		sdio_io_ctrl(prGlueInfo,
			wlanoidSet802dot11PowerSaveProfile,
			&ePowerMode,
			sizeof(ePowerMode),
			FALSE,
			TRUE,
			&U32_ret);
		#else
		wlanSetInformation(prGlueInfo->prAdapter,
			wlanoidSet802dot11PowerSaveProfile,
			&ePowerMode,
			sizeof(ePowerMode),
			&U32_ret);
		#endif	
	}
}

int wlan_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk(KERN_INFO "*********wlan_platform_suspend************\n");
	if(state.event == PM_EVENT_SUSPEND)
		wlanSuspend();	

	return 0;
}

int wlan_platform_resume(struct platform_device *pdev)

{
	printk(KERN_INFO "*********wlan_platform_resume************\n");
	wlanResume();

	return 0;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static INT_32
wlanProbe(
    PVOID pvData
    )
{
    struct net_device *prDev = NULL;
#if defined (_HIF_EHPI)
    UINT_32 u4MemBase = 0;
#endif
    P_WLANDEV_INFO_T prWlandevInfo = NULL;
    INT_32 i4DevIdx = 0;
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_ADAPTER_T prAdapter = NULL;
    INT_32 i4Status = 0;
    BOOL bRet = FALSE;


    do {

        //4 <0> Setup Debug Level
#if DBG
        int i;
        /* Initialize DEBUG CLASS of each module */
        for (i = 0; i < DBG_MODULE_NUM; i++) {
            aucDebugModule[i] = DBG_CLASS_ERROR | \
                                DBG_CLASS_WARN | \
                                DBG_CLASS_STATE | \
                                DBG_CLASS_EVENT;
            //aucDebugModule[i] = 0;
        }
        aucDebugModule[DBG_RFTEST_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_REQ_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
#if 0
        aucDebugModule[DBG_INIT_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_ARB_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_JOIN_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        //aucDebugModule[DBG_RX_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_TX_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_NIC_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_HAL_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_KEVIN_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO | DBG_CLASS_TEMP;
        aucDebugModule[DBG_SCAN_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_REQ_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        //aucDebugModule[DBG_MGT_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
        aucDebugModule[DBG_RSN_IDX] |= DBG_CLASS_TRACE;
        aucDebugModule[DBG_ROAMING_IDX] |= DBG_CLASS_TRACE | DBG_CLASS_INFO;
#endif
#endif /* DBG */

        //4 <1> Initialize the IO port of the interface
        /*  GeorgeKuo: pData has different meaning for _HIF_XXX:
         * _HIF_EHPI: pointer to memory base variable, which will be
         *      initialized by glBusInit().
         * _HIF_SDIO: bus driver handle
         */
#if defined (_HIF_EHPI)
        pvData = &u4MemBase;
#endif
#if 0  // glBusInit is done in SDIO_CLNT
        bRet = glBusInit(pvData);
#endif 
        bRet = TRUE;

        /* Cannot get IO address from interface */
        if (FALSE == bRet) {
            DBGLOG(INIT, ERROR, (KERN_ALERT "wlanProbe: glBusInit() fail\n"));
            i4Status = -EIO;
            break;
        }

        initCustomData();
	
      
		//4 <2> Create network device, Adapter, KalInfo, prDevHandler(netdev)
        if ((prDev = wlanNetCreate(pvData)) == NULL) {
            printk(KERN_ALERT "wlanProbe: No memory for dev and its private\n");
            i4Status = -ENOMEM;
            break;
        }

        prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);

        prAdapter = prGlueInfo->prAdapter;

        //4 <5> Start Device
        if (wlanAdapterStart(prAdapter, &gPlatformCfg.rWifiCustom) != WLAN_STATUS_SUCCESS) {
            i4Status = -EIO;
            printk(KERN_INFO DRV_NAME"wlanAdapterStart fail\n");
            break;
        }
	
       //4 <3> Register the card
        if ((i4DevIdx = wlanNetRegister(prDev)) < 0){
            i4Status = -ENXIO;
            printk(KERN_ALERT "wlanProbe: Cannot register the net_device context to the kernel\n");
            break;
        }

        //4 <4> Setup IRQ
        prWlandevInfo = &arWlanDevInfo[i4DevIdx];
#if defined(_HIF_SDIO)
        i4Status = glBusSetIrq(prDev, NULL, netdev_priv(prDev));
#else
        i4Status = glBusSetIrq(prDev, wlanHWInterrupt, prWlandevInfo);
#endif
        if (i4Status != WLAN_STATUS_SUCCESS) {
            printk(KERN_ALERT "wlanProbe: Set IRQ error\n");
            break;
        }
        prGlueInfo->i4DevIdx = i4DevIdx;

        //4 <6> Initialize /proc filesystem
#ifdef WLAN_INCLUDE_PROC
        if ( (i4Status = procInitProcfs(prDev, prDev->name)) < 0) {
            printk(KERN_ALERT "wlanProbe: init procfs failed\n");
            break;
        }
#endif /* WLAN_INCLUDE_PROC */

	//5 for platform_driver_register
	prNetDev = prDev;

#ifdef CFG_EINT_HANDLED_IN_WLAN
	//6 Enable the IRQ
	kalDevEnableIrq();
#endif
    }
    while (FALSE);

    //printk("prDev(0x%p), prGlueInfo->prDevHandler(0x%p)\n", prDev, prGlueInfo->prDevHandler);
    //printk("private of prDev(0x%p), prGlueInfo(0x%p)\n", netdev_priv(prDev), prGlueInfo);
    //printk("prGlueInfo->prAdapter(0x%p)\n", prGlueInfo->prAdapter);


    return i4Status;

} /* end of wlanProbe() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static VOID
wlanRemove(
    VOID
    )
{
    struct net_device *prDev = arWlanDevInfo[0].prDev;
    P_WLANDEV_INFO_T prWlandevInfo = &arWlanDevInfo[0];
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_ADAPTER_T prAdapter = NULL;

#ifndef _HIF_SDIO
    GLUE_SPIN_LOCK_DECLARATION();
#endif

    if(prDev == NULL){
       printk("[MT5921] wlanRemove prDev = NULL\n");
       return;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);

    if(prGlueInfo == NULL){
       printk("[MT5921] wlanRemove prGlueInfo = NULL\n");
       return;
    }
    prAdapter = prGlueInfo->prAdapter;
    
    if(prAdapter == NULL){
       printk("[MT5921] wlanRemove prAdapter = NULL\n");
       return;
    }

     //5 for platform_driver_register
       prNetDev = NULL;

    //4 <1> Remove /proc filesystem.
#ifdef WLAN_INCLUDE_PROC
     procRemoveProcfs(prDev, prDev->name);
#endif /* WLAN_INCLUDE_PROC */

    //4 <5> Unregister the card
    wlanNetUnregister(prDev);

    //4 <2> wlanAdapterStop
#ifndef _HIF_SDIO
    GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    wlanAdapterStop(prAdapter);

    printk(KERN_INFO DRV_NAME "Number of Stalled Packets = %ld\n", prGlueInfo->u4TxPendingFrameNum);

#ifdef CFG_EINT_HANDLED_IN_WLAN
    //Disable the IRQ
   kalDevDisableIrq();
#endif

    //4 <3> Free IRQ
    glBusFreeIrq(prDev, prWlandevInfo);

#ifndef _HIF_SDIO
    GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
#endif

    //4 <4> Release the Bus
    glBusRelease(prDev);

    //4 <6> Destroy the device
    wlanNetDestroy(prDev);
    prDev = NULL;

    return;
} /* end of wlanRemove() */

void initCustomData(void)
{
    if(gPlatformCfg.u4Customlen != CFG_FILE_WIFI_CUSTOM_REC_SIZE)
    {
        /* invalid custom data, use default value */
        printk("[MT5921][custom] invalid custom data.. use default setting ....\n");
        kalMemSet((void *)&gPlatformCfg.rWifiCustom,0,CFG_FILE_WIFI_CUSTOM_REC_SIZE);
		
	    gPlatformCfg.rWifiCustom.u4SdBlockSize = 512;      /* SDIO block size */
	    gPlatformCfg.rWifiCustom.u4SdBusWidth = 4;       /* SDIO bus width. 1 or 4 */
	    gPlatformCfg.rWifiCustom.u4SdClockRate = 0;      /* SDIO clock rate. (in unit of HZ) */

	    gPlatformCfg.rWifiCustom.u4BTCoexistWindowType = 0; /*  */
	    gPlatformCfg.rWifiCustom.u4EnableTxAutoFragmentForBT = 0; /*  */
	    gPlatformCfg.rWifiCustom.u4BtCR0 = 0;            /* BTCR0 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4BtCR1 = 0;            /* BTCR1 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4BtCR2 = 0;            /* BTCR2 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4BtCR3 = 0;            /* BTCR3 value (correcspondent to register setting) */

	    gPlatformCfg.rWifiCustom.u4SingleAclBtCR0 = 0;            /* BTCR0 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4SingleAclBtCR1 = 0;            /* BTCR1 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4SingleAclBtCR2 = 0;            /* BTCR2 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4SingleAclBtCR3 = 0;            /* BTCR3 value (correcspondent to register setting) */

	    gPlatformCfg.rWifiCustom.u4SingleMixBtCR0 = 0;            /* BTCR0 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4SingleMixBtCR1 = 0;            /* BTCR1 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4SingleMixBtCR2 = 0;            /* BTCR2 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4SingleMixBtCR3 = 0;            /* BTCR3 value (correcspondent to register setting) */

	    gPlatformCfg.rWifiCustom.u4DualAclBtCR0 = 0;            /* BTCR0 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4DualAclBtCR1 = 0;            /* BTCR1 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4DualAclBtCR2 = 0;            /* BTCR2 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4DualAclBtCR3 = 0;            /* BTCR3 value (correcspondent to register setting) */

	    gPlatformCfg.rWifiCustom.u4DualMixBtCR0 = 0;            /* BTCR0 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4DualMixBtCR1 = 0;            /* BTCR1 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4DualMixBtCR2 = 0;            /* BTCR2 value (correcspondent to register setting) */
	    gPlatformCfg.rWifiCustom.u4DualMixBtCR3 = 0;            /* BTCR3 value (correcspondent to register setting) */

	    gPlatformCfg.rWifiCustom.u4BTSetting = 1;        /* useless for BT only */
	    gPlatformCfg.rWifiCustom.u4SingleAntenna = 1;    /* 0: Not single antenna, 1: Single antenna */
	    gPlatformCfg.rWifiCustom.u4BtProfile = 0;        /* 0: No PTA, 1: SCO, 2: A2DP(ACL), 3: See btcr0-3 */
	    gPlatformCfg.rWifiCustom.u4PtaEnabled = 0;

	    //gPlatformCfg.rWifiCustom.aucMacAddr[6];      /* MAC address */
	    //gPlatformCfg.rWifiCustom.aucCountryCode[4] = CFG_COUNTRY_CODE;  /* Country code (in ISO 3166-1 expression, ex: "US", "TW")  */
	    gPlatformCfg.rWifiCustom.u4UapsdAc = CFG_INIT_UAPSD_AC;          /* UAPSD AC (in bitwise OR of: a. 0x11-AC0, b. 0x22-AC1, a. 0x44-AC0, a. 0x88-AC0) */
	    gPlatformCfg.rWifiCustom.u4PowerMode = CFG_INIT_POWER_SAVE_PROF;        /* Power mode (0: CAM, 1: MAX, 2: FAST) */
	    gPlatformCfg.rWifiCustom.u4AtimWindow = CFG_IBSS_ATIM_WINDOW;       /* ATIM Window value (in unit of TU) */
	    gPlatformCfg.rWifiCustom.u4VoipInterval = CFG_INIT_VOIP_INTERVAL;     /* VoIP interval (in unit of ms) */
	    gPlatformCfg.rWifiCustom.u4PollInterval = CFG_INIT_POLL_INTERVAL;     /* Poll interval (in unit of ms) */
	    gPlatformCfg.rWifiCustom.u4PollIntervalB = 500;    /* Poll interval, if Wi-Fi no traffic (in unit of ms) */
	    gPlatformCfg.rWifiCustom.u4L3PktFilterEn = CFG_L3_PATTERN_MATCH_WAKEUP;    /* Enable pattern search for broadcast ARP filter */
	    gPlatformCfg.rWifiCustom.u4AdHocMode = CFG_INIT_ADHOC_MODE;        /* Adhoc mode */
	    gPlatformCfg.rWifiCustom.u4RoamingEn = CFG_ROAMING;        /* Enable Roaming function */

	    gPlatformCfg.rWifiCustom.u4MultiDomainCap = 0;   /* Multi-domain capability */

	    gPlatformCfg.rWifiCustom.u4GPIO2_Mode = 0;       /* GPIO2 mode (0: GPI, 1: INT, 2: Daisy) */

	    gPlatformCfg.rWifiCustom.u4ViAifsnBias = 0;      /* Define VI AIFSN BIAS (Default: 0, 0~15) */
	                                        /* Actual AIFSN = AIFSN from AP + AIFSN Bias */
	    gPlatformCfg.rWifiCustom.u4ViMaxTxopLimit = 0xFFFF;   /* Define VI MAX TXOP LIMIT(A upper bound) (Default: 0xFFFF, 0~0xFFFF) */

	    gPlatformCfg.rWifiCustom.u4InitDelayInd = 100;     /* Define how many MS to indicate DISCONNECTION after initialized */


	    gPlatformCfg.rWifiCustom.u4UseWapi = 0;          /* Define use at wapi mode, control by wapi ui (Default: 0 disable, 1~0xFFFFFFFF: enable) */

	    gPlatformCfg.rWifiCustom.u4DaisyChainEnable = 1; /* whether Daisy Chain is enabled: 0: disabled, 1: enabled */

		gPlatformCfg.rWifiCustom.u2LedBlinkMode = 0;
		gPlatformCfg.rWifiCustom.u4LedBlinkOnTime = 0;
		gPlatformCfg.rWifiCustom.u4LedBlinkOffTime = 0;

        gPlatformCfg.rWifiCustom.ucWmmPsEnable = 0;
	gPlatformCfg.rWifiCustom.u4MultiDtimWake = 0;
		//gPlatformCfg.rWifiCustom.u4Resv[16];         /* Reserved */
	}
#if	DBG
	printk("[MT5921][CUSTOM] u4SdBlockSize                0x%lx\n", gPlatformCfg.rWifiCustom.u4SdBlockSize);
	printk("[MT5921][CUSTOM] u4SdBusWidth                 0x%lx\n", gPlatformCfg.rWifiCustom.u4SdBusWidth);
	printk("[MT5921][CUSTOM] u4SdClockRate                0x%lx\n", gPlatformCfg.rWifiCustom.u4SdClockRate);                         
	printk("[MT5921][CUSTOM] u4BTCoexistWindowType        0x%lx\n", gPlatformCfg.rWifiCustom.u4BTCoexistWindowType);
	printk("[MT5921][CUSTOM] u4EnableTxAutoFragmentForBT  0x%lx\n", gPlatformCfg.rWifiCustom.u4EnableTxAutoFragmentForBT);
	printk("[MT5921][CUSTOM] u4BtCR0                      0x%lx\n", gPlatformCfg.rWifiCustom.u4BtCR0);
	printk("[MT5921][CUSTOM] u4BtCR1                      0x%lx\n", gPlatformCfg.rWifiCustom.u4BtCR1);
	printk("[MT5921][CUSTOM] u4BtCR2                      0x%lx\n", gPlatformCfg.rWifiCustom.u4BtCR2);
	printk("[MT5921][CUSTOM] u4BtCR3                      0x%lx\n", gPlatformCfg.rWifiCustom.u4BtCR3);
	printk("[MT5921][CUSTOM] u4SingleAclBtCR0             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleAclBtCR0);	
	printk("[MT5921][CUSTOM] u4SingleAclBtCR1             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleAclBtCR1);
	printk("[MT5921][CUSTOM] u4SingleAclBtCR2             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleAclBtCR2);
	printk("[MT5921][CUSTOM] u4SingleAclBtCR3             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleAclBtCR3);
	printk("[MT5921][CUSTOM] u4SingleMixBtCR0             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleMixBtCR0);
	printk("[MT5921][CUSTOM] u4SingleMixBtCR1             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleMixBtCR1);
	printk("[MT5921][CUSTOM] u4SingleMixBtCR2             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleMixBtCR2);
	printk("[MT5921][CUSTOM] u4SingleMixBtCR3             0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleMixBtCR3);
	printk("[MT5921][CUSTOM] u4DualAclBtCR0               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualAclBtCR0);
	printk("[MT5921][CUSTOM] u4DualAclBtCR1               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualAclBtCR1);
	printk("[MT5921][CUSTOM] u4DualAclBtCR2               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualAclBtCR2);
	printk("[MT5921][CUSTOM] u4DualAclBtCR3               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualAclBtCR3);
	printk("[MT5921][CUSTOM] u4DualMixBtCR0               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualMixBtCR0);
	printk("[MT5921][CUSTOM] u4DualMixBtCR1               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualMixBtCR1);
	printk("[MT5921][CUSTOM] u4DualMixBtCR2               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualMixBtCR2);
	printk("[MT5921][CUSTOM] u4DualMixBtCR3               0x%lx\n", gPlatformCfg.rWifiCustom.u4DualMixBtCR3);
	printk("[MT5921][CUSTOM] u4BTSetting                  0x%lx\n", gPlatformCfg.rWifiCustom.u4BTSetting );
	printk("[MT5921][CUSTOM] u4SingleAntenna              0x%lx\n", gPlatformCfg.rWifiCustom.u4SingleAntenna );
	printk("[MT5921][CUSTOM] u4BtProfile                  0x%lx\n", gPlatformCfg.rWifiCustom.u4BtProfile );
	printk("[MT5921][CUSTOM] u4PtaEnabled                 0x%lx\n", gPlatformCfg.rWifiCustom.u4PtaEnabled);
	printk("[MT5921][CUSTOM] u4UapsdAc                    0x%lx\n", gPlatformCfg.rWifiCustom.u4UapsdAc );
	printk("[MT5921][CUSTOM] u4PowerMode                  0x%lx\n", gPlatformCfg.rWifiCustom.u4PowerMode);
	printk("[MT5921][CUSTOM] u4AtimWindow                 0x%lx\n", gPlatformCfg.rWifiCustom.u4AtimWindow);
	printk("[MT5921][CUSTOM] u4VoipInterval               0x%lx\n", gPlatformCfg.rWifiCustom.u4VoipInterval );
	printk("[MT5921][CUSTOM] u4PollInterval               0x%lx\n", gPlatformCfg.rWifiCustom.u4PollInterval );
	printk("[MT5921][CUSTOM] u4PollIntervalB              0x%lx\n", gPlatformCfg.rWifiCustom.u4PollIntervalB);
	printk("[MT5921][CUSTOM] u4L3PktFilterEn              0x%lx\n", gPlatformCfg.rWifiCustom.u4L3PktFilterEn);
	printk("[MT5921][CUSTOM] u4AdHocMode                  0x%lx\n", gPlatformCfg.rWifiCustom.u4AdHocMode );
	printk("[MT5921][CUSTOM] u4RoamingEn                  0x%lx\n", gPlatformCfg.rWifiCustom.u4RoamingEn );
	printk("[MT5921][CUSTOM] u4MultiDomainCap             0x%lx\n", gPlatformCfg.rWifiCustom.u4MultiDomainCap);
	printk("[MT5921][CUSTOM] u4GPIO2_Mode                 0x%lx\n", gPlatformCfg.rWifiCustom.u4GPIO2_Mode);
	printk("[MT5921][CUSTOM] u4ViAifsnBias                0x%lx\n", gPlatformCfg.rWifiCustom.u4ViAifsnBias);
	printk("[MT5921][CUSTOM] u4ViMaxTxopLimit             0x%lx\n", gPlatformCfg.rWifiCustom.u4ViMaxTxopLimit);
	printk("[MT5921][CUSTOM] u4InitDelayInd               0x%lx\n", gPlatformCfg.rWifiCustom.u4InitDelayInd);
	printk("[MT5921][CUSTOM] u4UseWapi                    0x%lx\n", gPlatformCfg.rWifiCustom.u4UseWapi );
	printk("[MT5921][CUSTOM] u4DaisyChainEnable           0x%lx\n", gPlatformCfg.rWifiCustom.u4DaisyChainEnable);
	printk("[MT5921][CUSTOM] u2LedBlinkMode               0x%lx\n", gPlatformCfg.rWifiCustom.u2LedBlinkMode );
	printk("[MT5921][CUSTOM] u4LedBlinkOnTime             0x%lx\n", gPlatformCfg.rWifiCustom.u4LedBlinkOnTime );
	printk("[MT5921][CUSTOM] u4LedBlinkOffTime            0x%lx\n", gPlatformCfg.rWifiCustom.u4LedBlinkOffTime );
	printk("[MT5921][CUSTOM] ucWmmPsEnable                0x%lx\n", gPlatformCfg.rWifiCustom.ucWmmPsEnable );
	printk("[MT5921][CUSTOM] u4MultiDtimWake              0x%lx\n", gPlatformCfg.rWifiCustom.u4MultiDtimWake );
#endif	
    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

//FIX ME
extern int btwlan_module_power_handler_ext( int id, int op );

extern int hifRegisterPlatformDriver (
	suspend_callback	wlanSuspend,
	resume_callback		wlanResume
    );


extern int hifUnregisterPlatformDriver (
    );

//1 Module Entry Point
static int __init initWlan(void)
{
	int ret = -1;
#if 0
    u32 wdata[64] = {0x01234567, 0x89ABCDEF};
    struct spi_dev *prSpiDev;
    int ret;
    int i = 0;
#endif

    printk( KERN_INFO "initWlan\n");

#if 0
    prSpiDev = spi_dev_get_by_minor(0);

    for (i = 0; i < 10; i++) {
        printk(KERN_INFO DRV_NAME"loop: %d\n", i);
        ret = spi_master_send(prSpiDev, (char *)wdata, 2*sizeof(u32));
    }

    printk("[HIKO DEBUG]XXXXXX ret=0x%X  \n\n", ret);
    return 0;
#endif
	ret = ((glRegisterBus(wlanProbe, wlanRemove) == WLAN_STATUS_SUCCESS) ? 0: -EIO);
	ret = ((hifRegisterPlatformDriver(wlan_platform_suspend, wlan_platform_resume) == WLAN_STATUS_SUCCESS) ? 0: -EIO);
	printk(KERN_INFO"hifRegisterPlatformDriver return %d\n", ret);

    return ret;
} /* end of initWlan() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
//1 Module Leave Point
static VOID __exit exitWlan(void)
{
   hifUnregisterPlatformDriver();	 
    glUnregisterBus(wlanRemove);

    printk( KERN_INFO "exitWlan\n");

    return;
} /* end of exitWlan() */

module_init(initWlan);
module_exit(exitWlan);


