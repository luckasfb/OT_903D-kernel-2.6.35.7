





#ifndef _PM_FSM_H
#define _PM_FSM_H



#define BEACON_TIMEOUT_STEP_DOWN_CHECK_INTERVAL 10000  /* in unit of ms */

#define DELAY_ENTER_PS_AFTER_CONNTECTED         5000  /* in unit of ms */

#define DEFAULT_TSF_DRIFT_WINDOW_TU             1

#define BEACON_TIMEOUT_COUNT_ADHOC              15
#define BEACON_TIMEOUT_COUNT_INFRA              10
#define BEACON_TIMEOUT_GUARD_TIME_SEC           1 /* Second */

#define BEACON_MAX_TIMEOUT_VALUE                100
#define BEACON_MIN_TIMEOUT_VALUE                5
#define BEACON_MAX_TIMEOUT_VALID                TRUE
#define BEACON_MIN_TIMEOUT_VALID                TRUE

#define NULL_TIMEOUT_COUNT                      10
#define NULL_MAX_TIMEOUT_VALUE                  10
#define NULL_MIN_TIMEOUT_VALUE                  5
#define NULL_MAX_TIMEOUT_VALID                  TRUE
#define NULL_MIN_TIMEOUT_VALID                  TRUE

#define TIMEOUT_COUNT                           10
#define MAX_TIMEOUT_VALUE                       200//80
#define MIN_TIMEOUT_VALUE                       8
#define MAX_TIMEOUT_VALID                       TRUE
#define MIN_TIMEOUT_VALID                       TRUE

#define DEFAULT_PS_MODE_SWITCH_INTERVAL         100         /* Used under PS mode, decide to switch to Active Mode. (need to > 100) in unit of ms */
#define DEFAULT_ACTIVE_MODE_SWITCH_INTERVAL     3000//500   /* Used under Active mode, decide to switch to PS Mode.(need to > 100) in unit of ms */
#define DEFAULT_FAST_ACTIVE_MEASURE_INTERVAL    3000        /* Used under Active mode, and use it as the window for counting TR packets number */

#define DEFAULT_PSP_SWITCH_TO_PS_RX_THRESHOLD   5       /* Used under Active mode, decide to switch to PS Mode. Unit: Num of Rx UC pkts */
#define DEFAULT_PSP_SWITCH_TO_ACT_RX_THRESHOLD  3       /* Used under PS mode, decide to switch to Active Mode. Unit: Num of Rx UC pkts per 100ms */
#define DEFAULT_PSP_SWITCH_TO_PS_TX_THRESHOLD   5       /* Used under Active mode, decide to switch to PS Mode. Unit: Num of TX data pkts */
#define DEFAULT_PSP_SWITCH_TO_ACT_TX_THRESHOLD  3       /* Used under PS mode, decide to switch to Active Mode. Unit: Num of TX data pkts per 100ms */
#define DEFAULT_PSP_CONSECUTE_RX_SWITCH_TO_ACT  3       /* Used under PS mode, decide to switch to Active Mode. Unit: Num of TX data pkts per 100ms */

#define DEFAULT_PSP_SWITCH_TO_PS_RX_THRESHOLD_VOIP  15  /* (< 2 calls + 5)  Unit: Num of Rx UC pkts per 100ms */
#define DEFAULT_PSP_SWITCH_TO_ACT_RX_THRESHOLD_VOIP 15  /* (> 2 calls + 10) Unit: Num of Rx UC pkts per 100ms */
#define DEFAULT_PSP_SWITCH_TO_PS_TX_THRESHOLD_VOIP  15  /* (< 2 calls + 5)  Unit: Num of TX data pkts per 100ms */
#define DEFAULT_PSP_SWITCH_TO_ACT_TX_THRESHOLD_VOIP 25  /* (> 2 calls + 15) Unit: Num of TX data pkts per 100ms */

