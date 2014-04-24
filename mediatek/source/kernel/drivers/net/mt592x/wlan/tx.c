






#include "precomp.h"

const UINT_8 aucBridgeTunnelEncap[6] = {
    0xAA, 0xAA, 0x03, 0x00, 0x00, 0xF8
};

const UINT_8 aucRfc1042Encap[6] = {
    0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00
};

static const UINT_16 au2CCKFragThresholdForBT[BT_COEXIST_WINDOW_TYPE_NUM][PREAMBLE_OPTION_NUM][4] = {
{ /* BT Window 650 */
    { /* Long Preamble(Default) */
        0,      /* RATE_1M_INDEX = 0 */ /* NOTE(Kevin): 0 means we should exclude this TX rate */
        0,      /* RATE_2M_INDEX */
        0,      /* RATE_5_5M_INDEX */
        288     /* RATE_11M_INDEX */
    },
    { /* Short Preamble Option */
        0,      /* RATE_1M_INDEX = 0 */
        0,      /* RATE_2M_INDEX */
        256,    /* RATE_5_5M_INDEX */
        544     /* RATE_11M_INDEX */
    }
},
{ /* BT Window 1250 */
    { /* Long Preamble(Default) */
        0,      /* RATE_1M_INDEX = 0 */
        0,      /* RATE_2M_INDEX */
        544,    /* RATE_5_5M_INDEX */
        1088    /* RATE_11M_INDEX */
    },
    { /* Short Preamble Option */
        0,      /* RATE_1M_INDEX = 0 */
        0,      /* RATE_2M_INDEX */
        672,    /* RATE_5_5M_INDEX */
        1376    /* RATE_11M_INDEX */
    }
},
{ /* BT Window 2500 */
    { /* Long Preamble(Default) */
        0,                                  /* RATE_1M_INDEX = 0 */
        480,                                /* RATE_2M_INDEX */
        1376,                               /* RATE_5_5M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX   /* RATE_11M_INDEX */
    },
    { /* Short Preamble Option */
        0,                                  /* RATE_1M_INDEX = 0 */
        544,                                /* RATE_2M_INDEX */
        1500,                               /* RATE_5_5M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX   /* RATE_11M_INDEX */
    }
}};


static const UINT_16 au2OFDMFragThresholdForBT[BT_COEXIST_WINDOW_TYPE_NUM - 1][8] = {
{ /* BT Window 650 */
    384,                                    /* RATE_6M_INDEX */
    576,                                    /* RATE_9M_INDEX */
    800,                                    /* RATE_12M_INDEX */
    1184,                                   /* RATE_18M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_24M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_36M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_48M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX       /* RATE_54M_INDEX */
},
{ /* BT Window 1250 */
    832,                                    /* RATE_6M_INDEX */
    1248,                                   /* RATE_9M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_12M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_18M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_24M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_36M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_48M_INDEX */
    DOT11_FRAGMENTATION_THRESHOLD_MAX       /* RATE_54M_INDEX */
}};


static const UINT_16 au2OFDMFragThresholdForBTWithProtection[BT_COEXIST_WINDOW_TYPE_NUM - 1][PREAMBLE_OPTION_NUM][8] = {
{ /* BT Window 650 */
    { /* Long Preamble(Default) */
        0,                                      /* RATE_6M_INDEX */
        0, /* 352, */                           /* RATE_9M_INDEX */
        480,                                    /* RATE_12M_INDEX */
        704,                                    /* RATE_18M_INDEX */
        992,                                    /* RATE_24M_INDEX */
        1500,                                   /* RATE_36M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_48M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX       /* RATE_54M_INDEX */
    },
    { /* Short Preamble Option */
        0,                                      /* RATE_6M_INDEX */
        0, /* 448, */                           /* RATE_9M_INDEX */
        608,                                    /* RATE_12M_INDEX */
        928,                                    /* RATE_18M_INDEX */
        1248,                                   /* RATE_24M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_36M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_48M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX       /* RATE_54M_INDEX */
    }
},
{ /* BT Window 1250 */
    { /* Long Preamble(Default) */
        0,                                      /* RATE_6M_INDEX */
        972,                                    /* RATE_9M_INDEX */
        1376,                                   /* RATE_12M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_18M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_24M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_36M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_48M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX       /* RATE_54M_INDEX */
    },
    { /* Short Preamble Option */
        0,                                      /* RATE_6M_INDEX */
        1068,                                   /* RATE_9M_INDEX */
        1532,                                   /* RATE_12M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_18M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_24M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_36M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX,      /* RATE_48M_INDEX */
        DOT11_FRAGMENTATION_THRESHOLD_MAX       /* RATE_54M_INDEX */
    }
}};

