






#include "precomp.h"

#define USE_INTERNAL_32K_RING_OSC   0
#define USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER      0

#define OSC_STABLE_TIME_US          1900//2000//1000
#define RF_SX_STABLE_TIME_US        120
#define PLL_STABLE_TIME_US          30

static UINT_16 u2OscStableTime      = (UINT_16)OSC_STABLE_TIME_US;
static UINT_16 u2RfSxStableTime     = (UINT_16)RF_SX_STABLE_TIME_US;
static UINT_16 u2PllStableTime      = (UINT_16)PLL_STABLE_TIME_US;



#define LP_CLOCK_ON_DELAY_P1_US(_32kCcrSlowCnt)      \
            (u2OscStableTime > (UINT_16)_32kCcrSlowCnt) ? \
            (u2OscStableTime - (UINT_16)_32kCcrSlowCnt) : 0

#define LP_CLOCK_ON_DELAY_P2_US(_32kCcrSlowCnt)      \
            (u2RfSxStableTime > (UINT_16)_32kCcrSlowCnt) ? \
            (u2RfSxStableTime - (UINT_16)_32kCcrSlowCnt) : 0

#define LP_CLOCK_ON_DELAY_P3_US(_32kCcrSlowCnt)      \
            (u2PllStableTime + (UINT_16)_32kCcrSlowCnt)

#define PRE_TBTT_INTERVAL           4


#define MPTCR_INFRASTRUCTURE_STA_SETTING_CONNECTED      \
                   (MPTCR_BMC_TIMEOUT_EN |              \
                    MPTCR_MORE_TRIG_EN |                \
                    MPTCR_TX_PSPOLL_TIMEOUT_EN |        \
                    MPTCR_BCN_CONTENT_CHK_EN |          \
                    MPTCR_APSD_TIMEOUT_EN |             \
                    MPTCR_BCN_TIMEOUT_EN |              \
                    MPTCR_TBTT_PERIOD_TIMER_EN |        \
                    MPTCR_BCN_UC_EN |                   \
                    MPTCR_TX_NULL_EN |                  \
                    MPTCR_BCN_BMC_EN |                  \
                    MPTCR_BCN_PARSE_TIM_EN |            \
                    MPTCR_RX_BMC_MGT_EN |               \
                    MPTCR_PREDTIM_TRIG_EN)

#define MPTCR_AD_HOC_STA_SETTING_CONNECTED              \
                   (MPTCR_BCN_CONTENT_CHK_EN |          \
                    MPTCR_BCN_TIMEOUT_EN |              \
                    MPTCR_TBTT_PERIOD_TIMER_EN |        \
                    MPTCR_PRETBTT_TRIG_EN)

#define MPTCR_AD_HOC_STA_SETTING_CREATE                 \
                   (MPTCR_TBTT_PERIOD_TIMER_EN)

#define HAL_SETUP_LOW_POWER_FUNC_FOR_BSS(_prAdapter, _pfgIsIbssMaster) \
    { \
        UINT_32 u4RegCir; \
        HAL_MCR_RD(_prAdapter, MCR_CIR, &u4RegCir); \
        *_pfgIsIbssMaster = ((u4RegCir & CIR_IBSS_MASTER) ? TRUE : FALSE); \
    }

