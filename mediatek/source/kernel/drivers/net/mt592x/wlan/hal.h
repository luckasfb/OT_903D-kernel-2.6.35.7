





#ifndef _HAL_H
#define _HAL_H




//4 2006/09/21, mikewu, Queue Init
typedef struct _QINFO_T {
    UINT_8          ucQueueId;
    UINT_32         u4SizeDW;
    UINT_32         u4OffsetDW;
} QINFO_T, *P_QINFO_T;




/* Macros for flag operations for the Adapter structure */
#define HAL_SET_FLAG(_M, _F)             ((_M).u4Flag |= (_F))
#define HAL_CLEAR_FLAG(_M, _F)           ((_M).u4Flag &= ~(_F))
#define HAL_TEST_FLAG(_M, _F)            ((_M).u4Flag & (_F))
#define HAL_TEST_FLAGS(_M, _F)           (((_M).u4Flag & (_F)) == (_F))

#if defined(_HIF_SDIO)

#if 0
#define HAL_MCR_RD(_prAdapter, _u4Offset, _pu4Value) \
    { \
        if (HAL_TEST_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR) == FALSE) { \
            if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
                ASSERT(0); \
            } \
            if (kalDevRegRead(_prAdapter->prGlueInfo, _u4Offset, _pu4Value) == FALSE) {\
                HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
                fgIsBusAccessFailed = TRUE; \
                DBGLOG(HAL, ERROR, ("HAL_MCR_RD access fail! 0x%lx: 0x%lx \n", (UINT_32)(_u4Offset), (UINT_32)*(_pu4Value))); \
            } \
        } else { \
            DBGLOG(HAL, WARN, ("ignore HAL_MCR_RD access! 0x%lx\n", (UINT_32)(_u4Offset))); \
        } \
    }
#endif
#define HAL_MCR_RD(_prAdapter, _u4Offset, _pu4Value) \
{  \
		HAL_CLEAR_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
		fgIsBusAccessFailed = FALSE; \
		if (kalDevRegRead(_prAdapter->prGlueInfo, _u4Offset, _pu4Value) == FALSE) {\
				HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
				fgIsBusAccessFailed = TRUE; \
				DBGLOG(HAL, ERROR, ("HAL_MCR_RD access fail! 0x%lx: 0x%lx \n", (UINT_32)(_u4Offset), (UINT_32)*(_pu4Value))); \
		} \
}
#if 0
#define HAL_MCR_WR(_prAdapter, _u4Offset, _u4Value) \
{ \
		if (HAL_TEST_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR) == FALSE) { \
				if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
						ASSERT(0); \
				} \
				if (kalDevRegWrite(_prAdapter->prGlueInfo, _u4Offset, _u4Value) == FALSE) {\
						HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
						fgIsBusAccessFailed = TRUE; \
						DBGLOG(HAL, ERROR, ("HAL_MCR_WR access fail! 0x%lx: 0x%lx \n", (UINT_32)(_u4Offset), (UINT_32)(_u4Offset))); \
				} \
		} else { \
				DBGLOG(HAL, WARN, ("ignore HAL_MCR_WR access! 0x%lx: 0x%lx \n", (UINT_32)(_u4Offset), (UINT_32)(_u4Offset))); \
		} \
}
#endif
#define HAL_MCR_WR(_prAdapter, _u4Offset, _u4Value) \
{ \
		HAL_CLEAR_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
		fgIsBusAccessFailed = FALSE; \
		if (kalDevRegWrite(_prAdapter->prGlueInfo, _u4Offset, _u4Value) == FALSE) {\
				HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
				fgIsBusAccessFailed = TRUE; \
				DBGLOG(HAL, ERROR, ("HAL_MCR_WR access fail! 0x%lx: 0x%lx \n", (UINT_32)(_u4Offset), (UINT_32)(_u4Offset))); \
		} \
}
#if 0
#define HAL_PORT_RD(_prAdapter, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) \
{ \
		fgResult = FALSE; \
		if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
				ASSERT(0); \
		} \
		if (HAL_TEST_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR) == FALSE) { \
				if (kalDevPortRead(_prAdapter->prGlueInfo, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) == FALSE) {\
						HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
						fgIsBusAccessFailed = TRUE; \
						DBGLOG(HAL, ERROR, ("HAL_PORT_RD access fail! 0x%x\n", _u2Port)); \
				} \
				else \
				fgResult = TRUE; \
		} else { \
				DBGLOG(HAL, WARN, ("ignore HAL_PORT_RD access! 0x%x\n", _u2Port)); \
		} \
}
#endif
#define HAL_PORT_RD(_prAdapter, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) \
{ \
		fgResult = FALSE; \
		if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
				ASSERT(0); \
		} \
        HAL_CLEAR_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
        fgIsBusAccessFailed = FALSE; \
		if (HAL_TEST_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR) == FALSE) { \
				if (kalDevPortRead(_prAdapter->prGlueInfo, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) == FALSE) {\
						HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
						fgIsBusAccessFailed = TRUE; \
						DBGLOG(HAL, ERROR, ("HAL_PORT_RD access fail! 0x%x\n", _u2Port)); \
				} \
				else \
				fgResult = TRUE; \
		} else { \
				DBGLOG(HAL, WARN, ("ignore HAL_PORT_RD access! 0x%x\n", _u2Port)); \
		} \
}
#if 0
#define HAL_PORT_WR(_prAdapter, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) \
    { \
        fgResult = FALSE; \
        if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
            ASSERT(0); \
        } \
        if (HAL_TEST_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR) == FALSE) { \
            if (kalDevPortWrite(_prAdapter->prGlueInfo, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) == FALSE) {\
                HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
                fgIsBusAccessFailed = TRUE; \
                DBGLOG(HAL, ERROR, ("HAL_PORT_WR access fail! 0x%x\n", _u2Port)); \
            } \
            else \
                fgResult = TRUE; \
        } else { \
            DBGLOG(HAL, WARN, ("ignore HAL_PORT_WR access! 0x%x\n", _u2Port)); \
        } \
    }
