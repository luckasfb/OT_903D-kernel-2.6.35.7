





#ifndef _ARB_FSM_H
#define _ARB_FSM_H


#define CONNECTION_LOST_INDICATION_TIMEOUT_SEC              9 // Seconds

#define SCAN_REQUEST_TIMEOUT_MSEC                           20000 // msec

typedef enum _ENUM_ARB_STATE_T {
    ARB_STATE_POWER_OFF = 0,
    ARB_STATE_RESET,
    ARB_STATE_RF_TEST,
    ARB_STATE_STANDBY,
    ARB_STATE_IDLE,
    ARB_STATE_SEARCH,
    ARB_STATE_BG_SSID_SCAN,
    ARB_STATE_JOIN,
    ARB_STATE_IBSS_ALONE,
    ARB_STATE_IBSS_MERGE,
    ARB_STATE_NORMAL_TR,
    ARB_STATE_SCAN,
    ARB_STATE_DEDICATED_MEASUREMEMT,
    ARB_STATE_NUM
} ENUM_ARB_STATE_T;

typedef struct _ARB_INFO_T {
    ENUM_ARB_STATE_T    ePreviousState;
    ENUM_ARB_STATE_T    eCurrentState;

    BOOLEAN             fgTestMode;
    BOOLEAN             fgTryFullScan;
    BOOLEAN             fgIsDiagnosingConnection;

    P_BSS_DESC_T        prTargetBssDesc;

    TIMER_T             rIbssAloneTimer;

    TIMER_T             rIndicationOfDisconnectTimer;

    TIMER_T             rProtectionTimerForAdHoc;

    /* HW Error flag */
    UINT_32             u4Flag;

    /* Counter for Power Control Semaphore */
    UINT_32             u4PwrCtrlBlockCnt;

    BOOLEAN             fgIsUnderActiveModeBeforeScan;

    OS_SYSTIME          rLastScanRequestTime;

} ARB_INFO_T, *P_ARB_INFO_T;




#define ARB_STATE_TRANSITION_FLAG   fgIsTransition
#define ARB_NEXT_STATE_VAR          eNextState

#define ARB_STATE_TRANSITION(prAdapter, rFromState, rToState) \
            { arbFsmTransAction_ ## rFromState ## _to_ ## rToState((P_ADAPTER_T)prAdapter); \
              ARB_NEXT_STATE_VAR = ARB_STATE_ ## rToState; \
              ARB_STATE_TRANSITION_FLAG = (BOOLEAN)TRUE; \
            }

