




#ifndef _RFTEST_H
#define _RFTEST_H



#define MTK_CUSTOM_OID_INTERFACE_VERSION     0x00005921    // for WPDWifi DLL
#define RF_AUTO_TEST_FUNCTION_TABLE_VERSION  0x01000000

#define RF_AT_PARAM_TX_80211HDR_BYTE_MAX     32
#define RF_AT_PARAM_TX_80211HDR_BYTE_DEFAULT 24

#define RF_AT_PARAM_TX_80211PAYLOAD_BYTE_MAX (2048) //can not >= this length

#define RF_AT_PARAM_POWER_MASK      BITS(0,7)
#define RF_AT_PARAM_POWER_MAX       RF_AT_PARAM_POWER_MASK
#define RF_AT_PARAM_RATE_MCS_MASK   BIT(31)
#define RF_AT_PARAM_RATE_MASK       BITS(0,7)
#define RF_AT_PARAM_RATE_CCK_MAX    3
#define RF_AT_PARAM_RATE_1M         0
#define RF_AT_PARAM_RATE_2M         1
#define RF_AT_PARAM_RATE_5_5M       2
#define RF_AT_PARAM_RATE_11M        3
#define RF_AT_PARAM_RATE_6M         4
#define RF_AT_PARAM_RATE_9M         5
#define RF_AT_PARAM_RATE_12M        6
#define RF_AT_PARAM_RATE_18M        7
#define RF_AT_PARAM_RATE_24M        8
#define RF_AT_PARAM_RATE_36M        9
#define RF_AT_PARAM_RATE_48M        10
#define RF_AT_PARAM_RATE_54M        11

#define RF_AT_PARAM_TXOPQUE_QMASK   BITS(16,31)
#define RF_AT_PARAM_TXOPQUE_TMASK   BITS(0,15)
#define RF_AT_PARAM_TXOPQUE_AC0     (0<<16)
#define RF_AT_PARAM_TXOPQUE_AC1     (1<<16)
#define RF_AT_PARAM_TXOPQUE_AC2     (2<<16)
#define RF_AT_PARAM_TXOPQUE_AC3     (3<<16)
#define RF_AT_PARAM_TXOPQUE_QOFFSET 16



#define RF_AT_PARAM_ANTENNA_ID_MAX  1

#define RF_AT_PARAM_PACKET_UNLIMITED  0

#define RF_AT_PARAM_TX_PKTLEN_BYTE_DEFAULT  1024
#define RF_AT_PARAM_TX_PKTLEN_BYTE_MAX  \
    ((UINT_16)(RF_AT_PARAM_TX_80211HDR_BYTE_DEFAULT +RF_AT_PARAM_TX_80211PAYLOAD_BYTE_MAX ))
#define RF_AT_PARAM_TX_DATA_STRUCT_MAX  \
                ((UINT_16)(RF_AT_PARAM_TX_PKTLEN_BYTE_MAX+ (TFCB_SIZE)))

#define RF_AT_PARAM_TX_PKTCNT_DEFAULT       1000
#define RF_AT_PARAM_TX_PKT_INTERVAL_US_DEFAULT  50


#define RF_AT_PARAM_TX_RETRY_MAX    6

#define RF_AT_TX_BUFFER_SIZE        (RF_AT_PARAM_TX_DATA_STRUCT_MAX + sizeof(QUE_ENTRY_T)) //4K+12 = 4060+32+12+4
#define RF_AT_TX_BUFFER_NUM_MAX     CFG_MAX_NUM_MSDU_INFO_FOR_TC0

typedef enum _ENUM_RF_AT_COMMAND_T {
    RF_AT_COMMAND_STOPTEST = 0,
    RF_AT_COMMAND_STARTTX,
    RF_AT_COMMAND_STARTRX,
    RF_AT_COMMAND_RESET,
    RF_AT_COMMAND_NUM
} ENUM_RF_AT_COMMAND_T;


typedef enum _ENUM_RF_AT_FUNCID_T {
    RF_AT_FUNCID_VERSION = 0,
    RF_AT_FUNCID_COMMAND,
    RF_AT_FUNCID_POWER,
    RF_AT_FUNCID_RATE,
    RF_AT_FUNCID_PREAMBLE,
    RF_AT_FUNCID_ANTENNA,
    RF_AT_FUNCID_PKTLEN,
    RF_AT_FUNCID_PKTCNT,
    RF_AT_FUNCID_PKTINTERVAL,
    RF_AT_FUNCID_ALC,
    RF_AT_FUNCID_TXOPLIMIT,
    RF_AT_FUNCID_ACKPOLICY,
    RF_AT_FUNCID_PKTCONTENT,
    RF_AT_FUNCID_RETRYLIMIT,
    RF_AT_FUNCID_QUEUE,
    RF_AT_FUNCID_BANDWIDTH,
    RF_AT_FUNCID_GI,
    RF_AT_FUNCID_STBC,
    RF_AT_FUNCID_IO_PIN_TEST,
    RF_AT_FUNCID_NUM
} ENUM_RF_AT_FUNCID_T;

typedef enum _ENUM_RF_AT_PREAMBLE_T {
    RF_AT_PREAMBLE_NORMAL = 0,
    RF_AT_PREAMBLE_CCK_SHORT,
    RF_AT_PREAMBLE_11N_MM,
    RF_AT_PREAMBLE_11N_GF,
    RF_AT_PREAMBLE_NUM
} ENUM_RF_AT_PREAMBLE_T;

typedef enum _ENUM_RF_AT_ACK_POLICY_T {
    RF_AT_ACK_POLICY_NORMAL = 0,
    RF_AT_ACK_POLICY_NOACK,
    RF_AT_ACK_POLICY_NOEXPLICTACK,
    RF_AT_ACK_POLICY_BLOCKACK,
    RF_AT_ACK_POLICY_NUM
} ENUM_RF_AT_ACK_POLICY_T;

