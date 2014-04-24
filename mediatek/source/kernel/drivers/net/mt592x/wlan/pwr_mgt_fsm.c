






#include "precomp.h"

extern ENUM_POWER_SAVE_PROFILE_T aPowerModeToPwrSaveProfMapping[];




#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugPmState[PM_STATE_NUM] = {
    DISP_STRING("PM_STATE_ACTIVE_MODE"),
    DISP_STRING("PM_STATE_WAIT_POWER_SAVE_READY"),
    DISP_STRING("PM_STATE_WAIT_ACTIVE_READY"),
    DISP_STRING("PM_STATE_POWER_SAVE_AWAKE"),
    DISP_STRING("PM_STATE_POWER_SAVE_DOZE")
};
/*lint -restore */
#endif /* DBG */

#if CFG_LP_IOT
UINT_8 g_ucSleepPeriod;
UINT_32 u4AccumBcnTimeoutCnt = 0;
UINT_32 u4AccumMibBcnLostCnt = 0;
OS_SYSTIME rTestStartTime = 0;

extern UINT_8 g_ucMaxRcpi ;
extern UINT_8 g_ucMinRcpi ;
extern UINT_8 g_ucOrgRcpi;
#endif



VOID
pmSetupPwrMgtInfo (
    IN  UINT_16         u2AID,
    IN  UINT_16         u2BcnIntv,
    IN  UINT_8          ucDtimPeriod,
    IN  UINT_16         u2AtimWindow,
    IN  BOOLEAN         fgIsUapsdConn,
    OUT P_PM_CONN_SETUP_INFO_T  prPmConnSetupInfo
    );

