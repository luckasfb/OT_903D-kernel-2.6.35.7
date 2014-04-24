
#ifdef	MTK_WAPI_SUPPORT
#include "precomp.h"









/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wapiParseWapiIE (
    IN  P_WAPI_INFO_ELEM_T prInfoElem,
    OUT P_WAPI_INFO_T      prWapiInfo
    )
{
    UINT_32                i;
    INT_32                 u4RemainWapiIeLen;
    UINT_16                u2Version;
    UINT_16                u2Cap = 0;
    UINT_32                u4GroupSuite = WAPI_CIPHER_SUITE_WPI;
    UINT_16                u2PairSuiteCount = 0;
    UINT_16                u2AuthSuiteCount = 0;
    PUCHAR                 pucPairSuite = NULL;
    PUCHAR                 pucAuthSuite = NULL;
    PUCHAR                 cp;

    DEBUGFUNC("wapiParseWapiIE");

    ASSERT(prInfoElem);
    ASSERT(prWapiInfo);

    /* Verify the length of the WAPI IE. */
    if (prInfoElem->ucLength < 6) {
        DBGLOG(WAPI, WARN,("WAPI IE length too short (length=%d)\n", prInfoElem->ucLength));
        return FALSE;
    }

    /* Check WAPI version: currently, we only support version 1. */
    WLAN_GET_FIELD_16(&prInfoElem->u2Version, &u2Version);
    if (u2Version != 1) {
        DBGLOG(WAPI, WARN, ("Unsupported WAPI IE version: %d\n", u2Version));
        return FALSE;
    }
    
    cp = (PUCHAR) &prInfoElem->u2AuthKeyMgtSuiteCount;
    u4RemainWapiIeLen = (INT_32) prInfoElem->ucLength - 2;

    do {
        if (u4RemainWapiIeLen == 0) {
            break;
        }

        /* 
           AuthCount    : 2
           AuthSuite    : 4 * authSuiteCount
           PairwiseCount: 2
           PairwiseSuite: 4 * pairSuiteCount
           GroupSuite   : 4
           Cap          : 2 */

        /* Parse the Authentication and Key Management Cipher Suite Count
           field. */
        if (u4RemainWapiIeLen < 2) {
            DBGLOG(WAPI, WARN,(
                "Fail to parse WAPI IE in auth & key mgt suite count (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2AuthSuiteCount);
        cp += 2;
        u4RemainWapiIeLen -= 2;

        /* Parse the Authentication and Key Management Cipher Suite List
           field. */
        i = (UINT_32) u2AuthSuiteCount * 4;
        if (u4RemainWapiIeLen < (INT_32) i) {
            DBGLOG(WAPI, WARN, (
                "Fail to parse WAPI IE in auth & key mgt suite list (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        pucAuthSuite = cp;

        cp += i;
        u4RemainWapiIeLen -= (INT_32) i;

        if (u4RemainWapiIeLen == 0) {
            break;
        }

        /* Parse the Pairwise Key Cipher Suite Count field. */
        if (u4RemainWapiIeLen < 2) {
            DBGLOG(WAPI, WARN,(
                "Fail to parse WAPI IE in pairwise cipher suite count (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2PairSuiteCount);
        cp += 2;
        u4RemainWapiIeLen -= 2;

        /* Parse the Pairwise Key Cipher Suite List field. */
        i = (UINT_32) u2PairSuiteCount * 4;
        if (u4RemainWapiIeLen < (INT_32) i) {
            DBGLOG(WAPI, WARN,(
                "Fail to parse WAPI IE in pairwise cipher suite list (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        pucPairSuite = cp;

        cp += i;
        u4RemainWapiIeLen -= (INT_32) i;

        /* Parse the Group Key Cipher Suite field. */
        if (u4RemainWapiIeLen < 4) {
            DBGLOG(WAPI, WARN,(
                "Fail to parse WAPI IE in group cipher suite (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_32(cp, &u4GroupSuite);
        cp += 4;
        u4RemainWapiIeLen -= 4;

        /* Parse the WAPI u2Capabilities field. */
        if (u4RemainWapiIeLen < 2) {
            DBGLOG(WAPI, WARN, ("Fail to parse WAPI IE in WAPI capabilities (IE len: %d)\n",
                prInfoElem->ucLength));
            return FALSE;
        }

        WLAN_GET_FIELD_16(cp, &u2Cap);
        u4RemainWapiIeLen -= 2;

        /* Todo:: BKID support */
    } while (FALSE);

    /* Save the WAPI information for the BSS. */

    prWapiInfo->ucElemId = ELEM_ID_WAPI;

    prWapiInfo->u2Version = u2Version;

    prWapiInfo->u4GroupKeyCipherSuite = u4GroupSuite;

    DBGLOG(WAPI, LOUD,
        ("WAPI: version %d, group key cipher suite %02x-%02x-%02x-%02x\n",
        u2Version, (UCHAR) (u4GroupSuite & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 8) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 16) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 24) & 0x000000FF)));

    if (pucPairSuite) {
        /* The information about the pairwise key cipher suites is present. */
        if (u2PairSuiteCount > MAX_NUM_SUPPORTED_CIPHER_SUITES) {
            u2PairSuiteCount = MAX_NUM_SUPPORTED_CIPHER_SUITES;
        }

        prWapiInfo->u4PairwiseKeyCipherSuiteCount = (UINT_32) u2PairSuiteCount;

        for (i = 0; i < (UINT_32) u2PairSuiteCount; i++) {
            WLAN_GET_FIELD_32(pucPairSuite,
                              &prWapiInfo->au4PairwiseKeyCipherSuite[i]);
            pucPairSuite += 4;

            DBGLOG(WAPI, LOUD,
                ("WAPI: pairwise key cipher suite [%d]: %02x-%02x-%02x-%02x\n",
                (UINT_8)i, (UCHAR) (prWapiInfo->au4PairwiseKeyCipherSuite[i] & 0x000000FF),
                (UCHAR) ((prWapiInfo->au4PairwiseKeyCipherSuite[i] >> 8) & 0x000000FF),
                (UCHAR) ((prWapiInfo->au4PairwiseKeyCipherSuite[i] >> 16) & 0x000000FF),
                (UCHAR) ((prWapiInfo->au4PairwiseKeyCipherSuite[i] >> 24) & 0x000000FF)));
        }
    }
    else {
        /* The information about the pairwise key cipher suites is not present.
           Use the default chipher suite for WAPI: WPI. */
        prWapiInfo->u4PairwiseKeyCipherSuiteCount = 1;
        prWapiInfo->au4PairwiseKeyCipherSuite[0] = WAPI_CIPHER_SUITE_WPI;

        DBGLOG(WAPI, LOUD,
            ("WAPI: pairwise key cipher suite: %02x-%02x-%02x-%02x (default)\n",
            (UCHAR) (prWapiInfo->au4PairwiseKeyCipherSuite[0] & 0x000000FF),
            (UCHAR) ((prWapiInfo->au4PairwiseKeyCipherSuite[0] >> 8) & 0x000000FF),
            (UCHAR) ((prWapiInfo->au4PairwiseKeyCipherSuite[0] >> 16) & 0x000000FF),
            (UCHAR) ((prWapiInfo->au4PairwiseKeyCipherSuite[0] >> 24) & 0x000000FF)));
    }

    if (pucAuthSuite) {
        /* The information about the authentication and key management suites
           is present. */
        if (u2AuthSuiteCount > MAX_NUM_SUPPORTED_AKM_SUITES) {
            u2AuthSuiteCount = MAX_NUM_SUPPORTED_AKM_SUITES;
        }

        prWapiInfo->u4AuthKeyMgtSuiteCount = (UINT_32) u2AuthSuiteCount;

        for (i = 0; i < (UINT_32) u2AuthSuiteCount; i++) {
            WLAN_GET_FIELD_32(pucAuthSuite, &prWapiInfo->au4AuthKeyMgtSuite[i]);
            pucAuthSuite += 4;

            DBGLOG(WAPI, LOUD,
                ("WAPI: AKM suite [%d]: %02x-%02x-%02x-%02x\n",
                (UINT_8)i, (UCHAR) (prWapiInfo->au4AuthKeyMgtSuite[i] & 0x000000FF),
                (UCHAR) ((prWapiInfo->au4AuthKeyMgtSuite[i] >> 8) & 0x000000FF),
                (UCHAR) ((prWapiInfo->au4AuthKeyMgtSuite[i] >> 16) & 0x000000FF),
                (UCHAR) ((prWapiInfo->au4AuthKeyMgtSuite[i] >> 24) & 0x000000FF)));
        }
    }
    else {
        /* The information about the authentication and key management suites
           is not present. Use the default AKM suite for WAPI. */
        prWapiInfo->u4AuthKeyMgtSuiteCount = 1;
        prWapiInfo->au4AuthKeyMgtSuite[0] = WAPI_AKM_SUITE_802_1X;

        DBGLOG(WAPI, LOUD,
            ("WAPI: AKM suite: %02x-%02x-%02x-%02x (default)\n",
            (UCHAR) (prWapiInfo->au4AuthKeyMgtSuite[0] & 0x000000FF),
            (UCHAR) ((prWapiInfo->au4AuthKeyMgtSuite[0] >> 8) & 0x000000FF),
            (UCHAR) ((prWapiInfo->au4AuthKeyMgtSuite[0] >> 16) & 0x000000FF),
            (UCHAR) ((prWapiInfo->au4AuthKeyMgtSuite[0] >> 24) & 0x000000FF)));
    }

    prWapiInfo->u2WapiCap = u2Cap;
    DBGLOG(WAPI, LOUD, ("WAPI: cap: 0x%04x\n", prWapiInfo->u2WapiCap));

    return TRUE;
}   /* wapiParseWapiIE */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wapiPerformPolicySelection (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_BSS_DESC_T        prBss
    )
{
    UINT_32                 i;
    UINT_32                 u4PairwiseCipher = 0;
    UINT_32                 u4GroupCipher = 0;
    UINT_32                 u4AkmSuite = 0;
    WAPI_INFO_T             rTmpWapiInfo;
    P_WAPI_INFO_T           prBssWapiInfo;

    DEBUGFUNC("wapiPerformPolicySelection");

    ASSERT(prAdapter);
    ASSERT(prBss);
    
    //prBss->u4RsnSelectedPairwiseCipher = 0;
    //prBss->u4RsnSelectedGroupCipher = 0;
    //prBss->u4RsnSelectedAKMSuite = 0;
    //prBss->ucEncLevel = 0;
 
	/* Notice!!!! WAPI AP not set the privacy bit for WAI and WAI-PSK at WZC configuration mode */

    if (prBss->prIEWAPI != NULL) {
		DBGLOG(WAPI, INFO, ("wapiParseWapiIE\n"));
        if (!wapiParseWapiIE((P_WAPI_INFO_ELEM_T)prBss->prIEWAPI, &rTmpWapiInfo)) {
            DBGLOG(WAPI, INFO, ("-- Policy selection fail for WAPI IE .\n"));
            return FALSE;
        }
    }
    else {
        if (prAdapter->fgWapiMode == FALSE) {
            DBGLOG(WAPI, TRACE,("-- No Protected BSS\n"));
            return TRUE;
        }
		else {
	        DBGLOG(WAPI, INFO, ("WAPI Information Element does not exist.\n"));
	        return FALSE;
		}
    }

    prBssWapiInfo = &rTmpWapiInfo;

    /* Select pairwise/group ciphers */
    for (i = 0; i < prBssWapiInfo->u4PairwiseKeyCipherSuiteCount; i++) {
        if (prBssWapiInfo->au4PairwiseKeyCipherSuite[i] == 
		  	prAdapter->rConnSettings.u4WapiSelectedPairwiseCipher) {
                u4PairwiseCipher = prBssWapiInfo->au4PairwiseKeyCipherSuite[i];
        }
    }
    if (prBssWapiInfo->u4GroupKeyCipherSuite == 
		    prAdapter->rConnSettings.u4WapiSelectedGroupCipher)
        u4GroupCipher = prBssWapiInfo->u4GroupKeyCipherSuite;
             
    /* Exception handler */
    /* If we cannot find proper pairwise and group cipher suites to join the
       BSS, do not check the supported AKM suites. */
    if (u4PairwiseCipher == 0 || u4GroupCipher == 0) {
        DBGLOG(WAPI, INFO,
            ("Failed to select pairwise/group cipher (0x%08lx/0x%08lx)\n",
            u4PairwiseCipher, u4GroupCipher));
        return FALSE;
    }

    /* Select AKM */
    /* If the driver cannot support any authentication suites advertised in
       the given BSS, we fail to perform RSNA policy selection. */
    /* Attempt to find any overlapping supported AKM suite. */
    for (i = 0; i < prBssWapiInfo->u4AuthKeyMgtSuiteCount; i++) {
        if (prBssWapiInfo->au4AuthKeyMgtSuite[i] == prAdapter->rConnSettings.u4WapiSelectedAKMSuite) {
            u4AkmSuite = prBssWapiInfo->au4AuthKeyMgtSuite[i];
            break;
        }
    }

    if (u4AkmSuite == 0) {
        DBGLOG(WAPI, INFO, ("Cannot support any AKM suites\n"));
        return FALSE;
    }

    DBGLOG(WAPI, TRACE,
        ("Selected pairwise/group cipher: %02x-%02x-%02x-%02x/%02x-%02x-%02x-%02x\n",
        (UINT_8) (u4PairwiseCipher & 0x000000FF),
        (UINT_8) ((u4PairwiseCipher >> 8) & 0x000000FF),
        (UINT_8) ((u4PairwiseCipher >> 16) & 0x000000FF),
        (UINT_8) ((u4PairwiseCipher >> 24) & 0x000000FF),
        (UINT_8) (u4GroupCipher & 0x000000FF),
        (UINT_8) ((u4GroupCipher >> 8) & 0x000000FF),
        (UINT_8) ((u4GroupCipher >> 16) & 0x000000FF),
        (UINT_8) ((u4GroupCipher >> 24) & 0x000000FF)));

    DBGLOG(WAPI, TRACE, ("Selected AKM suite: %02x-%02x-%02x-%02x\n",
        (UINT_8) (u4AkmSuite & 0x000000FF),
        (UINT_8) ((u4AkmSuite >> 8) & 0x000000FF),
        (UINT_8) ((u4AkmSuite >> 16) & 0x000000FF),
        (UINT_8) ((u4AkmSuite >> 24) & 0x000000FF)));

    //prBss->u4RsnSelectedPairwiseCipher = u4PairwiseCipher;
    //prBss->u4RsnSelectedGroupCipher = u4GroupCipher;
    //prBss->u4RsnSelectedAKMSuite = u4AkmSuite;

    return TRUE;
}  /* rsnPerformPolicySelection */

#endif
