




#ifndef _MAC_H
#define _MAC_H



//3 /* --------------- Constants for Ethernet/802.11 MAC --------------- */
/* MAC Address */
#define MAC_ADDR_LEN                            6

#define ETH_P_IPX                               0x8137 // Novell IPX
#define ETH_P_AARP                              0x80F3 // AppleTalk Address Resolution Protocol (AARP)
#define LLC_LEN                                 8 // LLC(3) + SNAP(3) + EtherType(2)

#define NULL_MAC_ADDR                           {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define BC_MAC_ADDR                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

/* Ethernet Frame Field Size, in byte */
#define ETHER_HEADER_LEN                        14
#define ETHER_TYPE_LEN                          2

#if SUPPORT_WAPI
/* WPI Frame Format */
#define KEYID_LEN                               1
#define KEYID_RSV_LEN                           1
#define PN_LEN                                  16
#define PDU_LEN                                 2
#define WPI_MIC_LEN                             16
#endif

/* IEEE 802.11 WLAN Frame Field Size, in byte */
#define WLAN_MAC_HEADER_LEN                     24 /* Address 4 excluded */
#define WLAN_MAC_HEADER_A4_LEN                  30 /* Address 4 included */
#define WLAN_MAC_HEADER_QOS_LEN                 26 /* QoS Control included */
#define WLAN_MAC_HEADER_A4_QOS_LEN              32 /* Address 4 and QoS Control included */
#define WLAN_MAC_MGMT_HEADER_LEN                24 /* Address 4 excluded */

/* 6.2.1.1.2 Semantics of the service primitive */
#define MSDU_MAX_LENGTH                         2304

/* 7.1.3.3.3 Broadcast BSSID */
#define BC_BSSID                                BC_MAC_ADDR

/* 7.1.3.7 FCS field */
#define FCS_LEN                                 4

/* 7.3.1.6 Listen Interval field */
#define DEFAULT_LISTEN_INTERVAL_BY_DTIM_PERIOD  2 // In unit of AP's DTIM interval,
#define DEFAULT_LISTEN_INTERVAL                 10

/* 7.3.2.1 Broadcast(Wildcard) SSID */
#define BC_SSID                                 ""
#define BC_SSID_LEN                             0

/* 7.3.2.2 Data Rate Value */
#define RATE_1M                                 2   /* 1M in unit of 500kb/s */
#define RATE_2M                                 4   /* 2M */
#define RATE_5_5M                               11  /* 5.5M */
#define RATE_11M                                22  /* 11M */
#define RATE_22M                                44  /* 22M */
#define RATE_33M                                66  /* 33M */
#define RATE_6M                                 12  /* 6M */
#define RATE_9M                                 18  /* 9M */
#define RATE_12M                                24  /* 12M */
#define RATE_18M                                36  /* 18M */
#define RATE_24M                                48  /* 24M */
#define RATE_36M                                72  /* 36M */
#define RATE_48M                                96  /* 48M */
#define RATE_54M                                108 /* 54M */
#define RATE_MASK                               BITS(0,6)   /* mask bits for the rate */
#define RATE_BASIC_BIT                          BIT(7)      /* mask bit for the rate belonging to the BSSBasicRateSet */

/* 8.3.2.2 TKIP MPDU formats */
#define TKIP_MIC_LEN                            8

/* 9.2.10 DIFS */
#define DIFS                                    2   /* 2 x aSlotTime */

/* 15.4.8.5 802.11k RCPI-dBm mapping*/
#define NDBM_LOW_BOUND_FOR_RCPI                 110
#define RCPI_LOW_BOUND                          0
#define RCPI_HIGH_BOUND                         220
#define RCPI_MEASUREMENT_NOT_AVAILABLE          255


/* PHY characteristics */
/* 17.4.4/18.3.3/19.8.4 Slot Time (aSlotTime) */
#define SLOT_TIME_LONG                          20  /* Long Slot Time */
#define SLOT_TIME_SHORT                         9   /* Short Slot Time */

#define SLOT_TIME_HR_DSSS                       SLOT_TIME_LONG  /* 802.11b aSlotTime */
#define SLOT_TIME_OFDM                          SLOT_TIME_SHORT /* 802.11a aSlotTime(20M Spacing) */
#define SLOT_TIME_OFDM_10M_SPACING              13              /* 802.11a aSlotTime(10M Spacing) */
#define SLOT_TIME_ERP_LONG                      SLOT_TIME_LONG  /* 802.11g aSlotTime(Long) */
#define SLOT_TIME_ERP_SHORT                     SLOT_TIME_SHORT /* 802.11g aSlotTime(Short) */

/* 17.4.4/18.3.3/19.8.4 Contention Window (aCWmin & aCWmax) */
#define CWMIN_OFDM                              15      /* 802.11a aCWmin */
#define CWMAX_OFDM                              1023    /* 802.11a aCWmax */

#define CWMIN_HR_DSSS                           31      /* 802.11b aCWmin */
#define CWMAX_HR_DSSS                           1023    /* 802.11b aCWmax */

#define CWMIN_ERP_0                             31      /* 802.11g aCWmin(0) - for only have 1/2/5/11Mbps Rates */
#define CWMIN_ERP_1                             15      /* 802.11g aCWmin(1) */
#define CWMAX_ERP                               1023    /* 802.11g aCWmax */

/* Short Inter-Frame Space (aSIFSTime) */
/* 15.3.3 802.11b aSIFSTime */
#define SIFS_TIME_HR_DSSS                       10
/* 17.4.4 802.11a aSIFSTime */
#define SIFS_TIME_OFDM                          16
/* 19.8.4 802.11g aSIFSTime */
#define SIFS_TIME_ERP                           10


#if CFG_ONLY_802_11A
/* Annex J - Channel set with 20MHz Channel spacing  */
#define CH_240                                  0xf0 // 5G + (-16)*5M
#define CH_244                                  0xf4 // 5G + (-12)*5M
#define CH_248                                  0xf8 // 5G + (-8)*5M
#define CH_252                                  0xfc // 5G + (-4)*5M
#define CH_8                                    0x8
#define CH_12                                   0xc
#define CH_16                                   0x10
#define CH_34                                   0x22
#define CH_36                                   0x24
#define CH_38                                   0x26
#define CH_40                                   0x28
#define CH_42                                   0x2a
#define CH_44                                   0x2c
#define CH_46                                   0x2e
#define CH_48                                   0x30
#define CH_52                                   0x34
#define CH_56                                   0x38
#define CH_58                                   0x3a
#define CH_60                                   0x3c
#define CH_100                                  0x64
#define CH_104                                  0x68
#define CH_108                                  0x6c
#define CH_112                                  0x70
#define CH_116                                  0x74
#define CH_120                                  0x78
#define CH_124                                  0x7c
#define CH_128                                  0x80
#define CH_132                                  0x84
#define CH_136                                  0x88
#define CH_140                                  0x8c
#define CH_149                                  0x95
#define CH_153                                  0x99
#define CH_157                                  0x9d
#define CH_161                                  0xA1
#else
/* 15.4.6.2 Number of operating channels */
#define CH_1                                    0x1
#define CH_2                                    0x2
#define CH_3                                    0x3
#define CH_4                                    0x4
#define CH_5                                    0x5
#define CH_6                                    0x6
#define CH_7                                    0x7
#define CH_8                                    0x8
#define CH_9                                    0x9
#define CH_10                                   0xa
#define CH_11                                   0xb
#define CH_12                                   0xc
#define CH_13                                   0xd
#define CH_14                                   0xe
#endif

#define MAXIMUM_OPERATION_CHANNEL_LIST          32


//3 /* --------------- IEEE 802.11 PICS --------------- */
/* Annex D - dot11OperationEntry 2 */
#define DOT11_RTS_THRESHOLD_MIN                 0
#define DOT11_RTS_THRESHOLD_MAX                 2347 // from Windows DDK
//#define DOT11_RTS_THRESHOLD_MAX                 3000 // from Annex D

