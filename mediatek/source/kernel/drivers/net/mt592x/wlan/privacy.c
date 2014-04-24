






#include "precomp.h"









/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
privacyInitialize (
    IN P_ADAPTER_T prAdapter
    )
{
    DEBUGFUNC("privacyInitialize");

    ASSERT(prAdapter);
    
#if CFG_TX_FRAGMENT
    txFragInfoUpdateForPrivacy (prAdapter, FALSE, FALSE);
#endif
    
    prAdapter->rConnSettings.rMib.dot11TranmitKeyAvailable = FALSE;

    prAdapter->rBssInfo.u4RsnSelectedGroupCipher = 0;
    prAdapter->rBssInfo.u4RsnSelectedPairwiseCipher = 0;
    prAdapter->rBssInfo.u4RsnSelectedAKMSuite = 0;

#if SUPPORT_WAPI
    /* Todo:: Clear the wapimode if the oid set wapi is supported */
    /* Flag to info this assoc info is set */
    if (prAdapter->fgUseWapi) {
        prAdapter->prGlueInfo->u2WapiAssocInfoIESz = 0;
        prAdapter->rSecInfo.fgBlockTxTraffic = FALSE;
        prAdapter->rSecInfo.fgBlockRxTraffic = FALSE;
    }
#endif

    nicPrivacyInitialize(prAdapter);
}   /* privacyInitialize */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
privacyClearPmkid (
    IN P_ADAPTER_T prAdapter
    )
{
    P_SEC_INFO_T prSecInfo;

    DEBUGFUNC("privacyClearPmkid");

    ASSERT(prAdapter);
    
    prSecInfo = &prAdapter->rSecInfo;
    ASSERT(prSecInfo);

    prSecInfo->u4PmkidCandicateCount = 0;
    prSecInfo->u4PmkidCacheCount = 0;
    kalMemZero((PVOID)prSecInfo->arPmkidCandicate, sizeof(PMKID_CANDICATE_T) * CFG_MAX_PMKID_CACHE);
    kalMemZero((PVOID)prSecInfo->arPmkidCache, sizeof(PMKID_ENTRY_T) * CFG_MAX_PMKID_CACHE);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
privacyRsnKeyHandshakeEnabled (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    
    ASSERT(prAdapter);

    prConnSettings = &prAdapter->rConnSettings;

    ASSERT(prConnSettings);
    
    ASSERT(prConnSettings->eEncStatus < ENUM_ENCRYPTION3_KEY_ABSENT);

    if (prConnSettings->eEncStatus == ENUM_ENCRYPTION_DISABLED) {
        return FALSE;
    }

    ASSERT(prConnSettings->eAuthMode < AUTH_MODE_NUM);
    if ((prConnSettings->eAuthMode >= AUTH_MODE_WPA) &&
        (prConnSettings->eAuthMode != AUTH_MODE_WPA_NONE)) {
        return TRUE;
    }

    return FALSE;
} /* privacyRsnKeyHandshakeEnabled */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
privacyTransmitKeyExist (
    IN P_ADAPTER_T prAdapter
    )
{

    ASSERT(prAdapter);

    if (prAdapter->rConnSettings.rMib.dot11TranmitKeyAvailable){
        return TRUE;
    }
    else { 
        return FALSE;
    }
} /* privacyTransmitKeyExist */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
privacyEnabled (
    IN P_ADAPTER_T prAdapter
    )
{
    DEBUGFUNC("privacyEnabled");

    ASSERT(prAdapter);
    ASSERT(prAdapter->rConnSettings.eEncStatus < ENUM_ENCRYPTION3_KEY_ABSENT);

    switch (prAdapter->rConnSettings.eEncStatus) {
        case ENUM_ENCRYPTION_DISABLED:
            return FALSE;
        case ENUM_ENCRYPTION1_ENABLED:
        case ENUM_ENCRYPTION2_ENABLED:
        case ENUM_ENCRYPTION3_ENABLED:
            return TRUE;
        default:
            DBGLOG(RSN, ERROR, ("Unknown encryption setting %d\n",
                prAdapter->rConnSettings.eEncStatus));
            break;
    }
    return FALSE;
} /* privacyEnabled */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
privacySetCipherSuite (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4CipherSuitesFlags
    )
{
    UINT_32 i;
    P_DOT11_RSNA_CONFIG_PAIRWISE_CIPHERS_ENTRY prEntry;
    P_IEEE_802_11_MIB_T prMib;

    ASSERT(prAdapter);

    prMib = &prAdapter->rConnSettings.rMib;

    ASSERT(prMib);
    
    if (u4CipherSuitesFlags == CIPHER_FLAG_NONE) {
        /* Disable all the pairwise cipher suites. */
        for (i = 0; i < MAX_NUM_SUPPORTED_CIPHER_SUITES; i++) {
            prMib->dot11RSNAConfigPairwiseCiphersTable[i].dot11RSNAConfigPairwiseCipherEnabled =
                FALSE;
        }

        /* Update the group cipher suite. */
        prMib->dot11RSNAConfigGroupCipher = WPA_CIPHER_SUITE_NONE;

        return;
    }

    for (i = 0; i < MAX_NUM_SUPPORTED_CIPHER_SUITES; i++) {
        prEntry = &prMib->dot11RSNAConfigPairwiseCiphersTable[i];

        switch (prEntry->dot11RSNAConfigPairwiseCipher) {
            case WPA_CIPHER_SUITE_WEP40:
            case RSN_CIPHER_SUITE_WEP40:
                 if (u4CipherSuitesFlags & CIPHER_FLAG_WEP40) {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = TRUE;
                 }
                 else {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = FALSE;
                 }
                 break;

            case WPA_CIPHER_SUITE_TKIP:
            case RSN_CIPHER_SUITE_TKIP:
                 if (u4CipherSuitesFlags & CIPHER_FLAG_TKIP) {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = TRUE;
                 }
                 else {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = FALSE;
                 }
                 break;

            case WPA_CIPHER_SUITE_CCMP:
            case RSN_CIPHER_SUITE_CCMP:
                 if (u4CipherSuitesFlags & CIPHER_FLAG_CCMP) {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = TRUE;
                 }
                 else {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = FALSE;
                 }
                 break;

            case WPA_CIPHER_SUITE_WEP104:
            case RSN_CIPHER_SUITE_WEP104:
                 if (u4CipherSuitesFlags & CIPHER_FLAG_WEP104) {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = TRUE;
                 }
                 else {
                     prEntry->dot11RSNAConfigPairwiseCipherEnabled = FALSE;
                 }
                 break;
            default:
                 break;
        }
    }

    /* Update the group cipher suite. */
    if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_CCMP, &i)) {
        prMib->dot11RSNAConfigGroupCipher = WPA_CIPHER_SUITE_CCMP;
    }
    else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_TKIP, &i)) {
        prMib->dot11RSNAConfigGroupCipher = WPA_CIPHER_SUITE_TKIP;
    }
    else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_WEP104, &i)) {
        prMib->dot11RSNAConfigGroupCipher = WPA_CIPHER_SUITE_WEP104;
    }
    else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_WEP40, &i)) {
        prMib->dot11RSNAConfigGroupCipher = WPA_CIPHER_SUITE_WEP40;
    }
    else {
        prMib->dot11RSNAConfigGroupCipher = WPA_CIPHER_SUITE_NONE;
    }

}   /* privacySetCipherSuite */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
privacyReloadDefaultWepKeys (
    IN P_ADAPTER_T prAdapter
    )
{
    DEBUGFUNC("privacyReloadDefaultWepKeys");

    DBGLOG(MGT, INFO, ("\n"));

    ASSERT(prAdapter);

    privacySetCipherSuite(prAdapter, CIPHER_FLAG_NONE);

    prAdapter->rConnSettings.eAuthMode = AUTH_MODE_OPEN;
    prAdapter->rConnSettings.eEncStatus = ENUM_ENCRYPTION_DISABLED;

    /* Clear Driver key and HW wlan table content */
    privacyInitialize(prAdapter);
    
}   /* privacyReloadDefaultWepKeys */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
privacySetKeyEntry (
    IN P_ADAPTER_T          prAdapter,
    IN P_PARAM_KEY_T        prNewKey
    )
{
    UINT_8                  ucKeyId;
    PUINT_8                 pucBssid;
    UINT_8                  ucCipher = 0;
    WLAN_STATUS             status = WLAN_STATUS_SUCCESS;
    UINT_8                  ucTxMicOffset;
    UINT_8                  ucRxMicOffset;
    BOOLEAN                 fgResult;
    P_CONNECTION_SETTINGS_T prConnSettings;     
    P_SEC_INFO_T            prSecInfo;

    DEBUGFUNC("privacySetKeyEntry");

    ASSERT(prAdapter);
    ASSERT(prNewKey);

    prConnSettings = &prAdapter->rConnSettings;

    prSecInfo = &prAdapter->rSecInfo;

    ucKeyId = (UCHAR)(prNewKey->u4KeyIndex & 0x000000ff);

    ASSERT(ucKeyId < MAX_KEY_NUM);

    if (prConnSettings->eAuthMode == AUTH_MODE_WPA_NONE) {
        ucTxMicOffset = MIC_RX_KEY_OFFSET;
        ucRxMicOffset = MIC_RX_KEY_OFFSET;
    }
    else {
        if (prNewKey->u4KeyIndex & IS_AUTHENTICATOR) {
            /* authenticator */
            ucTxMicOffset = MIC_RX_KEY_OFFSET;
            ucRxMicOffset = MIC_TX_KEY_OFFSET;
        }
        else {
            /* supplicant */
            ucTxMicOffset = MIC_TX_KEY_OFFSET;
            ucRxMicOffset = MIC_RX_KEY_OFFSET;
        }
    }

    /* Bit 31: 1 = Transmit key, 0 = non-transmit key
       Bit 30: 1 = Pairwise key, 0 = Group key */
    pucBssid = (PUINT_8)prNewKey->arBSSID;
    switch (prNewKey->u4KeyIndex & BITS(30,31)) {
    case 0x00000000:   /* 00, Group key that is receive only */
    case IS_TRANSMIT_KEY :      /* 10, Group key that can be used to transmit */
         if (((pucBssid[0] == 0xff) && (pucBssid[1] == 0xff) && 
              (pucBssid[2] == 0xff) && (pucBssid[3] == 0xff) && 
              (pucBssid[4] == 0xff) && (pucBssid[5] == 0xff)) ||
             (EQUAL_MAC_ADDR(pucBssid, prAdapter->rBssInfo.aucBSSID))) {
            switch (prConnSettings->eAuthMode) {
                /* Legacy only supports WEP */
            case AUTH_MODE_OPEN:
            case AUTH_MODE_SHARED:
            case AUTH_MODE_AUTO_SWITCH:
                if (prNewKey->u4KeyLength == WEP_40_LEN) {
                    ucCipher = CIPHER_SUITE_WEP40;
                }
                else if (prNewKey->u4KeyLength == WEP_104_LEN) {
                    ucCipher = CIPHER_SUITE_WEP104;
                }
                else if (prNewKey->u4KeyLength == LEGACY_KEY_MAX_LEN) {
                    ucCipher = CIPHER_SUITE_WEP128;
                }
                break;

            case AUTH_MODE_WPA:
            case AUTH_MODE_WPA_PSK:
            case AUTH_MODE_WPA2:
            case AUTH_MODE_WPA2_PSK:
                ucCipher =
                    GET_SELECTOR_TYPE(prAdapter->rBssInfo.u4RsnSelectedGroupCipher);
                break;

            /* WPA None Set Keys without handshaking */
            case AUTH_MODE_WPA_NONE:
                if (prAdapter->rConnSettings.eEncStatus == ENUM_ENCRYPTION2_ENABLED) {
                    ucCipher = CIPHER_SUITE_TKIP;
                }
                else if (prAdapter->rConnSettings.eEncStatus == ENUM_ENCRYPTION3_ENABLED) {
                    ucCipher = CIPHER_SUITE_CCMP;
                }
                else {
                    if (prNewKey->u4KeyLength == CCMP_KEY_LEN) {
                        ucCipher = CIPHER_SUITE_CCMP;
                    }
                    else if (prNewKey->u4KeyLength == TKIP_KEY_LEN) {
                        ucCipher = CIPHER_SUITE_TKIP;
                    }
                }
                break;

            default:
                DBGLOG(RSN, ERROR, ("Unknown authentication mode %d\n", 
                    prConnSettings->eAuthMode));
                ucCipher = CIPHER_SUITE_NONE;
                break;
            }
            
            fgResult = nicPrivacySetKeyEntry(prAdapter,
                                 (prNewKey->u4KeyIndex & IS_TRANSMIT_KEY) ? TRUE : FALSE,
                                  pucBssid,
                                 (UINT_8)ucKeyId,
                                  prNewKey->aucKeyMaterial,
                                 (UINT_8)prNewKey->u4KeyLength,
                                  ucCipher,
                                  ucTxMicOffset,
                                  ucRxMicOffset);
            if (fgResult == FALSE) {
                DBGLOG(RSN, WARN, ("Add key return resource issue\n"));
                return WLAN_STATUS_RESOURCES;
            }

            /* Check bit 31 if default group key used to transmit */
            if (prNewKey->u4KeyIndex & IS_TRANSMIT_KEY) {
                /* Set default Tx key index */
                prConnSettings->rMib.dot11WEPDefaultKeyID = (UINT_8)ucKeyId;
                prConnSettings->rMib.dot11TranmitKeyAvailable = TRUE;
#if CFG_TX_FRAGMENT
                {
                BOOLEAN fgIsTkipCipher = FALSE;
                if (ucCipher == CIPHER_SUITE_TKIP){
                    fgIsTkipCipher = TRUE;
                }
                txFragInfoUpdateForPrivacy(prAdapter, TRUE, fgIsTkipCipher);
                }
#endif
            }

            /* If the authentication mode is WPA2 or WPA2PSK, and after
               the group key is plumbed to driver, then we set the
               indication PMKID flag */
            if (prConnSettings->eAuthMode == AUTH_MODE_WPA2) {
                  ARB_CANCEL_TIMER(prAdapter, prSecInfo->rPreauthenticationTimer);
                  ARB_SET_TIMER(prAdapter,
                      prSecInfo->rPreauthenticationTimer,
                  SEC_TO_MSEC(WAIT_TIME_IND_PMKID_CANDICATE_SEC));
             }
         }
         break;

    case BIT(30): /* 01, Illegal combination */
         status = WLAN_STATUS_INVALID_DATA;
         break;

    case BITS(30,31): /* 11, Pairwise key that can be used to transmit */
        if ((ucKeyId != 0) ||
            ((pucBssid[0] == 0xff) && (pucBssid[1] == 0xff) && (pucBssid[2] == 0xff) &&
             (pucBssid[3] == 0xff) && (pucBssid[4] == 0xff) && (pucBssid[5] == 0xff))) {
            status = WLAN_STATUS_INVALID_DATA;
        }
        else {
            if (prConnSettings->eAuthMode < AUTH_MODE_WPA) {
                if (prNewKey->u4KeyLength == WEP_40_LEN) {
                    ucCipher = CIPHER_SUITE_WEP40;
                }
                else if (prNewKey->u4KeyLength == WEP_104_LEN) {
                    ucCipher = CIPHER_SUITE_WEP104;
                }
                else if (prNewKey->u4KeyLength == LEGACY_KEY_MAX_LEN) {
                    ucCipher = CIPHER_SUITE_WEP128;
                }
            }
            else {
                ucCipher =
                    GET_SELECTOR_TYPE(prAdapter->rBssInfo.u4RsnSelectedPairwiseCipher);
            }

            /* Set default Tx key index */
            if (EQUAL_MAC_ADDR(pucBssid, prAdapter->rBssInfo.aucBSSID)) {
                prConnSettings->rMib.dot11WEPDefaultKeyID = (UINT_8)ucKeyId;
                prConnSettings->rMib.dot11TranmitKeyAvailable = TRUE;
            }

            if (secFsmRunEventPTKInstalled(prAdapter, 
                                           pucBssid,
                                           prNewKey->aucKeyMaterial,
                                          (UINT_8)prNewKey->u4KeyLength,
                                           ucCipher,
                                           ucTxMicOffset,
                                           ucRxMicOffset) == FALSE) {
                /* for key handshake, set to template entry */
                } 
#if CFG_TX_FRAGMENT
            {
            BOOLEAN fgIsTkipCipher = FALSE;
            if (ucCipher == CIPHER_SUITE_TKIP){
                fgIsTkipCipher = TRUE;
            }
            txFragInfoUpdateForPrivacy(prAdapter, TRUE, fgIsTkipCipher);
            }
#endif    
        }
        break;
    default:
        break;
    }

    if (status == WLAN_STATUS_SUCCESS) {
        if (prNewKey->u4KeyIndex & IS_UNICAST_KEY) {
            /* Reset TSC */
        }
    } /* if() */

    return status;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
privacyClearKeyEntry (
    IN P_ADAPTER_T          prAdapter,
    IN BOOLEAN              fgPairwise,
    IN PUINT_8              pucBssid,
    IN UINT_8               ucKeyId
    )
{
    UINT_8                  aucKeyMaterial[MAX_KEY_LEN];
    UINT_8                  ucEntryIndex;
    P_WLAN_ENTRY_CTRL_T     prWlanCtrl;
    P_WT_CTRL_STRUC_T       prCtrl;

    DEBUGFUNC("privacySetIpcKeyEntry");

    ASSERT(prAdapter);
    ASSERT(ucKeyId < MAX_KEY_NUM);

    kalMemZero((PVOID)&aucKeyMaterial[0], MAX_KEY_LEN);
    
    if (fgPairwise /* && (EQUAL_MAC_ADDR(pucBssid, prAdapter->rBssInfo.aucBSSID))*/) {
        prAdapter->rConnSettings.rMib.dot11WEPDefaultKeyID = 0;
        prAdapter->rConnSettings.rMib.dot11TranmitKeyAvailable = FALSE;
    }

    if (nicPrivacyMatchForEntry(prAdapter,
                            pucBssid,
                            ucKeyId,
                            &ucEntryIndex)) {
        UINT_8              ucMac[6], ucMuar, ucRate1, ucRate2, ucRate3, ucRate4;                    
        BOOLEAN             fgUsed = FALSE, fgT1X= FALSE, fgR1X = FALSE, fgQoS = FALSE, fgAntenna = FALSE;
        P_STA_RECORD_T prSta;

        prWlanCtrl = &prAdapter->arWlanCtrl[ucEntryIndex];
        if (fgPairwise) {
            fgUsed = prWlanCtrl->fgUsed;
        }

        prCtrl = &prWlanCtrl->rCtrl;
        ASSERT(prCtrl);

        fgT1X = prCtrl->fgT1X;
        fgR1X = prCtrl->fgR1X;
        fgQoS = prCtrl->fgQoS;
        ucMuar = prCtrl->ucMuar;
        ucRate1 = prCtrl->ucRate1;
        ucRate2 = prCtrl->ucRate2;
        ucRate3 = prCtrl->ucRate3;
        ucRate4 = prCtrl->ucRate4;
        fgAntenna = prCtrl->fgAntenna;

        prSta = prAdapter->arWlanCtrl[ucEntryIndex].prSta;

        kalMemCopy(&ucMac[0], (PVOID)prAdapter->arWlanCtrl[ucEntryIndex].rSWKey.aucAddr, MAC_ADDR_LEN);
        kalMemZero((PVOID)&prAdapter->arWlanCtrl[ucEntryIndex], sizeof(WLAN_ENTRY_CTRL_T));
        kalMemCopy(prAdapter->arWlanCtrl[ucEntryIndex].rSWKey.aucAddr, (PVOID)&ucMac[0], MAC_ADDR_LEN);

        prAdapter->arWlanCtrl[ucEntryIndex].prSta = prSta;

        if (fgPairwise) {
            prWlanCtrl->fgUsed = fgUsed;
        }
        prCtrl->fgT1X = fgT1X;
        prCtrl->fgR1X = fgR1X;
        prCtrl->fgQoS = fgQoS;
        prCtrl->fgAntenna = fgAntenna;
        prCtrl->ucMuar = ucMuar;
        prCtrl->ucRate1 = ucRate1;
        prCtrl->ucRate2 = ucRate2;
        prCtrl->ucRate3 = ucRate3;
        prCtrl->ucRate4 = ucRate4;
        
        nicPrivacyDumpWlanTable(prAdapter, ucEntryIndex);

        /* Set key into H/W key table */
        if (!halWlanTableAccess(prAdapter, 
                           (PVOID)prWlanCtrl,
                           ucEntryIndex,
                           3)) {
            DBGLOG(RSN, TRACE, ("Fail to clear key !\n"));
        }
    }
    else {
        DBGLOG(RSN, TRACE, ("No matched Wlan Table to remove!!\n"));
    }

    return;
} /* privacyClearKeyEntry */

