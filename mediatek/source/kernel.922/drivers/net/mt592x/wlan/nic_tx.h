





#ifndef _NIC_TX_H
#define _NIC_TX_H



#define MAX_NUM_SW_TFCB_FOR_AC0     ((DWORD_TO_BYTE(HW_BUFF_DWSIZE_FOR_AC0) / \
                                     DOT11_FRAGMENTATION_THRESHOLD_MIN) + 1)
#define MAX_NUM_SW_TFCB_FOR_AC1     ((DWORD_TO_BYTE(HW_BUFF_DWSIZE_FOR_AC1) / \
                                     DOT11_FRAGMENTATION_THRESHOLD_MIN) + 1)
#define MAX_NUM_SW_TFCB_FOR_AC2     ((DWORD_TO_BYTE(HW_BUFF_DWSIZE_FOR_AC2) / \
                                     DOT11_FRAGMENTATION_THRESHOLD_MIN) + 1)
#define MAX_NUM_SW_TFCB_FOR_AC3     ((DWORD_TO_BYTE(HW_BUFF_DWSIZE_FOR_AC3) / \
                                     DOT11_FRAGMENTATION_THRESHOLD_MIN) + 1)
#define MAX_NUM_SW_TFCB_FOR_TS0     ((DWORD_TO_BYTE(HW_BUFF_DWSIZE_FOR_TSB) / \
                                     DOT11_FRAGMENTATION_THRESHOLD_MIN) + 1)

#define MAX_NUM_SW_TFCB_FOR_AC4     CFG_MAX_NUM_MSDU_INFO_FOR_TCM
                                    /* ((DWORD_TO_BYTE(HW_BUFF_DWSIZE_FOR_AC4) / \
                                     DOT11_FRAGMENTATION_THRESHOLD_MIN) + 1) */

#define MAX_TX_PRIVILEGE_LOCK_COUNT     1 /* The maximum concurrent lock count of
                                           * TX Privilege. When a ARBITER enter a
                                           * particular STATE, the corresponding MGMT Module
                                           * may take control of the TXQ.
                                           * Currently we only allow 1 MGMT Module take
                                           * TX Privilege at the same time.
                                           */

#define UPDATE_CURRENT_TX_RATE_INTERVAL_SEC     2 // Seconds

//3 /* Session for TX STATISTICS */
typedef enum _ENUM_TX_STATISTIC_COUNTER_T {
    TX_OS_MSDU_COUNT = 0,                   /* MSDU(Ethernet Frame) before fragmentation */
    TX_OS_MSDU_DROP_COUNT,                  /* MSDU(Ethernet Frame) drop count if not connected */
    TX_INTERNAL_MSDU_MMPDU_COUNT,           /* MSDU/MMPDU(Frame composed from Internal Protocol Stack) before fragmentation */

    TX_MPDU_OK_COUNT,                       /* OK = UCAST_OK + BMCAST_OK */
    TX_MPDU_RTS_OK_COUNT,                   /* count for successfully transmitted RTS frame*/
    TX_MPDU_RTS_FAIL_COUNT,                 /* count for failed RTS frame*/

    TX_BMCAST_MPDU_OK_COUNT,                /* B/MCAST MPDU tx ok */
    TX_UCAST_MPDU_OK_COUNT,                 /* UCAST MPDU tx ok */

    TX_BMCAST_MPDU_FAIL_COUNT,              /* B/MCAST MPDU tx fail */
    TX_UCAST_MPDU_FAIL_COUNT,               /* UCAST MPDU tx fail */

    TX_MPDU_TX_TWICE_OK_COUNT,              /* MPDU retry once and get ACK.
                                             * Simulate OID_802_3_XMIT_ONE_COLLISIONS.
                                             */

    TX_MPDU_TX_MORE_TWICE_OK_COUNT,         /* MPDU retry more than once and get ACK.
                                             * Simulate OID_802_3_XMIT_MORE_COLLISIONS.
                                             */

    TX_MPDU_ALL_ERR_COUNT,                  /* NOTE: We didn't include the "ATIM not ACK" count.
                                             * ALL_ERR = UCAST_FAIL + BMCAST_FAIL
                                             */

    TX_MPDU_PORT_CTRL_COUNT,                /* ALL_ERR = PORT_CTRL + LIFETIME_ERR + RTS_RETRY_ERR + MPDU_RETRY_ERR */
    TX_MPDU_LIFETIME_ERR_COUNT,
    TX_MPDU_RTS_RETRY_ERR_COUNT,
    TX_MPDU_MPDU_RETRY_ERR_COUNT,

    TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT,    /* MPDU_EXCESS_RETRY_ERR = RTS_RETRY_ERR + MPDU_RETRY_ERR
                                             * Simulate OID_802_3_XMIT_MAX_COLLISIONS.
                                             */

    TX_MPDU_TOTAL_COUNT,                    /* TOTAL == OK + ALL_ERR, didn't count the Retry */
    TX_MPDU_TOTAL_RETRY_COUNT,              /* TOTAL_RETRY == OK + (ACK Failure Retry) */

    TX_BEACON_MMPDU_COUNT,                  /* Beacon tx count */

    TX_MSDU_BYTES_COUNT,                    /* MSDU Tx Bytes count */
    TX_MSDU_OK_COUNT,
    TX_MSDU_FAIL_COUNT,

    TX_STATISTIC_COUNTER_NUM
} ENUM_TX_STATISTIC_COUNTER_T;

