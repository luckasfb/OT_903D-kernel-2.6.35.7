





#ifndef _IBSS_H
#define _IBSS_H



/* IBSS ALONE interval for broadcast Beacon and Reply ProbeResp MMPDU. */
#define IBSS_ALONE_TIMEOUT_BEACON_INTERVAL          200 // Beacon Interval, 200 * 100TU = 20 sec.

/* IBSS Protection Timeout check. */
#define IBSS_PROTECTION_TIMEOUT_CHECK_SEC           60 // sec.






VOID
ibssComposeBeaconProbeRespFrame (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_8      pucBuffer,
    IN PUINT_8      pucDestAddr,
    OUT PUINT_16    pu2WlanHeaderLen,
    OUT PUINT_16    pu2WlanBodyLen
    );

WLAN_STATUS
ibssPrepareBeaconFrame (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4MaxContentLen,
    OUT PUINT_8 pucBeaconContent,
    OUT PUINT_16 pu2ContentLen
    );

VOID
ibssProcessProbeRequest (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    );

WLAN_STATUS
ibssCheckCapabilityForAdHocMode (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    );

VOID
ibssProcessBeacon (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBSSDesc,
    IN P_SW_RFB_T prSwRfb
    );

WLAN_STATUS
ibssStartIBSS (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
ibssMergeIBSS (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    );

VOID
ibssStopIBSS (
    IN P_ADAPTER_T prAdapter
    );

VOID
ibssLeave (
    IN P_ADAPTER_T prAdapter
    );

#endif /* _IBSS_H */

