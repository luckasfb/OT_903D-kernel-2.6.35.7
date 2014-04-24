






#include "precomp.h"

extern PHY_ATTRIBUTE_T rPhyAttributes[];
extern ADHOC_MODE_ATTRIBUTE_T rAdHocModeAttributes[];







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ UINT_16
ibssBuildCapabilityInfo (
    IN P_ADAPTER_T prAdapter
    )
{
    P_BSS_INFO_T prBssInfo;
    UINT_16 u2CapInfo;


    ASSERT(prAdapter);
    prBssInfo = &prAdapter->rBssInfo;

    u2CapInfo = CAP_INFO_IBSS;

    if (privacyEnabled(prAdapter)) {
        u2CapInfo |= CAP_INFO_PRIVACY;
    }

    if (prBssInfo->fgIsShortPreambleAllowed) {
        u2CapInfo |= CAP_INFO_SHORT_PREAMBLE;
    }

    if (prBssInfo->fgUseShortSlotTime) {
        u2CapInfo |= CAP_INFO_SHORT_SLOT_TIME;
    }

    return u2CapInfo;

} /* end of ibssBuildCapabilityInfo() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ UINT_16
ibssBuildBeaconProbeRespFrameIEs (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer
    )
{
    P_BSS_INFO_T prBssInfo;
    UINT_8 aucAllSupportedRates[RATE_NUM] = {0};
    UINT_8 ucAllSupportedRatesLen;
    UINT_8 ucSupRatesLen;
    UINT_8 ucExtSupRatesLen;
    UINT_16 u2IeTotalLen = 0;
    UINT_8 ucIeLength;
#if CFG_SUPPORT_802_11D
    UINT_8 ucCountryInfoLen;
    P_CONNECTION_SETTINGS_T prConnSettings;
#endif

    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    prBssInfo = &prAdapter->rBssInfo;

#if CFG_SUPPORT_802_11D
    prConnSettings = &prAdapter->rConnSettings;
#endif

    //4 <1> Fill the SSID element. ID:0
    SSID_IE(pucBuffer)->ucId = ELEM_ID_SSID;

    COPY_SSID(SSID_IE(pucBuffer)->aucSSID,
              SSID_IE(pucBuffer)->ucLength,
              prBssInfo->aucSSID,
              prBssInfo->ucSSIDLen);

    pucBuffer += ELEM_HDR_LEN + prBssInfo->ucSSIDLen;
    u2IeTotalLen += ELEM_HDR_LEN + prBssInfo->ucSSIDLen;


    //4 <2> Fill the Supported Rates element. ID:1
    rateGetDataRatesFromRateSet(prBssInfo->u2OperationalRateSet,
                                prBssInfo->u2BSSBasicRateSet,
                                aucAllSupportedRates,
                                &ucAllSupportedRatesLen);

    ucSupRatesLen = ((ucAllSupportedRatesLen > ELEM_MAX_LEN_SUP_RATES) ?
                     ELEM_MAX_LEN_SUP_RATES : ucAllSupportedRatesLen);

    ucExtSupRatesLen = ucAllSupportedRatesLen - ucSupRatesLen;

    if (ucSupRatesLen) {
        SUP_RATES_IE(pucBuffer)->ucId = ELEM_ID_SUP_RATES;
        SUP_RATES_IE(pucBuffer)->ucLength = ucSupRatesLen;
        kalMemCopy(SUP_RATES_IE(pucBuffer)->aucSupportedRates,
                   aucAllSupportedRates,
                   ucSupRatesLen);

        pucBuffer += ELEM_HDR_LEN + ucSupRatesLen;
        u2IeTotalLen += ELEM_HDR_LEN + ucSupRatesLen;
    }


    //4 <3> Fill the DS Parameter Set element. ID:3
    DS_PARAM_IE(pucBuffer)->ucId = ELEM_ID_DS_PARAM_SET;
    DS_PARAM_IE(pucBuffer)->ucLength = ELEM_MAX_LEN_DS_PARAMETER_SET;
    DS_PARAM_IE(pucBuffer)->ucCurrChnl = prBssInfo->ucChnl;

    pucBuffer += ELEM_HDR_LEN + ELEM_MAX_LEN_DS_PARAMETER_SET;
    u2IeTotalLen += ELEM_HDR_LEN + ELEM_MAX_LEN_DS_PARAMETER_SET;


    //4 <4> IBSS Parameter Set element, ID: 6
    IBSS_PARAM_IE(pucBuffer)->ucId = ELEM_ID_IBSS_PARAM_SET;
    IBSS_PARAM_IE(pucBuffer)->ucLength = ELEM_MAX_LEN_IBSS_PARAMETER_SET;
    WLAN_SET_FIELD_16(&(IBSS_PARAM_IE(pucBuffer)->u2ATIMWindow), prBssInfo->u2ATIMWindow);

    pucBuffer += ELEM_HDR_LEN + ELEM_MAX_LEN_IBSS_PARAMETER_SET;
    u2IeTotalLen += ELEM_HDR_LEN + ELEM_MAX_LEN_IBSS_PARAMETER_SET;

    //4 5. Country Information Element, ID: 7, for 11H or 11D
#if CFG_SUPPORT_802_11D
    /* Set the Country Information element. (ID: 10) */
    if (prConnSettings->fgMultiDomainCapabilityEnabled) {
        if (prBssInfo->fgIsCountryInfoPresent) {
            domainConstructCountryInfoElem(prAdapter, pucBuffer, &ucCountryInfoLen);

            pucBuffer += ELEM_HDR_LEN + ucCountryInfoLen;
            u2IeTotalLen += ELEM_HDR_LEN + ucCountryInfoLen;
        }
    }
#endif
#if 0//
    /*
    NOTE:
        Following section is not enabled, by the statement of IEEE802.11,
        Chapter 10.3.2.2.2 "Semantics of the service primitive":
        Country information is only present only when TPC functionality is
        required, as specified in 11.8, or when dot11MultiDomainCapabilityEnabled
        is true.
    */
    if (prBssInfo->fgIsCountryInfoPresent) {
        domainConstructCountryInfoElem(prAdapter, pucBuffer, &ucCountryInfoLen);

        pucBuffer += ELEM_HDR_LEN + ucCountryInfoLen;
        u2IeTotalLen += ELEM_HDR_LEN + ucCountryInfoLen;
    }