#define DOT11_RTS_THRESHOLD_DEFAULT             \
            DOT11_RTS_THRESHOLD_MAX

/* Annex D - dot11OperationEntry 5 */
#define DOT11_FRAGMENTATION_THRESHOLD_MIN       256
#define DOT11_FRAGMENTATION_THRESHOLD_MAX       2346 // from Windows DDK
//#define DOT11_FRAGMENTATION_THRESHOLD_MAX       3000 // from Annex D

#define DOT11_FRAGMENTATION_THRESHOLD_DEFAULT   \
            DOT11_FRAGMENTATION_THRESHOLD_MAX

/* Annex D - dot11OperationEntry 6 */
#define DOT11_TRANSMIT_MSDU_LIFETIME_TU_MIN     1
#define DOT11_TRANSMIT_MSDU_LIFETIME_TU_MAX     0xFFFFffff
#define DOT11_TRANSMIT_MSDU_LIFETIME_TU_DEFAULT 4095 // 802.11 define 512
                                                     // MT5921 only aceept N <= 4095

/* Annex D - dot11OperationEntry 7 */
#define DOT11_RECEIVE_LIFETIME_TU_MIN           1
#define DOT11_RECEIVE_LIFETIME_TU_MAX           0xFFFFffff
#define DOT11_RECEIVE_LIFETIME_TU_DEFAULT       4096 // 802.11 define 512

/* Annex D - dot11StationConfigEntry 12 */
#define DOT11_BEACON_PERIOD_MIN                 1 // TU.
#define DOT11_BEACON_PERIOD_MAX                 0xffff // TU.
#define DOT11_BEACON_PERIOD_DEFAULT             100 // TU.

/* Annex D - dot11RegDomainsSupportValue */
#define REGULATION_DOMAIN_FCC                   0x10        /* FCC (US) */
#define REGULATION_DOMAIN_IC                    0x20        /* IC or DOC (Canada) */
#define REGULATION_DOMAIN_ETSI                  0x30        /* ETSI (Europe) */
#define REGULATION_DOMAIN_SPAIN                 0x31        /* Spain */
#define REGULATION_DOMAIN_FRANCE                0x32        /* France */
#define REGULATION_DOMAIN_JAPAN                 0x40        /* MKK (Japan) */
#define REGULATION_DOMAIN_CHINA                 0x50        /* China */
#define REGULATION_DOMAIN_OTHER                 0x00        /* Other */



//3 /* --------------- IEEE 802.11 MAC header fields --------------- */
/* 7.1.3.1 Masks for the subfields in the Frame Control field */
#define MASK_FC_PROTOCOL_VER                    BITS(0,1)
#define MASK_FC_TYPE                            BITS(2,3)
#define MASK_FC_SUBTYPE                         BITS(4,7)
#define MASK_FC_SUBTYPE_QOS_DATA                BIT(7)
#define MASK_FC_TO_DS                           BIT(8)
#define MASK_FC_FROM_DS                         BIT(9)
#define MASK_FC_MORE_FRAG                       BIT(10)
#define MASK_FC_RETRY                           BIT(11)
#define MASK_FC_PWR_MGT                         BIT(12)
#define MASK_FC_MORE_DATA                       BIT(13)
#define MASK_FC_PROTECTED_FRAME                 BIT(14)
#define MASK_FC_ORDER                           BIT(15)

#define MASK_FRAME_TYPE                         (MASK_FC_TYPE | MASK_FC_SUBTYPE)
#define MASK_TO_DS_FROM_DS                      (MASK_FC_TO_DS | MASK_FC_FROM_DS)

#define MAX_NUM_OF_FC_SUBTYPES                  16
#define OFFSET_OF_FC_SUBTYPE                    4


/* 7.1.3.1.2 MAC frame types and subtypes */
#define MAC_FRAME_TYPE_MGT                      0
#define MAC_FRAME_TYPE_CTRL                     BIT(2)
#define MAC_FRAME_TYPE_DATA                     BIT(3)
#define MAC_FRAME_TYPE_QOS_DATA                 (MAC_FRAME_TYPE_DATA | MASK_FC_SUBTYPE_QOS_DATA)

#define MAC_FRAME_ASSOC_REQ                     (MAC_FRAME_TYPE_MGT | 0x0000)
#define MAC_FRAME_ASSOC_RSP                     (MAC_FRAME_TYPE_MGT | 0x0010)
#define MAC_FRAME_REASSOC_REQ                   (MAC_FRAME_TYPE_MGT | 0x0020)
#define MAC_FRAME_REASSOC_RSP                   (MAC_FRAME_TYPE_MGT | 0x0030)
#define MAC_FRAME_PROBE_REQ                     (MAC_FRAME_TYPE_MGT | 0x0040)
#define MAC_FRAME_PROBE_RSP                     (MAC_FRAME_TYPE_MGT | 0x0050)
#define MAC_FRAME_BEACON                        (MAC_FRAME_TYPE_MGT | 0x0080)
#define MAC_FRAME_ATIM                          (MAC_FRAME_TYPE_MGT | 0x0090)
#define MAC_FRAME_DISASSOC                      (MAC_FRAME_TYPE_MGT | 0x00A0)
#define MAC_FRAME_AUTH                          (MAC_FRAME_TYPE_MGT | 0x00B0)
#define MAC_FRAME_DEAUTH                        (MAC_FRAME_TYPE_MGT | 0x00C0)
#define MAC_FRAME_ACTION                        (MAC_FRAME_TYPE_MGT | 0x00D0)

#define MAC_FRAME_BLOCK_ACK_REQ                 (MAC_FRAME_TYPE_CTRL | 0x0080)
#define MAC_FRAME_BLOCK_ACK                     (MAC_FRAME_TYPE_CTRL | 0x0090)
#define MAC_FRAME_PS_POLL                       (MAC_FRAME_TYPE_CTRL | 0x00A0)
#define MAC_FRAME_RTS                           (MAC_FRAME_TYPE_CTRL | 0x00B0)
#define MAC_FRAME_CTS                           (MAC_FRAME_TYPE_CTRL | 0x00C0)
#define MAC_FRAME_ACK                           (MAC_FRAME_TYPE_CTRL | 0x00D0)
#define MAC_FRAME_CF_END                        (MAC_FRAME_TYPE_CTRL | 0x00E0)
#define MAC_FRAME_CF_END_CF_ACK                 (MAC_FRAME_TYPE_CTRL | 0x00F0)

#define MAC_FRAME_DATA                          (MAC_FRAME_TYPE_DATA | 0x0000)
#define MAC_FRAME_DATA_CF_ACK                   (MAC_FRAME_TYPE_DATA | 0x0010)
#define MAC_FRAME_DATA_CF_POLL                  (MAC_FRAME_TYPE_DATA | 0x0020)
#define MAC_FRAME_DATA_CF_ACK_CF_POLL           (MAC_FRAME_TYPE_DATA | 0x0030)
#define MAC_FRAME_NULL                          (MAC_FRAME_TYPE_DATA | 0x0040)
#define MAC_FRAME_CF_ACK                        (MAC_FRAME_TYPE_DATA | 0x0050)
#define MAC_FRAME_CF_POLL                       (MAC_FRAME_TYPE_DATA | 0x0060)
#define MAC_FRAME_CF_ACK_CF_POLL                (MAC_FRAME_TYPE_DATA | 0x0070)
#define MAC_FRAME_QOS_DATA                      (MAC_FRAME_TYPE_DATA | 0x0080)
#define MAC_FRAME_QOS_DATA_CF_ACK               (MAC_FRAME_TYPE_DATA | 0x0090)
#define MAC_FRAME_QOS_DATA_CF_POLL              (MAC_FRAME_TYPE_DATA | 0x00A0)
#define MAC_FRAME_QOS_DATA_CF_ACK_CF_POLL       (MAC_FRAME_TYPE_DATA | 0x00B0)
#define MAC_FRAME_QOS_NULL                      (MAC_FRAME_TYPE_DATA | 0x00C0)
#define MAC_FRAME_QOS_CF_POLL                   (MAC_FRAME_TYPE_DATA | 0x00E0)
#define MAC_FRAME_QOS_CF_ACK_CF_POLL            (MAC_FRAME_TYPE_DATA | 0x00F0)