#define CEILING(_Value, _Base)    \
        (((UINT_32)(_Value) + (UINT_32)((_Base) - 1)) / (UINT_32)(_Base))



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmProgLowPwrInst (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       uc32kSlowCount,
    IN UINT_8       ucNumExtraSleepEntry,
    IN UINT_8       ucNumExtraWakeEntry,
    IN UINT_8       ucNumExtraOnEntry,
    IN UINT_32      au4ExtraLowPowerInst_sleep[],
    IN UINT_32      au4ExtraLowPowerInst_wake[],
    IN UINT_32      au4ExtraLowPowerInst_full_on[],
    IN BOOLEAN      fgForBtCoexist
    )
{
    UINT_32 u4LpInstSleepStartAddr = CFG_LOW_POWER_INST_START_ADDR;
    UINT_32 u4LpInstWakeStartAddr, u4LpInstOnStartAddr;
    UINT_32 i;
    UINT_32 u4NumOfSleepEntry = 0, u4NumOfWakeEntry = 0, u4NumOfOnEntry = 0;
#if USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER
    P_CONNECTION_SETTINGS_T prConnSettings;
#endif
    /* Low Power instructions */
    UINT_32 au4LowPowerInst_sleep[10];
    UINT_32 au4LowPowerInst_wake[10];
    UINT_32 au4LowPowerInst_full_on[5];
//    UINT_32 u4Value;

    DEBUGFUNC("halpmProgLowPwrInst");

    ASSERT(prAdapter);
    if (ucNumExtraSleepEntry) {
        ASSERT(au4ExtraLowPowerInst_sleep);
    }
    if (ucNumExtraWakeEntry) {
        ASSERT(au4ExtraLowPowerInst_wake);
    }
    if (ucNumExtraOnEntry) {
        ASSERT(au4ExtraLowPowerInst_full_on);
    }
    ASSERT(ucNumExtraSleepEntry <= NUM_OF_MAX_LP_INSTRUCTIONS);
    ASSERT(ucNumExtraWakeEntry <= NUM_OF_MAX_LP_INSTRUCTIONS);
    ASSERT(ucNumExtraOnEntry <= NUM_OF_MAX_LP_INSTRUCTIONS);


    /* ********************** */
    /* Low Power instructions */
    /* ********************** */
    /* --- sleep --- */

#if USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER
/*lint -save -e572 Excessive shift value (precision 1 shifted right by 16) */
    au4LowPowerInst_sleep[u4NumOfSleepEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_LCR, TRUE, (0 >> 16));    // LED off
/*lint -restore */
#else
    au4LowPowerInst_sleep[u4NumOfSleepEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_LCR, TRUE, (0 >> 16));    // LED off
#endif

#if PTA_ENABLED
    #if PTA_NEW_BOARD_DESIGN
    /* single antenna does not need to use ANT_P/ANT_N */
    au4LowPowerInst_sleep[u4NumOfSleepEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_IOPCR,
                                 FALSE,
                                 IO_SET_TRAP_PIN_INPUT_DIR |
                                 IO_SET_DBG_PORT_SEL);   // trap pin set to input mode
    #else
        au4LowPowerInst_sleep[u4NumOfSleepEntry++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_IOPCR,
                                     FALSE,
                                     IO_SET_TRAP_PIN_INPUT_DIR |

                                     ((prAdapter->rPtaInfo.fgSingleAntenna) ?
                                      (IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR |
                                       IOPCR_IO_TR_SW_P_DIR | IOPCR_IO_TR_SW_N_DIR) : 0) |
                                     IO_SET_DBG_PORT_SEL);   // trap pin set to input mode

    #endif
#else
    au4LowPowerInst_sleep[u4NumOfSleepEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_IOPCR,
                                 FALSE,
                                 IO_SET_TRAP_PIN_INPUT_DIR |
                                 IO_SET_DBG_PORT_SEL);   // trap pin set to input mode

#endif
        /* Note: place the instruction if GPIO or other pin need to be pull high/ low or
                 set to input mode for preventing current leakage here...
        */
        /* 31            27       24    23      20     19       16     15                                         0 */
    au4LowPowerInst_sleep[u4NumOfSleepEntry++] =
        LP_INST_CLOCK_OFF(0, 0, 1, 0,   0, 0, 0, 0,     0, 0, 0, 1);

    /* --- wake --- */
    au4LowPowerInst_wake[u4NumOfWakeEntry++] =
        /* 31           27       24    23       20     19       16     15                                         0 */
        LP_INST_CLOCK_ON(0, 0, 1, 0,    0, 0, 1, 0,     0, 0, 0, 1,     1, 0, LP_CLOCK_ON_DELAY_P1_US(uc32kSlowCount));
    au4LowPowerInst_wake[u4NumOfWakeEntry++] =
        LP_INST_CLOCK_ON(0, 0, 1, 0,    1, 1, 1, 0,     0, 1, 0, 1,     0, 0, LP_CLOCK_ON_DELAY_P2_US(uc32kSlowCount));
    au4LowPowerInst_wake[u4NumOfWakeEntry++] =
        LP_INST_CLOCK_ON(0, 0, 1, 0,    1, 1, 1, 1,     0, 1, 0, 0,     0, 0, LP_CLOCK_ON_DELAY_P3_US(uc32kSlowCount));
        /* Note: Here restore the pin attribute if it is changed for preventing current leakage before enter LP...
        */
#if PTA_ENABLED
    #if PTA_NEW_BOARD_DESIGN
        /* single antenna does not use ant_p/ant_n as output */
    au4LowPowerInst_wake[u4NumOfWakeEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_IOPCR,
                                 FALSE,
                                 IO_SET_TRAP_PIN_DEFAULT_ATTR |
                                 IO_SET_DBG_PORT_SEL); // trap pin set to output mode
    #else
    au4LowPowerInst_wake[u4NumOfWakeEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_IOPCR,
                                 FALSE,
                                 IO_SET_TRAP_PIN_DEFAULT_ATTR |
                                 ((prAdapter->rPtaInfo.fgSingleAntenna) ?
                                  (IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR |
                                   IOPCR_IO_TR_SW_P_DIR | IOPCR_IO_TR_SW_N_DIR) : 0) |
                                 IO_SET_DBG_PORT_SEL); // trap pin set to output mode
    #endif
#else
    au4LowPowerInst_wake[u4NumOfWakeEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_IOPCR,
                                 FALSE,
                                 IO_SET_TRAP_PIN_DEFAULT_ATTR |
                                 IO_SET_DBG_PORT_SEL); // trap pin set to output mode
#endif




    /* --- on --- */
#if PTA_ENABLED
#if 0
    if (fgForBtCoexist & prAdapter->rPtaInfo.fgSingleAntenna) {
        /* set single antenna by BTCER1.bit[28] */
        HAL_MCR_RD(prAdapter, MCR_BTCER1, &u4Value);
        au4LowPowerInst_full_on[u4NumOfOnEntry++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_BTCER1,
                                     TRUE,
                                     (u4Value | PTA_BTCER1_SINGLE_ANT)>>16);

        /* set antenna HW mode by RICR.bits[24:21] and RICR.bit[8], RICR.bit[6] */
        /* enable TX_PE/RX_PE  by RICR.bit[8] */
        HAL_MCR_RD(prAdapter, MCR_RICR, &u4Value);
        u4Value &= ~(RICR_ANT_SEL_N_SW_MODE |
                     RICR_ANT_SEL_P_SW_MODE |
                     RICR_SWANT_SEL_N_HIGH |
                     RICR_SWANT_SEL_P_HIGH |
                     RICR_RF_SW_MODE |
                     RICR_SW_TR_SW_N |
                     RICR_SW_TR_SW_P |
                     RICR_SW_RF_TX |
                     RICR_SW_RF_RX |
                     RICR_SW_TX_PE |
                     RICR_SW_RX_PE);
        au4LowPowerInst_full_on[u4NumOfOnEntry++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_RICR,
                                     TRUE,
                                     u4Value>>16);
        au4LowPowerInst_full_on[u4NumOfOnEntry++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_RICR,
                                     FALSE,
                                     u4Value);
    }
#endif
#endif

#if USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER
    prConnSettings = &prAdapter->rConnSettings;
    au4LowPowerInst_full_on[u4NumOfOnEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_LCR, TRUE, (prConnSettings->ucLedBlinkMode != 0) ? (LCR_LED_OUTPUT >> 16) : 0x0000);    // LED on
#else
    au4LowPowerInst_full_on[u4NumOfOnEntry++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_LCR, TRUE, prAdapter->prGlueInfo->u4LedSetting >> 16);    // LED on
#endif

    ASSERT(u4NumOfSleepEntry < (sizeof(au4LowPowerInst_sleep) / sizeof(UINT_32)));
    ASSERT(u4NumOfWakeEntry < (sizeof(au4LowPowerInst_wake) / sizeof(UINT_32)));
    ASSERT(u4NumOfOnEntry < (sizeof(au4LowPowerInst_full_on) / sizeof(UINT_32)));
    /* ***************************** */
    /* End of Low Power instructions */
    /* ***************************** */


    /* program the 1K WLAN FIFO start address */
    HAL_MCR_WR(prAdapter, MCR_HFCR, HFCR_SELECT_WLAN_FIFO | HFCR_FIFO_WRITE | u4LpInstSleepStartAddr);

    /* program for the "extra" SLEEP section */
    for (i = 0; i < ucNumExtraSleepEntry; i++) {
        ASSERT(au4ExtraLowPowerInst_sleep);
        HAL_MCR_WR(prAdapter, MCR_HFDR, au4ExtraLowPowerInst_sleep[i]);
        DBGLOG(LP, LOUD, ("Sleep %ld: 0x%08lx\n", i, au4ExtraLowPowerInst_sleep[i]));
    }
    /* program for the SLEEP section */
    for (i = 0; i < u4NumOfSleepEntry; i++) {
        HAL_MCR_WR(prAdapter, MCR_HFDR, au4LowPowerInst_sleep[i]);
        DBGLOG(LP, LOUD, ("Sleep %ld: 0x%08lx\n", i, au4LowPowerInst_sleep[i]));
    }

    /* Summation for the total entry number for entering SLEEP */
    u4NumOfSleepEntry += ucNumExtraSleepEntry;

    /* program the start/ end address of WLAN_on to Sleep, Initial to Sleep */
    HAL_MCR_WR(prAdapter, MCR_LPWSAR,
                LP_INST_WLAN_ON_to_SLEEP_ADDR(u4LpInstSleepStartAddr, u4NumOfSleepEntry) |
                LP_INST_INITIAL_to_SLEEP_ADDR(u4LpInstSleepStartAddr, u4NumOfSleepEntry)
                );

    DBGLOG(LP, INFO, ("LP inst (Sleep): %ld -> %ld (extra: %d)\n",
                        u4LpInstSleepStartAddr,
                        u4LpInstSleepStartAddr + u4NumOfSleepEntry - 1,
                        ucNumExtraSleepEntry));

    /* advance the start address */
    u4LpInstWakeStartAddr = u4LpInstSleepStartAddr + u4NumOfSleepEntry;

    /* program for the WAKE section */
    for (i = 0; i < u4NumOfWakeEntry; i++) {
        HAL_MCR_WR(prAdapter, MCR_HFDR, au4LowPowerInst_wake[i]);
        DBGLOG(LP, LOUD, ("Wake %ld: 0x%08lx\n", i, au4LowPowerInst_wake[i]));
    }
    /* program for the "extra" WAKE section */
    for (i = 0; i < ucNumExtraWakeEntry; i++) {
        ASSERT(au4ExtraLowPowerInst_wake);
        HAL_MCR_WR(prAdapter, MCR_HFDR, au4ExtraLowPowerInst_wake[i]);
        DBGLOG(LP, LOUD, ("Wake %ld: 0x%08lx\n", i, au4ExtraLowPowerInst_wake[i]));
    }

    /* Summation for the total entry number for entering AWAKE */
    u4NumOfWakeEntry += ucNumExtraWakeEntry;

    DBGLOG(LP, INFO, ("LP inst (wake): %ld -> %ld (extra: %d)\n",
                        u4LpInstWakeStartAddr,
                        u4LpInstWakeStartAddr + u4NumOfWakeEntry - 1,
                        ucNumExtraWakeEntry));

    /* advance the start address */
    u4LpInstOnStartAddr = u4LpInstWakeStartAddr + u4NumOfWakeEntry;

    /* program for the ON section */
    for (i = 0; i < u4NumOfOnEntry; i++) {
        HAL_MCR_WR(prAdapter, MCR_HFDR, au4LowPowerInst_full_on[i]);
        DBGLOG(LP, LOUD, ("On %ld: 0x%08lx\n", i, au4LowPowerInst_full_on[i]));
    }
    /* program for the "extra" ON section */
    for (i = 0; i < ucNumExtraOnEntry; i++) {
        ASSERT(au4ExtraLowPowerInst_full_on);
        HAL_MCR_WR(prAdapter, MCR_HFDR, au4ExtraLowPowerInst_full_on[i]);
        DBGLOG(LP, LOUD, ("On %ld: 0x%08lx\n", i, au4ExtraLowPowerInst_full_on[i]));
    }

    /* Summation for the total entry number for entering ON */
    u4NumOfOnEntry += ucNumExtraOnEntry;

    DBGLOG(LP, INFO, ("LP inst (on): %ld -> %ld (extra: %d)\n",
                        u4LpInstOnStartAddr,
                        u4LpInstOnStartAddr + u4NumOfOnEntry - 1,
                        ucNumExtraOnEntry));

    /* program the start/ end address of Sleep to WLAN_on, WLAN_on to ON */
    HAL_MCR_WR(prAdapter, MCR_LPOSAR,
                LP_INST_SLEEP_to_WLAN_ON_ADDR(u4LpInstWakeStartAddr, u4NumOfWakeEntry) |
                LP_INST_WLAN_ON_to_ON_ADDR(u4LpInstOnStartAddr, u4NumOfOnEntry)
                );

}   /* halpmProgLowPwrInst */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmCalculateGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 uc32kSlowCount,
    IN UINT_8 uc32kSlowCount_10,
    IN UINT_8 uc32kSlowCount_100,
    IN UINT_8 uc32kSlowCount_1000,
    OUT PUINT_16 pu2GuardTimePhase2,
    OUT PUINT_16 pu2GuardTimePhase3
    )
{
    UINT_32 u4X1, u4Y1, u4Z1;
    UINT_32 u4G1, u4L1;
    UINT_32 u4Value;

    ASSERT(prAdapter);
    ASSERT(pu2GuardTimePhase2);
    ASSERT(pu2GuardTimePhase3);
    ASSERT(uc32kSlowCount);

    /* The 32K should have a non-zero value for setting guard time */
    if (uc32kSlowCount == 0) {
        return;
    }

    /* NOTE: Here we do not include the non-clock relavent LP instruction write into the
             calculation of guard, which costs 50ns * 2 (with pipe-line) for each instrucion.
    */

    u4X1 = CEILING(LP_CLOCK_ON_DELAY_P1_US(uc32kSlowCount), uc32kSlowCount);
    u4Y1 = CEILING(LP_CLOCK_ON_DELAY_P2_US(uc32kSlowCount), uc32kSlowCount);
    u4Z1 = CEILING(LP_CLOCK_ON_DELAY_P3_US(uc32kSlowCount), uc32kSlowCount);

    u4G1 = (2 + 1 + 3 + u4X1 + 1 + u4Y1 + 1 + u4Z1);
    u4L1 = (7 + 1 + u4Y1 + 1 + u4Z1);

    // phase 2 guard time
    u4Value = (u4G1 +
               CEILING((u4G1 / 10)   * (UINT_32)uc32kSlowCount_10,   uc32kSlowCount) +
               CEILING((u4G1 / 100)  * (UINT_32)uc32kSlowCount_100,  uc32kSlowCount) +
               CEILING((u4G1 / 1000) * (UINT_32)uc32kSlowCount_1000, uc32kSlowCount)) *
               (UINT_32)uc32kSlowCount;
    *pu2GuardTimePhase2 = (UINT_16)u4Value;

    // phase 3 guard time
    u4Value = (u4L1 +
               CEILING((u4L1 / 10)   * (UINT_32)uc32kSlowCount_10,   uc32kSlowCount) +
               CEILING((u4L1 / 100)  * (UINT_32)uc32kSlowCount_100,  uc32kSlowCount) +
               CEILING((u4L1 / 1000) * (UINT_32)uc32kSlowCount_1000, uc32kSlowCount)) *
               (UINT_32)uc32kSlowCount;
    *pu2GuardTimePhase3 = (UINT_16)u4Value;

}   /* halpmCalculateGuardTime */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmRegInit (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4Tstr;

    ASSERT(prAdapter);

    /* Use CCA to decide whether to extend the timeout */
    HAL_MCR_WR(prAdapter, MCR_SSCR, SSCR_EXT_TIME_CTRL);

    halpmSetBeaconTimeout(prAdapter,
                          BEACON_MIN_TIMEOUT_VALUE,
                          BEACON_MAX_TIMEOUT_VALUE,
                          BEACON_MIN_TIMEOUT_VALID,
                          BEACON_MAX_TIMEOUT_VALID,
                          BEACON_TIMEOUT_COUNT_INFRA);
    halpmSetNullTimeout(prAdapter,
                        NULL_MIN_TIMEOUT_VALUE,
                        NULL_MAX_TIMEOUT_VALUE,
                        NULL_MIN_TIMEOUT_VALID,
                        NULL_MAX_TIMEOUT_VALID,
                        NULL_TIMEOUT_COUNT);
    halpmSetTimeout(prAdapter,
                    MIN_TIMEOUT_VALUE,
                    MAX_TIMEOUT_VALUE,
                    MIN_TIMEOUT_VALID,
                    MAX_TIMEOUT_VALID,
                    TIMEOUT_COUNT);

    /*  TSTR (0x1a8) */
    // NOTICE: pre-tbtt interval is necessary in current ASIC design for transmitting beacon!
    HAL_MCR_RD(prAdapter, MCR_TSTR, &u4Tstr);
    u4Tstr &= ~TSTR_PRETBTT_INTERVAL_MASK;
    u4Tstr |= ((PRE_TBTT_INTERVAL << 24) & TSTR_PRETBTT_INTERVAL_MASK);
    HAL_MCR_WR(prAdapter, MCR_TSTR, u4Tstr);


#if USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER
    {
    /*  LCR (0x94) */
    P_CONNECTION_SETTINGS_T prConnSettings = &prAdapter->rConnSettings;
    if (prConnSettings->ucLedBlinkMode) {
        HAL_MCR_WR(prAdapter, MCR_LCR, LCR_LED_OUTPUT);   // LED on, mode 0
    }
    }
#endif

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetOscStableTime (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_16      u2OscStableTimeUs
    )
{
    ASSERT(prAdapter);

    /* Use the default value unless the setting is not 0 carried from EEPROM */
    if (u2OscStableTimeUs) {

        /* Due to HEC 5300, it is limited that OSC stable time cannot greater
           than 1900 for one CLK_ON LP instruction */
        ASSERT(u2OscStableTimeUs <= 1900);

        u2OscStableTime = u2OscStableTimeUs;
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetupServicePeriodMechanism (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_32      u4ValidateSP,
    IN UINT_32      u4InvalidateSP
    )
{
    UINT_32 u4Spcr;

    ASSERT(prAdapter);

    /* Force to disable/ enable service period */
    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4Spcr);

    // enable SP
    if (u4ValidateSP & SP_BEACON) {
        u4Spcr &= ~SPCR_BEACON_SP_INVALID_MASK;
    }
    if (u4ValidateSP & SP_BMC) {
        u4Spcr &= ~SPCR_BMC_SP_INVALID_MASK;
    }
    if (u4ValidateSP & SP_QOS_CFPOLL) {
        u4Spcr &= ~SPCR_QOS_CFPOLL_SP_INVALID_MASK;
    }
    if (u4ValidateSP & SP_PS_POLL) {
        u4Spcr &= ~SPCR_PSPOLL_SP_INVALID_MASK;
    }
    if (u4ValidateSP & SP_APSD) {
        u4Spcr &= ~SPCR_TRIGGER_SP_INVALID_MASK;
    }
    // disable SP
    if (u4InvalidateSP & SP_BEACON) {
        u4Spcr |= SPCR_BEACON_SP_INVALID_MASK;
    }
    if (u4InvalidateSP & SP_BMC) {
        u4Spcr |= SPCR_BMC_SP_INVALID_MASK;
    }
    if (u4InvalidateSP & SP_QOS_CFPOLL) {
        u4Spcr |= SPCR_QOS_CFPOLL_SP_INVALID_MASK;
    }
    if (u4InvalidateSP & SP_PS_POLL) {
        u4Spcr |= SPCR_PSPOLL_SP_INVALID_MASK;
    }
    if (u4InvalidateSP & SP_APSD) {
        u4Spcr |= SPCR_TRIGGER_SP_INVALID_MASK;
    }
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4Spcr);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmEnableBeaconEarlyCheck (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_BEIR, &u4RegValue);

    u4RegValue &= BEIR_BCN_LOST_THRESHOLD;

    /* Set a rising edge of the enable bit for HW to reset the value also */
    HAL_MCR_WR(prAdapter, MCR_BEIR, u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_BEIR, u4RegValue | BEIR_BEI_CAL_EN);

}   /* halpmEnableBeaconEarlyCheck */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_16
halpmGetBeaconEarlyValue (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4EarlyTimeTu;

    ASSERT(prAdapter);

    /* Set a rising edge of the enable bit for HW to reset the value also */
    HAL_MCR_RD(prAdapter, MCR_BEIR, &u4EarlyTimeTu);
    u4EarlyTimeTu &= BEIR_BCN_EARLIER_INTERVAL_MASK;
    u4EarlyTimeTu *= 2;

    return (UINT_16)u4EarlyTimeTu;
}   /* halpmGetBeaconEarlyValue */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetTimeout (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucMinTimeoutValue,
    IN UINT_16      u2MaxTimeoutValue,
    IN BOOLEAN      fgMinTimeoutValid,
    IN BOOLEAN      fgMaxTimeoutValid,
    IN UINT_8       ucTimeoutCount
    )
{
    UINT_32 u4RegTr, u4RegAtcr;

    ASSERT(prAdapter);

    // Max timeout is in unit of 4 TU
    u2MaxTimeoutValue = (UINT_16)CEILING(u2MaxTimeoutValue, 4);// unconditional rounding

    if (u2MaxTimeoutValue > BITS(0, 7)) {
        u2MaxTimeoutValue = BITS(0, 7);
        ASSERT(0);
    }
    if (ucMinTimeoutValue > BITS(0, 5)) {
        ucMinTimeoutValue = BITS(0, 5);
        ASSERT(0);
    }


    HAL_MCR_RD(prAdapter, MCR_TR, &u4RegTr);
    u4RegTr &= ~TR_MIN_MAX_TIME_LIMIT_MASK;
    u4RegTr |= (((ucMinTimeoutValue << 0) & TR_MIN_TIME_LIMIT_MASK) |
                ((u2MaxTimeoutValue << 8) & TR_MAX_TIME_LIMIT_MASK) |
                (fgMinTimeoutValid ? TR_MIN_TIME_LIMIT_VALID : 0) |
                (fgMaxTimeoutValid ? TR_MAX_TIME_LIMIT_VALID : 0));
    HAL_MCR_WR(prAdapter, MCR_TR, u4RegTr);

    HAL_MCR_RD(prAdapter, MCR_ATCR, &u4RegAtcr);
    u4RegAtcr &= ~ATCR_TIMEOUT_COUNT_LIMIT;
    u4RegAtcr |= ((ucTimeoutCount << 4) & ATCR_TIMEOUT_COUNT_LIMIT);
    HAL_MCR_WR(prAdapter, MCR_ATCR, u4RegAtcr);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetBeaconTimeout (
    IN P_ADAPTER_T  prAdapter,
    UINT_8          ucMinTimeoutValue,
    UINT_16         u2MaxTimeoutValue,
    BOOLEAN         fgMinTimeoutValid,
    BOOLEAN         fgMaxTimeoutValid,
    UINT_8          ucTimeoutCount
    )
{
    UINT_32 u4RegTr, u4RegAtcr;

    ASSERT(prAdapter);

    // Max timeout is in unit of 4 TU
    u2MaxTimeoutValue = (UINT_16)CEILING(u2MaxTimeoutValue, 4);// unconditional rounding

    if (u2MaxTimeoutValue > BITS(0, 7)) {
        u2MaxTimeoutValue = BITS(0, 7);
        ASSERT(0);
    }
    if (ucMinTimeoutValue > BITS(0, 5)) {
        ucMinTimeoutValue = BITS(0, 5);
        ASSERT(0);
    }

    HAL_MCR_RD(prAdapter, MCR_TR, &u4RegTr);
    u4RegTr &= ~TR_BCN_MIN_MAX_TIME_LIMIT_MASK;
    u4RegTr |= (((ucMinTimeoutValue << 16) & TR_BCN_MIN_TIME_LIMIT_MASK) |
                ((u2MaxTimeoutValue << 24) & TR_BCN_MAX_TIME_LIMIT_MASK) |
                (fgMinTimeoutValid ? TR_BCN_MIN_TIME_LIMIT_VALID : 0) |
                (fgMaxTimeoutValid ? TR_BCN_MAX_TIME_LIMIT_VALID : 0));
    HAL_MCR_WR(prAdapter, MCR_TR, u4RegTr);

    HAL_MCR_RD(prAdapter, MCR_ATCR, &u4RegAtcr);
    u4RegAtcr &= ~ATCR_BCN_TIMEOUT_COUNT_LIMIT;
    u4RegAtcr |= (ucTimeoutCount & ATCR_BCN_TIMEOUT_COUNT_LIMIT);
    HAL_MCR_WR(prAdapter, MCR_ATCR, u4RegAtcr);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetNullTimeout (
    IN P_ADAPTER_T  prAdapter,
    UINT_8          ucMinTimeoutValue,
    UINT_16         u2MaxTimeoutValue,
    BOOLEAN         fgMinTimeoutValid,
    BOOLEAN         fgMaxTimeoutValid,
    UINT_8          ucTimeoutCount
    )
{
    UINT_32 u4Value;

    ASSERT(prAdapter);

    // Max timeout is in unit of 4 TU
    u2MaxTimeoutValue = (UINT_16)CEILING(u2MaxTimeoutValue, 4);// unconditional rounding

    if (u2MaxTimeoutValue > BITS(0, 7)) {
        u2MaxTimeoutValue = BITS(0, 7);
        ASSERT(0);
    }
    if (ucMinTimeoutValue > BITS(0, 5)) {
        ucMinTimeoutValue = BITS(0, 5);
        ASSERT(0);
    }

    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4Value);
    u4Value &= ~SPCR_NULL_MIN_MAX_TIME_LIMIT_MASK;

    HAL_MCR_WR(prAdapter,
               MCR_SPCR,
               u4Value |
               ucMinTimeoutValue << 16 |
               ucMinTimeoutValue << 22 |
               fgMinTimeoutValid << 23 |
               u2MaxTimeoutValue << 24);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSwitchPwrMgtBit (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgPwrMgtBit
    )
{
    UINT_32 u4Value;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_TXCR, &u4Value);
    u4Value &= ~TXCR_PWRMGT_SET;
    u4Value |= fgPwrMgtBit ? TXCR_PWRMGT_SET : 0;
    HAL_MCR_WR(prAdapter, MCR_TXCR, u4Value);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmEnterLowPower (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgEnableGlobalInt
    )
{
    ASSERT(prAdapter);

    if (fgEnableGlobalInt) {

#if 0 // NOTE(Kevin 2007/10/18): There is only one place to enable GINT - end of IST processing.
        prAdapter->fgIsIntEnable = TRUE; // NOTE(Kevin): Must placed before MCR GINT write.

        HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_ENABLE_GINT | HLPCR_LP_OWN_SET);
#endif
        prAdapter->fgIsIntEnableWithLPOwnSet = TRUE;

    }
    else {
        HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_LP_OWN_SET);
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halpmLeaveLowPower (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4LpOwnBackPollTimeUs
    )
{
#define LP_OWN_BACK_LOOP_DELAY_US   8   //exponential of 2

    UINT_32 i;
    UINT_32 u4RegValue;
    BOOLEAN fgStatus = TRUE;
    UINT_32 u4LoopCnt = u4LpOwnBackPollTimeUs / LP_OWN_BACK_LOOP_DELAY_US;

    ASSERT(prAdapter);

    /* Software get LP ownership */
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_LP_OWN_CLR);

    /* Delay for LP engine to complete its operation. */
    for (i = 0; i < u4LoopCnt; i++) {
        HAL_MCR_RD(prAdapter, MCR_CIR, &u4RegValue);
        if ((u4RegValue & (CIR_LP_STATE | CIR_PLL_READY)) != CIR_PLL_READY) {
            kalUdelay(LP_OWN_BACK_LOOP_DELAY_US);
        }
        else {
            break;
        }
    }
    if (i == u4LoopCnt) {
        fgStatus = FALSE;

        ERRORLOG(("LP cannot be own back (%ld)", u4LoopCnt));
        /* check the time of LP instructions need to perform from Sleep to On */
        //ASSERT(0);
    }

    return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halpmCalSlowClock (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_8 pucSlowClockCount,
    OUT PUINT_8 pucSlowClockCount_10,
    OUT PUINT_8 pucSlowClockCount_100,
    OUT PUINT_8 pucSlowClockCount_1000
    )
{
#define WAIT_32K_CAL_READY_LOOP_COUNT 10

    UINT_32 i;
    UINT_32 u4RegValue;
    BOOLEAN fgStatus = FALSE;

    ASSERT(prAdapter);
    ASSERT(pucSlowClockCount);
    ASSERT(pucSlowClockCount_10);
    ASSERT(pucSlowClockCount_100);
    ASSERT(pucSlowClockCount_1000);

#if USE_INTERNAL_32K_RING_OSC
    HAL_MCR_WR(prAdapter, MCR_32KCCR, X32KCCR_CLK_SEL_RING_OSC);
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_INTERNAL_32K_EN);
    HAL_MCR_WR(prAdapter, MCR_32KCCR, X32KCCR_CLK_SEL_RING_OSC | X32KCCR_CAL_START);
#else
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_EXTERNAL_32K_EN);
    HAL_MCR_WR(prAdapter, MCR_32KCCR, X32KCCR_CAL_START);
#endif

    for (i = 0; i < WAIT_32K_CAL_READY_LOOP_COUNT; i++) {
        HAL_MCR_RD(prAdapter, MCR_32KCCR, &u4RegValue);
        if (!(u4RegValue & X32KCCR_CAL_START)) {
            fgStatus = TRUE;
            break;
        }
        kalMdelay(50);
    }

    /* Calculate the slow clock interval in unit of nano-seconds */
    *pucSlowClockCount_1000     = (UINT_8)((u4RegValue & BITS(24, 27)) >> 24);
    *pucSlowClockCount_100      = (UINT_8)((u4RegValue & BITS(20, 23)) >> 20);
    *pucSlowClockCount_10       = (UINT_8)((u4RegValue & BITS(16, 19)) >> 16);
    *pucSlowClockCount          = (UINT_8)((u4RegValue & BITS(0, 5))   >> 0);

    if (*pucSlowClockCount == 0) {
        fgStatus = FALSE;
        DBGLOG(HAL, ERROR, ("Slow clock period = 0 (need to ensure 32K clock is correctly provided)\n"));
        ASSERT(*pucSlowClockCount);
    }

    return fgStatus;
}   /* halpmCalSlowClock */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmConfigLowPowerSettingInBss (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16     u2AID,
    IN UINT_16     u2BcnIntv,
    IN UINT_8      ucDtimPeriod,
    IN UINT_8      bmfgApsdEnAc,
    IN UINT_8      ucMultiDtimWake,
    IN UINT_8      ucMultiTbttWake,
    IN BOOLEAN     fgUseAgeQoSNull,
    IN UINT_8      ucAgeNullPeriod,
    IN UINT_8      ucQoSNullTid,
    IN UINT_8      ucTrgThr,
    IN UINT_16     u2HwTxLifeTime
    )
{
#define CLEAR_PSPOLL_TIMEOUT_BY_RECEIVED_BEACON     0
#define CLEAR_APSD_TIMEOUT_BY_RECEIVED_BEACON       0
#define RESTART_PSPOLL_WITHIN_PSPOLL_TIMEOUT        0
#define RESTART_TRIGGER_FRAME_WITHIN_APSD_TIMEOUT   0

    UINT_32 u4Lpcr, u4Ttpcr, u4Atcr, u4Txcr;
    BOOLEAN fgBcnRspPspoll = TRUE;
    BOOLEAN fgTxNullReset = TRUE;
    UINT_32 u4RegDbgr, u4SleepTimeout;
    P_PM_INFO_T prPmInfo;

    DEBUGFUNC("halpmConfigLowPowerSettingInBss");

    ASSERT(prAdapter);
    ASSERT(ucMultiDtimWake);
    ASSERT(u2BcnIntv);
    prPmInfo = &prAdapter->rPmInfo;

    // Setup AID, HW TX lifetime
    HAL_MCR_RD(prAdapter, MCR_LPCR, &u4Lpcr);
    u4Lpcr &= ~(LPCR_AID_MASK | LPCR_TX_LIFE_TIME_MASK);
    HAL_MCR_WR(prAdapter,
                MCR_LPCR,
                u4Lpcr |
                (u2AID << 0) |
                (u2HwTxLifeTime << 12));

    // Setup beacon interval, wakeup period
#if 0
    HAL_MCR_RD(prAdapter, MCR_TTPCR, &u4Ttpcr);
    u4Ttpcr = (u4Ttpcr & TTPCR_TBTT_CAL_ENABLE) |
              TTPCR_TBTT_CAL_ENABLE |
              ((ucMultiDtimWake << 28) & TTPCR_DTIM_WAKE_PERIOD_MASK) |
              ((ucMultiTbttWake << 24) & TTPCR_TBTT_WAKE_PERIOD_MASK) |
              ((ucDtimPeriod << 16) & TTPCR_DTIM_PERIOD_MASK) |
              (u2BcnIntv & TTPCR_BEACON_PERIOD_MASK);
#else
    u4Ttpcr = TTPCR_TBTT_CAL_ENABLE |
              ((ucMultiDtimWake << 28) & TTPCR_DTIM_WAKE_PERIOD_MASK) |
              ((ucMultiTbttWake << 24) & TTPCR_TBTT_WAKE_PERIOD_MASK) |
              ((ucDtimPeriod << 16) & TTPCR_DTIM_PERIOD_MASK) |
              (u2BcnIntv & TTPCR_BEACON_PERIOD_MASK);
    HAL_MCR_WR(prAdapter, MCR_TTPCR, u4Ttpcr);
#endif

    /* Change TID for trigger frame */
    if (!(bmfgApsdEnAc & BIT(AC3))) {
        ucQoSNullTid = bmfgApsdEnAc & BITS(0, 3);
    }

    // Setup U-APSD configurations and aging NULL frame type (QoS-Null)
    HAL_MCR_RD(prAdapter, MCR_TXCR, &u4Txcr);
    u4Txcr &= ~(TXCR_NULL_TYPE_QOS | TXCR_QOS_NULL_TID | TXCR_PWRMGT_SET |
                TXCR_AC_TRIG_ENABLED_MASK | TXCR_AC_DLVR_ENABLED_MASK |
                TXCR_AC_TRIG_FUNCTION_ENABLE);
    u4Txcr |=
        (fgUseAgeQoSNull ? TXCR_NULL_TYPE_QOS : 0) |
        ((ucQoSNullTid << 20) & TXCR_QOS_NULL_TID);
    if (bmfgApsdEnAc) {
        u4Txcr |=
            ((GET_TRIG_EN_AC_INFO_FROM_APSD_BMP(bmfgApsdEnAc) << 12) & TXCR_AC_TRIG_ENABLED_MASK) |
            ((GET_DLVR_EN_AC_INFO_FROM_APSD_BMP(bmfgApsdEnAc) << 4)  & TXCR_AC_DLVR_ENABLED_MASK) |
            TXCR_AC_TRIG_FUNCTION_ENABLE;

        /* Use trigger packet for buffered UC indication in beacon */
        if (GET_DLVR_EN_AC_INFO_FROM_APSD_BMP(bmfgApsdEnAc) == BITS(0, 3)) {
            // All the AC are delivery-enabled, use trigger frame
            fgBcnRspPspoll = FALSE;
        }
    }
    HAL_MCR_WR(prAdapter, MCR_TXCR, u4Txcr);

    prPmInfo->ucWmmPsConnWithTrig = fgBcnRspPspoll ? FALSE : TRUE;

    // Setup aging NULL type, NULL period, DTIM period, beacon interval...
    HAL_MCR_RD(prAdapter, MCR_ATCR, &u4Atcr);
    u4Atcr &= (ATCR_BCN_TIMEOUT_COUNT_LIMIT | ATCR_TIMEOUT_COUNT_LIMIT);
    u4Atcr |=
#if CLEAR_PSPOLL_TIMEOUT_BY_RECEIVED_BEACON
             (ATCR_PSPOLL_END_SP_EN) |
#endif
#if CLEAR_APSD_TIMEOUT_BY_RECEIVED_BEACON
             (ATCR_TRIGGER_END_SP_EN) |
#endif
#if RESTART_PSPOLL_WITHIN_PSPOLL_TIMEOUT
             (ATCR_PSPOLL_NEW_SP_EN) |
#endif
#if RESTART_TRIGGER_FRAME_WITHIN_APSD_TIMEOUT
             (ATCR_TRIGGER_NEW_SP_EN) |
#endif
             ((GET_DLVR_EN_AC_INFO_FROM_APSD_BMP(bmfgApsdEnAc) & BIT(AC3)) ?
                    0 : ATCR_MNGT_PSPOLL_EN) |
             ((((ucAgeNullPeriod * 1000) /
                (ucMultiDtimWake *
                 ucDtimPeriod *
                 u2BcnIntv)) << 16) & ATCR_TX_NULL_INTERVAL) |
             (fgTxNullReset ? ATCR_TX_NULL_RESET_CTRL : 0) |
             (fgBcnRspPspoll ? 0 : ATCR_BCN_POLL_QOS_NULL) |
             ((ucTrgThr << 8) & ATCR_TRIGGER_THRESHOLD);
    HAL_MCR_WR(prAdapter, MCR_ATCR, u4Atcr);

    /* Modify the sleep timeout for LP watchdog */
    u4SleepTimeout = ucMultiDtimWake * ucDtimPeriod * u2BcnIntv;
    u4SleepTimeout >>= 10; // in unit of 1024 TU
    u4SleepTimeout += 2;//u4SleepTimeout += 1;
    DBGLOG(LP, INFO, ("ucMultiTbttWake(%d), ucMultiDtimWake(%d), ucDtimPeriod(%d), u2BcnIntv(%d), u4SleepTimeout(%ld*1024TU)\n",
        ucMultiTbttWake, ucMultiDtimWake, ucDtimPeriod, u2BcnIntv, u4SleepTimeout));

    ASSERT(u4SleepTimeout <= DBGR_SLEEP_TIMEOUT_COUNT);
    u4SleepTimeout &= DBGR_SLEEP_TIMEOUT_COUNT;

    HAL_MCR_RD(prAdapter, MCR_DBGR, &u4RegDbgr);
    u4RegDbgr &= ~DBGR_SLEEP_TIMEOUT_COUNT;
    u4RegDbgr |= DBGR_SLEEP_TIMEOUT_UNIT_1024_TU;
    HAL_MCR_WR(prAdapter, MCR_DBGR, u4RegDbgr | u4SleepTimeout);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmConfigLowPowerSettingInAdhoc (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_16      u2BcnIntv,
    IN UINT_16      u2AtimWindow,
    IN UINT_8       ucMultiTbttWake,
    IN UINT_16      u2HwTxLifeTime,
    IN BOOLEAN      fgTbttCalCompansate
    )
{
    UINT_32 u4Ttpcr, u4Txcr, u4Tstr;
    UINT_16 u2NextTbtt;
    UINT_32 u4RegDbgr, u4SleepTimeout;

    ASSERT(prAdapter);

    ASSERT(ucMultiTbttWake);
    ASSERT(u2BcnIntv);

    // Setup beacon interval, wakeup period, with also update next TBTT by current TSF
    HAL_MCR_RD(prAdapter, MCR_TTPCR, &u4Ttpcr);
    HAL_MCR_WR(prAdapter, MCR_TTPCR, 0);
    u4Ttpcr = (u4Ttpcr & TTPCR_TBTT_CAL_ENABLE) |
              TTPCR_TBTT_CAL_ENABLE |
              ((/*prIbssPmInfo->ucMultipleDtimWake*/0 << 28) & TTPCR_DTIM_WAKE_PERIOD_MASK) |
              ((ucMultiTbttWake << 24) & TTPCR_TBTT_WAKE_PERIOD_MASK) |
              ((/*prIbssPmInfo->ucDtimPeriod*/0 << 16) & TTPCR_DTIM_PERIOD_MASK) |
              (u2BcnIntv & TTPCR_BEACON_PERIOD_MASK);
    HAL_MCR_WR(prAdapter, MCR_TTPCR, u4Ttpcr);

#if 0
    if (fgTbttCalCompansate) {
#else
    // Always do SW compansation in the calibration procedure
    {
#endif
        HAL_MCR_RD(prAdapter, MCR_TSTR, &u4Tstr);
        u2NextTbtt = (UINT_16)u4Tstr + u2BcnIntv;
        HAL_MCR_WR(prAdapter, MCR_TSTR, ((u4Tstr & ~TSTR_TBTTSTARTTIME_MASK) |
                                         (UINT_32)u2NextTbtt));
    }

    if (u2AtimWindow == 0) {
        halpmSetAtimWindowValue(prAdapter, 0);

        HAL_MCR_RD(prAdapter, MCR_TXCR, &u4Txcr);
        u4Txcr &= ~TXCR_IBSS_Q_STOP;
        HAL_MCR_WR(prAdapter, MCR_TXCR, u4Txcr);
    }
    else {
        halpmSetAtimWindowValue(prAdapter, u2AtimWindow);

        HAL_MCR_RD(prAdapter, MCR_TXCR, &u4Txcr);
        u4Txcr |= TXCR_IBSS_Q_STOP;
        HAL_MCR_WR(prAdapter, MCR_TXCR, u4Txcr);
    }

    /* Modify the sleep timeout for LP watchdog */
    u4SleepTimeout = ucMultiTbttWake * u2BcnIntv;
    u4SleepTimeout >>= 10; // in unit of 1024 TU
    u4SleepTimeout += 2;

    ASSERT(u4SleepTimeout <= DBGR_SLEEP_TIMEOUT_COUNT);
    u4SleepTimeout &= DBGR_SLEEP_TIMEOUT_COUNT;

    HAL_MCR_RD(prAdapter, MCR_DBGR, &u4RegDbgr);
    u4RegDbgr &= ~DBGR_SLEEP_TIMEOUT_COUNT;
    u4RegDbgr |= DBGR_SLEEP_TIMEOUT_UNIT_1024_TU;
    HAL_MCR_WR(prAdapter, MCR_DBGR, u4RegDbgr | u4SleepTimeout);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmEnableLowPowerFunctionsInBss (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgNeedTbttWakeup
    )
{
    UINT_32 u4Mptcr;
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);
    prPmInfo = &prAdapter->rPmInfo;

    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4Mptcr);
    if (fgNeedTbttWakeup) {
        u4Mptcr |= (MPTCR_INFRASTRUCTURE_STA_SETTING_CONNECTED |
                    MPTCR_PRETBTT_TRIG_EN);
    }
    else {
        u4Mptcr |= MPTCR_INFRASTRUCTURE_STA_SETTING_CONNECTED;
    }

    // For WMM-PS test workaround (for full-UAPSD connection)
    if (prPmInfo->ucWmmPsDisableUcPoll && prPmInfo->ucWmmPsConnWithTrig) {
        u4Mptcr &= ~MPTCR_BCN_UC_EN;
    }

#if CFG_DBG_BEACON_RCPI_MEASUREMENT
    /* disable beacon content check, and allow beacon pass to driver */
    u4Mptcr &= ~MPTCR_BCN_CONTENT_CHK_EN;
#endif

#if CFG_LP_PATTERN_SEARCH_SLT
    /* 1. disable beacon content check, and allow beacon pass to driver
       2. wakeup host on beacon is received
    */
    if (prAdapter->eSLTModeSel == SLT_MODE_LP) {
        u4Mptcr &= ~(MPTCR_BCN_CONTENT_CHK_EN | MPTCR_BCN_PARSE_TIM_EN);
    }
#endif

    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4Mptcr);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmEnableLowPowerFunctionsInAdhoc (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgCreateIbss
    )
{
    UINT_32 u4Mptcr;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4Mptcr);
    if (fgCreateIbss) {
        u4Mptcr |= MPTCR_AD_HOC_STA_SETTING_CREATE;
    }
    else {
        u4Mptcr |= MPTCR_AD_HOC_STA_SETTING_CONNECTED;
    }
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4Mptcr);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmDisableLowPowerFunctions (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4RegValue;
    UINT_32 u4RegReservedMask = 0;

    ASSERT(prAdapter);

    /* disable AC trigger function, which might cause APSD-SP if trigger frame
       is transmitted successfully */
    HAL_MCR_RD(prAdapter, MCR_TXCR, &u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_TXCR, u4RegValue & ~TXCR_AC_TRIG_FUNCTION_ENABLE);


    /* Disable all of the LP function (except few reserved bits) */
    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);

#if !CFG_USE_SW_ROOT_TIMER
    u4RegReservedMask |= MPTCR_T2_TIMER_EN;   //T2 is reserved to SW root timer
#endif

    /* Note: SCAN may also be aborted if it is ongoing, due to it's not able to
             keep this function on/off with other bit changed in the register
    */
#if 0 //Reserved GPIO control?
    u4RegReservedMask |= (MPTCR_GPIO0_TRIGGER |
                          MPTCR_GPIO1_TRIGGER |
                          MPTCR_GPIO2_TRIGGER);
#endif

    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue & u4RegReservedMask);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetupWakeupGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2GuardTimePhase2,
    IN UINT_16 u2GuardTimePhase3
    )
{
    UINT_32 u4Tstr, u4Lpcr;

#if CFG_FPGA_COMPENSATE_HALF_T_FOR_GUARD_TIME_HEC_4489
    // for the 1/2 T compansation (25.6 us for 1 T)
    u2GuardTimePhase2 += 13;
    u2GuardTimePhase3 += 13;
#endif

    ASSERT(prAdapter);

    /* calculate TSTR.guard_interval */
    HAL_MCR_RD(prAdapter, MCR_TSTR, &u4Tstr);
    u4Tstr &= ~TSTR_GUARD_INTERVAL_MASK;
    u4Tstr |= (CEILING(u2GuardTimePhase2, 64) << 16) & TSTR_GUARD_INTERVAL_MASK;
    HAL_MCR_WR(prAdapter, MCR_TSTR, u4Tstr);

    /* calculate LPCR.LPStableTime */
    HAL_MCR_RD(prAdapter, MCR_LPCR, &u4Lpcr);
    u4Lpcr &= ~LPCR_LP_STABLE_TIME_MASK;
    u4Lpcr |= (CEILING(u2GuardTimePhase3, 8) << 24) & LPCR_LP_STABLE_TIME_MASK;
    HAL_MCR_WR(prAdapter, MCR_LPCR, u4Lpcr);

}   /* nicpmGetLowPowerStableTime */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmConfigPattern (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucIndex,
    IN BOOLEAN      fgCheckBcA1,
    IN BOOLEAN      fgCheckMcA1,
    IN BOOLEAN      fgCheckUcA1,
    IN BOOLEAN      fgIpv4Ip,
    IN BOOLEAN      fgIpv6Icmp,
    IN BOOLEAN      fgGarpIpEqual,
    IN BOOLEAN      fgArpCtrl,
    IN BOOLEAN      fgAndOp,
    IN BOOLEAN      fgNotOp,
    IN UINT_8       ucPatternMask,
    IN UINT_8       ucPatternOffset,
    IN PUINT_32     pu4Pattern
    )
{
    UINT_32 u4Pir;
    UINT_32 u4TransShift;
    UINT_8  ucTransPatternMask;
    UINT_32 pu4TransPattern[2];
    UINT_32 u4Value;

    ASSERT(prAdapter);

    if (!pu4Pattern) {
        // NOT TO USE assert here!
        return;
    }

    /* NOTE:
       SW need to reorder the ucPatternMask and pu4Pattern due to HW optimization
       of the pattern search engine by ucPatternOffset
    */
    u4TransShift = (UINT_32)ucPatternOffset % 8;
    if (u4TransShift) {
        ucTransPatternMask =
            (ucPatternMask << u4TransShift) |
            (ucPatternMask >> (8 - u4TransShift));
        kalMemCopy(pu4TransPattern,
                   (PUINT_8)((UINT_32)pu4Pattern + u4TransShift),
                   8 - u4TransShift);
        kalMemCopy((PUINT_8)((UINT_32)pu4TransPattern + u4TransShift),
                   pu4Pattern,
                   u4TransShift);
    }
    else {
        ucTransPatternMask = ucPatternMask;
        kalMemCopy(pu4TransPattern, pu4Pattern, 8);
    }

    // index
    HAL_MCR_RD(prAdapter, MCR_PIR, &u4Pir);
    u4Pir &= ~PIR_PATTERN_INDEX;
    HAL_MCR_WR(prAdapter, MCR_PIR, u4Pir | PIR_PATTERN_WRITE_MODE | ucIndex);

    // opeations
    u4Value = 0;
    u4Value = fgCheckBcA1         << 24 |
              fgCheckMcA1         << 23 |
              fgCheckUcA1         << 22 |
              fgIpv4Ip            << 21 |
              fgIpv6Icmp          << 20 |
              fgGarpIpEqual       << 19 |
              fgArpCtrl           << 18 |
              fgAndOp             << 17 |
              fgNotOp             << 16 |
              ucTransPatternMask  <<  8 |
              ucPatternOffset     <<  0;
    HAL_MCR_WR(prAdapter, MCR_PMR, u4Value);

    // pattern
    HAL_MCR_WR(prAdapter, MCR_PPR, pu4TransPattern[0]);
    HAL_MCR_WR(prAdapter, MCR_PPR, pu4TransPattern[1]);
}