#endif

    /*
    //4 6. BSS Load, ID: 11

    //4 7. Power Constraint, ID: 32, for 11H

    //4 8. TPC Report, ID: 35, for 11H

    //4 9. Channel Switch Anouncement, ID: 37, for 11H

    //4 10. Quiet, ID: 40, for 11H

    //4 11. IBSS DFS, ID: 41, for 11H
*/

    //4 <12> ERP Information, ID: 42
    if (prBssInfo->fgIsERPPresent) {
        ERP_INFO_IE(pucBuffer)->ucId = ELEM_ID_ERP_INFO;
        ERP_INFO_IE(pucBuffer)->ucLength = ELEM_MAX_LEN_ERP;
        ERP_INFO_IE(pucBuffer)->ucERP = prBssInfo->ucERP;

        pucBuffer += ELEM_HDR_LEN + ELEM_MAX_LEN_ERP;
        u2IeTotalLen += ELEM_HDR_LEN + ELEM_MAX_LEN_ERP;
    }

    //4 <13> Extended Supported Rates element ID: 42
    if (ucExtSupRatesLen) {

        EXT_SUP_RATES_IE(pucBuffer)->ucId = ELEM_ID_EXTENDED_SUP_RATES;
        EXT_SUP_RATES_IE(pucBuffer)->ucLength = ucExtSupRatesLen;

        kalMemCopy(EXT_SUP_RATES_IE(pucBuffer)->aucExtSupportedRates,
                   &aucAllSupportedRates[ucSupRatesLen],
                   ucExtSupRatesLen);

        pucBuffer += ELEM_HDR_LEN + ucExtSupRatesLen;
        u2IeTotalLen += ELEM_HDR_LEN + ucExtSupRatesLen;
    }

    //4 <14> RSN, ID: 48

    //4 <15> WPA NONE , ID: 221
    ucIeLength = rsnGenerateWpaNoneIE(prAdapter, pucBuffer);

    pucBuffer += ucIeLength;
    u2IeTotalLen += ucIeLength;


    //4 <16> WMM

    return u2IeTotalLen;

} /* end of ibssBuildBeaconProbeRespFrameIEs() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ibssComposeBeaconProbeRespFrame (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_8      pucBuffer,
    IN PUINT_8      pucDestAddr,
    OUT PUINT_16    pu2WlanHeaderLen,
    OUT PUINT_16    pu2WlanBodyLen
    )
{
    P_WLAN_BEACON_FRAME_T prBeaconHdr;
    P_BSS_INFO_T prBssInfo;
    UINT_8 aucBCAddr[] = BC_MAC_ADDR;
    UINT_16 u2FrameCtrl;
    UINT_16 u2CapInfo;
    UINT_16 u2WlanBodyLen;


    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(pu2WlanHeaderLen);
    ASSERT(pu2WlanBodyLen);

    prBeaconHdr = (P_WLAN_BEACON_FRAME_T)pucBuffer;
    prBssInfo = &prAdapter->rBssInfo;

    //4 <1> Compose the frame header of the Beacon /ProbeResp frame.
    /* Fill the Frame Control field. */
    if (pucDestAddr) {
        u2FrameCtrl = MAC_FRAME_PROBE_RSP;
    } else {
        u2FrameCtrl = MAC_FRAME_BEACON;
        pucDestAddr = aucBCAddr;
    }
    WLAN_SET_FIELD_16(&prBeaconHdr->u2FrameCtrl, u2FrameCtrl);

    /* Fill the DA field with BCAST MAC ADDR or TA of ProbeReq. */
    COPY_MAC_ADDR(prBeaconHdr->aucDestAddr, pucDestAddr);

    /* Fill the SA field with our MAC Address. */
    COPY_MAC_ADDR(prBeaconHdr->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the BSSID field with current BSSID. */
    COPY_MAC_ADDR(prBeaconHdr->aucBSSID, prBssInfo->aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prBeaconHdr->u2SeqCtrl = 0;


    //4 <2> Compose the frame body's common fixed field part of the Beacon /ProbeResp frame.
    /* MAC will update TimeStamp field */

    /* Fill the Beacon Interval field. */
    WLAN_SET_FIELD_16(&prBeaconHdr->u2BeaconInterval, prBssInfo->u2BeaconInterval);

    u2CapInfo = ibssBuildCapabilityInfo(prAdapter);

    /* Fill the Capability Information field. */
    WLAN_SET_FIELD_16(&prBeaconHdr->u2CapInfo, u2CapInfo);


    //4 <3> Update the MAC header/body length.
    *pu2WlanHeaderLen = WLAN_MAC_MGMT_HEADER_LEN;
    u2WlanBodyLen = TIMESTAMP_FIELD_LEN +
                    BEACON_INTERVAL_FIELD_LEN +
                    CAP_INFO_FIELD_LEN;

    //4 <4> Compose the frame body's IEs of the Beacon /ProbeResp frame.
    u2WlanBodyLen += ibssBuildBeaconProbeRespFrameIEs(prAdapter, prBeaconHdr->aucInfoElem);

    *pu2WlanBodyLen = u2WlanBodyLen;

    return;
} /* end of ibssComposeBeaconProbeRespFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
ibssPrepareBeaconFrame (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4MaxContentLen,
    OUT PUINT_8 pucBeaconContent,
    OUT PUINT_16 pu2ContentLen
    )
{
    UINT_16 u2WlanHeaderLen;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;


    ASSERT(prAdapter);
    ASSERT(pucBeaconContent);
    ASSERT(pu2ContentLen);

    u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                          TIMESTAMP_FIELD_LEN + \
                          BEACON_INTERVAL_FIELD_LEN + \
                          CAP_INFO_FIELD_LEN + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                          (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES)) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_DS_PARAMETER_SET) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_IBSS_PARAMETER_SET) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_ERP) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_WPA_RSN) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);

    if (u4MaxContentLen < u2EstimatedFrameLen) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ibssComposeBeaconProbeRespFrame(prAdapter,
                                    pucBeaconContent,
                                    (PUINT_8)NULL,
                                    &u2WlanHeaderLen,
                                    &u2WlanBodyLen);

    ASSERT(u4MaxContentLen >= (UINT_32)(u2WlanHeaderLen + u2WlanBodyLen));

    *pu2ContentLen = (u2WlanHeaderLen + u2WlanBodyLen);

    return WLAN_STATUS_SUCCESS;

} /* end of ibssPrepareBeaconFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ibssProcessProbeRequest (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb
    )
{
    P_BSS_INFO_T prBssInfo;

    P_WLAN_MAC_MGMT_HEADER_T prMgtHdr;
    PUINT_8 pucIEBuf;

    P_IE_SSID_T prSSID = NULL;
    P_MSDU_INFO_T prMsduInfo = NULL;
    P_MGT_PACKET_T prMgtPacket = NULL;
    OS_SYSTIME rArrivalTime;
    UINT_16 u2WlanHeaderLen;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;
    UINT_32 u4Status = WLAN_STATUS_SUCCESS;
    DEBUGFUNC("ibssProcessProbeRequest");


    ASSERT(prAdapter);
    ASSERT(prSwRfb);

    prBssInfo = &prAdapter->rBssInfo;
    prMgtHdr = (P_WLAN_MAC_MGMT_HEADER_T)prSwRfb->pvHeader;
    pucIEBuf = (PUINT_8)prSwRfb->pvHeader + sizeof(WLAN_MAC_MGMT_HEADER_T);

    if (!nicpmIfAdhocStaMaster(prAdapter)) {
        return;
    }

    //4 <1> Check SSID
    prSSID = SSID_IE(pucIEBuf);
    if (prSSID->ucLength) {
        if (UNEQUAL_SSID(prBssInfo->aucSSID, prBssInfo->ucSSIDLen, prSSID->aucSSID, prSSID->ucLength)) {
            DBGLOG(MGT, LOUD, ("SSID not matched\n"));
            return;
        }
    }

    //4 <2> Allocate MSDU_INFO_T
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
    if (prMsduInfo == (P_MSDU_INFO_T)NULL) {
        DBGLOG(MGT, WARN, ("No MSDU_INFO_T to send Probe Response.\n"));
        return;
    }

    //4 <3> Allocate Frame Buffer (in MGT_PACKET_T) for ProbeResp Frame
    u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                          TIMESTAMP_FIELD_LEN + \
                          BEACON_INTERVAL_FIELD_LEN + \
                          CAP_INFO_FIELD_LEN + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                          (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES)) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_DS_PARAMETER_SET) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_IBSS_PARAMETER_SET) + \
                          (ELEM_HDR_LEN + ELEM_MAX_LEN_ERP);

    if (prAdapter->rConnSettings.eAuthMode == AUTH_MODE_WPA_NONE) {
        u2EstimatedFrameLen += (ELEM_HDR_LEN + ELEM_MAX_LEN_WPA_RSN);
    }

    if (prBssInfo->fgIsWmmAssoc) {
        u2EstimatedFrameLen += (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);
    }

    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, (UINT_32)u2EstimatedFrameLen);
    if (prMgtPacket == (P_MGT_PACKET_T)NULL) {
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        DBGLOG(MGT, WARN, ("No buffer to send Probe Response, required = %d.\n", u2EstimatedFrameLen));
        return;
    }

    //4 <4> Compose Association Request frame in MGT_PACKET_T.
    ibssComposeBeaconProbeRespFrame(prAdapter,
                                    MGT_PACKET_GET_BUFFER(prMgtPacket),
                                    prMgtHdr->aucSrcAddr,
                                    &u2WlanHeaderLen,
                                    &u2WlanBodyLen);
    //4 <5> Update the frame length to the packet descriptor (MGT_PACKET_T).
    mgtPacketPut(prMgtPacket, (u2WlanHeaderLen + u2WlanBodyLen));

    //4 <6> Update information in MSDU_INFO_T for TX Module.
    GET_CURRENT_SYSTIME(&rArrivalTime);

    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                       TRUE, \
                       TRUE, \
                       (PVOID)prMgtPacket, \
                       0, \
                       TXQ_TCM, \
                       (UINT_8)u2WlanHeaderLen, \
                       u2WlanBodyLen, \
                       MSDU_INFO_CTRL_FLAG_SPECIFY_AC | MSDU_INFO_CTRL_FLAG_BASIC_RATE | MSDU_INFO_CTRL_FLAG_NO_ACK, \
                       (PFN_TX_DONE_HANDLER)0, \
                       rArrivalTime, \
                       NULL \
                       );

    //4 <7> Inform ARB to send this Authentication Request frame.
    DBGLOG(MGT, LOUD, ("Send Probe Response frame\n"));
    if ((u4Status = arbFsmRunEventTxMmpdu(prAdapter,prMsduInfo)) != WLAN_STATUS_PENDING) {

        if(u4Status != WLAN_STATUS_SUCCESS) {
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            DBGLOG(MGT, ERROR, ("Send Probe Response fail.\n"));
        }


        //return WLAN_STATUS_FAILURE;
    }

    return;
} /* end of ibssProcessProbeRequest() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
ibssCheckCapabilityForAdHocMode (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;

    DEBUGFUNC("ibssCheckCapabilityForAdHocMode");


    ASSERT(prAdapter);
    ASSERT(prBssDesc);
    prConnSettings = &prAdapter->rConnSettings;

    do {
        //4 <1> Check the BSS Basic Rate Set for current AdHoc Mode
        if ((prConnSettings->eAdHocMode == AD_HOC_MODE_11B) &&
            (prBssDesc->u2BSSBasicRateSet & ~RATE_SET_HR_DSSS)) {
            break;
        }
#if 0 // Enable 11A if RF support.
        else if ((prConnSettings->eAdHocMode == AD_HOC_MODE_11A) &&
            (prBssDesc->u2BSSBasicRateSet & ~RATE_SET_OFDM)) {
            break;
        }
#endif

        //4 <2> Check the Short Slot Time.
#if 0 // Do not check ShortSlotTime until Wi-Fi define such policy        
        if (prConnSettings->eAdHocMode == AD_HOC_MODE_11G) {
            if (((prConnSettings->fgIsShortSlotTimeOptionEnable) &&
                 !(prBssDesc->u2CapInfo & CAP_INFO_SHORT_SLOT_TIME)) ||
                (!(prConnSettings->fgIsShortSlotTimeOptionEnable) &&
                 (prBssDesc->u2CapInfo & CAP_INFO_SHORT_SLOT_TIME))) {
                break;
            }
        }
#endif

#if CFG_IBSS_POWER_SAVE
        //4 <3> Check the ATIM window setting.
        if (prBssDesc->u2ATIMWindow) {
            if (!PM_IS_ADHOC_POWER_SAVE_CAPABLE(prAdapter)) {
                DBGLOG(LP, INFO, ("No Ad hoc PS capability (recvd ATIM window: %d)\n",
                    prBssDesc->u2ATIMWindow));
                break;
            }
        }
#else
        //4 <3> Check the ATIM window setting.
        if (prBssDesc->u2ATIMWindow) {
            break;
        }
#endif /* CFG_IBSS_POWER_SAVE */


        //4 <4> Check the Security setting.
        if (!rsnPerformPolicySelection(prAdapter, prBssDesc)) {
            break;
        }
        
        rStatus = WLAN_STATUS_SUCCESS;
    }
    while (FALSE);

    return rStatus;

} /* end of ibssCheckCapabilityForAdHocMode() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ibssProcessBeacon (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBSSDesc,
    IN P_SW_RFB_T prSwRfb
    )
{
    P_BSS_INFO_T prBssInfo;
    P_WLAN_BEACON_FRAME_T prWlanBeaconFrame;
    P_STA_RECORD_T prStaRec = NULL;
    BOOLEAN fgIsCheckCapability = FALSE;
    BOOLEAN fgIsCheckTSF = FALSE;
    BOOLEAN fgIsGoingMerging = FALSE;
    BOOLEAN fgIsGoingChangeDetection = FALSE;
    BOOLEAN fgIsSameBSSID = FALSE;
    BOOLEAN fgIsSameSSID = FALSE;

    DEBUGFUNC("ibssProcessBeacon");


    ASSERT(prAdapter);
    ASSERT(prBSSDesc);
    ASSERT(prSwRfb);

    prBssInfo = &prAdapter->rBssInfo;
    prWlanBeaconFrame = (P_WLAN_BEACON_FRAME_T)prSwRfb->pvHeader;

    //4 <1> Check if the BSS_DESC_T is IBSS.
    if (prBSSDesc->eBSSType != BSS_TYPE_IBSS) {
        return;
    }

    //4 <2> Process IBSS Beacon only after we create or merge with other IBSS.
    if (!prAdapter->fgIsIBSSActive) {
        return;
    }


    //4 <3> Get the STA_RECORD_T of TA.
    prStaRec = staRecGetStaRecordByAddr(prAdapter, prWlanBeaconFrame->aucSrcAddr);

    fgIsSameBSSID = UNEQUAL_MAC_ADDR(prBssInfo->aucBSSID, prBSSDesc->aucBSSID) ? FALSE : TRUE;
    
    fgIsSameSSID = UNEQUAL_SSID(prBssInfo->aucSSID, prBssInfo->ucSSIDLen,
                                prBSSDesc->aucSSID, prBSSDesc->ucSSIDLen) ? FALSE : TRUE;


    //4 <4> Change detection for Abort Event
    if ((prStaRec) && fgIsSameBSSID) {

        //4 <4.1> Check if the BSS_DESC_T's SSID is identical to mine.
        if (!fgIsSameSSID) {
            /* NOTE(Kevin): If remote peer change its SSID suddenly, but still has same BSSID,
             * we should leave this IBSS immediately. We'll do scan after calling
             * the ABORT event and check if there exist any valid peers (same SSID).
             */
             
            arbFsmRunEventAbort(prAdapter, TRUE);
        }

        //4 <4.2> Check if the BSS_DESC_T's Privacy in CAP_FIELD and WPA_NONE is identical to mine.
        if (!rsnPerformPolicySelection(prAdapter, prBSSDesc)) {
            /* NOTE(Kevin): If remote peer change its Security Setting suddenly, but still has same BSSID,
             * we should leave this IBSS immediately. We'll do scan after calling
             * the ABORT event and check if there exist any valid peers (same SSID).
             */
             
            arbFsmRunEventAbort(prAdapter, TRUE);
        }

        //4 <4.3> Check if the BSS_DESC_T's Channel is identical to mine.
        if (prBssInfo->ucChnl != prBSSDesc->ucChannelNum) {
            /* NOTE(Kevin): If remote peer change its channel suddenly, but still has same BSSID,
             * we should leave this IBSS immediately. We'll do scan after calling
             * the ABORT event and check if there exist any valid peers (same SSID).
             */
             
            arbFsmRunEventAbort(prAdapter, TRUE);
        }
    }


    //4 <5> Check if the BSS_DESC_T's SSID is identical to mine.
    if (!fgIsSameSSID) {
        return;
    }


    //4 <6> IBSS Merge Decision Flow for Processing Beacon.
    if (fgIsSameBSSID) {

        /* Same BSSID:
         * Case I.  This is a new TA and it has decide to merged with us.
         * Case II. This is an old TA and we've already merged together.
         */

        if (!prStaRec) {
            /* For Case I - Check this IBSS's capability first before adding this Sta Record. */
            fgIsCheckCapability = TRUE;

            /* If check is passed, then we perform merging with this new IBSS */
            fgIsGoingMerging = TRUE;

        }
        else {
            /* For Case II - Update rExpirationTime of Sta Record */
            GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);

            fgIsGoingChangeDetection = TRUE; /* We need to check if this STA is 11b */
        }
    }
    else {

        /* Unequal BSSID:
         * Case III. This is a new TA and we need to compare the TSF and get the winner.
         * Case IV.  This is an old TA and it merge into a new IBSS before we do the same thing.
         *           We need to compare the TSF to get the winner.
         * Case V.   This is an old TA and it restart a new IBSS. We also need to
         *           compare the TSF to get the winner.
         */

        /* For Case III, IV & V - We'll always check this new IBSS's capability first
         * before merging into new IBSS.
         */
        fgIsCheckCapability = TRUE;

        /* If check is passed, we need to perform TSF check to decide the major BSSID */
        fgIsCheckTSF = TRUE;

        /* For Case IV & V - We won't update rExpirationTime of Sta Record */
    }


    //4 <7> Check this BSS_DESC_T's capability.
    if (fgIsCheckCapability) {
        BOOLEAN fgIsCapabilityMatched = FALSE;


        do {
            if (!(BIT(prBSSDesc->ePhyType) & prAdapter->u2AvailablePhyTypeSet)) {

                DBGLOG(MGT, TRACE, ("Ignore BSS DESC MAC: "MACSTR", PhyType = %s not supported\n",
                    MAC2STR(prBSSDesc->aucBSSID),
                    ((prBSSDesc->ePhyType == PHY_TYPE_ERP_INDEX) ? "PHY_TYPE_ERP" :
                     ((prBSSDesc->ePhyType == PHY_TYPE_HR_DSSS_INDEX) ? "PHY_TYPE_HR_DSSS" : "PHY_TYPE_OFDM") ) ));

                break;
            }

            if ((prBSSDesc->u2BSSBasicRateSet &
                 ~(rPhyAttributes[prBSSDesc->ePhyType].u2SupportedRateSet)) ||
                prBSSDesc->fgIsUnknownBssBasicRate) {

                break;
            }

            if (ibssCheckCapabilityForAdHocMode(prAdapter, prBSSDesc) == WLAN_STATUS_FAILURE) {

                DBGLOG(MGT, TRACE,
                    ("Ignore BSS DESC MAC: "MACSTR", Capability is not supported for current AdHoc Mode.\n",
                     MAC2STR(prBSSDesc->aucBSSID)));

                break;
            }

            fgIsCapabilityMatched = TRUE;
        }
        while (FALSE);

        if (!fgIsCapabilityMatched) {

            if (prStaRec) {
                /* For Case II - We merge this STA_RECORD in RX Path.
                 *     Case IV & V - They change their BSSID after we merge with them.
                 */

                DBGLOG(MGT, WARN,
                    ("We merge a BSS DESC MAC: "MACSTR" which its Capability is not match with us.\n",
                     MAC2STR(prBSSDesc->aucBSSID)));
            }

            return;
        }
    }


    if (fgIsCheckTSF) {
#if !CFG_SW_TCL // NOTE(Kevin 2008/03/29): Use TCL in RFB
        if (prBSSDesc->fgIsLargerTSF) {
            fgIsGoingMerging = TRUE;
        }
        else {
            return;
        }
#else /* CFG_SW_TCL */
        ULARGE_INTEGER rCurrentTsf;

        NIC_GET_CURRENT_TSF(prAdapter, &rCurrentTsf);

        DBGLOG(SCAN, TRACE,
            ("\n\nCurrent TSF : %08lx-%08lx\n\n",
                rCurrentTsf.u.HighPart, rCurrentTsf.u.LowPart));

        if (rCurrentTsf.QuadPart > prBSSDesc->u8TimeStamp.QuadPart) {
            DBGLOG(SCAN, TRACE,
                ("Ignore BSS DESC MAC: ["MACSTR"], Current BSSID: ["MACSTR"].\n",
                    MAC2STR(prBSSDesc->aucBSSID), MAC2STR(prBssInfo->aucBSSID)));

            DBGLOG(SCAN, TRACE,
                ("\n\nBSS's TSF : %08lx-%08lx\n\n",
                    prBSSDesc->u8TimeStamp.u.HighPart, prBSSDesc->u8TimeStamp.u.LowPart));

            prBSSDesc->fgIsLargerTSF = FALSE;
            return;
        }
        else {
            prBSSDesc->fgIsLargerTSF = TRUE;
            fgIsGoingMerging = TRUE;
        }
#endif /* CFG_SW_TCL */
    }


    if (fgIsGoingMerging) {
        /* Inform ARB to do STATE TRANSITION
         * For Case I - If ARB in IBSS_ALONE, let it jump to NORMAL_TR after we know the new member.
         * For Case III, IV - Now this new BSSID wins the TSF, follow it.
         */
        arbFsmRunEventIbssMerge(prAdapter, prBSSDesc);
    }
    else if (fgIsGoingChangeDetection) {

        //4 <9> Protection of 11b member
        if (prBssInfo->fgIsERPPresent) {
            if ((!prBssInfo->fgIsProtection) && 
                (prBSSDesc->ePhyType == PHY_TYPE_HR_DSSS_INDEX)) {

                prBssInfo->fgIsProtection = TRUE;

                nicRateEnableProtection(prAdapter, CTS_PROTECTION_TYPE_802_11);

                prBssInfo->ucERP |= (ERP_INFO_NON_ERP_PRESENT | ERP_INFO_USE_PROTECTION);

                ARB_SET_TIMER(prAdapter,
                              prAdapter->rArbInfo.rProtectionTimerForAdHoc,
                              SEC_TO_MSEC(IBSS_PROTECTION_TIMEOUT_CHECK_SEC));

                kalMemZero(prBssInfo->aucBcnContent, sizeof(prBssInfo->aucBcnContent));
                prBssInfo->u2BcnLen = 0;

                ibssPrepareBeaconFrame(prAdapter,
                                       sizeof(prBssInfo->aucBcnContent),
                                       prBssInfo->aucBcnContent,
                                       &prBssInfo->u2BcnLen);

                nicStartBeacon(prAdapter);
            }
        }        
    }

    return;
} /* end of ibssProcessBeacon() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
ibssStartIBSS (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    P_STA_RECORD_T prStaRec = (P_STA_RECORD_T)NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    P_TX_CTRL_T prTxCtrl;

    DEBUGFUNC("ibssStartIBSS");


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;
    prTxCtrl = &prAdapter->rTxCtrl;

    //4 <1> After STANDBY STATE, we only create IBSS once
    if (!prAdapter->fgIsIBSSActive) {

    //4 <2> Update BSS_INFO_T from CONNECTION_SETTINGS_T
        //4 <2.A> PHY Type (Corresponding to 2.A in joinComplete() )
        prBssInfo->ePhyType =
            rAdHocModeAttributes[prConnSettings->eAdHocMode].ePhyTypeIndex;

        prAdapter->eCurrentPhyType = prBssInfo->ePhyType;

        //4 <2.B> Country Information
#if CFG_SUPPORT_802_11D
        if (prConnSettings->fgMultiDomainCapabilityEnabled) {
            if (domainGetDomainInfoByScanResult(prAdapter, &prBssInfo->rDomainInfo)) {

                /* use the domain get from the scan result */
                prBssInfo->fgIsCountryInfoPresent = TRUE;
                nicSetupOpChnlList(prAdapter, prBssInfo->rDomainInfo.u2CountryCode, FALSE);
            }
        }
