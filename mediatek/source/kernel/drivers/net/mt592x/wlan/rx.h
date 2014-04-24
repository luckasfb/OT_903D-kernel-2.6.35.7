




#ifndef _RX_H_
#define _RX_H_



#define MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS     3

#define UPDATE_BSS_RSSI_INTERVAL_SEC            3 // Seconds

/* Fragment information structure */
typedef struct _FRAG_INFO_T {
    UINT_16                 u2NextFragSeqCtrl;
    PUINT_8                 pucNextFragStart;
    P_SW_RFB_T              pr1stFrag;
    OS_SYSTIME              rReceiveLifetimeLimit; /* The receive time of 1st fragment */
} FRAG_INFO_T, *P_FRAG_INFO_T;





WLAN_STATUS
rxProcessMSDU (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
);

WLAN_STATUS
rxProcessMPDU (
    IN P_ADAPTER_T prAdapter,
    IN OUT PP_SW_RFB_T prSWRfb
);

#if SUPPORT_WPS
WLAN_STATUS
rxIndicateMgt(
    IN P_ADAPTER_T  prAdapter,
    IN P_SW_RFB_T   prSWRfb,
    IN UINT_8       ucFrameType
    );
#endif

WLAN_STATUS
rxProcessMgmtFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

WLAN_STATUS
rxProcessDataFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

WLAN_STATUS
rxFilterRecvPacket (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

BOOLEAN
rxIsDuplicateFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

WLAN_STATUS
rxProcessMPDUSecurity (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

#if SUPPORT_WAPI
WLAN_STATUS
rxProcessMPDUWpiSecurity (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );
#endif

P_SW_RFB_T
rxDefragMPDU(
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    );

WLAN_STATUS
rxWlanHeaderTranslation (
    IN  P_ADAPTER_T prAdapter,
    IN  P_SW_RFB_T  prSwRfb
    );

VOID
rxUpdateRssi (
    IN P_ADAPTER_T  prAdapter
    );

#if (CFG_DBG_BEACON_RCPI_MEASUREMENT || CFG_LP_PATTERN_SEARCH_SLT)
VOID
rxRssiClearRssiLinkQualityRecords (
    IN  P_ADAPTER_T prAdapter
    );

INT_32
rxGetAverageRssi(
    IN  P_ADAPTER_T prAdapter,
    IN  INT_32      i4NewRssi
    );

BOOL
rxBcnRcpiMeasure (
    IN  P_ADAPTER_T prAdapter,
    IN  P_SW_RFB_T  prSwRfb
    );
#endif /* CFG_DBG_BEACON_RCPI_MEASUREMENT || CFG_LP_PATTERN_SEARCH_SLT */

#endif /* _RX_H_ */

