





#ifndef _MT592X_HW_H
#define _MT592X_HW_H



#define TFCB_PAYLOAD_LEN_MASK               BITS(0, 11)
#define TFCB_WLAN_HEADER_LEN_MASK           BITS(1, 5)

#define TFCB_FLAG_HEADER_802_11             BIT(4)  /* UINT_16(u2LifeTimeQIdx) */
#define TFCB_FLAG_1X                        BIT(3)  /* UINT_16(u2LifeTimeQIdx) */

#define TFCB_LIFETIME_OFFSET                5       /* UINT_16(u2LifeTimeQIdx) */
#define TFCB_CHECKSUM_OFFSET                12      /* UINT_16(u2PayloadLength) */
#define TFCB_CHECKSUM_BIT_WIDTH             4
#define TFCB_PID_OFFSET                     0       /* UINT_8(ucWlanIdxCSUMPID) */
#define TFCB_WLAN_INDEX_OFFSET              4       /* UINT_8(ucWlanIdxCSUMPID) */
#define TFCB_WLAN_INDEX_DEFAULT             0xF

#define TFCB_QIDX_MASK                      BITS(0, 2)  /* UINT_16(u2LifeTimeQIdx) */
#define TFCB_LIFETIME_MASK                  BITS(5, 15) /* UINT_16(u2LifeTimeQIdx) */
#define TFCB_MAX_LIFETIME_IN_2TU            BITS(0, 10)
#define TFCB_CHKSUM_MASK                    BITS(0, (TFCB_CHECKSUM_BIT_WIDTH-1))
#define TFCB_TID_MASK                       BITS(0, 3)  /* UINT_8(ucFlagTID) */
#define TFCB_PID_MASK                       BITS(0, 1)  /* UINT_8(ucWlanIdxCSUMPID) */

#define TFCB_FLAG_NP                        BIT(4)  /* UINT_8(ucFlagTID) */
#define TFCB_FLAG_BR                        BIT(5)  /* UINT_8 */
#define TFCB_FLAG_DU                        BIT(6)  /* UINT_8 */
#define TFCB_FLAG_NA                        BIT(7)  /* UINT_8 */
#define TFCB_FLAG_TCP_UDP_CSUM              BIT(2)  /* UINT_8(ucWlanIdxCSUMPID) */
#define TFCB_FLAG_IP_CSUM                   BIT(3)  /* UINT_8 */

#define TFCB_SIZE                           sizeof(HW_TFCB_T)


#define TX_STATUS_QIDX_MASK                 BITS(0,2)   /* UINT_8(ucWIdxQIdx) */
#define TX_STATUS_WIDX_MASK                 BITS(4,7)   /* UINT_8(ucWIdxQIdx) */
#define TX_STATUS_WIDX_OFFSET               4           /* UINT_8(ucWIdxQIdx) */
#define TX_STATUS_PID_MASK                  BITS(0,1)   /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_MPDU_TX_COUNT_MASK        BITS(0,2)   /* UINT_8(ucFlagsMPDUTXCount) */
#define TX_STATUS_RTS_FAIL_COUNT_MASK       BITS(2,4)   /* UINT_8(ucRTSCountRateSequence) */
#define TX_STATUS_RTS_OK_COUNT_MASK         BITS(5,7)   /* UINT_8(ucRTSCountRateSequence) */
#define TX_STATUS_RTS_FAIL_COUNT_OFFSET     2
#define TX_STATUS_RTS_OK_COUNT_OFFSET       5


#define TX_STATUS_FLAG_E                    BIT(7)      /* UINT_8(ucFlagsMPDUTXCount) */
#define TX_STATUS_FLAG_BMC                  BIT(6)      /* UINT_8 */
#define TX_STATUS_FLAG_Q                    BIT(5)      /* UINT_8 */
#define TX_STATUS_FLAG_D                    BIT(4)      /* UINT_8 */
#define TX_STATUS_FLAG_M                    BIT(15)     /* UINT_16(u2FlagsALC) */
#define TX_STATUS_FLAG_1X                   BIT(14)     /* UINT_16 */
#define TX_STATUS_FLAG_BR                   BIT(13)     /* UINT_16 */
#define TX_STATUS_FLAG_PS                   BIT(12)     /* UINT_16 */

#define TX_STATUS_STATUS_MASK               BITS(2,7)   /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_MPDU_RETRY_ERR            BIT(2)      /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_RTS_RETRY_ERR             BIT(3)      /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_TX_OK                     BIT(4)      /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_LIFETIME_ERR              BIT(5)      /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_PORT_CONTROL              BIT(6)      /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_ATIM_NOT_ACK              BIT(7)      /* UINT_8(ucStatusIndicationPID) */
#define TX_STATUS_TX_ERR_MASK               (TX_STATUS_MPDU_RETRY_ERR | \
                                             TX_STATUS_RTS_RETRY_ERR | \
                                             TX_STATUS_LIFETIME_ERR | \
                                             TX_STATUS_PORT_CONTROL | \
                                             TX_STATUS_ATIM_NOT_ACK)

#define BCN_TFCB_PAYLOAD_LEN_MASK           BITS(0, 10)
#define BCN_TFCB_RATE_INDEX_OFFSET          16


//4 2006/09/06, mikewu, for pass the code build in Linux
/* Receive Frame Descriptor (RFD) size */
#define RFB_RFD_SZ                      0x08
#define RFB_RX_BUF_MAC_HEADER_SZ        32
#define RFB_RSI_SZ                      48

#define RX_ANT_MODE_DIVERSITY_AGC       0   /* AGC-based Rx antenna diversity */
#define RX_ANT_MODE_DIVERSITY_MPDU      1   /* MPDU-based Rx antenna diversity */
#define RX_ANT_MODE_FIXED_0             2   /* fixed Rx antenna 0 */
#define RX_ANT_MODE_FIXED_1             3   /* fixed Rx antenna 1 */
#define TBB_FLAG_EOL                    BIT(0)
#define DEVID_IDMSK                     BITS(0,15)

