






#include "precomp.h"



BOOLEAN fgIsBusAccessFailed = FALSE;


/* OID set handlers without the need to access HW register */
PFN_OID_HANDLER_FUNC apfnOidSetHandlerWOHwAccess[] = {
    wlanoidSetFrequency,
    wlanoidSetChannel,
    wlanoidSetBeaconInterval,
#if 0 /* SUPPORT_WPS */
    wlanoidSetAppIE,
    wlanoidSetFrameFilter
#endif /* SUPPORT_WPS */
};

/* OID query handlers without the need to access HW register */
PFN_OID_HANDLER_FUNC apfnOidQueryHandlerWOHwAccess[] = {
    wlanoidQueryPermanentAddr,
    wlanoidQueryCurrentAddr,
    wlanoidQueryBssid,
    wlanoidQuerySsid,
    wlanoidQueryInfrastructureMode,
    wlanoidQueryAuthMode,
    wlanoidQueryEncryptionStatus,
    wlanoidQueryPmkid,
    wlanoidQueryNetworkTypeInUse,
    wlanoidQueryRssi,
    wlanoidQueryBssidList,
    wlanoidQueryAcpiDevicePowerState,
    wlanoidQueryLinkSpeed,
    wlanoidQueryXmitOk,
    wlanoidQueryRcvOk,
    wlanoidQueryXmitError,
    wlanoidQueryRcvError,
    wlanoidQueryXmitOneCollision,
    wlanoidQueryXmitMoreCollisions,
    wlanoidQueryXmitMaxCollisions,

    wlanoidQueryVoipConnectionStatus,

    wlanoidQuerySupportedRates,
    wlanoidQueryDesiredRates,

    wlanoidQuery802dot11PowerSaveProfile,

    wlanoidQueryRoamingFunction,
    wlanoidQueryBeaconInterval,
    wlanoidQueryAtimWindow,
    wlanoidQueryFrequency,
#if CFG_LP_PATTERN_SEARCH_SLT
    wlanoidQuerySltResult,
    wlanoidSetSltMode,
#endif
    wlanoidQueryStatisticsForLinux,
};



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanIsHandlerNeedHwAccess (
    IN PFN_OID_HANDLER_FUNC pfnOidHandler,
    IN BOOLEAN              fgSetInfo
    )
{
    PFN_OID_HANDLER_FUNC* apfnOidHandlerWOHwAccess;
    UINT_32 i;
    UINT_32 u4NumOfElem;

    if (fgSetInfo) {
        apfnOidHandlerWOHwAccess = apfnOidSetHandlerWOHwAccess;
        u4NumOfElem = sizeof(apfnOidSetHandlerWOHwAccess) / sizeof(PFN_OID_HANDLER_FUNC);
    }
    else {
        apfnOidHandlerWOHwAccess = apfnOidQueryHandlerWOHwAccess;
        u4NumOfElem = sizeof(apfnOidQueryHandlerWOHwAccess) / sizeof(PFN_OID_HANDLER_FUNC);
    }

    for (i = 0; i < u4NumOfElem; i++) {
        if (apfnOidHandlerWOHwAccess[i] == pfnOidHandler) {
            return FALSE;
        }
    }

    return TRUE;
}   /* wlanIsHandlerNeedHwAccess */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanCardEjected (
    IN P_ADAPTER_T         prAdapter
    )
{
    DEBUGFUNC("wlanCardEjected");
    INITLOG(("\n"));

    ASSERT(prAdapter);

    /* mark that the card is being ejected, NDIS will shut us down soon */
    HAL_SET_FLAG(prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR);

    nicTxRelease(prAdapter);

} /* wlanCardEjected */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_ADAPTER_T
wlanAdapterCreate (
    IN P_GLUE_INFO_T prGlueInfo
    )
{
    P_ADAPTER_T prAdpater = (P_ADAPTER_T)NULL;

    DEBUGFUNC("wlanAdapterCreate");

    do {
        prAdpater = (P_ADAPTER_T)kalMemAlloc(sizeof(ADAPTER_T));

        if (!prAdpater) {
            DBGLOG(INIT, ERROR, ("Allocate ADAPTER memory ==> FAILED\n"));
            break;
        }

        kalMemZero(prAdpater, sizeof(ADAPTER_T));
        prAdpater->prGlueInfo = prGlueInfo;

    } while (FALSE);

    return prAdpater;
} /* wlanAdapterCreate */