/* 7.1.3.2 Mask for the AID value in the Duration/ID field */
#define MASK_DI_DURATION                        BITS(0,14)
#define MASK_DI_AID                             BITS(0,13)
#define MASK_DI_AID_MSB                         BITS(14,15)
#define MASK_DI_CFP_FIXED_VALUE                 BIT(15)

/* 7.1.3.4 Masks for the subfields in the Sequence Control field */
#define MASK_SC_SEQ_NUM                         BITS(4,15)
#define MASK_SC_FRAG_NUM                        BITS(0,3)
#define INVALID_SEQ_CTRL_NUM                    0x000F /* According to 6.2.1.1.2
                                                         * FRAG_NUM won't equal to 15
                                                         */

/* 7.1.3.5 QoS Control field */
#define TID_NUM                                 16
#define TID_MASK                                BITS(0,3)
#define EOSP                                    BIT(4)



//3 /* --------------- IEEE 802.11 frame body fields --------------- */
//3 Management frame body components (I): Fixed Fields.
/* 7.3.1.1 Authentication Algorithm Number field */
#define AUTH_ALGORITHM_NUM_FIELD_LEN                2

#define AUTH_ALGORITHM_NUM_OPEN_SYSTEM              0   /* Open System */
#define AUTH_ALGORITHM_NUM_SHARED_KEY               1   /* Shared Key */
#define AUTH_ALGORITHM_NUM_FAST_BSS_TRANSITION      2   /* Fast BSS Transition */

/* 7.3.1.2 Authentication Transaction Sequence Number field */
#define AUTH_TRANSACTION_SEQENCE_NUM_FIELD_LEN      2
#define AUTH_TRANSACTION_SEQ_1                      1
#define AUTH_TRANSACTION_SEQ_2                      2
#define AUTH_TRANSACTION_SEQ_3                      3
#define AUTH_TRANSACTION_SEQ_4                      4

/* 7.3.1.3 Beacon Interval field */
#define BEACON_INTERVAL_FIELD_LEN                   2

/* 7.3.1.4 Capability Information field */
#define CAP_INFO_FIELD_LEN                          2
#define CAP_INFO_ESS                                BIT(0)
#define CAP_INFO_IBSS                               BIT(1)
#define CAP_INFO_BSS_TYPE                           (CAP_INFO_ESS | CAP_INFO_IBSS)
#define CAP_INFO_CF_POLLABLE                        BIT(2)
#define CAP_INFO_CF_POLL_REQ                        BIT(3)
#define CAP_INFO_CF                                 (CAP_INFO_CF_POLLABLE | CAP_INFO_CF_POLL_REQ)
#define CAP_INFO_PRIVACY                            BIT(4)
#define CAP_INFO_SHORT_PREAMBLE                     BIT(5)
#define CAP_INFO_PBCC                               BIT(6)
#define CAP_INFO_CH_AGILITY                         BIT(7)
#define CAP_INFO_SPEC_MGT                           BIT(8)
#define CAP_INFO_QOS                                BIT(9)
#define CAP_INFO_SHORT_SLOT_TIME                    BIT(10)
#define CAP_INFO_APSD                               BIT(11)
#define CAP_INFO_RESERVED                           BIT(12)
#define CAP_INFO_DSSS_OFDM                          BIT(13)
#define CAP_INFO_DELAYED_BLOCK_ACK                  BIT(14)
#define CAP_INFO_IMM_BLOCK_ACK                      BIT(15)
/* STA usage of CF-Pollable and CF-Poll Request subfields */
/* STA: not CF-Pollable */
#define CAP_CF_STA_NOT_POLLABLE                     0x0000
/* STA: CF-Pollable, not requesting on the CF-Polling list */
#define CAP_CF_STA_NOT_ON_LIST                      CAP_INFO_CF_POLL_REQ
/* STA: CF-Pollable, requesting on the CF-Polling list */
#define CAP_CF_STA_ON_LIST                          CAP_INFO_CF_POLLABLE
/* STA: CF-Pollable, requesting never to be polled */
#define CAP_CF_STA_NEVER_POLLED                     (CAP_INFO_CF_POLLABLE | CAP_INFO_CF_POLL_REQ)

/* AP usage of CF-Pollable and CF-Poll Request subfields */
/* AP: No point coordinator (PC) */
#define CAP_CF_AP_NO_PC                             0x0000
/* AP: PC at AP for delivery only (no polling) */
#define CAP_CF_AP_DELIVERY_ONLY                     CAP_INFO_CF_POLL_REQ
/* AP: PC at AP for delivery and polling */
#define CAP_CF_AP_DELIVERY_POLLING                  CAP_INFO_CF_POLLABLE

/* 7.3.1.5 Current AP Address field */
#define CURR_AP_ADDR_FIELD_LEN                      MAC_ADDR_LEN

/* 7.3.1.6 Listen Interval field */
#define LISTEN_INTERVAL_FIELD_LEN                   2

/* 7.3.1.7 Reason Code field */
#define REASON_CODE_FIELD_LEN                       2

#define REASON_CODE_RESERVED                        0   /* Reseved */
#define REASON_CODE_UNSPECIFIED                     1   /* Unspecified reason */
#define REASON_CODE_PREV_AUTH_INVALID               2   /* Previous auth no longer valid */
#define REASON_CODE_DEAUTH_LEAVING_BSS              3   /* Deauth because sending STA is leaving BSS */
#define REASON_CODE_DISASSOC_INACTIVITY             4   /* Disassoc due to inactivity */
#define REASON_CODE_DISASSOC_AP_OVERLOAD            5   /* Disassoc because AP is unable to handle all assoc STAs */
#define REASON_CODE_CLASS_2_ERR                     6   /* Class 2 frame rx from nonauth STA */
#define REASON_CODE_CLASS_3_ERR                     7   /* Class 3 frame rx from nonassoc STA */
#define REASON_CODE_DISASSOC_LEAVING_BSS            8   /* Disassoc because sending STA is leaving BSS */
#define REASON_CODE_ASSOC_BEFORE_AUTH               9   /* STA requesting (re)assoc is not auth with responding STA */
#define REASON_CODE_DISASSOC_PWR_CAP_UNACCEPTABLE   10  /* Disassoc because the info in Power Capability is unacceptable */
#define REASON_CODE_DISASSOC_SUP_CHS_UNACCEPTABLE   11  /* Disassoc because the info in Supported Channels is unacceptable */
#define REASON_CODE_INVALID_INFO_ELEM               13  /* Invalid information element */
#define REASON_CODE_MIC_FAILURE                     14  /* MIC failure */
#define REASON_CODE_4_WAY_HANDSHAKE_TIMEOUT         15  /* 4-way handshake timeout */
#define REASON_CODE_GROUP_KEY_UPDATE_TIMEOUT        16  /* Group key update timeout */
#define REASON_CODE_DIFFERENT_INFO_ELEM             17  /* Info element in 4-way handshake different from (Re-)associate request/Probe response/Beacon */
#define REASON_CODE_MULTICAST_CIPHER_NOT_VALID      18  /* Multicast Cipher is not valid */
#define REASON_CODE_UNICAST_CIPHER_NOT_VALID        19  /* Unicast Cipher is not valid */
#define REASON_CODE_AKMP_NOT_VALID                  20  /* AKMP is not valid */
#define REASON_CODE_UNSUPPORTED_RSNE_VERSION        21  /* Unsupported RSNE version */
#define REASON_CODE_INVALID_RSNE_CAPABILITIES       22  /* Invalid RSNE Capabilities */
#define REASON_CODE_IEEE_802_1X_AUTH_FAILED         23  /* IEEE 802.1X Authentication failed */
#define REASON_CODE_CIPHER_REJECT_SEC_POLICY        24  /* Cipher suite rejected because of the security policy */
#define REASON_CODE_DISASSOC_UNSPECIFIED_QOS        32  /* Disassoc for unspecified, QoS-related reason */
#define REASON_CODE_DISASSOC_LACK_OF_BANDWIDTH      33  /* Disassoc because QAP lacks sufficient bandwidth for this QSTA */
#define REASON_CODE_DISASSOC_ACK_LOST_POOR_CHANNEL  34  /* Disassoc because of too many ACKs lost for AP transmissions and/or poor channel conditions */
#define REASON_CODE_DISASSOC_TX_OUTSIDE_TXOP_LIMIT  35  /* Disassoc because QSTA is transmitting outside the limits of its TXOPs */
#define REASON_CODE_PEER_WHILE_LEAVING              36  /* QSTA is leaving the QBSS or resetting */
#define REASON_CODE_PEER_REFUSE_DLP                 37  /* Peer does not want to use this mechanism */
#define REASON_CODE_PEER_SETUP_REQUIRED             38  /* Frames received but a setup is reqired */
#define REASON_CODE_PEER_TIME_OUT                   39  /* Time out */
#define REASON_CODE_PEER_CIPHER_UNSUPPORTED         45  /* Peer does not support the requested cipher suite */

