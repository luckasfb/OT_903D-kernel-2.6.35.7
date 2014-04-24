




#include "precomp.h"


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nichtTimerInit (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId,
    IN PFN_TIMER_CALLBACK prfnTimeout
    )
{
    P_TIMER_INFO_T prTimerInfo;

    DEBUGFUNC("htInitTimer");

    ASSERT(prAdapter);
    ASSERT(prfnTimeout);

    prTimerInfo = &prAdapter->rTimerInfo;

    if (prTimerInfo->afgHwTimerInUsed[rTimerId]) {
        WARNLOG(("Duplicated GP-Timer allocated"));
    }

    /* register the timer callback function */
    prTimerInfo->apfHwTimerHandler[rTimerId] = prfnTimeout;
    prTimerInfo->afgHwTimerInUsed[rTimerId] = TRUE;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nichtCancelTimer (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId
    )
{
    P_TIMER_INFO_T prTimerInfo;

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    /* de-register the timer callback function */
    prTimerInfo->apfHwTimerHandler[rTimerId] = NULL;
    prTimerInfo->afgHwTimerInUsed[rTimerId] = FALSE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
nichtSetTimer (
    IN P_ADAPTER_T      prAdapter,
    IN ENUM_HW_TIMER_T  rTimerId,
    IN HW_TIMER_MODE_E  rMode,
    IN HW_TIMER_FUNC_E  rAltFunc,
    IN BOOLEAN          fgTimeupEn,
    IN HW_TIMER_UNIT_E  rUnit,
    IN UINT_32          u4Count
    )
{
    P_TIMER_INFO_T prTimerInfo;
    UINT_32 u4RegMptcr;

    DEBUGFUNC("htSetTimer");

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    /* timer should be initialized at first */
    if (!prTimerInfo->afgHwTimerInUsed[rTimerId]) {
        return FALSE;
    }

    if ((rTimerId != ENUM_TIMER_2) && (rTimerId != ENUM_TIMER_3)) {
        ASSERT(0);
        return FALSE;
    }

    if (u4Count == 0) {
        ASSERT(u4Count);
        return FALSE;
    }

    if (u4Count > TDR_TIME_VALUE_MAX_T2_T3) {
        DBGLOG(NIC, ERROR, ("timer counter overflow: 0x%lx (only 16 bit is available)\n", u4Count));
        ASSERT(0);
        u4Count = TDR_TIME_VALUE_MAX_T2_T3;
    }

#if DBG
    if (!prTimerInfo->apfHwTimerHandler[rTimerId]) {
        DBGLOG(NIC, WARN, ("Install timer:%d without handler", rTimerId));
    }
#endif

    /* get register value */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegMptcr);

    /* disable timer */
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr & ~BIT(rTimerId + 17));

    /* Install the timer */
    HAL_MCR_WR(prAdapter,
                MCR_TDR,
                ((UINT_32)rTimerId << 30) |
                (TDR_TIME_VALUE_CTRL_PERIOD) |
                (fgTimeupEn ? TDR_TIMEUP_ENABLE : 0) |
                (rAltFunc << 24) |
                ((rMode == HW_TIMER_MODE_PERIODIC) ? TDR_TIMER_MODE_AUTO_REPEAT : 0) |
                ((rUnit == HW_TIMER_UNIT_1_TU) ? TDR_TIMER_COUNT_UNIT_TU : 0) |
                (u4Count & TDR_TIME_VALUE_MASK));

    /* enable timer */
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr | BIT(rTimerId + 17));

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
nichtSetTsfTimer (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId,
    IN HW_TIMER_MODE_E rMode,
    IN HW_TIMER_FUNC_E rAltFunc,
    IN BOOLEAN fgTimeupEn,
    IN UINT_32 u4StartTime,
    IN UINT_32 u4Period
    )
{
    UINT_32 u4RegMptcr;
    P_TIMER_INFO_T prTimerInfo;

    DEBUGFUNC("nichtSetTsfTimer");

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    /* timer should be initialized at first */
    if (!prTimerInfo->afgHwTimerInUsed[rTimerId]) {
        return FALSE;
    }

    if ((rTimerId != ENUM_TIMER_0) && (rTimerId != ENUM_TIMER_1)) {
        ASSERT(0);
        return FALSE;
    }

    if (!prTimerInfo->apfHwTimerHandler[rTimerId]) {
        DBGLOG(NIC, WARN, ("Install timer:%d without handler", rTimerId));
    }

    if ((u4StartTime >= TDR_TIME_VALUE_MAX_T0_T1)) {
        DBGLOG(NIC, ERROR, ("timer start time counter overflow: 0x%lx (only 22 bit is available)\n", u4StartTime));
        ASSERT(0);
        u4StartTime = TDR_TIME_VALUE_MAX_T0_T1;
    }
    if ((u4Period >= TDR_TIME_VALUE_MAX_T0_T1)) {
        DBGLOG(NIC, ERROR, ("timer period counter overflow: 0x%lx (only 22 bit is available)\n", u4Period));
        ASSERT(0);
        u4Period = TDR_TIME_VALUE_MAX_T0_T1;
    }

    /* get register value */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegMptcr);

    /* disable timer */
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr & ~BIT(rTimerId + 17));

    /* Install the timer */
    //start time
    HAL_MCR_WR(prAdapter,
                MCR_TDR,
                ((UINT_32)rTimerId << 30) |
                (fgTimeupEn ? TDR_TIMEUP_ENABLE : 0) |
                (rAltFunc << 24) |
                ((rMode == HW_TIMER_MODE_PERIODIC) ? TDR_TIMER_MODE_AUTO_REPEAT : 0) |
                (u4StartTime & TDR_TIME_VALUE_MASK));

    //period
    HAL_MCR_WR(prAdapter,
                MCR_TDR,
                ((UINT_32)rTimerId << 30) |
                TDR_TIME_VALUE_CTRL_PERIOD |
                (fgTimeupEn ? TDR_TIMEUP_ENABLE : 0) |
                (rAltFunc << 24) |
                ((rMode == HW_TIMER_MODE_PERIODIC) ? TDR_TIMER_MODE_AUTO_REPEAT : 0) |
                (u4StartTime & TDR_TIME_VALUE_MASK));

    /* enable timer */
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegMptcr | BIT(rTimerId + 17));

    return TRUE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nichtKillTimer (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    /* disable timer */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue & ~BIT(rTimerId + 17));

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcesT0TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TIMER_INFO_T  prTimerInfo;

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    DEBUGFUNC("nicProcesT0TimeupInterrupt");

    DBGLOG(NIC, INFO, ("T0 timeup\n"));

    if (prTimerInfo->apfHwTimerHandler[ENUM_TIMER_0]) {
        prTimerInfo->apfHwTimerHandler[ENUM_TIMER_0](prAdapter);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcesT1TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TIMER_INFO_T  prTimerInfo;

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    DEBUGFUNC("nicProcesT1TimeupInterrupt");

    DBGLOG(NIC, INFO, ("T1 timeup\n"));

    if (prTimerInfo->apfHwTimerHandler[ENUM_TIMER_1]) {
        prTimerInfo->apfHwTimerHandler[ENUM_TIMER_1](prAdapter);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcesT2TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TIMER_INFO_T  prTimerInfo;

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    DEBUGFUNC("nicProcesT2TimeupInterrupt");

    DBGLOG(NIC, INFO, ("T2 timeup\n"));

    if (prTimerInfo->apfHwTimerHandler[ENUM_TIMER_2]) {
        prTimerInfo->apfHwTimerHandler[ENUM_TIMER_2](prAdapter);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcesT3TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
    P_TIMER_INFO_T  prTimerInfo;

    ASSERT(prAdapter);

    prTimerInfo = &prAdapter->rTimerInfo;

    DEBUGFUNC("nicProcesT3TimeupInterrupt");

    DBGLOG(NIC, INFO, ("T3 timeup\n"));

    if (prTimerInfo->apfHwTimerHandler[ENUM_TIMER_3]) {
        prTimerInfo->apfHwTimerHandler[ENUM_TIMER_3](prAdapter);
    }
}


