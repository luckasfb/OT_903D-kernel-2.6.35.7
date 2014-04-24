




#ifndef _WLAN_OID_H
#define _WLAN_OID_H




#define PARAM_MAX_LEN_SSID                      32

#define PARAM_MAC_ADDR_LEN                      6

#define ETHERNET_HEADER_SZ                      14
#define ETHERNET_MIN_PKT_SZ                     60
#define ETHERNET_MAX_PKT_SZ                     1514

#define PARAM_MAX_LEN_RATES                     8
#define PARAM_MAX_LEN_RATES_EX                  16

#define PARAM_AUTH_REQUEST_REAUTH               0x01
#define PARAM_AUTH_REQUEST_KEYUPDATE            0x02
#define PARAM_AUTH_REQUEST_PAIRWISE_ERROR       0x06
#define PARAM_AUTH_REQUEST_GROUP_ERROR          0x0E

#define PARAM_EEPROM_READ_METHOD_READ           1
#define PARAM_EEPROM_READ_METHOD_GETSIZE        0

#define PARAM_WHQL_RSSI_MAX_DBM                 (-10)


/* Packet filter bit definitioin (UINT_32 bit-wise definition) */
#define PARAM_PACKET_FILTER_DIRECTED            0x00000001
#define PARAM_PACKET_FILTER_MULTICAST           0x00000002
#define PARAM_PACKET_FILTER_ALL_MULTICAST       0x00000004
#define PARAM_PACKET_FILTER_BROADCAST           0x00000008
#define PARAM_PACKET_FILTER_PROMISCUOUS         0x00000020
#define PARAM_PACKET_FILTER_ALL_LOCAL           0x00000080


#define PARAM_PACKET_FILTER_SUPPORTED   (PARAM_PACKET_FILTER_DIRECTED | \
                                         PARAM_PACKET_FILTER_MULTICAST | \
                                         PARAM_PACKET_FILTER_BROADCAST | \
                                         PARAM_PACKET_FILTER_ALL_MULTICAST)


#define BT_PROFILE_PARAM_LEN        8
typedef UINT_8          PARAM_MAC_ADDRESS[PARAM_MAC_ADDR_LEN];

typedef UINT_32         PARAM_KEY_INDEX;
typedef UINT_64         PARAM_KEY_RSC;
typedef INT_32          PARAM_RSSI;

typedef UINT_32         PARAM_FRAGMENTATION_THRESHOLD;
typedef UINT_32         PARAM_RTS_THRESHOLD;

typedef UINT_8          PARAM_RATES[PARAM_MAX_LEN_RATES];
typedef UINT_8          PARAM_RATES_EX[PARAM_MAX_LEN_RATES_EX];

typedef enum _ENUM_PARAM_PHY_TYPE_T {
    PHY_TYPE_802_11ABG = 0,             /*!< Can associated with 802.11abg AP, Scan dual band. */
    PHY_TYPE_802_11BG,                  /*!< Can associated with 802_11bg AP, Scan single band and not report 802_11a BSSs. */
    PHY_TYPE_802_11G,                   /*!< Can associated with 802_11g only AP, Scan single band and not report 802_11ab BSSs. */
    PHY_TYPE_802_11A,                   /*!< Can associated with 802_11a only AP, Scan single band and not report 802_11bg BSSs. */
    PHY_TYPE_802_11B,                   /*!< Can associated with 802_11b only AP, Scan single band and not report 802_11ag BSSs. */
    PHY_TYPE_NUM // 5
} ENUM_PARAM_PHY_TYPE_T, *P_ENUM_PARAM_PHY_TYPE_T;


typedef enum _ENUM_PARAM_AD_HOC_MODE_T {
    AD_HOC_MODE_11B = 0,           /*!< Create 11b IBSS if we support 802.11abg/802.11bg. */
    AD_HOC_MODE_MIXED_11BG,             /*!< Create 11bg mixed IBSS if we support 802.11abg/802.11bg/802.11g. */
    AD_HOC_MODE_11G,                    /*!< Create 11g only IBSS if we support 802.11abg/802.11bg/802.11g. */
    AD_HOC_MODE_11A,                    /*!< Create 11a only IBSS if we support 802.11abg. */
    AD_HOC_MODE_NUM // 4
} ENUM_PARAM_AD_HOC_MODE_T, *P_ENUM_PARAM_AD_HOC_MODE_T;


typedef enum _ENUM_PARAM_OP_MODE_T {
    NET_TYPE_IBSS = 0,                  /*!< Try to merge/establish an AdHoc, do periodic SCAN for merging. */
    NET_TYPE_INFRA,                     /*!< Try to join an Infrastructure, do periodic SCAN for joining. */
    NET_TYPE_AUTO_SWITCH,               /*!< Try to join an Infrastructure, if fail then try to merge or
                                         establish an AdHoc, do periodic SCAN for joining or merging. */
    NET_TYPE_DEDICATED_IBSS,            /*!< Try to merge an AdHoc first, if fail then establish AdHoc permanently, no more SCAN. */
    NET_TYPE_NUM // 4
} ENUM_PARAM_OP_MODE_T, *P_ENUM_PARAM_OP_MODE_T;


typedef enum _ENUM_PARAM_PREAMBLE_TYPE_T {
    PREAMBLE_TYPE_LONG = 0,
    PREAMBLE_TYPE_SHORT,
    PREAMBLE_TYPE_AUTO                  /*!< Try preamble short first, if fail tray preamble long. */
} ENUM_PARAM_PREAMBLE_TYPE_T, *P_ENUM_PARAM_PREAMBLE_TYPE_T;