const UINT_16 u2ExcludedRateSetForBT[BT_COEXIST_WINDOW_TYPE_NUM][PREAMBLE_OPTION_NUM] = {
{ /* BT Window 650 */
    /* Long Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M | RATE_SET_BIT_5_5M),
    /* Short Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M)
},
{ /* BT Window 1250 */
    /* Long Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M),
    /* Short Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M)
},
{ /* BT Window 2500 */
    /* Long Preamble(Default) */
    (RATE_SET_BIT_1M),
    /* Short Preamble(Default) */
    (RATE_SET_BIT_1M)
}};

const UINT_16 u2ExcludedRateSetForBTWithProtection[BT_COEXIST_WINDOW_TYPE_NUM][PREAMBLE_OPTION_NUM] = {
{ /* BT Window 650 */
    /* Long Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M | RATE_SET_BIT_5_5M | RATE_SET_BIT_6M | RATE_SET_BIT_9M),
    /* Short Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M | RATE_SET_BIT_6M | RATE_SET_BIT_9M)
},
{ /* BT Window 1250 */
    /* Long Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M | RATE_SET_BIT_6M),
    /* Short Preamble(Default) */
    (RATE_SET_BIT_1M | RATE_SET_BIT_2M | RATE_SET_BIT_6M)
},
{ /* BT Window 2500 */
    /* Long Preamble(Default) */
    (RATE_SET_BIT_1M),
    /* Short Preamble(Default) */
    (RATE_SET_BIT_1M)
}};






#if CFG_TX_FRAGMENT
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
txFragInfoUpdate (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;

    DEBUGFUNC("txFragInfoUpdate");


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    if (prConnSettings->fgIsEnableTxFragment) {
        prBssInfo->ucWlanDataFrameHeaderLen =
            (prBssInfo->fgIsWmmAssoc) ? WLAN_MAC_HEADER_QOS_LEN : WLAN_MAC_HEADER_LEN;

        prBssInfo->u2PayloadFragmentationThreshold =
            prConnSettings->u2FragmentationThreshold -
                (prBssInfo->ucWlanDataFrameHeaderLen + LLC_LEN +
                    (prBssInfo->fgRequireMICForFrag ? TKIP_MIC_LEN : 0) + FCS_LEN);
    }

    return;
} /* end of txFragInfoUpdate() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
txFragInfoUpdateForPrivacy (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsPrivacyEnable,
    IN BOOLEAN fgIsTkipCipher
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;

    DEBUGFUNC("txFragInfoUpdateForPrivacy");


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    if (prConnSettings->fgIsEnableTxFragment) {

        if (fgIsPrivacyEnable) {
            prBssInfo->fgIsPrivacyEnable = TRUE;

            if (fgIsTkipCipher) {
                prBssInfo->u2PayloadFragmentationThreshold =
                    prConnSettings->u2FragmentationThreshold -
                        (prBssInfo->ucWlanDataFrameHeaderLen + LLC_LEN + TKIP_MIC_LEN + FCS_LEN);

                prBssInfo->fgRequireMICForFrag = TRUE;
            }
            else {
                prBssInfo->fgRequireMICForFrag = FALSE;
            }
        }
        else {
            prBssInfo->fgIsPrivacyEnable = FALSE;
            prBssInfo->fgRequireMICForFrag = FALSE;
        }
    }

    return;
} /* end of txFragInfoUpdateForPrivacy() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ BOOLEAN
txFragMsduFromOS (
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
    UINT_16 u2OverallPayloadLength;
    UINT_16 u2FragThreshold, u2PayloadFragThreshold;

    DEBUGFUNC("txFragMsduFromOS");


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    do {

        //4 <1> Currently we don't support 802.11 Native Frame in this release
        if (prMsduInfo->fgIs802_11Frame) {
            ASSERT(0); // We may do ASSERT if we do not support in this release.
            break;
        }


        //4 <2> Check for BT fragmentation.
        if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
            P_STA_RECORD_T prStaRec; /* Pointer to the Station Record */

            //4 <2.A> Check if we have locate the peer STA_RECORD_T. (Done in arbFsmRunEventTxMsduFromOs())
