






#include "precomp.h"

/* 6.1.1.2 Interpretation of priority parameter in MAC service primitives */
/* Static convert the Priority Parameter/TID(User Priority/TS Identifier) to Traffic Class */
const UINT_8 aucPriorityParam2TC[] = {
    TC1,
    TC0,
    TC0,
    TC1,
    TC2,
    TC2,
    TC3,
    TC3,
    TCS0,
    TCS0,
    TCS0,
    TCS0,
    TCS0,
    TCS0,
    TCS0,
    TCS0
};

static const UINT_8 aucTC2ACI[] = {
    AC_BK,      /* TC0(Index), SW Traffic Class 0 will map to  */
    AC_BE,      /* TC1 */
    AC_VI,      /* TC2 */
    AC_VO,      /* TC3 */
};

static const UINT_8 aucACI2TXQ[] = {
    TXQ_AC1,    /* AC_BE(Index) */
    TXQ_AC0,    /* AC_BK */
    TXQ_AC2,    /* AC_VI */
    TXQ_AC3     /* AC_VO */
};

static const UINT_8 aucTC2DefaultAdmittedTXQ[TXQ_NUM] = {
    TXQ_AC0,    /* TC0(Index) = aucACI2TXQ[ aucTC2ACI[TC0] ] */
    TXQ_AC1,    /* TC1 */
    TXQ_AC2,    /* TC2 */
    TXQ_AC3,    /* TC3 */
    TXQ_TS0,    /* TS0 */
    TXQ_TCM     /* TCM, if ucTC(Index) == TCM, assign this MSDU_INFO_T to this TXQ */
};

#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugRateIndex[] = {
    DISP_STRING("RATE_1M_INDEX"),
    DISP_STRING("RATE_2M_INDEX"),
    DISP_STRING("RATE_5_5M_INDEX"),
    DISP_STRING("RATE_11M_INDEX"),
    DISP_STRING("RATE_22M_INDEX"),
    DISP_STRING("RATE_33M_INDEX"),
    DISP_STRING("RATE_6M_INDEX"),
    DISP_STRING("RATE_9M_INDEX"),
    DISP_STRING("RATE_12M_INDEX"),
    DISP_STRING("RATE_18M_INDEX"),
    DISP_STRING("RATE_24M_INDEX"),
    DISP_STRING("RATE_36M_INDEX"),
    DISP_STRING("RATE_48M_INDEX"),
    DISP_STRING("RATE_54M_INDEX")
};
/*lint -restore */
#endif /* DBG */






/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxInitialize (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TC_Q_PARAMETERS_T prTcQueue;
    P_TX_ACQ_PARAMETERS_T prTxACQueue;
    P_MSDU_INFO_MEM_CTRL_T prMsduInfoMemCtrl;
    P_SW_TFCB_T prSwTfcb;
    PUINT_8 pucMemHandle;
    UINT_32 i, j;

    DEBUGFUNC("nicTxInitialize");


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;
    prTcQueue = &prTxCtrl->arTcQPara[0];
    prTxACQueue = &prTxCtrl->arTxACQPara[0];

    //4 <0> Clear allocated memory.
    kalMemZero((PVOID) prTxCtrl->pucTxCached, prTxCtrl->u4TxCachedSize);

    //4 <1> Initialization of SW Traffic Class Queue Parameters
    prTxCtrl->arTcQPara[TC0 ].ucMaxNumOfMsduInfo = CFG_MAX_NUM_MSDU_INFO_FOR_TC0;
    prTxCtrl->arTcQPara[TC1 ].ucMaxNumOfMsduInfo = CFG_MAX_NUM_MSDU_INFO_FOR_TC1;
    prTxCtrl->arTcQPara[TC2 ].ucMaxNumOfMsduInfo = CFG_MAX_NUM_MSDU_INFO_FOR_TC2;
    prTxCtrl->arTcQPara[TC3 ].ucMaxNumOfMsduInfo = CFG_MAX_NUM_MSDU_INFO_FOR_TC3;
    prTxCtrl->arTcQPara[TCS0].ucMaxNumOfMsduInfo = CFG_MAX_NUM_MSDU_INFO_FOR_TS0;
    prTxCtrl->arTcQPara[TCM ].ucMaxNumOfMsduInfo = CFG_MAX_NUM_MSDU_INFO_FOR_TCM;

    pucMemHandle = prTxCtrl->pucTxCached;
    for (i = 0; i < TC_NUM; i++, prTcQueue++) {
        QUEUE_INITIALIZE(&prTcQueue->rFreeMsduInfoList);
        QUEUE_INITIALIZE(&prTcQueue->rOsSendQueue);

        for (j = 0; j < prTcQueue->ucMaxNumOfMsduInfo; j++) {
            prMsduInfoMemCtrl = (P_MSDU_INFO_MEM_CTRL_T)pucMemHandle;

            prMsduInfoMemCtrl->ucMsduPoolID = (UINT_8)i;
            QUEUE_INSERT_TAIL(&prTcQueue->rFreeMsduInfoList, &prMsduInfoMemCtrl->rMsduInfo.rQueEntry);

            pucMemHandle += ALIGN_4(sizeof(MSDU_INFO_MEM_CTRL_T));
        }

        DBGLOG(TX, INFO, ("TXQ: %ld, Max Number of MSDU_INFO_T: %d, In rFreeMsduInfoList: %ld\n", \
            i, prTcQueue->ucMaxNumOfMsduInfo, \
            prTcQueue->rFreeMsduInfoList.u4NumElem));
    }


    //4 <2> Initialization of HW TX Queue Parameters
    prTxCtrl->arTxACQPara[TXQ_AC0].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC0;
    prTxCtrl->arTxACQPara[TXQ_AC1].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC1;
    prTxCtrl->arTxACQPara[TXQ_AC2].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC2;
    prTxCtrl->arTxACQPara[TXQ_AC3].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC3;
    prTxCtrl->arTxACQPara[TXQ_TS0].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_TSB;
    prTxCtrl->arTxACQPara[TXQ_AC4].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC4;

    prTxCtrl->arTxACQPara[TXQ_AC0].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC0;
    prTxCtrl->arTxACQPara[TXQ_AC1].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC1;
    prTxCtrl->arTxACQPara[TXQ_AC2].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC2;
    prTxCtrl->arTxACQPara[TXQ_AC3].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC3;
    prTxCtrl->arTxACQPara[TXQ_TS0].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_TSB;
    prTxCtrl->arTxACQPara[TXQ_AC4].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC4;

    prTxCtrl->arTxACQPara[TXQ_AC0].ucMaxNumOfSwTfcb = MAX_NUM_SW_TFCB_FOR_AC0;
    prTxCtrl->arTxACQPara[TXQ_AC1].ucMaxNumOfSwTfcb = MAX_NUM_SW_TFCB_FOR_AC1;
    prTxCtrl->arTxACQPara[TXQ_AC2].ucMaxNumOfSwTfcb = MAX_NUM_SW_TFCB_FOR_AC2;
    prTxCtrl->arTxACQPara[TXQ_AC3].ucMaxNumOfSwTfcb = MAX_NUM_SW_TFCB_FOR_AC3;
    prTxCtrl->arTxACQPara[TXQ_TS0].ucMaxNumOfSwTfcb = MAX_NUM_SW_TFCB_FOR_TS0;
    prTxCtrl->arTxACQPara[TXQ_AC4].ucMaxNumOfSwTfcb = MAX_NUM_SW_TFCB_FOR_AC4;

    for (i = 0; i < TXQ_NUM; i++, prTxACQueue++) {
        QUEUE_INITIALIZE(&prTxACQueue->rSendWaitQueue);
        QUEUE_INITIALIZE(&prTxACQueue->rActiveChainList);
        QUEUE_INITIALIZE(&prTxACQueue->rFreeTFCBList);

        for (j = 0; j < prTxACQueue->ucMaxNumOfSwTfcb; j++) {
            prSwTfcb = (P_SW_TFCB_T)pucMemHandle;

            prSwTfcb->ucAC = (UINT_8)i;
            QUEUE_INSERT_TAIL(&prTxACQueue->rFreeTFCBList, &prSwTfcb->rQueEntry);

            pucMemHandle += ALIGN_4(sizeof(SW_TFCB_T));
        }

        TX_ACQ_RESET_ALL_CNTS(prTxCtrl, i);

#if CFG_TX_DBG_INCREASED_PID
        prTxACQueue->ucPacketID = 0;
#endif /* CFG_TX_DBG_INCREASED_PID */

        DBGLOG(TX, INFO, ("ACQ: %ld, Max Number of SW_TFCB_T: %d, In rFreeSwTfcbList: %ld\n", \
            i, prTxACQueue->ucMaxNumOfSwTfcb, \
            prTxACQueue->rFreeTFCBList.u4NumElem));
    }

    /* Check if the memory allocation consist with this initialization function */
    ASSERT((UINT_32)(pucMemHandle - prTxCtrl->pucTxCached) == prTxCtrl->u4TxCachedSize);


    //4 <3> Initialization of Default Mapping Table of  SW Traffic Class to HW TX Queue.
    kalMemCopy(prTxCtrl->aucTxQoSCtrl_TC2TXQ,
               (PUINT_8)aucTC2DefaultAdmittedTXQ,
               sizeof(aucTC2DefaultAdmittedTXQ));


    //4 <4> Initialization of TX Queue Status.
    /* Reset Bitmap of TX Active Queues & Non-empty AC Queues */
    prTxCtrl->ucTxActiveACQ = 0;
    prTxCtrl->ucTxSuspendACQ = 0;
    prTxCtrl->ucTxNonEmptyACQ = 0;

    /* Reset TX Privilege Lock Count */
    prTxCtrl->ucTxPrivilegeLockCount = 0;


    //4 <5> Initialization of TX Control Switch.
    /* Initialization of TX Control Flags */
    prTxCtrl->fgIsSignalFifoEmpty = FALSE;

    prTxCtrl->fgIsEnableTxVoipFlowCtrl = FALSE;
    prTxCtrl->fgIsPermitAccessingTXQ_VoIP = TRUE;

    prTxCtrl->fgIsTxMediumTimeAvailable = TRUE;

    prTxCtrl->fgIsEnableTriggerOfVoipScan = FALSE;
    prTxCtrl->fgIsRfTestTxMode = FALSE;

#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
    prTxCtrl->fgBlockTxDuringJoin = FALSE;
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */

#if CFG_TX_AGGREGATE_HW_FIFO
    prTxCtrl->fgAggregateTxFifo = FALSE;
#endif /* CFG_TX_AGGREGATE_HW_FIFO */

    //4 <6> Initialization of TX Debug Function.
#if CFG_TX_DBG_TFCB_CHKSUM || CFG_TX_DBG_SEQ_NUM
    halTxEnableDebugOption(prAdapter);
#endif /* CFG_TX_DBG_TFCB_CHKSUM || CFG_TX_DBG_SEQ_NUM */

#if CFG_TX_DBG_INT_FALSE_ALARM
    prTxCtrl->fgIsSkipTxFalseAlarmCheck = FALSE; /* Check TX False Alarm as could as possible */
#endif /* CFG_TX_DBG_INT_FALSE_ALARM */


    //4 <7> Clear all TX counters
    TX_RESET_ALL_CNTS(prTxCtrl);

    prTxCtrl->ucLastTxWlanIndex = 0;
    prTxCtrl->ucCurrRateIndex = RATE_1M_INDEX;
    GET_CURRENT_SYSTIME(&prTxCtrl->rCurrRateLastUpdateTime);

#if CFG_SDIO_TX_ENHANCE
    prTxCtrl->u4WriteBlockSize = kalGetSDIOWriteBlkSize(prAdapter->prGlueInfo);

    prTxCtrl->pucTxCoalescingBufPtr = prAdapter->pucCoalescingBufCached;
    prTxCtrl->u4TxCoalescingBufUsedBlkCount = 0;
    prTxCtrl->u4TxCoalescingBufMaxBlkNum = CFG_COALESCING_BUFFER_SIZE / prTxCtrl->u4WriteBlockSize;

    #if CFG_SDIO_DEBUG_AGGREGATING_RATIO
    prTxCtrl->u4TxAggregateFrameCount = 0;
    prTxCtrl->u4TxPacketCount = 0;
    prTxCtrl->u4TxSDIOCmdCount = 0;
    #endif /* CFG_SDIO_DEBUG_AGGREGATING_RATIO */
#endif /* CFG_SDIO_TX_ENHANCE */

    return;
} /* end of nicTxInitialize() */


#if CFG_TX_AGGREGATE_HW_FIFO
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxAggregateTXQ (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN fgAggregateTxFifo
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    nicTxFlushStopQueues(prAdapter, (UINT_8)TXQ_DATA_MASK, 0x0 /*(UINT_8)NULL*/);

    if (fgAggregateTxFifo && !prTxCtrl->fgAggregateTxFifo) {

        //nicTxFlushStopQueues(prAdapter, (UINT_8)TXQ_DATA_MASK, 0x0);

        prTxCtrl->arTxACQPara[TXQ_AC0].u2FreeBufferSizeDW = 0;
        prTxCtrl->arTxACQPara[TXQ_AC1].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC0 +
                                                            HW_BUFF_DWSIZE_FOR_AC1 +
                                                            HW_BUFF_DWSIZE_FOR_AC2 /* +
                                                            HW_BUFF_DWSIZE_FOR_AC3 */;
        prTxCtrl->arTxACQPara[TXQ_AC2].u2FreeBufferSizeDW = 0;
        //prTxCtrl->arTxACQPara[TXQ_AC3].u2FreeBufferSizeDW = 0; /* Some MGMT will use AC3 */

        prTxCtrl->arTxACQPara[TXQ_AC0].u2MaxBufferSizeDW = 0;
        prTxCtrl->arTxACQPara[TXQ_AC1].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC0 +
                                                           HW_BUFF_DWSIZE_FOR_AC1 +
                                                           HW_BUFF_DWSIZE_FOR_AC2 /* +
                                                           HW_BUFF_DWSIZE_FOR_AC3 */;
        prTxCtrl->arTxACQPara[TXQ_AC2].u2MaxBufferSizeDW = 0;
        //prTxCtrl->arTxACQPara[TXQ_AC3].u2MaxBufferSizeDW = 0; /* Some MGMT will use AC3 */

        prTxCtrl->fgAggregateTxFifo = TRUE;

        halAggregateHWTxDataQueue(prAdapter, TRUE);
    }
    else if (!fgAggregateTxFifo && prTxCtrl->fgAggregateTxFifo) {

        //nicTxFlushStopQueues(prAdapter, (UINT_8)TXQ_DATA_MASK, 0x0);

        prTxCtrl->arTxACQPara[TXQ_AC0].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC0;
        prTxCtrl->arTxACQPara[TXQ_AC1].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC1;
        prTxCtrl->arTxACQPara[TXQ_AC2].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC2;
        //prTxCtrl->arTxACQPara[TXQ_AC3].u2FreeBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC3;

        prTxCtrl->arTxACQPara[TXQ_AC0].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC0;
        prTxCtrl->arTxACQPara[TXQ_AC1].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC1;
        prTxCtrl->arTxACQPara[TXQ_AC2].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC2;
        //prTxCtrl->arTxACQPara[TXQ_AC3].u2MaxBufferSizeDW = HW_BUFF_DWSIZE_FOR_AC3;

        prTxCtrl->fgAggregateTxFifo = FALSE;

        halAggregateHWTxDataQueue(prAdapter, FALSE);
    }

    return;
} /* end of nicTxAggregateTXQ() */
#endif /* CFG_TX_AGGREGATE_HW_FIFO */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxRelease (
    IN P_ADAPTER_T  prAdapter
    )
{
    ASSERT(prAdapter);

    nicTxCleanUpActiveList(prAdapter);

    nicTxCleanUpSendWaitQue(prAdapter);

    nicTxCleanUpOsSendQue(prAdapter);

#if CFG_IBSS_POWER_SAVE
    nicTxCleanUpStaWaitQue(prAdapter);
#endif /* CFG_IBSS_POWER_SAVE */

    return;
} /* end of nicTxRelease() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_MSDU_INFO_T
nicTxAllocMsduInfo (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucTrafficClass
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_QUE_T prFreeMsduInfoList;
    P_MSDU_INFO_T prMsduInfo = (P_MSDU_INFO_T)NULL;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (ucTrafficClass <= TCM) {

        prFreeMsduInfoList = &prTxCtrl->arTcQPara[ucTrafficClass].rFreeMsduInfoList;

        QUEUE_REMOVE_HEAD(prFreeMsduInfoList, prMsduInfo, P_MSDU_INFO_T);

        /* Because we only can release the MSDU_INFO_T which has no associated TFCB to
         * rFreeMsduInfoList. Thus the value "ucFragFinishedCount" of MSDU_INFO_T
         * must be 0 when we acquire it from the free list at anytime.
         */
