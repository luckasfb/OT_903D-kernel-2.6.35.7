






#include "precomp.h"

extern PHY_ATTRIBUTE_T rPhyAttributes[];




#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugJoinState[JOIN_STATE_NUM] = {
    DISP_STRING("JOIN_STATE_IDLE"),
    DISP_STRING("JOIN_STATE_INIT"),
    DISP_STRING("JOIN_STATE_SEND_AUTH1"),
    DISP_STRING("JOIN_STATE_WAIT_AUTH2"),
    DISP_STRING("JOIN_STATE_SEND_AUTH3"),
    DISP_STRING("JOIN_STATE_WAIT_AUTH4"),
    DISP_STRING("JOIN_STATE_SEND_ASSOC1"),
    DISP_STRING("JOIN_STATE_WAIT_ASSOC2"),
    DISP_STRING("JOIN_STATE_COMPLETE"),
};
/*lint -restore */
#endif /* DBG */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
joinFsmInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    prJoinInfo->eCurrentState = JOIN_STATE_IDLE;

    prJoinInfo->prBssDesc = (P_BSS_DESC_T)NULL;

    prJoinInfo->ucAvailableAuthTypes = 0;
    prJoinInfo->ucCurrAuthAlgNum = AUTH_ALGORITHM_NUM_OPEN_SYSTEM;

    prJoinInfo->fgIsParameterAdopted = FALSE;


    ARB_INIT_TIMER(prAdapter,
                   prJoinInfo->rJoinTimer,
                   arbFsmRunEventJoinFailureTimeOut,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prJoinInfo->rTxRequestTimer,
                   arbFsmRunEventJoinTxTimeOut,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prJoinInfo->rRxResponseTimer,
                   arbFsmRunEventJoinRxTimeOut,
                   TRUE);

    return;
} /* end of joinFsmInit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
joinFsmTransAction_INIT_to_SEND_AUTH1 (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    /* Reset Send Auth/(Re)Assoc Frame Count */
    prJoinInfo->ucTxAuthAssocRetryCount = 0;

    return;
} /* end of joinFsmTransAction_INIT_to_SEND_AUTH1() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
joinFsmTransAction_INIT_to_IDLE (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    prJoinInfo->ucAvailableAuthTypes = 0;
    prJoinInfo->ucCurrAuthAlgNum = AUTH_ALGORITHM_NUM_OPEN_SYSTEM;

    /* NOTE: JOIN FAIL -
     * Restore original setting from current BSS_INFO_T.
     */
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        joinAdoptParametersFromCurrentBss(prAdapter);
    }

    return;
} /* end of joinFsmTransAction_INIT_to_IDLE() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
joinFsmTransAction_SEND_AUTH1_to_INIT (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    return;
} /* end of joinFsmTransAction_SEND_AUTH1_to_INIT() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
joinFsmTransAction_SEND_AUTH3_to_INIT (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    return;
} /* end of joinFsmTransAction_SEND_AUTH3_to_INIT() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
joinFsmTransAction_SEND_ASSOC1_to_INIT (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    return;
} /* end of joinFsmTransAction_SEND_ASSOC1_to_INIT() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
joinFsmTransAction_COMPLETE_to_IDLE (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;


    /* NOTE: JOIN SUCCESS -
     * Already update new settings in joinComplete().
     */
    prJoinInfo->ucAvailableAuthTypes = 0;
    prJoinInfo->ucCurrAuthAlgNum = AUTH_ALGORITHM_NUM_OPEN_SYSTEM;

    return;
} /* end of joinFsmTransAction_COMPLETE_to_IDLE() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmSteps (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_JOIN_STATE_T eNextState
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_STA_RECORD_T prStaRec;
    BOOLEAN fgIsTransition = (BOOLEAN)FALSE;
    /* Cause we are going to wait for Auth/Assoc Response Frame, so the default STATUS is PENDING */
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("joinFsmSteps");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    do {

        DBGLOG(JOIN, STATE, ("TRANSITION: [%s] -> [%s]\n",
                             apucDebugJoinState[prJoinInfo->eCurrentState],
                             apucDebugJoinState[eNextState]));

        /* NOTE(Kevin): This is the only place to change the eCurrentState(except initial) */
        prJoinInfo->eCurrentState = eNextState;


        fgIsTransition = (BOOLEAN)FALSE;
        switch (prJoinInfo->eCurrentState) {
        case JOIN_STATE_IDLE:
            {

#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
                P_TX_CTRL_T prTxCtrl = &prAdapter->rTxCtrl;


                //4 Allow TX Traffic to HW FIFO after leaving JOIN.
                prTxCtrl->fgBlockTxDuringJoin = FALSE;
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */

                /* Clear the Connecting Flag of BSS_DESC_T. */
                prJoinInfo->prBssDesc->fgIsConnecting = FALSE;
            }
            break;

        case JOIN_STATE_INIT:
            {
                do {
                    prStaRec = staRecGetStaRecordByAddrOrAddIfNotExist(prAdapter,
                                   prJoinInfo->prBssDesc->aucBSSID);

                    /* NOTE(Kevin): If we back to INIT STATE, pull STA back to
                     * STATE_1. Because currently we didn't support JOIN start from STATE_2.
                     */

                    /* Update Station Record - Class 1 Flag */
                    prStaRec->ucStaState = STA_STATE_1;

                    /* Update received RCPI */
                    prStaRec->rRcpi = prJoinInfo->prBssDesc->rRcpi;


                    /* Do tasks in INIT STATE */
                    if (prJoinInfo->ucAvailableAuthTypes &
                        (UINT_8)AUTH_TYPE_OPEN_SYSTEM) {

                        DBGLOG(JOIN, INFO, ("Try to do Authentication with AuthType == OPEN_SYSTEM.\n"));
                        prJoinInfo->ucAvailableAuthTypes &=
                            ~(UINT_8)AUTH_TYPE_OPEN_SYSTEM;

                        prJoinInfo->ucCurrAuthAlgNum = (UINT_8)AUTH_ALGORITHM_NUM_OPEN_SYSTEM;
                    }
                    else if (prJoinInfo->ucAvailableAuthTypes &
                        (UINT_8)AUTH_TYPE_SHARED_KEY) {

                        DBGLOG(JOIN, INFO, ("Try to do Authentication with AuthType == SHARED_KEY.\n"));

                        prJoinInfo->ucAvailableAuthTypes &=
                            ~(UINT_8)AUTH_TYPE_SHARED_KEY;

                        prJoinInfo->ucCurrAuthAlgNum = (UINT_8)AUTH_ALGORITHM_NUM_SHARED_KEY;
                    }
                    else if (prJoinInfo->ucAvailableAuthTypes &
                        (UINT_8)AUTH_TYPE_FAST_BSS_TRANSITION) {

                        DBGLOG(JOIN, INFO, ("Try to do Authentication with AuthType == FAST_BSS_TRANSITION.\n"));

                        prJoinInfo->ucAvailableAuthTypes &=
                            ~(UINT_8)AUTH_TYPE_FAST_BSS_TRANSITION;

                        prJoinInfo->ucCurrAuthAlgNum = (UINT_8)AUTH_ALGORITHM_NUM_FAST_BSS_TRANSITION;
                    }
                    else {
                        /* Increase Failure Count */
                        prStaRec->ucJoinFailureCount++;

                        /* Join fail if ucAvailableAuthType become empty */
                        JOIN_STATE_TRANSITION(prAdapter, INIT, IDLE);

                        /* Cancel JOIN Timer */
                        ARB_CANCEL_TIMER(prAdapter,
                                         prJoinInfo->rJoinTimer);

                        rStatus = WLAN_STATUS_FAILURE;
                        break; /* Break the do {} while (FALSE); */
                    }

                    if (!prJoinInfo->fgIsParameterAdopted) {

                        /* Set the Connecting Flag of BSS_DESC_T. */
                        prJoinInfo->prBssDesc->fgIsConnecting = TRUE;

                        /* Adopt parameter and call nic function to update some register here if necessary */
                        joinAdoptParametersFromPeerBss(prAdapter);
                    }

#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
                    {
                        P_TX_CTRL_T prTxCtrl = &prAdapter->rTxCtrl;

                        //4 Block TX Traffic to HW FIFO during JOIN.
                        prTxCtrl->fgBlockTxDuringJoin = TRUE;
                    }
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */

                    /* Update Station Record - Status/Reason Code */
                    prStaRec->u2StatusCode = STATUS_CODE_SUCCESSFUL;

                    /* Update the record activity time. */
                    GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);

                    /* Update the record join time. */
                    GET_CURRENT_SYSTIME(&prStaRec->rLastJoinTime);

                    JOIN_STATE_TRANSITION(prAdapter, INIT, SEND_AUTH1);
                }
                while (FALSE);
            }
            break;

        case JOIN_STATE_SEND_AUTH1:
            {
                /* Do tasks in INIT STATE */
                if (prJoinInfo->ucTxAuthAssocRetryCount >=
                    prJoinInfo->ucTxAuthAssocRetryLimit) {

                    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                                        prJoinInfo->prBssDesc->aucBSSID);
                    ASSERT(prStaRec);

                    /* Record the Status Code of Authentication Request */
                    prStaRec->u2StatusCode = STATUS_CODE_AUTH_TIMEOUT;

                    JOIN_STATE_TRANSITION(prAdapter, SEND_AUTH1, INIT);
                }
                else {
                    prJoinInfo->ucTxAuthAssocRetryCount++;

                    if (authSendAuthFrame(prAdapter,
                                          &prAdapter->rPeerBssInfo,
                                          (UINT_16)prJoinInfo->ucCurrAuthAlgNum,
                                          AUTH_TRANSACTION_SEQ_1) != WLAN_STATUS_SUCCESS) {

                        ARB_SET_TIMER(prAdapter,
                                      prJoinInfo->rTxRequestTimer,
                                      TU_TO_MSEC(TX_AUTHENTICATION_RETRY_TIMEOUT_TU));
                    }
                }
            }
            break;

        case JOIN_STATE_WAIT_AUTH2:
            break;

        case JOIN_STATE_SEND_AUTH3:
            {
                /* Do tasks in INIT STATE */
                if (prJoinInfo->ucTxAuthAssocRetryCount >=
                    prJoinInfo->ucTxAuthAssocRetryLimit) {

                    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                                        prJoinInfo->prBssDesc->aucBSSID);
                    ASSERT(prStaRec);

                    /* Record the Status Code of Authentication Request */
                    prStaRec->u2StatusCode = STATUS_CODE_AUTH_TIMEOUT;

                    JOIN_STATE_TRANSITION(prAdapter, SEND_AUTH3, INIT);
                }
                else {
                    prJoinInfo->ucTxAuthAssocRetryCount++;

                    if (authSendAuthFrame(prAdapter,
                                          &prAdapter->rPeerBssInfo,
                                          (UINT_16)prJoinInfo->ucCurrAuthAlgNum,
                                          AUTH_TRANSACTION_SEQ_3) != WLAN_STATUS_SUCCESS) {

                        ARB_SET_TIMER(prAdapter,
                                      prJoinInfo->rTxRequestTimer,
                                      TU_TO_MSEC(TX_AUTHENTICATION_RETRY_TIMEOUT_TU));
                    }
                }
            }
            break;

        case JOIN_STATE_WAIT_AUTH4:
            break;

        case JOIN_STATE_SEND_ASSOC1:
                /* Do tasks in INIT STATE */
                if (prJoinInfo->ucTxAuthAssocRetryCount >=
                    prJoinInfo->ucTxAuthAssocRetryLimit) {

                    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                                        prJoinInfo->prBssDesc->aucBSSID);
                    ASSERT(prStaRec);

                    /* Record the Status Code of Authentication Request */
                    prStaRec->u2StatusCode = STATUS_CODE_ASSOC_TIMEOUT;

                    JOIN_STATE_TRANSITION(prAdapter, SEND_ASSOC1, INIT);
                }
                else {
                    prJoinInfo->ucTxAuthAssocRetryCount++;

                    if (assocSendReAssocReqFrame(prAdapter,
                                                 prJoinInfo->prBssDesc,
                                                 prJoinInfo->fgIsReAssoc) != WLAN_STATUS_SUCCESS) {
                        ARB_SET_TIMER(prAdapter,
                                      prJoinInfo->rTxRequestTimer,
                                      TU_TO_MSEC(TX_ASSOCIATION_RETRY_TIMEOUT_TU));
                    }
                }

            break;

        case JOIN_STATE_WAIT_ASSOC2:
            break;

        case JOIN_STATE_COMPLETE:

            ARB_CANCEL_TIMER(prAdapter,
                             prJoinInfo->rJoinTimer);


            joinComplete(prAdapter);

            JOIN_STATE_TRANSITION(prAdapter, COMPLETE, IDLE);

            rStatus = WLAN_STATUS_SUCCESS;

            break;

        default:
            DBGLOG(JOIN, ERROR, ("Unknown JOIN STATE\n"));
            ASSERT(0);
            break;
        }

    }
    while (fgIsTransition);

    return rStatus;

} /* end of joinFsmSteps() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmRunEventStart (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_PEER_BSS_INFO_T prPeerBssInfo;

    DEBUGFUNC("joinFsmRunEventStart");


    ASSERT(prAdapter);
    ASSERT(prBssDesc);
    prJoinInfo = &prAdapter->rJoinInfo;
    prConnSettings = &prAdapter->rConnSettings;
    prPeerBssInfo = &prAdapter->rPeerBssInfo;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: START\n"));

    //4 <1> The previous JOIN process is not completed ?
    if (prJoinInfo->eCurrentState != JOIN_STATE_IDLE) {
        DBGLOG(JOIN, ERROR, ("Reentry of JOIN Module - JOIN_STATE is not in IDLE_STATE.\n"));
        return WLAN_STATUS_FAILURE;
    }

    //4 <2> Validation of JOIN Start Event
    if (prBssDesc->eBSSType != BSS_TYPE_INFRASTRUCTURE) {

        DBGLOG(JOIN, ERROR, ("Target BSS Type : %d is not supported by JOIN\n",
                             prBssDesc->eBSSType));

        return WLAN_STATUS_FAILURE;
    }

    //4 <3> Invoke ROAMING(ReAssoc Frame) or not
    if (prAdapter->eConnectionState == MEDIA_STATE_DISCONNECTED) {

        prJoinInfo->fgIsReAssoc = FALSE;

        switch (prConnSettings->eAuthMode) {
        case AUTH_MODE_OPEN:                /* Note: Omit break here. */
        case AUTH_MODE_WPA:
        case AUTH_MODE_WPA_PSK:
        case AUTH_MODE_WPA2:
        case AUTH_MODE_WPA2_PSK:
            prJoinInfo->ucAvailableAuthTypes = (UINT_8)AUTH_TYPE_OPEN_SYSTEM;
            break;


        case AUTH_MODE_SHARED:
            prJoinInfo->ucAvailableAuthTypes = (UINT_8)AUTH_TYPE_SHARED_KEY;
            break;


        case AUTH_MODE_AUTO_SWITCH:
            DBGLOG(JOIN, INFO, ("prConnSettings->eAuthMode == AUTH_MODE_AUTO_SWITCH\n"));
            prJoinInfo->ucAvailableAuthTypes = (UINT_8)(AUTH_TYPE_OPEN_SYSTEM |
                                                        AUTH_TYPE_SHARED_KEY);
            break;

        default:
            ASSERT(!(prConnSettings->eAuthMode == AUTH_MODE_WPA_NONE));
            DBGLOG(JOIN, ERROR, ("Authentication Algorithm : %d was not supported by JOIN\n",
                                 prConnSettings->eAuthMode));
            return WLAN_STATUS_FAILURE;
        }

        prJoinInfo->ucTxAuthAssocRetryLimit = TX_AUTH_ASSOCI_RETRY_LIMIT;

    }
    else {
        P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;


        ASSERT(prBssInfo->eBSSType == BSS_TYPE_INFRASTRUCTURE);
        ASSERT(!prBssDesc->fgIsConnected);

        DBGLOG(JOIN, INFO, ("JOIN for roaming, AUTH TYPE = %d\n", prBssInfo->ucRoamingAuthTypes));


        prJoinInfo->fgIsReAssoc = TRUE; /* We do roaming while the medium is connected */

        pmFsmRunEventAbort(prAdapter); /* Stop Power Mgt Module */

        /* TODO(Kevin): We may call a sub function to acquire the Roaming Auth Type */
        prJoinInfo->ucAvailableAuthTypes = prBssInfo->ucRoamingAuthTypes;

        prJoinInfo->ucTxAuthAssocRetryLimit = TX_AUTH_ASSOCI_RETRY_LIMIT_FOR_ROAMING;
    }

    prJoinInfo->ucRoamingAuthTypes = 0; /* Clear the Auth Type for Roaming, this value
                                         * will be setup during the JOIN process, and then save
                                         * to BSS_INFO_T for Fast BSS Transition later.
                                         */

    //4 <4> Save the Target SCAN Record.
    prJoinInfo->prBssDesc = prBssDesc;

    //4 <5> Clear the PEER_BSS_INFO_T.
    kalMemZero(prPeerBssInfo, sizeof(PEER_BSS_INFO_T));

    /* Update parameters to the PEER_BSS_INFO_T */
    COPY_MAC_ADDR(prPeerBssInfo->aucBSSID, prBssDesc->aucBSSID);

    //4 <6> Clear Flag to adopt paramter from BSS_DESC_T and apply them to HW later. (e.g. Channel ..)
    prJoinInfo->fgIsParameterAdopted = FALSE;

    //4 <7> Setup the Maximum JOIN Time.
    ARB_SET_TIMER(prAdapter,
                  prJoinInfo->rJoinTimer,
                  TU_TO_MSEC(JOIN_FAILURE_TIMEOUT_BEACON_INTERVAL *
                      DOT11_BEACON_PERIOD_DEFAULT));

    /* Trigger JOIN MODULE */
    return joinFsmSteps(prAdapter, JOIN_STATE_INIT);

} /* end of joinFsmRunEventStart() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
joinFsmRunEventAbort (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_STA_RECORD_T prStaRec;

    DEBUGFUNC("joinFsmRunEventAbort");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: ABORT\n"));


    /* NOTE(Kevin): when reach here, the ARB_STATE should be in ARB_STATE_JOIN. */
    ASSERT(prJoinInfo->prBssDesc);

    //4 <1> Update Flags and Elements of JOIN Module.
    /* Reset Send Auth/(Re)Assoc Frame Count */
    prJoinInfo->ucTxAuthAssocRetryCount = 0;

    /* Cancel all JOIN relative Timer */
    ARB_CANCEL_TIMER(prAdapter,
                     prJoinInfo->rTxRequestTimer);

    ARB_CANCEL_TIMER(prAdapter,
                     prJoinInfo->rRxResponseTimer);

    ARB_CANCEL_TIMER(prAdapter,
                     prJoinInfo->rJoinTimer);

    //4 <2> Update the associated STA_RECORD_T during JOIN.
    /* Get a Station Record if possible, TA == BSSID for AP */
    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                        prJoinInfo->prBssDesc->aucBSSID);
    if (prStaRec) {

        /* Update Station Record - Class 1 Flag */
        prStaRec->ucStaState = STA_STATE_1;
    }
