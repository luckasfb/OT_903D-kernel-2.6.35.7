






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxInitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;
    PUINT_8 pucMemHandle;
    P_SW_RFB_T prRfb = (P_SW_RFB_T)NULL;
    INT_32 i;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    //4 <0> Clear allocated memory.
    kalMemZero((PVOID) prRxCtrl->pucRxCached, prRxCtrl->u4RxCachedSize);

    //4 <1> Initialize the RFB lists
    QUEUE_INITIALIZE(&prRxCtrl->rFreeRFBWithBuffList);
    QUEUE_INITIALIZE(&prRxCtrl->rFreeRFBWOBuffList);
    QUEUE_INITIALIZE(&prRxCtrl->rReceivedRFBList);

    pucMemHandle = prRxCtrl->pucRxCached;
    for (i = CFG_RX_MAX_PKT_NUM; i != 0; i--) {
        prRfb = (P_SW_RFB_T)pucMemHandle;

        nicRxSetupRFB(prAdapter, prRfb);

        nicRxReturnRFB(prAdapter, prRfb);

        pucMemHandle += ALIGN_4(sizeof(SW_RFB_T));
    }

    ASSERT(prRxCtrl->rFreeRFBWithBuffList.u4NumElem == CFG_RX_MAX_PKT_NUM);

    /* Check if the memory allocation consist with this initialization function */
    ASSERT((UINT_32)(pucMemHandle - prRxCtrl->pucRxCached) == prRxCtrl->u4RxCachedSize);


    //4 <2> Initialization of RX Queue Status.
    prRxCtrl->fgIsDefragQueNonEmpty = FALSE;
    prRxCtrl->fgIsRxQueActive = FALSE;
    prRxCtrl->fgIsRxQueSuspend = FALSE;
    prRxCtrl->fgIsRfTestRxMode = FALSE;
#if (CFG_DBG_BEACON_RCPI_MEASUREMENT || CFG_LP_PATTERN_SEARCH_SLT)
    prRxCtrl->fgIsRxStatusG0 = TRUE;
    prRxCtrl->fgIsRxStatusG1 = TRUE;
    prRxCtrl->fgIsRxStatusG2 = TRUE;
    prRxCtrl->fgIsRxStatusG0Rssi = TRUE;
#else
    prRxCtrl->fgIsRxStatusG0 = FALSE;
    prRxCtrl->fgIsRxStatusG1 = FALSE;
    prRxCtrl->fgIsRxStatusG2 = FALSE;
    prRxCtrl->fgIsRxStatusG0Rssi = FALSE;