typedef enum _ENUM_TX_ACQ_STATISTIC_COUNTER_T {
    TX_ACQ_TFCB_COUNT = 0,                  /* TFCB send to HW FIFO count */
    TX_ACQ_TXDONE_COUNT,                    /* Return TFCB count */
    TX_ACQ_STATISTIC_COUNTER_NUM
} ENUM_TX_ACQ_STATISTIC_COUNTER_T;



//3 /* Session for TX QUEUES */
typedef enum _ENUM_TRAFFIC_CLASS_T {
    TC0 = 0,
    TC1,
    TC2,
    TC3,
    TCS0,
    TCM,
    TC_NUM /* Maximum number of Traffic Classes. */
} ENUM_TRAFFIC_CLASS_T;


typedef enum _ENUM_TX_QUEUE_T {
    TXQ_AC0 = 0,
    TXQ_AC1,
    TXQ_AC2,
    TXQ_AC3,
    TXQ_TS0,
    TXQ_AC4,
    TXQ_NUM  /* Define the maximum number which can be used in FOR LOOP */
} ENUM_TX_QUEUE_T;

#define TXQ_TCM                     TXQ_AC4

#define TXQ_BEACON_MASK             BIT(TXQ_TS0)

#define TXQ_ALL_MASK                (BIT(TXQ_AC0) | BIT(TXQ_AC1) | BIT(TXQ_AC2) | \
                                     BIT(TXQ_AC3) | BIT(TXQ_TS0) | BIT(TXQ_AC4))

#define TXQ_DATA_MASK               (BIT(TXQ_AC0) | BIT(TXQ_AC1) | BIT(TXQ_AC2) | \
                                     BIT(TXQ_AC3))

#define TXQ_MGMT_MASK               BIT(TXQ_AC4)


typedef struct _TX_AC_PARAM_AIFS_CW_T {
    UINT_16     u2CWmin;            /*!< CWmin */
    UINT_16     u2CWmax;            /*!< CWmax */
    UINT_16     u2TxopLimit;        /*!< TXOP limit */
    UINT_16     u2Aifsn;            /*!< AIFSN */
} TX_AC_PARAM_AIFS_CW_T, *P_TX_AC_PARAM_AIFS_CW_T;