#if 1
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmConfigPatternSearchFunction (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgBcA1En,
    IN BOOLEAN      fgMcA1En,
    IN BOOLEAN      fgUcA1En,
    IN BOOLEAN      fgBcA1MatchDrop,
    IN BOOLEAN      fgMcA1MatchDrop,
    IN BOOLEAN      fgUcA1MatchDrop,
    IN BOOLEAN      fgIpv6MatchCtrl
    )
{
    UINT_32 u4InstSleep = 0;
    UINT_32 u4InstOn = 0;
    UINT_32 u4Value;
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;


    /* NOTE:
        Currently, it is assume that pattern search is enabled during LP (WLAN_on),
        and is disabled in ON state. This includes patterns with different A1 (UC/ BC/ MC),
        with disregarding whether to drop or receive for the matched packets or not.
    */

    HAL_MCR_RD(prAdapter, MCR_PIR, &u4Value);
    u4Value &= ~(PIR_BC_PATTERN_SRCH_EN |
                 PIR_MC_PATTERN_SRCH_EN |
                 PIR_UC_PATTERN_SRCH_EN |
                 PIR_BC_MATCHING_OPERATION |
                 PIR_MC_MATCHING_OPERATION |
                 PIR_UC_MATCHING_OPERATION |
                 PIR_IPV6_FRAME_MATCH_CTRL);
    if (fgIpv6MatchCtrl) {
        u4Value |= PIR_IPV6_FRAME_MATCH_CTRL;
    }
    HAL_MCR_WR(prAdapter, MCR_PIR, u4Value);

    u4InstSleep = (fgBcA1En << 22) |
                  (fgMcA1En << 21) |
                  (fgUcA1En << 20) |
                  (fgBcA1MatchDrop << 18) |
                  (fgMcA1MatchDrop << 17) |
                  (fgUcA1MatchDrop << 16);

    if (prPmInfo->ucNumOfInstSleep < NUM_OF_MAX_LP_INSTRUCTIONS) {
        prPmInfo->au4LowPowerInst_sleep[prPmInfo->ucNumOfInstSleep++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_PIR, TRUE, (UINT_16)(u4InstSleep >> 16));
    }
    else {
        ERRORLOG(("NUM_OF_MAX_LP_INSTRUCTIONS(%d), prPmInfo->ucNumOfInstSleep(%d)\n",
            NUM_OF_MAX_LP_INSTRUCTIONS,
            prPmInfo->ucNumOfInstSleep));
        ASSERT(FALSE);
    }

    if (prAdapter->ucRevID == MTK_CHIP_MP_REVERSION_ID) {
        if (prPmInfo->ucNumOfInstSleep < NUM_OF_MAX_LP_INSTRUCTIONS) {
            prPmInfo->au4LowPowerInst_sleep[prPmInfo->ucNumOfInstSleep++] = LP_INST_DELAY(25, FALSE, FALSE);
        }
        else {
            ERRORLOG(("MTK_CHIP_MP_REVERSION_ID NUM_OF_MAX_LP_INSTRUCTIONS(%d), prPmInfo->ucNumOfInstSleep(%d)\n",
                NUM_OF_MAX_LP_INSTRUCTIONS,
                prPmInfo->ucNumOfInstSleep));
            ASSERT(FALSE);
        }
    }

#if CFG_LP_PATTERN_SEARCH_SLT
    if (prAdapter->eSLTModeSel == SLT_MODE_PATTERN_SEARCH) {
        /* Originally pattern search function is only enabled in sleep mode instruction.
          * Currentlly we would enable pattern search function in WLAN_ON instruction.
          */
        prPmInfo->au4LowPowerInst_on[prPmInfo->ucNumOfInstOn++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_PIR, TRUE, (UINT_16)(u4InstSleep >> 16));
    }
    else {
        prPmInfo->au4LowPowerInst_on[prPmInfo->ucNumOfInstOn++] =
            LP_INST_MCR_WR_HALF_WORD(MCR_PIR, TRUE, (UINT_16)(u4InstOn >> 16));
    }
#else
    prPmInfo->au4LowPowerInst_on[prPmInfo->ucNumOfInstOn++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_PIR, TRUE, (UINT_16)(u4InstOn >> 16));
#endif

    // Low power instruction programming
    NIC_PM_PROGRAM_LP_INSRUCTION(prAdapter, prPmInfo->fgIsContinousPollingEnabled);

#if CFG_LP_PATTERN_SEARCH_SLT
    if (prAdapter->eSLTModeSel == SLT_MODE_PATTERN_SEARCH) {
        HAL_MCR_WR(prAdapter, MCR_PIR, u4Value | u4InstSleep);
    }
#endif

} /* halpmConfigPatternSearchFunction */
#else
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmConfigPatternSearchFunction (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgBcA1En,
    IN BOOLEAN      fgMcA1En,
    IN BOOLEAN      fgUcA1En,
    IN BOOLEAN      fgBcA1MatchDrop,
    IN BOOLEAN      fgMcA1MatchDrop,
    IN BOOLEAN      fgUcA1MatchDrop,
    IN BOOLEAN      fgIpv6MatchCtrl
    )
{
    UINT_32 u4InstSleep = 0;
    UINT_32 u4InstOn = 0;
    UINT_32 u4Value;
    P_PM_INFO_T prPmInfo;

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;


    /* NOTE:
        Currently, it is assume that pattern search is enabled during LP (WLAN_on),
        and is disabled in ON state. This includes patterns with different A1 (UC/ BC/ MC),
        with disregarding whether to drop or receive for the matched packets or not.
    */

    HAL_MCR_RD(prAdapter, MCR_PIR, &u4Value);
    u4Value &= ~(PIR_BC_PATTERN_SRCH_EN |
                 PIR_MC_PATTERN_SRCH_EN |
                 PIR_UC_PATTERN_SRCH_EN |
                 PIR_BC_MATCHING_OPERATION |
                 PIR_MC_MATCHING_OPERATION |
                 PIR_UC_MATCHING_OPERATION |
                 PIR_IPV6_FRAME_MATCH_CTRL);
    if (fgIpv6MatchCtrl) {
        u4Value |= PIR_IPV6_FRAME_MATCH_CTRL;
    }
    HAL_MCR_WR(prAdapter, MCR_PIR, u4Value);

    u4InstSleep = (fgBcA1En << 22) |
                  (fgMcA1En << 21) |
                  (fgUcA1En << 20) |
                  (fgBcA1MatchDrop << 18) |
                  (fgMcA1MatchDrop << 17) |
                  (fgUcA1MatchDrop << 16);
    prPmInfo->au4LowPowerInst_sleep[prPmInfo->ucNumOfInstSleep++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_PIR, TRUE, (UINT_16)(u4InstSleep >> 16));

    if (prAdapter->ucRevID == MTK_CHIP_MP_REVERSION_ID) {
        prPmInfo->au4LowPowerInst_sleep[prPmInfo->ucNumOfInstSleep++] = LP_INST_DELAY(25, FALSE, FALSE);
    }

    prPmInfo->au4LowPowerInst_on[prPmInfo->ucNumOfInstOn++] =
        LP_INST_MCR_WR_HALF_WORD(MCR_PIR, TRUE, (UINT_16)(u4InstOn >> 16));

    // Low power instruction programming
    NIC_PM_PROGRAM_LP_INSRUCTION(prAdapter);

} /* halpmConfigPatternSearchFunction */
#endif
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
halpmIfAdhocStaMaster (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4Value;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_CIR, &u4Value);

    return ((u4Value & CIR_IBSS_MASTER) ? TRUE : FALSE);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmPowerOn (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 uc32kSlowCount
    )
{
    UINT_32 u4RegValue = 0;

    ASSERT(prAdapter);

    /* NOTE:
       Now uc32kSlowCount is used for calculating the delay time for different phases,
       and this value might be 0 on initial,
       if 32KHz clock is not calibrated before nor the clock is off.
    */
    //ASSERT(uc32kSlowCount);   // remove the assertion in purpose!

    /* NOTE:
       We should gate PLL first and then disable PLL.
       MCR_CIR.PLL_ready reflects current PLL gating status (not the PLL enable status).
    */
    // Enable PLL gating and disable all clock source (except OSC_en)
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_OSC_OUT_PD |         // bit14
                                     HLPCR_BG_PD |              // bit13
                                     HLPCR_PLL_PD |             // bit12
                                     HLPCR_ADC_BUFFER_PD |      // bit11
                                     HLPCR_INTERNAL_32K_PD |    // bit10
                                     HLPCR_EXTERNAL_32K_PD |    // bit9
                                     HLPCR_RF_SX_PD |           // bit8
                                     HLPCR_PLL_CLOCK_GATED);

    // logic reset (HIFMAC: bit 29, BB: B25)
    HAL_MCR_WR(prAdapter, MCR_HLPCR,
                HLPCR_PLL_CLOCK_GATED |
                HLPCR_BB_LOGRST | HLPCR_HIFMAC_LOGRST | HLPCR_LP_OWN_CLR);
    HAL_MCR_WR(prAdapter, MCR_HLPCR,
                HLPCR_PLL_CLOCK_GATED | HLPCR_OSC_EN);
    kalUdelay((UINT_32)LP_CLOCK_ON_DELAY_P1_US(uc32kSlowCount));