#define MCR_GPIOCR                      0x0050
#define GPIOCR_GPIO_1                   BIT(2)
#define GPIOCR_GPIO_0                   BIT(0)

#define RX_STATUS_PACKET_LENGTH_MASK        BITS(0,11)  /* UINT_16(u2PacketLength) */
#define RX_STATUS_BUFFER_LENGTH_MASK        BITS(0,9)   /* UINT_16(u2OverallBufferLengthDW) */


#define RX_STATUS_FLAG_FCS_ERROR      BIT(0)
#define RX_STATUS_FLAG_KEY_ERROR      BIT(1)
#define RX_STATUS_FLAG_FORMAT_ERROR   BIT(2)
#define RX_STATUS_FLAG_ICV_ERROR      BIT(3)
#define RX_STATUS_FLAG_TKIPMIC_ERROR  BIT(4)
#define RX_STATUS_FLAG_DATA_FRAME     BIT(5)
#define RX_STATUS_FLAG_BUFFERED_UCAST BIT(6)
#define RX_STATUS_FLAG_IAPP           BIT(7)
#define RX_STATUS_FLAG_QoS            BIT(8)
#define RX_STATUS_FLAG_A4             BIT(9)
#define RX_STATUS_FLAG_TCL            BIT(10)
#define RX_STATUS_FLAG_BSSID_MATCH    BIT(11)
#define RX_STATUS_FLAG_1X             BIT(12)
#define RX_STATUS_FLAG_WLAN_HEADER    BIT(13)
#define RX_STATUS_FLAG_BMCAST         BIT(14)
#define RX_STATUS_FLAG_MORE_PACKET    BIT(15)

#define RX_STATUS_FLAG_ERROR_MASK     (RX_STATUS_FLAG_FCS_ERROR | RX_STATUS_FLAG_ICV_ERROR | \
                                       RX_STATUS_FLAG_KEY_ERROR | RX_STATUS_FLAG_FORMAT_ERROR) // No TKIP MIC error


#define RX_STATUS_QOS_TID_MASK        0x000F

#define RX_STATUS_FC_TO_DS                  BIT(0)  /* UINT_8(ucFC) */
#define RX_STATUS_FC_FROM_DS                BIT(1)  /* UINT_8 */
#define RX_STATUS_FC_MORE_FRAG              BIT(2)  /* UINT_8 */
#define RX_STATUS_FC_RETRY                  BIT(3)  /* UINT_8 */
#define RX_STATUS_FC_POWER_MANAGEMENT       BIT(4)  /* UINT_8 */
#define RX_STATUS_FC_MORE_DATA              BIT(5)  /* UINT_8 */
#define RX_STATUS_FC_PROTECTION             BIT(6)  /* UINT_8 */
#define RX_STATUS_FC_ORDER                  BIT(7)  /* UINT_8 */

#define RX_STATUS_SEC_MASK            0x0F
#define RX_STATUS_KEYID_MASK          0x30
#define RX_STATUS_SEC_TKIP_NO_MIC     0x02
#define RX_STATUS_SEC_TKIP_MIC        0x03


#define RX_STATUS_FLAG_DROP_MASK    (RX_STATUS_FLAG_FCS_ERR | RX_STATUS_FLAG_FMT_ERR | RX_STATUS FLAG_A4)
#define RX_STATUS_HEADER_OFFSET_MASK  0xC0

#define RX_STATUS_CHKSUM_UDP_FRAME  0x0020
#define RX_STATUS_CHKSUM_TCP_FRAME  0x0010
#define RX_STATUS_CHKSUM_IP_FRAME   0x0008
#define RX_STATUS_CHKSUM_FRAME_MASK 0x0038
#define RX_STATUS_CHKSUM_UDP_OK     0x0004
#define RX_STATUS_CHKSUM_TCP_OK     0x0002
#define RX_STATUS_CHKSUM_IP_OK      0x0001
#define RX_STATUS_CHKSUM_VER_MASK   0x0007

#define RX_STATUS_IS_802_11(flag)           ((flag & RX_STATUS_FLAG_WLAN_HEADER)?TRUE:FALSE)
#define RX_STATUS_IS_A4(flag)               ((flag & RX_STATUS_FLAG_A4)?TRUE:FALSE)
#define RX_STATUS_IS_QoS(flag)              ((flag & RX_STATUS_FLAG_QoS)?TRUE:FALSE)
#define RX_STATUS_IS_BMC(flag)              ((flag & RX_STATUS_FLAG_BMCAST)?TRUE:FALSE)
#define RX_STATUS_IS_TCL(flag)              ((flag & RX_STATUS_FLAG_TCL)?TRUE:FALSE)
#define RX_STATUS_IS_BSM(flag)              ((flag & RX_STATUS_FLAG_BSSID_MATCH)?TRUE:FALSE)
#define RX_STATUS_IS_1X(flag)               ((flag & RX_STATUS_FLAG_1X)?TRUE:FALSE)