typedef struct _PARAM_SSID_T {
    UINT_32  u4SsidLen;      /*!< SSID length in bytes. Zero length is broadcast(any) SSID */
    UINT_8   aucSsid[PARAM_MAX_LEN_SSID];
} PARAM_SSID_T, *P_PARAM_SSID_T;


typedef enum _ENUM_PARAM_MEDIA_STATE_T {
    PARAM_MEDIA_STATE_CONNECTED,
    PARAM_MEDIA_STATE_DISCONNECTED
} ENUM_PARAM_MEDIA_STATE_T, *P_ENUM_PARAM_MEDIA_STATE_T;


typedef enum _ENUM_PARAM_NETWORK_TYPE_T {
    NETWORK_TYPE_FH,
    NETWORK_TYPE_DS,
    NETWORK_TYPE_OFDM5,
    NETWORK_TYPE_OFDM24,
    NETWORK_TYPE_AUTOMODE,
    NETWORK_TYPE_NUM                    /*!< Upper bound, not real case */
} ENUM_PARAM_NETWORK_TYPE_T, *P_ENUM_PARAM_NETWORK_TYPE_T;


typedef struct _PARAM_NETWORK_TYPE_LIST {
    UINT_32                     NumberOfItems;      /*!< At least 1 */
    ENUM_PARAM_NETWORK_TYPE_T   eNetworkType [1];
} PARAM_NETWORK_TYPE_LIST, *PPARAM_NETWORK_TYPE_LIST;

typedef enum _ENUM_PARAM_AUTH_MODE_T
{
    AUTH_MODE_OPEN,                     /*!< Open system */
    AUTH_MODE_SHARED,                   /*!< Shared key */
    AUTH_MODE_AUTO_SWITCH,              /*!< Either open system or shared key */
    AUTH_MODE_WPA,
    AUTH_MODE_WPA_PSK,
    AUTH_MODE_WPA_NONE,                 /*!< For Ad hoc */
    AUTH_MODE_WPA2,
    AUTH_MODE_WPA2_PSK,
    AUTH_MODE_NUM                       /*!< Upper bound, not real case */
} ENUM_PARAM_AUTH_MODE_T, *P_ENUM_PARAM_AUTH_MODE_T;

/* Encryption types */
typedef enum _ENUM_WEP_STATUS_T
{
    ENUM_WEP_ENABLED,
    ENUM_ENCRYPTION1_ENABLED = ENUM_WEP_ENABLED,
    ENUM_WEP_DISABLED,
    ENUM_ENCRYPTION_DISABLED = ENUM_WEP_DISABLED,
    ENUM_WEP_KEY_ABSENT,
    ENUM_ENCRYPTION1_KEY_ABSENT = ENUM_WEP_KEY_ABSENT,
    ENUM_WEP_NOT_SUPPORTED,
    ENUM_ENCRYPTION_NOT_SUPPORTED = ENUM_WEP_NOT_SUPPORTED,
    ENUM_ENCRYPTION2_ENABLED,
    ENUM_ENCRYPTION2_KEY_ABSENT,
    ENUM_ENCRYPTION3_ENABLED,
    ENUM_ENCRYPTION3_KEY_ABSENT
} ENUM_ENCRYPTION_STATUS_T, *P_ENUM_ENCRYPTION_STATUS_T;

typedef enum _ENUM_RELOAD_DEFAULTS
{
    ENUM_RELOAD_WEP_KEYS
} PARAM_RELOAD_DEFAULTS, *P_PARAM_RELOAD_DEFAULTS;

#if SUPPORT_WPS
/* Management frame type to which application IE is added */
enum {
       IEEE80211_APPIE_FRAME_BEACON            = 0,
       IEEE80211_APPIE_FRAME_PROBE_REQ         = 1,
       IEEE80211_APPIE_FRAME_PROBE_RESP        = 2,
       IEEE80211_APPIE_FRAME_ASSOC_REQ         = 3,
       IEEE80211_APPIE_FRAME_ASSOC_RESP        = 4,
       IEEE80211_APPIE_NUM_OF_FRAME            = 5
};
#endif

typedef enum _ENUM_PARAM_GPIO2_MODE_T
{
    GPIO2_MODE_EEPROM = 0,          /*!< Follow EEPROM GPIO2 setting */
    GPIO2_MODE_INT,                 /*!< 2.8 voltage interrupt, INT_N */
    GPIO2_MODE_DAISY_INPUT          /*!< Input of daisy chain for OSC_EN */
} ENUM_PARAM_GPIO2_MODE_T, *P_ENUM_PARAM_GPIO2_MODE_T;

/*--------------------------------------------------------------*/
/*! \brief Struct definition to indicate specific event.                */
/*--------------------------------------------------------------*/
typedef enum _ENUM_STATUS_TYPE_T
{
    ENUM_STATUS_TYPE_AUTHENTICATION,
    ENUM_STATUS_TYPE_MEDIA_STREAM_MODE,
    ENUM_STATUS_TYPE_CANDIDATE_LIST,
    ENUM_STATUS_TYPE_NUM              /*!< Upper bound, not real case */
} ENUM_STATUS_TYPE_T, *P_ENUM_STATUS_TYPE_T;