VOID
pmFsmRunEventNullTxDone (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    );

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_WAIT_POWER_SAVE_READY_to_POWER_SAVE_AWAKE (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prConnSettings = &prAdapter->rConnSettings;

    prPmInfo->fgIsUnderPowerSave = TRUE;

    if (prPmInfo->pfnPsIndication) {
        if (prPmInfo->pfnPsIndication(prAdapter, NULL, FALSE) == WLAN_STATUS_SUCCESS) {
            // do nothing
        }
        prPmInfo->pfnPsIndication = NULL;
    }

    // Enable continuous polling function
    if (prConnSettings->u4ContPollIntv) {
        pmEnableContinuousPollingFunc(prAdapter);
    }

    // Setup timer for VOIP application
    if (prConnSettings->fgIsVoipConn) {
        pmEnableVoipPollingFunc(prAdapter);
    }

    if (prPmInfo->eCurPsProf == ENUM_PSP_FAST_SWITCH) {
        ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmSwitchPsModeTimer);
    }

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_WAIT_POWER_SAVE_READY_to_ACTIVE_MODE (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    /* Force to switch back to active mode, with also the PS bit setting */
    nicpmSwitchPwrMgtMode(prAdapter, FALSE);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_WAIT_ACTIVE_READY_to_ACTIVE_MODE (
    IN P_ADAPTER_T prAdapter
    )
{
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_ACTIVE_MODE_to_WAIT_POWER_SAVE_READY (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    prAdapter->rPmInfo.fgPwrMode_PS = TRUE;

    /* Change the PS mode by transmitting NULL packet with PS bit changed */
    pmSendNullFrame(prAdapter, 0, prAdapter->rPmInfo.fgPwrMode_PS, TRUE, pmFsmRunEventNullTxDone);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_ACTIVE_MODE_to_POWER_SAVE_AWAKE (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    prPmInfo->fgIsUnderPowerSave = TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_POWER_SAVE_AWAKE_to_WAIT_ACTIVE_READY (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    prAdapter->rPmInfo.fgPwrMode_PS = FALSE;

    /* Change the PS mode by transmitting NULL packet with PS bit changed */
    pmSendNullFrame(prAdapter,
                    0,
                    prAdapter->rPmInfo.fgPwrMode_PS,
                    TRUE,
                    (PFN_TX_DONE_HANDLER)0);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_WAIT_POWER_SAVE_READY_to_WAIT_ACTIVE_READY (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    prAdapter->rPmInfo.fgPwrMode_PS = FALSE;

    /* Change the PS mode by transmitting NULL packet with PS bit changed */
    pmSendNullFrame(prAdapter,
                    0,
                    prAdapter->rPmInfo.fgPwrMode_PS,
                    TRUE,
                    (PFN_TX_DONE_HANDLER)0);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_POWER_SAVE_AWAKE_to_POWER_SAVE_DOZE (
    IN P_ADAPTER_T prAdapter
    )
{
    /* do nothing here */
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
pmFsmTransAction_POWER_SAVE_DOZE_to_POWER_SAVE_AWAKE (
    IN P_ADAPTER_T prAdapter
    )
{
    /* do nothing here */
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
pmFsmInit (
    P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
    P_CONNECTION_SETTINGS_T prConnectionSetting;
    BOOLEAN fgStatus;

    DEBUGFUNC("pmFsmInit");

    DBGLOG(LP, INFO, ("\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;
    prConnectionSetting = &prAdapter->rConnSettings;


    prPmInfo->eCurrentState = PM_STATE_ACTIVE_MODE;

    // initialize SW database
    prPmProfSetupInfo->ucMultiDtimWake = DEFAULT_MULTIPLE_DTIM_COUNT;
    prPmProfSetupInfo->ucMultiTbttWake = DEFAULT_MULTIPLE_TBTT_COUNT;
    prPmProfSetupInfo->fgUseAgeQoSNull = DEFAULT_USE_AGING_QOS_NULL;
    prPmProfSetupInfo->ucAgeNullPeriod = DEFAULT_AGING_NULL_INTERVAL;
    prPmProfSetupInfo->ucQoSNullTid    = DEFAULT_AGING_NULL_TID;
    prPmProfSetupInfo->bmfgApsdEnAc    = prConnectionSetting->bmfgApsdEnAc;
    prPmProfSetupInfo->ucUapsdSp       = DEFAULT_UAPSD_SERVICE_PERIOD_NUM;
    prPmProfSetupInfo->ucTrgThr        = DEFAULT_TRIGGER_THRESHOLD;
    prPmProfSetupInfo->u2HwTxLifeTime  = DEFAULT_HW_TX_PACKET_LIFETIME;
#if CFG_IBSS_POWER_SAVE
    prPmProfSetupInfo->fgAdhocPsCapability = TRUE;
#else
    prPmProfSetupInfo->fgAdhocPsCapability = FALSE;
#endif
    prPmProfSetupInfo->u2TxThrSwToAct  = DEFAULT_PSP_SWITCH_TO_ACT_TX_THRESHOLD;
    prPmProfSetupInfo->u2TxThrSwToPs   = DEFAULT_PSP_SWITCH_TO_PS_TX_THRESHOLD;
    prPmProfSetupInfo->u2RxThrSwToAct  = DEFAULT_PSP_SWITCH_TO_ACT_RX_THRESHOLD;
    prPmProfSetupInfo->u2RxThrSwToPs   = DEFAULT_PSP_SWITCH_TO_PS_RX_THRESHOLD;
    prPmProfSetupInfo->u2PsModeSwIntv  = DEFAULT_PS_MODE_SWITCH_INTERVAL;
    prPmProfSetupInfo->u2ActModeSwIntv = DEFAULT_FAST_ACTIVE_MEASURE_INTERVAL;

    /* NOTE:
        Enable following flags when L3 is needed
    */
#if CFG_L3_PATTERN_MATCH_WAKEUP
    prPmProfSetupInfo->fgBcPtrnSrchEn   = prConnectionSetting->fgArpFilterEn;
    prPmProfSetupInfo->fgMcPtrnSrchEn   = FALSE;
    prPmProfSetupInfo->fgUcPtrnSrchEn   = FALSE;
#else
    prPmProfSetupInfo->fgBcPtrnSrchEn   = FALSE;
    prPmProfSetupInfo->fgMcPtrnSrchEn   = FALSE;
    prPmProfSetupInfo->fgUcPtrnSrchEn   = FALSE;
#endif

    prPmProfSetupInfo->fgBcPtrnMatchRcv = FALSE;
    prPmProfSetupInfo->fgMcPtrnMatchRcv = FALSE;
    prPmProfSetupInfo->fgUcPtrnMatchRcv = FALSE;

    /* CR1853: workaround for large DTIM interval AP */
    prPmProfSetupInfo->u2DtimIntvThr = DEFAULT_DTIM_INTERVAL_THRESHOLD;
    prPmProfSetupInfo->ucWorkaroundTbttCount = DEFAULT_WORKAROUND_TBTT_COUNT;

    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;

    /* CR1853: handle abnormal cases */
    prPmInfo->fgIsWatchDogTriggered = FALSE;

	/* Renbang : for Bcn sent before TBTT  */
    prPmInfo->fgIsBcnTimeoutStep4 = FALSE;

#if CFG_LP_IOT
    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rPmReadBeaconTimeoutTimer,
                   pmFsmRunEventReadBeaconTimeoutCount,
                   TRUE);
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] UAPSD AC: 0x%x\n", prPmProfSetupInfo->bmfgApsdEnAc));
#endif

    // Initialize for PS-profile
    pmFsmRunEventSetPowerSaveProfile(prAdapter,
                                     aPowerModeToPwrSaveProfMapping[prConnectionSetting->rPwrMode]);

#if 1 //VoIP function enable/ disable
    {
    PARAM_VOIP_CONFIG   rVoipCfg;
    UINT_32 u4LenIn, u4LenOut;
    rVoipCfg.u4VoipTrafficInterval =
        prConnectionSetting->u4VoipTrafficInterval;
    u4LenIn = sizeof(PARAM_VOIP_CONFIG);
    wlanoidSetVoipConnectionStatus(prAdapter, &rVoipCfg, u4LenIn, &u4LenOut);
    }
#endif

    // related register initialize
    fgStatus = nicpmRegInit(prAdapter);

    // register the timers used for power management (use SW root timer)
    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rPmOnConnectDeferredTimer,
                   arbFsmRunEventOnConnectDeferredTimer,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rPmSwitchPsModeTimer,
                   arbFsmRunEventTimerPowerSaveSwitch,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rWaitBeaconWatchdogTimer,
                   arbFsmRunEventAbortOnWaitBeaconTimeOut,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rBeaconTimeoutHandlingTimer,
                   arbFsmRunEventBeaconTimeoutStepDownHandling,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rContiPollingCheckTimer,
                   arbFsmRunEventContiPollingCheckHandling,
                   TRUE);

    ARB_INIT_TIMER(prAdapter,
                   prPmInfo->rContiHwTxFailCheckTimer,
                   arbFsmRunEventContiHwTxFailCheckHandling,
                   TRUE);

    return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmFsmUnInit (
    P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;

    DEBUGFUNC("pmFsmUnInit");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    DBGLOG(LP, INFO, ("\n"));

    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmOnConnectDeferredTimer);
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmSwitchPsModeTimer);

    nicpmPowerOff(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmFsmStep (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_PM_STATE_T      eNextState
    )
{
    P_PM_INFO_T prPmInfo;
    ENUM_PM_STATE_T ePrevState;

    DEBUGFUNC("pmFsmStep");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    DBGLOG(LP, STATE, ("\nTRANSITION: [%s] -> [%s]\n\n",
                       apucDebugPmState[prPmInfo->eCurrentState],
                       apucDebugPmState[eNextState]));

    ePrevState = prPmInfo->eCurrentState;
    prPmInfo->eCurrentState = eNextState;

    /* Do tasks of the State that we just entered */
    switch (prPmInfo->eCurrentState) {
    /* Optimization for the state initialization procedure.
       Due to there's no job to do in PM_STATE_POWER_SAVE_AWAKE
       and PM_STATE_POWER_SAVE_DOZE state, and there's
       massive "own"/ "own back" processing during runtime.
       We extract these two state out of state initialization
       path for gaining the runtime performance
    */
    case PM_STATE_POWER_SAVE_AWAKE:
        /* things to be done are proceeded in the transition path */

        return;

    case PM_STATE_POWER_SAVE_DOZE:
        /* things to be done are proceeded in the transition path */

        return;

    case PM_STATE_ACTIVE_MODE:
        {
            if (ePrevState == PM_STATE_WAIT_ACTIVE_READY) {
                if (prPmInfo->pfnPsIndication) {
                    if (prPmInfo->pfnPsIndication(prAdapter, NULL, FALSE) == WLAN_STATUS_SUCCESS) {
                        // do nothing
                    }
                    prPmInfo->pfnPsIndication = NULL;
                }
            }
            prPmInfo->fgIsUnderPowerSave = FALSE;

            /* disable Low Power related functions */
            pmDisableContinuousPollingFunc(prAdapter);

            pmDisableVoipPollingFunc(prAdapter);

            if (prPmInfo->eCurPsProf == ENUM_PSP_FAST_SWITCH) {
                P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
                prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

                prPmInfo->fgFastSwitchInProgress = FALSE;

                prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum = 0;
                prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum = 0;

                GET_CURRENT_SYSTIME(&prPmInfo->rPmFastSwitchParam.rLastSwitchTime);

                /* For restoring the timer after switching back from scan */
                ARB_SET_TIMER(prAdapter,
                              prPmInfo->rPmSwitchPsModeTimer,
                              prPmProfSetupInfo->u2ActModeSwIntv);
            }

            arbFsmRunEventScanCheckForHangTimeOut(prAdapter);
        }

        break;

    case PM_STATE_WAIT_POWER_SAVE_READY:
        /* things to be done are proceeded in the transition path */

        break;

    case PM_STATE_WAIT_ACTIVE_READY:
        /* things to be done are proceeded in the transition path */

        break;

    default:
        ASSERT(0); /* Make sure we have handle all STATEs */
    }

    return;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventSetPowerSaveProfile (
    IN P_ADAPTER_T  prAdapter,
    IN ENUM_POWER_SAVE_PROFILE_T ePsProf
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_PM_INFO_T prPmInfo;
    ENUM_POWER_SAVE_PROFILE_T eOriginalPsProf;
    OS_SYSTIME rCurrentTime;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventSetPowerSaveProfile");

    ASSERT(prAdapter);

    prConnSettings = &prAdapter->rConnSettings;
    prPmInfo = &prAdapter->rPmInfo;
    eOriginalPsProf = prPmInfo->eCurPsProf;

    DBGLOG(LP, INFO, ("profile: %d\n", ePsProf));

    GET_CURRENT_SYSTIME(&rCurrentTime);

    /* return directly if the configured profile is identical to current setting */
    if (eOriginalPsProf == ePsProf) {
        return status;
    }
    else {
        /* restore original setting */
 //       ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmOnConnectDeferredTimer);

        if (eOriginalPsProf == ENUM_PSP_FAST_SWITCH) {
            ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmSwitchPsModeTimer);
        }

        pmDisableVoipPollingFunc(prAdapter);
    }

    prPmInfo->eCurPsProf = ePsProf;

    /* there's two methodology for process this request
       1. enter PS immediately (choose this one now)
       2. setup a timer for decision later
    */
    if (prAdapter->eConnectionState != MEDIA_STATE_CONNECTED) {
        return status;
    }

    /* Not allow power save profile switching once "continuous polling profile" is enabled */
    if (prPmInfo->fgIsContinousPollingEnabled) {
        return status;
    }

    if (ePsProf == ENUM_PSP_CONTINUOUS_ACTIVE) {
        if (prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
            pmFsmRunEventEnterActiveModeReq(prAdapter, NULL);
        }
        else if (prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
            /* modify ATIM window */
//            prPmProfSetupInfo->u2AtimWindow = 0;

            /* Re-Connect here? */

        }
    }
    else {
        if (prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
            if (!PM_IS_UNDER_POWER_SAVE_MODE(prAdapter)) {
                /* If it is under ACTIVE mode, set it to enter PS mode ASAP */


                /* if connected for over the pre-setting interval prevent enter PS */
                if (pmIsAbleToEnterPowerSaveMode(prAdapter)) {
                    pmFsmRunEventEnterPowerSaveModeReq(prAdapter, NULL);
                }
                else {
                    /* if not, setup timer and invoke enter PS procedure */
                    ARB_SET_TIMER(prAdapter,
                                  prPmInfo->rPmOnConnectDeferredTimer,
                                  (CFG_KEEP_ACTIVE_DWELL_TIME_ON_CONNECT_MSEC -
                                   SYSTIME_TO_MSEC(rCurrentTime - prAdapter->rLastConnectedTime)));
                }
            }
            else {
                /* If it is already under PS mode */

                if (ePsProf == ENUM_PSP_FAST_SWITCH) {
                    /* trigger from PS to ACTIVE is by the RX done handler
                       pmIndicateUcDataFrameReceived() */
                }

                // Setup timer for VOIP application
                if (prConnSettings->fgIsVoipConn) {
                    pmEnableVoipPollingFunc(prAdapter);
                }
            }
        }
        else if (prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
//            prPmProfSetupInfo->u2AtimWindow = 0;
        }
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventSetPowerSaveProfileWatchDog (
    IN P_ADAPTER_T  prAdapter,
    IN ENUM_POWER_SAVE_PROFILE_T ePsProf
    )
{
    P_PM_INFO_T prPmInfo;
    ASSERT(prAdapter);
    prPmInfo = &prAdapter->rPmInfo;
    prPmInfo->fgIsWatchDogTriggered = TRUE;
    return pmFsmRunEventSetPowerSaveProfile(prAdapter, ePsProf);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventEnterPowerSaveModeReq (
    IN P_ADAPTER_T  prAdapter,
    IN FUNC_CALLBACK_PS pfnPsIndication
    )
{
    P_PM_INFO_T prPmInfo;
    ENUM_PM_STATE_T eNextState;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventEnterPowerSaveModeReq");

    ASSERT(prAdapter);
    ASSERT(prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE);

    prPmInfo = &prAdapter->rPmInfo;
    eNextState = prPmInfo->eCurrentState;

    if (!pmIsAbleToEnterPowerSaveMode(prAdapter)) {
        /* Other modules request to enter PS mode should check related condition before
           invoking this method. This is only a safety check here. */
        DBGLOG(LP, WARN, ("Not able to enter PS mode here. (current state: %s)\n",
                            apucDebugPmState[prPmInfo->eCurrentState]));
//        ASSERT(0);
        return status;
    }


    DBGLOG(LP, INFO, ("current state: %s\n", apucDebugPmState[prPmInfo->eCurrentState]));

    switch (prPmInfo->eCurrentState) {
    case PM_STATE_ACTIVE_MODE:
        /* record the callback function */
        prPmInfo->pfnPsIndication = pfnPsIndication;
        PM_STATE_TRANSITION(prAdapter, ACTIVE_MODE, WAIT_POWER_SAVE_READY);

        break;

    case PM_STATE_WAIT_ACTIVE_READY:
        /* how do we handle this request in this state?? */
        //TODO:
        break;

    default: // NOTE(Kevin): To avoid compile warning.
        break;
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventOnConnectDeferredTask (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    P_BSS_INFO_T prBssInfo;
    UINT_16 u2Value;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventOnConnectDeferredTask");

    DBGLOG(LP, INFO, ("\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;

    /* CR1486, at wpa/wpa2, delay enter ps mode until key installed */
    if (privacyRsnKeyHandshakeEnabled(prAdapter) && !privacyTransmitKeyExist(prAdapter)) {
        DBGLOG(RSN, TRACE, ("Delay enter low power while key not installed at wpa mode!!\n"));

        ARB_SET_TIMER(prAdapter,
                      prPmInfo->rPmOnConnectDeferredTimer,
                      CFG_KEEP_ACTIVE_DWELL_TIME_ON_CONNECT_MSEC);

        return status;
    }

    /* Allow scan to be done after the dwell time connected to AP */
    if ((prBssInfo->eBSSType == BSS_TYPE_INFRASTRUCTURE) &&
        (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED)) {
        prAdapter->fgIsBlockToScan = FALSE;
    }
    else {
        ASSERT(0);
        return WLAN_STATUS_NOT_ACCEPTED;
    }

    /* Adjust the guard time ahead from TBTT for listening beacon transmitted
       by few "problematic" AP */
    u2Value = nicpmGetBeaconEarlyValue(prAdapter);
    if (u2Value) {
        DBGLOG(LP, INFO, ("!! Beacon earlier time: %d us\n", u2Value));
        nicpmAdjustWakeupGuardTime(prAdapter, u2Value);
    }

#if CFG_LP_IOT
    {
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] Beacon earlier time: %d us\n", u2Value));
    }
    {
    UINT_32 u4RegValue;
    NIC_UNSET_INT_EVENT(prAdapter, INT_EVENT_RCPI);

    HAL_MCR_RD(prAdapter, MCR_RR, &u4RegValue);
    // HAL_MCR_WR(prAdapter, MCR_RR, u4RegValue & ~RR_ENABLE_MA);
    g_ucMaxRcpi = g_ucMinRcpi = g_ucOrgRcpi = (UINT_8)(u4RegValue & BITS(0, 7));
    u4RegValue &= ~(RR_RCPI_HIGH_THRESHOLD_MASK | RR_RCPI_LOW_THRESHOLD_MASK);
    u4RegValue |= ((g_ucOrgRcpi + 3) << 16) | ((g_ucOrgRcpi - 3)  << 8) | RR_RCPI_PARM_1_OF_8;
    HAL_MCR_WR(prAdapter, MCR_RR, RR_ENABLE_MA | u4RegValue);
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] Init RCPI settings: 0x%x (%d dBm) (MCR_RR=0x%08X)\n",
                            u4RegValue & BITS(0, 7),
                            RCPI_TO_DBM(u4RegValue & BITS(0, 7)),
                            u4RegValue));
    }
#endif

    if (!PM_IS_USING_PS_PROFILE_CONTINUOUS_ACTIVE(prAdapter)) {
#if SUPPORT_WPS
        /* CR1486, CR1640 */
        /* For WPS, privacy disable also disable the driver entre ps ...*/
        /* Notice, at WPS mode, no key set, so the max 2 minutes walk time control the connection */
        if (
#if 0//SUPPORT_WAPI
            (!prAdapter->fgUseWapi) &&
#endif
            (prAdapter->rSecInfo.fgPrivacyCheckDisable) &&
            (prAdapter->rConnSettings.eAuthMode < AUTH_MODE_WPA) &&
            (prAdapter->rConnSettings.eOPMode == NET_TYPE_INFRA)) {
            DBGLOG(RSN, TRACE, ("Disable enter low power while ui set at wps mode!!\n"));
        }
        else
#endif
        pmFsmRunEventEnterPowerSaveModeReq(prAdapter, NULL);
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventEnterActiveModeReq (
    IN P_ADAPTER_T  prAdapter,
    IN FUNC_CALLBACK_PS pvPSCallBack
    )
{
    P_PM_INFO_T prPmInfo;
    ENUM_PM_STATE_T eNextState;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventEnterActiveModeReq");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    eNextState = prPmInfo->eCurrentState;

    DBGLOG(LP, INFO, ("current state: %s\n", apucDebugPmState[prPmInfo->eCurrentState]));

    switch (prPmInfo->eCurrentState) {
    case PM_STATE_POWER_SAVE_DOZE:
        /* return to awake state first */
        pmFsmRunEventEnterAwakeState(prAdapter);
        /* Notice: no "break" here */

    case PM_STATE_POWER_SAVE_AWAKE:
        PM_STATE_TRANSITION(prAdapter, POWER_SAVE_AWAKE, WAIT_ACTIVE_READY);
        break;
    case PM_STATE_WAIT_POWER_SAVE_READY:
        PM_STATE_TRANSITION(prAdapter, WAIT_POWER_SAVE_READY, WAIT_ACTIVE_READY);
        break;

    default: // NOTE(Kevin): To avoid compile warning.
        break;
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventPowerModeIndication (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgPowerSaveMode
    )
{
    P_PM_INFO_T prPmInfo;
    ENUM_PM_STATE_T eNextState;
    P_BSS_INFO_T prBssInfo;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventPowerModeIndication");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    eNextState = prPmInfo->eCurrentState;
    prBssInfo = &prAdapter->rBssInfo;

    /* check if it is connected under INFRASTRUCTURE power-saving profile */
    if ((prBssInfo->eBSSType == BSS_TYPE_INFRASTRUCTURE) &&
        (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED)) {

        /* change the PS state by the status reported by the TX status */
        if (fgPowerSaveMode) {
            if (prPmInfo->eCurrentState == PM_STATE_WAIT_POWER_SAVE_READY) {
                PM_STATE_TRANSITION(prAdapter, WAIT_POWER_SAVE_READY, POWER_SAVE_AWAKE);
            }
        }
        else {
            if (prPmInfo->eCurrentState == PM_STATE_WAIT_ACTIVE_READY) {
                PM_STATE_TRANSITION(prAdapter, WAIT_ACTIVE_READY, ACTIVE_MODE);
            }

            /* Special considerations of the exception handler on the transmission fail
               of NULL for entering PS.
             */

            if (prPmInfo->rNullState == NULL_STATE_FAIL) {
                prPmInfo->rNullState = NULL_STATE_NONE;

                if (prPmInfo->eCurrentState == PM_STATE_WAIT_POWER_SAVE_READY) {
                    PM_STATE_TRANSITION(prAdapter, WAIT_POWER_SAVE_READY, ACTIVE_MODE);

                    /* Force to do scan procedure even if it is not entering PS mode.
                       Note: packet loss may be introduced in this case!!
                     */
                    if (prPmInfo->pfnPsIndication == arbFsmRunEventScanRequest) {
                        arbFsmRunEventScanRequest(prAdapter, NULL, TRUE);
                        prPmInfo->pfnPsIndication = NULL;
                    }
                }

            }
        }
    }
    else {
        /* do nothing in this case */
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventOnConnect (
    IN  P_ADAPTER_T     prAdapter,
    IN  UINT_16         u2AID,
    IN  UINT_16         u2BcnIntv,
    IN  UINT_8          ucDtimPeriod,
    IN  UINT_16         u2AtimWindow
    )
{
//    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
    P_PM_CONN_SETUP_INFO_T prPmConnSetupInfo;
    P_PM_INFO_T prPmInfo;
    P_BSS_INFO_T prBssInfo;
    BOOLEAN fgIsUapsdConn;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventOnConnect");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
//    prPmProfSetupInfo = &prAdapter->rPmInfo.rPmProfSetupInfo;
    prPmConnSetupInfo = &prAdapter->rPmConnSetupInfo;

    DBGLOG(LP, INFO, ("\n"));

    /* CR1837 workaround: cancel beacon timeout handling timer before
    * reseting tuning step to 0.
    */
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rBeaconTimeoutHandlingTimer);

    /* Initialize beacon timeout handling step */
    prPmInfo->eBeaconTimeoutTuningStep = BCN_TO_STEP_0;

    prAdapter->fgWaitOneBeaconForTsfRecovery = FALSE;

    /* Renbang : recover power mode from exception case */
    if (FALSE != prPmInfo->fgIsWatchDogTriggered || FALSE != prPmInfo->fgIsBcnTimeoutStep4) {
        prPmInfo->fgIsWatchDogTriggered = FALSE;
		prPmInfo->fgIsBcnTimeoutStep4 = FALSE;
        pmFsmRunEventSetPowerSaveProfile(prAdapter,
            aPowerModeToPwrSaveProfMapping[prAdapter->rConnSettings.rPwrMode]);
    }

    /* check if it is work under UAPSD connection */
    if ((prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_UAPSD)) {
        fgIsUapsdConn = TRUE;
    }
    else {
        fgIsUapsdConn = FALSE;
    }

    pmSetupPwrMgtInfo(u2AID,
                      u2BcnIntv,
                      ucDtimPeriod,
                      u2AtimWindow,
                      fgIsUapsdConn,
                      prPmConnSetupInfo);

#if CFG_LP_IOT
    g_ucSleepPeriod = ucDtimPeriod * u2BcnIntv;

    DBGLOG(LP_IOT, INFO, ("[LP-IOT] DtimPeriod: %d\n", ucDtimPeriod));
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] BcnIntv: %d\n", u2BcnIntv));
    u4AccumBcnTimeoutCnt = 0;
    u4AccumMibBcnLostCnt = 0;

    GET_CURRENT_SYSTIME(&rTestStartTime);
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] Test start time: %d ms\n", rTestStartTime));

    ARB_SET_TIMER(prAdapter,
                  prPmInfo->rPmReadBeaconTimeoutTimer,
                  10000);

    pmSetIotBeaconTimeoutCheck(prAdapter);
#endif

    if (prBssInfo->eBSSType == BSS_TYPE_INFRASTRUCTURE) {
        ASSERT(prPmInfo->eCurrentState == PM_STATE_ACTIVE_MODE);

        nicpmSetupPmInfoInBss(
            prAdapter,
            prPmConnSetupInfo);

        prPmInfo->fgDtimTrigEn = TRUE;

        ARB_SET_TIMER(prAdapter,
                      prPmInfo->rPmOnConnectDeferredTimer,
                      CFG_KEEP_ACTIVE_DWELL_TIME_ON_CONNECT_MSEC);

        if (prPmInfo->fgIsBeaconTimeoutDetectionDisabled) {
            pmDisableBeaconTimeoutDetectionFunc(prAdapter, TRUE);
        }
    }
    else if (prBssInfo->eBSSType == BSS_TYPE_IBSS) {

#if 0// juji test only with non-IBSS PS sta
        prPmConnSetupInfo->u2AtimWindow = u2AtimWindow = 20;
#endif
        nicpmSetupPmInfoInAdhoc(
            prAdapter,
            prPmConnSetupInfo,
            FALSE);

        prAdapter->fgIsIBSSActive = TRUE;

#if CFG_IBSS_POWER_SAVE
        /* If ATIM window not equals to 0 in Ad hoc, enter PS mode directly */
        if (u2AtimWindow) {
            ENUM_PM_STATE_T eNextState = prPmInfo->eCurrentState;

            PM_STATE_TRANSITION(prAdapter, ACTIVE_MODE, POWER_SAVE_AWAKE);
        }
#endif /* CFG_IBSS_POWER_SAVE */
    }
    else {
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventOnCreateIbss (
    IN P_ADAPTER_T      prAdapter,
    IN  UINT_16         u2BcnIntv
    )
{
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;
    PM_CONN_SETUP_INFO_T rPmConnSetupInfo;
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    ASSERT(prPmInfo->eCurrentState == PM_STATE_ACTIVE_MODE);

    if (prAdapter->fgIsIBSSActive) {
        return status;
    }
    else {
        prAdapter->fgIsIBSSActive = TRUE;
    }

    if (prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
        pmSetupPwrMgtInfo(0, //u2AID,
                          u2BcnIntv,
                          0, //ucDtimPeriod,
                          0, //u2AtimWindow,
                          FALSE,
                          &rPmConnSetupInfo);

        nicpmSetupPmInfoInAdhoc(
            prAdapter,
            &rPmConnSetupInfo,
            TRUE);
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventAbort (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmFsmRunEventAbort");

    DBGLOG(LP, INFO, ("\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;
    prPmInfo->fgDtimTrigEn = FALSE;

    /* Change PS bit in the FC */
    nicpmSwitchPwrMgtMode(prAdapter, FALSE);

    /* disable continous polling mode */
    pmDisableContinuousPollingFunc(prAdapter);

    /* Disable VoIP polling timer, and SW buffer TX mechanism */
    pmDisableVoipPollingFunc(prAdapter);

    /* Disable Low Power related functions */
    nicpmAbortPmFunc(prAdapter);

    /* Disable Service period */
    nicpmSetupServicePeriodMechanism(prAdapter, 0, SP_ALL);

    /* Disable Pattern search */
    nicpmConfigPatternSearchFunc(prAdapter,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE);

    /* Cancel pre-allocated timer */
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmOnConnectDeferredTimer);
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmSwitchPsModeTimer);
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rWaitBeaconWatchdogTimer);
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rBeaconTimeoutHandlingTimer);

    /* Change to active mode directly */
    prPmInfo->eCurrentState = PM_STATE_ACTIVE_MODE;
    prPmInfo->fgIsUnderPowerSave = FALSE;

#if CFG_LP_IOT
    ARB_CANCEL_TIMER(prAdapter, prPmInfo->rPmReadBeaconTimeoutTimer);
    u4AccumBcnTimeoutCnt = 0;
    u4AccumMibBcnLostCnt = 0;
#endif

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmFsmRunEventEnterAwakeState (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    ENUM_PM_STATE_T eNextState;

    DEBUGFUNC("pmFsmRunEventEnterAwakeState");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    eNextState = prPmInfo->eCurrentState;

    if (prPmInfo->eCurrentState != PM_STATE_POWER_SAVE_DOZE) {
        // NOTE: not applied for IBSS PS!!
        DBGLOG(LP, LOUD, ("Not stay in PM_STATE_POWER_SAVE_DOZE"));
        return;
    }

    DBGLOG(LP, INFO, ("current state: %s\n", apucDebugPmState[prPmInfo->eCurrentState]));

    nicpmLeaveLowPower(prAdapter);

    /* Restore LED status */
//    pauUpdateLED(adapter_p);

    PM_STATE_TRANSITION(prAdapter, POWER_SAVE_DOZE, POWER_SAVE_AWAKE);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmFsmRunEventEnterDozeState (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgEnableGlobalInt
    )
{
    P_PM_INFO_T prPmInfo;
    ENUM_PM_STATE_T eNextState;

    DEBUGFUNC("pmFsmRunEventEnterDozeState");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    eNextState = prPmInfo->eCurrentState;


    if (prPmInfo->eCurrentState != PM_STATE_POWER_SAVE_AWAKE) {
        DBGLOG(LP, LOUD, ("Return due to not stay in PS AWAKE"));
        return;
    }

    DBGLOG(LP, INFO, ("current state: %s\n", apucDebugPmState[prPmInfo->eCurrentState]));

    do {

#if CFG_IBSS_POWER_SAVE
        if (PM_IS_UNDER_IBSS_POWER_SAVE_MODE(prAdapter)) {
            if (prPmInfo->fgIsIbssPsTxEnabled) {
                break;
            }
        }
        else {
            /* Note:
               Place this check here to workaround thex workaround code under IBSS power save.
               This is due to when CFG_TX_RET_TX_CTRL_EARLY flag enabled, SW will prevent to
               flush data queue for performance issue, which is caused by the packet pointer
               will be returned to OS directly right after programming it to HW,
               and these packets cannot be reclaimed in this way.
               Refer nicTxReclaimTxPackets().

               And rTxCtrl.ucTxNonEmptyACQ may not be updated and will cause SW
               preventing set HW LP own.
            */

            if (NIC_TX_TEST_AND_SET_IS_NEED_CHECK_TXQ_EMPTY(prAdapter)) {
                /* check if actived TX queue empty */
                if (NIC_TX_IS_ACTIVE_ACQ_NOT_EMPTY(prAdapter)) {
                    break;
                }
            }
        }
#else
        if (NIC_TX_TEST_AND_SET_IS_NEED_CHECK_TXQ_EMPTY(prAdapter)) {
            /* check if actived TX queue empty */
            if (NIC_TX_IS_ACTIVE_ACQ_NOT_EMPTY(prAdapter)) {
                break;
            }
        }
#endif /* CFG_IBSS_POWER_SAVE */

        /* It should wait at least one beacon for TSF recovery before enter LP */
        if (prAdapter->fgWaitOneBeaconForTsfRecovery) {
            break;
        }

        nicpmEnterLowPower(prAdapter, fgEnableGlobalInt);

        /* Turn off LED */
    //    pauConfigLED(adapter_p, NIC_LED_OUTPUT_GPIO, NIC_LED_NO_TRIGGER_OFF);

        PM_STATE_TRANSITION(prAdapter, POWER_SAVE_AWAKE, POWER_SAVE_DOZE);
    } while (FALSE);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmIndicateUcDataFrameReceived (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN fgMoreData
    )
{
    P_PM_INFO_T                 prPmInfo;
    OS_SYSTIME                  rCurSysTime;
    UINT_32                     u4TimeSlot;
    P_BSS_INFO_T                prBssInfo;
    P_PM_PROFILE_SETUP_INFO_T   prPmProfSetupInfo;
    UINT_32                     u4RxUcPktNumThr;

    DEBUGFUNC("pmIndicateUcDataFrameReceived");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    GET_CURRENT_SYSTIME(&rCurSysTime);

    ARB_FSM_SAVE_LAST_RX_UC_DATA_FRAME_SYSTIME(prAdapter, rCurSysTime);

    /* following are decisions only for connection under infrastructure mode */
    if ((prBssInfo->eBSSType != BSS_TYPE_INFRASTRUCTURE) ||
        (prAdapter->eConnectionState != MEDIA_STATE_CONNECTED)) {
        return;
    }

    if (prPmInfo->fgIsContinousPollingEnabled &&
        (prPmInfo->fgIsContinousPollingSuspended ||
        prPmInfo->fgContiPollIsStepUp == FALSE)) {
#if 0
        pmResumeContinuousPollingFunc(prAdapter);
#else
        pmResumeContinuousPollingFunc(prAdapter);
        pmContinuousPollingFreqStepUp(prAdapter, TRUE);
#endif
    }


    /* following are decisions only for PSP-fast switch mode */
    if (prPmInfo->eCurPsProf != ENUM_PSP_FAST_SWITCH) {
        return;
    }

    /* The slot is define in 1000ms under Active mode and 100ms under PS mode by default.
       Under PS mode, it counts the received unicast packet with more data bit been set.
       Under Active mode, it counts the received unicast packet.
    */
    if (PM_IS_UNDER_POWER_SAVE_MODE(prAdapter)) {
        u4TimeSlot = SYSTIME_TO_MSEC(rCurSysTime) / prPmProfSetupInfo->u2PsModeSwIntv;

        u4RxUcPktNumThr = prPmProfSetupInfo->u2RxThrSwToAct *
                            (prPmProfSetupInfo->u2PsModeSwIntv / 100);


        if (u4TimeSlot == prPmInfo->rPmFastSwitchParam.u4TimeSlot) {

            if (fgMoreData) {
                prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum++;
            }

            if (!prPmInfo->fgFastSwitchInProgress) {

                if (prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum >
                    u4RxUcPktNumThr) {

                    DBGLOG(LP, INFO, ("u4AccumRxUcPktNum: %ld, u4RxUcPktNumThr: %ld\n",
                                        prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum,
                                        u4RxUcPktNumThr));

                    DBGLOG(LP, INFO, ("Power save profile - FAST mode\n"));
                    DBGLOG(LP, INFO, ("Switch to active mode\n"));

                    pmFsmRunEventEnterActiveModeReq(prAdapter, NULL);

                    prPmInfo->fgFastSwitchInProgress = TRUE;
                }
            }
        }
        else {
            prPmInfo->rPmFastSwitchParam.u4TimeSlot = u4TimeSlot;
            prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum = 1;
            prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum = 0;
        }

        {
        P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;
        UINT_32 u4DtimWakeIntv;

        if (!prPmInfo->fgFastSwitchInProgress) {
            /* detect by the consecutive RX in STA wakeup intervals */
            u4DtimWakeIntv = prBssInfo->u2BeaconInterval * prBssInfo->ucDtimPeriod;

            if (CHECK_FOR_TIMEOUT(rCurSysTime,
                                  prPmInfo->rPmFastSwitchParam.rLastRxUcDataTime,
                                  MSEC_TO_SYSTIME(u4DtimWakeIntv + 20))) {

                prPmInfo->rPmFastSwitchParam.u4ConsecutiveRxCount = 0;
            } else {
                P_CONNECTION_SETTINGS_T prConnSettings;
                UINT_32 u4RxThr;

                prPmInfo->rPmFastSwitchParam.u4ConsecutiveRxCount++;

                prConnSettings = &prAdapter->rConnSettings;
                if (prConnSettings->u4VoipTrafficInterval) {
                    // should accept more than 2 VoIP call simultaneously under PS mode
                    u4RxThr = 2 * u4DtimWakeIntv / prConnSettings->u4VoipTrafficInterval +
                              DEFAULT_PSP_CONSECUTE_RX_SWITCH_TO_ACT;
                } else {
                    u4RxThr = DEFAULT_PSP_CONSECUTE_RX_SWITCH_TO_ACT;
                }

                if (prPmInfo->rPmFastSwitchParam.u4ConsecutiveRxCount > u4RxThr) {

                    pmFsmRunEventEnterActiveModeReq(prAdapter, NULL);

                    prPmInfo->fgFastSwitchInProgress = TRUE;
                }
            }
        }
        prPmInfo->rPmFastSwitchParam.rLastRxUcDataTime = rCurSysTime;
        }
    }
    else {
        prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum++;
    }

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmIndicateDataFrameTransmitted (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_32      u4DataPacketNum
    )
{
    P_PM_INFO_T                 prPmInfo;
    OS_SYSTIME                  rCurSysTime;
    UINT_32                     u4TimeSlot;
    P_BSS_INFO_T                prBssInfo;
    P_PM_PROFILE_SETUP_INFO_T   prPmProfSetupInfo;
    UINT_32                     u4TxPktNumThr;

    DEBUGFUNC("pmIndicateDataFrameTransmitted");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    if ((prBssInfo->eBSSType != BSS_TYPE_INFRASTRUCTURE) ||
        (prAdapter->eConnectionState != MEDIA_STATE_CONNECTED)) {
        return;
    }

    /* resume continuous polling mode on TX done event */
    if (prPmInfo->fgIsContinousPollingEnabled &&
        (prPmInfo->fgIsContinousPollingSuspended ||
        prPmInfo->fgContiPollIsStepUp == FALSE)) {
        pmResumeContinuousPollingFunc(prAdapter);
        pmContinuousPollingFreqStepUp(prAdapter, TRUE);
    }

    if (prPmInfo->eCurPsProf != ENUM_PSP_FAST_SWITCH) {
        return;
    }

    GET_CURRENT_SYSTIME(&rCurSysTime);

    /* The slot is define in 1000ms under Active mode and 100ms under PS mode by default, which counts the transmitted data packet.
    */
    if (PM_IS_UNDER_POWER_SAVE_MODE(prAdapter)) {
        u4TimeSlot = SYSTIME_TO_MSEC(rCurSysTime) / prPmProfSetupInfo->u2PsModeSwIntv;

        u4TxPktNumThr   = prPmProfSetupInfo->u2TxThrSwToAct *
                        (prPmProfSetupInfo->u2PsModeSwIntv / 100);

        if (u4TimeSlot == prPmInfo->rPmFastSwitchParam.u4TimeSlot) {

            prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum += u4DataPacketNum;

            if (!prPmInfo->fgFastSwitchInProgress) {

                if (prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum > u4TxPktNumThr) {

                    DBGLOG(LP, INFO, ("u4AccumTxDataPktNum: %ld, u4TxPktNumThr: %ld\n",
                                        prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum,
                                        u4TxPktNumThr));

                    DBGLOG(LP, INFO, ("Power save profile - FAST mode\n"));
                    DBGLOG(LP, INFO, ("Switch to active mode\n"));

                    pmFsmRunEventEnterActiveModeReq(prAdapter, NULL);

                    prPmInfo->fgFastSwitchInProgress = TRUE;
                }
            }
        }
        else {
            prPmInfo->rPmFastSwitchParam.u4TimeSlot = u4TimeSlot;
            prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum = 0;
            prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum = u4DataPacketNum;
        }
    }
    else {
        prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum += u4DataPacketNum;
    }

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmFsmRunEventEnterPowerSaveModeDecision (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    P_BSS_INFO_T prBssInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
    UINT_32     u4RxUcPktNum, u4TxPktNum;
    UINT_32     u4RxUcPktNumThr, u4TxPktNumThr;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;
    OS_SYSTIME                  rCurSysTime;

    DEBUGFUNC("pmFsmRunEventEnterPowerSaveModeDecision");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    if ((prBssInfo->eBSSType != BSS_TYPE_INFRASTRUCTURE) ||
        (prAdapter->eConnectionState != MEDIA_STATE_CONNECTED) ||
        (prPmInfo->eCurPsProf != ENUM_PSP_FAST_SWITCH)) {
        DBGLOG(LP, INFO, ("exit 1\n"));
        return WLAN_STATUS_NOT_ACCEPTED;
    }

    /* this handler only applied in ACTIVE mode */
    if (PM_IS_UNDER_POWER_SAVE_MODE(prAdapter)) {
        DBGLOG(LP, INFO, ("exit 2\n"));

        return status;
    }

    GET_CURRENT_SYSTIME(&rCurSysTime);

    if (!CHECK_FOR_TIMEOUT(rCurSysTime,
                           prPmInfo->rPmFastSwitchParam.rLastSwitchTime,
                           MSEC_TO_SYSTIME(DEFAULT_ACTIVE_MODE_SWITCH_INTERVAL))) {
        /* return if it does not stay for over DEFAULT_ACTIVE_MODE_SWITCH_INTERVAL in active mode */
        return status;
    }

    u4RxUcPktNum = prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum;
    u4TxPktNum = prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum;

    ASSERT(prPmProfSetupInfo->u2ActModeSwIntv);
    ASSERT(prPmProfSetupInfo->u2PsModeSwIntv);

    u4RxUcPktNumThr = DEFAULT_PSP_SWITCH_TO_PS_RX_THRESHOLD;
    u4TxPktNumThr   = DEFAULT_PSP_SWITCH_TO_PS_TX_THRESHOLD;

    if ((u4RxUcPktNum < u4RxUcPktNumThr) &&
        (u4TxPktNum < u4TxPktNumThr)) {
        if (pmFsmRunEventEnterPowerSaveModeReq(prAdapter, NULL) !=
            WLAN_STATUS_SUCCESS) {

            /* exception handler */
        }
    }

    /* repeat this timer handler */
    ARB_SET_TIMER(prAdapter,
                  prPmInfo->rPmSwitchPsModeTimer,
                  prPmProfSetupInfo->u2ActModeSwIntv);

    /* Clear the counters for next round */
    prPmInfo->rPmFastSwitchParam.u4AccumTxDataPktNum = 0;
    prPmInfo->rPmFastSwitchParam.u4AccumRxUcPktNum = 0;

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSetupPwrMgtInfo (
    IN  UINT_16         u2AID,
    IN  UINT_16         u2BcnIntv,
    IN  UINT_8          ucDtimPeriod,
    IN  UINT_16         u2AtimWindow,
    IN  BOOLEAN         fgIsUapsdConn,
    OUT P_PM_CONN_SETUP_INFO_T  prPmConnSetupInfo
    )
{
    ASSERT(prPmConnSetupInfo);

    prPmConnSetupInfo->u2AID        = u2AID;
    prPmConnSetupInfo->u2BcnIntv    = u2BcnIntv;
    prPmConnSetupInfo->ucDtimPeriod = ucDtimPeriod;
    prPmConnSetupInfo->u2AtimWindow = u2AtimWindow;
    prPmConnSetupInfo->fgIsUapsdConn = fgIsUapsdConn;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#if 0
VOID
pmSetPatternMatchParam (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucPatternList[],
    IN BOOLEAN     fgBlackListFilter,
    OUT UINT_8  ucPatternListInUsed[]
    )
{
    ASSERT(prAdapter);
    // IPv4
    // ARP, my own IP
    // ARP, my own ARP cache IP <-> MAC
    // NetBIOS name service
    // IPX
    // VoIP call?
#if 0
    /* IPv6 */
    // neighbor discovery
    // multicast listener discovery
#endif
}
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmComposeNullFrame (
    IN  P_ADAPTER_T     prAdapter,
    IN  PUINT_8         pucBuffer,
    IN  UINT_8          ucTid,
    OUT PUINT_32        pu4WlanHeaderLen,
    OUT PUINT_32        pu4WlanBodyLen
    )
{
    P_WLAN_MAC_HEADER_QOS_T     prMacHeader;
    UINT_16                     u2FrameCtrl;
    UINT_16                     u2QosControl = 0;
    P_BSS_INFO_T                prBssInfo;
    PUINT_8                     pucRA;

    DEBUGFUNC("pmComposeNullFrame");

    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(pu4WlanHeaderLen);
    ASSERT(pu4WlanBodyLen);

    prBssInfo = &prAdapter->rBssInfo;

    /* Compose the MAC header of the Null frame. */
    prMacHeader =(P_WLAN_MAC_HEADER_QOS_T) pucBuffer;

    if (prBssInfo->fgIsWmmAssoc) {
        u2FrameCtrl = (UINT_16) MAC_FRAME_QOS_NULL;

        u2QosControl = (UINT_16)ucTid;

        WLAN_SET_FIELD_16(&prMacHeader->u2QosCtrl, u2QosControl);
    }
    else {
        /* Fill the Frame Control field. */
        u2FrameCtrl = (UINT_16) MAC_FRAME_NULL;
    }

    /* Set the To DS and From DS bits in the Frame Control field. */
    ASSERT(prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE);

    /* Update the Frame Control field by current Operation Mode. */
    u2FrameCtrl |= MASK_FC_TO_DS;

    pucRA = &prAdapter->rBssInfo.aucBSSID[0];

    WLAN_SET_FIELD_16(&prMacHeader->u2FrameCtrl, u2FrameCtrl);

    /* Fill the Address 1 field with the Receive Address. */
    COPY_MAC_ADDR(prMacHeader->aucAddr1, pucRA);

    /* Fill the Address 2 field with our own MAC address. */
    COPY_MAC_ADDR(prMacHeader->aucAddr2, prAdapter->aucMacAddress);

    /* Fill the Address 3 field with the destination address. */
    COPY_MAC_ADDR(prMacHeader->aucAddr3, prAdapter->rBssInfo.aucBSSID);

    /* Set the MAC header length. */
    *pu4WlanBodyLen = 0;

    if (prBssInfo->fgIsWmmAssoc) {
        *pu4WlanHeaderLen = sizeof(WLAN_MAC_HEADER_QOS_T);
    }
    else {
        *pu4WlanHeaderLen = sizeof(WLAN_MAC_HEADER_T);
    }

    return;
}   /* pmComposeNullFrame */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmSendNullFrameForConnectionDiagnostic (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_TX_DONE_HANDLER  prFuncCallback
    )
{
    P_PM_INFO_T prPmInfo;

    DEBUGFUNC("pmSendNullFrameForConnectionDiagnostic");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    return pmSendNullFrame(prAdapter,
                           0,
                           prAdapter->rPmInfo.fgPwrMode_PS,
                           prPmInfo->fgIsContinousPollingEnabled ? FALSE : TRUE,
                           prFuncCallback);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmSendNullFrame (
    IN P_ADAPTER_T          prAdapter,
    IN UINT_8               ucTid,
    IN BOOLEAN              fgPsBit,
    IN BOOLEAN              fgUseBasicRate,
    IN PFN_TX_DONE_HANDLER  prFuncCallback
    )
{
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;
    UINT_32 u4WlanHeaderLen;
    UINT_32 u4WlanBodyLen;
    OS_SYSTIME rArrivalTime;
    P_PM_INFO_T prPmInfo;


    DEBUGFUNC("pmSendNullFrame");

    DBGLOG(MGT, INFO, ("\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    /* change PS bit in the FC (without care if NULL is transmitted or not) */
    nicpmSwitchPwrMgtMode(prAdapter, fgPsBit);

    /* Attempt to allocate a buffer to compose a Null frame. */
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
    if (!prMsduInfo) {
        WARNLOG(("No buffer for NULL frame\n"));
        return WLAN_STATUS_RESOURCES;
    }

    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, sizeof(WLAN_MAC_HEADER_QOS_T));
    if (!prMgtPacket) {
        WARNLOG(("No buffer for NULL frame\n"));
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        return WLAN_STATUS_RESOURCES;
    }

    /* Compose a Null frame. */
    /* NOTICE: due to current NULL frame is used to change PS mode
               under Infrastructure mode, here it only left the combination
               of ToDS = 1, FromDS = 0, and destination is to AP */
    pmComposeNullFrame(prAdapter,
                       MGT_PACKET_GET_BUFFER(prMgtPacket),
                       ucTid,
                       &u4WlanHeaderLen,
                       &u4WlanBodyLen);

    mgtPacketPut(prMgtPacket, u4WlanHeaderLen + u4WlanBodyLen);

    /* use AC3 for transmitting NULL frame */
    GET_CURRENT_SYSTIME(&rArrivalTime);

    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                       TRUE, \
                       TRUE, \
                       (PVOID)prMgtPacket, \
                       0, \
                       TXQ_AC3, \
                       (UINT_8)u4WlanHeaderLen, \
                       (UINT_16)u4WlanBodyLen, \
                       MSDU_INFO_CTRL_FLAG_SPECIFY_AC | \
                       ((fgUseBasicRate) ? MSDU_INFO_CTRL_FLAG_BASIC_RATE : 0), \
                       prFuncCallback, \
                       rArrivalTime, \
                       NULL \
                       );

    DBGLOG(MGT, TRACE, ("Send Null frame (PS:%d)\n", fgPsBit));

    if ((status = arbFsmRunEventTxMmpdu(prAdapter, prMsduInfo)) != WLAN_STATUS_PENDING) {
        if(status != WLAN_STATUS_SUCCESS)
        {
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            status = WLAN_STATUS_FAILURE;
        }
    }

    prPmInfo->rNullState = NULL_STATE_NONE;

    return status;
} /* pmSendNullFrame */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSetIpAddress (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_8      aucIpAddress
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    DEBUGFUNC("pmSetIpAddress");

    ASSERT(prAdapter);
    ASSERT(aucIpAddress);

    DBGLOG(LP, TRACE, ("\n"));

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;

//    if (aucIpAddress) {
	if(0 != aucIpAddress[0] + aucIpAddress[1] + aucIpAddress[2] + aucIpAddress[3]){
        prPmInfo->fgIpAddressIsValid = TRUE;
        kalMemCopy(&prPmInfo->u4IpAddress, aucIpAddress, 4);

        if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
            nicpmPatternConfigArpFilter(prAdapter, &prPmInfo->u4IpAddress);

            // Enable pattern search function
            nicpmConfigPatternSearchFunc(prAdapter,
                                         prPmProfSetupInfo->fgBcPtrnSrchEn,
                                         prPmProfSetupInfo->fgMcPtrnSrchEn,
                                         prPmProfSetupInfo->fgUcPtrnSrchEn,
                                         prPmProfSetupInfo->fgBcPtrnMatchRcv,
                                         prPmProfSetupInfo->fgMcPtrnMatchRcv,
                                         prPmProfSetupInfo->fgUcPtrnMatchRcv,
                                         FALSE);
        }
    }
    else {
        prPmInfo->fgIpAddressIsValid = FALSE;
        kalMemZero(&prPmInfo->u4IpAddress, 4);

	//FOR ARP filter
        nicpmConfigPattern(prAdapter,
                           0,   // entry index
                           FALSE,
                           FALSE,
                           FALSE,
                           FALSE,
                           FALSE,
                           FALSE,
                           FALSE,
                           FALSE,
                           FALSE,
                           0,
                           0,
                           &prPmInfo->u4IpAddress);
        /* Due to it is consider that only ARP pattern is to be set by this
           entry, pattern search is going to be disabled here directly */

        /* Disable Pattern search */
        nicpmConfigPatternSearchFunc(prAdapter,
                                     FALSE,
                                     FALSE,
                                     FALSE,
                                     FALSE,
                                     FALSE,
                                     FALSE,
                                     FALSE);

    }
}

#if CFG_IBSS_POWER_SAVE
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmEnableIbssPsTx (
    IN  P_ADAPTER_T     prAdapter
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (!prPmInfo->fgIsIbssPsTxEnabled) {

        // enable TBTT interrupt for TX operation
        NIC_SET_INT_EVENT(prAdapter, INT_EVENT_TBTT);

        // enable ATIM interrupt for TX operation
        NIC_SET_INT_EVENT(prAdapter, INT_EVENT_ATIM);

        // enable BCN TR OK interrupt for TX operation
        NIC_SET_INT_EVENT(prAdapter, INT_EVENT_BCN_TR_OK);


        // enable tbtt interrupt trigger
        NIC_PM_ENABLE_TBTT_INTR_TRIGGER(prAdapter);

        nicTxStartQueues(prAdapter, TXQ_DATA_MASK | TXQ_MGMT_MASK);

        prPmInfo->fgIsIbssPsTxEnabled = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmDisableIbssPsTx (
    IN  P_ADAPTER_T     prAdapter
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (prPmInfo->fgIsIbssPsTxEnabled) {

        // Disable TBTT interrupt for TX operation
        NIC_UNSET_INT_EVENT(prAdapter, INT_EVENT_TBTT);

        // Disable ATIM interrupt for TX operation
        NIC_UNSET_INT_EVENT(prAdapter, INT_EVENT_ATIM);

        // Disable BCN TR OK interrupt for TX operation
        NIC_UNSET_INT_EVENT(prAdapter, INT_EVENT_BCN_TR_OK);

        // Disable tbtt interrupt trigger
        NIC_PM_DISABLE_TBTT_INTR_TRIGGER(prAdapter);

        // Disable ALL the STA awake status to ensure it is treat as SLEEP for all STAs
        pmClearAllStationAwakeState(prAdapter);

        // Set this flag to ensure it will not transmit data packet within ATIM window
        PM_SET_FLAG_UNDER_ATIM_WINDOW(prAdapter);

        nicTxFlushStopQueues(prAdapter, 0x0 /*(UINT_8)NULL*/, TXQ_DATA_MASK | TXQ_MGMT_MASK);

        prPmInfo->fgIsIbssPsTxEnabled = FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmComposeAtimFrame (
    IN  P_ADAPTER_T     prAdapter,
    IN  PUINT_8         pucBuffer,
    IN  PUINT_8         prDestAddr,
    OUT PUINT_32        pu4WlanHeaderLen,
    OUT PUINT_32        pu4WlanBodyLen
    )
{
    P_WLAN_MAC_MGMT_HEADER_T    prMacHeader;
    UINT_16                     u2Tmp;

    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(prDestAddr);
    ASSERT(pu4WlanHeaderLen);
    ASSERT(pu4WlanBodyLen);

    /* Compose the MAC header of the Null frame. */
    prMacHeader =(P_WLAN_MAC_MGMT_HEADER_T) pucBuffer;

    u2Tmp = (UINT_16) MAC_FRAME_ATIM;

    WLAN_SET_FIELD_16(&prMacHeader->u2FrameCtrl, u2Tmp);

    /* Fill the Address 1, Address 2, and Address 3 fields */

    /* Fill the Address 1 field with the destination address. */
    COPY_MAC_ADDR(prMacHeader->aucDestAddr, prDestAddr);

    /* Fill the Address 2 field with our own MAC address. */
    COPY_MAC_ADDR(prMacHeader->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the Address 3 field with the current BSSID. */
    COPY_MAC_ADDR(prMacHeader->aucBSSID, prAdapter->rBssInfo.aucBSSID);

    /* Set the MAC header length. */
    *pu4WlanBodyLen = 0;
    *pu4WlanHeaderLen = sizeof(WLAN_MAC_MGMT_HEADER_T);
}   /* mgtComposeAtimFrame */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmSendAtimFrame (
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucDestAddr,
    IN PFN_TX_DONE_HANDLER  pfTxDoneHandler
    )
{
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;
    UINT_32 u4WlanHeaderLen;
    UINT_32 u4WlanBodyLen;
    OS_SYSTIME rArrivalTime;

    DEBUGFUNC("pmSendAtimFrame");

    ASSERT(prAdapter);
    ASSERT(pucDestAddr);
    ASSERT(pfTxDoneHandler);

    DBGLOG(LP, INFO, ("Send ATIM to DA: ["MACSTR"]\n", MAC2STR(pucDestAddr)));

    /* Attempt to allocate a buffer to compose a Null frame. */
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
    if (!prMsduInfo) {
        DBGLOG(LP, WARN, ("No MSDU_INFO_T to send ATIM frame.\n"));
        return WLAN_STATUS_RESOURCES;
    }

    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, sizeof(WLAN_MAC_MGMT_HEADER_T));
    if (!prMgtPacket) {
        DBGLOG(LP, WARN, ("No buffer for composing ATIM frame.\n"));
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        return WLAN_STATUS_RESOURCES;
    }

    /* Compose a Null frame. */
    /* NOTICE: due to current NULL frame is used to change PS mode
               under Infrastructure mode, here it only left the combination
               of ToDS = 1, FromDS = 0, and destination is to AP */
    pmComposeAtimFrame(prAdapter,
                       MGT_PACKET_GET_BUFFER(prMgtPacket),
                       pucDestAddr,
                       &u4WlanHeaderLen,
                       &u4WlanBodyLen);

    mgtPacketPut(prMgtPacket, u4WlanHeaderLen + u4WlanBodyLen);

    /* use AC4 for transmitting ATIM frame */
    GET_CURRENT_SYSTIME(&rArrivalTime);

    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                       TRUE, \
                       TRUE, \
                       (PVOID)prMgtPacket, \
                       0, \
                       TXQ_AC4, \
                       (UINT_8)u4WlanHeaderLen, \
                       (UINT_16)u4WlanBodyLen, \
                       MSDU_INFO_CTRL_FLAG_SPECIFY_AC | MSDU_INFO_CTRL_FLAG_BASIC_RATE, \
                       pfTxDoneHandler, \
                       rArrivalTime, \
                       staRecGetStaRecordByAddr(prAdapter, pucDestAddr) \
                       );

    if ((status = arbFsmRunEventTxMmpdu(prAdapter, prMsduInfo)) != WLAN_STATUS_PENDING) {
        if(status != WLAN_STATUS_SUCCESS) {
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            status = WLAN_STATUS_FAILURE;
        }

    }

    return status;
} /* pmSendAtimFrame */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSetAdhocStaAwake (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    )
{
    P_STA_RECORD_T prStaRec;

    DEBUGFUNC("pmSetAdhocStaAwake");

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);

    prStaRec = prMsduInfo->prStaRec;

    ASSERT(prStaRec);

    if (rTxDoneStatus == WLAN_STATUS_SUCCESS) {
        DBGLOG(LP, INFO, ("OK to send ATIM frame for STA:["MACSTR"]\n",
            MAC2STR(prStaRec->aucMacAddr)));

        prStaRec->fgIsAdhocStaAwake = TRUE;
    }
    else {
        DBGLOG(LP, INFO, ("Failed to send ATIM frame for STA:["MACSTR"]\n",
            MAC2STR(prStaRec->aucMacAddr)));

        /* Flush corrsepondent STA wait queue */
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSetAdhocAllStaAwake (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    )
{
    P_STA_INFO_T prStaInfo;

    DEBUGFUNC("pmSetAdhocAllStaAwake");

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prStaInfo = &prAdapter->rStaInfo;
    if (rTxDoneStatus == WLAN_STATUS_SUCCESS) {
        DBGLOG(LP, INFO, ("Set All STA awake\n"));

        prStaInfo->fgIsAllAdhocStaAwake = TRUE;
    }
    else {
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
pmClearAllStationAwakeState (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_STA_INFO_T    prStaInfo;
    P_LINK_T        prValidStaRecList;
    P_STA_RECORD_T  prStaRec;
    UINT_32         i;

    DBGLOG(LP, INFO, ("\n"));

    ASSERT(prAdapter);

    prStaInfo = &prAdapter->rStaInfo;

    /* To get the proper valid list through hash function */
    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            prStaRec->fgIsAdhocStaAwake = FALSE;
        }
    }

    prStaInfo->fgIsAllAdhocStaAwake = FALSE;

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
pmIsTxQueueEmptyByStaRec (
    IN P_ADAPTER_T          prAdapter,
    IN P_STA_RECORD_T       prStaRecord
    )
{
    UINT_32 i;
    BOOLEAN fgState = TRUE;

    ASSERT(prAdapter);
    ASSERT(prStaRecord);

    for (i = TC0; i <= TC3; i++) {
        if (QUEUE_IS_NOT_EMPTY(&prStaRecord->arStaWaitQueue[i])) {
            fgState = FALSE;
            break;
        }
    }

    return fgState;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
pmGetBufferedTxInfoInStaRec (
    IN      P_ADAPTER_T     prAdapter,
    IN OUT  PUINT_8         prStaAddrArray,
    IN      UINT_32         u4InBufferLen,
    OUT     PUINT_32        pu4StaNum
    )
{
    UINT_32         i;
    P_STA_RECORD_T  prStaRec;
    UINT_32         u4InBufLenLeft = u4InBufferLen;
    BOOLEAN         fgBufferEnough = TRUE;
    P_LINK_T        prValidStaRecList;
    P_STA_INFO_T    prStaInfo;

    ASSERT(prAdapter);
    ASSERT(prStaAddrArray);
    ASSERT(pu4StaNum);

    prStaInfo = &prAdapter->rStaInfo;


    *pu4StaNum = 0;

    for (i = 0; i < STA_RECORD_HASH_NUM; i++) {
        prValidStaRecList = &prStaInfo->arValidStaRecList[i];

        LINK_FOR_EACH_ENTRY(prStaRec, prValidStaRecList, rLinkEntry, STA_RECORD_T) {

            STA_RECORD_CHK_GUID(prStaRec);

            if (!pmIsTxQueueEmptyByStaRec(prAdapter, prStaRec)) {
                if (u4InBufLenLeft >= MAC_ADDR_LEN) {
                    COPY_MAC_ADDR(&prStaAddrArray[*pu4StaNum * MAC_ADDR_LEN],
                                    prStaRec->aucMacAddr);
                    u4InBufLenLeft -= MAC_ADDR_LEN;
                }
                else {
                    fgBufferEnough = FALSE;
                }
                *pu4StaNum += 1;
            }
        }
    }

    return fgBufferEnough;
}

#endif /* CFG_IBSS_POWER_SAVE */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSetAcpiPowerD0 (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    /* Setting this bit to set it to ARB_STATE_STANDBY after reseted */
    prAdapter->rAcpiState = ACPI_STATE_D0;

    nicpmPowerOn(prAdapter);

    arbFsmRunEventReset(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSetAcpiPowerD3 (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    /* Setting this bit to set it to ARB_STATE_POWER_OFF after aborted */
    prAdapter->fgIsEnterD3ReqIssued = TRUE;

    arbFsmRunEventAbort(prAdapter, FALSE);

    prAdapter->rAcpiState = ACPI_STATE_D3;
}

#if CFG_LP_IOT // used for emulation/ IOT field try only
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
pmSetIotBeaconTimeoutCheck (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_MIBSCR, &u4RegValue);
    u4RegValue |= BIT(22); // beacon timeout
    HAL_MCR_WR(prAdapter, MCR_MIBSCR, u4RegValue);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmFsmRunEventReadBeaconTimeoutCount (
    IN P_ADAPTER_T  prAdapter
    )
{
    UINT_32     u4RegValue;
    OS_SYSTIME  rCurrentSysTime;
    P_PM_INFO_T prPmInfo;
    UINT_32     u4BcnLostRate100000Fold;

    DEBUGFUNC("pmFsmRunEventReadBeaconTimeoutCount");

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    HAL_MCR_RD(prAdapter, MCR_MIBSCR, &u4RegValue);
    u4RegValue &= ~MIBSCR_INDEX_MASK;
    u4RegValue |= MIBSCR_BEACON_TIMEOUT_COUNT_EN; // beacon timeout
    u4RegValue |= 0xe;
    HAL_MCR_WR(prAdapter, MCR_MIBSCR, u4RegValue);

    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4RegValue);

    u4AccumMibBcnLostCnt += u4RegValue;

    GET_CURRENT_SYSTIME(&rCurrentSysTime);

    if ((g_ucSleepPeriod == 0) || (rCurrentSysTime == rTestStartTime)) {
        ASSERT(0);
        return;
    }

    u4BcnLostRate100000Fold = ((u4AccumMibBcnLostCnt*100000) /
                               ((rCurrentSysTime - rTestStartTime) / g_ucSleepPeriod));

    DBGLOG(LP_IOT, INFO, ("[LP-IOT] MIB Beacon timeout count: %d (lost rate: %d.%d%d%d )\n",
                        u4AccumMibBcnLostCnt,
                        u4BcnLostRate100000Fold / 1000,
                        (u4BcnLostRate100000Fold % 1000) / 100,
                        (u4BcnLostRate100000Fold % 100) / 10,
                        (u4BcnLostRate100000Fold % 10)
                        ));

//    DBGLOG(LP_IOT, INFO, ("[LP-IOT] Current timestamp: %d ms\n", rCurrentSysTime));

    if (CHECK_FOR_TIMEOUT(rCurrentSysTime,
                          rTestStartTime,
                          SEC_TO_MSEC(60 * 60))) { // 1hour = 60min = 60*60 sec= 3600*1000 ms
        DBGLOG(LP_IOT, INFO, ("[LP-IOT] connected for %d min (%d ms) (over 1 hour!!)\n",
                                (rCurrentSysTime - rTestStartTime)/60000,
                                (rCurrentSysTime - rTestStartTime)));
    }
    else {
        DBGLOG(LP_IOT, INFO, ("[LP-IOT] connected for %d min (%d ms)\n",
                                (rCurrentSysTime - rTestStartTime)/60000,
                                (rCurrentSysTime - rTestStartTime)));
    }
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] Max RCPI: 0x%x (%d dBm), Min RCPI: 0x%x (%d dBm)\n",
                            g_ucMaxRcpi,
                            RCPI_TO_DBM(g_ucMaxRcpi),
                            g_ucMinRcpi,
                            RCPI_TO_DBM(g_ucMinRcpi)));


    ARB_SET_TIMER(prAdapter,
                  prPmInfo->rPmReadBeaconTimeoutTimer,
                  10000);
}
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmEnableVoipPollingFunc (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_BSS_INFO_T                prBssInfo;
    P_PM_PROFILE_SETUP_INFO_T   prPmProfSetupInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;
    prConnSettings = &prAdapter->rConnSettings;

    /* Not allow VoIP profile switching once "continuous polling profile" is enabled */
    if (prPmInfo->fgIsContinousPollingEnabled) {
        return;
    }

    if (PM_IS_UNDER_POWER_SAVE_MODE(prAdapter) &&
        !prPmInfo->fgIsVoipPollingEnabled) {

        /* check if it is work under UAPSD connection, and AC3 (VO) is used as
           delivery-enabled AC
        */
        if ((prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_UAPSD) &&
            (GET_DLVR_EN_AC_INFO_FROM_APSD_BMP(prPmProfSetupInfo->bmfgApsdEnAc) & BIT(AC3))) {
            /* NOTE:
               use free run timer directly, since VoIP packet itself is a trigger frame also */
            NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                             HW_TIMER_FUNC_TSF_FREE_RUN,
                                             prConnSettings->u4VoipTrafficInterval,
                                             TRUE);

            nicTxVoipFlowCtrlEnable(prAdapter);

        }
        else {
            NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                             HW_TIMER_FUNC_TX_PS_POLL,
                                             prConnSettings->u4VoipTrafficInterval,
                                             TRUE);

            nicTxVoipFlowCtrlEnable(prAdapter);

        }

        /* setting up thresholds for PSP-fast switching */
        prPmProfSetupInfo->u2TxThrSwToAct  = DEFAULT_PSP_SWITCH_TO_ACT_TX_THRESHOLD_VOIP;
        prPmProfSetupInfo->u2TxThrSwToPs   = DEFAULT_PSP_SWITCH_TO_PS_TX_THRESHOLD_VOIP;
        prPmProfSetupInfo->u2RxThrSwToAct  = DEFAULT_PSP_SWITCH_TO_ACT_RX_THRESHOLD_VOIP;
        prPmProfSetupInfo->u2RxThrSwToPs   = DEFAULT_PSP_SWITCH_TO_PS_RX_THRESHOLD_VOIP;

        nicARSetProfile(prAdapter, AR_PROFILE_VOIP);

        prPmInfo->fgIsVoipPollingEnabled = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmDisableVoipPollingFunc (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T   prPmProfSetupInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    if (prPmInfo->fgIsVoipPollingEnabled) {
        nicTxVoipFlowCtrlDisable(prAdapter);
        NIC_HW_TIMER_UNSET_VOIP_POLL_TIMER(prAdapter);

        /* setting up thresholds for PSP-fast switching */
        prPmProfSetupInfo->u2TxThrSwToAct  = DEFAULT_PSP_SWITCH_TO_ACT_TX_THRESHOLD;
        prPmProfSetupInfo->u2TxThrSwToPs   = DEFAULT_PSP_SWITCH_TO_PS_TX_THRESHOLD;
        prPmProfSetupInfo->u2RxThrSwToAct  = DEFAULT_PSP_SWITCH_TO_ACT_RX_THRESHOLD;
        prPmProfSetupInfo->u2RxThrSwToPs   = DEFAULT_PSP_SWITCH_TO_PS_RX_THRESHOLD;

        nicARSetProfile(prAdapter, AR_PROFILE_THROUGHPUT);

        prPmInfo->fgIsVoipPollingEnabled = FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmEnableContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_BSS_INFO_T                prBssInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prConnSettings = &prAdapter->rConnSettings;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    /* following works is only done when connected */
    if (prAdapter->eConnectionState == MEDIA_STATE_DISCONNECTED) {
        return;
    }

    if (PM_IS_UNDER_POWER_SAVE_MODE(prAdapter)) {

        /* Note: It will overcede VoIP profile, which will use T3 timer also */
        pmDisableVoipPollingFunc(prAdapter);

        /* check if it is work under UAPSD connection */
        if ((prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_UAPSD) &&
            (prPmProfSetupInfo->bmfgApsdEnAc & BITS(0, 3)) &&
            (prPmProfSetupInfo->bmfgApsdEnAc & BITS(4, 7))) {
            NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                             HW_TIMER_FUNC_TX_TRG_FRM,
                                             prConnSettings->u4ContPollIntv,
                                             FALSE);
                                             /* modify for let PTA sense BT priority under LP */
                                             //prAdapter->rPtaInfo.fgSingleAntenna ? TRUE : FALSE);
        }
        else {
            NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                             HW_TIMER_FUNC_TX_PS_POLL,
                                             prConnSettings->u4ContPollIntv,
                                             FALSE);
                                             /* modify for let PTA sense BT priority under LP */
                                             //prAdapter->rPtaInfo.fgSingleAntenna ? TRUE : FALSE);
        }

#if 0
        /* Modify LP instruction @090218 */
        NIC_PM_PROGRAM_LP_INSRUCTION(prAdapter, TRUE);

        /* Modify LP wakeup timing @090218 */
        NIC_PM_MODIFY_WAKEUP_GUARD_TIME(prAdapter, TRUE);

        /* enable DTIM timeup interrupt @090218 */
        NIC_PM_CONFIG_DTIM_TIMEUP_INTR(prAdapter, TRUE);
#else
        {
        P_PTA_INFO_T        prPtaInfo;
        prPtaInfo = &prAdapter->rPtaInfo;
        if ((prPtaInfo->rBtProfile.eBtProfile == BT_PROFILE_MIXED) /*||
            (prPtaInfo->rBtProfile.eBtProfile == BT_PROFILE_ACL)*/
            && (prPmInfo->fgDtimTrigEn)){
            /* Disable DTIM wakeup @090224 */
            NIC_PM_CONFIG_DTIM_WAKEUP_FUNC(prAdapter, FALSE);
        } else if (prPmInfo->fgDtimTrigEn == FALSE) {
            /* restore DTIM wakeup @090224 */
            NIC_PM_CONFIG_DTIM_WAKEUP_FUNC(prAdapter, FALSE);
            NIC_PM_CONFIG_DTIM_WAKEUP_FUNC(prAdapter, TRUE);
        }
        }
#endif

        /* disable beacon timeout detection */
        nicpmSetupBeaconTimeoutDetection(prAdapter, FALSE);

        /* disable the setting for "more data" trigger */
        nicpmSetupMoreDataTrigger(prAdapter, FALSE);

        /* applied auto-rate for transmitting Null/PS Poll/trigger frame */
//        nicSetHwTxByBasicRate(prAdapter, FALSE);

        /* Enable continuos polling check timer */
        ARB_SET_TIMER(prAdapter,
                       prPmInfo->rContiPollingCheckTimer,
                       CONTI_POLLING_SUSPEND_CHECK_MSEC);

        prPmInfo->fgIsContinousPollingEnabled = TRUE;

    }
    else {
        /* Enter Power save mode */
        pmFsmRunEventEnterPowerSaveModeReq(prAdapter, NULL);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmDisableContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_CONNECTION_SETTINGS_T     prConnSettings;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prConnSettings = &prAdapter->rConnSettings;

    if (prPmInfo->fgIsContinousPollingEnabled) {
        NIC_HW_TIMER_UNSET_VOIP_POLL_TIMER(prAdapter);
#if 0
        /* Restore LP instruction and wakeup timing @090218 */
        NIC_PM_PROGRAM_LP_INSRUCTION(prAdapter, FALSE);

        /* Restore LP wakeup timing @090218 */
        NIC_PM_MODIFY_WAKEUP_GUARD_TIME(prAdapter, FALSE);

        /* disable DTIM timeup interrupt @090218 */
        NIC_PM_CONFIG_DTIM_TIMEUP_INTR(prAdapter, FALSE);
#else
        /* restore DTIM wakeup @090224 */
        NIC_PM_CONFIG_DTIM_WAKEUP_FUNC(prAdapter, TRUE);
#endif
        /* restore beacon timeout detection */
        nicpmSetupBeaconTimeoutDetection(prAdapter, TRUE);

        /* restore the setting for "more data" trigger */
        nicpmSetupMoreDataTrigger(prAdapter, TRUE);

        /* applied basic-rate for transmitting Null/PS Poll/trigger frame */
//        nicSetHwTxByBasicRate(prAdapter, TRUE);

        if (prPmInfo->eCurPsProf == ENUM_PSP_CONTINUOUS_ACTIVE) {
            if (prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
                pmFsmRunEventEnterActiveModeReq(prAdapter, NULL);
            }
            else if (prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
            }
        }

        /* Disable continuos polling check timer */
        ARB_CANCEL_TIMER(prAdapter,
                       prPmInfo->rContiPollingCheckTimer);

        prPmInfo->fgIsContinousPollingEnabled = FALSE;
        prPmInfo->fgIsContinousPollingSuspended = FALSE;

        /* Note: It will overcede VoIP profile, and it will be restored (if there's) */
        if (prConnSettings->fgIsVoipConn) {
            pmEnableVoipPollingFunc(prAdapter);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmResumeContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_BSS_INFO_T                prBssInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prConnSettings = &prAdapter->rConnSettings;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    if (prPmInfo->fgIsContinousPollingEnabled) {
        if (PM_IS_UNDER_POWER_SAVE_MODE(prAdapter)) {

            /* check if it is work under UAPSD connection */
            if ((prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_UAPSD) &&
                (prPmProfSetupInfo->bmfgApsdEnAc & BITS(0, 3)) &&
                (prPmProfSetupInfo->bmfgApsdEnAc & BITS(4, 7))) {
                NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                               HW_TIMER_FUNC_TX_TRG_FRM,
                                               prConnSettings->u4ContPollIntv,
                                               FALSE);
                                               /* modify for let PTA sense BT priority under LP */
                                               //prAdapter->rPtaInfo.fgSingleAntenna ? TRUE : FALSE);
            }
            else {
                NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                               HW_TIMER_FUNC_TX_PS_POLL,
                                               prConnSettings->u4ContPollIntv,
                                               FALSE);
                                               /* modify for let PTA sense BT priority under LP */
                                               //prAdapter->rPtaInfo.fgSingleAntenna ? TRUE : FALSE);
            }

            /* Enable continuos polling check timer */
            ARB_SET_TIMER(prAdapter,
                         prPmInfo->rContiPollingCheckTimer,
                         CONTI_POLLING_SUSPEND_CHECK_MSEC);

            prPmInfo->fgIsContinousPollingSuspended = FALSE;
        }
        else {
          /* It is assumed that it is under Power save mode for resumation */
          ASSERT(0);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmSuspendContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (prPmInfo->fgIsContinousPollingEnabled) {
        NIC_HW_TIMER_UNSET_VOIP_POLL_TIMER(prAdapter);

        /* Disable continuos polling check timer */
        ARB_CANCEL_TIMER(prAdapter,
                       prPmInfo->rContiPollingCheckTimer);

        prPmInfo->fgIsContinousPollingSuspended = TRUE;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmContinuousPollingFreqStepUp (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgStepUp
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_BSS_INFO_T                prBssInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;
    UINT_32                 u4PollIntv;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;
    prConnSettings = &prAdapter->rConnSettings;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (prPmInfo->fgIsContinousPollingEnabled) {
        if (fgStepUp) {
            u4PollIntv = prConnSettings->u4ContPollIntv;
            prPmInfo->fgContiPollIsStepUp = TRUE;
        } else {
            u4PollIntv = prConnSettings->u4ContPollIntvB;
            prPmInfo->fgContiPollIsStepUp = FALSE;
        }

        if (u4PollIntv) {
            /* check if it is work under UAPSD connection */
            if ((prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_UAPSD) &&
                (prPmProfSetupInfo->bmfgApsdEnAc & BITS(0, 3)) &&
                (prPmProfSetupInfo->bmfgApsdEnAc & BITS(4, 7))) {
                NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                               HW_TIMER_FUNC_TX_TRG_FRM,
                                               u4PollIntv,
                                               FALSE);
                                               /* modify for let PTA sense BT priority under LP */
                                               //prAdapter->rPtaInfo.fgSingleAntenna ? TRUE : FALSE);
            }
            else {
              NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter,
                                               HW_TIMER_FUNC_TX_PS_POLL,
                                               u4PollIntv,
                                               FALSE);
                                               /* modify for let PTA sense BT priority under LP */
                                               //prAdapter->rPtaInfo.fgSingleAntenna ? TRUE : FALSE);
            }
        } else {
            NIC_HW_TIMER_UNSET_VOIP_POLL_TIMER(prAdapter);
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmHandleContiPollingCheck (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    OS_SYSTIME rCurrentSysTime;

    ASSERT(prAdapter);
    prPmInfo = &prAdapter->rPmInfo;

    GET_CURRENT_SYSTIME(&rCurrentSysTime);

    /* if there's no unicast packet received within timeout,
       suspend continuous polling mode */
    if (CHECK_FOR_TIMEOUT(rCurrentSysTime,
                          ARB_FSM_GET_LAST_RX_UC_DATA_FRAME_SYSTIME(prAdapter),
                          MSEC_TO_SYSTIME(CONTI_POLLING_SUSPEND_CHECK_MSEC))) {

        pmContinuousPollingFreqStepUp(prAdapter, FALSE);
//        pmSuspendContinuousPollingFunc(prAdapter);

    } else {
        /* Enable continuos polling check timer */
        ARB_SET_TIMER(prAdapter,
                     prPmInfo->rContiPollingCheckTimer,
                     CONTI_POLLING_SUSPEND_CHECK_MSEC);
    }

    return;
} /* end of pmHandleContiPollingCheck() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
pmIsAbleToEnterPowerSaveMode (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    OS_SYSTIME  rCurrentSysTime;
    BOOLEAN     fgStatus = FALSE;

    ASSERT(prAdapter);
    ASSERT(prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE);

    prPmInfo = &prAdapter->rPmInfo;

    /* Entering PS request is not allowed before 1st beacon received after connected,
       this is the place PM related function activated
    */
    if ((prAdapter->fgBeaconReceivedAfterConnected == FALSE) ||
        (prAdapter->eConnectionState == MEDIA_STATE_DISCONNECTED)) {
        return fgStatus;
    }

    GET_CURRENT_SYSTIME(&rCurrentSysTime);

    if (CHECK_FOR_TIMEOUT(rCurrentSysTime,
                          prAdapter->rLastConnectedTime,
                          MSEC_TO_SYSTIME(CFG_KEEP_ACTIVE_DWELL_TIME_ON_CONNECT_MSEC))) {
        fgStatus = TRUE;
    }

    if (prPmInfo->ucWmmPsEnterPsAtOnce) {
        fgStatus = TRUE;
    }

    return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmFsmRunEventNullTxDone (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    )
{
    P_PM_INFO_T prPmInfo;


    ASSERT(prAdapter);
    prPmInfo = &prAdapter->rPmInfo;

    if (rTxDoneStatus == WLAN_STATUS_FAILURE) {
        prPmInfo->rNullState = NULL_STATE_FAIL;
    }
    else if (rTxDoneStatus == WLAN_STATUS_SUCCESS) {
        prPmInfo->rNullState = NULL_STATE_SUCCESS;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
pmBeaconTimeoutHandler (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_TX_DONE_HANDLER  prFuncCallback
    )
{
    P_PM_INFO_T prPmInfo;
    WLAN_STATUS status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("pmBeaconTimeoutHandler");

    DBGLOG(LP, INFO, ("\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    pmSendNullFrameForConnectionDiagnostic(prAdapter, prFuncCallback);

    if (!prPmInfo->fgIsContinousPollingEnabled) {
        /* Tune the beacon timeout (Step up) */
        if (!nicpmStepBeaconTimeout(prAdapter, TRUE)) {
            if( prPmInfo->fgIsBcnTimeoutStep4 == TRUE )
            {
                /* Renbang : Abort if BCN timeout still happened after changing PM to CAM */
                printk("[wifi]beacon final timeout!! disconnect....\n");
                status = WLAN_STATUS_NOT_ACCEPTED;
            }
            else
            {
                /* Renbang : if BCN timeout step 4 happened, change PM profile to CAM in case of missing any beacon */
                printk("[wifi]beacon timeout!! change PM profile to CAM...\n");
                prPmInfo->fgIsBcnTimeoutStep4 = TRUE; 
                pmFsmRunEventSetPowerSaveProfile(prAdapter, ENUM_PSP_CONTINUOUS_ACTIVE);
            }
        }
        else {

            /* Set a timeout for tuning beacon timeout (step down) */
            ARB_SET_TIMER(prAdapter,
                           prPmInfo->rBeaconTimeoutHandlingTimer,
                           BEACON_TIMEOUT_STEP_DOWN_CHECK_INTERVAL);

            /* Force to wait for the next beacon */
            prAdapter->fgWaitOneBeaconForTsfRecovery = TRUE;

            /* NIC disable/ enable beacon content check */
            nicpmResetBeaconContentCheck(prAdapter);

            /* Set a timeout for waiting the next beacon */
            ARB_SET_TIMER(prAdapter,
                           prPmInfo->rWaitBeaconWatchdogTimer,
                           WAIT_BEACON_ON_CONNECTED_MSEC);
        }
    }

    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
pmDisableBeaconTimeoutDetectionFunc (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgDisable
    )
{
    P_PM_INFO_T                 prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        /* disable/ enable beacon timeout detection */
        nicpmSetupBeaconTimeoutDetection(prAdapter, fgDisable ? FALSE : TRUE);
    }

    prPmInfo->fgIsBeaconTimeoutDetectionDisabled = fgDisable;
}