#define RX_STATUS_IS_FCS_ERROR(flag)        ((flag & RX_STATUS_FLAG_FCS_ERROR)?TRUE:FALSE)
#define RX_STATUS_IS_ICV_ERROR(flag)        ((flag & RX_STATUS_FLAG_ICV_ERROR)?TRUE:FALSE)
#define RX_STATUS_IS_KEY_ERROR(flag)        ((flag & RX_STATUS_FLAG_KEY_ERROR)?TRUE:FALSE)
#define RX_STATUS_IS_FORMAT_ERROR(flag)     ((flag & RX_STATUS_FLAG_FORMAT_ERROR)?TRUE:FALSE)
#define RX_STATUS_IS_ERROR(flag)            ((flag & RX_STATUS_FLAG_ERROR_MASK)?TRUE:FALSE)
#define RX_STATUS_IS_TKIP_MIC_ERROR(flag)   ((flag & RX_STATUS_FLAG_TKIPMIC_ERROR)>0?TRUE:FALSE)
#define RX_STATUS_IS_DATA_FRAME(flag)       ((flag & RX_STATUS_FLAG_DATA_FRAME)?TRUE:FALSE)

#define RX_STATUS_GET_SEQ_CTRL(_prRxStatus) (((P_RX_STATUS_T)(_prRxStatus))->u2SeqCtrl)

#define RX_STATUS_GET_RCPI(_prRxStatus)     (((P_RX_STATUS_T)(_prRxStatus))->ucRCPI)

#define RX_STATUS_IS_MORE_FRAG(_prRxStatus) \
            ((((P_RX_STATUS_T)(_prRxStatus))->ucFC & RX_STATUS_FC_MORE_FRAG) ? TRUE : FALSE)

#define RX_STATUS_IS_MORE_DATA(_prRxStatus) \
            ((((P_RX_STATUS_T)(_prRxStatus))->ucFC & RX_STATUS_FC_MORE_DATA) ? TRUE : FALSE)

#define RX_STATUS_IS_PROTECT(_prRxStatus) \
            ((((P_RX_STATUS_T)(_prRxStatus))->ucFC & RX_STATUS_FC_PROTECTION) ? TRUE : FALSE)

#define RX_STATUS_GET_TA(_prRxStatus)       (((P_RX_STATUS_T)(_prRxStatus))->aucTA)

#if CFG_LP_PATTERN_SEARCH_SLT

/* The wanted packet of pattern search */
#define RX_STATUS_PACKET_WANTED BIT(6)

#define RX_STATUS_IS_PACKET_WANTED(flag)    ((flag & RX_STATUS_PACKET_WANTED)?TRUE:FALSE)

#endif


#define RX_STATUS_G2_RATE_MASK      0x3F

#define RX_RFB_LEN_FIELD_LEN        4
#define RX_HEADER_OFFSET            2



#define MAX_TXB_LEN                         ALIGN_4(1562) // Round up to DW boundary

#define MAX_RXB_LEN                         ALIGN_4(1622) // Round up to DW boundary

/* 32K memory FIFO in MT5921 */
#define MAX_FIFO_SIZE                       (32 * 1024)

/* Expected maximum packet count for individual ACQ FIFO */
#define HW_Q_SIZE_FOR_AC0                   2
#define HW_Q_SIZE_FOR_AC1                   3
#define HW_Q_SIZE_FOR_AC2                   3 /* For Video Burst in TXOP */
#define HW_Q_SIZE_FOR_AC3                   2
#define HW_Q_SIZE_FOR_AC4                   2
#define HW_Q_SIZE_FOR_TSB                   2
#define HW_Q_SIZE_FOR_RX                    4

/* 391 x 2 = 782 */
#define HW_BUFF_DWSIZE_FOR_AC0              ((MAX_TXB_LEN / 4) * HW_Q_SIZE_FOR_AC0)
/* 391 x 3 = 1173 */
#define HW_BUFF_DWSIZE_FOR_AC1              ((MAX_TXB_LEN / 4) * HW_Q_SIZE_FOR_AC1)
/* 391 x 3 = 1173 */
#define HW_BUFF_DWSIZE_FOR_AC2              ((MAX_TXB_LEN / 4) * HW_Q_SIZE_FOR_AC2)
/* 391 x 2 = 782 */
#define HW_BUFF_DWSIZE_FOR_AC3              ((MAX_TXB_LEN / 4) * HW_Q_SIZE_FOR_AC3)

/* 391 x 2 = 782 */
//#define HW_BUFF_DWSIZE_FOR_AC4              ((MAX_TXB_LEN / 4) * HW_Q_SIZE_FOR_AC4)
#define HW_BUFF_DWSIZE_FOR_AC4              (512) // NOTE(Kevin 2007/12/07): 1K is enough for MGMT frame.

/* 391 x 2 = 782 */
//#define HW_BUFF_DWSIZE_FOR_TSB              ((MAX_TXB_LEN / 4) * HW_Q_SIZE_FOR_TSB)
#define HW_BUFF_DWSIZE_FOR_TSB              (192) // NOTE(Kevin 2007/12/07): 768 Bytes is enough for Beacon frame.

/* FIFO for scan */
#define HW_BUFF_DWSIZE_FOR_SCAN             (256)

/* FIFO for scan control */
#define HW_BUFF_DWSIZE_FOR_SCAN_CTRL        (64)

/* 406 x 4 = 1624 */
//#define HW_BUFF_DWSIZE_FOR_RX               ((MAX_RXB_LEN / 4) * HW_Q_SIZE_FOR_RX)
#define HW_BUFF_DWSIZE_FOR_RX               (BYTE_TO_DWORD(MAX_FIFO_SIZE) - \
                                             HW_BUFF_DWSIZE_FOR_AC0 - \
                                             HW_BUFF_DWSIZE_FOR_AC1 - \
                                             HW_BUFF_DWSIZE_FOR_AC2 - \
                                             HW_BUFF_DWSIZE_FOR_AC3 - \
                                             HW_BUFF_DWSIZE_FOR_AC4 - \
                                             HW_BUFF_DWSIZE_FOR_TSB - \
                                             HW_BUFF_DWSIZE_FOR_SCAN - \
                                             HW_BUFF_DWSIZE_FOR_SCAN_CTRL)