#endif 
#define HAL_PORT_WR(_prAdapter, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) \
    { \
        fgResult = FALSE; \
        if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
            ASSERT(0); \
        } \
        HAL_CLEAR_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
        fgIsBusAccessFailed = FALSE; \
        if (HAL_TEST_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR) == FALSE) { \
            if (kalDevPortWrite(_prAdapter->prGlueInfo, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) == FALSE) {\
                HAL_SET_FLAG(_prAdapter->rArbInfo, ADAPTER_FLAG_HW_ERR); \
                fgIsBusAccessFailed = TRUE; \
                DBGLOG(HAL, ERROR, ("HAL_PORT_WR access fail! 0x%x\n", _u2Port)); \
            } \
            else \
                fgResult = TRUE; \
        } else { \
            DBGLOG(HAL, WARN, ("ignore HAL_PORT_WR access! 0x%x\n", _u2Port)); \
        } \
    }
#else /* #if defined(_HIF_SDIO) */
#define HAL_MCR_RD(_prAdapter, _u4Offset, _pu4Value) \
    { \
        if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
            ASSERT(0); \
        } \
        kalDevRegRead(_prAdapter->prGlueInfo, _u4Offset, _pu4Value); \
    }

#define HAL_MCR_WR(_prAdapter, _u4Offset, _u4Value) \
    { \
        if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
            ASSERT(0); \
        } \
        kalDevRegWrite(_prAdapter->prGlueInfo, _u4Offset, _u4Value); \
    }

#define HAL_PORT_RD(_prAdapter, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) \
    { \
        if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
            ASSERT(0); \
        } \
        kalDevPortRead(_prAdapter->prGlueInfo, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize); \
    }

#define HAL_PORT_WR(_prAdapter, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize) \
    { \
        if (_prAdapter->rAcpiState == ACPI_STATE_D3) { \
            ASSERT(0); \
        } \
        kalDevPortWrite(_prAdapter->prGlueInfo, _u2Port, _u2Len, _pucBuf, _u2ValidBufSize); \
    }
#endif /* #if defined(_HIF_SDIO) */


#define HAL_MCR_RD_AND_WAIT(_pAdapter, _register, _pReadValue, _waitCondition, _waitDelay, _waitCount, _status) \
        { \
            UINT_32 count; \
            (_status) = FALSE; \
            for (count = 0; count < (_waitCount); count++) { \
                HAL_MCR_RD((_pAdapter), (_register), (_pReadValue)); \
                if ((_waitCondition)) { \
                    (_status) = TRUE; \
                    break; \
                } \
                kalUdelay((_waitDelay)); \
            } \
        }


#define HAL_MCR_WR_AND_WAIT(_pAdapter, _register, _writeValue, _busyMask, _waitDelay, _waitCount, _status) \
        { \
            UINT_32 u4Temp; \
            UINT_32 u4Count = _waitCount; \
            (_status) = FALSE; \
            HAL_MCR_WR((_pAdapter), (_register), (_writeValue)); \
            do { \
                kalUdelay((_waitDelay)); \
                HAL_MCR_RD((_pAdapter), (_register), &u4Temp); \
                if (!(u4Temp & (_busyMask))) { \
                    (_status) = TRUE; \
                    break; \
                } \
                u4Count--; \
            } while (u4Count); \
        }

#define HAL_INTR_ENABLE(prAdapter) \
    HAL_MCR_WR(prAdapter, \
        MCR_HLPCR, \
        HLPCR_ENABLE_GINT)

#define HAL_INTR_ENABLE_AND_LP_OWN_SET(prAdapter) \
    HAL_MCR_WR(prAdapter, \
        MCR_HLPCR, \
        (HLPCR_ENABLE_GINT | HLPCR_LP_OWN_SET))

#define HAL_LP_OWN_SET(prAdapter) \
    HAL_MCR_WR(prAdapter, \
        MCR_HLPCR, \
        HLPCR_LP_OWN_SET)