typedef struct _PARAM_802_11_CONFIG_FH_T {
    UINT_32                  u4Length;         /*!< Length of structure */
    UINT_32                  u4HopPattern;     /*!< Defined as 802.11 */
    UINT_32                  u4HopSet;         /*!< to one if non-802.11 */
    UINT_32                  u4DwellTime;      /*!< In unit of Kusec */
} PARAM_802_11_CONFIG_FH_T, *P_PARAM_802_11_CONFIG_FH_T;

typedef struct _PARAM_802_11_CONFIG_T {
    UINT_32                  u4Length;         /*!< Length of structure */
    UINT_32                  u4BeaconPeriod;   /*!< In unit of Kusec */
    UINT_32                  u4ATIMWindow;     /*!< In unit of Kusec */
    UINT_32                  u4DSConfig;       /*!< Channel frequency in unit of kHz */
    PARAM_802_11_CONFIG_FH_T rFHConfig;
} PARAM_802_11_CONFIG_T, *P_PARAM_802_11_CONFIG_T;

typedef struct _PARAM_STATUS_INDICATION_T
{
    ENUM_STATUS_TYPE_T   eStatusType;
} PARAM_STATUS_INDICATION_T, *P_PARAM_STATUS_INDICATION_T;

typedef struct _PARAM_AUTH_REQUEST_T
{
    UINT_32             u4Length;             /*!< Length of this struct */
    PARAM_MAC_ADDRESS   arBssid;
    UINT_32             u4Flags;              /*!< Definitions are as follows */
} PARAM_AUTH_REQUEST_T, *P_PARAM_AUTH_REQUEST_T;

typedef struct _PARAM_AUTH_EVENT_T
{
    PARAM_STATUS_INDICATION_T   rStatus;
    PARAM_AUTH_REQUEST_T        arRequest[1];
} PARAM_AUTH_EVENT_T, *P_PARAM_AUTH_EVENT_T;

/*! \brief Capabilities, privacy, rssi and IEs of each BSSID */
typedef struct _PARAM_BSSID_EX_T {
    UINT_32                         u4Length;             /*!< Length of structure */
    PARAM_MAC_ADDRESS               arMacAddress;         /*!< BSSID */
    UINT_8                          Reserved[2];
    PARAM_SSID_T                    rSsid;               /*!< SSID */
    UINT_32                         u4Privacy;            /*!< Need WEP encryption */
    PARAM_RSSI                      rRssi;               /*!< in dBm */
    ENUM_PARAM_NETWORK_TYPE_T       eNetworkTypeInUse;
    PARAM_802_11_CONFIG_T           rConfiguration;
    ENUM_PARAM_OP_MODE_T            eOpMode;
    PARAM_RATES_EX                  rSupportedRates;
    UINT_32                         u4IELength;
    //PUINT_8                         pucIEWPA;
    //PUINT_8                         pucIERSN;
    UINT_8                          aucIEs[1];
} PARAM_BSSID_EX_T, *P_PARAM_BSSID_EX_T;

typedef struct _PARAM_BSSID_LIST_EX {
    UINT_32                         u4NumberOfItems;      /*!< at least 1 */
    PARAM_BSSID_EX_T                arBssid[1];
} PARAM_BSSID_LIST_EX_T, *P_PARAM_BSSID_LIST_EX_T;


typedef struct _PARAM_WEP_T
{
    UINT_32             u4Length;             /*!< Length of structure */
    UINT_32             u4KeyIndex;           /*!< 0: pairwise key, others group keys */
    UINT_32             u4KeyLength;          /*!< Key length in bytes */
    UINT_8              aucKeyMaterial[1];    /*!< Key content by above setting */
} PARAM_WEP_T, *P_PARAM_WEP_T;

/*! \brief Key mapping of BSSID */
typedef struct _PARAM_KEY_T
{
    UINT_32             u4Length;             /*!< Length of structure */
    UINT_32             u4KeyIndex;           /*!< KeyID */
    UINT_32             u4KeyLength;          /*!< Key length in bytes */
    PARAM_MAC_ADDRESS   arBSSID;              /*!< MAC address */
    PARAM_KEY_RSC       rKeyRSC;
    UINT_8              aucKeyMaterial[1];    /*!< Key content by above setting */
} PARAM_KEY_T, *P_PARAM_KEY_T;

typedef struct _PARAM_REMOVE_KEY_T
{
    UINT_32             u4Length;             /*!< Length of structure */
    UINT_32             u4KeyIndex;           /*!< KeyID */
    PARAM_MAC_ADDRESS   arBSSID;               /*!< MAC address */
} PARAM_REMOVE_KEY_T, *P_PARAM_REMOVE_KEY_T;


typedef enum _PARAM_POWER_MODE
{
    Param_PowerModeCAM,
    Param_PowerModeMAX_PSP,
    Param_PowerModeFast_PSP,
    Param_PowerModeMax                      /* Upper bound, not real case */
} PARAM_POWER_MODE, *PPARAM_POWER_MODE;

typedef enum _PARAM_DEVICE_POWER_STATE
{
    ParamDeviceStateUnspecified = 0,
    ParamDeviceStateD0,
    ParamDeviceStateD1,
    ParamDeviceStateD2,
    ParamDeviceStateD3,
    ParamDeviceStateMaximum
} PARAM_DEVICE_POWER_STATE, *PPARAM_DEVICE_POWER_STATE;

#if CFG_SUPPORT_802_11D

/*! \brief The enumeration definitions for OID_IPN_MULTI_DOMAIN_CAPABILITY */
typedef enum _PARAM_MULTI_DOMAIN_CAPABILITY {
    ParamMultiDomainCapDisabled,
    ParamMultiDomainCapEnabled
} PARAM_MULTI_DOMAIN_CAPABILITY, *P_PARAM_MULTI_DOMAIN_CAPABILITY;
#endif