//3 /* Session for MSDU_INFO_T */
struct _MSDU_INFO_T { /* Total size of MSDU_INFO_T = 40 bytes */
    /* Queue Link Entry (4) */
    QUE_ENTRY_T             rQueEntry;

    /* Packet Handler (4) */
    PVOID                   pvPacket;

    /* System Timestamp (4) */
    OS_SYSTIME              rArrivalTime; /* OS System Time, the time which the
                                           * packet just arrived or been created.
                                           */

    /* MSDU Information (7) */
    BOOLEAN                 fgIs802_11Frame; /* Set to TRUE for 802.11 frame */
    BOOLEAN                 fgIsFromInternalProtocolStack;
                            /* Set to TRUE for frames which composed in Managment Layer */

    UINT_8                  ucTID; /* Traffic Identification */
    UINT_8                  ucTC; /* Traffic Class(terminology defined for SW processing) */

    UINT_16                 u2PayloadLength;
    UINT_8                  ucMacHeaderLength;

    /* Callback Function & Parameter (5) */
    BOOLEAN                 fgIsTxFailed; /* Set to TRUE if one of MPDUs get failure in transmission */
    PFN_TX_DONE_HANDLER     pfTxDoneHandler;

    /* Particular Control Flag & Reserved Field (1) */
    UINT_8                  ucControlFlag; /* For specify some Control Flags, e.g. Basic Rate */

    /* Fragment - Total Count, Processing Count, Finished Count (2) */
    UINT_8                  ucFragTotalCount;
                            /* Total count of fragment frame(MPDUs), this value will
                             * be figured out in txProcessMSDU().
                             */
    UINT_8                  ucFragFinishedCount;
                            /* Count of finished fragment frame(MPDUs), increase 1 once a MPDU
                             * has been successfully composed and send to HW FIFO.
                             */

    /* Reserved Padding Byte (1) - for future use.
     * NOTE(Kevin): This field may be used to extend the Control Flag if necessary.
     */
    UINT_8                  ucChkSumWapiFlag;

#if CFG_TX_FRAGMENT
    /* Fragment Threshold/Header Information (3) */
    UINT_16                 u2PayloadFragThreshold; /* Fragmentation threshold without WLAN Header & FCS */
    UINT_8                  ucFragWlanHeaderLength; /* If do fragmentation, we send them in 802.11 format */

    /* Per Packet Information (5) */
    UINT_8                  ucLLCLength; /* Inserted LLC length */
    PUINT_8                 pucLLC; /* Pointer to the corresponding LLC */
#endif /* CFG_TX_FRAGMENT */

#if CFG_IBSS_POWER_SAVE
    /* Get Station Object (4) */
    P_STA_RECORD_T          prStaRec; /* Pointer to the Station Record */
#endif /* CFG_IBSS_POWER_SAVE */

};

typedef struct _MSDU_INFO_MEM_CTRL_T {
    UINT_8          ucMsduPoolID;
    UINT_8          aucReserved[3];
    MSDU_INFO_T     rMsduInfo;
} MSDU_INFO_MEM_CTRL_T, *P_MSDU_INFO_MEM_CTRL_T;



//3 /* Session for SW_TFCB_T  */
/* Various variables that are required for traversing TX PATH and debug. */
typedef struct _SW_TFCB_T {
    QUE_ENTRY_T     rQueEntry;

    UINT_16         u2OverallBufferLength;
    UINT_8          ucAC;
#if (CFG_TX_DBG_INCREASED_PID || CFG_TX_DBG_FIXED_PID)
    UINT_8          ucPID;
#else
    UINT_8          aucReserved[1];
#endif /* CFG_TX_DBG_INCREASED_PID || CFG_TX_DBG_FIXED_PID */

    P_MSDU_INFO_T   prMsduInfo;
} SW_TFCB_T, *P_SW_TFCB_T, **PP_SW_TFCB_T;


