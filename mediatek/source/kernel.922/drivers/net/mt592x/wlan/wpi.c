
#ifdef	MTK_WAPI_SUPPORT
#include "wpi_pcrypt.h"
#include "precomp.h"

const UINT_8 aucBridgeTunnelEncap1[6] = {
    0xAA, 0xAA, 0x03, 0x00, 0x00, 0xF8
};

const UINT_8 aucRfc1042Encap1[6] = {
    0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00
};
#define WAPI_MAC_HEADER_FOR_MIC_SZ   28







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
txFragMsduFromOSForWapi (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    UINT_16 u2EtherTypeLen;
    UINT_8 aucDestAddr[MAC_ADDR_LEN];
    PUINT_8 pucLLC;
    UINT_8 ucLLCLength;
    UINT_16 u2FragThreshold = 0, u2PayloadFragThreshold;

    DEBUGFUNC("txFragMsduFromOSForWapi");

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    if (!prAdapter->fgUseWapi) {
        ASSERT(FALSE);
        return FALSE;
    }

    do {

        //4 <1> Check for BT fragmentation.
        if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
            ASSERT(FALSE);
        }
        else {
            u2FragThreshold = prConnSettings->u2FragmentationThreshold;
            u2PayloadFragThreshold = prBssInfo->u2PayloadFragmentationThreshold;
        }

        //4 <2> Process only 802.3/Ethernet Native Frame.

        // Call Kal function to check EtherType(per-packet process)
        if ( !kalQueryTxPacketHeader(prAdapter->prGlueInfo,
                                     prMsduInfo->pvPacket,
                                     &u2EtherTypeLen,
                                     &aucDestAddr[0]) ) {
            ASSERT(FALSE);
        }

        //4 <2.A> Check A1 after do Wlan Header Translation.
        if (IS_BMCAST_MAC_ADDR(aucDestAddr) &&
            prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
            break; /* Do not apply fragmentation, if A1 is BMCAST Address. */
        }

        //4 <2.B> Check actual LLC from EtherTypeLen Field.
        //printk("u2EtherTypeLen = %04x\n", u2EtherTypeLen);
        if (u2EtherTypeLen > 1500) {
            switch(u2EtherTypeLen) {
            case ETH_P_IPX:
            case ETH_P_AARP:
                DBGLOG(TX, INFO, ("Bridge-Tunnel Encapsulation\n"));

                pucLLC = (PUINT_8)&aucBridgeTunnelEncap1[0];
                ucLLCLength = sizeof(aucBridgeTunnelEncap1);
                break;

            default:
                DBGLOG(TX, INFO, ("RFC1042 Encapsulation\n"));

                pucLLC = (PUINT_8)&aucRfc1042Encap1[0];
                ucLLCLength = sizeof(aucRfc1042Encap1);
                break;
            }
        }
        else {
            pucLLC = (PUINT_8)NULL;
            ucLLCLength = 0;
        }

        //4 <3> Update fragmentation information in MSDU_INFO_T
        /* MPDU MAC Header Length = MAC Header Length from BSS_INFO */
        prMsduInfo->ucFragWlanHeaderLength = prBssInfo->ucWlanDataFrameHeaderLen;

        /* MPDU Payload Length = Threshold - MPDU MAC Header Length */
        prMsduInfo->u2PayloadFragThreshold = u2FragThreshold -
                                             (prMsduInfo->ucFragWlanHeaderLength + FCS_LEN);

        /* Update LLC information for this MSDU */
        prMsduInfo->ucLLCLength = ucLLCLength;
        prMsduInfo->pucLLC = pucLLC;

        prMsduInfo->ucMacHeaderLength = (prBssInfo->fgIsWmmAssoc) ? WLAN_MAC_HEADER_QOS_LEN : WLAN_MAC_HEADER_LEN;

        prMsduInfo->ucFragTotalCount = 1;

        return TRUE;
    }
    while (FALSE);

    return FALSE;

} /* end of txFragMsduFromOSForWapi() */

