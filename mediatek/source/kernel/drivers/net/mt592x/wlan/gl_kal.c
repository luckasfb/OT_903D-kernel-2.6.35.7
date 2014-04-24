






#include "gl_os.h"
#include "util.h"

#include "gl_wext.h"
#include "gl_wext_priv.h"



#if DBG
int allocatedMemSize = 0;
#endif

extern WIFI_CFG_DATA gPlatformCfg;




/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalAcquireSpinLock (
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_SPIN_LOCK_CATEGORY_E    rLockCategory,
    OUT PUINT_32                    pu4Flags
    )
{
    UINT_32 u4Flags = 0;

    ASSERT(prGlueInfo);
    ASSERT(pu4Flags);

    if (rLockCategory < SPIN_LOCK_NUM) {

#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
        spin_lock_bh(&prGlueInfo->rSpinLock[rLockCategory]);
#else /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
        spin_lock_irqsave(&prGlueInfo->rSpinLock[rLockCategory], u4Flags);
#endif /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */

        *pu4Flags = u4Flags;
    }

    return;
} /* end of kalAcquireSpinLock() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalReleaseSpinLock (
    IN P_GLUE_INFO_T                prGlueInfo,
    IN ENUM_SPIN_LOCK_CATEGORY_E    rLockCategory,
    IN UINT_32                      u4Flags
    )
{
    ASSERT(prGlueInfo);

    if (rLockCategory < SPIN_LOCK_NUM) {

#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
        spin_unlock_bh(&prGlueInfo->rSpinLock[rLockCategory]);
#else /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
        spin_unlock_irqrestore(&prGlueInfo->rSpinLock[rLockCategory], u4Flags);
#endif /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */

    }

    return;
} /* end of kalReleaseSpinLock() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateMACAddress (
    IN P_GLUE_INFO_T prGlueInfo,
    IN PUINT_8 pucMacAddr
    )
{
    ASSERT(prGlueInfo);
    ASSERT(pucMacAddr);

    memcpy(prGlueInfo->prDevHandler->dev_addr, pucMacAddr, PARAM_MAC_ADDR_LEN);

    return;
}


#if CFG_TCP_IP_CHKSUM_OFFLOAD
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalQueryTxChksumOffloadParam (
    IN PVOID pvPacket,
    OUT PUINT_8 pucFlag
    )
{
    struct sk_buff *skb = (struct sk_buff *)pvPacket;
    UINT_8 ucFlag = 0;

    ASSERT(pvPacket);
    ASSERT(pucFlag);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
    if (skb->ip_summed == CHECKSUM_HW)
#else
    if (skb->ip_summed == CHECKSUM_PARTIAL)
#endif
    {

#if DBG
        /* Kevin: do double check, we can remove this part in Normal Driver.
         * Because we register NIC feature with NETIF_F_IP_CSUM for MT5912B MAC, so
         * we'll process IP packet only.
         */
        if (skb->protocol != __constant_htons(ETH_P_IP)) {
            printk("Wrong skb->protocol( = %08x) for TX Checksum Offload.\n", skb->protocol);
        }
        else
#endif
        ucFlag |= (TX_CS_IP_GEN | TX_CS_TCP_UDP_GEN);
    }

    *pucFlag = ucFlag;

    return;
} /* kalQueryChksumOffloadParam */