#if DBG
    else {
        ASSERT(0); /* Shouldn't happened, because we already add this STA_RECORD_T at JOIN_STATE_INIT */
    }
#endif /* DBG */

    //4 <3> Pull back to IDLE.
    joinFsmSteps(prAdapter, JOIN_STATE_IDLE);

    //4 <4> If we are in Roaming, recover the settings of previous BSS.
    /* NOTE: JOIN FAIL -
     * Restore original setting from current BSS_INFO_T.
     */
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        joinAdoptParametersFromCurrentBss(prAdapter);
    }

    return;
} /* end of joinFsmRunEventAbort() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmRunEventTxDone (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    )
{
    P_JOIN_INFO_T prJoinInfo;
    ENUM_JOIN_STATE_T eNextState;
    /* Cause we are going to wait for Auth Response Frame, so the default STATUS is PENDING */
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("joinFsmRunEventTxDone");


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prJoinInfo = &prAdapter->rJoinInfo;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: TX DONE\n"));

    switch(prJoinInfo->eCurrentState) {
    case JOIN_STATE_SEND_AUTH1:
        {
            /* Check the outgoing frame is matched with current JOIN STATE */
            if (authCheckTxAuthFrame(prAdapter,

                                     prMsduInfo,
                                     prJoinInfo->ucCurrAuthAlgNum,
                                     AUTH_TRANSACTION_SEQ_1) != WLAN_STATUS_SUCCESS) {
                break;
            }

            if (rTxDoneStatus == WLAN_STATUS_SUCCESS) {
                eNextState = JOIN_STATE_WAIT_AUTH2;

                ARB_SET_TIMER(prAdapter,
                              prJoinInfo->rRxResponseTimer,
                              TU_TO_MSEC(DOT11_AUTHENTICATION_RESPONSE_TIMEOUT_TU));
            }
            /* TX Fail, do retry if possible */
            else {
                eNextState = prJoinInfo->eCurrentState;
            }

            rStatus = joinFsmSteps(prAdapter, eNextState);
        }
        break;

    case JOIN_STATE_SEND_AUTH3:
        {
            /* Check the outgoing frame is matched with current JOIN STATE */
            if (authCheckTxAuthFrame(prAdapter,

                                     prMsduInfo,
                                     (UINT_16)prJoinInfo->ucCurrAuthAlgNum,
                                     AUTH_TRANSACTION_SEQ_3) != WLAN_STATUS_SUCCESS) {
                break;
            }

            if (rTxDoneStatus == WLAN_STATUS_SUCCESS) {
                eNextState = JOIN_STATE_WAIT_AUTH4;

                ARB_SET_TIMER(prAdapter,
                              prJoinInfo->rRxResponseTimer,
                              TU_TO_MSEC(DOT11_AUTHENTICATION_RESPONSE_TIMEOUT_TU));
            }
            /* TX Fail, do retry if possible */
            else {
                eNextState = prJoinInfo->eCurrentState;
            }

            rStatus = joinFsmSteps(prAdapter, eNextState);
        }
        break;

    case JOIN_STATE_SEND_ASSOC1:
        {
            /* Check the outgoing frame is matched with current JOIN STATE */
            if (assocCheckTxReAssocReqFrame(prAdapter,

                                            prMsduInfo,
                                            prJoinInfo->fgIsReAssoc) != WLAN_STATUS_SUCCESS) {
                break;
            }

            if (rTxDoneStatus == WLAN_STATUS_SUCCESS) {
                eNextState = JOIN_STATE_WAIT_ASSOC2;

                ARB_SET_TIMER(prAdapter,
                              prJoinInfo->rRxResponseTimer,
                              TU_TO_MSEC(DOT11_ASSOCIATION_RESPONSE_TIMEOUT_TU));
            }
            /* TX Fail, do retry if possible */
            else {
                eNextState = prJoinInfo->eCurrentState;
            }

            rStatus = joinFsmSteps(prAdapter, eNextState);
        }
        break;

    default:
        break; /* Ignore other cases */
    }

    return rStatus;

} /* end of joinFsmRunEventTxDone() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmRunEventTxReqTimeOut (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;

    DEBUGFUNC("joinFsmRunEventTxReqTimeOut");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: TX REQ TIMEOUT\n"));

    switch(prJoinInfo->eCurrentState) {
    case JOIN_STATE_SEND_AUTH1:
    case JOIN_STATE_SEND_AUTH3:
    case JOIN_STATE_SEND_ASSOC1:
        return joinFsmSteps(prAdapter, prJoinInfo->eCurrentState);

    default:
        return WLAN_STATUS_PENDING;
    }

} /* end of joinFsmRunEventTxReqTimeOut() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmRunEventRxRespTimeOut (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_STA_RECORD_T prStaRec;
    ENUM_JOIN_STATE_T eNextState;
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("joinFsmRunEventRxRespTimeOut");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;
    eNextState = prJoinInfo->eCurrentState;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: RX RESP TIMEOUT\n"));

    /* Get a Station Record if possible, TA == BSSID for AP */
    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                        prJoinInfo->prBssDesc->aucBSSID);

    /* We have renew this Sta Record when in JOIN_STATE_INIT */
    ASSERT(prStaRec);

    switch(prJoinInfo->eCurrentState) {
    case JOIN_STATE_WAIT_AUTH2:
        /* Record the Status Code of Authentication Request */
        prStaRec->u2StatusCode = STATUS_CODE_AUTH_TIMEOUT;

        /* Pull back to INIT state to try other AuthType if have */
        eNextState = JOIN_STATE_SEND_AUTH1;
        break;

    case JOIN_STATE_WAIT_AUTH4:
        /* Record the Status Code of Authentication Request */
        prStaRec->u2StatusCode = STATUS_CODE_AUTH_TIMEOUT;

        /* Pull back to INIT state to try other AuthType if have */
        eNextState = JOIN_STATE_SEND_AUTH3;
        break;

    case JOIN_STATE_WAIT_ASSOC2:
        /* Record the Status Code of Authentication Request */
        prStaRec->u2StatusCode = STATUS_CODE_ASSOC_TIMEOUT;

        /* Pull back to INIT state to try other AuthType if have */
        eNextState = JOIN_STATE_SEND_ASSOC1;
        break;

    default:
        break; /* Ignore other cases */
    }


    if (eNextState != prJoinInfo->eCurrentState) {
        rStatus = joinFsmSteps(prAdapter, eNextState);
    }

    return rStatus;

} /* end of joinFsmRunEventRxRespTimeOut() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmRunEventRxAuthAssoc (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_BSS_DESC_T prBssDesc;
    P_STA_RECORD_T prStaRec;
    ENUM_JOIN_STATE_T eNextState;
    UINT_16 u2StatusCode;
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("joinFsmRunEventRxAuthAssoc");


    ASSERT(prAdapter);
    ASSERT(prSwRfb);
    prJoinInfo = &prAdapter->rJoinInfo;
    prBssDesc = prJoinInfo->prBssDesc;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: RX AUTH/ASSOC\n"));

    /* Get a Station Record if possible, TA == BSSID for AP */
    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                        prBssDesc->aucBSSID);

    /* We have renew this Sta Record when in JOIN_STATE_INIT */
    ASSERT(prStaRec);

    switch(prJoinInfo->eCurrentState) {
    case JOIN_STATE_SEND_AUTH1:
    case JOIN_STATE_WAIT_AUTH2:
        /* Check if the incoming frame is what we are waiting for */
        if (authProcessRxAuthFrame(prAdapter,
                                   prSwRfb,
                                   &prAdapter->rPeerBssInfo,
                                   (UINT_16)prJoinInfo->ucCurrAuthAlgNum,
                                   AUTH_TRANSACTION_SEQ_2,
                                   &u2StatusCode) == WLAN_STATUS_SUCCESS) {

            ARB_CANCEL_TIMER(prAdapter,
                             prJoinInfo->rRxResponseTimer);

            /* Record the Status Code of Authentication Request */
            prStaRec->u2StatusCode = u2StatusCode;

            if (u2StatusCode == STATUS_CODE_SUCCESSFUL) {
                if (prJoinInfo->ucCurrAuthAlgNum ==
                    (UINT_8)AUTH_ALGORITHM_NUM_SHARED_KEY) {

                    eNextState = JOIN_STATE_SEND_AUTH3;
                }
                else {
                    /* Update Station Record - Class 2 Flag */
                    prStaRec->ucStaState = STA_STATE_2;

                    eNextState = JOIN_STATE_SEND_ASSOC1;
                }
            }
            else {
                DBGLOG(JOIN, INFO, ("Auth Req was rejected by ["MACSTR"], Status Code = %d\n",
                    MAC2STR(prBssDesc->aucBSSID), u2StatusCode));

                eNextState = JOIN_STATE_INIT;
            }

            /* Reset Send Auth/(Re)Assoc Frame Count */
            prJoinInfo->ucTxAuthAssocRetryCount = 0;

            rStatus = joinFsmSteps(prAdapter, eNextState);
        }
        break;

    case JOIN_STATE_SEND_AUTH3:
    case JOIN_STATE_WAIT_AUTH4:
        /* Check if the incoming frame is what we are waiting for */
        if (authProcessRxAuthFrame(prAdapter,
                                   prSwRfb,
                                   &prAdapter->rPeerBssInfo,
                                   (UINT_16)prJoinInfo->ucCurrAuthAlgNum,
                                   AUTH_TRANSACTION_SEQ_4,
                                   &u2StatusCode) == WLAN_STATUS_SUCCESS) {

            ARB_CANCEL_TIMER(prAdapter,
                             prJoinInfo->rRxResponseTimer);

            /* Record the Status Code of Authentication Request */
            prStaRec->u2StatusCode = u2StatusCode;

            if (u2StatusCode == STATUS_CODE_SUCCESSFUL) {

                /* Update Station Record - Class 2 Flag */
                prStaRec->ucStaState = STA_STATE_2;

                eNextState = JOIN_STATE_SEND_ASSOC1;
            }
            else {
                DBGLOG(JOIN, INFO, ("Auth Req was rejected by ["MACSTR"], Status Code = %d\n",
                    MAC2STR(prBssDesc->aucBSSID), u2StatusCode));

                eNextState = JOIN_STATE_INIT;
            }

            /* Reset Send Auth/(Re)Assoc Frame Count */
            prJoinInfo->ucTxAuthAssocRetryCount = 0;

            rStatus = joinFsmSteps(prAdapter, eNextState);
        }
        break;

    case JOIN_STATE_SEND_ASSOC1:
    case JOIN_STATE_WAIT_ASSOC2:
        /* TRUE if the incoming frame is what we are waiting for */
        if (assocProcessRxReAssocRspFrame(prAdapter,
                                          prSwRfb,
                                          &prAdapter->rPeerBssInfo,
                                          prJoinInfo->fgIsReAssoc,
                                          &u2StatusCode) == WLAN_STATUS_SUCCESS) {

            ARB_CANCEL_TIMER(prAdapter,
                             prJoinInfo->rRxResponseTimer);

#if 1
            /* NOTE(Kevin): CM suggest to add MGMT workaround for those APs didn't check
             * the CAP Privacy Bit to overcome a corner case that the Privacy Bit
             * of our SCAN result didn't consist with AP's Association Resp.
             */
            if (u2StatusCode == STATUS_CODE_SUCCESSFUL) {
                #if SUPPORT_WAPI
                if (prAdapter->fgWapiMode){
                    /* For WAPI AP allow the customer use WZC to join mode, the privacy bit is 0 
                       evan at WAI & WAPI_PSK mode, but the assoc response set the privacy bit to 1 */
                    /* For specific this case, check the beacon WAPI IE is exist but privacy bit is 0 */                   
                    DBGLOG(WAPI, TRACE, ("Workaround the WAPI AP allow the customer use WZC to join\n"));
                }
                else
                #endif
                if (((prBssDesc->u2CapInfo & CAP_INFO_PRIVACY) && 
                    !(prAdapter->rPeerBssInfo.fgIsPrivacyEnabled)) ||
                    (!(prBssDesc->u2CapInfo & CAP_INFO_PRIVACY) && 
                    (prAdapter->rPeerBssInfo.fgIsPrivacyEnabled))) {

                    u2StatusCode = STATUS_CODE_CAP_NOT_SUPPORTED;
                }
            }
#endif

            /* Record the Status Code of Authentication Request */
            prStaRec->u2StatusCode = u2StatusCode;

            if (u2StatusCode == STATUS_CODE_SUCCESSFUL) {

                /* Update Station Record - Class 3 Flag */
                prStaRec->ucStaState = STA_STATE_3;
                prStaRec->ucJoinFailureCount = 0; // Clear history.

                eNextState = JOIN_STATE_COMPLETE;
            }
            else {
                DBGLOG(JOIN, INFO, ("Assoc Req was rejected by ["MACSTR"], Status Code = %d\n",
                    MAC2STR(prBssDesc->aucBSSID), u2StatusCode));

                eNextState = JOIN_STATE_INIT;
            }

            /* Reset Send Auth/(Re)Assoc Frame Count */
            prJoinInfo->ucTxAuthAssocRetryCount = 0;

            rStatus = joinFsmSteps(prAdapter, eNextState);
        }
        break;

    default:
        break; /* Ignore other cases */
    }

    return rStatus;

} /* end of joinFsmRunEventRxAuthAssoc() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
joinFsmRunEventJoinTimeOut (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_STA_RECORD_T prStaRec;

    DEBUGFUNC("joinFsmRunEventJoinTimeOut");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;

    DBGLOG(JOIN, EVENT, ("JOIN EVENT: JOIN TIMEOUT\n"));

    /* Get a Station Record if possible, TA == BSSID for AP */
    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                        prJoinInfo->prBssDesc->aucBSSID);

    /* We have renew this Sta Record when in JOIN_STATE_INIT */
    ASSERT(prStaRec);

    /* Record the Status Code of Authentication Request */
    prStaRec->u2StatusCode = STATUS_CODE_JOIN_TIMEOUT;

    /* Increase Failure Count */
    prStaRec->ucJoinFailureCount++;

    /* Reset Send Auth/(Re)Assoc Frame Count */
    prJoinInfo->ucTxAuthAssocRetryCount = 0;

    /* Cancel other JOIN relative Timer */
    ARB_CANCEL_TIMER(prAdapter,
                     prJoinInfo->rTxRequestTimer);

    ARB_CANCEL_TIMER(prAdapter,
                     prJoinInfo->rRxResponseTimer);

    /* Restore original setting from current BSS_INFO_T */
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        joinAdoptParametersFromCurrentBss(prAdapter);
    }

    /* Pull back to IDLE */
    joinFsmSteps(prAdapter, JOIN_STATE_IDLE);

    return WLAN_STATUS_FAILURE;

} /* end of joinFsmRunEventJoinTimeOut() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
joinAdoptParametersFromPeerBss (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_BSS_DESC_T prBssDesc;

    DEBUGFUNC("joinAdoptParametersFromPeerBss");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;
    prBssDesc = prJoinInfo->prBssDesc;

    //4 <1> Adopt Peer BSS' PHY TYPE
    prAdapter->eCurrentPhyType = prBssDesc->ePhyType;

    DBGLOG(JOIN, INFO, ("Target BSS[%s]'s PhyType = %s\n",
        prBssDesc->aucSSID, (prBssDesc->ePhyType == PHY_TYPE_ERP_INDEX) ? "ERP" : "HR_DSSS"));


    //4 <2> Adopt Peer BSS' Frequency(Band/Channel)
    DBGLOG(JOIN, INFO, ("Target BSS's Channel = %d, Band = %d\n",
        prBssDesc->ucChannelNum, prBssDesc->eBand));

    nicSwitchChannel(prAdapter,
                     prBssDesc->eBand,
                     prBssDesc->ucChannelNum,
                     10);

    prJoinInfo->fgIsParameterAdopted = TRUE;

    return;
} /* end of joinAdoptParametersFromPeerBss() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
joinAdoptParametersFromCurrentBss (
    IN P_ADAPTER_T prAdapter
    )
{
    //P_JOIN_INFO_T prJoinInfo = &prAdapter->rJoinInfo;
    P_BSS_INFO_T prBssInfo;


    ASSERT(prAdapter);
    prBssInfo = &prAdapter->rBssInfo;

    //4 <1> Adopt current BSS' PHY TYPE
    prAdapter->eCurrentPhyType = prBssInfo->ePhyType;

    //4 <2> Adopt current BSS' Frequency(Band/Channel)
    DBGLOG(JOIN, INFO, ("Current BSS's Channel = %d, Band = %d\n",
        prBssInfo->ucChnl, prBssInfo->eBand));

    nicSwitchChannel(prAdapter,
                     prBssInfo->eBand,
                     prBssInfo->ucChnl,
                     10);
    return;
} /* end of joinAdoptParametersFromCurrentBss() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
joinComplete (
    IN P_ADAPTER_T prAdapter
    )
{
    P_JOIN_INFO_T prJoinInfo;
    P_BSS_DESC_T prBssDesc;
    P_PEER_BSS_INFO_T prPeerBssInfo;
    P_BSS_INFO_T prBssInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_STA_RECORD_T prStaRec;
    P_TX_CTRL_T prTxCtrl;
#if CFG_SUPPORT_802_11D
    P_IE_COUNTRY_T          prIECountry;
#endif

    DEBUGFUNC("joinComplete");


    ASSERT(prAdapter);
    prJoinInfo = &prAdapter->rJoinInfo;
    prBssDesc = prJoinInfo->prBssDesc;
    prPeerBssInfo = &prAdapter->rPeerBssInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prConnSettings = &prAdapter->rConnSettings;
    prTxCtrl = &prAdapter->rTxCtrl;

//4 <1> Update Connecting & Connected Flag of BSS_DESC_T.
    /* Remove previous AP's Connection Flags if have */
    scanRemoveConnectionFlagOfBssDescByBssid(prAdapter, prBssInfo->aucBSSID);

    prBssDesc->fgIsConnected = TRUE; /* Mask as Connected */

    if (prBssDesc->fgIsHiddenSSID) {
        /* NOTE(Kevin): This is for the case of Passive Scan and the target BSS didn't
         * broadcast SSID on its Beacon Frame.
         */
        COPY_SSID(prBssDesc->aucSSID,
                  prBssDesc->ucSSIDLen,
                  prAdapter->rConnSettings.aucSSID,
                  prAdapter->rConnSettings.ucSSIDLen);

        if (prBssDesc->ucSSIDLen) {
            prBssDesc->fgIsHiddenSSID = FALSE;
        }
#if DBG
        else {
            ASSERT(0);
        }
#endif /* DBG */

        DBGLOG(JOIN, INFO, ("Hidden SSID! - Update SSID : %s\n", prBssDesc->aucSSID));
    }


