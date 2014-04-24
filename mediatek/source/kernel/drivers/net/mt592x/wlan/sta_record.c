






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecInitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STA_INFO_T prStaInfo;
    PUINT_8 pucMemHandle;
    P_STA_RECORD_T prStaRec;
    UINT_32 i;


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;

    //4 <0> Clear allocated memory.
    kalMemZero((PVOID) prStaInfo->pucStaRecCached, prStaInfo->u4StaRecCachedSize);

    ASSERT(IS_ALIGN_4((UINT_32)prStaInfo->pucStaRecCached));

    pucMemHandle = prStaInfo->pucStaRecCached;

    LINK_INITIALIZE(&prStaInfo->rFreeStaRecList);

    for (i = 0; i < CFG_MAX_NUM_STA_RECORD; i++) {

        prStaRec = (P_STA_RECORD_T)pucMemHandle;

        STA_RECORD_SET_GUID(prStaRec);

        LINK_INSERT_TAIL(&prStaInfo->rFreeStaRecList, &prStaRec->rLinkEntry);

        pucMemHandle += ALIGN_4(sizeof(STA_RECORD_T));
    }

    /* Check if the memory allocation consist with this initialization function */
    ASSERT((UINT_32)(pucMemHandle - prStaInfo->pucStaRecCached) == prStaInfo->u4StaRecCachedSize);

    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        LINK_INITIALIZE(&prStaInfo->arValidStaRecList[i]);
    }

    prStaInfo->ucValidStaRecNum = 0;

    return;
} /* end of staRecInitialize() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_STA_RECORD_T
staRecGetStaRecordByAddr (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucStaAddr
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;


    ASSERT(prAdapter);
    ASSERT(pucStaAddr);
    prStaInfo = &prAdapter->rStaInfo;

    if (pucStaAddr) {
        /* To get the proper valid list through hash function */
        prValidStaRecList = &prStaInfo->arValidStaRecList[STA_RECORD_HASH_FUNC(pucStaAddr)];

        /* Check if such STA record exists in a valid list */
        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            if (EQUAL_MAC_ADDR(pucStaAddr, prStaRec->aucMacAddr)) {
                return prStaRec;
            }
        }
    }

    return (P_STA_RECORD_T)NULL;

} /* end of staRecGetStaRecordByAddr() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_STA_RECORD_T
staRecGetStaRecordByAddrOrAddIfNotExist (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucStaAddr
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_LINK_T prFreeStaRecList;
    P_STA_RECORD_T prStaRec;
    UINT_32 i;


    ASSERT(prAdapter);
    ASSERT(pucStaAddr);
    prStaInfo = &prAdapter->rStaInfo;

    //4 <1> To get the proper valid list through hash function
    prValidStaRecList = &prStaInfo->arValidStaRecList[STA_RECORD_HASH_FUNC(pucStaAddr)];

    //4 <2> Check if such STA record exists in a valid list
    LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

        STA_RECORD_CHK_GUID(prStaRec);

        if (EQUAL_MAC_ADDR(prStaRec->aucMacAddr, pucStaAddr)) {
            return prStaRec;
        }
    }

    //4 <2> No such Record, try to alloc an entry of BSS Desc for this new BSS.
    do {
        prFreeStaRecList = &prStaInfo->rFreeStaRecList;

        /* Attempt to allocate a free station record from the free list. */
        LINK_REMOVE_HEAD(prFreeStaRecList, prStaRec, P_STA_RECORD_T);
        if (prStaRec) {
            break;
        }

        //4 <2.1> Remove the timeout one.
        staRecRemoveStaRecordByPolicy(prAdapter, (STA_RECORD_RM_POLICY_EXCLUDE_STATE_3 | \
                                                  STA_RECORD_RM_POLICY_TIMEOUT));

        //4 <2.2> Try to get an entry again.
        LINK_REMOVE_HEAD(prFreeStaRecList, prStaRec, P_STA_RECORD_T);
        if (prStaRec) {
            break;
        }

        //4 <2.3> Remove the oldest one.
        staRecRemoveStaRecordByPolicy(prAdapter, (STA_RECORD_RM_POLICY_EXCLUDE_STATE_3 | \
                                                  STA_RECORD_RM_POLICY_OLDEST));

        //4 <2.4> Try to get an entry again.
        LINK_REMOVE_HEAD(prFreeStaRecList, prStaRec, P_STA_RECORD_T);
        if (prStaRec) {
            break;
        }

        /* TODO(Kevin): Actually we will encounter this case if all of the 32 STA_RECORD_Ts
         * are occupied by 1 AP + 31 DLS STAs. Because they are STATE_3.
         */
        ASSERT(prStaRec);
    }
    while (FALSE);


    if (prStaRec) {

        STA_RECORD_CHK_GUID(prStaRec);

        /* Clear the acquired station record. */
        kalMemZero((PVOID)prStaRec, sizeof(STA_RECORD_T));

        STA_RECORD_SET_GUID(prStaRec);

        /* Fill address. */
        COPY_MAC_ADDR(prStaRec->aucMacAddr, pucStaAddr);

        /* Fill the initial STA state. */
        prStaRec->ucStaState = STA_STATE_1;

        /* Fill the initial Status/Reason Code. */
        prStaRec->u2StatusCode = STATUS_CODE_SUCCESSFUL;
        prStaRec->u2ReasonCode = REASON_CODE_RESERVED;

        /* Fill the initial SEQ_CTRL_NUM of previous frame. */
        prStaRec->u2LastRxSeqCtrl = INVALID_SEQ_CTRL_NUM;
        for (i = 0; i < TID_NUM; i++) {
            prStaRec->u2TIDLastRxSeqCtrl[i] = INVALID_SEQ_CTRL_NUM;
        }

        /* Set the power management mode of the STA to Active mode. */
        //prStaRec->ucPowerMgtMode = PWR_MGT_MODE_AM;

        /* To get the proper valid list through hash function */
        prValidStaRecList = &prStaInfo->arValidStaRecList[STA_RECORD_HASH_FUNC(pucStaAddr)];

        /* Add this STA record to the valid STA record list -
         * Let the entry with recent activity in the head.
         */
        LINK_INSERT_HEAD(prValidStaRecList, &prStaRec->rLinkEntry);

        /* Increase the counter of valid STA record */
        prStaInfo->ucValidStaRecNum++;

        DBGLOG(MGT, TRACE, ("Add record for STA ["MACSTR"]\n", \
            MAC2STR(pucStaAddr)));

        DBGLOG(MGT, INFO, ("STA record allocated number = %d\n", \
            prStaInfo->ucValidStaRecNum));
    }

    return prStaRec;

} /* end of staRecGetStaRecordByAddrOrAddIfNotExist() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecRemoveStaRecordByPolicy (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4RemovePolicy
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;
    P_FRAG_INFO_T prFragInfo;
    UINT_32 i, j;

    DEBUGFUNC("staRecRemoveStaRecordByPolicy");


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;

    if (u4RemovePolicy & STA_RECORD_RM_POLICY_TIMEOUT) {
        P_STA_RECORD_T prStaRecNext;
        OS_SYSTIME rCurrentTime;


        GET_CURRENT_SYSTIME(&rCurrentTime);

        for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
            prValidStaRecList = &prStaInfo->arValidStaRecList[i];

            LINK_FOR_EACH_ENTRY_SAFE(prStaRec, prStaRecNext, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

                STA_RECORD_CHK_GUID(prStaRec);

                if ((u4RemovePolicy & STA_RECORD_RM_POLICY_EXCLUDE_STATE_3) &&
                    (prStaRec->ucStaState == STA_STATE_3)) {
                    /* NOTE: Don't remove the Peer STA which is in STATE_3.
                     * e.g. 1. AP, 2. DLS, 3. Default Entry(BSSID) for AdHoc Mode.
                     */
                    continue;
                }

                if (CHECK_FOR_TIMEOUT(rCurrentTime, prStaRec->rUpdateTime,
                    SEC_TO_SYSTIME(STA_RECORD_TIMEOUT_SEC))) {

                    for (j = 0; j < MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS; j++) {
                        prFragInfo = &prStaRec->rFragInfo[j];

                        if (prFragInfo->pr1stFrag) {
                            nicRxReturnRFB(prAdapter, prFragInfo->pr1stFrag);
                            prFragInfo->pr1stFrag = (P_SW_RFB_T)NULL;
                        }
                    }

                    /* Remove this BSS Desc from the BSS Desc list */
                    LINK_REMOVE_KNOWN_ENTRY(prValidStaRecList, prStaRec);

                    DBGLOG(MGT, TRACE, ("Remove Sta Record ["MACSTR"]\n", \
                        MAC2STR(prStaRec->aucMacAddr)));

                    /* Decrease the counter of valid STA record */
                    prStaInfo->ucValidStaRecNum--;

                    /* Return this BSS Desc to the free BSS Desc list. */
                    LINK_INSERT_TAIL(&prStaInfo->rFreeStaRecList, &prStaRec->rLinkEntry);
                }
            }
        }
    }
    else if (u4RemovePolicy & STA_RECORD_RM_POLICY_OLDEST) {
        P_STA_RECORD_T prStaRecOldest = (P_STA_RECORD_T)NULL;
        P_LINK_T prValidStaRecListOldest = (P_LINK_T)NULL;


        for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
            prValidStaRecList = &prStaInfo->arValidStaRecList[i];

            LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

                STA_RECORD_CHK_GUID(prStaRec);

                if ((u4RemovePolicy & STA_RECORD_RM_POLICY_EXCLUDE_STATE_3) &&
                    (prStaRec->ucStaState == STA_STATE_3)) {
                    /* NOTE: Don't remove the Peer STA which is in STATE_3.
                     * e.g. 1. AP, 2. DLS, 3. Default Entry(BSSID) for AdHoc Mode.
                     */
                    continue;
                }

                if (!prStaRecOldest) { /* 1st element */
                    prStaRecOldest = prStaRec;
                    prValidStaRecListOldest = prValidStaRecList;
                    continue;
                }

                if (TIME_BEFORE(prStaRec->rUpdateTime, prStaRecOldest->rUpdateTime)) {
                    prStaRecOldest = prStaRec;
                    prValidStaRecListOldest = prValidStaRecList;
                }
            }
        }

        if (prStaRecOldest) {

            for (j = 0; j < MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS; j++) {
                prFragInfo = &prStaRec->rFragInfo[j];

                if (prFragInfo->pr1stFrag) {
                    nicRxReturnRFB(prAdapter, prFragInfo->pr1stFrag);
                    prFragInfo->pr1stFrag = (P_SW_RFB_T)NULL;
                }
            }

            /* Remove this Sta Record from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prValidStaRecListOldest, prStaRecOldest);

            DBGLOG(MGT, TRACE, ("Remove Sta Record ["MACSTR"]\n", \
                MAC2STR(prStaRecOldest->aucMacAddr)));

            /* Decrease the counter of valid STA record */
            prStaInfo->ucValidStaRecNum--;

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prStaInfo->rFreeStaRecList, &prStaRecOldest->rLinkEntry);
        }
    }
    else if (u4RemovePolicy & STA_RECORD_RM_POLICY_ENTIRE) {
        P_STA_RECORD_T prStaRecNext;

        for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
            prValidStaRecList = &prStaInfo->arValidStaRecList[i];

            LINK_FOR_EACH_ENTRY_SAFE(prStaRec, prStaRecNext, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

                STA_RECORD_CHK_GUID(prStaRec);

                if ((u4RemovePolicy & STA_RECORD_RM_POLICY_EXCLUDE_STATE_3) &&
                    (prStaRec->ucStaState == STA_STATE_3)) {
                    /* NOTE: Don't remove the Peer STA which is in STATE_3.
                     * e.g. 1. AP, 2. DLS, 3. Default Entry(BSSID) for AdHoc Mode.
                     */
                    continue;
                }

                for (j = 0; j < MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS; j++) {
                    prFragInfo = &prStaRec->rFragInfo[j];

                    if (prFragInfo->pr1stFrag) {
                        nicRxReturnRFB(prAdapter, prFragInfo->pr1stFrag);
                        prFragInfo->pr1stFrag = (P_SW_RFB_T)NULL;
                    }
                }

                /* Remove this BSS Desc from the BSS Desc list */
                LINK_REMOVE_KNOWN_ENTRY(prValidStaRecList, prStaRec);

                DBGLOG(MGT, TRACE, ("Remove Sta Record ["MACSTR"]\n", \
                    MAC2STR(prStaRec->aucMacAddr)));

                /* Decrease the counter of valid STA record */
                prStaInfo->ucValidStaRecNum--;

                /* Return this BSS Desc to the free BSS Desc list. */
                LINK_INSERT_TAIL(&prStaInfo->rFreeStaRecList, &prStaRec->rLinkEntry);
            }
        }

    }

    return;
} /* end of staRecRemoveStaRecordByPolicy() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecRemoveStaRecordForIBSS (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;
    P_STA_RECORD_T prStaRecNext;
    P_FRAG_INFO_T prFragInfo;
    UINT_32 i, j;

    DEBUGFUNC("staRecRemoveStaRecordForIBSS");


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;


    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

        LINK_FOR_EACH_ENTRY_SAFE(prStaRec, prStaRecNext, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            if (prStaRec->ucStaState == STA_STATE_3) {
                /* NOTE: STA with STATE_3 in IBSS is represent as the Default Entry
                 * (BSSID), no such peer exist actually.
                 */
                continue;
            }

            for (j = 0; j < MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS; j++) {
                prFragInfo = &prStaRec->rFragInfo[j];

                if (prFragInfo->pr1stFrag) {
                    nicRxReturnRFB(prAdapter, prFragInfo->pr1stFrag);
                    prFragInfo->pr1stFrag = (P_SW_RFB_T)NULL;
                }
            }

            /* Remove this BSS Desc from the BSS Desc list */
            LINK_REMOVE_KNOWN_ENTRY(prValidStaRecList, prStaRec);

            DBGLOG(MGT, TRACE, ("Remove Sta Record ["MACSTR"]\n", \
                MAC2STR(prStaRec->aucMacAddr)));

            /* Decrease the counter of valid STA record */
            prStaInfo->ucValidStaRecNum--;

            /* Return this BSS Desc to the free BSS Desc list. */
            LINK_INSERT_TAIL(&prStaInfo->rFreeStaRecList, &prStaRec->rLinkEntry);
        }
    }

    return;
} /* end of staRecRemoveStaRecordForIBSS() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecRemoveStateFlagOfAllStaRecords (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;
    UINT_32 i;


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;

    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

        if (LINK_IS_VALID(prValidStaRecList)) {
            LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

                STA_RECORD_CHK_GUID(prStaRec);

                prStaRec->ucStaState = STA_STATE_1;
            }
        }
    }

    return;
} /* end of staRecRemoveStateFlagOfAllStaRecords() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecClearStatusAndReasonCode (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;
    UINT_32 i;


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;

    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

        /* Check if the prValidStaRecList is valid, skip if the prValidStaRecList has not been
         * initiated.
         */
        if (LINK_IS_VALID(prValidStaRecList)) {
            LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

                STA_RECORD_CHK_GUID(prStaRec);

                prStaRec->u2StatusCode = STATUS_CODE_SUCCESSFUL;
                prStaRec->u2ReasonCode = REASON_CODE_RESERVED;
                prStaRec->ucJoinFailureCount = 0;
            }
        }
    }

    return;
} /* end of staRecClearStatusAndReasonCode() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecCheckDefragBufOfAllStaRecords (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;
    P_FRAG_INFO_T prFragInfo;
    OS_SYSTIME rCurrentTime;
    P_RX_CTRL_T prRxCtrl;
    BOOLEAN fgIsDefragQueNonEmpty;
    UINT_32 i, j;

    DEBUGFUNC("staRecCheckDefragBufOfAllStaRecords");

    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;
    prRxCtrl = &prAdapter->rRxCtrl;

    GET_CURRENT_SYSTIME(&rCurrentTime);

    fgIsDefragQueNonEmpty = FALSE;

    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            for (j = 0; j < MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS; j++) {

                prFragInfo = &prStaRec->rFragInfo[j];

                if (prFragInfo->pr1stFrag) {

                    /* I. If the receive timer for the MSDU or MMPDU that is stored in the
                     * fragments queue exceeds dot11MaxReceiveLifetime, we discard the
                     * uncompleted fragments.
                     * II. If we didn't receive the last MPDU for a period, we use
                     * this function for remove frames.
                     */
                    if (CHECK_FOR_EXPIRATION(rCurrentTime,
                        prFragInfo->rReceiveLifetimeLimit)) {

                        nicRxReturnRFB(prAdapter, prFragInfo->pr1stFrag);

                        prFragInfo->pr1stFrag = (P_SW_RFB_T)NULL;
                    }
                    else {
                        fgIsDefragQueNonEmpty = TRUE;
                    }
                }
            }
        }
    }

    prRxCtrl->fgIsDefragQueNonEmpty = fgIsDefragQueNonEmpty;

    return;
} /* end of staRecCheckDefragBufOfAllStaRecords() */


