





#ifndef _SCAN_H
#define _SCAN_H



#define BSS_DESC_TIMEOUT_SEC                5 // Second, used by Roaming and AdHoc, to get the newest as could as we can.
#define BSS_DESC_REMOVE_TIMEOUT_SEC         10 // Second.

#define BSS_DESC_GUID                       0x5A5A1461 /* Magic debug values */
#if DBG || BUILD_QA_DBG
#define BSS_DESC_SET_GUID(_prBssDesc)       {(_prBssDesc)->u4Magic = BSS_DESC_GUID;}
#define BSS_DESC_CHK_GUID(_prBssDesc)       ASSERT((_prBssDesc)->u4Magic == BSS_DESC_GUID)
#else
#define BSS_DESC_SET_GUID(_prBssDesc)
#define BSS_DESC_CHK_GUID(_prBssDesc)
#endif /* DBG || BUILD_QA_DBG */

#define SCAN_RM_POLICY_EXCLUDE_CONNECTED    BIT(0) // Remove SCAN result except the connected one.
#define SCAN_RM_POLICY_TIMEOUT              BIT(1) // Remove the timeout one
#define SCAN_RM_POLICY_OLDEST               BIT(2) // Remove the oldest one
#define SCAN_RM_POLICY_WEAKEST              BIT(3) // Remove the weakest rssi
#define SCAN_RM_POLICY_OLDEST_HIDDEN        BIT(4) // Remove the oldest one with hidden ssid
#define SCAN_RM_POLICY_SMART_WEAKEST        BIT(5) /* If there are more than half BSS which has the 
                                                    * same ssid as connection setting, remove the weakest one from them
                                                    * Else remove the weakest one.
                                                    */
#define SCAN_RM_POLICY_ENTIRE               BIT(6) // Remove entire SCAN result


typedef VOID (*PFN_SCAN_DONE_HANDLER)(IN P_ADAPTER_T,
                                      IN WLAN_STATUS);

typedef enum _ENUM_SCAN_TYPE_T
{
    SCAN_TYPE_PASSIVE_SCAN = 0,
    SCAN_TYPE_ACTIVE_SCAN,
    SCAN_TYPE_NUM
} ENUM_SCAN_TYPE_T, *P_ENUM_SCAN_TYPE_T;

typedef enum _ENUM_SCAN_METHOD_T
{
    SCAN_METHOD_FULL_SCAN = 0,
    SCAN_METHOD_ONLINE_SCAN,
    SCAN_METHOD_VOIP_ONLINE_SCAN,
    SCAN_METHOD_NUM
} ENUM_SCAN_METHOD_T, *P_ENUM_SCAN_METHOD_T;


struct _SCAN_REQ_CONFIG_T {
    ENUM_SCAN_METHOD_T      eScanMethod;

    ENUM_SCAN_TYPE_T        eScanType;

    PARAM_SSID_T            rSpecifiedSsid;

    RF_CHANNEL_INFO_T       arChnlInfoList[32];
    UINT_8                  ucNumOfScanChnl;

    UINT_8                  ucChnlDwellTimeMin;
    UINT_8                  ucChnlDwellTimeExt;

    UINT_8                  ucNumOfPrbReq;
    UINT_8                  ucNumOfSpecifiedSsidPrbReq;

    PFN_SCAN_DONE_HANDLER   pfScanDoneHandler;
};

typedef struct _BG_SCAN_SSID_CANDIDATE_T {
    UINT_8          ucNumHwSsidScanEntry;
    PARAM_SSID_T  arHwSsidScanEntry[16];
} BG_SCAN_SSID_CANDIDATE_T, *P_BG_SCAN_SSID_CANDIDATE_T;

typedef struct _BG_SCAN_CONFIG_T {
    BG_SCAN_SSID_CANDIDATE_T    rScanCandidate;
    UINT_16                     u2BaseWakePeriod;
    UINT_8                      ucBgScanStepOfWakePeriod;
    UINT_8                      ucBgScanMinRcpiThr;
    BOOLEAN                     fgIsFromUserSetting;
} BG_SCAN_CONFIG_T, *P_BG_SCAN_CONFIG_T;


typedef struct _SCAN_CONFIG_T {
    ENUM_SCAN_METHOD_T      eScanMethod;

    ENUM_SCAN_TYPE_T        eScanType;
    RF_CHANNEL_INFO_T       rOrgChnlInfo;
    UINT_16                 u2RxFifoThreshold;
    PARAM_SSID_T            rSpecifiedSsid;
    UINT_8                  ucNumOfPrbReq;
    UINT_8                  ucNumOfSpecifiedSsidPrbReq;
    UINT_8                  ucChnlDwellTimeMin;
    UINT_8                  ucChnlDwellTimeExt;
    BOOLEAN                 fgToHonorServicePeriod;
    BOOLEAN                 fgToEnableTriggerEvent;
    UINT_8                  ucNumOfScanChnl;
    RF_CHANNEL_INFO_T       arChnlInfoList[MAXIMUM_OPERATION_CHANNEL_LIST];

    UINT_8                  ucTotalScanChannelCount; /* For VOIP SCAN */
    UINT_8                  ucFinishedChannelCount;
    UINT_8                  ucNumOfPassiveScanInVoIP;


    BG_SCAN_CONFIG_T        rBgScanCfg;

    PFN_SCAN_DONE_HANDLER   pfScanDoneHandler;
} SCAN_CONFIG_T, *P_SCAN_CONFIG_T;


typedef struct _SCAN_INFO_T {
    UINT_32             u4BSSCachedSize;
    PUINT_8             pucBSSCached;

    ENUM_SCAN_STATE_T   eCurrentState;
    ENUM_HW_SCAN_MODE_T eCurrentHwScanMode;

    UINT_8              ucScanBlockInInitialPhaseCount;
    BOOLEAN             fgIsScanReqProceeding;

    SCAN_CONFIG_T       rScanConfig;

    SCAN_STATUS_T       rScanStatus;

    BACKUP_REGISTER_VALUE_T rBkupRegValue;

    LINK_T              rBSSDescList;

    LINK_T              rFreeBSSDescList;

    TIMER_T             rScanCheckForHangTimer;

    TIMER_T             rPartialScanTimer;

    OS_SYSTIME          rScanCompletedTime;

} SCAN_INFO_T, *P_SCAN_INFO_T;




VOID
scanInitialize (
    IN P_ADAPTER_T prAdapter
    );

P_BSS_DESC_T
scanSearchBssDescByPolicy(
    IN P_ADAPTER_T prAdapter
    );

P_BSS_DESC_T
scanSearchBssDescByBssid (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[]
    );

P_BSS_DESC_T
scanSearchBssDescOrAddIfNotExist (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BSS_TYPE_T eBSSType,
    IN UINT_8 aucBSSID[],
    IN UINT_8 aucSrcAddr[]
    );

VOID
scanRemoveBssDescsByPolicy (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4RemovePolicy
    );

VOID
scanRemoveBssDescByBssid (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[]
    );

VOID
scanRemoveConnectionFlagOfBssDescByBssid (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[]
    );

VOID
scanProcessBeaconAndProbeResp (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

WLAN_STATUS
scanSendProbeReqFrames (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8     pucSsid,
    IN UINT_32     u4SsidLen,
    IN UINT_8      ucNumOfPrbReq,
    IN UINT_8      ucNumOfSpecifiedSsidPrbReq
    );
#endif /* _SCAN_H */