//4 <2> Update BSS_INFO_T from BSS_DESC_T
    //4 <2.A> PHY Type
    prBssInfo->ePhyType = prBssDesc->ePhyType;

    //4 <2.B> BSS Type
    prBssInfo->eBSSType = BSS_TYPE_INFRASTRUCTURE;

    //4 <2.C> BSSID
    COPY_MAC_ADDR(prBssInfo->aucBSSID, prBssDesc->aucBSSID);

    DBGLOG(JOIN, INFO, ("JOIN to BSSID: ["MACSTR"]\n", MAC2STR(prBssDesc->aucBSSID)));


    //4 <2.D> SSID
    COPY_SSID(prBssInfo->aucSSID,
              prBssInfo->ucSSIDLen,
              prBssDesc->aucSSID,
              prBssDesc->ucSSIDLen);

    //4 <2.E> Channel / Band information.
    prBssInfo->eBand = prBssDesc->eBand;
    prBssInfo->ucChnl = prBssDesc->ucChannelNum;

    //4 <2.F> RSN/WPA information.
    secFsmRunEventStart(prAdapter);
    prBssInfo->u4RsnSelectedPairwiseCipher = prBssDesc->u4RsnSelectedPairwiseCipher;
    prBssInfo->u4RsnSelectedGroupCipher = prBssDesc->u4RsnSelectedGroupCipher;
    prBssInfo->u4RsnSelectedAKMSuite = prBssDesc->u4RsnSelectedAKMSuite;

    if (privacyRsnKeyHandshakeEnabled(prAdapter)) {
        prBssInfo->fgIsWPAorWPA2Enabled = TRUE;
    }
    else {
        prBssInfo->fgIsWPAorWPA2Enabled = FALSE;
    }

    //4 <2.G> Beacon interval.
    prBssInfo->u2BeaconInterval = prBssDesc->u2BeaconInterval;

    //4 <2.H> DTIM period.
    prBssInfo->ucDtimPeriod = prBssDesc->ucDTIMPeriod;

    //4 <2.I> ERP Information
    if ((prBssInfo->ePhyType == PHY_TYPE_ERP_INDEX) && // Our BSS's PHY_TYPE is ERP now.
        (prBssDesc->fgIsERPPresent)) {

        prBssInfo->fgIsERPPresent = TRUE;
        prBssInfo->ucERP = prBssDesc->ucERP; /* Save the ERP for later check */
    }
    else { /* Some AP, may send ProbeResp without ERP IE. Thus prBssDesc->fgIsERPPresent is FALSE. */
        prBssInfo->fgIsERPPresent = FALSE;
        prBssInfo->ucERP = 0;
    }

