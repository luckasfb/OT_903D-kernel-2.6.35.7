




#ifndef _NIC_RATE_H
#define _NIC_RATE_H



#define CTS_PROTECTION_TYPE_802_11          0
#define CTS_PROTECTION_TYPE_PROPRIETARY     1

typedef enum _ENUM_AUTORATE_TYPE_T {
    AR_DISABLE = 0, /* No Autorate */
    AR_HW,          /* Hardware Autorate */
    AR_SW           /* Software Autorate */
} ENUM_AUTORATE_TYPE_T, *P_ENUM_AUTORATE_TYPE_T;

typedef enum _ENUM_AUTORATE_PROFILE_T {
    AR_PROFILE_THROUGHPUT = 0, /* Throughput is major index */
    AR_PROFILE_VOIP,           /* VoIP application */
    AR_PROFILE_NUM
} ENUM_AUTORATE_PROFILE_T, *P_ENUM_AUTORATE_PROFILE_T;

typedef struct _AR_PROFILE_T {
    ENUM_AUTORATE_PROFILE_T eProfile;
    UINT_16                 u2FailCount_up_limit;
    UINT_16                 u2FailCount_down_limit;
    UINT_8                  ucRCParam;
    UINT_8                  ucPERParam;
    UINT_8                  aucReserved[2]; /* Just for alignment */
} AR_PROFILE_T, *P_AR_PROFILE_T;

/* Autorate configuration type structure by *Mike* */
typedef struct _AR_CTRL_T {
    ENUM_AUTORATE_TYPE_T    eType;
    P_AR_PROFILE_T          prAR_Profile;
    UINT_8                  aucARRate1Index[WLAN_TABLE_SIZE];
    UINT_8                  aucReserved2[3]; /* Just for alignment */
    UINT_16                 au2ARBits[WLAN_TABLE_SIZE];
    UINT_8                  aucReserved3[2]; /* Just for alignment */
} AR_CTRL_T, *P_AR_CTRL_T;

typedef struct _RATE_INFO_T {
    BOOLEAN     fgCTSProtectionEnabled;
    UINT_8      ucCTSProtectionMode;

    UINT_8      ucRTSCTSRateIndex;
    BOOLEAN     fgIsRTSCTSRateShortPreamble;

    UINT_8      ucBasicRateIndex;
    BOOLEAN     fgIsBasicRateShortPreamble;

    UINT_16     u2RTSThreshold;

    UINT_8      aucAckCtsRateIndex[RATE_NUM];
    BOOLEAN     fgIsAckCtsRateShortPreamble;
} RATE_INFO_T, *P_RATE_INFO_T;




VOID
nicARInit (
    IN  P_ADAPTER_T prAdapter
    );

VOID
nicAREnable (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsHWAR /* TRUE: Hardware Autorate, FALSE: Software Autorate */
    );

VOID
nicARDisable (
    IN  P_ADAPTER_T prAdapter
    );

VOID
nicARSetRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2RateSet,
    IN BOOLEAN fgIsShortPreamble,
    IN UINT_8 ucRate1,
    IN UINT_8 ucWlanIdx,
    IN BOOLEAN fgIsRate1Assigned
    );

VOID
nicARReset (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucWlanIdx
    );

VOID
nicARSetProfile (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_AUTORATE_PROFILE_T eARProfile
    );

VOID
nicRateEnableProtection (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucProtectionMode
    );

VOID
nicRateDisableProtection (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicRateSetCTSRTSRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucRateIndex,
    IN BOOLEAN fgIsShortPreamble
    );

VOID
nicRateSetBasicRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucRateIndex,
    IN BOOLEAN fgIsShortPreamble
    );

VOID
nicRateSetRTSThreshold (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2RTSThreshold
    );

VOID
nicRateSetAckCtsRate(
    IN P_ADAPTER_T prAdapter,
    UINT_8 aucAckCtsRateIndex[],
    BOOLEAN fgIsShortPreamble
    );

#endif /* _NIC_RATE_H */