#if DBG
        if (prMsduInfo) {
            ASSERT(prMsduInfo->ucFragFinishedCount == 0);
        }
#endif /* DBG */

#if 0
        DBGLOG(KEVIN, TRACE, ("arTxACQPara[%d].rFreeMsduInfoList = %d\n",
            ucTrafficClass, prFreeMsduInfoList->u4NumElem);
#endif
    }

#if 0
    if(!prMsduInfo){
        DBGLOG(MIKE, TRACE, ("No enough prMsduInfo(%d)\n", ucTrafficClass));
    }
#endif

    return prMsduInfo;

} /* end of nicTxAllocMsduInfo() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxReturnMsduInfo (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_QUE_T prFreeMsduInfoList;
    UINT_8 ucMsduPoolID;

    DEBUGFUNC("nicTxReturnMsduInfo");


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (prMsduInfo) {

        /* If and only if there is no associated TFCB waiting in ActiveChainList, then
         * we can release it. In another word, more than 1 MPDU still left in
         * HW TXQ.
         */
        if (prMsduInfo->ucFragFinishedCount == 0) {

            ucMsduPoolID = ENTRY_OF(prMsduInfo, MSDU_INFO_MEM_CTRL_T, rMsduInfo)->ucMsduPoolID;

            prFreeMsduInfoList = &prTxCtrl->arTcQPara[ucMsduPoolID].rFreeMsduInfoList;

            QUEUE_INSERT_TAIL(prFreeMsduInfoList, &prMsduInfo->rQueEntry);
        }

#if DBG
        else {
            DBGLOG(TX, ERROR, ("Try to discard a MSDU_INFO_T which still has some TFCBs(%d) in ActiveChainList.\n",
                prMsduInfo->ucFragFinishedCount));
        }
#endif /* DBG */

    }

    return;
} /* end of nicTxReturnMsduInfo() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxDiscardMsduInfo (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_QUE_T prFreeMsduInfoList;
    UINT_8 ucMsduPoolID;

    DEBUGFUNC("nicTxDiscardMsduInfo");


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (prMsduInfo) {

        /* If and only if there is no associated TFCB waiting in ActiveChainList, then
         * we can release it. In another word, more than 1 MPDU still left in
         * HW TXQ.
         */
        if (prMsduInfo->ucFragFinishedCount == 0) {

            if (prTxCtrl->fgIsRfTestTxMode) {
                rftestBufFreeRFTestBuf(prAdapter, (P_RFTEST_TXBUF_T)prMsduInfo->pvPacket);
            }
            else if (prMsduInfo->fgIsFromInternalProtocolStack) {
                mgtBufFreeMgtPacket(prAdapter, (P_MGT_PACKET_T)prMsduInfo->pvPacket);
            }
            else {
                if (prMsduInfo->pvPacket) {
                    kalSendComplete(prAdapter->prGlueInfo,
                                    prMsduInfo->pvPacket,
                                    (prMsduInfo->fgIsTxFailed ? WLAN_STATUS_FAILURE : WLAN_STATUS_SUCCESS));
                }
            }

            ucMsduPoolID = ENTRY_OF(prMsduInfo, MSDU_INFO_MEM_CTRL_T, rMsduInfo)->ucMsduPoolID;

            prFreeMsduInfoList = &prTxCtrl->arTcQPara[ucMsduPoolID].rFreeMsduInfoList;

            QUEUE_INSERT_TAIL(prFreeMsduInfoList, &prMsduInfo->rQueEntry);
        }
#if DBG
        else {
            DBGLOG(TX, ERROR, ("Try to discard a MSDU_INFO_T which still has some TFCBs(%d) in ActiveChainList.\n",
                prMsduInfo->ucFragFinishedCount));
        }
#endif /* DBG */

    }

    return;
} /* end of nicTxDiscardMsduInfo() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxNonQoSUpdateTXQParameters (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PHY_TYPE_INDEX_T ePhyTypeIndex
    )
{
    P_TX_AC_PARAM_AIFS_CW_T prTxAcParamAifsCw;
    UINT_16 u2CWmin, u2CWmax, u2Aifsn, u2TxopLimit;
    UINT_32 i;


    ASSERT(prAdapter);

    if (ePhyTypeIndex >= PHY_TYPE_INDEX_NUM) {
        return;
    }

    switch (ePhyTypeIndex) {
    case PHY_TYPE_ERP_INDEX:
        u2CWmin = CWMIN_ERP_1;
        u2CWmax = CWMAX_ERP;
        break;

    case PHY_TYPE_HR_DSSS_INDEX:
        u2CWmin = CWMIN_HR_DSSS;
        u2CWmax = CWMAX_HR_DSSS;
        break;

    case PHY_TYPE_OFDM_INDEX:
        u2CWmin = CWMIN_OFDM;
        u2CWmax = CWMAX_OFDM;
        break;

    default:
        return;
    }

    u2Aifsn = DIFS;
    u2TxopLimit = 0;

    prTxAcParamAifsCw = &prAdapter->rTxCtrl.arTxAcParamAifsCw[0];

    for (i = TXQ_AC0; i <= TXQ_AC3; i++) {
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2CWmin = u2CWmin;
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2CWmax = u2CWmax;
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2TxopLimit = u2TxopLimit;
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2Aifsn = u2Aifsn;
    }
    prTxAcParamAifsCw[TXQ_AC4] = prTxAcParamAifsCw[TXQ_AC0];

    halSetACParameters(prAdapter, prTxAcParamAifsCw);

    return;
} /* end of nicTxNonQoSUpdateTXQParameters() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxNonQoSAssignDefaultAdmittedTXQ (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    kalMemCopy(prTxCtrl->aucTxQoSCtrl_TC2TXQ,
               (PUINT_8)aucTC2DefaultAdmittedTXQ,
               sizeof(aucTC2DefaultAdmittedTXQ));

    return;
} /* end of nicTxNonQoSAssignDefaultAdmittedTXQ() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxQoSUpdateTXQParameters (
    IN P_ADAPTER_T prAdapter,
    IN WMM_AC_PARAM_T arWmmAcParams[]
    )
{
    P_TX_AC_PARAM_AIFS_CW_T prTxAcParamAifsCw;
    P_WMM_AC_PARAM_T prWmmAcParam;
    P_CONNECTION_SETTINGS_T prConnSettings;
    UINT_32 i;


    ASSERT(prAdapter);
    ASSERT(arWmmAcParams);

    prTxAcParamAifsCw = &prAdapter->rTxCtrl.arTxAcParamAifsCw[0];
    prConnSettings = &prAdapter->rConnSettings;

    for (i = AC_BE; i < AC_NUM; i++) {
        prWmmAcParam = &arWmmAcParams[i];

        prTxAcParamAifsCw[aucACI2TXQ[i]].u2CWmin = (UINT_16)ECW_TO_CW(prWmmAcParam->ucECWmin);
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2CWmax = (UINT_16)ECW_TO_CW(prWmmAcParam->ucECWmax);
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2TxopLimit = prWmmAcParam->u2TxopLimit;
        prTxAcParamAifsCw[aucACI2TXQ[i]].u2Aifsn = (UINT_16)prWmmAcParam->ucAifsn;
    }

    prTxAcParamAifsCw[TXQ_AC4] = prTxAcParamAifsCw[TXQ_AC3];

    prTxAcParamAifsCw[TXQ_AC2].u2Aifsn += (UINT_16)prConnSettings->ucViAifsnBias;
    prTxAcParamAifsCw[TXQ_AC2].u2TxopLimit =
        (prTxAcParamAifsCw[TXQ_AC2].u2TxopLimit < prConnSettings->u2ViMaxTxopLimit) ?
         prTxAcParamAifsCw[TXQ_AC2].u2TxopLimit : prConnSettings->u2ViMaxTxopLimit;

    halSetACParameters(prAdapter, prTxAcParamAifsCw);

    return;
} /* end of nicTxQoSUpdateTXQParameters() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxQoSAssignAdmittedTXQ (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucACI2AdmittedACI[]
    )
{
    P_TX_CTRL_T prTxCtrl;
    UINT_32 u4TC;


    ASSERT(prAdapter);
    ASSERT(aucACI2AdmittedACI);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* QoS Module will update the table of aucACI2AdmittedACI[] for AC_BE ~ AC_VO. */
    for (u4TC = TC0; u4TC <= TC3; u4TC++) {
        prTxCtrl->aucTxQoSCtrl_TC2TXQ[u4TC] =
            aucACI2TXQ[ aucACI2AdmittedACI[ aucTC2ACI[u4TC] ] ];
    }

    return;
} /* end of nicTxQoSAssignAdmittedTXQ() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxQoSRearrangeQueuesForAdmCtrl (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
    P_QUE_T prSendWaitQueue;

    QUE_T rSendWaitQueBackup;
    P_QUE_T prSendWaitQueBackup = &rSendWaitQueBackup;

    P_MSDU_INFO_T prMsduInfo;
    UINT_8 ucAC;
    INT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* We only apply the Admission Control to AC0~AC3, so TXQ_TS0 and TXQ_AC4 is not
     * included.
     */
    for (i = TXQ_AC3 ; i >= TXQ_AC0 ; i--) {
        prTxACQPara = &prTxCtrl->arTxACQPara[i];
        prSendWaitQueue = &prTxACQPara->rSendWaitQueue;

        if (QUEUE_IS_NOT_EMPTY(prSendWaitQueue)) {

            QUEUE_MOVE_ALL(prSendWaitQueBackup, prSendWaitQueue);

            while (QUEUE_IS_NOT_EMPTY(prSendWaitQueBackup)) {
                QUEUE_REMOVE_HEAD(prSendWaitQueBackup, prMsduInfo, P_MSDU_INFO_T);

                ASSERT(prMsduInfo); /* Because we already check prSendWaitQueBackup->u4NumElem > 0 */

                if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_SPECIFY_AC) {
                    ASSERT(prMsduInfo->ucTC < TXQ_NUM);

                    ucAC = prMsduInfo->ucTC;
                }
                else {
                    ASSERT(prMsduInfo->ucTC < TC_NUM);

                    ucAC = prTxCtrl->aucTxQoSCtrl_TC2TXQ[prMsduInfo->ucTC];
                }

                prTxACQPara = &prTxCtrl->arTxACQPara[ucAC];
                prSendWaitQueue = &prTxACQPara->rSendWaitQueue;

                QUEUE_INSERT_TAIL(prSendWaitQueue, &prMsduInfo->rQueEntry);
            }
        }
    }

    return;
} /* end of nicTxQoSRearrangeQueuesForAdmCtrl() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessAdmitTimeMetInterrupt(
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* The TX Medium Time is exhausted when get in here, thus the packets remain in HWQ will
     * get pending and need wait for available Medium Time of next Admit Time
     * Period.
     * During the period of wiating for available Medium Time, HW can enter
     * Low Power Mode to save power consumption.
     */
    prTxCtrl->fgIsTxMediumTimeAvailable = FALSE;

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicTxAcquireResourceAndTFCBs (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN UINT_8           ucAC,
    OUT P_QUE_T         prAllocatedTFCBList
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
    P_SW_TFCB_T prSwTfcb = (P_SW_TFCB_T)NULL;
    P_QUE_T prFreeTFCBList;
    UINT_16 u2RequiredBufferLength;
    UINT_16 u2RequiredBufferLengthDW;
    WLAN_STATUS u4Status = WLAN_STATUS_RESOURCES;


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    ASSERT(prAllocatedTFCBList);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* Check if there exist enough SW_TFCB_Ts */
    prTxACQPara = &prTxCtrl->arTxACQPara[ucAC];
    prFreeTFCBList = &prTxACQPara->rFreeTFCBList;

    ASSERT(prMsduInfo->ucFragTotalCount); /* It should be >= 1 MPDU */

    if (prFreeTFCBList->u4NumElem >= prMsduInfo->ucFragTotalCount) {

#if CFG_TX_FRAGMENT
        if (prMsduInfo->ucFragTotalCount > 1) {
            UINT_16 u2OverallPayloadLength;
            UINT_16 u2LastFragBufferLength;
            UINT_16 u2AllFragsBufferLengthDW;
            UINT_32 i;


            u2OverallPayloadLength =
                (UINT_16)((prMsduInfo->pucLLC) ? (prMsduInfo->ucLLCLength + ETHER_TYPE_LEN) : 0) +
                prMsduInfo->u2PayloadLength +
                (UINT_16)((prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_CALCULATE_MIC) ? TKIP_MIC_LEN : 0);


            /* Payload Length should not exceed 4096 bytes */
            ASSERT((prMsduInfo->u2PayloadFragThreshold & ~TFCB_PAYLOAD_LEN_MASK) == 0);

            /* WLAN Header Length should not exceed 62 bytes or be odd */
            ASSERT((prMsduInfo->ucFragWlanHeaderLength & ~TFCB_WLAN_HEADER_LEN_MASK) == 0);

            /* Individual Buffer Length for 1st ~ (N-1)th MPDU */
            u2RequiredBufferLength = (TFCB_SIZE + \
                 (UINT_16)(prMsduInfo->ucFragWlanHeaderLength & TFCB_WLAN_HEADER_LEN_MASK) + \
                 (prMsduInfo->u2PayloadFragThreshold & TFCB_PAYLOAD_LEN_MASK));

            /* Accumulated Buffer Length for 1st ~ (N-1)th MPDU */
            u2AllFragsBufferLengthDW = BYTE_TO_DWORD(u2RequiredBufferLength) * \
                                       (prMsduInfo->ucFragTotalCount - 1);

            /* Individual Buffer Length for (N)th MPDU */
            u2LastFragBufferLength = (TFCB_SIZE + \
                 (UINT_16)(prMsduInfo->ucFragWlanHeaderLength & TFCB_WLAN_HEADER_LEN_MASK) + \
                 ((u2OverallPayloadLength - (prMsduInfo->u2PayloadFragThreshold * \
                    (prMsduInfo->ucFragTotalCount - 1))) & TFCB_PAYLOAD_LEN_MASK));

            /* Accumulated Buffer Length for 1st ~ (N)th MPDU */
            u2AllFragsBufferLengthDW += BYTE_TO_DWORD(u2LastFragBufferLength);

            /* Required Buffer Length should not exceed the maximum size of HW FIFO. */
            ASSERT(u2AllFragsBufferLengthDW <= prTxACQPara->u2MaxBufferSizeDW);

            /* Try to allocate HW FIFO */
            if (u2AllFragsBufferLengthDW <= prTxACQPara->u2FreeBufferSizeDW) {
                prTxACQPara->u2FreeBufferSizeDW -= u2AllFragsBufferLengthDW;

                /* Allocate 1st ~ (N-1)th SW_TFCB_T */
                for (i = 0; i < prMsduInfo->ucFragTotalCount; i++) {
                    QUEUE_REMOVE_HEAD(prFreeTFCBList, prSwTfcb, P_SW_TFCB_T);

                    ASSERT(prSwTfcb);

                    /* Can we set ucAC as const ?, so we don't have to set ucAC again */
                    prSwTfcb->ucAC = ucAC;

                    if (i == (UINT_32)(prMsduInfo->ucFragTotalCount - 1)) {
                        prSwTfcb->u2OverallBufferLength = u2LastFragBufferLength;
                    }
                    else {
                        prSwTfcb->u2OverallBufferLength = u2RequiredBufferLength;
                    }

                    prSwTfcb->prMsduInfo = prMsduInfo;

                    QUEUE_INSERT_TAIL(prAllocatedTFCBList, &prSwTfcb->rQueEntry)
                }

                u4Status = WLAN_STATUS_SUCCESS;

            }

        }
        else
#endif /* CFG_TX_FRAGMENT */
        {
            /* <NOTE> Kevin[2007/03/14]: We put assert and OR the length MASK here to check
             * if the input length(Payload/WlanHeader) is acceptable by HW TFCB.
             */

            /* Payload Length should not exceed 4096 bytes */
            ASSERT((prMsduInfo->u2PayloadLength & ~TFCB_PAYLOAD_LEN_MASK) == 0);

            if (prMsduInfo->fgIs802_11Frame) {

                /* WLAN Header Length should not exceed 62 bytes or be odd */
                ASSERT((prMsduInfo->ucMacHeaderLength & ~TFCB_WLAN_HEADER_LEN_MASK) == 0);

                u2RequiredBufferLength = (TFCB_SIZE + \
                    (prMsduInfo->ucMacHeaderLength & (UINT_8)TFCB_WLAN_HEADER_LEN_MASK) + \
                    (prMsduInfo->u2PayloadLength & TFCB_PAYLOAD_LEN_MASK));
                #if SUPPORT_WAPI
                if (!prMsduInfo->fgIsFromInternalProtocolStack) {
                    if (prMsduInfo->ucChkSumWapiFlag & TX_WPI_ENCRYPT){
                        u2RequiredBufferLength = (TFCB_SIZE + \
                            (prMsduInfo->ucMacHeaderLength & (UINT_8)TFCB_WLAN_HEADER_LEN_MASK) + \
                            ((prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN) & TFCB_PAYLOAD_LEN_MASK));
                    }
                    else if (prMsduInfo->ucChkSumWapiFlag & TX_WPI_OPEN){
                        u2RequiredBufferLength = (TFCB_SIZE + \
                            (prMsduInfo->ucMacHeaderLength & (UINT_8)TFCB_WLAN_HEADER_LEN_MASK) + \
                            ((prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN) & TFCB_PAYLOAD_LEN_MASK));
                    }
                }
                #endif
            }
            else {
                u2RequiredBufferLength = (ETHER_HEADER_LEN + \
                    (prMsduInfo->u2PayloadLength & TFCB_PAYLOAD_LEN_MASK));
            }

            /* Due to frame stored in HW AC FIFO will align to DW boundary, so we pad to
             * DW to figure out the occupied FIFO resource it will take.
             */
            u2RequiredBufferLengthDW = BYTE_TO_DWORD(u2RequiredBufferLength);

            /* Required Buffer Length should not exceed the maximum size of HW FIFO. */
            ASSERT(u2RequiredBufferLengthDW <= prTxACQPara->u2MaxBufferSizeDW);

            /* Try to allocate HW FIFO */
            if (u2RequiredBufferLengthDW <= prTxACQPara->u2FreeBufferSizeDW) {
                prTxACQPara->u2FreeBufferSizeDW -= u2RequiredBufferLengthDW;

                QUEUE_REMOVE_HEAD(prFreeTFCBList, prSwTfcb, P_SW_TFCB_T);

                ASSERT(prSwTfcb);

                /* Can we set ucAC as const ?, so we don't have to set ucAC again */
                prSwTfcb->ucAC = ucAC;

                prSwTfcb->u2OverallBufferLength = u2RequiredBufferLength;

                prSwTfcb->prMsduInfo = prMsduInfo;

                QUEUE_INSERT_TAIL(prAllocatedTFCBList, &prSwTfcb->rQueEntry)

                u4Status = WLAN_STATUS_SUCCESS;
            }

        }

    }

    return u4Status;

}/* end of nicTxAcquireResourceAndTFCBs() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
nicTxReturnResource (
    IN P_ADAPTER_T  prAdapter,
    IN P_SW_TFCB_T  prSwTfcb
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;


    ASSERT(prAdapter);
    ASSERT(prSwTfcb);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* Check if this TFCB has obtained some HW Resources */
    if (prSwTfcb->u2OverallBufferLength) {
        prTxACQPara = &prTxCtrl->arTxACQPara[prSwTfcb->ucAC];

        prTxACQPara->u2FreeBufferSizeDW += BYTE_TO_DWORD(prSwTfcb->u2OverallBufferLength);

        /* Clean it to avoid error caused by duplicate process */
        prSwTfcb->u2OverallBufferLength = 0;
    }

    return;
} /* end of nicTxReturnResource() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxReturnTFCB (
    IN P_ADAPTER_T  prAdapter,
    IN P_SW_TFCB_T  prSwTfcb
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_QUE_T prFreeTFCBList;


    ASSERT(prAdapter);
    ASSERT(prSwTfcb);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (prSwTfcb) {

        nicTxReturnResource(prAdapter, prSwTfcb);

        prFreeTFCBList = &prTxCtrl->arTxACQPara[prSwTfcb->ucAC].rFreeTFCBList;

        QUEUE_INSERT_TAIL(prFreeTFCBList, &prSwTfcb->rQueEntry);
    }

    return;
} /* end of nicTxReturnTFCB() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxReturnTFCBs (
    IN P_ADAPTER_T  prAdapter,
    IN P_QUE_T      prTFCBListNeedToReturn
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_SW_TFCB_T prSwTfcb;
    P_QUE_T prFreeTFCBList;


    ASSERT(prAdapter);
    ASSERT(prTFCBListNeedToReturn);
    prTxCtrl = &prAdapter->rTxCtrl;

    while (QUEUE_IS_NOT_EMPTY(prTFCBListNeedToReturn)) {
        QUEUE_REMOVE_HEAD(prTFCBListNeedToReturn, prSwTfcb, P_SW_TFCB_T);

        ASSERT(prSwTfcb);

        nicTxReturnResource(prAdapter, prSwTfcb);

        prFreeTFCBList = &prTxCtrl->arTxACQPara[prSwTfcb->ucAC].rFreeTFCBList;

        QUEUE_INSERT_TAIL(prFreeTFCBList, &prSwTfcb->rQueEntry);
    };

    return;
} /* end of nicTxReturnTFCBs() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicTxSetSignalWhenFifoNonEmpty (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgClearSignal
    )
{
    P_TX_CTRL_T prTxCtrl;

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (fgClearSignal) {
        prTxCtrl->fgIsSignalFifoEmpty = FALSE;
        return TRUE;
    }
    else {
        /* if (NIC_TX_IS_ACTIVE_ACQ_EMPTY(prAdapter)) { */
        if (NIC_TX_IS_AC4_EMPTY(prAdapter)) {

            /* HW AC4 Fifo is empty, so the signal won't be set */
            prTxCtrl->fgIsSignalFifoEmpty = FALSE;
            return FALSE;
        }
        else {

            /* HW Fifo is not empty, so the signal will be set */
            prTxCtrl->fgIsSignalFifoEmpty = TRUE;

            return TRUE;
        }
    }

} /* end of nicTxSetSignalWhenFifoNonEmpty() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxSignalFifoEmptyEvent (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (prTxCtrl->fgIsSignalFifoEmpty &&
        NIC_TX_IS_AC4_EMPTY(prAdapter)
        /* NIC_TX_IS_ACTIVE_ACQ_EMPTY(prAdapter) */) {

        /* Call back WH Security Module */
        arbFsmRunEventSecTxFIFOEmpty(prAdapter);

        /* Turn off Signal */
        prTxCtrl->fgIsSignalFifoEmpty = FALSE;

    }

    return;
} /* end of nicTxSignalFifoEmptyEvent() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxVoipFlowCtrlEnable (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    prTxCtrl->fgIsEnableTxVoipFlowCtrl = TRUE;

    return;
} /* end of nicTxVoipFlowCtrlEnable() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxVoipFlowCtrlDisable (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    prTxCtrl->fgIsEnableTxVoipFlowCtrl = FALSE;
    prTxCtrl->fgIsPermitAccessingTXQ_VoIP = TRUE;

    /* Pull down more packets */
    nicTxRetransmitOfSendWaitQue(prAdapter);

    if (prTxCtrl->fgIsPacketInOsSendQueue) {
        nicTxRetransmitOfOsSendQue(prAdapter);
    }

