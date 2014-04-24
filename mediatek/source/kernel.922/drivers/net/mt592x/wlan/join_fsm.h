






#ifndef _JOIN_FSM_H
#define _JOIN_FSM_H


#include "gl_typedef.h"

#define AUTH_TYPE_OPEN_SYSTEM                       BIT(AUTH_ALGORITHM_NUM_OPEN_SYSTEM)
#define AUTH_TYPE_SHARED_KEY                        BIT(AUTH_ALGORITHM_NUM_SHARED_KEY)
#define AUTH_TYPE_FAST_BSS_TRANSITION               BIT(AUTH_ALGORITHM_NUM_FAST_BSS_TRANSITION)


#define TX_AUTH_ASSOCI_RETRY_LIMIT                  2

#define TX_AUTH_ASSOCI_RETRY_LIMIT_FOR_ROAMING      1

/* Retry interval for retransmiting authentication-request MMPDU. */
#define TX_AUTHENTICATION_RETRY_TIMEOUT_TU          100 // TU.

/* Retry interval for retransmiting association-request MMPDU. */
#define TX_ASSOCIATION_RETRY_TIMEOUT_TU             100 // TU.

/* Wait for a response to a transmitted authentication-request MMPDU. */
#define DOT11_AUTHENTICATION_RESPONSE_TIMEOUT_TU    512 // TU.

/* Wait for a response to a transmitted association-request MMPDU. */
#define DOT11_ASSOCIATION_RESPONSE_TIMEOUT_TU       512 // TU.

/* The maximum time to wait for JOIN process complete. */
#define JOIN_FAILURE_TIMEOUT_BEACON_INTERVAL        20 // Beacon Interval, 20 * 100TU = 2 sec.

/* Retry interval for next JOIN request. */
#define JOIN_RETRY_INTERVAL_SEC                     5 // Seconds

/* Maximum Retry Count for accept a JOIN request. */
#define JOIN_MAX_RETRY_FAILURE_COUNT                4 // Times

typedef enum _ENUM_JOIN_STATE_T {
    JOIN_STATE_IDLE = 0,
    JOIN_STATE_INIT,
    JOIN_STATE_SEND_AUTH1,
    JOIN_STATE_WAIT_AUTH2,
    JOIN_STATE_SEND_AUTH3,
    JOIN_STATE_WAIT_AUTH4,
    JOIN_STATE_SEND_ASSOC1,
    JOIN_STATE_WAIT_ASSOC2,
    JOIN_STATE_COMPLETE,
    JOIN_STATE_NUM
} ENUM_JOIN_STATE_T;

typedef struct _JOIN_INFO_T {
    ENUM_JOIN_STATE_T   eCurrentState;

    /* This is the pointer to the BSS description of target BSS chosen from SCAN result */
    P_BSS_DESC_T        prBssDesc;

    UINT_8              ucAvailableAuthTypes;
    UINT_8              ucRoamingAuthTypes;
    UINT_8              ucCurrAuthAlgNum;
    UINT_8              ucTxAuthAssocRetryCount;

    UINT_8              ucTxAuthAssocRetryLimit;

    /* A flag to indicate if we are going to do roaming in JOIN Module by using
     * ReAssoc Req Frame.
     */
    BOOLEAN             fgIsReAssoc;

    /* A flag to indicate if we already adopt the parameter of targeted BSS */
    BOOLEAN             fgIsParameterAdopted;


    TIMER_T             rJoinTimer;
    TIMER_T             rTxRequestTimer;
    TIMER_T             rRxResponseTimer;

} JOIN_INFO_T, *P_JOIN_INFO_T;



#define JOIN_STATE_TRANSITION_FLAG  fgIsTransition
#define JOIN_NEXT_STATE_VAR         eNextState

#define JOIN_STATE_TRANSITION(prAdapter, eFromState, eToState) \
            { joinFsmTransAction_ ## eFromState ## _to_ ## eToState((P_ADAPTER_T)prAdapter); \
              JOIN_NEXT_STATE_VAR = JOIN_STATE_ ## eToState; \
              JOIN_STATE_TRANSITION_FLAG = (BOOLEAN)TRUE; \
            }

VOID
joinFsmInit (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
joinFsmSteps (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_JOIN_STATE_T eNextState
    );

WLAN_STATUS
joinFsmRunEventStart (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    );

VOID
joinFsmRunEventAbort (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
joinFsmRunEventTxDone (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN WLAN_STATUS rTxDoneStatus
    );

WLAN_STATUS
joinFsmRunEventTxReqTimeOut (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
joinFsmRunEventRxRespTimeOut (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
joinFsmRunEventRxAuthAssoc (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    );

WLAN_STATUS
joinFsmRunEventJoinTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
joinAdoptParametersFromPeerBss (
    IN P_ADAPTER_T prAdapter
    );

VOID
joinAdoptParametersFromCurrentBss (
    IN P_ADAPTER_T prAdapter
    );

VOID
joinComplete (
    IN P_ADAPTER_T prAdapter
    );


#endif /* _JOIN_FSM_H */




