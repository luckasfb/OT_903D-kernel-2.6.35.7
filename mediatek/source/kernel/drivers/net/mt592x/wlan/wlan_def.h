





#ifndef _WLAN_DEF_H
#define _WLAN_DEF_H



/* PHY TYPE bit definitions */
#define PHY_TYPE_BIT_ERP        BIT(PHY_TYPE_ERP_INDEX)     /* ERP PHY (clause 19) */
//#define PHY_TYPE_BIT_DSSS       BIT(PHY_TYPE_DSSS_INDEX)    /* DSSS PHY (clause 15) */
#define PHY_TYPE_BIT_HR_DSSS    BIT(PHY_TYPE_HR_DSSS_INDEX) /* HR/DSSS PHY (clause 18) */
#define PHY_TYPE_BIT_OFDM       BIT(PHY_TYPE_OFDM_INDEX)    /* OFDM 5 GHz PHY (clause 17) */

/* PHY TYPE set definitions */
#define PHY_TYPE_SET_802_11ABG  (PHY_TYPE_BIT_OFDM | \
                                 PHY_TYPE_BIT_HR_DSSS | \
                                 PHY_TYPE_BIT_ERP)

#define PHY_TYPE_SET_802_11BG   (PHY_TYPE_BIT_HR_DSSS | \
                                 PHY_TYPE_BIT_ERP)

#define PHY_TYPE_SET_802_11A    (PHY_TYPE_BIT_OFDM)

#define PHY_TYPE_SET_802_11G    (PHY_TYPE_BIT_ERP)

#define PHY_TYPE_SET_802_11B    (PHY_TYPE_BIT_HR_DSSS)


/* Rate set bit definitions */
#define RATE_SET_BIT_1M         BIT(RATE_1M_INDEX)      /* Bit 0: 1M */
#define RATE_SET_BIT_2M         BIT(RATE_2M_INDEX)      /* Bit 1: 2M */
#define RATE_SET_BIT_5_5M       BIT(RATE_5_5M_INDEX)    /* Bit 2: 5.5M */
#define RATE_SET_BIT_11M        BIT(RATE_11M_INDEX)     /* Bit 3: 11M */
#define RATE_SET_BIT_22M        BIT(RATE_22M_INDEX)     /* Bit 4: 22M */
#define RATE_SET_BIT_33M        BIT(RATE_33M_INDEX)     /* Bit 5: 33M */
#define RATE_SET_BIT_6M         BIT(RATE_6M_INDEX)      /* Bit 8: 6M */
#define RATE_SET_BIT_9M         BIT(RATE_9M_INDEX)      /* Bit 9: 9M */
#define RATE_SET_BIT_12M        BIT(RATE_12M_INDEX)     /* Bit 10: 12M */
#define RATE_SET_BIT_18M        BIT(RATE_18M_INDEX)     /* Bit 11: 18M */
#define RATE_SET_BIT_24M        BIT(RATE_24M_INDEX)     /* Bit 12: 24M */
#define RATE_SET_BIT_36M        BIT(RATE_36M_INDEX)     /* Bit 13: 36M */
#define RATE_SET_BIT_48M        BIT(RATE_48M_INDEX)     /* Bit 14: 48M */
#define RATE_SET_BIT_54M        BIT(RATE_54M_INDEX)     /* Bit 15: 54M */

/* Rate set definitions */
#define RATE_SET_HR_DSSS            (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M | \
                                     RATE_SET_BIT_5_5M | \
                                     RATE_SET_BIT_11M)

#define RATE_SET_ERP                (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M | \
                                     RATE_SET_BIT_5_5M | \
                                     RATE_SET_BIT_11M | \
                                     RATE_SET_BIT_6M | \
                                     RATE_SET_BIT_9M | \
                                     RATE_SET_BIT_12M | \
                                     RATE_SET_BIT_18M | \
                                     RATE_SET_BIT_24M | \
                                     RATE_SET_BIT_36M | \
                                     RATE_SET_BIT_48M | \
                                     RATE_SET_BIT_54M)