#if CFG_SUPPORT_802_11D
    //4 <2.J> Country inforamtion of the associated AP
    if (prConnSettings->fgMultiDomainCapabilityEnabled) {
        DOMAIN_INFO_ENTRY   rDomainInfo;
        if (domainGetDomainInfoByScanResult(prAdapter, &rDomainInfo)) {
            if (prBssDesc->prIECountry) {
                prIECountry = prBssDesc->prIECountry;

                domainParseCountryInfoElem(prIECountry, &prBssInfo->rDomainInfo);

                /* use the domain get from the BSS info */
                prBssInfo->fgIsCountryInfoPresent = TRUE;
                nicSetupOpChnlList(prAdapter, prBssInfo->rDomainInfo.u2CountryCode, FALSE);
            } else {
                /* use the domain get from the scan result */
                prBssInfo->fgIsCountryInfoPresent = TRUE;
                nicSetupOpChnlList(prAdapter, rDomainInfo.u2CountryCode, FALSE);
            }
        }
    }
#endif

    //4 <2.K> Signal Power of the associated AP
    prBssInfo->rRcpi = prBssDesc->rRcpi;
    prBssInfo->rRssi = RCPI_TO_dBm(prBssInfo->rRcpi);
    GET_CURRENT_SYSTIME(&prBssInfo->rRssiLastUpdateTime);

    //4 <2.L> Capability Field of the associated AP
    prBssInfo->u2CapInfo = prBssDesc->u2CapInfo;
    
    DBGLOG(JOIN, INFO, ("prBssInfo-> fgIsERPPresent = %d, ucERP = %02x, rRcpi = %d, rRssi = %ld\n",
        prBssInfo->fgIsERPPresent, prBssInfo->ucERP, prBssInfo->rRcpi, prBssInfo->rRssi));


