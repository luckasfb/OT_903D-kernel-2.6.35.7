





#ifndef _CONFIG_H
#define _CONFIG_H


#ifdef	MTK_WAPI_SUPPORT
	#define SUPPORT_WAPI 1
#else
	#define SUPPORT_WAPI 0
#endif
//2 Flags for OS capability
#define CFG_TCP_IP_CHKSUM_OFFLOAD               1 /* 1: Enable TCP/IP header checksum offload
                                                     0: Disable */

//2 Flags for Driver Features
#define CFG_TX_FRAGMENT                         1 /* 1: Enable TX fragmentation
                                                     0: Disable */

#define CFG_ROAMING                             1 /* 1(default): Enable Roaming
                                                     0: Disable */

#define CFG_IBSS_POWER_SAVE                     1 /* 1(default): Enable IBSS Power Save
                                                     0: Disable */

#define CFG_COUNTRY_CODE                        NULL //"US"

#define CFG_SUPPORT_802_11D                     1 /* 1(default): Enable 802.11d
                                                     0: Disable */

#define CFG_IBSS_POWER_SAVE_WITH_THROUGHPUT_ENHANCE  1 /* 1(default): Enable IBSS Power Save with
                                                                      throughput enhancement.
                                                          0: Disable */

#if BUILD_USE_EEPROM
#define CFG_SUPPORT_EXT_CONFIG      0 /* 1: Enable external configuration support (ex. NVRAM or file)
                                         0: Disable. Use EEPROM configuration. */
#else
#define CFG_SUPPORT_EXT_CONFIG      1 /* 1: Enable external configuration support (ex. NVRAM or file)
                                         0: Disable. Use EEPROM configuration. */
#endif

#if BUILD_META
#define CFG_SDIO_DEVICE_DRIVER_WO_NDIS                         1    /* 1: Enable: compile MT5921 to be a SDIO device I/O driver.
                                                                                 0: Disabled: compile MT5921 to be a miniport driver as usual. */
#else
#define CFG_SDIO_DEVICE_DRIVER_WO_NDIS                         0    /* 1: Enable: compile MT5921 to be a SDIO device I/O driver.
                                                                                 0: Disabled: compile MT5921 to be a miniport driver as usual. */
#endif

//2 Flags for Driver Parameters
#define CFG_EHPI_FASTER_BUS_TIMING              0 /* 1: Do workaround for faster bus timing
                                                     0(default): Disable */

#if defined(_HIF_SDIO)
    #define CFG_SDIO_STATUS_ENHANCE                 1 /* 1(default): Enable SDIO ISR & TX/RX status enhance mode
                                                         0: Disable */
    #define CFG_SDIO_TX_ENHANCE                     0 /* 1: Enable SDIO TX enhance mode(Multiple frames in single BLOCK CMD)
                                                         0(default): Disable */
    #define CFG_SDIO_RX_ENHANCE                     0 /* 1: Enable SDIO RX enhance mode(Multiple frames in single BLOCK CMD)
                                                         0(default): Disable */

    #if CFG_SDIO_TX_ENHANCE || CFG_SDIO_RX_ENHANCE
        #define CFG_SDIO_STATUS_ENHANCE             1
    #endif /* CFG_SDIO_TX_ENHANCE || CFG_SDIO_RX_ENHANCE */

    #define CFG_SDIO_DEBUG_AGGREGATING_RATIO        0 /* 1: Enable SDIO counter to check aggregating ratio
                                                         0(default): Disable */

    #define CFG_SDIO_FAST_BLOCK_CALCULATION_MODE    0 /* 1: Enable fast block calculation(No division).
                                                         0(default): Disable */

    #ifdef WINDOWS_CE
    #define CFG_SDIO_PATHRU_MODE                    1 /* 1: Suport pass through (PATHRU) mode
                                                         0: Disable */
    #else
    #define CFG_SDIO_PATHRU_MODE                    0 /* 0: Always disable if WINDOWS_CE is not defined */
    #endif
#else /* NOT SDIO */

    #define CFG_SDIO_STATUS_ENHANCE                 0 /* 0: Always disable if !_HIF_SDIO */
    #define CFG_SDIO_TX_ENHANCE                     0 /* 0: Always disable if !_HIF_SDIO */
    #define CFG_SDIO_RX_ENHANCE                     0 /* 0: Always disable if !_HIF_SDIO */
    #define CFG_SDIO_DEBUG_AGGREGATING_RATIO        0 /* 0: Always disable if !_HIF_SDIO */
    #define CFG_SDIO_PATHRU_MODE                    0 /* 0: Always disable if !_HIF_SDIO */

#endif /* _HIF_SDIO */

