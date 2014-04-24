






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
authComposeAuthFrame (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum,
    IN UINT_16 u2StatusCode,
    IN PUINT_8 pucChallengeText,
    IN UINT_8 ucChallengeTextLen,
    OUT PUINT_16 pu2WlanHeaderLen,
    OUT PUINT_16 pu2WlanBodyLen
    )
{
    P_WLAN_AUTH_FRAME_T prAuthFrame;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2FrameCtrl;


    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(aucBSSID);
    ASSERT(pucChallengeText);
    ASSERT(pu2WlanHeaderLen);
    ASSERT(pu2WlanBodyLen);

    prAuthFrame = (P_WLAN_AUTH_FRAME_T)pucBuffer;

    //4 <1> Compose the frame header of the Authentication frame.
    /* Fill the Frame Control field. */
    u2FrameCtrl = MAC_FRAME_AUTH;

    /* If this frame is the third frame in the shared key authentication
     * sequence, it shall be encrypted.
     */
    if ((u2AuthAlgNum == AUTH_ALGORITHM_NUM_SHARED_KEY) &&
        (u2TransactionSeqNum == AUTH_TRANSACTION_SEQ_3) &&
        (pucChallengeText != NULL) &&
        (ucChallengeTextLen > 0)) {

        u2FrameCtrl |= MASK_FC_PROTECTED_FRAME; /* HW will also detect this bit for applying encryption */
    }
    WLAN_SET_FIELD_16(&prAuthFrame->u2FrameCtrl, u2FrameCtrl);

    /* Fill the DA field with Target BSSID. */
    COPY_MAC_ADDR(prAuthFrame->aucDestAddr, aucBSSID);

    /* Fill the SA field with our MAC Address. */
    COPY_MAC_ADDR(prAuthFrame->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the BSSID field with Target BSSID. */
    COPY_MAC_ADDR(prAuthFrame->aucBSSID, aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prAuthFrame->u2SeqCtrl = 0;


    //4 <2> Compose the frame body's fixed field part of the Authentication frame.
    /* Fill the Authentication Algorithm Number field. */
    WLAN_SET_FIELD_16(&prAuthFrame->u2AuthAlgNum, u2AuthAlgNum);

    /* Fill the Authentication Transaction Sequence Number field. */
    WLAN_SET_FIELD_16(&prAuthFrame->u2AuthTransSeqNo, u2TransactionSeqNum);

    /* Fill the Status Code field. */
    WLAN_SET_FIELD_16(&prAuthFrame->u2StatusCode, u2StatusCode);

    //4 <3> Update the MAC header/body length.
    *pu2WlanHeaderLen = WLAN_MAC_MGMT_HEADER_LEN;
    u2WlanBodyLen = AUTH_ALGORITHM_NUM_FIELD_LEN +
                    AUTH_TRANSACTION_SEQENCE_NUM_FIELD_LEN +
                    STATUS_CODE_FIELD_LEN;


    //4 <4> Compose the frame body's IEs part of the Authentication frame.
    /* Fill the Challenge Text field if the challenge text is provided by the
       caller. */
    if ((pucChallengeText != NULL) && (ucChallengeTextLen > 0)) {
        P_IE_CHALLENGE_TEXT_T prIeChallengeText =
            (P_IE_CHALLENGE_TEXT_T)&prAuthFrame->aucInfoElem[0];

        ASSERT(ucChallengeTextLen <= ELEM_MAX_LEN_CHALLENGE_TEXT);

        prIeChallengeText->ucId = ELEM_ID_CHALLENGE_TEXT;
        prIeChallengeText->ucLength = ucChallengeTextLen;

        kalMemCopy(prIeChallengeText->aucChallengeText,
                   pucChallengeText,
                   ucChallengeTextLen);

        u2WlanBodyLen += ELEM_HDR_LEN + (UINT_16)ucChallengeTextLen;
    }

    *pu2WlanBodyLen = u2WlanBodyLen;

    return;
} /* end of authComposeAuthFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
authSendAuthFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_PEER_BSS_INFO_T prPeerBssInfo,
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum
    )
{
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;

    UINT_16 u2WlanHeaderLen;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;
    UINT_32 u4Status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("authSendAuthFrame");


    ASSERT(prAdapter);
    ASSERT(prPeerBssInfo);

    //4 <1> Allocate MSDU_INFO_T
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, TCM);
    if (prMsduInfo == (P_MSDU_INFO_T)NULL) {
        DBGLOG(JOIN, WARN, ("No MSDU_INFO_T to send Auth Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }

    //4 <2> Allocate Frame Buffer (in MGT_PACKET_T) for Authentication Frame
    /* Only consider SEQ_1 & SEQ_3 */
    if ((u2AuthAlgNum == AUTH_ALGORITHM_NUM_SHARED_KEY) &&
        (u2TransactionSeqNum == AUTH_TRANSACTION_SEQ_3)) {
        u2EstimatedFrameLen = (WLAN_MAC_MGMT_HEADER_LEN +
                               AUTH_ALGORITHM_NUM_FIELD_LEN +
                               AUTH_TRANSACTION_SEQENCE_NUM_FIELD_LEN +
                               STATUS_CODE_FIELD_LEN +
                               (ELEM_HDR_LEN + ELEM_MAX_LEN_CHALLENGE_TEXT));
    }
    else {
        u2EstimatedFrameLen = (WLAN_MAC_MGMT_HEADER_LEN +
                               AUTH_ALGORITHM_NUM_FIELD_LEN +
                               AUTH_TRANSACTION_SEQENCE_NUM_FIELD_LEN +
                               STATUS_CODE_FIELD_LEN);
    }

    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, (UINT_32)u2EstimatedFrameLen);
    if (prMgtPacket == (P_MGT_PACKET_T)NULL) {
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        DBGLOG(JOIN, WARN, ("No buffer to send Auth Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }


    //4 <3> Compose Authentication Request frame in MGT_PACKET_T.
    authComposeAuthFrame(prAdapter,
                         MGT_PACKET_GET_BUFFER(prMgtPacket),
                         prPeerBssInfo->aucBSSID,
                         u2AuthAlgNum,
                         u2TransactionSeqNum,
                         STATUS_CODE_SUCCESSFUL,
                         prPeerBssInfo->rIeChallengeText.aucChallengeText,
                         prPeerBssInfo->rIeChallengeText.ucLength,
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
    DBGLOG(JOIN, LOUD, ("Send Auth Request frame\n"));
    /* TODO(Kevin): We should handle the WLAN_STATUS_RESOURCE & WLAN_STATUS_FAILURE
     *       separately for BUS_ERROR.
     */
    if ( (u4Status = arbFsmRunEventTxMmpdu(prAdapter,prMsduInfo)) != WLAN_STATUS_PENDING) {

        if(u4Status != WLAN_STATUS_SUCCESS) {
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            DBGLOG(JOIN, ERROR, ("Send Auth Request frame fail.\n"));

            return WLAN_STATUS_FAILURE;

        }

    }

    return WLAN_STATUS_SUCCESS;

} /* end of authSendAuthFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ VOID
authComposeDeauthFrame (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2ReasonCode,
    OUT PUINT_16 pu2WlanHeaderLen,
    OUT PUINT_16 pu2WlanBodyLen
    )
{
    P_WLAN_DEAUTH_FRAME_T prDeauthFrame;
    UINT_16 u2FrameCtrl;


    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(aucBSSID);
    ASSERT(pu2WlanHeaderLen);
    ASSERT(pu2WlanBodyLen);

    prDeauthFrame = (P_WLAN_DEAUTH_FRAME_T)pucBuffer;

    //4 <1> Compose the frame header of the Deauthentication frame.
    /* Fill the Frame Control field. */
    u2FrameCtrl = MAC_FRAME_DEAUTH;
    WLAN_SET_FIELD_16(&prDeauthFrame->u2FrameCtrl, u2FrameCtrl);

    /* Fill the DA field with Target BSSID. */
    COPY_MAC_ADDR(prDeauthFrame->aucDestAddr, aucBSSID);

    /* Fill the SA field with our MAC Address. */
    COPY_MAC_ADDR(prDeauthFrame->aucSrcAddr, prAdapter->aucMacAddress);

    /* Fill the BSSID field with Target BSSID. */
    COPY_MAC_ADDR(prDeauthFrame->aucBSSID, aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prDeauthFrame->u2SeqCtrl = 0;


    //4 <2> Compose the frame body's fixed field part of the Authentication frame.
    /* Fill the Status Code field. */
    WLAN_SET_FIELD_16(&prDeauthFrame->u2ReasonCode, u2ReasonCode);

    //4 <3> Update the MAC header/body length.
    *pu2WlanHeaderLen = WLAN_MAC_MGMT_HEADER_LEN;
    *pu2WlanBodyLen = REASON_CODE_FIELD_LEN;

    return;
} /* end of authComposeDeauthFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
authSendDeauthFrame (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucBSSID[],
    IN UINT_16 u2ReasonCode,
    IN UINT_8 ucTQ,
    IN PFN_TX_DONE_HANDLER pfTxDoneHandler
    )
{
    P_MSDU_INFO_T prMsduInfo;
    P_MGT_PACKET_T prMgtPacket;

    UINT_16 u2WlanHeaderLen;
    UINT_16 u2WlanBodyLen;
    UINT_16 u2EstimatedFrameLen;
    UINT_32 u4Status = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("authSendDeauthFrame");


    ASSERT(prAdapter);
    ASSERT(aucBSSID);

    //4 <1> Allocate MSDU_INFO_T
    prMsduInfo = nicTxAllocMsduInfo(prAdapter, ucTQ);
    if (prMsduInfo == (P_MSDU_INFO_T)NULL) {
        DBGLOG(JOIN, WARN, ("No MSDU_INFO_T to send Deauth Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }

    //4 <2> Allocate Frame Buffer (in MGT_PACKET_T) for Deauthentication Frame
    u2EstimatedFrameLen = (WLAN_MAC_MGMT_HEADER_LEN + REASON_CODE_FIELD_LEN);

    prMgtPacket = mgtBufAllocateMgtPacket(prAdapter, (UINT_32)u2EstimatedFrameLen);
    if (prMgtPacket == (P_MGT_PACKET_T)NULL) {
        nicTxReturnMsduInfo(prAdapter, prMsduInfo);
        DBGLOG(JOIN, WARN, ("No buffer to send Deauth Request.\n"));
        return WLAN_STATUS_RESOURCES;
    }

    //4 <3> Compose Deauthentication Request frame in MGT_PACKET_T.
    authComposeDeauthFrame(prAdapter,
                           MGT_PACKET_GET_BUFFER(prMgtPacket),
                           aucBSSID,
                           u2ReasonCode,
                           &u2WlanHeaderLen,
                           &u2WlanBodyLen);

    //4 <4> Update the frame length to the packet descriptor (MGT_PACKET_T).
    mgtPacketPut(prMgtPacket, (u2WlanHeaderLen + u2WlanBodyLen));


    //4 <5> Update information in MSDU_INFO_T for TX Module.
    //4 <Todo> The callback function should become argument for module
    MSDU_INFO_OBJ_INIT(prMsduInfo, \
                       TRUE, \
                       TRUE, \
                       (PVOID)prMgtPacket, \
                       0, \
                       ucTQ, \
                       (UINT_8)u2WlanHeaderLen, \
                       u2WlanBodyLen, \
                       (MSDU_INFO_CTRL_FLAG_SPECIFY_AC | \
                        MSDU_INFO_CTRL_FLAG_BASIC_RATE | \
                        MSDU_INFO_CTRL_FLAG_LIFETIME_NEVER_EXPIRE), \
                       pfTxDoneHandler, \
                       (OS_SYSTIME)NULL, \
                       NULL \
                       );

    //4 <6> Inform ARB to send this Deauthentication Request frame.
    DBGLOG(JOIN, LOUD, ("Send Deauth Request frame\n"));
    /* TODO(Kevin): We should handle the WLAN_STATUS_RESOURCE & WLAN_STATUS_FAILURE
     *       separately for BUS_ERROR.
     */
    if ((u4Status = arbFsmRunEventTxMmpdu(prAdapter,prMsduInfo)) != WLAN_STATUS_PENDING) {

        if(u4Status != WLAN_STATUS_SUCCESS) {
            mgtBufFreeMgtPacket(prAdapter, prMgtPacket);

            nicTxReturnMsduInfo(prAdapter, prMsduInfo);

            DBGLOG(JOIN, ERROR, ("Send Deauth Request frame fail.\n"));

            return WLAN_STATUS_FAILURE;
        }


    }

    return WLAN_STATUS_SUCCESS;

} /* end of authSendDeauthFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
authCheckTxAuthFrame (
    IN P_ADAPTER_T  prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum
    )
{
    P_WLAN_AUTH_FRAME_T prAuthFrame;
    UINT_16 u2TxFrameCtrl;
    UINT_16 u2TxAuthAlgNum;
    UINT_16 u2TxTransactionSeqNum;

    DEBUGFUNC("authCheckTxAuthFrame");


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);

    if (prMsduInfo->fgIsFromInternalProtocolStack) {
        prAuthFrame = (P_WLAN_AUTH_FRAME_T)(MGT_PACKET_GET_BUFFER(prMsduInfo->pvPacket));
    }
    else {
        DBGLOG(JOIN, WARN,
            ("An Authentication Frame is not from Internal Protocol Stack.\n"));
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }

    WLAN_GET_FIELD_16(&prAuthFrame->u2FrameCtrl, &u2TxFrameCtrl)
    u2TxFrameCtrl &= MASK_FRAME_TYPE;
    if (u2TxFrameCtrl != MAC_FRAME_AUTH) {
        return WLAN_STATUS_FAILURE;
    }

    WLAN_GET_FIELD_16(&prAuthFrame->u2AuthAlgNum, &u2TxAuthAlgNum)
    if (u2TxAuthAlgNum != u2AuthAlgNum) {
        return WLAN_STATUS_FAILURE;
    }

    WLAN_GET_FIELD_16(&prAuthFrame->u2AuthTransSeqNo, &u2TxTransactionSeqNum)
    if (u2TxTransactionSeqNum != u2TransactionSeqNum) {
        return WLAN_STATUS_FAILURE;
    }

    return WLAN_STATUS_SUCCESS;

} /* end of authCheckTxAuthFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
authProcessRxAuthFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN P_PEER_BSS_INFO_T prPeerBssInfo,
    IN UINT_16 u2AuthAlgNum,
    IN UINT_16 u2TransactionSeqNum,
    OUT PUINT_16 pu2StatusCode
    )
{
    P_WLAN_AUTH_FRAME_T prAuthFrame;
    UINT_16 u2RxFrameCtrl;
    UINT_16 u2RxAuthAlgNum;
    UINT_16 u2RxTransactionSeqNum;
    UINT_16 u2RxStatusCode;

    DEBUGFUNC("authProcessRxAuthFrame");


    ASSERT(prAdapter);
    ASSERT(prSwRfb);
    ASSERT(prPeerBssInfo);
    ASSERT(pu2StatusCode);

    //4 <1> locate the Authentication Frame.
    prAuthFrame = (P_WLAN_AUTH_FRAME_T) prSwRfb->pvHeader;

    //4 <2> Parse the Header of Authentication Frame.
    WLAN_GET_FIELD_16(&prAuthFrame->u2FrameCtrl, &u2RxFrameCtrl)
    u2RxFrameCtrl &= MASK_FRAME_TYPE;
    if (u2RxFrameCtrl != MAC_FRAME_AUTH) {
        return WLAN_STATUS_FAILURE;
    }

    if (prSwRfb->u2FrameLength < (WLAN_MAC_MGMT_HEADER_LEN +
                                  AUTH_ALGORITHM_NUM_FIELD_LEN +
                                  AUTH_TRANSACTION_SEQENCE_NUM_FIELD_LEN +
                                  STATUS_CODE_FIELD_LEN)) {
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }

    /* Check if this Auth Frame is coming from Target BSSID */
    if (UNEQUAL_MAC_ADDR(prAuthFrame->aucSrcAddr, prPeerBssInfo->aucBSSID)) {
        DBGLOG(JOIN, LOUD, ("Ignore Auth Frame from other BSS ["MACSTR"]\n",
            MAC2STR(prAuthFrame->aucSrcAddr)));
        return WLAN_STATUS_FAILURE;
    }

    //4 <3> Parse the Fixed Fields of Authentication Frame Body.
    WLAN_GET_FIELD_16(&prAuthFrame->u2AuthAlgNum, &u2RxAuthAlgNum);
    if (u2RxAuthAlgNum != u2AuthAlgNum) {
        DBGLOG(JOIN, LOUD, ("Discard Auth frame with auth type = %d, current = %d\n",
            u2RxAuthAlgNum, u2AuthAlgNum));
        return WLAN_STATUS_FAILURE;
    }

    WLAN_GET_FIELD_16(&prAuthFrame->u2AuthTransSeqNo, &u2RxTransactionSeqNum);
    if (u2RxTransactionSeqNum != u2TransactionSeqNum) {
        DBGLOG(JOIN, LOUD, ("Discard Auth frame with Transaction Seq No = %d\n",
            u2RxTransactionSeqNum));
        return WLAN_STATUS_FAILURE;
    }

    WLAN_GET_FIELD_16(&prAuthFrame->u2StatusCode, &u2RxStatusCode);
    *pu2StatusCode = u2RxStatusCode;

    //4 <4> Ignore IE_CHALLENGE_TEXT part if the Status Code is not SUCCESSFUL.
    if (u2RxStatusCode != STATUS_CODE_SUCCESSFUL) {

        DBGLOG(JOIN, LOUD, ("Auth Failure - Status Code = %d.\n", u2RxStatusCode));
        /* NOTE(Kevin): We should return SUCCESS to indicate that the current frame
         * is parsed OK, the Status Code will be handled by JOIN FSM.
         */
        return WLAN_STATUS_SUCCESS;
    }

    //4 <5> Parse the Information Elements of Authentication Frame Body.
    if ((u2AuthAlgNum == AUTH_ALGORITHM_NUM_SHARED_KEY) &&
        (u2TransactionSeqNum == AUTH_TRANSACTION_SEQ_2)) {
        P_IE_CHALLENGE_TEXT_T prRxIeChallengeText =
            (P_IE_CHALLENGE_TEXT_T)&prAuthFrame->aucInfoElem[0];

        /* Check if there exist the Challenge Text IE */
        if ((prSwRfb->u2FrameLength < (WLAN_MAC_MGMT_HEADER_LEN +
                                      AUTH_ALGORITHM_NUM_FIELD_LEN +
                                      AUTH_TRANSACTION_SEQENCE_NUM_FIELD_LEN +
                                      STATUS_CODE_FIELD_LEN +
                                      (ELEM_HDR_LEN + ELEM_MIN_LEN_CHALLENGE_TEXT))) ||
            (prRxIeChallengeText->ucId != ELEM_ID_CHALLENGE_TEXT)) {

            DBGLOG(JOIN, LOUD, ("Discard Auth frame because the IE of Challenge Text was collapsed.\n"));
            return WLAN_STATUS_FAILURE;
        }

        /* Save the Challenge Text from Auth Seq 2 Frame, before sending Auth Seq 3 Frame */
        COPY_IE(&prPeerBssInfo->rIeChallengeText, prRxIeChallengeText);
    }


    return WLAN_STATUS_SUCCESS;

} /* end of authProcessRxAuthFrame() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
authProcessRxDeauthFrame (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSwRfb,
    IN UINT_8 aucBSSID[],
    OUT PUINT_16 pu2ReasonCode
    )
{
    P_WLAN_DEAUTH_FRAME_T prDeauthFrame;
    UINT_16 u2RxFrameCtrl;
    UINT_16 u2RxReasonCode;

    DEBUGFUNC("authProcessRxDeauthFrame");


    ASSERT(prAdapter);
    ASSERT(prSwRfb);
    ASSERT(aucBSSID);
    ASSERT(pu2ReasonCode);

    //4 <1> locate the Deauthentication Frame.
    prDeauthFrame = (P_WLAN_DEAUTH_FRAME_T) prSwRfb->pvHeader;

    //4 <2> Parse the Header of Authentication Frame.
    WLAN_GET_FIELD_16(&prDeauthFrame->u2FrameCtrl, &u2RxFrameCtrl)
    u2RxFrameCtrl &= MASK_FRAME_TYPE;
    if (u2RxFrameCtrl != MAC_FRAME_DEAUTH) {
        return WLAN_STATUS_FAILURE;
    }

    if (prSwRfb->u2FrameLength < (WLAN_MAC_MGMT_HEADER_LEN +
                                  REASON_CODE_FIELD_LEN)) {
        ASSERT(0);
        return WLAN_STATUS_FAILURE;
    }

    /* Check if this Deauth Frame is coming from Target BSSID */
    if (UNEQUAL_MAC_ADDR(prDeauthFrame->aucSrcAddr, aucBSSID)) {
        DBGLOG(JOIN, LOUD, ("Ignore Deauth Frame from other BSS ["MACSTR"]\n",
            MAC2STR(prDeauthFrame->aucSrcAddr)));
        return WLAN_STATUS_FAILURE;
    }

    //4 <3> Parse the Fixed Fields of Deauthentication Frame Body.
    WLAN_GET_FIELD_16(&prDeauthFrame->u2ReasonCode, &u2RxReasonCode);
    *pu2ReasonCode = u2RxReasonCode;

    return WLAN_STATUS_SUCCESS;

} /* end of authProcessRxDeauthFrame() */


