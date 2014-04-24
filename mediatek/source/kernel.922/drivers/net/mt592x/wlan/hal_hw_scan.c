






#include "precomp.h"





UINT_32 u4ScanInstCount;
BOOLEAN fgScanInited = FALSE;




/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanEnable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    UINT_32 u4RegMptcr;

    ASSERT(prAdapter);

    /* start HW scan function */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegMptcr);

    if (eScanMode == ENUM_HW_SCAN_NORMAL_SCAN) {
        /* HW scan does not worried about if AC0~AC4 will be stopped by
           HW for IBSS-PS (ATIM window != 0 & IBSS mode)
        */

        /* Sanity check for the HW scan state and configuration */
        if (u4RegMptcr & MPTCR_SSID_SRCH) {
            u4RegMptcr &= ~MPTCR_SSID_SRCH;
            ASSERT(0);
        }

        HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr | MPTCR_SCAN_EN);
    }
    else if (eScanMode == ENUM_HW_SCAN_BG_SSID_SCAN) {
        u4RegMptcr &= ~(MPTCR_PRETBTT_TRIG_EN | MPTCR_PREDTIM_TRIG_EN);

        /* Sanity check for the HW scan state and configuration */
        if (u4RegMptcr & MPTCR_SCAN_EN) {
            u4RegMptcr &= ~MPTCR_SCAN_EN;
            ASSERT(0);
        }

        /* enable SSID search function */
        HAL_MCR_WR(prAdapter,
                    MCR_MPTCR,
                    u4RegMptcr |
                    MPTCR_SSID_SRCH |
                    MPTCR_TBTT_PERIOD_TIMER_EN);
    }
    else {
        ASSERT(0);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halHwScanDisable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
#define POLLING_LOOP_COUNT  100

    UINT_32 i, u4RegValue;

    ASSERT(prAdapter);

    /* Disable function */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);

    if (eScanMode == ENUM_HW_SCAN_NORMAL_SCAN) {
        HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue & ~MPTCR_SCAN_EN);

        /* polling until scan is disabled */
        for (i = 0; i < POLLING_LOOP_COUNT; i ++) {
            HAL_MCR_RD(prAdapter, MCR_CIR, &u4RegValue);
            if (u4RegValue & CIR_SCAN_BUSY) {
                kalUdelay(10);
            }
            else {
                break;
            }
        }
        if (i == POLLING_LOOP_COUNT) {
            ASSERT(0);
            return FALSE;
        }
    }
    else if (eScanMode == ENUM_HW_SCAN_BG_SSID_SCAN) {
        HAL_MCR_WR(prAdapter,
                   MCR_MPTCR,
                   u4RegValue & ~(MPTCR_SSID_SRCH | MPTCR_TBTT_PERIOD_TIMER_EN));

        /* Basically, this function is disabled automatically when LP ownback */
    }
    else {
        ASSERT(0);
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanSetBgScanParam (
    IN P_ADAPTER_T          prAdapter,
    IN P_BG_SCAN_CONFIG_T   prBgScanCfg
    )
{
#if CFG_WORKAROUND_HEC_5269
    UINT_32 u4RegMptcr;
#endif
    UINT_32 u4RegDbgr;
    UINT_32 u4RegAtcr;
    UINT_32 u4RegSscr;
    UINT_32 u4RegTtpcr;
    UINT_32 i, j;
    P_PARAM_SSID_T prSsidEntry;
    UINT_8 ucDtimPeriod;
    P_BG_SCAN_SSID_CANDIDATE_T  prScanCandidate;
    UINT_16 u2BaseWakePeriod;
    UINT_8 ucStepOfWakePeriod;
    UINT_32 u4SleepTimeout;

    DEBUGFUNC("halHwScanSetBgScanParam");

    ASSERT(prAdapter);
    ASSERT(prBgScanCfg);

    prScanCandidate = &prBgScanCfg->rScanCandidate;
    u2BaseWakePeriod = prBgScanCfg->u2BaseWakePeriod;
    ucStepOfWakePeriod = prBgScanCfg->ucBgScanStepOfWakePeriod;

    /* return if NULL SSID entry */
    if (prScanCandidate->ucNumHwSsidScanEntry == 0) {
        DBGLOG(SCAN, ERROR, ("NULL SSID entry\n\r"));
        return;
    }
    else if (prScanCandidate->ucNumHwSsidScanEntry > HAL_HW_SCAN_ENTRY_NUMBER) {
        prScanCandidate->ucNumHwSsidScanEntry = HAL_HW_SCAN_ENTRY_NUMBER; //maximum 16 entries
    }

    /* set NULL interval to 0 for disabling DTIM wakeup function to trigger INT */
    HAL_MCR_RD(prAdapter, MCR_ATCR, &u4RegAtcr);
    u4RegAtcr &= ~ATCR_TX_NULL_INTERVAL;
    HAL_MCR_WR(prAdapter, MCR_ATCR, u4RegAtcr);

    /* program background scan period and its exponential */
    if (ucStepOfWakePeriod > 7) {
        ucStepOfWakePeriod = 7;
    }
    ucDtimPeriod = 128 >> ucStepOfWakePeriod;
    u2BaseWakePeriod /= ucDtimPeriod;

    u4RegTtpcr = (u2BaseWakePeriod | (ucDtimPeriod << 16) | (1 << 28));
    HAL_MCR_WR(prAdapter, MCR_TTPCR, u4RegTtpcr);

#if CFG_WORKAROUND_HEC_6796
    /* do this logic reset for preventing HEC6796, which is may cause SW fail to program
       the SSID entry in few cases.
    */
    nicLogicReset(prAdapter);
#endif

    HAL_MCR_RD(prAdapter, MCR_SSCR, &u4RegSscr);
    u4RegSscr &= ~SSCR_SSID_NUMBER;
    HAL_MCR_WR(prAdapter, MCR_SSCR,
                u4RegSscr |
                (prBgScanCfg->ucBgScanMinRcpiThr << 24) |
                (prScanCandidate->ucNumHwSsidScanEntry & SSCR_SSID_NUMBER) |
                SSCR_SSID_START);

    /* program SSIDs */
    for (i = 0; i < prScanCandidate->ucNumHwSsidScanEntry; i++) {
        /* get the entry of SSID history */
        prSsidEntry = &prScanCandidate->arHwSsidScanEntry[i];

        HAL_MCR_WR(prAdapter, MCR_SDR, prSsidEntry->u4SsidLen);

        for (j = 0; j < ((prSsidEntry->u4SsidLen + 3) >> 2); j++) {
            HAL_MCR_WR(prAdapter, MCR_SDR,
                        ((PUINT_32)prSsidEntry->aucSsid)[j]);
        }
    }

#if DBG
    /* check if SSID program completed */
    HAL_MCR_RD(prAdapter, MCR_SSCR, &u4RegSscr);
    if (u4RegSscr & SSCR_SSID_START) {
        DBGLOG(SCAN, ERROR, ("polling timeout\n\r"));
        ASSERT(0);
    }
#endif

#if CFG_WORKAROUND_HEC_5269
    /* prevent TX done status cause HW fail to enter sleep or wakeup */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegMptcr);
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr | MPTCR_TX_DONE_SLEEP_CTRL);
#endif

    /* Modify the sleep timeout for LP watchdog */
    u4SleepTimeout = prBgScanCfg->u2BaseWakePeriod << (ucStepOfWakePeriod + 1);
    u4SleepTimeout >>= 10; // in unit of 1024 TU
    u4SleepTimeout += 1;

    ASSERT(u4SleepTimeout <= DBGR_SLEEP_TIMEOUT_COUNT);
    u4SleepTimeout &= DBGR_SLEEP_TIMEOUT_COUNT;

    HAL_MCR_RD(prAdapter, MCR_DBGR, &u4RegDbgr);
    u4RegDbgr &= ~DBGR_SLEEP_TIMEOUT_COUNT;
    u4RegDbgr |= DBGR_SLEEP_TIMEOUT_UNIT_1024_TU;
    HAL_MCR_WR(prAdapter, MCR_DBGR, u4RegDbgr | u4SleepTimeout);

    /* Set to infrastructure mode */
    halSetOPMode(prAdapter, OP_MODE_INFRASTRUCTURE);

} /* nicHwScanSetBgScanParam */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
halHwScanSetPerChnlInst (
    IN P_ADAPTER_T          prAdapter,
    IN P_RF_CHANNEL_INFO_T  prChnlInfoList,
    OUT PUINT_32            pu4ChSwInstNum
    )
{
    UINT_32 u4FifoWrPtr = 0;
    UINT_32 u4McrCfpr;
    UINT_32 u4McrOfpr;
    BOOLEAN fgTxFilterForJp;

    ASSERT(prAdapter);
    ASSERT(prChnlInfoList);
    ASSERT(pu4ChSwInstNum);

    /* Initial process for recording the number of instruction
       for channel switch */
    HAL_HW_SCAN_GET_INST_NUMBER_INIT(prAdapter, &u4FifoWrPtr);

    fgTxFilterForJp = nicSetTxFilterForJP(prAdapter,
                                          prChnlInfoList->ucChannelNum,
                                          prChnlInfoList->eBand);

    if (halSetRFSwitchChnlInst(prAdapter,
                            prChnlInfoList->ucChannelNum,
                            prChnlInfoList->eBand,
                            fgTxFilterForJp) != WLAN_STATUS_SUCCESS) {

        return WLAN_STATUS_NOT_ACCEPTED;
    }

    /* Stall 100 us for synthesizer stable. */
    HAL_HW_SCAN_SET_INST_DELAY(prAdapter, 100, FALSE);

    /* Program TX power gain */
    if (halSetTxPowerGain(prAdapter,
                          prChnlInfoList->ucChannelNum,
                          prChnlInfoList->eBand,
                          &u4McrCfpr,
                          &u4McrOfpr) == WLAN_STATUS_SUCCESS) {

        HAL_HW_SCAN_SET_INST_MCR_WR(prAdapter,
                                    MCR_CFPR,
                                    u4McrCfpr,
                                    FALSE,
                                    TRUE);
        HAL_HW_SCAN_SET_INST_MCR_WR(prAdapter,
                                    MCR_OFPR,
                                    u4McrOfpr,
                                    TRUE,
                                    TRUE);
    }

    /* Record the number of instruction for channel switch
       at the end of the LP instruction programming */
    HAL_HW_SCAN_GET_INST_NUMBER_END(prAdapter, u4FifoWrPtr, pu4ChSwInstNum);

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanSetScanCtrlInst (
    IN P_ADAPTER_T          prAdapter,
    IN P_RF_CHANNEL_INFO_T  parChnlInfoList,
    IN UINT_8               ucNumOfScanChnl,
    IN ENUM_SCAN_TYPE_T     eScanType
    )
{
    UINT_32 i;
    UINT_32 u4ScanCtrlWrPr = 0;
    UINT_32 u4ChnlSwInstNum = 0, u4BandSwInstNum = 0;
#if DBG
    UINT_32 u4ChnlSwLastInstNum = 0;
#endif

    DEBUGFUNC("halHwScanSetScanCtrlInst");

    ASSERT(prAdapter);
    ASSERT(parChnlInfoList);

    /* Note: A band support should be considered later, it only consider BG band
             currently */

    /* program preamble scan ctrl - reserve the slot for the instruction */
    HAL_HW_SCAN_PRESERVE_INST_SCAN_CTRL(prAdapter, &u4ScanCtrlWrPr);

    for (i = 0; i < ucNumOfScanChnl; i++) {
        halHwScanSetPerChnlInst(prAdapter,
                                &parChnlInfoList[i],
                                &u4ChnlSwInstNum);

#if DBG
        /* note: we do not check if 'u4ChSwInstNum' are identical for
                 each channel currently
        */
        if ((i != 0) && (u4ChnlSwLastInstNum != u4ChnlSwInstNum)) {
            DBGLOG(SCAN, TRACE, ("u4ChnlSwLastInstNum: %ld, u4ChnlSwInstNum: %ld",
                u4ChnlSwLastInstNum, u4ChnlSwInstNum));
            ASSERT(0);
        }
        else {
            u4ChnlSwLastInstNum = u4ChnlSwInstNum;
        }
#endif
    }

    /* program preamble scan ctrl - by the instructions for each channel switch */
    HAL_HW_SCAN_RESTORE_INST_SCAN_CTRL(prAdapter,
                                       eScanType,
                                       ucNumOfScanChnl,
                                       u4BandSwInstNum,
                                       u4ChnlSwInstNum,
                                       u4ScanCtrlWrPr);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanSetInitialInst (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    UINT_32 u4QueueDwStart = 0xffffffff;
    UINT_32 u4DwSize;

    DEBUGFUNC("halHwScanSetInitialInst");

    ASSERT(prAdapter);
    ASSERT(prScanCfg);

    /* find scan queue start address */
    if (!halGetQueueInfo(prAdapter, ENUM_QUEUE_ID_SCAN, &u4QueueDwStart, &u4DwSize)) {
        DBGLOG(SCAN, ERROR, ("Cannot find the scan queue within memory map!"));
        ASSERT(0);
        return;
    }

    HAL_HW_SCAN_INST_START(prAdapter, u4QueueDwStart);

    /* Program parameters for channel switch */
    halHwScanSetScanCtrlInst(prAdapter,
                             prScanCfg->arChnlInfoList,
                             prScanCfg->ucNumOfScanChnl,
                             prScanCfg->eScanType);

    /* Program the end address */
    HAL_HW_SCAN_INST_END(prAdapter,
                         (UINT_32)ENUM_QUEUE_ID_SCAN,
                         u4QueueDwStart,
                         u4DwSize);


}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanSetTerminateInst (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    UINT_32 u4QueueDwStart = 0xffffffff;
    UINT_32 u4DwSize;
    UINT_32 u4ChnlSwInstNum;

    DEBUGFUNC("halHwScanSetTerminateInst");

    ASSERT(prAdapter);
    ASSERT(prScanCfg);


    if (eScanMode == ENUM_HW_SCAN_BG_SSID_SCAN) {
        // there's no this function under BG SSID SCAN
        return;
    }

    /* find scan ctrl queue start address */
    if (!halGetQueueInfo(prAdapter, ENUM_QUEUE_ID_SCAN_CTRL, &u4QueueDwStart, &u4DwSize)) {
        DBGLOG(SCAN, ERROR, ("Cannot find the scan control queue within memory map!"));
        ASSERT(0);
        return;
    }

    HAL_HW_SCAN_INST_START(prAdapter, u4QueueDwStart);

    halHwScanSetPerChnlInst(prAdapter,
                            &prScanCfg->rOrgChnlInfo,
                            &u4ChnlSwInstNum);

    HAL_HW_SCAN_INST_END(prAdapter,
                         (UINT_32)ENUM_QUEUE_ID_SCAN_CTRL,
                         u4QueueDwStart,
                         u4DwSize);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanRegisterInit (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    UINT_32 u4RegValue;
    P_SCAN_INFO_T prScanInfo;
    P_BACKUP_REGISTER_VALUE_T prBkupReg;

    ASSERT(prAdapter);
    ASSERT(prScanCfg);

    prScanInfo = &prAdapter->rScanInfo;
    prBkupReg = &prScanInfo->rBkupRegValue;

    /* program scan channel idle time */
    HAL_MCR_RD(prAdapter, MCR_SSCR, &u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_SSCR,
               (u4RegValue & ~(SSCR_MIN_SCAN_TIME | SSCR_EXT_SCAN_TIME)) |
               (prScanCfg->ucChnlDwellTimeExt << 16) |
               (prScanCfg->ucChnlDwellTimeMin << 8));

    if (eScanMode == ENUM_HW_SCAN_NORMAL_SCAN) {
        HAL_MCR_RD(prAdapter, MCR_MPTCR, &prBkupReg->u4RegOrgLpEnFunc);
        u4RegValue = prBkupReg->u4RegOrgLpEnFunc;

        // Honor the trigger event during scan?
        if (!prScanCfg->fgToEnableTriggerEvent) {
            /* for the TBTT timer */
            u4RegValue &= ~MPTCR_PRETBTT_PREDTIM_TRIG_EN;

            /* <TODO> for the VoIP timer (T3), change to free run timer */
        }
        HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue);

        // Honor service period before scan?
        if (!prScanCfg->fgToHonorServicePeriod) {
            HAL_MCR_RD(prAdapter, MCR_SPCR, &prBkupReg->u4RegSpcr);

            // Disable Service period
            halpmSetupServicePeriodMechanism(prAdapter, 0, SP_ALL);
        }

        // RFIFO threshold
        if (prScanCfg->u2RxFifoThreshold) {
            HAL_MCR_RD(prAdapter, MCR_HITR, &u4RegValue);
            u4RegValue = (UINT_32)prScanCfg->u2RxFifoThreshold & HFCR_RX_INT_THRESHOLD;
            HAL_MCR_WR(prAdapter, MCR_HITR, u4RegValue);
        }
    }
    else if (eScanMode == ENUM_HW_SCAN_BG_SSID_SCAN) {
        /* program BG SSID specific parameters */
        halHwScanSetBgScanParam(prAdapter, &prScanCfg->rBgScanCfg);

        // Disable Service period
        halpmSetupServicePeriodMechanism(prAdapter, 0, SP_ALL);
    }

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halHwScanRegisterUnInit (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    )
{
    P_SCAN_INFO_T prScanInfo;
    P_BACKUP_REGISTER_VALUE_T prBkupReg;
    UINT_32 u4RegMptcr;

    ASSERT(prAdapter);
    ASSERT(prScanCfg);

    prScanInfo = &prAdapter->rScanInfo;
    prBkupReg = &prScanInfo->rBkupRegValue;

    /* ----------------------------------------------------------- */
    /* Program LP instruction to restore original register setting */
    /* ----------------------------------------------------------- */

    if (eScanMode == ENUM_HW_SCAN_NORMAL_SCAN) {
        // RFIFO threshold
        if (prScanCfg->u2RxFifoThreshold) {
            HAL_MCR_WR(prAdapter, MCR_HITR, prBkupReg->u4RegOrgRxFifoThr);
        }

        // Honor service period before scan? (and Reset timeout count)
        if (prScanCfg->fgToHonorServicePeriod == FALSE) {
            HAL_MCR_WR(prAdapter, MCR_SPCR, prBkupReg->u4RegSpcr);
        }

        // Honor the trigger event during scan?
        HAL_MCR_WR(prAdapter, MCR_MPTCR, prBkupReg->u4RegOrgLpEnFunc);
    }
    else if (eScanMode == ENUM_HW_SCAN_BG_SSID_SCAN) {
        HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegMptcr);
        u4RegMptcr &= ~(MPTCR_SSID_SRCH |
#if CFG_WORKAROUND_HEC_5269
                        MPTCR_TX_DONE_SLEEP_CTRL |
#endif
                        MPTCR_TBTT_PERIOD_TIMER_EN);
        HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr);
    }
}