#if CFG_IBSS_POWER_SAVE
            if (prMsduInfo->prStaRec) {
                prStaRec = prMsduInfo->prStaRec;
            }
            else
#endif /* CFG_IBSS_POWER_SAVE */
            //4 <2.B> Get STA_RECORD_T from default entry.
            {
                /* NOTE(Kevin 2008/1/15): for DLS, we need search STA_RECORD_T in
                 * arbFsmRunEventTxMsduFromOs() according to its DestAddr[].
                 */
                prStaRec = nicPrivacyGetStaRecordByWlanIndex(prAdapter, 0); // Get the default STA_RECORD.
                if (!prStaRec) {
                    /* NOTE(Kevin 2008/1/21):
                     * 1. If the eConnectionState is not in MEDIA_STATE_CONNECTED
                     *    (when driver load), then there should be no STA_RECORD_T.
                     * 2. When User set the OID: Infrastructure Mode which will
                     *    cause the whole WLAN Entry be flushed immediately before
                     *    another OID: SSID be set.
                     */
                    break;
                    //ASSERT(prStaRec);
                }
            }

            //4 <2.C> Read dynamic fragmentation threshold for BT coexist.
            u2FragThreshold = txDynamicFragThresholdInitForBT(prAdapter, prStaRec);

            //4 <2.D> Check which fragmentation threshold we'll apply.
            if ((u2FragThreshold >= prConnSettings->u2FragmentationThreshold) ||
                (u2FragThreshold == 0 /* Error handling */)) {

                u2FragThreshold = prConnSettings->u2FragmentationThreshold;
                u2PayloadFragThreshold = prBssInfo->u2PayloadFragmentationThreshold;
            }
            else {
                //4 <2.E> Roughly calculate the fragmentation threshold of Payload part.
                u2PayloadFragThreshold = u2FragThreshold -
                    (prBssInfo->ucWlanDataFrameHeaderLen + LLC_LEN +
                        (prBssInfo->fgRequireMICForFrag ? TKIP_MIC_LEN : 0) + FCS_LEN);
            }

        }
        else {
            u2FragThreshold = prConnSettings->u2FragmentationThreshold;
            u2PayloadFragThreshold = prBssInfo->u2PayloadFragmentationThreshold;
        }


        //4 <3> Process only 802.3/Ethernet Native Frame.

        //4 <3.A> Roughly check if current payload length is less than current Fragmentation Threshold.
        if ((prMsduInfo->u2PayloadLength) <= u2PayloadFragThreshold) {
            break;
        }


        // Call Kal function to check EtherType(per-packet process)
        if ( !kalQueryTxPacketHeader(prAdapter->prGlueInfo,
                                     prMsduInfo->pvPacket,
                                     &u2EtherTypeLen,
                                     &aucDestAddr[0]) ) {
            break;
        }

        //4 <3.B> Check A1 after do Wlan Header Translation.
        if (IS_BMCAST_MAC_ADDR(aucDestAddr) &&
            prAdapter->eCurrentOPMode == OP_MODE_IBSS) {
            break; /* Do not apply fragmentation, if A1 is BMCAST Address. */
        }

        //4 <3.C> Check actual LLC from EtherTypeLen Field.
        //printk("u2EtherTypeLen = %04x\n", u2EtherTypeLen);
        if (u2EtherTypeLen > 1500) {
            switch(u2EtherTypeLen) {
            case ETH_P_IPX:
            case ETH_P_AARP:
                DBGLOG(TX, INFO, ("Bridge-Tunnel Encapsulation\n"));

                pucLLC = (PUINT_8)&aucBridgeTunnelEncap[0];
                ucLLCLength = sizeof(aucBridgeTunnelEncap);
                break;

            default:
                DBGLOG(TX, INFO, ("RFC1042 Encapsulation\n"));

                pucLLC = (PUINT_8)&aucRfc1042Encap[0];
                ucLLCLength = sizeof(aucRfc1042Encap);
                break;
            }
        }
        else {
            pucLLC = (PUINT_8)NULL;
            ucLLCLength = 0;
        }

        /* Calculate the MPDU Payload length */
        u2OverallPayloadLength =
            (UINT_16)((pucLLC) ? (ucLLCLength + ETHER_TYPE_LEN) : 0) +
            prMsduInfo->u2PayloadLength +
            (UINT_16)(prBssInfo->fgRequireMICForFrag ? TKIP_MIC_LEN : 0);


        //4 <3.D> Precisely check if we need to apply  fragmentation for this MSDU_INFO_T.
        if ((UINT_16)((UINT_16)prBssInfo->ucWlanDataFrameHeaderLen +
            u2OverallPayloadLength + FCS_LEN) <= u2FragThreshold) {
            break; // Didn't exceed the fragmentation threshold.
        }

        //4 <4> Update fragmentation information in MSDU_INFO_T
        /* MPDU MAC Header Length = MAC Header Length from BSS_INFO */
        prMsduInfo->ucFragWlanHeaderLength = prBssInfo->ucWlanDataFrameHeaderLen;

        /* MPDU Payload Length = Threshold - MPDU MAC Header Length */
        prMsduInfo->u2PayloadFragThreshold = u2FragThreshold -
                                             (prMsduInfo->ucFragWlanHeaderLength + FCS_LEN);

        /* Update LLC information for this MSDU */
        prMsduInfo->ucLLCLength = ucLLCLength;
        prMsduInfo->pucLLC = pucLLC;

        /* Update MIC flag for this MSDU */
        if (prBssInfo->fgRequireMICForFrag) {
            prMsduInfo->ucControlFlag |= MSDU_INFO_CTRL_FLAG_CALCULATE_MIC;
        }

        /* Update CSUM flag for this MSDU */