#if USE_INTERNAL_32K_RING_OSC
    HAL_MCR_WR(prAdapter, MCR_HLPCR,
                HLPCR_PLL_CLOCK_GATED |
                HLPCR_BG_EN | HLPCR_INTERNAL_32K_EN | HLPCR_RF_SX_EN);// BG/ 32K/ RF
#else
    HAL_MCR_WR(prAdapter, MCR_HLPCR,
                HLPCR_PLL_CLOCK_GATED |
                HLPCR_BG_EN | HLPCR_EXTERNAL_32K_EN | HLPCR_RF_SX_EN);// BG/ 32K/ RF
#endif
    kalUdelay((UINT_32)LP_CLOCK_ON_DELAY_P2_US(uc32kSlowCount));
    HAL_MCR_WR(prAdapter, MCR_HLPCR,
                HLPCR_PLL_CLOCK_GATED |
                HLPCR_OSC_OUT_EN | HLPCR_PLL_EN | HLPCR_ADC_BUFFER_EN);
    kalUdelay((UINT_32)LP_CLOCK_ON_DELAY_P3_US(uc32kSlowCount));
    HAL_MCR_WR(prAdapter, MCR_HLPCR, 0);// disable CLOCK gating


    // restore trap pin to output mode
    HAL_MCR_RD(prAdapter, MCR_IOPCR, &u4RegValue);
    u4RegValue |= (IO_SET_TRAP_PIN_DEFAULT_ATTR | IO_SET_DBG_PORT_SEL);