#if DBG
#define ARB_INIT_TIMER(_adapter_p, _timer, _callbackFunc, _fgNeedHwAccess) \
        { \
            kalMemCopy((_timer).aucDbgString, #_callbackFunc, \
                (sizeof((_timer).aucDbgString) < sizeof(#_callbackFunc) ? \
                sizeof((_timer).aucDbgString) : sizeof(#_callbackFunc))); \
            (_timer).aucDbgString[sizeof((_timer).aucDbgString) - 1] = 0; \
            timerInitTimer(_adapter_p, \
                           &(_timer), \
                           (PFN_MGMT_TIMEOUT_FUNC)_callbackFunc, \
                           (UINT_32)(_adapter_p), \
                           (BOOLEAN)(_fgNeedHwAccess)); \
        }
#else
#define ARB_INIT_TIMER(_adapter_p, _timer, _callbackFunc, _fgNeedHwAccess) \
        { \
            timerInitTimer(_adapter_p, \
                           &(_timer), \
                           (PFN_MGMT_TIMEOUT_FUNC)_callbackFunc, \
                           (UINT_32)(_adapter_p), \
                           (BOOLEAN)(_fgNeedHwAccess)); \
        }
#endif

#define ARB_SET_TIMER(_adapter_p, _timer, _timeout_ms) \
        { \
            timerStartTimer(_adapter_p, \
                            &(_timer), \
                            _timeout_ms); \
        }

#define ARB_CANCEL_TIMER(_adapter_p, _timer) \
        { \
            timerStopTimer(_adapter_p, &(_timer)); \
        }

#define ARB_INDICATE_PS_STATUS(_adapter_p, _fgPsState) \
        { \
            pmFsmRunEventPowerModeIndication(_adapter_p, _fgPsState); \
        }

#define ARB_INDICATE_UC_DATA_FRAME_RECEICED(_adapter_p, _fgMoreDataBit) \
        { \
            if ((_adapter_p->rArbInfo.eCurrentState == ARB_STATE_NORMAL_TR) && \
                (!_adapter_p->rScanInfo.fgIsScanReqProceeding)) { \
                pmIndicateUcDataFrameReceived(_adapter_p, _fgMoreDataBit); \
            } \
        }

#define ARB_INDICATE_DATA_FRAME_TRANSMITTED(_adapter_p, _u4DataPacketNum) \
        { \
            if ((_adapter_p->rArbInfo.eCurrentState == ARB_STATE_NORMAL_TR) && \
                (!_adapter_p->rScanInfo.fgIsScanReqProceeding)) { \
                pmIndicateDataFrameTransmitted(_adapter_p, _u4DataPacketNum); \
            } \
        }

#define ARB_FSM_SAVE_LAST_RX_UC_DATA_FRAME_SYSTIME(_prAdapter, _rCurSysTime) \
                    _prAdapter->rLastRxUcDataSysTime = _rCurSysTime;

#define ARB_FSM_GET_LAST_RX_UC_DATA_FRAME_SYSTIME(_prAdapter) \
                    _prAdapter->rLastRxUcDataSysTime

#define BEACON_TIMEOUT_GUARD_TIME_FROM_UC_DATA_MSEC         100 // milliseconds

#define WAIT_BEACON_ON_CONNECTED_MSEC                       2000 // milliseconds

#define CONTI_POLLING_SUSPEND_CHECK_MSEC                    500 // milliseconds

#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS
#define RFTEST_PKT_TX_INTERVAL                                          1
#endif

#define ARB_ACQUIRE_POWER_CONTROL_FROM_PM_IN_OS_TX_PATH(_prAdapter) \
    { \
        P_ARB_INFO_T _prArbInfo = &_prAdapter->rArbInfo; \
        switch (_prArbInfo->eCurrentState) { \
        case ARB_STATE_BG_SSID_SCAN: \
            PM_DISABLE_BG_SSID_SCAN(_prAdapter); \
            break; \
        case ARB_STATE_IDLE: \
            nicpmPowerOn(_prAdapter); \
            break; \
        default: \
            break; \
        } \
        /* Increase Block to Enter Low Power Semaphore count */ \
        _prArbInfo->u4PwrCtrlBlockCnt++; \
    }

#define ARB_TEST_AND_GET_POWER_CONTROL_FROM_PM_IN_TX_PATH(_prAdapter) \
    { \
        P_ARB_INFO_T _prArbInfo = &_prAdapter->rArbInfo; \
        switch (_prArbInfo->eCurrentState) { \
        /* OS TX packets will be dropped directly if it is not under NORMAL_TR state */ \
        /* And it is already awake to MGMT TX packets */ \
        case ARB_STATE_NORMAL_TR: \
            PM_BLOCK_TO_ENTER_LOW_POWER(_prAdapter); \
            break; \
        default: \
            break; \
        } \
    }


#define ARB_ACQUIRE_POWER_CONTROL_FROM_PM(_prAdapter) \
    { \
        P_ARB_INFO_T _prArbInfo = &_prAdapter->rArbInfo; \
        switch (_prArbInfo->eCurrentState) { \
        case ARB_STATE_NORMAL_TR: \
            PM_BLOCK_TO_ENTER_LOW_POWER(_prAdapter); \
            break; \
        case ARB_STATE_BG_SSID_SCAN: \
            PM_DISABLE_BG_SSID_SCAN(_prAdapter); \
            break; \
        case ARB_STATE_IDLE: \
            nicpmPowerOn(_prAdapter); \
            break; \
        default: \
            break; \
        } \
        /* Increase Block to Enter Low Power Semaphore count */ \
        _prArbInfo->u4PwrCtrlBlockCnt++; \
    }

#define ARB_ACQUIRE_POWER_CONTROL_FROM_PM_IN_IST(_prAdapter) \
    { \
        P_ARB_INFO_T _prArbInfo = &_prAdapter->rArbInfo; \
        switch (_prArbInfo->eCurrentState) { \
        case ARB_STATE_NORMAL_TR: \
            PM_BLOCK_TO_ENTER_LOW_POWER(_prAdapter); \
            break; \
        case ARB_STATE_BG_SSID_SCAN: \
            PM_DISABLE_BG_SSID_SCAN(_prAdapter); \
            break; \
        case ARB_STATE_IDLE: \
            nicpmPowerOn(_prAdapter); \
            break; \
        default: \
            break; \
        } \
        /* Increase Block to Enter Low Power Semaphore count */ \
        _prArbInfo->u4PwrCtrlBlockCnt++; \
    }

#define ARB_RECLAIM_POWER_CONTROL_TO_PM(_prAdapter) \
    { \
        P_ARB_INFO_T _prArbInfo = &_prAdapter->rArbInfo; \
        ASSERT(_prArbInfo->u4PwrCtrlBlockCnt != 0); \
        /* Decrease Block to Enter Low Power Semaphore count */ \
        _prArbInfo->u4PwrCtrlBlockCnt--; \
        switch (_prArbInfo->eCurrentState) { \
        case ARB_STATE_NORMAL_TR: \
            /* Only performs subsequent procedure if \
               LP semaphore count equals to 0 */ \
            if (_prArbInfo->u4PwrCtrlBlockCnt == 0) { \
                PM_ABLE_TO_ENTER_LOW_POWER(_prAdapter, FALSE); \
            } \
            break; \
        case ARB_STATE_BG_SSID_SCAN: \
            PM_ENABLE_BG_SSID_SCAN(_prAdapter, FALSE); \
            break; \
        case ARB_STATE_IDLE: \
            nicpmPowerOff(_prAdapter); \
            break; \
        default: \
            break; \
        } \
    }

#define ARB_RECLAIM_POWER_CONTROL_TO_PM_IN_IST(_prAdapter) \
    { \
        P_ARB_INFO_T _prArbInfo = &_prAdapter->rArbInfo; \
        ASSERT(_prArbInfo->u4PwrCtrlBlockCnt != 0); \
        /* Decrease Block to Enter Low Power Semaphore count */ \
        _prArbInfo->u4PwrCtrlBlockCnt--; \
        switch (_prArbInfo->eCurrentState) { \
        case ARB_STATE_NORMAL_TR: \
            /* Only performs subsequent procedure if \
               LP semaphore count equals to 0 */ \
            if (_prArbInfo->u4PwrCtrlBlockCnt == 0) { \
                PM_ABLE_TO_ENTER_LOW_POWER(_prAdapter, TRUE); \
            } \
            break; \
        case ARB_STATE_BG_SSID_SCAN: \
            PM_ENABLE_BG_SSID_SCAN(_prAdapter, TRUE); \
            break; \
        case ARB_STATE_IDLE: \
            nicpmPowerOff(_prAdapter); \
            break; \
        default: \
            break; \
        } \
    }

#define IS_ARB_IN_RFTEST_STATE(_prAdapter) \
            ((BOOLEAN)(_prAdapter->rArbInfo.eCurrentState == ARB_STATE_RF_TEST))

#define IS_ARB_IN_BG_SSID_SCAN_STATE(_prAdapter) \
            ((BOOLEAN)(_prAdapter->rArbInfo.eCurrentState == ARB_STATE_BG_SSID_SCAN))


#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS
WLAN_STATUS
arbFsmInit(
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventPacketTxIntervalTimeOut(
    IN P_ADAPTER_T prAdapter
    );

#else
WLAN_STATUS
arbFsmInit(
    IN P_ADAPTER_T prAdapter,
    IN P_REG_INFO_T prRegInfo
    );
#endif
VOID
arbFsmUnInit (
    P_ADAPTER_T prAdapter
    );

VOID
arbFsmSteps (
    P_ADAPTER_T prAdapter,
    ENUM_ARB_STATE_T rNextState
    );

BOOLEAN
arbFsmRunEventIST (
    IN P_ADAPTER_T      prAdapter
    );

VOID
arbFsmRunEventRootTimerHandler (
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventReset (
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventAbort (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsDelayMediaStateIndication
    );

WLAN_STATUS
arbFsmRunEventTxMsduFromOs (
    IN P_ADAPTER_T      prAdapter,
    IN P_PACKET_INFO_T  prPacketInfo
    );

WLAN_STATUS
arbFsmRunEventTxMmpdu (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    );

VOID
arbFsmRunEventConnectionTest (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventConnectionDiagnosis (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rStatus
    );

VOID
arbFsmRunEventJoinRxClassError (
    IN P_ADAPTER_T prAdapter,
    IN P_STA_RECORD_T prStaRec
    );

VOID
arbFsmRunEventJoinRxAuthAssoc (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    );

VOID
arbFsmRunEventJoinRxTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventJoinTxTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventJoinTxDone (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN WLAN_STATUS      rTxDoneStatus
    );

VOID
arbFsmRunEventJoinFailureTimeOut (
    IN P_ADAPTER_T      prAdapter
    );

VOID
arbFsmRunEventJoinDisassoc (
    IN P_ADAPTER_T prAdapter
    );


WLAN_STATUS
arbFsmRunEventSecKeyInstalled(
    IN P_ADAPTER_T          prAdapter,
    IN P_PARAM_KEY_T        prKey
    );

VOID
arbFsmRunEventSecDeauthDone(
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN WLAN_STATUS      rTxDoneStatus
    );

VOID
arbFsmRunEventConnectionEndOfCounterMeasure(
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventConnectionStartCounterMeasure(
    IN P_ADAPTER_T  prAdapter
    );

BOOLEAN
arbFsmRunEventSecTxFlowControl(
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgClearSignal
    );

VOID
arbFsmRunEventSecTxFIFOEmpty(
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventSecCancelEAPoLTimer(
    IN P_ADAPTER_T          prAdapter
    );

VOID
arbFsmRunEventSecIndicatePmkidCand(
    IN P_ADAPTER_T          prAdapter
    );

VOID
arbFsmRunEventSecCounterMeasureDone(
    IN P_ADAPTER_T          prAdapter
    );

WLAN_STATUS
arbFsmRunEventScanRequest (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_REQ_CONFIG_T prScanReqConfig,
    IN BOOLEAN fgForceToScanInActiveMode
    );

WLAN_STATUS
arbFsmRunEventScanSetPsVoipTrap (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_REQ_CONFIG_T prScanReqConfig,
    IN BOOLEAN fgForceToScanInActiveMode
    );

VOID
arbFsmRunEventScanPartialScanTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventScanCheckForHangTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventScanDone (
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventScanPsVoipPartialScanStop (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventOnConnectDeferredTimer (
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventTimerPowerSaveSwitch (
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventTimeupVoipInterval (
    IN P_ADAPTER_T  prAdapter
    );

VOID
arbFsmRunEventIbssAloneTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventIbssProtectionTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventIbssMerge (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    );

VOID
arbFsmRunEventProcessDeauth (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    );

VOID
arbFsmRunEventProcessDisassoc (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    );

VOID
arbFsmRunEventProcessProbeReq (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    );

#if CFG_IBSS_POWER_SAVE
VOID
arbSetAdhocStaAwake (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    );

VOID
arbSetAdhocAllStaAwake (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    );
#endif /* CFG_IBSS_POWER_SAVE */

VOID
arbFsmRunEventRoamingRCPI (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventRoamingDecision (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventRoamingDiscovery (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventIndicationOfDisconnectTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventRftestEnterTestMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PUINT_8           pucEepromBuf,
    IN  UINT_32           u4EepromBufByteLen
    );

VOID
arbFsmRunEventRftestAbortTestMode (
    IN  P_ADAPTER_T       prAdapter
    );

WLAN_STATUS
arbFsmRunEventRftestSetAutoTest (
    IN P_ADAPTER_T                   prAdapter,
    IN P_PARAM_MTK_WIFI_TEST_STRUC_T prRfATCfg
    );

WLAN_STATUS
arbFsmRunEventRftestQueryAutoTest (
    IN P_ADAPTER_T                   prAdapter,
    IN P_PARAM_MTK_WIFI_TEST_STRUC_T prRfATStatus
    );

VOID
arbFsmRunEventAbortOnWaitBeaconTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventBeaconTimeoutStepDownHandling (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventContiPollingCheckHandling (
    IN P_ADAPTER_T prAdapter
    );

VOID
arbFsmRunEventContiHwTxFailCheckHandling (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_PEEK_RCPI_VALUE_PERIOD_SEC
VOID
arbFsmRunEventRcpiDiagnostic (
    IN P_ADAPTER_T prAdapter
    );
#endif /* CFG_PEEK_RCPI_VALUE_PERIOD_SEC */


#endif /* _ARB_FSM_H */

