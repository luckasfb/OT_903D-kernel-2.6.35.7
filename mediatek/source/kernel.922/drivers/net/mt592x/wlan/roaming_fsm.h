






#ifndef _ROAMING_FSM_H
#define _ROAMING_FSM_H



/* Roaming Stable timeout interval. */
#define ROAMING_STABLE_TIMEOUT_SEC                  7 // Seconds

/* Roaming Decision retry interval if the RCPI value was invalid */
#define ROAMING_DECISION_INVALID_RCPI_TIMEOUT_MSEC  100 // Milli Seconds.

#define ROAMING_DECISION_TIMEOUT_SEC                1 // Seconds.

/* Roaming Discovery interval, SCAN result need to be updated */
#define ROAMING_DISCOVERY_TIMEOUT_SEC               5 // Seconds.




/* Partial SCAN interval for Roaming Discovery */
#define ROAMING_PARTIAL_SCAN_TIMEOUT_MSEC           20 // Milliseconds.



/* Roaming Out retry interval if previous roaming out is failure or no available candidate. */
#define ROAMING_OUT_RETRY_TIMEOUT_SEC               20 // Second.


#define ROAMING_NO_SWING_RCPI_STEP                  10 // Step = 5dBm


// Received Channel Power Indicator
#define ROAMING_WLAN_RCPI_THRESHOLD                 80 // -70dBm, RCPI = (-70 + 110) x 2.
#define ROAMING_RCPI_STEP                           10 // Step = 5dBm

#define ROAMING_WLAN_RCPI_BOUNDARY_THRESHOLD        55 // -82.5dBm
#define ROAMING_RCPI_BOUNDARY_STEP                  4 // Step = 2dBm

#define ROAMING_GSM_RCPI_HIGH_THRESHOLD             74 // -73dBm, RCPI = (-73 + 110) x 2.
#define ROAMING_GSM_RCPI_LOW_THRESHOLD              70 // -75dBm, RCPI = (-75 + 110) x 2.


typedef enum _ENUM_ROAMING_STATE_T {
    ROAMING_STATE_IDLE = 0,
    ROAMING_STATE_DECISION,
    ROAMING_STATE_DISCOVERY,
    ROAMING_STATE_ROAM,
    ROAMING_STATE_NUM
} ENUM_ROAMING_STATE_T;


typedef struct _ROAMING_INFO_T {
    ENUM_ROAMING_STATE_T    eCurrentState;

    SCAN_REQ_CONFIG_T       rScanReqConfig;

    TIMER_T                 rRoamingDecisionTimer;

    TIMER_T                 rRoamingDiscoveryTimer;

    UINT_16                 u2DiscoveryTimeoutMillisecond;

    BOOLEAN                 fgIsScanTriggered;
    BOOLEAN                 fgIsScanCompleted;
    BOOLEAN                 fgIsRCPIEventEnabled;
    BOOLEAN                 fgIsRoamingFail;

    OS_SYSTIME              rRoamingDiscoveryUpdateTime;

} ROAMING_INFO_T, *P_ROAMING_INFO_T;



#define ROAMING_STATE_TRANSITION_FLAG   fgIsTransition
#define ROAMING_NEXT_STATE_VAR          eNextState

#define ROAMING_STATE_TRANSITION(prAdapter, eFromState, eToState) \
            { roamingFsmTransAction_ ## eFromState ## _to_ ## eToState((P_ADAPTER_T)prAdapter); \
              ROAMING_NEXT_STATE_VAR = ROAMING_STATE_ ## eToState; \
              ROAMING_STATE_TRANSITION_FLAG = (BOOLEAN)TRUE; \
            }

VOID
roamingFsmInit (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
roamingFsmSteps (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_ROAMING_STATE_T eNextState
    );

WLAN_STATUS
roamingFsmRunEventStart (
    IN P_ADAPTER_T prAdapter
    );

VOID
roamingFsmRunEventRoamFail (
    IN P_ADAPTER_T prAdapter
    );

VOID
roamingFsmRunEventAbort (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
roamingFsmRunEventRCPI (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
roamingFsmRunEventDecision (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
roamingFsmRunEventDiscovery (
    IN P_ADAPTER_T prAdapter
    );

VOID
roamingDiscoveryScanDoneHandler (
    IN P_ADAPTER_T prAdapter,
    IN WLAN_STATUS rStatus
    );

VOID
roamingReTriggerEventDecision (
    IN P_ADAPTER_T prAdapter
    );

VOID
roamingReTriggerEventDiscovery (
    IN P_ADAPTER_T prAdapter
    );




#endif /* _ROAMING_FSM_H */