#define HAL_MSCR_IS_RX_G0_EN(_prAdapter, _pfgEn) \
        { \
            UINT_32 u4McrMCSR = 0;\
            HAL_MCR_RD(_prAdapter, MCR_MSCR, &u4McrMCSR);\
            *(_pfgEn) = (BOOLEAN)(u4McrMCSR & MSCR_RX_STATUS_G0_EN);\
        }

#define HAL_MSCR_IS_RX_G1_EN(_prAdapter, _pfgEn) \
        { \
            UINT_32 u4McrMCSR = 0;\
            HAL_MCR_RD(_prAdapter, MCR_MSCR, &u4McrMCSR);\
            *(_pfgEn) = (BOOLEAN)(u4McrMCSR & MSCR_RX_STATUS_G1_EN);\
        }

#define HAL_MSCR_IS_RX_G2_EN(_prAdapter, _pfgEn) \
        { \
            UINT_32 u4McrMCSR = 0;\
            HAL_MCR_RD(_prAdapter, MCR_MSCR, &u4McrMCSR);\
            *(_pfgEn) = (BOOLEAN)(u4McrMCSR & MSCR_RX_STATUS_G2_EN);\
        }

#define HAL_MSCR_IS_RX_G0_RSSI(_prAdapter, _pfgRssi) \
        { \
            UINT_32 u4McrMCSR = 0;\
            HAL_MCR_RD(_prAdapter, MCR_MSCR, &u4McrMCSR);\
            *(_pfgRssi) = (BOOLEAN)(u4McrMCSR & MSCR_RX_BBP_RXSTS_SEL);\
        }

#define HAL_INTR_DISABLE(prAdapter) \
    HAL_MCR_WR(prAdapter, \
        MCR_HLPCR, \
        HLPCR_DISABLE_GINT)

#define HAL_MCR_IER_WR(_prAdapter,_u4IER) \
    HAL_MCR_WR(_prAdapter, MCR_HIER, _u4IER);

#define HAL_MCR_SCIER_WR(_prAdapter,_u4SCIER) \
    HAL_MCR_WR(_prAdapter, MCR_HSCIER, _u4SCIER);

#define HAL_READ_INTR_STATUS(_prAdapter, _u4IntrStatus) \
{ \
    HAL_MCR_RD(_prAdapter, \
        MCR_HISR, \
        &_u4IntrStatus);\
}

#define HAL_WRITE_INTR_STATUS(_prAdapter, _u4IntrStatus) \
    HAL_MCR_WR(_prAdapter, \
        MCR_HISR, \
        _u4IntrStatus)


/* This macro is used to get the current TSF timer value. */
#define HAL_GET_CURRENT_TSF(_prAdapter, _pu8Tsf) \
        do { \
            UINT_32 (_ul1), (_ul2); \
            HAL_MCR_RD(_prAdapter, MCR_LTTR1, &(_ul1)); \
            HAL_MCR_RD(_prAdapter, MCR_LTTR0, &((_pu8Tsf)->u.LowPart)); \
            HAL_MCR_RD(_prAdapter, MCR_LTTR1, &(_ul2)); \
            if ((_ul1) == (_ul2)) { \
                (_pu8Tsf)->u.HighPart = (_ul1); \
            } else if ((_pu8Tsf)->u.LowPart > 0x80000000) { \
                (_pu8Tsf)->u.HighPart = (_ul1); \
            } else { \
                (_pu8Tsf)->u.HighPart = (_ul2); \
            } \
        } while(0)

#define HAL_CLICK_GPIO0(_pAdapter) \
        { \
            UINT_32 _u4Temp; \
            HAL_MCR_RD(_pAdapter, MCR_SCR, &_u4Temp); \
            _u4Temp ^= SCR_GPIO0_WDATA; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO0_ENABLE_OUTPUT_MODE); \
            _u4Temp ^= SCR_GPIO0_WDATA; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO0_ENABLE_OUTPUT_MODE); \
        }

#define HAL_CLICK_GPIO1(_pAdapter) \
        { \
            UINT_32 _u4Temp; \
            HAL_MCR_RD(_pAdapter, MCR_SCR, &_u4Temp); \
            _u4Temp ^= SCR_GPIO1_WDATA; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO1_ENABLE_OUTPUT_MODE); \
            _u4Temp ^= SCR_GPIO1_WDATA; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO1_ENABLE_OUTPUT_MODE); \
        }

#define HAL_SET_GPIO0_BIT(_pAdapter, _fgBitHigh) \
        { \
            UINT_32 _u4Temp; \
            HAL_MCR_RD(_pAdapter, MCR_SCR, &_u4Temp); \
            _u4Temp &= ~SCR_GPIO0_WDATA; \
            _u4Temp |= _fgBitHigh ? SCR_GPIO1_WDATA : 0; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO0_ENABLE_OUTPUT_MODE); \
        }