//3 /* Session for TX_CTRL_T */
/* Elements in this structure is classified according to TC (Traffic Class) value. */
typedef struct _TC_Q_PARAMETERS_T {
    QUE_T                   rFreeMsduInfoList;

    /* OS Send Queue store packets which are waiting for available SW resource (MSDU_INFO_T)
     * which was occupied by previous packets.
     */
    QUE_T                   rOsSendQueue;

    UINT_8                  ucMaxNumOfMsduInfo;
} TC_Q_PARAMETERS_T, *P_TC_Q_PARAMETERS_T;


/* Elements in this structure is classified according to AC (Access Category) value. */
typedef struct _TX_ACQ_PARAMETERS_T {
    /* Native packets which have obtained a MSDU_INFO_T but lack of available space
     * of HW FIFO to send out will hold in Send Wait Queue.
     */
    QUE_T                   rSendWaitQueue;

    /* Active Chain store pakets which have been write to HW FIFO and waiting for
     * transmission done.
     */
    QUE_T                   rActiveChainList;

    QUE_T                   rFreeTFCBList;
    UINT_16                 u2FreeBufferSizeDW;

    UINT_16                 u2MaxBufferSizeDW;

    UINT_8                  ucMaxNumOfSwTfcb;

    #if CFG_TX_DBG_INCREASED_PID
    UINT_8                  ucPacketID;
    #endif /* CFG_TX_DBG_INCREASED_PID */

    UINT_16                 u2LifeTimeLimit;

    UINT_64                 au8Statistics[TX_ACQ_STATISTIC_COUNTER_NUM]; /* TX Counters for HAL */

} TX_ACQ_PARAMETERS_T, *P_TX_ACQ_PARAMETERS_T;


