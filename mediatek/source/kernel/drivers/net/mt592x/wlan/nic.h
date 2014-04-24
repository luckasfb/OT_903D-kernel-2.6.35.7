




#ifndef _NIC_H
#define _NIC_H






/* MCR/BBCR/RF configuration entry structure */
struct _REG_ENTRY_T {
    UINT_32      u4Offset;
    UINT_32      u4Value;
};

struct _TABLE_ENTRY_T {
    P_REG_ENTRY_T     pu4TablePtr;
    UINT_16      u2Size;
};

struct _TABLE_RF_ENTRY_T {
    P_RF_CHANNEL_PROG_ENTRY    pu4TableRfPtr;
    UINT_16      u2Size;
};


/* INT status to event map */
typedef const struct _INT_EVENT_MAP_T {
    UINT_32     u4Int;
    UINT_32     u4Event;
    UINT_8      ucRegNum;
} INT_EVENT_MAP_T, *P_INT_EVENT_MAP_T;


enum ENUM_INT_EVENT_T {
    INT_EVENT_ABNORMAL,
    INT_EVENT_WATCH_DOG,
    INT_EVENT_TSF_DRIFT,
    INT_EVENT_SCAN_DONE,
    INT_EVENT_RX,
    INT_EVENT_TX,
    INT_EVENT_T0_TIMEUP,
    INT_EVENT_T1_TIMEUP,
    INT_EVENT_T2_TIMEUP,
    INT_EVENT_T3_TIMEUP,
    INT_EVENT_ADMIT_TIME_MET,
    INT_EVENT_BEACON_TIMEOUT,
    INT_EVENT_TX_NULL_FAIL,
    INT_EVENT_TX_TRIG_FAIL,
    INT_EVENT_TX_PSPOLL_FAIL,
    INT_EVENT_BMC_TIMEOUT,
    INT_EVENT_APSD_TIMEOUT,
    INT_EVENT_PSPOLL_TIMEOUT,
    INT_EVENT_TBTT,
    INT_EVENT_BCN_TR_OK,
    INT_EVENT_ATIM,
    INT_EVENT_BCN_TX_OK,
    INT_EVENT_RCPI,
    INT_EVENT_ALC,
    INT_EVENT_NUM
};


/* RX FILTER Setting to  map */
typedef const struct _RX_FILTER_MAP_T {
/*    UINT_32     u4RxFilter;*/
    BOOLEAN     fgEnabled;
    UINT_32      u4Value;
} RX_FILTER_MAP_T, *P_RX_FILTER_MAP_T;

/* NIC RF channel programming entry structure */
typedef struct _CHANNEL_BAND_FREQ_MAP {
    UINT_8      ucChannelNum;
    ENUM_BAND_T eBand;
    UINT_32     u4ChannelFreq;
} CHANNEL_BAND_FREQ_MAP, *P_CHANNEL_BAND_FREQ_MAP;


enum ENUM_RX_FILTER_T {
    RXFILTER_RXSAMEBSSIDPRORESP,
    RXFILTER_RXDIFFBSSIDPRORESP,
    RXFILTER_RXSAMEBSSIDATIM,
    RXFILTER_RXDIFFBSSIDATIM,
    RXFILTER_RXSAMEBSSIDNULL,
    RXFILTER_RXDIFFBSSIDNULL,
    RXFILTER_RXSAMEBSSIDBCN,
    RXFILTER_RXDIFFBSSIDBCN,
    RXFILTER_RXPROREQ,
    RXFILTER_DROPVERSIONNOT0,
    RXFILTER_RXNOACK,
    RXFILTER_RXMGMTFRAME,
    RXFILTER_RXDATAFRAME,
    RXFILTER_DROPDIFFBSSIDMGT,
    RXFILTER_DROPA3OWNSA,
    RXFILTER_DROPDIFFBSSIDA3,
    RXFILTER_DROPDIFFBSSIDA2,
    RXFILTER_RXBCFRAME,
    RXFILTER_MCTABLENOCHK,
    RXFILTER_RXMCFRAME,
    RXFILTER_RXPROMISCUOUSFRAME,
    RXFILTER_DROPFCS,
    RXFILTER_NUM
};

/*! \brief Subband information in domains */
typedef struct _DOMAIN_SUBBAND_INFO {
    UINT_8          ucFirstChannelNum;          /*!< The lowest channel number in the subband */
    UINT_8          ucNumChannels;              /*!< Number of channel in this domaim */
    INT_8           cMaxTxPowerLevel;           /*!< Max allowed tx power (dBm) in this subband */
    UINT_32         u4MibBandsSupportedValue;   /*!< dot11FrequencyBandsSupported value for 5 GHz: MIB_U_NII_BAND_x */
} DOMAIN_SUBBAND_INFO, *P_DOMAIN_SUBBAND_INFO;

