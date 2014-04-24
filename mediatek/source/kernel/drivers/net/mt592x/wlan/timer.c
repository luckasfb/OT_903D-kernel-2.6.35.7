






#include "precomp.h"







/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerInitialize (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_TIMER_CALLBACK   prfnTimerHandler
    )
{
    ASSERT(prAdapter);
    ASSERT(prfnTimerHandler);

    LINK_INITIALIZE(&prAdapter->rTimerList);

#if CFG_USE_SW_ROOT_TIMER
    kalOsTimerInitialize(prAdapter->prGlueInfo, (PVOID)prfnTimerHandler);
#else
    nichtTimerInit(prAdapter, WLAN_ROOT_TIMER, prfnTimerHandler);

    NIC_SET_INT_EVENT(prAdapter, WLAN_ROOT_TIMER_INT);
#endif

    return;
} /* end of timerInitialize() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerUnInitialize (
    IN P_ADAPTER_T  prAdapter
    )
{
    ASSERT(prAdapter);

    LINK_INITIALIZE(&prAdapter->rTimerList);

#if CFG_USE_SW_ROOT_TIMER
    /* <TODO> to reclaim the resource */

#else
    nichtCancelTimer(prAdapter, WLAN_ROOT_TIMER);

    NIC_UNSET_INT_EVENT(prAdapter, WLAN_ROOT_TIMER_INT);

    nichtKillTimer(prAdapter, WLAN_ROOT_TIMER);
#endif

    return;
} /* end of timerInitialize() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerInitTimer (
    IN P_ADAPTER_T              prAdapter,
    IN P_TIMER_T                prTimer,
    IN PFN_MGMT_TIMEOUT_FUNC    pfFunc,
    IN UINT_32                  u4Data,
    IN BOOLEAN                  fgNeedHwAccess
    )
{
    ASSERT(prAdapter);
    ASSERT(prTimer);
    ASSERT(pfFunc);

    LINK_ENTRY_INITIALIZE(&prTimer->rLinkEntry);
    prTimer->pfMgmtTimeOutFunc  = pfFunc;
    prTimer->u4Data             = u4Data;
    prTimer->fgNeedHwAccess     = fgNeedHwAccess;

    return;

} /* end of timerInitTimer() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOL
timerSetTimer (
    IN P_ADAPTER_T prAdapter,
    IN OS_SYSTIME  rTimeout
    )
{
    ASSERT(prAdapter);

#if CFG_USE_SW_ROOT_TIMER

    return kalSetTimer(prAdapter->prGlueInfo, rTimeout);

#else
    /* milli-second to TU, and include the OS timer resolution */
    return nichtSetTimer(prAdapter,
                         WLAN_ROOT_TIMER,
                         HW_TIMER_MODE_ONE_SHOT,
                         HW_TIMER_FUNC_TSF_FREE_RUN,
                         TRUE,
                         HW_TIMER_UNIT_1_TU,
                         SYSTIME_TO_TU(rTimeout) + (MSEC_PER_SEC / KAL_HZ));
#endif
} /* end of kalSetTimer() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerStopTimer (
    IN P_ADAPTER_T              prAdapter,
    IN P_TIMER_T                prTimer

    )
{
    ASSERT(prAdapter);
    ASSERT(prTimer);

    if (timerPendingTimer(prTimer)) {
        P_LINK_T prTimerList = &prAdapter->rTimerList;

        LINK_REMOVE_KNOWN_ENTRY(prTimerList, &prTimer->rLinkEntry);

#if (!CFG_USE_SW_ROOT_TIMER)
        /* When using HW timer,
           kill the HW timer source if no pending tasks to serve */
        if (prTimerList->u4NumElem == 0) {
            nichtKillTimer(prAdapter,
                           WLAN_ROOT_TIMER);
        }
