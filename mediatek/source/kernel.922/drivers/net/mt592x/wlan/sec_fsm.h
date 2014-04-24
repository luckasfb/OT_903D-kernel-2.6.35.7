




#ifndef _SEC_FSM_H
#define _SEC_FSM_H


#include "gl_typedef.h"


/* TKIP CounterMeasure interval for Rejoin to Network. */
#define COUNTER_MEASURE_TIMEOUT_INTERVAL_SEC        60

/* Timeout to wait the EAPoL Error Report frame Send out. */
#define EAPOL_REPORT_SEND_TIMEOUT_INTERVAL_SEC       1

typedef UINT_32             SEC_STATUS, *P_SEC_STATUS;

typedef enum _ENUM_SEC_STATE_T {
    SEC_STATE_IDLE = 0,
    SEC_STATE_PORT_CONTROL,
    SEC_STATE_FIFO_BUSY,
    SEC_STATE_CHECK_OK,
    SEC_STATE_SEND_EAPOL,
    SEC_STATE_SEND_DEAUTH,
    SEC_STATE_NUM
} ENUM_SEC_STATE_T;

/* WPA2 PMKID candicate structure */
typedef struct _PMKID_CANDICATE_T {
    PARAM_MAC_ADDRESS   arBssid;
    UINT_32             u4PreAuthFlags;
} PMKID_CANDICATE_T, *P_PMKID_CANDICATE_T;

/* WPA2 PMKID cache structure */
typedef struct _PMKID_ENTRY_T {
    PARAM_BSSID_INFO_T  rBssidInfo;
    BOOLEAN             fgPmkidExist;
} PMKID_ENTRY_T, *P_PMKID_ENTRY_T;

typedef struct _SEC_INFO_T {
    ENUM_SEC_STATE_T   eCurrentState;

    /* Stored the current bss wpa rsn cap filed, used for roaming policy */
    BOOLEAN                 fgWpaCapPresent;
    UINT_16                 u2WpaCap;
    UINT_16                 u2RsnCap;
    
    /* While Do CounterMeasure procedure, check the EAPoL Error report have send out */
    BOOLEAN                 fgCheckEAPoLTxDone;

    UINT_32                 u4RsnaLastMICFailTime;

    /* Boolean flag to let port control check only T/R AP or include DLS */
    BOOLEAN                 fgBlockOnlyAPTraffic;

    BOOLEAN                 fgBlockTxTraffic;
    BOOLEAN                 fgBlockRxTraffic;

    /* By the flow chart of 802.11i, 
               wait 60 sec before associating to same AP 
               or roaming to a new AP
               or sending data in IBSS,
               keep a timer for handle the 60 sec counterMeasure */
    TIMER_T                 rRsnaBlockTrafficTimer;
#if DBG
    BOOLEAN                 fgRsnaEAPoLReportState;
#endif
    TIMER_T                 rRsnaEAPoLReportTimeoutTimer;

    TIMER_T                 rPreauthenticationTimer;

    /* Buffer for WPA2 PMKID */
    /* The PMKID cache lifetime is expire by media_disconnect_indication */
    UINT_32                 u4PmkidCandicateCount;
    PMKID_CANDICATE_T       arPmkidCandicate[CFG_MAX_PMKID_CACHE];
    UINT_32                 u4PmkidCacheCount;
    PMKID_ENTRY_T           arPmkidCache[CFG_MAX_PMKID_CACHE];
    BOOLEAN                 fgIndicatePMKID;

    /* CR1486, CR1640 */
    /* for WPS, disable the privacy check for AP selection policy */
    BOOLEAN                 fgPrivacyCheckDisable;
} SEC_INFO_T, *P_SEC_INFO_T;


typedef SEC_STATUS (*PFN_SEC_FSM_STATE_HANDLER)(IN P_ADAPTER_T);




#define SEC_STATE_TRANSITION_FLAG   fgIsTransition
#define SEC_NEXT_STATE_VAR          eNextState

#define SEC_STATE_TRANSITION(prAdapter, eFromState, eToState) \
                { secFsmTransAction_ ## eFromState ## _to_ ## eToState((P_ADAPTER_T)prAdapter); \
                  SEC_NEXT_STATE_VAR = SEC_STATE_ ## eToState; \
                  SEC_STATE_TRANSITION_FLAG = (BOOLEAN)TRUE; \
                }


/*--------------------------------------------------------------*/
/* Routines to handle the sec check                             */
/*--------------------------------------------------------------*/
/***** Routines in sec_fsm.c *****/
VOID
secFsmInit(
    IN P_ADAPTER_T          prAdapter
    );

VOID
secFsmRunEventInit(
    IN P_ADAPTER_T          prAdapter
    );

VOID
secFsmRunEventStart(
    IN P_ADAPTER_T          prAdapter
    );

VOID
secFsmRunEventAbort(
    IN P_ADAPTER_T          prAdapter
    );

BOOLEAN
secFsmRunEventPTKInstalled(
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid,
    IN PUINT_8              pucKeyMaterial,
    IN UINT_8               ucKeyLen,
    IN UINT_8               ucCipherMode,
    IN UINT_8               ucTxMicOffset,
    IN UINT_8               ucRxMicOffset
    );

VOID
secFsmRunEventFifoEmpty(
    IN P_ADAPTER_T          prAdapter
    );

VOID
secFsmRunEventEapolTxDone(
    IN P_ADAPTER_T           prAdapter

    );

VOID
secFsmRunEventDeauthTxDone(
    IN P_ADAPTER_T           prAdapter

    );

VOID
secFsmRunEventStartCounterMeasure(
    IN P_ADAPTER_T           prAdapter
    );

VOID
secFsmRunEventEndOfCounterMeasure(
    IN P_ADAPTER_T          prAdapter
    );

#endif /* _SEC_FSM_H */