typedef struct _DOMAIN_INFO_ENTRY {
    /* Numeric form of two character country code as described in ISO/IEC
    ** 3166-1. Ex:"TW" will be stored as 0x5457.
    */
    UINT_16             u2CountryCode;

    DOMAIN_SUBBAND_INFO rBand24;  /*!< 2.4 GHz subband info */
} DOMAIN_INFO_ENTRY, *P_DOMAIN_INFO_ENTRY, **P_P_DOMAIN_INFO_ENTRY;

extern INT_EVENT_MAP_T arIntEventMapTable[];
extern const UINT_8 ucIntEventMapSize;
extern const UINT_8 ucIntRegNum;
extern const UINT_32 au4IntRegs[];
extern const UINT_32 au4IERRegs[];
extern RX_FILTER_MAP_T arRxFilterMapTable[];




/* Opeations for prefetch interrupt status related handling */
#define NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter) \
    _prAdapter->fgRedoProcessISTFlag = TRUE

#define NIC_UNSET_REDO_PROCESS_IST_FLAG(_prAdapter) \
    _prAdapter->fgRedoProcessISTFlag = FALSE

#define NIC_IS_NEED_TO_REDO_PROCESS_IST(_prAdapter) \
    (_prAdapter->fgRedoProcessISTFlag == TRUE)


/* This macro is used to enable interrupt enable bit */
#define NIC_SET_INT_EVENT(_prAdapter, _INT_EVENT) \
    { \
        UINT_32 u4IntIdx = 0; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (_INT_EVENT == arIntEventMapTable[u4IntIdx].u4Event) {\
                _prAdapter->au4IntEnable[arIntEventMapTable[u4IntIdx].ucRegNum] |= arIntEventMapTable[u4IntIdx].u4Int;\
                HAL_MCR_WR(_prAdapter, au4IERRegs[arIntEventMapTable[u4IntIdx].ucRegNum], _prAdapter->au4IntEnable[arIntEventMapTable[u4IntIdx].ucRegNum]); \
                break; \
            } \
        } \
    }

#if DBG
/* This macro is used to disable interrupt enable bit and status bit */
#define NIC_UNSET_INT_EVENT(_prAdapter, _INT_EVENT) \
    { \
        UINT_32 u4IntIdx = 0; \
        UINT_32 u4RegValue = 0; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (_INT_EVENT == arIntEventMapTable[u4IntIdx].u4Event) {\
                _prAdapter->au4IntEnable[arIntEventMapTable[u4IntIdx].ucRegNum] &= ~arIntEventMapTable[u4IntIdx].u4Int;\
                HAL_MCR_WR(_prAdapter, au4IERRegs[arIntEventMapTable[u4IntIdx].ucRegNum], _prAdapter->au4IntEnable[arIntEventMapTable[u4IntIdx].ucRegNum]); \
                HAL_MCR_RD(_prAdapter, \
                           au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                           &u4RegValue); \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] |= u4RegValue; \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] &= ~arIntEventMapTable[u4IntIdx].u4Int; \
                if (u4RegValue & ~arIntEventMapTable[u4IntIdx].u4Int) { \
                    NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter); \
                } \
                if (_prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] == 0) { \
                    _prAdapter->fgIntrMayFalseAlarmFlag = TRUE; \
                } \
                break; \
            } \
        } \
    }

/* This macro is used to clear interrupt status bits (without disable interrupt enable bit) */
#define NIC_RESET_INT_EVENT(_prAdapter, _INT_EVENT) \
    { \
        UINT_32 u4IntIdx = 0; \
        UINT_32 u4RegValue = 0; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (_INT_EVENT == arIntEventMapTable[u4IntIdx].u4Event) {\
                HAL_MCR_RD(_prAdapter, \
                           au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                           &u4RegValue); \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] |= u4RegValue; \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] &= ~arIntEventMapTable[u4IntIdx].u4Int; \
                if (u4RegValue & ~arIntEventMapTable[u4IntIdx].u4Int) { \
                    NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter); \
                } \
                if (_prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] == 0) { \
                    _prAdapter->fgIntrMayFalseAlarmFlag = TRUE; \
                } \
                break; \
            } \
        } \
    }