#endif

    //4 <3> Clear all RX counters
    RX_RESET_ALL_CNTS(prRxCtrl);


    //4 <4> Rx filter default value
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXSAMEBSSIDPRORESP);
    NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXDIFFBSSIDPRORESP);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXSAMEBSSIDBCN);
    NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXDIFFBSSIDBCN);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXNOACK);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_DROPVERSIONNOT0);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_DROPA3OWNSA);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_DROPDIFFBSSIDA3);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_DROPDIFFBSSIDA2);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXMCFRAME);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXBCFRAME);
    NIC_SET_RX_FILTER(prAdapter, RXFILTER_DROPFCS);
    NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXSAMEBSSIDNULL); //090202 juji
    NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXDIFFBSSIDNULL); //090202 juji

    return;
} /* end of nicRxInitialize() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxUninitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_SW_RFB_T prRfb = (P_SW_RFB_T)NULL;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (prRxCtrl) {

        do {

            QUEUE_REMOVE_HEAD(&prRxCtrl->rReceivedRFBList, prRfb, P_SW_RFB_T);
            if (!prRfb) {
                break;
            }

            if (prRfb->pvPacket) {
                kalPacketFree(prAdapter->prGlueInfo, prRfb->pvPacket);
            }
            prRfb->pvPacket= NULL;
        }
        while (TRUE);

        do {

            QUEUE_REMOVE_HEAD(&prRxCtrl->rFreeRFBWithBuffList, prRfb, P_SW_RFB_T);
            if (!prRfb) {
                break;
            }

            if (prRfb->pvPacket) {
                kalPacketFree(prAdapter->prGlueInfo, prRfb->pvPacket);
            }
            prRfb->pvPacket= NULL;
        }
        while (TRUE);

    }

    return;
} /* end of nicRxUninitialize() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxProcessRFBs (
    IN  P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_QUE_T prReceivedRFBList;
    P_SW_RFB_T prSWRfb = NULL;
    UINT_32 u4CurrentRxBufferCount;
    BOOLEAN fgIsRetained = FALSE;

    DEBUGFUNC("nicRxProcessRFBs");


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;
    prReceivedRFBList = &prRxCtrl->rReceivedRFBList;

    prRxCtrl->ucNumIndPacket = 0;
    u4CurrentRxBufferCount = prRxCtrl->rFreeRFBWithBuffList.u4NumElem +
                             prReceivedRFBList->u4NumElem;

    //4 <1> Remove SW_RFB from rReceivedRFBList Queue
    do {

        QUEUE_REMOVE_HEAD(prReceivedRFBList, prSWRfb, P_SW_RFB_T);
        if (!prSWRfb) {
            break;
        }

        if (prRxCtrl->fgIsRfTestRxMode) {
            rftestProcessRxMPDU(prAdapter, &prSWRfb);
            nicRxReturnRFB(prAdapter, prSWRfb);
            continue;
        }

        /* Process MPDU */
        if (rxProcessMPDU(prAdapter, &prSWRfb) != WLAN_STATUS_SUCCESS) {

            DBGLOG(RX, TRACE, ("Process MPDU error\n"));

            if (prSWRfb) {
                nicRxReturnRFB(prAdapter, prSWRfb);
            }
            continue;
        }

        /* Process MSDU */
        if (rxProcessMSDU(prAdapter, prSWRfb)) {
            DBGLOG(RX, TRACE, ("Process MSDU error\n"));

            nicRxReturnRFB(prAdapter, prSWRfb);
            continue;
        }

        if (prSWRfb->fgIsDataFrame &&
            (prAdapter->eConnectionStateIndicated == MEDIA_STATE_CONNECTED)) {

            if (rxProcessDataFrame(prAdapter, prSWRfb)) { /*rxProcessDataFrame failed*/

                nicRxReturnRFB(prAdapter, prSWRfb);
            }
            else {/*rxProcessDataFrame success*/

                #if defined (WINDOWS_DDK) /* CR1088 */
                fgIsRetained = TRUE;
                #else
                fgIsRetained = ((u4CurrentRxBufferCount < CFG_RX_MGMT_PKT_NUM_THRESHOLD) ?
                                TRUE : FALSE);
                #endif

                if (kalProcessRxPacket(prAdapter->prGlueInfo,
                                       prSWRfb->pvPacket,
                                       prSWRfb->pvHeader,
                                       prSWRfb->u2FrameLength,
                                       &fgIsRetained, prSWRfb->aeCSUM) != WLAN_STATUS_SUCCESS) {
                    DBGLOG(NIC, ERROR, ("kalProcessRxPacket return value != WLAN_STATUS_SUCCESS\n"));
                    ASSERT(0);

                    nicRxReturnRFB(prAdapter, prSWRfb);
                    continue;
                }
                else {
                    UINT_64 u8FrameLength;


                    u8FrameLength = (UINT_64)prSWRfb->u2FrameLength;

                    RX_ADD_CNT(prRxCtrl, RX_MSDU_BYTES_COUNT, u8FrameLength);


                    prRxCtrl->apvIndPacket[prRxCtrl->ucNumIndPacket++] = prSWRfb->pvPacket;
                }

                if (fgIsRetained) {
                    /* TODO : error handling of nicRxSetupRFB */
                    nicRxSetupRFB(prAdapter, prSWRfb);
                    nicRxReturnRFB(prAdapter, prSWRfb);
                }
                else{
                    u4CurrentRxBufferCount--;

                    prSWRfb->pvPacket = NULL;
                    nicRxReturnRFB(prAdapter, prSWRfb);
                }
            }

        }
        else {
            rxProcessMgmtFrame(prAdapter, prSWRfb);
            nicRxReturnRFB(prAdapter, prSWRfb);
        }

    }
    while (TRUE);

    if (prRxCtrl->ucNumIndPacket > 0) {
        DBGLOG(RX, TRACE, ("%d packets will be indicated\n", prRxCtrl->ucNumIndPacket));

        RX_ADD_CNT(prRxCtrl, RX_DATA_INDICATION_COUNT, prRxCtrl->ucNumIndPacket);

        if (kalRxIndicatePkts(prAdapter->prGlueInfo,
                              prRxCtrl->apvIndPacket,
                              prRxCtrl->ucNumIndPacket)) {/* Deal with the current RFB */
            DBGLOG(RX, TRACE, ("Indicate MSDUs error\n"));
        }
    }

    return;

} /* end of nicRxProcessRFBs() */