//4 2007/10/8, mikewu, this is rewritten by Mike
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateRxCSUMOffloadParam (
    IN PVOID pvPacket,
    IN ENUM_CSUM_RESULT_T aeCSUM[]
    )
{
    struct sk_buff *skb = (struct sk_buff *)pvPacket;

    ASSERT(pvPacket);

    if ( (aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_SUCCESS || aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_SUCCESS)&&
        ( (aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_SUCCESS) || (aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_SUCCESS)) ) {
        skb->ip_summed = CHECKSUM_UNNECESSARY;
    }
    else {
        skb->ip_summed = CHECKSUM_NONE;
#if DBG
        if (aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_NONE && aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_NONE) {
            DBGLOG(RX, TRACE, ("RX: \"non-IPv4/IPv6\" Packet\n"));
        }
        else if (aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_FAILED) {
            DBGLOG(RX, TRACE, ("RX: \"bad IP Checksum\" Packet\n"));
        }
        else if (aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_FAILED) {
            DBGLOG(RX, TRACE, ("RX: \"bad TCP Checksum\" Packet\n"));
        }
        else if (aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_FAILED) {
            DBGLOG(RX, TRACE, ("RX: \"bad UDP Checksum\" Packet\n"));
        }
        else {

        }
#endif
    }

} /* kalUpdateRxCSUMOffloadParam */
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalPacketFree(
    IN P_GLUE_INFO_T   prGlueInfo,
    IN PVOID           pvPacket
    )
{
    dev_kfree_skb((struct sk_buff *)pvPacket);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
PVOID
kalPacketAlloc (
    IN P_GLUE_INFO_T    prGlueInfo,
    IN UINT_32          u4Size,
    OUT PUINT_8         *ppucData
    )
{
    struct sk_buff  *prSkb = dev_alloc_skb(u4Size);

    if (prSkb) {
        *ppucData = (PUINT_8)(prSkb->data);
    }
#if DBG
{
    PUINT_32 pu4Head = (PUINT_32)&prSkb->cb[0];
    *pu4Head = (UINT_32)prSkb->head;
    DBGLOG(RX, TRACE, ("prSkb->head = %#lx, prSkb->cb = %#lx\n", (UINT_32)prSkb->head, *pu4Head));
}
#endif
    return (PVOID)prSkb;
}


#if CFG_USE_SW_ROOT_TIMER
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalOsTimerInitialize (
    IN P_GLUE_INFO_T prGlueInfo,
    IN PVOID         prTimerHandler
    )
{
    struct timer_list *prTimer = &prGlueInfo->rMasterTimer;

    /* Setup master timer. This master timer is the root timer for following
       management timers. */

    init_timer(prTimer);
    prTimer->function = (PFN_LINUX_TIMER_FUNC)prTimerHandler;
    prTimer->data = (unsigned long)(prGlueInfo->prAdapter);

    INITLOG(("Init SW master Timer -- OK\n"));
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
kalSetTimer (
    IN P_GLUE_INFO_T prGlueInfo,
    IN OS_SYSTIME    rInterval
    )
{
    struct timer_list *prTimer = &prGlueInfo->rMasterTimer;

    del_timer(prTimer);

    prTimer->expires = jiffies + rInterval;

    add_timer(prTimer);

    return TRUE; /* success */
} /* end of kalSetTimer() */
#endif /* CFG_USE_SW_ROOT_TIMER */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalProcessRxPacket (
    IN P_GLUE_INFO_T      prGlueInfo,
    IN PVOID              pvPacket,
    IN PUINT_8            pucPacketStart,
    IN UINT_32            u4PacketLen,
    IN PBOOLEAN           pfgIsRetain,
    IN ENUM_CSUM_RESULT_T aerCSUM[]
    )
{
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    struct sk_buff *skb = (struct sk_buff *)pvPacket;

    skb->data = (unsigned char *)pucPacketStart;
    skb->tail = (unsigned char *)((UINT_32)pucPacketStart + u4PacketLen);
    skb->len = (unsigned int)u4PacketLen;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
    kalUpdateRxCSUMOffloadParam(skb, aerCSUM);
#endif
    *pfgIsRetain = FALSE;

    return rStatus;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalRxIndicatePkts (
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PVOID            apvPkts[],
    IN UINT_8           ucPktNum
    )
{
    UINT_8 ucIdx = 0;
    struct net_device *prNetDev = prGlueInfo->prDevHandler;
    struct sk_buff *prSkb = NULL;

    ASSERT(prGlueInfo);
    ASSERT(apvPkts);

    for(ucIdx = 0; ucIdx < ucPktNum; ucIdx++) {
        prSkb = apvPkts[ucIdx];
#if DBG
        do {
            PUINT_8 pu4Head = (PUINT_8)&prSkb->cb[0];
            UINT_32 u4HeadValue = 0;
            kalMemCopy(&u4HeadValue, pu4Head, sizeof(u4HeadValue));
            DBGLOG(RX, TRACE, ("prSkb->head = 0x%p, prSkb->cb = 0x%lx\n", pu4Head, u4HeadValue));
        } while (0);
#endif

        prNetDev->last_rx = jiffies;
        prSkb->protocol = eth_type_trans(prSkb, prNetDev);
        prSkb->dev = prNetDev;
        DBGLOG_MEM8(RX, TRACE, (PUINT_32)prSkb->data, prSkb->len);

        netif_rx(prSkb);
        wlanReturnPacket(prGlueInfo->prAdapter, NULL);
    }

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalIndicateStatusAndComplete (
    IN P_GLUE_INFO_T    prGlueInfo,
    IN WLAN_STATUS      eStatus,
    IN PVOID            pvBuf,
    IN UINT_32          u4BufLen
    )
{
    UINT_32 bufLen;
    P_PARAM_STATUS_INDICATION_T pStatus = (P_PARAM_STATUS_INDICATION_T)pvBuf;
    P_PARAM_AUTH_EVENT_T pAuth = (P_PARAM_AUTH_EVENT_T)pStatus;
    P_PARAM_PMKID_CANDIDATE_LIST_T pPmkid =
        (P_PARAM_PMKID_CANDIDATE_LIST_T)(pStatus + 1);
    PARAM_MAC_ADDRESS arBssid;

    ASSERT(prGlueInfo);


    switch (eStatus) {
    case WLAN_STATUS_MEDIA_CONNECT:
        /* switch netif on */
        netif_carrier_on(prGlueInfo->prDevHandler);
		msleep(50);
        /* indicate assoc event */
        wlanQueryInformation(prGlueInfo->prAdapter,
            wlanoidQueryBssid,
            &arBssid[0],
            sizeof(arBssid),
            &bufLen);
        wext_indicate_wext_event(prGlueInfo, SIOCGIWAP, arBssid, bufLen);

        prGlueInfo->eParamMediaStateIndicated = PARAM_MEDIA_STATE_CONNECTED;

        do {
            /* print message on console */
            PARAM_SSID_T ssid;
            wlanQueryInformation(prGlueInfo->prAdapter,
                wlanoidQuerySsid,
                &ssid,
                sizeof(ssid),
                &bufLen);
            ssid.aucSsid[(ssid.u4SsidLen > PARAM_MAX_LEN_SSID) ?
                PARAM_MAX_LEN_SSID : ssid.u4SsidLen ] = '\0';
            printk(KERN_INFO "[wifi] %s netif_carrier_on [ssid:%s " MACSTR "]\n",
                prGlueInfo->prDevHandler->name,
                ssid.aucSsid,
                MAC2STR(arBssid));
        } while(0);
        break;

    case WLAN_STATUS_MEDIA_DISCONNECT:
        /* indicate disassoc event */
        wext_indicate_wext_event(prGlueInfo, SIOCGIWAP, NULL, 0);
        /* For CR 90 and CR99, While supplicant do reassociate, driver will do netif_carrier_off first,
           after associated success, at joinComplete(), do netif_carier_on,
           but for unknown reason, the supplicant 1x pkt will not called the driver
           hardStartXmit, for template workaround these bugs, add this compiling flag
        */
        /* switch netif off */
        netif_carrier_off(prGlueInfo->prDevHandler);

#if 1   /* CONSOLE_MESSAGE */
        printk(KERN_INFO "[wifi] %s netif_carrier_off\n", prGlueInfo->prDevHandler->name);
#endif

        prGlueInfo->eParamMediaStateIndicated = PARAM_MEDIA_STATE_DISCONNECTED;

        break;

    case WLAN_STATUS_SCAN_COMPLETE:
        /* indicate scan complete event */
        wext_indicate_wext_event(prGlueInfo, SIOCGIWSCAN, NULL, 0);
        break;

    case WLAN_STATUS_MSDU_OK:
        if (netif_running(prGlueInfo->prDevHandler)) {
            netif_wake_queue(prGlueInfo->prDevHandler);
        }
        break;

    case WLAN_STATUS_MEDIA_SPECIFIC_INDICATION:
        if (pStatus) {
            switch (pStatus->eStatusType) {
            case ENUM_STATUS_TYPE_AUTHENTICATION:
                /*
                printk(KERN_NOTICE "ENUM_STATUS_TYPE_AUTHENTICATION: L(%ld) [" MACSTR "] F:%lx\n",
                    pAuth->Request[0].Length,
                    MAC2STR(pAuth->Request[0].Bssid),
                    pAuth->Request[0].Flags);
                */
                /* indicate (UC/GC) MIC ERROR event only */
                if ((pAuth->arRequest[0].u4Flags ==
                        PARAM_AUTH_REQUEST_PAIRWISE_ERROR) ||
                        (pAuth->arRequest[0].u4Flags ==
                        PARAM_AUTH_REQUEST_GROUP_ERROR)) {
                    wext_indicate_wext_event(prGlueInfo,
                        IWEVMICHAELMICFAILURE,
                        (unsigned char *)&pAuth->arRequest[0],
                        pAuth->arRequest[0].u4Length);
                }
                break;

            case ENUM_STATUS_TYPE_CANDIDATE_LIST:
                /*
                printk(KERN_NOTICE "Param_StatusType_PMKID_CandidateList: Ver(%ld) Num(%ld)\n",
                    pPmkid->u2Version,
                    pPmkid->u4NumCandidates);
                if (pPmkid->u4NumCandidates > 0) {
                    printk(KERN_NOTICE "candidate[" MACSTR "] preAuth Flag:%lx\n",
                        MAC2STR(pPmkid->arCandidateList[0].rBSSID),
                        pPmkid->arCandidateList[0].fgFlags);
                }
                */
                {
                 UINT_32  i = 0;

                 P_PARAM_PMKID_CANDIDATE_T prPmkidCand = (P_PARAM_PMKID_CANDIDATE_T)&pPmkid->arCandidateList[0];

                 for (i=0; i<pPmkid->u4NumCandidates; i++) {
                    wext_indicate_wext_event(prGlueInfo,
                        IWEVPMKIDCAND,
                        (unsigned char *)&pPmkid->arCandidateList[i],
                        pPmkid->u4NumCandidates);
                    prPmkidCand += sizeof(PARAM_PMKID_CANDIDATE_T);
                }
                }
                break;

            default:
                /* case ENUM_STATUS_TYPE_MEDIA_STREAM_MODE */
                /*
                printk(KERN_NOTICE "unknown media specific indication type:%x\n",
                    pStatus->StatusType);
                */
                break;
            }
        }
        else {
            /*
            printk(KERN_WARNING "media specific indication buffer NULL\n");
            */
        }
        break;

    default:
        /*
        printk(KERN_WARNING "unknown indication:%lx\n", eStatus);
        */
        break;
    }
} /* kalIndicateStatusAndComplete */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateReAssocReqInfo (
    IN P_GLUE_INFO_T    prGlueInfo,
    IN PUINT_8          pucFrameBody,
    IN UINT_32          u4FrameBodyLen,
    IN BOOLEAN          fgReassocRequest
    )
{
    PUINT_8             cp;

    if (fgReassocRequest) {
        if (u4FrameBodyLen < 15) {
            /*
            printk(KERN_WARNING "frameBodyLen too short:%ld\n", frameBodyLen);
            */
            return;
        }
    }
    else {
        if (u4FrameBodyLen < 9) {
            /*
            printk(KERN_WARNING "frameBodyLen too short:%ld\n", frameBodyLen);
            */
            return;
        }
    }

    cp = pucFrameBody;

    if (fgReassocRequest) {
        /* Capability information field 2 */
        /* Listen interval field 2*/
        /* Current AP address 6 */
        cp += 10;
        u4FrameBodyLen -= 10;
    }
    else {
        /* Capability information field 2 */
        /* Listen interval field 2*/
        cp += 4;
        u4FrameBodyLen -= 4;
    }

    wext_indicate_wext_event(prGlueInfo, IWEVASSOCREQIE, cp, u4FrameBodyLen);
    return;
}



#if CFG_TX_FRAGMENT
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalQueryTxPacketHeader (
    IN P_GLUE_INFO_T prGlueInfo,
    IN PVOID         pvPacket,
    OUT PUINT_16     pu2EtherTypeLen,
    OUT PUINT_8      pucEthDestAddr
    )
{
    struct sk_buff *prSkb = (struct sk_buff *)pvPacket;

    *pu2EtherTypeLen = (UINT_16)ntohs(prSkb->protocol);
    memcpy(pucEthDestAddr, prSkb->data, PARAM_MAC_ADDR_LEN);

    return TRUE;
}
#endif /* CFG_TX_FRAGMENT */


#if CFG_TX_DBG_PACKET_ORDER
UINT_32 u4PacketIDOut[16] = {0};
#endif /* CFG_TX_DBG_PACKET_ORDER */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalSendCompleteAndAwakeQueue (
    IN P_GLUE_INFO_T prGlueInfo,
    IN PVOID pvPacket
    )
{
    ASSERT(pvPacket);
    ASSERT(prGlueInfo->u4TxPendingFrameNum);

#if CFG_TX_DBG_PACKET_ORDER
    {
        struct sk_buff *skb = (struct sk_buff *)pvPacket;
        UINT_8 ucTID = skb->cb[4];
        UINT_32 u4IDIn = *(PUINT_32)&skb->cb[12];

        ASSERT(ucTID <= 15);

        if (u4IDIn == u4PacketIDOut[ucTID]) {
            u4PacketIDOut[ucTID]++;
        }
        else {
            ASSERT(0);
        }
    }
#endif /* CFG_TX_DBG_PACKET_ORDER */

    dev_kfree_skb((struct sk_buff *)pvPacket);

    prGlueInfo->u4TxPendingFrameNum--;
    if (prGlueInfo->u4TxPendingFrameNum < CFG_TX_STOP_NETIF_QUEUE_THRESHOLD) {
        netif_wake_queue(prGlueInfo->prDevHandler);
    }

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
kalQueryRegistryMacAddr (
    IN  P_GLUE_INFO_T   prGlueInfo,
    OUT PUINT_8        paucMacAddr
    )
{
    UINT_8 aucZeroMac[MAC_ADDR_LEN] = {0,0,0,0,0,0}
    DEBUGFUNC("kalQueryRegistryMacAddr");

    ASSERT(prGlueInfo);
    ASSERT(paucMacAddr);

    kalMemCopy((PVOID) paucMacAddr, (PVOID)aucZeroMac, MAC_ADDR_LEN);

    return;
} /* end of kalQueryRegistryMacAddr() */

#if CFG_SUPPORT_EXT_CONFIG
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_32
kalReadExtCfg (
    IN P_GLUE_INFO_T prGlueInfo
    )
{
#if DBG
    int i;
#endif
    ASSERT(prGlueInfo);

    printk("[MT5921][kalReadExtCfg]NVRAM size %lu\n", gPlatformCfg.u4Cfglen);

    if( gPlatformCfg.u4Cfglen < sizeof(prGlueInfo->au2ExtCfg) ||
		sizeof(gPlatformCfg.rWifiNvram) < sizeof(prGlueInfo->au2ExtCfg) ) {
		prGlueInfo->u4ExtCfgLength = 0;
		return 0;
    }

    kalMemCopy( prGlueInfo->au2ExtCfg, (char *)&gPlatformCfg.rWifiNvram, sizeof(prGlueInfo->au2ExtCfg) );
#if DBG
    for( i = 0; i< sizeof(prGlueInfo->au2ExtCfg)/2; i++){
        printk("[MT5921][kalReadExtCfg]nvram cfg data[%d] 0x%x\n", i, prGlueInfo->au2ExtCfg[i]);
    }
#endif

    prGlueInfo->u4ExtCfgLength = sizeof(prGlueInfo->au2ExtCfg);

    return prGlueInfo->u4ExtCfgLength;
}

#endif