#define RATE_SET_OFDM               (RATE_SET_BIT_6M | \
                                     RATE_SET_BIT_9M | \
                                     RATE_SET_BIT_12M | \
                                     RATE_SET_BIT_18M | \
                                     RATE_SET_BIT_24M | \
                                     RATE_SET_BIT_36M | \
                                     RATE_SET_BIT_48M | \
                                     RATE_SET_BIT_54M)

#define RATE_SET_ALL                 RATE_SET_ERP

#define BASIC_RATE_SET_HR_DSSS      (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M)

#define BASIC_RATE_SET_HR_DSSS_ERP  (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M | \
                                     RATE_SET_BIT_5_5M | \
                                     RATE_SET_BIT_11M)

#define BASIC_RATE_SET_ERP          (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M | \
                                     RATE_SET_BIT_5_5M | \
                                     RATE_SET_BIT_11M | \
                                     RATE_SET_BIT_6M | \
                                     RATE_SET_BIT_12M | \
                                     RATE_SET_BIT_24M)

#define BASIC_RATE_SET_OFDM         (RATE_SET_BIT_6M | \
                                     RATE_SET_BIT_12M | \
                                     RATE_SET_BIT_24M)

#define INITIAL_RATE_SET_RCPI_100    RATE_SET_ALL

#define INITIAL_RATE_SET_RCPI_80    (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M | \
                                     RATE_SET_BIT_5_5M | \
                                     RATE_SET_BIT_11M | \
                                     RATE_SET_BIT_6M | \
                                     RATE_SET_BIT_9M | \
                                     RATE_SET_BIT_12M | \
                                     RATE_SET_BIT_24M)

#define INITIAL_RATE_SET_RCPI_60    (RATE_SET_BIT_1M | \
                                     RATE_SET_BIT_2M | \
                                     RATE_SET_BIT_5_5M | \
                                     RATE_SET_BIT_11M | \
                                     RATE_SET_BIT_6M)

#define INITIAL_RATE_SET(_rcpi)     (INITIAL_RATE_SET_ ## _rcpi)

#define RCPI_100                    100 /* -60 dBm */
#define RCPI_80                     80  /* -70 dBm */
#define RCPI_60                     60  /* -80 dBm */


/* The number of RCPI records used to calculate their average value */
#define MAX_NUM_RCPI_RECORDS                    10

/* The number of RCPI records used to calculate their average value */
#define NO_RCPI_RECORDS                         -128
#define MAX_RCPI_DBM                            0
#define MIN_RCPI_DBM                            -100

typedef UINT_16                 PHY_TYPE, *P_PHY_TYPE;
typedef UINT_8                  RCPI, *P_RCPI;
typedef UINT_8                  ALC_VAL, *P_ALC_VAL;

/* NOTE(Kevin): We may design proprietary Operation Mode if a BSS carry some particular IE */
typedef enum _ENUM_OP_MODE_T {
    OP_MODE_INFRASTRUCTURE = 0,         /* Infrastructure */
    OP_MODE_IBSS,                       /* AdHoc */
    OP_MODE_RESERVED,                   /* Reserved */
    OP_MODE_NUM
} ENUM_OP_MODE_T, *P_ENUM_OP_MODE_T;

typedef enum _ENUM_MEDIA_STATE_T {
    MEDIA_STATE_DISCONNECTED = 0,
    MEDIA_STATE_CONNECTED
} ENUM_MEDIA_STATE, *P_ENUM_MEDIA_STATE;

typedef enum _ENUM_BSS_TYPE_T {
    BSS_TYPE_INFRASTRUCTURE = 1,
    BSS_TYPE_IBSS
} ENUM_BSS_TYPE_T, *P_ENUM_BSS_TYPE_T;

typedef enum _ENUM_PHY_TYPE_INDEX_T {
    PHY_TYPE_ERP_INDEX = 0,     /* ERP PHY (clause 19) */
    PHY_TYPE_HR_DSSS_INDEX,     /* HR/DSSS PHY (clause 18) */
    PHY_TYPE_OFDM_INDEX,        /* OFDM 5 GHz PHY (clause 17) */
    //PHY_TYPE_DSSS_INDEX,        /* DSSS PHY (clause 15) */ /* NOTE(Kevin): We don't use this now */
    PHY_TYPE_INDEX_NUM // 3
} ENUM_PHY_TYPE_INDEX_T, *P_ENUM_PHY_TYPE_INDEX_T;

