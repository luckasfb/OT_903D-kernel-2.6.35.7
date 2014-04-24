




#ifndef _NIC_RX_H
#define _NIC_RX_H


extern RX_FILTER_MAP_T arRxFilterMapTable[];


typedef enum _ENUM_RX_STATISTIC_COUNTER_T {
    RX_MPDU_TOTAL_COUNT = 0,                /* TOTAL = MGMT + DATA + CTRL */
    RX_DATA_FRAME_COUNT,                    /* DATA = BMCAST_DATA + UCAST_DATA */
    RX_UCAST_DATA_FRAME_COUNT,
    RX_BMCAST_DATA_FRAME_COUNT,

    RX_MGMT_FRAME_COUNT,                    /* MGMT = BMCAST_MGMT + UCAST_MGMT */
    RX_UCAST_MGMT_FRAME_COUNT,
    RX_BMCAST_MGMT_FRAME_COUNT,

    RX_CTRL_FRAME_COUNT,                    /* No BMCAST in this category */

    RX_FIFO_FULL_DROP_COUNT,                /* FIFO_FULL is not included in TOTAL */
    RX_SIZE_ERR_DROP_COUNT,

    RX_ERROR_DROP_COUNT,                    /* ERROR(FCS/ICV/Format/Key) are not included in TOTAL */
    RX_FCS_ERR_DROP_COUNT,
    RX_FORMAT_ERR_DROP_COUNT,
    RX_ICV_ERR_DROP_COUNT,
    RX_KEY_ERR_DROP_COUNT,

    RX_TKIP_ERR_DROP_COUNT,                 /* TKIP ERROR is included in TOTAL */

    RX_RETRY_FRAME_COUNT,
    RX_DUPLICATE_DROP_COUNT,                /* Duplicate is included in TOTAL (* Only check UCAST frames) */

    RX_DATA_CLASS_ERR_DROP_COUNT,
    RX_DATA_PORT_CTRL_DROP_COUNT,

    RX_DATA_INDICATION_COUNT,               /* Null Data Frame will not be included in this category */

#if CFG_TCP_IP_CHKSUM_OFFLOAD
    RX_CSUM_TCP_FAILED_COUNT,
    RX_CSUM_UDP_FAILED_COUNT,
    RX_CSUM_IP_FAILED_COUNT,
    RX_CSUM_TCP_SUCCESS_COUNT,
    RX_CSUM_UDP_SUCCESS_COUNT,
    RX_CSUM_IP_SUCCESS_COUNT,
    RX_CSUM_UNKNOWN_L4_PKT_COUNT,
    RX_CSUM_UNKNOWN_L3_PKT_COUNT,
    RX_IP_V6_PKT_CCOUNT,
#endif

    RX_MPDU_CCK_SHORT_PREAMBLE_COUNT,       /* CCK short preamble packet in TOTAL */
    RX_MPDU_CCK_LONG_PREAMBLE_COUNT,        /* CCK long preamble packet in TOTAL */
    RX_BB_FCS_ERROR_COUNT,                  /* BB rx MPDU with FCS error in TOTAL */
    RX_BB_FIFO_FULL_COUNT,                  /* BB rx MPDU with FCS correct but is dicarded due to Fifo full in TOTAL */
    RX_BB_MPDU_COUNT,                       /* BB rx MPDU number in TOTAL */
    RX_BB_CHANNEL_IDLE_COUNT,               /*Channel idle count in unit of slot*/
    RX_BB_CCA_TIME_COUNT,                   /* BB CCA active timer in unit of us in TOTAL */
    RX_BB_CCANAVTX_TIME_COUNT,              /* BB CCA/NAV/Tx active timer in unit of us in TOTAL */
    RXTX_BEACON_TIMEOUT_COUNT,              /* Beacon timeout count in TOTAL */

    RX_MSDU_BYTES_COUNT,                    /* MSDU Rx Bytes count */


    RX_STATISTIC_COUNTER_NUM
} ENUM_RX_STATISTIC_COUNTER_T;

typedef enum _ENUM_RX_MULTICAST_TYPE_T {
    MC_TYPE_DENY_ALL,
    MC_TYPE_ALLOW_LIST,
    MC_TYPE_ALLOW_ALL,
    MC_TYPE_UPDATE_LIST_ONLY,
    MC_TYPE_NUM
} ENUM_RX_MULTICAST_TYPE_T, *P_ENUM_RX_MULTICAST_TYPE_T;


struct _SW_RFB_T {
    QUE_ENTRY_T             rQueEntry;
    PVOID                   pvPacket;     /* ptr to rx Packet Descriptor */
    PUINT_8                 pucRecvBuff; /* ptr to receive data buffer */

    P_RX_STATUS_T           prRxStatus;         /* ptr to rx status */
    P_STA_RECORD_T          prStaRec;           /* SwRfb_p's frame's source station record. */

    PVOID                   pvHeader;
    PVOID                   pvBody;

    UINT_16                 u2FrameLength;    /* total size of receive frame */
    UINT_16                 u2MACHeaderLength;/* MAC header length of received frame */

    BOOLEAN                 fgIs8023;           /* flag of 802.3 */
    BOOLEAN                 fgIsDataFrame;
    BOOLEAN                 fgIsNullData;
    BOOLEAN                 fgFragmented;

    ENUM_CSUM_RESULT_T      aeCSUM[CSUM_TYPE_NUM];

    UINT_8                  ucRxFlags;        /* QoS, A4 */
    UINT_8                  ucQosTID;         /* if QoS frame, this is TID in QoS Control Field */