#if CFG_WORKAROUND_BG_SSID_SCAN_DONE
/* This macro is used to clear redundent scan done event due to ownback from BG SSID scan */
#define NIC_VALIDATE_BG_SSID_SCAN_DONE_INT_EVENT(_prAdapter) \
    { \
        UINT_32 u4IntIdx = 0; \
        UINT_32 u4IntScanDoneIdx = 0; \
        UINT_32 u4RegValue; \
        BOOLEAN fgScanDone = FALSE, fgRxDone = FALSE, fgMcrRead = FALSE; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (INT_EVENT_SCAN_DONE == arIntEventMapTable[u4IntIdx].u4Event) {\
                if (!fgMcrRead) { \
                    HAL_MCR_RD(_prAdapter, \
                               au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                               &u4RegValue); \
                    fgMcrRead = TRUE; \
                } \
                if (u4RegValue & arIntEventMapTable[u4IntIdx].u4Int) { \
                    fgScanDone = TRUE; \
                    u4IntScanDoneIdx = u4IntIdx; \
                } \
            } else if (INT_EVENT_RX == arIntEventMapTable[u4IntIdx].u4Event) {\
                if (!fgMcrRead) { \
                    HAL_MCR_RD(_prAdapter, \
                               au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                               &u4RegValue); \
                    fgMcrRead = TRUE; \
                } \
                if (u4RegValue & arIntEventMapTable[u4IntIdx].u4Int) { \
                    fgRxDone = TRUE; \
                } \
            } \
        } \
        if (fgScanDone == TRUE && fgRxDone == FALSE) { \
            DBGLOG(HAL, WARN, ("Clear scan done in NIC_VALIDATE_BG_SSID_SCAN_DONE_INT_EVENT\n")); \
            { \
            UINT_32 u4Rfcr; \
            HAL_MCR_RD(prAdapter, MCR_RFCR, &u4Rfcr); \
            DBGLOG(HAL, INFO, ("@@ RX filter: 0x%08lx\n", u4Rfcr)); \
            } \
            _prAdapter->au4IntStatus[arIntEventMapTable[u4IntScanDoneIdx].ucRegNum] |= u4RegValue; \
            _prAdapter->au4IntStatus[arIntEventMapTable[u4IntScanDoneIdx].ucRegNum] &= ~arIntEventMapTable[u4IntScanDoneIdx].u4Int; \
            if (u4RegValue & ~arIntEventMapTable[u4IntScanDoneIdx].u4Int) { \
                NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter); \
            } \
            if (_prAdapter->au4IntStatus[arIntEventMapTable[u4IntScanDoneIdx].ucRegNum] == 0) { \
                _prAdapter->fgIntrMayFalseAlarmFlag = TRUE; \
            } \
        } \
    }

#endif
#else
/* This macro is used to disable interrupt enable bit and status bit */
#define NIC_UNSET_INT_EVENT(_prAdapter, _INT_EVENT) \
    { \
        UINT_32 u4IntIdx = 0; \
        UINT_32 u4RegValue; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (_INT_EVENT == arIntEventMapTable[u4IntIdx].u4Event) {\
                _prAdapter->au4IntEnable[arIntEventMapTable[u4IntIdx].ucRegNum] &= ~arIntEventMapTable[u4IntIdx].u4Int;\
                HAL_MCR_WR(_prAdapter, au4IERRegs[arIntEventMapTable[u4IntIdx].ucRegNum], _prAdapter->au4IntEnable[arIntEventMapTable[u4IntIdx].ucRegNum]); \
                HAL_MCR_RD(_prAdapter, \
                           au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                           &u4RegValue); \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] |= u4RegValue; \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] &= ~arIntEventMapTable[u4IntIdx].u4Int; \
                if (u4RegValue & ~arIntEventMapTable[u4IntIdx].u4Int) { \
                    NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter); \
                } \
                break; \
            } \
        } \
    }

/* This macro is used to clear interrupt status bits (without disable interrupt enable bit) */
#define NIC_RESET_INT_EVENT(_prAdapter, _INT_EVENT) \
    { \
        UINT_32 u4IntIdx = 0; \
        UINT_32 u4RegValue; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (_INT_EVENT == arIntEventMapTable[u4IntIdx].u4Event) {\
                HAL_MCR_RD(_prAdapter, \
                           au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                           &u4RegValue); \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] |= u4RegValue; \
                _prAdapter->au4IntStatus[arIntEventMapTable[u4IntIdx].ucRegNum] &= ~arIntEventMapTable[u4IntIdx].u4Int; \
                if (u4RegValue & ~arIntEventMapTable[u4IntIdx].u4Int) { \
                    NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter); \
                } \
                break; \
            } \
        } \
    }