typedef enum _ENUM_RATE_INDEX_T {
    RATE_1M_INDEX = 0,      /* 1M */
    RATE_2M_INDEX,          /* 2M */
    RATE_5_5M_INDEX,        /* 5.5M */
    RATE_11M_INDEX,         /* 11M */
    RATE_22M_INDEX,         /* 22M */
    RATE_33M_INDEX,         /* 33M */
    RATE_6M_INDEX,          /* 6M */
    RATE_9M_INDEX,          /* 9M */
    RATE_12M_INDEX,         /* 12M */
    RATE_18M_INDEX,         /* 18M */
    RATE_24M_INDEX,         /* 24M */
    RATE_36M_INDEX,         /* 36M */
    RATE_48M_INDEX,         /* 48M */
    RATE_54M_INDEX,         /* 54M */
    RATE_NUM // 14
} ENUM_RATE_INDEX_T, *P_ENUM_RATE_INDEX_T;

/* Used for HAL */
typedef enum _ENUM_PREMABLE_OPTION_T {
    PREAMBLE_DEFAULT_LONG_NONE = 0, /* LONG for PHY_TYPE_HR_DSSS, NONE for PHY_TYPE_OFDM */
    PREAMBLE_OPTION_SHORT, /* SHORT mandatory for PHY_TYPE_ERP, SHORT option for PHY_TYPE_HR_DSSS */
    PREAMBLE_OPTION_NUM
} ENUM_PREMABLE_OPTION_T, *P_ENUM_PREMABLE_OPTION_T;



typedef enum _ENUM_ADHOC_TYPE_T {
    ADHOC_TYPE_NONE,
    ADHOC_TYPE_11B,
    ADHOC_TYPE_11BG,
    ADHOC_TYPE_11G,
    ADHOC_TYPE_NUM
} ENUM_ADHOC_TYPE_T, *P_ENUM_ADHOC_TYPE_T;


typedef enum _ENUM_BAND_T {
    BAND_24G = 0,                       /* 2.4G */
    BAND_5G,                            /* 5G */
    BAND_TYPE_NUM
} ENUM_BAND_T, *P_ENUM_BAND_T;

typedef struct _RF_CHANNEL_INFO_T {
    UINT_8          ucChannelNum;
    ENUM_BAND_T     eBand;
} RF_CHANNEL_INFO_T, *P_RF_CHANNEL_INFO_T;

typedef struct _RF_INFO_T {
    ENUM_BAND_T     eBand;
    UINT_8          ucChannelNum;
    UINT_8          ucaSlotTime;
} RF_INFO_T, *P_RF_INFO_T;

/* Session for Thermo function */
typedef enum _ENUM_THERMO_STATE_T {
    UNDEF_TEMP = 0,
    LOW_TEMP,
    NORMAL_TEMP,
    HIGH_TEMP
} ENUM_THERMO_STATE_T, *P_ENUM_THERMO_STATE_T;



#define WLAN_GET_FIELD_16(_memAddr_p, _value_p) \
        { \
            PUINT_8 __cp = (PUINT_8) (_memAddr_p); \
            *(PUINT_16)(_value_p) = ((UINT_16) __cp[0]) | ((UINT_16) __cp[1] << 8); \
        }

#define WLAN_GET_FIELD_32(_memAddr_p, _value_p) \
        { \
            PUINT_8 __cp = (PUINT_8) (_memAddr_p); \
            *(PUINT_32)(_value_p) = ((UINT_32) __cp[0])       | ((UINT_32) __cp[1] << 8) | \
                                    ((UINT_32) __cp[2] << 16) | ((UINT_32) __cp[3] << 24); \
        }

#define WLAN_GET_FIELD_64(_memAddr_p, _value_p) \
        { \
            PUINT_8 __cp = (PUINT_8) (_memAddr_p); \
            *(PUINT_64)(_value_p) = \
                ((UINT_64) __cp[0])       | ((UINT_64) __cp[1] << 8)  | \
                ((UINT_64) __cp[2] << 16) | ((UINT_64) __cp[3] << 24) | \
                ((UINT_64) __cp[4] << 32) | ((UINT_64) __cp[5] << 40) | \
                ((UINT_64) __cp[6] << 48) | ((UINT_64) __cp[7] << 56); \
        }