#if PTA_ENABLED
    #if PTA_NEW_BOARD_DESIGN
    HAL_MCR_WR(prAdapter,
               MCR_IOPCR,
               u4RegValue);
    #else
    HAL_MCR_WR(prAdapter,
               MCR_IOPCR,
               u4RegValue
                |(prAdapter->rPtaInfo.fgSingleAntenna) ? (IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR |
                                                     IOPCR_IO_TR_SW_P_DIR | IOPCR_IO_TR_SW_N_DIR) : 0);
    #endif
#else
    HAL_MCR_WR(prAdapter,
               MCR_IOPCR,
               u4RegValue);

#endif
#if PTA_ENABLED
    if (prAdapter->rPtaInfo.fgSingleAntenna) {
        if (prAdapter->rPtaInfo.u4PTAWireMode == PTA_SW_1_WIRE_MODE) {
            /* IOUDR */
            HAL_MCR_RD(prAdapter, MCR_IOUDR, &u4RegValue);
            u4RegValue &= ~(IOUDR_BT_PRI_PD | IOUDR_BT_PRI_PU);
            HAL_MCR_WR(prAdapter, MCR_IOUDR, u4RegValue);
        }

        /* RICR */
        HAL_MCR_RD(prAdapter, MCR_RICR, &u4RegValue);
        u4RegValue &= ~(RICR_RF_SW_MODE         |
                        RICR_ANT_SEL_P_SW_MODE  |
                        RICR_ANT_SEL_N_SW_MODE);

        u4RegValue |= (RICR_PA2EN               |
                       RICR_PA5EN);
        HAL_MCR_WR(prAdapter, MCR_RICR, u4RegValue);

        if (prAdapter->rPtaInfo.eCurrentState == PTA_STATE_ON) {
            /* BTCER0 */
            HAL_MCR_RD(prAdapter, MCR_BTCER0, &u4RegValue);
            u4RegValue |= (PTA_BTCER0_COEXIST_EN);
            HAL_MCR_WR(prAdapter, MCR_BTCER0, u4RegValue);

#if !PTA_NEW_BOARD_DESIGN
            /* GeorgeKuo(20090805): new single-antenna design. */
            nicPtaSetAnt(prAdapter, FALSE);
#endif
        }
        else {
#if !PTA_NEW_BOARD_DESIGN
            /* GeorgeKuo(20090805): new single-antenna design. */
            nicPtaSetAnt(prAdapter, TRUE);
#endif
        }
    }
    else { /* Dual antenna */

        if (prAdapter->rPtaInfo.u4PTAWireMode == PTA_SW_1_WIRE_MODE) {
            /* IOUDR */
            HAL_MCR_RD(prAdapter, MCR_IOUDR, &u4RegValue);
            u4RegValue &= ~(IOUDR_BT_PRI_PD | IOUDR_BT_PRI_PU);
            HAL_MCR_WR(prAdapter, MCR_IOUDR, u4RegValue);
        }
        if (prAdapter->rPtaInfo.eCurrentState == PTA_STATE_ON) {
            /* BTCER0 */
            HAL_MCR_RD(prAdapter, MCR_BTCER0, &u4RegValue);
            u4RegValue |= (PTA_BTCER0_COEXIST_EN);
            HAL_MCR_WR(prAdapter, MCR_BTCER0, u4RegValue);
        }
    }