//4 <3> Update BSS_INFO_T from PEER_BSS_INFO_T & NIC RATE FUNC
    //4 <3.A> Association ID
    prBssInfo->u2AssocId = prPeerBssInfo->u2AssocId;

    //4 <3.B> WMM Infomation
    if (prAdapter->fgIsEnableWMM &&
        (prPeerBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_WMM)) {

        prBssInfo->fgIsWmmAssoc = TRUE;
        prTxCtrl->rTxQForVoipAccess = TXQ_AC3;

        qosWmmInfoInit(&prBssInfo->rWmmInfo, (prBssInfo->ePhyType == PHY_TYPE_HR_DSSS_INDEX) ? TRUE : FALSE);

        if (prPeerBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_AC_PARAM_PRESENT) {
            kalMemCopy(&prBssInfo->rWmmInfo,
                       &prPeerBssInfo->rWmmInfo,
                       sizeof(WMM_INFO_T));
        }
        else {
            kalMemCopy(&prBssInfo->rWmmInfo,
                       &prPeerBssInfo->rWmmInfo,
                       sizeof(WMM_INFO_T) - sizeof(prPeerBssInfo->rWmmInfo.arWmmAcParams));
        }
    }
    else {
        prBssInfo->fgIsWmmAssoc = FALSE;
        prTxCtrl->rTxQForVoipAccess = TXQ_AC1;

        kalMemZero(&prBssInfo->rWmmInfo, sizeof(WMM_INFO_T));
    }


    //4 <3.C> Operational Rate Set & BSS Basic Rate Set
    prBssInfo->u2OperationalRateSet = prPeerBssInfo->u2OperationalRateSet;
    prBssInfo->u2BSSBasicRateSet = prPeerBssInfo->u2BSSBasicRateSet;


    //4 <3.D> Short Preamble
    if (prBssInfo->fgIsERPPresent) {

        /* NOTE(Kevin 2007/12/24): Truth Table.
         * Short Preamble Bit in
         * <AssocReq>     <AssocResp w/i ERP>     <BARKER(Long)>  Final Driver Setting(Short)
         * TRUE            FALSE                  FALSE           FALSE(shouldn't have such case, use the AssocResp)
         * TRUE            FALSE                  TRUE            FALSE
         * FALSE           FALSE                  FALSE           FALSE(shouldn't have such case, use the AssocResp)
         * FALSE           FALSE                  TRUE            FALSE
         * TRUE            TRUE                   FALSE           TRUE(follow ERP)
         * TRUE            TRUE                   TRUE            FALSE(follow ERP)
         * FALSE           TRUE                   FALSE           FALSE(shouldn't have such case, and we should set to FALSE)
         * FALSE           TRUE                   TRUE            FALSE(we should set to FALSE)
         */
        if ((prPeerBssInfo->fgIsShortPreambleAllowed) &&
            ((prConnSettings->ePreambleType == PREAMBLE_TYPE_SHORT) || /* Short Preamble Option Enable is TRUE */
             ((prConnSettings->ePreambleType == PREAMBLE_TYPE_AUTO) &&
              (prBssDesc->u2CapInfo & CAP_INFO_SHORT_PREAMBLE)))) {

            prBssInfo->fgIsShortPreambleAllowed = TRUE;

            if (prBssInfo->ucERP & ERP_INFO_BARKER_PREAMBLE_MODE) {
                prBssInfo->fgUseShortPreamble = FALSE;
            }
            else {
                prBssInfo->fgUseShortPreamble = TRUE;
            }
        }
        else {
            prBssInfo->fgIsShortPreambleAllowed = FALSE;
            prBssInfo->fgUseShortPreamble = FALSE;
        }
    }
    else {
        /* NOTE(Kevin 2007/12/24): Truth Table.
         * Short Preamble Bit in
         * <AssocReq>     <AssocResp w/o ERP>     Final Driver Setting(Short)
         * TRUE            FALSE                  FALSE
         * FALSE           FALSE                  FALSE
         * TRUE            TRUE                   TRUE
         * FALSE           TRUE(status success)   TRUE
         * --> Honor the result of prPeerBssInfo.
         */

        prBssInfo->fgIsShortPreambleAllowed = prBssInfo->fgUseShortPreamble =
            prPeerBssInfo->fgIsShortPreambleAllowed;
    }

    DBGLOG(JOIN, INFO, ("prBssInfo->fgIsShortPreambleAllowed = %d, prBssInfo->fgUseShortPreamble = %d\n",
        prBssInfo->fgIsShortPreambleAllowed, prBssInfo->fgUseShortPreamble));


    //4 <3.E> Short Slot Time
    prBssInfo->fgUseShortSlotTime =
        prPeerBssInfo->fgUseShortSlotTime; /* AP support Short Slot Time */

    DBGLOG(JOIN, INFO, ("prBssInfo->fgUseShortSlotTime = %d\n",
        prBssInfo->fgUseShortSlotTime));

    nicSetSlotTime(prAdapter,
                   prBssInfo->ePhyType,
                   ((prConnSettings->fgIsShortSlotTimeOptionEnable &&
                     prBssInfo->fgUseShortSlotTime) ? TRUE : FALSE));


    //4 <3.F> Update Tx Rate for Control Frame
    bssUpdateTxRateForControlFrame(prAdapter);


    //4 <3.G> Save the available Auth Types during Roaming (Design for Fast BSS Transition).
    //if (prAdapter->fgIsEnableRoaming) /* NOTE(Kevin): Always prepare info for roaming */
    {

        if (prJoinInfo->ucCurrAuthAlgNum == AUTH_ALGORITHM_NUM_OPEN_SYSTEM) {
            prJoinInfo->ucRoamingAuthTypes |= AUTH_TYPE_OPEN_SYSTEM;
        }
        else if (prJoinInfo->ucCurrAuthAlgNum == AUTH_ALGORITHM_NUM_SHARED_KEY) {
            prJoinInfo->ucRoamingAuthTypes |= AUTH_TYPE_SHARED_KEY;
        }

        prBssInfo->ucRoamingAuthTypes = prJoinInfo->ucRoamingAuthTypes;


        /* Set the stable time of the associated BSS. We won't do roaming decision
         * during the stable time.
         */
        SET_EXPIRATION_TIME(prBssInfo->rRoamingStableExpirationTime,
            SEC_TO_SYSTIME(ROAMING_STABLE_TIMEOUT_SEC));
    }


    //4 <3.H> Update Parameter for TX Fragmentation Threshold