#define WLAN_SET_FIELD_16(_memAddr_p, _value) \
        { \
            PUINT_8 __cp = (PUINT_8) (_memAddr_p); \
            __cp[0] = (UINT_8) (_value); \
            __cp[1] = (UINT_8) ((_value) >> 8); \
        }

#define WLAN_SET_FIELD_32(_memAddr_p, _value) \
        { \
            PUINT_8 __cp = (PUINT_8) (_memAddr_p); \
            __cp[0] = (UINT_8) (_value); \
            __cp[1] = (UINT_8) ((_value) >> 8); \
            __cp[2] = (UINT_8) ((_value) >> 16); \
            __cp[3] = (UINT_8) ((_value) >> 24); \
        }

#ifdef SUPPORT_WAPI
#define WLAN_SET_FIELD_16_BE(_memAddr_p, _value) \
        { \
            PUINT_8 __cp = (PUINT_8) (_memAddr_p); \
            __cp[0] = (UINT_8) ((_value) >> 8); \
            __cp[1] = (UINT_8) (_value); \
        }

/* This macro increase 128-bit value by index */
#define INC_128_BITS(value_p) \
    {\
        *((PUINT_8)(value_p) + 0) += 1; \
        if (*((PUINT_8)(value_p) + 0) == 0) { \
            *((PUINT_8)(value_p) + 1) += 1; \
            if (*((PUINT_8)(value_p) + 1) == 0) { \
                *((PUINT_8)(value_p) + 2) += 1; \
                if (*((PUINT_8)(value_p) + 2) == 0) { \
                    *((PUINT_8)(value_p) + 3) += 1; \
                    if (*((PUINT_8)(value_p) + 3) == 0) { \
                        *((PUINT_8)(value_p) + 4) += 1; \
                        if (*((PUINT_8)(value_p) + 4) == 0) { \
                            *((PUINT_8)(value_p) + 5) += 1; \
                            if (*((PUINT_8)(value_p) + 5) == 0) { \
                                *((PUINT_8)(value_p) + 6) += 1; \
                                if (*((PUINT_8)(value_p) + 6) == 0) { \
                                    *((PUINT_8)(value_p) + 7) += 1; \
                                    if (*((PUINT_8)(value_p) + 7) == 0) { \
                                        *((PUINT_8)(value_p) + 8) += 1; \
                                        if (*((PUINT_8)(value_p) + 8) == 0) { \
                                            *((PUINT_8)(value_p) + 9) += 1; \
                                            if (*((PUINT_8)(value_p) + 9) == 0) { \
                                                *((PUINT_8)(value_p) + 10) += 1; \
                                                if (*((PUINT_8)(value_p) + 10) == 0) { \
                                                    *((PUINT_8)(value_p) + 11) += 1; \
                                                    if (*((PUINT_8)(value_p) + 11) == 0) { \
                                                        *((PUINT_8)(value_p) + 12) += 1; \
                                                        if (*((PUINT_8)(value_p) + 12) == 0) { \
                                                            *((PUINT_8)(value_p) + 13) += 1; \
                                                            if (*((PUINT_8)(value_p) + 13) == 0) { \
                                                                *((PUINT_8)(value_p) + 14) += 1; \
                                                                if (*((PUINT_8)(value_p) + 14) == 0) { \
                                                                    *((PUINT_8)(value_p) + 15) += 1; \
                                                                } \
                                                            } \
                                                        } \
                                                    } \
                                                } \
                                            } \
                                        } \
                                    } \
                                } \
                            } \
                        } \
                    } \
                } \
            } \
        } \
    }
#endif

/*----------------------------------------------------------------------------*/
/* Routines in assoc.c                                                        */
/*----------------------------------------------------------------------------*/
WLAN_STATUS
assocSendReAssocReqFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc,
    IN BOOLEAN fgIsReAssoc
    );

