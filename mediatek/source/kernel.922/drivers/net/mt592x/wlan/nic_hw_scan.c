






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicHwScanConfig (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    ASSERT(prAdapter);
    ASSERT(prScanCfg);

    halHwScanSetInitialInst(prAdapter, prScanCfg, eScanMode);

    halHwScanSetTerminateInst(prAdapter, prScanCfg, eScanMode);

    halHwScanRegisterInit(prAdapter,
                          prScanCfg,
                          eScanMode);

    if (eScanMode == ENUM_HW_SCAN_BG_SSID_SCAN) {
        P_BSS_INFO_T prBssInfo = &prAdapter->rBssInfo;
        kalMemZero(prBssInfo->aucBSSID, MAC_ADDR_LEN);
        prBssInfo->eBSSType = BSS_TYPE_INFRASTRUCTURE;
        nicSetupBSS(prAdapter, prBssInfo);
    }
}   /* nicHwScanConfig */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicHwScanConfigRestore (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    ASSERT(prAdapter);
    ASSERT(prScanCfg);

    halHwScanRegisterUnInit(prAdapter,
                            prScanCfg,
                            eScanMode);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicHwScanEnable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    ASSERT(prAdapter);

    halHwScanEnable(prAdapter, eScanMode);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicHwScanDisable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    ASSERT(prAdapter);

    /* Disable function and polling until function is disabled */
    halHwScanDisable(prAdapter, eScanMode);

    // NOTE: not to unmask scan done interrupt due to this procedure will
    //       always be invoked in BG SSID scan
//    NIC_RESET_INT_EVENT(prAdapter, INT_EVENT_SCAN_DONE);

} /* nicHwScanDisable */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicHwScanGetLastScannedChnlFreq (
    IN  P_ADAPTER_T prAdapter,
    OUT PUINT_8     pucChnlIdx,
    OUT PUINT_8     pucBandIdx
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);
    ASSERT(pucChnlIdx);
    ASSERT(pucBandIdx);

    /* read the HW register for the scanned channel/ band index information */
    HAL_MCR_RD(prAdapter, MCR_BSSSAR, &u4RegValue);

    /* get the last scanned channel and band index */
    *pucChnlIdx = (UINT_8)((u4RegValue & BSSSAR_CHNL_INDEX_MASK) >> BSSSAR_CHNL_INDEX_OFFSET);
    *pucBandIdx = (UINT_8)((u4RegValue & BSSSAR_BAND_INDEX_MASK) >> BSSSAR_BAND_INDEX_OFFSET);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicHwScanScannedChnlFreqBand (
    IN  P_ADAPTER_T     prAdapter,
    IN  UINT_16         u2ChnlBandInfo,
    OUT PUINT_8         pucChnlNum,
    OUT P_ENUM_BAND_T   prBandNum
    )
{
    P_SCAN_CONFIG_T prScanCfg;
    UINT_8  ucChnlIdx, ucBandIdx;

    ASSERT(prAdapter);
    ASSERT(pucChnlNum);
    ASSERT(prBandNum);

    prScanCfg = &prAdapter->rScanInfo.rScanConfig;

    /* Not to apply the channel index setting when the HW scan index is not valid */
    if ((u2ChnlBandInfo & ~BIT(7)) == 0) {
        P_RF_INFO_T prRFInfo;

        /* Use current channel information if RFB's channel info is 0 */
        prRFInfo = &prAdapter->rRFInfo;        
        *pucChnlNum = prRFInfo->ucChannelNum;
        *prBandNum = prRFInfo->eBand;
        return;
    }

    ucChnlIdx = (UINT_8)((u2ChnlBandInfo & BITS(8, 15)) >> 8) - 1;
    ucBandIdx = (UINT_8)(u2ChnlBandInfo & BITS(0, 6)) - 1;

#if DBG & 0
    if (ucChnlIdx > (sizeof(prScanCfg->arChnlInfoList) / sizeof(RF_CHANNEL_INFO_T)) ||
        ucBandIdx > (sizeof(prScanCfg->arChnlInfoList) / sizeof(RF_CHANNEL_INFO_T))) {
        ucChnlIdx = 0;
        ucBandIdx = 0;
    }
#endif

    if (ucChnlIdx < MAXIMUM_OPERATION_CHANNEL_LIST) {
        *pucChnlNum = prScanCfg->arChnlInfoList[ucChnlIdx].ucChannelNum;
        *prBandNum = prScanCfg->arChnlInfoList[ucChnlIdx].eBand;
    }

    DBGLOG(SCAN, INFO, ("Channel Num: %d, Band: %s (CH Index: %d, BAND Index: %d)\n",
                        *pucChnlNum, (*prBandNum) ? "5GHz" : "2.4GHz",
                        ucChnlIdx, ucBandIdx));

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcesScanDoneInterrupt(
    IN P_ADAPTER_T      prAdapter
    )
{
    ASSERT(prAdapter);

    arbFsmRunEventScanDone(prAdapter);
}