/* juji: sanity check of the memory partition */
#if 0
#if ((HW_BUFF_DWSIZE_FOR_AC0 + \
      HW_BUFF_DWSIZE_FOR_AC1 + \
      HW_BUFF_DWSIZE_FOR_AC2 + \
      HW_BUFF_DWSIZE_FOR_AC3 + \
      HW_BUFF_DWSIZE_FOR_AC4 + \
      HW_BUFF_DWSIZE_FOR_TSB + \
      HW_BUFF_DWSIZE_FOR_RX + \
      HW_BUFF_DWSIZE_FOR_SCAN + \
      HW_BUFF_DWSIZE_FOR_SCAN_CTRL) > (MAX_FIFO_SIZE / 4))
    #error "ERROR! 32K bytes memory usage overflow !"
#endif
#endif


#if CFG_SDIO_STATUS_ENHANCE
#if defined(_HIF_SDIO) && defined (WINDOWS_DDK)
/*On XP, maximum Tx+Rx Statue <= 64-4(HISR)*/
#define SDIO_MAXIMUM_RX_STATUS              10 /* have to be 2x, max 14 */
#define SDIO_MAXIMUM_TX_STATUS              5  /* max 15 */
#else
#define SDIO_MAXIMUM_RX_STATUS              6 /* have to be 2x, max 14 */
#define SDIO_MAXIMUM_TX_STATUS              6 /* max 15 */
#endif
#endif /* CFG_SDIO_STATUS_ENHANCE */


/* The rate define at MT5921 spec */
#define RATE_CCK_1M_LONG              0x00
#define RATE_CCK_2M_LONG              0x01
#define RATE_CCK_5_5M_LONG            0x02
#define RATE_CCK_11M_LONG             0x03
#define RATE_CCK_2M_SHORT             0x11
#define RATE_CCK_5_5M_SHORT           0x12
#define RATE_CCK_11M_SHORT            0x13
#define RATE_OFDM_6M                  0x2B
#define RATE_OFDM_9M                  0x2F
#define RATE_OFDM_12M                 0x2A
#define RATE_OFDM_18M                 0x2E
#define RATE_OFDM_24M                 0x29
#define RATE_OFDM_36M                 0x2D
#define RATE_OFDM_48M                 0x28
#define RATE_OFDM_54M                 0x2C

#define RATE_CCK_SHORT_PREAMBLE       0x10

/* ACK Rate definition */
#define ACK_RATE_CCK_1M              0x00
#define ACK_RATE_CCK_2M              0x01
#define ACK_RATE_CCK_5_5M            0x02
#define ACK_RATE_CCK_11M             0x03
#define ACK_RATE_OFDM_6M             0x1B
#define ACK_RATE_OFDM_9M             0x1F
#define ACK_RATE_OFDM_12M            0x1A
#define ACK_RATE_OFDM_18M            0x1E
#define ACK_RATE_OFDM_24M            0x19
#define ACK_RATE_OFDM_36M            0x1D
#define ACK_RATE_OFDM_48M            0x18
#define ACK_RATE_OFDM_54M            0x1C

#define WLAN_TABLE_SIZE               13
#define WLAN_TABLE_SET_ALL            15
#define WLAN_TABLE_DEFAULT_ENTRY      0
/* used as the key handshake template key, and for legacy 1x sendout without encrypt */
#define WLAN_TABLE_TEMPLATE_ENTRY     1

/* cipher suite selectors define at MT5921 spec */
#define CIPHER_SUITE_NONE               0
#define CIPHER_SUITE_WEP40              1
#define CIPHER_SUITE_TKIP               2
#define CIPHER_SUITE_TKIP_WO_MIC        3
#define CIPHER_SUITE_CCMP               4
#define CIPHER_SUITE_WEP104             5
#define CIPHER_SUITE_WEP128             7

#define HW_PATTERN_SEARCH_SET_NUMBER    32


/* IO PIN mode selection */
#define IO_TRSW_P_DIR                   1   /* 1: output,
                                               0: input */

#define IO_TRSW_N_DIR                   1   /* 1: output,
                                               0: input */

#define IO_ANT_SEL_P_DIR                0   /* 1: output,
                                               0: input */

#define IO_ANT_SEL_N_DIR                0   /* 1: output,
                                               0: input */

#define IO_WLAN_ACT_DIR                 1   /* 1: output,
                                               0: input */

#define IO_SET_TRAP_PIN_DEFAULT_ATTR    ((IO_TRSW_P_DIR     << 12) | \
                                         (IO_TRSW_N_DIR     << 11) | \
                                         (IO_ANT_SEL_P_DIR  << 10) | \
                                         (IO_ANT_SEL_N_DIR  << 9)  | \
                                         (IO_WLAN_ACT_DIR   << 8))

#define IO_SET_TRAP_PIN_INPUT_DIR       ((IO_WLAN_ACT_DIR   << 8))

#define IO_SET_DBG_PORT_SEL             (0x0)

/* A data structure which is identical with HW MAC TX descriptor */
typedef struct _HW_TFCB_T {
    UINT_8          ucDA[4];
    UINT_16         u2WlanHeaderLength;
    UINT_16         u2LifeTimeQIdx;
    UINT_16         u2PayloadLength;
    UINT_8          ucFlagTID;
    UINT_8          ucWlanIdxCSUMPID;
} HW_TFCB_T, *P_HW_TFCB_T;


/* A data structure which is identical with HW TX status */
typedef struct _TX_STATUS_T {
    UINT_8          ucWIdxQIdx;
    UINT_8          ucStatusIndicationPID;
    UINT_16         u2TransmissionDelay;
    UINT_8          ucFlagsMPDUTXCount;
    UINT_8          ucRTSCountRateSequence;
    UINT_16         u2FlagsALC;
} TX_STATUS_T, *P_TX_STATUS_T;