#if CFG_TX_FRAGMENT
    txFragInfoUpdate(prAdapter);
#endif /* CFG_TX_FRAGMENT */


//4 <4> Update STA_RECORD_T
    /* Get a Station Record if possible */
    prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                        prBssDesc->aucBSSID);

    if (prStaRec) {
        UINT_16 u2OperationalRateSet, u2DesiredRateSet;

        //4 <4.A> Desired Rate Set
        u2OperationalRateSet = (rPhyAttributes[prBssInfo->ePhyType].u2SupportedRateSet &
                                prBssInfo->u2OperationalRateSet);

        u2DesiredRateSet = (u2OperationalRateSet & prConnSettings->u2DesiredRateSet);
        if (u2DesiredRateSet) {
            prStaRec->u2DesiredRateSet = u2DesiredRateSet;
        }
        else {
            /* For Error Handling - The Desired Rate Set is not covered in Operational Rate Set. */
            prStaRec->u2DesiredRateSet = u2OperationalRateSet;
        }

        /* Try to set the best initial rate for this entry */
        if (!rateGetBestInitialRateIndex(prStaRec->u2DesiredRateSet,
                                         prStaRec->rRcpi,
                                         &prStaRec->ucCurrRate1Index)) {

            if (!rateGetLowestRateIndexFromRateSet(prStaRec->u2DesiredRateSet,
                                                   &prStaRec->ucCurrRate1Index)) {
                ASSERT(0);
            }
        }

        DBGLOG(JOIN, INFO, ("prStaRec->ucCurrRate1Index = %d\n",
            prStaRec->ucCurrRate1Index));

        //4 <4.B> Preamble Mode
        prStaRec->fgIsShortPreambleOptionEnable =
            prBssInfo->fgUseShortPreamble;

        //4 <4.C> QoS Flag
        prStaRec->fgIsQoS = prBssInfo->fgIsWmmAssoc;
    }