#if !CFG_SDIO_STATUS_ENHANCE
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxReceiveRFBs (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_QUE_T prFreeRFBWithBuffList;
    P_QUE_T prReceivedRFBList;
    P_SW_RFB_T prSWRfb = NULL;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;
    prFreeRFBWithBuffList = &prRxCtrl->rFreeRFBWithBuffList;
    prReceivedRFBList = &prRxCtrl->rReceivedRFBList;

    do {

        QUEUE_REMOVE_HEAD(prFreeRFBWithBuffList, prSWRfb, P_SW_RFB_T);
        /* The "RFB with Buffer" list is empty */
        if (!prSWRfb) {
            P_QUE_T prFreeRFBWOBuffList = &prRxCtrl->rFreeRFBWOBuffList;
            P_SW_RFB_T prSWRfbWOBuff;

            do {
                /* Try to get the RFB from "RFB without Buffer" list */
                QUEUE_REMOVE_HEAD(prFreeRFBWOBuffList, prSWRfbWOBuff, P_SW_RFB_T);
                if (!prSWRfbWOBuff) {
                    break;
                }

                if (nicRxSetupRFB(prAdapter, prSWRfbWOBuff) != WLAN_STATUS_SUCCESS) {
                    QUEUE_INSERT_HEAD(prFreeRFBWOBuffList, &prSWRfbWOBuff->rQueEntry);
                    break;
                }
                else {
                    QUEUE_INSERT_HEAD(prFreeRFBWithBuffList, &prSWRfbWOBuff->rQueEntry);
                }
            }
            while (TRUE);

            QUEUE_REMOVE_HEAD(prFreeRFBWithBuffList, prSWRfb, P_SW_RFB_T);
            /* The "RFB with Buffer" list is still empty ! */
            if (!prSWRfb) {
                DBGLOG(RX, TRACE, ("No More RFB\n"));
                break; /* Break outter do {} while loop */
            }
        }

        ASSERT(prSWRfb);

        if (halRxFillRFB(prAdapter, prSWRfb)) {
            DBGLOG(RX, TRACE, ("halRxFillRFB failed\n"));
            nicRxReturnRFB(prAdapter, prSWRfb);
            break;
        }

        QUEUE_INSERT_TAIL(prReceivedRFBList, &prSWRfb->rQueEntry);

        RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);

    }
    while (HAL_IS_MORE_RX_PKT(prSWRfb->prRxStatus));

    return;

} /* end of nicReceiveRFBs() */