#endif

        //4 <2.C> BSS Type
        prBssInfo->eBSSType = BSS_TYPE_IBSS;

        //4 <2.D> BSSID
        {
            UINT_8 aucBSSID[MAC_ADDR_LEN];
            PUINT_16 pu2BSSID = (PUINT_16)&aucBSSID[0];
            UINT_32 i;

            for (i = 0; i < sizeof(aucBSSID)/sizeof(UINT_16); i++) {
                nicGetRandomNumber(prAdapter, &pu2BSSID[i]);
            }

            aucBSSID[0] &= ~0x01; // 7.1.3.3.3 - The individual/group bit of the address is set to 0.
            aucBSSID[0] |= 0x02; // 7.1.3.3.3 - The universal/local bit of the address is set to 1.

            COPY_MAC_ADDR(prBssInfo->aucBSSID, aucBSSID);
        }

        //4 <2.E> SSID
        COPY_SSID(prBssInfo->aucSSID,
                  prBssInfo->ucSSIDLen,
                  prAdapter->rConnSettings.aucSSID,
                  prAdapter->rConnSettings.ucSSIDLen);

        //4 <2.F> Channel / Band information
        prBssInfo->eBand = (ENUM_BAND_T)prConnSettings->ucChannelBand;
        prBssInfo->ucChnl = prConnSettings->ucChannelNum;


        //4 <2.G> RSN/WPA information - Not needed in AdHoc(W.H)

        //4 <2.H> Beacon interval
        prBssInfo->u2BeaconInterval = prConnSettings->u2BeaconPeriod;

        //4 <2.I> ATIM Window (Corresponding to DTIM)
        prBssInfo->u2ATIMWindow = prConnSettings->u2AtimWindow;

        //4 <2.J> ERP Information
        if (prBssInfo->ePhyType == PHY_TYPE_ERP_INDEX) {
            prBssInfo->fgIsERPPresent = TRUE;
        }
        else { /* Some AP, may send ProbeResp without ERP IE. Thus prBssDesc->fgIsERPPresent is FALSE. */
            prBssInfo->fgIsERPPresent = FALSE;
        }
        prBssInfo->ucERP = 0;

        DBGLOG(MGT, INFO, ("prBssInfo->fgIsERPPresent = %d, prBssInfo->ucERP = %02x\n",
            prBssInfo->fgIsERPPresent, prBssInfo->ucERP));

    //4 <3> NIC RATE FUNC
        //4 <3.B > WMM Infomation - Setup Variables
        if (prAdapter->fgIsEnableWMM && prConnSettings->fgIsAdHocQoSEnable) {
            prBssInfo->fgIsWmmAssoc = TRUE;
            prTxCtrl->rTxQForVoipAccess = TXQ_AC3;
        }
        else {
            prBssInfo->fgIsWmmAssoc = FALSE;
            prTxCtrl->rTxQForVoipAccess = TXQ_AC1;

            kalMemZero(&prBssInfo->rWmmInfo, sizeof(WMM_INFO_T));
        }

        //4 <3.C> Operational Rate Set & BSS Basic Rate Set
        prBssInfo->u2OperationalRateSet =
            rPhyAttributes[prBssInfo->ePhyType].u2SupportedRateSet;

        prBssInfo->u2BSSBasicRateSet =
            rAdHocModeAttributes[prConnSettings->eAdHocMode].u2BSSBasicRateSet;

        //4 <3.D> Short Preamble
        if ( rPhyAttributes[prBssInfo->ePhyType].fgIsShortPreambleOptionImplemented &&
            ((prConnSettings->ePreambleType == PREAMBLE_TYPE_SHORT) || /* Short Preamble Option Enable is TRUE */
             (prConnSettings->ePreambleType == PREAMBLE_TYPE_AUTO)) ) {

            prBssInfo->fgIsShortPreambleAllowed = TRUE;
            prBssInfo->fgUseShortPreamble = TRUE;
        }
        else {
            prBssInfo->fgIsShortPreambleAllowed = FALSE;
            prBssInfo->fgUseShortPreamble = FALSE;
        }

        //4 <3.D> Update ERP after Short Preamble.
        if (prBssInfo->fgIsERPPresent) {
            if (!prBssInfo->fgUseShortPreamble) {
                prBssInfo->ucERP |= ERP_INFO_BARKER_PREAMBLE_MODE;
            }
        }

        //4 <3.E> Short Slot Time
        // 7.3.1.4 For IBSS, the Short Slot Time subfield shall be set to 0.
        prBssInfo->fgUseShortSlotTime = FALSE;

        DBGLOG(MGT, INFO, ("prBssInfo->fgUseShortSlotTime = %d\n",
            prBssInfo->fgUseShortSlotTime));

        nicSetSlotTime(prAdapter,
                       prBssInfo->ePhyType,
                       FALSE);


        //4 <3.F> Update Tx Rate for Control Frame
        bssUpdateTxRateForControlFrame(prAdapter);

        //4 <3.G> Save the available Auth Types during Roaming  - Not needed in AdHoc.

        //4 <3.H> Update Parameter for TX Fragmentation Threshold
