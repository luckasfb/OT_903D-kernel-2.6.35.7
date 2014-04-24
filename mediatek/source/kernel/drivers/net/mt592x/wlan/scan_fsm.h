





#ifndef _SCAN_FSM_H
#define _SCAN_FSM_H




/* The dwell time for staying in ACTIVE mode when connected to AP (unit: ms) */
#define DEFAULT_SCAN_TYPE                           SCAN_TYPE_ACTIVE_SCAN
#define DEFAULT_SCAN_PER_CHNL_MIN_DWELL_TIME        20
#define DEFAULT_SCAN_PER_CHNL_EXT_DWELL_TIME        120
#define DEFAULT_SCAN_RX_FIFO_THRESHOLD              0
#define DEFAULT_SCAN_NUM_OF_PROBE_REQUEST           3
#define DEFAULT_SCAN_NUM_OF_SPECIFIED_PROBE_REQ     0
#define DEFAULT_SCAN_HONOR_SERVICE_PERIOD           0
#define DEFAULT_SCAN_ENABLE_TRIGGER_EVENT           0

#define BG_SSID_SCAN_BASE_WAKEUP_PERIOD             3000    // unit: TU (min: 1, max: 65535)
#define BG_SSID_SCAN_WAKEUP_PERIOD_STEPS            5       // steps of exp. growth (min: 0, max: 7)
#define BG_SSID_SCAN_WAKEUP_MIN_RCPI                0


#define SCAN_CHANNEL_DWELL_TIME_MIN                 12 // Millisecond
#define SCAN_CHANNEL_DWELL_TIME_EXT                 98 // Millisecond
#define VOIP_SCAN_CHANNEL_DWELL_TIME_EXT            8 // Millisecond

#define FULL_SCAN_TOTAL_PROBE_REQ_NUM               3 // Number of frame
#define FULL_SCAN_SPECIFIC_PROBE_REQ_NUM            1 // Number of frame

#define PARTIAL_SCAN_TOTAL_PROBE_REQ_NUM            2 // Number of frame
#define PARTIAL_SCAN_SPECIFIC_PROBE_REQ_NUM         1 // Number of frame

#define VOIP_PARTIAL_SCAN_TOTAL_PROBE_REQ_NUM       1 // Number of frame
#define VOIP_PARTIAL_SCAN_SPECIFIC_PROBE_REQ_NUM    0 // Number of frame

#define PS_VOIP_PARTIAL_SCAN_TOTAL_PROBE_REQ_NUM    1 // Number of frame
#define PS_VOIP_PARTIAL_SCAN_SPECIFIC_PROBE_REQ_NUM 0 // Number of frame

#define ROAMING_SCAN_TOTAL_PROBE_REQ_NUM            1 // Number of frame
#define ROAMING_SCAN_SPECIFIC_PROBE_REQ_NUM         1 // Number of frame

#define PARTIAL_SCAN_TIMEOUT_MSEC                   20 // Millisecond

#define VOIP_PARTIAL_SCAN_TIMEOUT_MSEC              20 // Millisecond

#define SCAN_VOIP_PASSIVE_SCAN_INTERVAL             3

#define SCAN_CHECK_FOR_HANG_GUARD_TIME_MSEC         1000 // Millisecond

#define INTERLACED_SCAN_CHANNEL_GROUPS_NUM          3 // Groups(e.g. 1/4/7/10, 2/5/8/11, 3/6/9)

typedef enum _ENUM_SCAN_STATE_T {
    SCAN_STATE_IDLE = 0,
    SCAN_STATE_ACTIVE,
    SCAN_STATE_NUM
} ENUM_SCAN_STATE_T;

typedef enum _ENUM_HW_SCAN_MODE_T
{
    ENUM_HW_SCAN_NORMAL_SCAN,
    ENUM_HW_SCAN_BG_SSID_SCAN
} ENUM_HW_SCAN_MODE_T, *PENUM_HW_SCAN_MODE_T;



typedef struct _SCAN_STATUS_T {
    UINT_8 ucLastScanChnlIdx;
    UINT_8 ucLastScanBandIdx;
} SCAN_STATUS_T, *P_SCAN_STATUS_T;

typedef struct _BACKUP_REGISTER_VALUE_T {
    UINT_32 u4RegSpcr;
    UINT_32 u4RegOrgRxFifoThr;
    UINT_32 u4RegOrgLpEnFunc;
} BACKUP_REGISTER_VALUE_T, *P_BACKUP_REGISTER_VALUE_T;




#define SCAN_NEXT_STATE_VAR          eNextState

#define SCAN_STATE_TRANSITION(prAdapter, rFromState, rToState) \
                { scanFsmTransAction_ ## rFromState ## _to_ ## rToState (prAdapter); \
                  SCAN_NEXT_STATE_VAR = SCAN_STATE_ ## rToState; \
                  DBGLOG(SCAN, TRACE, (("SCAN STATE TRANSITION: [%s] --> [%s]\n"), \
                                   #rFromState, #rToState)); \
                   scanFsmStep(prAdapter, SCAN_NEXT_STATE_VAR); \
                }

#define SCAN_STATE_TRANSITION1(prAdapter, rFromState, rToState, rPostFix) \
                { \
                  scanFsmTransAction_ ## rFromState ## _to_ ## rToState ## _ ## rPostFix(prAdapter); \
                  SCAN_NEXT_STATE_VAR = SCAN_STATE_ ## rToState; \
                  DBGLOG(SCAN, TRACE, (("SCAN STATE TRANSITION: [%s] --> [%s]\n"), \
                                   #rFromState, #rToState)); \
                   scanFsmStep(prAdapter, SCAN_NEXT_STATE_VAR); \
                }


VOID
scanFsmInit (
    P_ADAPTER_T prAdapter
    );

WLAN_STATUS
scanFsmRunEventScanReqSetup (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_REQ_CONFIG_T prScanReqConfig
    );

VOID
scanFsmRunEventScanReqCleanUp (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
scanFsmRunEventStart (
    IN P_ADAPTER_T  prAdapter,
    IN ENUM_HW_SCAN_MODE_T eHwScanMode
    );

WLAN_STATUS
scanFsmRunEventScanDone (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
scanFsmRunEventScanAbort (
    IN P_ADAPTER_T  prAdapter
    );

WLAN_STATUS
scanFsmRunEventScanStop (
    IN P_ADAPTER_T  prAdapter
    );

VOID
scanCheckScanStatus (
    IN P_ADAPTER_T prAdapter
    );

VOID
scanSetupOriginalChannel (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    );

#endif /* _SCAN_FSM_H */