#if CFG_WORKAROUND_BG_SSID_SCAN_DONE
/* This macro is used to clear redundent scan done event due to ownback from BG SSID scan */
#define NIC_VALIDATE_BG_SSID_SCAN_DONE_INT_EVENT(_prAdapter) \
    { \
        UINT_32 u4IntIdx = 0; \
        UINT_32 u4IntScanDoneIdx = 0; \
        UINT_32 u4RegValue; \
        BOOLEAN fgScanDone = FALSE, fgRxDone = FALSE, fgMcrRead = FALSE; \
        for (u4IntIdx = 0; u4IntIdx < INT_EVENT_NUM; u4IntIdx++) { \
            if (INT_EVENT_SCAN_DONE == arIntEventMapTable[u4IntIdx].u4Event) {\
                if (!fgMcrRead) { \
                    HAL_MCR_RD(_prAdapter, \
                               au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                               &u4RegValue); \
                    fgMcrRead = TRUE; \
                } \
                if (u4RegValue & arIntEventMapTable[u4IntIdx].u4Int) { \
                    fgScanDone = TRUE; \
                    u4IntScanDoneIdx = u4IntIdx; \
                } \
            } else if (INT_EVENT_RX == arIntEventMapTable[u4IntIdx].u4Event) {\
                if (!fgMcrRead) { \
                    HAL_MCR_RD(_prAdapter, \
                               au4IntRegs[arIntEventMapTable[u4IntIdx].ucRegNum], \
                               &u4RegValue); \
                    fgMcrRead = TRUE; \
                } \
                if (u4RegValue & arIntEventMapTable[u4IntIdx].u4Int) { \
                    fgRxDone = TRUE; \
                } \
            } \
        } \
        if (fgScanDone == TRUE && fgRxDone == FALSE) { \
            _prAdapter->au4IntStatus[arIntEventMapTable[u4IntScanDoneIdx].ucRegNum] |= u4RegValue; \
            _prAdapter->au4IntStatus[arIntEventMapTable[u4IntScanDoneIdx].ucRegNum] &= ~arIntEventMapTable[u4IntScanDoneIdx].u4Int; \
            if (u4RegValue & ~arIntEventMapTable[u4IntScanDoneIdx].u4Int) { \
                NIC_SET_REDO_PROCESS_IST_FLAG(_prAdapter); \
            } \
        } \
    }

#endif
#endif // if DBG

#if CFG_WORKAROUND_BG_SSID_SCAN_DONE
#define NIC_DEL_INT_SCAN_DONE_EVENT(_prAdapter) \
    { \
        HAL_DEL_INT_SCAN_DONE_EVENT(_prAdapter); \
    }

#define NIC_ADD_INT_SCAN_DONE_EVENT(_prAdapter) \
    { \
        HAL_ADD_INT_SCAN_DONE_EVENT(_prAdapter); \
    }
#endif

#define NIC_SET_RX_FILTER(_prAdapter, _RX_FILTER) \
    if (arRxFilterMapTable[_RX_FILTER].fgEnabled){ \
        _prAdapter->rRxCtrl.fgRxFilter[_RX_FILTER] = TRUE; \
        _prAdapter->rRxCtrl.u4RxFilterReg |= arRxFilterMapTable[_RX_FILTER].u4Value; \
        halSetRxFilters(_prAdapter, _prAdapter->rRxCtrl.u4RxFilterReg); \
    }

#define NIC_UNSET_RX_FILTER(_prAdapter, _RX_FILTER) \
    if (arRxFilterMapTable[_RX_FILTER].fgEnabled){ \
        _prAdapter->rRxCtrl.fgRxFilter[_RX_FILTER] = FALSE; \
        _prAdapter->rRxCtrl.u4RxFilterReg &= ~arRxFilterMapTable[_RX_FILTER].u4Value; \
        halSetRxFilters(_prAdapter, _prAdapter->rRxCtrl.u4RxFilterReg); \
    }

#define NIC_GET_CURRENT_TSF(_prAdapter, _pu8TsfTime) HAL_GET_CURRENT_TSF(_prAdapter, _pu8TsfTime)

#define NIC_PM_ENABLE_TBTT_INTR_TRIGGER(_prAdapter) \
    { \
        UINT_32 u4Value; \
        HAL_MCR_RD(_prAdapter, MCR_MPTCR, &u4Value); \
        HAL_MCR_WR(_prAdapter, MCR_MPTCR, u4Value | MPTCR_TBTT_TIMEUP_EN); \
    }

#define NIC_PM_DISABLE_TBTT_INTR_TRIGGER(_prAdapter) \
    { \
        UINT_32 u4Value; \
        HAL_MCR_RD(_prAdapter, MCR_MPTCR, &u4Value); \
        HAL_MCR_WR(_prAdapter, MCR_MPTCR, u4Value & ~MPTCR_TBTT_TIMEUP_EN); \
    }

#define NIC_QUERY_RX_G0_EN(_prAdapter, _pfgEn)      HAL_MSCR_IS_RX_G0_EN(_prAdapter, _pfgEn)
#define NIC_QUERY_RX_G1_EN(_prAdapter, _pfgEn)      HAL_MSCR_IS_RX_G1_EN(_prAdapter, _pfgEn)
#define NIC_QUERY_RX_G2_EN(_prAdapter, _pfgEn)      HAL_MSCR_IS_RX_G2_EN(_prAdapter, _pfgEn)
#define NIC_QUERY_RX_G0_RSSI(_prAdapter, _pfgRssi)  HAL_MSCR_IS_RX_G0_RSSI(_prAdapter, _pfgRssi)

