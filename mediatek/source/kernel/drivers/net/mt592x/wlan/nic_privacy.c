






#include "precomp.h"


/* key type */
typedef enum _ENUM_KEY_TYPE_T {
    ENUM_KEY_TYPE_DEFAULT_KEY,
    ENUM_KEY_TYPE_GROUP_KEY    = ENUM_KEY_TYPE_DEFAULT_KEY,
    ENUM_KEY_TYPE_PAIRWISE_KEY,
    ENUM_KEY_TYPE_MAX
} ENUM_KEY_TYPE_T, *P_ENUM_KEY_TYPE_T;







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacyInitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_8      i;
    
    DEBUGFUNC("nicPrivacyInitialize");

    ASSERT(prAdapter);

    /* Clear Driver adapter key and HW wlan table content */
    for (i = 0; i < WLAN_TABLE_SIZE; i++) {
        kalMemZero((PVOID)&prAdapter->arWlanCtrl[i], sizeof(WLAN_ENTRY_CTRL_T));
    }
    halWlanTableInit(prAdapter);

}   /* nicPrivacyInitialize */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacyEnableHwTxPortControl (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32                    u4tmp = 0;

    DEBUGFUNC("nicPrivacyEnableHwTxPortControl");
    
    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_HCR, &u4tmp);
    HAL_MCR_WR(prAdapter, MCR_HCR, HCR_1X_CK_EN | u4tmp);

}   /* nicPrivacyEnableHwTxPortControl */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacySeekForEntry (
    IN  P_ADAPTER_T         prAdapter,
    IN  BOOLEAN             fgTxKey,
    IN  PUINT_8             pucBssid,
    IN  UINT_32             u4KeyId,              
    IN  P_STA_RECORD_T      prSta,
    OUT PUINT_8             pucEntryIndex
    )
{
    UCHAR                   i = 0;
    P_SW_KEY_STRUC_T        prSwKey = NULL;
    P_WT_CTRL_STRUC_T       prCtrl = NULL;
    UCHAR                   aucBcAddr[] = BC_MAC_ADDR;
    UCHAR                   aucNullAddr[] = NULL_MAC_ADDR;

    DEBUGFUNC("nicPrivacySeekForEntry");

    ASSERT(prAdapter);
    ASSERT(pucEntryIndex);

    if (pucBssid == NULL) {
        return FALSE;
    }
    
    *pucEntryIndex = WLAN_TABLE_SET_ALL;

    if (fgTxKey) {
        if (kalMemCmp(pucBssid, aucBcAddr, MAC_ADDR_LEN) == 0 ||
            kalMemCmp(pucBssid, aucNullAddr, MAC_ADDR_LEN) == 0) {
            *pucEntryIndex = WLAN_TABLE_DEFAULT_ENTRY;
        }
        else if (kalMemCmp(prAdapter->rBssInfo.aucBSSID, pucBssid, MAC_ADDR_LEN) == 0){
            *pucEntryIndex = WLAN_TABLE_DEFAULT_ENTRY;
        }
        else { /* DLS Case or IBSS WEP and WPA */
            for (i = WLAN_TABLE_TEMPLATE_ENTRY + 1 ; i < WLAN_TABLE_SIZE; i++) {
                prSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[i].rSWKey;

                if (prAdapter->arWlanCtrl[i].fgUsed == FALSE) {
                    *pucEntryIndex = i;
                    break;
                }
                if (kalMemCmp(prSwKey->aucAddr, pucBssid, MAC_ADDR_LEN) == 0) {
                    if (prSwKey->ucKeyId == (u4KeyId & BITS(0,1))){
                        *pucEntryIndex = i;
                        break;
                    }
                }
            }
        }
    }
    else {
        /* Windows use BSSID to set BC key */
        for (i = WLAN_TABLE_SIZE - 1 ; i > WLAN_TABLE_TEMPLATE_ENTRY; i--) {
            prSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[i].rSWKey;

            if (prAdapter->arWlanCtrl[i].fgUsed == FALSE) {
                *pucEntryIndex = i;
                break;
            }
            if (kalMemCmp(prSwKey->aucAddr, pucBssid, MAC_ADDR_LEN) == 0) {
                if (prSwKey->ucKeyId == (u4KeyId & BITS(0,1))){
                    *pucEntryIndex = i;
                    break;
                }
            }
        }
    }

    if (*pucEntryIndex == WLAN_TABLE_SET_ALL) {
        DBGLOG(NIC, WARN, ("No available wlan table entry !!\n"));
        ASSERT(FALSE);
        return FALSE;
    }
    else {
        /* For Windows, Add legacy key before SSID, will use entry 0 for fgTxKey == TRUE 
           And After Join Success, the initial will clear the setting */
        prSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[*pucEntryIndex].rSWKey;
        prCtrl = (P_WT_CTRL_STRUC_T)&prAdapter->arWlanCtrl[*pucEntryIndex].rCtrl;

        if (prSwKey)
            kalMemCopy(prSwKey->aucAddr, pucBssid, MAC_ADDR_LEN);

        if ((prAdapter->rConnSettings.eAuthMode < AUTH_MODE_WPA) &&
            (*pucEntryIndex == WLAN_TABLE_TEMPLATE_ENTRY)) {
            prCtrl->fgTV = FALSE;
        }
        else {   
            prCtrl->fgTV = fgTxKey;
        }
        prCtrl->fgRV = TRUE;

        prAdapter->arWlanCtrl[*pucEntryIndex].fgUsed = TRUE;

        /* save the sta record pointer */
        prAdapter->arWlanCtrl[*pucEntryIndex].prSta = prSta;
    }

    DBGLOG(NIC, TRACE, ("entry %d alloced\n", *pucEntryIndex));
 
    return TRUE;
} /* nicPrivacySeekForEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacyMatchForEntry (
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucBssid,
    IN  UINT_8              ucKeyId,
    OUT PUINT_8             pucEntryIndex
    )
{
    CHAR                    i = 0;
    P_SW_KEY_STRUC_T        prSwKey = NULL;
    BOOLEAN                 fgMatch = FALSE;
    
    DEBUGFUNC("nicPrivacyMatchForEntry");

    ASSERT(prAdapter);
    ASSERT(pucEntryIndex);
    
    if (pucBssid == NULL) {
        return FALSE;
    }
    
    *pucEntryIndex = WLAN_TABLE_SET_ALL;

    for (i = 0; i < WLAN_TABLE_SIZE; i++) {
        prSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[i].rSWKey;

        if (kalMemCmp(prSwKey->aucAddr, pucBssid, MAC_ADDR_LEN) == 0 &&
            prAdapter->arWlanCtrl[i].fgUsed == TRUE) {
            *pucEntryIndex = i;
            fgMatch = TRUE;
            break;
        }
    }
    
    if (*pucEntryIndex == WLAN_TABLE_SET_ALL) { /* Broadcast */
        if (privacyTransmitKeyExist(prAdapter) && 
            ucKeyId == (prAdapter->rConnSettings.rMib.dot11WEPDefaultKeyID) && 
            prAdapter->arWlanCtrl[WLAN_TABLE_DEFAULT_ENTRY].fgUsed == TRUE)  {
                *pucEntryIndex = WLAN_TABLE_DEFAULT_ENTRY;
                fgMatch = TRUE;
        }
        else {

            for (i = WLAN_TABLE_SIZE - 1; i >= WLAN_TABLE_SIZE - 4 ; i--) {
                prSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[i].rSWKey;
            
                if (prSwKey->ucKeyId == ucKeyId &&
                    prAdapter->arWlanCtrl[i].fgUsed == TRUE) {
                    *pucEntryIndex = i;
                    fgMatch = TRUE;
                    break;
                }
            }
        }
    }

    if (fgMatch == FALSE)
        return FALSE;
    
    DBGLOG(NIC, TRACE, ("entry %d matched\n", *pucEntryIndex));
    
    return TRUE;
} /* nicPrivacyMatchForEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacyInvalidKey (
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid
   )
{
    P_WLAN_ENTRY_CTRL_T     prWlanCtrl;
    UINT_8                  ucEntryIndex;  

    DEBUGFUNC("nicPrivacyInvalidKey");

    ASSERT(prAdapter);
    
    if (nicPrivacyMatchForEntry(prAdapter,
                                pucBssid, 
                                0,
                                &ucEntryIndex) == FALSE){
        return FALSE;
    }

    prWlanCtrl = &prAdapter->arWlanCtrl[ucEntryIndex];

    prWlanCtrl->rCtrl.fgT1X = TRUE;
    prWlanCtrl->rCtrl.fgR1X = TRUE;
    prWlanCtrl->rCtrl.fgTKV = FALSE;
    prWlanCtrl->rSWKey.ucCipher = CIPHER_SUITE_NONE;

    nicPrivacyDumpWlanTable(prAdapter, ucEntryIndex);
    if (halWlanTableAccess(prAdapter,
                          (PVOID)prWlanCtrl,
                           ucEntryIndex, 
                           HWTDR_UPDATE_MODE_0) == FALSE) {
        ASSERT(FALSE);
    }
    return TRUE;
} /* nicPrivacyInvalidKey */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacyPortControl (
    IN P_ADAPTER_T        prAdapter,
    IN PUINT_8            pucBssid,
    IN BOOLEAN            fgTxPort,
    IN BOOLEAN            fgRxPort
   )
{
    P_WT_CTRL_STRUC_T     prCtrl;
    UINT_8                ucEntryIndex;  

    DEBUGFUNC("nicPrivacyPortControl");

    ASSERT(prAdapter);
    
    if (nicPrivacyMatchForEntry(prAdapter,
                                pucBssid, 
                                0,
                               &ucEntryIndex) == FALSE){
        if (kalMemCmp(prAdapter->rBssInfo.aucBSSID, pucBssid, MAC_ADDR_LEN) == 0)
            ucEntryIndex = WLAN_TABLE_DEFAULT_ENTRY;
        else {
            return;
        }
    }
    
    prCtrl = (P_WT_CTRL_STRUC_T)&prAdapter->arWlanCtrl[ucEntryIndex].rCtrl;

    if (prAdapter->rSecInfo.fgBlockOnlyAPTraffic) {
        prAdapter->rSecInfo.fgBlockTxTraffic = fgTxPort;
        prAdapter->rSecInfo.fgBlockRxTraffic = fgRxPort;
    }
    else {
        prCtrl->fgT1X = fgTxPort;
        prCtrl->fgR1X = fgRxPort;
    }

    return;
} /* nicPrivacyPortControl */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacyUpdateBySta (
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
    )
{
    UINT_8                   ucEntryIndex;
    P_SW_KEY_STRUC_T         prSwKey = NULL, prDefSwKey = NULL;
    P_WT_CTRL_STRUC_T        prCtrl = NULL, prDefCtrl = NULL;

    DEBUGFUNC("nicPrivacyUpdateBySta");

    ASSERT(prAdapter);
    
    if (nicPrivacySeekForEntry(prAdapter,
                               TRUE,
                               (PUINT_8)pucBSSID, 
                               0,
                               prSta,
                               &ucEntryIndex) == FALSE){
        return FALSE;
    }

    prCtrl = (P_WT_CTRL_STRUC_T)&prAdapter->arWlanCtrl[ucEntryIndex].rCtrl;
    prSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[ucEntryIndex].rSWKey;

    prDefCtrl = (P_WT_CTRL_STRUC_T)&prAdapter->arWlanCtrl[WLAN_TABLE_DEFAULT_ENTRY].rCtrl;
    prDefSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[WLAN_TABLE_DEFAULT_ENTRY].rSWKey;


    if (pfgQoS) {                                
        prCtrl->fgQoS = *pfgQoS;
    }  

    if (pfgAntenna) {                                
        prCtrl->fgAntenna = *pfgAntenna;
    }

    if (pucMuar) {                                
        prCtrl->ucMuar = *pucMuar;
    }
    
    if (pucRate1) {                                
        prCtrl->ucRate1 = *pucRate1;
    }
    if (pucRate2) {                                
        prCtrl->ucRate2 = *pucRate2;
    }    
    if (pucRate3) {                                
        prCtrl->ucRate3 = *pucRate3;
    }    
    if (pucRate4) {                                
        prCtrl->ucRate4 = *pucRate4;
    }        

    /* The IBSS WEP & WPA-NONE, the key should copy to the entry */
    if (prAdapter->rConnSettings.eOPMode == NET_TYPE_IBSS &&
        prAdapter->rConnSettings.eEncStatus != ENUM_WEP_DISABLED) {
        if (privacyTransmitKeyExist(prAdapter) && ucEntryIndex !=0 ) {
            prCtrl->fgRKV = 
                prAdapter->arWlanCtrl[WLAN_TABLE_DEFAULT_ENTRY].rCtrl.fgRKV;
            if ((prAdapter->rConnSettings.eAuthMode < AUTH_MODE_WPA) &&
                (ucEntryIndex == WLAN_TABLE_TEMPLATE_ENTRY)){
                prCtrl->fgTKV = FALSE;
            }
            else {     
                prCtrl->fgTKV = prDefCtrl->fgTKV;
            }
            prSwKey->ucRCA1 = prDefSwKey->ucRCA1;
            prSwKey->fgRCA2 = prDefSwKey->fgRCA2;
            prSwKey->fgRCID = prDefSwKey->fgRCID;
            prSwKey->ucKeyId = prDefSwKey->ucKeyId;
            prSwKey->ucCipher= prDefSwKey->ucCipher;
            
            kalMemCopy(&prSwKey->aucKeyMaterial[0],
                prDefSwKey->aucKeyMaterial,
                TKIP_KEY_LEN - ( 2 * MIC_KEY_LEN));
            kalMemCopy(&prSwKey->aucRxMicMaterial[0],
                &prDefSwKey->aucRxMicMaterial[0],
                MIC_KEY_LEN);
            kalMemCopy(&prSwKey->aucTxMicMaterial[0],
                &prDefSwKey->aucTxMicMaterial[0],
                MIC_KEY_LEN);
            
        }
    }

    nicPrivacyDumpWlanTable(prAdapter, ucEntryIndex);

    return halWlanTableAccess(prAdapter, 
                             (PVOID)&prAdapter->arWlanCtrl[ucEntryIndex],
                              ucEntryIndex,
                              HWTDR_UPDATE_MODE_3);     /* Set key into H/W key table */

} /* nicPrivacyUpdateBySta */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacySetKeyEntry (
    IN P_ADAPTER_T          prAdapter,
    IN BOOLEAN              fgTxKey,
    IN PUINT_8              pucBssid,
    IN UINT_8               ucKeyId,
    IN PUINT_8              pucKeyMaterial,
    IN UINT_8               ucKeyLen,
    IN UINT_8               ucCipherMode,
    IN UINT_8               ucTxMicOffset,
    IN UINT_8               ucRxMicOffset
    )
{
    P_WLAN_ENTRY_CTRL_T     prWlanCtrl;
    P_SW_KEY_STRUC_T        prSwKey = NULL;
    P_WT_CTRL_STRUC_T       prCtrl = NULL;
    UINT_8                  ucEntryIndex;
    P_STA_RECORD_T          prSta;
    UINT_8                  aucBCAddr[] = BC_MAC_ADDR;
    
    DEBUGFUNC("nicPrivacySetKeyEntry");

    ASSERT(prAdapter); 
    ASSERT(pucBssid);
    ASSERT(pucKeyMaterial); 

    prSta = staRecGetStaRecordByAddr(prAdapter, pucBssid);
    
    nicPrivacySeekForEntry(prAdapter, 
                           fgTxKey,
                           pucBssid, 
                           ucKeyId,
                           prSta,
                           &ucEntryIndex);

    prWlanCtrl = &prAdapter->arWlanCtrl[ucEntryIndex];
    prSwKey = &prWlanCtrl->rSWKey;
    prCtrl = &prWlanCtrl->rCtrl;

    /* !!! Do This will clear the QoS,... setting */
    //kalMemZero(prCfgKey, sizeof(WLAN_ENTRY_CTRL_T));

    /* Pairwise key always uses 0 as key ID */
    switch (ucCipherMode) {
        case CIPHER_SUITE_NONE: /* FALL THROUGH */
        case CIPHER_SUITE_WEP40: /* FALL THROUGH */
        case CIPHER_SUITE_TKIP: /* FALL THROUGH */
        case CIPHER_SUITE_TKIP_WO_MIC: /* FALL THROUGH */
        case CIPHER_SUITE_CCMP: /* FALL THROUGH */
        case CIPHER_SUITE_WEP104: /* FALL THROUGH */
        case CIPHER_SUITE_WEP128: /* FALL THROUGH */
            prSwKey->ucCipher = ucCipherMode;
            break;
        default:
            DBGLOG(RSN, WARN, ("Unknown Cipher mode %d\n", ucCipherMode));
            prSwKey->ucCipher = CIPHER_SUITE_NONE;
            break;
    }

    prSwKey->ucKeyId = ucKeyId;

    kalMemCopy(prSwKey->aucAddr, pucBssid, MAC_ADDR_LEN);
    kalMemCopy(prSwKey->aucKeyMaterial, pucKeyMaterial, TKIP_KEY_LEN - (MIC_KEY_LEN * 2));

    kalMemCopy(prSwKey->aucRxMicMaterial, pucKeyMaterial + ucRxMicOffset, MIC_KEY_LEN);
    kalMemCopy(prSwKey->aucTxMicMaterial, pucKeyMaterial + ucTxMicOffset, MIC_KEY_LEN);

    if (fgTxKey) {
        if (kalMemCmp(pucBssid, aucBCAddr, MAC_ADDR_LEN) == 0) {
            prSwKey->ucRCA1 = HWTDR_RCA1_MUAR_BC;
        }
        else {            
            prSwKey->ucRCA1 = HWTDR_RCA1_MUAR_ONLY;
        }
        prCtrl->fgTV = TRUE;
        prCtrl->fgTKV = TRUE;
    }
    else {
        prSwKey->ucRCA1 = HWTDR_RCA1_MUAR_BC;
        prSwKey->fgRCA2 = FALSE;
    }

    prSwKey->fgRCID = TRUE;
    prCtrl->fgRV = TRUE;
    prCtrl->fgRKV = TRUE;

    nicPrivacyDumpWlanTable(prAdapter, ucEntryIndex);

    return halWlanTableAccess(prAdapter, 
                             (PVOID)prWlanCtrl,
                              ucEntryIndex,
                              HWTDR_UPDATE_MODE_3);     /* Set key into H/W key table */
  
} /* nicPrivacySetIpcKeyEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacySetKeyToTemplateEntry (
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid,
    IN PUINT_8              pucKeyMaterial,
    IN UINT_8               ucKeyLen,
    IN UINT_8               ucCipherMode,
    IN UINT_8               ucTxMicOffset,
    IN UINT_8               ucRxMicOffset
    )
{
    P_WLAN_ENTRY_CTRL_T     prWlanCtrl;
    P_SW_KEY_STRUC_T        prSwKey = NULL;
    P_WT_CTRL_STRUC_T       prCtrl = NULL;
    P_WT_CTRL_STRUC_T       prOrgCtrl = NULL;
    UINT_8                  ucEntryIndex;

    DEBUGFUNC("nicPrivacySetKeyToTemplateEntry");

    ucEntryIndex = WLAN_TABLE_TEMPLATE_ENTRY;

    ASSERT(prAdapter);
    ASSERT(pucBssid);
    ASSERT(pucKeyMaterial);

    if (!nicPrivacyGetWlanIndexByAddr(prAdapter, pucBssid, &ucEntryIndex)) {
        ASSERT(FALSE);
    }

    prWlanCtrl = &prAdapter->arWlanCtrl[WLAN_TABLE_TEMPLATE_ENTRY];
    prSwKey = &prWlanCtrl->rSWKey;
    prCtrl = &prWlanCtrl->rCtrl;
    prOrgCtrl = &prAdapter->arWlanCtrl[ucEntryIndex].rCtrl;

    kalMemZero(prWlanCtrl, sizeof(WLAN_ENTRY_CTRL_T));

    switch (ucCipherMode) {
        case CIPHER_SUITE_NONE: /* FALL THROUGH */
        case CIPHER_SUITE_WEP40: /* FALL THROUGH */
        case CIPHER_SUITE_TKIP: /* FALL THROUGH */
        case CIPHER_SUITE_TKIP_WO_MIC: /* FALL THROUGH */
        case CIPHER_SUITE_CCMP: /* FALL THROUGH */
        case CIPHER_SUITE_WEP104: /* FALL THROUGH */
        case CIPHER_SUITE_WEP128: /* FALL THROUGH */
            prSwKey->ucCipher = ucCipherMode;
            break;
        default:
            DBGLOG(RSN, WARN, ("Unknown Cipher mode %d\n", ucCipherMode));
            prSwKey->ucCipher = CIPHER_SUITE_NONE;
            break;
    }

    prSwKey->ucKeyId = 0;

    kalMemCopy(prSwKey->aucAddr, pucBssid, MAC_ADDR_LEN);
    kalMemCopy(prSwKey->aucKeyMaterial, pucKeyMaterial, TKIP_KEY_LEN - (MIC_KEY_LEN * 2));

    kalMemCopy(prSwKey->aucRxMicMaterial, pucKeyMaterial + ucRxMicOffset, MIC_KEY_LEN);
    kalMemCopy(prSwKey->aucTxMicMaterial, pucKeyMaterial + ucTxMicOffset, MIC_KEY_LEN);

    prCtrl->fgQoS = prOrgCtrl->fgQoS;
    prCtrl->fgAntenna = prOrgCtrl->fgAntenna;
    prCtrl->ucMuar = prOrgCtrl->ucMuar;
    prCtrl->ucRate1 = prOrgCtrl->ucRate1;
    prCtrl->ucRate2 = prOrgCtrl->ucRate2;
    prCtrl->ucRate3 = prOrgCtrl->ucRate3;
    prCtrl->ucRate4 = prOrgCtrl->ucRate4;

    prSwKey->ucRCA1 = HWTDR_RCA1_MUAR_ONLY;
    prSwKey->fgRCA2 = TRUE;
    prSwKey->fgRCID = TRUE;
    prCtrl->fgTV = FALSE;
    prCtrl->fgRKV = TRUE;
    prCtrl->fgRV = TRUE;

    if (prAdapter->rSecInfo.fgBlockOnlyAPTraffic) {
        //check the bssid is ap?
        prAdapter->rSecInfo.fgBlockTxTraffic = TRUE;
        prAdapter->rSecInfo.fgBlockRxTraffic = FALSE;
    }
    else {
        prCtrl->fgT1X = TRUE;
        prCtrl->fgR1X = FALSE;
    }

    nicPrivacyDumpWlanTable(prAdapter, WLAN_TABLE_TEMPLATE_ENTRY);

    /* Set key into H/W key table */
    if (!halWlanTableAccess(prAdapter, 
                             (PVOID)prWlanCtrl,
                              WLAN_TABLE_TEMPLATE_ENTRY,
                              HWTDR_UPDATE_MODE_3)) {
                              
    }
  
} /* nicPrivacySetKeyToTemplateEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_STA_RECORD_T
nicPrivacyGetWlanIndexByAddr (
    IN  P_ADAPTER_T          prAdapter,
    IN  PUINT_8              pucBssid,
    OUT PUINT_8              pucWlanIndex
    )
{
    UINT_8                   i;
    PUINT_8                  pucAddr;

    ASSERT(prAdapter);
    ASSERT(pucBssid);
    ASSERT(pucWlanIndex);
    
    for (i = 0; i < WLAN_TABLE_SIZE; i++) {
        pucAddr = &prAdapter->arWlanCtrl[i].rSWKey.aucAddr[0];

        if (prAdapter->arWlanCtrl[i].fgUsed &&
            kalMemCmp(pucAddr, pucBssid, MAC_ADDR_LEN) == 0) {
            *pucWlanIndex = i;
            break;
        }
    }

    if (i >= WLAN_TABLE_SIZE) {
        return NULL;
    }

    return prAdapter->arWlanCtrl[i].prSta;
} /* nicPrivacyGetWlanIndexByAddr */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_STA_RECORD_T
nicPrivacyGetStaRecordByWlanIndex (
    IN  P_ADAPTER_T         prAdapter,
    IN  UINT_8              ucWlanIndex
    )
{
    P_WLAN_ENTRY_CTRL_T prWlanCtrl;

    ASSERT(prAdapter);

    if (ucWlanIndex < WLAN_TABLE_SIZE) {
        prWlanCtrl = &prAdapter->arWlanCtrl[ucWlanIndex];

        return prWlanCtrl->prSta;
    }
#if DBG
    else {
        ASSERT(FALSE);
    }
#endif /* DBG */

    return (P_STA_RECORD_T)NULL;

} /* nicPrivacyGetStaRecordByWlanIndex */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacyCopyFromTemplateEntry (
    IN P_ADAPTER_T          prAdapter
    )
{
    PUINT_8                 pucBssid;
    UINT_8                  ucEntry;
    P_WLAN_ENTRY_CTRL_T     prWlanCtrl, prOrgCfgEntry;

    DEBUGFUNC("nicPrivacyCopyFromTemplateEntry");

    ASSERT(prAdapter);
    
    pucBssid = &prAdapter->arWlanCtrl[WLAN_TABLE_TEMPLATE_ENTRY].rSWKey.aucAddr[0];
    
    if (nicPrivacyMatchForEntry(prAdapter, 
                               pucBssid, 
                               0,
                               &ucEntry) == FALSE){
        return FALSE;
    }

    /* Open the port for orginal entry */
    nicPrivacyPortControl(prAdapter, pucBssid, FALSE, FALSE);
    
    prOrgCfgEntry = &prAdapter->arWlanCtrl[WLAN_TABLE_TEMPLATE_ENTRY];
    prWlanCtrl = &prAdapter->arWlanCtrl[ucEntry];

    kalMemCopy((PVOID)prWlanCtrl, prOrgCfgEntry, sizeof(WLAN_ENTRY_CTRL_T));
    prWlanCtrl->rCtrl.fgTV = TRUE;
    prWlanCtrl->rCtrl.fgTKV = TRUE;

    prAdapter->arWlanCtrl[ucEntry].fgUsed = TRUE;

    nicPrivacyDumpWlanTable(prAdapter, ucEntry);

    return halWlanTableAccess(prAdapter, 
                             (PVOID)prWlanCtrl, 
                              ucEntry,
                              HWTDR_UPDATE_MODE_3);
    
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacyInvalidEntryRx (
    IN P_ADAPTER_T          prAdapter,
    IN PUINT_8              pucBssid
    )
{
    UINT_8                  ucEntry;
    
    DEBUGFUNC("nicPrivacyInvalidEntryRx");

    ASSERT(prAdapter);

    if (nicPrivacyMatchForEntry(prAdapter, 
                           pucBssid, 
                           0,
                           &ucEntry) == FALSE){
        return;                   
    }

    /* Invalid entry 0 for AP, x for DLS rv=0 */
    prAdapter->arWlanCtrl[ucEntry].rCtrl.fgRV = FALSE;

    nicPrivacyDumpWlanTable(prAdapter, ucEntry);

    if (!halWlanTableAccess(prAdapter, 
        (PVOID)&prAdapter->arWlanCtrl[ucEntry], 
        ucEntry, 
        HWTDR_UPDATE_MODE_0)) {
        DBGLOG(NIC, TRACE, ("Access Wlan table failed\n"));
    }

    return;
} /* nicPrivacyValidEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacyInvalidTemplateEntry (
    IN P_ADAPTER_T          prAdapter
    )
{
    DEBUGFUNC("nicPrivacyInvalidTemplateEntry");

    ASSERT(prAdapter);
    
    prAdapter->arWlanCtrl[WLAN_TABLE_TEMPLATE_ENTRY].rCtrl.fgTV = FALSE;
    prAdapter->arWlanCtrl[WLAN_TABLE_TEMPLATE_ENTRY].rCtrl.fgRV = FALSE;

    nicPrivacyDumpWlanTable(prAdapter, WLAN_TABLE_TEMPLATE_ENTRY);

    if (!halWlanTableAccess(prAdapter, 
        (PVOID)&prAdapter->arWlanCtrl[WLAN_TABLE_TEMPLATE_ENTRY], 
        WLAN_TABLE_TEMPLATE_ENTRY, 
        HWTDR_UPDATE_MODE_0)){
    }

    return;
} /* nicPrivacyInvalidTemplateEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacySetWlanTable (
    IN  P_ADAPTER_T          prAdapter,
    IN  UINT_8               ucEntry,
    IN  P_STA_RECORD_T       prSta,
    IN  P_WT_CTRL_STRUC_T    prWTCtrl,
    IN  P_SW_KEY_STRUC_T     prSwKey
    )
{
    P_WLAN_ENTRY_CTRL_T      prWlanEntry;

    ASSERT(prAdapter);
    ASSERT(prWTCtrl);
    ASSERT(prSwKey);

    prWlanEntry = &prAdapter->arWlanCtrl[ucEntry];
        
    kalMemZero((PUINT_8)prWlanEntry, sizeof(WLAN_ENTRY_CTRL_T));

    /* Assign the variable here */
    prWlanEntry->fgUsed = TRUE;
    prWlanEntry->prSta = prSta;

    kalMemCopy((PUINT_8)&prWlanEntry->rCtrl, prWTCtrl, sizeof(WT_CTRL_STRUC_T));
    kalMemCopy((PUINT_8)&prWlanEntry->rSWKey, prSwKey, sizeof(SW_KEY_STRUC_T));
    
    return halWlanTableAccess(prAdapter, 
                              (PVOID)prWlanEntry,
                              ucEntry,
                              HWTDR_UPDATE_MODE_3);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicPrivacyClearWlanTable (
    IN  P_ADAPTER_T          prAdapter,
    IN  UINT_8               ucEntry
    )
{
    P_WLAN_ENTRY_CTRL_T      prWlanEntry;

    ASSERT(prAdapter);
    
    prWlanEntry = &prAdapter->arWlanCtrl[ucEntry];
        
    kalMemZero((PUINT_8)prWlanEntry, sizeof(WLAN_ENTRY_CTRL_T));

    /* Assign the variable here */
    prWlanEntry->fgUsed = FALSE;
    prWlanEntry->prSta = NULL;
    
    return halWlanTableAccess(prAdapter, 
                              (PVOID)prWlanEntry,
                              ucEntry,
                              HWTDR_UPDATE_MODE_3);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPrivacyDumpWlanTable (
    IN  P_ADAPTER_T          prAdapter,
    IN  UINT_8               ucEntry
    )
{
#if DBG
    UINT_8                   i;
    UINT_8                   ucMinIndex, ucMaxIndex;
    P_WT_CTRL_STRUC_T        prCtrl;
    P_SW_KEY_STRUC_T         prKey;

    DEBUGFUNC("nicPrivacyDumpWlanTable");

    ASSERT(prAdapter);
    
    if (ucEntry >= WLAN_TABLE_SIZE && ucEntry != WLAN_TABLE_SET_ALL)
        return;
    
    if (ucEntry == WLAN_TABLE_SET_ALL) {
        ucMinIndex = WLAN_TABLE_DEFAULT_ENTRY;
        ucMaxIndex = WLAN_TABLE_SIZE - 1;
    }
    else  {
        ucMinIndex = ucEntry;
        ucMaxIndex = ucEntry;
    }
    
    for (i=ucMinIndex; i<=ucMaxIndex; i++) {
        prCtrl = &prAdapter->arWlanCtrl[i].rCtrl;
        prKey = &prAdapter->arWlanCtrl[i].rSWKey;
        
        DBGLOG(HAL, TRACE, ("=============================================\n"));
        DBGLOG(HAL, TRACE, ("Index %d\n", i));
        DBGLOG(HAL, TRACE, ("TV %d TKV %d T1x %d\n", 
                            prCtrl->fgTV, prCtrl->fgTKV, prCtrl->fgT1X));
        DBGLOG(HAL, TRACE, ("RV %d RKV %d R1x %d\n", 
                            prCtrl->fgRV, prCtrl->fgRKV, prCtrl->fgR1X));
        DBGLOG(HAL, TRACE, ("RCA1 %d RCA2 %d RCID %d\n", 
                            prKey->ucRCA1, prKey->fgRCA2, prKey->fgRCID));
        DBGLOG(HAL, TRACE, ("Cipher %d KeyId %d \n", 
                            prKey->ucCipher, prKey->ucKeyId));
        DBGLOG(HAL, TRACE, ("Q %d A %d IKV %d\n", 
                            prCtrl->fgQoS, prCtrl->fgAntenna, prKey->fgIKV));
        DBGLOG(HAL, TRACE, ("Rate1 %02x Rate2 %02x Rate3 %02x Rate4 %02x \n", 
                            prCtrl->ucRate1, prCtrl->ucRate2, prCtrl->ucRate3, prCtrl->ucRate4));
        DBGLOG(HAL, TRACE, ("DA " MACSTR" muar %d\n", 
                            MAC2STR(prKey->aucAddr), prCtrl->ucMuar));
        DBGLOG_MEM8(HAL, TRACE, prKey->aucKeyMaterial, TKIP_KEY_LEN - (MIC_KEY_LEN * 2));
        DBGLOG_MEM8(HAL, TRACE, prKey->aucRxMicMaterial, MIC_KEY_LEN);
        DBGLOG_MEM8(HAL, TRACE, prKey->aucTxMicMaterial, MIC_KEY_LEN);
        
    }
#endif
}