typedef enum _ENUM_RF_AUTOTEST_STATE_T {
    RF_AUTOTEST_STATE_STANDBY = 0,
    RF_AUTOTEST_STATE_TX,
    RF_AUTOTEST_STATE_RX,
    RF_AUTOTEST_STATE_RESET,
    RF_AUTOTEST_STATE_NUM
} ENUM_RF_AUTOTEST_STATE_T;


typedef struct _RFTEST_SETTING_STRUC_T {
    UINT_8                  ucTxPowerGain;
    UINT_8                  ucAntenna;
    UINT_8                  ucRetryLimit;
    UINT_8                  ucTxQue;
    UINT_8                  ucTxRate;
    BOOLEAN                 fgALCEn;
    ENUM_RF_AT_PREAMBLE_T   ePreamble;
    ENUM_RF_AT_ACK_POLICY_T eAckPolicy;
    UINT_32                 u4PacketLen;
    UINT_32                 u4PacketCnt;
    UINT_32                 u4PacketIntervalUs;
    UINT_32                 u4QueTxop;
    UINT_8                  aucPktContent[RF_AT_PARAM_TX_PKTLEN_BYTE_MAX];
} RFTEST_SETTING_STRUC_T, *P_RFTEST_SETTING_STRUC_T;

typedef struct _RFTEST_STATUS_STRUC_T {
    UINT_32                 u4TxedPacketCnt; /* Packet send to s/w queue */
    UINT_32                 u4TxDoneCnt;     /* TxDone Comeback number */
    UINT_32                 u4DSConfig;      /* Channel frequency in unit of kHz */
    UINT_64                 u8RxedPacketCnt; /* RxDone Comeback number */

} RFTEST_STATUS_STRUC_T, *P_RFTEST_STATUS_STRUC_T;


typedef struct _RFTEST_INFO_T {
    ENUM_RF_AUTOTEST_STATE_T    eCurrentState;
    RFTEST_SETTING_STRUC_T      rSetting;
    RFTEST_STATUS_STRUC_T       rStatus;

    /* TX */
    UINT_32                     u4TxCachedSize;
    PUINT_8                     pucTxCached;     /* Memory handler */

    QUE_T                       rTxBufFreeQueue; /* Packet Queue */

    /* RX */
} RFTEST_INFO_T, *P_RFTEST_INFO_T;


typedef struct _RFTEST_TXBUF_T {
    QUE_ENTRY_T     rQueEntry;
    UINT_8          aucData[1];
} RFTEST_TXBUF_T, *P_RFTEST_TXBUF_T;






/* The macro to set the txpower gain to OFPR or CFPR. */
#define RFTEST_SET_TX_POWER_GAIN(_pAdapter, _register, _txGain, _setMask, _setOffset) \
            RFTEST_SET_SUBFIELD_WITH_VALUE(_pAdapter, _register, _txGain, _setMask, _setOffset)

/* The macro to set the Tx count limit to MTCLR. */
#define RFTEST_SET_TX_CNT_LIMIT(_pAdapter, _register, _limit, _setMask, _setOffset) \
                RFTEST_SET_SUBFIELD_WITH_VALUE(_pAdapter, _register, _limit, _setMask, _setOffset)

/* The macro to set the TXOP limit to ACTXOPLR0~1. */
#define RFTEST_SET_TXOP_LIMIT(_pAdapter, _register, _txopLimit, _setMask, _setOffset) \
                RFTEST_SET_SUBFIELD_WITH_VALUE(_pAdapter, _register, _txopLimit, _setMask, _setOffset)


/* The macro to set the sub filed value to specific MCR. */
#define RFTEST_SET_SUBFIELD_WITH_VALUE(_pAdapter, _register, _value, _fieldMask, _fieldOffset) \
        { \
            UINT_32 u4MCR; \
            HAL_MCR_RD((_pAdapter), (_register), (&u4MCR)); \
            u4MCR = (u4MCR & ~ (_fieldMask)) | (((_value)<< (_fieldOffset)) &(_fieldMask) ); \
            HAL_MCR_WR((_pAdapter), (_register), (u4MCR));\
        }

#define RFTEST_BUFENT_GET_BUFFER(_prBufEntry)  \
        (((P_RFTEST_TXBUF_T)(_prBufEntry))->aucData)


/***** Routines in rftest.c *****/

WLAN_STATUS
rftestInit (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
rftestTxModeInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestTx (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestBufFreeRFTestBuf (
    IN P_ADAPTER_T      prAdapter,
    IN P_RFTEST_TXBUF_T prTxBufEntry
    );

WLAN_STATUS
rftestSet80211Configuration (
    IN P_ADAPTER_T prAdapter,
    IN P_PARAM_802_11_CONFIG_T prNewConfig
    );


WLAN_STATUS
rftestQuery80211Configuration (
    IN P_ADAPTER_T prAdapter,
    IN P_PARAM_802_11_CONFIG_T prQueryConfig
    );

WLAN_STATUS
rftestUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestTxStopTx (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestSetSWDefaultValue (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestUpdateRxConfigToHw (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
rftestRxModeInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestRxStopRx (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestProcessRxMPDU (
    IN P_ADAPTER_T prAdapter,
    IN OUT PP_SW_RFB_T pprSWRfb
    );

VOID
rftestRxCheckHwConfig (
    IN P_ADAPTER_T prAdapter
    );

VOID
rftestAdoptBbRfByThermo (
    IN P_ADAPTER_T prAdapter
    );

#endif /* _RFTEST_H */