#if SUPPORT_WPI_AVOID_LOCAL_BUFFER

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
PUINT_8
halTxComposeWpiPktInCoalescingBuf (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN P_SW_TFCB_T      prSwTfcb
    )
{
    PUINT_8 pucOutputBuf; /* Pointer to TFCB Frame */
    P_HW_TFCB_T prHwTfcb;
    P_WLAN_MAC_HEADER_T prWlanHeader;
    //UINT_16 u2FrameCtrl;
    ETH_FRAME_T rEthFrame;
    UINT_16 u2FrameOffset;
    UINT_16 u2PayloadOffset;
    UINT_8 ucTemp[2048];
    UINT_8 ucTemp1[2048];

    DEBUGFUNC("halTxComposeWpiPktInCoalescingBuf");

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    ASSERT(prSwTfcb);

    pucOutputBuf = prAdapter->pucFragCoalescingBufCached;
    prWlanHeader = (P_WLAN_MAC_HEADER_T)&pucOutputBuf[TFCB_SIZE];
    prHwTfcb = (P_HW_TFCB_T)&pucOutputBuf[0];

    u2FrameOffset = TFCB_SIZE;
    u2FrameOffset += prMsduInfo->ucFragWlanHeaderLength;

    /* Decide the address of MPDU's payload */
    u2PayloadOffset = u2FrameOffset;
    if (prMsduInfo->pucLLC) {
        u2FrameOffset += (prMsduInfo->ucLLCLength + ETHER_TYPE_LEN);
        u2FrameOffset -= ETHER_HEADER_LEN;
    }
    else {
        u2FrameOffset -= ETHER_HEADER_LEN;
    }

    //WLAN_GET_FIELD_16(&prWlanHeader->u2FrameCtrl, &u2FrameCtrl);

    /* Update the field of IP/TCP checksum */
    if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_CALCULATE_CSUM) {
        ASSERT(FALSE);
    }

    /* For MIC align 16 bytes issue, use a local buffer to cal MIC */
    /* And for data encrypt, use this local buffer */
    /* Reserved a MAC Header + KEYID + RSV + L + Pad + Payload + Pad buffer */

    if (prMsduInfo->ucChkSumWapiFlag & TX_WPI_ENCRYPT) {
        UINT_8 ucMacLen, ucPadLen1, ucPadLen2;
        P_WLAN_MAC_HEADER_QOS_T prMacHdr;
        UINT_8 ucQosIdxLen;
        P_IEEE_802_11_MIB_T prMib = &prAdapter->rConnSettings.rMib;
        UINT_8   ucMic[WPI_MIC_LEN];
        UINT_8   ucIv[PN_LEN];
        UINT_8   ucIv2[PN_LEN];
        BOOLEAN  fgQoS;
        DBGLOG(WAPI, TRACE, ("TX_WPI_ENCRYPT\r\n"));

        fgQoS = prAdapter->rBssInfo.fgIsWmmAssoc ? TRUE : FALSE;

        ucMacLen = fgQoS ? WLAN_MAC_HEADER_QOS_LEN : WLAN_MAC_HEADER_LEN;
        ucQosIdxLen = fgQoS ? KEYID_LEN + KEYID_RSV_LEN + PDU_LEN + 2 :
            KEYID_LEN + KEYID_RSV_LEN + PDU_LEN;

        ucPadLen1 = 16 - ((WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen) % 16);
        if (ucPadLen1 == 16)
            ucPadLen1 = 0;
        ucPadLen2 = 16 - ((prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN) % 16);
        if (ucPadLen2 == 16)
            ucPadLen2 = 0;

        /* Compose a MIC verify frame */

        /* Type + payload */
        kalCopyFrame(prAdapter->prGlueInfo,
                     prMsduInfo->pvPacket,
                     &ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ+ ucQosIdxLen + ucPadLen1
                     + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN - ETHER_HEADER_LEN]);

        kalMemCopy(&rEthFrame, /*.aucDestAddr[0] removed for klocwork warning */
                   &ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1
                     + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN - ETHER_HEADER_LEN],
                   ETHER_HEADER_LEN);

        /* Do Mac header Translation */
        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

        /* put the LLC */
        kalMemCopy((PVOID)&ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                   (PVOID)prMsduInfo->pucLLC,
                   (UINT_32)prMsduInfo->ucLLCLength);

        prMacHdr = (P_WLAN_MAC_HEADER_QOS_T)&pucOutputBuf[TFCB_SIZE];

        wpi_mic_compose(1, prMib->ucWpiActivedPWKey, prMsduInfo->u2PayloadLength, prMacHdr, (PVOID)&ucTemp, fgQoS);

        if (1) {
            UINT_32 i;
            for (i=0;i <16;i++)
                ucIv[i] = prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucPN[15-i];
        }

        /* PN + 2 */
        INC_128_BITS(ucIv);
        INC_128_BITS(ucIv);

        if (1) {
            UINT_32 i;
            for (i=0;i <16;i++)
                ucIv2[i] = ucIv[15-i];
        }

        /* WPI MIC */
        wpi_pmac((UINT_8 *)ucIv2,
                 ucTemp,
                 (prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + ucPadLen2 + WAPI_MAC_HEADER_FOR_MIC_SZ+ ucQosIdxLen + ucPadLen1) / 16,
                 (UINT_8 *)prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucWPICK,
                 ucMic);

        //DBGLOG_MEM8(WAPI, TRACE, ucMic, 16);

        kalMemCopy(&ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN] , ucMic, WPI_MIC_LEN);

        /* WPI Encrypt */
        if (wpi_encrypt(ucIv2,
                        &ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                        prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + WPI_MIC_LEN,
                        (UINT_8 *)prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucWPIEK,
                        ucTemp1)){
            ASSERT(FALSE);
        }


        /* Copy the encrypted packet to actualy address to avoid the align issue!!!! */
        kalMemCopy(&pucOutputBuf[TFCB_SIZE + ucMacLen + KEYID_LEN + KEYID_RSV_LEN + PN_LEN],
            ucTemp1,
            prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + WPI_MIC_LEN);
        /* key index */
        pucOutputBuf[TFCB_SIZE + ucMacLen] = prMib->ucWpiActivedPWKey;

        /* Rsv */
        pucOutputBuf[TFCB_SIZE + ucMacLen + KEYID_LEN] = 0;

        /* PN */
        kalMemCopy(&pucOutputBuf[TFCB_SIZE + ucMacLen + KEYID_LEN + KEYID_RSV_LEN],
            ucIv,  PN_LEN);

        {
            UINT_32 i;
            for (i=0;i <16;i++)
                prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucPN[i] = ucIv[15-i];
        }

        #if 0 /* Not needed this ? */
        /* Do Mac header Translation */
        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

        kalMemCopy(&rEthFrame, /* .aucDestAddr[0] removed for klocwork checking */
                   prMacHdr->aucAddr3,
                   ETHER_HEADER_LEN);
        #endif
        {
            P_WLAN_MAC_HEADER_QOS_T prWlanDataFrameHeader;
            prWlanDataFrameHeader = (P_WLAN_MAC_HEADER_QOS_T)&pucOutputBuf[TFCB_SIZE];
            prWlanDataFrameHeader->u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;
        }
    }
    else if (prMsduInfo->ucChkSumWapiFlag & TX_WPI_OPEN) {

        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

        kalCopyFrame(prAdapter->prGlueInfo,
                     prMsduInfo->pvPacket,
                     ucTemp);

        kalMemCopy(&rEthFrame, /*.aucDestAddr[0] removed for klocwork checking */
                   ucTemp,
                   ETHER_HEADER_LEN);

        kalMemCopy((PVOID)&pucOutputBuf[u2PayloadOffset + MAC_ADDR_LEN],
            (PVOID)&ucTemp[ETHER_HEADER_LEN - ETHER_TYPE_LEN], prMsduInfo->u2PayloadLength + LLC_LEN);

        /* Overwrite the DA/SA field in coalescing buffer */
        if (prMsduInfo->pucLLC) {
            kalMemCopy((PVOID)&pucOutputBuf[u2PayloadOffset],
                       (PVOID)prMsduInfo->pucLLC,
                       (UINT_32)prMsduInfo->ucLLCLength);
        }

        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

    {
        P_WLAN_MAC_HEADER_QOS_T prWlanDataFrameHeader;
        prWlanDataFrameHeader = (P_WLAN_MAC_HEADER_QOS_T)&pucOutputBuf[TFCB_SIZE];
        prWlanDataFrameHeader->u2FrameCtrl &= ~MASK_FC_PROTECTED_FRAME;
    }

    }

    halTxComposeFrameControlBlock(prAdapter, prSwTfcb, NULL, prHwTfcb);

    return pucOutputBuf;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