#else

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxSDIOReceiveRFBs (
    IN  P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_QUE_T prFreeRFBWithBuffList;
    P_QUE_T prReceivedRFBList;
    P_SW_RFB_T prSWRfb = (P_SW_RFB_T)NULL;
    P_SDIO_CTRL_T prSDIOCtrl;
    UINT_32 i;

    DEBUGFUNC("nicRxSDIOReceiveRFBs");


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;
    prFreeRFBWithBuffList = &prRxCtrl->rFreeRFBWithBuffList;
    prReceivedRFBList = &prRxCtrl->rReceivedRFBList;
    prSDIOCtrl = &prAdapter->rSDIOCtrl;

    for (i = 0; i < SDIO_MAXIMUM_RX_STATUS; i++) {

        if (!(prSDIOCtrl->au2RxLengthDW[i])) {
            break;
        }

        QUEUE_REMOVE_HEAD(prFreeRFBWithBuffList, prSWRfb, P_SW_RFB_T);
        /* The "RFB with Buffer" list is empty */
        if (!prSWRfb) {
            P_QUE_T prFreeRFBWOBuffList = &prRxCtrl->rFreeRFBWOBuffList;
            P_SW_RFB_T prSWRfbWOBuff;

            do {
                /* Try to get the RFB from "RFB without Buffer" list */
                QUEUE_REMOVE_HEAD(prFreeRFBWOBuffList, prSWRfbWOBuff, P_SW_RFB_T);
                if (!prSWRfbWOBuff) {
                    break;
                }

                if (nicRxSetupRFB(prAdapter, prSWRfbWOBuff) != WLAN_STATUS_SUCCESS) {
                    QUEUE_INSERT_HEAD(prFreeRFBWOBuffList, &prSWRfbWOBuff->rQueEntry);
                    break;
                }
                else {
                    QUEUE_INSERT_HEAD(prFreeRFBWithBuffList, &prSWRfbWOBuff->rQueEntry);
                }
            }
            while (TRUE);

            QUEUE_REMOVE_HEAD(prFreeRFBWithBuffList, prSWRfb, P_SW_RFB_T);
            /* The "RFB with Buffer" list is still empty ! */
            if (!prSWRfb) {
                DBGLOG(RX, TRACE, ("No More RFB\n"));
                break; /* Break outter for loop */
            }
        }

        ASSERT(prSWRfb);

        if (halSDIORxFillRFB(prAdapter, prSDIOCtrl->au2RxLengthDW[i], prSWRfb) == WLAN_STATUS_FAILURE) {
            DBGLOG(RX, TRACE, ("halRxFillRFB failed\n"));
            nicRxReturnRFB(prAdapter, prSWRfb);
            break;
        }

        //Clear RX STATUS in STATUS Buffer.
        prSDIOCtrl->au2RxLengthDW[i] = 0;

        QUEUE_INSERT_TAIL(prReceivedRFBList, &prSWRfb->rQueEntry);

        RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);
    }

    return;
}/* end of nicRxSDIOReceiveRFBs() */

#endif /* CFG_SDIO_STATUS_ENHANCE */

#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicRxSetupRFB (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prRfb
    )
{
    P_GLUE_INFO_T prGlueInfo;


    ASSERT(prAdapter);
    ASSERT(prRfb);

    prGlueInfo = prAdapter->prGlueInfo;

    kalMemZero(((PUINT_8)prRfb + OFFSET_OF(SW_RFB_T, prRxStatus)),
                   (sizeof(SW_RFB_T)-OFFSET_OF(SW_RFB_T, prRxStatus)));


    if (prRfb->pucRecvBuff == NULL) {
        if (prGlueInfo->u4PayloadPoolSz >= CFG_RX_MAX_PKT_SIZE) {
            prGlueInfo->u4PayloadPoolSz -= CFG_RX_MAX_PKT_SIZE;
            prRfb->pucRecvBuff = prGlueInfo->pucPayloadPool;
            prGlueInfo->pucPayloadPool += CFG_RX_MAX_PKT_SIZE;
        }
    }

    prRfb->prRxStatus = (P_RX_STATUS_T)(prRfb->pucRecvBuff);

    return WLAN_STATUS_SUCCESS;

} /* end of nicRxSetupRFB() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxReturnRFB (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prRfb
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_QUE_ENTRY_T prQueEntry;
     P_QUE_T prFreeRFBWithBuffList;


    ASSERT(prAdapter);
    ASSERT(prRfb);
    prRxCtrl = &prAdapter->rRxCtrl;
    prQueEntry = &prRfb->rQueEntry;

    /* The processing on this RFB is done, so put it back on the tail of
       our list */
    prFreeRFBWithBuffList = &prRxCtrl->rFreeRFBWithBuffList;

    QUEUE_INSERT_TAIL(prFreeRFBWithBuffList, prQueEntry);


    return;
} /* end of nicRxReturnRFB() */


