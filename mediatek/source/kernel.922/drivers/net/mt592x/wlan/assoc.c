






#include "precomp.h"

extern PHY_ATTRIBUTE_T rPhyAttributes[];







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ UINT_16
assocBuildCapabilityInfo (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    UINT_16 u2CapInfo;

    DEBUGFUNC("assocBuildCapabilityInfo");

    ASSERT(prAdapter);
    ASSERT(prBssDesc);
    prConnSettings = &prAdapter->rConnSettings;

    /* Set up our requested capabilities. */
    u2CapInfo = CAP_INFO_ESS;
    u2CapInfo |= CAP_CF_STA_NOT_POLLABLE;

    /* 7.3.1.4 */
    if ( (rPhyAttributes[prBssDesc->ePhyType].fgIsShortPreambleOptionImplemented) &&
        ( (prConnSettings->ePreambleType == PREAMBLE_TYPE_SHORT) || /* Short Preamble Option Enable is TRUE */
         ((prConnSettings->ePreambleType == PREAMBLE_TYPE_AUTO) &&
          (prBssDesc->u2CapInfo & CAP_INFO_SHORT_PREAMBLE)) ) ) {

        /* Case I: Implemented == TRUE and Short Preamble Option Enable == TRUE.
         * Case II: Implemented == TRUE and Short Preamble == AUTO (depends on
         *          BSS_DESC_T's capability)
         */
        u2CapInfo |= CAP_INFO_SHORT_PREAMBLE;
    }

    if (rPhyAttributes[prBssDesc->ePhyType].fgIsShortSlotTimeOptionImplemented &&
        prConnSettings->fgIsShortSlotTimeOptionEnable) {
        u2CapInfo |= CAP_INFO_SHORT_SLOT_TIME;
    }

    if (prBssDesc->u2CapInfo & CAP_INFO_PRIVACY)
        u2CapInfo |= CAP_INFO_PRIVACY;

    DBGLOG(JOIN, INFO, ("Compose Capability = 0x%04x for Target BSS ["MACSTR"].\n",
        u2CapInfo, MAC2STR(prBssDesc->aucBSSID)));

    return u2CapInfo;

} /* end of assocBuildCapabilityInfo() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ UINT_16
assocBuildReAssocReqFrameIEs (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc,
    IN PUINT_8 pucBuffer
    )
{
    UINT_8 aucAllSupportedRates[RATE_NUM] = {0};
    UINT_8 ucAllSupportedRatesLen;
    UINT_8 ucSupRatesLen;
    UINT_8 ucExtSupRatesLen;
    UINT_16 u2IeTotalLen = 0;
    UINT_16 u2SupportedRateSet;
    UINT_8 ucIeLength;

    DEBUGFUNC("assocBuildReAssocReqFrameIEs");

    ASSERT(prAdapter);
    ASSERT(prBssDesc);
    ASSERT(pucBuffer);

    /* Fill the SSID element. */
    SSID_IE(pucBuffer)->ucId = ELEM_ID_SSID;

    if (prBssDesc->fgIsHiddenSSID) {
        /* NOTE(Kevin): This is for the case of Passive Scan and the target BSS didn't
         * broadcast SSID on its Beacon Frame.
         */
        COPY_SSID(SSID_IE(pucBuffer)->aucSSID,
                  SSID_IE(pucBuffer)->ucLength,
                  prAdapter->rConnSettings.aucSSID,
                  prAdapter->rConnSettings.ucSSIDLen);

        pucBuffer += ELEM_HDR_LEN + prAdapter->rConnSettings.ucSSIDLen;
        u2IeTotalLen += ELEM_HDR_LEN + prAdapter->rConnSettings.ucSSIDLen;
    }
    else {
        COPY_SSID(SSID_IE(pucBuffer)->aucSSID,
                  SSID_IE(pucBuffer)->ucLength,
                  prBssDesc->aucSSID,
                  prBssDesc->ucSSIDLen);

        pucBuffer += ELEM_HDR_LEN + prBssDesc->ucSSIDLen;
        u2IeTotalLen += ELEM_HDR_LEN + prBssDesc->ucSSIDLen;
    }


    /* NOTE(Kevin 2008/12/19): 16.3.6.3 MLME-ASSOCIATE.indication -
     * SupportedRates - The set of data rates that are supported by the STA
     * that is requesting association.
     * Original(Portable Driver): Only send the Rates that we'll support. 
     * New: Send the Phy Rates if the result of following & operation == NULL.
     */
    //rateGetDataRatesFromRateSet((prBssDesc->u2OperationalRateSet &
    //                             rPhyAttributes[prBssDesc->ePhyType].u2SupportedRateSet),

    u2SupportedRateSet = (prBssDesc->u2OperationalRateSet &
                          rPhyAttributes[prBssDesc->ePhyType].u2SupportedRateSet);

    ASSERT(u2SupportedRateSet);

    if (!u2SupportedRateSet) {
        u2SupportedRateSet = rPhyAttributes[prBssDesc->ePhyType].u2SupportedRateSet;
    }

    rateGetDataRatesFromRateSet(u2SupportedRateSet,
                                   0x0, /*(UINT_16)NULL*/
                                   aucAllSupportedRates,
                                   &ucAllSupportedRatesLen);

    ucSupRatesLen = ((ucAllSupportedRatesLen > ELEM_MAX_LEN_SUP_RATES) ?
                     ELEM_MAX_LEN_SUP_RATES : ucAllSupportedRatesLen);

    ucExtSupRatesLen = ucAllSupportedRatesLen - ucSupRatesLen;


    /* Fill the Supported Rates element. */
    if (ucSupRatesLen) {
        SUP_RATES_IE(pucBuffer)->ucId = ELEM_ID_SUP_RATES;
        SUP_RATES_IE(pucBuffer)->ucLength = ucSupRatesLen;
        kalMemCopy(SUP_RATES_IE(pucBuffer)->aucSupportedRates,
                   aucAllSupportedRates,
                   ucSupRatesLen);

        pucBuffer += ELEM_HDR_LEN + ucSupRatesLen;
        u2IeTotalLen += ELEM_HDR_LEN + ucSupRatesLen;
    }


    /* Fill the Extended Supported Rates element. */
    if (ucExtSupRatesLen) {

        EXT_SUP_RATES_IE(pucBuffer)->ucId = ELEM_ID_EXTENDED_SUP_RATES;
        EXT_SUP_RATES_IE(pucBuffer)->ucLength = ucExtSupRatesLen;

        kalMemCopy(EXT_SUP_RATES_IE(pucBuffer)->aucExtSupportedRates,
                   &aucAllSupportedRates[ucSupRatesLen],
                   ucExtSupRatesLen);

        pucBuffer += ELEM_HDR_LEN + ucExtSupRatesLen;
        u2IeTotalLen += ELEM_HDR_LEN + ucExtSupRatesLen;
    }