typedef struct _COUNTRY_STRING_ENTRY {
    UINT_8  aucCountryCode[2];
    UINT_8  aucEnvironmentCode[2];
} COUNTRY_STRING_ENTRY, *P_COUNTRY_STRING_ENTRY;

/* Power management related definition and enumerations */
#define UAPSD_NONE                              0
#define UAPSD_AC0                               (BIT(0) | BIT(4))
#define UAPSD_AC1                               (BIT(1) | BIT(5))
#define UAPSD_AC2                               (BIT(2) | BIT(6))
#define UAPSD_AC3                               (BIT(3) | BIT(7))
#define UAPSD_ALL                               (UAPSD_AC0 | UAPSD_AC1 | UAPSD_AC2 | UAPSD_AC3)

typedef enum _ENUM_POWER_SAVE_PROFILE_T
{
    ENUM_PSP_CONTINUOUS_ACTIVE = 0,
    ENUM_PSP_CONTINUOUS_POWER_SAVE,
    ENUM_PSP_FAST_SWITCH,
    ENUM_PSP_NUM
} ENUM_POWER_SAVE_PROFILE_T, *PENUM_POWER_SAVE_PROFILE_T;


/*--------------------------------------------------------------*/
/*! \brief Set/Query testing type.                              */
/*--------------------------------------------------------------*/
typedef struct _PARAM_802_11_TEST_T
{
    UINT_32             u4Length;
    UINT_32             u4Type;
    union
    {
        PARAM_AUTH_EVENT_T  AuthenticationEvent;
        PARAM_RSSI          RssiTrigger;
    } u;
} PARAM_802_11_TEST_T, *P_PARAM_802_11_TEST_T;

/*--------------------------------------------------------------*/
/*! \brief Set/Query authentication and encryption capability.  */
/*--------------------------------------------------------------*/
typedef struct _PARAM_AUTH_ENCRYPTION_T
{
    ENUM_PARAM_AUTH_MODE_T      eAuthModeSupported;
    ENUM_ENCRYPTION_STATUS_T    eEncryptStatusSupported;
} PARAM_AUTH_ENCRYPTION_T, *P_PARAM_AUTH_ENCRYPTION_T;

typedef struct _PARAM_CAPABILITY_T
{
     UINT_32                  u4Length;
     UINT_32                  u4Version;
     UINT_32                  u4NoOfPMKIDs;
     UINT_32                  u4NoOfAuthEncryptPairsSupported;
     PARAM_AUTH_ENCRYPTION_T  arAuthenticationEncryptionSupported[1];
} PARAM_CAPABILITY_T, *P_PARAM_CAPABILITY_T;

typedef UINT_8   PARAM_PMKID_VALUE[16];

typedef struct _PARAM_BSSID_INFO_T
{
    PARAM_MAC_ADDRESS   arBSSID;
    PARAM_PMKID_VALUE   arPMKID;
} PARAM_BSSID_INFO_T, *P_PARAM_BSSID_INFO_T;

typedef struct _PARAM_PMKID_T
{
    UINT_32             u4Length;
    UINT_32             u4BSSIDInfoCount;
    PARAM_BSSID_INFO_T  arBSSIDInfo[1];
} PARAM_PMKID_T, *P_PARAM_PMKID_T;

/*! \brief PMKID candidate lists. */
typedef struct _PARAM_PMKID_CANDIDATE_T {
    PARAM_MAC_ADDRESS   arBSSID;
    UINT_32             u4Flags;
} PARAM_PMKID_CANDIDATE_T, *P_PARAM_PMKID_CANDIDATE_T;

/* Flags for PMKID Candidate list structure */
#define PARAM_PMKID_CANDIDATE_PREAUTH_ENABLED   0x01

typedef struct _PARAM_PMKID_CANDIDATE_LIST_T
{
    UINT_32                   u4Version;            /*!< Version */
    UINT_32                   u4NumCandidates;      /*!< How many candidates follow */
    PARAM_PMKID_CANDIDATE_T   arCandidateList[1];
} PARAM_PMKID_CANDIDATE_LIST_T, *P_PARAM_PMKID_CANDIDATE_LIST_T;


typedef struct _PARAM_CUSTOM_MCR_RW_STRUC_T {
    UINT_32             u4McrOffset;
    UINT_32             u4McrData;
} PARAM_CUSTOM_MCR_RW_STRUC_T, *P_PARAM_CUSTOM_MCR_RW_STRUC_T;


typedef struct _PARAM_CUSTOM_EEPROM_RW_STRUC_T {
    UINT_8              ucEepromMethod; /* For read only read: 1, query size: 0*/
    UINT_8              ucEepromIndex;
    UINT_8              reserved;
    UINT_16            u2EepromData;
} PARAM_CUSTOM_EEPROM_RW_STRUC_T, *P_PARAM_CUSTOM_EEPROM_RW_STRUC_T;

typedef struct _PARAM_CUSTOM_WMM_PS_TEST_STRUC_T {
    UINT_8  bmfgApsdEnAc;           /* b0~3: trigger-en AC0~3. b4~7: delivery-en AC0~3 */
    UINT_8  ucIsEnterPsAtOnce;      /* enter PS immediately without 5 second guard after connected */
    UINT_8  ucIsDisableUcTrigger;   /* not to trigger UC on beacon TIM is matched (under U-APSD) */
    UINT_8  reserved;
} PARAM_CUSTOM_WMM_PS_TEST_STRUC_T, *P_PARAM_CUSTOM_WMM_PS_TEST_STRUC_T;