rxProcessMPDUWpiSecurity (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    )
{
    WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

    ASSERT(prAdapter);
    ASSERT(prSWRfb);

    //4 <1> wpi decrypt and pn check
    {
        if (prAdapter->fgUseWapi &&
            RX_STATUS_IS_802_11(prSWRfb->prRxStatus->u2StatusFlag) &&
            RX_STATUS_IS_DATA_FRAME(prSWRfb->prRxStatus->u2StatusFlag) &&
            RX_STATUS_IS_PROTECT(prSWRfb->prRxStatus)) {
            PUCHAR              pucMic1;   /* MIC  */
            UCHAR               aucMic2[WPI_MIC_LEN] = {0}; /* MIC' */
            PUCHAR              pucFrameBody;
            UINT_16             u2FrameBodyLen;
            BOOLEAN             fgStatus = FALSE;
            UINT_8              ucMacLen = 24, ucPadLen1, ucPadLen2;
            UINT_8              ucTemp[2000];
            UINT_8              ucTemp1[2000];
            P_WLAN_MAC_HEADER_QOS_T prMacHeader;
            P_IEEE_802_11_MIB_T prMib = &prAdapter->rConnSettings.rMib;
            UINT_8              ucKeyIdx;
            BOOLEAN             fgUCFrame = TRUE;
            UINT_8              ucQosIdxLen;
            P_PARAM_WPI_KEY_T   prKey;
            UINT_8              ucRxPN[PN_LEN],  ucRxPN1[PN_LEN];
            UINT_8              ucRxMic[WPI_MIC_LEN];
            BOOLEAN             fgQoS = FALSE;

            pucFrameBody = prSWRfb->pvBody;
            u2FrameBodyLen = prSWRfb->u2FrameLength - prSWRfb->u2MACHeaderLength;

            if (u2FrameBodyLen <  17)
                return TRUE;

            DBGLOG(WAPI, TRACE, ("Before MPDU Decapsulate: %d 0x%p 0x%p\r\n", u2FrameBodyLen,
                pucFrameBody, prSWRfb->prRxStatus));

            prMacHeader = (P_WLAN_MAC_HEADER_QOS_T)prSWRfb->pvHeader;

            if (RX_STATUS_IS_QoS(prSWRfb->prRxStatus->u2StatusFlag))
                fgQoS = TRUE;

            if (fgQoS) {
                ucMacLen = WLAN_MAC_HEADER_QOS_LEN;
                ucQosIdxLen = KEYID_LEN + KEYID_RSV_LEN + PDU_LEN + 2 ;
            }
            else {
                ucMacLen = WLAN_MAC_HEADER_LEN;
                ucQosIdxLen = KEYID_LEN + KEYID_RSV_LEN + PDU_LEN;
            }

            ucPadLen1 = 16 - ((WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen) % 16);
            if (ucPadLen1 == 16) ucPadLen1 = 0;

            /* Todo:: PN Check */
            kalMemCopy(ucRxPN1, &pucFrameBody[KEYID_LEN + KEYID_RSV_LEN], PN_LEN);

            if (1) {
                UINT_32 i;
                for (i=0;i<16;i++)
                    ucRxPN[i] = ucRxPN1[15-i];
            }

            DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)ucRxPN, 16);

            if (prMacHeader->aucAddr1[0] & BIT(0))
                fgUCFrame = FALSE;

            ucKeyIdx = *pucFrameBody;

            if (fgUCFrame) {
                /* Should use the key by rx key idx */
                if (prMib->fgPairwiseKeyUsed[ucKeyIdx] == FALSE) {					
                    DBGLOG(WAPI, TRACE, ("Drop rcv unicast frame due the key not exist !!\r\n"));
			return WLAN_STATUS_FAILURE;
                }
                else {
                    prKey = &prMib->rWapiPairwiseKey[ucKeyIdx];
		}
            }
            else {
                prKey = &prMib->rWapiGroupKey[ucKeyIdx];
		}
            kalMemCopy(ucTemp1, &pucFrameBody[KEYID_LEN + KEYID_RSV_LEN + PN_LEN], u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN));

            if (wpi_decrypt((PUINT_8)ucRxPN,
                    ucTemp1 /* &pucFrameBody[KEYID_LEN + KEYID_RSV_LEN + PN_LEN] */,
                    (UINT_32)(u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN)),
                    (UINT_8 *)prKey->aucWPIEK,
                    &ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1])) {
		  DBGLOG(WAPI, TRACE, ("wpi_decrypt failed!!\r\n"));
		  ASSERT(FALSE);

		  return WLAN_STATUS_FAILURE;
            }
            if (sizeof(ucTemp) >=
                (WPI_MIC_LEN + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN))
                ) {
			kalMemCopy(ucRxMic,
                    &ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN)],
                    WPI_MIC_LEN);
            }
            else {
                ERRORLOG(("sizeof(ucTemp)(%d) < copy length + offset(%d), WPI_MIC_LEN(%d), WAPI_MAC_HEADER_FOR_MIC_SZ(%d), ucQosIdxLen(%d), ucPadLen1(%d), u2FrameBodyLen(%d), \n",
                    sizeof(ucTemp),
                    (WPI_MIC_LEN + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN)),
                    WPI_MIC_LEN, WAPI_MAC_HEADER_FOR_MIC_SZ, ucQosIdxLen, ucPadLen1, u2FrameBodyLen));
                ERRORLOG(("KEYID_LEN(%d), KEYID_RSV_LEN(%d), PN_LEN(%d), WPI_MIC_LEN(%d)\n",
                    KEYID_LEN, KEYID_RSV_LEN, PN_LEN,WPI_MIC_LEN));
                ASSERT(FALSE);
            }
            kalMemZero(&ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN)],
                WPI_MIC_LEN);
            DBGLOG_MEM32(WAPI, TRACE,
                (PUINT_8)&ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN));

            /* Mic check */
            if (1){
                wpi_mic_compose(0, ucKeyIdx, u2FrameBodyLen, prMacHeader, (PUINT_8)ucTemp, fgQoS);
                u2FrameBodyLen -= (KEYID_LEN + KEYID_RSV_LEN + PN_LEN);
                ucPadLen2 = 16 - ((u2FrameBodyLen - WPI_MIC_LEN)% 16);
                if (ucPadLen2 == 16)
                    ucPadLen2 = 0;
		/* WPI MIC */
                wpi_pmac(ucRxPN,
                         ucTemp,
                         (u2FrameBodyLen - WPI_MIC_LEN + ucPadLen2 + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1)/16,
                         prKey->aucWPICK,
                         aucMic2);

                /* verify MIC and MIC' */
                pucMic1 = ucRxMic;

                DBGLOG(WAPI, TRACE, ("pucMic1 MIC:\r\n"));
                DBGLOG_MEM8(WAPI, TRACE, pucMic1, WPI_MIC_LEN);
                DBGLOG(WAPI, TRACE, ("aucMic2 MIC':\r\n"));
                DBGLOG_MEM8(WAPI, TRACE, aucMic2, WPI_MIC_LEN);

                if (pucMic1[0] == aucMic2[0] && pucMic1[1] == aucMic2[1] &&
                    pucMic1[2] == aucMic2[2] && pucMic1[3] == aucMic2[3] &&
                    pucMic1[4] == aucMic2[4] && pucMic1[5] == aucMic2[5] &&
                    pucMic1[6] == aucMic2[6] && pucMic1[7] == aucMic2[7] &&
                    pucMic1[8] == aucMic2[8] && pucMic1[9] == aucMic2[9] &&
                    pucMic1[10] == aucMic2[10] && pucMic1[11] == aucMic2[11] &&
                    pucMic1[12] == aucMic2[12] && pucMic1[13] == aucMic2[13] &&
                    pucMic1[14] == aucMic2[14] && pucMic1[15] == aucMic2[15]
                    ) {
			                  u2FrameBodyLen -= (WPI_MIC_LEN);

                    /* Copy the encrypted packet to actualy address */
                    kalMemCopy(pucFrameBody,
                            &ucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                            u2FrameBodyLen);

                    prSWRfb->u2FrameLength = u2FrameBodyLen + prSWRfb->u2MACHeaderLength;

                    DBGLOG(WAPI, TRACE, ("After MSDU Decapsulate:\r\n"));
                    DBGLOG(WAPI, TRACE, ("Frame body: (length = %u)\r\n", u2FrameBodyLen));
                    DBGLOG_MEM8(WAPI, TRACE, pucFrameBody, u2FrameBodyLen);

                    fgStatus = TRUE;
                }
                else {
                    fgStatus = FALSE;
                }

            }
            //return fgStatus;
            if (fgStatus) {
                /* Change the actived key */
                if (fgUCFrame) {
                    prMib->ucWpiActivedPWKey = ucKeyIdx;
                    DBGLOG(WAPI, TRACE, ("Change the Unicast key index to %d\r\n", ucKeyIdx));
                }
            }
            else {
                /* Drop frame */
                return WLAN_STATUS_FAILURE;
            }
        }/*  */
    }

    return u4Status;
}
#else
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
PUINT_8
halTxComposeWpiPktInCoalescingBuf (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN P_SW_TFCB_T      prSwTfcb
    )
{
    PUINT_8 pucOutputBuf; /* Pointer to TFCB Frame */
    P_HW_TFCB_T prHwTfcb;
    P_WLAN_MAC_HEADER_T prWlanHeader;
    ETH_FRAME_T rEthFrame;
    UINT_16 u2FrameOffset;
    UINT_16 u2PayloadOffset;

    PUINT_8 pucTemp;

    DEBUGFUNC("halTxComposeWpiPktInCoalescingBuf");

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    ASSERT(prSwTfcb);

    pucOutputBuf = prAdapter->pucFragCoalescingBufCached;
    prWlanHeader = (P_WLAN_MAC_HEADER_T)&pucOutputBuf[TFCB_SIZE];
    prHwTfcb = (P_HW_TFCB_T)&pucOutputBuf[0];

    u2FrameOffset = TFCB_SIZE;
    u2FrameOffset += prMsduInfo->ucFragWlanHeaderLength;

    /* Decide the address of MPDU's payload */
    u2PayloadOffset = u2FrameOffset;
    if (prMsduInfo->pucLLC) {
        u2FrameOffset += (prMsduInfo->ucLLCLength + ETHER_TYPE_LEN);
        u2FrameOffset -= ETHER_HEADER_LEN;
    }
    else {
        u2FrameOffset -= ETHER_HEADER_LEN;
    }

    //WLAN_GET_FIELD_16(&prWlanHeader->u2FrameCtrl, &u2FrameCtrl);

    /* Update the field of IP/TCP checksum */
    if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_CALCULATE_CSUM) {
        ASSERT(FALSE);
    }

    pucTemp = prAdapter->ucTemp;

    if ((UINT_32)pucTemp % 4)
        pucTemp += (4 - ((UINT_32)pucTemp % 4));

    /* For MIC align 16 bytes issue, use a local buffer to cal MIC */
    /* And for data encrypt, use this local buffer */
    /* Reserved a MAC Header + KEYID + RSV + L + Pad + Payload + Pad buffer */

    if (prMsduInfo->ucChkSumWapiFlag & TX_WPI_ENCRYPT) {
        UINT_8 ucMacLen, ucPadLen1, ucPadLen2;
        P_WLAN_MAC_HEADER_QOS_T prMacHdr;
        UINT_8 ucQosIdxLen;
        P_IEEE_802_11_MIB_T prMib = &prAdapter->rConnSettings.rMib;
        UINT_8   ucMic[WPI_MIC_LEN];
        UINT_8   ucIv[PN_LEN];
        UINT_8   ucIv2[PN_LEN];
        BOOLEAN  fgQoS;
        DBGLOG(WAPI, TRACE, ("TX_WPI_ENCRYPT\r\n"));
		
        fgQoS = prAdapter->rBssInfo.fgIsWmmAssoc ? TRUE : FALSE;

        ucMacLen = fgQoS ? WLAN_MAC_HEADER_QOS_LEN : WLAN_MAC_HEADER_LEN;
        ucQosIdxLen = fgQoS ? KEYID_LEN + KEYID_RSV_LEN + PDU_LEN + 2 :
            KEYID_LEN + KEYID_RSV_LEN + PDU_LEN;

        ucPadLen1 = 16 - ((WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen) % 16);
        if (ucPadLen1 == 16) {
            ucPadLen1 = 0;
        }
        ucPadLen2 = 16 - ((prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN) % 16);
        if (ucPadLen2 == 16) {
            ucPadLen2 = 0;
        }

        /* Compose a MIC verify frame */
        /* Type + payload */
        kalCopyFrame(prAdapter->prGlueInfo,
                     prMsduInfo->pvPacket,
                     &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ+ ucQosIdxLen + ucPadLen1
                     + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN - ETHER_HEADER_LEN]);

        kalMemCopy(&rEthFrame, /*.aucDestAddr[0] removed for klocwork checking */
                   &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1
                     + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN - ETHER_HEADER_LEN],
                   ETHER_HEADER_LEN);

        /* Do Mac header Translation */
        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

        /* put the LLC */
        kalMemCopy((PVOID)&pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                   (PVOID)prMsduInfo->pucLLC,
                   (UINT_32)prMsduInfo->ucLLCLength);

        prMacHdr = (P_WLAN_MAC_HEADER_QOS_T)&pucOutputBuf[TFCB_SIZE];

	wpi_mic_compose(1, prMib->ucWpiActivedPWKey, prMsduInfo->u2PayloadLength, prMacHdr, (PVOID)pucTemp, fgQoS);

        if (1) {
            UINT_32 i;
            for (i=0;i <16;i++)
                ucIv[i] = prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucPN[15-i];
        }

        /* PN + 2 */
        INC_128_BITS(ucIv);
        INC_128_BITS(ucIv);

        if (1) {
            UINT_32 i;
            for (i=0;i <16;i++)
                ucIv2[i] = ucIv[15-i];
        }
        /* WPI MIC */
        wpi_pmac((UINT_8 *)ucIv2,
                 pucTemp,
                 (prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + ucPadLen2 + WAPI_MAC_HEADER_FOR_MIC_SZ+ ucQosIdxLen + ucPadLen1) / 16,
                 (UINT_8 *)prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucWPICK,
                 ucMic);

        //DBGLOG_MEM8(WAPI, TRACE, ucMic, 16);

        kalMemCopy(&pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN] , ucMic, WPI_MIC_LEN);

        /* WPI Encrypt */
        if (wpi_encrypt(ucIv2,
                        &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                        prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + WPI_MIC_LEN,
                        (UINT_8 *)prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucWPIEK,
                        &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1] /*ucTemp1*/)){
            ASSERT(FALSE);
        }


        /* Copy the encrypted packet to actualy address to avoid the align issue!!!! */
        kalMemCopy(&pucOutputBuf[TFCB_SIZE + ucMacLen + KEYID_LEN + KEYID_RSV_LEN + PN_LEN],
            &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1]  /*ucTemp1*/ ,
            prMsduInfo->u2PayloadLength + prMsduInfo->ucLLCLength + ETHER_TYPE_LEN + WPI_MIC_LEN);
        /* key index */
        pucOutputBuf[TFCB_SIZE + ucMacLen] = prMib->ucWpiActivedPWKey;

        /* Rsv */
        pucOutputBuf[TFCB_SIZE + ucMacLen + KEYID_LEN] = 0;

        /* PN */
        kalMemCopy(&pucOutputBuf[TFCB_SIZE + ucMacLen + KEYID_LEN + KEYID_RSV_LEN],
            ucIv,  PN_LEN);

        {
            UINT_32 i;
            for (i=0;i <16;i++)
                prMib->rWapiPairwiseKey[prMib->ucWpiActivedPWKey].aucPN[i] = ucIv[15-i];
        }

        #if 0 /* Not needed this ? */
        /* Do Mac header Translation */
        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

        kalMemCopy(&rEthFrame, /*.aucDestAddr[0] removed for klocwork checking */
                   prMacHdr->aucAddr3,
                   ETHER_HEADER_LEN);
        #endif
        {
            P_WLAN_MAC_HEADER_QOS_T prWlanDataFrameHeader;
            prWlanDataFrameHeader = (P_WLAN_MAC_HEADER_QOS_T)&pucOutputBuf[TFCB_SIZE];
            prWlanDataFrameHeader->u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;
        }
    }
    else if (prMsduInfo->ucChkSumWapiFlag & TX_WPI_OPEN) {		
        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

        kalCopyFrame(prAdapter->prGlueInfo,
                     prMsduInfo->pvPacket,
                     pucTemp);

        kalMemCopy(&rEthFrame, /*.aucDestAddr[0] removed for klocwork checking */
                   pucTemp,
                   ETHER_HEADER_LEN);

        kalMemCopy((PVOID)&pucOutputBuf[u2PayloadOffset + MAC_ADDR_LEN],
            (PVOID)&pucTemp[ETHER_HEADER_LEN - ETHER_TYPE_LEN], prMsduInfo->u2PayloadLength + LLC_LEN);

        /* Overwrite the DA/SA field in coalescing buffer */
        if (prMsduInfo->pucLLC) {
            kalMemCopy((PVOID)&pucOutputBuf[u2PayloadOffset],
                       (PVOID)prMsduInfo->pucLLC,
                       (UINT_32)prMsduInfo->ucLLCLength);
        }

        txFragComposeWlanDataFrameHeader(prAdapter,
                                         &rEthFrame,
                                         prMsduInfo->ucTID,
                                         &pucOutputBuf[TFCB_SIZE]);

    {
        P_WLAN_MAC_HEADER_QOS_T prWlanDataFrameHeader;
        prWlanDataFrameHeader = (P_WLAN_MAC_HEADER_QOS_T)&pucOutputBuf[TFCB_SIZE];
        prWlanDataFrameHeader->u2FrameCtrl &= ~MASK_FC_PROTECTED_FRAME;
    }

    }

    halTxComposeFrameControlBlock(prAdapter, prSwTfcb, NULL, prHwTfcb);

    return pucOutputBuf;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