/* A data structure which is identical with HW RX status */
typedef struct _RX_STATUS_T {
    UINT_16    u2PacketLength;
    UINT_16    u2OverallBufferLengthDW;
    UINT_16    u2QoSCtrl;
    UINT_16    u2StatusFlag;
    UINT_16    u2SeqCtrl;
    UINT_8     ucRCPI;
    UINT_8     ucFC;
    UINT_8     ucWlanIdx;
    UINT_8     ucKIdxSecMode;
    UINT_8     aucRSC[6];
    UINT_8     aucTA[6];
    UINT_8     ucDropPacketNum;
    UINT_8     ucReserved;
#if 0
    P_RX_STATUS_G0_T    prG0;
    P_RX_STATUS_G1_T    prG1;
    P_RX_STATUS_G2_T    prG2;
#endif

} RX_STATUS_T, *P_RX_STATUS_T;


typedef struct _RX_STATUS_G0_T {
    UINT_32 u4TimestampLow;
    UINT_32 u4TimestampHigh;
} RX_STATUS_G0_T, *P_RX_STATUS_G0_T;


typedef struct _RX_STATUS_G1_T {
    UINT_32 u4CRC;
} RX_STATUS_G1_T, *P_RX_STATUS_G1_T;


typedef struct _RX_STATUS_G2_T {
    UINT_8  ucLQ0;
    UINT_8  ucLQ1;
    UINT_8  ucLQ2;
    UINT_8  ucLQ3;
    UINT_8  ucLQ4;
    UINT_8  ucLQ5;
    UINT_16 u2NFRate;
} RX_STATUS_G2_T, *P_RX_STATUS_G2_T;






/* wlan structure for wlan setting and key configuration */
typedef struct _HW_WLAN_TABLE_T {
    UINT_32      u4Ctrl;                /*!< First DW, including index, mode,
                                             the wlan table ctrol bit field,
                                             cipher and key id */
    UINT_32      u4Rate;     /*!< Rate 1 ~ Rate 4, and R */
    UINT_8       aucAddr[MAC_ADDR_LEN]; /*!< mapping address */
    UINT_8       ucMUAR_ID;  /*!< MUAR Index */
    UINT_8       ucReserved;
    UINT_8       aucKey[32]; /*!< Key material */
} HW_WLAN_TABLE_T, *P_HW_WLAN_TABLE_T;


#if CFG_SDIO_STATUS_ENHANCE
typedef struct _ISAR_BIND_STATUS_T {
    UINT_32            u4Hisr;
    UINT_16            au2RxLengthDW[SDIO_MAXIMUM_RX_STATUS];
    TX_STATUS_T        arTxStatus[SDIO_MAXIMUM_TX_STATUS];
} ISAR_BIND_STATUS_T, *P_ISAR_BIND_STATUS_T;
#endif /* CFG_SDIO_STATUS_ENHANCE */


/* Number of Interrupt status registers */
typedef enum _ENUM_INT_T {
    INT_HISR = 0,
    INT_HSCISR,
    INT_NUM
} ENUM_INT_T, *P_ENUM_INT_T;



#if defined(_HIF_EHPI) || defined(_HIF_SPI)
    #define TFCB_FRAME_PAD_TO_DW(u2Length)      ALIGN_4(u2Length)
#else
    #define TFCB_FRAME_PAD_TO_DW(u2Length)      (u2Length)
#endif /* _HIF_EHPI */

#define TX_STATUS_GET_QUEUE_INDEX(prTxStatus) \
    (((P_TX_STATUS_T)(prTxStatus))->ucWIdxQIdx & TX_STATUS_QIDX_MASK)

#define TX_STATUS_GET_WLAN_INDEX(prTxStatus) \
    ((((P_TX_STATUS_T)(prTxStatus))->ucWIdxQIdx & TX_STATUS_WIDX_MASK) >> TX_STATUS_WIDX_OFFSET)

#define TX_STATUS_TEST_OK_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucStatusIndicationPID & TX_STATUS_TX_OK) ? TRUE : FALSE))

#define TX_STATUS_TEST_ERR_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucStatusIndicationPID & TX_STATUS_TX_ERR_MASK) ? TRUE : FALSE))

#define TX_STATUS_TEST_PORT_CTRL_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucStatusIndicationPID & TX_STATUS_PORT_CONTROL) ? TRUE : FALSE))

#define TX_STATUS_TEST_LIFETIME_ERR_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucStatusIndicationPID & TX_STATUS_LIFETIME_ERR) ? TRUE : FALSE))

#define TX_STATUS_TEST_RTS_RETRY_ERR_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucStatusIndicationPID & TX_STATUS_RTS_RETRY_ERR) ? TRUE : FALSE))

#define TX_STATUS_TEST_MPDU_RETRY_ERR_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucStatusIndicationPID & TX_STATUS_MPDU_RETRY_ERR) ? TRUE : FALSE))

#define TX_STATUS_TEST_BMC_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucFlagsMPDUTXCount & TX_STATUS_FLAG_BMC) ? TRUE : FALSE))

#define TX_STATUS_TEST_DATA_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucFlagsMPDUTXCount & TX_STATUS_FLAG_D) ? TRUE : FALSE))

#define TX_STATUS_TEST_QOS_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->ucFlagsMPDUTXCount & TX_STATUS_FLAG_Q) ? TRUE : FALSE))

#define TX_STATUS_GET_MPDU_TX_CNT(prTxStatus) \
    (((P_TX_STATUS_T)(prTxStatus))->ucFlagsMPDUTXCount & TX_STATUS_MPDU_TX_COUNT_MASK)