WLAN_STATUS
assocSendDisAssocFrame(
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2ReasonCode,
    IN PFN_TX_DONE_HANDLER pfTxDoneHandler
    );

WLAN_STATUS
assocCheckTxReAssocReqFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN BOOLEAN fgIsReAssoc
    );

WLAN_STATUS
assocProcessRxReAssocRspFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN P_PEER_BSS_INFO_T prPeerBssInfo,
    IN BOOLEAN fgIsReAssoc,
    OUT PUINT_16 pu2StatusCode
    );

WLAN_STATUS
assocProcessRxDisassocFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN UINT_8 aucBSSID[],
    OUT PUINT_16 pu2ReasonCode
    );

/*----------------------------------------------------------------------------*/
/* Routines in auth.c                                                         */
/*----------------------------------------------------------------------------*/
WLAN_STATUS
authSendAuthFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_PEER_BSS_INFO_T prPeerBssInfo,
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum
    );

WLAN_STATUS
authSendDeauthFrame (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2ReasonCode,
    IN UINT_8 ucTQ,
    IN PFN_TX_DONE_HANDLER pfTxDoneHandler
    );

WLAN_STATUS
authCheckTxAuthFrame (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum
    );

WLAN_STATUS
authProcessRxAuthFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN P_PEER_BSS_INFO_T prPeerBssInfo,
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum,
    OUT PUINT_16 pu2StatusCode
    );

WLAN_STATUS
authProcessRxDeauthFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN UINT_8 aucBSSID[],
    OUT PUINT_16 pu2ReasonCode
    );



/*----------------------------------------------------------------------------*/
/* Routines in bss.c                                                          */
/*----------------------------------------------------------------------------*/
VOID
bssProcessBeacon (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBSSDesc,
    IN P_SW_RFB_T prSwRfb
    );

VOID
bssUpdateTxRateForControlFrame (
    IN P_ADAPTER_T prAdapter
    );

VOID
bssLeave (
    IN P_ADAPTER_T prAdapter
    );

/*----------------------------------------------------------------------------*/
/* Routines in parse.c                                                        */
/*----------------------------------------------------------------------------*/
BOOLEAN
parseCheckForWFAInfoElem (
    IN PUINT_8 pucBuf,
    OUT PUINT_8 pucOuiType,
    OUT PUINT_16 pu2SubTypeVersion
    );

/*----------------------------------------------------------------------------*/
/* Routines in rate.c                                                         */
/*----------------------------------------------------------------------------*/
VOID
rateGetRateSetFromIEs (
    IN P_IE_SUPPORTED_RATE_T prIeSupportedRate,
    IN P_IE_EXT_SUPPORTED_RATE_T prIeExtSupportedRate,
    OUT PUINT_16 pu2OperationalRateSet,
    OUT PUINT_16 pu2BSSBasicRateSet,
    OUT PBOOLEAN pfgIsUnknownBSSBasicRate
    );

VOID
rateGetDataRatesFromRateSet (
    IN UINT_16 u2OperationalRateSet,
    IN UINT_16 u2BSSBasicRateSet,
    OUT PUINT_8 pucDataRates,
    OUT PUINT_8 pucDataRatesLen
    );

VOID
rateGetRateSetFromDataRates (
    IN PUINT_8 pucDataRates,
    IN UINT_8 ucDataRatesLen,
    OUT PUINT_16 pu2RateSet
    );

VOID
rateSetAckCtsDataRatesFromRateSet (
    IN UINT_16 u2OperationalRateSet,
    IN UINT_16 u2BSSBasicRateSet,
    IN OUT UINT_8 aucAckCtsRateIndex[]
    );

BOOLEAN
rateGetHighestRateIndexFromRateSet (
    IN UINT_16 u2RateSet,
    OUT PUINT_8 pucHighestRateIndex
    );

BOOLEAN
rateGetLowestRateIndexFromRateSet (
    IN UINT_16 u2RateSet,
    OUT PUINT_8 pucLowestRateIndex
    );

BOOLEAN
rateGetBestInitialRateIndex (
    IN UINT_16 u2RateSet,
    IN RCPI rRcpi,
    OUT PUINT_8 pucInitialRateIndex
    );


#endif /* _WLAN_DEF_H */