#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanAdapterDestroy (
    IN P_ADAPTER_T prAdapter
    )
{

    if (!prAdapter) {
        return;
    }
    else {
        P_RFTEST_INFO_T prRfTestInfo;
        P_RX_CTRL_T prRxCtrl;
        P_TX_CTRL_T prTxCtrl;

        prRxCtrl = &prAdapter->rRxCtrl;
        prTxCtrl = &prAdapter->rTxCtrl;
        prRfTestInfo = &prAdapter->rRFTestInfo;

        if (prRxCtrl->pucRxCached) {
            kalMemFree(prRxCtrl->pucRxCached, prRxCtrl->u4RxCachedSize);
        }

        if (prTxCtrl->pucTxCached) {
            kalMemFree(prTxCtrl->pucTxCached, prTxCtrl->u4TxCachedSize);
        }

        if (prRfTestInfo->pucTxCached) {
            kalMemFree(prRfTestInfo->pucTxCached, prRfTestInfo->u4TxCachedSize);
        }

        kalMemFree(prAdapter, sizeof(ADAPTER_T));
    }

    return;
} /* wlanAdapterDestroy */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanAdapterStart (
    IN P_ADAPTER_T prAdapter,
    IN P_REG_INFO_T prRegInfo
    )
{
    WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
    P_PTA_INFO_T prPtaInfo;
    P_PTA_PARAM_T prPtaParam;


    ASSERT(prAdapter);

    DEBUGFUNC("wlanAdapterStart");

    prPtaInfo = &prAdapter->rPtaInfo;
    prPtaParam  = &prPtaInfo->rPtaParam;

    prPtaInfo->fgSingleAntenna = TRUE;
    prPtaInfo->u4PTAWireMode = PTA_SW_1_WIRE_MODE;
    prPtaParam->u4BtCR0 = 0;
    prPtaParam->u4BtCR1 = 0;
    prPtaParam->u4BtCR2 = 0;
    prPtaParam->u4BtCR3 = 0;

    //4 <1> Initial ARB Fsm.
    if ((rStatus = arbFsmInit(prAdapter) ) != WLAN_STATUS_SUCCESS) {
        return rStatus;
    }

    arbFsmRunEventReset(prAdapter);


    /*==================================================*/

    return rStatus;
} /* wlanAdapterStart */
#else

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanAdapterDestroy (
    IN P_ADAPTER_T prAdapter
    )
{

    if (!prAdapter) {
        return;
    }

    kalMemFree(prAdapter, sizeof(ADAPTER_T));

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanAdapterStart (
    IN P_ADAPTER_T prAdapter,
    IN P_REG_INFO_T prRegInfo
    )
{
    WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;


    ASSERT(prAdapter);

    ARB_ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

    do {
        if ((u4Status = arbFsmInit(prAdapter, prRegInfo) ) != WLAN_STATUS_SUCCESS) {
            break;
        }

        // Trigger ARB FSM.
        arbFsmRunEventReset(prAdapter);

#if defined(LINUX)
        kalUpdateMACAddress(prAdapter->prGlueInfo,
                            prAdapter->aucMacAddress);
#endif /* LINUX */
    } while (FALSE);

    ARB_RECLAIM_POWER_CONTROL_TO_PM(prAdapter);

    return u4Status;
} /* wlanAdapterStart */
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanAdapterStop (
    IN P_ADAPTER_T prAdapter
    )
{
    WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

    ASSERT(prAdapter);
    /* Due to there's a trap condition for not permitting to do HW access under ACPI-D3,
     * here's a flag for preventing this case in the adapter stop procedure.
     * This is a easier method for not to effect subroutine-internal mechanism.
     */
    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
         fgIsBusAccessFailed = TRUE;
    }

    ARB_ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

    /* To disassociate connected AP, to avoid whql unload load driver test error */
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        arbFsmRunEventJoinDisassoc(prAdapter);
    }

    nicDisableInterrupt(prAdapter);

    arbFsmUnInit(prAdapter);

    nicTxRelease(prAdapter);

    nicRxUninitialize(prAdapter);

    ARB_RECLAIM_POWER_CONTROL_TO_PM(prAdapter);

    nicReleaseAdapterMemory(prAdapter);