#if DBG
    else {
        ASSERT(0);
    }
#endif /* DBG */


//4 <5> Update NIC
    //4 <5.A> Update BSSID & Operation Mode
    nicSetupBSS(prAdapter, prBssInfo);

    //4 <5.B> Update WLAN Table.
    if (nicSetHwBySta(prAdapter, prStaRec) == FALSE) {
        ASSERT(FALSE);
    }

    //4 <5.C> Update Desired Rate Set for BT.
#if CFG_TX_FRAGMENT
    if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
        txRateSetInitForBT(prAdapter, prStaRec);
    }
#endif /* CFG_TX_FRAGMENT */

    //4 <5.D> TX AC Parameter and TX/RX Queue Control
    if (prBssInfo->fgIsWmmAssoc) {

#if CFG_TX_AGGREGATE_HW_FIFO
        nicTxAggregateTXQ(prAdapter, FALSE);
#endif /* CFG_TX_AGGREGATE_HW_FIFO */

        qosUpdateWMMParametersAndAssignAllowedACI(prAdapter, &prBssInfo->rWmmInfo);
    }
    else {

#if CFG_TX_AGGREGATE_HW_FIFO
        nicTxAggregateTXQ(prAdapter, TRUE);
#endif /* CFG_TX_AGGREGATE_HW_FIFO */

        nicTxNonQoSAssignDefaultAdmittedTXQ(prAdapter);

        nicTxNonQoSUpdateTXQParameters(prAdapter,
                                       prBssInfo->ePhyType);
    }