#if CFG_TX_FRAGMENT
        txFragInfoUpdate(prAdapter);
#endif /* CFG_TX_FRAGMENT */


    //4 <4> Update Default STA_RECORD_T
        {
            UINT_16 u2DesiredRateSet;


            /* We create a new BSS, so remove entire old records. */
            staRecRemoveStaRecordByPolicy(prAdapter, STA_RECORD_RM_POLICY_ENTIRE);

            /* Add a STA Record for default entry */
            prStaRec = staRecGetStaRecordByAddrOrAddIfNotExist(prAdapter,
                           prBssInfo->aucBSSID);

            /* **NOTE(Kevin): This is the default entry(BSSID), we use STATE_3 to avoid
             * been clean up by aging func.
             */
            prStaRec->ucStaState = STA_STATE_3;

            prStaRec->rRcpi = RCPI_HIGH_BOUND;

            /* Update the record activity time. */
            GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);


            //4 <4.A> Desired Rate Set, use BasicRateSet for default(BMCAST) entry
            u2DesiredRateSet = (prConnSettings->u2DesiredRateSet &
                                    prBssInfo->u2BSSBasicRateSet);
            if (u2DesiredRateSet) {
                prStaRec->u2DesiredRateSet = u2DesiredRateSet;
            }
            else {
                /* For Error Handling - The Desired Rate Set is not covered in Operational Rate Set. */
                prStaRec->u2DesiredRateSet = prBssInfo->u2OperationalRateSet;
            }

            /* Try to set the best initial rate for default entry */
            if (!rateGetBestInitialRateIndex(prStaRec->u2DesiredRateSet,
                                             prStaRec->rRcpi,
                                             &prStaRec->ucCurrRate1Index)) {

                if (!rateGetLowestRateIndexFromRateSet(prStaRec->u2DesiredRateSet,
                                                       &prStaRec->ucCurrRate1Index)) {
                    ASSERT(0);
                }
            }

            DBGLOG(MGT, TRACE, ("Initial Rate Index for entry ["MACSTR"] = %d\n",
                MAC2STR(prStaRec->aucMacAddr), prStaRec->ucCurrRate1Index));

            //4 <4.B> Preamble Mode
            prStaRec->fgIsShortPreambleOptionEnable =
                prBssInfo->fgUseShortPreamble;

            //4 <4.C> QoS Flag set to FALSE for default entry (BMCAST frames)
            prStaRec->fgIsQoS = FALSE;

            //4 <4.D> Update WLAN Table for default Entry.
            if (nicSetHwBySta(prAdapter, prStaRec) == FALSE) {
                ASSERT(FALSE);
            }

            //4 <4.E> Update Desired Rate Set for BT.