#define TX_STATUS_GET_RTS_OK_CNT(prTxStatus) \
    ((((P_TX_STATUS_T)(prTxStatus))->ucRTSCountRateSequence & TX_STATUS_RTS_OK_COUNT_MASK) >> TX_STATUS_RTS_OK_COUNT_OFFSET)

#define TX_STATUS_GET_RTS_FAIL_CNT(prTxStatus) \
    ((((P_TX_STATUS_T)(prTxStatus))->ucRTSCountRateSequence & TX_STATUS_RTS_FAIL_COUNT_MASK) >> TX_STATUS_RTS_FAIL_COUNT_OFFSET)

#define TX_STATUS_TEST_MORE_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->u2FlagsALC & TX_STATUS_FLAG_M) ? TRUE : FALSE))

#define TX_STATUS_GET_CURRENT_PS_STATE(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->u2FlagsALC & TX_STATUS_FLAG_PS) ? TRUE : FALSE))

#define TX_STATUS_TEST_1X_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->u2FlagsALC & TX_STATUS_FLAG_1X) ? TRUE : FALSE))

#define TX_STATUS_TEST_BR_FLAG(prTxStatus) \
    ((BOOLEAN)((((P_TX_STATUS_T)(prTxStatus))->u2FlagsALC & TX_STATUS_FLAG_BR) ? TRUE : FALSE))

#define RX_STATUS_TEST_MORE_FLAG(prRxStatus) \
    ((BOOLEAN)((((P_RX_STATUS_T)(prRxStatus))->u2StatusFlag & RX_STATUS_FLAG_MORE_PACKET) ? TRUE : FALSE))


#define LP_INSTR_MCR_WR_CODE        (0x1 << 28)
#define LP_INSTR_RFCR_WR_CODE       (0x3 << 28)
#define LP_INSTR_CLK_OFF_CODE       ((UINT_32)0xc << 28)
#define LP_INSTR_CLK_ON_CODE        ((UINT_32)0xd << 28)
#define LP_INSTR_DELAY_CODE         (0x6 << 28)
#define LP_INSTR_SCAN_CODE          (0x5 << 28)

#define LP_INSTR_MCR_WR_ADDR_MASK   BITS(16, 27)
#define LP_INSTR_MCR_WR_DATA_MASK   BITS(0, 15)

#define LP_INSTR_RF_WR_DATA_MASK    BITS(0, 23)

#define LP_INSTR_DELAY_P2_SKIP      BIT(15)
#define LP_INSTR_DELAY_UNIT_TU      BIT(14)

#define LP_INSTR_SCAN_ACTIVE_SCAN       BIT(27)
#define LP_INSTR_SCAN_CH_SW_NUM_MASK    BITS(0, 7)
#define LP_INSTR_SCAN_BD_SW_NUM_MASK    BITS(8, 15)
#define LP_INSTR_SCAN_TOTAL_CH_NUM_MASK   BITS(16, 23)

#define LP_INST_MCR_WR(_register, _value, _out_low_word,_out_high_word) \
        (_out_low_word) =   (LP_INSTR_MCR_WR_CODE                   | \
                             (((_register) << 16)                   & LP_INSTR_MCR_WR_ADDR_MASK) | \
                             (((_value) & BITS(0,15))               & LP_INSTR_MCR_WR_DATA_MASK)); \
        (_out_high_word) =  (LP_INSTR_MCR_WR_CODE                   | \
                             (((_register + 2) << 16)               & LP_INSTR_MCR_WR_ADDR_MASK) | \
                             ((((_value) & BITS(16,31)) >> 16)      & LP_INSTR_MCR_WR_DATA_MASK))

#define LP_INST_BBCR_WR(_register, _value, _out_lower_dword) \
                            (LP_INSTR_MCR_WR_CODE | \
                             (((_register) << 16)        & LP_INSTR_MCR_WR_ADDR_MASK) | \
                             (((_value) & BITS(0,15))    & LP_INSTR_MCR_WR_DATA_MASK))

#define LP_INST_RFCR_WR(_value) \
                            (LP_INSTR_RFCR_WR_CODE | \
                             ((_value) & LP_INSTR_RF_WR_DATA_MASK))

#define LP_INST_MCR_WR_HALF_WORD(_register, _fgIsHighWord, _value) \
                            (LP_INSTR_MCR_WR_CODE | \
                                ((((_register) + (((BOOLEAN)_fgIsHighWord) << 1)) << 16) & LP_INSTR_MCR_WR_ADDR_MASK) | \
                                ((_value) & LP_INSTR_MCR_WR_DATA_MASK))

#define LP_INST_CLOCK_ON(_AS, _BS, _OS, _PS,    _A, _B, _O, _P,     _SS, _S, _CS, _C_32K,   _DS, _D_TU, _delayCount) \
                            (LP_INSTR_CLK_ON_CODE | \
                                (_AS    << 27) | \
                                (_BS    << 26) | \
                                (_OS    << 25) | \
                                (_PS    << 24) | \
                                (_A     << 23) | \
                                (_B     << 22) | \
                                (_O     << 21) | \
                                (_P     << 20) | \
                                (_SS    << 19) | \
                                (_S     << 18) | \
                                (_CS    << 17) | \
                                (_C_32K << 16) | \
                                (_DS    << 15) | \
                                (_D_TU  << 14) | \
                                ((_delayCount) & BITS(0, 13)))

#define LP_INST_CLOCK_OFF(_AS, _BS, _OS, _PS,   _A, _B, _O, _P,     _SS, _S, _CS, _C_32K) \
                            (LP_INSTR_CLK_OFF_CODE | \
                                (_AS    << 27) | \
                                (_BS    << 26) | \
                                (_OS    << 25) | \
                                (_PS    << 24) | \
                                (_A     << 23) | \
                                (_B     << 22) | \
                                (_O     << 21) | \
                                (_P     << 20) | \
                                (_SS    << 19) | \
                                (_S     << 18) | \
                                (_CS    << 17) | \
                                (_C_32K << 16))