#if CFG_SDIO_TX_ENHANCE
    halTxLeftClusteredMpdu(prAdapter);
#endif /* CFG_SDIO_TX_ENHANCE */

    return;
} /* end of nicTxVoipFlowCtrlDisable() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxVoipFlowCtrlCheckForSuspend (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (prTxCtrl->fgIsEnableTxVoipFlowCtrl) {

        /* Check if the "TX Queue for VoIP access" is active and it's empty */
        if ((prTxCtrl->ucTxActiveACQ & BIT(prTxCtrl->rTxQForVoipAccess)) &
            ~(prTxCtrl->ucTxNonEmptyACQ)) {
            prTxCtrl->fgIsPermitAccessingTXQ_VoIP = FALSE;

            /* "TX Queue for VoIP access" is empty now,
               and it's the best time to trigger VOIP SCAN */
            if (prTxCtrl->fgIsEnableTriggerOfVoipScan) {
                arbFsmRunEventScanRequest(prAdapter, NULL, FALSE);
            }
        }
    }

    return;
} /* end of nicTxVoipFlowCtrlCheckForSuspend() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxVoipFlowCtrlResumePendingFrames (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* NOTE(Kevin): Before call this function, ARB already check if exist any buffered
     * VOICE(VOIP) packet.
     */

    if (prTxCtrl->fgIsEnableTxVoipFlowCtrl) {
        prTxCtrl->fgIsPermitAccessingTXQ_VoIP = TRUE; /* Turn on the Control Flag of VOICE TXQ */

        /* Pull down more packets */
        nicTxRetransmitOfSendWaitQue(prAdapter);

        if (prTxCtrl->fgIsPacketInOsSendQueue) {
            nicTxRetransmitOfOsSendQue(prAdapter);
        }

#if CFG_SDIO_TX_ENHANCE
        halTxLeftClusteredMpdu(prAdapter);
#endif /* CFG_SDIO_TX_ENHANCE */
    }

    return;
} /* end of nicTxVoipFlowCtrlResumePendingFrames() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicTxService (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
#if CFG_SDIO_TX_ENHANCE
    ,IN BOOLEAN         fgIsAggregate
#endif /* CFG_SDIO_TX_ENHANCE */
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
    QUE_T rAllocatedTFCBList;
    P_QUE_T prSendWaitQueue;
    UINT_8 ucAC;

    WLAN_STATUS rStatus = WLAN_STATUS_PENDING; /* Default is PENDING for slave mode */


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prTxCtrl = &prAdapter->rTxCtrl;

    QUEUE_INITIALIZE(&rAllocatedTFCBList);

    do {
        //4 <1> According the Traffic Class, assign it to an outgoing Access Category (HWQ).
        if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_SPECIFY_AC) {
            ASSERT(prMsduInfo->ucTC < TXQ_NUM);

            /* For example, if Management Layer specify the outgoing TXQ is TXQ_AC3.
             * we will force to assign the ucAC = TXQ_AC3, so this MSDU_INFO_T won't
             * be influence by admission control if this TXQ has not been granted yet.
             */
            ucAC = prMsduInfo->ucTC;
        }
        else {
            ASSERT(prMsduInfo->ucTC < TC_NUM);

            ucAC = prTxCtrl->aucTxQoSCtrl_TC2TXQ[prMsduInfo->ucTC];
        }

        //4 <2> Get the Send Wait Queue of the corresponding AC.
        prTxACQPara = &prTxCtrl->arTxACQPara[ucAC];
        prSendWaitQueue = &prTxACQPara->rSendWaitQueue;


        //4 <3> Check a Switch Flag of Tx Flow Control, then check Send Wait Queue for
        //4 packet ordering, and do acquire resource at last step.
        /* If the flag of permission of accessing HW FIFO is FALSE, enqueue the
         * incomming packet to the SendWaitQueue directly.
         */
        if (
#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
            (prTxCtrl->fgBlockTxDuringJoin && (TXQ_DATA_MASK & BIT(ucAC))) ||
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */

            ((!prTxCtrl->fgIsPermitAccessingTXQ_VoIP) && (ucAC == prTxCtrl->rTxQForVoipAccess)) ||
            QUEUE_IS_NOT_EMPTY(prSendWaitQueue) ||
            ((nicTxAcquireResourceAndTFCBs(prAdapter, prMsduInfo, ucAC, &rAllocatedTFCBList)
              == WLAN_STATUS_RESOURCES))) {

            /* Kevin[2007/03/07]: Queue the MSDU_INFO_T according to their
             * current Access Category(AC).
             */
            QUEUE_INSERT_TAIL(prSendWaitQueue, &prMsduInfo->rQueEntry);
        }

        if (QUEUE_IS_EMPTY(&rAllocatedTFCBList)) { /* Can not obtain any resource */
            break;
        }

        /* Clear the Transmission Status Flag
         * Assume a MSDU has various MPDUs, if one of them is failed, then this MSDU is failed
         */
        prMsduInfo->fgIsTxFailed = FALSE;

        /* None of any fragment has been processed(write to HW FIFO) yet */
        prMsduInfo->ucFragFinishedCount = 0;


        /*  =============================================================
         *      Acquire Power Control within the submodule.
         *
         *    This is done for solely optimization for power consumption,
         *  and modules need to concern whether to acquire power control
         *  or not for their operation depends on if HW access is needed.
         *  =============================================================
         */
        ARB_TEST_AND_GET_POWER_CONTROL_FROM_PM_IN_TX_PATH(prAdapter);

        //4 <4> Try to send TFCBs to HW FIFO now.
