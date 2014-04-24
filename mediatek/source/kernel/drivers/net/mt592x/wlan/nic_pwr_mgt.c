






#include "precomp.h"








/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSwitchPwrMgtMode (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsPwrSave
    )
{
    ASSERT(prAdapter);

    halpmSwitchPwrMgtBit(prAdapter, fgIsPwrSave);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmEnterLowPower (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgEnableGlobalInt
    )
{
    ASSERT(prAdapter);

    halpmEnterLowPower(prAdapter, fgEnableGlobalInt);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmLeaveLowPower (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (!halpmLeaveLowPower(prAdapter, prPmInfo->u2GuardTimePhase2 + 1000)) {
        /* Recovery procedure when clock is not ready */
        nicpmPowerOn(prAdapter);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmSetupPmInfoInAdhoc (
    IN P_ADAPTER_T              prAdapter,
    IN P_PM_CONN_SETUP_INFO_T   prPmConnSetupInfo,
    IN BOOLEAN                  fgCreateIbss
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;


    ASSERT(prAdapter);
    ASSERT(prPmConnSetupInfo);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;

    /* program beacon search timeout */
#if CFG_IBSS_POWER_SAVE
    if (prPmConnSetupInfo->u2AtimWindow) {
        /* it is reasonable that beacon timeout is lager than ATIM window
        */
        halpmSetBeaconTimeout(prAdapter,
                              0,
                              prPmConnSetupInfo->u2AtimWindow,
                              FALSE,
                              TRUE,
                              BEACON_TIMEOUT_COUNT_ADHOC);
    } else
#endif /* CFG_IBSS_POWER_SAVE */
    {
        halpmSetBeaconTimeout(prAdapter,
                              BEACON_MIN_TIMEOUT_VALUE,
                              prPmConnSetupInfo->u2BcnIntv - 4,//BEACON_MAX_TIMEOUT_VALUE,
                              TRUE,
                              TRUE,
                              BEACON_TIMEOUT_COUNT_ADHOC);
    }


    if (fgCreateIbss) {
        /* To create a new IBSS
        */

        /* Reset local TSF timer to 0, and next TBTT timer will be updated/ calculated by
           halpmConfigLowPowerSettingInAdhoc()
        */
        halpmResetTSFTimer(prAdapter);
    }
    else {
        /* To merge with an IBSS
        */

        // Enable TSF drift detection, and value set to 1 TU
        halpmSetTsfDriftWindow(prAdapter, DEFAULT_TSF_DRIFT_WINDOW_TU);

        // Enable Service period
        nicpmSetupServicePeriodMechanism(prAdapter, SP_BEACON, 0);

        // Setup ARP pattern search for allowing only our ARP request/ response match our IP address
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

    halpmConfigLowPowerSettingInAdhoc(prAdapter,
                                      prPmConnSetupInfo->u2BcnIntv,
                                      prPmConnSetupInfo->u2AtimWindow,
                                      1,    //prPmProfSetupInfo->ucMultiTbttWake,
                                      prPmProfSetupInfo->u2HwTxLifeTime,
                                      (fgCreateIbss) ? FALSE : TRUE);


    // Enable HW function(s)
    halpmEnableLowPowerFunctionsInAdhoc(prAdapter, fgCreateIbss);

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmSetupPmInfoInBss (
    IN P_ADAPTER_T              prAdapter,
    IN P_PM_CONN_SETUP_INFO_T   prPmConnSetupInfo
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
    P_BSS_INFO_T prBssInfo;
    UINT_8 ucDtimPeriod;
    UINT_32 u4SleepPeriodTU;

    DEBUGFUNC("nicpmSetupPmInfoInBss");

    ASSERT(prAdapter);
    ASSERT(prPmConnSetupInfo);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;
    prBssInfo = &prAdapter->rBssInfo;

    /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;

#if DBG
    /* prevent divide by 0 */
    if (prPmConnSetupInfo->ucDtimPeriod == 0) {
        prPmConnSetupInfo->ucDtimPeriod = 1;
        ASSERT(0);
    }
    if (prPmProfSetupInfo->ucMultiDtimWake == 0) {
        prPmProfSetupInfo->ucMultiDtimWake = 1;
        ASSERT(0);
    }
    if (prPmConnSetupInfo->u2BcnIntv == 0) {
        prPmConnSetupInfo->u2BcnIntv = 100;
        ASSERT(0);
    }
#endif

#if CFG_LP_PATTERN_SEARCH_SLT
    switch (prAdapter->eSLTModeSel) {
    case SLT_MODE_NORMAL:
        ucDtimPeriod = prPmConnSetupInfo->ucDtimPeriod;
        break;
    case SLT_MODE_LP:
        ucDtimPeriod = 1;
        break;
    case SLT_MODE_PATTERN_SEARCH:
    default:
        ucDtimPeriod = prPmConnSetupInfo->ucDtimPeriod;
    }
#else
    ucDtimPeriod = prPmConnSetupInfo->ucDtimPeriod;
#endif

    /* CR1853: Calculate sleep period in unit of TU */
    u4SleepPeriodTU = prPmProfSetupInfo->ucMultiDtimWake *
        ucDtimPeriod *
        prPmConnSetupInfo->u2BcnIntv;

    if (u4SleepPeriodTU > prPmProfSetupInfo->u2DtimIntvThr) {
        /* GeorgeKuo: It's a strange case that DL UC packets will be delayed
        * more than u2DtimIntvThr seconds (default:2s). Change behavior: wake up
        * every ucMultiTbttWake TBTT to get UC packets more quickly.
        */
        if ( (prPmProfSetupInfo->ucWorkaroundTbttCount * prPmConnSetupInfo->u2BcnIntv)
            > prPmProfSetupInfo->u2DtimIntvThr) {
            /* if workaround TBTT count or beacon interval are too large, wake
            * up every TBTT.
            */
            prPmProfSetupInfo->ucMultiTbttWake = 1;
        }
        else {
            /* else use workaround TBTT count (default:2) as multi TBTT count */
            prPmProfSetupInfo->ucMultiTbttWake = prPmProfSetupInfo->ucWorkaroundTbttCount;
        }
        DBGLOG(LP, INFO, ("u4SleepPeriodTU(%ld) > u2DtimIntvThr(%d), wake up every (%d) TBTT\n",
            u4SleepPeriodTU,
            prPmProfSetupInfo->u2DtimIntvThr,
            prPmProfSetupInfo->ucWorkaroundTbttCount));
    }
    else {
        prPmProfSetupInfo->ucMultiTbttWake = DEFAULT_MULTIPLE_TBTT_COUNT;
    }

//for fixed DTIMWakeUp
#if 1
{
    UINT_32 wakeUp = prPmProfSetupInfo->ucMultiDtimWake;

    if(gPlatformCfg.rWifiCustom.u4MultiDtimWake){
	wakeUp = gPlatformCfg.rWifiCustom.u4MultiDtimWake;
    do_div(wakeUp, prPmConnSetupInfo->u2BcnIntv);	

    if(wakeUp < 1)
	wakeUp = 1;
    else if(wakeUp > 7)
	wakeUp = 7; 	
    }


    halpmConfigLowPowerSettingInBss(prAdapter,
                                    prPmConnSetupInfo->u2AID,
                                    prPmConnSetupInfo->u2BcnIntv,
                                    ucDtimPeriod,
                                    (prPmConnSetupInfo->fgIsUapsdConn) ?
                                    prPmProfSetupInfo->bmfgApsdEnAc : 0,
                                    (UINT_8)wakeUp,
                                    prPmProfSetupInfo->ucMultiTbttWake,
                                    (prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_WMM) ?
                                        prPmProfSetupInfo->fgUseAgeQoSNull : 0,
                                    prPmProfSetupInfo->ucAgeNullPeriod,
                                    prPmProfSetupInfo->ucQoSNullTid,
                                    prPmProfSetupInfo->ucTrgThr,
                                    prPmProfSetupInfo->u2HwTxLifeTime);
}
#else
    halpmConfigLowPowerSettingInBss(prAdapter,
                                    prPmConnSetupInfo->u2AID,
                                    prPmConnSetupInfo->u2BcnIntv,
                                    ucDtimPeriod,
                                    (prPmConnSetupInfo->fgIsUapsdConn) ?
                                        prPmProfSetupInfo->bmfgApsdEnAc : 0,
                                    prPmProfSetupInfo->ucMultiDtimWake,
                                    prPmProfSetupInfo->ucMultiTbttWake,
                                    (prBssInfo->rWmmInfo.ucWmmFlag & WMM_FLAG_SUPPORT_WMM) ?
                                        prPmProfSetupInfo->fgUseAgeQoSNull : 0,
                                    prPmProfSetupInfo->ucAgeNullPeriod,
                                    prPmProfSetupInfo->ucQoSNullTid,
                                    prPmProfSetupInfo->ucTrgThr,
                                    prPmProfSetupInfo->u2HwTxLifeTime);
#endif

    /* program beacon search timeout */
    halpmSetBeaconTimeout(prAdapter,
                          BEACON_MIN_TIMEOUT_VALUE,
                          prPmConnSetupInfo->u2BcnIntv - 4,//BEACON_MAX_TIMEOUT_VALUE,
                          TRUE,
                          TRUE,
                          BEACON_TIMEOUT_COUNT_INFRA);

    // Check the beacon earlier time transmitted by AP ahead from TBTT */
    halpmEnableBeaconEarlyCheck(prAdapter);

    // Enable TSF drift detection, and value set to 1 TU
    halpmSetTsfDriftWindow(prAdapter, DEFAULT_TSF_DRIFT_WINDOW_TU);

    // Enable Service period (with disable all first)
    nicpmSetupServicePeriodMechanism(prAdapter, 0, SP_ALL);
    nicpmSetupServicePeriodMechanism(prAdapter, SP_ALL, 0);

    // Setup ARP pattern search for allowing only our ARP request match our IP address
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

    // Enable HW function(s)
    halpmEnableLowPowerFunctionsInBss(prAdapter,
                                      ((prPmProfSetupInfo->ucMultiTbttWake) ? TRUE : FALSE));

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmAbortPmFunc (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;


    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;

    // Restore ATIM window to 0 (disable the function)
    halpmSetAtimWindowValue(prAdapter, 0);

    // Disable all LP function
    halpmDisableLowPowerFunctions(prAdapter);

    // Disable TSF drift detection
    nicpmSetTsfDriftWindow(prAdapter, 0);

    // Disable pattern search function
    nicpmConfigPatternSearchFunc(prAdapter,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 FALSE);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmRegInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T prPmInfo;
    BOOLEAN fgStatus;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    // Low power related register initialization
    halpmRegInit(prAdapter);

    /* Set OSC stable time */
    halpmSetOscStableTime(prAdapter, prAdapter->rEEPROMCtrl.u2OscStableTimeUs);

    // Calibrate 32K slow clock
    fgStatus = nicpmCalSlowClock(prAdapter, &prAdapter->rPmInfo.rSlowClkCnt);

    if (fgStatus) {
        /* Low Power instructions */
        if (prAdapter->ucRevID == MTK_CHIP_MP_REVERSION_ID) {
            prPmInfo->au4LowPowerInst_sleep[prPmInfo->ucNumOfInstSleep++] =
                LP_INST_DELAY(25, FALSE, FALSE);
        }
        // Low power instruction programming
        NIC_PM_PROGRAM_LP_INSRUCTION(prAdapter, FALSE);

        // Program wakeup guard time
        nicpmSetupWakeupGuardTime(prAdapter, &prAdapter->rPmInfo.rSlowClkCnt);
    }

    nicSetHwTxByBasicRate(prAdapter, TRUE);

    return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSetupServicePeriodMechanism (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_32      u4ValidateSP,
    IN UINT_32      u4InvalidateSP
    )
{
    ASSERT(prAdapter);

    halpmSetupServicePeriodMechanism(prAdapter, u4ValidateSP, u4InvalidateSP);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmCalSlowClock (
    IN P_ADAPTER_T prAdapter,
    OUT P_PM_SLOW_CLOCK_COUNT_T prSlowClkCnt
    )
{
    ASSERT(prAdapter);
    ASSERT(prSlowClkCnt);

    return halpmCalSlowClock(prAdapter,
                             &prSlowClkCnt->uc32kSlowCount,
                             &prSlowClkCnt->uc32kSlowCount_10,
                             &prSlowClkCnt->uc32kSlowCount_100,
                             &prSlowClkCnt->uc32kSlowCount_1000);

}   /* nicpmSlowClockCal */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSetupWakeupGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN P_PM_SLOW_CLOCK_COUNT_T prSlowClkCnt
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);
    ASSERT(prSlowClkCnt);

    prPmInfo = &prAdapter->rPmInfo;

    halpmCalculateGuardTime(prAdapter,
                            prSlowClkCnt->uc32kSlowCount,
                            prSlowClkCnt->uc32kSlowCount_10,
                            prSlowClkCnt->uc32kSlowCount_100,
                            prSlowClkCnt->uc32kSlowCount_1000,
                            &prPmInfo->u2GuardTimePhase2,
                            &prPmInfo->u2GuardTimePhase3);

    halpmSetupWakeupGuardTime(prAdapter,
                              prPmInfo->u2GuardTimePhase2,
                              prPmInfo->u2GuardTimePhase3);
}   /* nicpmSetupWakeupGuardTime */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmAdjustWakeupGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2BeaconEarlyOffset
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    prPmInfo->u2GuardTimePhase2 += u2BeaconEarlyOffset;
    prPmInfo->u2GuardTimePhase3 += u2BeaconEarlyOffset;

    halpmSetupWakeupGuardTime(prAdapter,
                              prPmInfo->u2GuardTimePhase2,
                              prPmInfo->u2GuardTimePhase3);
}   /* nicpmAdjustWakeupGuardTime */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_16
nicpmGetBeaconEarlyValue (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    return halpmGetBeaconEarlyValue(prAdapter);
}   /* nicpmGetBeaconEarlyValue */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTbttInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{

#if CFG_IBSS_POWER_SAVE

    ASSERT(prAdapter);

    if (!PM_IS_UNDER_IBSS_POWER_SAVE_MODE(prAdapter)) {
        return;
    }

    PM_SET_FLAG_UNDER_ATIM_WINDOW(prAdapter);

    DBGLOG(LP, INFO, (" \n"));

    nicTxDisableTxQueueActiveState(prAdapter);

    /* Flush and reclaim TX active chain, send wait queue into STA wait queue */
    nicTxReclaimTxPackets(prAdapter);

    /* clear IBSS STA awake state recorded by station record */
    pmClearAllStationAwakeState(prAdapter);

    /* Enable AC4 for ATIM */
    nicTxStartQueues(prAdapter, TXQ_MGMT_MASK);
#endif /* CFG_IBSS_POWER_SAVE */

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessBeaconTxRxOkInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{

#if CFG_IBSS_POWER_SAVE
    UINT_32 i;
    UINT_8 aucBCAddr[] = BC_MAC_ADDR;
    P_STA_INFO_T prStaInfo;
    BOOLEAN fgIsGcStaWaitQueueNotEmpty = FALSE;
    UINT_8 aucMacAddrArray[MAC_ADDR_LEN * CFG_MAX_NUM_STA_RECORD];
    UINT_32 u4NumSta;


    ASSERT(prAdapter);
    prStaInfo = &prAdapter->rStaInfo;

    if (!PM_IS_UNDER_IBSS_POWER_SAVE_MODE(prAdapter)) {
        return;
    }

    if (prAdapter->rArbInfo.eCurrentState == ARB_STATE_SCAN) {
        return;
    }

    DBGLOG(LP, INFO, ("\n"));

    /* Send broadcast ATIM to let all station keep awake,
       if there's any broadcast or multicast entries
       (do we need to send unicast ATIM anyway?)
    */
    for (i = TC0; i <= TC3; i++) {
        if (QUEUE_IS_NOT_EMPTY(&prStaInfo->arGcStaWaitQueue[i])) {
            fgIsGcStaWaitQueueNotEmpty = TRUE;
            break;
        }
    }


    if (fgIsGcStaWaitQueueNotEmpty) {
        pmSendAtimFrame(prAdapter,
                         aucBCAddr,
                         arbSetAdhocAllStaAwake);

    }
    else {
        if (pmGetBufferedTxInfoInStaRec(prAdapter,
                                        aucMacAddrArray,
                                        sizeof(aucMacAddrArray),
                                        &u4NumSta)) {

            /* Send unicast ATIM frames by station wait queue */
            for (i = 0; i < u4NumSta; i++) {

                pmSendAtimFrame(prAdapter,
                                 &aucMacAddrArray[i * MAC_ADDR_LEN],
                                 arbSetAdhocStaAwake);
            }
        }
        else {
            ASSERT(0);
        }
    }

#endif /* CFG_IBSS_POWER_SAVE */

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessAtimWindowTimeoutInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
#if CFG_IBSS_POWER_SAVE

    ASSERT(prAdapter);

    if (!PM_IS_UNDER_IBSS_POWER_SAVE_MODE(prAdapter)) {
        return;
    }

    PM_SET_FLAG_OUTSIDE_ATIM_WINDOW(prAdapter);

    DBGLOG(LP, INFO, ("\n"));

    nicTxDisableTxQueueActiveState(prAdapter);

    /* flush AC4 for in-completed ATIM packets */
    nicTxFlushStopQueues(prAdapter, TXQ_MGMT_MASK, TXQ_MGMT_MASK);

    /* Enable AC0~3, AC4 (for out-of-bound MGMT frame) */
    nicTxStartQueues(prAdapter, TXQ_DATA_MASK | TXQ_MGMT_MASK);

    /* Trigger TX actions */
    if (nicTxRetransmitOfStaWaitQue(prAdapter)) {
        pmDisableIbssPsTx(prAdapter);
    }

#endif /* CFG_IBSS_POWER_SAVE */

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmIfAdhocStaMaster (
    IN P_ADAPTER_T      prAdapter
    )
{
    ASSERT(prAdapter);

    return halpmIfAdhocStaMaster(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmPowerOn (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    halpmPowerOn(prAdapter, prAdapter->rPmInfo.rSlowClkCnt.uc32kSlowCount);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmPowerOff (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    halpmPowerOff(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmEnableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    halpmEnableTimeoutCounter(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmDisableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    halpmDisableTimeoutCounter(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSetTsfDriftWindow (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8    ucDriftWindow
    )
{
    ASSERT(prAdapter);

    halpmSetTsfDriftWindow(prAdapter, ucDriftWindow);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmPatternConfigArpFilter (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_32     pu4IpAddr
    )
{
    UINT_8 i;
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);
    ASSERT(pu4IpAddr);

    prPmInfo = &prAdapter->rPmInfo;

    if (!prPmInfo->fgIpAddressIsValid) {
        return FALSE;
    }


    /* This pattern include UC/ BC/ MC ARP request and response.
     * It will configure index 0 always, and keep others empty.
     */
    nicpmConfigPattern(prAdapter,
                       0,       // Use index 0 now
                       TRUE,
                       TRUE,
                       TRUE,
                       FALSE,
                       FALSE,
                       FALSE,
                       TRUE,
                       FALSE,   // Use OR now
                       FALSE,
                       BITS(0, 3),
                       0x20,
                       pu4IpAddr);

#if 1 // overwrite other entries to zero
    for (i = 1; i < HW_PATTERN_SEARCH_SET_NUMBER; i++) {
        nicpmConfigPattern(prAdapter,
                           i,   // entry index
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
                           NULL);
    }
#endif

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmConfigPattern (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucIndex,
    IN BOOLEAN      fgCheckBcA1,
    IN BOOLEAN      fgCheckMcA1,
    IN BOOLEAN      fgCheckUcA1,
    IN BOOLEAN      fgIpv4Ip,
    IN BOOLEAN      fgIpv6Icmp,
    IN BOOLEAN      fgGarpIpEqual,
    IN BOOLEAN      fgArpCtrl,
    IN BOOLEAN      fgAndOp,
    IN BOOLEAN      fgNotOp,
    IN UINT_8       ucPatternMask,
    IN UINT_8       ucPatternOffset,
    IN PUINT_32     pu4Pattern
    )
{
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    halpmConfigPattern(prAdapter,
                       ucIndex,
                       fgCheckBcA1,
                       fgCheckMcA1,
                       fgCheckUcA1,
                       fgIpv4Ip,
                       fgIpv6Icmp,
                       fgGarpIpEqual,
                       fgArpCtrl,
                       fgAndOp,
                       fgNotOp,
                       ucPatternMask,
                       ucPatternOffset,
                       pu4Pattern);


    if (fgCheckBcA1 && ucPatternMask) {
        prPmInfo->u4BcPatternXIsValid |= (UINT_32)(1 << ucIndex);
    }
    else {
        prPmInfo->u4BcPatternXIsValid &= ~((UINT_32)(1 << ucIndex));
    }
    if (fgCheckMcA1 && ucPatternMask) {
        prPmInfo->u4McPatternXIsValid |= (UINT_32)(1 << ucIndex);
    }
    else {
        prPmInfo->u4McPatternXIsValid &= ~((UINT_32)(1 << ucIndex));
    }
    if (fgCheckUcA1 && ucPatternMask) {
        prPmInfo->u4UcPatternXIsValid |= (UINT_32)(1 << ucIndex);
    }
    else {
        prPmInfo->u4UcPatternXIsValid &= ~((UINT_32)(1 << ucIndex));
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmConfigPatternSearchFunc (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgBcA1En,
    IN BOOLEAN      fgMcA1En,
    IN BOOLEAN      fgUcA1En,
    IN BOOLEAN      fgBcA1MatchDrop,
    IN BOOLEAN      fgMcA1MatchDrop,
    IN BOOLEAN      fgUcA1MatchDrop,
    IN BOOLEAN      fgIpv6MatchCtrl
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    /* It should check the pattern validation,
       if intending to ENABLE pattern search function.
     */
    if (!prPmInfo->u4BcPatternXIsValid && fgBcA1En) {
        fgBcA1En = FALSE;
    }
    if (!prPmInfo->u4McPatternXIsValid && fgMcA1En) {
        fgMcA1En = FALSE;
    }
    if (!prPmInfo->u4UcPatternXIsValid && fgUcA1En) {
        fgUcA1En = FALSE;
    }

    /* Note: It will over-write the setting in the PM profile */

#if 0 // Application should always invoke this function after connected
    // Backup the setting which is duplicated in "prPmProfSetupInfo"
    prPmProfSetupInfo->fgBcPtrnSrchEn = fgBcA1En;
    prPmProfSetupInfo->fgMcPtrnSrchEn = fgMcA1En;
    prPmProfSetupInfo->fgUcPtrnSrchEn = fgUcA1En;
    prPmProfSetupInfo->fgBcPtrnMatchRcv = fgBcA1MatchDrop;
    prPmProfSetupInfo->fgMcPtrnMatchRcv = fgMcA1MatchDrop;
    prPmProfSetupInfo->fgUcPtrnMatchRcv = fgUcA1MatchDrop;
#endif

    // Enable pattern search function
    halpmConfigPatternSearchFunction(prAdapter,
                                     fgBcA1En,
                                     fgMcA1En,
                                     fgUcA1En,
                                     fgBcA1MatchDrop,
                                     fgMcA1MatchDrop,
                                     fgUcA1MatchDrop,
                                     fgIpv6MatchCtrl);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmResetTSFTimer (
    IN P_ADAPTER_T prAdapter
    )
{
   halpmResetTSFTimer(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSetToBeaconTimeoutStep (
    IN P_ADAPTER_T              prAdapter,
    IN ENUM_PM_BCN_TO_STEPS_T   eStep
    )
{
    P_PM_INFO_T prPmInfo;
    P_BSS_INFO_T prBssInfo;
    UINT_8          ucMinTimeoutValue = 0;
    UINT_16         u2MaxTimeoutValue = 0;
    BOOLEAN         fgMinTimeoutValid = FALSE;
    BOOLEAN         fgMaxTimeoutValid = FALSE;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prBssInfo = &prAdapter->rBssInfo;


    if (eStep >= BCN_TO_STEP_NUM) {
        ASSERT(0);
        return;
    }

    switch (eStep) {
    case BCN_TO_STEP_0:
        ucMinTimeoutValue = BEACON_MIN_TIMEOUT_VALUE;
        u2MaxTimeoutValue = prBssInfo->u2BeaconInterval - 4;
        fgMinTimeoutValid = TRUE;
        fgMaxTimeoutValid = TRUE;
        break;
    case BCN_TO_STEP_1:
        ucMinTimeoutValue = 15;
        u2MaxTimeoutValue = prBssInfo->u2BeaconInterval - 4;
        fgMinTimeoutValid = TRUE;
        fgMaxTimeoutValid = TRUE;
        break;
    case BCN_TO_STEP_2:
        ucMinTimeoutValue = 31;
        u2MaxTimeoutValue = prBssInfo->u2BeaconInterval - 4;
        fgMinTimeoutValid = TRUE;
        fgMaxTimeoutValid = TRUE;
        break;
    case BCN_TO_STEP_3:
        ucMinTimeoutValue = 63;
        u2MaxTimeoutValue = prBssInfo->u2BeaconInterval - 4;
        fgMinTimeoutValid = TRUE;
        fgMaxTimeoutValid = TRUE;
        break;
    case BCN_TO_STEP_4:
        ucMinTimeoutValue = 0;
        u2MaxTimeoutValue = prBssInfo->u2BeaconInterval - 4;
        fgMinTimeoutValid = FALSE;
        fgMaxTimeoutValid = TRUE;
        break;
    default:
        break;
    }

    /* program beacon search timeout */
    halpmSetBeaconTimeout(prAdapter,
                          ucMinTimeoutValue,
                          u2MaxTimeoutValue,
                          fgMinTimeoutValid,
                          fgMaxTimeoutValid,
                          BEACON_TIMEOUT_COUNT_INFRA);

    prPmInfo->eBeaconTimeoutTuningStep = eStep;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmStepBeaconTimeout (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgStepUp
    )
{
    P_PM_INFO_T prPmInfo;
    BOOLEAN         fgStatus = TRUE;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;

    if (fgStepUp) {
        DBGLOG(INIT, INFO, ("Step Up (%d)\n", prPmInfo->eBeaconTimeoutTuningStep));
    } else {
        DBGLOG(INIT, INFO, ("Step Down (%d)\n", prPmInfo->eBeaconTimeoutTuningStep));
    }

    if (fgStepUp) {
        /* Stepping UP */
        switch (prPmInfo->eBeaconTimeoutTuningStep) {
        case BCN_TO_STEP_0:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_1);
            break;
        case BCN_TO_STEP_1:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_2);
            break;
        case BCN_TO_STEP_2:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_3);
            break;
        case BCN_TO_STEP_3:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_4);
            break;
        case BCN_TO_STEP_4:
            fgStatus = FALSE;
            break;
        default:
            break;
        }
    }
    else {
        /* Stepping DOWN */
        switch (prPmInfo->eBeaconTimeoutTuningStep) {
        case BCN_TO_STEP_0:
            ASSERT(0);
            break;
        case BCN_TO_STEP_1:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_0);
            fgStatus = FALSE;
            break;
        case BCN_TO_STEP_2:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_1);
            break;
        case BCN_TO_STEP_3:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_2);
            break;
        case BCN_TO_STEP_4:
            nicpmSetToBeaconTimeoutStep(prAdapter, BCN_TO_STEP_3);
            break;
        default:
            break;
        }
    }

    return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmResetBeaconContentCheck (
    IN P_ADAPTER_T prAdapter
    )
{
    halpmSetupBeaconContentCheck(prAdapter, FALSE);
    halpmSetupBeaconContentCheck(prAdapter, TRUE);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSetupMoreDataTrigger (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    )
{
    ASSERT(prAdapter);

    halpmSetupMoreDataTrigger(prAdapter, fgEnable);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicpmSetupBeaconTimeoutDetection (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    )
{
    ASSERT(prAdapter);

    halpmSetupBeaconTimeoutDetection(prAdapter, fgEnable);
}