#define NIC_MAC_TX_RX_CONTROL(_prAdapter, _txEn, _rxEn) \
    { \
        UINT_32 u4Value; \
        HAL_MCR_RD(_prAdapter, MCR_RICR, &u4Value); \
        u4Value = (u4Value & ~ RICR_MAC_TXRX_MASK);\
        if (( _txEn)) { \
            u4Value |= RICR_MAC_TX_EN;\
        }\
        if ((_rxEn)) { \
            u4Value |= RICR_MAC_RX_EN;\
        }\
        HAL_MCR_WR(_prAdapter, MCR_RICR, u4Value ); \
    }

#define NIC_ALC_EN(_prAdapter, _avePara, _maxThreshold, _minThreshold, _initVal) \
            halALCREnable( (_prAdapter), (_avePara), (_maxThreshold), (_minThreshold), _initVal)
#define NIC_ALC_DISABLE(_prAdapter) \
                halALCRDisable( (_prAdapter))

#define NIC_ALC_GET_RAW_VALUE(_prAdapter, _prRawVal) \
                halALCRGetRawValue( (_prAdapter), (_prRawVal))

#define NIC_ALC_GET_CAL_VALUE(_prAdapter, _prCalVal) \
                halALCRGetCalValue( (_prAdapter), (_prCalVal))

#define NIC_ALC_TRIGGER(_prAdapter) \
                halALCRTriggerALC( (_prAdapter))

#define NIC_THERMO_UPDATE_TX_GAIN(_prAdapter, _channelnum, _eBand, _alcCal) \
                halThermoUpdateTxGain( (_prAdapter), (_channelnum), (_eBand), (_alcCal))

#define NIC_THERMO_UPDATE_RX_SETTING(_prAdapter, _alcCal) \
                halThermoUpdateRxSetting( (_prAdapter),(_alcCal))


#define COUNTRY_CODE_NULL               ((UINT_16)0x0)

/* ISO/IEC 3166-1 two-character country codes */
#define COUNTRY_CODE_AT (((UINT_16) 'A' << 8) | (UINT_16) 'T') /* Austria       */
#define COUNTRY_CODE_AU (((UINT_16) 'A' << 8) | (UINT_16) 'U') /* Australia     */
#define COUNTRY_CODE_BE (((UINT_16) 'B' << 8) | (UINT_16) 'E') /* Belgium       */
#define COUNTRY_CODE_CA (((UINT_16) 'C' << 8) | (UINT_16) 'A') /* Canada        */
#define COUNTRY_CODE_CH (((UINT_16) 'C' << 8) | (UINT_16) 'H') /* Switzerland   */
#define COUNTRY_CODE_CN (((UINT_16) 'C' << 8) | (UINT_16) 'N') /* China         */
#define COUNTRY_CODE_DE (((UINT_16) 'D' << 8) | (UINT_16) 'E') /* Germany       */
#define COUNTRY_CODE_DK (((UINT_16) 'D' << 8) | (UINT_16) 'K') /* Denmark       */
#define COUNTRY_CODE_ES (((UINT_16) 'E' << 8) | (UINT_16) 'S') /* Spain         */
#define COUNTRY_CODE_EU (((UINT_16) 'E' << 8) | (UINT_16) 'U') /* ETSI (Europe) */
#define COUNTRY_CODE_FI (((UINT_16) 'F' << 8) | (UINT_16) 'I') /* Finland       */
#define COUNTRY_CODE_FR (((UINT_16) 'F' << 8) | (UINT_16) 'R') /* France        */
#define COUNTRY_CODE_GB (((UINT_16) 'G' << 8) | (UINT_16) 'B') /* United Kingdom */
#define COUNTRY_CODE_GR (((UINT_16) 'G' << 8) | (UINT_16) 'R') /* Greece        */
#define COUNTRY_CODE_IE (((UINT_16) 'I' << 8) | (UINT_16) 'E') /* Ireland       */
#define COUNTRY_CODE_IL (((UINT_16) 'I' << 8) | (UINT_16) 'L') /* Israel        */
#define COUNTRY_CODE_IS (((UINT_16) 'I' << 8) | (UINT_16) 'S') /* Iceland       */
#define COUNTRY_CODE_IT (((UINT_16) 'I' << 8) | (UINT_16) 'T') /* Italy         */
#define COUNTRY_CODE_JO (((UINT_16) 'J' << 8) | (UINT_16) 'O') /* Jordan        */
#define COUNTRY_CODE_JP (((UINT_16) 'J' << 8) | (UINT_16) 'P') /* Japan         */
#define COUNTRY_CODE_KR (((UINT_16) 'K' << 8) | (UINT_16) 'R') /* Korea         */
#define COUNTRY_CODE_LI (((UINT_16) 'L' << 8) | (UINT_16) 'I') /* Liechtenstein */
#define COUNTRY_CODE_LU (((UINT_16) 'L' << 8) | (UINT_16) 'U') /* Luxembourg    */
#define COUNTRY_CODE_NL (((UINT_16) 'N' << 8) | (UINT_16) 'L') /* Netherlands   */
#define COUNTRY_CODE_NO (((UINT_16) 'N' << 8) | (UINT_16) 'O') /* Norway        */
#define COUNTRY_CODE_PT (((UINT_16) 'P' << 8) | (UINT_16) 'T') /* Portugal      */
#define COUNTRY_CODE_SE (((UINT_16) 'S' << 8) | (UINT_16) 'E') /* Sweden        */
#define COUNTRY_CODE_SG (((UINT_16) 'S' << 8) | (UINT_16) 'G') /* Singapore     */
#define COUNTRY_CODE_TW (((UINT_16) 'T' << 8) | (UINT_16) 'W') /* Taiwan        */
#define COUNTRY_CODE_TH (((UINT_16) 'T' << 8) | (UINT_16) 'H') /* Thailand      */
#define COUNTRY_CODE_US (((UINT_16) 'U' << 8) | (UINT_16) 'S') /* United States */