/*  RF Test for rx status */
typedef struct  _PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T {
    UINT_32             u4IntRxOk;            /*!< number of packets that Rx ok from interrupt */
    UINT_32             u4IntCrcErr;          /*!< number of packets that CRC error from interrupt */
    UINT_32             u4IntShort;           /*!< number of packets that is short preamble from interrupt */
    UINT_32             u4IntLong;            /*!< number of packets that is long preamble from interrupt */
    UINT_32             u4PauRxPktCount;      /*!< number of packets that Rx ok from PAU */
    UINT_32             u4PauCrcErrCount;     /*!< number of packets that CRC error from PAU */
    UINT_32             u4PauRxFifoFullCount; /*!< number of packets that is short preamble from PAU */
    UINT_32             u4PauCCACount;        /*!< CCA rising edge count */
} PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T, *P_PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T;

/* Renbang : ++ (20100319)*/
typedef struct  _PARAM_CUSTOM_RFTEST_RX_STATUS_EX_STRUC_T {
	PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T rRxStatus;

    /* Renbang : RSSI record */
#if 1	
    UINT_32 			u4RssiMax;
    UINT_32 			u4RssiMin;
    UINT_32 			u4RssiAvg;
    UINT_32 			u4RssiVar;
#endif	
} PARAM_CUSTOM_RFTEST_RX_STATUS_EX_STRUC_T, *P_PARAM_CUSTOM_RFTEST_RX_STATUS_EX_STRUC_T;
/* Renbang : -- (20100319)*/

/*  RF Test for tx status */
typedef struct _PARAM_CUSTOM_RFTEST_TX_STATUS_STRUC_T {
    UINT_32             u4PktSentStatus;
    UINT_32             u4PktSentCount;
    UINT_16             u2AvgAlc;
    UINT_8              ucCckGainControl;
    UINT_8              ucOfdmGainControl;
} PARAM_CUSTOM_RFTEST_TX_STATUS_STRUC_T, *P_PARAM_CUSTOM_RFTEST_TX_STATUS_STRUC_T;

typedef struct _PTA_PARAM_T {
    UINT_32     u4BtCR0;  /*BTCER0*/
    UINT_32     u4BtCR1;  /*BTCER1, Bit[30,29]: 2'b00: 1-wire, 2'b01: 2-wire,
                                                2'b10: 3-wire, 2'b11: 4-wire */
    UINT_32     u4BtCR2;  /*BTCER2*/
    UINT_32     u4BtCR3;  /*BTCER3*/
} PTA_PARAM_T, *P_PTA_PARAM_T;

typedef struct _PARAM_CUSTOM_BT_COEXIST_T {
    PTA_PARAM_T         rPtaParam;

    UINT_32             u4IsEnableTxAutoFragmentForBT;
    UINT_32             eBTCoexistWindowType;
} PARAM_CUSTOM_BT_COEXIST_T, *P_PARAM_CUSTOM_BT_COEXIST_T;


typedef struct _PARAM_QOS_TSINFO {
    UINT_8           ucTrafficType;         /* Traffic Type: 1 for isochronous 0 for asynchronous */
    UINT_8           ucTid;                 /* TSID: must be between 8 ~ 15 */
    UINT_8           ucDirection;           /* direction */
    UINT_8           ucAccessPolicy;        /* access policy */
    UINT_8           ucAggregation;         /* aggregation */
    UINT_8           ucApsd;                /* APSD */
    UINT_8           ucuserPriority;        /* user priority */
    UINT_8           ucTsInfoAckPolicy;     /* TSINFO ACK policy */
    UINT_8           ucSchedule;            /* Schedule */
} PARAM_QOS_TSINFO, *P_PARAM_QOS_TSINFO;

typedef struct _PARAM_QOS_TSPEC {
    PARAM_QOS_TSINFO rTsInfo;               /* TS info field */
    UINT_16          u2NominalMSDUSize;     /* nominal MSDU size */
    UINT_16          u2MaxMSDUsize;         /* maximum MSDU size */
    UINT_32          u4MinSvcIntv;          /* minimum service interval */
    UINT_32          u4MaxSvcIntv;          /* maximum service interval */
    UINT_32          u4InactIntv;           /* inactivity interval */
    UINT_32          u4SpsIntv;             /* suspension interval */
    UINT_32          u4SvcStartTime;        /* service start time */
    UINT_32          u4MinDataRate;         /* minimum Data rate */
    UINT_32          u4MeanDataRate;        /* mean data rate */
    UINT_32          u4PeakDataRate;        /* peak data rate */
    UINT_32          u4MaxBurstSize;        /* maximum burst size */
    UINT_32          u4DelayBound;          /* delay bound */
    UINT_32          u4MinPHYRate;          /* minimum PHY rate */
    UINT_16          u2Sba;                 /* surplus bandwidth allowance */
    UINT_16          u2MediumTime;          /* medium time */
} PARAM_QOS_TSPEC, *P_PARAM_QOS_TSPEC;

typedef struct _PARAM_QOS_ADDTS_REQ_INFO {
    PARAM_QOS_TSPEC             rTspec;
} PARAM_QOS_ADDTS_REQ_INFO, *P_PARAM_QOS_ADDTS_REQ_INFO;

typedef struct _PARAM_VOIP_CONFIG {
    UINT_32         u4VoipTrafficInterval;  /* 0: disable VOIP configuration */
} PARAM_VOIP_CONFIG, *P_PARAM_VOIP_CONFIG;

