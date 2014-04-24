






#include "precomp.h"




#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugRoamingState[ROAMING_STATE_NUM] = {
    DISP_STRING("ROAMING_STATE_IDLE"),
    DISP_STRING("ROAMING_STATE_DECISION"),
    DISP_STRING("ROAMING_STATE_DISCOVERY"),
    DISP_STRING("ROAMING_STATE_ROAM"),
};
/*lint -restore */
#endif /* DBG */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
roamingFsmTransAction_DECISION_to_IDLE (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    /* NOTE(Kevin): Cancel Timer & Event INT. */

    /* Stop RCPI threshold immediately */
    nicRRTriggerDisable(prAdapter);

    prRoamingInfo->fgIsRCPIEventEnabled = FALSE;

    ARB_CANCEL_TIMER(prAdapter,
                     prRoamingInfo->rRoamingDecisionTimer);

    return;
} /* end of roamingFsmTransAction_DECISION_to_IDLE() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
roamingFsmTransAction_DISCOVERY_to_IDLE (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    /* NOTE(Kevin): Cancel Timer but No Event INT. */
    ARB_CANCEL_TIMER(prAdapter,
                     prRoamingInfo->rRoamingDiscoveryTimer);


    prRoamingInfo->fgIsScanTriggered = FALSE;
    prRoamingInfo->fgIsScanCompleted = FALSE;

    return;
} /* end of roamingFsmTransAction_DISCOVERY_to_IDLE() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
roamingFsmTransAction_ROAM_to_IDLE (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    /* NOTE(Kevin): No Timer & No Event INT to cancel. */
    return;
} /* end of roamingFsmTransAction_ROAM_to_IDLE() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
roamingFsmTransAction_DECISION_to_DISCOVERY (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    return;
} /* end of roamingFsmTransAction_DECISION_to_DISCOVERY() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
roamingFsmTransAction_DISCOVERY_to_ROAM (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    return;
} /* end of roamingFsmTransAction_DISCOVERY_to_ROAM() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
roamingFsmInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    prRoamingInfo->eCurrentState = ROAMING_STATE_IDLE;


    prRoamingInfo->rRoamingDiscoveryUpdateTime = 0;

    prRoamingInfo->fgIsScanTriggered = FALSE;
    prRoamingInfo->fgIsScanCompleted = FALSE;
    prRoamingInfo->fgIsRCPIEventEnabled = FALSE;
    prRoamingInfo->fgIsRoamingFail = FALSE;

    ARB_INIT_TIMER(prAdapter,
                   prRoamingInfo->rRoamingDecisionTimer,
                   arbFsmRunEventRoamingDecision,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prRoamingInfo->rRoamingDiscoveryTimer,
                   arbFsmRunEventRoamingDiscovery,
                   TRUE);

    return;
} /* end of roamingFsmInit() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
roamingFsmSteps (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_ROAMING_STATE_T eNextState
    )
{
    P_ROAMING_INFO_T prRoamingInfo;
    P_BSS_INFO_T prBssInfo;
    P_BSS_DESC_T prBssDesc;
    BOOLEAN fgIsTransition = (BOOLEAN)FALSE;
    /* If the signal of AP is stable and good, the default STATUS is PENDING */
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("roamingFsmSteps");


    ASSERT(prAdapter);
    prBssInfo = &prAdapter->rBssInfo;
    prRoamingInfo = &prAdapter->rRoamingInfo;

    prConnSettings = &prAdapter->rConnSettings;

    do {
        DBGLOG(ROAMING, STATE, ("TRANSITION: [%s] -> [%s]\n",
                                apucDebugRoamingState[prRoamingInfo->eCurrentState],
                                apucDebugRoamingState[eNextState]));

        prRoamingInfo->eCurrentState = eNextState;


        fgIsTransition = (BOOLEAN)FALSE;
        switch (prRoamingInfo->eCurrentState) {

        case ROAMING_STATE_DECISION:
            {
                OS_SYSTIME rCurrentTime;
                RCPI rRcpi;

                do {
                    GET_CURRENT_SYSTIME(&rCurrentTime);

                    DBGLOG(ROAMING, INFO, ("rCurrentTime = %08lx, prBssInfo->rRoamingStableExpirationTime = %08lx\n",
                        rCurrentTime, prBssInfo->rRoamingStableExpirationTime));

                    //4 <1> Check if we has sufficient stable time
                    /* Do ROAMING decision after the Stable Timer timeout to avoid frequently roaming. */
                    if (!CHECK_FOR_EXPIRATION(rCurrentTime,
                            prBssInfo->rRoamingStableExpirationTime)) {

                        DBGLOG(ROAMING, INFO, ("Set stable period after roaming in BSS.\n"));

                        nicRRTriggerDisable(prAdapter);
                        prRoamingInfo->fgIsRCPIEventEnabled = FALSE;


                        /* TODO(Kevin): The stable time also can be adjustable according some external conditions
                         * - eg. temperature or current channel load.
                         */
                        ARB_SET_TIMER(prAdapter,
                                      prRoamingInfo->rRoamingDecisionTimer,
                                      SEC_TO_MSEC(ROAMING_STABLE_TIMEOUT_SEC));

                        prRoamingInfo->fgIsRoamingFail = FALSE;
                        break;
                    }


#if SUPPORT_WPS
                    /* CR1486, CR1640 */
                    /* Notice, at WPS mode, no key set, so the wzc profile control the connection */
                    if (
#if 0//SUPPORT_WAPI
                        (!prAdapter->fgUseWapi) &&
#endif
                        (prAdapter->rSecInfo.fgPrivacyCheckDisable) &&
                        (prAdapter->rConnSettings.eAuthMode < AUTH_MODE_WPA) &&
                        (prAdapter->rConnSettings.eOPMode == NET_TYPE_INFRA)) {

                        DBGLOG(RSN, TRACE, ("Disable roaming while at wps mode!!\n"));
                        ARB_SET_TIMER(prAdapter,
                                      prRoamingInfo->rRoamingDecisionTimer,
                                      SEC_TO_MSEC(ROAMING_STABLE_TIMEOUT_SEC));
                        break;
                    }
#endif
                    if (privacyRsnKeyHandshakeEnabled(prAdapter) &&
                        !privacyTransmitKeyExist(prAdapter)) {

                        DBGLOG(ROAMING, INFO, ("Reschedule timer because wpa handshake not complete!!\n"));

                        ARB_SET_TIMER(prAdapter,
                                      prRoamingInfo->rRoamingDecisionTimer,
                                      SEC_TO_MSEC(ROAMING_STABLE_TIMEOUT_SEC));
                        break;
                    }

                    //4 <2> Get Current RCPI value.
                    nicRRGetRCPI(prAdapter, &rRcpi);


                    //4 <3> Check if previous roaming was failed
                    if (prRoamingInfo->fgIsRoamingFail) {
                        RCPI rRcpiHighBound, rRcpiLowBound;
                        RCPI rLowBoundStep;

                        /* NOTE: When we do association, fgIsRCPIEventEnabled should
                         * be FALSE to reduce RCPI INT noise.
                         */
                        ASSERT(!prRoamingInfo->fgIsRCPIEventEnabled);

                        if (rRcpi == RCPI_MEASUREMENT_NOT_AVAILABLE) {

                            ASSERT(0);

                            /* Use previous RCPI value */
                            rRcpi = prBssInfo->rRcpi;
                        }
                        else {
                            //4 <3.A> Also update RCPI /RSSI to BSS_INFO_T for information
                            prBssInfo->rRcpi = rRcpi;
                            prBssInfo->rRssi = RCPI_TO_dBm(rRcpi);
                            GET_CURRENT_SYSTIME(&prBssInfo->rRssiLastUpdateTime);

                            //4 <3.B> Also update RCPI to P_BSS_DESC_T for compare
                            if ((prBssDesc = scanSearchBssDescByBssid(prAdapter,
                                    prBssInfo->aucBSSID)) != (P_BSS_DESC_T)NULL) {
                                prBssDesc->rRcpi = rRcpi;
                                prBssDesc->rUpdateTime = prBssInfo->rRssiLastUpdateTime;
                            }
                        }

                        rRcpiHighBound = rRcpi + ROAMING_RCPI_STEP;

                        rLowBoundStep = ((rRcpi - ROAMING_RCPI_STEP) <= ROAMING_WLAN_RCPI_BOUNDARY_THRESHOLD) ? \
                                        ROAMING_RCPI_BOUNDARY_STEP : ROAMING_RCPI_STEP;

                        rRcpiLowBound = ((rRcpi - rLowBoundStep) > RCPI_LOW_BOUND) ? \
                                        (rRcpi - rLowBoundStep) : \
                                        RCPI_LOW_BOUND;

                        DBGLOG(ROAMING, INFO, ("<1> Current RCPI = %d, RSSI = %ld\n",
                            prBssInfo->rRcpi, prBssInfo->rRssi));

                        /* NOTE: (The RCPI INT will arise sooner or later) */

                        /* Enable both High and Low Threshold */
                        nicRRTriggerEnable(prAdapter,
                                           rRcpiHighBound,
                                           rRcpiLowBound);

                        prRoamingInfo->fgIsRCPIEventEnabled = TRUE;

                        prRoamingInfo->fgIsRoamingFail = FALSE;
                        break;
                    }


                    //4 <4> Error handling of RCPI value.
                    if (rRcpi == RCPI_MEASUREMENT_NOT_AVAILABLE) {

                        ASSERT(0);

                        /* NOTE: To read RCPI value again later.
                         * If RCPI Event was enabled, RCPI INT will still come in.
                         * If RCPI Event was NOT enabled, set timer to read it later.
                         * (e.g. The Decision Event after Roaming Stable Timeout.)
                         */
                        if (!prRoamingInfo->fgIsRCPIEventEnabled) {
                            ARB_SET_TIMER(prAdapter,
                                          prRoamingInfo->rRoamingDecisionTimer,
                                          ROAMING_DECISION_INVALID_RCPI_TIMEOUT_MSEC);
                            break;
                        }
                    }


                    //4 <5.A> Update RCPI /RSSI to BSS_INFO_T
                    ASSERT(rRcpi <= RCPI_HIGH_BOUND);

                    prBssInfo->rRcpi = rRcpi;
                    prBssInfo->rRssi = RCPI_TO_dBm(rRcpi);
                    GET_CURRENT_SYSTIME(&prBssInfo->rRssiLastUpdateTime);

                    //4 <5.B> Also update RCPI to P_BSS_DESC_T for compare
                    if ((prBssDesc = scanSearchBssDescByBssid(prAdapter,
                            prBssInfo->aucBSSID)) != (P_BSS_DESC_T)NULL) {
                        prBssDesc->rRcpi = rRcpi;
                        prBssDesc->rUpdateTime = prBssInfo->rRssiLastUpdateTime;
                    }

                    DBGLOG(ROAMING, INFO, ("<2> Current RCPI = %d, RSSI = %ld\n",
                        prBssInfo->rRcpi, prBssInfo->rRssi));


                    //4 <6> Good Signal Field
                    if (prBssInfo->rRcpi >= ROAMING_WLAN_RCPI_THRESHOLD) {

                        DBGLOG(ROAMING, INFO, ("Good Signal Field, Set RCPI threshold = [%d/%d].\n",
                            ROAMING_WLAN_RCPI_THRESHOLD, RCPI_HIGH_BOUND));

                        /* Enable Low Threshold */
                        nicRRTriggerEnable(prAdapter,
                                           RCPI_HIGH_BOUND,
                                           ROAMING_WLAN_RCPI_THRESHOLD);

                        prRoamingInfo->fgIsRCPIEventEnabled = TRUE;
                    }
                    //4 <7> Poor Signal Field
                    else {

                        DBGLOG(ROAMING, INFO, ("Poor Signal Field, Disable RCPI threshold for SCAN.\n"));

                        /* NOTE: (The INT will not arise, but still average RR) */

                        /* Disable both High and Low Threshold, try to Scan and Join */
                        nicRRTriggerDisable(prAdapter);

                        prRoamingInfo->fgIsRCPIEventEnabled = FALSE;

                        /* GSM/WLAN dual band roaming indication */
                        if (prAdapter->fgIsEnableRoamingGsm) {
                            if (prBssInfo->rRcpi >= ROAMING_GSM_RCPI_HIGH_THRESHOLD) {
                                /* TODO(Kevin): call kal() Functions to suggest roam back to WLAN */
                            }
                            else if (prBssInfo->rRcpi <= ROAMING_GSM_RCPI_LOW_THRESHOLD) {
                                /* TODO(Kevin): call kal() Functions to suggest roam back to GSM */
                            }
                        }

                        ASSERT(!prRoamingInfo->fgIsScanTriggered);
                        ASSERT(!prRoamingInfo->fgIsScanCompleted);

#if 0 /* NOTE(Kevin): now we won't check for TimeOut, always apply Scan Request */
                        if (!CHECK_FOR_TIMEOUT(rCurrentTime,
                                              prRoamingInfo->rRoamingDiscoveryUpdateTime,
                                              SEC_TO_SYSTIME(ROAMING_DISCOVERY_TIMEOUT_SEC))) {
                            ROAMING_STATE_TRANSITION(prAdapter, DECISION, ROAM);
                        }
                        else
#endif
                        {
                            ROAMING_STATE_TRANSITION(prAdapter, DECISION, DISCOVERY);
                        }
                    }

                }
                while (FALSE);

            }
            break;

        case ROAMING_STATE_DISCOVERY:
            {
                P_SCAN_REQ_CONFIG_T prScanReqConfig = &prRoamingInfo->rScanReqConfig;

                /* Wait for SCAN completed */
                if (prRoamingInfo->fgIsScanTriggered) {

                    if (prRoamingInfo->fgIsScanCompleted) {
                        ROAMING_STATE_TRANSITION(prAdapter, DISCOVERY, ROAM);
                    }
                    else {
                        /* We will wait another 300 msec to check for SCAN result.
                         */
                        prRoamingInfo->u2DiscoveryTimeoutMillisecond = 300;

                        ARB_SET_TIMER(prAdapter,
                                      prRoamingInfo->rRoamingDiscoveryTimer,
                                      prRoamingInfo->u2DiscoveryTimeoutMillisecond);
                    }
                }
                /* Request for SCAN */
                else {
                    if (PM_IS_VOIP_POLLING_ENABLED(prAdapter)) {
                        prScanReqConfig->eScanMethod = SCAN_METHOD_VOIP_ONLINE_SCAN;

                        /* 12 milliseconds. If detect the air activity by MDRDY,
                         * this is the transmission time of 1Mbps with 1536 Bytes.
                         */
                        prScanReqConfig->ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

                        /* 20 milliseconds for VOIP application, N = 20 - Min(12) = 8 milliseconds */
                        prScanReqConfig->ucChnlDwellTimeExt = VOIP_SCAN_CHANNEL_DWELL_TIME_EXT;
                    }
                    else if (prConnSettings->fgIsVoipConn) {

                        prScanReqConfig->eScanMethod = SCAN_METHOD_VOIP_ONLINE_SCAN;

                        /* 12 milliseconds. If detect the air activity by MDRDY,
                         * this is the transmission time of 1Mbps with 1536 Bytes.
                         */
                        prScanReqConfig->ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

                        /* 20 milliseconds for VOIP application, N = 20 - Min(12) = 8 milliseconds */
                        prScanReqConfig->ucChnlDwellTimeExt = VOIP_SCAN_CHANNEL_DWELL_TIME_EXT;
                    }
                    else {
                        prScanReqConfig->eScanMethod = SCAN_METHOD_ONLINE_SCAN;

                        /* 12 milliseconds. If detect the air activity by MDRDY,
                         * this is the transmission time of 1Mbps with 1536 Bytes.
                         */
                        prScanReqConfig->ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

                        /* N milliseconds = 100TU(Beacon Interval) - min + 10TU
                         * (if Beacon Drift) = 98TU(round down to millisecond).
                         */
                        prScanReqConfig->ucChnlDwellTimeExt = SCAN_CHANNEL_DWELL_TIME_EXT;
                    }

                    prScanReqConfig->ucNumOfPrbReq = ROAMING_SCAN_TOTAL_PROBE_REQ_NUM;
                    prScanReqConfig->ucNumOfSpecifiedSsidPrbReq = \
                        ((prAdapter->rConnSettings.eConnectionPolicy == CONNECT_BY_SSID_ANY) ?
                             0 : ROAMING_SCAN_SPECIFIC_PROBE_REQ_NUM);

                    /* To increase roaming speed, we use following timeout to check
                     * if SCAN was completed.
                     */

                    /* The shortest time to complete SCAN will be
                     * 12 msec x 14 Channels = 168 msec.
                     * So at least we should wait for 300 msec to check SCAN result.
                     */
                    prRoamingInfo->u2DiscoveryTimeoutMillisecond = 300;

                    ARB_SET_TIMER(prAdapter,
                                  prRoamingInfo->rRoamingDiscoveryTimer,
                                  prRoamingInfo->u2DiscoveryTimeoutMillisecond);

                    GET_CURRENT_SYSTIME(&prRoamingInfo->rRoamingDiscoveryUpdateTime);

                    rStatus = WLAN_STATUS_ROAM_DISCOVERY;

                }
            }
            break;

        case ROAMING_STATE_ROAM:
            {
                prRoamingInfo->fgIsScanTriggered = FALSE;
                prRoamingInfo->fgIsScanCompleted = FALSE;

                rStatus = WLAN_STATUS_ROAM_OUT_FIND_BEST;

            }
            break;

        case ROAMING_STATE_IDLE:
            break;

        default:
            ASSERT(0);
            break;
        }


    }
    while (fgIsTransition);


    return rStatus;

} /* end of roamingFsmSteps() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
roamingFsmRunEventStart (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;
    P_BSS_INFO_T prBssInfo;
    P_SCAN_REQ_CONFIG_T prScanReqConfig;

    DEBUGFUNC("roamingFsmRunEventStart");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prScanReqConfig = &prRoamingInfo->rScanReqConfig;

    DBGLOG(ROAMING, EVENT, ("roamingFsmRunEventStart\n"));


    //4 <1> Validation of ROAMING Start Event
    if ((prAdapter->eConnectionState == MEDIA_STATE_DISCONNECTED) ||
        (prBssInfo->eBSSType == BSS_TYPE_IBSS)) {
        DBGLOG(ROAMING, ERROR, ("We should connect to an AP first before enabling the Roaming Module.\n"));
        return WLAN_STATUS_FAILURE;
    }

    //4 <2.A> Start from IDLE, this should be the 1st time JOIN.
    if (prRoamingInfo->eCurrentState == ROAMING_STATE_IDLE) {

        //4 <3> Update local SCAN Request Configuration Structure.
        kalMemZero(&prScanReqConfig->rSpecifiedSsid,
                   sizeof(PARAM_SSID_T));

        COPY_SSID(prScanReqConfig->rSpecifiedSsid.aucSsid,
                  prScanReqConfig->rSpecifiedSsid.u4SsidLen,
                  prBssInfo->aucSSID,
                  (UINT_32)prBssInfo->ucSSIDLen);

        prScanReqConfig->eScanType = SCAN_TYPE_ACTIVE_SCAN;

        prScanReqConfig->pfScanDoneHandler = roamingDiscoveryScanDoneHandler;
    }
    //4 <2.B> Start from ROAM, successfully roaming to other BSS.
    else if (prRoamingInfo->eCurrentState == ROAMING_STATE_ROAM) {
        /* NOTE: There are 2 cases from JOIN to NORMAL_TR.
         * 1. Do transition of JOIN to NORMAL_TR after joinComplete().
         *    So we'll update the rRoamingStableExpirationTime.
         *    If Roaming Stable was not expired, will set the fgIsRoamingFail to FALSE.
         * 2. Event of JOIN fail (e.g. JoinTxTimeOut()).
         *    So we'll set the fgIsRoamingFail = TRUE;
         */
        prRoamingInfo->fgIsRoamingFail = TRUE;
    }
    //4 <2.C> The previous ROAMING process is hanging in other STATE ?
    else {
        DBGLOG(ROAMING, ERROR, ("ROAMING Module is hanging in %s.\n",
            ((prRoamingInfo->eCurrentState == ROAMING_STATE_DECISION) ?
             "ROAMING_STATE_DECISION" : "ROAMING_STATE_ROAM")));
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }


    /* NOTE(Kevin): The number of RF channels we intend to scan will be setup after
     * receiving the Roaming Start Event each time.
     *     prScanReqConfig->arChnlInfoList[];
     *     prScanReqConfig->ucNumOfScanChnl;
     */

    {
        UINT_32 i, j, u4ScanIndex = 0;
        UINT_32 u4NumOfElem = 0;

        if (sizeof(prScanReqConfig->arChnlInfoList[0]) != 0) {
            u4NumOfElem = sizeof(prScanReqConfig->arChnlInfoList) /
                sizeof(prScanReqConfig->arChnlInfoList[0]);
        }

        prScanReqConfig->ucNumOfScanChnl = (UINT_8)prAdapter->u2NicOpChnlNum;

        for (j = 0; j < INTERLACED_SCAN_CHANNEL_GROUPS_NUM; j++) {

            for (i = j; i < prScanReqConfig->ucNumOfScanChnl;
                i += INTERLACED_SCAN_CHANNEL_GROUPS_NUM, u4ScanIndex++) {
                /* add boundary checking */
                if ((u4ScanIndex < u4NumOfElem) && (i < MAXIMUM_OPERATION_CHANNEL_LIST)) {
                    prScanReqConfig->arChnlInfoList[u4ScanIndex].ucChannelNum =
                        prAdapter->arNicOpChnList[i].ucChannelNum;

                    prScanReqConfig->arChnlInfoList[u4ScanIndex].eBand =
                        prAdapter->arNicOpChnList[i].eBand;
                }
                else {
                    ERRORLOG(("boundary checking fail: u4ScanIndex(%ld) < u4NumOfElem(%ld), i(%ld) < MAXIMUM_OPERATION_CHANNEL_LIST(%d)",
                        u4ScanIndex, u4NumOfElem, i, MAXIMUM_OPERATION_CHANNEL_LIST));
                    ASSERT(FALSE);
                }
            }
        }
    }

    /* Trigger ROAMING MODULE */
    return roamingFsmSteps(prAdapter, ROAMING_STATE_DECISION);

} /* end of roamingFsmRunEventStart() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
roamingFsmRunEventRoamFail (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;

    DEBUGFUNC("roamingFsmRunEventStart");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    DBGLOG(ROAMING, EVENT, ("\n\nroamingFsmRunEventRoamFail\n\n"));

    if (prRoamingInfo->eCurrentState == ROAMING_STATE_ROAM) {

        prRoamingInfo->fgIsRoamingFail = TRUE;

        /* Trigger ROAMING MODULE */
        roamingFsmSteps(prAdapter, ROAMING_STATE_DECISION);
    }

    return;
} /* end of roamingFsmRunEventRoamFail() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
roamingFsmRunEventAbort (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;
    ENUM_ROAMING_STATE_T eNextState;
    BOOLEAN fgIsTransition = (BOOLEAN)FALSE;

    DEBUGFUNC("roamingFsmRunEventAbort");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;
    eNextState = prRoamingInfo->eCurrentState;

    DBGLOG(ROAMING, EVENT, ("roamingFsmRunEventAbort\n"));

    //4 <1> Do abort event.
    switch(prRoamingInfo->eCurrentState) {

    case ROAMING_STATE_DECISION:
        {
            ROAMING_STATE_TRANSITION(prAdapter, DECISION, IDLE);
        }
        break;

    case ROAMING_STATE_DISCOVERY:
        {
            ROAMING_STATE_TRANSITION(prAdapter, DISCOVERY, IDLE);
        }
        break;

    case ROAMING_STATE_ROAM:
        {
            ROAMING_STATE_TRANSITION(prAdapter, ROAM, IDLE);
        }
        break;

    default:
        break;
    }

    /* Call arbFsmSteps() when we are going to change ARB STATE */
    if (prRoamingInfo->eCurrentState != eNextState) {
        roamingFsmSteps(prAdapter, eNextState);
    }

    return;
} /* end of roamingFsmRunEventAbort() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
roamingFsmRunEventRCPI (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;
    ENUM_ROAMING_STATE_T eNextState;
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("roamingFsmRunEventRCPI");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;
    eNextState = prRoamingInfo->eCurrentState;

    DBGLOG(ROAMING, EVENT, ("roamingFsmRunEventRCPI\n"));

    if (prRoamingInfo->eCurrentState == ROAMING_STATE_DECISION) {

        rStatus = roamingFsmSteps(prAdapter, eNextState);
    }
#if DBG
    else {
        ASSERT(0);
    }
#endif

    return rStatus;

} /* end of roamingFsmRunEventRCPI() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
roamingFsmRunEventDecision (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;
    ENUM_ROAMING_STATE_T eNextState;
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("roamingFsmRunEventDecision");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;
    eNextState = prRoamingInfo->eCurrentState;

    DBGLOG(ROAMING, EVENT, ("roamingFsmRunEventDecision\n"));

    if (prRoamingInfo->eCurrentState == ROAMING_STATE_DECISION) {

        rStatus = roamingFsmSteps(prAdapter, eNextState);
    }
#if DBG
    else {
        ASSERT(0);
    }
#endif

    return rStatus;

} /* end of roamingFsmRunEventDecision() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
roamingFsmRunEventDiscovery (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;
    ENUM_ROAMING_STATE_T eNextState;
    WLAN_STATUS rStatus = WLAN_STATUS_PENDING;

    DEBUGFUNC("roamingFsmRunEventDiscovery");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;
    eNextState = prRoamingInfo->eCurrentState;

    DBGLOG(ROAMING, EVENT, ("roamingFsmRunEventDiscovery\n"));

    if (prRoamingInfo->eCurrentState == ROAMING_STATE_DISCOVERY) {

        rStatus = roamingFsmSteps(prAdapter, eNextState);
    }
#if DBG
    else {
        ASSERT(0);
    }
#endif

    return rStatus;

} /* end of roamingFsmRunEventDiscovery() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
roamingDiscoveryScanDoneHandler (
    IN P_ADAPTER_T prAdapter,
    IN WLAN_STATUS rStatus
    )
{
    P_ROAMING_INFO_T prRoamingInfo;


    DEBUGFUNC("roamingDiscoveryScanDoneHandler");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    DBGLOG(ROAMING, EVENT, ("roamingDiscoveryScanDoneHandler: %s\n",
        ((rStatus == WLAN_STATUS_SUCCESS) ? "WLAN_STATUS_SUCCESS" : "WLAN_STATUS_FAILURE" )));

    prRoamingInfo->fgIsScanCompleted = TRUE;

    return;
} /* end of roamingDiscoveryScanDoneHandler() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
roamingReTriggerEventDecision (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;

    DEBUGFUNC("roamingReTriggerEventDecision");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    DBGLOG(ROAMING, EVENT, ("roamingReTriggerEventDecision\n"));

    ARB_SET_TIMER(prAdapter,
                  prRoamingInfo->rRoamingDecisionTimer,
                  SEC_TO_MSEC(ROAMING_DECISION_TIMEOUT_SEC));

    return;
} /* end of roamingReTriggerEventDecision() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
roamingReTriggerEventDiscovery (
    IN P_ADAPTER_T prAdapter
    )
{
    P_ROAMING_INFO_T prRoamingInfo;

    DEBUGFUNC("roamingReTriggerEventDiscovery");


    ASSERT(prAdapter);
    prRoamingInfo = &prAdapter->rRoamingInfo;

    DBGLOG(ROAMING, EVENT, ("roamingReTriggerEventDiscovery\n"));

    ARB_SET_TIMER(prAdapter,
                  prRoamingInfo->rRoamingDiscoveryTimer,
                  prRoamingInfo->u2DiscoveryTimeoutMillisecond);

    return;
} /* end of roamingReTriggerEventDiscovery() */