/* 7.3.1.8 AID field */
#define AID_FIELD_LEN                               2
#define AID_MASK                                    BITS(0,13)
#define AID_MSB                                     BITS(14,15)
#define AID_MIN_VALUE                               1
#define AID_MAX_VALUE                               2007

/* 7.3.1.9 Status Code field */
#define STATUS_CODE_FIELD_LEN                       2

#define STATUS_CODE_SUCCESSFUL                      0   /* Successful */
#define STATUS_CODE_UNSPECIFIED_FAILURE             1   /* Unspecified failure */
#define STATUS_CODE_CAP_NOT_SUPPORTED               10  /* Cannot support all requested cap in the Cap Info field */
#define STATUS_CODE_REASSOC_DENIED_WITHOUT_ASSOC    11  /* Reassoc denied due to inability to confirm that assoc exists */
#define STATUS_CODE_ASSOC_DENIED_OUTSIDE_STANDARD   12  /* Assoc denied due to reason outside the scope of this std. */
#define STATUS_CODE_AUTH_ALGORITHM_NOT_SUPPORTED    13  /* Responding STA does not support the specified auth algorithm */
#define STATUS_CODE_AUTH_OUT_OF_SEQ                 14  /* Rx an auth frame with auth transaction seq num out of expected seq */
#define STATUS_CODE_AUTH_REJECTED_CHAL_FAIL         15  /* Auth rejected because of challenge failure */
#define STATUS_CODE_AUTH_REJECTED_TIMEOUT           16  /* Auth rejected due to timeout waiting for next frame in sequence */
#define STATUS_CODE_ASSOC_DENIED_AP_OVERLOAD        17  /* Assoc denied because AP is unable to handle additional assoc STAs */
#define STATUS_CODE_ASSOC_DENIED_RATE_NOT_SUPPORTED 18  /* Assoc denied due to requesting STA not supporting all of basic rates */
#define STATUS_CODE_ASSOC_DENIED_NO_SHORT_PREAMBLE  19  /* Assoc denied due to requesting STA not supporting short preamble */
#define STATUS_CODE_ASSOC_DENIED_NO_PBCC            20  /* Assoc denied due to requesting STA not supporting PBCC */
#define STATUS_CODE_ASSOC_DENIED_NO_CH_AGILITY      21  /* Assoc denied due to requesting STA not supporting channel agility */
#define STATUS_CODE_ASSOC_REJECTED_NO_SPEC_MGT      22  /* Assoc rejected because Spectrum Mgt capability is required */
#define STATUS_CODE_ASSOC_REJECTED_PWR_CAP          23  /* Assoc rejected because the info in Power Capability is unacceptable */
#define STATUS_CODE_ASSOC_REJECTED_SUP_CHS          24  /* Assoc rejected because the info in Supported Channels is unacceptable */
#define STATUS_CODE_ASSOC_DENIED_NO_SHORT_SLOT_TIME 25  /* Assoc denied due to requesting STA not supporting short slot time */
#define STATUS_CODE_ASSOC_DENIED_NO_DSSS_OFDM       26  /* Assoc denied due to requesting STA not supporting DSSS-OFDM */
#define STATUS_CODE_UNSPECIFIED_QOS_FAILURE         32  /* Unspecified, QoS-related failure */
#define STATUS_CODE_ASSOC_DENIED_BANDWIDTH          33  /* Assoc denied due to insufficient bandwidth to handle another QSTA */
#define STATUS_CODE_ASSOC_DENIED_POOR_CHANNEL       34  /* Assoc denied due to excessive frame loss rates and/or poor channel conditions */
#define STATUS_CODE_ASSOC_DENIED_NO_QOS_FACILITY    35  /* Assoc denied due to requesting STA not supporting QoS facility */
#define STATUS_CODE_REQ_DECLINED                    37  /* Request has been declined */
#define STATUS_CODE_REQ_INVALID_PARAMETER_VALUE     38  /* Request has not been successful as one or more parameters have invalid values */
#define STATUS_CODE_REQ_NOT_HONORED_TSPEC           39  /* TS not created because request cannot be honored. Suggested TSPEC provided. */
#define STATUS_CODE_INVALID_INFO_ELEMENT            40  /* Invalid information element */
#define STATUS_CODE_INVALID_GROUP_CIPHER            41  /* Invalid group cipher */
#define STATUS_CODE_INVALID_PAIRWISE_CIPHER         42  /* Invalid pairwise cipher */
#define STATUS_CODE_INVALID_AKMP                    43  /* Invalid AKMP */
#define STATUS_CODE_UNSUPPORTED_RSN_IE_VERSION      44  /* Unsupported RSN information element version */
#define STATUS_CODE_INVALID_RSN_IE_CAP              45  /* Invalid RSN information element capabilities */
#define STATUS_CODE_CIPHER_SUITE_REJECTED           46  /* Cipher suite rejected because of security policy */
#define STATUS_CODE_REQ_NOT_HONORED_TS_DELAY        47  /* TS not created becasue request cannot be honored. Attempt to create a TS later. */
#define STATUS_CODE_DIRECT_LINK_NOT_ALLOWED         48  /* Direct Link is not allowed in the BSS by policy */
#define STATUS_CODE_DESTINATION_STA_NOT_PRESENT     49  /* Destination STA is not present within this QBSS */
#define STATUS_CODE_DESTINATION_STA_NOT_QSTA        50  /* Destination STA is not a QSTA */
#define STATUS_CODE_ASSOC_DENIED_LARGE_LIS_INTERVAL 51  /* Association denied because the ListenInterval is too large */

/* proprietary definition of reserved field of Status Code */
#define STATUS_CODE_JOIN_FAILURE                    0xFFF0  /* Join failure */
#define STATUS_CODE_JOIN_TIMEOUT                    0xFFF1  /* Join timeout */
#define STATUS_CODE_AUTH_TIMEOUT                    0xFFF2  /* Authentication timeout */
#define STATUS_CODE_ASSOC_TIMEOUT                   0xFFF3  /* (Re)Association timeout */
#define STATUS_CODE_CCX_CCKM_REASSOC_FAILURE        0xFFF4  /* CCX CCKM reassociation failure */


/* 7.3.1.10 Timestamp field */
#define TIMESTAMP_FIELD_LEN                         8