#define NIC_PM_PROGRAM_LP_INSRUCTION(_prAdapter, _fgForBtCoexist) \
    { \
        P_PM_INFO_T _prPmInfo = &_prAdapter->rPmInfo; \
        halpmProgLowPwrInst(_prAdapter, \
                            _prPmInfo->rSlowClkCnt.uc32kSlowCount, \
                            _prPmInfo->ucNumOfInstSleep, \
                            _prPmInfo->ucNumOfInstAwake, \
                            _prPmInfo->ucNumOfInstOn, \
                            _prPmInfo->au4LowPowerInst_sleep, \
                            _prPmInfo->au4LowPowerInst_awake, \
                            _prPmInfo->au4LowPowerInst_on, \
                            _fgForBtCoexist); \
    }

#define NIC_PM_MODIFY_WAKEUP_GUARD_TIME(_prAdapter, _fgExtendBy1BT_Slot) \
    { \
        P_PM_INFO_T _prPmInfo = &_prAdapter->rPmInfo; \
        halpmSetupWakeupGuardTime(_prAdapter, \
                                  _prPmInfo->u2GuardTimePhase2 + ((_fgExtendBy1BT_Slot) ? 625 : 0), \
                                  _prPmInfo->u2GuardTimePhase3 + ((_fgExtendBy1BT_Slot) ? 625 : 0)); \
    }

#define NIC_PM_CONFIG_DTIM_TIMEUP_INTR(_prAdapter, _fgEnable) \
    { \
        if (_fgEnable) { \
            NIC_PM_ENABLE_TBTT_INTR_TRIGGER(_prAdapter); \
        } else { \
            NIC_PM_DISABLE_TBTT_INTR_TRIGGER(_prAdapter); \
        } \
    }

#define NIC_PM_CONFIG_DTIM_WAKEUP_FUNC(_prAdapter, _fgEnable) \
    { \
        UINT_32 u4Value; \
        P_PM_INFO_T  prPmInfo =  &_prAdapter->rPmInfo; \
        HAL_MCR_RD(_prAdapter, MCR_MPTCR, &u4Value); \
        if (_fgEnable) { \
            HAL_MCR_WR(_prAdapter, MCR_MPTCR, u4Value | MPTCR_PREDTIM_TRIG_EN); \
            prPmInfo->fgDtimTrigEn = TRUE; \
        } else { \
            HAL_MCR_WR(_prAdapter, MCR_MPTCR, u4Value & ~MPTCR_PREDTIM_TRIG_EN); \
            prPmInfo->fgDtimTrigEn = FALSE; \
        } \
    }

#define NIC_PM_WMM_PS_DISABLE_UC_TRIG(_prAdapter, _fgDisableTrig) \
    { \
        UINT_32 u4Mptcr; \
        HAL_MCR_RD(_prAdapter, MCR_MPTCR, &u4Mptcr); \
        if (_fgDisableTrig) { \
            u4Mptcr &= ~MPTCR_BCN_UC_EN; \
        } else { \
            u4Mptcr |= MPTCR_BCN_UC_EN; \
        } \
        HAL_MCR_WR(_prAdapter, MCR_MPTCR, u4Mptcr); \
    }

__KAL_INLINE__ BOOLEAN testAndSet (
    IN PBOOLEAN pfgFlag
    )
{
    BOOLEAN fgFlag;

    ASSERT(pfgFlag);

    fgFlag = *pfgFlag;
    *pfgFlag = TRUE;

    return fgFlag;
}