#define DEFAULT_MULTIPLE_DTIM_COUNT             1
#define DEFAULT_MULTIPLE_TBTT_COUNT             0
#define DEFAULT_USE_AGING_QOS_NULL              1
#define DEFAULT_AGING_NULL_INTERVAL             30
#define DEFAULT_AGING_NULL_TID                  7
#define DEFAULT_UAPSD_SERVICE_PERIOD_NUM        WMM_MAX_SP_LENGTH_2//WMM_MAX_SP_LENGTH_ALL//0
#define DEFAULT_TRIGGER_THRESHOLD               (1500 / 64) // unit: 64 byte
#define DEFAULT_HW_TX_PACKET_LIFETIME           100

#define DEFAULT_CONTI_POLL_INTV_DURING_STEP_DOWN_MS     500

#define DEFAULT_CONTI_HW_TX_FAIL                5

#define DEFAULT_DTIM_INTERVAL_THRESHOLD   2000
#define DEFAULT_WORKAROUND_TBTT_COUNT    2

/*! \brief TX done handler for PS mode change callback */
typedef WLAN_STATUS (*FUNC_CALLBACK_PS)(IN P_ADAPTER_T prAdapter,
                                        IN P_SCAN_REQ_CONFIG_T prScanReqConfig,
                                        IN BOOLEAN fgForceToScanInActiveMode);


#define GET_DLVR_EN_AC_INFO_FROM_APSD_BMP(_bmfgApsdEnAc) \
        (((_bmfgApsdEnAc) >> 4) & BITS(0, 3))

#define GET_TRIG_EN_AC_INFO_FROM_APSD_BMP(_bmfgApsdEnAc) \
        (((_bmfgApsdEnAc) >> 0) & BITS(0, 3))

/* Definition for service period set */
#define SP_BEACON                       BIT(0)
#define SP_BMC                          BIT(1)
#define SP_QOS_CFPOLL                   BIT(2)
#define SP_PS_POLL                      BIT(3)
#define SP_APSD                         BIT(4)
#define SP_ALL                          BITS(0, 4)

#define NUM_OF_MAX_LP_INSTRUCTIONS          8


typedef struct _PM_CONN_SETUP_INFO_T {
    UINT_16     u2AID;
    UINT_16     u2BcnIntv;
    UINT_8      ucDtimPeriod;
    UINT_16     u2AtimWindow;
    BOOLEAN     fgIsUapsdConn;
} PM_CONN_SETUP_INFO_T, *P_PM_CONN_SETUP_INFO_T;

typedef struct _PM_PROFILE_SETUP_INFO_T {
    /* Profile setup */
    UINT_8      ucMultiDtimWake;    /* Wakeup timer for multiple DTIM period */
    UINT_8      ucMultiTbttWake;    /* Wakeup timer for multiple TBTT period */
    UINT_8      ucQoSNullTid;       /* 0~15, includes QoS Null trigger frame and aging NULL */
    UINT_8      bmfgApsdEnAc;       /* b0~3: trigger-en AC0~3. b4~7: delivery-en AC0~3 */
    UINT_8      ucUapsdSp;          /* Number of triggered packets in UAPSD */
    UINT_8      ucTrgThr;           /* Trigger threshold. (unit: 64 byte) */
    UINT_8      ucAgeNullPeriod;    /* (QoS-) Null frame interval to keep connection. (unit: second) */
    BOOLEAN     fgUseAgeQoSNull;    /* Whether to use QoS NULL for aging prevention (1: QoS-NULL, 0: NULL) */
    BOOLEAN     fgAdhocPsCapability;/* Whether to enable IBSS power saving function */
    UINT_16     u2HwTxLifeTime;     /* Packet life time for the HW transmitted packets */

    UINT_16     u2TxThrSwToAct;     /* Unit: Num of TX pkts per 100 ms */
    UINT_16     u2TxThrSwToPs;      /* Unit: Num of TX pkts per 100 ms */
    UINT_16     u2RxThrSwToAct;     /* Unit: Num of Rx UC pkts per 100 ms */
    UINT_16     u2RxThrSwToPs;      /* Unit: Num of Rx UC pkts per 100 ms */
    UINT_16     u2PsModeSwIntv;     /* Interval used under PSP-fast, checking if to enter active mode. Unit: ms */
    UINT_16     u2ActModeSwIntv;    /* Interval used under PSP-fast, checking if to enter PS mode. Unit: ms */

    BOOLEAN     fgBcPtrnSrchEn;     /* BC pattern search function. */
    BOOLEAN     fgMcPtrnSrchEn;     /* MC pattern search function. */
    BOOLEAN     fgUcPtrnSrchEn;     /* UC pattern search function. */

    BOOLEAN     fgBcPtrnMatchRcv;   /* Filter for the matched BC packet. TRUE: black list, FALSE: white list */
    BOOLEAN     fgMcPtrnMatchRcv;   /* Filter for the matched MC packet. TRUE: black list, FALSE: white list */
    BOOLEAN     fgUcPtrnMatchRcv;   /* Filter for the matched UC packet. TRUE: black list, FALSE: white list */

    UINT_16     u2DtimIntvThr;  /* Dtim interval threshold in unit of TU */
    UINT_8      ucWorkaroundTbttCount;  /* Dtim interval threshold in unit of TU */
} PM_PROFILE_SETUP_INFO_T, *P_PM_PROFILE_SETUP_INFO_T;