#if CFG_SDIO_TX_ENHANCE
        if (halTxTfcbs(prAdapter, prMsduInfo, &rAllocatedTFCBList, fgIsAggregate) == FALSE) {
#else
        if (halTxTfcbs(prAdapter, prMsduInfo, &rAllocatedTFCBList) == FALSE) {
#endif /* CFG_SDIO_TX_ENHANCE */

            /* <TODO> Kevin[2007/03/07]: Update TX BUS ERROR COUNTER here */

            nicTxReturnTFCBs(prAdapter, &rAllocatedTFCBList);

            /* <TODO> We may put a BUS_ERROR of WLAN_STATUS to indicate the caller */
            rStatus = WLAN_STATUS_FAILURE; /* Let GLUE Layer handle it */
        }

#if CFG_TX_RET_TX_CTRL_EARLY
        if (!prMsduInfo->pvPacket) {
            /* NOTE(Kevin): If bus access error (eg. remove the SDIO card suddenly)
             * We still report the rStatus is SUCCESS, because we already free that
             * frame while copy it to coalescing buffer.
             */
            rStatus = WLAN_STATUS_SUCCESS; /* This frame has been freed */
        }
#endif /* CFG_TX_RET_TX_CTRL_EARLY */
    }
    while (FALSE);

    return rStatus;

} /* end of nicTxService() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxRetransmitOfOsSendQue (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TC_Q_PARAMETERS_T prTcQPara;
    P_QUE_T prOsSendQueue;
    P_QUE_ENTRY_T prQueueEntry;
    P_NATIVE_PACKET prPacket;
    P_MSDU_INFO_T prMsduInfo;

    UINT_8 ucTID; /* Priority Parameter in 6.2.1.1.2 Semantics of the service primitive */
    UINT_8 ucTC; /* "Traffic Class" SW(Driver) resource classification */

    UINT_8 ucMacHeaderLen;
    UINT_16 u2PayloadLen;
    OS_SYSTIME rArrivalTime;

    BOOLEAN fgIsMorePackets = FALSE; /* TRUE if there exist a non-empty rOsSendQueue at the end */
    INT_32 i;
    UINT_32 u4Status = WLAN_STATUS_SUCCESS;

    BOOLEAN fgIs1x = FALSE;
    UINT_8  ucControlFlag;
    P_MGT_PACKET_T prMgtPacket = NULL;
    UINT_32 u2EstimatedFrameLen = 0;

    DEBUGFUNC("nicTxRetransmitOfOsSendQue");


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    for (i = (TC_NUM- 1); i >= 0; i--) {

        prTcQPara = &prTxCtrl->arTcQPara[i];
        prOsSendQueue = &prTcQPara->rOsSendQueue;

        while (QUEUE_IS_NOT_EMPTY(prOsSendQueue)) {

            prQueueEntry = QUEUE_GET_HEAD(prOsSendQueue);

            ASSERT(prQueueEntry);

            prPacket = KAL_GET_PKT_DESCRIPTOR(prQueueEntry);
            ucTID = KAL_GET_PKT_TID(prPacket);

            //4 <1> Convert the Priority Parameter/TID (User Priority/TSID) to Traffic Class(TC).
            if (prAdapter->rBssInfo.fgIsWmmAssoc) {
                ucTC = aucPriorityParam2TC[ucTID];
            }
            else {
                ucTC = TC1; /* When associated with non-QoS AP, set ucTC to TC1 for all packets. */
            }

            DBGLOG(TX, LOUD, ("ucTID = %d, ucTC = %d\n", ucTID, ucTC));

            fgIs1x = KAL_GET_PKT_IS1X(prPacket);

            // May acquire Spin Lock here.
            if ((prMsduInfo = nicTxAllocMsduInfo(prAdapter, ucTC)) != (P_MSDU_INFO_T)NULL) {
                if ((fgIs1x) && (prAdapter->rConnSettings.eAuthMode <= AUTH_MODE_AUTO_SWITCH)){

                    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, u2EstimatedFrameLen);
                    if (!prMgtPacket) {
                        DBGLOG(TX, TRACE, ("No buffer for Legacy 802.1x frame\n"));
                        /* Release Msdu */
                        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
                        prMsduInfo = NULL;
                    }
                    else {
                        QUEUE_REMOVE_HEAD(prOsSendQueue, prQueueEntry, P_QUE_ENTRY_T);
                    }
                }
                else {
                    QUEUE_REMOVE_HEAD(prOsSendQueue, prQueueEntry, P_QUE_ENTRY_T);
                }
            }
            // May release Spin Lock here.

            if (!prMsduInfo) {
                fgIsMorePackets = TRUE;
                break;
            }

            ucMacHeaderLen = KAL_GET_PKT_HEADER_LEN(prPacket);
            u2PayloadLen = KAL_GET_PKT_PAYLOAD_LEN(prPacket);
            rArrivalTime = KAL_GET_PKT_ARRIVAL_TIME(prPacket);

            if (
#if SUPPORT_WAPI
                !prAdapter->fgUseWapi &&
#endif
                (fgIs1x) && (prAdapter->rConnSettings.eAuthMode <= AUTH_MODE_AUTO_SWITCH)){

                ucControlFlag = MSDU_INFO_CTRL_FLAG_DISABLE_PRIVACY_BIT;
                if (prAdapter->rBssInfo.fgIsWmmAssoc) {
                    u2EstimatedFrameLen = WLAN_MAC_HEADER_QOS_LEN + \
                                          LLC_LEN + \
                                          u2PayloadLen;
                }
                else {
                    u2EstimatedFrameLen = WLAN_MAC_HEADER_LEN + \
                                          LLC_LEN + \
                                          u2PayloadLen;
                }

                ASSERT(prMgtPacket);

                nicCompose802_11DataFrame(prAdapter,
                                          MGT_PACKET_GET_BUFFER(prMgtPacket),
                                          ucTID,
                                          FALSE,
                                          prPacket,
                                          &ucMacHeaderLen);

                mgtPacketPut(prMgtPacket, u2EstimatedFrameLen);

                MSDU_INFO_OBJ_INIT(prMsduInfo, \
                                   TRUE, \
                                   TRUE, \
                                   (PVOID)prMgtPacket, \
                                   ucTID, \
                                   ucTC, \
                                   (UINT_8)ucMacHeaderLen, \
                                   LLC_LEN + u2PayloadLen, \
                                   ucControlFlag, \
                                   NULL, \
                                   KAL_GET_PKT_ARRIVAL_TIME(prPacket), \
                                   NULL \
                                   );

                kalSendComplete(prAdapter->prGlueInfo, prPacket, WLAN_STATUS_SUCCESS);
                prPacket = NULL;

                if ((u4Status = arbFsmRunEventTxMmpdu(prAdapter,prMsduInfo)) != WLAN_STATUS_PENDING) {

                    if(u4Status != WLAN_STATUS_SUCCESS){
                        mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

                        nicTxReturnMsduInfo(prAdapter, prMsduInfo);

                        DBGLOG(TX, ERROR, ("Send Legacy 802.1x frame fail.\n"));

                        u4Status = WLAN_STATUS_FAILURE;
                    }
                }
            }
            else {

#if SUPPORT_WAPI
                if (prAdapter->fgUseWapi) {
                    if (fgIs1x)
                        ucControlFlag = MSDU_INFO_CTRL_FLAG_DISABLE_PRIVACY_BIT;
                    else
                        ucControlFlag = MSDU_INFO_CTRL_FLAG_NONE;
                    //4 <2> Initialization of MSDU_INFO_T.
                    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                                   TRUE, \
                                   FALSE, \
                                   (PVOID)prPacket, \
                                   ucTID, \
                                   ucTC, \
                                   ucMacHeaderLen, \
                                   u2PayloadLen, \
                                   ucControlFlag, \
                                   (PFN_TX_DONE_HANDLER)0, \
                                   rArrivalTime, \
                                   NULL \
                                   );
                }
                else {
                  MSDU_INFO_OBJ_INIT(prMsduInfo, \
                                   ((ucMacHeaderLen >= WLAN_MAC_HEADER_LEN) ? TRUE : FALSE), \
                                   FALSE, \
                                   (PVOID)prPacket, \
                                   ucTID, \
                                   ucTC, \
                                   ucMacHeaderLen, \
                                   u2PayloadLen, \
                                   MSDU_INFO_CTRL_FLAG_NONE, \
                                   (PFN_TX_DONE_HANDLER)0, \
                                   rArrivalTime, \
                                   NULL \
                                   );
                    }
#else
                //4 <2> Initialization of MSDU_INFO_T.
                MSDU_INFO_OBJ_INIT(prMsduInfo, \
                                   ((ucMacHeaderLen >= WLAN_MAC_HEADER_LEN) ? TRUE : FALSE), \
                                   FALSE, \
                                   (PVOID)prPacket, \
                                   ucTID, \
                                   ucTC, \
                                   ucMacHeaderLen, \
                                   u2PayloadLen, \
                                   MSDU_INFO_CTRL_FLAG_NONE, \
                                   (PFN_TX_DONE_HANDLER)0, \
                                   rArrivalTime, \
                                   NULL \
                                   );

#endif
                //4 <3> Process MSDU Integrity Protection & Fragmentation
                if (txProcessMSDU(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {

                    prMsduInfo->fgIsTxFailed = TRUE;

                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                    break;
                }

                //4 <4> Forward MSDU_INFO_T to NIC Layer

#if CFG_SDIO_TX_ENHANCE
                if ((u4Status = nicTxService(prAdapter, prMsduInfo, TRUE)) == WLAN_STATUS_FAILURE) {
#else
                if ((u4Status = nicTxService(prAdapter, prMsduInfo)) == WLAN_STATUS_FAILURE) {
#endif /* CFG_SDIO_TX_ENHANCE */

                    prMsduInfo->fgIsTxFailed = TRUE;

                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);
                    DBGLOG(MIKE, TRACE, ("nicTxService Failed\n"));
                    break;
                }
            }
        }
    }

    if (!fgIsMorePackets) {
        prTxCtrl->fgIsPacketInOsSendQueue = FALSE;
    }

    return;
} /* end of nicTxRetransmitOfOsSendQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxRetransmitOfSendWaitQue (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;

    P_MSDU_INFO_T prMsduInfo;
    P_QUE_T prSendWaitQueue;
    QUE_T rAllocatedTFCBList;

    INT_32 i;

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    {

        for (i = TXQ_AC4; i >= TXQ_AC0; i--) {

            if (
#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
                (prTxCtrl->fgBlockTxDuringJoin && (TXQ_DATA_MASK & BIT(i))) ||
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */
                (!prTxCtrl->fgIsPermitAccessingTXQ_VoIP && (i == prTxCtrl->rTxQForVoipAccess))) {
                continue;
            }

            QUEUE_INITIALIZE(&rAllocatedTFCBList);

            prTxACQPara = &prTxCtrl->arTxACQPara[i];
            prSendWaitQueue = &prTxACQPara->rSendWaitQueue;
            while (QUEUE_IS_NOT_EMPTY(prSendWaitQueue)) {

                prMsduInfo = (P_MSDU_INFO_T)QUEUE_GET_HEAD(prSendWaitQueue);

                ASSERT(prMsduInfo);
                /* NOTE(Kevin 2008/1/16): We'll do Auto Fragment only when the
                 * Media Status was CONNECTED.
                 */
#if CFG_TX_FRAGMENT
                if ((prAdapter->rConnSettings.fgIsEnableTxFragment) &&
                    //(prAdapter->rConnSettings.fgIsEnableTxAutoFragmentForBT) &&
                    (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED)) {

                    if (txFragmentation(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {
                        break;
                    }
                }
#endif /* CFG_TX_FRAGMENT */

                if (nicTxAcquireResourceAndTFCBs(prAdapter, prMsduInfo, (UINT_8)i,
                    &rAllocatedTFCBList) == WLAN_STATUS_RESOURCES) {
                    break;
                }

                QUEUE_REMOVE_HEAD(prSendWaitQueue, prMsduInfo, P_MSDU_INFO_T);

                ASSERT(prMsduInfo);

                /* Clear the Transmission Status Flag
                 * Assume a MSDU has various MPDUs, if one of them is failed, then this MSDU is failed
                 */
                prMsduInfo->fgIsTxFailed = FALSE;

                /* None of any fragment has been processed(write to HW FIFO) yet */
                prMsduInfo->ucFragFinishedCount = 0;

#if CFG_SDIO_TX_ENHANCE
                if (halTxTfcbs(prAdapter, prMsduInfo, &rAllocatedTFCBList, TRUE) == FALSE) {
#else
                if (halTxTfcbs(prAdapter, prMsduInfo, &rAllocatedTFCBList) == FALSE) {
#endif /* CFG_SDIO_TX_ENHANCE */

                    /* <TODO> Kevin[2007/03/07]: Update TX BUS ERROR COUNTER here */

                    nicTxReturnTFCBs(prAdapter, &rAllocatedTFCBList);

                    prMsduInfo->fgIsTxFailed = TRUE;

                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                    return;
                }

            }
        }
    }

    return;
} /* end of nicTxRetransmitOfSendWaitQue() */