/*802.11 Statistics Struct*/
typedef struct _PARAM_802_11_STATISTICS_STRUCT_T {
    UINT_32         u4Length;             // Length of structure
    LARGE_INTEGER   rTransmittedFragmentCount;
    LARGE_INTEGER   rMulticastTransmittedFrameCount;
    LARGE_INTEGER   rFailedCount;
    LARGE_INTEGER   rRetryCount;
    LARGE_INTEGER   rMultipleRetryCount;
    LARGE_INTEGER   rRTSSuccessCount;
    LARGE_INTEGER   rRTSFailureCount;
    LARGE_INTEGER   rACKFailureCount;
    LARGE_INTEGER   rFrameDuplicateCount;
    LARGE_INTEGER   rReceivedFragmentCount;
    LARGE_INTEGER   rMulticastReceivedFrameCount;
    LARGE_INTEGER   rFCSErrorCount;
} PARAM_802_11_STATISTICS_STRUCT_T, *P_PARAM_802_11_STATISTICS_STRUCT_T;

/* Linux Network Device Statistics Struct */
typedef struct _PARAM_LINUX_NETDEV_STATISTICS_T {
    UINT_32         u4RxPackets;
    UINT_32         u4TxPackets;
    UINT_32         u4RxBytes;
    UINT_32         u4TxBytes;
    UINT_32         u4RxErrors;
    UINT_32         u4TxErrors;
    UINT_32         u4Multicast;
} PARAM_LINUX_NETDEV_STATISTICS_T, *P_PARAM_LINUX_NETDEV_STATISTICS_T;


typedef enum _ENUM_PARAM_BT_COEXIST_WINDOW_T {
    BT_COEXIST_WINDOW_650 = 0,
    BT_COEXIST_WINDOW_1250,
    BT_COEXIST_WINDOW_2500,
    BT_COEXIST_WINDOW_TYPE_NUM
} ENUM_PARAM_BT_COEXIST_WINDOW_T, *P_ENUM_PARAM_BT_COEXIST_WINDOW_T;

/*  Pattern search relate configurations */
typedef struct  _PARAM_CUSTOM_PATTERN_SEARCH_CONFIG_STRUC_T {
    struct {
        BOOLEAN fgBcPatternMatchEnable;
        BOOLEAN fgMcPatternMatchEnable;
        BOOLEAN fgUcPatternMatchEnable;
        BOOLEAN fgBcPatternMatchOperation;
        BOOLEAN fgMcPatternMatchOperation;
        BOOLEAN fgUcPatternMatchOperation;
        BOOLEAN fgIpv6MatchCtrl;
    } rFunc;
    struct {
        IN UINT_8       ucIndex;
        IN BOOLEAN      fgCheckBcA1;
        IN BOOLEAN      fgCheckMcA1;
        IN BOOLEAN      fgCheckUcA1;
        IN BOOLEAN      fgIpv4Ip;
        IN BOOLEAN      fgIpv6Icmp;
        IN BOOLEAN      fgGarpIpEqual;
        IN BOOLEAN      fgArpCtrl;
        IN BOOLEAN      fgAndOp;
        IN BOOLEAN      fgNotOp;
        IN UINT_8       ucPatternMask;
        IN UINT_8       ucPatternOffset;
        IN UINT_32      au4Pattern[2];
    } arPattern[32];
} PARAM_CUSTOM_PATTERN_SEARCH_CONFIG_STRUC_T, *P_PARAM_CUSTOM_PATTERN_SEARCH_CONFIG_STRUC_T;


/* For Continuous Poll Profile Struct */
typedef struct _PARAM_CONTINUOUS_POLL_T {
    UINT_32         u4PollInterval; // 0: disable poll, else: enable with specific interval
} PARAM_CONTINUOUS_POLL_T, *P_PARAM_CONTINUOUS_POLL_T;

#if PTA_ENABLED

typedef enum _ENUM_BT_CMD_T {
    BT_CMD_PROFILE = 0,
    BT_CMD_UPDATE,
    BT_CMD_NUM
} ENUM_BT_CMD_T;

typedef enum _ENUM_BT_PROFILE_T {
    BT_PROFILE_CUSTOM = 0,
    BT_PROFILE_SCO,
    BT_PROFILE_ACL,
    BT_PROFILE_MIXED,
    BT_PROFILE_NO_CONNECTION,
    BT_PROFILE_NUM
} ENUM_BT_PROFILE_T;
#if PTA_NEW_BOARD_DESIGN
#define BTPPARAM_PTA_MODE_OFFSET  6
#define BTPPARAM_PTA_MODE_VALID   BIT(0)
#define BTPPARAM_PTA_MODE_SINGLE  0
#define BTPPARAM_PTA_MODE_DUAL    BIT(1)
#endif
typedef struct _PTA_PROFILE_T {
    ENUM_BT_PROFILE_T eBtProfile;
    union {
        UINT_8 aucBTPParams[BT_PROFILE_PARAM_LEN];
                                /*  0: sco reserved slot time,
                                    1: sco idle slot time,
                                    2: acl throughput,
                                    3: bt tx power,
                                    4: bt rssi
                                    5: VoIP interval
                                    6: BIT(0) Use this field, BIT(1) 0 apply single/ 1 dual PTA setting. 
                                */
        UINT_32 au4Btcr[4];
    } u;
} PTA_PROFILE_T, *P_PTA_PROFILE_T;