#define  CFG_EINT_HANDLED_IN_WLAN   1
//#undef   CFG_EINT_HANDLED_IN_WLAN	
#if defined(LINUX)
    #define CFG_TX_BUFFER_IS_SCATTER_LIST       0 /* 1: Do frame copy before write to TX FIFO.
                                                        Used when Network buffer is scatter-gather.
                                                     0(default): Do not copy frame */
#else /* WINDOWS/WINCE */
    #define CFG_TX_BUFFER_IS_SCATTER_LIST       1
#endif /* LINUX */


/* NOTE: We copy frame to coalescing buffer before doing fragment process. */
#if CFG_TX_FRAGMENT
    #define CFG_FRAGMENT_COALESCING_BUFFER_SIZE 2048
#else
    #define CFG_FRAGMENT_COALESCING_BUFFER_SIZE 0
#endif /* CFG_TX_FRAGMENT  */


/* NOTE: The definition of CFG_COALESCING_BUFFER_SIZE is dependent. */
#if CFG_SDIO_TX_ENHANCE
    #define CFG_COALESCING_BUFFER_SIZE          8192
#elif CFG_TX_BUFFER_IS_SCATTER_LIST && !CFG_TX_FRAGMENT
    #define CFG_COALESCING_BUFFER_SIZE          2048
#else
    #define CFG_COALESCING_BUFFER_SIZE          0
#endif /* CFG_SDIO_TX_ENHANCE  */



/* Maximum number of SW MSDU_INFO_T(Native Packet descriptor) for individual Traffic Class */
#define CFG_MAX_NUM_MSDU_INFO_FOR_TC0           10 /* number */
#define CFG_MAX_NUM_MSDU_INFO_FOR_TC1           10
#define CFG_MAX_NUM_MSDU_INFO_FOR_TC2           10
#define CFG_MAX_NUM_MSDU_INFO_FOR_TC3           10
#define CFG_MAX_NUM_MSDU_INFO_FOR_TS0           10
#if CFG_IBSS_POWER_SAVE
    /* NOTE(Kevin): Send ATIM by using TCM class of SW resource.
     */
    #define CFG_MAX_NUM_MSDU_INFO_FOR_TCM       24
#else
    #define CFG_MAX_NUM_MSDU_INFO_FOR_TCM       10
#endif /* CFG_IBSS_POWER_SAVE */

/* TODO(Kevin): Will refine this value with the size of coalescing buffer */
#define CFG_TX_MAX_PKT_SIZE                     1600


#define CFG_TX_RET_TX_CTRL_EARLY                1 /* 1(default): Free the original TX buffer immediately
                                                        once it copy to coalescing buffer.
                                                     0: Free the original TX buffer after TX DONE */

#define CFG_TX_AGGREGATE_HW_FIFO                0 /* 1: When associated to Legacy AP(nonQoS),
                                                        do aggregate AC0/1/2 to AC2.
                                                     0(default): Arrange TX FIFO in QoS Style */

#define CFG_TX_STOP_WRITE_TX_FIFO_UNTIL_JOIN    0 /* 1: During roaming, stop write packets to HW
                                                        FIFO after join complete.
                                                        When join complete, packets in HW TX FIFO will
                                                        be flushed first, and then enable the flag to
                                                        allow packets to be writen to HW FIFO.

                                                        NOTE(Kevin): We should set to 1 for WM7. 
                                                          Because it use Native 802.11 frame format, thus
                                                          the packets in HW FIFO before join complete still
                                                          has RA equal to previous AP while roaming.

                                                     0: Packet can write to HW FIFO during JOIN.

                                                        NOTE(Kevin): Consider the roaming case for VOIP - If
                                                          we set to 0, we will still buffer VOIP datagram
                                                          to HW FIFO, however if it may takes long time to
                                                          finish the EAPOL to new AP and those VOIP datagram
                                                          will timeout by Application. So we may feel that 
                                                          set to 1 to this flag may has better performance
                                                          for roaming VOIP.
                                                          <TODO> Apply different lifetime to differ HW Queue */


/* Maximum number of SW RX packet buffer */
#define CFG_RX_MAX_PKT_NUM                      10

/* A threshold to retain RX packet buffer */
#define CFG_RX_MGMT_PKT_NUM_THRESHOLD           3

/* Maximum RX packet size, if exceed this value, drop incoming packet */
#define CFG_RX_MAX_PKT_SIZE                     2048

/* Minimum RX packet size, if lower than this value, drop incoming packet */
#define CFG_RX_MIN_PKT_SIZE                     10 /* 802.11 Control Frame is 10 bytes */


/* Maximum number of BSS in the SCAN list */
#define CFG_MAX_NUM_BSS_LIST                    48

/* Maximum size of IE buffer of each SCAN record */
#define CFG_IE_BUFFER_SIZE                      512

/* Maximum number of STA records */
#define CFG_MAX_NUM_STA_RECORD                  32