#if !CFG_SDIO_STATUS_ENHANCE

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ UINT_32
nicTxProcessTFCBs (
    IN P_ADAPTER_T  prAdapter,
    IN P_QUE_T      prMsduInfoDoneList,
    OUT PBOOLEAN    pfgPsBit
    )
{
    P_TX_CTRL_T prTxCtrl;
    TX_STATUS_T rTxStatus;
    P_SW_TFCB_T prSwTfcb;
    P_MSDU_INFO_T prMsduInfo;
    UINT_32 u4TfcbNum = 0;
    UINT_32 u4TxCount;
    P_STA_RECORD_T prStaRec;
    UINT_8 ucWlanIndex;

    DEBUGFUNC("nicTxProcessTFCBs");


    ASSERT(prAdapter);
    ASSERT(prMsduInfoDoneList);
    ASSERT(pfgPsBit);
    prTxCtrl = &prAdapter->rTxCtrl;

    do {
        //4 <1> Get a TX_STATUS and associated SW_TFCB_T back.
        if (halTxReadTxStatus(prAdapter, &rTxStatus, &prSwTfcb) == FALSE) {
            break;
        }

        u4TfcbNum++;

        //4 <2> Get the associated MSDU_INFO_T and free the SW_TFCB_T.
        prMsduInfo = prSwTfcb->prMsduInfo;

        /* Time to let SW_TFCB_T go */
        nicTxReturnTFCB(prAdapter, prSwTfcb);

        if (!prMsduInfo) {
            ASSERT(0);
            break;
        }

        //4 <3> Get the associated STA_RECORD_T, update activity time &  current rate for BT coexist
        /* NOTE(Kevin 2008/1/16): The WLAN Index will be valid after the media status
         * is valid.
         */
        if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {

            ucWlanIndex = HAL_GET_MPDU_WLAN_INDEX(&rTxStatus);

            prTxCtrl->ucLastTxWlanIndex = ucWlanIndex;

            prStaRec = nicPrivacyGetStaRecordByWlanIndex(prAdapter, ucWlanIndex);

            if (prStaRec) {

                if (HAL_IS_TX_STATUS_OK(&rTxStatus)) {
                    /* Update the record activity time. */
                    GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);
                }

#if CFG_TX_FRAGMENT
                if (prAdapter->rConnSettings.fgIsEnableTxAutoFragmentForBT) {

                    /* We won't update the current rate index if it use BR */
                    if (!HAL_IS_TX_STATUS_BR_PKT(&rTxStatus)) {
                        if (prStaRec) {
                            halARGetRate(prAdapter,
                                         ucWlanIndex,
                                         &prStaRec->ucCurrRate1Index);

                            prTxCtrl->ucCurrRateIndex = prStaRec->ucCurrRate1Index;

                            DBGLOG(TX, INFO, ("WLAN_INDEX = %d, Current Rate1Index = %s\n",
                                ucWlanIndex, apucDebugRateIndex[prStaRec->ucCurrRate1Index]));
                        }
                    }
                }
#endif /* CFG_TX_FRAGMENT */

            }
        }

        //4 <4> 1X ARB Event to security module.
        if (prMsduInfo->ucFragFinishedCount == prMsduInfo->ucFragTotalCount) {
            if (HAL_IS_TX_STATUS_1X_PKT(&rTxStatus))
                secFsmRunEventEapolTxDone(prAdapter);
        }

        //4 <5> Update the Fragment Counter. e.g if value == 1, means one frag remains in FIFO.
        if (prMsduInfo->ucFragFinishedCount > 0) {
            prMsduInfo->ucFragFinishedCount--;
        }
#if DBG
        else {
            ASSERT(0);
        }
#endif /* DBG */


        //4 <6> Update TX Statistic Counter.
        TX_INC_CNT(prTxCtrl, TX_MPDU_TOTAL_COUNT);

        u4TxCount = HAL_GET_MPDU_TX_CNT(&rTxStatus);
        TX_ADD_CNT(prTxCtrl, TX_MPDU_TOTAL_RETRY_COUNT, u4TxCount);

        TX_ADD_CNT(prTxCtrl, TX_MPDU_RTS_OK_COUNT, \
            HAL_GET_MPDU_RTS_OK_CNT(&rTxStatus))

        TX_ADD_CNT(prTxCtrl, TX_MPDU_RTS_FAIL_COUNT, \
            HAL_GET_MPDU_RTS_FAIL_CNT(&rTxStatus))

        if (HAL_IS_TX_STATUS_OK(&rTxStatus)) {
            TX_INC_CNT(prTxCtrl, TX_MPDU_OK_COUNT);

            if (u4TxCount >= 2) {
                if (u4TxCount == 2) {
                    TX_INC_CNT(prTxCtrl, TX_MPDU_TX_TWICE_OK_COUNT);
                }
                else {
                    TX_INC_CNT(prTxCtrl, TX_MPDU_TX_MORE_TWICE_OK_COUNT);
                }
            }

            if (HAL_IS_TX_STATUS_BMC(&rTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_BMCAST_MPDU_OK_COUNT);
            }
            else {
                TX_INC_CNT(prTxCtrl, TX_UCAST_MPDU_OK_COUNT);
            }
        }
        else {
            TX_INC_CNT(prTxCtrl, TX_MPDU_ALL_ERR_COUNT);

            if (HAL_IS_TX_STATUS_MPDU_RETRY_ERR(&rTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_MPDU_RETRY_ERR_COUNT);

                TX_INC_CNT(prTxCtrl, TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT);
            }
            else if (HAL_IS_TX_STATUS_RTS_RETRY_ERR(&rTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_RTS_RETRY_ERR_COUNT);

                TX_INC_CNT(prTxCtrl, TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT);
            }
            else if (HAL_IS_TX_STATUS_LIFETIME_ERR(&rTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_LIFETIME_ERR_COUNT);
            }
            else if (HAL_IS_TX_STATUS_PORT_CTRL(&rTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_PORT_CTRL_COUNT);
            }

            if (HAL_IS_TX_STATUS_BMC(&rTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_BMCAST_MPDU_FAIL_COUNT);
            }
            else {
                TX_INC_CNT(prTxCtrl, TX_UCAST_MPDU_FAIL_COUNT);
            }

            DBGLOG(TX, INFO, ("TFCB tx fail, status = %08x\n",
                rTxStatus.ucStatusIndicationPID &= TX_STATUS_STATUS_MASK));

            if (prMsduInfo->fgIsTxFailed == FALSE) {
                prMsduInfo->fgIsTxFailed = TRUE;
            }
        }


        //4 <7> Store current PS state by querying HW and send ARB Event later.
        *pfgPsBit = HAL_GET_CURRENT_PS_STATE(&rTxStatus);


        /* There is no associated TFCB waiting for TX_DONE and stuck in the activeChainList */
        if (prMsduInfo->ucFragFinishedCount == 0) {
            QUEUE_INSERT_TAIL(prMsduInfoDoneList, &prMsduInfo->rQueEntry);
        }

    }
    while (HAL_IS_MORE_TX_STATUS(&rTxStatus));


    return u4TfcbNum;

} /* end of nicTxProcessTFCBs() */


#else /* CFG_SDIO_STATUS_ENHANCE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ UINT_32
nicTxSDIOProcessTFCBs (
    IN P_ADAPTER_T  prAdapter,
    IN P_QUE_T      prMsduInfoDoneList,
    OUT PBOOLEAN    pfgPsBit
    )
{
    P_SDIO_CTRL_T prSDIOCtrl;
    P_TX_CTRL_T prTxCtrl;
    P_TX_STATUS_T prTxStatus;
    P_SW_TFCB_T prSwTfcb;
    P_MSDU_INFO_T prMsduInfo;
    UINT_32 u4TfcbNum = 0;
    UINT_32 u4TxCount;
    P_STA_RECORD_T prStaRec;
    UINT_8 ucWlanIndex;
    UINT_32 i;

    DEBUGFUNC("nicTxSDIOProcessTFCBs");


    ASSERT(prAdapter);
    ASSERT(prMsduInfoDoneList);
    ASSERT(pfgPsBit);
    prSDIOCtrl = &prAdapter->rSDIOCtrl;
    prTxCtrl = &prAdapter->rTxCtrl;


    for (i = 0; i < SDIO_MAXIMUM_TX_STATUS; i++) {

        //4 <1> Get a TX_STATUS from SDIO_CTRL_T.
        prTxStatus = &prSDIOCtrl->arTxStatus[i];
        if (!(*(PUINT_32)prTxStatus)) {
            continue;
        }

        //4 <2> Get the associated SW_TFCB_T back.
        if (halSDIOTxProcessTxStatus(prAdapter, prTxStatus, &prSwTfcb) == FALSE) {
            continue;
        }

        u4TfcbNum++;

        //4 <3> Get the associated MSDU_INFO_T and free the SW_TFCB_T.
        prMsduInfo = prSwTfcb->prMsduInfo;

        /* Time to let SW_TFCB_T go */
        nicTxReturnTFCB(prAdapter, prSwTfcb);

        if (!prMsduInfo) {
            ASSERT(0);
            continue;
        }

        //4 <4> Get the associated STA_RECORD_T, update activity time &  current rate for BT coexist
        /* NOTE(Kevin 2008/1/16): The WLAN Index will be valid after the media status
         * is valid.
         */
        if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {

            ucWlanIndex = HAL_GET_MPDU_WLAN_INDEX(prTxStatus);

            prTxCtrl->ucLastTxWlanIndex = ucWlanIndex; /* Update last Tx Wlan Index */

            prStaRec = nicPrivacyGetStaRecordByWlanIndex(prAdapter, ucWlanIndex);

            if (prStaRec) {

                if (HAL_IS_TX_STATUS_OK(prTxStatus)) {
                    /* Update the record activity time. */
                    GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);
                }

#if CFG_TX_FRAGMENT
                if (prAdapter->rConnSettings.fgIsEnableTxAutoFragmentForBT) {

                    /* We won't update the current rate index if it use BR */
                    if (!HAL_IS_TX_STATUS_BR_PKT(prTxStatus)) {
                        if (prStaRec) {
                            halARGetRate(prAdapter,
                                         ucWlanIndex,
                                         &prStaRec->ucCurrRate1Index);

                            prTxCtrl->ucCurrRateIndex = prStaRec->ucCurrRate1Index;

                            DBGLOG(TX, INFO, ("WLAN_INDEX = %d, Current Rate1Index = %s\n",
                                ucWlanIndex, apucDebugRateIndex[prStaRec->ucCurrRate1Index]));
                        }
                    }
                }
#endif /* CFG_TX_FRAGMENT */

            }
        }

        //4 <5> 1X ARB Event to security module.
        if (prMsduInfo->ucFragFinishedCount == prMsduInfo->ucFragTotalCount) {
            if (HAL_IS_TX_STATUS_1X_PKT(prTxStatus))
                secFsmRunEventEapolTxDone(prAdapter);
        }

        //4 <6> Update the Fragment Counter. e.g if value == 1, means one frag remains in FIFO.
        if (prMsduInfo->ucFragFinishedCount > 0) {
            prMsduInfo->ucFragFinishedCount--;
        }
#if DBG
        else {
            ASSERT(0);
        }
#endif /* DBG */

        //4 <7> Update TX Statistic Counter.
        TX_INC_CNT(prTxCtrl, TX_MPDU_TOTAL_COUNT);

        u4TxCount = HAL_GET_MPDU_TX_CNT(prTxStatus);
        TX_ADD_CNT(prTxCtrl, TX_MPDU_TOTAL_RETRY_COUNT, u4TxCount);

        TX_ADD_CNT(prTxCtrl, TX_MPDU_RTS_OK_COUNT, \
            HAL_GET_MPDU_RTS_OK_CNT(prTxStatus))

        TX_ADD_CNT(prTxCtrl, TX_MPDU_RTS_FAIL_COUNT, \
            HAL_GET_MPDU_RTS_FAIL_CNT(prTxStatus))

        if (HAL_IS_TX_STATUS_OK(prTxStatus)) {
            TX_INC_CNT(prTxCtrl, TX_MPDU_OK_COUNT);

            if (u4TxCount >= 2) {
                if (u4TxCount == 2) {
                    TX_INC_CNT(prTxCtrl, TX_MPDU_TX_TWICE_OK_COUNT);
                }
                else {
                    TX_INC_CNT(prTxCtrl, TX_MPDU_TX_MORE_TWICE_OK_COUNT);
                }
            }

            if (HAL_IS_TX_STATUS_BMC(prTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_BMCAST_MPDU_OK_COUNT);
            }
            else {
                TX_INC_CNT(prTxCtrl, TX_UCAST_MPDU_OK_COUNT);
            }
        }
        else {
            TX_INC_CNT(prTxCtrl, TX_MPDU_ALL_ERR_COUNT);

            if (HAL_IS_TX_STATUS_MPDU_RETRY_ERR(prTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_MPDU_RETRY_ERR_COUNT);

                TX_INC_CNT(prTxCtrl, TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT);
            }
            else if (HAL_IS_TX_STATUS_RTS_RETRY_ERR(prTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_RTS_RETRY_ERR_COUNT);

                TX_INC_CNT(prTxCtrl, TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT);
            }
            else if (HAL_IS_TX_STATUS_LIFETIME_ERR(prTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_LIFETIME_ERR_COUNT);
            }
            else if (HAL_IS_TX_STATUS_PORT_CTRL(prTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_MPDU_PORT_CTRL_COUNT);
            }

            if (HAL_IS_TX_STATUS_BMC(prTxStatus)) {
                TX_INC_CNT(prTxCtrl, TX_BMCAST_MPDU_FAIL_COUNT);
            }
            else {
                TX_INC_CNT(prTxCtrl, TX_UCAST_MPDU_FAIL_COUNT);
            }

            DBGLOG(TX, INFO, ("TFCB tx fail, status = %08x\n",
                prTxStatus->ucStatusIndicationPID &= TX_STATUS_STATUS_MASK));

            if (prMsduInfo->fgIsTxFailed == FALSE) {
                prMsduInfo->fgIsTxFailed = TRUE;
            }
        }


        //4 <8> Store current PS state by querying HW and send ARB Event later.
        *pfgPsBit = HAL_GET_CURRENT_PS_STATE(prTxStatus);

        /* There is no associated TFCB waiting for TX_DONE and stuck in the activeChainList */
        if (prMsduInfo->ucFragFinishedCount == 0) {
            QUEUE_INSERT_TAIL(prMsduInfoDoneList, &prMsduInfo->rQueEntry);
        }

        //4 <9> Clear TX_STATUS in STATUS Buffer.
        kalMemZero(prTxStatus, sizeof(TX_STATUS_T));

    }


    return u4TfcbNum;

} /* end of nicTxSDIOProcessTFCBs() */

#endif /* CFG_SDIO_STATUS_ENHANCE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTxInterrupt(
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    QUE_T rMsduInfoTxDoneQueue;
    P_MSDU_INFO_T prMsduInfo;
    BOOLEAN fgPsState = FALSE;
    UINT_32 u4TfcbNum = 0;
    UINT_32 u4DataPktTxOkCnt = 0;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    //4 <1> Reset the returned MSDU_INFO_T queue.
    QUEUE_INITIALIZE(&rMsduInfoTxDoneQueue);

    //4 <2> Collect MSDU_INFO_T if all the associated MPDUs(SW_TFCB_T) get TX_DONE.
#if !CFG_SDIO_STATUS_ENHANCE
    u4TfcbNum = nicTxProcessTFCBs(prAdapter, &rMsduInfoTxDoneQueue, &fgPsState);
#else
    u4TfcbNum = nicTxSDIOProcessTFCBs(prAdapter, &rMsduInfoTxDoneQueue, &fgPsState);
#endif /* CFG_SDIO_STATUS_ENHANCE */

    //4 <3> Report FIFO empty event to ARB
    nicTxSignalFifoEmptyEvent(prAdapter);

    //4 <4> Start processing MSDU_INFO_T if have
    QUEUE_REMOVE_HEAD(&rMsduInfoTxDoneQueue, prMsduInfo, P_MSDU_INFO_T);
    while (prMsduInfo) {
        PVOID pvPacket = prMsduInfo->pvPacket;

        //4 <5> Trigger call back function
        if (prMsduInfo->pfTxDoneHandler) {
            prMsduInfo->pfTxDoneHandler(prAdapter, prMsduInfo,
                (prMsduInfo->fgIsTxFailed ? WLAN_STATUS_FAILURE : WLAN_STATUS_SUCCESS));
        }

        //4 <6> Return the associated memory handle
        if (prTxCtrl->fgIsRfTestTxMode) {
            rftestBufFreeRFTestBuf(prAdapter, (P_RFTEST_TXBUF_T)pvPacket);
        }
        else if (prMsduInfo->fgIsFromInternalProtocolStack) {
            mgtBufFreeMgtPacket(prAdapter, (P_MGT_PACKET_T)pvPacket);
        }
        else {

#if CFG_TX_RET_TX_CTRL_EARLY
            if (pvPacket)
#endif /* CFG_TX_RET_TX_CTRL_EARLY */
            {
                kalSendComplete(prAdapter->prGlueInfo,
                                pvPacket,
                                (prMsduInfo->fgIsTxFailed ? WLAN_STATUS_FAILURE : WLAN_STATUS_SUCCESS));
            }

            if (!prMsduInfo->fgIsTxFailed) {
                UINT_64 u8FrameLength;

                u8FrameLength = (UINT_64)prMsduInfo->ucMacHeaderLength;
                u8FrameLength += (UINT_64)prMsduInfo->u2PayloadLength;

                TX_ADD_CNT(prTxCtrl, TX_MSDU_BYTES_COUNT, u8FrameLength);
                TX_INC_CNT(prTxCtrl, TX_MSDU_OK_COUNT);

                u4DataPktTxOkCnt++;
            }
            else {
                TX_INC_CNT(prTxCtrl, TX_MSDU_FAIL_COUNT);
            }
        }

        //4 <7> Return this MSDU_INFO_T
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);

        QUEUE_REMOVE_HEAD(&rMsduInfoTxDoneQueue, prMsduInfo, P_MSDU_INFO_T);
    };