/*----------------------------------------------------------------------------*/
/* Routines in nic.c                                                          */
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicAllocateAdapterMemory (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicReleaseAdapterMemory (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicDisableInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicEnableInterrupt (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
nicProcessIST (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
nicReadConfig (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgEepromRecall
    );

VOID
nicInitializeChip (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicGetDefaultEEPROMCfg (
    IN P_ADAPTER_T prAdapter,
    OUT P_EEPROM_CTRL_T prEEPROMCtrl
    );

BOOLEAN
nicSetHwBySta (
    IN P_ADAPTER_T      prAdapter,
    IN P_STA_RECORD_T   prSta
    );

BOOLEAN
nicSetQoSBySta (
    IN P_ADAPTER_T prAdapter,
    IN  PUINT_8             pucBSSID
    );

BOOLEAN
nicSetAntennaBySta (
    IN  P_ADAPTER_T prAdapter,
    IN  PUINT_8             pucBSSID,
    IN  BOOLEAN     fgAntenna
    );

BOOLEAN
nicSetRateBySta (
    IN  P_ADAPTER_T        prAdapter,
    IN  PUINT_8             pucBSSID,
    IN  UINT_8             ucRate1,
    IN  UINT_8             ucRate2,
    IN  UINT_8             ucRate3,
    IN  UINT_8             ucRate4
    );

VOID
nicChipReset (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicInitializeConnectionSettings (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicSetConnectionSettingsByGlueParam (
    IN P_ADAPTER_T prAdapter,
    IN P_REG_INFO_T prRegInfo
    );

VOID
nicInitializeAdapter (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicSetAvailablePhyTypeSet (
    IN P_ADAPTER_T prAdapter
    );

WLAN_STATUS
nicCheckAvailablePhyTypeSet (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PARAM_PHY_TYPE_T eNewNetworkType
    );

VOID
nicSetSlotTime (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PHY_TYPE_INDEX_T ePhyTypeIndex,
    IN BOOLEAN fgUseShortSlotTime
    );

VOID
nicSwitchChannel (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BAND_T eBand,
    IN UINT_8      ucChannelNum,
    IN UINT_8      ucTxPwrLimit
    );

VOID
nicSetupBSS (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_INFO_T prBSSInfo
    );

VOID
nicStartBeacon (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicStopBeacon (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicResumeBeacon (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicPauseBeacon (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicExistIBSS (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessAbnormalInterrupt(
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessWatchDogInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcessTsfDriftInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcessBeaconTxOkInterrupt (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_TCP_IP_CHKSUM_OFFLOAD
VOID
nicSetCSUMOffload (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4CSUMFlags
    );
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

VOID
nicRRAverageEnable (
    IN P_ADAPTER_T  prAdapter
    );

VOID
nicRRAverageDisable (
    IN P_ADAPTER_T  prAdapter
    );

VOID
nicRRTriggerEnable (
    IN P_ADAPTER_T  prAdapter,
    IN RCPI         rRCPIUpperThreshold,
    IN RCPI         rRCPILowerThreshold
    );

VOID
nicRRTriggerDisable (
    IN P_ADAPTER_T  prAdapter
    );

VOID
nicRRGetRCPI (
    IN P_ADAPTER_T  prAdapter,
    OUT P_RCPI      prRCPI
    );

VOID
nicProcessRcpiInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessBeaconTimeoutInterrupt(
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessTxNullFailInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessTxTriggerFailInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessTxPsPollFailInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessRxBmcTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessApsdTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessPsPollTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicLogicReset (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicSetUnicastAddr (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicSetMulticastAddrList (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 paucMulticastAddrList,
    IN UINT_8 ucNum
    );

WLAN_STATUS
nicGetChannelBandFromFreq (
    IN  P_ADAPTER_T   prAdapter,
    IN  UINT_32       u4ChannelFreq,
    OUT P_ENUM_BAND_T peBand,
    OUT PUINT_8      pucChannelNum
    );

BOOLEAN
nicEepromWrite16 (
    IN P_ADAPTER_T  	prAdapter,
    IN UINT_8      	  	ucEepromWordOffset,
    IN UINT_16          u2EepromData
    );


BOOLEAN
nicEepromRead16 (
    IN      P_ADAPTER_T  	prAdapter,
    IN      UINT_8      	ucEepromWordOffset,
    OUT     PUINT_16        pu2EepromData
    );

UINT_16
nicEepromGetSize (
    IN      P_ADAPTER_T  	prAdapter
    );

/*----------------------------------------------------------------------------*/
/* Routines in nic_hw_scan.c                                                  */
/*----------------------------------------------------------------------------*/
VOID
nicHwScanGetLastScannedChnlFreq (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8     pucChnlIdx,
    IN PUINT_8     pucBandIdx
    );

VOID
nicHwScanConfig (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
nicHwScanConfigRestore (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
nicHwScanDisable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
nicHwScanEnable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
nicHwScanScannedChnlFreqBand (
    IN  P_ADAPTER_T     prAdapter,
    IN  UINT_16         u2QoSCtrlInfo,
    OUT PUINT_8         pucChnlNum,
    OUT P_ENUM_BAND_T   prBandNum
    );

VOID
nicProcesScanDoneInterrupt(
    IN P_ADAPTER_T      prAdapter
    );

BOOLEAN
nicpmStepBeaconTimeout (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgStepUp
    );

VOID
nicpmResetBeaconContentCheck (
    IN P_ADAPTER_T prAdapter
    );

/*----------------------------------------------------------------------------*/
/* Routines in nic_pwr_mgt.c                                                  */
/*----------------------------------------------------------------------------*/
BOOLEAN
nicpmRegInit (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
nicpmCalSlowClock (
    IN P_ADAPTER_T prAdapter,
    OUT P_PM_SLOW_CLOCK_COUNT_T prSlowClkCnt
    );

VOID
nicpmSetupWakeupGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN P_PM_SLOW_CLOCK_COUNT_T prSlowClkCnt
    );

BOOLEAN
nicpmPatternMatchInit (
    IN P_ADAPTER_T  prAdapter
    );

BOOLEAN
nicpmSetupPmInfoInBss (
    IN P_ADAPTER_T              prAdapter,
    IN P_PM_CONN_SETUP_INFO_T   prPmConnSetupInfo
    );

BOOLEAN
nicpmSetupPmInfoInAdhoc (
    IN P_ADAPTER_T          prAdapter,
    IN P_PM_CONN_SETUP_INFO_T   prPmConnSetupInfo,
    IN BOOLEAN                  fgCreateIbss
    );

VOID
nicpmSwitchPwrMgtMode (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIsPwrSave
    );

VOID
nicpmLeaveLowPower (
    IN P_ADAPTER_T prAdapter
    );

UINT_16
nicpmGetBeaconEarlyValue (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicpmAdjustWakeupGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2BeaconEarlyOffset
    );

VOID
nicpmEnterLowPower (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgEnableGlobalInt
    );

VOID
nicpmLeaveLowPower (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
nicpmPatternConfigArpFilter (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_32     pu4IpAddr
    );

VOID
nicpmAbortPmFunc (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicpmSetupServicePeriodMechanism (
    IN P_ADAPTER_T  prAdapter,
    UINT_32         u4ValidateSP,
    UINT_32         u4InvalidateSP
    );

BOOLEAN
nicpmIfAdhocStaMaster (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicpmPowerOn (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicpmPowerOff (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicpmEnableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicpmDisableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicpmSetTsfDriftWindow (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8    ucDriftWindow
    );

VOID
nicpmConfigPattern (
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
    );

VOID
nicpmConfigPatternSearchFunc (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgBcA1En,
    IN BOOLEAN      fgMcA1En,
    IN BOOLEAN      fgUcA1En,
    IN BOOLEAN      fgBcA1MatchDrop,
    IN BOOLEAN      fgMcA1MatchDrop,
    IN BOOLEAN      fgUcA1MatchDrop,
    IN BOOLEAN      fgIpv6MatchCtrl
    );

VOID
nicpmResetTSFTimer (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessTbttInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcessBeaconTxRxOkInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicProcessAtimWindowTimeoutInterrupt (
    IN P_ADAPTER_T      prAdapter
    );

VOID
nicSetHwTxByBasicRate (
    IN  P_ADAPTER_T prAdapter,
    IN BOOLEAN fgUseBasicRate
    );

VOID
nicGetRandomNumber (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_16 pu2Number
    );

BOOLEAN
nicGetChannelNumByFreq (
    IN UINT_32      u4FreqInkHz,
    OUT UINT_8      *pucChannelNum,
    OUT ENUM_BAND_T *peBand
    );

UINT_32
nicGetFreqByChannelNum (
    IN UINT_8          ucChannelNum,
    IN ENUM_BAND_T     eBand
    );



VOID
nicThermoTimerInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicThermoTimerTimeOut (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicThermoInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicThermoIntInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicProcessALCInterrupt (
    IN P_ADAPTER_T prAdapter
    );


VOID
nicThermoIntUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicThermoTimerUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicThermoUnInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicSetupOpChnlList (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2CountryCode,
    IN BOOLEAN fgUpdateIbssChannelInfo
    );

BOOLEAN
nicSetTxFilterForJP(
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    );

VOID
nicSetGPIO1Mode (
    IN P_ADAPTER_T prAdapter
    );

VOID
nicSetGPIO2Mode (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PARAM_GPIO2_MODE_T eGPIO2_Mode
    );

void
nicRestoreSpiDefMode(
    IN P_ADAPTER_T prAdapter
    );

VOID
nicCompose802_11DataFrame (
    IN  P_ADAPTER_T     prAdapter,
    IN  PUINT_8         pucBuffer,
    IN  UINT_8          ucTid,
    IN  BOOLEAN         privacy,
    IN  P_NATIVE_PACKET pvWlanBody,
    OUT PUINT_8         pucWlanHeaderLen
    );

VOID
nicpmSetupMoreDataTrigger (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    );

VOID
nicpmSetupBeaconTimeoutDetection (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    );

#endif /* _NIC_H */