typedef enum _ENUM_BT_PROFILE_PARAM_T {
    BTP_PARAM_SCO_RESERVED_TIME = 0,
    BTP_PARAM_SCO_IDLE_TIME,
    BTP_PARAM_ACL_THROUGHPUT,
    BTP_PARAM_ACL_TX_POWER,
    BTP_PARAM_ACL_RX_POWER,
    BTP_PARAM_VOIP_INTERVAL,
    BTP_PARAM_NUM
} ENUM_BT_PROFILE_PARAM_T;

typedef struct _PTA_IPC_T {
    UINT_8      ucCmd;
    UINT_8      ucLen;
    union {
        PTA_PROFILE_T rProfile;
        UINT_8      aucBTPParams[BT_PROFILE_PARAM_LEN];
    } u;
} PARAM_PTA_IPC_T, *P_PARAM_PTA_IPC_T, PTA_IPC_T, *P_PTA_IPC_T;

#endif

/* Renbang : ++ (20100319)*/
typedef enum _ENUM_CFG_SRC_TYPE_T {
    CFG_SRC_TYPE_EEPROM,
    CFG_SRC_TYPE_NVRAM,
    CFG_SRC_TYPE_UNKNOWN,
    CFG_SRC_TYPE_NUM                    /*!< Upper bound, not real case */
} ENUM_CFG_SRC_TYPE_T, *P_ENUM_CFG_SRC_TYPE_T;

typedef enum _ENUM_EEPROM_TYPE_T {
    EEPROM_TYPE_NO,
    EEPROM_TYPE_PRESENT,
    EEPROM_TYPE_NUM                    /*!< Upper bound, not real case */
} ENUM_EEPROM_TYPE_T, *P_ENUM_EEPROM_TYPE_T;
/* Renbang : -- (20100319)*/
#if 0 /* SUPPORT_WAPI */
#define MAX_TURTLENECK_NUMBER       16
#define BKID_LEN                    16

typedef struct _WPI_BKID{
    UINT_8       ucBKIDIdentify[BKID_LEN];
} WPI_BKID;

typedef struct _PARAM_WAPI_ASSOC_INFO_T {
    UINT_8       ucElementID;
    UINT_8       ucLength;
    UINT_16      u2Version;
    UINT_16      u2AKMNum;
    UINT_32      u4AKMList[MAX_TURTLENECK_NUMBER];
    UINT_16      u2SingleCodeNum;
    UINT_32      u4SingleCodeList[MAX_TURTLENECK_NUMBER];
    UINT_32      u4MultiCode;
    UINT_16      u2WapiAbility;
    UINT_16      u2BKIDNumber;
    WPI_BKID     BKIDList[MAX_TURTLENECK_NUMBER];
} PARAM_WAPI_ASSOC_INFO_T, *P_PARAM_WAPI_ASSOC_INFO_T;
#endif

#if SUPPORT_WAPI
typedef enum _ENUM_KEY_TYPE {
    ENUM_WPI_PAIRWISE_KEY = 0,
    ENUM_WPI_GROUP_KEY
} ENUM_KEY_TYPE; 

typedef enum _ENUM_WPI_PROTECT_TYPE
{
    ENUM_WPI_NONE,
    ENUM_WPI_RX,
    ENUM_WPI_TX,
    ENUM_WPI_RX_TX
} ENUM_WPI_PROTECT_TYPE;

typedef struct _PARAM_WPI_KEY_T {
    ENUM_KEY_TYPE           eKeyType;
    ENUM_WPI_PROTECT_TYPE   eDirection;
    UINT_8                  ucKeyID;
    UINT_8                  aucRsv[3];
    UINT_8                  aucAddrIndex[12];
    UINT_32                 u4LenWPIEK;
    UINT_8                  aucWPIEK[256];
    UINT_32                 u4LenWPICK;
    UINT_8                  aucWPICK[256];
    UINT_8                  aucPN[16];
} PARAM_WPI_KEY_T, *P_PARAM_WPI_KEY_T;

#endif



