




#ifndef _NIC_PRIVACY_H
#define _NIC_PRIVACY_H




/*! \brief key structure for NIC key configuration */
typedef struct _SW_KEY_STRUC_T {
    UCHAR               ucRCA1;               /* check U/B/M of A1 */
    BOOLEAN             fgRCA2;               /* check address 2 */
    BOOLEAN             fgRCID;               /* check key ID */
    UINT_8              ucKeyId;              /* key ID, range: 0~3 */
    UINT_8              ucCipher;             /* cipher, range: 0~6 */
    BOOLEAN             fgIKV;
    UINT_8              aucAddr[MAC_ADDR_LEN]; /* mapping address */
    UINT_8              aucKeyMaterial[TKIP_KEY_LEN - (2 * MIC_KEY_LEN)];   /* key material */
    UINT_8              aucRxMicMaterial[MIC_KEY_LEN];  /* Rx MIC key material */
    UINT_8              aucTxMicMaterial[MIC_KEY_LEN];  /* Tx MIC key material */
} SW_KEY_STRUC_T, *P_SW_KEY_STRUC_T;

/*! \brief wlan table setting exclude key structure */
typedef struct _WT_CTRL_STRUC_T {
    BOOLEAN             fgTV;                 /*!< Tx Entry valid */
    BOOLEAN             fgTKV;                /*!< Tx Key valid */
    BOOLEAN             fgRV;                 /*!< Rx Entry valid */
    BOOLEAN             fgRKV;                /*!< Rx Key valid */
    BOOLEAN             fgT1X;                /*!< Tx Port Control bit */
    BOOLEAN             fgR1X;                /*!< Rx Port Control bit */
    UINT_8              ucMuar;               /*!< the index for rx a1 muar match */
    BOOLEAN             fgQoS;                /*!< QoS setting bit */
    BOOLEAN             fgAntenna;            /*!< Antenna setting bit */
    UINT_8              ucRate1;              /*!< Rate 1 setting */
    UINT_8              ucRate2;              /*!< Rate 2 setting */
    UINT_8              ucRate3;              /*!< Rate 3 setting */
    UINT_8              ucRate4;              /*!< Rate 4 setting */
} WT_CTRL_STRUC_T, *P_WT_CTRL_STRUC_T;

/*! \brief key entry configuration structure */
typedef struct _WLAN_ENTRY_CTRL_T {
    BOOLEAN             fgUsed;
    P_STA_RECORD_T      prSta;
    WT_CTRL_STRUC_T     rCtrl;
    SW_KEY_STRUC_T      rSWKey;
} WLAN_ENTRY_CTRL_T, *P_WLAN_ENTRY_CTRL_T;




VOID
nicPrivacyEnableHwTxPortControl(
    IN P_ADAPTER_T          prAdapter
    );

VOID
nicPrivacyInitialize(
    IN P_ADAPTER_T          prAdapter
    );

BOOLEAN
nicPrivacyInvalidKey(
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid
   );

VOID
nicPrivacyPortControl(
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid,
    IN BOOLEAN              fgTxPort,
    IN BOOLEAN              fgRxPort
   );

BOOLEAN
nicPrivacySetKeyEntry(
    IN P_ADAPTER_T           prAdapter,
    IN BOOLEAN               fgTxKey,
    IN PUINT_8               pucBssid,
    IN UINT_8                ucKeyId,
    IN PUINT_8               pucKeyMaterial,
    IN UINT_8                ucKeyLen,
    IN UINT_8                ucCipherMode,
    IN UINT_8                ucTxMicOffset,
    IN UINT_8                ucRxMicOffset
    );

BOOLEAN
nicPrivacyMatchForEntry(
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucBssid,
    IN  UINT_8              ucKeyId,
    OUT PUINT_8             pucEntryIndex
    );

BOOLEAN
nicPrivacyUpdateBySta(
    IN P_ADAPTER_T           prAdapter,
    IN PUINT_8               pucBSSID,
    IN P_STA_RECORD_T        prSta,
    IN PBOOLEAN              pfgQoS,
    IN PBOOLEAN              pfgAntenna,
    IN PUINT_8               pucMuar,
    IN PUINT_8               pucRate1,
    IN PUINT_8               pucRate2,
    IN PUINT_8               pucRate3,
    IN PUINT_8               pucRate4
    );

VOID
nicPrivacySetKeyToTemplateEntry (
    IN P_ADAPTER_T           prAdapter,
    IN PUINT_8               pucBssid,
    IN PUINT_8               pucKeyMaterial,
    IN UINT_8                ucKeyLen,
    IN UINT_8                ucCipherMode,
    IN UINT_8                ucTxMicOffset,
    IN UINT_8                ucRxMicOffset
    );

P_STA_RECORD_T
nicPrivacyGetWlanIndexByAddr(
    IN  P_ADAPTER_T          prAdapter,
    IN  PUINT_8              pucBssid,
    OUT PUINT_8              pucWlanIndex
    );

P_STA_RECORD_T
nicPrivacyGetStaRecordByWlanIndex(
    IN  P_ADAPTER_T         prAdapter,
    IN  UINT_8              ucWlanIndex
    );

BOOLEAN
nicPrivacyCopyFromTemplateEntry(
    IN P_ADAPTER_T           prAdapter
    );

VOID
nicPrivacyInvalidEntryRx(
    IN P_ADAPTER_T           prAdapter,
    IN PUINT_8               pucBssid
    );

VOID
nicPrivacyInvalidTemplateEntry(
    IN P_ADAPTER_T           prAdapter
    );

BOOLEAN
nicPrivacySetWlanTable(
    IN  P_ADAPTER_T          prAdapter,
    IN  UINT_8               ucEntry,
    IN  P_STA_RECORD_T       prSta,
    IN  P_WT_CTRL_STRUC_T    prWTCtrl,
    IN  P_SW_KEY_STRUC_T     prSwKey
    );

BOOLEAN
nicPrivacyClearWlanTable(
    IN  P_ADAPTER_T          prAdapter,
    IN  UINT_8               ucEntry
    );

VOID
nicPrivacyDumpWlanTable(
    IN  P_ADAPTER_T          prAdapter,
    IN  UINT_8               ucEntry
    );

#endif /* _NIC_PRIVACY_H */