#if CFG_IBSS_POWER_SAVE
    //4 <8> trigger STA wait queue
    if (PM_IS_UNDER_IBSS_POWER_SAVE_MODE(prAdapter)) {
        if (PM_CHECK_IF_OUTSIDE_ATIM_WINDOW(prAdapter)) {

            //4 <9> Retransmit frames in rSendWaitQueue[]
            nicTxRetransmitOfSendWaitQue(prAdapter);

            //4 <10> Retransmit frames in arGcStaWaitQueue[] && arStaWaitQueue[]
            if (nicTxRetransmitOfStaWaitQue(prAdapter)) {
                /* Only trying to invoke this function at ATIM timeup event */
                //pmDisableIbssPsTx(prAdapter);
            }

            //4 <11> Retransmit frames in rOsSendQueue[]
            if (prTxCtrl->fgIsPacketInOsSendQueue) {
                nicTxRetransmitOfOsSendQue(prAdapter);
            }

    #if CFG_SDIO_TX_ENHANCE
            halTxLeftClusteredMpdu(prAdapter);
    #endif /* CFG_SDIO_TX_ENHANCE */

        }
    }
    else {
        //4 <9> Retransmit frames in rSendWaitQueue[]
        nicTxRetransmitOfSendWaitQue(prAdapter);

        //4 <11> Retransmit frames in rOsSendQueue[]
        if (prTxCtrl->fgIsPacketInOsSendQueue) {
            nicTxRetransmitOfOsSendQue(prAdapter);
        }

    #if CFG_SDIO_TX_ENHANCE
        halTxLeftClusteredMpdu(prAdapter);
    #endif /* CFG_SDIO_TX_ENHANCE */

    }
#else
    //4 <9> Retransmit frames in rSendWaitQueue[]
    nicTxRetransmitOfSendWaitQue(prAdapter);

    //4 <11> Retransmit frames in rOsSendQueue[]
    if (prTxCtrl->fgIsPacketInOsSendQueue) {
        nicTxRetransmitOfOsSendQue(prAdapter);
    }

    #if CFG_SDIO_TX_ENHANCE
    halTxLeftClusteredMpdu(prAdapter);
    #endif /* CFG_SDIO_TX_ENHANCE */

#endif /* CFG_IBSS_POWER_SAVE */

    //4 <12> Update Current TX Rate to KAL.
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        OS_SYSTIME rCurrentTime;

        GET_CURRENT_SYSTIME(&rCurrentTime);
        if (CHECK_FOR_TIMEOUT(rCurrentTime,
                              prTxCtrl->rCurrRateLastUpdateTime,
                              SEC_TO_SYSTIME(UPDATE_CURRENT_TX_RATE_INTERVAL_SEC))) {

            nicTxGetCurrentTxDataRate(prAdapter);
            prTxCtrl->rCurrRateLastUpdateTime = rCurrentTime;
        }
    }

    //4 <13> Do VOIP Flow Control
    nicTxVoipFlowCtrlCheckForSuspend(prAdapter);

    //4 <14> Indicate PM module for current PS state
    if (u4TfcbNum) {
        ARB_INDICATE_PS_STATUS(prAdapter, fgPsState);
    }

    //4 <15> Indicate PM module for TX data packet count, which is used under PSP-fast switching mode
    if (u4DataPktTxOkCnt) {
        ARB_INDICATE_DATA_FRAME_TRANSMITTED(prAdapter, u4DataPktTxOkCnt);
    }

    //4 <16> Do rftest tx
    if (prTxCtrl->fgIsRfTestTxMode) {
#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS
        ARB_SET_TIMER(prAdapter, prTxCtrl->rPktTxIntervalTimer, RFTEST_PKT_TX_INTERVAL);
#else
        rftestTx(prAdapter);
#endif
    }

    return;
} /* end of nicProcessTxInterrupt() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxCleanUpOsSendQue (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_QUE_T prOsSendQueue;
    P_QUE_ENTRY_T prQueueEntry;
    P_NATIVE_PACKET prPacket;
    INT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    for (i = TCM; i >= TC0; i--) {

        prOsSendQueue = &prTxCtrl->arTcQPara[i].rOsSendQueue;

        QUEUE_REMOVE_HEAD(prOsSendQueue, prQueueEntry, P_QUE_ENTRY_T);
        while (prQueueEntry) {

            prPacket = KAL_GET_PKT_DESCRIPTOR(prQueueEntry);
            ASSERT(prPacket);
            if(prPacket){
                kalSendComplete(prAdapter->prGlueInfo,
                    prPacket,
                    WLAN_STATUS_FAILURE);
            }
            QUEUE_REMOVE_HEAD(prOsSendQueue, prQueueEntry, P_QUE_ENTRY_T);
        }
    }

    return;
} /* end of nicTxCleanUpOsSendQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxCleanUpSendWaitQue (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_QUE_T prSendWaitQueue;
    P_MSDU_INFO_T prMsduInfo;
    INT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    for (i = TXQ_AC4; i >= TXQ_AC0; i--) {
        prSendWaitQueue = &prTxCtrl->arTxACQPara[i].rSendWaitQueue;

        QUEUE_REMOVE_HEAD(prSendWaitQueue, prMsduInfo, P_MSDU_INFO_T);
        while (prMsduInfo) {

            prMsduInfo->fgIsTxFailed = TRUE;

            nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

            QUEUE_REMOVE_HEAD(prSendWaitQueue, prMsduInfo, P_MSDU_INFO_T);
        }
    }

    return;
} /* end of nicTxCleanUpSendWaitQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxCleanUpActiveList (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* Flush the non empty ACQs and also stop the active ACQs */
    nicTxFlushStopQueues(prAdapter,
                         prTxCtrl->ucTxNonEmptyACQ,
                         prTxCtrl->ucTxActiveACQ);
    return;
} /* end of nicTxCleanUpActiveList() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxStartQueues (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucStartQues
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* Only the disabled one will be started */
    ucStartQues &= (~prTxCtrl->ucTxActiveACQ & TXQ_ALL_MASK);

    if (ucStartQues) {

        prTxCtrl->ucTxActiveACQ |= ucStartQues;

        halStartQueues(prAdapter, ucStartQues);
    }

    return;
} /* end of nicTxStartQueues() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxDisableTxQueueActiveState (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    prTxCtrl->ucTxActiveACQ &= ~(TXQ_DATA_MASK | TXQ_MGMT_MASK);

    return;
} /* end of nicTxDisableTxQueueActiveState() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxFlushStopQueues (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucFlushQues,
    IN UINT_8 ucStopQues
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
    P_QUE_T prActiveChainList;
    QUE_T rQueueBackup[TXQ_NUM];
    P_SW_TFCB_T prSwTfcb;
    P_MSDU_INFO_T prMsduInfo;
    UINT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* Only the active one can be stopped */
    ucStopQues &= prTxCtrl->ucTxActiveACQ;

    /* We can flush any queue even if it has been stopped */
    if (ucFlushQues) {
        /* Some queues will been stopped, remove from Active Queue Status */
        if (ucStopQues) {
            prTxCtrl->ucTxActiveACQ &= ~ucStopQues;
        }

        /* Do flush & stop first */
        halFlushStopQueues(prAdapter, ucFlushQues, ucStopQues);

        /* Immediately clean out the Active Chain List and backup those SW_TFCB_Ts */
        for (i = TXQ_AC0; i < TXQ_NUM; i++) {
            if (ucFlushQues & BIT(i)) {
                prTxACQPara = &prTxCtrl->arTxACQPara[i];
                prActiveChainList = &prTxACQPara->rActiveChainList;

                if (QUEUE_IS_NOT_EMPTY(prActiveChainList)) {
                    QUEUE_MOVE_ALL((&rQueueBackup[i]), prActiveChainList);
                }
                else {
                    QUEUE_INITIALIZE(&rQueueBackup[i]);
                }
            }
            else {
                QUEUE_INITIALIZE(&rQueueBackup[i]);
            }
        }

#if CFG_SDIO_STATUS_ENHANCE
        {
            P_SDIO_CTRL_T prSDIOCtrl = &prAdapter->rSDIOCtrl;
            P_TX_STATUS_T prTxStatus;

            for (i = 0; i < SDIO_MAXIMUM_TX_STATUS; i++) {
                prTxStatus = &prSDIOCtrl->arTxStatus[i];

                if ((*(PUINT_32)prTxStatus) &&
                    (BIT(HAL_TX_STATUS_GET_QUEUE_INDEX(prTxStatus)) & ucFlushQues)) {

                    kalMemZero(prTxStatus, sizeof(TX_STATUS_T));
                }
            }
        }
#endif /* CFG_SDIO_STATUS_ENHANCE */

        /* Update the bitmap of "Nonempty AC Queues" */
        prTxCtrl->ucTxNonEmptyACQ &= ~ucFlushQues;

        /* Process and return SW_TFCB_T */
        for (i = TXQ_AC0; i < TXQ_NUM; i++) {
            if (ucFlushQues & BIT(i)) {
                while (QUEUE_IS_NOT_EMPTY(&rQueueBackup[i])) {
                    QUEUE_REMOVE_HEAD((&rQueueBackup[i]), prSwTfcb, P_SW_TFCB_T);

                    ASSERT(prSwTfcb);
                    /* Get back the mother MSDU_INFO_T */
                    prMsduInfo = prSwTfcb->prMsduInfo;

                    ASSERT(prMsduInfo);
                    ASSERT(prMsduInfo->ucFragFinishedCount > 0);

                    prMsduInfo->ucFragFinishedCount--;

                    /* Time to let SW_TFCB_T go */
                    nicTxReturnTFCB(prAdapter, prSwTfcb);

                    /* TX fail due to flushed */
                    prMsduInfo->fgIsTxFailed = TRUE;

                    /* Discard MSDU_INFO_T */
                    if (prMsduInfo->ucFragFinishedCount == 0) {
                        nicTxDiscardMsduInfo(prAdapter, prMsduInfo);
                    }
                }
            }
        }

        /* Process and return MSDU_INFO_T in rSendWaitQueue */
        for (i = 0; i < TXQ_NUM; i++) {
            if (ucFlushQues & BIT(i)) {
                P_QUE_T prSendWaitQueue;


                prTxACQPara = &prTxCtrl->arTxACQPara[i];
                prSendWaitQueue = &prTxACQPara->rSendWaitQueue;

                QUEUE_REMOVE_HEAD(prSendWaitQueue, prMsduInfo, P_MSDU_INFO_T);
                while (prMsduInfo) {

                    prMsduInfo->fgIsTxFailed = TRUE;

                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                    QUEUE_REMOVE_HEAD(prSendWaitQueue, prMsduInfo, P_MSDU_INFO_T);
                }
            }
        }

    }
    else if (ucStopQues) {
        /* Some queues will been stopped, remove from Active Queue Status */
        prTxCtrl->ucTxActiveACQ &= ~ucStopQues;

        halFlushStopQueues(prAdapter, ucFlushQues, ucStopQues);
    }

    return;

} /* end of nicTxFlushStopQueues() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxRefreshQueues (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
    P_QUE_T prActiveChainList;
    QUE_T rQueueBackup;
    QUE_T rAllocatedTFCBList;

    P_SW_TFCB_T prSwTfcb;
    P_MSDU_INFO_T prMsduInfo;
    INT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;


    /* We assume this function will be called by Logic Reset,
     * Update the bitmap of "Nonempty AC Queues" to 0 first before refill frames.
     */
    prTxCtrl->ucTxNonEmptyACQ = 0;

    /* Assume the caller is Logic Reset Handler */
    for (i = (TXQ_NUM - 1); i >= 0; i--) {
        prTxACQPara = &prTxCtrl->arTxACQPara[i];
        prActiveChainList = &prTxACQPara->rActiveChainList;

        if (QUEUE_IS_EMPTY(prActiveChainList)) {
            continue;
        }

        QUEUE_MOVE_ALL(&rQueueBackup, prActiveChainList);

        do {

            QUEUE_REMOVE_HEAD(&rQueueBackup, prSwTfcb, P_SW_TFCB_T);

            if (!prSwTfcb) {
                break;
            }

            /* Get back the mother MSDU_INFO_T */
            prMsduInfo = prSwTfcb->prMsduInfo;

            ASSERT(prMsduInfo);
            ASSERT(prMsduInfo->ucFragFinishedCount > 0);

            QUEUE_INITIALIZE(&rAllocatedTFCBList);

#if CFG_TX_FRAGMENT
            if (prMsduInfo->ucFragTotalCount > 1) { /* Has more than two MPDUs */

                //4 <1> Get More SW_TFCB_T belong to this MSDU_INFO_T from Active Chain List.
                QUEUE_INSERT_TAIL(&rAllocatedTFCBList, &prSwTfcb->rQueEntry);

                if (prMsduInfo->ucFragFinishedCount > 0) {
                    prMsduInfo->ucFragFinishedCount--;
                }
#if DBG
                else {
                    ASSERT(0);
                }
#endif /* DBG */

                while (prMsduInfo->ucFragFinishedCount > 0) {

                    QUEUE_REMOVE_HEAD(&rQueueBackup, prSwTfcb, P_SW_TFCB_T);

                    if (!prSwTfcb) {
                        ASSERT(0); /* We should have more SW_TFCB_T due to ucFragFinishedCount > 0. */
                        break;
                    }

                    if (prMsduInfo != prSwTfcb->prMsduInfo) {

                        QUEUE_INSERT_HEAD(&rQueueBackup, &prSwTfcb->rQueEntry);

                        ASSERT(0); /* Lost SW_TFCB_T ? */
                        break;
                    }
                    else {

                        QUEUE_INSERT_TAIL(&rAllocatedTFCBList, &prSwTfcb->rQueEntry);

                        prMsduInfo->ucFragFinishedCount--;
                    }
                }

                if (prMsduInfo->ucFragFinishedCount > 0) {
                    /* We should get all the SW_TFCB_T out for this MSDU_INFO_T */
                    prMsduInfo->ucFragFinishedCount = 0;
                    ASSERT(0);
                }

                //4 <2> Check if we hold all the SW_TFCB_Ts of this MSDU_INFO_T.
                /* Some fragments belong to this MSDU_INFO_T have got their TX_DONE and
                 * also been removed from ActiveChainList before refresh.
                 * We decide to drop the incomplete MSDU.
                 */
                if (rAllocatedTFCBList.u4NumElem != prMsduInfo->ucFragTotalCount) {

                    ASSERT(rAllocatedTFCBList.u4NumElem < prMsduInfo->ucFragTotalCount); /* ASSERT the ">" CASE */

                    /* Time to let SW_TFCB_T go */
                    nicTxReturnTFCBs(prAdapter, &rAllocatedTFCBList);

                    /* TX fail due to flushed */
                    prMsduInfo->fgIsTxFailed = TRUE;

                    /* Discard MSDU_INFO_T */
                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                    continue; /* Do next SW_TFCB_T */
                }
            }
            else
#endif /* CFG_TX_FRAGMENT */
            {
                QUEUE_INSERT_TAIL(&rAllocatedTFCBList, &prSwTfcb->rQueEntry);
            }

            /* Clear the Transmission Status Flag
             * Assume a MSDU has various MPDUs, if one of them is failed, then this MSDU is failed
             */
            prMsduInfo->fgIsTxFailed = FALSE;

            /* None of any fragment has been processed(write to HW FIFO) yet */
            prMsduInfo->ucFragFinishedCount = 0;

#if CFG_TX_RET_TX_CTRL_EARLY
            if (!prMsduInfo->pvPacket) {
                //DbgPrint("-->nicTxRefreshQueues(): prMsduInfo->pvPacket is NULL\n");

                /* Time to let SW_TFCB_T go */
                nicTxReturnTFCBs(prAdapter, &rAllocatedTFCBList);

                /* TX fail due to flushed */
                prMsduInfo->fgIsTxFailed = TRUE;

                /* Discard MSDU_INFO_T */
                nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                continue; /* Do next SW_TFCB_T */
            }
#endif /* CFG_TX_RET_TX_CTRL_EARLY && CFG_TX_RET_TX_CTRL_EARLY */


#if CFG_SDIO_TX_ENHANCE
            if (halTxTfcbs(prAdapter, prMsduInfo, &rAllocatedTFCBList, TRUE) == FALSE) {
#else
            if (halTxTfcbs(prAdapter, prMsduInfo, &rAllocatedTFCBList) == FALSE) {
#endif /* CFG_SDIO_TX_ENHANCE */

                /* <TODO> Kevin[2007/03/07]: Update TX BUS ERROR COUNTER here */

                nicTxReturnTFCBs(prAdapter, &rAllocatedTFCBList);

                prMsduInfo->fgIsTxFailed = TRUE;

                nicTxDiscardMsduInfo(prAdapter, prMsduInfo);
            }

        }
        while(TRUE);

    }

    return;
} /* end of nicTxRefreshQueues() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxAcquirePrivilegeOfTxQue (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucStartQues,
    IN UINT_8 ucStopQues,
    IN UINT_8 ucFlushQues
    )
{
    P_TX_CTRL_T prTxCtrl;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    /* There is only one Module can acquire TX Privilege at the same time. */
    ASSERT(prTxCtrl->ucTxSuspendACQ == 0);

    ASSERT(prTxCtrl->ucTxPrivilegeLockCount < MAX_TX_PRIVILEGE_LOCK_COUNT);
    if (prTxCtrl->ucTxPrivilegeLockCount++ == 0) {

        /* Backup the Active TX Queues for resuming the TX activities after the Privilege was released. */
        prTxCtrl->ucTxSuspendACQ = prTxCtrl->ucTxActiveACQ;

        /* Flush and stop the specified TX Queues first */
        nicTxFlushStopQueues(prAdapter, ucFlushQues, ucStopQues);

        /* Start the specified TX Queues */
        if (ucStartQues) {
            nicTxStartQueues(prAdapter, ucStartQues);
        }
    }

    return;
} /* end of nicTxAcquirePrivilegeOfTxQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxReleasePrivilegeOfTxQue (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucFlushQues
    )
{
    P_TX_CTRL_T prTxCtrl;
    UINT_8 ucStartQues;
    UINT_8 ucStopQues;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    ASSERT(prTxCtrl->ucTxPrivilegeLockCount > 0);
    if ((prTxCtrl->ucTxPrivilegeLockCount) &&
        (--prTxCtrl->ucTxPrivilegeLockCount == 0)) {

        /* Restore the original activities of TX Queues after the Privilege was released. */
        ucStartQues = prTxCtrl->ucTxSuspendACQ & (~prTxCtrl->ucTxActiveACQ & TXQ_ALL_MASK);
        ucStopQues = (~prTxCtrl->ucTxSuspendACQ & TXQ_ALL_MASK) & prTxCtrl->ucTxActiveACQ;

        /* Clear the storage of Suspend TX Queues. */
        prTxCtrl->ucTxSuspendACQ = 0;

        /* Flush and stop the specified TX Queues first */
        nicTxFlushStopQueues(prAdapter, ucFlushQues, ucStopQues);

        /* Start the specified TX Queues */
        if (ucStartQues) {
            nicTxStartQueues(prAdapter, ucStartQues);
        }
    }

    return;
} /* end of nicTxReleasePrivilegeOfTxQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TC_Q_PARAMETERS_T prTcQPara;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
    PUINT_8 pucCurrBuf = pucBuffer;
    UINT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    if (pucBuffer) {} /* For Windows, we'll print directly instead of sprintf() */
    ASSERT(pu4Count);

    SPRINTF(pucCurrBuf, ("\nTX CTRL STATUS:"));
    SPRINTF(pucCurrBuf, ("\n==============="));
    SPRINTF(pucCurrBuf, ("\n                      TC0       TC1       TC2       TC3      TCS0       TC4"));
    SPRINTF(pucCurrBuf, ("\n---------------------------------------------------------------------------"));
    SPRINTF(pucCurrBuf, ("\nFREE MSDU LIST :"));
    prTcQPara = &prTxCtrl->arTcQPara[0];
    for (i = 0; i < TC_NUM; i++, prTcQPara++) {
        SPRINTF(pucCurrBuf,
            ("%4ld/%4d ", prTcQPara->rFreeMsduInfoList.u4NumElem,
                (int)prTcQPara->ucMaxNumOfMsduInfo));
    }
    SPRINTF(pucCurrBuf, ("\nOS SEND QUEUE  :"));
    prTcQPara = &prTxCtrl->arTcQPara[0];
    for (i = 0; i < TC_NUM; i++, prTcQPara++) {
        SPRINTF(pucCurrBuf, ("%9ld ", prTcQPara->rOsSendQueue.u4NumElem));
    }
    SPRINTF(pucCurrBuf, ("\n* fgIsPacketInOsSendQueue: %d\n",
        (int)prTxCtrl->fgIsPacketInOsSendQueue));


    SPRINTF(pucCurrBuf, ("\n                      AC0       AC1       AC2       AC3       TS0       AC4"));
    SPRINTF(pucCurrBuf, ("\n---------------------------------------------------------------------------"));
    SPRINTF(pucCurrBuf, ("\nMSDU SEND WAIT :"));
    prTxACQPara = &prTxCtrl->arTxACQPara[0];
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("%9ld ", prTxACQPara[i].rSendWaitQueue.u4NumElem));
    }

    SPRINTF(pucCurrBuf, ("\nTFCB FREE LIST :"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("%4ld/%4d ", prTxACQPara[i].rFreeTFCBList.u4NumElem,
                (int)prTxACQPara[i].ucMaxNumOfSwTfcb));
    }

    SPRINTF(pucCurrBuf, ("\nTFCB ACT CHAIN :"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("%9ld ", prTxACQPara[i].rActiveChainList.u4NumElem));
    }

    SPRINTF(pucCurrBuf, ("\nHW FIFO SIZE   :"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("%4d/%4d ", prTxACQPara[i].u2FreeBufferSizeDW,
                prTxACQPara[i].u2MaxBufferSizeDW));
    }

    SPRINTF(pucCurrBuf, ("\nACTIVE HW QUE  :"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("        %c ", ((prTxCtrl->ucTxActiveACQ & BIT(i)) ? '*' : ' ')));
    }

    SPRINTF(pucCurrBuf, ("\nNONEMPTY HW QUE:"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("        %c ", ((prTxCtrl->ucTxNonEmptyACQ & BIT(i)) ? '*' : ' ')));
    }
    SPRINTF(pucCurrBuf, ("\n"));

    *pu4Count = (UINT_32)((UINT_32)pucCurrBuf - (UINT_32)pucBuffer);

    return;
} /* end of nicTxQueryStatus() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxQueryStatistics (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    P_TX_CTRL_T prTxCtrl;
    PUINT_8 pucCurrBuf = pucBuffer;
    UINT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;
    if (pucBuffer) {} /* For Windows, we'll print directly instead of sprintf() */
    ASSERT(pu4Count);