#if 0//
#define LP_INST_DELAY(_value, _phase2Skip, _unitTu) \
                                (LP_INSTR_DELAY_CODE | \
                                 ((_phase2Skip) ? LP_INSTR_DELAY_P2_SKIP : 0) | \
                                 ((_unitTu) ? LP_INSTR_DELAY_UNIT_TU : 0) | \
                                 (_value))

#define LP_INST_SCAN_CTRL(_scanType, _totalChnlNum, _bandSwNum, _chnlSwNum) \
                                (LP_INSTR_SCAN_CODE | \
                                 ((_scanType == ENUM_SCAN_TYPE_ACTIVE_SCAN) ? LP_INSTR_SCAN_ACTIVE_SCAN : 0) \
                                 ((_chnlSwNum << 0) & LP_INSTR_SCAN_CH_SW_NUM_MASK) | \
                                 ((_bandSwNum << 8) & LP_INSTR_SCAN_CH_SW_NUM_MASK) | \
                                 ((_totalChnlNum << 16) & LP_INSTR_SCAN_CH_SW_NUM_MASK))
#else
#define LP_INST_DELAY(_value, _phase2Skip, _unitTu) \
                                (LP_INSTR_DELAY_CODE | \
                                 (_phase2Skip << 15) | \
                                 (_unitTu << 14) | \
                                 (_value))

#define LP_INST_SCAN_CTRL(_scanTypeActive, _totalChnlNum, _bandSwNum, _chnlSwNum) \
                                (LP_INSTR_SCAN_CODE | \
                                 (_scanTypeActive << 27) | \
                                 ((_chnlSwNum << 0) & LP_INSTR_SCAN_CH_SW_NUM_MASK) | \
                                 ((_bandSwNum << 8) & LP_INSTR_SCAN_BD_SW_NUM_MASK) | \
                                 ((_totalChnlNum << 16) & LP_INSTR_SCAN_TOTAL_CH_NUM_MASK))
#endif

#define HAL_HW_SCAN_INST_START(prAdapter, u4QueueDwStart) \
        { \
            HAL_MCR_WR(prAdapter, MCR_HFCR, HFCR_FIFO_WRITE | u4QueueDwStart); \
            u4ScanInstCount = 0; \
            fgScanInited = TRUE; \
        }

#define HAL_HW_SCAN_INST_END(prAdapter, u4QueueId, u4QueueDwStart, u4QueueSizeQuota) \
        if (fgScanInited) { \
            /* program queue start/end address */ \
            HAL_MCR_WR(prAdapter, MCR_QAR, \
                        (u4QueueDwStart | ((u4QueueDwStart + u4ScanInstCount - 1) << 13) | \
                        (u4QueueId << 28))); \
            DBGLOG(HAL, TRACE, ("W MCR_QAR: 0x%lx (s:0x%lx, e:0x%lx)\n", \
                (u4QueueDwStart | ((u4QueueDwStart + u4ScanInstCount - 1) << 13) | \
                (u4QueueId << 28)), u4QueueDwStart, \
                (u4QueueDwStart + u4ScanInstCount - 1))); \
            /* check if the instruction overflow */ \
            if (u4ScanInstCount > u4QueueSizeQuota) { \
                ERRORLOG(("Instruction overflown")); \
                ASSERT(0); \
            } \
            fgScanInited = FALSE; \
        }

#define HAL_HW_SCAN_SET_INST_MCR_WR(prAdapter, u4RegOffset, u4RegValue, fgHiWdEn, fgLoWdEn) \
        if (fgScanInited) { \
            UINT_32 u4Buffer[2]; \
            LP_INST_MCR_WR(u4RegOffset, u4RegValue, u4Buffer[0], u4Buffer[1]); \
            if (fgLoWdEn) { \
                HAL_MCR_WR(prAdapter, MCR_HFDR, u4Buffer[0]); \
                u4ScanInstCount ++; \
            } \
            if (fgHiWdEn) { \
                HAL_MCR_WR(prAdapter, MCR_HFDR, u4Buffer[1]); \
                u4ScanInstCount ++; \
            } \
        }

#if DBG
#define HAL_HW_SCAN_GET_INST_NUMBER_INIT(prAdapter, pu4FifoWrPtr) \
        if (fgScanInited) { \
            HAL_MCR_RD(prAdapter, MCR_HFCR, pu4FifoWrPtr); \
            *pu4FifoWrPtr &= HFCR_FIFO_ADDR_MASK; \
        }

#define HAL_HW_SCAN_GET_INST_NUMBER_END(prAdapter, u4OrgFifoWrPtr, pu4UsedInstNum) \
        if (fgScanInited) { \
            UINT_32 u4CurFifoWrPr; \
            HAL_MCR_RD(prAdapter, MCR_HFCR, &u4CurFifoWrPr); \
            u4CurFifoWrPr &= HFCR_FIFO_ADDR_MASK; \
            *pu4UsedInstNum = u4CurFifoWrPr - u4OrgFifoWrPtr; \
        }
#else
#define HAL_HW_SCAN_GET_INST_NUMBER_INIT(prAdapter, pu4FifoWrPtr) \
        if (fgScanInited) { \
            *pu4FifoWrPtr = u4ScanInstCount & HFCR_FIFO_ADDR_MASK; \
        }

#define HAL_HW_SCAN_GET_INST_NUMBER_END(prAdapter, u4OrgFifoWrPtr, pu4UsedInstNum) \
        if (fgScanInited) { \
            UINT_32 u4CurFifoWrPr; \
            u4CurFifoWrPr = u4ScanInstCount & HFCR_FIFO_ADDR_MASK; \
            *pu4UsedInstNum = u4CurFifoWrPr - u4OrgFifoWrPtr; \
        }