#define HAL_SET_GPIO1_BIT(_pAdapter, _fgBitHigh) \
        { \
            UINT_32 _u4Temp; \
            HAL_MCR_RD(_pAdapter, MCR_SCR, &_u4Temp); \
            _u4Temp &= ~SCR_GPIO1_WDATA; \
            _u4Temp |= _fgBitHigh ? SCR_GPIO1_WDATA : 0; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO1_ENABLE_OUTPUT_MODE); \
        }

#define HAL_SET_GPIO2_BIT(_pAdapter, _fgBitHigh) \
        { \
            UINT_32 _u4Temp; \
            HAL_MCR_RD(_pAdapter, MCR_SCR, &_u4Temp); \
            _u4Temp &= ~SCR_GPIO2_WDATA; \
            _u4Temp |= _fgBitHigh ? SCR_GPIO2_WDATA : 0; \
            HAL_MCR_WR(_pAdapter, MCR_SCR, _u4Temp | SCR_GPIO2_ENABLE_OUTPUT_MODE); \
        }

#if CFG_WORKAROUND_BG_SSID_SCAN_DONE
#define HAL_DEL_INT_SCAN_DONE_EVENT(_prAdapter) \
    { \
        _prAdapter->au4IntStatus[INT_HISR] &= ~HISR_SCAN_DONE; \
    }

#define HAL_ADD_INT_SCAN_DONE_EVENT(_prAdapter) \
    { \
        _prAdapter->au4IntStatus[INT_HISR] |= HISR_SCAN_DONE; \
    }
#endif

/*----------------------------------------------------------------------------*/
/* TX_STATUS Wrapper for HAL                                                  */
/*----------------------------------------------------------------------------*/
#define HAL_TX_STATUS_GET_QUEUE_INDEX(prTxStatus)       \
    TX_STATUS_GET_QUEUE_INDEX(prTxStatus)

#define HAL_IS_MORE_TX_STATUS(prTxStatus)       TX_STATUS_TEST_MORE_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_OK(prTxStatus)         TX_STATUS_TEST_OK_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_ERR(prTxStatus)        TX_STATUS_TEST_ERR_FLAG(prTxStatus)

#define HAL_GET_CURRENT_PS_STATE(prTxStatus)    TX_STATUS_GET_CURRENT_PS_STATE(prTxStatus)

#define HAL_IS_TX_STATUS_PORT_CTRL(prTxStatus)          \
    TX_STATUS_TEST_PORT_CTRL_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_LIFETIME_ERR(prTxStatus)       \
    TX_STATUS_TEST_LIFETIME_ERR_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_RTS_RETRY_ERR(prTxStatus)      \
    TX_STATUS_TEST_RTS_RETRY_ERR_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_MPDU_RETRY_ERR(prTxStatus)     \
    TX_STATUS_TEST_MPDU_RETRY_ERR_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_BMC(prTxStatus)        TX_STATUS_TEST_BMC_FLAG(prTxStatus)

#define HAL_GET_MPDU_TX_CNT(prTxStatus)         TX_STATUS_GET_MPDU_TX_CNT(prTxStatus)

#define HAL_IS_TX_STATUS_1X_PKT(prTxStatus)     TX_STATUS_TEST_1X_FLAG(prTxStatus)

#define HAL_IS_TX_STATUS_BR_PKT(prTxStatus)     TX_STATUS_TEST_BR_FLAG(prTxStatus)

#define HAL_GET_MPDU_RTS_OK_CNT(prTxStatus)     TX_STATUS_GET_RTS_OK_CNT(prTxStatus)

#define HAL_GET_MPDU_RTS_FAIL_CNT(prTxStatus)   TX_STATUS_GET_RTS_FAIL_CNT(prTxStatus)

#define HAL_GET_MPDU_WLAN_INDEX(prTxStatus)     TX_STATUS_GET_WLAN_INDEX(prTxStatus)


/*----------------------------------------------------------------------------*/
/* RX_STATUS Wrapper for HAL                                                  */
/*----------------------------------------------------------------------------*/
#define HAL_RX_STATUS_IS_QOS(_prRxStatus)       RX_STATUS_IS_QoS(_prRxStatus->u2StatusFlag)

#define HAL_RX_STATUS_GET_SEQCTRL(_prRxStatus)  _prRxStatus->u2SeqCtrl

#define HAL_RX_STATUS_IS_RETRY(_prRxStatus)     _prRxStatus->ucFC & RX_STATUS_FC_RETRY

#define HAL_RX_STATUS_GET_SEQCTRL(_prRxStatus)  _prRxStatus->u2SeqCtrl


#define HAL_RX_STATUS_BSSID_MATCHED(_prRxStatus) RX_STATUS_IS_BSM(_prRxStatus->u2StatusFlag)


#define HAL_RX_STATUS_GET_RCPI(_prRxStatus)     RX_STATUS_GET_RCPI(_prRxStatus)

#define HAL_RX_STATUS_GET_TA(_prRxStatus)       RX_STATUS_GET_TA(_prRxStatus)

#define HAL_IS_MORE_RX_PKT(prRxStatus)          RX_STATUS_TEST_MORE_FLAG(prRxStatus)