/*--------------------------------------------------------------*/
/* Routines to set parameters or query information.             */
/*--------------------------------------------------------------*/
/***** Routines in wlan_oid.c *****/
WLAN_STATUS
wlanoidQueryNetworkTypesSupported(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryNetworkTypeInUse(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetNetworkTypeInUse (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryBssid(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetBssidListScan(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryBssidList(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetBssid(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetSsid(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQuerySsid(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryInfrastructureMode(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetInfrastructureMode(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryAuthMode(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetAuthMode(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetEncryptionStatus(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryEncryptionStatus(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetAddWep(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetRemoveWep(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetAddKey(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetRemoveKey(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetReloadDefaults(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetTest(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryCapability(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryFrequency (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetFrequency (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );


WLAN_STATUS
wlanoidQueryAtimWindow (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetAtimWindow (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );


WLAN_STATUS
wlanoidSetChannel (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryRssi(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryRssiTrigger(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetRssiTrigger(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryRtsThreshold (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetRtsThreshold (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetPatternConfig (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetPwrMgmtProfParam (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryPwrMgmtProfParam (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQuery802dot11PowerSaveProfile (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSet802dot11PowerSaveProfile (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       prSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryPmkid(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetPmkid(
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQuerySupportedRates(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryDesiredRates (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetDesiredRates (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryPermanentAddr (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryCurrentAddr (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryPermanentAddr (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryLinkSpeed(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryMcrRead (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetMcrWrite (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidOidTest(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );


WLAN_STATUS
wlanoidQueryEepromRead (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID        pvQueryBuffer,
    IN  UINT_32      u4QueryBufferLen,
    OUT PUINT_32     pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetEepromWrite (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryRfTestRxStatus (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID        pvQueryBuffer,
    IN  UINT_32      u4QueryBufferLen,
    OUT PUINT_32     pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryRfTestTxStatus (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID        pvQueryBuffer,
    IN  UINT_32      u4QueryBufferLen,
    OUT PUINT_32     pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryOidInterfaceVersion (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryVendorId(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryMulticastList(
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetMulticastList(
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryRcvError (
    IN  P_ADAPTER_T     prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryRcvNoBuffer (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryRcvCrcError (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryStatistics (
    IN  P_ADAPTER_T     prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryStatisticsForLinux (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryRcvOk(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryXmitOk(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryXmitError (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryXmitOneCollision (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryXmitMoreCollisions (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryXmitMaxCollisions (
    IN  P_ADAPTER_T       prAdapter,
    IN   PVOID           pvQueryBuffer,
    IN   UINT_32         u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );


WLAN_STATUS
wlanoidSetCurrentPacketFilter(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryCurrentPacketFilter (
    IN P_ADAPTER_T prAdapter,
    IN  PVOID      pvQueryBuffer,
    IN  UINT_32    u4QueryBufferLen,
    OUT PUINT_32   pu4QueryInfoLen
    );

#if PTA_ENABLED
WLAN_STATUS
wlanoidSetBtCoexistCtrl (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );
#endif


WLAN_STATUS
wlanoidSetAcpiDevicePowerState (
    IN P_ADAPTER_T prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryAcpiDevicePowerState (
    IN P_ADAPTER_T prAdapter,
    IN  PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

#if 0 /* SUPPORT_WPS */
WLAN_STATUS
wlanoidSetAppIE(
    IN P_ADAPTER_T        prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetFrameFilter(
    IN P_ADAPTER_T        prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );
#endif


WLAN_STATUS
wlanoidSetDisassociate (
    IN P_ADAPTER_T        prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryFragThreshold (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetFragThreshold (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetVoipConnectionStatus (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryVoipConnectionStatus (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetBgSsidParam (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidAddTS (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidDelTS (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryRoamingFunction (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetRoamingFunction (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetWiFiWmmPsTest (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryAdHocMode (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetAdHocMode (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryBeaconInterval (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetBeaconInterval (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetCurrentAddr (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

#if CFG_TCP_IP_CHKSUM_OFFLOAD
WLAN_STATUS
wlanoidSetCSUMOffload (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
);
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

#if CFG_LP_PATTERN_SEARCH_SLT
WLAN_STATUS
wlanoidQuerySltResult(
    IN P_ADAPTER_T  prAdapter,
    OUT  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    );


WLAN_STATUS
wlanoidSetSltMode(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );
#endif /* CFG_LP_PATTERN_SEARCH_SLT */

WLAN_STATUS
wlanoidSetIpAddress (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryCurrentCountry (
    IN P_ADAPTER_T prAdapter,
    IN PVOID pvQueryBuffer,
    IN UINT_32 u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetCurrentCountry (
    IN P_ADAPTER_T prAdapter,
    IN PVOID pvSetBuffer,
    IN UINT_32 u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

#if CFG_SUPPORT_802_11D
WLAN_STATUS
wlanoidQueryMultiDomainCap (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetMultiDomainCap (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );
#endif

WLAN_STATUS
wlanoidSetGPIO2Mode (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetContinuousPollProfile (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetDisableBeaconDetectionFunc (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryDisableBeaconDetectionFunc (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

#if defined(LINUX)
VOID
wlanoidQueryDrvStatusForLinuxProc (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
wlanoidQueryRxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
wlanoidSetRxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter
    );

VOID
wlanoidQueryTxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );

VOID
wlanoidSetTxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter
    );
#endif
#if PTA_ENABLED
WLAN_STATUS
wlanoidSetBT (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );


WLAN_STATUS
wlanoidQueryBT (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );
WLAN_STATUS
wlanoidQueryBtSingleAntenna (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidQueryPta (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetBtSingleAntenna (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetPta (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetFixedRxGain (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetPta (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );
#endif

WLAN_STATUS
wlanoidSetDisablePriavcyCheck (
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    );


WLAN_STATUS
wlanoidQueryContinuousPollInterval (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );
#if CFG_SUPPORT_EXT_CONFIG

WLAN_STATUS
wlanoidQueryNvramRead (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );
WLAN_STATUS
wlanoidSetNvramWrite (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );
#endif   

WLAN_STATUS
wlanoidQueryCfgSrcType(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
  );    
  
WLAN_STATUS
wlanoidQueryEepromType(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
  );     
#if SUPPORT_WAPI
WLAN_STATUS
wlanoidSetWapiMode(
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    );

WLAN_STATUS
wlanoidSetWapiAssocInfo(
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    );
#endif

#if SUPPORT_WAPI
WLAN_STATUS
wlanoidSetWapiKey(
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    );
#endif
#if PTA_NEW_BOARD_DESIGN
WLAN_STATUS
wlanoidSetPreferredAnt (
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    );

WLAN_STATUS
wlanoidQueryPreferredAnt (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );
#endif

WLAN_STATUS
wlanoidQueryWmmPsMode(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    );

WLAN_STATUS
wlanoidSetWmmPsMode (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    );


#endif /* _WLAN_OID_H */