#if CFG_DBG_STA_RECORD
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
staRecQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    P_STA_INFO_T prStaInfo;
    P_LINK_T prValidStaRecList;
    P_STA_RECORD_T prStaRec;
    PUINT_8 pucCurrBuf = pucBuffer;
    UINT_32 i;


    ASSERT(prAdapter);
    if (pucBuffer) {} /* For Windows, we'll print directly instead of sprintf() */
    ASSERT(pu4Count);
  
    prStaInfo = &prAdapter->rStaInfo;

    SPRINTF(pucCurrBuf, ("\n\nSTA RECORD STATUS:"));
    SPRINTF(pucCurrBuf, ("\n=================="));
#if CFG_IBSS_POWER_SAVE
    {
        UINT_32 j;

        SPRINTF(pucCurrBuf, ("\n                      TC0       TC1       TC2       TC3      TCS0       TC4"));
        SPRINTF(pucCurrBuf, ("\n---------------------------------------------------------------------------"));
        SPRINTF(pucCurrBuf, ("\nBMC WAIT QUEUE :"));
        for (j = 0; j < TC_NUM; j++) {
            SPRINTF(pucCurrBuf, ("%9ld ", prStaInfo->arGcStaWaitQueue[j].u4NumElem));
        }
        SPRINTF(pucCurrBuf, ("\n* fgIsAllAdhocStaAwake : %d",
            prStaInfo->fgIsAllAdhocStaAwake));
        SPRINTF(pucCurrBuf, ("\n"));
    }