#if CFG_TX_FRAGMENT
            if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
                txRateSetInitForBT(prAdapter, prStaRec);
            }
#endif /* CFG_TX_FRAGMENT */

            //4 <4.F> Legacy Flag
            prStaRec->fgIsLegacy = FALSE; /* Set to FALSE for default entry */

        }


    //4 <5> Compose Beacon
        kalMemZero(prBssInfo->aucBcnContent, sizeof(prBssInfo->aucBcnContent));
        prBssInfo->u2BcnLen = 0;

        rStatus = ibssPrepareBeaconFrame(prAdapter,
                                         sizeof(prBssInfo->aucBcnContent),
                                         prBssInfo->aucBcnContent,
                                         &prBssInfo->u2BcnLen);



//        prAdapter->fgIsIBSSActive = TRUE;
    }


//4 <6> Update NIC
    //4 <6.A> Update BSSID & Operation Mode
    nicSetupBSS(prAdapter, prBssInfo);

    //4 <6.B> TX AC Parameter and TX/RX Queue Control
    if (prBssInfo->fgIsWmmAssoc) {
        qosWmmInfoInit(&prBssInfo->rWmmInfo,
                       (prBssInfo->ePhyType == PHY_TYPE_HR_DSSS_INDEX) ? TRUE : FALSE);

        qosUpdateWMMParametersAndAssignAllowedACI(prAdapter, &prBssInfo->rWmmInfo);
        ASSERT(0); /* NOTE(Kevin): Not test yet */
    }
    else {
        kalMemZero(&prBssInfo->rWmmInfo, sizeof(WMM_INFO_T));

        nicTxNonQoSAssignDefaultAdmittedTXQ(prAdapter);

        nicTxNonQoSUpdateTXQParameters(prAdapter,
                                       prBssInfo->ePhyType);
    }

    //4 <6.C> Setup IBSS' Frequency(Band/Channel)
    nicSwitchChannel(prAdapter, prBssInfo->eBand, prBssInfo->ucChnl, 0);