typedef struct _TX_CTRL_T {

    UINT_32                 u4TxCachedSize;
    PUINT_8                 pucTxCached; /* memory handler */

#if 0 /* Unused in Slave Mode */
    UINT_32                 u4TxUnCachedSize;
    PUINT_8                 pucTxUnCached;
#endif

//4 [Mike][2008/04/29] This is for SDIO Tx Enhanced Mode
#if CFG_SDIO_TX_ENHANCE
    PUINT_8                 pucTxCoalescingBufPtr;
    UINT_32                 u4TxCoalescingBufUsedBlkCount;
    UINT_32                 u4TxCoalescingBufMaxBlkNum;

    UINT_32                 u4WriteBlockSize;

    #if CFG_SDIO_DEBUG_AGGREGATING_RATIO
    UINT_32                 u4TxAggregateFrameCount; /* Number of aggregated frames */
    UINT_32                 u4TxPacketCount; /* For calculating the TX aggregation ratio */
    UINT_32                 u4TxSDIOCmdCount; /* For calculating the TX aggregation ratio */

    #endif /* CFG_SDIO_DEBUG_AGGREGATING_RATIO */
#endif /* CFG_SDIO_TX_ENHANCE */

    UINT_8                  aucTxQoSCtrl_TC2TXQ[TC_NUM]; /* Input TC and get the corresponding TXQ_AC */

    TC_Q_PARAMETERS_T       arTcQPara[TC_NUM];

    TX_ACQ_PARAMETERS_T     arTxACQPara[TXQ_NUM];

    TX_AC_PARAM_AIFS_CW_T   arTxAcParamAifsCw[TXQ_NUM];

    UINT_64                 au8Statistics[TX_STATISTIC_COUNTER_NUM]; /* TX Counters */

    UINT_8                  ucTxActiveACQ; /* Current active AC Queues */
    UINT_8                  ucTxNonEmptyACQ; /* Current non-empty AC Queues */

    UINT_8                  ucTxSuspendACQ; /* When a particular ARB_STATE acquire the
                                             * TX privilege, save the ucTxActiveACQ to
                                             * ucTxSuspendACQ for later restore.
                                             */
    UINT_8                  ucTxPrivilegeLockCount; /* Counter of TX privilege been acquired */

    /* A flag to indicate that at least one packet exist in rOsSendQueue queue of arTcQPara[] */
    BOOLEAN                 fgIsPacketInOsSendQueue;

                            /* A Flag for SEC Module which effects all TXQs to make
                             * sure FIFO is empty while cipher key change.
                             */
    BOOLEAN                 fgIsSignalFifoEmpty; /* When TRUE, TX module will signal an event of FIFO empty */

    BOOLEAN                 fgIsEnableTxVoipFlowCtrl;
    BOOLEAN                 fgIsPermitAccessingTXQ_VoIP;/* TRUE: able to access TX queue used for VoIP.
                                                           FALSE: block the use of TX queue used for VoIP. */

    ENUM_TX_QUEUE_T         rTxQForVoipAccess;          /* Under WMM associationm it is assigned to AC3.
                                                           Otherwise, it is assigned to AC1 */

    BOOLEAN                 fgIsEnableTriggerOfVoipScan; /* When TRUE, TX module will
                                                          * signal an event of AC3 FIFO empty
                                                          * for VOIP SCAN.
                                                          */

    BOOLEAN                 fgIsTxMediumTimeAvailable;
                            /* A flag to indiate that we have available medium time for TX
                             * under circumstance of Admission Control.
                             */
    BOOLEAN                 fgIsRfTestTxMode;
                            /* When TRUE, In RFTest mode*/

#if CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN
    BOOLEAN                 fgBlockTxDuringJoin; /* We stop sending Data Frames to TX
                                                  * FIFO because we may repartition
                                                  * HW FIFO after join complete.
                                                  */
#endif /* CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN */

#if CFG_TX_AGGREGATE_HW_FIFO
    BOOLEAN                 fgAggregateTxFifo; /* Aggregate AC0~3 TX FIFO into AC1 */
#endif /* CFG_TX_AGGREGATE_HW_FIFO */

#if CFG_TX_DBG_INT_FALSE_ALARM
    BOOLEAN                 fgIsSkipTxFalseAlarmCheck;
#endif /* CFG_TX_DBG_INT_FALSE_ALARM */


    OS_SYSTIME              rCurrRateLastUpdateTime; /* The last time when update Current Tx Rate */
    UINT_8                  ucLastTxWlanIndex; /* The WlanIdx of TX Done */
    UINT_8                  ucCurrRateIndex; /* Current TX Rate Index */
#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS
    TIMER_T         rPktTxIntervalTimer;
#endif
} TX_CTRL_T, *P_TX_CTRL_T;







#define NIC_TX_IS_ACTIVE_ACQ_NOT_EMPTY(_prAdapter) \
        (_prAdapter->rTxCtrl.ucTxActiveACQ & _prAdapter->rTxCtrl.ucTxNonEmptyACQ)

#define NIC_TX_IS_ACQ_NOT_EMPTY(_prAdapter) \
        (_prAdapter->rTxCtrl.ucTxNonEmptyACQ)

#define NIC_TX_IS_ACTIVE_ACQ_EMPTY(_prAdapter) \
        (_prAdapter->rTxCtrl.ucTxActiveACQ & ~(_prAdapter->rTxCtrl.ucTxNonEmptyACQ))

#define NIC_TX_IS_AC4_EMPTY(_prAdapter) \
        (BIT(TXQ_AC4) & ~(_prAdapter->rTxCtrl.ucTxNonEmptyACQ))

#define NIC_TX_IS_BUFFER_VOICE_PACKET(_prAdapter) \
        ( QUEUE_IS_NOT_EMPTY( &(_prAdapter->rTxCtrl.arTxACQPara[_prAdapter->rTxCtrl.rTxQForVoipAccess].rSendWaitQueue) ) || \
          QUEUE_IS_NOT_EMPTY( &(_prAdapter->rTxCtrl.arTcQPara[_prAdapter->rTxCtrl.rTxQForVoipAccess].rOsSendQueue) ) )