#if CFG_TCP_IP_CHKSUM_OFFLOAD
        if (prMsduInfo->ucChkSumWapiFlag) {
            #if SUPPORT_WAPI
            if ((prMsduInfo->ucChkSumWapiFlag & TX_WPI_ENCRYPT) ||
                (prMsduInfo->ucChkSumWapiFlag & TX_WPI_OPEN)) {
            }
            else
            #endif
            prMsduInfo->ucControlFlag |= MSDU_INFO_CTRL_FLAG_CALCULATE_CSUM;
        }
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

        /* Calculate the MPDU count */
        u2OverallPayloadLength +=
            (prMsduInfo->u2PayloadFragThreshold - 1);

        prMsduInfo->ucFragTotalCount = (UINT_8)
            (u2OverallPayloadLength / prMsduInfo->u2PayloadFragThreshold);

        DBGLOG(TX, INFO, ("prMsduInfo->ucFragTotalCount = %d\n",
            prMsduInfo->ucFragTotalCount));

        return TRUE;
    }
    while (FALSE);

    return FALSE;

} /* end of txFragMsduFromOS() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
__KAL_INLINE__ BOOLEAN
txFragMmpdu (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_WLAN_MAC_HEADER_T prWlanHeader;
    UINT_16 u2FrameCtrl;
    UINT_16 u2OverallPayloadLength;
    UINT_16 u2FragThreshold;

    DEBUGFUNC("txFragMmpdu");


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);
    prConnSettings = &prAdapter->rConnSettings;

    do {
        //4 <1> Currently there is no 802.3/Ethernet Frame which is composed by ourself.
        if (!prMsduInfo->fgIs802_11Frame) {
            ASSERT(0); // We may do ASSERT if we do not support in this release.
            break;
        }

        //4 <2> Process only 802.11 Frame.
        prWlanHeader = (P_WLAN_MAC_HEADER_T)MGT_PACKET_GET_BUFFER(prMsduInfo->pvPacket);
        WLAN_GET_FIELD_16(&prWlanHeader->u2FrameCtrl, &u2FrameCtrl);
        u2FrameCtrl &= MASK_FC_TYPE;

        if ((u2FrameCtrl != MAC_FRAME_TYPE_MGT) ||
            IS_BMCAST_MAC_ADDR(prWlanHeader->aucAddr1)) {
            if (u2FrameCtrl != MAC_FRAME_TYPE_DATA) {
                /* Excluding
                 * CTRL Frame,
                 * Null/QoS Null Data Frame
                 * BCAST MGT Frame.
                 */
                break;
            }
        }

        //4 <3> Check for BT fragmentation.
        if ((prAdapter->rConnSettings.fgIsEnableTxAutoFragmentForBT) &&
            (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) &&
            !(prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_BASIC_RATE)) {
            P_STA_RECORD_T prStaRec; /* Pointer to the Station Record */

            //4 <3.A> Check if we have locate the peer STA_RECORD_T. (Done in arbFsmRunEventTxMsduFromOs())
#if CFG_IBSS_POWER_SAVE
            if (prMsduInfo->prStaRec) {
                prStaRec = prMsduInfo->prStaRec;
            }
            else
#endif /* CFG_IBSS_POWER_SAVE */
            //4 <3.B> Get STA_RECORD_T from default entry.
            {
                /* NOTE(Kevin 2008/1/15): for DLS, we need search STA_RECORD_T in
                 * arbFsmRunEventTxMsduFromOs() according to its DestAddr[].
                 */
                prStaRec = nicPrivacyGetStaRecordByWlanIndex(prAdapter, 0); // Get the default STA_RECORD.
                if (!prStaRec) {
                    /* NOTE(Kevin 2008/1/21):
                     * 1. If the eConnectionState is not in MEDIA_STATE_CONNECTED
                     *    (when driver load), then there should be no STA_RECORD_T.
                     * 2. When User set the OID: Infrastructure Mode which will
                     *    cause the whole WLAN Entry be flushed immediately before
                     *    another OID: SSID be set.
                     */
                    break;
                    //ASSERT(prStaRec);
                }
            }

            //4 <3.C> Read dynamic fragmentation threshold for BT coexist.
            u2FragThreshold = txDynamicFragThresholdInitForBT(prAdapter, prStaRec);

            //4 <3.D> Check which fragmentation threshold we'll apply.
            if ((u2FragThreshold >= prConnSettings->u2FragmentationThreshold) ||
                (u2FragThreshold == 0 /* Error handling */)) {

                u2FragThreshold = prConnSettings->u2FragmentationThreshold;
            }
        }
        else {
            u2FragThreshold = prConnSettings->u2FragmentationThreshold;
        }

        //4 <4> Fragmentation Threshold check.
        if ( (((UINT_16)prMsduInfo->ucMacHeaderLength +
            prMsduInfo->u2PayloadLength + FCS_LEN) <=
            u2FragThreshold) ) {
            break; // Didn't exceed the fragmentation threshold.
        }

        //4 <5> Update fragmentation information in MSDU_INFO_T
        /* MPDU MAC Header Length = MSDU MAC Header Length */
        prMsduInfo->ucFragWlanHeaderLength = prMsduInfo->ucMacHeaderLength;

        /* MPDU Payload Length = Threshold - MPDU MAC Header Length */
        prMsduInfo->u2PayloadFragThreshold = u2FragThreshold -
                                             (prMsduInfo->ucFragWlanHeaderLength + FCS_LEN);

        /* No LLC information for MMPDU */
        prMsduInfo->ucLLCLength = 0; // Just in case.
        prMsduInfo->pucLLC = (PUINT_8)NULL;

        /* Calculate the MPDU count */
        u2OverallPayloadLength =
            prMsduInfo->u2PayloadLength +
            (prMsduInfo->u2PayloadFragThreshold - 1);

        prMsduInfo->ucFragTotalCount = (UINT_8)
            (u2OverallPayloadLength / prMsduInfo->u2PayloadFragThreshold);

        DBGLOG(TX, INFO, ("prMsduInfo->ucFragTotalCount = %d\n",
            prMsduInfo->ucFragTotalCount));

        return TRUE; // Do Fragmentation.
    }
    while (FALSE);

    return FALSE;

} /* end of txFragMmpdu() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
txFragComposeWlanDataFrameHeader (
    IN P_ADAPTER_T prAdapter,
    IN P_ETH_FRAME_T prEthFrame,
    IN UINT_8 ucTID,
    IN PUINT_8 pucOutput
    )
{
    P_BSS_INFO_T prBssInfo;
    P_WLAN_MAC_HEADER_QOS_T prWlanDataFrameHeader;
    UINT_16 u2FrameCtrl;


    ASSERT(prAdapter);
    ASSERT(prEthFrame);
    ASSERT(pucOutput);
    prBssInfo = &prAdapter->rBssInfo;
    prWlanDataFrameHeader = (P_WLAN_MAC_HEADER_QOS_T)pucOutput;

    /* Update the Frame Control field for the common part. */
    u2FrameCtrl = ((prBssInfo->fgIsWmmAssoc) ? MAC_FRAME_TYPE_QOS_DATA : MAC_FRAME_TYPE_DATA);
    u2FrameCtrl |= ((prBssInfo->fgIsPrivacyEnable) ? MASK_FC_PROTECTED_FRAME : 0);

    if (prAdapter->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
        /* Update the Frame Control field by current Operation Mode. */
        u2FrameCtrl |= MASK_FC_TO_DS;

        /* Fill the RA field with Current BSSID. */
        COPY_MAC_ADDR(prWlanDataFrameHeader->aucAddr1, prBssInfo->aucBSSID);

        /* Fill the TA field with our MAC Address. */
        COPY_MAC_ADDR(prWlanDataFrameHeader->aucAddr2, prAdapter->aucMacAddress);

        /* Fill the DA field with DA. */
        COPY_MAC_ADDR(prWlanDataFrameHeader->aucAddr3, prEthFrame->aucDestAddr);
    }
    else if (prAdapter->eCurrentOPMode == OP_MODE_IBSS) {

        /* Fill the RA field with DA. */
        COPY_MAC_ADDR(prWlanDataFrameHeader->aucAddr1, prEthFrame->aucDestAddr);

        /* Fill the TA field with our MAC Address. */
        COPY_MAC_ADDR(prWlanDataFrameHeader->aucAddr2, prAdapter->aucMacAddress);

        /* Fill the BSSID field with Current BSSID. */
        COPY_MAC_ADDR(prWlanDataFrameHeader->aucAddr3, prBssInfo->aucBSSID);
    }

    /* Fill the Frame Control field. */
    WLAN_SET_FIELD_16(&prWlanDataFrameHeader->u2FrameCtrl, u2FrameCtrl);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prWlanDataFrameHeader->u2SeqCtrl = 0;

    if (prBssInfo->fgIsWmmAssoc) {
        UINT_16 u2QosControl = ucTID;

        WLAN_SET_FIELD_16(&prWlanDataFrameHeader->u2QosCtrl, u2QosControl);
    }

    return;
} /* end of txFragComposeWlanDataFrameHeader() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
txFragmentation (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    )
{

    ASSERT(prAdapter);
    ASSERT(prMsduInfo);

    do {
        //4 <1> Do fragment for frame when in RF test mode
        if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
            //TODO: Fragment for RFtest packets.
            /* No break here for setting up ucFragTotalCount */
        }
        //4 <2> Do fragment for frame from Internal Protocol Stack
        else if (prMsduInfo->fgIsFromInternalProtocolStack) {
            if (txFragMmpdu(prAdapter, prMsduInfo)) {
                break;
            }
        }
        #if SUPPORT_WAPI
        //4 <3> Do wapi frame format handle, not fragment for wpi frame
        else if (prAdapter->fgUseWapi && prMsduInfo->fgIs802_11Frame) {
            if (txFragMsduFromOSForWapi(prAdapter, prMsduInfo)) {
                break;
            }
        }
        #endif
        //4 <4> Do fragment for frame from OS Layer.
        else if (txFragMsduFromOS(prAdapter, prMsduInfo)) {
            break;
        }

        //4 <4> No fragmentation for this frame
        /* MSDU/MMPDU = 1 MPDU */
        prMsduInfo->ucFragTotalCount = 1;
    }
    while (FALSE);

    return WLAN_STATUS_SUCCESS;

} /* end of txFragmentation() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_16
txDynamicFragThresholdInitForBT (
    IN P_ADAPTER_T      prAdapter,
    IN P_STA_RECORD_T   prStaRec
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    UINT_16 u2FragThreshold;


    ASSERT(prAdapter);
    ASSERT(prStaRec);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    //4 <1> Get proper fragmentation threshold according to current TX Rate.
    if (prStaRec->ucCurrRate1Index >= RATE_6M_INDEX) {
        if (prConnSettings->eBTCoexistWindowType < BT_COEXIST_WINDOW_2500) {
            UINT_8 ucIndex = prStaRec->ucCurrRate1Index - RATE_6M_INDEX;

            if (prBssInfo->fgIsProtection) {
                u2FragThreshold = au2OFDMFragThresholdForBTWithProtection
                                     [prConnSettings->eBTCoexistWindowType] \
                                     [prStaRec->fgIsShortPreambleOptionEnable ? PREAMBLE_OPTION_SHORT : PREAMBLE_DEFAULT_LONG_NONE] \
                                     [ucIndex];
            }
            else {
                u2FragThreshold = au2OFDMFragThresholdForBT
                                     [prConnSettings->eBTCoexistWindowType] \
                                     [ucIndex];
            }
        }
        else {
            u2FragThreshold = DOT11_FRAGMENTATION_THRESHOLD_MAX;
        }
    }
    else if( prStaRec->ucCurrRate1Index < RATE_22M_INDEX ){
        u2FragThreshold = au2CCKFragThresholdForBT \
                              [prConnSettings->eBTCoexistWindowType] \
                              [prStaRec->fgIsShortPreambleOptionEnable ? PREAMBLE_OPTION_SHORT : PREAMBLE_DEFAULT_LONG_NONE] \
                              [prStaRec->ucCurrRate1Index];
    }
    else {
        /* RATE_22M_INDEX or RATE_33M_INDEX */
        ERRORLOG(("Unsupported rate index: 0x%x\n", prStaRec->ucCurrRate1Index));
        ASSERT(0);
        u2FragThreshold = DOT11_FRAGMENTATION_THRESHOLD_MAX;
    }

    DBGLOG(TX, INFO, ("u2FragThreshold = %d(eBTCoexistWindowType = %d, fgIsShortPreambleOptionEnable= %d, ucCurrRate1Index = %d)\n",
        u2FragThreshold, prConnSettings->eBTCoexistWindowType, prStaRec->fgIsShortPreambleOptionEnable, prStaRec->ucCurrRate1Index));


    ASSERT(u2FragThreshold); /* NOTE(Kevin): if the u2FragThreshold == 0,
                              * we should mask out the corresponding TX rate in
                              * the desired rate set STA_RECORD_T.
                              * So we shouldn't have the case of u2FragThreshold == 0.
                              * Put an assert to check for it.
                              */

    return u2FragThreshold;

} /* end of txDynamicFragThresholdInitForBT() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
txRateSetInitForBT (
    IN P_ADAPTER_T      prAdapter,
    IN P_STA_RECORD_T   prStaRec
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    UINT_16 u2ExcludedRateSet, u2DesiredRateSet;
    UINT_8 ucWlanIndex;
    UINT_8 ucCurrRate1Index = RATE_NUM;
    UINT_8 i;


    ASSERT(prAdapter);
    ASSERT(prStaRec);
    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    if (prBssInfo->fgIsProtection) {
        u2ExcludedRateSet = u2ExcludedRateSetForBTWithProtection \
                                [prConnSettings->eBTCoexistWindowType] \
                                [prStaRec->fgIsShortPreambleOptionEnable ? PREAMBLE_OPTION_SHORT : PREAMBLE_DEFAULT_LONG_NONE];
    }
    else {
        u2ExcludedRateSet = u2ExcludedRateSetForBT \
                                [prConnSettings->eBTCoexistWindowType] \
                                [prStaRec->fgIsShortPreambleOptionEnable ? PREAMBLE_OPTION_SHORT : PREAMBLE_DEFAULT_LONG_NONE];
    }

    u2DesiredRateSet = (prStaRec->u2DesiredRateSet & ~u2ExcludedRateSet);
    if (!u2DesiredRateSet /* Error Handling */) {

        u2DesiredRateSet = prStaRec->u2DesiredRateSet;
        ASSERT(0);
    }

    for (i = RATE_1M_INDEX; i <= prStaRec->ucCurrRate1Index; i++) {
        if (BIT(i) & u2DesiredRateSet) {
            ucCurrRate1Index = i;
        }
    }

    if (!nicPrivacyGetWlanIndexByAddr(prAdapter, &prStaRec->aucMacAddr[0], &ucWlanIndex)) {
        ASSERT(0);
    }

    nicARSetRate(prAdapter,
                 u2DesiredRateSet,
                 prStaRec->fgIsShortPreambleOptionEnable,
                 ucCurrRate1Index,
                 ucWlanIndex,
                 TRUE);

    return;
} /* end of txRateSetInitForBT() */