#endif
    /* Note: place the instruction if GPIO or other pin need to be pull high/ low or
             set to input mode for preventing current leakage here...
    */


    /* turn on LED */
#if USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER
    {
        P_CONNECTION_SETTINGS_T prConnSettings = &prAdapter->rConnSettings;
        if (prConnSettings->ucLedBlinkMode) {
            HAL_MCR_RD(prAdapter, MCR_LCR, &u4RegValue);
            HAL_MCR_WR(prAdapter, MCR_LCR, u4RegValue | (LCR_LED_OUTPUT));
        }
    }
#else
    /* LED Control */
    u4RegValue = 0;   
    u4RegValue &= ~LCR_LED_POLARITY; /* Active High */

    /* set LED timing */
    u4RegValue |= ((prAdapter->prGlueInfo->u4LedBlinkOnTime << 8) & LCR_LED_ON_CONT) |
                  (prAdapter->prGlueInfo->u4LedBlinkOffTime & LCR_LED_OFF_CONT);    

    /* set LED Mode */
  
    switch(prAdapter->prGlueInfo->u2LedBlinkMode)
    {
        case CFG_LED_MODE_NONE :
            u4RegValue = 0;
            break;
        case CFG_LED_MODE_TX :
            u4RegValue |= LCR_LED_MODE_TX_WO_BEACON | LCR_LED_MODE_TX_BEACON;
            break;
        case CFG_LED_MODE_RX :
            u4RegValue |= LCR_LED_MODE_RX_EX_RFCR_BEACON | LCR_LED_MODE_RX_RFCR_BEACON;
            break;
        case CFG_LED_MODE_TX_RX :
            u4RegValue |= LCR_LED_MODE_TX_WO_BEACON | LCR_LED_MODE_TX_BEACON | LCR_LED_MODE_RX_EX_RFCR_BEACON | LCR_LED_MODE_RX_RFCR_BEACON;
            break;    
        default :
            u4RegValue = 0;
            break;            
    }
    
    prAdapter->prGlueInfo->u4LedSetting = u4RegValue;

    //u4RegValue |= LCR_LED_MODE_TX_WO_BEACON | LCR_LED_MODE_TX_BEACON | LCR_LED_MODE_RX_EX_RFCR_BEACON | LCR_LED_MODE_RX_RFCR_BEACON;
    //kal_prompt_trace(MOD_WNDRV, "halpmPowerOn MCR_LCR : %x", u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_LCR, u4RegValue);