#endif /* CFG_IBSS_POWER_SAVE */

    SPRINTF(pucCurrBuf, ("\nFREE STA RECORD LIST          :%8ld",
        prStaInfo->rFreeStaRecList.u4NumElem));

    SPRINTF(pucCurrBuf, ("\nTOTAL VALID STA RECORD NUM    :%8d",
        prStaInfo->ucValidStaRecNum));


    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

    SPRINTF(pucCurrBuf, ("\nVALID STA RECORD LIST[%ld]      :%8ld",
        i, prValidStaRecList->u4NumElem));

        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            SPRINTF(pucCurrBuf, ("\nSTA MAC ADDR ["MACSTR"]: \n    STATE = %s",
                MAC2STR(prStaRec->aucMacAddr),
                ((prStaRec->ucStaState == STA_STATE_3) ? "STA_STATE_3" :
                 ((prStaRec->ucStaState == STA_STATE_2) ? "STA_STATE_2" :
                  ((prStaRec->ucStaState == STA_STATE_1) ? "STA_STATE_1" : "STA_STATE_UNKNOWN"))) ));

            SPRINTF(pucCurrBuf, ("\n    STATUS CODE = %d", prStaRec->u2StatusCode));
            SPRINTF(pucCurrBuf, ("\n    REASON CODE = %d", prStaRec->u2ReasonCode));
#if CFG_IBSS_POWER_SAVE
            {
                UINT_32 j;


                SPRINTF(pucCurrBuf, ("\n                      TC0       TC1       TC2       TC3      TCS0       TC4"));
                SPRINTF(pucCurrBuf, ("\n---------------------------------------------------------------------------"));
                SPRINTF(pucCurrBuf, ("\nSTA WAIT QUEUE :"));
                for (j = 0; j < TC_NUM; j++) {
                    SPRINTF(pucCurrBuf, ("%9ld ", prStaRec->arStaWaitQueue[j].u4NumElem));
                }
                SPRINTF(pucCurrBuf, ("\n* fgIsAdhocStaAwake : %d",
                    prStaRec->fgIsAdhocStaAwake));
                SPRINTF(pucCurrBuf, ("\n"));
            }
#endif /* CFG_IBSS_POWER_SAVE */
        }
    }

    SPRINTF(pucCurrBuf, ("\n"));

    *pu4Count = (UINT_32)((UINT_32)pucCurrBuf - (UINT_32)pucBuffer);

    return;

} /* end of staRecQueryStatus() */
#endif /* CFG_DBG_STA_RECORD */