typedef enum _ENUM_ACPI_STATE_T {
    ACPI_STATE_D0 = 0,
    ACPI_STATE_D1,
    ACPI_STATE_D2,
    ACPI_STATE_D3
} ENUM_ACPI_STATE_T;

typedef enum _ENUM_NULL_STATE_T {
    NULL_STATE_NONE = 0,
    NULL_STATE_SUCCESS,
    NULL_STATE_FAIL
} ENUM_NULL_STATE_T;

typedef enum _ENUM_PM_STATE_T {
    PM_STATE_ACTIVE_MODE = 0,
    PM_STATE_WAIT_POWER_SAVE_READY,
    PM_STATE_WAIT_ACTIVE_READY,
    PM_STATE_POWER_SAVE_AWAKE,
    PM_STATE_POWER_SAVE_DOZE,
    PM_STATE_NUM
} ENUM_PM_STATE_T;

typedef enum _ENUM_PM_BCN_TO_STEPS_T {
    BCN_TO_STEP_0 = 0,
    BCN_TO_STEP_1,
    BCN_TO_STEP_2,
    BCN_TO_STEP_3,
    BCN_TO_STEP_4,
    BCN_TO_STEP_NUM
} ENUM_PM_BCN_TO_STEPS_T;


typedef struct _PM_SLOW_CLOCK_COUNT_T {
    UINT_8 uc32kSlowCount;
    UINT_8 uc32kSlowCount_10;
    UINT_8 uc32kSlowCount_100;
    UINT_8 uc32kSlowCount_1000;
} PM_SLOW_CLOCK_COUNT_T, *P_PM_SLOW_CLOCK_COUNT_T;

typedef struct _PM_FAST_SWITCH_PARAM_T {
    UINT_32 u4TimeSlot;
    UINT_32 u4AccumRxUcPktNum;
    UINT_32 u4AccumTxDataPktNum;
    OS_SYSTIME rLastSwitchTime; /* last time switch to active mode */
    OS_SYSTIME rLastRxUcDataTime;
    UINT_32 u4ConsecutiveRxCount; /* consecutive Rx in STA wakeup intervals */
} PM_FAST_SWITCH_PARAM_T, *P_PM_FAST_SWITCH_PARAM_T;