#endif    

    /* clear interrupt status, due to logic reset is done in previous step
       (halpmPowerOn) */
    HAL_MCR_RD(prAdapter, MCR_HISR, &u4RegValue);
    HAL_MCR_RD(prAdapter, MCR_HSCISR, &u4RegValue);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmPowerOff (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    /* turn off LED */
#if USE_DEBUG_LED_SIGNAL_FOR_LOW_POWER
    HAL_MCR_RD(prAdapter, MCR_LCR, &u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_LCR, u4RegValue & ~LCR_LED_OUTPUT);
#else    
    u4RegValue = 0;
    HAL_MCR_WR(prAdapter, MCR_LCR, u4RegValue);
#endif

    /* Note: place the instruction if GPIO or other pin need to be pull high/ low or
             set to input mode for preventing current leakage here...
    */
#if PTA_ENABLED
    if (prAdapter->rPtaInfo.fgSingleAntenna) {
        /* RICR */
        HAL_MCR_RD(prAdapter, MCR_RICR, &u4RegValue);
        u4RegValue &= ~(RICR_SWANT_SEL_N_HIGH   |
                        RICR_PA2EN              |
                        RICR_PA5EN              |
                        RICR_SW_TR_SW_N         |
                        RICR_SW_RXADC_DCCAL_EN  |
                        RICR_SW_RF_TX           |
                        RICR_SW_RF_RX           |
                        RICR_SW_TR_SW_P         |
                        RICR_SW_TX_PE           |
                        RICR_SW_RX_PE);

        u4RegValue |= (RICR_RF_SW_MODE          |
                       RICR_ANT_SEL_P_SW_MODE   |
                       RICR_ANT_SEL_N_SW_MODE   |
                       RICR_SWANT_SEL_P_HIGH);
        HAL_MCR_WR(prAdapter, MCR_RICR, u4RegValue);

        /* BTCER1 */
        HAL_MCR_RD(prAdapter, MCR_BTCER1, &u4RegValue);
        u4RegValue &= ~(PTA_BTCER1_SINGLE_ANT);
        HAL_MCR_WR(prAdapter, MCR_BTCER1, u4RegValue);

        if (prAdapter->rPtaInfo.eCurrentState == PTA_STATE_ON) {
            /* BTCER0 */
            HAL_MCR_RD(prAdapter, MCR_BTCER0, &u4RegValue);
            u4RegValue &= ~(PTA_BTCER0_COEXIST_EN);
            HAL_MCR_WR(prAdapter, MCR_BTCER0, u4RegValue);
        }

        if (prAdapter->rPtaInfo.u4PTAWireMode == PTA_SW_1_WIRE_MODE) {
            /* IOUDR */
            HAL_MCR_RD(prAdapter, MCR_IOUDR, &u4RegValue);
            u4RegValue &= ~(IOUDR_BT_PRI_PD | IOUDR_BT_PRI_PU);
            u4RegValue |= IOUDR_BT_PRI_PD;
            HAL_MCR_WR(prAdapter, MCR_IOUDR, u4RegValue);
        }
    }
    else { /* Dual antenna */
        if (prAdapter->rPtaInfo.eCurrentState == PTA_STATE_ON) {
            /* BTCER0 */
            HAL_MCR_RD(prAdapter, MCR_BTCER0, &u4RegValue);
            u4RegValue &= ~(PTA_BTCER0_COEXIST_EN);
            HAL_MCR_WR(prAdapter, MCR_BTCER0, u4RegValue);
        }

        if (prAdapter->rPtaInfo.u4PTAWireMode == PTA_SW_1_WIRE_MODE) {
            /* IOUDR */
            HAL_MCR_RD(prAdapter, MCR_IOUDR, &u4RegValue);
            u4RegValue &= ~(IOUDR_BT_PRI_PD | IOUDR_BT_PRI_PU);
            u4RegValue |= IOUDR_BT_PRI_PD;
            HAL_MCR_WR(prAdapter, MCR_IOUDR, u4RegValue);
        }
    }
#endif
    /* set all trap pin to input mode to prevent current leakage.
     * (keep AntSel as the user configured value)
     */
    HAL_MCR_RD(prAdapter, MCR_IOPCR, &u4RegValue);
    u4RegValue &= ~IOPCR_ALL_TRAP_PIN_OUTPUT_EN;
#if PTA_ENABLED
    #if PTA_NEW_BOARD_DESIGN
    HAL_MCR_WR(prAdapter,
               MCR_IOPCR,
               u4RegValue);
    #else
    HAL_MCR_WR(prAdapter,
               MCR_IOPCR,
               u4RegValue |
               (prAdapter->rPtaInfo.fgSingleAntenna) ? (IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR |
                                                     IOPCR_IO_TR_SW_P_DIR | IOPCR_IO_TR_SW_N_DIR) : 0);
    #endif
#else
    HAL_MCR_WR(prAdapter,
               MCR_IOPCR,
               u4RegValue);
#endif

    /* NOTE:
       32KHz clock (internal/ external) is turned off in this procedure.
       But slow clock calibration procedure is only done in PM module initialization
       procedure now.
    */

    /* NOTE:
       We should gate PLL first and then disable PLL.
       MCR_CIR.PLL_ready reflects current PLL gating status (not the PLL enable status).
    */
    // Enable PLL gating and disable RF SX
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_RF_SX_PD | HLPCR_PLL_CLOCK_GATED);
    // Disable all clock sources, and enable PLL gating
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_PD_ALL | HLPCR_PLL_CLOCK_GATED);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmEnableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4RegSpcr;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4RegSpcr);
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4RegSpcr & ~SPCR_TO_COUNTER_RESET_CTRL);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmDisableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4RegSpcr;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4RegSpcr);
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4RegSpcr | SPCR_TO_COUNTER_RESET_CTRL);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetTsfDriftWindow (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8    ucDriftWindow
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_TTAR, &u4RegValue);
    u4RegValue &= ~TTAR_TSF_TSF_DRIFT_WINDOW ;
    HAL_MCR_WR(prAdapter, MCR_TTAR, u4RegValue | (ucDriftWindow << 16));

    HAL_MCR_RD(prAdapter, MCR_LPCR, &u4RegValue);
    u4RegValue &= ~LPCR_TSF_DRIFT_INTR_EN;
    u4RegValue |= (ucDriftWindow == 0) ? 0 : LPCR_TSF_DRIFT_INTR_EN;
    HAL_MCR_WR(prAdapter, MCR_LPCR, u4RegValue);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetAtimWindowValue (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16    u2AtimWindow
    )
{
    ASSERT(prAdapter);

    HAL_MCR_WR(prAdapter, MCR_ACWR, u2AtimWindow);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmResetTSFTimer (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    HAL_MCR_WR(prAdapter, MCR_UTTR0, 0);
    HAL_MCR_WR(prAdapter, MCR_UTTR1, 0);
    HAL_MCR_WR(prAdapter, MCR_TTAR, TTAR_TSF_TIMER_VALUE_CHANGE);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetupBeaconContentCheck (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgEnable
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);
    u4RegValue &= ~MPTCR_BCN_CONTENT_CHK_EN;
    u4RegValue |= (fgEnable) ? MPTCR_BCN_CONTENT_CHK_EN : 0;
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetupMoreDataTrigger (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    )
{
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);
    u4RegValue &= ~MPTCR_MORE_TRIG_EN;
    u4RegValue |= (fgEnable) ? MPTCR_MORE_TRIG_EN : 0;
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halpmSetupBeaconTimeoutDetection (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    )
{
    BOOLEAN fgBcnSpInvalid;
    UINT_32 u4RegValue;

    ASSERT(prAdapter);

    /* invalid SP before setting beacon timeout disable */
    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4RegValue);
    fgBcnSpInvalid = (u4RegValue & SPCR_BEACON_SP_INVALID_MASK) ? TRUE : FALSE;
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4RegValue | SPCR_BEACON_SP_INVALID_MASK);

    HAL_MCR_RD(prAdapter, MCR_MPTCR, &u4RegValue);
    u4RegValue &= ~MPTCR_BCN_TIMEOUT_EN;
    u4RegValue |= (fgEnable) ? MPTCR_BCN_TIMEOUT_EN : 0;
    HAL_MCR_WR(prAdapter, MCR_MPTCR, u4RegValue);

    /* restore SP before setting beacon timeout disable */
    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4RegValue);
    if (!fgBcnSpInvalid) {
        u4RegValue &= ~SPCR_BEACON_SP_INVALID_MASK;
    }
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4RegValue);

    /* reset timeout counter */
    HAL_MCR_RD(prAdapter, MCR_SPCR, &u4RegValue);
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4RegValue | SPCR_TO_COUNTER_RESET_CTRL);
    HAL_MCR_WR(prAdapter, MCR_SPCR, u4RegValue & ~SPCR_TO_COUNTER_RESET_CTRL);
}

