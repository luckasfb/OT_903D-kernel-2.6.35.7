



#include "precomp.h"


#if CFG_SUPPORT_802_11D
/*! \brief used for selecting domain */
typedef struct _VALID_DOMAIN_T {
    UINT_16 countryCode;    /*!< 2-byte country code */
    UINT_8  count;  /*!< number of scan results announce this country code */
    UINT_8  index;  /*!< the index of first scan result carry this country code */
} VALID_DOMAIN_T, P_VALID_DOMAIN_T;
#endif





#if CFG_SUPPORT_802_11D
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
domainParseCountryInfoElem (
    IN P_IE_COUNTRY_T pCountryIE,
    OUT P_DOMAIN_INFO_ENTRY pDomainInfo
    )
{
    UINT_32 i/*, j*/;
    UINT_32 u4TripletCount;
    PUINT_8 pucTriplet;
    P_DOMAIN_SUBBAND_INFO pSubbandInfo;
    P_COUNTRY_INFO_TRIPLET_T pCountryInfoTriplet;
    P_COUNTRY_INFO_SUBBAND_TRIPLET_T pSubbandTriplet;
    DOMAIN_INFO_ENTRY localDomainInfo;

    DEBUGFUNC("domainParseCountryInfoElem");

    ASSERT(pDomainInfo);
    ASSERT(pCountryIE);

    /* Verify the length of the Country Information element. */
    if (pCountryIE->ucLength < ELEM_MIN_LEN_COUNTRY_INFO) {
        DBGLOG(MGT, LOUD,
               ("Invalid Country Information length %d\n",
               pCountryIE->ucLength));
        return FALSE;
    }

    kalMemZero(&localDomainInfo, sizeof(DOMAIN_INFO_ENTRY));
    localDomainInfo.u2CountryCode =
        ((UINT_16)pCountryIE->aucCountryStr[0] << 8) |
        (UINT_16)pCountryIE->aucCountryStr[1];

    pucTriplet = (PUINT_8)&pCountryIE->arSubbandTriplet[0];
//    j = 0;
    /* Minus 1 for 3-octet country string */
    u4TripletCount = (UINT_32)(pCountryIE->ucLength /
        ELEM_ID_COUNTRY_INFO_SUBBAND_TRIPLET_LEN_FIXED) - 1;
    DBGLOG(MGT, LOUD, ("u4TripletCount %ld\n", u4TripletCount));

    /* Note: Only process for 1 subband for supporting 2.4GHz only */
    u4TripletCount = 1;
    pSubbandInfo = &localDomainInfo.rBand24;

    for (i = 0; i < u4TripletCount; i++) {
        pCountryInfoTriplet = (P_COUNTRY_INFO_TRIPLET_T)pucTriplet;
        if (pCountryInfoTriplet->ucParam1 >= 201) {
            /* This is a Regulatory Triplet. TODO. */
        }
        else {
            pSubbandTriplet = (P_COUNTRY_INFO_SUBBAND_TRIPLET_T)pucTriplet;

            pSubbandInfo->ucFirstChannelNum = pSubbandTriplet->ucFirstChnlNum;
            pSubbandInfo->ucNumChannels     = pSubbandTriplet->ucNumOfChnl;
            pSubbandInfo->cMaxTxPowerLevel  = pSubbandTriplet->cMaxTxPwrLv;
        }

        /* For compiler packing issue, iterate on PUINT_8 pucTriplet rather than
        ** struct *P_COUNTRY_INFO_TRIPLET_T.
        */
        pucTriplet += ELEM_ID_COUNTRY_INFO_TRIPLET_LEN_FIXED;
    }

    kalMemCopy(pDomainInfo, &localDomainInfo, sizeof(DOMAIN_INFO_ENTRY));
    return TRUE;
} /* domainParseCountryInfoElem */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
domainConstructCountryInfoElem (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_8 pucInfoElem,
    OUT PUINT_8 pucInfoElemLen
    )
{
    P_BSS_INFO_T prBssInfo;
    UINT_8 ucLen;
    P_DOMAIN_SUBBAND_INFO pSubbandInfo;
//    P_COUNTRY_INFO_SUBBAND_TRIPLET_T pSubbandTriplet;
//    UINT_32 i;

    DEBUGFUNC("domainConstructCountryInfoElem");

    ASSERT(prAdapter);
    ASSERT(pucInfoElem);
    ASSERT(pucInfoElemLen);

    prBssInfo = &prAdapter->rBssInfo;

    COUNTRY_IE(pucInfoElem)->ucId = ELEM_ID_COUNTRY_INFO;

    /* Fill the Country String field. */
    COUNTRY_IE(pucInfoElem)->aucCountryStr[0] =
        (UINT_8)(prBssInfo->rDomainInfo.u2CountryCode >> 8);
    COUNTRY_IE(pucInfoElem)->aucCountryStr[1] =
        (UINT_8)(prBssInfo->rDomainInfo.u2CountryCode & 0x00FF);
    COUNTRY_IE(pucInfoElem)->aucCountryStr[2] = ' ';
    ucLen = 3;

    /* Fill the First Channel Number, Number of Channels, and Maximum Transmit
       Power Level fields for each subband. */
#if 0
    for (i = 0; i < 2; ++i) {
        pSubbandInfo = &prBssInfo->rDomainInfo.rBand24[i];

        if (pSubbandInfo->ucFirstChannelNum && pSubbandInfo->ucNumChannels) {
            COUNTRY_IE(pucInfoElem)->arSubbandTriplet[i].ucFirstChnlNumber =
                pSubbandInfo->ucFirstChannelNum;
            COUNTRY_IE(pucInfoElem)->arSubbandTriplet[i].ucNumOfChnls =
                pSubbandInfo->ucNumChannels;
                // NOTE: type not match (sign -> un-sign)
            COUNTRY_IE(pucInfoElem)->arSubbandTriplet[i].ucMaxTxPowerLevel =
                pSubbandInfo->cMaxTxPowerLevel;

            ucLen += ELEM_ID_COUNTRY_INFO_SUBBAND_TRIPLET_LEN_FIXED;
        }
    }
#else
        pSubbandInfo = &prBssInfo->rDomainInfo.rBand24;

        if (pSubbandInfo->ucFirstChannelNum && pSubbandInfo->ucNumChannels) {
            COUNTRY_IE(pucInfoElem)->arSubbandTriplet[0].ucFirstChnlNum =
                pSubbandInfo->ucFirstChannelNum;
            COUNTRY_IE(pucInfoElem)->arSubbandTriplet[0].ucNumOfChnl =
                pSubbandInfo->ucNumChannels;
            // NOTE: type not match (sign -> un-sign)
            COUNTRY_IE(pucInfoElem)->arSubbandTriplet[0].cMaxTxPwrLv =
                pSubbandInfo->cMaxTxPowerLevel;

            ucLen += ELEM_ID_COUNTRY_INFO_SUBBAND_TRIPLET_LEN_FIXED;
        }
#endif

    /* Fill the pad, if needed. P_IE_COUNTRY_T */
    if (ucLen & 1) {
#if 1   /* Fix klocwork array boundary checking issue */
        pucInfoElem[OFFSET_OF(IE_COUNTRY_T, aucCountryStr) + ucLen] = 0;
#else
        *(PUINT_8)(&COUNTRY_IE(pucInfoElem)->aucCountryStr[0] + ucLen) = 0;
#endif
        ucLen += 1;
    }

    /* Fill the Length field. */
    COUNTRY_IE(pucInfoElem)->ucLength = ucLen;

    *pucInfoElemLen = ucLen + 2;
} /* domainConstructCountryInfoElem */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
domainGetDomainInfoByScanResult (
    IN P_ADAPTER_T prAdapter,
    OUT P_DOMAIN_INFO_ENTRY pDomainInfo
    )
{
    DOMAIN_INFO_ENTRY   rDomainInfo;
    P_BSS_DESC_T prBssDesc = NULL;
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    BOOLEAN fgIsDomainValid = FALSE;
    BOOLEAN fgIsChannelValid = FALSE;
    UINT_32 i;

    ASSERT(prAdapter);
    ASSERT(pDomainInfo);

    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

        BSS_DESC_CHK_GUID(prBssDesc);

        DBGLOG(SCAN, TRACE, ("BSS DESC(%#lx): MAC: "MACSTR"\n",
            (UINT_32)prBssDesc, MAC2STR(prBssDesc->aucBSSID)));

        if (prBssDesc->prIECountry) {
            if (domainParseCountryInfoElem(prBssDesc->prIECountry, &rDomainInfo)) {
                if (fgIsDomainValid) {
                    if (pDomainInfo->u2CountryCode != rDomainInfo.u2CountryCode) {
                        fgIsDomainValid = FALSE;
                        break;
                    }
                }
                kalMemCopy(pDomainInfo, &rDomainInfo, sizeof(rDomainInfo));
                fgIsDomainValid = TRUE;
            }
        }
    }

    /* It also validates if the channel of the scanned BSSs is valid for the resolved domain */
    if (fgIsDomainValid) {
        LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

            BSS_DESC_CHK_GUID(prBssDesc);

            fgIsChannelValid = FALSE;

            for (i = 0; i < prAdapter->u2NicOpChnlNum; i++) {
                if ((prBssDesc->eBand == prAdapter->arNicOpChnList[i].eBand) &&
                    (prBssDesc->ucChannelNum == prAdapter->arNicOpChnList[i].ucChannelNum)) {

                    fgIsChannelValid = TRUE;
                    break;
                }
            }

            if (!fgIsChannelValid) {
                break;
            }
        }
    }

    return fgIsDomainValid & fgIsChannelValid;
}
#endif


