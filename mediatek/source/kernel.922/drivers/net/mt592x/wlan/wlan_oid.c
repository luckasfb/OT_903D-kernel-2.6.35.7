






#include "precomp.h"

extern PHY_ATTRIBUTE_T rPhyAttributes[];
extern const UINT_8 aucDataRate[];

#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugRateIndex[] = {
            "1M",
            "2M",
            "5_5M",
            "11M",
            "22M",
            "33M",
            "6M",
            "9M",
            "12M",
            "18M",
            "24M",
            "36M",
            "48M",
            "54M"};
/*lint -restore */
#endif /* DBG */

ENUM_POWER_SAVE_PROFILE_T aPowerModeToPwrSaveProfMapping[] =
{
    ENUM_PSP_CONTINUOUS_ACTIVE,     //Param_PowerModeCAM
    ENUM_PSP_CONTINUOUS_POWER_SAVE, //Param_PowerModeMAX_PSP
    ENUM_PSP_FAST_SWITCH            //Param_PowerModeFast_PSP
};


#if DBG
extern UINT_8  aucDebugModule[DBG_MODULE_NUM];
extern UINT_32 u4DebugModule;
UINT_32 u4DebugModuleTemp;
#endif /* DBG */





/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryNetworkTypesSupported (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    UINT_32 u4NumItem = 0;
    ENUM_PARAM_NETWORK_TYPE_T eSupportedNetworks[NETWORK_TYPE_NUM];
    PPARAM_NETWORK_TYPE_LIST prSupported;

    /* The array of all physical layer network subtypes that the driver supports. */

    DEBUGFUNC("wlanoidQueryNetworkTypesSupported");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    ASSERT(pvQueryBuffer);

    /* Init. */
    for (u4NumItem = 0; u4NumItem < NETWORK_TYPE_NUM; u4NumItem++) {
        eSupportedNetworks[u4NumItem] = 0;
    }

    u4NumItem = 0;

    eSupportedNetworks[u4NumItem] = NETWORK_TYPE_DS;
    u4NumItem ++;

    eSupportedNetworks[u4NumItem] = NETWORK_TYPE_OFDM24;
    u4NumItem ++;

    *pu4QueryInfoLen =
        (UINT_32)OFFSET_OF(PARAM_NETWORK_TYPE_LIST, eNetworkType) +
        (u4NumItem * sizeof(ENUM_PARAM_NETWORK_TYPE_T));

    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    prSupported = (PPARAM_NETWORK_TYPE_LIST)pvQueryBuffer;
    prSupported->NumberOfItems = u4NumItem;
    kalMemCopy(prSupported->eNetworkType,
        eSupportedNetworks,
        u4NumItem * sizeof(ENUM_PARAM_NETWORK_TYPE_T));

    DBGLOG(REQ,
        TRACE,
        ("NDIS supported network type list: %ld\n",
        prSupported->NumberOfItems));
    DBGLOG_MEM8(REQ, INFO, prSupported, *pu4QueryInfoLen);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryNetworkTypesSupported */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryNetworkTypeInUse (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_BSS_INFO_T prCurrentBss;
    ENUM_PARAM_NETWORK_TYPE_T rCurrentNetworkTypeInUse = NETWORK_TYPE_OFDM24;

    DEBUGFUNC("wlanoidQueryNetworkTypeInUse");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    if (u4QueryBufferLen < sizeof(ENUM_PARAM_NETWORK_TYPE_T)) {
        *pu4QueryInfoLen = sizeof(ENUM_PARAM_NETWORK_TYPE_T);
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    prCurrentBss = &prAdapter->rBssInfo;


    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        switch (prCurrentBss->ePhyType) {
        case PHY_TYPE_ERP_INDEX:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_OFDM24;
            DBGLOG(REQ,
                INFO,
                ("Current network type in use: OFDM24 (connected)\n"));
            break;

        case PHY_TYPE_HR_DSSS_INDEX:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_DS;
                DBGLOG(REQ,
                    INFO,
                ("Current network type in use: DS (connected)\n"));
            break;

        case PHY_TYPE_OFDM_INDEX:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_OFDM5;
                DBGLOG(REQ,
                    INFO,
                ("Current network type in use: OFDM5 (connected)\n"));
            break;

        default:
            ASSERT(0); /* Make sure we have handle all STATEs */

        }
    }
    else {
        //kent below line for test
        switch (prAdapter->rConnSettings.eDesiredPhyType) {
        case PHY_TYPE_802_11ABG:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_AUTOMODE;
            DBGLOG(REQ, INFO, ("Current network type in use: AUTO Mode\n"));
            break;
        case PHY_TYPE_802_11BG:
        case PHY_TYPE_802_11G:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_OFDM24;
            DBGLOG(REQ, INFO, ("Current network type in use: OFDM24 Mode\n"));
            break;
        case PHY_TYPE_802_11A:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_OFDM5;
            DBGLOG(REQ, INFO, ("Current network type in use: OFDM5 Mode\n"));
            break;
        case PHY_TYPE_802_11B:
            rCurrentNetworkTypeInUse = NETWORK_TYPE_DS;
            DBGLOG(REQ, INFO, ("Current network type in use: DS Mode\n"));
            break;
        default:
            DBGLOG(REQ, INFO, ("Current network type in use: Unknown\n"));
        }
    }

    ASSERT(pvQueryBuffer);
    *(P_ENUM_PARAM_NETWORK_TYPE_T)pvQueryBuffer = rCurrentNetworkTypeInUse;
    *pu4QueryInfoLen = sizeof(ENUM_PARAM_NETWORK_TYPE_T);

    DBGLOG(REQ, TRACE, ("Network type in use: %d\n", rCurrentNetworkTypeInUse));

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryNetworkTypeInUse */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetNetworkTypeInUse (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    ENUM_PARAM_NETWORK_TYPE_T eNewNetworkType;
    ENUM_PARAM_PHY_TYPE_T ePhyType = PHY_TYPE_802_11ABG;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    DEBUGFUNC("wlanoidSetNetworkTypeInUse");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (u4SetBufferLen < sizeof(ENUM_PARAM_NETWORK_TYPE_T)) {
        *pu4SetInfoLen = sizeof(ENUM_PARAM_NETWORK_TYPE_T);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    eNewNetworkType = *(P_ENUM_PARAM_NETWORK_TYPE_T)pvSetBuffer;
    *pu4SetInfoLen = sizeof(ENUM_PARAM_NETWORK_TYPE_T);

    DBGLOG(REQ,
        INFO,
        ("New network type: %d mode\n", eNewNetworkType));

    switch (eNewNetworkType) {

    case NETWORK_TYPE_DS:
        ePhyType = PHY_TYPE_802_11B;
        rStatus = nicCheckAvailablePhyTypeSet(prAdapter, ePhyType);
        break;

    case NETWORK_TYPE_OFDM5:
        ePhyType = PHY_TYPE_802_11A;
        rStatus = nicCheckAvailablePhyTypeSet(prAdapter, ePhyType);
        break;

    case NETWORK_TYPE_OFDM24:
        ePhyType = PHY_TYPE_802_11G;
        rStatus = nicCheckAvailablePhyTypeSet(prAdapter, ePhyType);
        break;

    case NETWORK_TYPE_AUTOMODE:
        ePhyType = PHY_TYPE_802_11ABG;
        rStatus = nicCheckAvailablePhyTypeSet(prAdapter, ePhyType);
        break;

    case NETWORK_TYPE_FH:
        DBGLOG(REQ, INFO, ("Not support network type: %d\n", eNewNetworkType));
        rStatus = WLAN_STATUS_NOT_SUPPORTED;
        break;

    default:
        DBGLOG(REQ, INFO, ("Unknown network type: %d\n", eNewNetworkType));
        rStatus = WLAN_STATUS_INVALID_DATA;
        break;
    }

    /* Verify if we support the new network type. */
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("Unknown network type: %d\n", eNewNetworkType));
    }
    else {
        /* Save  network type in use. */
        prAdapter->rConnSettings.eDesiredPhyType = ePhyType;
        nicSetAvailablePhyTypeSet(prAdapter);
    }

    return rStatus;
} /* wlanoidSetNetworkTypeInUse */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryBssid (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryBssid");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = MAC_ADDR_LEN;

    if (u4QueryBufferLen < MAC_ADDR_LEN) {
        ASSERT(pu4QueryInfoLen);
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(u4QueryBufferLen >= MAC_ADDR_LEN);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return rStatus;
    }

    if (prAdapter->eConnectionStateIndicated == MEDIA_STATE_CONNECTED) {
        COPY_MAC_ADDR(pvQueryBuffer, prAdapter->rBssInfo.aucBSSID);

        DBGLOG(REQ,
            TRACE,
            ("Associated BSSID: "MACSTR"\n",
            MAC2STR(prAdapter->rBssInfo.aucBSSID)));
    }
    else if (prAdapter->rConnSettings.eOPMode == NET_TYPE_IBSS) {
        UINT_8 aucTemp[MAC_ADDR_LEN] = {0};
        // return IBSS MAC Address as described by MSDN
        if (!kalMemCmp(prAdapter->rBssInfo.aucBSSID, aucTemp, MAC_ADDR_LEN)) {
            /* all 0 */
            COPY_MAC_ADDR(aucTemp, prAdapter->aucMacAddress);
            aucTemp[0] &= ~BIT(0); // 7.1.3.3.3 - The individual/group bit of the address is set to 0.
            aucTemp[0] |= BIT(1); // 7.1.3.3.3 - The universal/local bit of the address is set to 1.
            COPY_MAC_ADDR(pvQueryBuffer, aucTemp);
        }
        else {
            COPY_MAC_ADDR(pvQueryBuffer, prAdapter->rBssInfo.aucBSSID);
        }
        DBGLOG(REQ,
            TRACE,
            ("Current BSSID in: "MACSTR" (MAC addr)\n",
            MAC2STR(pvQueryBuffer)));

    }
    else {
        DBGLOG(REQ, TRACE, ("The NIC is not associated with an access point.\n"));
        rStatus = WLAN_STATUS_ADAPTER_NOT_READY;
    }

    return rStatus;
} /* wlanoidQueryBssid */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanoidBuildBssListFromScanResult (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvDest,
    IN  UINT_32           u4MaxBufLen
    )
{
    P_PARAM_BSSID_EX_T prBssidEx;
    P_PARAM_BSSID_LIST_EX_T prList;
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prScanResult = (P_BSS_DESC_T)NULL;
    PUINT_8 cp;
    UINT_32 tmpLen = 0;
    UINT_32 u4BssRecLen;
    UINT_8 rateLen;

    DEBUGFUNC("wlanoidBuildBssListFromScanResult");

    ASSERT(prAdapter);
    ASSERT(pvDest);

    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    prList = (P_PARAM_BSSID_LIST_EX_T)pvDest;
    cp = (PUINT_8)&prList->arBssid[0];

    tmpLen = OFFSET_OF(PARAM_BSSID_LIST_EX_T, arBssid);

    prList->u4NumberOfItems = 0;

    /* Maximum report CFG_MAX_NUM_BSS_LIST */
    ASSERT(prAdapter->rScanInfo.rBSSDescList.u4NumElem <= CFG_MAX_NUM_BSS_LIST);

    /* Refresh the list of BSSIDs with the newest scan results. */
    LINK_FOR_EACH_ENTRY(prScanResult, prBSSDescList, rLinkEntry, BSS_DESC_T) {

        BSS_DESC_CHK_GUID(prScanResult);

        DBGLOG(REQ,
            INFO,
            ("SSID: %s Channel: %d BCN Interval: %d\n",
            prScanResult->aucSSID,
            prScanResult->ucChannelNum,
            prScanResult->u2BeaconInterval));

        /* Update PARAM_BSSID_EX */
        prBssidEx = (P_PARAM_BSSID_EX_T)cp;

        /* Calculate this BSS's record length and make sure buffer is enough
        * This may happen when scan is not complete and OS query scan result.
        */
        u4BssRecLen = ((UINT_32)OFFSET_OF(PARAM_BSSID_EX_T, aucIEs) +
            prScanResult->u2IELength + 3) & 0xfffffffc;

        if (tmpLen + u4BssRecLen > u4MaxBufLen)
            break;


        /* Fill in the BSSID list with the scan result. */
        COPY_MAC_ADDR((PVOID)prBssidEx->arMacAddress, prScanResult->aucBSSID);

        COPY_SSID(prBssidEx->rSsid.aucSsid,
                  prBssidEx->rSsid.u4SsidLen,
                  prScanResult->aucSSID,
                  (UINT_32)prScanResult->ucSSIDLen);

        if (prScanResult->u2CapInfo & CAP_INFO_PRIVACY) {
            prBssidEx->u4Privacy = 1;
        }
        else {
            prBssidEx->u4Privacy = 0;
        }

        prBssidEx->rRssi = RCPI_TO_dBm(prScanResult->rRcpi);

        /*For WHQL test, Rssi should be in range -10 ~ -200 dBm*/
        if(prBssidEx->rRssi > PARAM_WHQL_RSSI_MAX_DBM) {
            prBssidEx->rRssi = PARAM_WHQL_RSSI_MAX_DBM;
        }

        if (prScanResult->ePhyType == PHY_TYPE_ERP_INDEX) {
            prBssidEx->eNetworkTypeInUse = NETWORK_TYPE_OFDM24;
        }
        else if (prScanResult->ePhyType == PHY_TYPE_HR_DSSS_INDEX) {
            prBssidEx->eNetworkTypeInUse = NETWORK_TYPE_DS;
        }
        else {
            ASSERT(prScanResult->ePhyType == PHY_TYPE_OFDM_INDEX);
            prBssidEx->eNetworkTypeInUse = NETWORK_TYPE_OFDM5;
        }

        prBssidEx->rConfiguration.u4Length = sizeof(PARAM_802_11_CONFIG_T);
        prBssidEx->rConfiguration.u4BeaconPeriod =
            (UINT_32)prScanResult->u2BeaconInterval;
        prBssidEx->rConfiguration.u4ATIMWindow =
            (UINT_32)prScanResult->u2ATIMWindow;

        {
            P_RF_CHANNEL_PROG_ENTRY prChProgEntry;

            prChProgEntry =
                halRFGetRFChnlProgEntry(prAdapter,
                                        (UINT_8)prScanResult->ucChannelNum ,
                                        prScanResult->eBand);
            if (prChProgEntry) {
                prBssidEx->rConfiguration.u4DSConfig = prChProgEntry->u4ChannelFreq;
            }
        }

        prBssidEx->rConfiguration.rFHConfig.u4Length =
            sizeof(PARAM_802_11_CONFIG_FH_T);

        if (prScanResult->eBSSType == BSS_TYPE_INFRASTRUCTURE) {
            prBssidEx->eOpMode = NET_TYPE_INFRA;
        }
        else {
            prBssidEx->eOpMode = NET_TYPE_IBSS;
        }

        /* Build supRates from rate set */
        rateGetDataRatesFromRateSet(
            prScanResult->u2OperationalRateSet,
            0x0, /*(UINT_16)NULL*/
            prBssidEx->rSupportedRates,
            &rateLen);

        DBGLOG(REQ,
            TRACE,
            ("Rate Len:%d Scan BSS:0x%08x\n",
            rateLen,
            prScanResult->u2OperationalRateSet));
#if DBG
        DBGLOG(REQ,
            INFO,
            ("Rates %d %d %d %d %d %d %d %d - %d %d %d %d %d %d %d %d\n",
            prBssidEx->rSupportedRates[0],
            prBssidEx->rSupportedRates[1],
            prBssidEx->rSupportedRates[2],
            prBssidEx->rSupportedRates[3],
            prBssidEx->rSupportedRates[4],
            prBssidEx->rSupportedRates[5],
            prBssidEx->rSupportedRates[6],
            prBssidEx->rSupportedRates[7],
            prBssidEx->rSupportedRates[8],
            prBssidEx->rSupportedRates[9],
            prBssidEx->rSupportedRates[10],
            prBssidEx->rSupportedRates[11],
            prBssidEx->rSupportedRates[12],
            prBssidEx->rSupportedRates[13],
            prBssidEx->rSupportedRates[14],
            prBssidEx->rSupportedRates[15]));
#endif

        prBssidEx->u4IELength = prScanResult->u2IELength;
        kalMemCopy((PVOID)prBssidEx->aucIEs,
            (PVOID)prScanResult->aucIEBuf,
            prScanResult->u2IELength);

        prBssidEx->u4Length = ALIGN_4((UINT_32)OFFSET_OF(PARAM_BSSID_EX_T, aucIEs) + prBssidEx->u4IELength);

        DBGLOG(RSN, INFO, DUMPVAR(prBssidEx->u4Length, "%ld"));
        DBGLOG(RSN, INFO, DUMPVAR(prBssidEx->u4IELength, "%ld"));

        cp += prBssidEx->u4Length;

        tmpLen += prBssidEx->u4Length;
        prList->u4NumberOfItems ++;
    }

    DBGLOG(REQ, TRACE, ("Result:%ld total len:%ld\n", prList->u4NumberOfItems, tmpLen));

    ASSERT(prList->u4NumberOfItems <= CFG_MAX_NUM_BSS_LIST);

    return TRUE;
} /* wlanoidBuildBssListFromScanResult */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryBssidList (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_SCAN_INFO_T prScanInfo;
    P_LINK_T prBSSDescList;
    P_BSS_DESC_T prBssDesc = (P_BSS_DESC_T)NULL;
    UINT_32 u4BssidListExLen;

    DEBUGFUNC("wlanoidQueryBssidList");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    prScanInfo =  &prAdapter->rScanInfo;
    prBSSDescList = &prScanInfo->rBSSDescList;

    DBGLOG(REQ, INFO, DUMPVAR(prBSSDescList->u4NumElem, "%lu"));

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in qeury BSSID list! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        u4BssidListExLen = sizeof(PARAM_BSSID_LIST_EX_T);
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

#if !defined(LINUX)
    if (prAdapter->fgIsRadioOff) {
        u4BssidListExLen = sizeof(PARAM_BSSID_LIST_EX_T);
    }
    else
#endif /* LINUX */
    {
        u4BssidListExLen = 0;

        if (LINK_IS_VALID(prBSSDescList)) {
            LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry, BSS_DESC_T) {

                BSS_DESC_CHK_GUID(prBssDesc);

                u4BssidListExLen += (sizeof(PARAM_BSSID_EX_T) - 1 +
                    ALIGN_4(prBssDesc->u2IELength));
            }
        }

        if (u4BssidListExLen) {
            u4BssidListExLen += 4; // u4NumberOfItems.
        }
        else {
            u4BssidListExLen = sizeof(PARAM_BSSID_LIST_EX_T);
        }
    }

    *pu4QueryInfoLen = u4BssidListExLen;

    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    /* Clear the buffer */
    kalMemZero(pvQueryBuffer, u4BssidListExLen);

    /* Build up BSSID list from scan result */
#if !defined(LINUX)
    if (!prAdapter->fgIsRadioOff)
#endif /* LINUX */
    {
        if (prBSSDescList->u4NumElem) {
            wlanoidBuildBssListFromScanResult(prAdapter, pvQueryBuffer, u4QueryBufferLen);
        }
    }

    DBGLOG(REQ, LOUD, ("NDIS extended BSSID list:\n"));
    DBGLOG_MEM8(REQ, LOUD, pvQueryBuffer, *pu4QueryInfoLen);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryBssidList */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetBssidListScan (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    SCAN_REQ_CONFIG_T rScanReqConfig;
    BOOLEAN fgIsSpecifiedSSID = FALSE;
    WLAN_STATUS rStatus;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("wlanoidSetBssidListScan");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = u4SetBufferLen;

    prConnSettings = &prAdapter->rConnSettings;

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

#if CFG_LP_IOT
    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        return WLAN_STATUS_SUCCESS;
    }
#endif

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set BSSID list scan! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    if (prAdapter->fgIsRadioOff) {
        DBGLOG(REQ, WARN, ("Return from BSSID list scan! (radio off). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_SUCCESS;
    }

    kalMemZero(&rScanReqConfig.rSpecifiedSsid,
               sizeof(PARAM_SSID_T));

    if ((prAdapter->rConnSettings.fgIsConnReqIssued) &&
        (prAdapter->rConnSettings.eConnectionPolicy != CONNECT_BY_SSID_ANY)) {
		//add by mtk80743
		if(pvSetBuffer != NULL && u4SetBufferLen != 0){
			COPY_SSID(prAdapter->rConnSettings.aucSSID,
					  prAdapter->rConnSettings.ucSSIDLen,
						pvSetBuffer,
						u4SetBufferLen);

		}else{//end mtk80743
        COPY_SSID(rScanReqConfig.rSpecifiedSsid.aucSsid,
                  rScanReqConfig.rSpecifiedSsid.u4SsidLen,
                  prAdapter->rConnSettings.aucSSID,
                  prAdapter->rConnSettings.ucSSIDLen);
		}

        fgIsSpecifiedSSID = TRUE;

        DBGLOG(REQ, TRACE, ("Scan SSID:%s SSID Len:%d\n",
            prAdapter->rConnSettings.aucSSID,
            prAdapter->rConnSettings.ucSSIDLen));
    }

    if (prAdapter->eConnectionState == MEDIA_STATE_DISCONNECTED) {

        rScanReqConfig.eScanMethod = SCAN_METHOD_FULL_SCAN;
        /* 12 milliseconds. If detect the air activity by MDRDY,
         * this is the transmission time of 1Mbps with 1536 Bytes.
         */
        rScanReqConfig.ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

        /* N milliseconds = 100TU(Beacon Interval) - min + 10TU
         * (if Beacon Drift) = 98TU(round down to millisecond).
         */
        rScanReqConfig.ucChnlDwellTimeExt = SCAN_CHANNEL_DWELL_TIME_EXT;

        rScanReqConfig.ucNumOfPrbReq = FULL_SCAN_TOTAL_PROBE_REQ_NUM;
        rScanReqConfig.ucNumOfSpecifiedSsidPrbReq =
            fgIsSpecifiedSSID ? FULL_SCAN_SPECIFIC_PROBE_REQ_NUM : 0;
    }
    else {

        if (PM_IS_VOIP_POLLING_ENABLED(prAdapter)) {
            rScanReqConfig.eScanMethod = SCAN_METHOD_VOIP_ONLINE_SCAN;

            /* 12 milliseconds. If detect the air activity by MDRDY,
             * this is the transmission time of 1Mbps with 1536 Bytes.
             */
            rScanReqConfig.ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

            /* 20 milliseconds for VOIP application, N = 20 - Min(12) = 8 milliseconds */
            rScanReqConfig.ucChnlDwellTimeExt = VOIP_SCAN_CHANNEL_DWELL_TIME_EXT;

            rScanReqConfig.ucNumOfPrbReq = PS_VOIP_PARTIAL_SCAN_TOTAL_PROBE_REQ_NUM;
            rScanReqConfig.ucNumOfSpecifiedSsidPrbReq = PS_VOIP_PARTIAL_SCAN_SPECIFIC_PROBE_REQ_NUM;
        }
        else if (prConnSettings->fgIsVoipConn) {
            rScanReqConfig.eScanMethod = SCAN_METHOD_VOIP_ONLINE_SCAN;

            /* 12 milliseconds. If detect the air activity by MDRDY,
             * this is the transmission time of 1Mbps with 1536 Bytes.
             */
            rScanReqConfig.ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

            /* 20 milliseconds for VOIP application, N = 20 - Min(12) = 8 milliseconds */
            rScanReqConfig.ucChnlDwellTimeExt = VOIP_SCAN_CHANNEL_DWELL_TIME_EXT;

            rScanReqConfig.ucNumOfPrbReq = VOIP_PARTIAL_SCAN_TOTAL_PROBE_REQ_NUM;
            rScanReqConfig.ucNumOfSpecifiedSsidPrbReq = VOIP_PARTIAL_SCAN_SPECIFIC_PROBE_REQ_NUM;
        }
        else {
            rScanReqConfig.eScanMethod = SCAN_METHOD_ONLINE_SCAN;

            /* 12 milliseconds. If detect the air activity by MDRDY,
             * this is the transmission time of 1Mbps with 1536 Bytes.
             */
            rScanReqConfig.ucChnlDwellTimeMin = SCAN_CHANNEL_DWELL_TIME_MIN;

            /* N milliseconds = 100TU(Beacon Interval) - min + 10TU
             * (if Beacon Drift) = 98TU(round down to millisecond).
             */
            rScanReqConfig.ucChnlDwellTimeExt = SCAN_CHANNEL_DWELL_TIME_EXT;

            rScanReqConfig.ucNumOfPrbReq = PARTIAL_SCAN_TOTAL_PROBE_REQ_NUM;
            rScanReqConfig.ucNumOfSpecifiedSsidPrbReq =
                fgIsSpecifiedSSID ? PARTIAL_SCAN_SPECIFIC_PROBE_REQ_NUM : 0;
        }
    }

    rScanReqConfig.eScanType = SCAN_TYPE_ACTIVE_SCAN;

    rScanReqConfig.pfScanDoneHandler = (PFN_SCAN_DONE_HANDLER)0;

    {
        UINT_32 i, j, u4ScanIndex = 0;

        rScanReqConfig.ucNumOfScanChnl = (UINT_8)prAdapter->u2NicOpChnlNum;

        for (j = 0; j < INTERLACED_SCAN_CHANNEL_GROUPS_NUM; j++) {
            for (i = j;
                 ((i < rScanReqConfig.ucNumOfScanChnl)
                    && (i < MAXIMUM_OPERATION_CHANNEL_LIST)
                    && (u4ScanIndex < sizeof(rScanReqConfig.arChnlInfoList) /sizeof(rScanReqConfig.arChnlInfoList[0])));
                 i += INTERLACED_SCAN_CHANNEL_GROUPS_NUM, u4ScanIndex++) {

                rScanReqConfig.arChnlInfoList[u4ScanIndex].ucChannelNum =
                    prAdapter->arNicOpChnList[i].ucChannelNum;
                rScanReqConfig.arChnlInfoList[u4ScanIndex].eBand =
                    prAdapter->arNicOpChnList[i].eBand;
            }
        }
    }

    if ((rStatus = arbFsmRunEventScanRequest(prAdapter, &rScanReqConfig, FALSE)) ==
        WLAN_STATUS_FAILURE) {
        DBGLOG(REQ, TRACE, ("Scan Request was failed due to another SCAN is proceeding.\n"));

        rStatus = WLAN_STATUS_SUCCESS;
        /* NOTE(Kevin): What will happen if reply the WLAN_STATUS_FAILURE to OID */
    }
    else {
        /* NOTE(Kevin): Because we always return SUCCESS, so we should do aging at
         * least for OID_SCAN.
         */
        scanRemoveBssDescsByPolicy(prAdapter, (SCAN_RM_POLICY_EXCLUDE_CONNECTED | \
                                               SCAN_RM_POLICY_TIMEOUT) );
    }

    return rStatus;
} /* wlanoidSetBssidListScan */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetBssid (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    UINT_8 aucAnyBSSID[] = BC_BSSID;
    UINT_8 aucNullBSSID[] = NULL_MAC_ADDR;
    PUINT_8 pucNewBSSID;
    BOOLEAN fgTryToAssociate = FALSE;


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);
    prConnSettings = &prAdapter->rConnSettings;

    DBGLOG(REQ, TRACE, ("\n"));

    *pu4SetInfoLen = MAC_ADDR_LEN;

    if (u4SetBufferLen != MAC_ADDR_LEN) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->fgIsRadioOff || (prAdapter->rAcpiState == ACPI_STATE_D3)) {
        DBGLOG(REQ, WARN, ("Fail in set BSSID! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    pucNewBSSID = (PUINT_8)pvSetBuffer;

    if (EQUAL_MAC_ADDR(pucNewBSSID, aucNullBSSID)) {
        DBGLOG(REQ, TRACE, ("Can't accept null BSSID\n"));
        return WLAN_STATUS_INVALID_DATA;
    }

    /* NOTE(Kevin): We do association or reassociation only if the desired BSSID
     * is changed. We don't care if it is not changed from MSDN.
     */
    if (UNEQUAL_MAC_ADDR(prConnSettings->aucBSSID, pucNewBSSID)) {

        fgTryToAssociate = TRUE;

        /* Update the BSSID in Connection Settings */
        COPY_MAC_ADDR(prConnSettings->aucBSSID, pucNewBSSID);

        /* Update the related flags */
        if (EQUAL_MAC_ADDR(prConnSettings->aucBSSID, aucAnyBSSID)) {
            prConnSettings->fgIsConnByBssidIssued = FALSE;
            prConnSettings->fgIsEnableRoaming = TRUE;
        }
        else {
            prConnSettings->fgIsConnByBssidIssued = TRUE;
            prConnSettings->fgIsEnableRoaming = FALSE;
        }
    }


    /* NOTE(Kevin): We only do association within the current Extended Service Set.
     * Which means we should know the connection policy of SSID first.
     */
    if ((fgTryToAssociate) &&
        (prConnSettings->fgIsConnReqIssued)) {

        //Housekeeping for STA_RECORD_T aging func, do it only at Connection Req Event.
        staRecRemoveStaRecordByPolicy(prAdapter, STA_RECORD_RM_POLICY_TIMEOUT);

        /* Send the "Abort" event to arbiter. Always TRUE for delaying Disconnect
         * Indication.
         */
        arbFsmRunEventAbort(prAdapter, TRUE); /* Reset to STANDBY */
    }

    /* TODO(Kevin): Driver should attempt to associate with the BSSID even if the
     * BSSID is not in the BSSID scan list.
     */

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetBssid() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetSsid (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_SSID_T prNewSsid;
    P_CONNECTION_SETTINGS_T prConnSettings;
    BOOLEAN fgIsDelayMediaStateIndication = FALSE;
    P_SW_KEY_STRUC_T         prDefSwKey = NULL;

    DEBUGFUNC("wlanoidSetSsid");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);
    prConnSettings = &prAdapter->rConnSettings;

    DBGLOG(REQ, TRACE, ("\n"));

    *pu4SetInfoLen = sizeof(PARAM_SSID_T);

    if (u4SetBufferLen < *pu4SetInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4SetBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set SSID! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    prNewSsid = (P_PARAM_SSID_T)pvSetBuffer;

    /* Verify the length of the new SSID. */
    if (prNewSsid->u4SsidLen > PARAM_MAX_LEN_SSID) {
        DBGLOG(REQ,
            TRACE,
            ("Invalid SSID length %lu\n",
            prNewSsid->u4SsidLen));
        return WLAN_STATUS_INVALID_DATA;
    }

    /* WPA-NONE, at Windows Mobile 6.x, the encryption status is set after the key,
           For TKIP/AES, key length is always 32, so we needed to update the cipher at here,
           to let the data path can work */
    if (prConnSettings->eAuthMode == AUTH_MODE_WPA_NONE) {

        prDefSwKey = (P_SW_KEY_STRUC_T)&prAdapter->arWlanCtrl[WLAN_TABLE_DEFAULT_ENTRY].rSWKey;

        if (prConnSettings->eEncStatus == ENUM_ENCRYPTION2_ENABLED) {
            prDefSwKey->ucCipher = CIPHER_SUITE_TKIP;
        }
        else if (prConnSettings->eEncStatus == ENUM_ENCRYPTION3_ENABLED) {
            prDefSwKey->ucCipher = CIPHER_SUITE_CCMP;
        }

        if (prDefSwKey->ucCipher == CIPHER_SUITE_TKIP ||
            prDefSwKey->ucCipher == CIPHER_SUITE_CCMP) {
            nicPrivacyDumpWlanTable(prAdapter, WLAN_TABLE_DEFAULT_ENTRY);

            halWlanTableAccess(prAdapter,
                               (PVOID)prDefSwKey,
                                WLAN_TABLE_DEFAULT_ENTRY,
                                HWTDR_UPDATE_MODE_3);     /* Set key into H/W key table */
        }
    }

    if (EQUAL_SSID(prConnSettings->aucSSID, prConnSettings->ucSSIDLen,
            prNewSsid->aucSsid, (UINT_8)prNewSsid->u4SsidLen)) {
        fgIsDelayMediaStateIndication = TRUE;
    }
    else {
        fgIsDelayMediaStateIndication = FALSE;

        /* Set the SSID with the specified value or set it to any SSID if the SSID
           is not specified. */
        kalMemZero(prConnSettings->aucSSID, sizeof(prConnSettings->aucSSID));
        COPY_SSID(prConnSettings->aucSSID,
                  prConnSettings->ucSSIDLen,
                  prNewSsid->aucSsid,
                  (UINT_8)prNewSsid->u4SsidLen);
    }

    DBGLOG(REQ, INFO, ("New SSID:%s SSID Len:%d\n",
        prConnSettings->aucSSID, prConnSettings->ucSSIDLen));

    prConnSettings->fgIsConnReqIssued = TRUE;

    if (prConnSettings->ucSSIDLen) {
        prConnSettings->eConnectionPolicy = CONNECT_BY_SSID_BEST_RSSI;
    }
    else {
        prConnSettings->eConnectionPolicy = CONNECT_BY_SSID_ANY;
    }


    //Housekeeping for STA_RECORD_T aging func, do it only at Connection Req Event.
    staRecRemoveStaRecordByPolicy(prAdapter, STA_RECORD_RM_POLICY_TIMEOUT);

    /* Send the "Abort" event to arbiter. */
    arbFsmRunEventAbort(prAdapter, fgIsDelayMediaStateIndication); /* Reset to STANDBY */

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetSsid() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQuerySsid (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_PARAM_SSID_T prAssociatedSsid;

    DEBUGFUNC("wlanoidQuerySsid");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(PARAM_SSID_T);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    prAssociatedSsid = (P_PARAM_SSID_T)pvQueryBuffer;

    kalMemZero(prAssociatedSsid->aucSsid, sizeof(prAssociatedSsid->aucSsid));

    if (prAdapter->eConnectionStateIndicated == MEDIA_STATE_CONNECTED) {
        prAssociatedSsid->u4SsidLen = (UINT_32)prAdapter->rConnSettings.ucSSIDLen;

        if (prAdapter->rConnSettings.ucSSIDLen) {
            kalMemCopy(prAssociatedSsid->aucSsid,
                prAdapter->rConnSettings.aucSSID,
                prAdapter->rConnSettings.ucSSIDLen);
        }

        DBGLOG(REQ,
            TRACE,
            ("Currently associated SSID: \"%s\"\n",
            prAdapter->rConnSettings.aucSSID));
    }
    else {
        prAssociatedSsid->u4SsidLen = 0;

        DBGLOG(REQ, TRACE, ("Null SSID\n"));
    }

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQuerySsid */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryInfrastructureMode (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryInfrastructureMode");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(ENUM_PARAM_OP_MODE_T);

    if (u4QueryBufferLen < sizeof(ENUM_PARAM_OP_MODE_T)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    *(P_ENUM_PARAM_OP_MODE_T)pvQueryBuffer =
        prAdapter->rConnSettings.eOPMode;

    /*
    ** According to OID_802_11_INFRASTRUCTURE_MODE
    ** If there is no prior OID_802_11_INFRASTRUCTURE_MODE,
    ** NDIS_STATUS_ADAPTER_NOT_READY shall be returned.
    */
#if DBG
    switch (*(P_ENUM_PARAM_OP_MODE_T)pvQueryBuffer) {
        case NET_TYPE_IBSS:
             DBGLOG(REQ, INFO, ("IBSS mode\n"));
             break;
        case NET_TYPE_INFRA:
             DBGLOG(REQ, INFO, ("Infrastructure mode\n"));
             break;
        default:
             DBGLOG(REQ, INFO, ("Automatic mode\n"));
    }
#endif

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryInfrastructureMode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetInfrastructureMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    ENUM_PARAM_OP_MODE_T eOpMode;

    DEBUGFUNC("wlanoidSetInfrastructureMode");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(ENUM_PARAM_OP_MODE_T);

    if (u4SetBufferLen < sizeof(ENUM_PARAM_OP_MODE_T)) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4SetBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set infrastructure mode! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    eOpMode = *(P_ENUM_PARAM_OP_MODE_T)pvSetBuffer;
    /* Verify the new infrastructure mode. */
    if (eOpMode >= NET_TYPE_NUM) {
        DBGLOG(REQ, TRACE, ("Invalid mode value 0x%x (0x%p)\n", eOpMode, pvSetBuffer));
        return WLAN_STATUS_INVALID_DATA;
    }

#if DBG
    switch (eOpMode) {
        case NET_TYPE_IBSS:
             DBGLOG(REQ, INFO, ("New mode: IBSS mode\n"));
             break;

        case NET_TYPE_INFRA:
             DBGLOG(REQ, INFO, ("New mode: Infrastructure mode\n"));
             break;

        default:
             DBGLOG(REQ, INFO, ("New mode: Automatic mode\n"));
    }
#endif

    /* Save the new infrastructure mode setting. */
    prAdapter->rConnSettings.eOPMode = eOpMode;

    /* Clean S/W and H/W key table */
    privacyInitialize(prAdapter);

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetInfrastructureMode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryAuthMode (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryAuthMode");

    ASSERT(prAdapter);

    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(ENUM_PARAM_AUTH_MODE_T);

    if (u4QueryBufferLen < sizeof(ENUM_PARAM_AUTH_MODE_T)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    *(P_ENUM_PARAM_AUTH_MODE_T)pvQueryBuffer = prAdapter->rConnSettings.eAuthMode;

#if DBG
    switch (*(P_ENUM_PARAM_AUTH_MODE_T)pvQueryBuffer) {
    case AUTH_MODE_OPEN:
        DBGLOG(REQ, INFO, ("Current auth mode: Open\n"));
        break;

    case AUTH_MODE_SHARED:
        DBGLOG(REQ, INFO, ("Current auth mode: Shared\n"));
        break;

    case AUTH_MODE_AUTO_SWITCH:
        DBGLOG(REQ, INFO, ("Current auth mode: Auto-switch\n"));
        break;

    case AUTH_MODE_WPA:
        DBGLOG(REQ, INFO, ("Current auth mode: WPA\n"));
        break;

    case AUTH_MODE_WPA_PSK:
        DBGLOG(REQ, INFO, ("Current auth mode: WPA PSK\n"));
        break;

    case AUTH_MODE_WPA_NONE:
        DBGLOG(REQ, INFO, ("Current auth mode: WPA None\n"));
        break;

    case AUTH_MODE_WPA2:
        DBGLOG(REQ, INFO, ("Current auth mode: WPA2\n"));
        break;

    case AUTH_MODE_WPA2_PSK:
        DBGLOG(REQ, INFO, ("Current auth mode: WPA2 PSK\n"));
        break;

    default:
        DBGLOG(REQ,
            INFO,
            ("Current auth mode: %d\n",
            *(P_ENUM_PARAM_AUTH_MODE_T)pvQueryBuffer));
    }
#endif
    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryAuthMode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAuthMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    UINT_32               u4AkmSuite, i;
    P_DOT11_RSNA_CONFIG_AUTHENTICATION_SUITES_ENTRY prEntry;

    DEBUGFUNC("wlanoidSetAuthMode");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);
    ASSERT(pvSetBuffer);

    *pu4SetInfoLen = sizeof(ENUM_PARAM_AUTH_MODE_T);

    if (u4SetBufferLen < sizeof(ENUM_PARAM_AUTH_MODE_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set Authentication mode! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    /* Check if the new authentication mode is valid. */
    if (*(P_ENUM_PARAM_AUTH_MODE_T)pvSetBuffer >= AUTH_MODE_NUM) {
        DBGLOG(REQ,
            TRACE,
            ("Invalid auth mode %d\n",
            *(P_ENUM_PARAM_AUTH_MODE_T)pvSetBuffer));
        return WLAN_STATUS_INVALID_DATA;
    }

    switch (*(P_ENUM_PARAM_AUTH_MODE_T)pvSetBuffer) {
    case AUTH_MODE_WPA:
    case AUTH_MODE_WPA_PSK:
    case AUTH_MODE_WPA2:
    case AUTH_MODE_WPA2_PSK:
        /* infrastructure mode only */
        if (prAdapter->rConnSettings.eOPMode != NET_TYPE_INFRA) {
            return WLAN_STATUS_NOT_ACCEPTED;
        }
        break;

    case AUTH_MODE_WPA_NONE:
    /* ad hoc mode only */
        if (prAdapter->rConnSettings.eOPMode != NET_TYPE_IBSS) {
            return WLAN_STATUS_NOT_ACCEPTED;
        }
        break;

    default:
        ;
    }

    /* Save the new authentication mode. */
    prAdapter->rConnSettings.eAuthMode = *(P_ENUM_PARAM_AUTH_MODE_T)pvSetBuffer;

    if (prAdapter->rConnSettings.eAuthMode >= AUTH_MODE_WPA) {
        switch(prAdapter->rConnSettings.eAuthMode) {
        case AUTH_MODE_WPA:
            u4AkmSuite = WPA_AKM_SUITE_802_1X;
            break;

        case AUTH_MODE_WPA_PSK:
            u4AkmSuite = WPA_AKM_SUITE_PSK;
            break;

        case AUTH_MODE_WPA_NONE:
            u4AkmSuite = WPA_AKM_SUITE_NONE;
            break;

        case AUTH_MODE_WPA2:
            u4AkmSuite = RSN_AKM_SUITE_802_1X;
            break;

        case AUTH_MODE_WPA2_PSK:
            u4AkmSuite = RSN_AKM_SUITE_PSK;
            break;

        default:
            u4AkmSuite = 0;
        }
    }
    else {
        u4AkmSuite = 0;
    }

    /* Enable the specific AKM suite only. */
    for (i = 0; i < MAX_NUM_SUPPORTED_AKM_SUITES; i++) {
        prEntry = &prAdapter->rConnSettings.rMib.dot11RSNAConfigAuthenticationSuitesTable[i];

        if (prEntry->dot11RSNAConfigAuthenticationSuite == u4AkmSuite) {
            prEntry->dot11RSNAConfigAuthenticationSuiteEnabled = TRUE;
        }
        else {
            prEntry->dot11RSNAConfigAuthenticationSuiteEnabled = FALSE;
        }
    }

#if DBG
    switch (prAdapter->rConnSettings.eAuthMode) {
    case AUTH_MODE_OPEN:
        DBGLOG(REQ, INFO, ("New auth mode: open\n"));
        break;

    case AUTH_MODE_SHARED:
        DBGLOG(REQ, INFO, ("New auth mode: shared\n"));
        break;

    case AUTH_MODE_AUTO_SWITCH:
        DBGLOG(REQ, INFO, ("New auth mode: auto-switch\n"));
        break;

    case AUTH_MODE_WPA:
        DBGLOG(REQ, INFO, ("New auth mode: WPA\n"));
        break;

    case AUTH_MODE_WPA_PSK:
        DBGLOG(REQ, INFO, ("New auth mode: WPA PSK\n"));
        break;

    case AUTH_MODE_WPA_NONE:
        DBGLOG(REQ, INFO, ("New auth mode: WPA None\n"));
        break;

    case AUTH_MODE_WPA2:
        DBGLOG(REQ, INFO, ("New auth mode: WPA2\n"));
        break;

    case AUTH_MODE_WPA2_PSK:
        DBGLOG(REQ, INFO, ("New auth mode: WPA2 PSK\n"));
        break;

    default:
        DBGLOG(REQ,
            INFO,
            ("New auth mode: unknown (%d)\n",
            prAdapter->rConnSettings.eAuthMode));
    }
#endif
    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetAuthMode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetReloadDefaults (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    WLAN_STATUS           rStatus = WLAN_STATUS_SUCCESS;
    ENUM_PARAM_NETWORK_TYPE_T    eNetworkType;
    UINT_32               u4Len;

    DEBUGFUNC("wlanoidSetReloadDefaults");

    ASSERT(prAdapter);

    ASSERT(pu4SetInfoLen);
    *pu4SetInfoLen = sizeof(PARAM_RELOAD_DEFAULTS);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set Reload default! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    /* Verify the available reload options and reload the settings. */
    switch (*(P_PARAM_RELOAD_DEFAULTS)pvSetBuffer) {
    case ENUM_RELOAD_WEP_KEYS:
        /* Reload available default WEP keys from the permanent
            storage. */
        privacyReloadDefaultWepKeys(prAdapter);
        break;

    default:
        DBGLOG(REQ,
            TRACE,
            ("Invalid reload option %d\n",
            *(P_PARAM_RELOAD_DEFAULTS)pvSetBuffer));
        rStatus = WLAN_STATUS_INVALID_DATA;
    }

    /* OID_802_11_RELOAD_DEFAULTS requiest to reset to auto mode */
    eNetworkType = NETWORK_TYPE_AUTOMODE;
    wlanoidSetNetworkTypeInUse(prAdapter, &eNetworkType, sizeof(eNetworkType), &u4Len);

    return rStatus;
} /* wlanoidSetReloadDefaults */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAddWep (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_PARAM_WEP_T prNewWepKey;
    UINT_32       u4KeyId, u4SetLen;
    UINT_8        keyBuffer[sizeof(PARAM_KEY_T) + LEGACY_KEY_MAX_LEN];
    P_PARAM_KEY_T prParamKey = (P_PARAM_KEY_T)keyBuffer;
    UINT_8        aucBCAddr[] = BC_MAC_ADDR;

    DEBUGFUNC("wlanoidSetAddWep");

    ASSERT(prAdapter);

    *pu4SetInfoLen = OFFSET_OF(PARAM_WEP_T, aucKeyMaterial);

    if (u4SetBufferLen < OFFSET_OF(PARAM_WEP_T, aucKeyMaterial)) {
        ASSERT(pu4SetInfoLen);
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set add WEP! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    prNewWepKey = (P_PARAM_WEP_T)pvSetBuffer;

    /* Verify the total buffer for minimum length. */
    if (u4SetBufferLen < OFFSET_OF(PARAM_WEP_T, aucKeyMaterial) + prNewWepKey->u4KeyLength) {
        DBGLOG(RSN, WARN, ("Invalid total buffer length (%d) than minimum length (%d)\n",
                          (UINT_8)u4SetBufferLen,
                          (UINT_8)OFFSET_OF(PARAM_WEP_T, aucKeyMaterial)));

        *pu4SetInfoLen = OFFSET_OF(PARAM_WEP_T, aucKeyMaterial);
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Verify the key structure length. */
    if (prNewWepKey->u4Length > u4SetBufferLen) {
        DBGLOG(RSN, WARN, ("Invalid key structure length (%d) greater than total buffer length (%d)\n",
                          (UINT_8)prNewWepKey->u4Length,
                          (UINT_8)u4SetBufferLen));

        *pu4SetInfoLen = u4SetBufferLen;
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Verify the key material length for maximum key material length:16 */
    if (prNewWepKey->u4KeyLength > LEGACY_KEY_MAX_LEN) {
        DBGLOG(RSN, WARN, ("Invalid key material length (%d) greater than maximum key material length (16)\n",
            (UINT_8)prNewWepKey->u4KeyLength));

        *pu4SetInfoLen = u4SetBufferLen;
        return WLAN_STATUS_INVALID_DATA;
    }

    *pu4SetInfoLen = u4SetBufferLen;

    u4KeyId = prNewWepKey->u4KeyIndex & WEP_KEY_ID_FIELD;

    /* Verify whether key index is valid or not, current version
       driver support only 4 global WEP keys setting by this OID */
    if (u4KeyId > MAX_KEY_NUM - 1) {
        DBGLOG(RSN, ERROR, ("Error, invalid WEP key ID: %d\n", (UINT_8)u4KeyId));
        return WLAN_STATUS_INVALID_DATA;
    }

    prParamKey->u4KeyIndex = u4KeyId;

    /* Transmit key */
    if (prNewWepKey->u4KeyIndex & IS_TRANSMIT_KEY) {
        prParamKey->u4KeyIndex |= IS_TRANSMIT_KEY;
    }

    /* Per client key */
    if (prNewWepKey->u4KeyIndex & IS_UNICAST_KEY) {
        prParamKey->u4KeyIndex |= IS_UNICAST_KEY;
    }

    prParamKey->u4KeyLength = prNewWepKey->u4KeyLength;

    kalMemCopy(prParamKey->arBSSID, aucBCAddr, MAC_ADDR_LEN);

    kalMemCopy(prParamKey->aucKeyMaterial,
        prNewWepKey->aucKeyMaterial,
        prNewWepKey->u4KeyLength);

    prParamKey->u4Length = OFFSET_OF(PARAM_KEY_T, aucKeyMaterial) + prNewWepKey->u4KeyLength;

    wlanoidSetAddKey(prAdapter,
        (PVOID)prParamKey,
        prParamKey->u4Length,
        &u4SetLen);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetAddWep */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetRemoveWep (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    UINT_32               u4KeyId, u4SetLen;
    PARAM_REMOVE_KEY_T    rRemoveKey;
    UINT_8                aucBCAddr[] = BC_MAC_ADDR;

    DEBUGFUNC("wlanoidSetRemoveWep");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_KEY_INDEX);

    if (u4SetBufferLen < sizeof(PARAM_KEY_INDEX)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);
    u4KeyId = *(PUINT_32)pvSetBuffer;

    /* Dump PARAM_WEP content. */
    DBGLOG(REQ, INFO, ("Set: Dump PARAM_KEY_INDEX content\n"));
    DBGLOG(REQ, INFO, ("Index : 0x%08lx\n", u4KeyId));

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set remove WEP! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    if (u4KeyId & IS_TRANSMIT_KEY) {
        /* Bit 31 should not be set */
        DBGLOG(RSN, ERROR, ("Invalid WEP key index: 0x%08lx\n", u4KeyId));
        return WLAN_STATUS_INVALID_DATA;
    }

    u4KeyId &= BITS(0,7);

    /* Verify whether key index is valid or not. Current version
        driver support only 4 global WEP keys. */
    if (u4KeyId > MAX_KEY_NUM - 1) {
        DBGLOG(RSN, ERROR, ("invalid WEP key ID %lu\n", u4KeyId));
        return WLAN_STATUS_INVALID_DATA;
    }

    rRemoveKey.u4Length = sizeof(PARAM_REMOVE_KEY_T);
    rRemoveKey.u4KeyIndex = *(PUINT_32)pvSetBuffer;

    kalMemCopy(rRemoveKey.arBSSID, aucBCAddr, MAC_ADDR_LEN);

    wlanoidSetRemoveKey(prAdapter,
        (PVOID)&rRemoveKey,
        sizeof(PARAM_REMOVE_KEY_T),
        &u4SetLen);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetRemoveWep */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAddKey (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_PARAM_KEY_T prNewKey;

    DEBUGFUNC("wlanoidSetAddKey");
    DBGLOG(RSN, INFO, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set add key! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    prNewKey = (P_PARAM_KEY_T) pvSetBuffer;

    /* Verify the key structure length. */
    if (prNewKey->u4Length > u4SetBufferLen) {
        DBGLOG(RSN, WARN, ("Invalid key structure length (%d) greater than total buffer length (%d)\n",
                          (UINT_8)prNewKey->u4Length,
                          (UINT_8)u4SetBufferLen));

        *pu4SetInfoLen = u4SetBufferLen;
        return WLAN_STATUS_INVALID_LENGTH;
    }

    /* Verify the key material length for key material buffer */
    if (prNewKey->u4KeyLength > prNewKey->u4Length - OFFSET_OF(PARAM_KEY_T, aucKeyMaterial)) {
        DBGLOG(RSN, WARN, ("Invalid key material length (%d)\n", (UINT_8)prNewKey->u4KeyLength));
        *pu4SetInfoLen = u4SetBufferLen;
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Exception check */
    if (prNewKey->u4KeyIndex & 0x0fffff00) {
        return WLAN_STATUS_INVALID_DATA;
    }

    if (!(prNewKey->u4KeyLength == WEP_40_LEN  || prNewKey->u4KeyLength == WEP_104_LEN ||
          prNewKey->u4KeyLength == CCMP_KEY_LEN|| prNewKey->u4KeyLength == TKIP_KEY_LEN))
    {
        return WLAN_STATUS_INVALID_DATA;
    }

    *pu4SetInfoLen = u4SetBufferLen;

    /* Dump PARAM_KEY content. */
    DBGLOG(REQ, INFO, ("Set: Dump PARAM_KEY content\n"));
    DBGLOG(REQ, INFO, ("Length    : 0x%08lx\n", prNewKey->u4Length));
    DBGLOG(REQ, INFO, ("Key Index : 0x%08lx\n", prNewKey->u4KeyIndex));
    DBGLOG(REQ, INFO, ("Key Length: 0x%08lx\n", prNewKey->u4KeyLength));
    DBGLOG(REQ, INFO, ("BSSID:\n"));
    DBGLOG_MEM8(REQ, INFO, prNewKey->arBSSID, sizeof(PARAM_MAC_ADDRESS));
    DBGLOG(REQ, INFO, ("Key RSC:\n"));
    DBGLOG_MEM8(REQ, INFO, &prNewKey->rKeyRSC, sizeof(PARAM_KEY_RSC));
    DBGLOG(REQ, INFO, ("Key Material:\n"));
    DBGLOG_MEM8(REQ, INFO, prNewKey->aucKeyMaterial, prNewKey->u4KeyLength);

    if (arbFsmRunEventSecKeyInstalled(prAdapter, prNewKey) != WLAN_STATUS_SUCCESS)
        return WLAN_STATUS_INVALID_DATA;

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetAddKey */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetRemoveKey (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_REMOVE_KEY_T  prRemovedKey;
    UINT_32               u4KeyId;
    PUINT_8               pucBssid;
    SW_KEY_STRUC_T        rSwKey;

    DEBUGFUNC("wlanoidSetRemoveKey");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_REMOVE_KEY_T);

    if (u4SetBufferLen < sizeof(PARAM_REMOVE_KEY_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set remove key! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    prRemovedKey = (P_PARAM_REMOVE_KEY_T)pvSetBuffer;

    /* Dump PARAM_REMOVE_KEY content. */
    DBGLOG(REQ, INFO, ("Set: Dump PARAM_REMOVE_KEY content\n"));
    DBGLOG(REQ, INFO, ("Length    : 0x%08lx\n", prRemovedKey->u4Length));
    DBGLOG(REQ, INFO, ("Key Index : 0x%08lx\n", prRemovedKey->u4KeyIndex));
    DBGLOG(REQ, INFO, ("BSSID:\n"));
    DBGLOG_MEM8(REQ, INFO, prRemovedKey->arBSSID, MAC_ADDR_LEN);

    /* Check bit 31: this bit should always 0 */
    if (prRemovedKey->u4KeyIndex & IS_TRANSMIT_KEY) {
        /* Bit 31 should not be set */
        DBGLOG(RSN, ERROR, ("invalid key index: 0x%08lx\n",
            prRemovedKey->u4KeyIndex));
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Check bits 8 ~ 29 should always be 0 */
    if (prRemovedKey->u4KeyIndex & BITS(8, 29)) {
        /* Bit 31 should not be set */
        DBGLOG(RSN, ERROR, ("invalid key index: 0x%08lx\n",
            prRemovedKey->u4KeyIndex));
        return WLAN_STATUS_INVALID_DATA;
    }

    kalMemZero(&rSwKey, sizeof(SW_KEY_STRUC_T));
    u4KeyId = prRemovedKey->u4KeyIndex & 0x000000ff;
    pucBssid = (PUINT_8)prRemovedKey->arBSSID;

    ASSERT(u4KeyId < MAX_KEY_NUM);

    /* Pairwise key */
    switch (prRemovedKey->u4KeyIndex & IS_UNICAST_KEY) {
    case IS_UNICAST_KEY:
        privacyClearKeyEntry(prAdapter, TRUE, pucBssid, (UINT_8)u4KeyId);
        break;

    case 0: /* Group key */
        privacyClearKeyEntry(prAdapter, FALSE, pucBssid, (UINT_8)u4KeyId);
        break;
    } /* switch() */

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetRemoveKey */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryEncryptionStatus (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    BOOLEAN                 fgTransmitKeyAvailable = FALSE;
    ENUM_ENCRYPTION_STATUS_T  eEncStatus = 0;

    DEBUGFUNC("wlanoidQueryEncryptionStatus");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(ENUM_ENCRYPTION_STATUS_T);

    fgTransmitKeyAvailable = privacyTransmitKeyExist(prAdapter);

    switch (prAdapter->rConnSettings.eEncStatus) {
    case ENUM_ENCRYPTION3_ENABLED:
        if (fgTransmitKeyAvailable) {
            eEncStatus = ENUM_ENCRYPTION3_ENABLED;
        }
        else {
            eEncStatus = ENUM_ENCRYPTION3_KEY_ABSENT;
        }
        break;

    case ENUM_ENCRYPTION2_ENABLED:
        if (fgTransmitKeyAvailable) {
            eEncStatus = ENUM_ENCRYPTION2_ENABLED;
            break;
        }
        else {
            eEncStatus = ENUM_ENCRYPTION2_KEY_ABSENT;
        }
        break;

    case ENUM_ENCRYPTION1_ENABLED:
        if (fgTransmitKeyAvailable) {
            eEncStatus = ENUM_ENCRYPTION1_ENABLED;
        }
        else {
            eEncStatus = ENUM_ENCRYPTION1_KEY_ABSENT;
        }
        break;

    case ENUM_ENCRYPTION_DISABLED:
        eEncStatus = ENUM_ENCRYPTION_DISABLED;
        break;

    default:
        DBGLOG(RSN, ERROR, ("Unknown Encryption Status Setting:%d\n",
            prAdapter->rConnSettings.eEncStatus));
    }

#if DBG
    DBGLOG(REQ,
        INFO,
        ("Encryption status: %d Return:%d\n",
        prAdapter->rConnSettings.eEncStatus,
        eEncStatus));
#endif
    ASSERT(pvQueryBuffer);

    *(P_ENUM_ENCRYPTION_STATUS_T)pvQueryBuffer = eEncStatus;

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryEncryptionStatus */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetEncryptionStatus (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    WLAN_STATUS           rStatus = WLAN_STATUS_SUCCESS;
    ENUM_ENCRYPTION_STATUS_T eEewEncrypt;

    DEBUGFUNC("wlanoidSetEncryptionStatus");

    /* <Todo> the oid follow the set_encryption_status then add key ?
              this make the tx/rx key material for wpa_none,
              expect the upper layer make sure this */

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(ENUM_ENCRYPTION_STATUS_T);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set encryption status! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    eEewEncrypt = *(P_ENUM_ENCRYPTION_STATUS_T)pvSetBuffer;
    DBGLOG(REQ, TRACE, ("ENCRYPTION_STATUS %d\n", eEewEncrypt));

    switch (eEewEncrypt) {
    case ENUM_ENCRYPTION_DISABLED: /* Disable WEP, TKIP, AES */
        DBGLOG(REQ, INFO, ("Disable Encryption\n"));
        /* 1595. Do we still need to support shared key without encryption. */
        privacySetCipherSuite(prAdapter,
            CIPHER_FLAG_WEP40  |
            CIPHER_FLAG_WEP104 |
            CIPHER_FLAG_WEP128);
       /* Odyssey will set a key even using open auth and no sec,
          clear the setting key at this mode */
        nicPrivacyInitialize(prAdapter);
        break;

    case ENUM_ENCRYPTION1_ENABLED: /* Enable WEP. Disable TKIP, AES */
        DBGLOG(REQ, INFO, ("Enable Encryption1\n"));
        privacySetCipherSuite(prAdapter,
            CIPHER_FLAG_WEP40  |
            CIPHER_FLAG_WEP104 |
            CIPHER_FLAG_WEP128);
        break;

    case ENUM_ENCRYPTION2_ENABLED: /* Enable WEP, TKIP. Disable AES */
        DBGLOG(REQ, INFO, ("Enable Encryption2\n"));
        privacySetCipherSuite(prAdapter,
            CIPHER_FLAG_WEP40  |
            CIPHER_FLAG_WEP104 |
            CIPHER_FLAG_WEP128 |
            CIPHER_FLAG_TKIP);
        break;

    case ENUM_ENCRYPTION3_ENABLED: /* Enable WEP, TKIP, AES */
        DBGLOG(REQ, INFO, ("Enable Encryption3\n"));
        privacySetCipherSuite(prAdapter,
            CIPHER_FLAG_WEP40  |
            CIPHER_FLAG_WEP104 |
            CIPHER_FLAG_WEP128 |
            CIPHER_FLAG_TKIP   |
            CIPHER_FLAG_CCMP);
        break;

    default:
        DBGLOG(RSN, WARN, ("Unacceptible encryption status: %d\n",
            *(P_ENUM_ENCRYPTION_STATUS_T)pvSetBuffer));

        privacySetCipherSuite(prAdapter, CIPHER_FLAG_NONE);

        rStatus = WLAN_STATUS_NOT_SUPPORTED;
    }

    if (rStatus == WLAN_STATUS_SUCCESS) {
        /* Save the new encryption status. */
        prAdapter->rConnSettings.eEncStatus =
            *(P_ENUM_ENCRYPTION_STATUS_T)pvSetBuffer;
    }

    return rStatus;
} /* wlanoidSetEncryptionStatus */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetTest (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_802_11_TEST_T prTest;
    PVOID                 pvTestData;
    PVOID                 pvStatusBuffer;
    UINT_32               u4StatusBufferSize;

    DEBUGFUNC("wlanoidSetTest");

    ASSERT(prAdapter);

    ASSERT(pu4SetInfoLen);
    ASSERT(pvSetBuffer);

    *pu4SetInfoLen = u4SetBufferLen;

    prTest = (P_PARAM_802_11_TEST_T)pvSetBuffer;

    DBGLOG(REQ, TRACE, ("Test - Type %ld\n", prTest->u4Type));

    switch (prTest->u4Type) {
    case 1:     /* Type 1: generate an authentication event */
        pvTestData = (PVOID)&prTest->u.AuthenticationEvent;
        pvStatusBuffer = (PVOID)prAdapter->aucIndicationEventBuffer;
        u4StatusBufferSize = prTest->u4Length - 8;
        break;

    case 2:     /* Type 2: generate an RSSI status indication */
        pvTestData = (PVOID)&prTest->u.RssiTrigger;
        pvStatusBuffer = (PVOID)&prAdapter->rBssInfo.rRssi;
        u4StatusBufferSize = sizeof(PARAM_RSSI);
        break;

    default:
        return WLAN_STATUS_INVALID_DATA;
    }

    ASSERT(u4StatusBufferSize <= 180);
    if (u4StatusBufferSize > 180) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    /* Get the contents of the StatusBuffer from the test structure. */
    kalMemCopy(pvStatusBuffer, pvTestData, u4StatusBufferSize);

    kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
        WLAN_STATUS_MEDIA_SPECIFIC_INDICATION,
        pvStatusBuffer,
        u4StatusBufferSize);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetTest */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryCapability (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_PARAM_CAPABILITY_T  prCap;
    P_PARAM_AUTH_ENCRYPTION_T prAuthenticationEncryptionSupported;

    DEBUGFUNC("wlanoidQueryCapability");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = 4 * sizeof(UINT_32) + 14 * sizeof(PARAM_AUTH_ENCRYPTION_T);

    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    prCap = (P_PARAM_CAPABILITY_T)pvQueryBuffer;

    prCap->u4Length = *pu4QueryInfoLen;
    prCap->u4Version = 2; /* WPA2 */
    prCap->u4NoOfPMKIDs = CFG_MAX_PMKID_CACHE;
    prCap->u4NoOfAuthEncryptPairsSupported = 14;

    prAuthenticationEncryptionSupported =
        &prCap->arAuthenticationEncryptionSupported[0];

    prAuthenticationEncryptionSupported[0].eAuthModeSupported =
        AUTH_MODE_OPEN;
    prAuthenticationEncryptionSupported[0].eEncryptStatusSupported =
        ENUM_ENCRYPTION_DISABLED;

    prAuthenticationEncryptionSupported[1].eAuthModeSupported =
        AUTH_MODE_OPEN;
    prAuthenticationEncryptionSupported[1].eEncryptStatusSupported =
        ENUM_ENCRYPTION1_ENABLED;

    prAuthenticationEncryptionSupported[2].eAuthModeSupported =
        AUTH_MODE_SHARED;
    prAuthenticationEncryptionSupported[2].eEncryptStatusSupported =
        ENUM_ENCRYPTION_DISABLED;

    prAuthenticationEncryptionSupported[3].eAuthModeSupported =
        AUTH_MODE_SHARED;
    prAuthenticationEncryptionSupported[3].eEncryptStatusSupported =
        ENUM_ENCRYPTION1_ENABLED;

    prAuthenticationEncryptionSupported[4].eAuthModeSupported =
        AUTH_MODE_WPA;
    prAuthenticationEncryptionSupported[4].eEncryptStatusSupported =
        ENUM_ENCRYPTION2_ENABLED;

    prAuthenticationEncryptionSupported[5].eAuthModeSupported =
        AUTH_MODE_WPA;
    prAuthenticationEncryptionSupported[5].eEncryptStatusSupported =
        ENUM_ENCRYPTION3_ENABLED;

    prAuthenticationEncryptionSupported[6].eAuthModeSupported =
        AUTH_MODE_WPA_PSK;
    prAuthenticationEncryptionSupported[6].eEncryptStatusSupported =
        ENUM_ENCRYPTION2_ENABLED;

    prAuthenticationEncryptionSupported[7].eAuthModeSupported =
        AUTH_MODE_WPA_PSK;
    prAuthenticationEncryptionSupported[7].eEncryptStatusSupported =
        ENUM_ENCRYPTION3_ENABLED;

    prAuthenticationEncryptionSupported[8].eAuthModeSupported =
        AUTH_MODE_WPA_NONE;
    prAuthenticationEncryptionSupported[8].eEncryptStatusSupported =
        ENUM_ENCRYPTION2_ENABLED;

    prAuthenticationEncryptionSupported[9].eAuthModeSupported =
        AUTH_MODE_WPA_NONE;
    prAuthenticationEncryptionSupported[9].eEncryptStatusSupported =
        ENUM_ENCRYPTION3_ENABLED;

    prAuthenticationEncryptionSupported[10].eAuthModeSupported =
        AUTH_MODE_WPA2;
    prAuthenticationEncryptionSupported[10].eEncryptStatusSupported =
        ENUM_ENCRYPTION2_ENABLED;

    prAuthenticationEncryptionSupported[11].eAuthModeSupported =
        AUTH_MODE_WPA2;
    prAuthenticationEncryptionSupported[11].eEncryptStatusSupported =
        ENUM_ENCRYPTION3_ENABLED;

    prAuthenticationEncryptionSupported[12].eAuthModeSupported =
        AUTH_MODE_WPA2_PSK;
    prAuthenticationEncryptionSupported[12].eEncryptStatusSupported =
        ENUM_ENCRYPTION2_ENABLED;

    prAuthenticationEncryptionSupported[13].eAuthModeSupported =
        AUTH_MODE_WPA2_PSK;
    prAuthenticationEncryptionSupported[13].eEncryptStatusSupported =
        ENUM_ENCRYPTION3_ENABLED;

    return WLAN_STATUS_SUCCESS;

} /* wlanoidQueryCapability */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryPmkid (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    UINT_32               i;
    P_PARAM_PMKID_T       prPmkid;

    DEBUGFUNC("wlanoidQueryPmkid");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = OFFSET_OF(PARAM_PMKID_T, arBSSIDInfo) +
        prAdapter->rSecInfo.u4PmkidCacheCount * sizeof(PARAM_BSSID_INFO_T);

    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    prPmkid = (P_PARAM_PMKID_T)pvQueryBuffer;

    prPmkid->u4Length = *pu4QueryInfoLen;
    prPmkid->u4BSSIDInfoCount = prAdapter->rSecInfo.u4PmkidCacheCount;

    for (i = 0; i < prAdapter->rSecInfo.u4PmkidCacheCount; i++) {
        kalMemCopy(prPmkid->arBSSIDInfo[i].arBSSID,
            prAdapter->rSecInfo.arPmkidCache[i].rBssidInfo.arBSSID,
            sizeof(PARAM_MAC_ADDRESS));
        kalMemCopy(prPmkid->arBSSIDInfo[i].arPMKID,
            prAdapter->rSecInfo.arPmkidCache[i].rBssidInfo.arPMKID,
            sizeof(PARAM_PMKID_VALUE));
    }

    return WLAN_STATUS_SUCCESS;

} /* wlanoidQueryPmkid */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetPmkid (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    UINT_32               i, j;
    P_PARAM_PMKID_T       prPmkid;

    DEBUGFUNC("wlanoidSetPmkid");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = u4SetBufferLen;

    /* It's possibble BSSIDInfoCount is zero, because OS wishes to clean PMKID */
    if (u4SetBufferLen < OFFSET_OF(PARAM_PMKID_T, arBSSIDInfo)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    ASSERT(pvSetBuffer);
    prPmkid = (P_PARAM_PMKID_T)pvSetBuffer;

    if (u4SetBufferLen <
            ((prPmkid->u4BSSIDInfoCount * sizeof(PARAM_BSSID_INFO_T)) +
            OFFSET_OF(PARAM_PMKID_T, arBSSIDInfo))) {
        return WLAN_STATUS_INVALID_DATA;
    }

    if (prPmkid->u4BSSIDInfoCount > CFG_MAX_PMKID_CACHE) {
        return WLAN_STATUS_INVALID_DATA;
    }

    DBGLOG(REQ, INFO, ("Count %lu\n", prPmkid->u4BSSIDInfoCount));

    /* This OID replace everything in the PMKID cache. */
    if (prPmkid->u4BSSIDInfoCount == 0) {
        prAdapter->rSecInfo.u4PmkidCacheCount = 0;
        kalMemZero(prAdapter->rSecInfo.arPmkidCache, sizeof(PMKID_ENTRY_T) * CFG_MAX_PMKID_CACHE);
    }
    if ((prAdapter->rSecInfo.u4PmkidCacheCount + prPmkid->u4BSSIDInfoCount > CFG_MAX_PMKID_CACHE)) {
        prAdapter->rSecInfo.u4PmkidCacheCount = 0;
        kalMemZero(prAdapter->rSecInfo.arPmkidCache, sizeof(PMKID_ENTRY_T) * CFG_MAX_PMKID_CACHE);
    }

    /*
    The driver can only clear its PMKID cache whenever it make a media disconnect
    indication. Otherwise, it must change the PMKID cache only when set through this OID.
    */

    for (i = 0; i < prPmkid->u4BSSIDInfoCount; i++) {
        /* Search for desired BSSID. If desired BSSID is found,
            then set the PMKID */
        if (!rsnSearchPmkidEntry(prAdapter,
                (PUINT_8)prPmkid->arBSSIDInfo[i].arBSSID,
                &j)) {
            /* No entry found for the specified BSSID, so add one entry */
            if (prAdapter->rSecInfo.u4PmkidCacheCount < CFG_MAX_PMKID_CACHE - 1) {
                j = prAdapter->rSecInfo.u4PmkidCacheCount;
                kalMemCopy(prAdapter->rSecInfo.arPmkidCache[j].rBssidInfo.arBSSID,
                    prPmkid->arBSSIDInfo[i].arBSSID,
                    sizeof(PARAM_MAC_ADDRESS));
                prAdapter->rSecInfo.u4PmkidCacheCount++;
            }
            else {
                j = CFG_MAX_PMKID_CACHE;
            }
        }

        if (j < CFG_MAX_PMKID_CACHE) {
            kalMemCopy(prAdapter->rSecInfo.arPmkidCache[j].rBssidInfo.arPMKID,
                prPmkid->arBSSIDInfo[i].arPMKID,
                sizeof(PARAM_PMKID_VALUE));
            prAdapter->rSecInfo.arPmkidCache[j].fgPmkidExist = TRUE;
        }
    }

    return WLAN_STATUS_SUCCESS;

} /* wlanoidSetPmkid */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQuerySupportedRates (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    PARAM_RATES_EX rSupportedRates = {0};  /* supported rates for this STA in a BSS */
    UINT_8 ucSupportedRatesLen;
    UINT_16 u2SupportedRateSet = 0;
    ENUM_PHY_TYPE_INDEX_T ePhyTypeIndex;

    DEBUGFUNC("wlanoidQuerySupportedRates");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    for (ePhyTypeIndex = PHY_TYPE_ERP_INDEX; ePhyTypeIndex < PHY_TYPE_INDEX_NUM; ePhyTypeIndex++) {
        if (prAdapter->u2AvailablePhyTypeSet & BIT(ePhyTypeIndex)) {
            u2SupportedRateSet |= rPhyAttributes[ePhyTypeIndex].u2SupportedRateSet;
        }
    }

    /* Initialize the supported rates of the NIC by the MIB. */
    rateGetDataRatesFromRateSet(u2SupportedRateSet,
                                0x0, /*(UINT_16)NULL*/
                                rSupportedRates,
                                &ucSupportedRatesLen);

    *pu4QueryInfoLen = sizeof(PARAM_RATES_EX);

    if (u4QueryBufferLen < *pu4QueryInfoLen ) {
        DBGLOG(REQ, WARN, ("Invalid length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    kalMemCopy(pvQueryBuffer,
               (PVOID)&rSupportedRates[0],
               sizeof(PARAM_RATES_EX));

    DBGLOG(REQ, INFO,
        ("Current supported rates: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
         *(PUINT_8) pvQueryBuffer, *((PUINT_8) pvQueryBuffer + 1),
         *((PUINT_8) pvQueryBuffer + 2), *((PUINT_8) pvQueryBuffer + 3),
         *((PUINT_8) pvQueryBuffer + 4), *((PUINT_8) pvQueryBuffer + 5),
         *((PUINT_8) pvQueryBuffer + 6), *((PUINT_8) pvQueryBuffer + 7)));
    DBGLOG(REQ, INFO,
        ("                         0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
         *((PUINT_8) pvQueryBuffer + 8), *((PUINT_8) pvQueryBuffer + 9),
         *((PUINT_8) pvQueryBuffer + 10), *((PUINT_8) pvQueryBuffer + 11),
         *((PUINT_8) pvQueryBuffer + 12), *((PUINT_8) pvQueryBuffer + 13),
         *((PUINT_8) pvQueryBuffer + 14), *((PUINT_8) pvQueryBuffer + 15)));

    return WLAN_STATUS_SUCCESS;
} /* end of wlanoidQuerySupportedRates() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryDesiredRates (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    PARAM_RATES_EX rDesiredRates = {0}; /* supported rates for this STA in a BSS */
    UINT_8 ucDesiredRatesLen;

    DEBUGFUNC("wlanoidQueryDesiredRates");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    DBGLOG(REQ, INFO, ("\n"));

    prConnSettings = &prAdapter->rConnSettings;

    rateGetDataRatesFromRateSet(prConnSettings->u2DesiredRateSet,
                                0x0, /*(UINT_16)NULL*/
                                rDesiredRates,
                                &ucDesiredRatesLen);

    *pu4QueryInfoLen = (UINT_32)ucDesiredRatesLen;

    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    kalMemCopy(pvQueryBuffer,
               (PVOID)&rDesiredRates[0],
               ucDesiredRatesLen);

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryDesiredRates() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetDesiredRates (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_BSS_INFO_T prBssInfo;
    PUINT_8 pucNewRate;
    UINT_16 u2UserDesiredRateSet;
    P_STA_RECORD_T prStaRec;
#if DBG
    UINT_32 i;
#endif /* DBG */

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    prConnSettings = &prAdapter->rConnSettings;
    prBssInfo = &prAdapter->rBssInfo;

    *pu4SetInfoLen = sizeof(PARAM_RATES);

    if ((u4SetBufferLen != sizeof(PARAM_RATES)) &&
        (u4SetBufferLen != sizeof(PARAM_RATES_EX))) {

        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set desired rate! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    pucNewRate = (PUINT_8)pvSetBuffer;

    rateGetRateSetFromDataRates(pucNewRate,
                                (UINT_8)u4SetBufferLen,
                                &u2UserDesiredRateSet);

    if (u2UserDesiredRateSet) {
        prConnSettings->u2DesiredRateSet = u2UserDesiredRateSet;
#if DBG
        DBGLOG(REQ, TRACE, ("Update User Desire Rate Set : "));
        for (i = RATE_1M_INDEX; i < RATE_NUM; i++) {
            if (BIT(i) & prConnSettings->u2DesiredRateSet) {
                DBGLOG(REQ, TRACE, ("%s, ", apucDebugRateIndex[i]));
            }
        }
        DBGLOG(REQ, TRACE, ("\n"));
#endif /* DBG */
    }
    else {
        return WLAN_STATUS_INVALID_DATA;
    }

    *pu4SetInfoLen = u4SetBufferLen;

    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {

        /* Get a Station Record if possible */
        prStaRec = staRecGetStaRecordByAddr(prAdapter,
                                            prBssInfo->aucBSSID);

        if (prStaRec) {
            UINT_16 u2OperationalRateSet, u2DesiredRateSet;

            u2OperationalRateSet = (rPhyAttributes[prBssInfo->ePhyType].u2SupportedRateSet &
                                    prBssInfo->u2OperationalRateSet);

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

            if (nicSetHwBySta(prAdapter, prStaRec) == FALSE) {
                ASSERT(FALSE);
            }

#if CFG_TX_FRAGMENT
            if (prConnSettings->fgIsEnableTxAutoFragmentForBT) {
                txRateSetInitForBT(prAdapter, prStaRec);
            }
#endif /* CFG_TX_FRAGMENT */
        }
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetDesiredRates() */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryMaxFrameSize (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryMaxFrameSize");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    *(PUINT_32)pvQueryBuffer = ETHERNET_MAX_PKT_SZ - ETHERNET_HEADER_SZ;
    *pu4QueryInfoLen = sizeof(UINT_32);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryMaxFrameSize */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryMaxTotalSize (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryMaxTotalSize");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    *(PUINT_32)pvQueryBuffer = ETHERNET_MAX_PKT_SZ;
    *pu4QueryInfoLen = sizeof(UINT_32);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryMaxTotalSize */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryVendorId (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
#if DBG
    PUINT_8               cp;
#endif
    DEBUGFUNC("wlanoidQueryVendorId");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    kalMemCopy(pvQueryBuffer, prAdapter->aucMacAddress, 3);
    *((PUINT_8)pvQueryBuffer + 3) = 1;
    *pu4QueryInfoLen = sizeof(UINT_32);

#if DBG
    cp = (PUINT_8)pvQueryBuffer;
    DBGLOG(REQ, LOUD, ("Vendor ID=%02x-%02x-%02x-%02x\n", cp[0], cp[1], cp[2], cp[3]));
#endif

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryVendorId */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRssi (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    PARAM_RSSI rRssi;
    DEBUGFUNC("wlanoidQueryRssi");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PARAM_RSSI);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Too short length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    if (prAdapter->eConnectionStateIndicated == MEDIA_STATE_DISCONNECTED) {
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    rxUpdateRssi(prAdapter);

    rRssi = prAdapter->rBssInfo.rRssi;

    /*For WHQL test, Rssi should be in range -10 ~ -200 dBm*/
    if (rRssi > PARAM_WHQL_RSSI_MAX_DBM) {
        rRssi = PARAM_WHQL_RSSI_MAX_DBM;
    }

    ASSERT(pvQueryBuffer);

    *(PINT_32)pvQueryBuffer = rRssi;

    DBGLOG(REQ, INFO, ("Current RSSI: %ld dBm\n", *(PINT_32)pvQueryBuffer));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryRssi() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRssiTrigger (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryRssiTrigger");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PARAM_RSSI);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Too short length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);
    *(PINT_32) pvQueryBuffer = prAdapter->rRssiTriggerValue;

    DBGLOG(REQ, INFO, ("RSSI trigger: %ld dBm\n", *(PINT_32) pvQueryBuffer));

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryRssiTrigger */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetRssiTrigger (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    DEBUGFUNC("wlanoidSetRssiTrigger");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_RSSI);

    /* Save the RSSI trigger value to the Adapter structure */
    prAdapter->rRssiTriggerValue = *(PINT_32) pvSetBuffer;

    /* If the RSSI trigger value is equal to the current RSSI value, the
       indication triggers immediately. We need to indicate the protocol
       that an RSSI status indication event triggers. */
    if (prAdapter->rRssiTriggerValue == prAdapter->rBssInfo.rRssi) {
        kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
            WLAN_STATUS_MEDIA_SPECIFIC_INDICATION,
            (PVOID) &prAdapter->rBssInfo.rRssi, sizeof(PARAM_RSSI));
    }

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetRssiTrigger */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetCurrentLookahead (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    DEBUGFUNC("wlanoidSetCurrentLookahead");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        *pu4SetInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    *pu4SetInfoLen = sizeof(UINT_32);
    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetCurrentLookahead */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRcvError (
    IN  P_ADAPTER_T         prAdapter,
    IN  PVOID               pvQueryBuffer,
    IN  UINT_32             u4QueryBufferLen,
    OUT PUINT_32            pu4QueryInfoLen
    )
{
    P_RX_CTRL_T         prRxCtrl;
    WLAN_STATUS      rStatus = WLAN_STATUS_SUCCESS;
    UINT_64                  u8Tmp;

    DEBUGFUNC("wlanoidQueryRcvError");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in query receive error! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    /* NOTE(Kevin): WZC query RCV_ERROR often, so don't query HW Counter Here.
     * Thus we can return SUCCESS instead of PENDING for WinXP when WZC Query RCV_ERROR.
     */
    //statisticsPeekHWCounter(prAdapter);

    prRxCtrl = &prAdapter->rRxCtrl;
    u8Tmp = prRxCtrl->au8Statistics[RX_ERROR_DROP_COUNT] + \
                   prRxCtrl->au8Statistics[RX_FIFO_FULL_DROP_COUNT] -\
                   prRxCtrl->au8Statistics[RX_FCS_ERR_DROP_COUNT] +\
                   prRxCtrl->au8Statistics[RX_BB_FCS_ERROR_COUNT];

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)u8Tmp;

        DBGLOG(REQ, LOUD, ("Rx Error: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
        rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = u8Tmp;

        DBGLOG(REQ, LOUD, ("Rx Error: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);

    return rStatus;
} /* wlanoidQueryRcvError */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRcvNoBuffer (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_RX_CTRL_T         prRxCtrl;
    WLAN_STATUS      rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryRcvNoBuffer");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in query receive no buffer! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    statisticsPeekHWCounter(prAdapter);

    prRxCtrl = &prAdapter->rRxCtrl;

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)prRxCtrl->au8Statistics[RX_FIFO_FULL_DROP_COUNT];

        DBGLOG(REQ, LOUD, ("Rx FIFO full drop count: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
        rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = prRxCtrl->au8Statistics[RX_FIFO_FULL_DROP_COUNT];

        DBGLOG(REQ, LOUD, ("Rx FIFO full drop count: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);

    return rStatus;
}   /* wlanoidQueryRcvNoBuffer */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRcvCrcError (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_RX_CTRL_T         prRxCtrl;
    WLAN_STATUS      rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryRcvCrcError");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in query receive crc error! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }
    statisticsPeekHWCounter(prAdapter);

    prRxCtrl = &prAdapter->rRxCtrl;

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)prRxCtrl->au8Statistics[RX_BB_FCS_ERROR_COUNT];

        DBGLOG(REQ, LOUD, ("Rx Crc error count: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
        rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = prRxCtrl->au8Statistics[RX_BB_FCS_ERROR_COUNT];

        DBGLOG(REQ, LOUD, ("Rx Crc error count: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);

    return rStatus;
}   /* wlanoidQueryRcvCrcError */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryStatistics (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_RX_CTRL_T                             prRxCtrl;
    P_TX_CTRL_T                             prTxCtrl;
    P_PARAM_802_11_STATISTICS_STRUCT_T      prStatistics;
    WLAN_STATUS      rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryStatistics");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PARAM_802_11_STATISTICS_STRUCT_T);

    if (u4QueryBufferLen < sizeof(PARAM_802_11_STATISTICS_STRUCT_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in query statistics! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    statisticsPeekHWCounter(prAdapter);

    prRxCtrl = &prAdapter->rRxCtrl;
    prTxCtrl = &prAdapter->rTxCtrl;

    prStatistics = (P_PARAM_802_11_STATISTICS_STRUCT_T) pvQueryBuffer;

    prStatistics->u4Length =  sizeof(PARAM_802_11_STATISTICS_STRUCT_T);

    /* Tx Counters */
    prStatistics->rTransmittedFragmentCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_OK_COUNT];

    prStatistics->rMulticastTransmittedFrameCount.QuadPart =
        prTxCtrl->au8Statistics[TX_BMCAST_MPDU_OK_COUNT];

    prStatistics->rFailedCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_ALL_ERR_COUNT];

    prStatistics->rRetryCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_TX_TWICE_OK_COUNT] +
        prTxCtrl->au8Statistics[TX_MPDU_TX_MORE_TWICE_OK_COUNT];

    prStatistics->rMultipleRetryCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_TX_MORE_TWICE_OK_COUNT];

    /* Ctrl Frame Counters */
    prStatistics->rRTSSuccessCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_RTS_OK_COUNT];

    prStatistics->rRTSFailureCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_RTS_FAIL_COUNT];

    prStatistics->rACKFailureCount.QuadPart =
        prTxCtrl->au8Statistics[TX_MPDU_TOTAL_RETRY_COUNT] -
        prTxCtrl->au8Statistics[TX_MPDU_OK_COUNT];
        /* prTxCtrl->au8Statistics[TX_MPDU_MPDU_RETRY_ERR_COUNT]; */

    /* Rx Counters */
    prStatistics->rFrameDuplicateCount.QuadPart =
        prRxCtrl->au8Statistics[RX_DUPLICATE_DROP_COUNT];

    prStatistics->rReceivedFragmentCount.QuadPart =
        prRxCtrl->au8Statistics[RX_MPDU_TOTAL_COUNT];

    prStatistics->rMulticastReceivedFrameCount.QuadPart = \
        prRxCtrl->au8Statistics[RX_BMCAST_DATA_FRAME_COUNT] + \
        prRxCtrl->au8Statistics[RX_BMCAST_MGMT_FRAME_COUNT] ;

    prStatistics->rFCSErrorCount.QuadPart =
        prRxCtrl->au8Statistics[RX_BB_FCS_ERROR_COUNT];

    return rStatus;
}   /* wlanoidQueryStatistics */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryStatisticsForLinux (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_RX_CTRL_T prRxCtrl;
    P_TX_CTRL_T prTxCtrl;
    P_PARAM_LINUX_NETDEV_STATISTICS_T prCustomNetDevStat;

    DEBUGFUNC("wlanoidQueryStatisticsForLinux");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    *pu4QueryInfoLen = sizeof(PARAM_LINUX_NETDEV_STATISTICS_T);

    if (u4QueryBufferLen < sizeof(PARAM_LINUX_NETDEV_STATISTICS_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    prRxCtrl = &prAdapter->rRxCtrl;
    prTxCtrl = &prAdapter->rTxCtrl;

    ASSERT(pvQueryBuffer);

    prCustomNetDevStat = (P_PARAM_LINUX_NETDEV_STATISTICS_T)pvQueryBuffer;

    prCustomNetDevStat->u4RxPackets = (UINT_32)prRxCtrl->au8Statistics[RX_DATA_INDICATION_COUNT];
    prCustomNetDevStat->u4TxPackets = (UINT_32)prTxCtrl->au8Statistics[TX_MSDU_OK_COUNT];
    prCustomNetDevStat->u4RxBytes   = (UINT_32)prRxCtrl->au8Statistics[RX_MSDU_BYTES_COUNT];
    prCustomNetDevStat->u4TxBytes   = (UINT_32)prTxCtrl->au8Statistics[TX_MSDU_BYTES_COUNT];
    prCustomNetDevStat->u4RxErrors  = (UINT_32)prRxCtrl->au8Statistics[RX_ERROR_DROP_COUNT];
    prCustomNetDevStat->u4TxErrors  = (UINT_32)prTxCtrl->au8Statistics[TX_MSDU_FAIL_COUNT];
    prCustomNetDevStat->u4Multicast = (UINT_32)prTxCtrl->au8Statistics[RX_BMCAST_DATA_FRAME_COUNT];

    return WLAN_STATUS_SUCCESS;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryPermanentAddr (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{

    DEBUGFUNC("wlanoidQueryPermanentAddr");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufLen != 0) {
        ASSERT(pvQueryBuf);
#if 0
    kalMemCopy(pvQueryBuf, prAdapter->permanentMacAddress, MAC_ADDR_LEN);
#else
    kalMemCopy(pvQueryBuf, prAdapter->rEEPROMCtrl.aucMacAddress, MAC_ADDR_LEN);
#endif
    *pu4QueryInfoLen = MAC_ADDR_LEN;
    }

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryPermanentAddr */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryCurrentAddr (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{

    DEBUGFUNC("wlanoidQueryCurrentAddr");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = MAC_ADDR_LEN;

    if (u4QueryBufferLen < MAC_ADDR_LEN) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    kalMemCopy(pvQueryBuffer, prAdapter->aucMacAddress, MAC_ADDR_LEN);

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryCurrentAddr */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryLinkSpeed(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{
    UINT_8 ucCurrTxDataRate;

    DEBUGFUNC("wlanoidQueryLinkSpeed");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        ucCurrTxDataRate = aucDataRate[prAdapter->rTxCtrl.ucCurrRateIndex];
    }
    else {
        ucCurrTxDataRate = RATE_1M;
    }

    *(PUINT_32)pvQueryBuffer = (UINT_32)((UINT_32)ucCurrTxDataRate * 5000);

    DBGLOG(REQ, LOUD,
        ("Link speed = %dMbps\n", ucCurrTxDataRate / 2));

    return WLAN_STATUS_SUCCESS;
} /* end of wlanoidQueryLinkSpeed() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryMcrRead (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{
    P_PARAM_CUSTOM_MCR_RW_STRUC_T prMcrRdInfo;

    DEBUGFUNC("wlanoidQueryMcrRead");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T);

    if (u4QueryBufferLen < sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    prMcrRdInfo = (P_PARAM_CUSTOM_MCR_RW_STRUC_T)pvQueryBuffer;

    HAL_MCR_RD(prAdapter,
               prMcrRdInfo->u4McrOffset & BITS(2,31), //address is in DWORD unit
               &prMcrRdInfo->u4McrData);

    DBGLOG(REQ, INFO, ("MCR Read: Offset = %#08lx, Data = %#08lx\n",
        prMcrRdInfo->u4McrOffset, prMcrRdInfo->u4McrData));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryMcrRead() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidOidTest(
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
	PVOID	pContent = NULL;
	PVOID   pTemp = NULL;
	UINT_32 index = 0;
	CHAR    indexTemp[10];
	
	ASSERT(pvSetBuffer);

	DBGLOG(REQ, INFO, ("%s :%s\n", __FUNCTION__, pvSetBuffer));
	pTemp = strchr(pvSetBuffer, ' ');

	if(pTemp == NULL){
		DBGLOG(REQ, INFO, ("%s: get pTemp NULL\n", __FUNCTION__));
		return WLAN_STATUS_INVALID_LENGTH;
	}
	else
		DBGLOG(REQ, INFO,("%s get pTemp 0x%x~0x%x\n", __FUNCTION__, pTemp, pvSetBuffer));
	
	if(pTemp - pvSetBuffer > 9)
		return WLAN_STATUS_INVALID_LENGTH;
	else{
		strncpy(indexTemp, pvSetBuffer, pTemp - pvSetBuffer); 
		indexTemp[pTemp - pvSetBuffer] = '\0';
		DBGLOG(REQ, INFO, ("%s: indexTemp %s\n", __FUNCTION__, indexTemp));
		sscanf(indexTemp, "%d", &index);
		DBGLOG(REQ, INFO,("%s get Index %d\n", __FUNCTION__, index));
	}	

	pContent = pTemp + 1;
	switch (index){
		case 1:
		{
			UINT_8	u8Ip[4];
			UINT_32 u32Ip[4];				

#if 0
			if(4 == sscanf(pContent, "%d.%d.%d.%d", u32Ip + 3, u32Ip + 2, u32Ip + 1, u32Ip)){
#else
			if(4 == sscanf(pContent, "%d.%d.%d.%d", u32Ip, u32Ip + 1, u32Ip + 2, u32Ip + 3)){
#endif
				kalMemZero(u8Ip, sizeof(u8Ip));
				u8Ip [0] = u32Ip[0]&0xff,u8Ip [1] = u32Ip[1]&0xff,
				u8Ip [2] = u32Ip[2]&0xff,u8Ip [3] = u32Ip[3]&0xff;
				DBGLOG(REQ, INFO,("%s get IP %d.%d.%d.%d \n", __FUNCTION__, 
						*u8Ip, *(u8Ip + 1), *(u8Ip + 2), *(u8Ip + 3)));
				wlanoidSetIpAddress(prAdapter,
							&u8Ip,
							sizeof(u8Ip),
							pu4SetInfoLen);
			}else
				DBGLOG(REQ, INFO, ("%s sscanf failed\n", __FUNCTION__));
				
		}
			break;

		case 2:
		{
			UINT_32 on = 55;
			
			if(1 == sscanf(pContent, "%d", &on)){
				DBGLOG(REQ, INFO,("%s get ON %d\n", __FUNCTION__, on));

				if(1 == on)
					wlanRxSetBroadcast(prAdapter, 1);
				else if(0 == on)
					wlanRxSetBroadcast(prAdapter, 0);

			}


		}
			break;

		case 3:
		{
			P_GLUE_INFO_T pGlue = prAdapter->prGlueInfo;
			struct net_device *prNetDev = to_net_dev(pGlue);
			struct in_device *prIn;
			struct in_ifaddr *prIfaddr;
			UINT_32  ip;
			
			DBGLOG(REQ, INFO,("%s get %s\n", __FUNCTION__, 
				prNetDev->name));	
			prIn = (struct in_device *)(prNetDev->ip_ptr);
			DBGLOG(REQ, INFO,("%s 0x%x  get  prIn->dev 0x%x\n", __FUNCTION__, 
				prNetDev, prIn->dev));	
#if 0
			ASSERT(prIn);
			prIfaddr = prIn->ifa_list;
			ASSERT(prIfaddr);
			ip = prIfaddr->ifa_local;
			//ip = prIn->ifa_list->ifa_local;
			DBGLOG(REQ, INFO,("%s get %s ipv4 0x%x\n", __FUNCTION__, 
				prNetDev->name,	ip));	
#endif
		}
			break;
	}

	return  WLAN_STATUS_SUCCESS;

}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_8  PER_FLAG = 0;

WLAN_STATUS
wlanoidSetMcrWrite (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_PARAM_CUSTOM_MCR_RW_STRUC_T prMcrWrInfo;
    UINT_32 u4Tmp;

    DEBUGFUNC("wlanoidSetMcrWrite");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T);

    if (u4SetBufferLen < sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);

    prMcrWrInfo = (P_PARAM_CUSTOM_MCR_RW_STRUC_T)pvSetBuffer;

    DBGLOG(REQ, INFO, ("MCR Write: Offset = %#08lx, Data = %#08lx\n",
        prMcrWrInfo->u4McrOffset, prMcrWrInfo->u4McrData));

#if 1
    if (prMcrWrInfo->u4McrOffset >= 0x500) {
        /* Backdoor configurations functions,
           this is used for debugging only.
        */

        PARAM_VOIP_CONFIG rVoipCfg;
        UINT_32 u4LenIn, u4LenOut;

        switch (prMcrWrInfo->u4McrOffset) {
        case 0x500:
            pmFsmRunEventSetPowerSaveProfile(prAdapter, ENUM_PSP_CONTINUOUS_ACTIVE);
            break;
        case 0x501:
            pmFsmRunEventSetPowerSaveProfile(prAdapter, ENUM_PSP_CONTINUOUS_POWER_SAVE);
            break;
        case 0x502:
            pmFsmRunEventSetPowerSaveProfile(prAdapter, ENUM_PSP_FAST_SWITCH);
            break;
        case 0x503: /* Enable VoIP polling function (20ms) */
            rVoipCfg.u4VoipTrafficInterval = 20;
            u4LenIn = sizeof(PARAM_VOIP_CONFIG);
            wlanoidSetVoipConnectionStatus(prAdapter, &rVoipCfg, u4LenIn, &u4LenOut);
            break;
        case 0x504: /* Disable VoIP polling function */
            rVoipCfg.u4VoipTrafficInterval = 0;
            u4LenIn = sizeof(PARAM_VOIP_CONFIG);
            wlanoidSetVoipConnectionStatus(prAdapter, &rVoipCfg, u4LenIn, &u4LenOut);
            break;
#if DBG
        /* Configuration of debug message (module/ level) */
        case 0x505:
            u4DebugModuleTemp = prMcrWrInfo->u4McrData;
            break;
        case 0x506:
            aucDebugModule[u4DebugModuleTemp] = (UINT_8)prMcrWrInfo->u4McrData;
            break;
#endif
        case 0x510:
            prAdapter->eConnectionState = MEDIA_STATE_CONNECTED;
            prAdapter->eConnectionStateIndicated = MEDIA_STATE_CONNECTED;

            kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
                WLAN_STATUS_MEDIA_CONNECT, (PVOID) 0, 0);
            break;
        case 0x511:
            prAdapter->eConnectionState = MEDIA_STATE_DISCONNECTED;
            kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
                WLAN_STATUS_MEDIA_DISCONNECT, (PVOID) 0, 0);
            break;
        case 0x512: /* Disable to respond UC indication for passing WiFi under full U-APSD case */
            {
            UINT_32 u4RegValue;
            HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);
            HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue & ~MPTCR_BCN_UC_EN);
            }
            break;
        case 0x520: /* Query debug TX/ RX counters */
            nicTxQueryStatus(prAdapter,
                (PUINT_8)NULL,&u4Tmp);
            nicTxQueryStatistics(prAdapter,
                (PUINT_8)NULL,&u4Tmp);
            nicRxQueryStatus(prAdapter,
                (PUINT_8)NULL,&u4Tmp);
            nicRxQueryStatistics(prAdapter,
                (PUINT_8)NULL,&u4Tmp);
            break;
        case 0x530: /* Set to ACPI D0 state */
            pmSetAcpiPowerD0(prAdapter);
            break;
        case 0x531: /* Set to ACPI D3 state */
            pmSetAcpiPowerD3(prAdapter);
            break;
        case 0x532: /*Setting up ATIM window under IBSS */
            {
            P_PM_INFO_T prPmInfo;
            P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
            prPmInfo = &prAdapter->rPmInfo;
            prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;
            prAdapter->rConnSettings.u2AtimWindow = (UINT_16)prMcrWrInfo->u4McrData;
            }
            break;
        case 0x533: /* Configure U-APSD AC function */
            {
            P_PM_INFO_T prPmInfo;
            P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
            prPmInfo = &prAdapter->rPmInfo;
            prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;
            prPmProfSetupInfo->bmfgApsdEnAc = (UINT_8)prMcrWrInfo->u4McrData;
            }
            break;
        case 0x534: /* Restore pattern search function */
            {
            P_PM_INFO_T prPmInfo;
            P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
            prPmInfo = &prAdapter->rPmInfo;
            prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

            /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
            prPmInfo->ucNumOfInstSleep = 0;
            prPmInfo->ucNumOfInstAwake = 0;
            prPmInfo->ucNumOfInstOn = 0;

            nicpmConfigPatternSearchFunc(prAdapter,
                                         prPmProfSetupInfo->fgBcPtrnSrchEn,
                                         prPmProfSetupInfo->fgMcPtrnSrchEn,
                                         prPmProfSetupInfo->fgUcPtrnSrchEn,
                                         prPmProfSetupInfo->fgBcPtrnMatchRcv,
                                         prPmProfSetupInfo->fgMcPtrnMatchRcv,
                                         prPmProfSetupInfo->fgUcPtrnMatchRcv,
                                         FALSE);
            }
            break;
        case 0x535: /* Disable pattern search function */
            {
            P_PM_INFO_T prPmInfo;
            P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;
            prPmInfo = &prAdapter->rPmInfo;
            prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

            /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
            prPmInfo->ucNumOfInstSleep = 0;
            prPmInfo->ucNumOfInstAwake = 0;
            prPmInfo->ucNumOfInstOn = 0;

            nicpmConfigPatternSearchFunc(prAdapter,
                                         FALSE,
                                         FALSE,
                                         FALSE,
                                         FALSE,
                                         FALSE,
                                         FALSE,
                                         FALSE);
            }
            break;


        case 0x540: /* WMMPS B.K disable UC PSPoll */
            {
                P_PM_INFO_T prPmInfo;
                prPmInfo = &prAdapter->rPmInfo;
                prPmInfo->ucWmmPsDisableUcPoll = (UINT_8)prMcrWrInfo->u4McrData;

                if (prPmInfo->ucWmmPsDisableUcPoll && prPmInfo->ucWmmPsConnWithTrig) {
                    NIC_PM_WMM_PS_DISABLE_UC_TRIG(prAdapter, TRUE);
                }
                else {
                    NIC_PM_WMM_PS_DISABLE_UC_TRIG(prAdapter, FALSE);
                }
            }
            break;

        case 0x541:
            /* Save the new power mode. */
            prAdapter->rConnSettings.rPwrMode = (PARAM_POWER_MODE)prMcrWrInfo->u4McrData;

            if (prAdapter->rConnSettings.rPwrMode == Param_PowerModeCAM) {

                pmFsmRunEventSetPowerSaveProfile(
                    prAdapter,
                    aPowerModeToPwrSaveProfMapping[prAdapter->rConnSettings.rPwrMode]);

            }
            else {
                P_PM_INFO_T prPmInfo;
                prPmInfo = &prAdapter->rPmInfo;

                pmFsmRunEventSetPowerSaveProfile(
                        prAdapter,
                        aPowerModeToPwrSaveProfMapping[prAdapter->rConnSettings.rPwrMode]);

                prPmInfo->ucWmmPsEnterPsAtOnce  = 1;
            }
            break;

        case 0x542:
            {
                UINT_32 u4PacketFilter = 0;
                UINT_8 ucMCAddrList[6] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01};

                UINT_32 u4SetInfoLen;

                u4PacketFilter |= PARAM_PACKET_FILTER_MULTICAST;

                wlanSetInformation(prAdapter,
                                   wlanoidSetCurrentPacketFilter,
                                   &u4PacketFilter,
                                   sizeof(u4PacketFilter),
                                   &u4SetInfoLen);

                wlanSetInformation(prAdapter,
                                   wlanoidSetMulticastList,
                                   &ucMCAddrList[0],
                                   (6),
                                   &u4SetInfoLen);
            }
		    break;
        case 0x543:
            {
				UINT_32 u4SetInfoLen;

				printk("[MT5921] MCR write wlanoidSetWmmPsMode, %d\n",prMcrWrInfo->u4McrData);
                wlanSetInformation(prAdapter,
                                   wlanoidSetWmmPsMode,
                                   &prMcrWrInfo->u4McrData,
                                   sizeof(prMcrWrInfo->u4McrData),
                                   &u4SetInfoLen);
            }		
		    break;
	case 0x545:
	    {
		PER_FLAG = prMcrWrInfo->u4McrData;
		printk("[MT5921] MCR write PER_FLAG %d\n", prMcrWrInfo->u4McrData);	

	    }
		break;				

        }
    }
    else {
        HAL_MCR_WR(prAdapter,
                   (prMcrWrInfo->u4McrOffset & BITS(2,31)), //address is in DWORD unit
                   prMcrWrInfo->u4McrData);
    }
#else
        HAL_MCR_WR(prAdapter,
                  prMcrWrInfo->u4McrOffset & 0xfffc,
                  prMcrWrInfo->u4McrData);
#endif


    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetMcrWrite */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryEepromRead (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID        pvQueryBuffer,
    IN  UINT_32      u4QueryBufferLen,
    OUT PUINT_32     pu4QueryInfoLen
    )
{

    P_PARAM_CUSTOM_EEPROM_RW_STRUC_T prEepromRdInfo;
    UINT_16     u2Data;
    BOOLEAN     fgStatus;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryEepromRead");

    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    prEepromRdInfo = (P_PARAM_CUSTOM_EEPROM_RW_STRUC_T) pvQueryBuffer;

    if(prEepromRdInfo->ucEepromMethod == PARAM_EEPROM_READ_METHOD_READ) {
        fgStatus = nicEepromRead16(prAdapter,
                            prEepromRdInfo->ucEepromIndex,
                            &u2Data
                            );
        if(fgStatus){
            prEepromRdInfo->u2EepromData = u2Data;
            DBGLOG(REQ, INFO, ("EEPROM Read: index=%#X, data=%#02X\n",
                    prEepromRdInfo->ucEepromIndex, u2Data));
        }
        else{
            DBGLOG(REQ, ERROR, ("EEPROM Read Failed: index=%#x.\n",
                    prEepromRdInfo->ucEepromIndex));
            rStatus = WLAN_STATUS_FAILURE;
        }
    }
    else if (prEepromRdInfo->ucEepromMethod == PARAM_EEPROM_READ_METHOD_GETSIZE) {
        prEepromRdInfo->u2EepromData = nicEepromGetSize( prAdapter);
        DBGLOG(REQ, INFO, ("EEPROM size =%d\n", prEepromRdInfo->u2EepromData));
    }

    *pu4QueryInfoLen = sizeof(PARAM_CUSTOM_EEPROM_RW_STRUC_T);


    return rStatus;
}   /* wlanoidQueryEepromRead */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetEepromWrite (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    P_PARAM_CUSTOM_EEPROM_RW_STRUC_T prEepromWrInfo;
    BOOLEAN     fgStatus;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidSetEepromWrite");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_CUSTOM_EEPROM_RW_STRUC_T);

    prEepromWrInfo = (P_PARAM_CUSTOM_EEPROM_RW_STRUC_T) pvSetBuffer;

    DBGLOG(REQ, INFO, ("EEPROM Write: index=0x%x, data=0x%x\n",
        prEepromWrInfo->ucEepromIndex, prEepromWrInfo->u2EepromData));

    fgStatus = nicEepromWrite16(prAdapter,
                                prEepromWrInfo->ucEepromIndex,
                                prEepromWrInfo->u2EepromData
                                );
    if(fgStatus == FALSE){
        DBGLOG(REQ, ERROR, ("EEPROM Write Failed.\n"));
        rStatus = WLAN_STATUS_FAILURE;
    }
    return rStatus;
}   /* wlanoidSetEepromWrite */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRfTestRxStatus (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID        pvQueryBuffer,
    IN  UINT_32      u4QueryBufferLen,
    OUT PUINT_32     pu4QueryInfoLen
    )
{
    P_PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T prRxStatus;
    P_RX_CTRL_T                 prRxCtrl;
    UINT_32                     u4Tmp;

    DEBUGFUNC("wlanoidQueryRfTestRxStatus");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prRxCtrl= &prAdapter->rRxCtrl;

    *pu4QueryInfoLen = sizeof(PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    prRxStatus = (P_PARAM_CUSTOM_RFTEST_RX_STATUS_STRUC_T) pvQueryBuffer;

    /* Query from interrupt result. */
    prRxStatus->u4IntRxOk = (UINT_32)prRxCtrl->au8Statistics[RX_MPDU_TOTAL_COUNT];
    prRxStatus->u4IntCrcErr= (UINT_32)prRxCtrl->au8Statistics[RX_FCS_ERR_DROP_COUNT];
    prRxStatus->u4IntShort= (UINT_32)prRxCtrl->au8Statistics[RX_MPDU_CCK_SHORT_PREAMBLE_COUNT];
    prRxStatus->u4IntLong= (UINT_32)prRxCtrl->au8Statistics[RX_MPDU_CCK_LONG_PREAMBLE_COUNT];

    DBGLOG(REQ, INFO, ("intRxOk: %ld\n", prRxStatus->u4IntRxOk));
    DBGLOG(REQ, INFO, ("intCrcErr: %ld\n", prRxStatus->u4IntCrcErr));
    DBGLOG(REQ, INFO, ("intLong: %ld\n", prRxStatus->u4IntShort));
    DBGLOG(REQ, INFO, ("intShort: %ld\n", prRxStatus->u4IntLong));

    statisticsPeekHWCounter(prAdapter);
    prRxStatus->u4PauCrcErrCount = (UINT_32) prRxCtrl->au8Statistics[RX_BB_FCS_ERROR_COUNT];
    prRxStatus->u4PauRxFifoFullCount = (UINT_32) prRxCtrl->au8Statistics[RX_BB_FIFO_FULL_COUNT];
    prRxStatus->u4PauRxPktCount = (UINT_32) prRxCtrl->au8Statistics[RX_BB_MPDU_COUNT];

    //For CCA time, we need to query it by ourselves.
    HAL_MCR_RD(prAdapter, MCR_MIBSCR, &u4Tmp);
    u4Tmp &= ~MIBSCR_INDEX_MASK;
    u4Tmp |= (MIBSCR_CCATIME_INDEX);
    HAL_MCR_WR(prAdapter, MCR_MIBSCR, u4Tmp);

    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    RX_ADD_CNT(prRxCtrl, RX_BB_CCA_TIME_COUNT, u4Tmp);
    prRxStatus->u4PauCCACount = (UINT_32) prRxCtrl->au8Statistics[RX_BB_CCA_TIME_COUNT];

    return WLAN_STATUS_SUCCESS;
}   /* rftestQueryTestRxStatus */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRfTestTxStatus (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID        pvQueryBuffer,
    IN  UINT_32      u4QueryBufferLen,
    OUT PUINT_32     pu4QueryInfoLen
    )
{
    P_PARAM_CUSTOM_RFTEST_TX_STATUS_STRUC_T prTxStatus;
    P_TX_CTRL_T                 prTxCtrl;
    P_RFTEST_SETTING_STRUC_T    prSetting;
    ALC_VAL                     rAlcVal;

    DEBUGFUNC("wlanoidQueryRfTestTxStatus");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prTxCtrl= &prAdapter->rTxCtrl;
    prSetting = &prAdapter->rRFTestInfo.rSetting;
    *pu4QueryInfoLen = sizeof(PARAM_CUSTOM_RFTEST_TX_STATUS_STRUC_T);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    prTxStatus = (P_PARAM_CUSTOM_RFTEST_TX_STATUS_STRUC_T) pvQueryBuffer;

    /* Query from interrupt result */
    prTxStatus->u4PktSentCount = (UINT_32)prTxCtrl->au8Statistics[TX_MPDU_TOTAL_COUNT];
    prTxStatus->u4PktSentStatus = (UINT_32)prTxCtrl->au8Statistics[TX_MPDU_OK_COUNT];
    if(prSetting->fgALCEn) {
        NIC_ALC_GET_CAL_VALUE(prAdapter, &rAlcVal);
        prTxStatus->u2AvgAlc = rAlcVal;
    }
    else{
        prTxStatus->u2AvgAlc = 0 ;
    }
    if ( prSetting->ucTxRate <= RF_AT_PARAM_RATE_11M ) {
        prTxStatus->ucCckGainControl = prSetting->ucTxPowerGain;
        prTxStatus->ucOfdmGainControl = 0;
    }
    else {
        prTxStatus->ucCckGainControl = 0;
        prTxStatus->ucOfdmGainControl = prSetting->ucTxPowerGain;
    }

    DBGLOG(REQ, INFO, ("pktSentCount = %ld\n", prTxStatus->u4PktSentCount));
    return WLAN_STATUS_SUCCESS;
}   /* rftestQueryTestRxStatus */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryXmitOk (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_TX_CTRL_T     prTxCtrl;
    WLAN_STATUS     rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryXmitOk");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_64);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    prTxCtrl = &prAdapter->rTxCtrl;
    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32) pvQueryBuffer = (UINT_32) prTxCtrl->au8Statistics[TX_MPDU_OK_COUNT];

        DBGLOG(REQ, LOUD, ("Tx OK: %ld\n", *(PUINT_32) pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
        rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64) pvQueryBuffer =  prTxCtrl->au8Statistics[TX_MPDU_OK_COUNT];

        DBGLOG(REQ, LOUD, ("Tx OK: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64) pvQueryBuffer));
    }
    *pu4QueryInfoLen = sizeof(UINT_64);


    return rStatus;
}   /* wlanoidQueryXmitOk */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRcvOk (
    IN  P_ADAPTER_T     prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    )
{
    P_RX_CTRL_T     prRxCtrl;
    WLAN_STATUS     rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryRcvOk");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_64);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    prRxCtrl = &prAdapter->rRxCtrl;
    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32) pvQueryBuffer = (UINT_32) prRxCtrl->au8Statistics[RX_DATA_INDICATION_COUNT];
        DBGLOG(REQ, LOUD, ("Rx OK: %ld\n", *(PUINT_32) pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
        rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64) pvQueryBuffer =  prRxCtrl->au8Statistics[RX_DATA_INDICATION_COUNT];
        DBGLOG(REQ, LOUD, ("Rx OK: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64) pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);

    return rStatus;
}   /* wlanoidQueryRcvOk */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryXmitError (
    IN  P_ADAPTER_T     prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    )
{
    P_TX_CTRL_T     prTxCtrl;
    WLAN_STATUS     rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryXmitError");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prTxCtrl = &prAdapter->rTxCtrl;

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)prTxCtrl->au8Statistics[TX_MPDU_ALL_ERR_COUNT];

        DBGLOG(REQ, LOUD, ("Tx Error: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
       rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = prTxCtrl->au8Statistics[TX_MPDU_ALL_ERR_COUNT];

        DBGLOG(REQ, LOUD, ("Tx Error: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);

    return rStatus;
} /* wlanoidQueryXmitError */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryXmitOneCollision (
    IN  P_ADAPTER_T     prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    )
{
    P_TX_CTRL_T     prTxCtrl;
    WLAN_STATUS     rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryXmitOneCollision");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prTxCtrl = &prAdapter->rTxCtrl;

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)prTxCtrl->au8Statistics[TX_MPDU_TX_TWICE_OK_COUNT];

        DBGLOG(REQ, LOUD, ("Tx one retry and get ACK: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
       rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = prTxCtrl->au8Statistics[TX_MPDU_TX_TWICE_OK_COUNT];

        DBGLOG(REQ, LOUD, ("Tx one retry and get ACK: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);
    return rStatus;

} /* wlanoidQueryXmitOneCollision */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryXmitMoreCollisions (
    IN  P_ADAPTER_T     prAdapter,
    IN  PVOID           pvQueryBuffer,
    IN  UINT_32         u4QueryBufferLen,
    OUT PUINT_32        pu4QueryInfoLen
    )
{
    P_TX_CTRL_T     prTxCtrl;
    WLAN_STATUS     rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryXmitMoreCollisions");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prTxCtrl = &prAdapter->rTxCtrl;

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)prTxCtrl->au8Statistics[TX_MPDU_TX_MORE_TWICE_OK_COUNT];

        DBGLOG(REQ, LOUD, ("Tx more than 2 retry and get ACK: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
       rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = prTxCtrl->au8Statistics[TX_MPDU_TX_MORE_TWICE_OK_COUNT];

        DBGLOG(REQ, LOUD, ("Tx more than 2 retry and get ACK: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);
    return rStatus;

} /* wlanoidQueryXmitMoreCollisions */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryXmitMaxCollisions (
    IN   P_ADAPTER_T     prAdapter,
    IN   PVOID           pvQueryBuffer,
    IN   UINT_32         u4QueryBufferLen,
    OUT  PUINT_32        pu4QueryInfoLen
    )
{
    P_TX_CTRL_T     prTxCtrl;
    WLAN_STATUS     rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryXmitMaxCollisions");
    DBGLOG(REQ, TRACE, ("\n"));

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prTxCtrl = &prAdapter->rTxCtrl;

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (pvQueryBuffer == NULL) {
        ASSERT(FALSE);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (u4QueryBufferLen == sizeof(UINT_32)) {
        /* Return a 32-bit counter. */
        *(PUINT_32)pvQueryBuffer = (UINT_32)prTxCtrl->au8Statistics[TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT];

        DBGLOG(REQ, LOUD, ("Tx max collisions and abort: %ld\n", *(PUINT_32)pvQueryBuffer));
    }
    else if (u4QueryBufferLen < sizeof(UINT_64)) {
        /* Not enough room for the query information. */
       rStatus = WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        /* Return a 64-bit counter. */
        *(PUINT_64)pvQueryBuffer = prTxCtrl->au8Statistics[TX_MPDU_MPDU_EXCESS_RETRY_ERR_COUNT];

        DBGLOG(REQ, LOUD, ("Tx max collisions and abort: %"DBG_PRINTF_64BIT_DEC"\n", *(PUINT_64)pvQueryBuffer));
    }

    *pu4QueryInfoLen = sizeof(UINT_64);
    return rStatus;
}   /* wlanoidQueryXmitMaxCollisions */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryOidInterfaceVersion (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuffer,
    IN  UINT_32  u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen)
{
    DEBUGFUNC("wlanoidQueryOidInterfaceVersion");


    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    *(PUINT_32) pvQueryBuffer = MTK_CUSTOM_OID_INTERFACE_VERSION ;
    *pu4QueryInfoLen = sizeof(UINT_32);

    DBGLOG(REQ, INFO, ("Custom OID interface version: %#08lX\n",
        *(PUINT_32) pvQueryBuffer));

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryOidInterfaceVersion */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryMulticastList(
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    ASSERT(prAdapter->ucMulticastAddrNum <= MAX_NUM_GROUP_ADDR);

    *pu4QueryInfoLen = (UINT_32)prAdapter->ucMulticastAddrNum * MAC_ADDR_LEN;

    /* Check if the query buffer is large enough to hold all the query
       information. */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        /* Not enough room for the query information. */
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    kalMemCopy(pvQueryBuffer,
               prAdapter->aucMulticastAddr,
               *pu4QueryInfoLen);

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryMulticastList() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetMulticastList(
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    /* The data must be a multiple of the Ethernet address size. */
    if (u4SetBufferLen % MAC_ADDR_LEN != 0) {
        DBGLOG(REQ, WARN, ("Invalid MC list length %ld\n", u4SetBufferLen));

        *pu4SetInfoLen = ((u4SetBufferLen + MAC_ADDR_LEN - 1) /
            MAC_ADDR_LEN) * MAC_ADDR_LEN;

        return WLAN_STATUS_INVALID_LENGTH;
    }

    *pu4SetInfoLen = u4SetBufferLen;

    /* Verify if we can support so many multicast addresses. */
    if (u4SetBufferLen / MAC_ADDR_LEN > MAX_NUM_GROUP_ADDR) {
        DBGLOG(REQ, WARN, ("Too many MC addresses\n"));

        return WLAN_STATUS_MULTICAST_FULL;
    }

    /* NOTE(Kevin): Windows may set u4SetBufferLen == 0 &&
     * pvSetBuffer == NULL to clear exist Multicast List.
     */
    if (u4SetBufferLen) {
        ASSERT(pvSetBuffer);
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set multicast list! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    nicRxSetMulticast(prAdapter,
                      MC_TYPE_UPDATE_LIST_ONLY,
                      pvSetBuffer,
                      (UINT_8)(u4SetBufferLen / MAC_ADDR_LEN));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetMulticastList() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetCurrentPacketFilter (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    UINT_32 u4NewPacketFilter;
    //UINT_32 u4FilterChanges;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidSetCurrentPacketFilter");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }
    ASSERT(pvSetBuffer);

    /* Set the new packet filter. */
    u4NewPacketFilter = *(PUINT_32) pvSetBuffer;

    DBGLOG(REQ, INFO, ("New packet filter: %#08lx\n", u4NewPacketFilter));

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set current packet filter! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    do {
        /* Verify the bits of the new packet filter. If any bits are set that
           we don't support, leave. */
        if (u4NewPacketFilter & ~(PARAM_PACKET_FILTER_SUPPORTED)) {
            DBGLOG(RSN, WARN, ("Filter Type %#08lx not supported\n", u4NewPacketFilter));
            rStatus = WLAN_STATUS_NOT_SUPPORTED;
            break;
        }

        /* Need to enable or disable promiscuous support depending on the new
           filter. */

        if (u4NewPacketFilter & PARAM_PACKET_FILTER_PROMISCUOUS) {
            DBGLOG(REQ, INFO, ("Enable promiscuous mode\n"));
            //4 2007/11/27, mikewu, <<TODO>> Enable Promiscuous mode
            wlanSetPromiscuousMode(prAdapter, TRUE);
        }
        else {
            DBGLOG(REQ, INFO, ("Disable promiscuous mode\n"));
            //4 2007/11/27, mikewu, <<TODO>> Disable Promiscuous mode
            wlanSetPromiscuousMode(prAdapter, FALSE);
        }

        if (u4NewPacketFilter & PARAM_PACKET_FILTER_ALL_MULTICAST) {
            DBGLOG(REQ, INFO, ("Enable all-multicast mode\n"));
            //4 2007/11/27, mikewu, <<TODO>> Enable all multicast mode
            nicRxSetMulticast(prAdapter, MC_TYPE_ALLOW_ALL, (PUINT_8)NULL, 0);
        }
        else if (u4NewPacketFilter & PARAM_PACKET_FILTER_MULTICAST) {
            DBGLOG(REQ, INFO, ("Enable multicast\n"));

            /* Set multicast address table by calling regChangeMCAddress. */
            //4 2007/11/27, mikewu, <<TODO>>Check MC address list
            /* NOTE(Kevin): it also clear the Multicast List */
            nicRxSetMulticast(prAdapter, MC_TYPE_ALLOW_LIST, (PUINT_8)NULL, 0);
        }
        else {
            DBGLOG(REQ, INFO, ("Disable multicast\n"));
            //4 2007/11/27, mikewu, <<TODO>> Disable multicast
            nicRxSetMulticast(prAdapter, MC_TYPE_DENY_ALL, (PUINT_8)NULL, 0);
        }

        if (u4NewPacketFilter & PARAM_PACKET_FILTER_BROADCAST) {
            DBGLOG(REQ, INFO, ("Enable Broadcast\n"));
            //4 2007/11/27, mikewu, <<TODO>> Enable Broadcast
            wlanRxSetBroadcast(prAdapter, TRUE);
        }
        else {
            DBGLOG(REQ, INFO, ("Disable Broadcast\n"));
            //4 2007/11/27, mikewu, <<TODO>> Disable Broadcast
            wlanRxSetBroadcast(prAdapter, FALSE);
        }

    } while (FALSE);

    // Store the packet filter
    prAdapter->u4OsPacketFilter = u4NewPacketFilter;

    return rStatus;
}   /* wlanoidSetCurrentPacketFilter */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryCurrentPacketFilter (
    IN P_ADAPTER_T  prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryCurrentPacketFilter");
    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen >= sizeof(UINT_32)) {
        ASSERT(pvQueryBuffer);
        *(PUINT_32) pvQueryBuffer = prAdapter->u4OsPacketFilter;
    }

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryCurrentPacketFilter */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryAcpiDevicePowerState (
    IN P_ADAPTER_T prAdapter,
    IN  PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
#if DBG
    PPARAM_DEVICE_POWER_STATE prPowerState;
#endif

    DEBUGFUNC("wlanoidQueryAcpiDevicePowerState");
    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PARAM_DEVICE_POWER_STATE);

    ASSERT(pvQueryBuffer);
#if DBG
    prPowerState = (PPARAM_DEVICE_POWER_STATE) pvQueryBuffer;
    switch (*prPowerState) {
    case ParamDeviceStateD0:
        DBGLOG(REQ, INFO, ("Query Power State: D0\n"));
        break;
    case ParamDeviceStateD1:
        DBGLOG(REQ, INFO, ("Query Power State: D1\n"));
        break;
    case ParamDeviceStateD2:
        DBGLOG(REQ, INFO, ("Query Power State: D2\n"));
        break;
    case ParamDeviceStateD3:
        DBGLOG(REQ, INFO, ("Query Power State: D3\n"));
        break;
    default:
        break;
    }
#endif

    *(PPARAM_DEVICE_POWER_STATE) pvQueryBuffer = ParamDeviceStateD3;
    DBGLOG(REQ, INFO, ("Ready to transition to D3\n"));

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryAcpiDevicePowerState */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAcpiDevicePowerState (
    IN P_ADAPTER_T prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    PPARAM_DEVICE_POWER_STATE prPowerState;

    DEBUGFUNC("wlanoidSetAcpiDevicePowerState");
    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_DEVICE_POWER_STATE);

    ASSERT(pvSetBuffer);
    prPowerState = (PPARAM_DEVICE_POWER_STATE) pvSetBuffer;
    switch (*prPowerState) {
    case ParamDeviceStateD0:
        DBGLOG(REQ, INFO, ("Set Power State: D0\n"));
        kalDevSetPowerState(prAdapter->prGlueInfo, (UINT_32)ParamDeviceStateD0);
        pmSetAcpiPowerD0(prAdapter);
        break;
    case ParamDeviceStateD1:
        DBGLOG(REQ, INFO, ("Set Power State: D1\n"));
        /* no break here */
    case ParamDeviceStateD2:
        DBGLOG(REQ, INFO, ("Set Power State: D2\n"));
        /* no break here */
    case ParamDeviceStateD3:
        DBGLOG(REQ, INFO, ("Set Power State: D3\n"));
        pmSetAcpiPowerD3(prAdapter);
        kalDevSetPowerState(prAdapter->prGlueInfo, (UINT_32)ParamDeviceStateD3);
#if !defined(WINDOWS_DDK)
        /* CR01858: set flag to FALSE to avoid automatic reconnection after
        * switching from D3 to D0 state.
        */
        DBGLOG(REQ, INFO, ("Clear fgIsConnReqIssued\n"));
        prAdapter->rConnSettings.fgIsConnReqIssued = FALSE;
        //DBGLOG(REQ, INFO, ("Set fgScanListPending = TRUE\n"));
        //prAdapter->prGlueInfo->fgScanListPending = TRUE;
        //prAdapter->rScanInfo->fgFullScanDone = FALSE;
#endif
        break;
    default:
        break;
    }

    return WLAN_STATUS_SUCCESS;
} /* end of wlanoidSetAcpiDevicePowerState() */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryFragThreshold (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryFragThreshold");

    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

#if CFG_TX_FRAGMENT
    if (u4QueryBufferLen < sizeof(PARAM_FRAGMENTATION_THRESHOLD)) {
        *pu4QueryInfoLen = sizeof(PARAM_FRAGMENTATION_THRESHOLD);
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    *(PUINT_32)pvQueryBuffer = (UINT_32)prAdapter->rConnSettings.u2FragmentationThreshold;
    *pu4QueryInfoLen = sizeof(PARAM_FRAGMENTATION_THRESHOLD);

    DBGLOG(REQ, INFO, ("Current fragmentation threshold: %d\n",
        prAdapter->rConnSettings.u2FragmentationThreshold));

    return WLAN_STATUS_SUCCESS;

#else

    return WLAN_STATUS_NOT_SUPPORTED;
#endif /* CFG_TX_FRAGMENT */

} /* end of wlanoidQueryFragThreshold() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetFragThreshold (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
#if CFG_TX_FRAGMENT
    P_CONNECTION_SETTINGS_T prConnSettings;
    UINT_32 u4NewFragThreshold;

    DEBUGFUNC("wlanoidSetFragThreshold");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    prConnSettings = &prAdapter->rConnSettings;

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }


    DBGLOG(REQ, INFO, ("\n"));

    if (u4SetBufferLen < sizeof(PARAM_FRAGMENTATION_THRESHOLD)) {
        *pu4SetInfoLen = sizeof(PARAM_FRAGMENTATION_THRESHOLD);
        return WLAN_STATUS_INVALID_LENGTH;
    }

    u4NewFragThreshold = *(PUINT_32)pvSetBuffer;
    *pu4SetInfoLen = sizeof(PARAM_FRAGMENTATION_THRESHOLD);


    /* Verify the new fragmentation threshold. */
    /* Zero means to disable fragmentation.
      Refer to OID_802_11_FRAGMENTATION_THRESHOLD
    */
    if ((u4NewFragThreshold != 0) &&
        ((u4NewFragThreshold < DOT11_FRAGMENTATION_THRESHOLD_MIN) ||
         (u4NewFragThreshold > DOT11_FRAGMENTATION_THRESHOLD_MAX) ||
         (u4NewFragThreshold & 0x01))) {

        DBGLOG(REQ, TRACE, ("Invalid fragmentation threshold %ld\n",
            u4NewFragThreshold));
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Save the new fragmentation threshold to the MIB. */

    prConnSettings->u2FragmentationThreshold = (UINT_16)u4NewFragThreshold;

    if (prConnSettings->u2FragmentationThreshold) {
        prConnSettings->fgIsEnableTxFragment = TRUE;

        if (prAdapter->eConnectionState ==
            MEDIA_STATE_CONNECTED) {
            txFragInfoUpdate(prAdapter);
        }
    }
    else {
        prConnSettings->fgIsEnableTxFragment = FALSE;
        prConnSettings->fgIsEnableTxAutoFragmentForBT = FALSE;
    }

    return WLAN_STATUS_SUCCESS;

#else

    return WLAN_STATUS_NOT_SUPPORTED;
#endif /* CFG_TX_FRAGMENT */

} /* end of wlanoidSetFragThreshold() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRtsThreshold (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryRtsThreshold");

    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    if (u4QueryBufferLen < sizeof(PARAM_RTS_THRESHOLD)) {
        *pu4QueryInfoLen = sizeof(PARAM_RTS_THRESHOLD);
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    *(PUINT_32)pvQueryBuffer = (UINT_32)prAdapter->rConnSettings.u2RTSThreshold;
    *pu4QueryInfoLen = sizeof(PARAM_RTS_THRESHOLD);

    DBGLOG(REQ, INFO, ("Current RTS threshold: %d\n", prAdapter->rConnSettings.u2RTSThreshold));

    return WLAN_STATUS_SUCCESS;

} /* wlanoidQueryRtsThreshold */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetRtsThreshold (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    UINT_16 u2NewRTSThreshold;

    DEBUGFUNC("wlanoidSetRtsThreshold");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    *pu4SetInfoLen = sizeof(PARAM_RTS_THRESHOLD);

    if (u4SetBufferLen < sizeof(PARAM_RTS_THRESHOLD)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    u2NewRTSThreshold = (UINT_16)(*(PUINT_32)pvSetBuffer);

    /* Verify the new RTS threshold. */
    if (u2NewRTSThreshold > DOT11_RTS_THRESHOLD_MAX) {

        DBGLOG(REQ, TRACE, ("Invalid RTS threshold %d\n",
            u2NewRTSThreshold));
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Save the new RTS threshold to the MIB. */
    prAdapter->rConnSettings.u2RTSThreshold = u2NewRTSThreshold;

    nicRateSetRTSThreshold(prAdapter, u2NewRTSThreshold);

    return WLAN_STATUS_SUCCESS;

} /* wlanoidSetRtsThreshold */


#if PTA_ENABLED
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetBtCoexistCtrl (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_PARAM_CUSTOM_BT_COEXIST_T prBtSetting;
#if CFG_TX_FRAGMENT
    P_CONNECTION_SETTINGS_T     prConnSettings;
#endif /* CFG_TX_FRAGMENT */

    DEBUGFUNC("wlanoidSetBtCoexistCtrl");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_CUSTOM_BT_COEXIST_T);

    if (u4SetBufferLen != sizeof(PARAM_CUSTOM_BT_COEXIST_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }
    ASSERT(pvSetBuffer);
    prBtSetting = (P_PARAM_CUSTOM_BT_COEXIST_T) pvSetBuffer;

#if CFG_TX_FRAGMENT
    prConnSettings = &prAdapter->rConnSettings;
    if (prBtSetting->eBTCoexistWindowType >= BT_COEXIST_WINDOW_TYPE_NUM) {
        return WLAN_STATUS_INVALID_DATA;
    }
#endif /* CFG_TX_FRAGMENT */

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set BT setting because of ACPI_D3\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    /* Store parameters to connSetting for chip reset path */
    kalMemCopy((PVOID) &prAdapter->rConnSettings.rPtaParam,
            &prBtSetting->rPtaParam, sizeof(PTA_PARAM_T));

    ptaFsmRunEventSetConfig(prAdapter, &prBtSetting->rPtaParam);

#if CFG_TX_FRAGMENT
    /* Judge if TX auto-fragment is applied for BT-coexist after
     * ptaFsmRunEventSetConfig() or ptaFsmInit() is invoked
     */
    prConnSettings->fgTryTxAutoFragmentForBT =
            prBtSetting->u4IsEnableTxAutoFragmentForBT ? TRUE : FALSE;
    prConnSettings->eBTCoexistWindowType =
        (ENUM_PARAM_BT_COEXIST_WINDOW_T) prBtSetting->eBTCoexistWindowType;

    prConnSettings->fgIsEnableTxAutoFragmentForBT =
            (prConnSettings->fgIsEnableTxFragment &&
             prAdapter->rPtaInfo.eCurrentState == PTA_STATE_ON) ?
            prConnSettings->fgTryTxAutoFragmentForBT : FALSE;
#endif /* CFG_TX_FRAGMENT */

    return WLAN_STATUS_SUCCESS;

} /* wlanoidSetBtCoexistCtrl */
#endif /* end of PTA_ENABLED */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidAddTS (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    ASSERT(prAdapter);

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in Add TS! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    // NOT IMPLEMENTED YET!
    ASSERT(0);

    return WLAN_STATUS_SUCCESS;

}   /* wlanoidAddTS */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidDelTS (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    ASSERT(prAdapter);

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in Del TS! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    // NOT IMPLEMENTED YET!
    ASSERT(0);

    return WLAN_STATUS_SUCCESS;

}   /* wlanoidDelTS */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryVoipConnectionStatus (
    IN P_ADAPTER_T  prAdapter,
    OUT  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("wlanoidQueryVoipConnectionStatus");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    prConnSettings = &prAdapter->rConnSettings;

    ASSERT(pvQueryBuf);

    *((PUINT_32)pvQueryBuf) = prConnSettings->u4VoipTrafficInterval;

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryVoipConnectionStatus() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetVoipConnectionStatus (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    P_PARAM_VOIP_CONFIG prVoipCfg;
    P_CONNECTION_SETTINGS_T prConnSettings;


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_VOIP_CONFIG);

    if (u4SetBufferLen < sizeof(PARAM_VOIP_CONFIG)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set VoIP polling! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    prConnSettings = &prAdapter->rConnSettings;

    ASSERT(pvSetBuffer);
    prVoipCfg = (P_PARAM_VOIP_CONFIG)pvSetBuffer;

    if (prVoipCfg->u4VoipTrafficInterval) {
        prConnSettings->fgIsVoipConn = TRUE;
        prConnSettings->u4VoipTrafficInterval = prVoipCfg->u4VoipTrafficInterval;

        /* Setup for SCAN module */

        /* Setup for PM module */
        // Setup timer for VOIP application
        pmEnableVoipPollingFunc(prAdapter);
    }
    else {
        prConnSettings->fgIsVoipConn = FALSE;
        prConnSettings->u4VoipTrafficInterval = 0;

        pmDisableVoipPollingFunc(prAdapter);
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetVoipConnectionStatus() */


#if 0 /* SUPPORT_WPS */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAppIE(
    IN P_ADAPTER_T        prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    UINT_32     u4Buffer[2];
    UINT_32     u4MgmtType;
    UINT_32     u4Len;
    PUINT_8     pucBuf;

    DEBUGFUNC("wlanoidSetAppIE");


    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    kalMemCopyFromUser(u4Buffer, pvSetBuffer, sizeof(u4Buffer));
    u4MgmtType = u4Buffer[0];
    u4Len = u4Buffer[1];

    *pu4SetInfoLen = u4SetBufferLen;


    if (u4MgmtType > IEEE80211_APPIE_NUM_OF_FRAME || (u4Len > 300)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    pucBuf = (PUINT_8)((UINT_32)pvSetBuffer + sizeof(u4Buffer));

    kalMemCopyFromUser(prAdapter->aucAppIE[u4MgmtType], pucBuf, u4Len);
    prAdapter->aucAppIELen[u4MgmtType] = u4Len;

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetAppIE() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetFrameFilter(
    IN P_ADAPTER_T        prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    UINT_32     u4Buffer[2];
    UINT_32     u4MgmtType;
    UINT_32     u4Len;
    //PUINT_8     pucBuf;

    DEBUGFUNC("wlanoidSetFrameFilter");


    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    kalMemCopyFromUser(u4Buffer, pvSetBuffer, sizeof(u4Buffer));
    u4MgmtType = u4Buffer[0];
    u4Len = u4Buffer[1];

    *pu4SetInfoLen = u4SetBufferLen;

    /* Todo: the filter */
    if (u4MgmtType == 0) {
        prAdapter->fgIndMgt = FALSE;
    }
    else if (u4MgmtType != 0) {
        prAdapter->fgIndMgt = TRUE;
    }

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetFrameFilter */
#endif /* SUPPORT_WPS */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetDisassociate (
    IN P_ADAPTER_T        prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{

    DEBUGFUNC("wlanoidSetDisassociate");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = 0;

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set disassociate! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    if (prAdapter->eConnectionState == MEDIA_STATE_CONNECTED) {
        arbFsmRunEventJoinDisassoc(prAdapter);
    }
    else {
        prAdapter->rConnSettings.fgIsConnReqIssued = FALSE;
        arbFsmRunEventAbort(prAdapter, FALSE);

//        prAdapter->fgIsRadioOff = TRUE;
    }

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetDisassociate */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQuery802dot11PowerSaveProfile (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQuery802dot11PowerSaveProfile");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (u4QueryBufferLen!=0) {
        ASSERT(pvQueryBuffer);
    *(PPARAM_POWER_MODE) pvQueryBuffer = prAdapter->rConnSettings.rPwrMode;
    *pu4QueryInfoLen = sizeof(PARAM_POWER_MODE);
    }
    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSet802dot11PowerSaveProfile (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       prSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    DEBUGFUNC("wlanoidSet802dot11PowerSaveProfile");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_POWER_MODE);

    ASSERT(prSetBuffer);

    /* Check if the new power mode is valid. */
    if (*(PPARAM_POWER_MODE) prSetBuffer >= Param_PowerModeMax) {
        WARNLOG(("Invalid power mode %d\n",
            *(PPARAM_POWER_MODE) prSetBuffer));
        return WLAN_STATUS_INVALID_DATA;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    /* Save the new power mode. */
    prAdapter->rConnSettings.rPwrMode = *(PPARAM_POWER_MODE) prSetBuffer;

    pmFsmRunEventSetPowerSaveProfile(
            prAdapter,
            aPowerModeToPwrSaveProfMapping[prAdapter->rConnSettings.rPwrMode]);

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetAcpiDevicePowerStateMode() */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetBgSsidParam (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_SCAN_CONFIG_T     prScanCfg;

    DEBUGFUNC("wlanoidSetBgSsidParam");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (u4SetBufferLen != sizeof(BG_SCAN_CONFIG_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    prScanCfg = &prAdapter->rScanInfo.rScanConfig;

    *pu4SetInfoLen = sizeof(BG_SCAN_CONFIG_T);

    if (prScanCfg->rBgScanCfg.rScanCandidate.ucNumHwSsidScanEntry) {
        kalMemCopy(
            &prScanCfg->rBgScanCfg,
            pvSetBuffer,
            sizeof(BG_SCAN_CONFIG_T));

        prScanCfg->rBgScanCfg.fgIsFromUserSetting = TRUE;
    }
    else {
        kalMemZero(
            &prScanCfg->rBgScanCfg,
            sizeof(BG_SCAN_CONFIG_T));
    }

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetBgSsidParam */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetPatternConfig (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_CUSTOM_PATTERN_SEARCH_CONFIG_STRUC_T prPtrnCfg;
    UINT_32 i;
    P_PM_INFO_T prPmInfo;

    DEBUGFUNC("wlanoidSetPatternConfig");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    prPtrnCfg = (P_PARAM_CUSTOM_PATTERN_SEARCH_CONFIG_STRUC_T) pvSetBuffer;
    *pu4SetInfoLen = sizeof(PARAM_CUSTOM_PATTERN_SEARCH_CONFIG_STRUC_T);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set pattern search function! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    prPmInfo = &prAdapter->rPmInfo;

    /* Initialize LP instruction number, all of the extra instruction need to be re-programmed again */
    prPmInfo->ucNumOfInstSleep = 0;
    prPmInfo->ucNumOfInstAwake = 0;
    prPmInfo->ucNumOfInstOn = 0;

    // setup the pattern (total 32 patterns in HW engine)
    for (i = 0; i < HW_PATTERN_SEARCH_SET_NUMBER; i++) {
        nicpmConfigPattern(prAdapter,
                           prPtrnCfg->arPattern[i].ucIndex,
                           prPtrnCfg->arPattern[i].fgCheckBcA1,
                           prPtrnCfg->arPattern[i].fgCheckMcA1,
                           prPtrnCfg->arPattern[i].fgCheckUcA1,
                           prPtrnCfg->arPattern[i].fgIpv4Ip,
                           prPtrnCfg->arPattern[i].fgIpv6Icmp,
                           prPtrnCfg->arPattern[i].fgGarpIpEqual,
                           prPtrnCfg->arPattern[i].fgArpCtrl,
                           prPtrnCfg->arPattern[i].fgAndOp,
                           prPtrnCfg->arPattern[i].fgNotOp,
                           prPtrnCfg->arPattern[i].ucPatternMask,
                           prPtrnCfg->arPattern[i].ucPatternOffset,
                           prPtrnCfg->arPattern[i].au4Pattern);
    }

    // setup the function
    nicpmConfigPatternSearchFunc(prAdapter,
                                 prPtrnCfg->rFunc.fgBcPatternMatchEnable,
                                 prPtrnCfg->rFunc.fgMcPatternMatchEnable,
                                 prPtrnCfg->rFunc.fgUcPatternMatchEnable,
                                 prPtrnCfg->rFunc.fgBcPatternMatchOperation,
                                 prPtrnCfg->rFunc.fgMcPatternMatchOperation,
                                 prPtrnCfg->rFunc.fgUcPatternMatchOperation,
                                 prPtrnCfg->rFunc.fgIpv6MatchCtrl);


    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetPatternConfig */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetPwrMgmtProfParam (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    DEBUGFUNC("wlanoidSetPwrMgmtProfParam");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PM_PROFILE_SETUP_INFO_T);
    if (u4SetBufferLen != sizeof(PM_PROFILE_SETUP_INFO_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    kalMemCopy(
        prPmProfSetupInfo,
        pvSetBuffer,
        sizeof(PM_PROFILE_SETUP_INFO_T));

    return WLAN_STATUS_SUCCESS;
}   /* wlanoidSetPwrMgmtProfParam */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryPwrMgmtProfParam (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    DEBUGFUNC("wlanoidQueryPwrMgmtProfParam");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PM_PROFILE_SETUP_INFO_T);

    if (u4QueryBufLen != sizeof(PM_PROFILE_SETUP_INFO_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuf);


    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    kalMemCopy(
        pvQueryBuf,
        prPmProfSetupInfo,
        sizeof(PM_PROFILE_SETUP_INFO_T));


    return WLAN_STATUS_SUCCESS;
}   /* wlanoidQueryPwrMgmtProfParam */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryRoamingFunction (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryRoamingFunction");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    *(PUINT_32)pvQueryBuffer = (UINT_32)prAdapter->rConnSettings.fgIsEnableRoaming;

    DBGLOG(REQ, INFO, ("Current Roaming Status: %d\n",
        prAdapter->rConnSettings.fgIsEnableRoaming));

    return WLAN_STATUS_SUCCESS;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetRoamingFunction (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    DEBUGFUNC("wlanoidSetRoamingFunction");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    ASSERT(pvSetBuffer);

    if (*(PUINT_32)pvSetBuffer == 1) { /* Turn on Roaming */
        if (!prAdapter->rConnSettings.fgIsEnableRoaming) {

            prAdapter->rConnSettings.fgIsEnableRoaming = TRUE;

            roamingFsmRunEventStart(prAdapter);
        }
    }
    else if (*(PUINT_32)pvSetBuffer == 0) { /* Turn off Roaming */

        if (prAdapter->rConnSettings.fgIsEnableRoaming) {

            roamingFsmRunEventAbort(prAdapter);

            prAdapter->rConnSettings.fgIsEnableRoaming = FALSE;
        }
    }
    else {
        return WLAN_STATUS_INVALID_DATA;
    }

    return WLAN_STATUS_SUCCESS;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryAdHocMode (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryAdHocMode");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    *(PUINT_32)pvQueryBuffer = (UINT_32)prAdapter->rConnSettings.eAdHocMode;

    DBGLOG(REQ, INFO, ("Current AdHoc Mode: %d\n",
        prAdapter->rConnSettings.eAdHocMode));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryAdHocMode() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAdHocMode (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    DEBUGFUNC("wlanoidSetAdHocMode");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);

    switch(*(PUINT_32)pvSetBuffer) {
    case AD_HOC_MODE_11B:
    case AD_HOC_MODE_MIXED_11BG:
    case AD_HOC_MODE_11G:
    case AD_HOC_MODE_11A:
        prAdapter->rConnSettings.eAdHocMode =
            (ENUM_PARAM_AD_HOC_MODE_T)(*(PUINT_32)pvSetBuffer);
        break;

    default:
        return WLAN_STATUS_INVALID_DATA;
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetAdHocMode() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryFrequency (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_RF_INFO_T prRFInfo;

    DEBUGFUNC("wlanoidQueryFrequency");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prRFInfo = &prAdapter->rRFInfo;

    DBGLOG(REQ, INFO, ("\n"));

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    *(PUINT_32)pvQueryBuffer =
        nicGetFreqByChannelNum(prRFInfo->ucChannelNum,
                               prRFInfo->eBand);

    DBGLOG(REQ, INFO, ("Current RF frequency: %ld\n",
                        *(PUINT_32)pvQueryBuffer));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryFrequency() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetFrequency (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    WLAN_STATUS  rStatus = WLAN_STATUS_SUCCESS;
    PUINT_32 pu4FreqInKHz;
    UINT_8 ucChannelNum;
    ENUM_BAND_T eBand;


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);
    pu4FreqInKHz = (PUINT_32)pvSetBuffer;

    if (!nicGetChannelNumByFreq(*pu4FreqInKHz, &ucChannelNum, &eBand)) {
        DBGLOG(REQ, TRACE, ("Invalid Frequency = %ld\n", *pu4FreqInKHz));
        return WLAN_STATUS_INVALID_DATA;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        PARAM_802_11_CONFIG_T rNewConfig = {0};
        rNewConfig.u4DSConfig = *(PUINT_32)pu4FreqInKHz;
        rStatus =
            rftestSet80211Configuration(
                prAdapter,
                &rNewConfig
                );
        if(rStatus == WLAN_STATUS_SUCCESS)
        {
            DBGLOG(REQ, INFO, ("Change channel successfully.\n"));
        }
        else{
            DBGLOG(REQ, ERROR, ("Can not switch RF channel\n"));

        }
    }
    else {
        prAdapter->rConnSettings.ucChannelNum = ucChannelNum;
        prAdapter->rConnSettings.ucChannelBand = (UINT_8)eBand;
        prAdapter->rConnSettings.u4FreqInKHz = *pu4FreqInKHz;

        if (prAdapter->rConnSettings.eOPMode != NET_TYPE_INFRA) {
            /* Send the "Abort" event to arbiter. */
            arbFsmRunEventAbort(prAdapter, TRUE); /* Reset to STANDBY */
        }
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetFrequency() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetChannel (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    PUINT_32 pu4ChannelNum;
    UINT_8 ucChannelNum;
    UINT_32 u4FreqInKHz;


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    ASSERT(pvSetBuffer);
    pu4ChannelNum = (PUINT_32)pvSetBuffer;

    ucChannelNum = (UINT_8)*pu4ChannelNum;

    /* TODO(Kevin) : Consider 802.11a according to PhyType Selection */
    u4FreqInKHz = nicGetFreqByChannelNum(ucChannelNum, BAND_24G);
    if (u4FreqInKHz == 0) {

        DBGLOG(REQ, TRACE, ("Invalid Channel Number = %d\n", ucChannelNum));
        return WLAN_STATUS_INVALID_DATA;
    }

    prAdapter->rConnSettings.ucChannelNum = ucChannelNum;
    prAdapter->rConnSettings.ucChannelBand = (UINT_8)BAND_24G;
    prAdapter->rConnSettings.u4FreqInKHz = u4FreqInKHz;

    if (prAdapter->rConnSettings.eOPMode != NET_TYPE_INFRA) {
        /* Send the "Abort" event to arbiter. */
        arbFsmRunEventAbort(prAdapter, TRUE); /* Reset to STANDBY */
    }


    return WLAN_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryBeaconInterval (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryBeaconInterval");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    if (prAdapter->eConnectionState== MEDIA_STATE_CONNECTED) {
        *(PUINT_32)pvQueryBuffer =
            (UINT_32)prAdapter->rBssInfo.u2BeaconInterval;
    }
    else {
        if (prAdapter->rConnSettings.eOPMode == NET_TYPE_INFRA) {
            *(PUINT_32)pvQueryBuffer = 0;
        }
        else {
            *(PUINT_32)pvQueryBuffer =
                (UINT_32)prAdapter->rConnSettings.u2BeaconPeriod;
        }
    }

    DBGLOG(REQ, INFO, ("Current beacon interval: %ld\n",
                        *(PUINT_32)pvQueryBuffer));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryBeaconInterval() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetBeaconInterval (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PUINT_32 pu4BeaconInterval;

    DEBUGFUNC("wlanoidSetBeaconInterval");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    ASSERT(pvSetBuffer);
    pu4BeaconInterval = (PUINT_32)pvSetBuffer;

    /* TODO(Kevin) : Consider 802.11a according to PhyType Selection */
    if ((*pu4BeaconInterval < DOT11_BEACON_PERIOD_MIN) ||
        (*pu4BeaconInterval > DOT11_BEACON_PERIOD_MAX)) {
        DBGLOG(REQ, TRACE, ("Invalid Beacon Interval = %ld\n", *pu4BeaconInterval));
        return WLAN_STATUS_INVALID_DATA;
    }

    prAdapter->rConnSettings.u2BeaconPeriod = (UINT_16)*pu4BeaconInterval;

    DBGLOG(REQ, INFO, ("Set beacon interval: %d\n",
        prAdapter->rConnSettings.u2BeaconPeriod));


    if (prAdapter->rConnSettings.eOPMode != NET_TYPE_INFRA) {
        /* Send the "Abort" event to arbiter. */
        arbFsmRunEventAbort(prAdapter, TRUE); /* Reset to STANDBY */
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetBeaconInterval() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryAtimWindow (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQueryAtimWindow");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    DBGLOG(REQ, INFO, ("\n"));

    *pu4QueryInfoLen = sizeof(UINT_32);

    if (u4QueryBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_BUFFER_TOO_SHORT;
    }

    ASSERT(pvQueryBuffer);

    if (prAdapter->rConnSettings.eOPMode == NET_TYPE_IBSS) {

        if (prAdapter->eConnectionState== MEDIA_STATE_CONNECTED) {
            *(PUINT_32)pvQueryBuffer =
                (UINT_32)prAdapter->rBssInfo.u2ATIMWindow;
        }
        else {
            *(PUINT_32)pvQueryBuffer =
                (UINT_32)prAdapter->rConnSettings.u2AtimWindow;
        }
    }
    else {
        *(PUINT_32)pvQueryBuffer = 0;
    }

    DBGLOG(REQ, INFO, ("Current ATIM window: %ld\n",
                        *(PUINT_32)pvQueryBuffer));

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQueryAtimWindow() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetAtimWindow (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PUINT_32 pu4AtimWindow;

    DEBUGFUNC("wlanoidSetAtimWindow");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    ASSERT(pvSetBuffer);
    pu4AtimWindow = (PUINT_32)pvSetBuffer;

    prAdapter->rConnSettings.u2AtimWindow= (UINT_16)*pu4AtimWindow;

    DBGLOG(REQ, INFO, ("Set ATIM window: %d\n",
        prAdapter->rConnSettings.u2AtimWindow));


    if (prAdapter->rConnSettings.eOPMode != NET_TYPE_INFRA) {
        /* Send the "Abort" event to arbiter. */
        arbFsmRunEventAbort(prAdapter, TRUE); /* Reset to STANDBY */
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetAtimWindow() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetCurrentAddr (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PUINT_8 pucNewMacAddr;
    UINT_8 aucZeroMacAddr[MAC_ADDR_LEN] = {0,0,0,0,0,0};
    DEBUGFUNC("wlanoidSetCurrentAddrForLinux");


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = PARAM_MAC_ADDR_LEN;

    if (u4SetBufferLen < PARAM_MAC_ADDR_LEN) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    ASSERT(pvSetBuffer);
    pucNewMacAddr = (PUINT_8)pvSetBuffer;

    if (IS_BMCAST_MAC_ADDR(pucNewMacAddr)) {
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Accept the change request only when the NEW MAC Address is different from
     * current User Setting.
     */
    if (UNEQUAL_MAC_ADDR(prAdapter->rConnSettings.aucMacAddress, pucNewMacAddr)) {

        /* Update MAC Address in Connection Settings */
        COPY_MAC_ADDR(prAdapter->rConnSettings.aucMacAddress, pucNewMacAddr);

        /* If the input MAC Address is NULL, use the MAC Address from EEPROM. */
        if (EQUAL_MAC_ADDR(pucNewMacAddr, aucZeroMacAddr)) {

            COPY_MAC_ADDR(prAdapter->aucMacAddress, prAdapter->rEEPROMCtrl.aucMacAddress);
        }
        else {
            COPY_MAC_ADDR(prAdapter->aucMacAddress, pucNewMacAddr);
        }

        nicSetUnicastAddr(prAdapter);

        /* Do ABORT because we change our MAC Address. */
        arbFsmRunEventAbort(prAdapter, FALSE);
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidSetCurrentAddr() */


#if CFG_TCP_IP_CHKSUM_OFFLOAD
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetCSUMOffload (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    UINT_32 u4CSUMFlags;


    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set TCP/ IP checksum offload! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);

    u4CSUMFlags = *(PUINT_32)pvSetBuffer;

    nicSetCSUMOffload(prAdapter, u4CSUMFlags);

    return WLAN_STATUS_SUCCESS;

}
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */


#if CFG_LP_PATTERN_SEARCH_SLT
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQuerySltResult (
    IN P_ADAPTER_T  prAdapter,
    OUT  PVOID    pvQueryBuf,
    IN  UINT_32  u4QueryBufLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{
    DEBUGFUNC("wlanoidQuerySltResult");


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    if (prAdapter->eSLTModeSel == SLT_MODE_PATTERN_SEARCH) {
        *pu4QueryInfoLen = sizeof(UINT_32);
    }
    else {
        *pu4QueryInfoLen = sizeof(BEACON_RCPI_RECORED_T);
    }

    if (prAdapter->eConnectionStateIndicated != MEDIA_STATE_CONNECTED) {
        DBGLOG(REQ, TRACE, ("Query LP SLT result fail under disconnected state!\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }


    if (prAdapter->eSLTModeSel == SLT_MODE_PATTERN_SEARCH) {
        if (u4QueryBufLen < sizeof(UINT_32)) {
            return WLAN_STATUS_BUFFER_TOO_SHORT;
        }
        else {
            ASSERT(pvQueryBuf);
            kalMemCopy(pvQueryBuf, &prAdapter->fgPatternSearchMatch, sizeof(BOOLEAN));
        }
    }
    else {
        if (u4QueryBufLen < sizeof(BEACON_RCPI_RECORED_T)) {
            return WLAN_STATUS_BUFFER_TOO_SHORT;
        }
        else {
            ASSERT(pvQueryBuf);
            kalMemCopy(pvQueryBuf, &prAdapter->rBcnRCPIRecord, sizeof(BEACON_RCPI_RECORED_T));
        }
    }

    return WLAN_STATUS_SUCCESS;

} /* end of wlanoidQuerySltResult() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetSltMode (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
)
{
    P_ENUM_SLT_MODE_T peSltMode;
    PARAM_SSID_T rOriSsid;
    UINT_32 u4SetInfoLen;
    DEBUGFUNC("wlanoidSetSltMode");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (u4SetBufferLen < sizeof(ENUM_SLT_MODE_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }
    else {
        *pu4SetInfoLen = sizeof(ENUM_SLT_MODE_T);
    }

    peSltMode = (P_ENUM_SLT_MODE_T)pvSetBuffer;

    switch (*peSltMode) {
    case SLT_MODE_NORMAL:
    case SLT_MODE_LP:
    case SLT_MODE_PATTERN_SEARCH:
        prAdapter->fgPatternSearchMatch = FALSE;
        break;
    default:
        return WLAN_STATUS_INVALID_DATA;
    }

    prAdapter->eSLTModeSel = *peSltMode;
    /* Send the "Abort" event to arbiter. Always TRUE for delaying Disconnect
     * Indication.
     */

     rOriSsid.u4SsidLen = prAdapter->rConnSettings.ucSSIDLen;
     kalMemCopy(rOriSsid.aucSsid,
                prAdapter->rConnSettings.aucSSID,
                rOriSsid.u4SsidLen);


    wlanoidSetDisassociate(prAdapter,
                    NULL,
                    0,
                    &u4SetInfoLen);

    wlanoidSetSsid(prAdapter,
                    &rOriSsid,
                    sizeof(PARAM_SSID_T),
                    &u4SetInfoLen);

//    arbFsmRunEventAbort(prAdapter, FALSE); /* Reset to STANDBY */

//    arbFsmRunEventConnectionReq(prAdapter); /* Reconnected to AP */


    return WLAN_STATUS_SUCCESS;
} /* end of wlanSetSltMode() */

#endif /* CFG_LP_PATTERN_SEARCH_SLT */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetIpAddress (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PUINT_8 pucIpAddress;

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    pucIpAddress = (PUINT_8)pvSetBuffer;

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set IP Address! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    pmSetIpAddress(prAdapter, pucIpAddress);

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetWiFiWmmPsTest (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    P_PARAM_CUSTOM_WMM_PS_TEST_STRUC_T prWmmPsTestInfo;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    P_PM_INFO_T prPmInfo;
    P_PM_PROFILE_SETUP_INFO_T prPmProfSetupInfo;

    DEBUGFUNC("wlanoidSetWiFiWmmPsTest");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    prPmInfo = &prAdapter->rPmInfo;
    prPmProfSetupInfo = &prPmInfo->rPmProfSetupInfo;

    *pu4SetInfoLen = sizeof(P_PARAM_CUSTOM_WMM_PS_TEST_STRUC_T);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    prWmmPsTestInfo = (P_PARAM_CUSTOM_WMM_PS_TEST_STRUC_T) pvSetBuffer;

    prPmInfo->ucWmmPsEnterPsAtOnce  = prWmmPsTestInfo->ucIsEnterPsAtOnce;
    prPmInfo->ucWmmPsDisableUcPoll = prWmmPsTestInfo->ucIsDisableUcTrigger;

    /* it will apply the disable trig or not immediately */
    if (prPmInfo->ucWmmPsDisableUcPoll && prPmInfo->ucWmmPsConnWithTrig) {
        NIC_PM_WMM_PS_DISABLE_UC_TRIG(prAdapter, TRUE);
    }
    else {
        NIC_PM_WMM_PS_DISABLE_UC_TRIG(prAdapter, FALSE);
    }

    prPmProfSetupInfo->bmfgApsdEnAc = prWmmPsTestInfo->bmfgApsdEnAc;

    return rStatus;
}   /* wlanoidSetEepromWrite */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryCurrentCountry (
    IN P_ADAPTER_T prAdapter,
    IN PVOID pvQueryBuffer,
    IN UINT_32 u4QueryBufferLen,
    OUT PUINT_32 pu4QueryInfoLen
    )
{
    P_COUNTRY_STRING_ENTRY pCountryStr;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("domainQueryCurrentCountry");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prConnSettings = &prAdapter->rConnSettings;

    *pu4QueryInfoLen = sizeof(COUNTRY_STRING_ENTRY);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        WARNLOG(("Invalid length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    pCountryStr = (P_COUNTRY_STRING_ENTRY)pvQueryBuffer;

    pCountryStr->aucCountryCode[0] = (UINT_8)(prConnSettings->u2CountryCode >> 8);
    pCountryStr->aucCountryCode[1] =
        (UINT_8)(prConnSettings->u2CountryCode & 0x00FF);

    pCountryStr->aucEnvironmentCode[0] = ' ';
    pCountryStr->aucEnvironmentCode[0] = '\0';

#if DBG
    if (pCountryStr->aucCountryCode[0] >= 0x20 &&
        pCountryStr->aucCountryCode[1] >= 0x20) {
        DBGLOG(REQ,
            INFO,
            ("Current country: %c%c\n",
            pCountryStr->aucCountryCode[0],
            pCountryStr->aucCountryCode[1]));
    }
    else {
        DBGLOG(REQ,
            INFO,
            ("Current country: 0x%02x%02x\n",
            pCountryStr->aucCountryCode[0],
            pCountryStr->aucCountryCode[1]));
    }
#endif

    return WLAN_STATUS_SUCCESS;
} /* domainQueryCurrentCountry */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetCurrentCountry (
    IN P_ADAPTER_T prAdapter,
    IN PVOID pvSetBuffer,
    IN UINT_32 u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_COUNTRY_STRING_ENTRY pCountryStr;
    UINT_16 countryCode;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("domainSetCurrentCountry");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);
    prConnSettings = &prAdapter->rConnSettings;

    if (u4SetBufferLen < 2) {
        WARNLOG(("Invalid length %ld\n", u4SetBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    *pu4SetInfoLen = sizeof(COUNTRY_STRING_ENTRY);

    ASSERT(pvSetBuffer);
    pCountryStr = (P_COUNTRY_STRING_ENTRY)pvSetBuffer;

    countryCode = ((UINT_16)pCountryStr->aucCountryCode[0] << 8) |
                   (UINT_16)pCountryStr->aucCountryCode[1];

    DBGLOG(REQ,
        INFO,
        ("New regulatory domain: %c%c\n",
        pCountryStr->aucCountryCode[0],
        pCountryStr->aucCountryCode[1]));

#if CFG_SUPPORT_802_11D
    if (prConnSettings->fgMultiDomainCapabilityEnabled) {
        prConnSettings->u2CountryCode = countryCode;
        /* Not to apply country setting!! */
    }
    else {
        prConnSettings->u2CountryCode = countryCode;
        nicSetupOpChnlList(prAdapter, prConnSettings->u2CountryCode, FALSE);
    }
#else
    prConnSettings->u2CountryCode = countryCode;
    nicSetupOpChnlList(prAdapter, prConnSettings->u2CountryCode, FALSE);
#endif

    return WLAN_STATUS_SUCCESS;
} /* domainSetCurrentCountry */


#if CFG_SUPPORT_802_11D
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryMultiDomainCap (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("domainQueryMultiDomainCap");

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);

    prConnSettings = &prAdapter->rConnSettings;

    *pu4QueryInfoLen = sizeof(PARAM_MULTI_DOMAIN_CAPABILITY);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        WARNLOG(("Invalid length %ld\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    if (FALSE != prConnSettings->fgMultiDomainCapabilityEnabled) {
        DBGLOG(REQ, INFO, ("Current multi-domain cap: Enabled\n"));
        *(P_PARAM_MULTI_DOMAIN_CAPABILITY)pvQueryBuffer =
            ParamMultiDomainCapEnabled;
    }
    else {
        DBGLOG(REQ, INFO, ("Current multi-domain cap: Disabled\n"));
        *(P_PARAM_MULTI_DOMAIN_CAPABILITY)pvQueryBuffer =
            ParamMultiDomainCapDisabled;
    }

    return WLAN_STATUS_SUCCESS;
} /* domainQueryMultiDomainCap */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetMultiDomainCap (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PARAM_MULTI_DOMAIN_CAPABILITY paramMultiDomain;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("domainSetMultiDomainCap");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    prConnSettings = &prAdapter->rConnSettings;

    *pu4SetInfoLen = sizeof(PARAM_MULTI_DOMAIN_CAPABILITY);

    /* Check for set buffer length */
    if (u4SetBufferLen < *pu4SetInfoLen) {
        WARNLOG(("Invalid length %ld\n", u4SetBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);
    paramMultiDomain = *(P_PARAM_MULTI_DOMAIN_CAPABILITY)pvSetBuffer;

    if (ParamMultiDomainCapEnabled == paramMultiDomain) {
        if (FALSE != prConnSettings->fgMultiDomainCapabilityEnabled) {
            /* The same as current setting, return SUCCESS. */
            return WLAN_STATUS_SUCCESS;
        }
    }
    else if (ParamMultiDomainCapDisabled == paramMultiDomain){
        if (FALSE == prConnSettings->fgMultiDomainCapabilityEnabled) {
            /* The same as current setting, return SUCCESS. */
            return WLAN_STATUS_SUCCESS;
        }
    }
    else {
        DBGLOG(REQ,
            TRACE,
            ("Invalid multi-domain cap: %ld\n",
            *(PUINT_32)pvSetBuffer));

        return WLAN_STATUS_INVALID_DATA;
    }

    if (ParamMultiDomainCapEnabled == paramMultiDomain) {
        DBGLOG(REQ, INFO, ("New multi-domain cap: Enabled\n"));

        prConnSettings->fgMultiDomainCapabilityEnabled = TRUE;
    }
    else {
        ASSERT(ParamMultiDomainCapDisabled == paramMultiDomain);
        DBGLOG(REQ, INFO, ("New multi-domain cap: Disabled\n"));

        prConnSettings->fgMultiDomainCapabilityEnabled = FALSE;
    }

    return WLAN_STATUS_SUCCESS;
} /* domainSetMultiDomainCap */
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetGPIO2Mode (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_CONNECTION_SETTINGS_T     prConnSettings;

    DEBUGFUNC("wlanoidSetGPIO2Mode");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    prConnSettings = &prAdapter->rConnSettings;

    *pu4SetInfoLen = sizeof(ENUM_PARAM_GPIO2_MODE_T);

    if (u4SetBufferLen != sizeof(ENUM_PARAM_GPIO2_MODE_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvSetBuffer);
    if (*(P_ENUM_PARAM_GPIO2_MODE_T)pvSetBuffer > GPIO2_MODE_DAISY_INPUT) {
        return WLAN_STATUS_INVALID_DATA;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set GPIO2 setting because of ACPI_D3\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    /* Store parameters to connSetting for chip reset path */
    prConnSettings->eGPIO2_Mode = *(P_ENUM_PARAM_GPIO2_MODE_T) pvSetBuffer;
    nicSetGPIO2Mode(prAdapter, prConnSettings->eGPIO2_Mode);

    return WLAN_STATUS_SUCCESS;

} /* wlanoidSetGPIO2Mode */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetContinuousPollProfile (
    IN  P_ADAPTER_T  prAdapter,
    IN  PVOID    pvSetBuffer,
    IN  UINT_32  u4SetBufferLen,
    OUT PUINT_32 pu4SetInfoLen
    )
{
    P_CONNECTION_SETTINGS_T     prConnSettings;

    DEBUGFUNC("wlanoidSetContinuousPollProfile");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    prConnSettings = &prAdapter->rConnSettings;

    *pu4SetInfoLen = sizeof(PARAM_CONTINUOUS_POLL_T);

    if (u4SetBufferLen != sizeof(PARAM_CONTINUOUS_POLL_T)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set continuous poll because of ACPI_D3\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    /* Store parameters to connSetting */
    prConnSettings->u4ContPollIntv = *(PUINT_32) pvSetBuffer;

    if (prConnSettings->u4ContPollIntv) {
        pmEnableContinuousPollingFunc(prAdapter);
    }
    else {
        pmDisableContinuousPollingFunc(prAdapter);
    }

    return WLAN_STATUS_SUCCESS;

} /* wlanoidSetContinuousPollProfile */

#if defined(LINUX)
VOID
wlanoidQueryDrvStatusForLinuxProc (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    UINT_32 u4TotalCount = 0;
    UINT_32 u4Count;

    ASSERT(prAdapter);

    nicTxQueryStatus(prAdapter, pucBuffer, &u4Count);
    u4TotalCount += u4Count;

    pucBuffer += u4Count;
    nicRxQueryStatus(prAdapter, pucBuffer, &u4Count);
    u4TotalCount += u4Count;

#if CFG_TX_DBG_MGT_BUF
    pucBuffer += u4Count;
    mgtBufQueryStatus(prAdapter, pucBuffer, &u4Count);
    u4TotalCount += u4Count;
#endif /* CFG_TX_DBG_MGT_BUF */

#if CFG_DBG_STA_RECORD
    pucBuffer += u4Count;
    staRecQueryStatus(prAdapter, pucBuffer, &u4Count);
    u4TotalCount += u4Count;
#endif /* CFG_DBG_STA_RECORD */

    *pu4Count = u4TotalCount;

    return;
}

VOID
wlanoidQueryRxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    UINT_32 u4Count;

    ASSERT(prAdapter);

    nicRxQueryStatistics(prAdapter, pucBuffer, &u4Count);

    *pu4Count = u4Count;

    return;
}


VOID
wlanoidSetRxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter
    )
{

    ASSERT(prAdapter);

    nicRxSetStatistics(prAdapter);

    return;
}


VOID
wlanoidQueryTxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    )
{
    UINT_32 u4Count;

    ASSERT(prAdapter);

    nicTxQueryStatistics(prAdapter, pucBuffer, &u4Count);

    *pu4Count = u4Count;

    return;
}


VOID
wlanoidSetTxStatisticsForLinuxProc (
    IN P_ADAPTER_T prAdapter
    )
{

    ASSERT(prAdapter);
    nicTxSetStatistics(prAdapter);

    return;
}

#endif
#if PTA_ENABLED
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetBT (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{

    P_PTA_IPC_T   prPtaIpc;


    DEBUGFUNC("wlanoidSetBT");

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PTA_IPC_T);
    if (u4SetBufferLen != sizeof(PTA_IPC_T)) {
        WARNLOG(("Invalid length %ld\n", u4SetBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set BT profile because of ACPI_D3\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    prPtaIpc = (P_PTA_IPC_T)pvSetBuffer;

    switch (prPtaIpc->ucCmd) {
    case BT_CMD_PROFILE:
        nicPTASetProfile(prAdapter, &prPtaIpc->u.rProfile);
        break;

    case BT_CMD_UPDATE:
        nicPTAUpdateParams(prAdapter, prPtaIpc->u.aucBTPParams);
        break;

    default:
        ASSERT(0);
        break;
    }

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryBT (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_PARAM_PTA_IPC_T prPtaIpc;
    UINT_32 u4QueryBuffLen;

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(PARAM_PTA_IPC_T);

    /* Check for query buffer length */
    if (u4QueryBufferLen != sizeof(PARAM_PTA_IPC_T)) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    prPtaIpc = (P_PARAM_PTA_IPC_T)pvQueryBuffer;
    prPtaIpc->ucCmd = BT_CMD_PROFILE;
    prPtaIpc->ucLen = sizeof(prPtaIpc->u);
    nicPtaGetProfile(prAdapter, (PUINT_8)&prPtaIpc->u, &u4QueryBuffLen);

    return WLAN_STATUS_SUCCESS;
}


WLAN_STATUS
wlanoidQueryBtSingleAntenna (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_PTA_INFO_T prPtaInfo;
    PUINT_32 pu4SingleAntenna;

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(UINT_32);

    /* Check for query buffer length */
    if (u4QueryBufferLen != sizeof(UINT_32)) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    prPtaInfo = &prAdapter->rPtaInfo;
    pu4SingleAntenna = (PUINT_32)pvQueryBuffer;

    if(prPtaInfo->fgSingleAntenna) {
        //printk(KERN_WARNING DRV_NAME"Q Single Ant = 1\r\n");
        *pu4SingleAntenna = 1;
    } else {
        //printk(KERN_WARNING DRV_NAME"Q Single Ant = 0\r\n");
        *pu4SingleAntenna = 0;
    }

    return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanoidQueryPta (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_PTA_INFO_T prPtaInfo;
    PUINT_32 pu4Pta;

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(UINT_32);

    /* Check for query buffer length */
    if (u4QueryBufferLen != sizeof(UINT_32)) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    prPtaInfo = &prAdapter->rPtaInfo;
    pu4Pta = (PUINT_32)pvQueryBuffer;

    if(prPtaInfo->fgEnabled) {
        //printk(KERN_WARNING DRV_NAME"PTA = 1\r\n");
        *pu4Pta = 1;
    } else {
        //printk(KERN_WARNING DRV_NAME"PTA = 0\r\n");
        *pu4Pta = 0;
    }

    return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanoidSetBtSingleAntenna (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{

    PUINT_32        pu4SingleAntenna;
    UINT_32         u4SingleAntenna;
    P_PTA_INFO_T    prPtaInfo;

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    prPtaInfo = &prAdapter->rPtaInfo;

    *pu4SetInfoLen = sizeof(UINT_32);
    if (u4SetBufferLen != sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set antenna because of ACPI_D3\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    pu4SingleAntenna = (PUINT_32)pvSetBuffer;
    u4SingleAntenna = *pu4SingleAntenna;

    if (u4SingleAntenna == 0) {
        //printk(KERN_WARNING DRV_NAME"Set Single Ant = 0\r\n");
        prPtaInfo->fgSingleAntenna = FALSE;
    } else {
        //printk(KERN_WARNING DRV_NAME"Set Single Ant = 1\r\n");
        prPtaInfo->fgSingleAntenna = TRUE;
    }
    ptaFsmRunEventSetConfig(prAdapter, &prPtaInfo->rPtaParam);

    return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS
wlanoidSetPta (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PUINT_32    pu4PtaCtrl;
    UINT_32     u4PtaCtrl;

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);
    if (u4SetBufferLen != sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set BT setting because of ACPI_D3\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    pu4PtaCtrl = (PUINT_32)pvSetBuffer;
    u4PtaCtrl = *pu4PtaCtrl;

    if (u4PtaCtrl == 0) {
        //printk(KERN_WARNING DRV_NAME"Set Pta= 0\r\n");
        nicPtaSetFunc(prAdapter, FALSE);
    } else {
        //printk(KERN_WARNING DRV_NAME"Set Pta= 1\r\n");
        nicPtaSetFunc(prAdapter, TRUE);
    }

    return WLAN_STATUS_SUCCESS;
}
/* Renbang : ++ (20100319)*/
#if 1
WLAN_STATUS
wlanoidSetFixedRxGain (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PUINT_32    pu4RxFixedGain;
    UINT_32     u4RxFixedGain;

    ASSERT(prAdapter);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(UINT_32);
    if (u4SetBufferLen != sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail to set BT setting because of ACPI_D3\r\n"));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    ASSERT(pvSetBuffer);
    pu4RxFixedGain = (PUINT_32)pvSetBuffer;
    u4RxFixedGain = *pu4RxFixedGain;

    if (u4RxFixedGain == 0) {
        printk(KERN_WARNING DRV_NAME"[BWCS]Disable RX fixed gain\r\n");
        halBBEnableFixedRxGain(prAdapter, FALSE);
    } else {
        printk(KERN_WARNING DRV_NAME"[BWCS]Enable RX fixed gain\r\n");
        halBBEnableFixedRxGain(prAdapter, TRUE);
    }

    return WLAN_STATUS_SUCCESS;
}
#endif
#endif
/* Renbang : -- (20100319)*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryContinuousPollInterval (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_CONNECTION_SETTINGS_T prConnSetting;
    PUINT_32 pu4ContPollIntv;


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(UINT_32);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    prConnSetting = &prAdapter->rConnSettings;
    pu4ContPollIntv = (PUINT_32)pvQueryBuffer;


    *pu4ContPollIntv = prConnSetting->u4ContPollIntv;
    *pu4QueryInfoLen = sizeof(UINT_32);
    return WLAN_STATUS_SUCCESS;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryDisableBeaconDetectionFunc (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    P_PM_INFO_T                 prPmInfo;
    PUINT_32                    pu4IsBeaconTimeoutDetectionDisabled;

    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(UINT_32);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);

    prPmInfo = &prAdapter->rPmInfo;
    pu4IsBeaconTimeoutDetectionDisabled = (PUINT_32)pvQueryBuffer;

    *pu4IsBeaconTimeoutDetectionDisabled = (UINT_32)prPmInfo->fgIsBeaconTimeoutDetectionDisabled;

    return WLAN_STATUS_SUCCESS;
} /* wlanoidQueryBeaconDetectionFunc */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetDisableBeaconDetectionFunc (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    PBOOLEAN pfgDisableBeaconDection;
    P_CONNECTION_SETTINGS_T prConnectionSetting;

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    pfgDisableBeaconDection = (PBOOLEAN)pvSetBuffer;
    prConnectionSetting = &prAdapter->rConnSettings;

    if (u4SetBufferLen < sizeof(UINT_32)) {
        return WLAN_STATUS_INVALID_LENGTH;
    }

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set beacon timeout detection! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    if (*pfgDisableBeaconDection == TRUE) {
        /* Enter PSP-MAX mode */
        pmFsmRunEventSetPowerSaveProfile(prAdapter, ENUM_PSP_CONTINUOUS_POWER_SAVE);

        /* Control the function of beacon timeout detection */
        pmDisableBeaconTimeoutDetectionFunc(prAdapter, TRUE);
    } else {
        /* Restore to original PSP mode */
        pmFsmRunEventSetPowerSaveProfile(prAdapter,
                                         aPowerModeToPwrSaveProfMapping[prConnectionSetting->rPwrMode]);

        /* Control the function of beacon timeout detection */
        pmDisableBeaconTimeoutDetectionFunc(prAdapter, FALSE);
    }

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetBeaconDetectionFunc */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetDisablePriavcyCheck (
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    )
{
    UINT_32           u4Value;

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    u4Value = *(PUINT_32)pvSetBuffer;

    if (u4Value >= 1)
        prAdapter->rSecInfo.fgPrivacyCheckDisable = TRUE;
    else
        prAdapter->rSecInfo.fgPrivacyCheckDisable = FALSE;

    return WLAN_STATUS_SUCCESS;
}    
/* Renbang : ++ (20100319)*/
#if CFG_SUPPORT_EXT_CONFIG
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryNvramRead (
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{

    P_PARAM_CUSTOM_EEPROM_RW_STRUC_T prEepromRdInfo;
    UINT_16     u2Data;
    BOOLEAN     fgStatus;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryEepromRead");

    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    prEepromRdInfo = (P_PARAM_CUSTOM_EEPROM_RW_STRUC_T) pvQueryBuffer;

	if( prAdapter->prGlueInfo->eCfgSrcType != CFG_SRC_TYPE_NVRAM )
	{
        DBGLOG(REQ, ERROR, ("NVRAM is not current cfg src!!\r\n"));
		return WLAN_STATUS_FAILURE;
	}

	if(prEepromRdInfo->ucEepromMethod == PARAM_EEPROM_READ_METHOD_READ) {
        fgStatus = cfgDataRead16(prAdapter,
                            prEepromRdInfo->ucEepromIndex,
                            &u2Data
                            );
        if(fgStatus){
            prEepromRdInfo->u2EepromData = u2Data;
            DBGLOG(REQ, INFO, ("NVRAM Read: index=%#X, data=%#02X\r\n",
                    prEepromRdInfo->ucEepromIndex, u2Data));
        }
        else{
            DBGLOG(REQ, ERROR, ("NVRAM Read Failed: index=%#x.\r\n",
                    prEepromRdInfo->ucEepromIndex));
            rStatus = WLAN_STATUS_FAILURE;
        }
    }
    else if (prEepromRdInfo->ucEepromMethod == PARAM_EEPROM_READ_METHOD_GETSIZE) {
        prEepromRdInfo->u2EepromData = (UINT_16)(prAdapter->prGlueInfo->u4ExtCfgLength);
        DBGLOG(REQ, INFO, ("EEPROM size =%d\r\n", prEepromRdInfo->u2EepromData));
        DBGLOG(REQ, INFO, ("EEPROM size =%d\r\n", prAdapter->prGlueInfo->u4ExtCfgLength));
    }

    *pu4QueryInfoLen = sizeof(PARAM_CUSTOM_EEPROM_RW_STRUC_T);


    return rStatus;
}   /* wlanoidQueryNvramRead */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetNvramWrite (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    P_PARAM_CUSTOM_EEPROM_RW_STRUC_T prEepromWrInfo;
    BOOLEAN     fgStatus;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidSetEepromWrite");
    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = sizeof(PARAM_CUSTOM_EEPROM_RW_STRUC_T);

	if( prAdapter->prGlueInfo->eCfgSrcType != CFG_SRC_TYPE_NVRAM )
	{
        DBGLOG(REQ, ERROR, ("NVRAM is not current cfg src!!\r\n"));
		return WLAN_STATUS_FAILURE;
	}

    prEepromWrInfo = (P_PARAM_CUSTOM_EEPROM_RW_STRUC_T) pvSetBuffer;

    DBGLOG(REQ, INFO, ("NVRAM Write: index=0x%x, data=0x%x\r\n",
        prEepromWrInfo->ucEepromIndex, prEepromWrInfo->u2EepromData));

    fgStatus = cfgDataWrite16(prAdapter,
                                prEepromWrInfo->ucEepromIndex,
                                prEepromWrInfo->u2EepromData
                                );
    if(fgStatus == FALSE){
        DBGLOG(REQ, ERROR, ("NVRAM Write Failed.\r\n"));
        rStatus = WLAN_STATUS_FAILURE;
    }
    return rStatus;
}   /* wlanoidSetEepromWrite */

#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryCfgSrcType(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
  )
{
	  ASSERT( prAdapter );

#if CFG_SUPPORT_EXT_CONFIG        
    *( P_ENUM_CFG_SRC_TYPE_T )pvQueryBuffer = prAdapter->prGlueInfo->eCfgSrcType;
#else 
    *( P_ENUM_CFG_SRC_TYPE_T )pvQueryBuffer = CFG_SRC_TYPE_EEPROM;
#endif  
    
    *pu4QueryInfoLen = sizeof(ENUM_CFG_SRC_TYPE_T);

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryEepromType(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
  )
{
	  ASSERT( prAdapter );

    if ( nicEepromGetSize( prAdapter ) )
    {
        DBGLOG(REQ, INFO, ("EEPROM : present!\r\n"));
        *( P_ENUM_EEPROM_TYPE_T) pvQueryBuffer = EEPROM_TYPE_PRESENT;       
    }
    else
    {
        DBGLOG(REQ, INFO, ("EEPROM : not present!\r\n"));
        *( P_ENUM_EEPROM_TYPE_T )pvQueryBuffer = EEPROM_TYPE_NO;
    
    }    
    
    *pu4QueryInfoLen = sizeof(P_ENUM_EEPROM_TYPE_T);

    return WLAN_STATUS_SUCCESS;
}
/* Renbang : -- (20100319)*/

#if SUPPORT_WAPI
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetWapiMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    DEBUGFUNC("wlanoidSetWapiMode");
    DBGLOG(WAPI, TRACE, ("\r\n"));

    if (*(PUINT_32)pvSetBuffer) {
        prAdapter->fgWapiMode = TRUE;
        prAdapter->rSecInfo.fgBlockTxTraffic = TRUE;
        prAdapter->rSecInfo.fgBlockRxTraffic = TRUE;
    }
    else {
        prAdapter->fgWapiMode = FALSE;
        prAdapter->rSecInfo.fgBlockTxTraffic = FALSE;
        prAdapter->rSecInfo.fgBlockRxTraffic = FALSE;
    }
    return WLAN_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetWapiAssocInfo (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_WAPI_INFO_ELEM_T    prWapiInfo;
    PUINT_8               cp;
    UINT_16               u2AuthSuiteCount = 0;
    UINT_16               u2PairSuiteCount = 0;
    UINT_32               u4AuthKeyMgtSuite = 0;
    UINT_32               u4PairSuite = 0;
    UINT_32               u4GroupSuite = 0;
    
    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    DEBUGFUNC("wlanoidSetWapiAssocInfo");
    DBGLOG(WAPI, TRACE, ("\r\n"));

    prWapiInfo = (P_WAPI_INFO_ELEM_T)pvSetBuffer;
    if (u4SetBufferLen < 20 /* From EID to Group cipher */)
        return WLAN_STATUS_INVALID_LENGTH;
    
    if (prWapiInfo->ucElemId != ELEM_ID_WAPI)
        DBGLOG(WAPI, WARN, ("Not WAPI IE ?!\n"));

    if (prWapiInfo->ucLength < 18)
        return WLAN_STATUS_INVALID_LENGTH;
        
    /* Skip Version check */
    cp = (PUINT_8)&prWapiInfo->u2AuthKeyMgtSuiteCount;
    
    WLAN_GET_FIELD_16(cp, &u2AuthSuiteCount);

    if (u2AuthSuiteCount>1)
        return WLAN_STATUS_INVALID_LENGTH;

    cp += 2;
    WLAN_GET_FIELD_32(cp, &u4AuthKeyMgtSuite);

    DBGLOG(WAPI, TRACE,
        ("WAPI: Assoc Info auth mgt suite [%d]: %02x-%02x-%02x-%02x\n",
        u2AuthSuiteCount,
        (UCHAR) (u4AuthKeyMgtSuite & 0x000000FF),
        (UCHAR) ((u4AuthKeyMgtSuite >> 8) & 0x000000FF),
        (UCHAR) ((u4AuthKeyMgtSuite >> 16) & 0x000000FF),
        (UCHAR) ((u4AuthKeyMgtSuite >> 24) & 0x000000FF)));

    if (u4AuthKeyMgtSuite != WAPI_AKM_SUITE_802_1X &&
        u4AuthKeyMgtSuite != WAPI_AKM_SUITE_PSK)
        ASSERT(FALSE);
    
    cp += 4;
    WLAN_GET_FIELD_16(cp, &u2PairSuiteCount);
    if (u2PairSuiteCount>1)
        return WLAN_STATUS_INVALID_LENGTH;
            
    cp += 2;
    WLAN_GET_FIELD_32(cp, &u4PairSuite);
    DBGLOG(WAPI, TRACE,
        ("WAPI: Assoc Info pairwise cipher suite [%d]: %02x-%02x-%02x-%02x\n",
        u2PairSuiteCount,
        (UCHAR) (u4PairSuite & 0x000000FF),
        (UCHAR) ((u4PairSuite >> 8) & 0x000000FF),
        (UCHAR) ((u4PairSuite >> 16) & 0x000000FF),
        (UCHAR) ((u4PairSuite >> 24) & 0x000000FF)));

    if (u4PairSuite != WAPI_CIPHER_SUITE_WPI)
        ASSERT(FALSE);

    cp += 4;
    WLAN_GET_FIELD_32(cp, &u4GroupSuite);
    DBGLOG(WAPI, TRACE,
        ("WAPI: Assoc Info group cipher suite : %02x-%02x-%02x-%02x\n",
        (UCHAR) (u4GroupSuite & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 8) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 16) & 0x000000FF),
        (UCHAR) ((u4GroupSuite >> 24) & 0x000000FF)));

    if (u4GroupSuite != WAPI_CIPHER_SUITE_WPI)
        ASSERT(FALSE);
    
    prAdapter->rConnSettings.u4WapiSelectedAKMSuite = u4AuthKeyMgtSuite;
    prAdapter->rConnSettings.u4WapiSelectedPairwiseCipher = u4PairSuite;
    prAdapter->rConnSettings.u4WapiSelectedGroupCipher = u4GroupSuite;
    
    kalMemCopy(prAdapter->prGlueInfo->aucWapiAssocInfoIEs,  pvSetBuffer, u4SetBufferLen);
    prAdapter->prGlueInfo->u2WapiAssocInfoIESz = (UINT_16)u4SetBufferLen;
    DBGLOG(WAPI, TRACE, ("Assoc Info IE sz %ld\n", u4SetBufferLen));

    return WLAN_STATUS_SUCCESS;

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetWapiKey (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_WPI_KEY_T prNewKey;
    P_IEEE_802_11_MIB_T prMib;
    PUINT_8             pc;
    DEBUGFUNC("wlanoidSetWapiKey");
    DBGLOG(WAPI, TRACE, ("\r\n"));

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    prMib = &prAdapter->rConnSettings.rMib;

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_SUCCESS;
    }
    prNewKey = (P_PARAM_WPI_KEY_T) pvSetBuffer;

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(WAPI, WARN, ("Fail in set add key! (Adapter not ready). ACPI=D%d, Radio=%d\r\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)pvSetBuffer, 560);
    pc = (PUINT_8)pvSetBuffer;

    /* Verify the key structure length. */
    //if (sizeof(PARAM_WPI_KEY_T)!= u4SetBufferLen) {
        //DBGLOG(WAPI, ERROR, ("Invalid key structure length (%d) not equal the total buffer length (%d)\r\n",
        //                  (UINT_16)sizeof(PARAM_WPI_KEY_T),
        //                  (UINT_16)u4SetBufferLen));

    //   *pu4SetInfoLen = u4SetBufferLen;
    //    return WLAN_STATUS_INVALID_LENGTH;
    //}

    /* Exception check */
    if (prNewKey->ucKeyID != 0x1 ||
        prNewKey->ucKeyID != 0x0) {
        DBGLOG(WAPI, ERROR, ("Invalid key ID (%d)\r\n", prNewKey->ucKeyID));
        prNewKey->ucKeyID = prNewKey->ucKeyID & BIT(0);
        //return WLAN_STATUS_INVALID_DATA;
    }

#if 0
    /* Verify the key material length for key material buffer */
    if (prNewKey->u4KeyLength > prNewKey->u4Length - OFFSET_OF(PARAM_KEY_T, aucKeyMaterial)) {
        DBGLOG(RSN, WARN, ("Invalid key material length (%d)\n", (UINT_8)prNewKey->u4KeyLength));
        *pu4SetInfoLen = u4SetBufferLen;
        return WLAN_STATUS_INVALID_DATA;
    }

    /* Exception check */
    if (prNewKey->u4KeyIndex & 0x0fffff00) {
        return WLAN_STATUS_INVALID_DATA;
    }

    if (!(prNewKey->u4KeyLength == WEP_40_LEN  || prNewKey->u4KeyLength == WEP_104_LEN ||
          prNewKey->u4KeyLength == CCMP_KEY_LEN|| prNewKey->u4KeyLength == TKIP_KEY_LEN))
    {
        return WLAN_STATUS_INVALID_DATA;
    }
#endif

    *pu4SetInfoLen = u4SetBufferLen;

    /* Dump P_PARAM_WPI_KEY_T content. */
    DBGLOG(WAPI, TRACE, ("Set: Dump P_PARAM_WPI_KEY_T content\r\n"));
    DBGLOG(WAPI, TRACE, ("TYPE      : %d\r\n", prNewKey->eKeyType));
    DBGLOG(WAPI, TRACE, ("Direction : %d\r\n", prNewKey->eDirection));
    DBGLOG(WAPI, TRACE, ("KeyID     : %d\r\n", prNewKey->ucKeyID));
    DBGLOG(WAPI, TRACE, ("AddressIndex:\r\n"));
    DBGLOG_MEM8(WAPI, TRACE, prNewKey->aucAddrIndex, 12);
    prNewKey->u4LenWPIEK = 16;

    SMS4KeyExt(prNewKey->aucWPIEK,  (PUINT_32)prNewKey->aucWPIEK, 0);
    //kalMemCopy((PUINT_8)prNewKey->aucWPIEK, (PUINT_8)&pc[25], prNewKey->u4LenWPIEK);
    DBGLOG(WAPI, TRACE, ("EN Key(%d): %x %x %x %x\r\n", (UINT_8)prNewKey->u4LenWPIEK,
        pc[25],pc[26],pc[27],pc[28]));
    DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)prNewKey->aucWPIEK, (UINT_8)prNewKey->u4LenWPIEK);
    prNewKey->u4LenWPICK = 16;

    SMS4KeyExt(prNewKey->aucWPICK,  (PUINT_32)prNewKey->aucWPICK, 0);

    //kalMemCopy((PUINT_8)prNewKey->aucWPICK, (PUINT_8)&pc[25+256+4], prNewKey->u4LenWPICK);
    DBGLOG(WAPI, TRACE, ("CK Key(%d):\r\n", (UINT_8)prNewKey->u4LenWPICK));
    DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)prNewKey->aucWPICK, (UINT_8)prNewKey->u4LenWPICK);
    DBGLOG(WAPI, TRACE, ("PN:\r\n"));
    if (prNewKey->eKeyType == 0){
            prNewKey->aucPN[0] = 0x5c;
            prNewKey->aucPN[1] = 0x36;
            prNewKey->aucPN[2] = 0x5c;
            prNewKey->aucPN[3] = 0x36;
            prNewKey->aucPN[4] = 0x5c;
            prNewKey->aucPN[5] = 0x36;
            prNewKey->aucPN[6] = 0x5c;
            prNewKey->aucPN[7] = 0x36;
            prNewKey->aucPN[8] = 0x5c;
            prNewKey->aucPN[9] = 0x36;
            prNewKey->aucPN[10] = 0x5c;
            prNewKey->aucPN[11] = 0x36;
            prNewKey->aucPN[12] = 0x5c;
            prNewKey->aucPN[13] = 0x36;
            prNewKey->aucPN[14] = 0x5c;
            prNewKey->aucPN[15] = 0x36;
    }

    DBGLOG_MEM8(WAPI, TRACE, (PUINT_8)prNewKey->aucPN, PN_LEN);


    if (prNewKey->eKeyType == ENUM_WPI_PAIRWISE_KEY) {
        kalMemCopy(&prMib->rWapiPairwiseKey[prNewKey->ucKeyID], prNewKey, sizeof(PARAM_WPI_KEY_T));
        prMib->fgPairwiseKeyUsed[prNewKey->ucKeyID] = TRUE;
        prMib->ucWpiActivedPWKey = prNewKey->ucKeyID;
    }
    else if (prNewKey->eKeyType == ENUM_WPI_GROUP_KEY) {
        prAdapter->rConnSettings.rMib.fgWapiKeyInstalled = TRUE;
        kalMemCopy(&prMib->rWapiGroupKey[prNewKey->ucKeyID], prNewKey, sizeof(PARAM_WPI_KEY_T));
        prMib->fgGroupKeyUsed[prNewKey->ucKeyID] = TRUE;
        //prMib->fgGroupKeyEnabled[prNewKey->ucKeyID] = TRUE;
        prAdapter->rSecInfo.fgBlockTxTraffic = FALSE;
        prAdapter->rSecInfo.fgBlockRxTraffic = FALSE;
     }
    else
        ASSERT(FALSE);

    return WLAN_STATUS_SUCCESS;
} /* wlanoidSetAddKey */
#endif
#if PTA_NEW_BOARD_DESIGN
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetPreferredAnt (
    IN  P_ADAPTER_T   prAdapter,
    IN  PVOID         pvSetBuffer,
    IN  UINT_32       u4SetBufferLen,
    OUT PUINT_32      pu4SetInfoLen
    )
{
    UINT_32           u4Value;

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    u4Value = *(PUINT_32)pvSetBuffer;

    if(prAdapter->rPtaInfo.fgSingleAntenna == FALSE){
        /* for dual antenna board there is no need to setup preferred antenna path */
        return WLAN_STATUS_SUCCESS;
    }

    switch(u4Value){
        case 0:/* WiFi */
            printk(KERN_WARNING DRV_NAME"%s %d\n", __FUNCTION__, __LINE__);
            nicPtaSetAnt(prAdapter, TRUE);
            break;
        case 1:/* BT */
            printk(KERN_WARNING DRV_NAME"%s %d\n", __FUNCTION__, __LINE__);
            nicPtaSetAnt(prAdapter, FALSE);
            break;
        default:
            printk(KERN_WARNING DRV_NAME"Unexpected antenna %ld\n", u4Value);
            return WLAN_STATUS_INVALID_DATA;
            break;
    }
    return WLAN_STATUS_SUCCESS;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryPreferredAnt (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    )
{
    PUINT_32 pu4PrefAnt;
    BOOL    fgPreferWiFi;


    ASSERT(prAdapter);
    ASSERT(pu4QueryInfoLen);
    if (u4QueryBufferLen) {
        ASSERT(pvQueryBuffer);
    }

    *pu4QueryInfoLen = sizeof(UINT_32);

    /* Check for query buffer length */
    if (u4QueryBufferLen < *pu4QueryInfoLen) {
        DBGLOG(REQ, WARN, ("Invalid length %lu\n", u4QueryBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    ASSERT(pvQueryBuffer);
    
    pu4PrefAnt = (PUINT_32)pvQueryBuffer;
    
    if(prAdapter->rPtaInfo.fgSingleAntenna == FALSE){
        /* for dual antenna board there is preferred antenna path */
        *pu4PrefAnt = 0;/* WiFi */
        *pu4QueryInfoLen = sizeof(UINT_32);
        return WLAN_STATUS_SUCCESS;
    }

    nicPtaGetAnt(prAdapter, &fgPreferWiFi);

    if(fgPreferWiFi){
        *pu4PrefAnt = 0;/* WiFi */
    }else{
        *pu4PrefAnt = 1;/* BT */
    }

    *pu4QueryInfoLen = sizeof(UINT_32);
    return WLAN_STATUS_SUCCESS;

}
#endif


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidQueryWmmPsMode(
    IN  P_ADAPTER_T       prAdapter,
    OUT PVOID             pvQueryBuffer,
    IN  UINT_32           u4QueryBufferLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

    DEBUGFUNC("wlanoidQueryWMMPS");

    ASSERT(prAdapter);
    ASSERT(pvQueryBuffer);
    ASSERT(pu4QueryInfoLen);

    *(PUINT_8)pvQueryBuffer = gPlatformCfg.rWifiCustom.ucWmmPsEnable;
    *pu4QueryInfoLen = sizeof(UINT_8);

    return rStatus;
}   /* wlanoidQueryNvramRead */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidSetWmmPsMode (
    IN P_ADAPTER_T  prAdapter,
    IN  PVOID       pvSetBuffer,
    IN  UINT_32     u4SetBufferLen,
    OUT PUINT_32    pu4SetInfoLen
    )
{
    BOOLEAN     fgStatus;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_8 ucWmmPsEn = 0;


    DEBUGFUNC("wlanoidSetEepromWrite");
    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    ucWmmPsEn = *(PUINT_8)pvSetBuffer;
    *pu4SetInfoLen = sizeof(UINT_8);

    if(ucWmmPsEn > 1){
	    printk("[MT5921][wlanoidSetWMMPS] invalid WMMPS status %d\n",ucWmmPsEn);
        return WLAN_STATUS_FAILURE;
    }
    
    fgStatus = customDataWrite8(prAdapter,
                                OFFSET_OF(WIFI_CUSTOM_PARAM_STRUCT, ucWmmPsEnable),
                                ucWmmPsEn
                                );
    if(fgStatus == FALSE){
	    printk("[MT5921][wlanoidSetWMMPS] Write WMMPS status failed\n");
        rStatus = WLAN_STATUS_FAILURE;
    }
    return rStatus;
}   /* wlanoidSetEepromWrite */