#endif

#if MT5911_BB
#define HAL_HW_SCAN_SET_INST_BBCR_WR(prAdapter, u4RegOffset, u4RegValue) \
        if (fgScanInited) { \
            UINT_32 u4Buffer[1]; \
            LP_INST_BBCR_WR(u4RegOffset, u4RegValue, u4Buffer[0]); \
            HAL_MCR_WR(prAdapter, MCR_HFDR, u4Buffer[0]); \
            u4ScanInstCount ++; \
        }

#define HAL_HW_SCAN_SET_INST_RFCR_WR(prAdapter, u4RegValue) \
        if (fgScanInited) { \
            HAL_MCR_WR(prAdapter, MCR_HFDR, LP_INST_RFCR_WR(u4RegValue)); \
            u4ScanInstCount ++; \
        }
#endif
#if 1// temp! remove later
#define HAL_HW_SCAN_SET_INST_RFCR_WR(prAdapter, u4RegValue) \
        if (fgScanInited) { \
            HAL_MCR_WR(prAdapter, MCR_HFDR, LP_INST_RFCR_WR(u4RegValue)); \
            u4ScanInstCount ++; \
        }
#endif
#if MT5921_BB
#define HAL_HW_SCAN_SET_INST_BBCR_WR(prAdapter, u4RegOffset, u4RegValue) \
        if (fgScanInited) { \
            HAL_MCR_WR(prAdapter, MCR_HFDR, \
                LP_INST_BBCR_WR(u4RegOffset << 2, u4RegValue)); \
            u4ScanInstCount ++; \
        }

#define HAL_HW_SCAN_SET_INST_RFCR_WR(prAdapter, u4RegOffset, u4RegValue) \
        if (fgScanInited) { \
            HAL_MCR_WR(prAdapter, MCR_HFDR, LP_RFCR_WR(u4RegValue)); \
            u4ScanInstCount ++; \
        }
#endif

#define HAL_HW_SCAN_RESTORE_INST_SCAN_CTRL(prAdapter,rScanType, u4TotalChnlNum, u4BandSwNum, u4ChSwNum, u4ScanCtrlWrPr) \
        if (fgScanInited) { \
            UINT_32 u4CurFifoWrPr; \
            HAL_MCR_RD(prAdapter, MCR_HFCR, &u4CurFifoWrPr); \
            HAL_MCR_WR(prAdapter, MCR_HFCR, HFCR_FIFO_WRITE | (u4ScanCtrlWrPr & HFCR_FIFO_ADDR_MASK)); \
            HAL_MCR_WR(prAdapter, MCR_HFDR, \
                        LP_INST_SCAN_CTRL(rScanType, u4TotalChnlNum, u4BandSwNum, u4ChSwNum)); \
            HAL_MCR_WR(prAdapter, MCR_HFCR, HFCR_FIFO_WRITE | (u4CurFifoWrPr & HFCR_FIFO_ADDR_MASK)); \
        }

#define HAL_HW_SCAN_PRESERVE_INST_SCAN_CTRL(prAdapter, pu4ScanCtrlWrPr) \
        if (fgScanInited) { \
            HAL_MCR_RD(prAdapter, MCR_HFCR, pu4ScanCtrlWrPr); \
            /* dummy instruction */ \
            HAL_MCR_WR(prAdapter, MCR_HFDR, LP_INST_DELAY(10, FALSE, FALSE)); \
            u4ScanInstCount ++; \
        }

#define HAL_HW_SCAN_SET_INST_DELAY(prAdapter, u2Delay, fgUnitTu) \
        if (fgScanInited) { \
            HAL_MCR_WR(prAdapter, \
                       MCR_HFDR, \
                       LP_INST_DELAY(u2Delay, FALSE, fgUnitTu)); \
            u4ScanInstCount ++; \
        }

#define LP_INST_SLEEP_to_WLAN_ON_ADDR(_startAddr, _numInst) \
            ((((_startAddr) << 24) & BITS(24, 31)) | (((_startAddr + _numInst - 1) << 16) & BITS(16, 23)))

#define LP_INST_WLAN_ON_to_ON_ADDR(_startAddr, _numInst) \
            ((((_startAddr) << 8) & BITS(8, 15)) | (((_startAddr + _numInst - 1) << 0) & BITS(0, 7)))


#define LP_INST_WLAN_ON_to_SLEEP_ADDR(_startAddr, _numInst) \
            ((((_startAddr) << 24) & BITS(24, 31)) | (((_startAddr + _numInst - 1) << 16) & BITS(16, 23)))

#define LP_INST_INITIAL_to_SLEEP_ADDR(_startAddr, _numInst) \
            ((((_startAddr) << 8) & BITS(8, 15)) | (((_startAddr + _numInst - 1) << 0) & BITS(0, 7)))


__KAL_INLINE__ VOID
mt592x_hwDataTypeCheck (
    VOID
    )
{

    DATA_STRUC_INSPECTING_ASSERT(sizeof(HW_TFCB_T) == 12);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(TX_STATUS_T) == 8);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(RX_STATUS_T) == 28);

#if CFG_SDIO_STATUS_ENHANCE
    #if defined(_HIF_SDIO) && defined (WINDOWS_DDK)
        /* NOTE(Kevin): Because XP will use Byte Mode = 64Bytes in Bus Driver.
         * If Enhance Length > 64, Bus driver will issue Two "Byte Mode CMD" and
         * cause read wrong data.
         */
        DATA_STRUC_INSPECTING_ASSERT(sizeof(ISAR_BIND_STATUS_T) == 64);
    #endif
#endif /* CFG_SDIO_STATUS_ENHANCE */

    return;
}

#endif /* _MT592X_HW_H */