#define SPRINTF_TX_COUNTER(eCounter) \
    SPRINTF(pucCurrBuf, ("%-35s : %"DBG_PRINTF_64BIT_DEC"\n", #eCounter, TX_GET_CNT(prTxCtrl, eCounter)))


    SPRINTF_TX_COUNTER(TX_OS_MSDU_COUNT);
    SPRINTF_TX_COUNTER(TX_OS_MSDU_DROP_COUNT);
    SPRINTF_TX_COUNTER(TX_INTERNAL_MSDU_MMPDU_COUNT);

    SPRINTF_TX_COUNTER(TX_MPDU_OK_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_RTS_OK_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_RTS_FAIL_COUNT);

    SPRINTF_TX_COUNTER(TX_BMCAST_MPDU_OK_COUNT);
    SPRINTF_TX_COUNTER(TX_UCAST_MPDU_OK_COUNT);
    SPRINTF_TX_COUNTER(TX_BMCAST_MPDU_FAIL_COUNT);
    SPRINTF_TX_COUNTER(TX_UCAST_MPDU_FAIL_COUNT);

    SPRINTF_TX_COUNTER(TX_MPDU_TX_TWICE_OK_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_TX_MORE_TWICE_OK_COUNT);

    SPRINTF_TX_COUNTER(TX_MPDU_ALL_ERR_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_PORT_CTRL_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_LIFETIME_ERR_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_RTS_RETRY_ERR_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_MPDU_RETRY_ERR_COUNT);

    SPRINTF_TX_COUNTER(TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT);

    SPRINTF_TX_COUNTER(TX_MPDU_TOTAL_COUNT);
    SPRINTF_TX_COUNTER(TX_MPDU_TOTAL_RETRY_COUNT);

    SPRINTF_TX_COUNTER(TX_BEACON_MMPDU_COUNT);
    SPRINTF_TX_COUNTER(TX_MSDU_BYTES_COUNT);
    SPRINTF_TX_COUNTER(TX_MSDU_OK_COUNT);
    SPRINTF_TX_COUNTER(TX_MSDU_FAIL_COUNT);

    SPRINTF(pucCurrBuf, ("\n\n                          AC0       AC1       AC2       AC3       TS0       AC4"));
    SPRINTF(pucCurrBuf, ("\n-------------------------------------------------------------------------------"));
    SPRINTF(pucCurrBuf, ("\nTX_ACQ_TFCB_COUNT  :"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("%9"DBG_PRINTF_64BIT_DEC" ", TX_ACQ_GET_CNT(prTxCtrl, i, TX_ACQ_TFCB_COUNT)));
    }

    SPRINTF(pucCurrBuf, ("\nTX_ACQ_TXDONE_COUNT:"));
    for (i = 0; i < TXQ_NUM; i++) {
        SPRINTF(pucCurrBuf,
            ("%9"DBG_PRINTF_64BIT_DEC" ", TX_ACQ_GET_CNT(prTxCtrl, i, TX_ACQ_TXDONE_COUNT)));
    }
    SPRINTF(pucCurrBuf, ("\n"));

    *pu4Count = (UINT_32)((UINT_32)pucCurrBuf - (UINT_32)pucBuffer);

    return;
} /* end of nicTxQueryStatistics() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxSetStatistics (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    UINT_32 i;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    TX_RESET_ALL_CNTS(prTxCtrl);

    for (i = TXQ_AC0; i < TXQ_NUM; i++) {
        TX_ACQ_RESET_ALL_CNTS(prTxCtrl, i);
    }

    return;
} /* end of nicTxSetStatistics() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxGetCurrentTxDataRate (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    UINT_8 ucCurrRateIndex;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;

    halARGetRate(prAdapter,
                 prTxCtrl->ucLastTxWlanIndex,
                 &ucCurrRateIndex);

    ASSERT(ucCurrRateIndex <= RATE_54M_INDEX);

    prTxCtrl->ucCurrRateIndex = ucCurrRateIndex;

    return;
} /* end of nicTxGetCurrentTxDataRate() */


#if CFG_IBSS_POWER_SAVE

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxReclaimTxPackets (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_TX_ACQ_PARAMETERS_T prTxACQPara;
#if !CFG_TX_RET_TX_CTRL_EARLY
    P_QUE_T prActiveChainList;
    P_SW_TFCB_T prSwTfcb;
#endif
    P_QUE_T prSendWaitQueue;
    QUE_T rQueueBackup[TXQ_NUM];

    P_MSDU_INFO_T prMsduInfo;
    P_STA_RECORD_T prStaRec;
    P_STA_INFO_T prStaInfo;
    QUE_T arQueueRetransmit[TXQ_NUM];
    P_QUE_ENTRY_T prQueueEntry;
    INT_32 i;

    DEBUGFUNC("nicTxReclaimTxPackets");


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;
    prStaInfo = &prAdapter->rStaInfo;

    for (i = TXQ_AC0; i <= TXQ_AC3; i++) {
        QUEUE_INITIALIZE(&arQueueRetransmit[i]);
    }

    /* Flush and stop all TX queues */
    /* NOTE(Kevin): Already sync with prTxCtrl->ucTxActiveACQ in function -
     * nicTxDisableTxQueueActiveState().
     */
#if CFG_TX_RET_TX_CTRL_EARLY
    /* NOTE: not to flush AC0~AC3 queue for preserving data frames stay inside HW FIFO.
             This is the workaround after CFG_TX_RET_TX_CTRL_EARLY compiling flag is enabled,
             which will not flush the data queues (and the packet is not able to be reclaimed
             due to it is returned to OS right after it is programmed to HW FIFO).
    */
    halFlushStopQueues(prAdapter, 0x0 /*(UINT_8)NULL*/, TXQ_DATA_MASK);
#else
    halFlushStopQueues(prAdapter, TXQ_DATA_MASK, TXQ_DATA_MASK);

    /* Update the bitmap of "Nonempty AC Queues" */
    prTxCtrl->ucTxNonEmptyACQ &= ~TXQ_DATA_MASK;

    /* Clean out the Active Chain List and backup those SW_TFCB_Ts */
    for (i = TXQ_AC0; i <= TXQ_AC3; i++) {
        prTxACQPara = &prTxCtrl->arTxACQPara[i];
        prActiveChainList = &prTxACQPara->rActiveChainList;

        if (QUEUE_IS_NOT_EMPTY(prActiveChainList)) {
            QUEUE_MOVE_ALL((&rQueueBackup[i]), prActiveChainList);
        }
        else {
            QUEUE_INITIALIZE(&rQueueBackup[i]);
        }
    }

    /* Reclaim TX packets from Active chain and Send wait queue */
    for (i = TXQ_AC3; i >= TXQ_AC0; i--) {
        while (QUEUE_IS_NOT_EMPTY(&rQueueBackup[i])) {
            QUEUE_REMOVE_HEAD(&rQueueBackup[i], prSwTfcb, P_SW_TFCB_T);

            /* Get back the mother MSDU_INFO_T */
            prMsduInfo = prSwTfcb->prMsduInfo;

            prStaRec = prMsduInfo->prStaRec;

            ASSERT(prMsduInfo->ucFragFinishedCount > 0);

            prMsduInfo->ucFragFinishedCount--;

            /* Time to let SW_TFCB_T go */
            nicTxReturnTFCB(prAdapter, prSwTfcb);

            if (prMsduInfo->ucFragFinishedCount == 0) {

                if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_BMCAST) {

                    /* Only queue valid packet (it will be NULL for data packets */
                    if (prMsduInfo->pvPacket) {
                        prQueueEntry = (P_QUE_ENTRY_T)
                            KAL_GET_PKT_QUEUE_ENTRY(prMsduInfo->pvPacket);

                        QUEUE_INSERT_TAIL(&prStaInfo->arGcStaWaitQueue[i],
                                          prQueueEntry);
                    }

                    /* detach MSDU_INFO and queue into STA wait queue */
                    nicTxReturnMsduInfo(prAdapter, prMsduInfo);

                    DBGLOG(LP, INFO, ("Enqueue packet: %#08lx to arGcStaWaitQueue[%ld] (num: %ld)\n",
                        (UINT_32)prMsduInfo->pvPacket, i, prStaInfo->arGcStaWaitQueue[i].u4NumElem));
                }
                else if (prStaRec) {
                    /* Only queue valid packet (it will be NULL for data packets */
                    if (prMsduInfo->pvPacket) {
                        prQueueEntry = (P_QUE_ENTRY_T)
                            KAL_GET_PKT_QUEUE_ENTRY(prMsduInfo->pvPacket);

                        QUEUE_INSERT_TAIL(&prStaRec->arStaWaitQueue[i],
                                          prQueueEntry);
                    }

                    /* detach MSDU_INFO and queue into STA wait queue */
                    nicTxReturnMsduInfo(prAdapter, prMsduInfo);

                    DBGLOG(LP, INFO, ("Enqueue packet: %#08lx to arStaWaitQueue[%ld] (num: %ld)\n",
                        (UINT_32)prMsduInfo->pvPacket, i, prStaRec->arStaWaitQueue[i].u4NumElem));
                }
                else {
                    QUEUE_INSERT_TAIL(&arQueueRetransmit[i], &prMsduInfo->rQueEntry);
                }
            }
        }
    }
#endif

    /* Process packets in Send Wait Queue */
    for (i = TXQ_AC0; i <= TXQ_AC3; i++) {
        prTxACQPara = &prTxCtrl->arTxACQPara[i];
        prSendWaitQueue = &prTxACQPara->rSendWaitQueue;

        if (QUEUE_IS_NOT_EMPTY(prSendWaitQueue)) {
            QUEUE_MOVE_ALL((&rQueueBackup[i]), prSendWaitQueue);
        }
        else {
            QUEUE_INITIALIZE(&rQueueBackup[i]);
        }
    }

    for (i = TXQ_AC3; i >= TXQ_AC0; i--) {

        QUEUE_REMOVE_HEAD(&rQueueBackup[i], prMsduInfo, P_MSDU_INFO_T);
        while (prMsduInfo) {

            prStaRec = prMsduInfo->prStaRec;

            if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_BMCAST) {
                /* Only queue valid packet (it will be NULL for data packets */
                if (prMsduInfo->pvPacket) {
                    prQueueEntry = (P_QUE_ENTRY_T)
                        KAL_GET_PKT_QUEUE_ENTRY(prMsduInfo->pvPacket);

                    QUEUE_INSERT_TAIL(&prStaInfo->arGcStaWaitQueue[i],
                                      prQueueEntry);
                }

                /* detach MSDU_INFO and queue in to STA wait queue */
                nicTxReturnMsduInfo(prAdapter, prMsduInfo);

                DBGLOG(LP, INFO, ("Enqueue packet: %#08lx to arGcStaWaitQueue[%ld] (num: %ld)\n",
                    (UINT_32)prMsduInfo->pvPacket, i, prStaInfo->arGcStaWaitQueue[i].u4NumElem));
            }
            else if (prStaRec) {
                /* Only queue valid packet (it will be NULL for data packets */
                if (prMsduInfo->pvPacket) {
                    prQueueEntry = (P_QUE_ENTRY_T)
                        KAL_GET_PKT_QUEUE_ENTRY(prMsduInfo->pvPacket);

                    QUEUE_INSERT_TAIL(&prStaRec->arStaWaitQueue[i],
                                      prQueueEntry);
                }

                /* detach MSDU_INFO and queue into STA wait queue */
                nicTxReturnMsduInfo(prAdapter, prMsduInfo);

                DBGLOG(LP, INFO, ("Enqueue packet: %#08lx to arStaWaitQueue[%ld] (num: %ld)\n",
                    (UINT_32)prMsduInfo->pvPacket, i, prStaInfo->arGcStaWaitQueue[i].u4NumElem));
            }
            else {
                QUEUE_INSERT_TAIL(&arQueueRetransmit[i], &prMsduInfo->rQueEntry);
            }

            QUEUE_REMOVE_HEAD(&rQueueBackup[i], prMsduInfo, P_MSDU_INFO_T);
        }
    }

    // send queued packets in a pile
    for (i = TXQ_AC3; i >= TXQ_AC0; i--) {
        while (QUEUE_IS_NOT_EMPTY(&arQueueRetransmit[i])) {
            QUEUE_REMOVE_HEAD(&arQueueRetransmit[i], prMsduInfo, P_MSDU_INFO_T);

#if CFG_SDIO_TX_ENHANCE
            if (nicTxService(prAdapter, prMsduInfo, FALSE) == WLAN_STATUS_FAILURE) {
#else
            if (nicTxService(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {
#endif /* CFG_SDIO_TX_ENHANCE */
                nicTxDiscardMsduInfo(prAdapter, prMsduInfo);
            }
        }
    }

    return;
} /* end of nicTxReclaimTxPackets() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicTxRetransmitOfStaWaitQue (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_STA_INFO_T    prStaInfo;
    P_STA_RECORD_T  prStaRec;
    P_LINK_T        prValidStaRecList;
    P_QUE_T         prWaitQueue;
    P_QUE_ENTRY_T   prQueueEntry;
    BOOLEAN         fgIsAllStaQueueEmpty = TRUE;

    P_NATIVE_PACKET prPacket;
    UINT_8 ucTID;

    UINT_8 ucMacHeaderLen;
    UINT_16 u2PayloadLen;
    OS_SYSTIME rArrivalTime;

    P_MSDU_INFO_T prMsduInfo;
    INT_32 i, j, k;

    DEBUGFUNC("nicTxRetransmitOfStaWaitQue");


    ASSERT(prAdapter);

    prStaInfo = &prAdapter->rStaInfo;

    DBGLOG(LP, INFO, ("\n"));

    if (!PM_IS_UNDER_IBSS_POWER_SAVE_MODE(prAdapter)) {
        return TRUE;
    }

    //4 <1> Dequeue Process for B/MCAST frame.
    for (i = TC3; i >= TC0; i--) {

        if (QUEUE_IS_NOT_EMPTY(&prStaInfo->arGcStaWaitQueue[i])) {
            fgIsAllStaQueueEmpty = FALSE;
        }

        if (prStaInfo->fgIsAllAdhocStaAwake) {

            prWaitQueue = &prStaInfo->arGcStaWaitQueue[i];
            while (QUEUE_IS_NOT_EMPTY(prWaitQueue)) {

                prQueueEntry = QUEUE_GET_HEAD(prWaitQueue);

                ASSERT(prQueueEntry);

                prPacket = KAL_GET_PKT_DESCRIPTOR(prQueueEntry);
                ucTID = KAL_GET_PKT_TID(prPacket);

                //4 <1> Skip converting the Priority Parameter/TID (User Priority/TSID) to Traffic Class(TC).
                //ucTC = aucPriorityParam2TC[ucTID];

                //DBGLOG(LP, INFO, ("ucTID = %d, ucTC = %ld\n", ucTID, i));

                // May acquire Spin Lock here.
                if ((prMsduInfo = nicTxAllocMsduInfo(prAdapter, (UINT_8)i)) != (P_MSDU_INFO_T)NULL) {
                    QUEUE_REMOVE_HEAD(prWaitQueue, prQueueEntry, P_QUE_ENTRY_T);
                }
                // May release Spin Lock here.

                if (!prMsduInfo) {
                    break;
                }

                //DBGLOG(LP, INFO, ("Dequeue packet: %#08lx from arGcStaWaitQueue[%ld] (num: %ld)\n",
                    //(UINT_32)prPacket, i, prWaitQueue->u4NumElem));

                ucMacHeaderLen = KAL_GET_PKT_HEADER_LEN(prPacket);
                u2PayloadLen = KAL_GET_PKT_PAYLOAD_LEN(prPacket);
                rArrivalTime = KAL_GET_PKT_ARRIVAL_TIME(prPacket);

                //4 <2> Initialization of MSDU_INFO_T.
                MSDU_INFO_OBJ_INIT(prMsduInfo, \
                                   ((ucMacHeaderLen >= WLAN_MAC_HEADER_LEN) ? TRUE : FALSE), \
                                   FALSE, \
                                   (PVOID)prPacket, \
                                   ucTID, \
                                   (UINT_8)i, \
                                   ucMacHeaderLen, \
                                   u2PayloadLen, \
                                   MSDU_INFO_CTRL_FLAG_BMCAST, \
                                   (PFN_TX_DONE_HANDLER)0, \
                                   rArrivalTime, \
                                   NULL \
                                   );


                //4 <3> Process MSDU Integrity Protection & Fragmentation
                if (txProcessMSDU(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {

                    prMsduInfo->fgIsTxFailed = TRUE;

                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                    break;
                }


                //4 <4> Forward MSDU_INFO_T to NIC Layer
#if CFG_SDIO_TX_ENHANCE
                if (nicTxService(prAdapter, prMsduInfo, TRUE) == WLAN_STATUS_FAILURE) {
#else
                if (nicTxService(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {
#endif

                    prMsduInfo->fgIsTxFailed = TRUE;

                    nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                    break;
                }
            }
        }
    }

    //4 <2> Dequeue Process for UCAST frame of IBSS member
    for (j = 0; j < (INT_32)STA_RECORD_HASH_NUM; j++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[j];

        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            for (k = TC3; k >= TC0; k--) {

                if (QUEUE_IS_NOT_EMPTY(&prStaRec->arStaWaitQueue[k])) {
                    fgIsAllStaQueueEmpty = FALSE;
                }

                if (prStaRec->fgIsAdhocStaAwake || prStaInfo->fgIsAllAdhocStaAwake) {

                    prWaitQueue = &prStaRec->arStaWaitQueue[k];
                    while (QUEUE_IS_NOT_EMPTY(prWaitQueue)) {

                        prQueueEntry = QUEUE_GET_HEAD(prWaitQueue);

                        ASSERT(prQueueEntry);

                        prPacket = KAL_GET_PKT_DESCRIPTOR(prQueueEntry);
                        ucTID = KAL_GET_PKT_TID(prPacket);

                        //4 <1> Skip converting the Priority Parameter/TID (User Priority/TSID) to Traffic Class(TC).
                        //ucTC = aucPriorityParam2TC[ucTID];

                        //DBGLOG(LP, INFO, ("ucTID = %d, ucTC = %ld\n", ucTID, k));

                        // May acquire Spin Lock here.
                        if ((prMsduInfo = nicTxAllocMsduInfo(prAdapter, (UINT_8)k)) != (P_MSDU_INFO_T)NULL) {
                            QUEUE_REMOVE_HEAD(prWaitQueue, prQueueEntry, P_QUE_ENTRY_T);
                        }
                        // May release Spin Lock here.

                        if (!prMsduInfo) {
                            break;
                        }

                        //DBGLOG(LP, INFO, ("Dequeue packet: %#08lx from arStaWaitQueue[%ld] (num: %ld)\n",
                            //(UINT_32)prPacket, k, prWaitQueue->u4NumElem));

                        ucMacHeaderLen = KAL_GET_PKT_HEADER_LEN(prPacket);
                        u2PayloadLen = KAL_GET_PKT_PAYLOAD_LEN(prPacket);
                        rArrivalTime = KAL_GET_PKT_ARRIVAL_TIME(prPacket);

                        //4 <2> Initialization of MSDU_INFO_T.
                        MSDU_INFO_OBJ_INIT(prMsduInfo, \
                                           ((ucMacHeaderLen >= WLAN_MAC_HEADER_LEN) ? TRUE : FALSE), \
                                           FALSE, \
                                           (PVOID)prPacket, \
                                           ucTID, \
                                           (UINT_8)k, \
                                           ucMacHeaderLen, \
                                           u2PayloadLen, \
                                           MSDU_INFO_CTRL_FLAG_NONE, \
                                           (PFN_TX_DONE_HANDLER)0, \
                                           rArrivalTime, \
                                           prStaRec \
                                           );


                        //4 <3> Process MSDU Integrity Protection & Fragmentation
                        if (txProcessMSDU(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {

                            prMsduInfo->fgIsTxFailed = TRUE;

                            nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                            break;
                        }


                        //4 <4> Forward MSDU_INFO_T to NIC Layer
#if CFG_SDIO_TX_ENHANCE
                        if (nicTxService(prAdapter, prMsduInfo, TRUE) == WLAN_STATUS_FAILURE) {
#else
                        if (nicTxService(prAdapter, prMsduInfo) == WLAN_STATUS_FAILURE) {
#endif

                            prMsduInfo->fgIsTxFailed = TRUE;

                            nicTxDiscardMsduInfo(prAdapter, prMsduInfo);

                            break;
                        }

                    }
                }
            }
        }
    }

    return fgIsAllStaQueueEmpty;

} /* end of nicTxRetransmitOfStaWaitQue() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicTxCleanUpStaWaitQue (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;

    P_QUE_T prWaitQueue;
    P_QUE_ENTRY_T prQueueEntry;
    P_NATIVE_PACKET prPacket;

    INT_32 i, j, k;


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;

    //4 <1> Clean out the Native Packet which are stalled in GcStaWaitQueue.
    for (i = TC3; i >= TC0; i--) {
        prWaitQueue = &prStaInfo->arGcStaWaitQueue[i];

        QUEUE_REMOVE_HEAD(prWaitQueue, prQueueEntry, P_QUE_ENTRY_T);
        while (prQueueEntry) {

            prPacket = KAL_GET_PKT_DESCRIPTOR(prQueueEntry);
            ASSERT(prPacket);
            if(prPacket) {
                kalSendComplete(prAdapter->prGlueInfo,
                    prPacket,
                    WLAN_STATUS_FAILURE);
            }
            QUEUE_REMOVE_HEAD(prWaitQueue, prQueueEntry, P_QUE_ENTRY_T);
        }
    }

    //4 <2> Clean out the Native Packet which are stalled in StaWaitQueue.
    for (j = 0; j < (INT_32)STA_RECORD_HASH_NUM; j++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[j];

        /* NOTE(Kevin): This is for the case if we insert module when the card is
         * NOT present. Driver will detect and do driver unload procedures immediately
         * if card was not found. Because the STA List will be initialized after
         * card found, so above case will cause an invalid Link Operation and
         * get hang.
         */
        if (LINK_IS_INVALID(prValidStaRecList)) {
            break;
        }

        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            for (k = TC3; k >= TC0; k--) {
                prWaitQueue = &prStaRec->arStaWaitQueue[k];

                QUEUE_REMOVE_HEAD(prWaitQueue, prQueueEntry, P_QUE_ENTRY_T);
                while (prQueueEntry) {

                    prPacket = KAL_GET_PKT_DESCRIPTOR(prQueueEntry);
                    ASSERT(prPacket);
                    if(prPacket) {
                        kalSendComplete(prAdapter->prGlueInfo,
                            prPacket,
                            WLAN_STATUS_FAILURE);
                    }
                    QUEUE_REMOVE_HEAD(prWaitQueue, prQueueEntry, P_QUE_ENTRY_T);
                }
            }
        }
    }

    return;
} /* end of nicTxCleanUpStaWaitQue() */

#endif /* CFG_IBSS_POWER_SAVE */



