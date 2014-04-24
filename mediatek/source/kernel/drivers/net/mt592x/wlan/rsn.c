






#include "precomp.h"

extern PHY_ATTRIBUTE_T rPhyAttributes[];








/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnParseRsnIE (
    IN  P_RSN_INFO_ELEM_T prInfoElem,
    OUT P_RSN_INFO_T      prRsnInfo
    )
{
    UINT_32               i;
    INT_32                u4RemainRsnIeLen;
    UINT_16               u2Version;
    UINT_16               u2Cap = 0;
    UINT_32               u4GroupSuite = RSN_CIPHER_SUITE_CCMP;
    UINT_16               u2PairSuiteCount= 0;
    UINT_16               u2AuthSuiteCount = 0;
    PUINT_8               pucPairSuite = NULL;
    PUINT_8               pucAuthSuite = NULL;
    PUINT_8               cp;

    DEBUGFUNC("rsnParseRsnIE");

    ASSERT(prInfoElem);
    ASSERT(prRsnInfo);

    /* Verify the length of the RSN IE. */
    if (prInfoElem->ucLength < 2) {
        DBGLOG(RSN, WARN, ("RSN IE length too short (length=%d)\n", prInfoElem->ucLength));
        return FALSE;
    }

    /* Check RSN version: currently, we only support version 1. */
    WLAN_GET_FIELD_16(&prInfoElem->u2Version, &u2Version);
    if (u2Version != 1) {
        DBGLOG(RSN, WARN,("Unsupported RSN IE version: %d\n", u2Version));
        return FALSE;
    }

    cp = (PUCHAR) &prInfoElem->u4GroupKeyCipherSuite;
    u4RemainRsnIeLen = (INT_32) prInfoElem->ucLength - 2;

    do {
        if (u4RemainRsnIeLen == 0) {
            break;
        }

        /* Parse the Group Key Cipher Suite field. */
        if (u4RemainRsnIeLen < 4) {
            DBGLOG(RSN, WARN,("Fail to parse RSN IE in group cipher suite (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_32(cp, &u4GroupSuite);
        cp += 4;
        u4RemainRsnIeLen -= 4;

        if (u4RemainRsnIeLen == 0) {
            break;
        }

        /* Parse the Pairwise Key Cipher Suite Count field. */
        if (u4RemainRsnIeLen < 2) {
            DBGLOG(RSN, WARN,("Fail to parse RSN IE in pairwise cipher suite count (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2PairSuiteCount);
        cp += 2;
        u4RemainRsnIeLen -= 2;

        /* Parse the Pairwise Key Cipher Suite List field. */
        i = (UINT_32) u2PairSuiteCount * 4;
        if (u4RemainRsnIeLen < (INT_32) i) {
            DBGLOG(RSN, WARN,("Fail to parse RSN IE in pairwise cipher suite list (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        pucPairSuite = cp;

        cp += i;
        u4RemainRsnIeLen -= (INT_32) i;

        if (u4RemainRsnIeLen == 0) {
            break;
        }

        /* Parse the Authentication and Key Management Cipher Suite Count field. */
        if (u4RemainRsnIeLen < 2) {
            DBGLOG(RSN, WARN,("Fail to parse RSN IE in auth & key mgt suite count (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2AuthSuiteCount);
        cp += 2;
        u4RemainRsnIeLen -= 2;

        /* Parse the Authentication and Key Management Cipher Suite List
           field. */
        i = (UINT_32) u2AuthSuiteCount * 4;
        if (u4RemainRsnIeLen < (INT_32) i) {
            DBGLOG(RSN, WARN,("Fail to parse RSN IE in auth & key mgt suite list (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        pucAuthSuite = cp;

        cp += i;
        u4RemainRsnIeLen -= (INT_32) i;

        if (u4RemainRsnIeLen == 0) {
            break;
        }

        /* Parse the RSN u2Capabilities field. */
        if (u4RemainRsnIeLen < 2) {
            DBGLOG(RSN, WARN,("Fail to parse RSN IE in RSN capabilities (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2Cap);
    } while (FALSE);

    /* Save the RSN information for the BSS. */
    prRsnInfo->ucElemId = ELEM_ID_RSN;

    prRsnInfo->u2Version = u2Version;

    prRsnInfo->u4GroupKeyCipherSuite = u4GroupSuite;

    DBGLOG(RSN, INFO,
        ("RSN: version %d, group key cipher suite %02x-%02x-%02x-%02x\n",
        u2Version, (UCHAR) (u4GroupSuite & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 8) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 16) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 24) & 0x000000FF)));

    if (pucPairSuite) {
        /* The information about the pairwise key cipher suites is present. */
        if (u2PairSuiteCount > MAX_NUM_SUPPORTED_CIPHER_SUITES) {
            u2PairSuiteCount = MAX_NUM_SUPPORTED_CIPHER_SUITES;
        }

        prRsnInfo->u4PairwiseKeyCipherSuiteCount = (UINT_32) u2PairSuiteCount;

        for (i = 0; i < (UINT_32) u2PairSuiteCount; i++) {
            WLAN_GET_FIELD_32(pucPairSuite,
                &prRsnInfo->au4PairwiseKeyCipherSuite[i]);
            pucPairSuite += 4;

            DBGLOG(RSN, INFO,
                ("RSN: pairwise key cipher suite [%d]: %02x-%02x-%02x-%02x\n",
                (UINT_8)i, (UCHAR) (prRsnInfo->au4PairwiseKeyCipherSuite[i] & 0x000000FF),
                (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[i] >> 8) & 0x000000FF),
                (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[i] >> 16) & 0x000000FF),
                (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[i] >> 24) & 0x000000FF)));
        }
    }
    else {
        /* The information about the pairwise key cipher suites is not present.
           Use the default chipher suite for RSN: CCMP. */
        prRsnInfo->u4PairwiseKeyCipherSuiteCount = 1;
        prRsnInfo->au4PairwiseKeyCipherSuite[0] = RSN_CIPHER_SUITE_CCMP;

        DBGLOG(RSN, INFO,
            ("RSN: pairwise key cipher suite: %02x-%02x-%02x-%02x (default)\n",
            (UCHAR) (prRsnInfo->au4PairwiseKeyCipherSuite[0] & 0x000000FF),
            (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[0] >> 8) & 0x000000FF),
            (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[0] >> 16) & 0x000000FF),
            (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[0] >> 24) & 0x000000FF)));
    }

    if (pucAuthSuite) {
        /* The information about the authentication and key management suites
           is present. */
        if (u2AuthSuiteCount > MAX_NUM_SUPPORTED_AKM_SUITES) {
            u2AuthSuiteCount = MAX_NUM_SUPPORTED_AKM_SUITES;
        }

        prRsnInfo->u4AuthKeyMgtSuiteCount = (UINT_32) u2AuthSuiteCount;

        for (i = 0; i < (UINT_32) u2AuthSuiteCount; i++) {
            WLAN_GET_FIELD_32(pucAuthSuite, &prRsnInfo->au4AuthKeyMgtSuite[i]);
            pucAuthSuite += 4;

            DBGLOG(RSN, INFO,
                ("RSN: AKM suite [%d]: %02x-%02x-%02x-%02x\n",
                (UINT_8)i, (UCHAR) (prRsnInfo->au4AuthKeyMgtSuite[i] & 0x000000FF),
                (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[i] >> 8) & 0x000000FF),
                (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[i] >> 16) & 0x000000FF),
                (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[i] >> 24) & 0x000000FF)));
        }
    }
    else {
        /* The information about the authentication and key management suites
           is not present. Use the default AKM suite for RSN. */
        prRsnInfo->u4AuthKeyMgtSuiteCount = 1;
        prRsnInfo->au4AuthKeyMgtSuite[0] = RSN_AKM_SUITE_802_1X;

        DBGLOG(RSN, INFO,
            ("RSN: AKM suite: %02x-%02x-%02x-%02x (default)\n",
            (UCHAR) (prRsnInfo->au4AuthKeyMgtSuite[0] & 0x000000FF),
            (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[0] >> 8) & 0x000000FF),
            (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[0] >> 16) & 0x000000FF),
            (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[0] >> 24) & 0x000000FF)));
    }

    prRsnInfo->u2RsnCap = u2Cap;

    DBGLOG(RSN, INFO, ("RSN cap: 0x%04x\n", prRsnInfo->u2RsnCap));

    return TRUE;
}   /* rsnParseRsnIE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnParseWpaIE (
    IN  P_WPA_INFO_ELEM_T prInfoElem,
    OUT P_RSN_INFO_T      prWpaInfo
    )
{
    UINT_32               i;
    INT_32                u4RemainWpaIeLen;
    UINT_16               u2Version;
    UINT_16               u2Cap = 0;
    UINT_32               u4GroupSuite = WPA_CIPHER_SUITE_TKIP;
    UINT_16               u2PairSuiteCount = 0;
    UINT_16               u2AuthSuiteCount = 0;
    PUCHAR                pucPairSuite = NULL;
    PUCHAR                pucAuthSuite = NULL;
    PUCHAR                cp;
    BOOLEAN               fgCapPresent = FALSE;

    DEBUGFUNC("rsnParseWpaIE");

    ASSERT(prInfoElem);
    ASSERT(prWpaInfo);

    /* Verify the length of the WPA IE. */
    if (prInfoElem->ucLength < 6) {
        DBGLOG(RSN, WARN,("WPA IE length too short (length=%d)\n", prInfoElem->ucLength));
        return FALSE;
    }

    /* Check WPA version: currently, we only support version 1. */
    WLAN_GET_FIELD_16(&prInfoElem->u2Version, &u2Version);
    if (u2Version != 1) {
        DBGLOG(RSN, WARN, ("Unsupported WPA IE version: %d\n", u2Version));
        return FALSE;
    }

    cp = (PUCHAR) &prInfoElem->u4GroupKeyCipherSuite;
    u4RemainWpaIeLen = (INT_32) prInfoElem->ucLength - 6;

    do {
        if (u4RemainWpaIeLen == 0) {
            break;
        }

        /* WPA_OUI      : 4
           Version      : 2
           GroupSuite   : 4
           PairwiseCount: 2
           PairwiseSuite: 4 * pairSuiteCount
           AuthCount    : 2
           AuthSuite    : 4 * authSuiteCount
           Cap          : 2 */

        /* Parse the Group Key Cipher Suite field. */
        if (u4RemainWpaIeLen < 4) {
            DBGLOG(RSN, WARN,(
                "Fail to parse WPA IE in group cipher suite (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_32(cp, &u4GroupSuite);
        cp += 4;
        u4RemainWpaIeLen -= 4;

        if (u4RemainWpaIeLen == 0) {
            break;
        }

        /* Parse the Pairwise Key Cipher Suite Count field. */
        if (u4RemainWpaIeLen < 2) {
            DBGLOG(RSN, WARN,(
                "Fail to parse WPA IE in pairwise cipher suite count (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2PairSuiteCount);
        cp += 2;
        u4RemainWpaIeLen -= 2;

        /* Parse the Pairwise Key Cipher Suite List field. */
        i = (UINT_32) u2PairSuiteCount * 4;
        if (u4RemainWpaIeLen < (INT_32) i) {
            DBGLOG(RSN, WARN,(
                "Fail to parse WPA IE in pairwise cipher suite list (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        pucPairSuite = cp;

        cp += i;
        u4RemainWpaIeLen -= (INT_32) i;

        if (u4RemainWpaIeLen == 0) {
            break;
        }

        /* Parse the Authentication and Key Management Cipher Suite Count
           field. */
        if (u4RemainWpaIeLen < 2) {
            DBGLOG(RSN, WARN,(
                "Fail to parse WPA IE in auth & key mgt suite count (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2AuthSuiteCount);
        cp += 2;
        u4RemainWpaIeLen -= 2;

        /* Parse the Authentication and Key Management Cipher Suite List
           field. */
        i = (UINT_32) u2AuthSuiteCount * 4;
        if (u4RemainWpaIeLen < (INT_32) i) {
            DBGLOG(RSN, WARN, (
                "Fail to parse WPA IE in auth & key mgt suite list (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        pucAuthSuite = cp;

        cp += i;
        u4RemainWpaIeLen -= (INT_32) i;

        if (u4RemainWpaIeLen == 0) {
            break;
        }

        /* Parse the WPA u2Capabilities field. */
        if (u4RemainWpaIeLen < 2) {
            DBGLOG(RSN, WARN, ("Fail to parse WPA IE in WPA capabilities (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        fgCapPresent = TRUE;
        WLAN_GET_FIELD_16(cp, &u2Cap);
        u4RemainWpaIeLen -= 2;
    } while (FALSE);

    /* Save the WPA information for the BSS. */

    prWpaInfo->ucElemId = ELEM_ID_WPA;

    prWpaInfo->u2Version = u2Version;

    prWpaInfo->u4GroupKeyCipherSuite = u4GroupSuite;

    DBGLOG(RSN, LOUD,
        ("WPA: version %d, group key cipher suite %02x-%02x-%02x-%02x\n",
        u2Version, (UCHAR) (u4GroupSuite & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 8) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 16) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 24) & 0x000000FF)));

    if (pucPairSuite) {
        /* The information about the pairwise key cipher suites is present. */
        if (u2PairSuiteCount > MAX_NUM_SUPPORTED_CIPHER_SUITES) {
            u2PairSuiteCount = MAX_NUM_SUPPORTED_CIPHER_SUITES;
        }

        prWpaInfo->u4PairwiseKeyCipherSuiteCount = (UINT_32) u2PairSuiteCount;

        for (i = 0; i < (UINT_32) u2PairSuiteCount; i++) {
            WLAN_GET_FIELD_32(pucPairSuite,
                              &prWpaInfo->au4PairwiseKeyCipherSuite[i]);
            pucPairSuite += 4;

            DBGLOG(RSN, LOUD,
                ("WPA: pairwise key cipher suite [%d]: %02x-%02x-%02x-%02x\n",
                (UINT_8)i, (UCHAR) (prWpaInfo->au4PairwiseKeyCipherSuite[i] & 0x000000FF),
                (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[i] >> 8) & 0x000000FF),
                (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[i] >> 16) & 0x000000FF),
                (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[i] >> 24) & 0x000000FF)));
        }
    }
    else {
        /* The information about the pairwise key cipher suites is not present.
           Use the default chipher suite for WPA: TKIP. */
        prWpaInfo->u4PairwiseKeyCipherSuiteCount = 1;
        prWpaInfo->au4PairwiseKeyCipherSuite[0] = WPA_CIPHER_SUITE_TKIP;

        DBGLOG(RSN, LOUD,
            ("WPA: pairwise key cipher suite: %02x-%02x-%02x-%02x (default)\n",
            (UCHAR) (prWpaInfo->au4PairwiseKeyCipherSuite[0] & 0x000000FF),
            (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[0] >> 8) & 0x000000FF),
            (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[0] >> 16) & 0x000000FF),
            (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[0] >> 24) & 0x000000FF)));
    }

    if (pucAuthSuite) {
        /* The information about the authentication and key management suites
           is present. */
        if (u2AuthSuiteCount > MAX_NUM_SUPPORTED_AKM_SUITES) {
            u2AuthSuiteCount = MAX_NUM_SUPPORTED_AKM_SUITES;
        }

        prWpaInfo->u4AuthKeyMgtSuiteCount = (UINT_32) u2AuthSuiteCount;

        for (i = 0; i < (UINT_32) u2AuthSuiteCount; i++) {
            WLAN_GET_FIELD_32(pucAuthSuite, &prWpaInfo->au4AuthKeyMgtSuite[i]);
            pucAuthSuite += 4;

            DBGLOG(RSN, LOUD,
                ("WPA: AKM suite [%d]: %02x-%02x-%02x-%02x\n",
                (UINT_8)i, (UCHAR) (prWpaInfo->au4AuthKeyMgtSuite[i] & 0x000000FF),
                (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[i] >> 8) & 0x000000FF),
                (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[i] >> 16) & 0x000000FF),
                (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[i] >> 24) & 0x000000FF)));
        }
    }
    else {
        /* The information about the authentication and key management suites
           is not present. Use the default AKM suite for WPA. */
        prWpaInfo->u4AuthKeyMgtSuiteCount = 1;
        prWpaInfo->au4AuthKeyMgtSuite[0] = WPA_AKM_SUITE_802_1X;

        DBGLOG(RSN, LOUD,
            ("WPA: AKM suite: %02x-%02x-%02x-%02x (default)\n",
            (UCHAR) (prWpaInfo->au4AuthKeyMgtSuite[0] & 0x000000FF),
            (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[0] >> 8) & 0x000000FF),
            (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[0] >> 16) & 0x000000FF),
            (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[0] >> 24) & 0x000000FF)));
    }

    if (fgCapPresent) {
        prWpaInfo->fgRsnCapPresent = TRUE;
        prWpaInfo->u2RsnCap = u2Cap;
        DBGLOG(RSN, LOUD, ("WPA: RSN cap: 0x%04x\n", prWpaInfo->u2RsnCap));
    }
    else { 
        prWpaInfo->fgRsnCapPresent = FALSE;
        prWpaInfo->u2RsnCap = 0;
    }

    return TRUE;
}   /* rsnParseWpaIE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnSearchSupportedCipher (
    IN  P_ADAPTER_T         prAdapter,
    IN  UINT_32             u4Cipher,
    OUT PUINT_32            pu4Index
    )
{
    UINT_8 i;
    P_DOT11_RSNA_CONFIG_PAIRWISE_CIPHERS_ENTRY prEntry;

    DEBUGFUNC("rsnSearchSupportedCipher");

    ASSERT(prAdapter);
    ASSERT(pu4Index);
    
    for (i = 0; i < MAX_NUM_SUPPORTED_CIPHER_SUITES; i++) {
        prEntry = &prAdapter->rConnSettings.rMib.dot11RSNAConfigPairwiseCiphersTable[i];
        if (prEntry->dot11RSNAConfigPairwiseCipher == u4Cipher &&
            prEntry->dot11RSNAConfigPairwiseCipherEnabled) {
            *pu4Index = i;
            return TRUE;
        }
    }
    return FALSE;
}   /* rsnSearchSupportedCipher */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnSearchAKMSuite (
    IN  P_ADAPTER_T         prAdapter,
    IN  UINT_32             u4AkmSuite,
    OUT PUINT_32            pu4Index
    )
{
    UINT_8 i;
    P_DOT11_RSNA_CONFIG_AUTHENTICATION_SUITES_ENTRY prEntry;

    DEBUGFUNC("rsnSearchAKMSuite");

    ASSERT(prAdapter);
    ASSERT(pu4Index);
    
    for (i = 0; i < MAX_NUM_SUPPORTED_AKM_SUITES; i++) {
        prEntry = &prAdapter->rConnSettings.rMib.dot11RSNAConfigAuthenticationSuitesTable[i];
        if (prEntry->dot11RSNAConfigAuthenticationSuite == u4AkmSuite &&
            prEntry->dot11RSNAConfigAuthenticationSuiteEnabled) {
            *pu4Index = i;
            return TRUE;
        }
    }
    return FALSE;
}   /* rsnSearchAKMSuite */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnPerformPolicySelection (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_BSS_DESC_T        prBss
    )
{
    UINT_32                 i, j;
    BOOLEAN                 fgSuiteSupported;
    UINT_32                 u4PairwiseCipher = 0;
    UINT_32                 u4GroupCipher = 0;
    UINT_32                 u4AkmSuite = 0;
    RSN_INFO_T              rTmpRsnInfo;
    P_RSN_INFO_T            prBssRsnInfo;

    DEBUGFUNC("rsnPerformPolicySelection");

    ASSERT(prAdapter);
    ASSERT(prBss);
    
    prBss->u4RsnSelectedPairwiseCipher = 0;
    prBss->u4RsnSelectedGroupCipher = 0;
    prBss->u4RsnSelectedAKMSuite = 0;
    prBss->ucEncLevel = 0;

#if SUPPORT_WPS
    /* CR1640, disable the AP select privacy check */
    if ( prAdapter->rSecInfo.fgPrivacyCheckDisable &&
        (prAdapter->rConnSettings.eAuthMode < AUTH_MODE_WPA) &&
        (prAdapter->rConnSettings.eOPMode == NET_TYPE_INFRA)) {
        DBGLOG(RSN, TRACE,("-- Skip the Protected BSS check\n"));
        return TRUE;
    }
#endif
    
    /* Protection is not required in this BSS. */
    if ((prBss->u2CapInfo & CAP_INFO_PRIVACY) == 0 ) {

        if (privacyEnabled(prAdapter) == FALSE) {
            DBGLOG(RSN, TRACE,("-- No Protected BSS\n"));
            return TRUE;
        }
        else {
            DBGLOG(RSN, TRACE,("-- Protected BSS\n"));
            return FALSE;
        }
    }

    /* Protection is required in this BSS. */
    if ((prBss->u2CapInfo & CAP_INFO_PRIVACY) != 0) {
        if (privacyEnabled(prAdapter) == FALSE) {
            DBGLOG(RSN, TRACE,("-- Protected BSS\n"));
            return FALSE;
        }
    }

    if (prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA ||
        prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA_PSK ||
        prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA_NONE) {

        if (prBss->prIEWPA != NULL) {
            if (!rsnParseWpaIE((P_WPA_INFO_ELEM_T)prBss->prIEWPA, &rTmpRsnInfo)) {
                DBGLOG(RSN, INFO, ("-- Policy selection fail for WPA IE .\n"));
                return FALSE;
            }
        }
        else {
            DBGLOG(RSN, INFO, ("WPA Information Element does not exist.\n"));
            return FALSE;
        }
    }
    else if (prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA2 ||
        prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA2_PSK) {

        if (prBss->prIERSN != NULL) {
            if (!rsnParseRsnIE((P_RSN_INFO_ELEM_T)prBss->prIERSN, &rTmpRsnInfo)) {
                DBGLOG(RSN, TRACE, ("-- Policy selection fail for RSN IE .\n"));
                return FALSE;
            }
        }
        else {
            DBGLOG(RSN, TRACE, ("RSN Information Element does not exist.\n"));
            return FALSE;
        }
    }
    else if (prAdapter->rConnSettings.eEncStatus != ENUM_ENCRYPTION1_ENABLED) {
        /* If the driver is configured to use WEP only, ignore this BSS. */
        DBGLOG(MGT, INFO, ("-- Not WEP-only legacy BSS\n"));
        return FALSE;
    }
    else if (prAdapter->rConnSettings.eEncStatus == ENUM_ENCRYPTION1_ENABLED) {
        /* If the driver is configured to use WEP only, use this BSS. */
        DBGLOG(MGT, INFO, ("-- WEP-only legacy BSS\n"));
        return TRUE;
    }

    prBssRsnInfo = &rTmpRsnInfo;

    if (prBssRsnInfo->u4PairwiseKeyCipherSuiteCount == 1 &&
        GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[0]) ==
        CIPHER_SUITE_NONE) {
        /* Since the pairwise cipher use the same cipher suite as the group
           cipher in the BSS, we check the group cipher suite against the
           current encryption status. */
        fgSuiteSupported = FALSE;

        switch (prBssRsnInfo->u4GroupKeyCipherSuite) {
        case WPA_CIPHER_SUITE_CCMP:
        case RSN_CIPHER_SUITE_CCMP:
             if (prAdapter->rConnSettings.eEncStatus ==
                 ENUM_ENCRYPTION3_ENABLED) {
                 fgSuiteSupported = TRUE;
             }
             break;

        case WPA_CIPHER_SUITE_TKIP:
        case RSN_CIPHER_SUITE_TKIP:
             if (prAdapter->rConnSettings.eEncStatus ==
                 ENUM_ENCRYPTION2_ENABLED) {
                 fgSuiteSupported = TRUE;
             }
             break;

        case WPA_CIPHER_SUITE_WEP40:
        case WPA_CIPHER_SUITE_WEP104:
             if (prAdapter->rConnSettings.eEncStatus ==
                 ENUM_ENCRYPTION1_ENABLED) {
                 fgSuiteSupported = TRUE;
             }
             break;
        }

        if (fgSuiteSupported) {
            u4PairwiseCipher = WPA_CIPHER_SUITE_NONE;
            u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
        }
#if DBG
        else {
            DBGLOG(RSN, INFO,
                ("Inproper encryption status %d for group-key-only BSS\n",
                prAdapter->rConnSettings.eEncStatus));
        }
#endif
    }
    else {
        fgSuiteSupported = FALSE;

        /* Select pairwise/group ciphers */
        switch (prAdapter->rConnSettings.eEncStatus)
        {
        case ENUM_ENCRYPTION3_ENABLED:
             for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
                 if (GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
                     == CIPHER_SUITE_CCMP) {
                     u4PairwiseCipher = prBssRsnInfo->au4PairwiseKeyCipherSuite[i];
                 }
             }
             u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
             break;

        case ENUM_ENCRYPTION2_ENABLED:
             for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
                 if (GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
                     == CIPHER_SUITE_TKIP) {
                     u4PairwiseCipher = prBssRsnInfo->au4PairwiseKeyCipherSuite[i];
                 }
             }
             if (GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite) ==
                 CIPHER_SUITE_CCMP) {
                 DBGLOG(RSN, INFO, ("Cannot join CCMP BSS\n"));
             }
             else {
                 u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
             }
             break;

        case ENUM_ENCRYPTION1_ENABLED:
             for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
                 if (GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
                     == CIPHER_SUITE_WEP40 ||
                     GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
                     == CIPHER_SUITE_WEP104) {
                     u4PairwiseCipher = prBssRsnInfo->au4PairwiseKeyCipherSuite[i];
                 }
             }
             if (GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite) ==
                 CIPHER_SUITE_CCMP ||
                 GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite) ==
                 CIPHER_SUITE_TKIP) {
                 DBGLOG(RSN, INFO, ("Cannot join CCMP/TKIP BSS\n"));
             }
             else {
                 u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
             }
             break;

        default:
             break;
        }
    }

    /* Exception handler */
    /* If we cannot find proper pairwise and group cipher suites to join the
       BSS, do not check the supported AKM suites. */
    if (u4PairwiseCipher == 0 || u4GroupCipher == 0) {
        DBGLOG(RSN, INFO,
            ("Failed to select pairwise/group cipher (0x%08lx/0x%08lx)\n",
            u4PairwiseCipher, u4GroupCipher));
        return FALSE;
    }

    /* Verify if selected pairwisse cipher is supported */
    fgSuiteSupported = rsnSearchSupportedCipher(prAdapter, u4PairwiseCipher, &i);

    /* Verify if selected group cipher is supported */
    if (fgSuiteSupported) {
        fgSuiteSupported = rsnSearchSupportedCipher(prAdapter, u4GroupCipher, &i);
    }

    if (!fgSuiteSupported) {
        DBGLOG(RSN, INFO,
            ("Failed to support selected pairwise/group cipher (0x%08lx/0x%08lx)\n",
            u4PairwiseCipher, u4GroupCipher));
        return FALSE;
    }

    /* Select AKM */
    /* If the driver cannot support any authentication suites advertised in
       the given BSS, we fail to perform RSNA policy selection. */
    /* Attempt to find any overlapping supported AKM suite. */
    for (i = 0; i < prBssRsnInfo->u4AuthKeyMgtSuiteCount; i++) {
        if (rsnSearchAKMSuite(prAdapter, prBssRsnInfo->au4AuthKeyMgtSuite[i],
            &j)) {
            u4AkmSuite = prBssRsnInfo->au4AuthKeyMgtSuite[i];
            break;
        }
    }

    if (u4AkmSuite == 0) {
        DBGLOG(RSN, INFO, ("Cannot support any AKM suites\n"));
        return FALSE;
    }

    DBGLOG(RSN, TRACE,
        ("Selected pairwise/group cipher: %02x-%02x-%02x-%02x/%02x-%02x-%02x-%02x\n",
        (UINT_8) (u4PairwiseCipher & 0x000000FF),
        (UINT_8) ((u4PairwiseCipher >> 8) & 0x000000FF),
        (UINT_8) ((u4PairwiseCipher >> 16) & 0x000000FF),
        (UINT_8) ((u4PairwiseCipher >> 24) & 0x000000FF),
        (UINT_8) (u4GroupCipher & 0x000000FF),
        (UINT_8) ((u4GroupCipher >> 8) & 0x000000FF),
        (UINT_8) ((u4GroupCipher >> 16) & 0x000000FF),
        (UINT_8) ((u4GroupCipher >> 24) & 0x000000FF)));

    DBGLOG(RSN, TRACE, ("Selected AKM suite: %02x-%02x-%02x-%02x\n",
        (UINT_8) (u4AkmSuite & 0x000000FF),
        (UINT_8) ((u4AkmSuite >> 8) & 0x000000FF),
        (UINT_8) ((u4AkmSuite >> 16) & 0x000000FF),
        (UINT_8) ((u4AkmSuite >> 24) & 0x000000FF)));

    if (GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_CCMP){
        prBss->ucEncLevel = 3;
    }
    else if (GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_TKIP){
        prBss->ucEncLevel = 2;
    }
    else if (GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_WEP40 ||
        GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_WEP104) {
        prBss->ucEncLevel = 1;
    }
    else {
        ASSERT(FALSE);
    }
    prBss->u4RsnSelectedPairwiseCipher = u4PairwiseCipher;
    prBss->u4RsnSelectedGroupCipher = u4GroupCipher;
    prBss->u4RsnSelectedAKMSuite = u4AkmSuite;

    return TRUE;

}  /* rsnPerformPolicySelection */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_8
rsnGenerateWpaNoneIE (
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucIeStartAddr
    )
{
    UINT_32                 i;
    P_WPA_INFO_ELEM_T       prWpaIE;
    UINT_32                 u4Suite;
    UINT_16                 u2SuiteCount;
    PUINT_8                 cp, cp2;
    UINT_8                  ucExpendedLen = 0;

    DEBUGFUNC("rsnGenerateWpaNoneIE");

    ASSERT(prAdapter);
    ASSERT(pucIeStartAddr);

    if (prAdapter->rConnSettings.eAuthMode != AUTH_MODE_WPA_NONE) {
        return 0;
    }

    prWpaIE = (P_WPA_INFO_ELEM_T)pucIeStartAddr;

    /* Start to construct a WPA IE. */
    /* Fill the Element ID field. */
    prWpaIE->ucElemId = ELEM_ID_WPA;

    /* Fill the OUI and OUI Type fields. */
    prWpaIE->aucOui[0] = 0x00;
    prWpaIE->aucOui[1] = 0x50;
    prWpaIE->aucOui[2] = 0xF2;
    prWpaIE->ucOuiType = VENDOR_OUI_TYPE_WPA;

    /* Fill the Version field. */
    WLAN_SET_FIELD_16(&prWpaIE->u2Version, 1);    /* version 1 */
    ucExpendedLen = 6;

    /* Fill the Pairwise Key Cipher Suite List field. */
    u2SuiteCount = 0;
    cp = (PUINT_8)&prWpaIE->aucPairwiseKeyCipherSuite1[0];

    if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_CCMP, &i)) {
        u4Suite = WPA_CIPHER_SUITE_CCMP;
    }
    else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_TKIP, &i)) {
        u4Suite = WPA_CIPHER_SUITE_TKIP;
    }
    else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_WEP104, &i)) {
        u4Suite = WPA_CIPHER_SUITE_WEP104;
    }
    else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_WEP40, &i)) {
        u4Suite = WPA_CIPHER_SUITE_WEP40;
    }
    else {
        u4Suite = WPA_CIPHER_SUITE_TKIP;
    }

    WLAN_SET_FIELD_32(cp, u4Suite);
    u2SuiteCount++;
    ucExpendedLen += 4;
    cp += 4;

    /* Fill the Group Key Cipher Suite field as the same in pair-wise key. */
    WLAN_SET_FIELD_32(&prWpaIE->u4GroupKeyCipherSuite, u4Suite);
    ucExpendedLen += 4;


    /* Fill the Pairwise Key Cipher Suite Count field. */
    WLAN_SET_FIELD_16(&prWpaIE->u2PairwiseKeyCipherSuiteCount, u2SuiteCount);
    ucExpendedLen += 2;

    cp2 = cp;

    /* Fill the Authentication and Key Management Suite List field. */
    u2SuiteCount = 0;
    cp += 2;

    if (rsnSearchAKMSuite(prAdapter, WPA_AKM_SUITE_802_1X, &i)) {
        u4Suite = WPA_AKM_SUITE_802_1X;
    }
    else if (rsnSearchAKMSuite(prAdapter, WPA_AKM_SUITE_PSK, &i)) {
        u4Suite = WPA_AKM_SUITE_PSK;
    }
    else {
        u4Suite = WPA_AKM_SUITE_NONE;
    }

    /* This shall be the only avaiable value for current implementation */
    ASSERT(u4Suite == WPA_AKM_SUITE_NONE);

    WLAN_SET_FIELD_32(cp, u4Suite);
    u2SuiteCount++;
    ucExpendedLen += 4;
    cp += 4;

    /* Fill the Authentication and Key Management Suite Count field. */
    WLAN_SET_FIELD_16(cp2, u2SuiteCount);
    ucExpendedLen += 2;

    /* Fill the Length field. */
    prWpaIE->ucLength = (UINT_8)ucExpendedLen;

    /* Increment the total IE length for the Element ID and Length fields. */
    ucExpendedLen += 2;

    return ucExpendedLen;
} /* rsnGenerateWpaNoneIE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_8
rsnGenerateWPARSNIE (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_BSS_DESC_T        prCurrentBss,
    IN  PUINT_8             pucIeStartAddr
    )
{
    UINT_32                 u4Entry;
    PUCHAR                  cp;
    UINT_8                  ucExpendedLen = 0;

    DEBUGFUNC("rsnGenerateWPARSNIE");

    ASSERT(prAdapter);
    ASSERT(prCurrentBss);
    ASSERT(pucIeStartAddr);
    
    if (privacyRsnKeyHandshakeEnabled(prAdapter))
    {
        if (prCurrentBss->prIERSN &&
            ((prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA2) ||
            (prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA2_PSK)))
        {
            /* Construct a RSN IE for association request frame. */
            RSN_IE(pucIeStartAddr)->ucElemId = ELEM_ID_RSN;
            RSN_IE(pucIeStartAddr)->ucLength = ELEM_ID_RSN_LEN_FIXED;
            WLAN_SET_FIELD_16(&RSN_IE(pucIeStartAddr)->u2Version, 1); // Version
            WLAN_SET_FIELD_32(&RSN_IE(pucIeStartAddr)->u4GroupKeyCipherSuite,
                prCurrentBss->u4RsnSelectedGroupCipher); // Group key suite
            cp = (PUCHAR) &RSN_IE(pucIeStartAddr)->aucPairwiseKeyCipherSuite1[0];
            WLAN_SET_FIELD_16(&RSN_IE(pucIeStartAddr)->u2PairwiseKeyCipherSuiteCount, 1);
            WLAN_SET_FIELD_32(cp, prCurrentBss->u4RsnSelectedPairwiseCipher);
            cp += 4;
            WLAN_SET_FIELD_16(cp, 1); // AKM suite count
            cp += 2;
            WLAN_SET_FIELD_32(cp, prCurrentBss->u4RsnSelectedAKMSuite); // AKM suite
            cp += 4;
            WLAN_SET_FIELD_16(cp, 0); // Capabilities
            cp += 2;
            if (rsnSearchPmkidEntry(prAdapter, prCurrentBss->aucBSSID, &u4Entry)) {
                if (prAdapter->rSecInfo.arPmkidCache[u4Entry].fgPmkidExist) {
                    RSN_IE(pucIeStartAddr)->ucLength = 38;
                    WLAN_SET_FIELD_16(cp, 1); // PMKID count
                    cp += 2;
                    kalMemCopy(cp, (PVOID)prAdapter->rSecInfo.arPmkidCache[u4Entry].rBssidInfo.arPMKID, 
                        sizeof(PARAM_PMKID_VALUE));
                    ucExpendedLen = 40;
                    pucIeStartAddr += 40;
                }
                else {
                    WLAN_SET_FIELD_16(cp, 0); // PMKID count
                    ucExpendedLen = ELEM_ID_RSN_LEN_FIXED + 2;
                    pucIeStartAddr += ELEM_ID_RSN_LEN_FIXED +2;
                }
            }
            else {
                WLAN_SET_FIELD_16(cp, 0); // PMKID count
                ucExpendedLen = ELEM_ID_RSN_LEN_FIXED + 2;
                pucIeStartAddr += ELEM_ID_RSN_LEN_FIXED +2;
            }
        }
        else if (prCurrentBss->prIEWPA)
        {
            /* Construct a WPA IE for association request frame. */
            WPA_IE(pucIeStartAddr)->ucElemId = ELEM_ID_WPA;
            WPA_IE(pucIeStartAddr)->ucLength = ELEM_ID_WPA_LEN_FIXED;
            WPA_IE(pucIeStartAddr)->aucOui[0] = 0x00;
            WPA_IE(pucIeStartAddr)->aucOui[1] = 0x50;
            WPA_IE(pucIeStartAddr)->aucOui[2] = 0xF2;
            WPA_IE(pucIeStartAddr)->ucOuiType = VENDOR_OUI_TYPE_WPA;
            WLAN_SET_FIELD_16(&WPA_IE(pucIeStartAddr)->u2Version, 1);

            WLAN_SET_FIELD_32(&WPA_IE(pucIeStartAddr)->u4GroupKeyCipherSuite,
                prCurrentBss->u4RsnSelectedGroupCipher);

            cp = (PUCHAR) &WPA_IE(pucIeStartAddr)->aucPairwiseKeyCipherSuite1[0];

            WLAN_SET_FIELD_16(&WPA_IE(pucIeStartAddr)->u2PairwiseKeyCipherSuiteCount, 1);
            WLAN_SET_FIELD_32(cp, prCurrentBss->u4RsnSelectedPairwiseCipher);
            cp += 4;

            WLAN_SET_FIELD_16(cp, 1);
            cp += 2;
            WLAN_SET_FIELD_32(cp, prCurrentBss->u4RsnSelectedAKMSuite);
            cp += 4;

            WPA_IE(pucIeStartAddr)->ucLength = ELEM_ID_WPA_LEN_FIXED;
            ucExpendedLen = ELEM_ID_WPA_LEN_FIXED + 2;
            pucIeStartAddr += ELEM_ID_WPA_LEN_FIXED + 2;
        }
    }
    return ucExpendedLen;
} /* rsnGenerateWPARSNIE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
rsnGenerateAuthReqEvent (
    IN P_ADAPTER_T       prAdapter,
    IN BOOLEAN           fgFlags
    )
{
    P_PARAM_AUTH_EVENT_T prAuthEvent;

    DEBUGFUNC("rsnGenerateAuthReqEvent");

    ASSERT(prAdapter);

    prAuthEvent = (P_PARAM_AUTH_EVENT_T)prAdapter->aucIndicationEventBuffer;

    /* Status type: Authentication Event */
    prAuthEvent->rStatus.eStatusType = ENUM_STATUS_TYPE_AUTHENTICATION;

    /* Authentication request */
    prAuthEvent->arRequest[0].u4Length = sizeof(PARAM_AUTH_REQUEST_T);
    kalMemCopy((PVOID)prAuthEvent->arRequest[0].arBssid, (PVOID)prAdapter->rBssInfo.aucBSSID, PARAM_MAC_ADDR_LEN);

    if (fgFlags == TRUE)
        prAuthEvent->arRequest[0].u4Flags = PARAM_AUTH_REQUEST_GROUP_ERROR;
    else
        prAuthEvent->arRequest[0].u4Flags = PARAM_AUTH_REQUEST_PAIRWISE_ERROR;

    kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
        WLAN_STATUS_MEDIA_SPECIFIC_INDICATION,
        (PVOID)prAuthEvent,
        sizeof(PARAM_STATUS_INDICATION_T) + sizeof(PARAM_AUTH_REQUEST_T));

} /* rsnGenerateAuthReqEvent */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnTxProcessMSDU (
    IN P_ADAPTER_T      prAdapter,
    IN P_PACKET_INFO_T  prPacketInfo
    )
{
    P_WLAN_ENTRY_CTRL_T parWlanCtrl = NULL;

    ASSERT(prAdapter);
    ASSERT(prPacketInfo);

    if (prAdapter->rSecInfo.fgBlockOnlyAPTraffic) {
        if (prAdapter->rSecInfo.fgBlockTxTraffic &&
            (prPacketInfo->fgIs1x == FALSE)) {

            if (prAdapter->fgBypassPortCtrlForRoaming) {
                DBGLOG(RSN, TRACE, ("Let non-1x packet bypass the Port Control!\n"));
            }
            else {
                DBGLOG(RSN, TRACE, ("Drop Tx packet due Port Control!\n"));
                return FALSE;
            }
        }
    }
    else {
        UINT_8 ucEntryIndex = WLAN_TABLE_SET_ALL;

        ASSERT(prPacketInfo->pucDestAddr); 
        
        if (nicPrivacyMatchForEntry(prAdapter, prPacketInfo->pucDestAddr, 0, &ucEntryIndex)) {
            if (ucEntryIndex < WLAN_TABLE_SIZE) {
                parWlanCtrl = &prAdapter->arWlanCtrl[ucEntryIndex];
                ASSERT(parWlanCtrl);
                if (parWlanCtrl->rCtrl.fgT1X &&
                    (prPacketInfo->fgIs1x == FALSE)) {
                    DBGLOG(RSN, TRACE, ("Drop Tx packet due Port Control!\n"));
                    return FALSE;
                }
            }
            else {
                ASSERT(0);
            }
        }
        else {
        }
    }

    return TRUE;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnRxProcessMSDU (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T  prSWRfb
    )
{

    ASSERT(prAdapter);
    ASSERT(prSWRfb);

    #if CFG_WORKAROUND_HEC_5988
    if (prSWRfb->fgIsDataFrame &&
        !RX_STATUS_IS_1X(prSWRfb->prRxStatus->u2StatusFlag)) {
        PUINT_8 pucOffset_ethertype;
        pucOffset_ethertype = (PUINT_8)prSWRfb->pvHeader;
        if ((pucOffset_ethertype[12] == (UINT_8)(ETH_P_1X >> 8)) &&
            (pucOffset_ethertype[13] == (UINT_8)(ETH_P_1X))) {
            prSWRfb->prRxStatus->u2StatusFlag |= RX_STATUS_FLAG_1X;
        }
    }
    #endif

    #if SUPPORT_WAPI
    if (prAdapter->fgUseWapi && prSWRfb->fgIsDataFrame) {
        PUINT_8 pucOffset_ethertype;
        pucOffset_ethertype = (PUINT_8)prSWRfb->pvHeader;
        if ((pucOffset_ethertype[12] == (UINT_8)(ETH_WPI_1X >> 8)) &&
            (pucOffset_ethertype[13] == (UINT_8)(ETH_WPI_1X))) {
            prSWRfb->prRxStatus->u2StatusFlag |= RX_STATUS_FLAG_1X;
        }
        else {
            if (prAdapter->rSecInfo.fgBlockRxTraffic) {
                DBGLOG(WAPI, WARN, ("WPI Drop rcv data due port control !\r\n"));
                DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)prSWRfb->pvBody, 14);
                return FALSE;
            }
        }
    }
    #endif

     /* Drop received non-encrypted non-802.1x data packet for auth mode > wpa, 
        not handle open and shared key mode, issue shared key for only mgmt */
    if (prSWRfb->fgIsDataFrame &&
        (prAdapter->rConnSettings.eAuthMode >= AUTH_MODE_WPA) && 
        !RX_STATUS_IS_1X(prSWRfb->prRxStatus->u2StatusFlag)) {

        if (!RX_STATUS_IS_PROTECT(prSWRfb->prRxStatus)) {
            DBGLOG(RSN, WARN, ("Drop rcv non-encrypted data frame!\n"));
            return FALSE;
        }
    }

    if (prAdapter->rSecInfo.fgBlockOnlyAPTraffic) {
        if (prAdapter->rSecInfo.fgBlockRxTraffic &&
            !RX_STATUS_IS_1X(prSWRfb->prRxStatus->u2StatusFlag)) {
            DBGLOG(RSN, TRACE, ("Drop Rx packet due Port Control!\n"));
            return FALSE;
        }
    }
    else {
        UINT_8 ucEntryIndex;
        
        if (nicPrivacyMatchForEntry(prAdapter, prSWRfb->prRxStatus->aucTA, 0, &ucEntryIndex)) {
            if (ucEntryIndex < WLAN_TABLE_SIZE) {
                if (prAdapter->arWlanCtrl[ucEntryIndex].rCtrl.fgR1X && 
                    !RX_STATUS_IS_1X(prSWRfb->prRxStatus->u2StatusFlag)) {
                    DBGLOG(RSN, TRACE, ("Drop Rx packet due Port Control!\n"));
                    return FALSE;
                }
            }
            else {
                return TRUE;
            }
        }
    }
    
    return TRUE;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