#else


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicRxSetupRFB (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prRfb
    )
{
    PVOID   pvPacket;
    PUINT_8 pucRecvBuff;


    ASSERT(prAdapter);
    ASSERT(prRfb);

    if (!prRfb->pvPacket) {
        kalMemZero(prRfb, sizeof(SW_RFB_T));

        pvPacket = kalPacketAlloc(prAdapter->prGlueInfo,
                                  CFG_RX_MAX_PKT_SIZE,
                                  &pucRecvBuff);

        if (pvPacket == NULL) {
            return WLAN_STATUS_RESOURCES;
        }

        prRfb->pvPacket = pvPacket;
        //4 2006/10/17, mikewu, use pvRecvBuff instead of receiveBuffer_p

        ASSERT(IS_ALIGN_4((UINT_32)pucRecvBuff));

        prRfb->pucRecvBuff= pucRecvBuff;
    }
    else {
        kalMemZero(((PUINT_8)prRfb + OFFSET_OF(SW_RFB_T, prRxStatus)),
                   (sizeof(SW_RFB_T)-OFFSET_OF(SW_RFB_T, prRxStatus)));
    }

    prRfb->prRxStatus = (P_RX_STATUS_T)(prRfb->pucRecvBuff);

    return WLAN_STATUS_SUCCESS;

} /* end of nicRxSetupRFB() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxReturnRFB (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prRfb
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_QUE_ENTRY_T prQueEntry;


    ASSERT(prAdapter);
    ASSERT(prRfb);
    prRxCtrl = &prAdapter->rRxCtrl;
    prQueEntry = &prRfb->rQueEntry;

    /* The processing on this RFB is done, so put it back on the tail of
       our list */
    if (prRfb->pvPacket) {
        P_QUE_T prFreeRFBWithBuffList = &prRxCtrl->rFreeRFBWithBuffList;

        QUEUE_INSERT_TAIL(prFreeRFBWithBuffList, prQueEntry);
    }
    else {
        P_QUE_T prFreeRFBWOBuffList = &prRxCtrl->rFreeRFBWOBuffList;

        QUEUE_INSERT_TAIL(prFreeRFBWOBuffList, prQueEntry);
    }

    return;
} /* end of nicRxReturnRFB() */

#endif /* CFG_SDIO_DEVICE_DRIVER_WO_NDIS */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessRxInterrupt (
    IN  P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (prRxCtrl->fgIsDefragQueNonEmpty) {
        staRecCheckDefragBufOfAllStaRecords(prAdapter);
    }

    //DBGLOG(RX, TRACE, ("Enter RX\n"));

    //4 2007/06/20, collect RFB into RxRoughQueue
#if !CFG_SDIO_STATUS_ENHANCE
    nicRxReceiveRFBs(prAdapter);
#else
    nicRxSDIOReceiveRFBs(prAdapter);