typedef struct _PM_INFO_T {
    ENUM_PM_STATE_T    eCurrentState;

    FUNC_CALLBACK_PS pfnPsIndication;

    ENUM_PM_BCN_TO_STEPS_T  eBeaconTimeoutTuningStep;

    BOOLEAN         fgContiPollIsStepUp;

    TIMER_T         rContiHwTxFailCheckTimer;

    TIMER_T         rContiPollingCheckTimer;

    TIMER_T         rBeaconTimeoutHandlingTimer;

    TIMER_T         rWaitBeaconWatchdogTimer;

    TIMER_T         rPmOnConnectDeferredTimer;   /* timer for able to enter PS after connected */

    TIMER_T         rPmSwitchPsModeTimer;       /* timer used in ENUM_PSP_SWITCH to switch PS mode */

    ENUM_NULL_STATE_T rNullState;

    PM_PROFILE_SETUP_INFO_T rPmProfSetupInfo;

    ENUM_POWER_SAVE_PROFILE_T eCurPsProf;

    PM_SLOW_CLOCK_COUNT_T rSlowClkCnt;

    /* phase 2/ 3 guard time (unit: us) */
    UINT_16 u2GuardTimePhase2;
    UINT_16 u2GuardTimePhase3;

    BOOLEAN fgIpAddressIsValid;
    UINT_32 u4IpAddress;
    UINT_32 u4BcPatternXIsValid;
    UINT_32 u4McPatternXIsValid;
    UINT_32 u4UcPatternXIsValid;

#if CFG_LP_IOT // used only for emulation/ IOT field try
    TIMER_T         rPmReadBeaconTimeoutTimer;       /* timer used in ENUM_PSP_SWITCH to switch PS mode */
#endif

    PM_FAST_SWITCH_PARAM_T  rPmFastSwitchParam;

    UINT_8          u4ContiHwTxFail;
    BOOLEAN         fgIsContinousPollingEnabled;
    BOOLEAN         fgIsContinousPollingSuspended;
    BOOLEAN         fgIsVoipPollingEnabled;
    BOOLEAN         fgIsUnderPowerSave;

    BOOLEAN         fgFastSwitchInProgress;

    BOOLEAN         fgPwrMode_PS; /* target/ current PS bit setting */

    // For WMM-PS only
    UINT_8          ucWmmPsEnterPsAtOnce;
    UINT_8          ucWmmPsDisableUcPoll;
    UINT_8          ucWmmPsConnWithTrig;

    BOOLEAN         fgIsBeaconTimeoutDetectionDisabled;
    BOOLEAN         fgDtimTrigEn;

#if CFG_IBSS_POWER_SAVE
    BOOLEAN     fgIsIbssPsTxEnabled;
    BOOLEAN     fgIsOutsideAtimWondow;
#endif /* CFG_IBSS_POWER_SAVE */

    /* Variables for the "extra" LP instructions.
     * Modules should maintains the number and instruction content validation when progromming it.
     */
    UINT_8      ucNumOfInstSleep;
    UINT_8      ucNumOfInstAwake;
    UINT_8      ucNumOfInstOn;
    UINT_32     au4LowPowerInst_sleep[NUM_OF_MAX_LP_INSTRUCTIONS];
    UINT_32     au4LowPowerInst_awake[NUM_OF_MAX_LP_INSTRUCTIONS];
    UINT_32     au4LowPowerInst_on[NUM_OF_MAX_LP_INSTRUCTIONS];

    BOOLEAN     fgIsWatchDogTriggered;
    BOOLEAN     fgIsBcnTimeoutStep4;
} PM_INFO_T, *P_PM_INFO_T;


#define PM_NEXT_STATE_VAR          eNextState