//4 <7> Start to sending Beacons.
    nicStartBeacon(prAdapter);

    return rStatus;

} /* end of ibssStartIBSS() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
ibssMergeIBSS (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    P_STA_RECORD_T prStaRec = (P_STA_RECORD_T)NULL;
    BOOLEAN fgIsGoingMergingOut = FALSE;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    P_TX_CTRL_T prTxCtrl;
#if CFG_SUPPORT_802_11D
    P_IE_COUNTRY_T          prIECountry;
#endif

    DEBUGFUNC("ibssMergeIBSS");


    ASSERT(prAdapter);
    ASSERT(prBssDesc);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;
    prTxCtrl = &prAdapter->rTxCtrl;

    //4 <1> IBSS Merge Decision Flow for IBSS MERGE STATE.
    if (!prAdapter->fgIsIBSSActive) {
        fgIsGoingMergingOut = TRUE;

        /* Reset TSF due to IBSS is in-active (never enter IBSS-alone),
           and it should always sync other's TSF */
        nicpmResetTSFTimer(prAdapter);
    }
    else {
        /* Merge In */
        if (UNEQUAL_MAC_ADDR(prBssInfo->aucBSSID, prBssDesc->aucBSSID)) {
            if (prBssDesc->fgIsLargerTSF) {
                fgIsGoingMergingOut = TRUE;
            }
            else {
                ASSERT(0); /* Such case should be blocked in ibssProcessBeacon() */
            }
        }
    }

    if (fgIsGoingMergingOut) {

    //4 <2> Update BSS_INFO_T from CONNECTION_SETTINGS_T
        //4 <2.A> PHY Type (Corresponding to 2.A in joinComplete() )
        prBssInfo->ePhyType = prBssDesc->ePhyType;

        prAdapter->eCurrentPhyType = prBssInfo->ePhyType;

        //4 <2.B> BSS Type
        prBssInfo->eBSSType = BSS_TYPE_IBSS;

        //4 <2.C> BSSID
        COPY_MAC_ADDR(prBssInfo->aucBSSID, prBssDesc->aucBSSID);

        //4 <2.D> SSID
        COPY_SSID(prBssInfo->aucSSID,
                  prBssInfo->ucSSIDLen,
                  prBssDesc->aucSSID,
                  prBssDesc->ucSSIDLen);

        //4 <2.E> Channel / Band information
        prBssInfo->eBand = prBssDesc->eBand;
        prBssInfo->ucChnl = prBssDesc->ucChannelNum;

        //4 <2.F> RSN/WPA information
        secFsmRunEventStart(prAdapter);

        //4 <2.G> Beacon interval
        prBssInfo->u2BeaconInterval = prBssDesc->u2BeaconInterval;

        //4 <2.H> ATIM Window (Corresponding to DTIM)
        prBssInfo->u2ATIMWindow = prBssDesc->u2ATIMWindow;

        //4 <2.I> ERP Information
        if ((prBssInfo->ePhyType == PHY_TYPE_ERP_INDEX) && // Our BSS's PHY_TYPE is ERP now.
            (prBssDesc->fgIsERPPresent)) {

            prBssInfo->fgIsERPPresent = TRUE;
            prBssInfo->ucERP = prBssDesc->ucERP; /* Save the ERP for later check */
        }
        else { /* Some AP, may send ProbeResp without ERP IE. Thus prBssDesc->fgIsERPPresent is FALSE. */
            prBssInfo->fgIsERPPresent = FALSE;
            prBssInfo->ucERP = 0;
        }

        DBGLOG(MGT, INFO, ("prBssInfo->fgIsERPPresent = %d, prBssInfo->ucERP = %02x\n",
            prBssInfo->fgIsERPPresent, prBssInfo->ucERP));

#if CFG_SUPPORT_802_11D
        //4 <2.J> Country inforamtion of the associated AP
        if (prConnSettings->fgMultiDomainCapabilityEnabled) {
            DOMAIN_INFO_ENTRY   rDomainInfo;
            if (domainGetDomainInfoByScanResult(prAdapter, &rDomainInfo)) {
                if (prBssDesc->prIECountry) {
                    prIECountry = prBssDesc->prIECountry;

                    domainParseCountryInfoElem(prIECountry, &prBssInfo->rDomainInfo);

                    /* use the domain get from the BSS info */
                    prBssInfo->fgIsCountryInfoPresent = TRUE;
                    nicSetupOpChnlList(prAdapter, prBssInfo->rDomainInfo.u2CountryCode, FALSE);
                } else {
                    /* use the domain get from the scan result */
                    prBssInfo->fgIsCountryInfoPresent = TRUE;
                    nicSetupOpChnlList(prAdapter, rDomainInfo.u2CountryCode, FALSE);
                }
            }
        }
#endif

    //4 <3> NIC RATE FUNC
        //4 <3.B > WMM Infomation - Setup Variables
#if 0 /* TODO */
        if (prAdapter->fgIsEnableWMM && prConnSettings->fgIsAdHocQoSEnable) {
            prBssInfo->fgIsWmmAssoc = TRUE;
            prTxCtrl->rTxQForVoipAccess = TXQ_AC3;
        }
        else
#endif
        {
            prBssInfo->fgIsWmmAssoc = FALSE;
            prTxCtrl->rTxQForVoipAccess = TXQ_AC1;

            kalMemZero(&prBssInfo->rWmmInfo, sizeof(WMM_INFO_T));
        }

        //4 <3.C> Operational Rate Set & BSS Basic Rate Set
        prBssInfo->u2OperationalRateSet = prBssDesc->u2OperationalRateSet;
        prBssInfo->u2BSSBasicRateSet = prBssDesc->u2BSSBasicRateSet;

        //4 <3.D> Short Preamble
#if 0 /* TODO */
        if ( rPhyAttributes[prBssInfo->ePhyType].fgIsShortPreambleOptionImplemented &&
            ((prConnSettings->ePreambleType == PREAMBLE_TYPE_SHORT) || /* Short Preamble Option Enable is TRUE */
             (prConnSettings->ePreambleType == PREAMBLE_TYPE_AUTO)) ) {

            prBssInfo->fgIsShortPreambleAllowed = TRUE;
            prBssInfo->fgUseShortPreamble = TRUE;
        }
        else
#endif
        {
            if (prBssDesc->u2CapInfo & CAP_INFO_SHORT_PREAMBLE) {
                prBssInfo->fgIsShortPreambleAllowed = TRUE;
                prBssInfo->fgUseShortPreamble = TRUE; /* TODO */
            }
            else {
                prBssInfo->fgIsShortPreambleAllowed = FALSE;
                prBssInfo->fgUseShortPreamble = FALSE;
            }
        }

        //4 <3.D'> Update ERP after Short Preamble.
        if (prBssInfo->fgIsERPPresent) {
            if (!prBssInfo->fgUseShortPreamble) {
                prBssInfo->ucERP |= ERP_INFO_BARKER_PREAMBLE_MODE;
            }
        }

        //4 <3.E> Short Slot Time
        // 7.3.1.4 For IBSS, the Short Slot Time subfield shall be set to 0.
        prBssInfo->fgUseShortSlotTime = FALSE;

        DBGLOG(MGT, INFO, ("prBssInfo->fgUseShortSlotTime = %d\n",
            prBssInfo->fgUseShortSlotTime));

        nicSetSlotTime(prAdapter,
                       prBssInfo->ePhyType,
                       FALSE);


        //4 <3.F> Update Tx Rate for Control Frame
        bssUpdateTxRateForControlFrame(prAdapter);

        //4 <3.G> Save the available Auth Types during Roaming  - Not needed in AdHoc.

        //4 <3.H> Update Parameter for TX Fragmentation Threshold
#if CFG_TX_FRAGMENT
        txFragInfoUpdate(prAdapter);
#endif /* CFG_TX_FRAGMENT */


    //4 <4> Update Default STA_RECORD_T
        {
            UINT_16 u2OperationalRateSet, u2DesiredRateSet;


            /* We merge into a new BSS, so remove entire old records. */
            staRecRemoveStaRecordByPolicy(prAdapter, STA_RECORD_RM_POLICY_ENTIRE);

            /* Add a STA Record for default entry */
            prStaRec = staRecGetStaRecordByAddrOrAddIfNotExist(prAdapter,
                           prBssInfo->aucBSSID);

            /* **NOTE(Kevin): This is the default entry(BSSID), we use STATE_3 to avoid
             * been clean up by aging func.
             */
            prStaRec->ucStaState = STA_STATE_3;

            prStaRec->rRcpi = RCPI_HIGH_BOUND;

            /* Update the record activity time. */
            GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);

            //4 <4.A> Desired Rate Set, use BasicRateSet for default(BMCAST) entry
            u2OperationalRateSet = (rPhyAttributes[prBssInfo->ePhyType].u2SupportedRateSet &
                                    prBssInfo->u2BSSBasicRateSet);

            u2DesiredRateSet = (u2OperationalRateSet & prConnSettings->u2DesiredRateSet);
            if (u2DesiredRateSet) {
                prStaRec->u2DesiredRateSet = u2DesiredRateSet;
            }
            else {
                /* For Error Handling - The Desired Rate Set is not covered in Operational Rate Set. */
                prStaRec->u2DesiredRateSet = u2OperationalRateSet;
            }

            /* Try to set the best initial rate for default entry */
            if (!rateGetBestInitialRateIndex(prStaRec->u2DesiredRateSet,
                                             prStaRec->rRcpi,
                                             &prStaRec->ucCurrRate1Index)) {

                if (!rateGetLowestRateIndexFromRateSet(prStaRec->u2DesiredRateSet,
                                                       &prStaRec->ucCurrRate1Index)) {
                    ASSERT(0);
                }
            }

            DBGLOG(MGT, TRACE, ("Initial Rate Index for entry ["MACSTR"] = %d\n",
                MAC2STR(prStaRec->aucMacAddr), prStaRec->ucCurrRate1Index));

            //4 <4.B> Preamble Mode
            prStaRec->fgIsShortPreambleOptionEnable =
                prBssInfo->fgUseShortPreamble;

            //4 <4.C> QoS Flag set to FALSE for default entry (BMCAST frames)
            prStaRec->fgIsQoS = FALSE;

            //4 <4.D> Update WLAN Table for default Entry.
            if (nicSetHwBySta(prAdapter, prStaRec) == FALSE) {
                ASSERT(FALSE);
            }

            //4 <4.E> Update Desired Rate Set for BT.
#if CFG_TX_FRAGMENT
            if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
                txRateSetInitForBT(prAdapter, prStaRec);
            }