#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
    {
        prTxCtrl->fgBlockTxDuringJoin = FALSE;

    #if !CFG_TX_AGGREGATE_HW_FIFO /* TX FIFO AGGREGATE already do flush once */
        nicTxFlushStopQueues(prAdapter, (UINT_8)TXQ_DATA_MASK, 0x0 /*(UINT_8)NULL*/);
    #endif /* CFG_TX_AGGREGATE_HW_FIFO */

        nicTxRetransmitOfSendWaitQue(prAdapter);

        if (prTxCtrl->fgIsPacketInOsSendQueue) {
            nicTxRetransmitOfOsSendQue(prAdapter);
        }

    #if CFG_SDIO_TX_ENHANCE
        halTxLeftClusteredMpdu(prAdapter);
    #endif /* CFG_SDIO_TX_ENHANCE */

    }
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */


//4 <6> Setup CONNECTION flag.
    prAdapter->eConnectionState = MEDIA_STATE_CONNECTED;
    prAdapter->eConnectionStateIndicated = MEDIA_STATE_CONNECTED;

    if (prJoinInfo->fgIsReAssoc) {
        prAdapter->fgBypassPortCtrlForRoaming = TRUE;
    }
    else {
        prAdapter->fgBypassPortCtrlForRoaming = FALSE;
    }

    kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
        WLAN_STATUS_MEDIA_CONNECT,
        (PVOID)NULL,
        0);

    return;
} /* end of joinComplete() */