/* 7.3.1.11 Category of Action field */
#define CATEGORY_QOS_ACTION                         1   /* QoS action */
#define CATEGORY_DLS_ACTION                         2   /* Direct Link Protocol (DLP) action */
#define CATEGORY_BLOCK_ACK_ACTION                   3   /* Block ack action */
#define CATEGORY_WME_MGT_NOTIFICATION               17  /* WME management notification */



//3 Management frame body components (II): Information Elements.
/* 7.3.2 Element IDs of information elements */
#define ELEM_HDR_LEN                                2

#define ELEM_ID_SSID                                0   /* SSID */
#define ELEM_ID_SUP_RATES                           1   /* Supported rates */
#define ELEM_ID_FH_PARAM_SET                        2   /* FH parameter set */
#define ELEM_ID_DS_PARAM_SET                        3   /* DS parameter set */
#define ELEM_ID_CF_PARAM_SET                        4   /* CF parameter set */
#define ELEM_ID_TIM                                 5   /* TIM */
#define ELEM_ID_IBSS_PARAM_SET                      6   /* IBSS parameter set */
#define ELEM_ID_COUNTRY_INFO                        7   /* Country information */
#define ELEM_ID_HOPPING_PATTERN_PARAM               8   /* Hopping pattern parameters */
#define ELEM_ID_HOPPING_PATTERN_TABLE               9   /* Hopping pattern table */
#define ELEM_ID_REQUEST                             10  /* Request */
#define ELEM_ID_BSS_LOAD                            11  /* BSS load */
#define ELEM_ID_EDCA_PARAM_SET                      12  /* EDCA parameter set */
#define ELEM_ID_TSPEC                               13  /* Traffic specification (TSPEC) */
#define ELEM_ID_TCLAS                               14  /* Traffic classification (TCLAS) */
#define ELEM_ID_SCHEDULE                            15  /* Schedule */
#define ELEM_ID_CHALLENGE_TEXT                      16  /* Challenge text */

#define ELEM_ID_PWR_CONSTRAINT                      32  /* Power constraint */
#define ELEM_ID_PWR_CAP                             33  /* Power capability */
#define ELEM_ID_TPC_REQ                             34  /* TPC request */
#define ELEM_ID_TPC_REPORT                          35  /* TPC report */
#define ELEM_ID_SUP_CHS                             36  /* Supported channels */
#define ELEM_ID_CH_SW_ANNOUNCEMENT                  37  /* Channel switch announcement */
#define ELEM_ID_MEASUREMENT_REQ                     38  /* Measurement request */
#define ELEM_ID_MEASUREMENT_REPORT                  39  /* Measurement report */
#define ELEM_ID_QUIET                               40  /* Quiet */
#define ELEM_ID_IBSS_DFS                            41  /* IBSS DFS */

#define ELEM_ID_ERP_INFO                            42  /* ERP information */
#define ELEM_ID_TS_DELAY                            43  /* TS delay */
#define ELEM_ID_TCLAS_PROCESSING                    44  /* TCLAS processing */
#define ELEM_ID_QOS_MGT_ACTION                      45  /* QoS management action */
#define ELEM_ID_QOS_CAP                             46  /* QoS capability */

#define ELEM_ID_RSN                                 48  /* RSN IE */

#define ELEM_ID_EXTENDED_SUP_RATES                  50  /* Extended supported rates */

#define ELEM_ID_VENDOR                              221 /* Vendor specific IE */
#define ELEM_ID_WPA                                 ELEM_ID_VENDOR /* WPA IE */
#define ELEM_ID_WMM                                 ELEM_ID_VENDOR /* WMM IE */


/* 7.3.2.1 SSID element */
#define ELEM_MAX_LEN_SSID                           32

/* 7.3.2.2 Supported Rates */
#define ELEM_MAX_LEN_SUP_RATES                      8

/* 7.3.2.4 DS Parameter Set */
#define ELEM_MAX_LEN_DS_PARAMETER_SET               1

/* 7.3.2.6 TIM */
#define ELEM_MAX_LEN_TIM                            254

/* 7.3.2.7 IBSS Parameter Set element */
#define ELEM_MAX_LEN_IBSS_PARAMETER_SET             2

/* 7.3.2.8 Challenge Text element */
#define ELEM_MIN_LEN_CHALLENGE_TEXT                 1
#define ELEM_MAX_LEN_CHALLENGE_TEXT                 253

/* 7.3.2.9 Country Information element */
/* Country IE should contain at least 3-bytes country code string and one subband triplet. */
#define ELEM_MIN_LEN_COUNTRY_INFO                   6

#define ELEM_ID_COUNTRY_INFO_TRIPLET_LEN_FIXED              3
#define ELEM_ID_COUNTRY_INFO_SUBBAND_TRIPLET_LEN_FIXED      3
#define ELEM_ID_COUNTRY_INFO_REGULATORY_TRIPLET_LEN_FIXED   3


/* 7.3.2.13 ERP Information element */
#define ELEM_MAX_LEN_ERP                            1
/* -- bits in the ERP Information element */
#define ERP_INFO_NON_ERP_PRESENT                    BIT(0)  /* NonERP_Present bit */
#define ERP_INFO_USE_PROTECTION                     BIT(1)  /* Use_Protection bit */
#define ERP_INFO_BARKER_PREAMBLE_MODE               BIT(2)  /* Barker_Preamble_Mode bit */


/* 7.3.2.14 Extended Supported Rates */
#define ELEM_MAX_LEN_EXTENDED_SUP_RATES             255

/* 7.3.2.25 RSN information element */
#define ELEM_MAX_LEN_WPA_RSN                        38 /* one pairwise, one AKM suite, one PMKID */
#define ELEM_MAX_LEN_RSN_IE                         40
#define ELEM_MAX_LEN_WAPI_ASSOC_INFO                40 /* one pairwise, one AKM suite, one BKID */

//3 Management frame body components (III): 7.4 Action frame format details.
/* 7.4.2 QoS Action frame details */
#define ACTION_ADDTS_REQ                            0   /* ADDTS request */
#define ACTION_ADDTS_RSP                            1   /* ADDTS response */
#define ACTION_DELTS                                2   /* DELTS */
#define ACTION_SCHEDULE                             3   /* Schedule */

/* 7.4.3 DLS Action frame details */
#define ACTION_DLS_REQ                              0   /* DLS request */
#define ACTION_DLS_RSP                              1   /* DLS response */
#define ACTION_DLS_TEARDOWN                         2   /* DLS teardown */

/* 7.4.4 Block ack  Action frame details */
#define ACTION_ADDBA_REQ                            0   /* ADDBA request */
#define ACTION_ADDBA_RSP                            1   /* ADDBA response */
#define ACTION_DELBA                                2   /* DELBA */



//3 /* --------------- WFA  frame body fields --------------- */
#define VENDOR_OUI_WFA                              { 0x00, 0x50, 0xF2 }
#define VENDOR_OUI_TYPE_WPA                         1
#define VENDOR_OUI_TYPE_WMM                         2
#define VENDOR_OUI_TYPE_WPS                         4

/* VERSION(2 octets for WPA) / SUBTYPE(1 octet)-VERSION(1 octet) fields for WMM in WFA IE */
#define VERSION_WPA                                 0x0001 /* Little Endian Format */
#define VENDOR_OUI_SUBTYPE_VERSION_WMM_INFO         0x0100
#define VENDOR_OUI_SUBTYPE_VERSION_WMM_PARAM        0x0101

/* SUBTYPE(1 octet) for WMM */
#define VENDOR_OUI_SUBTYPE_WMM_INFO                 0x00 /* WMM Spec version 1.1 */
#define VENDOR_OUI_SUBTYPE_WMM_PARAM                0x01

/* VERSION(1 octet) for WMM */
#define VERSION_WMM                                 0x01 /* WMM Spec version 1.1 */