rsnTkipHandleMICFailure (
    IN  P_ADAPTER_T         prAdapter,
    IN  BOOLEAN             fgErrorKeyType
    )
{
    UINT_32                 u4RsnaCurrentMICFailTime;

    DEBUGFUNC("rsnTkipHandleMICFailure");

    ASSERT(prAdapter);
    
    /* Record the MIC error occur time. */
    GET_CURRENT_SYSTIME(&u4RsnaCurrentMICFailTime);

    /* Generate authentication request event. */
    DBGLOG(RSN, LOUD, ("Generate TKIP MIC error event (type: 0%d)\n",
        fgErrorKeyType));

    /* If less than 60 seconds have passed since a previous TKIP MIC failure,
       disassociate from the AP and wait for 60 seconds before (re)associating
       with the same AP. */
    if (prAdapter->rSecInfo.u4RsnaLastMICFailTime != 0 && !CHECK_FOR_TIMEOUT(u4RsnaCurrentMICFailTime,
            prAdapter->rSecInfo.u4RsnaLastMICFailTime, SEC_TO_SYSTIME(TKIP_COUNTERMEASURE_SEC))) {
        /* If less than 60 seconds expired since last MIC error, we have to
           block traffic. */
        if (nicPrivacyInvalidKey(prAdapter, prAdapter->rBssInfo.aucBSSID) == FALSE) {
            DBGLOG(RSN, TRACE, ("Invalid key fail!!\n"));
        }

        DBGLOG(RSN, INFO, ("Start blocking traffic!\n"));
        rsnGenerateAuthReqEvent(prAdapter, fgErrorKeyType);

        secFsmRunEventStartCounterMeasure(prAdapter);
    }
    else {
        rsnGenerateAuthReqEvent(prAdapter, fgErrorKeyType);
        DBGLOG(RSN, WARN, ("First TKIP MIC error!\n"));
    }

    COPY_SYSTIME(prAdapter->rSecInfo.u4RsnaLastMICFailTime, u4RsnaCurrentMICFailTime);
}   /* rsnTkipHandleMICFailure */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
rsnSelectPmkidCandidateList (
    IN P_ADAPTER_T             prAdapter,
    IN P_BSS_DESC_T            prBssDesc
    )
{
    P_CONNECTION_SETTINGS_T    prConnSettings;
    P_BSS_INFO_T               prBssInfo;

    DEBUGFUNC("rsnSelectPmkidCandidateList");

    ASSERT(prAdapter);
    ASSERT(prBssDesc);

    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    ASSERT(prConnSettings);
    ASSERT(prBssDesc);

    /* Search a BSS with the same SSID from the given BSS description set. */
    DBGLOG(RSN, TRACE, ("Check scan result ["MACSTR"]\n",
        MAC2STR(prBssDesc->aucBSSID)));

    if (UNEQUAL_SSID(prBssDesc->aucSSID, prBssDesc->ucSSIDLen,
                   prConnSettings->aucSSID, prConnSettings->ucSSIDLen)) {
        DBGLOG(RSN, TRACE, ("-- SSID not matched\n"));
        return;
    }

    if ((prBssDesc->u2BSSBasicRateSet &
         ~(rPhyAttributes[prBssInfo->ePhyType].u2SupportedRateSet)) ||
        prBssDesc->fgIsUnknownBssBasicRate) {
        DBGLOG(RSN, TRACE, ("-- Rate set not matched\n"));
        return;
    }

    if (prBssDesc->u4RsnSelectedPairwiseCipher != prAdapter->rBssInfo.u4RsnSelectedPairwiseCipher ||
        prBssDesc->u4RsnSelectedGroupCipher != prAdapter->rBssInfo.u4RsnSelectedGroupCipher ||
        prBssDesc->u4RsnSelectedAKMSuite != prAdapter->rBssInfo.u4RsnSelectedAKMSuite) {
        DBGLOG(RSN, TRACE, ("-- Encrypt status not matched\n"));
        return;
    }

    rsnUpdatePmkidCandidateList(prAdapter, prBssDesc);

}   /* rsnSelectPmkidCandidateList */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
rsnUpdatePmkidCandidateList (
    IN P_ADAPTER_T             prAdapter,
    IN P_BSS_DESC_T            prBssDesc
    )
{
    UINT_32                    i;
    P_CONNECTION_SETTINGS_T    prConnSettings;
    
    DEBUGFUNC("rsnUpdatePmkidCandidateList");

    ASSERT(prAdapter);
    ASSERT(prBssDesc);

    prConnSettings = &prAdapter->rConnSettings;
    ASSERT(prConnSettings);
    
    if (UNEQUAL_SSID(prBssDesc->aucSSID, prBssDesc->ucSSIDLen,
                   prConnSettings->aucSSID, prConnSettings->ucSSIDLen)) {
        DBGLOG(RSN, TRACE, ("-- SSID not matched\n"));
        return;
    }

    for (i = 0; i < CFG_MAX_PMKID_CACHE; i++) {
        if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID, prAdapter->rSecInfo.arPmkidCandicate[i].arBssid))
            return;
    }

    /* If the number of selected BSSID exceed MAX_NUM_PMKID_CACHE(16),
       then we only store MAX_NUM_PMKID_CACHE(16) in PMKID cache */
    if ((prAdapter->rSecInfo.u4PmkidCandicateCount + 1)  > CFG_MAX_PMKID_CACHE) {
        prAdapter->rSecInfo.u4PmkidCandicateCount --;
    }

    i = prAdapter->rSecInfo.u4PmkidCandicateCount;

    COPY_MAC_ADDR((PVOID)prAdapter->rSecInfo.arPmkidCandicate[i].arBssid, 
        (PVOID)prBssDesc->aucBSSID);

    if (prBssDesc->u2RsnCap & MASK_RSNIE_CAP_PREAUTH) {
        prAdapter->rSecInfo.arPmkidCandicate[i].u4PreAuthFlags = 1;
        DBGLOG(RSN, TRACE, ("Add " MACSTR " with pre-auth to candidate list\n",
            MAC2STR(prAdapter->rSecInfo.arPmkidCandicate[i].arBssid)));
    }
    else {
        prAdapter->rSecInfo.arPmkidCandicate[i].u4PreAuthFlags = 0;
        DBGLOG(RSN, TRACE, ("Add " MACSTR " without pre-auth to candidate list\n",
            MAC2STR(prAdapter->rSecInfo.arPmkidCandicate[i].arBssid)));
    }

    prAdapter->rSecInfo.u4PmkidCandicateCount ++;

}   /* rsnUpdatePmkidCandidateList */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnSearchPmkidEntry (
    IN P_ADAPTER_T             prAdapter,
    IN PUINT_8                 pucBssid,
    OUT PUINT_32               pu4EntryIndex
    )
{
    UINT_32 i;

    DEBUGFUNC("rsnSearchPmkidEntry");

    ASSERT(prAdapter);
    ASSERT(pucBssid);
    ASSERT(pu4EntryIndex);

    if (prAdapter->rSecInfo.u4PmkidCacheCount > CFG_MAX_PMKID_CACHE) {
        return FALSE;
    }
    
    ASSERT(prAdapter->rSecInfo.u4PmkidCacheCount <= CFG_MAX_PMKID_CACHE);
 
    /* Search for desired BSSID */
    for (i = 0; i < prAdapter->rSecInfo.u4PmkidCacheCount; i++) {
        if (!kalMemCmp(prAdapter->rSecInfo.arPmkidCache[i].rBssidInfo.arBSSID, pucBssid,
            sizeof(PARAM_MAC_ADDRESS))) {
            break;
        }
    }

    /* If desired BSSID is found, then set the PMKID */
    if (i < prAdapter->rSecInfo.u4PmkidCacheCount) {
        *pu4EntryIndex = i;

        return TRUE;
    }

    return FALSE;
}   /* rsnSearchPmkidEntry */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
rsnCheckPmkidCandicate (
    IN P_ADAPTER_T             prAdapter
   )
{
    UINT_32                    i; // Index for PMKID candicate
    UINT_32                    j; // Indix for PMKID cache
    BOOLEAN                    status = FALSE;

    DEBUGFUNC("rsnCheckPmkidCandicate");

    ASSERT(prAdapter);

    /* Check for each candicate */
    for (i = 0; i < prAdapter->rSecInfo.u4PmkidCandicateCount; i++) {
        for (j = 0; j < prAdapter->rSecInfo.u4PmkidCacheCount; j++) {
            if (!kalMemCmp(prAdapter->rSecInfo.arPmkidCache[j].rBssidInfo.arBSSID,
                    prAdapter->rSecInfo.arPmkidCandicate[i].arBssid,
                    PARAM_MAC_ADDR_LEN)) {
                break;
            }
        }

        /* No entry found in PMKID cache for the candicate, add new one */
        if (j == prAdapter->rSecInfo.u4PmkidCacheCount && prAdapter->rSecInfo.u4PmkidCacheCount < CFG_MAX_PMKID_CACHE) {
            kalMemCopy((PVOID)prAdapter->rSecInfo.arPmkidCache[prAdapter->rSecInfo.u4PmkidCacheCount].rBssidInfo.arBSSID,
                (PVOID)prAdapter->rSecInfo.arPmkidCandicate[i].arBssid,
                PARAM_MAC_ADDR_LEN);
            prAdapter->rSecInfo.arPmkidCache[prAdapter->rSecInfo.u4PmkidCacheCount].fgPmkidExist = FALSE;
            prAdapter->rSecInfo.u4PmkidCacheCount++;

            status = TRUE;
        }
    }

    return status;
} /* rsnCheckPmkidCandicate */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
rsnCheckPmkidCache (
    IN P_ADAPTER_T          prAdapter,
    IN P_BSS_DESC_T         prBss
    )
{
    P_SEC_INFO_T            prSecInfo;
    DEBUGFUNC("rsnCheckPmkidCandicate");

    ASSERT(prAdapter);
    ASSERT(prBss);
    
    prSecInfo = &prAdapter->rSecInfo;

    ASSERT(prSecInfo);
    
    if ((prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) &&
       (prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA2)) {
        rsnSelectPmkidCandidateList(prAdapter, prBss);

        if (prSecInfo->u4PmkidCandicateCount && 
            (prSecInfo->fgIndicatePMKID == FALSE)) {
            /* Set indication flag of PMKID to TRUE, and then connHandleNetworkConnection()
               will indicate this later */
            prSecInfo->fgIndicatePMKID = rsnCheckPmkidCandicate(prAdapter);
            if (prSecInfo->fgIndicatePMKID) {
                if (!prSecInfo->arPmkidCache[prAdapter->rSecInfo.u4PmkidCacheCount].fgPmkidExist){
                    DBGLOG(RSN, WARN, ("Prepare a timer to indicate candidate "MACSTR"\n",
                        MAC2STR(prAdapter->rSecInfo.arPmkidCache[prAdapter->rSecInfo.u4PmkidCacheCount].rBssidInfo.arBSSID)));
                    prAdapter->rSecInfo.fgIndicatePMKID = TRUE;
                    ARB_CANCEL_TIMER(prAdapter, prSecInfo->rPreauthenticationTimer);
                    ARB_SET_TIMER(prAdapter,
                            prSecInfo->rPreauthenticationTimer,
                            SEC_TO_MSEC(WAIT_TIME_IND_PMKID_CANDICATE_SEC));
                }
            }
        }
    }
}
    

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
rsnGeneratePmkidIndication (
    IN P_ADAPTER_T                 prAdapter,
    IN UINT_32                     u4Flags
    )
{
    P_PARAM_STATUS_INDICATION_T    prStatusEvent;
    P_PARAM_PMKID_CANDIDATE_LIST_T prPmkidEvent;
    UINT_8                         i, j = 0, count = 0;
    UINT_32                        u4LenOfUsedBuffer;
    P_SEC_INFO_T                   prSecInfo = NULL;

    DEBUGFUNC("rsnGeneratePmkidIndication");

    ASSERT(prAdapter);

    prStatusEvent =
        (P_PARAM_STATUS_INDICATION_T)prAdapter->aucIndicationEventBuffer;

    /* Status type: PMKID Candidatelist Event */
    prStatusEvent->eStatusType = ENUM_STATUS_TYPE_CANDIDATE_LIST;
    ASSERT(prStatusEvent);

    prPmkidEvent = (P_PARAM_PMKID_CANDIDATE_LIST_T)(&prStatusEvent->eStatusType + 1);
    ASSERT(prPmkidEvent);

    prSecInfo = &prAdapter->rSecInfo;
    ASSERT(prSecInfo);

    for (i = 0; i < prSecInfo->u4PmkidCandicateCount; i++) {
        for (j = 0; j < prSecInfo->u4PmkidCacheCount; j++) {
            if (EQUAL_MAC_ADDR( prSecInfo->arPmkidCache[j].rBssidInfo.arBSSID, 
                prSecInfo->arPmkidCandicate[i].arBssid) &&
                (prSecInfo->arPmkidCache[j].fgPmkidExist == TRUE)){
                break;
            }
        }
        if (count >= CFG_MAX_PMKID_CACHE) {
            break;
        }

        if (j == prSecInfo->u4PmkidCacheCount) {
            kalMemCopy((PVOID)prPmkidEvent->arCandidateList[count].arBSSID,
                (PVOID)prSecInfo->arPmkidCandicate[i].arBssid,
                PARAM_MAC_ADDR_LEN);
            prPmkidEvent->arCandidateList[count].u4Flags =
                prSecInfo->arPmkidCandicate[i].u4PreAuthFlags;
            count++;
        }
    }

    /* PMKID Candidate List */
    prPmkidEvent->u4Version = 1;
    prPmkidEvent->u4NumCandidates = count;

    u4LenOfUsedBuffer = sizeof(ENUM_STATUS_TYPE_T) + (2 * sizeof(UINT_32)) +
        (count * sizeof(PARAM_PMKID_CANDIDATE_LIST_T));

    kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
        WLAN_STATUS_MEDIA_SPECIFIC_INDICATION,
        (PVOID) prAdapter->aucIndicationEventBuffer,
        u4LenOfUsedBuffer);

}   /* rsnGeneratePmkidIndication */