#if defined(_HIF_SPI)
    /* Note: restore the SPI Mode Select from 32 bit to default */
    nicRestoreSpiDefMode(prAdapter);
#endif

    return u4Status;
} /* wlanAdapterStop */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
wlanISR (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgGlobalIntrCtrl
    )
{
    ASSERT(prAdapter);

    if (prAdapter->rAcpiState != ACPI_STATE_D3) {
#if CFG_TEST_IO_PIN
        if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
            if (!prAdapter->fgIntVarified) {
                UINT_32 u4TmpValue = 0, u4ScrOriVal = 0;
                HAL_MCR_RD(prAdapter, MCR_SCR, &u4TmpValue);
                u4ScrOriVal = u4TmpValue;

                if ((u4TmpValue & SCR_BT_ACT_SEL) ||
                        (u4TmpValue & SCR_GPIO0_ENABLE_OUTPUT_MODE) ||
                        (u4TmpValue & SCR_GPIO2_ENABLE_OUTPUT_MODE)) {

                    u4TmpValue &= ~(SCR_GPIO0_ENABLE_OUTPUT_MODE);
                    u4TmpValue &= ~(SCR_GPIO2_ENABLE_OUTPUT_MODE);
                    u4TmpValue &= ~(SCR_BT_ACT_SEL);
                    HAL_MCR_WR(prAdapter, MCR_SCR, u4TmpValue);
                    HAL_MCR_RD(prAdapter, MCR_SCR, &u4TmpValue);
                }

                if (u4TmpValue & SCR_GPIO0_RDATA) {
                    /* GPIO_0 Fail. */
                    prAdapter->u4IntIORslt &= ~(BIT(0));
                }
                else {
                    /* GPIO_0 OK. */
                    prAdapter->u4IntIORslt |= BIT(0);
                }

                if (u4TmpValue & SCR_GPIO2_RDATA) {
                    /* GPIO_2 Fail. */
                    prAdapter->u4IntIORslt &= ~(BIT(1));
                }
                else {
                    /* GPIO_2 OK. */
                    prAdapter->u4IntIORslt |= BIT(1);
                }

                HAL_MCR_WR(prAdapter, MCR_SCR, u4ScrOriVal);

                prAdapter->fgIntVarified = TRUE;
            }
        }
#endif

        if (fgGlobalIntrCtrl) {
            nicDisableInterrupt(prAdapter);
        }
        return TRUE;
    }
    else {
        DBGLOG(HAL, WARN, ("wlanISR triggered on ACPI-D3\n"));
        return FALSE;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanIST (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    if (arbFsmRunEventIST(prAdapter)) {
        nicEnableInterrupt(prAdapter);
    }

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanSendPacket (
    IN P_ADAPTER_T          prAdapter,
    IN P_PACKET_INFO_T      prPacketInfo
    )
{
    WLAN_STATUS rStatus;


    ASSERT(prAdapter);
    ASSERT(prPacketInfo);
    ASSERT(prPacketInfo->ucTID < TID_NUM);

    //4 <1> Check acceptable TX length if OS send unexpected large packets.
    if ((prPacketInfo->ucMacHeaderLength + prPacketInfo->u2PayloadLength) > CFG_TX_MAX_PKT_SIZE) {
        return WLAN_STATUS_FAILURE;
    }

#if 0 // NOTE: for optimization for power issue, this is removed to TX module inside
    ARB_ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
#else
    ARB_ACQUIRE_POWER_CONTROL_FROM_PM_IN_OS_TX_PATH(prAdapter);
#endif

    rStatus = arbFsmRunEventTxMsduFromOs(prAdapter, prPacketInfo);

    ARB_RECLAIM_POWER_CONTROL_TO_PM(prAdapter);

    return rStatus;

} /* end of wlanSendPacket() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanReturnPacket (
    IN P_ADAPTER_T prAdapter,
    IN PVOID pvPacket
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_SW_RFB_T prSWRfb = NULL;

    ASSERT(prAdapter);
    prRxCtrl = &prAdapter->rRxCtrl;

    if (pvPacket) {
        kalPacketFree(prAdapter->prGlueInfo, pvPacket);
    }
    if (QUEUE_IS_NOT_EMPTY(&prRxCtrl->rFreeRFBWOBuffList)) {
        QUEUE_REMOVE_HEAD(&prRxCtrl->rFreeRFBWOBuffList, prSWRfb, P_SW_RFB_T);
        if(!prSWRfb){
            return;
        }
        if(nicRxSetupRFB(prAdapter, prSWRfb)){
            QUEUE_INSERT_HEAD(&prRxCtrl->rFreeRFBWOBuffList, &prSWRfb->rQueEntry);
            return;
        }
        nicRxReturnRFB(prAdapter, prSWRfb);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanQueryInformation (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_OID_HANDLER_FUNC pfnOidQryHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    OUT PUINT_32            pu4QryInfoLen
    )
{
    WLAN_STATUS status = WLAN_STATUS_FAILURE;

    ASSERT(prAdapter);
    ASSERT(pu4QryInfoLen);

    if (wlanIsHandlerNeedHwAccess(pfnOidQryHandler, FALSE) && !IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        ARB_ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
        status = pfnOidQryHandler(prAdapter,
                                    pvInfoBuf,
                                    u4InfoBufLen,
                                    pu4QryInfoLen);

        ARB_RECLAIM_POWER_CONTROL_TO_PM(prAdapter);
    }
    else {
        status = pfnOidQryHandler(prAdapter,
                                    pvInfoBuf,
                                    u4InfoBufLen,
                                    pu4QryInfoLen);
    }

    return status;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanSetInformation (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_OID_HANDLER_FUNC pfnOidSetHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    OUT PUINT_32            pu4SetInfoLen
    )
{
    WLAN_STATUS status = WLAN_STATUS_FAILURE;

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    if (wlanIsHandlerNeedHwAccess(pfnOidSetHandler, TRUE) && !IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        ARB_ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

        status = pfnOidSetHandler(prAdapter,
                                    pvInfoBuf,
                                    u4InfoBufLen,
                                    pu4SetInfoLen);

        ARB_RECLAIM_POWER_CONTROL_TO_PM(prAdapter);
    }
    else {
        status = pfnOidSetHandler(prAdapter,
                                    pvInfoBuf,
                                    u4InfoBufLen,
                                    pu4SetInfoLen);
    }
    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanSetPromiscuousMode (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnablePromiscuousMode
    )
{
    ASSERT(prAdapter);

    if(fgEnablePromiscuousMode) {
        nicRxEnablePromiscuousMode(prAdapter);
    }
    else {
        nicRxDisablePromiscuousMode(prAdapter);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanRxSetBroadcast (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnableBroadcast
    )
{
    ASSERT(prAdapter);

    nicRxSetBroadcast(prAdapter, fgEnableBroadcast);
}


#if SUPPORT_WAPI
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
wlanSetWapiMode (
    IN P_ADAPTER_T          prAdapter,
    IN UINT_32              u4UseWapi
    )
{
    DEBUGFUNC("wlanSetWapiMode");
    ASSERT(prAdapter);

    if (u4UseWapi) {
        prAdapter->fgUseWapi = TRUE;
        prAdapter->rSecInfo.fgPrivacyCheckDisable = TRUE;
	    prAdapter->rSecInfo.fgBlockTxTraffic = TRUE;
        prAdapter->rSecInfo.fgBlockRxTraffic = TRUE;
    }
    else {
        prAdapter->fgUseWapi = FALSE;
        prAdapter->rSecInfo.fgPrivacyCheckDisable = FALSE;
        prAdapter->rSecInfo.fgBlockTxTraffic = FALSE;
        prAdapter->rSecInfo.fgBlockRxTraffic = FALSE;
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanQueryWapiMode (
    IN P_ADAPTER_T          prAdapter
    )
{
    DEBUGFUNC("wlanQueryWapiMode");
    ASSERT(prAdapter);

    return prAdapter->fgUseWapi;
}
#endif