#endif /* CFG_TX_FRAGMENT */

            //4 <4.F> Legacy Flag
            prStaRec->fgIsLegacy = FALSE; /* Set to FALSE for default entry */

        }


    //4 <5> Compose Beacon
        kalMemZero(prBssInfo->aucBcnContent, sizeof(prBssInfo->aucBcnContent));
        prBssInfo->u2BcnLen = 0;

        rStatus = ibssPrepareBeaconFrame(prAdapter,
                                         sizeof(prBssInfo->aucBcnContent),
                                         prBssInfo->aucBcnContent,
                                         &prBssInfo->u2BcnLen);

//        prAdapter->fgIsIBSSActive = TRUE;
    }


//4 <6> Update NIC
    //4 <6.A> Update BSSID & Operation Mode
    nicSetupBSS(prAdapter, prBssInfo);

    //4 <6.B> TX AC Parameter and TX/RX Queue Control
    if (prBssInfo->fgIsWmmAssoc) {
        qosWmmInfoInit(&prBssInfo->rWmmInfo,
                       (prBssInfo->ePhyType == PHY_TYPE_HR_DSSS_INDEX) ? TRUE : FALSE);

        qosUpdateWMMParametersAndAssignAllowedACI(prAdapter, &prBssInfo->rWmmInfo);
        ASSERT(0); /* NOTE(Kevin): Not test yet */
    }
    else {
        nicTxNonQoSAssignDefaultAdmittedTXQ(prAdapter);

        nicTxNonQoSUpdateTXQParameters(prAdapter,
                                       prBssInfo->ePhyType);
    }

    //4 <6.C> Setup IBSS' Frequency(Band/Channel)
    nicSwitchChannel(prAdapter, prBssInfo->eBand, prBssInfo->ucChnl, 0);


    //4 <7> Update Peer STA_RECORD_T
    {
        UINT_16 u2OperationalRateSet, u2DesiredRateSet;


        //4 <7.A> Add a STA Record first.
        prStaRec = staRecGetStaRecordByAddrOrAddIfNotExist(prAdapter,
                       prBssDesc->aucSrcAddr);

        /* **NOTE(Kevin): This is not the default entry(BSSID), we won't use STATE_3 to avoid
         * been clean up by aging func.
         */

        /* Update received RCPI */
        prStaRec->rRcpi = prBssDesc->rRcpi;

        /* Update the record activity time. */
        GET_CURRENT_SYSTIME(&prStaRec->rUpdateTime);


        //4 <7.B> Desired Rate Set, use Peer's OperationalRateSet for AdHoc entry
        u2OperationalRateSet = (rPhyAttributes[prBssInfo->ePhyType].u2SupportedRateSet &
                                prBssDesc->u2OperationalRateSet);

        u2DesiredRateSet = (u2OperationalRateSet & prConnSettings->u2DesiredRateSet);
        if (u2DesiredRateSet) {
            prStaRec->u2DesiredRateSet = u2DesiredRateSet;
        }
        else {
            /* For Error Handling - The Desired Rate Set is not covered in Operational Rate Set. */
            prStaRec->u2DesiredRateSet = u2OperationalRateSet;
        }

        /* Try to set the best initial rate for this entry */
        if (!rateGetBestInitialRateIndex(prStaRec->u2DesiredRateSet,
                                         prStaRec->rRcpi,
                                         &prStaRec->ucCurrRate1Index)) {

            if (!rateGetLowestRateIndexFromRateSet(prStaRec->u2DesiredRateSet,
                                                   &prStaRec->ucCurrRate1Index)) {
                ASSERT(0);
            }
        }

        DBGLOG(MGT, TRACE, ("Initial Rate Index for entry ["MACSTR"] = %d\n",
            MAC2STR(prStaRec->aucMacAddr), prStaRec->ucCurrRate1Index));

        //4 <7.C> Preamble Mode
        prStaRec->fgIsShortPreambleOptionEnable =
            prBssInfo->fgUseShortPreamble;

        //4 <7.D> QoS Flag
        prStaRec->fgIsQoS = prBssInfo->fgIsWmmAssoc;

        //4 <7.E> Update WLAN Table for default Entry.
        if (nicSetHwBySta(prAdapter, prStaRec) == FALSE) {
            ASSERT(FALSE);
        }

        //4 <7.F> Update Desired Rate Set for BT.
#if CFG_TX_FRAGMENT
        if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
            txRateSetInitForBT(prAdapter, prStaRec);
        }