#if SUPPORT_WAPI
    /* ASSOC INFO IE ID: 0x68 */
    if (prAdapter->prGlueInfo->u2WapiAssocInfoIESz) {
        kalMemCopy(pucBuffer, prAdapter->prGlueInfo->aucWapiAssocInfoIEs, prAdapter->prGlueInfo->u2WapiAssocInfoIESz /* sizeof(PARAM_WAPI_ASSOC_INFO_T */);
        pucBuffer+= prAdapter->prGlueInfo->u2WapiAssocInfoIESz /* sizeof(PARAM_WAPI_ASSOC_INFO_T */;
        u2IeTotalLen += prAdapter->prGlueInfo->u2WapiAssocInfoIESz;
    }
#endif

    ucIeLength = rsnGenerateWPARSNIE(prAdapter, prBssDesc, pucBuffer);
    u2IeTotalLen += ucIeLength;
    pucBuffer += ucIeLength;

    /* Fill the WMM Information element, if the current AP supports WMM. */
    if (prAdapter->fgIsEnableWMM &&
        (prBssDesc->ucWmmFlag & WMM_FLAG_SUPPORT_WMM)) {


        DBGLOG(MGT, TRACE, ("WMM FLAG: 0x%02x\n", prBssDesc->ucWmmFlag));

        ucIeLength = qosConstructWMMInfoElem(prAdapter, prBssDesc->ucWmmFlag, pucBuffer);

        u2IeTotalLen += ucIeLength;
        pucBuffer += ucIeLength;
    }

    return u2IeTotalLen;

} /* end of assocBuildReAssocReqFrameIEs() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
assocComposeReAssocReqFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc,
    IN PUINT_8 pucBuffer,
    IN BOOLEAN fgIsReAssoc,
    OUT PUINT_16 pu2WlanHeaderLen,
    OUT PUINT_16 pu2WlanBodyLen
    )
{
    P_WLAN_ASSOC_REQ_FRAME_T prAssocFrame;
    PUINT_8 pucIEsBuffer;
    UINT_16 u2FrameCtrl;
    UINT_16 u2CapInfo;
    UINT_16 u2ListenInterval;
    UINT_16 u2WlanBodyLen;

    DEBUGFUNC("assocComposeReAssocReqFrame");

    ASSERT(prAdapter);
    ASSERT(prBssDesc);
    ASSERT(pucBuffer);
    ASSERT(pu2WlanHeaderLen);
    ASSERT(pu2WlanBodyLen);

    prAssocFrame = (P_WLAN_ASSOC_REQ_FRAME_T)pucBuffer;

    //4 <1> Compose the frame header of the (Re)Association Request  frame.
    /* Fill the Frame Control field. */
    if (fgIsReAssoc) {
        u2FrameCtrl = MAC_FRAME_REASSOC_REQ;
    } else {
        u2FrameCtrl = MAC_FRAME_ASSOC_REQ;
    }
    WLAN_SET_FIELD_16(&prAssocFrame->u2FrameCtrl, u2FrameCtrl);

    /* Fill the DA field with Target BSSID. */
    COPY_MAC_ADDR(prAssocFrame->aucDestAddr, prBssDesc->aucBSSID);

    /* Fill the SA field with our MAC Address. */
    COPY_MAC_ADDR(prAssocFrame->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the BSSID field with Target BSSID. */
    COPY_MAC_ADDR(prAssocFrame->aucBSSID, prBssDesc->aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prAssocFrame->u2SeqCtrl = 0;


    //4 <2> Compose the frame body's common fixed field part of the (Re)Association Request  frame.
    u2CapInfo = assocBuildCapabilityInfo(prAdapter, prBssDesc);

    /* Fill the Capability Information field. */
    WLAN_SET_FIELD_16(&prAssocFrame->u2CapInfo, u2CapInfo);


    /* Calculate the listen interval for the maximum power mode. Currently, we
       set it to the value 2 times DTIM period. */
    if (prBssDesc->ucDTIMPeriod) {
        u2ListenInterval = prBssDesc->ucDTIMPeriod * DEFAULT_LISTEN_INTERVAL_BY_DTIM_PERIOD;
    }
    else{
        DBGLOG(MGT, TRACE, ("Use default listen interval\n"));
        u2ListenInterval = DEFAULT_LISTEN_INTERVAL;
    }

    /* Fill the Listen Interval field. */
    WLAN_SET_FIELD_16(&prAssocFrame->u2ListenInterval, u2ListenInterval);


    //4 <3> Update the MAC header/body length.
    *pu2WlanHeaderLen = WLAN_MAC_MGMT_HEADER_LEN;
    u2WlanBodyLen = CAP_INFO_FIELD_LEN +
                    LISTEN_INTERVAL_FIELD_LEN;

    //4 <4> Compose the Current AP Address field for ReAssociation Request  frame.
    /* Fill the Current AP Address field. */
    if (fgIsReAssoc) {
        P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;

        P_WLAN_REASSOC_REQ_FRAME_T prReAssocFrame =
            (P_WLAN_REASSOC_REQ_FRAME_T)prAssocFrame;

        COPY_MAC_ADDR(prReAssocFrame->aucCurrentAPAddr, prBssInfo->aucBSSID);
        u2WlanBodyLen += CURR_AP_ADDR_FIELD_LEN;

        /* Update the Start Address of IEs Buffer */
        pucIEsBuffer = prReAssocFrame->aucInfoElem;
    }
    else {
        /* Update the Start Address of IEs Buffer */
        pucIEsBuffer = prAssocFrame->aucInfoElem;
    }

    //4 <5> Compose the frame body's IEs of the (Re)Association Request  frame.
    u2WlanBodyLen += assocBuildReAssocReqFrameIEs(prAdapter, prBssDesc, pucIEsBuffer);

    //4 <6> Update the Reassociation request information
    kalUpdateReAssocReqInfo(prAdapter->prGlueInfo,
                            (PUINT_8)&prAssocFrame->u2CapInfo,
                            u2WlanBodyLen,
                            fgIsReAssoc);

    *pu2WlanBodyLen = u2WlanBodyLen;

    return;
} /* end of assocComposeReAssocReqFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
assocSendReAssocReqFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_DESC_T prBssDesc,
    IN BOOLEAN fgIsReAssoc
    )
{
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;
    UINT_16 u2WlanHeaderLen;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;
    UINT_32 u4Status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("assocSendReAssocReqFrame");


    ASSERT(prAdapter);
    ASSERT(prBssDesc);

    //4 <1> Allocate MSDU_INFO_T
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
    if (prMsduInfo == (P_MSDU_INFO_T)NULL) {
        DBGLOG(JOIN, WARN, ("No MSDU_INFO_T to send (Re)Association Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }

    //4 <2> Allocate Frame Buffer (in MGT_PACKET_T) for (Re)Association Frame
    //4 whsu: notice WPS and 802.11r IE
#if SUPPORT_WAPI
    if (fgIsReAssoc) {
        u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                              CAP_INFO_FIELD_LEN + \
                              LISTEN_INTERVAL_FIELD_LEN + \
                              CURR_AP_ADDR_FIELD_LEN + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                              (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES)) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WAPI_ASSOC_INFO) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WPA_RSN) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);
    }
    else {
        u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                              CAP_INFO_FIELD_LEN + \
                              LISTEN_INTERVAL_FIELD_LEN + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                              (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES)) + \
                              (ELEM_MAX_LEN_WAPI_ASSOC_INFO) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WPA_RSN) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);
    }
#else
    
    if (fgIsReAssoc) {
        u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                              CAP_INFO_FIELD_LEN + \
                              LISTEN_INTERVAL_FIELD_LEN + \
                              CURR_AP_ADDR_FIELD_LEN + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                              (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES)) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WPA_RSN) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);
    }
    else {
        u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + \
                              CAP_INFO_FIELD_LEN + \
                              LISTEN_INTERVAL_FIELD_LEN + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SSID) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_SUP_RATES) + \
                              (ELEM_HDR_LEN + (RATE_NUM - ELEM_MAX_LEN_SUP_RATES)) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WPA_RSN) + \
                              (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);
    }
#endif
    /* TODO(Kevin): Update u2EstimatedFrameLen according WPA/RSN/WMM(ok)/WMMSA status */


    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, (UINT_32)u2EstimatedFrameLen);
    if (prMgtPacket == (P_MGT_PACKET_T)NULL) {
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        DBGLOG(JOIN, WARN, ("No buffer to send (Re)Association Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }


    //4 <3> Compose Association Request frame in MGT_PACKET_T.
    assocComposeReAssocReqFrame(prAdapter,
                                prBssDesc,
                                MGT_PACKET_GET_BUFFER(prMgtPacket),
                                fgIsReAssoc,
                                &u2WlanHeaderLen,
                                &u2WlanBodyLen);

    //4 <4> Update the frame length to the packet descriptor (MGT_PACKET_T).
    mgtPacketPut(prMgtPacket, (u2WlanHeaderLen + u2WlanBodyLen));

    //4 <5> Update information in MSDU_INFO_T for TX Module.
    /* Bcz JOIN FSM will handle this frame, so set with lifetime never expire */
    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                       TRUE, \
                       TRUE, \
                       (PVOID)prMgtPacket, \
                       0, \
                       TXQ_TCM, \
                       (UINT_8)u2WlanHeaderLen, \
                       u2WlanBodyLen, \
                       (MSDU_INFO_CTRL_FLAG_SPECIFY_AC | \
                        MSDU_INFO_CTRL_FLAG_BASIC_RATE | \
                        MSDU_INFO_CTRL_FLAG_LIFETIME_NEVER_EXPIRE), \
                       arbFsmRunEventJoinTxDone, \
                       (OS_SYSTIME)NULL, \
                       NULL \
                       );

    //4 <6> Inform ARB to send this Authentication Request frame.
    DBGLOG(JOIN, LOUD, ("Send Assocation Request frame\n"));
    /* TODO(Kevin): We should handle the WLAN_STATUS_RESOURCE & WLAN_STATUS_FAILURE
     *       separately for BUS_ERROR.
     */
    if ((u4Status = arbFsmRunEventTxMmpdu(prAdapter,prMsduInfo)) != WLAN_STATUS_PENDING) {

        if(u4Status != WLAN_STATUS_SUCCESS){
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            DBGLOG(JOIN, ERROR, ("Send Auth Request frame fail.\n"));

            return WLAN_STATUS_FAILURE;
        }
    }

    return WLAN_STATUS_SUCCESS;

} /* end of assocSendReAssocReqFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
assocComposeDisAssocFrame (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2ReasonCode,
    OUT PUINT_16 pu2WlanHeaderLen,
    OUT PUINT_16 pu2WlanBodyLen
    )
{
    P_WLAN_DISASSOC_FRAME_T prDisAssocFrame;
    UINT_16 u2FrameCtrl;
    UINT_16 u2WlanBodyLen;


    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(aucBSSID);
    ASSERT(pu2WlanHeaderLen);
    ASSERT(pu2WlanBodyLen);

    prDisAssocFrame = (P_WLAN_DISASSOC_FRAME_T)pucBuffer;

    //4 <1> Compose the frame header of the DisAssociation  frame.
    /* Fill the Frame Control field. */
    u2FrameCtrl = MAC_FRAME_DISASSOC;

    WLAN_SET_FIELD_16(&prDisAssocFrame->u2FrameCtrl, u2FrameCtrl);

    /* Fill the DA field with Target BSSID. */
    COPY_MAC_ADDR(prDisAssocFrame->aucDestAddr, aucBSSID);

    /* Fill the SA field with our MAC Address. */
    COPY_MAC_ADDR(prDisAssocFrame->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the BSSID field with Target BSSID. */
    COPY_MAC_ADDR(prDisAssocFrame->aucBSSID, aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prDisAssocFrame->u2SeqCtrl = 0;

    //4 <2> Compose the frame body's fixed field part of the Disassociation frame.
    /* Fill the Reason Code field. */
    WLAN_SET_FIELD_16(&prDisAssocFrame->u2ReasonCode, u2ReasonCode);

    //4 <3> Update the MAC header/body length.
    *pu2WlanHeaderLen = WLAN_MAC_MGMT_HEADER_LEN;
    u2WlanBodyLen = REASON_CODE_FIELD_LEN;

    *pu2WlanBodyLen = u2WlanBodyLen;

    return;
} /* end of assocComposeDisAssocFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
assocSendDisAssocFrame (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2ReasonCode,
    IN PFN_TX_DONE_HANDLER pfTxDoneHandler
    )
{
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;
    UINT_16 u2WlanHeaderLen;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;
    UINT_32 u4Status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("assocSendDisAssocFrame");


    ASSERT(prAdapter);
    ASSERT(aucBSSID);

    //4 <1> Allocate MSDU_INFO_T
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
    if (prMsduInfo == (P_MSDU_INFO_T)NULL) {
        DBGLOG(JOIN, WARN, ("No MSDU_INFO_T to send Disassociation frame.\n"));
        return WLAN_STATUS_RESOURCES;
    }

    //4 <2> Allocate Frame Buffer (in MGT_PACKET_T) for (Re)Association Frame
    //4 whsu: notice WPS and 802.11r IE
    u2EstimatedFrameLen = WLAN_MAC_MGMT_HEADER_LEN + REASON_CODE_FIELD_LEN;

    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, (UINT_32)u2EstimatedFrameLen);
    if (prMgtPacket == (P_MGT_PACKET_T)NULL) {
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        DBGLOG(JOIN, WARN, ("No buffer to send Disassociation Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }


    //4 <3> Compose Association Request frame in MGT_PACKET_T.
    assocComposeDisAssocFrame(prAdapter,
                              MGT_PACKET_GET_BUFFER(prMgtPacket),
                              aucBSSID,
                              u2ReasonCode,
                              &u2WlanHeaderLen,
                              &u2WlanBodyLen);

    //4 <4> Update the frame length to the packet descriptor (MGT_PACKET_T).
    mgtPacketPut(prMgtPacket, (u2WlanHeaderLen + u2WlanBodyLen));

    //4 <5> Update information in MSDU_INFO_T for TX Module.
    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                       TRUE, \
                       TRUE, \
                       (PVOID)prMgtPacket, \
                       0, \
                       TXQ_AC3, \
                       (UINT_8)u2WlanHeaderLen, \
                       u2WlanBodyLen, \
                       (MSDU_INFO_CTRL_FLAG_SPECIFY_AC | \
                        MSDU_INFO_CTRL_FLAG_BASIC_RATE | \
                        MSDU_INFO_CTRL_FLAG_LIFETIME_NEVER_EXPIRE), \
                       pfTxDoneHandler, \
                       (OS_SYSTIME)NULL, \
                       NULL \
                       );

    //4 <6> Inform ARB to send this Disassociation frame.
    DBGLOG(JOIN, LOUD, ("Send DisAssocation frame\n"));
    /* TODO(Kevin): We should handle the WLAN_STATUS_RESOURCE & WLAN_STATUS_FAILURE
     *       separately for BUS_ERROR.
     */
    if ((u4Status = arbFsmRunEventTxMmpdu(prAdapter,prMsduInfo)) != WLAN_STATUS_PENDING) {

        if(u4Status != WLAN_STATUS_SUCCESS){
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            DBGLOG(JOIN, ERROR, ("Send Disassociation frame fail.\n"));

            return WLAN_STATUS_FAILURE;
        }

    }

    return WLAN_STATUS_SUCCESS;

} /* end of assocSendDisAssocFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
assocCheckTxReAssocReqFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN BOOLEAN fgIsReAssoc
    )
{
    P_WLAN_ASSOC_REQ_FRAME_T prAssocFrame;
    UINT_16 u2TxFrameCtrl;

    DEBUGFUNC("assocCheckTxReAssocReqFrame");

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    
    if (prMsduInfo->fgIsFromInternalProtocolStack) {
        prAssocFrame = (P_WLAN_ASSOC_REQ_FRAME_T)(MGT_PACKET_GET_BUFFER(prMsduInfo->pvPacket));
    }
    else {
        DBGLOG(JOIN, WARN,
            ("An Association Request Frame is not from Internal Protocol Stack.\n"));
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }


    WLAN_GET_FIELD_16(&prAssocFrame->u2FrameCtrl, &u2TxFrameCtrl)
    u2TxFrameCtrl &= MASK_FRAME_TYPE;
    if (fgIsReAssoc) {
        if (u2TxFrameCtrl != MAC_FRAME_REASSOC_REQ) {
            return WLAN_STATUS_FAILURE;
        }
    }
    else {
        if (u2TxFrameCtrl != MAC_FRAME_ASSOC_REQ) {
            return WLAN_STATUS_FAILURE;
        }
    }

    return WLAN_STATUS_SUCCESS;

} /* end of assocCheckTxReAssocReqFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
assocProcessRxReAssocRspFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN P_PEER_BSS_INFO_T prPeerBssInfo,
    IN BOOLEAN fgIsReAssoc,
    OUT PUINT_16 pu2StatusCode
    )
{
    P_WLAN_ASSOC_RSP_FRAME_T prAssocRspFrame;
    P_IE_SUPPORTED_RATE_T prIeSupportedRate = (P_IE_SUPPORTED_RATE_T)NULL;
    P_IE_EXT_SUPPORTED_RATE_T prIeExtSupportedRate = (P_IE_EXT_SUPPORTED_RATE_T)NULL;
    UINT_16 u2RxFrameCtrl;
    UINT_16 u2RxCapInfo;
    UINT_16 u2RxStatusCode;
    UINT_16 u2RxAssocId;
    UINT_16 u2IEsLen;
    PUINT_8 pucIEsBuffer;
    UINT_16 u2Offset;

    DEBUGFUNC("assocProcessRxReAssocRspFrame");


    ASSERT(prAdapter);
    ASSERT(prSwRfb);
    ASSERT(prPeerBssInfo);
    ASSERT(pu2StatusCode);

    if (prSwRfb->u2FrameLength < (WLAN_MAC_MGMT_HEADER_LEN +
                                  CAP_INFO_FIELD_LEN +
                                  STATUS_CODE_FIELD_LEN +
                                  AID_FIELD_LEN)) {
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }

    DBGLOG(JOIN, LOUD, ("prSwRfb->u2FrameLength = %d\n", prSwRfb->u2FrameLength));

    //4 <1> locate the (Re)Assocation Resp Frame.
    prAssocRspFrame = (P_WLAN_ASSOC_RSP_FRAME_T) prSwRfb->pvHeader;

    //4 <2> Parse the Header of (Re)Assocation Resp Frame.
    WLAN_GET_FIELD_16(&prAssocRspFrame->u2FrameCtrl, &u2RxFrameCtrl);
    u2RxFrameCtrl &= MASK_FRAME_TYPE;
    if (fgIsReAssoc) {
        if (u2RxFrameCtrl != MAC_FRAME_REASSOC_RSP) {
            return WLAN_STATUS_FAILURE;
        }
    }
    else {
        if (u2RxFrameCtrl != MAC_FRAME_ASSOC_RSP) {
            return WLAN_STATUS_FAILURE;
        }
    }

    /* Check if this Assoc Resp Frame is coming from Target BSSID */
    if (UNEQUAL_MAC_ADDR(prAssocRspFrame->aucSrcAddr, prPeerBssInfo->aucBSSID)) {
        DBGLOG(JOIN, TRACE, ("Ignore Assoc Resp Frame from other BSS ["MACSTR"]\n",
            MAC2STR(prAssocRspFrame->aucSrcAddr)));
        return WLAN_STATUS_FAILURE;
    }

    //4 <3> Parse the Fixed Fields of (Re)Assocation Resp Frame Body.
    WLAN_GET_FIELD_16(&prAssocRspFrame->u2StatusCode, &u2RxStatusCode);
    *pu2StatusCode = u2RxStatusCode;

    //4 <4> Ignore CAP_INFO, AID & IEs part if the Status Code is not SUCCESSFUL.
    if (u2RxStatusCode != STATUS_CODE_SUCCESSFUL) {

        DBGLOG(JOIN, LOUD, ("Assoc Failure - Status Code = %d.\n", u2RxStatusCode));

        /* NOTE(Kevin): We should return SUCCESS to indicate that the current frame
         * is parsed OK, the Status Code will be handled by JOIN FSM.
         */
        return WLAN_STATUS_SUCCESS;
    }

    //4 <5> Parse the Fixed Fields of (Re)Assocation Resp Frame Body.
    WLAN_GET_FIELD_16(&prAssocRspFrame->u2CapInfo, &u2RxCapInfo);
    if (u2RxCapInfo & CAP_INFO_PRIVACY) {
        prPeerBssInfo->fgIsPrivacyEnabled = TRUE;
    }
    else {
        prPeerBssInfo->fgIsPrivacyEnabled = FALSE;
    }

    if (u2RxCapInfo & CAP_INFO_SHORT_PREAMBLE) {
        prPeerBssInfo->fgIsShortPreambleAllowed = TRUE;
    }
    else {
        prPeerBssInfo->fgIsShortPreambleAllowed = FALSE;
    }

    if (u2RxCapInfo & CAP_INFO_SHORT_SLOT_TIME) {
        prPeerBssInfo->fgUseShortSlotTime = TRUE;
    }
    else {
        prPeerBssInfo->fgUseShortSlotTime = FALSE;
    }

    WLAN_GET_FIELD_16(&prAssocRspFrame->u2AssocId, &u2RxAssocId);
    /* Note: We didn't remove the AID MSB */
    if (u2RxAssocId < (AID_MIN_VALUE | AID_MSB) ||
        u2RxAssocId > (AID_MAX_VALUE | AID_MSB)) {
        DBGLOG(JOIN, WARN, ("Invalid AID: 0x%04x\n", u2RxAssocId));
    }
    prPeerBssInfo->u2AssocId = u2RxAssocId & AID_MASK;



    //4 <5> Parse the Information Elements of (Re)Assocation Resp Frame Body.
    pucIEsBuffer = &prAssocRspFrame->aucInfoElem[0];
    u2IEsLen = prSwRfb->u2FrameLength - (WLAN_MAC_MGMT_HEADER_LEN +
                                         CAP_INFO_FIELD_LEN +
                                         STATUS_CODE_FIELD_LEN +
                                         AID_FIELD_LEN);

    IE_FOR_EACH(pucIEsBuffer, u2IEsLen, u2Offset) {
        switch (IE_ID(pucIEsBuffer)) {
        case ELEM_ID_SUP_RATES:
            prIeSupportedRate = SUP_RATES_IE(pucIEsBuffer);
            break;

        case ELEM_ID_EXTENDED_SUP_RATES:
            prIeExtSupportedRate = EXT_SUP_RATES_IE(pucIEsBuffer);
            break;

        case ELEM_ID_VENDOR:
            /* NOTE(Kevin): Skip parse WMM in order to save some times if we know
             * we didn't enable the WMM at driver side.
             */
            if (prAdapter->fgIsEnableWMM) {
                UINT_8 ucOuiType;
                UINT_16 u2SubTypeVersion;

                if (parseCheckForWFAInfoElem(pucIEsBuffer, &ucOuiType, &u2SubTypeVersion)) {
                    /* Parse WMM capability */
                    if ((ucOuiType == VENDOR_OUI_TYPE_WMM) &&
                        (u2SubTypeVersion == VENDOR_OUI_SUBTYPE_VERSION_WMM_PARAM)) {

                        if (!qosParseWMMParamElem(pucIEsBuffer, &prPeerBssInfo->rWmmInfo)) {
                            /* Clear the WMM capability if parse error */
                            prPeerBssInfo->rWmmInfo.ucWmmFlag = 0;
                        }
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    rateGetRateSetFromIEs(prIeSupportedRate,
                          prIeExtSupportedRate,
                          &prPeerBssInfo->u2OperationalRateSet,
                          &prPeerBssInfo->u2BSSBasicRateSet,
                          &prPeerBssInfo->fgIsUnknownBSSBasicRate);

    if (prPeerBssInfo->fgIsUnknownBSSBasicRate) {
        DBGLOG(JOIN, WARN, ("BSS has unknown BSS Basic Rate: Operational Rate Set = %04x, BSS Basic Rate Set = %04x\n",
            prPeerBssInfo->u2OperationalRateSet, prPeerBssInfo->u2BSSBasicRateSet));
    }


    /* TODO(Kevin): put the information into GLUE Layer ? */
    /* Update the information in the structure used to query and set
       OID_802_11_ASSOCIATION_INFORMATION. */
    kalUpdateReAssocRspInfo(prAdapter->prGlueInfo,
                           (PUINT_8)&prAssocRspFrame->u2CapInfo,
                           (UINT_32)(prSwRfb->u2FrameLength - WLAN_MAC_MGMT_HEADER_LEN));

    return WLAN_STATUS_SUCCESS;

} /* end of assocProcessRxReAssocRspFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
assocProcessRxDisassocFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN UINT_8 aucBSSID[],
    OUT PUINT_16 pu2ReasonCode
    )
{
    P_WLAN_DISASSOC_FRAME_T prDisassocFrame;
    UINT_16 u2RxFrameCtrl;
    UINT_16 u2RxReasonCode;

    DEBUGFUNC("assocProcessRxDisassocFrame");

    ASSERT(prAdapter);
    ASSERT(prSwRfb);
    ASSERT(aucBSSID);
    ASSERT(pu2ReasonCode);

    //4 <1> locate the Deauthentication Frame.
    prDisassocFrame = (P_WLAN_DISASSOC_FRAME_T) prSwRfb->pvHeader;

    //4 <2> Parse the Header of Authentication Frame.
    WLAN_GET_FIELD_16(&prDisassocFrame->u2FrameCtrl, &u2RxFrameCtrl)
    u2RxFrameCtrl &= MASK_FRAME_TYPE;
    if (u2RxFrameCtrl != MAC_FRAME_DISASSOC) {
        return WLAN_STATUS_FAILURE;
    }

    if (prSwRfb->u2FrameLength < (WLAN_MAC_MGMT_HEADER_LEN +
                                  REASON_CODE_FIELD_LEN)) {
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }

    /* Check if this Disassoc Frame is coming from Target BSSID */
    if (UNEQUAL_MAC_ADDR(prDisassocFrame->aucSrcAddr, aucBSSID)) {
        DBGLOG(JOIN, LOUD, ("Ignore Disassoc Frame from other BSS ["MACSTR"]\n",
            MAC2STR(prDisassocFrame->aucSrcAddr)));
        return WLAN_STATUS_FAILURE;
    }

    //4 <3> Parse the Fixed Fields of Deauthentication Frame Body.
    WLAN_GET_FIELD_16(&prDisassocFrame->u2ReasonCode, &u2RxReasonCode);
    *pu2ReasonCode = u2RxReasonCode;

    return WLAN_STATUS_SUCCESS;

} /* end of assocProcessRxDisassocFrame() */

