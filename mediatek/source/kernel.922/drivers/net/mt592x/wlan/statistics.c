






#include "precomp.h"










/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsInit (
    IN P_ADAPTER_T prAdapter
    )
{

    DEBUGFUNC("statisticsInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

#if CFG_STATISTICS_TIMER_EN
    statisticsTimerInit(prAdapter);
#else
    statisticsIntInit(prAdapter);
#endif
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsTimerInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STATISTICS_INFO_T       prStatisInfo;


    DEBUGFUNC("statisticsTimerInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prStatisInfo = &prAdapter->rStatisticsInfo;

    ARB_INIT_TIMER(prAdapter,
                   prStatisInfo->rPeekTimer,
                   statisticsTimerTimeOut,
                   TRUE);

    ARB_SET_TIMER(prAdapter,
                  prStatisInfo->rPeekTimer,
                  CFG_TIMER_PEEK_HW_COUNTERS_PERIOD_MSEC);
    
    prStatisInfo->fgUseTimerTrig = TRUE;
    prStatisInfo->fgEn = TRUE;

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsIntInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STATISTICS_INFO_T       prStatisInfo;


    DEBUGFUNC("statisticsIntInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prStatisInfo = &prAdapter->rStatisticsInfo;
    prStatisInfo->fgUseTimerTrig = FALSE;
    prStatisInfo->fgEn = TRUE;

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsUnInit (
    IN P_ADAPTER_T prAdapter
    )
{

    DEBUGFUNC("statisticsUnInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

#if CFG_STATISTICS_TIMER_EN
    statisticsTimerUnInit(prAdapter);
#else
    statisticsIntUnInit(prAdapter);
#endif
    return;
}



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsTimerUnInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STATISTICS_INFO_T       prStatisInfo;


    DEBUGFUNC("statisticsTimerUnInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prStatisInfo = &prAdapter->rStatisticsInfo;
    prStatisInfo->fgEn = FALSE;
    ARB_CANCEL_TIMER(prAdapter,
                  prStatisInfo->rPeekTimer);

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsIntUnInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STATISTICS_INFO_T       prStatisInfo;


    DEBUGFUNC("statisticsIntUnInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prStatisInfo = &prAdapter->rStatisticsInfo;
    prStatisInfo->fgEn = TRUE;

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsTimerTimeOut (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STATISTICS_INFO_T       prStatisInfo;


    DEBUGFUNC("statisticsTimerTimeOut");

    ASSERT(prAdapter);

    DBGLOG(REQ, TRACE, ("\n"));

    statisticsPeekHWCounter(prAdapter);

    prStatisInfo = &prAdapter->rStatisticsInfo;
    ARB_SET_TIMER(prAdapter,
                  prStatisInfo->rPeekTimer,
                  CFG_TIMER_PEEK_HW_COUNTERS_PERIOD_MSEC);
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsIntEevnt (
    IN P_ADAPTER_T prAdapter
    )
{
    P_STATISTICS_INFO_T       prStatisInfo;
    OS_SYSTIME                rCurrentSysTime;

    DEBUGFUNC("statisticsIntEevnt");

    ASSERT(prAdapter);

    prStatisInfo = &prAdapter->rStatisticsInfo;

    if(prStatisInfo->fgEn == TRUE && prStatisInfo->fgUseTimerTrig == FALSE) {
        DBGLOG(REQ, TEMP, ("\n"));
        GET_CURRENT_SYSTIME(&rCurrentSysTime);
        if (CHECK_FOR_TIMEOUT(rCurrentSysTime,
                prStatisInfo->rLastIntPeekTime,
                MSEC_TO_SYSTIME(CFG_INT_PEEK_HW_COUNTERS_TIMEOUT_MSEC))) {
            DBGLOG(REQ, INFO, ("Int Event timeout\n"));
            DBGLOG(REQ, INFO, ("Current time = %ld, Previous time = %ld\n", \
                (UINT_32)rCurrentSysTime, (UINT_32)prStatisInfo->rLastIntPeekTime));
            statisticsPeekHWCounter(prAdapter);
            COPY_SYSTIME(prStatisInfo->rLastIntPeekTime, rCurrentSysTime);
        }
    }
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
statisticsPeekHWCounter (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_TX_CTRL_T         prTxCtrl;
    P_RX_CTRL_T         prRxCtrl;
    UINT_32                 u4Tmp;

    DEBUGFUNC("statisticsPeekHWCounter");

    DBGLOG(REQ, LOUD, ("\n"));

    ASSERT(prAdapter);

    prTxCtrl = &prAdapter->rTxCtrl;
    prRxCtrl = &prAdapter->rRxCtrl;

    /* Query from MCR. */
    HAL_MCR_RD(prAdapter, MCR_MIBSCR, &u4Tmp);
    u4Tmp &= ~MIBSCR_INDEX_MASK;
    u4Tmp |= (MIBSCR_TX_COUNT_INDEX);
    HAL_MCR_WR(prAdapter, MCR_MIBSCR, u4Tmp);

    /* MIBSCR_TX_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);

   /* MIBSCR_BEACON_TX_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    TX_ADD_CNT(prTxCtrl, TX_BEACON_MMPDU_COUNT, u4Tmp);
    DBGLOG(REQ, LOUD, ("Beacon Tx count: %"DBG_PRINTF_64BIT_DEC", this time = %ld\n",\
        prTxCtrl->au8Statistics[TX_BEACON_MMPDU_COUNT], u4Tmp ));

    /* MIBSCR_FRAME_RETRY_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);

    /* MIBSCR_RTS_RETRY_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);

    HAL_MCR_RD(prAdapter, MCR_MIBSCR, &u4Tmp);
    u4Tmp &= ~MIBSCR_INDEX_MASK;
    u4Tmp |= (MIBSCR_RX_FCS_ERROR_COUNT_INDEX);
    HAL_MCR_WR(prAdapter, MCR_MIBSCR, u4Tmp);

    /* MIBSCR_RX_FCS_ERROR_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    RX_ADD_CNT(prRxCtrl, RX_BB_FCS_ERROR_COUNT, u4Tmp);
    DBGLOG(REQ, LOUD, ("pauCrcErr: %"DBG_PRINTF_64BIT_DEC", this time = %ld\n",\
        prRxCtrl->au8Statistics[RX_BB_FCS_ERROR_COUNT], u4Tmp ));

    /* MIBSCR_RX_FIFO_FULL_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    RX_ADD_CNT(prRxCtrl, RX_BB_FIFO_FULL_COUNT, u4Tmp);
    DBGLOG(REQ, LOUD, ("pauRxFifoFull: %"DBG_PRINTF_64BIT_DEC", this time = %ld\n",\
        prRxCtrl->au8Statistics[RX_BB_FIFO_FULL_COUNT], u4Tmp));

    /* MIBSCR_RX_MPDU_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    RX_ADD_CNT(prRxCtrl, RX_BB_MPDU_COUNT, u4Tmp);
    DBGLOG(REQ, LOUD, ("pauRxPkt: %"DBG_PRINTF_64BIT_DEC", this time = %ld\n",\
         prRxCtrl->au8Statistics[RX_BB_MPDU_COUNT], u4Tmp));

    /* MIBSCR_CHANNEL_IDLE_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    RX_ADD_CNT(prRxCtrl, RX_BB_CHANNEL_IDLE_COUNT, u4Tmp);
    DBGLOG(REQ, LOUD, ("pauChannelidle: %"DBG_PRINTF_64BIT_DEC", this time = %ld\n",\
         prRxCtrl->au8Statistics[RX_BB_CHANNEL_IDLE_COUNT], u4Tmp));

//Skip CCA and CCA/NAV/TX for 11h and 11k

    HAL_MCR_RD(prAdapter, MCR_MIBSCR, &u4Tmp);
    u4Tmp &= ~MIBSCR_INDEX_MASK;
    u4Tmp |= (MIBSCR_BEACON_TIMEOUT_COUNT_INDEX);
    HAL_MCR_WR(prAdapter, MCR_MIBSCR, u4Tmp);

    /* MIBSCR_BEACON_TIMEOUT_COUNT_INDEX */
    HAL_MCR_RD(prAdapter, MCR_MIBSDR, &u4Tmp);
    RX_ADD_CNT(prRxCtrl, RXTX_BEACON_TIMEOUT_COUNT, u4Tmp);
    DBGLOG(REQ, LOUD, ("Beacon Timeout : %"DBG_PRINTF_64BIT_DEC", this time = %ld\n",\
         prRxCtrl->au8Statistics[RXTX_BEACON_TIMEOUT_COUNT], u4Tmp));

}
