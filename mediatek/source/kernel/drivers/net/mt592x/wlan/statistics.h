




#ifndef _STATISTICS_H
#define _STATISTICS_H




typedef struct _STATISTICS_INFO_T {

    TIMER_T                 rPeekTimer;       /* Read h/w counters timer */
    OS_SYSTIME              rLastIntPeekTime; /* system time for last int event.*/
    BOOL                    fgUseTimerTrig; /*True:  Timer trigger.
                                              False: Int trigger*/
    BOOL                    fgEn;

} STATISTICS_INFO_T, *P_STATISTICS_INFO_T;





/***** Routines in statistics.c *****/
VOID
statisticsInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsTimerInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsIntInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsTimerUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsIntUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsTimerTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
statisticsIntEevnt (
    IN P_ADAPTER_T prAdapter
    );


VOID
statisticsPeekHWCounter (
    IN P_ADAPTER_T  prAdapter
    );


#endif /* _STATISTICS_H */