/* WMM-2.2.1 WMM Information Element */
#define ELEM_MIN_LEN_WFA_OUI_TYPE_SUBTYPE           6


//3 MAC Header.


/* Ethernet Frame Structure */
typedef struct _ETH_FRAME_T {
    UINT_8      aucDestAddr[MAC_ADDR_LEN];
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];
    UINT_16     u2TypeLen;
    UINT_8      aucData[1];
} ETH_FRAME_T, *P_ETH_FRAME_T;


/* IEEE 802.11 WLAN Frame Structure */
/* WLAN MAC Header (without Address 4 and QoS Control fields) */
typedef struct _WLAN_MAC_HEADER_T {
    UINT_16     u2FrameCtrl;
    UINT_16     u2DurationID;
    UINT_8      aucAddr1[MAC_ADDR_LEN];
    UINT_8      aucAddr2[MAC_ADDR_LEN];
    UINT_8      aucAddr3[MAC_ADDR_LEN];
    UINT_16     u2SeqCtrl;
} WLAN_MAC_HEADER_T, *P_WLAN_MAC_HEADER_T;


//#pragma pack(push,1) // Not supported by Linux Colibri.
#pragma pack(1)

/* WLAN MAC Header (QoS Control fields included) */
typedef struct _WLAN_MAC_HEADER_QOS_T {
    UINT_16     u2FrameCtrl;
    UINT_16     u2DurationID;
    UINT_8      aucAddr1[MAC_ADDR_LEN];
    UINT_8      aucAddr2[MAC_ADDR_LEN];
    UINT_8      aucAddr3[MAC_ADDR_LEN];
    UINT_16     u2SeqCtrl;
    UINT_16     u2QosCtrl;
} __KAL_ATTRIB_PACKED__ WLAN_MAC_HEADER_QOS_T, *P_WLAN_MAC_HEADER_QOS_T;

/* WLAN MAC Header (Address 4 included) */
typedef struct _WLAN_MAC_HEADER_A4_T {
    UINT_16     u2FrameCtrl;
    UINT_16     u2DurationID;
    UINT_8      aucAddr1[MAC_ADDR_LEN];
    UINT_8      aucAddr2[MAC_ADDR_LEN];
    UINT_8      aucAddr3[MAC_ADDR_LEN];
    UINT_16     u2SeqCtrl;
    UINT_8      aucAddr4[MAC_ADDR_LEN];
} __KAL_ATTRIB_PACKED__ WLAN_MAC_HEADER_A4_T, *P_WLAN_MAC_HEADER_A4_T;

//#pragma pack(pop) // Not supported by Linux Colibri.
#pragma pack()


/* WLAN MAC Header (Address 4 and QoS Control fields included) */
typedef struct _WLAN_MAC_HEADER_A4_QOS_T {
    UINT_16     u2FrameCtrl;
    UINT_16     u2DurationID;
    UINT_8      aucAddr1[MAC_ADDR_LEN];
    UINT_8      aucAddr2[MAC_ADDR_LEN];
    UINT_8      aucAddr3[MAC_ADDR_LEN];
    UINT_16     u2SeqCtrl;
    UINT_8      aucAddr4[MAC_ADDR_LEN];
    UINT_16     u2QosCtrl;
} WLAN_MAC_HEADER_A4_QOS_T, *P_WLAN_MAC_HEADER_A4_QOS_T;


/* 7.2.3 WLAN MAC Header for Management Frame - MMPDU */
typedef struct _WLAN_MAC_MGMT_HEADER_T {
    UINT_16     u2FrameCtrl;
    UINT_16     u2Duration;
    UINT_8      aucDestAddr[MAC_ADDR_LEN];
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];
    UINT_8      aucBSSID[MAC_ADDR_LEN];
    UINT_16     u2SeqCtrl;
} WLAN_MAC_MGMT_HEADER_T, *P_WLAN_MAC_MGMT_HEADER_T;


//#pragma pack(push,1) // Not supported by Linux Colibri.
#pragma pack(1)

//3 WLAN Management Frame.
/* 7.2.3.1 WLAN Management Frame - Beacon Frame */
typedef struct _WLAN_BEACON_FRAME_T {
    /* Beacon header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Beacon frame body */
    UINT_32     au4Timestamp[2];            /* Timestamp */
    UINT_16     u2BeaconInterval;           /* Beacon Interval */
    UINT_16     u2CapInfo;                  /* Capability */
    UINT_8      aucInfoElem[1];             /* Various IEs, start from SSID */
} __KAL_ATTRIB_PACKED__ WLAN_BEACON_FRAME_T, *P_WLAN_BEACON_FRAME_T;

typedef struct _WLAN_BEACON_FRAME_BODY_T {
    /* Beacon frame body */
    UINT_32     au4Timestamp[2];            /* Timestamp */
    UINT_16     u2BeaconInterval;           /* Beacon Interval */
    UINT_16     u2CapInfo;                  /* Capability */
    UINT_8      aucInfoElem[1];             /* Various IEs, start from SSID */
} __KAL_ATTRIB_PACKED__ WLAN_BEACON_FRAME_BODY_T, *P_WLAN_BEACON_FRAME_BODY_T;


/* 7.2.3.3 WLAN Management Frame - Disassociation Frame */
typedef struct _WLAN_DISASSOC_FRAME_T {
    /* Authentication MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Disassociation frame body */
    UINT_16     u2ReasonCode;               /* Reason code */
} __KAL_ATTRIB_PACKED__ WLAN_DISASSOC_FRAME_T, *P_WLAN_DISASSOC_FRAME_T;


/* 7.2.3.4 WLAN Management Frame - Association Request frame */
typedef struct _WLAN_ASSOC_REQ_FRAME_T {
    /* Association Request MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Association Request frame body */
    UINT_16     u2CapInfo;                  /* Capability information */
    UINT_16     u2ListenInterval;           /* Listen interval */
    UINT_8      aucInfoElem[1];             /* Information elements, include WPA IE */
} __KAL_ATTRIB_PACKED__ WLAN_ASSOC_REQ_FRAME_T, *P_WLAN_ASSOC_REQ_FRAME_T;


/* 7.2.3.5 WLAN Management Frame - Association Response frame */
typedef struct _WLAN_ASSOC_RSP_FRAME_T {
    /* Association Response MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Association Response frame body */
    UINT_16     u2CapInfo;                  /* Capability information */
    UINT_16     u2StatusCode;               /* Status code */
    UINT_16     u2AssocId;                  /* Association ID */
    UINT_8      aucInfoElem[1];             /* Information elements, such as
                                               supported rates, and etc. */
} __KAL_ATTRIB_PACKED__ WLAN_ASSOC_RSP_FRAME_T, *P_WLAN_ASSOC_RSP_FRAME_T;


/* 7.2.3.6 WLAN Management Frame - Reassociation Request frame */
typedef struct _WLAN_REASSOC_REQ_FRAME_T {
    /* Reassociation Request MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Reassociation Request frame body */
    UINT_16     u2CapInfo;                  /* Capability information */
    UINT_16     u2ListenInterval;           /* Listen interval */
    UINT_8      aucCurrentAPAddr[MAC_ADDR_LEN]; /* Current AP address */
    UINT_8      aucInfoElem[1];             /* Information elements, include WPA IE */
} __KAL_ATTRIB_PACKED__ WLAN_REASSOC_REQ_FRAME_T, *P_WLAN_REASSOC_REQ_FRAME_T;


typedef WLAN_ASSOC_RSP_FRAME_T WLAN_REASSOC_RSP_FRAME_T, *P_WLAN_REASSOC_RSP_FRAME_T;

/* 7.2.3.9 WLAN Management Frame - Probe Response Frame */
typedef WLAN_BEACON_FRAME_T WLAN_PROBE_RSP_FRAME_T, *P_WLAN_PROBE_RSP_FRAME_T;

