





#ifndef _TX_H
#define _TX_H




//4 /* TX Call Back Function Prototype  */
typedef VOID (*PFN_TX_DONE_HANDLER)(IN P_ADAPTER_T,
                                    IN P_MSDU_INFO_T,
                                    IN WLAN_STATUS);




#if CFG_TX_FRAGMENT
VOID
txFragInfoUpdate (
    IN P_ADAPTER_T prAdapter
    );

VOID
txFragInfoUpdateForPrivacy (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsPrivacyEnable,
    IN BOOLEAN fgIsTkipCipher
    );

VOID
txFragComposeWlanDataFrameHeader (
    IN P_ADAPTER_T prAdapter,
    IN P_ETH_FRAME_T prEthFrame,
    IN UINT_8 ucTID,
    IN PUINT_8 pucOutput
    );

WLAN_STATUS
txFragmentation (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    );

#if SUPPORT_WAPI
BOOLEAN
txFragMsduFromOSForWapi (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    );
#endif

UINT_16
txDynamicFragThresholdInitForBT (
    IN P_ADAPTER_T      prAdapter,
    IN P_STA_RECORD_T   prStaRec
    );

VOID
txRateSetInitForBT (
    IN P_ADAPTER_T      prAdapter,
    IN P_STA_RECORD_T   prStaRec
    );
#endif /* CFG_TX_FRAGMENT */

WLAN_STATUS
txProcessMSDU (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    );

#endif /* _TX_H */