#define NIC_TX_SET_VOIP_FLOW_CONTROL_RESUME(_prAdapter) \
        {((P_ADAPTER_T)(_prAdapter))->rTxCtrl.fgIsPermitAccessingTXQ_VoIP = TRUE;}

#define NIC_TX_TEST_AND_SET_IS_NEED_CHECK_TXQ_EMPTY(_prAdapter) \
        ( testAndSet( &((_prAdapter)->rTxCtrl.fgIsTxMediumTimeAvailable) ) )

#define NIC_TX_SET_VOIP_SCAN_TRIGGER_EVENT(_prAdapter) \
        {((P_ADAPTER_T)(_prAdapter))->rTxCtrl.fgIsEnableTriggerOfVoipScan = TRUE;}

#define NIC_TX_UNSET_VOIP_SCAN_TRIGGER_EVENT(_prAdapter) \
        {((P_ADAPTER_T)(_prAdapter))->rTxCtrl.fgIsEnableTriggerOfVoipScan = FALSE;}



#define MSDU_INFO_CTRL_FLAG_NONE                    0
#define MSDU_INFO_CTRL_FLAG_SPECIFY_AC              BIT(0) /* To specify a particular AC for this MSDU_INFO_T */
#define MSDU_INFO_CTRL_FLAG_BASIC_RATE              BIT(1) /* Send this MSDU_INFO_T in Basic Rate */
#define MSDU_INFO_CTRL_FLAG_NO_ACK                  BIT(2) /* Send this MSDU_INFO_T without waiting for ACK */
#define MSDU_INFO_CTRL_FLAG_BMCAST                  BIT(3) /* This MSDU_INFO_T is BMCAST frame(A1 Address - Flag for IBSS PS) */
#define MSDU_INFO_CTRL_FLAG_CALCULATE_MIC           BIT(4) /* This MSDU_INFO_T require calculate MIC before doing fragmentation */
#define MSDU_INFO_CTRL_FLAG_CALCULATE_CSUM          BIT(5) /* This MSDU_INFO_T require CSUM Offload by SW */
#define MSDU_INFO_CTRL_FLAG_LIFETIME_NEVER_EXPIRE   BIT(6) /* This MSDU_INFO_T's lifetime won't expire */
#define MSDU_INFO_CTRL_FLAG_DISABLE_PRIVACY_BIT     BIT(7) /* This MSDU_INFO_T is None privacy frame, for native 802.11 */


#if CFG_IBSS_POWER_SAVE
    #define MSDU_INFO_OBJ_INIT_STA_REC(_prMsduInfo, _prStaRecord)   \
        (((P_MSDU_INFO_T)_prMsduInfo)->prStaRec = (P_STA_RECORD_T)(_prStaRecord))
#else
    #define MSDU_INFO_OBJ_INIT_STA_REC(_prMsduInfo, _prStaRecord)
#endif /* CFG_IBSS_POWER_SAVE*/

#define MSDU_INFO_OBJ_INIT(_prMsduInfo, \
                           _fgIs802_11, \
                           _fgIsFromInternalProtocolStack, \
                           _pvPacketDescriptor, \
                           _ucTID, \
                           _ucTrafficClass, \
                           _ucMacHeaderLength, \
                           _u2PayloadLength, \
                           _ucControlFlag, \
                           _pfCallBackFunction, \
                           _rArrivalTime, \
                           _prStaRecord \
                           ) \
        { \
            ((P_MSDU_INFO_T)_prMsduInfo)->fgIs802_11Frame = (_fgIs802_11); \
            ((P_MSDU_INFO_T)_prMsduInfo)->fgIsFromInternalProtocolStack = (_fgIsFromInternalProtocolStack); \
            ((P_MSDU_INFO_T)_prMsduInfo)->pvPacket = (_pvPacketDescriptor); \
            ((P_MSDU_INFO_T)_prMsduInfo)->ucTID = (_ucTID); \
            ((P_MSDU_INFO_T)_prMsduInfo)->ucTC = (_ucTrafficClass); \
            ((P_MSDU_INFO_T)_prMsduInfo)->u2PayloadLength = (_u2PayloadLength); \
            ((P_MSDU_INFO_T)_prMsduInfo)->ucMacHeaderLength = (_ucMacHeaderLength); \
            ((P_MSDU_INFO_T)_prMsduInfo)->ucControlFlag = (_ucControlFlag); \
            ((P_MSDU_INFO_T)_prMsduInfo)->pfTxDoneHandler = (_pfCallBackFunction); \
            ((P_MSDU_INFO_T)_prMsduInfo)->rArrivalTime = (_rArrivalTime); \
            MSDU_INFO_OBJ_INIT_STA_REC(_prMsduInfo, _prStaRecord); \
        }