#endif /* CFG_TX_FRAGMENT */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
txProcessMSDU (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo
    )
{
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;


    ASSERT(prAdapter);
    ASSERT(prMsduInfo);

    //4 <1> Check if we need to do tx csum offload
#if CFG_TCP_IP_CHKSUM_OFFLOAD
    if (!prMsduInfo->fgIsFromInternalProtocolStack) {
        kalQueryTxChksumOffloadParam(prMsduInfo->pvPacket, &prMsduInfo->ucChkSumWapiFlag);
    }
#endif

#if SUPPORT_WAPI
    /* For wapi, open mode packet, at MSDU_INFO_T
           set fgIs802_11 = TRUE and MSDU_INFO_T set TX_WPI_OPEN,
           for wpi mode packet,
           set fgIs802_11 = TRUE and MSDU_INFO_T set TX_WPI_ENCRYPT,
           The fgIs802_11 = TRUE is used to let the driver handle this frame as 802.11 to
           reserved frame space, do header translation....
       */
    if (prAdapter->fgUseWapi &&
        !prMsduInfo->fgIsFromInternalProtocolStack) {

        if (prAdapter->rConnSettings.rMib.fgWapiKeyInstalled) {
            if (prMsduInfo->ucControlFlag & MSDU_INFO_CTRL_FLAG_DISABLE_PRIVACY_BIT) {
                prMsduInfo->ucChkSumWapiFlag &= ~TX_WPI_ENCRYPT;
                prMsduInfo->ucChkSumWapiFlag |= TX_WPI_OPEN;
                DBGLOG(WAPI, TRACE, ("802.1x use open \r\n"));
            }
            else {
                prMsduInfo->ucChkSumWapiFlag &= ~TX_WPI_OPEN;
                prMsduInfo->ucChkSumWapiFlag |= TX_WPI_ENCRYPT;
            }
        }
        else {
            prMsduInfo->ucChkSumWapiFlag &= ~TX_WPI_ENCRYPT;
            prMsduInfo->ucChkSumWapiFlag |= TX_WPI_OPEN;
        }
    }
#endif

    //4 <2> Check TX Fragmentation
#if CFG_TX_FRAGMENT
    if (prAdapter->rConnSettings.fgIsEnableTxFragment) {
        rStatus = txFragmentation(prAdapter, prMsduInfo);
    }
    else
#endif
    {
        /* MSDU/MMPDU = 1 MPDU */
        prMsduInfo->ucFragTotalCount = 1;
    }

    return rStatus;

} /* end of txProcessMSDU() */


