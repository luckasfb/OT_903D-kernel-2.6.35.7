






#include "precomp.h"




#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugSecState[SEC_STATE_NUM] = {
    DISP_STRING("SEC_STATE_IDLE"),
    DISP_STRING("SEC_STATE_PORT_CONTROL"),
    DISP_STRING("SEC_STATE_FIFO_BUSY"),
    DISP_STRING("SEC_STATE_CHECK_OK"),
    DISP_STRING("SEC_STATE_SEND_EAPOL"),
    DISP_STRING("SEC_STATE_SEND_DEAUTH"),
};
/*lint -restore */
#endif /* DBG */


VOID
secFsmRunEventFifoEmpty(
    IN P_ADAPTER_T          prAdapter
    );


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_SEC_INFO_T prSecInfo;
    UINT_8       i;

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;
    ASSERT(prSecInfo);

    //AT WHQL Standby test, driver do chip reset on the way to D3,
    //While driver back from D3, no infrastructure and key will set,
    //driver needed to resotore the key exist in prAdapter->arWlanCtrl to HW
    if (!privacyRsnKeyHandshakeEnabled(prAdapter)) {
        for (i = 0; i < WLAN_TABLE_SIZE; i++) {
            if (prAdapter->arWlanCtrl[i].fgUsed) {
                if (!nicPrivacySetWlanTable(prAdapter, i, NULL,
                    &prAdapter->arWlanCtrl[i].rCtrl,
                    &prAdapter->arWlanCtrl[i].rSWKey)){
                    DBGLOG(RSN, TRACE, ("secFsmInit fail!\n"));
                    return;
                }
            }
        }
    }

    privacyClearPmkid(prAdapter);
    nicPrivacyEnableHwTxPortControl(prAdapter);

    prSecInfo->fgBlockOnlyAPTraffic = TRUE;
    prSecInfo->fgBlockTxTraffic = FALSE;
    prSecInfo->fgBlockRxTraffic = FALSE;

    prSecInfo->fgCheckEAPoLTxDone = FALSE;

    prSecInfo->eCurrentState = SEC_STATE_IDLE;

    ARB_INIT_TIMER(prAdapter,
                   prSecInfo->rRsnaEAPoLReportTimeoutTimer,
                   arbFsmRunEventSecCancelEAPoLTimer,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prSecInfo->rRsnaBlockTrafficTimer,
                   arbFsmRunEventSecCounterMeasureDone,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prSecInfo->rPreauthenticationTimer,
                   arbFsmRunEventSecIndicatePmkidCand,
                   TRUE);

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_IDLE_to_PORT_CONTROL (
    IN P_ADAPTER_T prAdapter
    )
{

    ASSERT(prAdapter);

    nicPrivacyPortControl(prAdapter, prAdapter->rBssInfo.aucBSSID, TRUE, TRUE);

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_IDLE_to_CHECK_OK (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    ASSERT((prAdapter->arWlanCtrl[0].rCtrl.fgT1X == FALSE) &&
        (prAdapter->arWlanCtrl[0].rCtrl.fgR1X == FALSE));
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_PORT_CONTROL_to_FIFO_BUSY (
    IN P_ADAPTER_T prAdapter
    )
{
    return;
 }


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_PORT_CONTROL_to_CHECK_OK (
    IN P_ADAPTER_T          prAdapter
    )
{

    ASSERT(prAdapter);

    nicPrivacyPortControl(prAdapter, prAdapter->rBssInfo.aucBSSID, FALSE, FALSE);

    prAdapter->fgBypassPortCtrlForRoaming = FALSE;

    nicTxStartQueues(prAdapter, (TXQ_DATA_MASK | TXQ_MGMT_MASK) );

    DBGLOG(ARB, TRACE, ("secFsmTransAction_PORT_CONTROL_to_CHECK_OK\n"));
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_FIFO_BUSY_to_CHECK_OK (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    prAdapter->fgBypassPortCtrlForRoaming = FALSE;

    nicTxStartQueues(prAdapter, (TXQ_DATA_MASK | TXQ_MGMT_MASK) );

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_CHECK_OK_to_SEND_EAPOL (
    IN P_ADAPTER_T prAdapter
    )
{
    P_SEC_INFO_T   prSecInfo;

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;
    ASSERT(prSecInfo);

    prSecInfo->fgCheckEAPoLTxDone = TRUE;

    ARB_SET_TIMER(prAdapter,
                  prSecInfo->rRsnaEAPoLReportTimeoutTimer,
                  SEC_TO_MSEC(EAPOL_REPORT_SEND_TIMEOUT_INTERVAL_SEC));

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_CHECK_OK_to_FIFO_BUSY (
    IN P_ADAPTER_T prAdapter
    )
{
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_SEND_EAPOL_to_SEND_DEAUTH (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    /* Compose deauth frame to AP, a call back function for tx done */
    if (authSendDeauthFrame(prAdapter,
                            prAdapter->rBssInfo.aucBSSID,
                            REASON_CODE_MIC_FAILURE,
                            AC3,
                            arbFsmRunEventSecDeauthDone) != WLAN_STATUS_SUCCESS) {
        ASSERT(FALSE);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
secFsmTransAction_SEND_DEAUTH_to_IDLE (
    IN P_ADAPTER_T prAdapter
    )
{

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmSteps (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_SEC_STATE_T     eNextState
    )
{
    P_SEC_INFO_T            prSecInfo;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;
    ASSERT(prSecInfo);

    DEBUGFUNC("secFsmSteps");
    do {
        /* Do entering Next State */
        DBGLOG(RSN, STATE, ("\nTRANSITION: [%s] -> [%s]\n\n",
                            apucDebugSecState[prSecInfo->eCurrentState],
                            apucDebugSecState[eNextState]));

        prSecInfo->eCurrentState = eNextState;

        /* Do nothing at this State */

    }
    while (fgIsTransition);

    return;

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventStart (
    IN P_ADAPTER_T          prAdapter
    )
{
    P_SEC_INFO_T            prSecInfo;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;
    ENUM_SEC_STATE_T        eNextState;

    DBGLOG(RSN, TRACE, ("secFsmRunEventStart\n"));

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;
    eNextState = prSecInfo->eCurrentState;

    if (prSecInfo == NULL)
        return;

    prAdapter->rSecInfo.u4RsnaLastMICFailTime = 0;

    arbFsmRunEventSecTxFlowControl(prAdapter, TRUE);

    if (privacyRsnKeyHandshakeEnabled(prAdapter) == TRUE) {
        prAdapter->rConnSettings.rMib.dot11TranmitKeyAvailable = FALSE;
        nicPrivacyInitialize(prAdapter);
        SEC_STATE_TRANSITION(prAdapter, IDLE, PORT_CONTROL);
    }
    else {
#if SUPPORT_WAPI
        prAdapter->rConnSettings.rMib.fgWapiKeyInstalled = FALSE;
#endif
        SEC_STATE_TRANSITION(prAdapter, IDLE, CHECK_OK);
    }

    secFsmSteps(prAdapter, eNextState);

    return;
} /* secFsmRunEventStart */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventAbort (
    IN P_ADAPTER_T          prAdapter
    )
{
    P_SEC_INFO_T            prSecInfo;

    DBGLOG(RSN, TRACE, ("secFsmRunEventAbort\n"));

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;

    if (prSecInfo == NULL)
        return;

    if (prSecInfo->eCurrentState == SEC_STATE_SEND_EAPOL) {
        if (prSecInfo->fgCheckEAPoLTxDone == FALSE) {
            DBGLOG(RSN, TRACE, ("EAPOL STATE not match the flag\n"));
            ARB_CANCEL_TIMER(prAdapter, prSecInfo->rRsnaEAPoLReportTimeoutTimer);
        }
    }

    prSecInfo->fgBlockTxTraffic = FALSE;
    prSecInfo->fgBlockRxTraffic = FALSE;

#if SUPPORT_WAPI
    prAdapter->rConnSettings.rMib.fgWapiKeyInstalled = FALSE;
    if (prAdapter->fgUseWapi && prAdapter->fgWapiMode) {
        prSecInfo->fgBlockTxTraffic = TRUE;
        prSecInfo->fgBlockRxTraffic = TRUE;
    }
#endif
    prSecInfo->eCurrentState = SEC_STATE_IDLE;

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
secFsmRunEventPTKInstalled (
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid,
    IN PUINT_8              pucKeyMaterial,
    IN UINT_8               ucKeyLen,
    IN UINT_8               ucCipherMode,
    IN UINT_8               ucTxMicOffset,
    IN UINT_8               ucRxMicOffset
    )
{
    P_SEC_INFO_T            prSecInfo;
    ENUM_SEC_STATE_T        eNextState;
    BOOLEAN                 fgStatus = TRUE;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;
    eNextState = prSecInfo->eCurrentState;

    switch(prSecInfo->eCurrentState) {
    case SEC_STATE_PORT_CONTROL:
        if (arbFsmRunEventSecTxFlowControl(prAdapter, FALSE)==FALSE){
            if (!nicPrivacySetKeyEntry(prAdapter,
                TRUE,
                pucBssid,
                0,
                pucKeyMaterial,
                ucKeyLen,
                ucCipherMode,
                ucTxMicOffset,
                ucRxMicOffset)){
                DBGLOG(RSN, TRACE, ("secFsmRunEventPTKInstalled fail!\n"));
            }
            SEC_STATE_TRANSITION(prAdapter, PORT_CONTROL, CHECK_OK);
        }
        else {
            nicPrivacySetKeyToTemplateEntry(prAdapter,
                pucBssid,
                pucKeyMaterial,
                ucKeyLen,
                ucCipherMode,
                ucTxMicOffset,
                ucRxMicOffset);
            nicPrivacyInvalidEntryRx(prAdapter, pucBssid);
            SEC_STATE_TRANSITION(prAdapter, PORT_CONTROL, FIFO_BUSY);
        }
        break;

    case SEC_STATE_FIFO_BUSY:
        /* Keep at this state */
        /* oid query this and return TRUE or FALSE to
           Overwrite or do nothing for add PTK */
        fgStatus = FALSE;                      
        break;

    case SEC_STATE_CHECK_OK:
        if (arbFsmRunEventSecTxFlowControl(prAdapter, FALSE)==FALSE ||
            prAdapter->eConnectionState != MEDIA_STATE_CONNECTED){
            if (!nicPrivacySetKeyEntry(prAdapter,
                    TRUE,
                    pucBssid,
                    0,
                    pucKeyMaterial,
                    ucKeyLen,
                    ucCipherMode,
                    ucTxMicOffset,
                    ucRxMicOffset)) {
                DBGLOG(RSN, TRACE, ("secFsmRunEventPTKInstalled fail!\n"));
            }
            /* Not Transition at this state */
        }
        else {
            nicPrivacySetKeyToTemplateEntry(prAdapter,
                pucBssid,
                pucKeyMaterial,
                ucKeyLen,
                ucCipherMode,
                ucTxMicOffset,
                ucRxMicOffset);
            nicPrivacyInvalidEntryRx(prAdapter, pucBssid);
            SEC_STATE_TRANSITION(prAdapter, CHECK_OK, FIFO_BUSY);
        }
        break;

    default:

        if (!nicPrivacySetKeyEntry(prAdapter,
                                  TRUE,
                                  pucBssid,
                                  0,
                                  pucKeyMaterial,
                                  ucKeyLen,
                                  ucCipherMode,
                                  ucTxMicOffset,
                                  ucRxMicOffset)){
            fgStatus = FALSE;                      
        }

        break;
    }

    if (prSecInfo->eCurrentState != eNextState) {
        secFsmSteps(prAdapter, eNextState);
    }

    return fgStatus;

} /* end of secFsmRunEventPTKInstalled() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventFifoEmpty (
    IN P_ADAPTER_T          prAdapter
    )
{
    P_SEC_INFO_T            prSecInfo;
    ENUM_SEC_STATE_T        eNextState;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;

    ASSERT(prAdapter);
    
    prSecInfo = &prAdapter->rSecInfo;
    ASSERT(prSecInfo);
    
    eNextState = prSecInfo->eCurrentState;

    switch(prSecInfo->eCurrentState) {
    case SEC_STATE_PORT_CONTROL:
        /* Copy entry from template entry */
        if (nicPrivacyCopyFromTemplateEntry(prAdapter) == FALSE){
            break;
        }
        nicPrivacyInvalidTemplateEntry(prAdapter);
        SEC_STATE_TRANSITION(prAdapter, PORT_CONTROL, CHECK_OK);
        break;

    case SEC_STATE_FIFO_BUSY:

        /* Copy entry from template entry */
        if (nicPrivacyCopyFromTemplateEntry(prAdapter) == FALSE){
            break;
        }
        nicPrivacyInvalidTemplateEntry(prAdapter);
        SEC_STATE_TRANSITION(prAdapter, FIFO_BUSY, CHECK_OK);
        break;

    case SEC_STATE_CHECK_OK:
        DBGLOG(RSN, TRACE, ("Do nothing while at this state!\n"));
        break;
        
    default:
        ERRORLOG(("Called at un-expected state [%s]\n ", apucDebugSecState[prSecInfo->eCurrentState]));
        break;
    }

    if (prSecInfo->eCurrentState != eNextState) {
        secFsmSteps(prAdapter, eNextState);
    }

    return;
} /* end of secFsmRunEventFifoEmpty() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventStartCounterMeasure (
    IN P_ADAPTER_T          prAdapter
    )
{
    P_SEC_INFO_T            prSecInfo;
    ENUM_SEC_STATE_T        eNextState;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;

    DEBUGFUNC("secFsmRunEventStartCounterMeasure");

    ASSERT(prAdapter);

    prSecInfo = &prAdapter->rSecInfo;
    eNextState = prSecInfo->eCurrentState;

    DBGLOG(RSN, TRACE, ("Sec state %s\n", apucDebugSecState[prSecInfo->eCurrentState]));

    prAdapter->rSecInfo.u4RsnaLastMICFailTime = 0;

    switch(prSecInfo->eCurrentState) {
    case SEC_STATE_CHECK_OK:
        {
            //<Todo> dls port control
            SEC_STATE_TRANSITION(prAdapter, CHECK_OK, SEND_EAPOL);
        }
        break;

    default:
        break;
    }

    /* Call arbFsmSteps() when we are going to change ARB STATE */
    if (prSecInfo->eCurrentState != eNextState) {
        secFsmSteps(prAdapter, eNextState);
    }

    return;

} /* secFsmRunEventStartCounterMeasure */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventEapolTxDone (
    IN P_ADAPTER_T          prAdapter

    )
{
    P_SEC_INFO_T            prSecInfo;
    ENUM_SEC_STATE_T        eNextState;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;

    DEBUGFUNC("secFsmRunEventEapolTxDone");

    ASSERT(prAdapter);
    prSecInfo = &prAdapter->rSecInfo;
    eNextState = prSecInfo->eCurrentState;

    DBGLOG(RSN, TRACE, ("Sec state %s\n", apucDebugSecState[prSecInfo->eCurrentState]));

    switch(prSecInfo->eCurrentState) {
    case SEC_STATE_SEND_EAPOL:
        if (prSecInfo->fgCheckEAPoLTxDone == FALSE) {
            ASSERT(0);
        }

        prSecInfo->fgCheckEAPoLTxDone = FALSE;
        ARB_CANCEL_TIMER(prAdapter, prSecInfo->rRsnaEAPoLReportTimeoutTimer);

        SEC_STATE_TRANSITION(prAdapter, SEND_EAPOL, SEND_DEAUTH);
        break;
    default:
        break;
    }

    if (prSecInfo->eCurrentState != eNextState) {
        secFsmSteps(prAdapter, eNextState);
    }

    return;

}/* secFsmRunEventEapolTxDone */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventDeauthTxDone (
    IN P_ADAPTER_T  prAdapter

    )
{
    P_SEC_INFO_T     prSecInfo;

    DEBUGFUNC("secFsmRunEventDeauthTxDone");

    ASSERT(prAdapter);
    
    prSecInfo = &prAdapter->rSecInfo;
    DBGLOG(RSN, TRACE, ("Sec state %s\n", apucDebugSecState[prSecInfo->eCurrentState]));

    switch(prSecInfo->eCurrentState) {
    case SEC_STATE_SEND_DEAUTH:

        DBGLOG(RSN, TRACE, ("Set timer %d\n", COUNTER_MEASURE_TIMEOUT_INTERVAL_SEC));

        arbFsmRunEventConnectionStartCounterMeasure(prAdapter);
        ARB_SET_TIMER(prAdapter,
                      prSecInfo->rRsnaBlockTrafficTimer,
                      SEC_TO_MSEC(COUNTER_MEASURE_TIMEOUT_INTERVAL_SEC));
        break;

    default:
        ASSERT(0);
        break;
    }

    return;
}/* secFsmRunEventDeauthTxDone */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
secFsmRunEventEndOfCounterMeasure (
    IN P_ADAPTER_T          prAdapter
    )
{
    P_SEC_INFO_T            prSecInfo;
    ENUM_SEC_STATE_T        eNextState;
    BOOLEAN                 fgIsTransition = (BOOLEAN)FALSE;

    DEBUGFUNC("secFsmRunEventEndOfCounterMeasure");

    ASSERT(prAdapter);
    prSecInfo = &prAdapter->rSecInfo;
    eNextState = prSecInfo->eCurrentState;
    
    DBGLOG(RSN, TRACE, ("Sec state %s\n", apucDebugSecState[prSecInfo->eCurrentState]));

    switch(prSecInfo->eCurrentState) {
    case SEC_STATE_SEND_DEAUTH:
        {
            SEC_STATE_TRANSITION(prAdapter, SEND_DEAUTH, IDLE);

            //info the arb change from NORMAL_TR to SEARCH
            //delay to 60 sec timeout event
            arbFsmRunEventConnectionEndOfCounterMeasure(prAdapter);
        }
        break;

    default:
        ASSERT(0);
    }

    /* Call arbFsmSteps() when we are going to change ARB STATE */
    if (prSecInfo->eCurrentState != eNextState) {
        secFsmSteps(prAdapter, eNextState);
    }

    return;
}/* end of secFsmRunEventEndOfCounterMeasure */