#endif /* CFG_SDIO_STATUS_ENHANCE */

    nicRxProcessRFBs(prAdapter);

    return;

} /* end of nicProcessRxInterrupt() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxStartQueue (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    /* Start it only when it was disabled. */
    if (!prRxCtrl->fgIsRxQueActive) {
        prRxCtrl->fgIsRxQueActive = TRUE;

        halStartQueues(prAdapter, BIT(RXQ));
    }

    return;
} /* end of nicRxStartQueue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxFlushStopQueue (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsFlushRxQue,
    IN BOOLEAN fgIsStopRxQue
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (fgIsFlushRxQue || fgIsStopRxQue) {

        if (fgIsStopRxQue) {
            prRxCtrl->fgIsRxQueActive = FALSE;
        }

        halFlushStopQueues(prAdapter,
            (fgIsFlushRxQue ? BIT(RXQ) : 0),
            (fgIsStopRxQue ? BIT(RXQ) : 0));

#if CFG_SDIO_STATUS_ENHANCE
        if (fgIsFlushRxQue) {
            P_SDIO_CTRL_T prSDIOCtrl = &prAdapter->rSDIOCtrl;

            kalMemZero(prSDIOCtrl->au2RxLengthDW, sizeof(prSDIOCtrl->au2RxLengthDW));
        }
#endif /* CFG_SDIO_STATUS_ENHANCE */

    }

    return;
} /* end of nicRxFlushStopQueue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxAcquirePrivilegeOfRxQue (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsStartRxQue,
    IN BOOLEAN fgIsStopRxQue,
    IN BOOLEAN fgIsFlushRxQue
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    ASSERT(prRxCtrl->fgIsRxQueSuspend == (BOOLEAN)FALSE);

    /* Backup the flag of RX Queue's activity for resuming the RX after the Privilege was released. */
    prRxCtrl->fgIsRxQueSuspend = prRxCtrl->fgIsRxQueActive;

    /* NOTE(Kevin): For most cases, we'll not issue fgIsStartRxQue and fgIsStopRxQue
     * at the same time. But we won't exclude this condition that "Stop & Flush" firstly
     * before Start it if we need such combination one day.
     */

    /* Flush and stop the RX Queue first */
    nicRxFlushStopQueue(prAdapter, fgIsFlushRxQue, fgIsStopRxQue);

    /* Start the R Queue */
    if (fgIsStartRxQue) {
        nicRxStartQueue(prAdapter);
    }

    return;
} /* end of nicRxAcquirePrivilegeOfRxQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxReleasePrivilegeOfRxQue (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsFlushRxQue,
    IN BOOLEAN fgIsKeepRxActive
    )
{
    P_RX_CTRL_T prRxCtrl;
    BOOLEAN fgIsStartRxQue;
    BOOLEAN fgIsStopRxQue;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    /* fgIsKeepRxActive can set to TRUE if only if
     * 1. We know RX is enabled in current STATE.
     * 2. We need to let RX keep enabled in next STATE.
     */
    if (fgIsKeepRxActive) {
        prRxCtrl->fgIsRxQueSuspend = TRUE;
    }

    /* Restore the original activity of RX Queue after the Privilege was released. */
    fgIsStartRxQue = ((prRxCtrl->fgIsRxQueSuspend && (!prRxCtrl->fgIsRxQueActive)) ? TRUE : FALSE);
    fgIsStopRxQue = (((!prRxCtrl->fgIsRxQueSuspend) && prRxCtrl->fgIsRxQueActive) ? TRUE : FALSE);
    //fgIsStopRxQue = ((prRxCtrl->fgIsRxQueActive) ? TRUE : FALSE); // (force it to stop for SW/HW sync just in case)

    /* NOTE(Kevin 2007/12/7): Truth Table.
     * fgIsRxQueSuspend fgIsRxQueActive fgIsStartRxQue fgIsStopRxQue
     * TRUE             FALSE           TRUE           FALSE
     * FALSE            TRUE            FALSE          TRUE
     * TRUE             TRUE            FALSE          FALSE//-->TRUE(force it to stop for SW/HW sync)
     * FALSE            FALSE           FALSE          FALSE
     */

    /* Clear the flag of Suspend RX Queue. */
    prRxCtrl->fgIsRxQueSuspend = FALSE;

    /* Flush and stop the RX Queue first */
    nicRxFlushStopQueue(prAdapter, fgIsFlushRxQue, fgIsStopRxQue);

    /* Start the RX Queue */
    if (fgIsStartRxQue) {
        nicRxStartQueue(prAdapter);
    }

    return;
} /* end of nicRxReleasePrivilegeOfRxQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    P_RX_CTRL_T prRxCtrl;
    PUINT_8 pucCurrBuf = pucBuffer;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;
    if (pucBuffer) {} /* For Windows, we'll print directly instead of sprintf() */
    ASSERT(pu4Count);

    SPRINTF(pucCurrBuf, ("\n\nRX CTRL STATUS:"));
    SPRINTF(pucCurrBuf, ("\n==============="));
    SPRINTF(pucCurrBuf, ("\nFREE RFB w/i BUF LIST :%9ld", prRxCtrl->rFreeRFBWithBuffList.u4NumElem));
    SPRINTF(pucCurrBuf, ("\nFREE RFB w/o BUF LIST :%9ld", prRxCtrl->rFreeRFBWOBuffList.u4NumElem));
    SPRINTF(pucCurrBuf, ("\nRECEIVED RFB LIST     :%9ld", prRxCtrl->rReceivedRFBList.u4NumElem));

    SPRINTF(pucCurrBuf, ("\n\n"));

    *pu4Count = (UINT_32)((UINT_32)pucCurrBuf - (UINT_32)pucBuffer);

    return;
} /* end of nicRxQueryStatus() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxQueryStatistics (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    P_RX_CTRL_T prRxCtrl;
    PUINT_8 pucCurrBuf = pucBuffer;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;
    if (pucBuffer) {} /* For Windows, we'll print directly instead of sprintf() */
    ASSERT(pu4Count);