    P_RX_STATUS_G0_T        prG0;
    P_RX_STATUS_G1_T        prG1;
    P_RX_STATUS_G2_T        prG2;
};


/* RX configuration type structure by *Mike* */
typedef struct _RX_CTRL_T {
    UINT_32                 u4RxCachedSize;
    PUINT_8                 pucRxCached;

    QUE_T                   rFreeRFBWithBuffList; /* RFB list with buffers */
    QUE_T                   rFreeRFBWOBuffList;   /* RFB list without buffers */
    QUE_T                   rReceivedRFBList;

    PVOID                   apvIndPacket[CFG_RX_MAX_PKT_NUM];
    UINT_8                  ucNumIndPacket;

    BOOLEAN                 fgIsDefragQueNonEmpty;

    BOOLEAN                 fgIsRxQueActive;
    BOOLEAN                 fgIsRxQueSuspend;

    UINT_64                 au8Statistics[RX_STATISTIC_COUNTER_NUM]; /* RX Counters */

    BOOLEAN                 fgRxFilter[RXFILTER_NUM];
    UINT_32                 u4RxFilterReg;

    BOOLEAN                 fgRxMulticastPkt;
    BOOLEAN                 fgRxMulticastPktByTable;


    BOOLEAN                 fgIsRxStatusG0; //TRUE when G0 field is enabled.
    BOOLEAN                 fgIsRxStatusG1;
    BOOLEAN                 fgIsRxStatusG2;
    BOOLEAN                 fgIsRxStatusG0Rssi; /*TRUE when G0 field is
                                            show RSSI group */

    BOOLEAN                 fgIsRfTestRxMode;
                           /* When TRUE, In RFTest mode*/

	BOOLEAN                 fgEnablePromiscuousMode;
} RX_CTRL_T, *P_RX_CTRL_T;




#define RX_INC_CNT(prRxCtrl, eCounter)              \
    {((P_RX_CTRL_T)prRxCtrl)->au8Statistics[eCounter]++;}

#define RX_ADD_CNT(prRxCtrl, eCounter, u8Amount)    \
    {((P_RX_CTRL_T)prRxCtrl)->au8Statistics[eCounter] += (UINT_64)u8Amount;}

#define RX_GET_CNT(prRxCtrl, eCounter)              \
    (((P_RX_CTRL_T)prRxCtrl)->au8Statistics[eCounter])

#define RX_RESET_ALL_CNTS(prRxCtrl)                 \
    {kalMemZero(&prRxCtrl->au8Statistics[0], sizeof(prRxCtrl->au8Statistics));}

#define NIC_RFB_IS_QOS(_prSWRfb)            \
            HAL_RX_STATUS_IS_QOS( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus)

#define NIC_RFB_GET_SEQCTRL(_prSWRfb)       \
            HAL_RX_STATUS_GET_SEQCTRL( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus)

#define NIC_RFB_IS_RETRY(_prSWRfb)          \
            HAL_RX_STATUS_IS_RETRY( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus)

#define NIC_RFB_GET_RCPI(_prSWRfb)          \
            ( (RCPI)HAL_RX_STATUS_GET_RCPI( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus) )

#define NIC_RFB_BSSID_MATCHED(_prSWRfb)     \
            HAL_RX_STATUS_BSSID_MATCHED( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus)

#define NIC_RFB_GET_TA(_prSWRfb)            \
            HAL_RX_STATUS_GET_TA( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus)

#define NIC_RFB_IS_TCL(_prSWRfb)            \
            HAL_RX_STATUS_IS_TCL( ((P_SW_RFB_T)(_prSWRfb))->prRxStatus)

VOID
nicRxInitialize (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicRxUninitialize (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicRxProcessRFBs (
    IN  P_ADAPTER_T prAdapter
    );

#if !CFG_SDIO_STATUS_ENHANCE
VOID
nicRxReceiveRFBs (
    IN  P_ADAPTER_T prAdapter
    );
#else
VOID
nicRxSDIOReceiveRFBs (
    IN  P_ADAPTER_T prAdapter
    );
#endif /* CFG_SDIO_STATUS_ENHANCE */

WLAN_STATUS
nicRxSetupRFB (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prRfb
    );

VOID
nicRxReturnRFB (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prRfb
    );

VOID
nicProcessRxInterrupt (
    IN  P_ADAPTER_T prAdapter
    );

VOID
nicRxStartQueue (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicRxFlushStopQueue (
    IN P_ADAPTER_T prAdapter,
    BOOLEAN fgIsFlushRxQue,
    BOOLEAN fgIsStopRxQue
    );

VOID
nicRxAcquirePrivilegeOfRxQue (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsStartRxQue,
    IN BOOLEAN fgIsStopRxQue,
    IN BOOLEAN fgIsFlushRxQue
    );

VOID
nicRxReleasePrivilegeOfRxQue (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsFlushRxQue,
    IN BOOLEAN fgIsKeepRxActive
    );

VOID
nicRxQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
nicRxQueryStatistics (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
nicRxSetStatistics (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_TCP_IP_CHKSUM_OFFLOAD
VOID
nicRxUpdateCSUMStatistics (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_CSUM_RESULT_T aeCSUM[]
    );
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

VOID
nicRxEnablePromiscuousMode (
    IN P_ADAPTER_T prAdapter
    );
VOID
nicRxDisablePromiscuousMode (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicRxSetMulticast (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_RX_MULTICAST_TYPE_T eMCType,
    IN PUINT_8 prMCAddrList,
    IN UINT_8 ucNum
    );

void
nicRxSetBroadcast (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgEnableBroadcast
    );

#endif /* _NIC_RX_H */