#define TX_INC_CNT(prTxCtrl, eCounter)              \
    {((P_TX_CTRL_T)prTxCtrl)->au8Statistics[eCounter]++;}

#define TX_ADD_CNT(prTxCtrl, eCounter, u8Amount)    \
    {((P_TX_CTRL_T)prTxCtrl)->au8Statistics[eCounter] += (UINT_64)u8Amount;}

#define TX_GET_CNT(prTxCtrl, eCounter)              \
    (((P_TX_CTRL_T)prTxCtrl)->au8Statistics[eCounter])

#define TX_RESET_ALL_CNTS(prTxCtrl)                 \
    {kalMemZero(&prTxCtrl->au8Statistics[0], sizeof(prTxCtrl->au8Statistics));}

#define TX_ACQ_INC_CNT(prTxCtrl, ucAC, eCounter)    \
    {((P_TX_CTRL_T)prTxCtrl)->arTxACQPara[ucAC].au8Statistics[eCounter]++;}

#define TX_ACQ_GET_CNT(prTxCtrl, ucAC, eCounter)    \
    (((P_TX_CTRL_T)prTxCtrl)->arTxACQPara[ucAC].au8Statistics[eCounter])

#define TX_ACQ_RESET_ALL_CNTS(prTxCtrl, ucAC)       \
    {kalMemZero(&prTxCtrl->arTxACQPara[ucAC].au8Statistics[0], \
        sizeof(prTxCtrl->arTxACQPara[ucAC].au8Statistics));}



VOID
nicTxInitialize (
    IN P_ADAPTER_T  prAdapter
    );


#if CFG_TX_AGGREGATE_HW_FIFO
VOID
nicTxAggregateTXQ (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN fgAggregateTxFifo
    );
#endif /* CFG_TX_AGGREGATE_HW_FIFO */

VOID
nicTxRelease (
    IN P_ADAPTER_T  prAdapter
    );

P_MSDU_INFO_T
nicTxAllocMsduInfo (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucTrafficClass
    );

VOID
nicTxReturnMsduInfo (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    );

VOID
nicTxDiscardMsduInfo (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    );

VOID
nicTxNonQoSUpdateTXQParameters (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PHY_TYPE_INDEX_T ePhyTypeIndex
    );