#define CFG_LP_PATTERN_SEARCH_SLT                                  1   /* 1: Enable LP SLT test
                                                           0: Disable LP SLT test */

/* The dwell time for staying in ACTIVE mode when connected to AP */
#if CFG_LP_PATTERN_SEARCH_SLT
#define CFG_KEEP_ACTIVE_DWELL_TIME_ON_CONNECT_MSEC  500
#else
#define CFG_KEEP_ACTIVE_DWELL_TIME_ON_CONNECT_MSEC  5000
#endif

#define CFG_USE_SW_ROOT_TIMER                       0

#define CFG_LOW_POWER_INST_START_ADDR               0

#define CFG_LP_IOT                                  0   /* 1: Enable LP IOT test
                                                           0: Disable LP IOT test */

#define CFG_IBSS_ATIM_WINDOW                        0   /* 0: Creating IBSS without PS function
                                                           non-0: create IBSS with PS function */

#define CFG_INIT_UAPSD_AC                           UAPSD_ALL//UAPSD_NONE//

#define CFG_INIT_POWER_SAVE_PROF                    ENUM_PSP_FAST_SWITCH

#define CFG_INIT_VOIP_INTERVAL                      0   /* > 0: Enable VOIP
                                                             0: Disable */

#define CFG_INIT_POLL_INTERVAL                      0   /* > 0: Enable CONTINUOUS POLL
                                                             0: Disable */

#if CFG_LP_PATTERN_SEARCH_SLT
#define CFG_L3_PATTERN_MATCH_WAKEUP                 1   /* 1: Enable L3 pattern search function
                                                           0: Disable L3 pattern search function */
#else
#define CFG_L3_PATTERN_MATCH_WAKEUP                 0   /* 1: Enable L3 pattern search function
                                                           0: Disable L3 pattern search function */
#endif

/* This should only be enabled under FPGA version, shall not be enabled under ASIC version */
#define CFG_FPGA_COMPENSATE_HALF_T_FOR_GUARD_TIME_HEC_4489   0

#define CFG_WORKAROUND_HEC_5269                     1
#define CFG_WORKAROUND_HEC_5512                     1

#define CFG_WORKAROUND_BG_SSID_SCAN_DONE            1

#define CFG_WORKAROUND_HEC_5988                     1

#define CFG_WORKAROUND_HEC_6796                     1



#define CFG_MAX_PMKID_CACHE                     16      /* max number of PMKID cache 
                                                           16(default) : The Max PMKID cache */

#define CFG_INIT_ADHOC_MODE                     AD_HOC_MODE_MIXED_11BG



#define CFG_ONLY_802_11A                        0 /* 1: Enable this will let driver run
                                                    at 802.11a only, disable 11g */


#define CFG_INIT_TX_POWER_LIMIT                 10


#if CFG_ONLY_802_11A
#define CFG_SUPPORT_SSID_RECOVER_STATE          0 /* 1: Support ARB SSID recover state
                                                     0: Do not support ARB SSID recover state */
#else
#define CFG_SUPPORT_SSID_RECOVER_STATE          1 /* 1: Support ARB SSID recover state
                                                     0: Do not support ARB SSID recover state */
#endif

#if defined(_HIF_SDIO) && defined(WINDOWS_CE)
    #define CFG_IST_LOOP_COUNT                  3
#else
    #define CFG_IST_LOOP_COUNT                  3
#endif /* _HIF_SDIO */

#define CFG_INT_WRITE_CLEAR                     0

#define CFG_TEST_IO_PIN 1 /* 1: Enable INT_N and GPIO_0 Pin test.
                                                             0: Disable */

//2 Flags for Driver Debug Options
#define CFG_TX_DBG_TFCB_CHKSUM                  0 /* 1: Enable diagnosing TX by using TFCB Checksum by
                                                        halTxEnableDebugOption().
                                                     0: Disable */

#define CFG_TX_DBG_INCREASED_PID                0

#define CFG_TX_DBG_FIXED_PID                    0

#if (CFG_TX_DBG_INCREASED_PID && CFG_TX_DBG_FIXED_PID)
    #error Conflict of compiler flags.
#endif /* (CFG_TX_DBG_INCREASED_PID && CFG_TX_DBG_FIXED_PID) */

#define CFG_TX_DBG_SEQ_NUM                      0 /* 1: Enable tracing of TX SEQ_NUM from
                                                        TX_STATUS.
                                                     0: Disable */

#define CFG_TX_DBG_INT_FALSE_ALARM              0 /* 1: Enable diagnose TX_DONE INT False Alarm
                                                        (Need DBG == 1 for ASSERT()).
                                                     0(default): Disable */


#if DBG || BUILD_QA_DBG
#define CFG_TX_DBG_MGT_BUF                      1 /* 1: Enable diagnosing maximum depth
                                                        of allocated TX Management Buffer.
                                                     0: Disable */
