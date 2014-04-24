





#ifndef _NIC_HW_TIMER_H
#define _NIC_HW_TIMER_H



#define WLAN_ROOT_TIMER     ENUM_TIMER_2
#define WLAN_ROOT_TIMER_INT INT_EVENT_T2_TIMEUP

#define VOIP_POLL_TIMER     ENUM_TIMER_3
#define VOIP_POLL_TIMER_INT INT_EVENT_T3_TIMEUP



/* Alternative function of Timers */
typedef enum _HW_TIMER_FUNC_E {
    HW_TIMER_FUNC_TSF_FREE_RUN,
    HW_TIMER_FUNC_TX_TRG_FRM,
    HW_TIMER_FUNC_TX_PS_POLL,
    HW_TIMER_FUNC_SAPSD,
    HW_TIMER_FUNC_TS,
    NUM_HW_TIMER_FUNCS
} HW_TIMER_FUNC_E, *P_HW_TIMER_FUNC_E;

/* Timers trigger mode */
typedef enum _HW_TIMER_MODE_E {
    HW_TIMER_MODE_ONE_SHOT,
    HW_TIMER_MODE_PERIODIC,
    NUM_HW_TIMER_MODES
} HW_TIMER_MODE_E, *P_HW_TIMER_MODE_E;

/* Timers unit */
typedef enum _HW_TIMER_UNIT_E {
    HW_TIMER_UNIT_1_US,
    HW_TIMER_UNIT_1_TU,
    NUM_HW_TIMER_UNITS
} HW_TIMER_UNIT_E, *P_HW_TIMER_UNIT_E;




//3 /* Session for CONNECTION SETTINGS */
typedef enum _ENUM_HW_TIMER_T {
    ENUM_TIMER_0,
    ENUM_TIMER_1,
    ENUM_TIMER_2,
    ENUM_TIMER_3,
    ENUM_TIMER_NUM
} ENUM_HW_TIMER_T, *P_ENUM_HW_TIMER_T;


typedef struct _TIMER_INFO_T {

    PFN_TIMER_CALLBACK apfHwTimerHandler[ENUM_TIMER_NUM];
    BOOLEAN     afgHwTimerInUsed[ENUM_TIMER_NUM];

} TIMER_INFO_T, *P_TIMER_INFO_T;



#define NIC_HW_TIMER_SET_VOIP_POLL_TIMER(prAdapter, eFunc, u4Period, fgTimeupIntEn) \
    { \
        nichtSetTimer(prAdapter, \
                      VOIP_POLL_TIMER, \
                      HW_TIMER_MODE_PERIODIC, \
                      eFunc, \
                      fgTimeupIntEn, \
                      HW_TIMER_UNIT_1_TU, \
                      u4Period); \
        NIC_SET_INT_EVENT(prAdapter, VOIP_POLL_TIMER_INT); \
    }

#define NIC_HW_TIMER_UNSET_VOIP_POLL_TIMER(prAdapter) \
    { \
        UINT_32 u4Value; \
        HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4Value); \
        u4Value |= (SPCR_PSPOLL_SP_INVALID_MASK | SPCR_TRIGGER_SP_INVALID_MASK); \
        HAL_MCR_WR(prAdapter, MCR_MPTCR, u4Value); \
        u4Value &= ~(SPCR_PSPOLL_SP_INVALID_MASK | SPCR_TRIGGER_SP_INVALID_MASK); \
        HAL_MCR_WR(prAdapter, MCR_MPTCR, u4Value); \
        nichtKillTimer(prAdapter, \
                      VOIP_POLL_TIMER); \
        NIC_UNSET_INT_EVENT(prAdapter, VOIP_POLL_TIMER_INT); \
    }
        /* Old work around method about WPD00001530, add logic reset
         * nicLogicReset(prAdapter) in above macro
         * NIC_HW_TIMER_UNSET_VOIP_POLL_TIMER()
         */


VOID
nichtTimerInit (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId,
    IN PFN_TIMER_CALLBACK prfnTimeout
    );

VOID
nichtCancelTimer (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId
    );

BOOL
nichtSetTimer (
    IN P_ADAPTER_T      prAdapter,
    IN ENUM_HW_TIMER_T  rTimerId,
    IN HW_TIMER_MODE_E  rMode,
    IN HW_TIMER_FUNC_E  rAltFunc,
    IN BOOLEAN          fgTimeupEn,
    IN HW_TIMER_UNIT_E  rUnit,
    IN UINT_32          u4Count
    );

BOOL
nichtSetTsfTimer (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId,
    IN HW_TIMER_MODE_E  rMode,
    IN HW_TIMER_FUNC_E rAltFunc,
    IN BOOLEAN fgTimeupEn,
    IN UINT_32 u4StartTime,
    IN UINT_32 u4Period
    );

VOID
nichtKillTimer (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_TIMER_T rTimerId
    );

VOID
nicProcesT0TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcesT1TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcesT2TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcesT3TimeupInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

#endif /* _NIC_HW_TIMER_H */