VOID
nicTxNonQoSAssignDefaultAdmittedTXQ (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxQoSUpdateTXQParameters (
    IN P_ADAPTER_T prAdapter,
    IN WMM_AC_PARAM_T arWmmAcParams[]
    );

VOID
nicTxQoSAssignAdmittedTXQ (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucACI2AdmittedACI[]
    );

VOID
nicTxQoSRearrangeQueuesForAdmCtrl (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessAdmitTimeMetInterrupt(
    IN P_ADAPTER_T      prAdapter
    );

WLAN_STATUS
nicTxAcquireResourceAndTFCBs (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN UINT_8           ucAC,
    OUT P_QUE_T         prAllocatedTFCBList
    );

VOID
nicTxReturnTFCB (
    IN P_ADAPTER_T  prAdapter,
    IN P_SW_TFCB_T  prSwTfcb
    );

VOID
nicTxReturnTFCBs (
    IN P_ADAPTER_T  prAdapter,
    IN P_QUE_T      prTFCBListNeedToReturn
    );

BOOLEAN
nicTxSetSignalWhenFifoNonEmpty (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgClearSignal    
    );

VOID
nicTxSignalFifoEmptyEvent(
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxVoipFlowCtrlEnable (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicTxVoipFlowCtrlDisable (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicTxVoipFlowCtrlCheckForSuspend (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicTxVoipFlowCtrlResumePendingFrames (
    IN P_ADAPTER_T      prAdapter
    );

WLAN_STATUS
nicTxService (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
#if CFG_SDIO_TX_ENHANCE
    ,IN BOOLEAN         fgIsAggregate
#endif /* CFG_SDIO_TX_ENHANCE */
    );

VOID
nicTxRetransmitOfOsSendQue (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicTxRetransmitOfSendWaitQue (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcessTxInterrupt(
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicTxCleanUpOsSendQue (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxCleanUpSendWaitQue (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxCleanUpActiveList (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxStartQueues (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucStartQues
    );

VOID
nicTxDisableTxQueueActiveState (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxFlushStopQueues (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucFlushQues,
    IN UINT_8 ucStopQues
    );

VOID
nicTxRefreshQueues (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxAcquirePrivilegeOfTxQue (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucStartQues,
    IN UINT_8 ucStopQues,
    IN UINT_8 ucFlushQues
    );

VOID
nicTxReleasePrivilegeOfTxQue (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucFlushQues
    );

VOID
nicTxQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
nicTxQueryStatistics (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
nicTxSetStatistics (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxGetCurrentTxDataRate (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_IBSS_POWER_SAVE
VOID
nicTxReclaimTxPackets (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
nicTxRetransmitOfStaWaitQue (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicTxCleanUpStaWaitQue (
    IN P_ADAPTER_T prAdapter
    );
#endif /* CFG_IBSS_POWER_SAVE */

__KAL_INLINE__ VOID
nicTxDataTypeCheck (
    VOID
    )
{
    /* We use the same bitmap setting in ENUM_TX_QUEUE_T(NIC_TX.H) which is
     * identical to ENUM_HW_QUE_T(MT592X_REG.H), So that we simplify the control
     * parameter of SW & HW TXQ between NIC_TX & HAL Layer.
     */
    DATA_STRUC_INSPECTING_ASSERT(TXQ_AC0 == AC0);
    DATA_STRUC_INSPECTING_ASSERT(TXQ_AC1 == AC1);
    DATA_STRUC_INSPECTING_ASSERT(TXQ_AC2 == AC2);
    DATA_STRUC_INSPECTING_ASSERT(TXQ_AC3 == AC3);
    DATA_STRUC_INSPECTING_ASSERT(TXQ_TS0 == TS0);
    DATA_STRUC_INSPECTING_ASSERT(TXQ_AC4 == AC4);


    DATA_STRUC_INSPECTING_ASSERT(sizeof(TX_AC_PARAM_AIFS_CW_T) == 8);

#if CFG_TX_FRAGMENT && CFG_IBSS_POWER_SAVE
    #define MSDU_INFO_T_EXT     (8+4)
#elif CFG_TX_FRAGMENT
    #define MSDU_INFO_T_EXT     8
#elif CFG_IBSS_POWER_SAVE
    #define MSDU_INFO_T_EXT     4
#else
    #define MSDU_INFO_T_EXT     0
#endif /* CFG_TX_FRAGMENT */

    DATA_STRUC_INSPECTING_ASSERT(sizeof(MSDU_INFO_T) == (28 + MSDU_INFO_T_EXT));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(MSDU_INFO_MEM_CTRL_T) == (32 + MSDU_INFO_T_EXT));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(SW_TFCB_T) == 12);

    return;
}
#endif /* _NIC_TX_H */