#define HAL_RX_STATUS_IS_TCL(_prRxStatus)       RX_STATUS_IS_TCL(_prRxStatus->u2StatusFlag)

/*----------------------------------------------------------------------------*/
/* Thermo Wrapper for HAL                                                     */
/*----------------------------------------------------------------------------*/
#define HAL_THERMO_2_DEGREE(_prEEPROMCtrl, _ucThermo, _cTemperature ) \
{\
    INT_8 _cAbsTemp;\
    INT_8 _ucThermoVal;\
    UINT_8 _ucThermoSlope;\
    _cAbsTemp = _prEEPROMCtrl->cAbsTemp;\
    _ucThermoVal = _prEEPROMCtrl->ucThermoValue;\
    _ucThermoSlope = _prEEPROMCtrl->ucThermoSensorSlop;\
    _cTemperature = ((INT_8)(_ucThermo - _ucThermoVal) *EEPROM_VGA_THERMO_SLOPE_BASE)/_ucThermoSlope;\
    _cTemperature += _cAbsTemp;\
}

#define HAL_DEGREE_2_THERMO(_prEEPROMCtrl, _cTemperature, _ucThermo ) \
{\
    INT_8 _cAbsTemp;\
    INT_8 _ucThermoVal;\
    UINT_8 _ucThermoSlope;\
    _cAbsTemp = _prEEPROMCtrl->cAbsTemp;\
    _ucThermoVal = _prEEPROMCtrl->ucThermoValue;\
    _ucThermoSlope = _prEEPROMCtrl->ucThermoSensorSlop;\
    _ucThermo = ((_cTemperature - _cAbsTemp)*_ucThermoSlope)/EEPROM_VGA_THERMO_SLOPE_BASE ;\
    _ucThermo += _ucThermoVal;\
}

// constant definition
#define HAL_HW_SCAN_ENTRY_NUMBER    16