#define SPRINTF_RX_COUNTER(eCounter) \
    SPRINTF(pucCurrBuf, ("%-30s : %ld\n", #eCounter, (UINT_32)prRxCtrl->au8Statistics[eCounter]))


    SPRINTF_RX_COUNTER(RX_MPDU_TOTAL_COUNT);

    SPRINTF_RX_COUNTER(RX_DATA_FRAME_COUNT);
    SPRINTF_RX_COUNTER(RX_UCAST_DATA_FRAME_COUNT);
    SPRINTF_RX_COUNTER(RX_BMCAST_DATA_FRAME_COUNT);

    SPRINTF_RX_COUNTER(RX_MGMT_FRAME_COUNT);
    SPRINTF_RX_COUNTER(RX_UCAST_MGMT_FRAME_COUNT);
    SPRINTF_RX_COUNTER(RX_BMCAST_MGMT_FRAME_COUNT);

    SPRINTF_RX_COUNTER(RX_CTRL_FRAME_COUNT);

    SPRINTF_RX_COUNTER(RX_FIFO_FULL_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_SIZE_ERR_DROP_COUNT);

    SPRINTF_RX_COUNTER(RX_ERROR_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_FCS_ERR_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_FORMAT_ERR_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_ICV_ERR_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_KEY_ERR_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_TKIP_ERR_DROP_COUNT);

    SPRINTF_RX_COUNTER(RX_RETRY_FRAME_COUNT);
    SPRINTF_RX_COUNTER(RX_DUPLICATE_DROP_COUNT);

    SPRINTF_RX_COUNTER(RX_DATA_CLASS_ERR_DROP_COUNT);
    SPRINTF_RX_COUNTER(RX_DATA_PORT_CTRL_DROP_COUNT);

    SPRINTF_RX_COUNTER(RX_DATA_INDICATION_COUNT);

#if CFG_TCP_IP_CHKSUM_OFFLOAD
    SPRINTF_RX_COUNTER(RX_CSUM_TCP_FAILED_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_UDP_FAILED_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_IP_FAILED_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_TCP_SUCCESS_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_UDP_SUCCESS_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_IP_SUCCESS_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_UNKNOWN_L4_PKT_COUNT);
    SPRINTF_RX_COUNTER(RX_CSUM_UNKNOWN_L3_PKT_COUNT);
    SPRINTF_RX_COUNTER(RX_IP_V6_PKT_CCOUNT);
#endif

    SPRINTF_RX_COUNTER(RX_MSDU_BYTES_COUNT);

    *pu4Count = (UINT_32)((UINT_32)pucCurrBuf - (UINT_32)pucBuffer);

    return;
} /* end of nicRxQueryStatistics() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxSetStatistics (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    RX_RESET_ALL_CNTS(prRxCtrl);

    return;
} /* end of nicRxSetStatistics() */


#if CFG_TCP_IP_CHKSUM_OFFLOAD
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxUpdateCSUMStatistics (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_CSUM_RESULT_T aeCSUM[]
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    ASSERT(aeCSUM);
    prRxCtrl = &prAdapter->rRxCtrl;

    if ((aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_SUCCESS) ||
        (aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_SUCCESS)) {

        RX_INC_CNT(prRxCtrl, RX_CSUM_IP_SUCCESS_COUNT);
    }
    else if ((aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_FAILED) ||
             (aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_FAILED)) {

        RX_INC_CNT(prRxCtrl, RX_CSUM_IP_FAILED_COUNT);
    }
    else if ((aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_NONE) &&
             (aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_NONE)) {

        RX_INC_CNT(prRxCtrl, RX_CSUM_UNKNOWN_L3_PKT_COUNT);
    }
    else {
        ASSERT(0);
    }

    if (aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_SUCCESS) {
        RX_INC_CNT(prRxCtrl, RX_CSUM_TCP_SUCCESS_COUNT);
    }
    else if (aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_FAILED) {
        RX_INC_CNT(prRxCtrl, RX_CSUM_TCP_FAILED_COUNT);
    }
    else if (aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_SUCCESS) {
        RX_INC_CNT(prRxCtrl, RX_CSUM_UDP_SUCCESS_COUNT);
    }
    else if (aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_FAILED) {
        RX_INC_CNT(prRxCtrl, RX_CSUM_UDP_FAILED_COUNT);
    }
    else if ((aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_NONE) &&
             (aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_NONE)) {

        RX_INC_CNT(prRxCtrl, RX_CSUM_UNKNOWN_L4_PKT_COUNT);
    }
    else {
        ASSERT(0);
    }

    return;
} /* end of nicRxUpdateCSUMStatistics() */
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxEnablePromiscuousMode (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (!prRxCtrl->fgEnablePromiscuousMode) {
        nicRxFlushStopQueue(prAdapter, TRUE, TRUE);
        nicTxFlushStopQueues(prAdapter, TXQ_DATA_MASK | TXQ_MGMT_MASK, TXQ_DATA_MASK | TXQ_MGMT_MASK);
        halSetRxHeaderTranslation(prAdapter, FALSE);
        NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXPROMISCUOUSFRAME);
        prRxCtrl->fgEnablePromiscuousMode = TRUE;
        nicRxStartQueue(prAdapter);
    }

    return;
} /* end of nicRxEnablePromiscuousMode() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxDisablePromiscuousMode (
    IN P_ADAPTER_T prAdapter
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (prRxCtrl->fgEnablePromiscuousMode) {
        nicRxFlushStopQueue(prAdapter, TRUE, TRUE);

        NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXPROMISCUOUSFRAME);
        halSetRxHeaderTranslation(prAdapter, TRUE);

        prRxCtrl->fgEnablePromiscuousMode = FALSE;

        nicRxStartQueue(prAdapter);
    }

    return;
} /* end of nicRxDisablePromiscuousMode() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxSetMulticast (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_RX_MULTICAST_TYPE_T eMCType,
    IN PUINT_8 prMCAddrList,
    IN UINT_8 ucNum
    )
{
    P_RX_CTRL_T prRxCtrl;


    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (ucNum > 0) {
        ASSERT(prMCAddrList);
    }

    switch (eMCType) {
    case MC_TYPE_DENY_ALL:
        prRxCtrl->fgRxMulticastPkt = FALSE;
        prRxCtrl->fgRxMulticastPktByTable = FALSE;
        NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXMCFRAME);
        nicSetMulticastAddrList(prAdapter, (PUINT_8)NULL, 0);
        break;

    case MC_TYPE_ALLOW_LIST:
        prRxCtrl->fgRxMulticastPkt = TRUE;
        NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXMCFRAME);
        prRxCtrl->fgRxMulticastPktByTable = TRUE;
        NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_MCTABLENOCHK);
        break;

    case MC_TYPE_ALLOW_ALL:
        prRxCtrl->fgRxMulticastPkt = TRUE;
        NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXMCFRAME);
        prRxCtrl->fgRxMulticastPktByTable = FALSE;
        NIC_SET_RX_FILTER(prAdapter, RXFILTER_MCTABLENOCHK);
        /* We still have Multicast Address List */
        break;

    case MC_TYPE_UPDATE_LIST_ONLY:
        nicSetMulticastAddrList(prAdapter, prMCAddrList, ucNum);
        break;

    default:
        break;
    }

    return;
} /* end of nicRxSetMulticast() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRxSetBroadcast (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgEnableBroadcast
    )
{
    ASSERT(prAdapter);

    if (fgEnableBroadcast) {
        NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXBCFRAME);
    }
    else {
        NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXBCFRAME);
    }

    return;
} /* end of nicRxSetBroadcast() */