/* 7.2.3.10 WLAN Management Frame - Authentication Frame */
typedef struct _WLAN_AUTH_FRAME_T {
    /* Authentication MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Authentication frame body */
    UINT_16     u2AuthAlgNum;               /* Authentication algorithm number */
    UINT_16     u2AuthTransSeqNo;           /* Authentication transaction sequence number */
    UINT_16     u2StatusCode;               /* Status code */
    UINT_8      aucInfoElem[1];             /* Various IEs for Fast BSS Transition */
} __KAL_ATTRIB_PACKED__ WLAN_AUTH_FRAME_T, *P_WLAN_AUTH_FRAME_T;


/* 7.2.3.11 WLAN Management Frame - Deauthentication Frame */
typedef struct _WLAN_DEAUTH_FRAME_T {
    /* Authentication MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Deauthentication frame body */
    UINT_16     u2ReasonCode;               /* Reason code */
} __KAL_ATTRIB_PACKED__ WLAN_DEAUTH_FRAME_T, *P_WLAN_DEAUTH_FRAME_T;



//3 Information Elements.
/* 7.3.2 Generic element format */
typedef struct _IE_HDR_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucInfo[1];
} __KAL_ATTRIB_PACKED__ IE_HDR_T, *P_IE_HDR_T;

/* 7.3.2.1 SSID element */
typedef struct _IE_SSID_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucSSID[ELEM_MAX_LEN_SSID];
} __KAL_ATTRIB_PACKED__ IE_SSID_T, *P_IE_SSID_T;

/* 7.3.2.2 Supported Rates element */
typedef struct _IE_SUPPORTED_RATE_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucSupportedRates[ELEM_MAX_LEN_SUP_RATES];
} __KAL_ATTRIB_PACKED__ IE_SUPPORTED_RATE_T, *P_IE_SUPPORTED_RATE_T;

/* 7.3.2.4 DS Parameter Set element */
typedef struct _IE_DS_PARAM_SET_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      ucCurrChnl;
} __KAL_ATTRIB_PACKED__ IE_DS_PARAM_SET_T, *P_IE_DS_PARAM_SET_T;

/* 7.3.2.6 TIM */
typedef struct _IE_TIM_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      ucDTIMCount;
    UINT_8      ucDTIMPeriod;
    UINT_8      ucBitmapControl;
    UINT_8      aucPartialVirtualMap[1];
} __KAL_ATTRIB_PACKED__ IE_TIM_T, *P_IE_TIM_T;

/* 7.3.2.7 IBSS Parameter Set element */
typedef struct _IE_IBSS_PARAM_SET_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_16     u2ATIMWindow;
} __KAL_ATTRIB_PACKED__ IE_IBSS_PARAM_SET_T, *P_IE_IBSS_PARAM_SET_T;

/* 7.3.2.8 Challenge Text element */
typedef struct _IE_CHALLENGE_TEXT_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucChallengeText[ELEM_MAX_LEN_CHALLENGE_TEXT];
} __KAL_ATTRIB_PACKED__ IE_CHALLENGE_TEXT_T, *P_IE_CHALLENGE_TEXT_T;

/* 7.3.2.9 Country information element */
#if CFG_SUPPORT_802_11D
/*! \brief COUNTRY_INFO_TRIPLET is defined for the COUNTRY_INFO_ELEM structure. */
typedef struct _COUNTRY_INFO_TRIPLET_T {
    UINT_8      ucParam1;                 /*!< If param1 >= 201, this triplet is referred to as
                                             Regulatory Triplet in 802_11J. */
    UINT_8      ucParam2;
    UINT_8      ucParam3;
} __KAL_ATTRIB_PACKED__ COUNTRY_INFO_TRIPLET_T, *P_COUNTRY_INFO_TRIPLET_T;

typedef struct _COUNTRY_INFO_SUBBAND_TRIPLET_T {
    UINT_8      ucFirstChnlNum;        /*!< First Channel Number */
    UINT_8      ucNumOfChnl;            /*!< Number of Channels */
    INT_8       cMaxTxPwrLv;        /*!< Maximum Transmit Power Level */
} __KAL_ATTRIB_PACKED__ COUNTRY_INFO_SUBBAND_TRIPLET_T, *P_COUNTRY_INFO_SUBBAND_TRIPLET_T;

typedef struct _COUNTRY_INFO_REGULATORY_TRIPLET_T {
    UINT_8      ucRegExtId;               /*!< Regulatory Extension Identifier, should
                                             be greater than or equal to 201 */
    UINT_8      ucRegClass;               /*!< Regulatory Class */
    UINT_8      ucCoverageClass;          /*!< Coverage Class, unsigned 1-octet value 0~31
                                           , 32~255 reserved */
} __KAL_ATTRIB_PACKED__ COUNTRY_INFO_REGULATORY_TRIPLET_T, *P_COUNTRY_INFO_REGULATORY_TRIPLET_T;

typedef struct _IE_COUNTRY_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucCountryStr[3];
    COUNTRY_INFO_SUBBAND_TRIPLET_T arSubbandTriplet[1];
} __KAL_ATTRIB_PACKED__ IE_COUNTRY_T, *P_IE_COUNTRY_T;
#endif /* CFG_SUPPORT_802_11D */

/* 7.3.2.13 ERP element */
typedef struct _IE_ERP_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      ucERP;
} __KAL_ATTRIB_PACKED__ IE_ERP_T, *P_IE_ERP_T;

/* 7.3.2.14 Extended Supported Rates element */
typedef struct _IE_EXT_SUPPORTED_RATE_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucExtSupportedRates[ELEM_MAX_LEN_EXTENDED_SUP_RATES];
} __KAL_ATTRIB_PACKED__ IE_EXT_SUPPORTED_RATE_T, *P_IE_EXT_SUPPORTED_RATE_T;



//3 7.4 Action Frame.
/* 7.4 Action frame format */
typedef struct _WLAN_ACTION_FRAME {
    /* Action MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* Action frame body */
    UINT_8      ucCategory;                 /* Category */
    UINT_8      ucActionDetails[1];         /* Action details */
} __KAL_ATTRIB_PACKED__ WLAN_ACTION_FRAME, *P_WLAN_ACTION_FRAME;


/* 7.4.2.1 ADDTS Request frame format */
typedef struct _ACTION_ADDTS_REQ_FRAME {
    /* ADDTS Request MAC header */
    UINT_16     frameCtrl;                  /* Frame Control */
    UINT_16     duration;                   /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     seqCtrl;                    /* Sequence Control */
    /* ADDTS Request frame body */
    UINT_8      ucCategory;                 /* Category */
    UINT_8      ucActionDetails[1];         /* Action details */
    UINT_8      ucDialogToken;              /* Dialog Token */
    UINT_8      aucInfoElem[1];             /* Information elements, such as
                                               TS Delay, and etc. */
} ACTION_ADDTS_REQ_FRAME, *P_ACTION_ADDTS_REQ_FRAME;


/* 7.4.2.2 ADDTS Response frame format */
typedef struct _ACTION_ADDTS_RSP_FRAME {
    /* ADDTS Response MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* ADDTS Response frame body */
    UINT_8      ucCategory;                 /* Category */
    UINT_8      ucActionDetails[1];         /* Action details */
    UINT_8      ucDialogToken;              /* Dialog Token */
    UINT_16     u2StatusCode;               /* Status Code */
    UINT_8      aucInfoElem[1];             /* Information elements, such as
                                               TS Delay, and etc. */
} ACTION_ADDTS_RSP_FRAME, *P_ACTION_ADDTS_RSP_FRAME;