rxProcessMPDUWpiSecurity (
    IN P_ADAPTER_T prAdapter,
    IN P_SW_RFB_T prSWRfb
    )
{
    WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

    ASSERT(prAdapter);
    ASSERT(prSWRfb);

    //4 <1> wpi decrypt and pn check
    {
        if (prAdapter->fgUseWapi &&
            RX_STATUS_IS_802_11(prSWRfb->prRxStatus->u2StatusFlag) &&
            RX_STATUS_IS_DATA_FRAME(prSWRfb->prRxStatus->u2StatusFlag) &&
            RX_STATUS_IS_PROTECT(prSWRfb->prRxStatus)) {
            PUCHAR              pucMic1;   /* MIC  */
            UCHAR               aucMic2[WPI_MIC_LEN] = {0}; /* MIC' */
            PUCHAR              pucFrameBody;
            UINT_16             u2FrameBodyLen;
            BOOLEAN             fgStatus = FALSE;
            UINT_8              ucMacLen = 24, ucPadLen1, ucPadLen2;
            P_WLAN_MAC_HEADER_QOS_T prMacHeader;
            P_IEEE_802_11_MIB_T prMib = &prAdapter->rConnSettings.rMib;
            UINT_8              ucKeyIdx;
            BOOLEAN             fgUCFrame = TRUE;
            UINT_8              ucQosIdxLen;
            P_PARAM_WPI_KEY_T   prKey;
            UINT_8              ucRxPN[PN_LEN],  ucRxPN1[PN_LEN];
            UINT_8              ucRxMic[WPI_MIC_LEN];
            BOOLEAN             fgQoS = FALSE;
            PUINT_8             pucTemp;
            pucFrameBody = prSWRfb->pvBody;
            u2FrameBodyLen = prSWRfb->u2FrameLength - prSWRfb->u2MACHeaderLength;

            if (u2FrameBodyLen < 17) {
                return TRUE;
            }
            pucTemp = prAdapter->ucTemp;

            if ((UINT_32)pucTemp % 4) {
                pucTemp += (4 - ((UINT_32)pucTemp % 4));
            }
            DBGLOG(WAPI, TRACE, ("Before MPDU Decapsulate: %d 0x%p 0x%p\r\n", u2FrameBodyLen,
                pucFrameBody, prSWRfb->prRxStatus));

            prMacHeader = (P_WLAN_MAC_HEADER_QOS_T)prSWRfb->pvHeader;
            DBGLOG(WAPI, TRACE, ("prMacHeader 0x%p\r\n", prMacHeader));

            DBGLOG_MEM32(WAPI, TRACE, (PUINT_8)pucFrameBody, u2FrameBodyLen);

            if (RX_STATUS_IS_QoS(prSWRfb->prRxStatus->u2StatusFlag)) {
                fgQoS = TRUE;
            }
            if (fgQoS) {
                ucMacLen = WLAN_MAC_HEADER_QOS_LEN;
                ucQosIdxLen = KEYID_LEN + KEYID_RSV_LEN + PDU_LEN + 2 ;
            }
            else {
                ucMacLen = WLAN_MAC_HEADER_LEN;
                ucQosIdxLen = KEYID_LEN + KEYID_RSV_LEN + PDU_LEN;
            }

            ucPadLen1 = 16 - ((WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen) % 16);
            if (ucPadLen1 == 16) {
                ucPadLen1 = 0;
            }

            /* Todo:: PN Check */
            kalMemCopy(ucRxPN1, &pucFrameBody[KEYID_LEN + KEYID_RSV_LEN], PN_LEN);

            if (1) {
                UINT_32 i;
                for (i=0;i<16;i++)
                    ucRxPN[i] = ucRxPN1[15-i];
            }

            DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)ucRxPN, 16);

            if (prMacHeader->aucAddr1[0] & BIT(0)) {
                fgUCFrame = FALSE;
            }
            ucKeyIdx = *pucFrameBody;

            DBGLOG(WAPI, TRACE, ("rcv wpi frame, key index: %d\r\n", ucKeyIdx));
            if (fgUCFrame) {
                /* Should use the key by rx key idx */
                if (prMib->fgPairwiseKeyUsed[ucKeyIdx] == FALSE) {
                    DBGLOG(WAPI, WARN, ("Drop rcv unicast frame due the key not exist !!\r\n"));
                    return WLAN_STATUS_FAILURE;
                }
                else {
                    prKey = &prMib->rWapiPairwiseKey[ucKeyIdx];
                }
            }
            else {
                prKey = &prMib->rWapiGroupKey[ucKeyIdx];
            }

            DBGLOG_MEM8(WAPI, TRACE, (UINT_8 *)prKey->aucWPIEK, 16);

            kalMemCopy(&pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1] /*ucTemp1*/, &pucFrameBody[KEYID_LEN + KEYID_RSV_LEN + PN_LEN], u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN));

            /* WPI Decrypt */
            if (wpi_decrypt((PUINT_8)ucRxPN,
                    &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1]/* ucTemp1*/,
                    (UINT_32)(u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN)),
                    (UINT_8 *)prKey->aucWPIEK,
                    &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1])) {
                ASSERT(FALSE);
                return WLAN_STATUS_FAILURE;
            }
            /* Add size checking for klocwork array boundary checking:
             * pucTemp points to prAdapter->ucTemp[].
             */
            if (sizeof(prAdapter->ucTemp) >=
                (WPI_MIC_LEN + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN))
                ) {
                kalMemCopy(ucRxMic,
                    &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN)],
                    WPI_MIC_LEN);
            }
            else {				
                ERRORLOG(("sizeof(prAdapter->ucTemp)(%d) < copy length+offset(%d), WPI_MIC_LEN(%d), WAPI_MAC_HEADER_FOR_MIC_SZ(%d), ucQosIdxLen(%d), ucPadLen1(%d), u2FrameBodyLen(%d), \n",
                    sizeof(prAdapter->ucTemp),
                    (WPI_MIC_LEN + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN)),
                    WPI_MIC_LEN, WAPI_MAC_HEADER_FOR_MIC_SZ, ucQosIdxLen, ucPadLen1, u2FrameBodyLen));
                ERRORLOG(("KEYID_LEN(%d), KEYID_RSV_LEN(%d), PN_LEN(%d), WPI_MIC_LEN(%d)\n",
                    KEYID_LEN, KEYID_RSV_LEN, PN_LEN,WPI_MIC_LEN));
                ASSERT(FALSE);
            }
            kalMemZero(&pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1 + u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN + WPI_MIC_LEN)], WPI_MIC_LEN);
            DBGLOG_MEM32(WAPI, TRACE,
                (PUINT_8)&pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                u2FrameBodyLen - (KEYID_LEN + KEYID_RSV_LEN + PN_LEN));

            /* Mic check */
            if (1){
                wpi_mic_compose(0, ucKeyIdx, u2FrameBodyLen, prMacHeader, (PUINT_8)pucTemp, fgQoS);
                u2FrameBodyLen -= (KEYID_LEN + KEYID_RSV_LEN + PN_LEN);
                ucPadLen2 = 16 - ((u2FrameBodyLen - WPI_MIC_LEN)% 16);
                if (ucPadLen2 == 16) {
                    ucPadLen2 = 0;
                }
                DBGLOG(WAPI, TRACE, ("Rx Mic \r\n"));

                DBGLOG_MEM8(WAPI, TRACE, pucTemp, u2FrameBodyLen - WPI_MIC_LEN + ucPadLen2 + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1);
                /* WPI MIC */
                wpi_pmac(ucRxPN,
                         pucTemp,
                         (u2FrameBodyLen - WPI_MIC_LEN + ucPadLen2 + WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1)/16,
                         prKey->aucWPICK,
                         aucMic2);

                /* verify MIC and MIC' */
                pucMic1 = ucRxMic;


                DBGLOG(WAPI, TRACE, ("MIC:\r\n"));
                DBGLOG_MEM8(WAPI, TRACE, pucMic1, WPI_MIC_LEN);
                DBGLOG(WAPI, TRACE, ("MIC':\r\n"));
                DBGLOG_MEM8(WAPI, TRACE, aucMic2, WPI_MIC_LEN);

                if (pucMic1[0] == aucMic2[0] && pucMic1[1] == aucMic2[1] &&
                    pucMic1[2] == aucMic2[2] && pucMic1[3] == aucMic2[3] &&
                    pucMic1[4] == aucMic2[4] && pucMic1[5] == aucMic2[5] &&
                    pucMic1[6] == aucMic2[6] && pucMic1[7] == aucMic2[7] &&
                    pucMic1[8] == aucMic2[8] && pucMic1[9] == aucMic2[9] &&
                    pucMic1[10] == aucMic2[10] && pucMic1[11] == aucMic2[11] &&
                    pucMic1[12] == aucMic2[12] && pucMic1[13] == aucMic2[13] &&
                    pucMic1[14] == aucMic2[14] && pucMic1[15] == aucMic2[15]
                    ) {
                    u2FrameBodyLen -= (WPI_MIC_LEN);

                    /* Copy the encrypted packet to actualy address */
                    kalMemCopy(pucFrameBody,
                            &pucTemp[WAPI_MAC_HEADER_FOR_MIC_SZ + ucQosIdxLen + ucPadLen1],
                            u2FrameBodyLen);

                    prSWRfb->u2FrameLength = u2FrameBodyLen + prSWRfb->u2MACHeaderLength;

                    DBGLOG(WAPI, TRACE, ("After MSDU Decapsulate:\r\n"));
                    DBGLOG(WAPI, TRACE, ("Frame body: (length = %u)\r\n", u2FrameBodyLen));
                    DBGLOG_MEM8(WAPI, TRACE, pucFrameBody, u2FrameBodyLen);

                    fgStatus = TRUE;
                }
                else {
                    fgStatus = FALSE;
                }

            }
            //return fgStatus;
            if (fgStatus) {
                /* Change the actived key */
                if (fgUCFrame) {
                    prMib->ucWpiActivedPWKey = ucKeyIdx;
                    DBGLOG(WAPI, TRACE, ("Change the Unicast key index to %d\r\n", ucKeyIdx));
                }
            }
            else {
                /* Drop frame */
                return WLAN_STATUS_FAILURE;
            }
        }/*  */
    }

    return u4Status;
}
#endif
#endif