#endif /* CFG_TX_FRAGMENT */


        //4 <7.G> Legacy Flag
        prStaRec->fgIsLegacy =
            (prBssDesc->ePhyType == PHY_TYPE_HR_DSSS_INDEX) ? TRUE : FALSE;
    }


//4 <8> Start to sending Beacons.
    nicStartBeacon(prAdapter);


//4 <9> Indicate media status connect.
    if (prAdapter->eConnectionState != MEDIA_STATE_CONNECTED) {

        prAdapter->eConnectionState = MEDIA_STATE_CONNECTED;
        prAdapter->eConnectionStateIndicated = MEDIA_STATE_CONNECTED;

        kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
            WLAN_STATUS_MEDIA_CONNECT,
            (PVOID)0,
            0);
        }

    return rStatus;

} /* end of ibssMergeIBSS() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ibssStopIBSS (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);


    nicStopBeacon(prAdapter);

    return;
} /* end of ibssStopIBSS() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
ibssLeave (
    IN P_ADAPTER_T prAdapter
    )
{
    P_BSS_INFO_T prBssInfo;


    ASSERT(prAdapter);
    prBssInfo = &prAdapter->rBssInfo;

#if 0 /* CR1790: only remove this record when beacon lost is confirmed */
    //4 <1> Remove current BSS' BSS_DESC
    /* Because there is no any association process for Ad-Hoc, thus we should remove
     * this Scan Record immediately and scan for a new IBSS after back to SEARCH
     * State.
     */
    scanRemoveBssDescByBssid(prAdapter, prBssInfo->aucBSSID);
#endif
#if 0
    //4 <1> Flush all BSS_DESC_T including current BSS's BSS_DESC_T
    scanRemoveBssDescsByPolicy(prAdapter, SCAN_RM_POLICY_ENTIRE);
#endif

    //4 <2> Remove all PEER STA_RECORDs
    staRecRemoveStaRecordForIBSS(prAdapter);

    //4 <3> Remove STATE_3 flag of all the associated STA_RECORD_Ts (AP, DLSs)
    if (!prAdapter->fgIsIBSSActive) {
        staRecRemoveStateFlagOfAllStaRecords(prAdapter);
    }

    //4 <3> Stop sending Beacon and change Operation Mode to Infrastructure Mode.
    nicExistIBSS(prAdapter);

    return;
} /* end of ibssLeave() */