/* 7.4.2.3 DELTS frame format */
typedef struct _ACTION_DELTS_FRAME {
    /* DELTS MAC header */
    UINT_16     u2FrameCtrl;                /* Frame Control */
    UINT_16     u2DurationID;               /* Duration */
    UINT_8      aucDestAddr[MAC_ADDR_LEN];  /* DA */
    UINT_8      aucSrcAddr[MAC_ADDR_LEN];   /* SA */
    UINT_8      aucBSSID[MAC_ADDR_LEN];     /* BSSID */
    UINT_16     u2SeqCtrl;                  /* Sequence Control */
    /* DELTS frame body */
    UINT_8      ucCategory;                 /* Category */
    UINT_8      ucActionDetails[1];         /* Action details */
    UINT_8      aucTsInfo[3];               /* TS Info */
} ACTION_DELTS_FRAME, *P_ACTION_DELTS_FRAME;

//#pragma pack(pop) // Not supported by Linux Colibri.
#pragma pack()


//3 Information Elements from WFA.
typedef struct _IE_WFA_T {
    UINT_8      ucId;
    UINT_8      ucLength;
    UINT_8      aucOui[3];
    UINT_8      ucOuiType;
    UINT_8      aucOuiSubTypeVersion[2];
    /*!< Please be noted. WPA defines a 16 bit field version
      instead of one subtype field and one version field*/
} IE_WFA_T, *P_IE_WFA_T;




/* Convert the ECWmin(max) to CWmin(max) */
#define ECW_TO_CW(_ECW)         ((1 << (_ECW)) - 1)

/* Convert the RCPI to dBm */
#define RCPI_TO_dBm(_rcpi)                          \
    ((PARAM_RSSI)(((_rcpi) > RCPI_HIGH_BOUND ? RCPI_HIGH_BOUND : (_rcpi)) >> 1) - NDBM_LOW_BOUND_FOR_RCPI)

/* Convert the dBm to RCPI */
#define dBm_TO_RCPI(_dbm)                           \
    (RCPI)( ( (((PARAM_RSSI)(_dbm) + NDBM_LOW_BOUND_FOR_RCPI) << 1) > RCPI_HIGH_BOUND) ? RCPI_HIGH_BOUND : \
            ( (((PARAM_RSSI)(_dbm) + NDBM_LOW_BOUND_FOR_RCPI) << 1) < RCPI_LOW_BOUND ? RCPI_LOW_BOUND : \
             (((PARAM_RSSI)(_dbm) + NDBM_LOW_BOUND_FOR_RCPI) << 1) ) )

/* Convert an unsigned char pointer to an information element pointer */
#define IE_ID(fp)               (((P_IE_HDR_T) fp)->ucId)
#define IE_LEN(fp)              (((P_IE_HDR_T) fp)->ucLength)
#define IE_SIZE(fp)             (ELEM_HDR_LEN + IE_LEN(fp))

#define SSID_IE(fp)             ((P_IE_SSID_T) fp)

#define SUP_RATES_IE(fp)        ((P_IE_SUPPORTED_RATE_T) fp)

#define DS_PARAM_IE(fp)         ((P_IE_DS_PARAM_SET_T) fp)

#define TIM_IE(fp)              ((P_IE_TIM_T) fp)

#define IBSS_PARAM_IE(fp)       ((P_IE_IBSS_PARAM_SET_T) fp)

#define ERP_INFO_IE(fp)         ((P_IE_ERP_T) fp)

#define EXT_SUP_RATES_IE(fp)    ((P_IE_EXT_SUPPORTED_RATE_T) fp)

#define WFA_IE(fp)              ((P_IE_WFA_T) fp)

#if CFG_SUPPORT_802_11D
#define COUNTRY_IE(fp)          ((P_IE_COUNTRY_T) fp)
#endif




/* The macro to check if the MAC address is B/MCAST Address */
#define IS_BMCAST_MAC_ADDR(_pucDestAddr)            \
    ((BOOLEAN) ( ((PUINT_8)(_pucDestAddr))[0] & BIT(0) ))

/* The macro to check if the MAC address is UCAST Address */
#define IS_UCAST_MAC_ADDR(_pucDestAddr)             \
    ((BOOLEAN) !( ((PUINT_8)(_pucDestAddr))[0] & BIT(0) ))

/* The macro to copy the MAC address */
#define COPY_MAC_ADDR(_pucDestAddr, _pucSrcAddr)    \
    kalMemCopy(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN)

/* The macro to check if two MAC addresses are equal */
#define EQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr)   \
    (!kalMemCmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

/* The macro to check if two MAC addresses are not equal */
#define UNEQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr) \
    (kalMemCmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))


/* The macro to check whether two SSIDs are equal */
#define EQUAL_SSID(pucSsid1, ucSsidLen1, pucSsid2, ucSsidLen2) \
    (((ucSsidLen1) == (ucSsidLen2)) && \
        !kalMemCmp(pucSsid1, pucSsid2, (ucSsidLen1 > ELEM_MAX_LEN_SSID ? ELEM_MAX_LEN_SSID : ucSsidLen1)))

/* The macro to check whether two SSIDs are equal */
#define UNEQUAL_SSID(pucSsid1, ucSsidLen1, pucSsid2, ucSsidLen2) \
    (((ucSsidLen1) != (ucSsidLen2)) || \
        kalMemCmp(pucSsid1, pucSsid2, (ucSsidLen1 > ELEM_MAX_LEN_SSID ? ELEM_MAX_LEN_SSID : ucSsidLen1)))

/* The macro to copy the SSID, the length of pucDestSsid should have at least 32 bytes */
#define COPY_SSID(pucDestSsid, ucDestSsidLen, pucSrcSsid, ucSrcSsidLen) \
    do { \
        ucDestSsidLen = ucSrcSsidLen; \
        if (ucSrcSsidLen) { \
            ASSERT(ucSrcSsidLen <= ELEM_MAX_LEN_SSID); \
            kalMemCopy(pucDestSsid, pucSrcSsid, ((ucSrcSsidLen > ELEM_MAX_LEN_SSID) ? ELEM_MAX_LEN_SSID : ucSrcSsidLen)); \
        } \
    } while (FALSE)

/* The macro to copy the IE */
#define COPY_IE(pucDestIE, pucSrcIE) \
    do { \
        kalMemCopy((PUINT_8)pucDestIE, \
                   (PUINT_8)pucSrcIE,\
                   IE_SIZE(pucSrcIE)); \
    } while (FALSE)

#define IE_FOR_EACH(_pucIEsBuf, _u2IEsBufLen, _u2Offset) \
    for ((_u2Offset) = 0; ((_u2Offset) < (_u2IEsBufLen)); \
        (_u2Offset) += IE_SIZE(_pucIEsBuf), (_pucIEsBuf) += IE_SIZE(_pucIEsBuf))


__KAL_INLINE__ VOID
macDataTypeCheck (
    VOID
    )
{
    DATA_STRUC_INSPECTING_ASSERT(sizeof(ETH_FRAME_T) == (15+(1)));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_MAC_HEADER_T) == 24);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_MAC_HEADER_QOS_T) == 26);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_MAC_HEADER_A4_T) == 30);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_MAC_HEADER_A4_QOS_T) == 32);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_MAC_MGMT_HEADER_T) == 24);
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_BEACON_FRAME_T) == (24+13));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_BEACON_FRAME_BODY_T) == (13));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_ASSOC_REQ_FRAME_T) == (24+5));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_ASSOC_RSP_FRAME_T) == (24+7));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_REASSOC_REQ_FRAME_T) == (24+11));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_AUTH_FRAME_T) == (24+7));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(WLAN_DEAUTH_FRAME_T) == (24+2));


    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_HDR_T) == (2+1));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_SSID_T) == (2+32));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_SUPPORTED_RATE_T) == (2+8));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_DS_PARAM_SET_T) == (2+1));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_IBSS_PARAM_SET_T) == (2+2));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_CHALLENGE_TEXT_T) == (2+253));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_ERP_T) == (2+1));
    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_EXT_SUPPORTED_RATE_T) == (2+255));

    DATA_STRUC_INSPECTING_ASSERT(sizeof(IE_WFA_T) == (2+6));

    return;
}

#endif /* _MAC_H */