#else
#define CFG_TX_DBG_MGT_BUF                      0
#endif /* DBG || BUILD_QA_DBG */


#define CFG_TX_DBG_PACKET_ORDER                 0 /* 1: Enable diagnosing Packet Order
                                                        to make sure they are first-in-first-out
                                                        if belong to the same TID(Linux only).
                                                     0(default): Disable */

#if DBG || BUILD_QA_DBG
#define CFG_DBG_STA_RECORD                      1 /* 1: Enable diagnosing STA_RECORD.
                                                     0(default): Disable */
#else
#define CFG_DBG_STA_RECORD                      0
#endif /* DBG || BUILD_QA_DBG */

#define CFG_INTERNAL_MEMORY_TEST                0 /* 1: Enable diagnose internal memory(HW FIFO).
                                                     0(default): Disable */

#define CFG_DATA_PORT_ACCESS_TEST               0 /* 1: Enable diagnose internal memory(HW FIFO).
                                                     0(default): Disable */

#define CFG_STATISTICS_TIMER_EN                 0 /* 1: Get Statistics from MCR in Timer.
                                                     0(default): Get Statistics from MCR in INT. */

#define CFG_TIMER_PEEK_HW_COUNTERS_PERIOD_MSEC        1677720  //Millisecond, for timer use
#define CFG_INT_PEEK_HW_COUNTERS_TIMEOUT_MSEC         1677720  //Millisecond, for int use
                                                      /* 2^24(bit) * 100us ~= 1677second */
#define CETK_NDIS_PERFORMANCE_WORKAROUND        1 /* Workaround CETK NDIS Performance
                                                     Protocol Reserved filed no clear issue
                                                     1 (default) : Enabled the workaround */

#define CFG_THERMO_INT_EN                       1
#define CFG_THERMO_TIMER_EN                     0  /* 1: Enable Thermo Timer
                                                      0: Disable */
#if (CFG_THERMO_INT_EN && CFG_THERMO_TIMER_EN)
    #error Conflict of compiler flags, Thermo.
#endif /* (CFG_THERMO_INT_EN && CFG_THERMO_TIMER_EN) */

#if CFG_THERMO_TIMER_EN
//#define CFG_PEEK_THERMO_PERIOD_MSEC             5000 // Millisecond
#define CFG_PEEK_THERMO_PERIOD_MSEC             1000 // Millisecond
#define CFG_FAKE_THERMO_VALUE_DBG_EN            0  /* 1: Enable debug fake therm value.
                                                      0: Disable */
#else
#define CFG_FAKE_THERMO_VALUE_DBG_EN            0  /* 0: Always disable if TIMER_EN == 0 */
#endif /* CFG_THERMO_TIMER_EN */


#define CFG_PEEK_RCPI_VALUE_PERIOD_SEC          0 // Seconds
                                                  /* > 0: Enable debug RCPI value
                                                       0: Disable */

#define CFG_DBG_BEACON_RCPI_MEASUREMENT         0 /* 1: Enable
                                                     0: Disable */

#define CFG_SW_TCL                              0 /* 1: Using SW to compare the TSF.
                                                     0(default): Use HW from RFB. */

#define LINT_SAVE_AND_DISABLE                   /*lint -save -e* */

#define LINT_RESTORE                            /*lint -restore */

#define LINT_EXT_HEADER_BEGIN                   LINT_SAVE_AND_DISABLE

#define LINT_EXT_HEADER_END                     LINT_RESTORE

#define PTA_ENABLED                             1
#if PTA_ENABLED 
#if SUPPORT_NEW_PTA_BOARD
#define PTA_NEW_BOARD_DESIGN                    1
#else
#define PTA_NEW_BOARD_DESIGN                    0
#endif
#endif
#define CFG_INIT_VI_AIFS_BIAS                   0

#define CFG_INIT_VI_MAX_TXOP_LIMIT              0xFFFF

#define SUPPORT_STREAM_IO                       0 /* default: disable */

#define SUPPORT_WPS                             1 /* default: enable */

#define SUPPORT_CR1809_WPS_AP_WORKAROUND        1 /* default: enable */

#if SUPPORT_WAPI
/* Have to enable this for header translation */
#undef CFG_TX_FRAGMENT
#define CFG_TX_FRAGMENT                         1 /* 1: Enable TX fragmentation
                                                                                            0: Disable */

#define SUPPORT_WPI_AVOID_LOCAL_BUFFER          1 /* 0: use local buffer,
                                                     1: Default Use adapter buffer alloc at driver init */

#endif

#define CFG_LED_MODE_NONE   0
#define CFG_LED_MODE_TX     1
#define CFG_LED_MODE_RX     2
#define CFG_LED_MODE_TX_RX  3






#endif /* _CONFIG_H */