BOOLEAN
halVerifyChipID (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
halVerifyRevID (
    IN P_ADAPTER_T prAdapter
    );

VOID
halMCRInit (
    IN P_ADAPTER_T prAdapter
    );

BOOL
halGetQueueInfo (
    IN P_ADAPTER_T prAadapter,
    IN ENUM_QUEUE_ID_T rQueueId,
    OUT PUINT_32 pu4DwOffset,
    OUT PUINT_32 pu4DwSize
    );

VOID
halHWQueueInit (
    IN P_ADAPTER_T prAdapter
    );


#if CFG_TX_AGGREGATE_HW_FIFO
VOID
halAggregateHWTxDataQueue (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgAggregateTxFifo
    );
#endif /* CFG_TX_AGGREGATE_HW_FIFO */


#if CFG_INTERNAL_MEMORY_TEST
VOID
halHWQueueMemoryTest (
    IN P_ADAPTER_T prAdapter
    );
#endif /* CFG_INTERNAL_MEMORY_TEST */

#if CFG_DATA_PORT_ACCESS_TEST
VOID
halBeaconQueueDataPortTest (
    IN P_ADAPTER_T prAdapter
    );
#endif /* CFG_DATA_PORT_ACCESS_TEST */

VOID
halWlanTableInit(
    IN  P_ADAPTER_T         prAdapter
    );

BOOLEAN
halWlanTableBusyStatus(
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
halWlanTableAccess(
    IN  P_ADAPTER_T         prAdapter,
    IN  PVOID               pvWlanCtrl,
    IN  UCHAR               ucEntryIndex,
    IN  UCHAR               ucMode
    );

BOOLEAN
halWlanTableVerify(
    IN  P_ADAPTER_T         prAdapter,
    IN  PUINT_8             pucWKey,
    IN  UCHAR               ucEntryIndex,
    IN  UCHAR               ucLen
    );

VOID
halPTACtrl(
    IN  P_ADAPTER_T         prAdapter,
    IN  BOOLEAN             fgEnable
    );

VOID
halPTASetConfig(
    IN  P_ADAPTER_T         prAdapter,
    IN  P_PTA_PARAM_T       prPtaParam
    );

VOID
halChipReset (
    IN P_ADAPTER_T prAdapter
    );

VOID
halSetSlotTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4SlotTime
    );

VOID
halReadIntStatus (
    IN P_ADAPTER_T prAdapter,
    OUT UINT_32 au4IntStatus[]
    );

WLAN_STATUS
halRxFillRFB (
    IN P_ADAPTER_T prAdapter,
    IN OUT P_SW_RFB_T prSWRfb
    );

VOID
halRxReadDone (
    IN P_ADAPTER_T prAdapter
    );

VOID
halSetRxHeaderTranslation(
    IN P_ADAPTER_T      prAdapter,
    IN BOOLEAN          fgEnable
    );

VOID
halTxComposeFrameControlBlock (
    IN P_ADAPTER_T  prAdapter,
    IN P_SW_TFCB_T  prSwTfcb,
    IN PUINT_8      pucDA,
    OUT P_HW_TFCB_T prHwTfcb
    );

#if CFG_TX_FRAGMENT
VOID
halTxComposeFirstFragInCoalescingBuf (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN P_SW_TFCB_T      prSwTfcb,
    IN OUT PUINT_8      pucOutputBuf
    );

PUINT_8
halTxComposeTfcbFragFrame (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN P_SW_TFCB_T      prSwTfcb
    );

#if SUPPORT_WAPI
PUINT_8
halTxComposeWpiPktInCoalescingBuf(
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN P_SW_TFCB_T      prSwTfcb
    );
#endif
#endif /* CFG_TX_FRAGMENT */


#if CFG_SDIO_TX_ENHANCE
BOOL
halTxAggregateMpdu (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_16      u2OverallBufferLength,
    IN PPUINT_8     ppucOutputBuf
    );

BOOL
halTxLeftClusteredMpdu (
    IN P_ADAPTER_T  prAdapter
    );

#endif /* CFG_SDIO_TX_ENHANCE */


BOOL
halTxTfcbs (
    IN P_ADAPTER_T      prAdapter,
    IN P_MSDU_INFO_T    prMsduInfo,
    IN P_QUE_T          prTFCBsReadyToHwFIFO
#if CFG_SDIO_TX_ENHANCE
    ,IN BOOLEAN         fgIsAggregate
#endif /* CFG_SDIO_TX_ENHANCE */
    );

#if CFG_TX_DBG_TFCB_CHKSUM || CFG_TX_DBG_SEQ_NUM
VOID
halTxEnableDebugOption (
    IN P_ADAPTER_T  prAdapter
    );
#endif /* CFG_TX_DBG_TFCB_CHKSUM || CFG_TX_DBG_SEQ_NUM */

BOOL
halTxReadTxStatus (
    IN P_ADAPTER_T      prAdapter,
    OUT P_TX_STATUS_T   prTxStatusDone,
    OUT PP_SW_TFCB_T    pprSwTfcbDone
    );

VOID
halFlushStopQueues (
    IN P_ADAPTER_T prAadapter,
    IN UINT_8 ucFlushQues,
    IN UINT_8 ucStopQues
    );

VOID
halStartQueues (
    IN P_ADAPTER_T prAadapter,
    IN UINT_8 ucStartQues
    );

/* */
P_RF_CHANNEL_PROG_ENTRY
halRFGetRFChnlProgEntry (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    );

/* HW SCAN related */
WLAN_STATUS
halSetRFSwitchChnlInst (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand,
    IN BOOLEAN     fgSetBBTXfilterforJp
    );

VOID
halHwScanEnable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
halHwScanSetBgScanParam (
    IN P_ADAPTER_T          prAdapter,
    IN P_BG_SCAN_CONFIG_T   prBgScanCfg
    );

VOID
halHwScanSetScanChannelSwitchInst (
    IN P_ADAPTER_T          prAdapter,
    IN P_RF_CHANNEL_INFO_T  parChnlInfoList,
    IN UINT_8               ucNumOfScanChnl
    );

WLAN_STATUS
halHwScanSetPerChnlInst (
    IN P_ADAPTER_T          prAdapter,
    IN P_RF_CHANNEL_INFO_T  parChnlInfoList,
    OUT PUINT_32             pu4ChSwInstNum
    );

VOID
halHwScanSetInitialInst (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
halHwScanSetTerminateInst (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

BOOLEAN
halHwScanDisable (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
halHwScanRegisterInit (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );

VOID
halHwScanRegisterUnInit (
    IN P_ADAPTER_T prAdapter,
    IN P_SCAN_CONFIG_T prScanCfg,
    IN ENUM_HW_SCAN_MODE_T eScanMode
    );


/* power management related */
VOID
halpmSwitchPwrMgtBit (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgPwrMgtBit
    );

VOID
halpmEnterLowPower (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN     fgEnableGlobalInt
    );

BOOLEAN
halpmLeaveLowPower (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4LpOwnBackPollTimeUs
    );

BOOLEAN
halpmCalSlowClock (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_8 pucSlowClockCount,
    OUT PUINT_8 pucSlowClockCount_10,
    OUT PUINT_8 pucSlowClockCount_100,
    OUT PUINT_8 pucSlowClockCount_1000
    );

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
    IN UINT_8      ucAgeQoSNullTid,
    IN UINT_8      ucTrgThr,
    IN UINT_16     u2HwTxLifeTime
    );

VOID
halpmConfigLowPowerSettingInAdhoc (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_16      u2BcnIntv,
    IN UINT_16      u2AtimWindow,
    IN UINT_8       ucMultiTbttWake,
    IN UINT_16      u2HwTxLifeTime,
    IN BOOLEAN      fgTbttCalCompansate
    );

VOID
halpmEnableLowPowerFunctionsInBss (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgNeedTbttWakeup
    );

VOID
halpmEnableLowPowerFunctionsInAdhoc (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgCreateIbss
    );

VOID
halpmSetBeaconTimeout (
    IN P_ADAPTER_T  prAdapter,
    UINT_8          ucMinTimeoutValue,
    UINT_16         u2MaxTimeoutValue,
    BOOLEAN         fgMinTimeoutValid,
    BOOLEAN         fgMaxTimeoutValid,
    UINT_8          ucTimeoutCount
    );

VOID
halpmSetTimeout (
    IN P_ADAPTER_T  prAdapter,
    UINT_8          ucMinTimeoutValue,
    UINT_16         u2MaxTimeoutValue,
    BOOLEAN         fgMinTimeoutValid,
    BOOLEAN         fgMaxTimeoutValid,
    UINT_8          ucTimeoutCount
    );

VOID
halpmSetNullTimeout (
    IN P_ADAPTER_T  prAdapter,
    UINT_8          ucMinTimeoutValue,
    UINT_16         u2MaxTimeoutValue,
    BOOLEAN         fgMinTimeoutValid,
    BOOLEAN         fgMaxTimeoutValid,
    UINT_8          ucTimeoutCount
    );

VOID
halpmEnableBeaconEarlyCheck (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmEnableLowPowerFunctions (
    IN P_ADAPTER_T prAdapter
    );

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
    );

VOID
halpmCalculateGuardTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 uc32kSlowCount,
    IN UINT_8 uc32kSlowCount_10,
    IN UINT_8 uc32kSlowCount_100,
    IN UINT_8 uc32kSlowCount_1000,
    OUT PUINT_16 pu2GuardTimePhase2,
    OUT PUINT_16 pu2GuardTimePhase3
    );

VOID
halpmRegInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmSetupWakeupGuardTime (
    IN P_ADAPTER_T prAdapter,
    OUT UINT_16 u2GuardTimePhase2,
    OUT UINT_16 u2GuardTimePhase3
    );

UINT_16
halpmGetBeaconEarlyValue (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmSetupServicePeriodMechanism (
    IN P_ADAPTER_T  prAdapter,
    UINT_32         u4ValidateSP,
    UINT_32         u4InvalidateSP
    );

VOID
halpmConfigPattern (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucIndex,
    IN BOOLEAN      fgCheckUcA1,
    IN BOOLEAN      fgCheckBcA1,
    IN BOOLEAN      fgCheckMcA1,
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
halpmConfigPatternSearchFunction (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgBcA1En,
    IN BOOLEAN      fgMcA1En,
    IN BOOLEAN      fgUcA1En,
    IN BOOLEAN      fgBcA1MatchDrop,
    IN BOOLEAN      fgMcA1MatchDrop,
    IN BOOLEAN      fgUcA1MatchDrop,
    IN BOOLEAN      fgIpv6MatchCtrl
    );

BOOLEAN
halpmIfAdhocStaMaster (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmPowerOn (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 uc32kSlowCount
    );

VOID
halpmPowerOff (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmEnableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmDisableTimeoutCounter (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmSetTsfDriftWindow (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8    ucDriftWindow
    );

VOID
halpmSetAtimWindowValue (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16    u2AtimWindow
    );

VOID
halpmResetTSFTimer (
    IN P_ADAPTER_T prAdapter
    );

VOID
halpmDisableLowPowerFunctions (
    IN P_ADAPTER_T prAdapter
    );

/* autorate */

VOID
halARInitialize (
    IN  P_ADAPTER_T prAdapter
    );

VOID
halAREnable (
    IN  P_ADAPTER_T prAdapter
    );

VOID
halARSetParam (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2FailCount_up_limit,
    IN UINT_16 u2FailCount_down_limit,
    IN UINT_8 ucARRCParam,
    IN UINT_8 ucARPERParam
    );

VOID
halARSetRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucWlanIdx,
    IN UINT_16 u2RateSet,
    IN BOOLEAN fgIsShortPreamble,
    IN UINT_8 ucRate1
    );

VOID
halARGetRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucWlanIdx,
    OUT PUINT_8 pucRate1Index
    );

VOID
halARReset (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucWlanIdx
    );


VOID
halEnableCTSProtectionMode (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucType
    );

VOID
halDisableCTSProtectionMode (
    IN P_ADAPTER_T prAdapter
    );

VOID
halSetBasicRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucRateIndex,
    IN BOOLEAN fgIsShortPreamble
    );

VOID
halSetCTSRTSRate (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucRateIndex,
    IN BOOLEAN fgIsShortPreamble
    );

VOID
halSetRTSThreshold (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2RTSThreshold
    );

VOID
halSetAckCtsRate(
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 aucAckCtsRateIndex[],
    IN BOOLEAN fgIsShortPreamble
    );

VOID
halSetOPMode (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_OP_MODE_T eOPMode
    );

VOID
halSetBSSID (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 aucBSSID
    );

VOID
halSetACParameters (
    IN P_ADAPTER_T prAdapter,
    IN TX_AC_PARAM_AIFS_CW_T arTxAcParamAifsCw[]
    );

VOID
halSetRxFilters (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4RxFilter
    );

BOOLEAN
halSetBeaconContent (
    IN P_ADAPTER_T  prAdapter,
    IN PUINT_8      pucBeaconContent,
    IN UINT_32      u4BcnLen,
    IN UINT_8       ucRateIndex,
    IN BOOLEAN      fgIsShortPreamble
    );

VOID
halpmResetTSFTimer (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_TCP_IP_CHKSUM_OFFLOAD
VOID
halSetCSUMOffload (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4CSUMFlags
    );
#endif

VOID
halRREnable (
    IN P_ADAPTER_T  prAdapter,
    IN RCPI         rRCPIUpperThreshold,
    IN RCPI         rRCPILowerThreshold
    );

VOID
halRRDisable (
    IN P_ADAPTER_T prAdapter
    );

VOID
halRRGetRCPI (
    IN P_ADAPTER_T  prAdapter,
    OUT P_RCPI      prRCPI
    );

VOID
halALCREnable (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_32      u4AvePara,
    IN ALC_VAL      rALCMaxThreshold,
    IN ALC_VAL      rALCMinThreshold,
    IN ALC_VAL      rALCInitVal
    );

VOID
halALCRDisable (
    IN P_ADAPTER_T prAdapter
    );

VOID
halALCRGetRawValue (
    IN P_ADAPTER_T  prAdapter,
    OUT P_ALC_VAL   prRawValue
    );

VOID
halALCRGetCalValue (
    IN P_ADAPTER_T  prAdapter,
    OUT P_ALC_VAL   prCalValue
    );

VOID
halALCRTriggerALC (
    IN P_ADAPTER_T  prAdapter
    );

VOID
halLogicReset (
    IN P_ADAPTER_T prAdapter
    );

VOID
halSetAdminCtrlMediumTime (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucTxQueueId,
    IN UINT_16 u2MediumTime
    );

VOID
halEnableAdminCtrl (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucTxQueueId
    );

VOID
halDisableAdminCtrl (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucTxQueueId
    );

VOID
halSetMACAddr (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucIdx,
    IN PUINT_8      pucMACAddr
    );

VOID
halClearMulticastAddrList (
    IN P_ADAPTER_T prAdapter
    );
/* EEPROM related*/

BOOLEAN
halEepromWrite16 (
    IN P_ADAPTER_T  	prAdapter,
    IN UINT_8      	  	ucEepromWordOffset,
    IN UINT_16          u2EepromData
    );

BOOLEAN
halEepromRead16 (
    IN      P_ADAPTER_T  	prAdapter,
    IN      UINT_8      	ucEepromWordOffset,
    OUT     PUINT_16        pu2EepromData
    );

UINT_16
halEepromGetSize (
    IN      P_ADAPTER_T  	prAdapter
    );

VOID
halEepromRecall (
    IN      P_ADAPTER_T     prAdapter
    );

VOID
halSetHwTxByBasicRate (
    IN  P_ADAPTER_T prAdapter,
    IN BOOLEAN fgUseBasicRate
    );

VOID
halGetRandomNumber (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_16 pu2Number
    );

#if CFG_SDIO_STATUS_ENHANCE
VOID
halSDIOInit (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_WORKAROUND_HEC_5269
VOID
halSDIOReadIntStatus (
    IN P_ADAPTER_T prAdapter,
    OUT UINT_32 au4IntStatus[],
    OUT PBOOLEAN pfgValidTxStatus
    );
#else
VOID
halSDIOReadIntStatus (
    IN P_ADAPTER_T prAdapter,
    OUT UINT_32 au4IntStatus[]
    );
#endif

BOOL
halSDIOTxProcessTxStatus (
    IN P_ADAPTER_T      prAdapter,
    IN P_TX_STATUS_T    prTxStatusDone,
    OUT PP_SW_TFCB_T    pprSwTfcbDone
    );

WLAN_STATUS
halSDIORxFillRFB (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2RxLengthDW,
    IN OUT P_SW_RFB_T prSWRfb
    );
#endif /* CFG_SDIO_STATUS_ENHANCE */


VOID
halpmSetOscStableTime (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_16      u2OscStableTimeUs
    );


VOID
halRxConfigRxStatusGroup (
    IN P_ADAPTER_T prAdapter
    );

VOID
halThermoUpdateTxGain (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucChannelNum,
    IN ENUM_BAND_T  eBand,
    IN UINT_8       ucThermo
    );

WLAN_STATUS
halSetTxPowerGain (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand,
    OUT PUINT_32   pu4McrCFPR,
    OUT PUINT_32   pu4MCROFPR
    );

VOID
halThermoUpdateRxSetting (
    IN P_ADAPTER_T  prAdapter,
    IN UINT_8       ucThermo
    );

VOID
halpmSetupBeaconContentCheck (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgEnable
    );

VOID
halpmSetupMoreDataTrigger (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    );

VOID
halpmSetupBeaconTimeoutDetection (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnable
    );


#endif /* _HAL_H */

