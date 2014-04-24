





#ifndef _RSN_H
#define _RSN_H



/* ----- Definitions for Cipher Suite Selectors ----- */
#define RSN_CIPHER_SUITE_USE_GROUP_KEY  0x00AC0F00
#define RSN_CIPHER_SUITE_WEP40          0x01AC0F00
#define RSN_CIPHER_SUITE_TKIP           0x02AC0F00
#define RSN_CIPHER_SUITE_CCMP           0x04AC0F00
#define RSN_CIPHER_SUITE_WEP104         0x05AC0F00

#define WPA_CIPHER_SUITE_NONE           0x00F25000
#define WPA_CIPHER_SUITE_WEP40          0x01F25000
#define WPA_CIPHER_SUITE_TKIP           0x02F25000
#define WPA_CIPHER_SUITE_CCMP           0x04F25000
#define WPA_CIPHER_SUITE_WEP104         0x05F25000

/* ----- Definitions for Authentication and Key Management Suite Selectors ----- */
#define RSN_AKM_SUITE_NONE              0x00AC0F00
#define RSN_AKM_SUITE_802_1X            0x01AC0F00
#define RSN_AKM_SUITE_PSK               0x02AC0F00

#define WPA_AKM_SUITE_NONE              0x00F25000
#define WPA_AKM_SUITE_802_1X            0x01F25000
#define WPA_AKM_SUITE_PSK               0x02F25000

#define ELEM_ID_RSN_LEN_FIXED           20  /* The RSN IE len for associate request */

#define ELEM_ID_WPA_LEN_FIXED           22  /* The RSN IE len for associate request */

#define MASK_RSNIE_CAP_PREAUTH          BIT(0)

#define GET_SELECTOR_TYPE(x)           ((UINT_8)(((x) >> 24) & 0x000000FF))
#define SET_SELECTOR_TYPE(x, y)         x = (((x) & 0x00FFFFFF) | (((UINT_32)(y) << 24) & 0xFF000000))

/* Cihpher suite flags */
#define CIPHER_FLAG_NONE                        0x00000000
#define CIPHER_FLAG_WEP40                       0x00000001 /* BIT 1 */
#define CIPHER_FLAG_TKIP                        0x00000002 /* BIT 2 */
#define CIPHER_FLAG_CCMP                        0x00000008 /* BIT 4 */
#define CIPHER_FLAG_WEP104                      0x00000010 /* BIT 5 */
#define CIPHER_FLAG_WEP128                      0x00000020 /* BIT 6 */

#define WAIT_TIME_IND_PMKID_CANDICATE_SEC       10 /* seconds */
#define TKIP_COUNTERMEASURE_SEC                 60 /* seconds */

/* Structure of RSN Information */
typedef struct _RSN_INFO_T {
    UINT_8          ucElemId;
    UINT_16         u2Version;
    UINT_32         u4GroupKeyCipherSuite;
    UINT_32         u4PairwiseKeyCipherSuiteCount;
    UINT_32         au4PairwiseKeyCipherSuite[MAX_NUM_SUPPORTED_CIPHER_SUITES];
    UINT_32         u4AuthKeyMgtSuiteCount;
    UINT_32         au4AuthKeyMgtSuite[MAX_NUM_SUPPORTED_CIPHER_SUITES];
    UINT_16         u2RsnCap;
    BOOLEAN         fgRsnCapPresent;
} RSN_INFO_T, *P_RSN_INFO_T;

/* RSN Information element format */
typedef struct _RSN_INFO_ELEM_T {
    UCHAR           ucElemId;
    UCHAR           ucLength;
    UINT_16         u2Version;
    UINT_32         u4GroupKeyCipherSuite;
    UINT_16         u2PairwiseKeyCipherSuiteCount;
    UCHAR           aucPairwiseKeyCipherSuite1[4];
} RSN_INFO_ELEM_T, *P_RSN_INFO_ELEM_T;

/* WPA Information element format */
typedef struct _WPA_INFO_ELEM_T {
    UCHAR       ucElemId;
    UCHAR       ucLength;
    UCHAR       aucOui[3];
    UCHAR       ucOuiType;
    UINT_16     u2Version;
    UINT_32     u4GroupKeyCipherSuite;
    UINT_16     u2PairwiseKeyCipherSuiteCount;
    UCHAR       aucPairwiseKeyCipherSuite1[4];
} WPA_INFO_ELEM_T, *P_WPA_INFO_ELEM_T;



#define RSN_IE(fp)              ((P_RSN_INFO_ELEM_T) fp)
#define WPA_IE(fp)              ((P_WPA_INFO_ELEM_T) fp)


BOOLEAN
rsnRxProcessMSDU(
    IN P_ADAPTER_T          prAdapter,
    IN P_SW_RFB_T           prSWRfb
    );

BOOLEAN
rsnTxProcessMSDU(
    IN P_ADAPTER_T          prAdapter,
    IN P_PACKET_INFO_T      prPacketInfo
    );

BOOLEAN
rsnParseRsnIE(
    IN  P_RSN_INFO_ELEM_T   prInfoElem,
    OUT P_RSN_INFO_T        prRsnInfo
    );

BOOLEAN
rsnParseWpaIE(
    IN  P_WPA_INFO_ELEM_T   prInfoElem,
    OUT P_RSN_INFO_T        prWpaInfo
    );

BOOLEAN
rsnSearchSupportedCipher(
    IN  P_ADAPTER_T         prAdapter,
    IN  UINT_32             u4Cipher,
    OUT PUINT_32            pu4Index
    );

BOOLEAN
rsnSearchAKMSuite(
    IN  P_ADAPTER_T         prAdapter,
    IN  UINT_32             u4AkmSuite,
    OUT PUINT_32            pu4Index
    );

BOOLEAN
rsnPerformPolicySelection(
    IN  P_ADAPTER_T         prAdapter,
    IN  P_BSS_DESC_T        prBss
    );

UINT_8
rsnGenerateWPARSNIE(
    IN  P_ADAPTER_T         prAdapter,
    IN  P_BSS_DESC_T        prCurrentBss,
    IN  PUINT_8             pucIeStartAddr
    );

UINT_8
rsnGenerateWpaNoneIE(
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucIeStartAddr
    );

VOID
rsnGenerateAuthReqEvent(
    IN  P_ADAPTER_T         prAdapter,
    IN BOOLEAN              fgFlags
    );

VOID
rsnTkipHandleMICFailure(
    IN P_ADAPTER_T          prAdapter,
    IN  BOOLEAN             fgErrorKeyType
    );

VOID
rsnSelectPmkidCandidateList(
    IN P_ADAPTER_T          prAdapter,
    IN P_BSS_DESC_T         prBssDesc
    );

VOID
rsnUpdatePmkidCandidateList(
    IN P_ADAPTER_T          prAdapter,
    IN P_BSS_DESC_T         prBssDesc
    );

BOOLEAN
rsnSearchPmkidEntry(
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid,
    OUT PUINT_32            pu4EntryIndex
    );

BOOLEAN
rsnCheckPmkidCandicate(
    IN P_ADAPTER_T          prAdapter
   );

VOID
rsnCheckPmkidCache(
    IN P_ADAPTER_T          prAdapter,
    IN P_BSS_DESC_T         prBss
    );

VOID
rsnGeneratePmkidIndication(
    IN P_ADAPTER_T          prAdapter,
    IN UINT_32              u4Flags
    );


#endif /* _RSN_H */