#define PM_STATE_TRANSITION(_prAdapter, rFromState, rToState) \
                { pmFsmTransAction_ ## rFromState ## _to_ ## rToState (_prAdapter); \
                  PM_NEXT_STATE_VAR = PM_STATE_ ## rToState; \
                  DBGLOG(LP, TRACE, (("PM STATE TRANSITION: [%s] --> [%s]\n"), \
                                   #rFromState, #rToState)); \
                  pmFsmStep(_prAdapter, PM_NEXT_STATE_VAR); \
                }

#define PM_STATE_TRANSITION1(_prAdapter, rFromState, rToState, rPostFix) \
                { \
                  WLAN_STATUS u4Status; \
                  status = pmFsmTransAction_ ## rFromState ## _to_ ## rToState ## _ ## rPostFix(_prAdapter); \
                  PM_NEXT_STATE_VAR = PM_STATE_ ## rToState; \
                  DBGLOG(LP, TRACE, (("SCAN STATE TRANSITION: [%s] --> [%s]\n"), \
                                   #rFromState, #rToState)); \
                  pmFsmStep(_prAdapter, PM_NEXT_STATE_VAR); \
                }

#define PM_ENABLE_BG_SSID_SCAN(_prAdapter, _fgEnableInt) \
    { \
        nicHwScanEnable(_prAdapter, ENUM_HW_SCAN_BG_SSID_SCAN); \
        nicpmEnterLowPower(_prAdapter, _fgEnableInt); \
    }

/* NOTE2: Scan done event reset is done in scanFsmTransAction_IDLE_to_ACTIVE() */
#if CFG_WORKAROUND_BG_SSID_SCAN_DONE
#define PM_DISABLE_BG_SSID_SCAN(_prAdapter) \
    { \
        nicpmLeaveLowPower(_prAdapter); \
        nicHwScanDisable(_prAdapter, ENUM_HW_SCAN_BG_SSID_SCAN); \
        /*NIC_VALIDATE_BG_SSID_SCAN_DONE_INT_EVENT(_prAdapter);*/ \
        NIC_DEL_INT_SCAN_DONE_EVENT(_prAdapter); \
    }
#else
#define PM_DISABLE_BG_SSID_SCAN(_prAdapter) \
    { \
        nicpmLeaveLowPower(_prAdapter); \
        nicHwScanDisable(_prAdapter, ENUM_HW_SCAN_BG_SSID_SCAN); \
    }
#endif

#define PM_BLOCK_TO_ENTER_LOW_POWER(_prAdapter) \
    { \
        pmFsmRunEventEnterAwakeState(_prAdapter); \
    }

#define PM_ABLE_TO_ENTER_LOW_POWER(_prAdapter, _fgEnableInt) \
    { \
        pmFsmRunEventEnterDozeState(_prAdapter, _fgEnableInt); \
    }

#define PM_IS_AC_QUEUE_DELIVERY_AND_TRIGGER_ENABLED(_prAdapter, eQueNum) \
       ((_prAdapter->rPmInfo.rPmProfSetupInfo.bmfgApsdEnAc & ((BIT(0) | BIT(4)) << eQueNum)) == \
       ((BIT(0) | BIT(4)) << eQueNum))



#define PM_IS_ADHOC_POWER_SAVE_CAPABLE(_prAdapter) \
       (_prAdapter->rPmInfo.rPmProfSetupInfo.fgAdhocPsCapability)

#define PM_CHECK_IF_OUTSIDE_ATIM_WINDOW(_prAdapter) \
        (_prAdapter->rPmInfo.fgIsOutsideAtimWondow)

#define PM_SET_FLAG_OUTSIDE_ATIM_WINDOW(_prAdapter) \
        (_prAdapter->rPmInfo.fgIsOutsideAtimWondow = TRUE)

#define PM_SET_FLAG_UNDER_ATIM_WINDOW(_prAdapter) \
        (_prAdapter->rPmInfo.fgIsOutsideAtimWondow = FALSE)


#define PM_IS_UNDER_POWER_SAVE_MODE(_prAdapter) \
        (_prAdapter->rPmInfo.fgIsUnderPowerSave)

#define PM_IS_UNDER_IBSS_POWER_SAVE_MODE(_prAdapter) \
        ((_prAdapter->eCurrentOPMode == OP_MODE_IBSS) && \
         (_prAdapter->rPmInfo.fgIsUnderPowerSave))

#define PM_IS_USING_PS_PROFILE_CONTINUOUS_ACTIVE(_prAdapter) \
        (_prAdapter->rPmInfo.eCurPsProf == ENUM_PSP_CONTINUOUS_ACTIVE)

#define PM_IS_VOIP_POLLING_ENABLED(_prAdapter) \
        (_prAdapter->rPmInfo.fgIsVoipPollingEnabled)


BOOLEAN
pmFsmInit (
    P_ADAPTER_T prAdapter
    );

VOID
pmFsmUnInit (
    P_ADAPTER_T prAdapter
    );

WLAN_STATUS
pmFsmRunEventEnterPowerSaveModeReq (
    IN P_ADAPTER_T  prAdapter,
    IN FUNC_CALLBACK_PS pfnPsIndication
    );

WLAN_STATUS
pmFsmRunEventOnConnectDeferredTask (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
pmFsmRunEventEnterPowerSaveModeDecision (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
pmFsmRunEventEnterActiveModeReq (
    IN P_ADAPTER_T  prAdapter,
    IN FUNC_CALLBACK_PS pvPSCallBack
    );

WLAN_STATUS
pmFsmRunEventPowerModeIndication (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN fgPowerSaveMode
    );

VOID
pmFsmRunEventEnterAwakeState (
    IN P_ADAPTER_T prAdapter
    );

VOID
pmFsmRunEventEnterDozeState (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgEnableGlobalInt
    );

WLAN_STATUS
pmSendNullFrameForConnectionDiagnostic (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_TX_DONE_HANDLER  prFuncCallback
    );

WLAN_STATUS
pmSendNullFrame (
    IN P_ADAPTER_T          prAdapter,
    IN UINT_8               ucTid,
    IN BOOLEAN              fgPsBit,
    IN BOOLEAN              fgUseBasicRate,
    IN PFN_TX_DONE_HANDLER  prFuncCallback
    );

WLAN_STATUS
pmFsmRunEventSetPowerSaveProfile (
    IN P_ADAPTER_T  prAdapter,
    IN ENUM_POWER_SAVE_PROFILE_T ePsProf
    );

WLAN_STATUS
pmFsmRunEventSetPowerSaveProfileWatchDog (
    IN P_ADAPTER_T  prAdapter,
    IN ENUM_POWER_SAVE_PROFILE_T ePsProf
    );

VOID
pmDisableBeaconTimeoutDetectionFunc (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgDisable
    );

VOID
pmContinuousPollingFreqStepUp (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgStepUp
    );

VOID
pmSetIpAddress (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_8      aucIpAddress
    );

WLAN_STATUS
pmFsmRunEventOnConnect (
    IN P_ADAPTER_T      prAdapter,
    IN  UINT_16         u2AID,
    IN  UINT_16         u2BcnIntv,
    IN  UINT_8          ucDtimPeriod,
    IN  UINT_16         u2AtimWindow
    );

WLAN_STATUS
pmFsmRunEventAbort (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
pmFsmRunEventOnCreateIbss (
    IN P_ADAPTER_T      prAdapter,
    IN  UINT_16         u2BcnIntv
    );

VOID
pmSetAdhocStaAwake (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    );

VOID
pmSetAdhocAllStaAwake (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    );

VOID
pmEnableIbssPsTx (
    IN  P_ADAPTER_T     prAdapter
    );

VOID
pmDisableIbssPsTx (
    IN  P_ADAPTER_T     prAdapter
    );

WLAN_STATUS
pmSendAtimFrame (
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucDestAddr,
    IN PFN_TX_DONE_HANDLER  pfTxDoneHandler
    );

BOOLEAN
pmClearAllStationAwakeState (
    IN P_ADAPTER_T      prAdapter
    );

BOOLEAN
pmGetBufferedTxInfoInStaRec (
    IN      P_ADAPTER_T     prAdapter,
    IN OUT  PUINT_8         prStaAddrArray,
    IN      UINT_32         u4InBufferLen,
    OUT     PUINT_32        pu4StaNum
    );

VOID
pmSetAcpiPowerD0 (
    IN P_ADAPTER_T prAdapter
    );

VOID
pmSetAcpiPowerD3 (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_LP_IOT // used only for emulation/ IOT field try
BOOLEAN
pmSetIotBeaconTimeoutCheck (
    IN P_ADAPTER_T prAdapter
    );

VOID
pmFsmRunEventReadBeaconTimeoutCount (
    IN P_ADAPTER_T  prAdapter
    );

#endif

VOID
pmEnableVoipPollingFunc (
    IN P_ADAPTER_T  prAdapter
    );

VOID
pmDisableVoipPollingFunc (
    IN P_ADAPTER_T  prAdapter
    );

VOID
pmIndicateUcDataFrameReceived (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN fgMoreData
    );

VOID
pmIndicateDataFrameTransmitted (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_32      u4DataPacketNum
    );

BOOLEAN
pmIsAbleToEnterPowerSaveMode (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
pmBeaconTimeoutHandler (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_TX_DONE_HANDLER  prFuncCallback
    );

VOID
pmEnableContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    );

VOID
pmDisableContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    );

VOID
pmResumeContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    );

VOID
pmSuspendContinuousPollingFunc (
    IN P_ADAPTER_T  prAdapter
    );

VOID
pmHandleContiPollingCheck (
    IN P_ADAPTER_T prAdapter
    );

#endif /* _PM_FSM_H */