#endif
    }
} /* timerStopTimer */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerStartTimer (
    IN P_ADAPTER_T              prAdapter,
    IN P_TIMER_T                prTimer,
    IN UINT_32                  u4TimeoutMs
    )
{
    P_LINK_T prTimerList;
    OS_SYSTIME rExpiredSysTime;

    ASSERT(prAdapter);
    ASSERT(prTimer);

    prTimerList= &prAdapter->rTimerList;

    /* To avoid infinite loop in function wlanDoTimeOutCheck(),
     * zero interval is not permitted.
     */
    if (u4TimeoutMs == 0) {
        u4TimeoutMs = 1;
    }

    rExpiredSysTime = kalGetTimeTick() + MSEC_TO_SYSTIME(u4TimeoutMs);

    /* If no timer pending or the fast time interval is used. */
    if (prTimerList->prNext == (P_LINK_ENTRY_T)prTimerList ||
        TIME_BEFORE(rExpiredSysTime, prAdapter->rNextExpiredSysTime)) {
        if (timerSetTimer(prAdapter, MSEC_TO_SYSTIME(u4TimeoutMs))) {
            prAdapter->rNextExpiredSysTime = rExpiredSysTime;
        }
    }

    /* Add this timer to checking list */
    prTimer->rExpiredSysTime = rExpiredSysTime;

    if (!timerPendingTimer(prTimer)) {
        LINK_INSERT_TAIL(prTimerList, &prTimer->rLinkEntry);
    }

    return;
} /* timerStartTimer */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerDoTimeOutCheck (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_LINK_T prTimerList;
    P_LINK_ENTRY_T prLinkEntry;
    P_TIMER_T prTimer;
    OS_SYSTIME rCurSysTime;

    DEBUGFUNC("timerDoTimeOutCheck");

    ASSERT(prAdapter);

    prTimerList= &prAdapter->rTimerList;

    GET_CURRENT_SYSTIME(&rCurSysTime);

    /* Set the permitted max timeout value for new one */
    prAdapter->rNextExpiredSysTime = rCurSysTime + MGMT_MAX_TIMEOUT_INTERVAL;

    LINK_FOR_EACH(prLinkEntry, prTimerList) {
        prTimer = LINK_ENTRY(prLinkEntry, TIMER_T, rLinkEntry);

        /* Check if this entry is timeout. */
        if (!TIME_BEFORE(rCurSysTime, prTimer->rExpiredSysTime)) {
            timerStopTimer(prAdapter, prTimer);

#if CFG_USE_SW_ROOT_TIMER
            if (prTimer->fgNeedHwAccess) {
                ARB_ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);


                DBGLOG(INIT, INFO, ("Timer Handler: ->%s(): SYS_TIME = %ld\n",
                    prTimer->aucDbgString, u4CurTime));

                (prTimer->pfMgmtTimeOutFunc)(prTimer->u4Data); // It may add timer there.

                DBGLOG(INIT, INFO, ("Timer Handler: <-%s()\n", prTimer->aucDbgString));

                ARB_RECLAIM_POWER_CONTROL_TO_PM(prAdapter);
            }
            else
#endif
            {

                DBGLOG(INIT, INFO, ("(NO HW ACCESS)Timer Handler: ->%s(): SYS_TIME = %ld\n",
                    prTimer->aucDbgString, rCurSysTime));


                (prTimer->pfMgmtTimeOutFunc)(prTimer->u4Data); // It may add timer there.

                DBGLOG(INIT, INFO, ("(NO HW ACCESS)Timer Handler: <-%s()\n", prTimer->aucDbgString));
            }

            /* Search entire list again because of nest del and add timers
             * and current MGMT_TIMER could be volatile after stopped
             */
            prLinkEntry = (P_LINK_ENTRY_T)prTimerList;
        }
        else if (TIME_BEFORE(prTimer->rExpiredSysTime, prAdapter->rNextExpiredSysTime)) {
            prAdapter->rNextExpiredSysTime = prTimer->rExpiredSysTime;
        }
    } /* end of for loop */

    /* Setup the prNext timeout event. It is possible the timer was already
     * set in the above timeout callback function.
     */
    if (!LINK_IS_EMPTY(prTimerList)) {
        ASSERT(TIME_AFTER(prAdapter->rNextExpiredSysTime, rCurSysTime));

        if (!timerSetTimer(prAdapter,
                           (OS_SYSTIME)((INT_32)prAdapter->rNextExpiredSysTime - (INT_32)rCurSysTime))) {
            /* If fail to set timer, set max value to permit new one */
            prAdapter->rNextExpiredSysTime = rCurSysTime + MGMT_MAX_TIMEOUT_INTERVAL;
        }
    }

} /* end of wlanDoTimeOutCheck() */

#if !CFG_USE_SW_ROOT_TIMER
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
timerDoTimeOutCheckOnLogicReset (
    IN P_ADAPTER_T  prAdapter
    )
{
    P_LINK_T prTimerList;
    P_LINK_ENTRY_T prLinkEntry;
    P_TIMER_T prTimer;
    OS_SYSTIME rCurSysTime;

    DEBUGFUNC("timerDoTimeOutCheckOnLogicReset");

    ASSERT(prAdapter);

    prTimerList= &prAdapter->rTimerList;

    GET_CURRENT_SYSTIME(&rCurSysTime);

    /* Set the permitted max timeout value for new one */
    prAdapter->rNextExpiredSysTime = rCurSysTime + MGMT_MAX_TIMEOUT_INTERVAL;

    LINK_FOR_EACH(prLinkEntry, prTimerList) {
        prTimer = LINK_ENTRY(prLinkEntry, TIMER_T, rLinkEntry);

        /* Check if this entry is timeout. */
        if (!TIME_BEFORE(rCurSysTime, prTimer->rExpiredSysTime)) {

            /* not to do timer callback due to it will also cause another nested state transition,
               which is not allowed in current architecture */

            prAdapter->rNextExpiredSysTime = rCurSysTime + 1;
            break;
        }
        else if (TIME_BEFORE(prTimer->rExpiredSysTime, prAdapter->rNextExpiredSysTime)) {
            prAdapter->rNextExpiredSysTime = prTimer->rExpiredSysTime;
        }
    } /* end of for loop */

    /* Setup the prNext timeout event. It is possible the timer was already
     * set in the above timeout callback function.
     */
    if (!LINK_IS_EMPTY(prTimerList)) {
        ASSERT(TIME_AFTER(prAdapter->rNextExpiredSysTime, rCurSysTime));

        if (!timerSetTimer(prAdapter,
                           (OS_SYSTIME)((INT_32)prAdapter->rNextExpiredSysTime - (INT_32)rCurSysTime))) {
            /* If fail to set timer, set max value to permit new one */
            prAdapter->rNextExpiredSysTime = rCurSysTime + MGMT_MAX_TIMEOUT_INTERVAL;
        }
    }

} /* end of wlanDoTimeOutCheck() */
#endif

