






#include "precomp.h"








/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidRftestSetTestMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_RFTEST_INFO_T  prRfTestInfo;
#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS
    BOOL fgStatus = FALSE;
#endif

    DEBUGFUNC("wlanoidRftestSetTestMode");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = 0;

    if (u4SetBufferLen == 0 ||
        u4SetBufferLen == sizeof(prRfTestInfo->u4Length)) {
        /* If setBufferLen is zero, we need to load EEPROM */
        arbFsmRunEventRftestEnterTestMode(prAdapter, NULL, 0);

#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS
        // Calibrate 32K slow clock
        /* meta mode does not run into 32K validation and there is no return code. Therefore, we need to invoke calibration here. */
        fgStatus = nicpmCalSlowClock(prAdapter, &prAdapter->rPmInfo.rSlowClkCnt);

        if(fgStatus == FALSE){
            return WLAN_STATUS_FAILURE;
        }
#endif

#if !(CFG_SDIO_DEVICE_DRIVER_WO_NDIS)
        /* Currently no idea how to handle meta mode. However, in normal build, entering test mode
           means going to test wifi. set preferred antenna path to wifi.
        */
#if PTA_NEW_BOARD_DESIGN
        if(prAdapter->rPtaInfo.fgSingleAntenna){
            nicPtaSetAnt(prAdapter, TRUE);
        }
#endif
#endif

        return WLAN_STATUS_SUCCESS;
    }
    /* MT5921 does not support EEPROM content change by this oid cmd */

    prRfTestInfo = pvSetBuffer;

    DBGLOG(REQ, ERROR, ("Invalid data. nicInfoContentLen: %ld, Length:%ld\n",
        prRfTestInfo->u4NicInfoContentLen,
        prRfTestInfo->u4Length));

    return WLAN_STATUS_INVALID_DATA;
}   /* wlanoidRftestSetTestMode */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidRftestSetAbortTestMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    DEBUGFUNC("wlanoidRftestSetAbortTestMode");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    *pu4SetInfoLen = 0;

    arbFsmRunEventRftestAbortTestMode(prAdapter);

#if !(CFG_SDIO_DEVICE_DRIVER_WO_NDIS)
        /* Currently no idea how to handle meta mode. However, in normal build, entering test mode
           means going to test wifi. set preferred antenna path to wifi.
        */
#if PTA_NEW_BOARD_DESIGN
    if(prAdapter->rPtaInfo.fgSingleAntenna){
        nicPtaSetAnt(prAdapter, FALSE);
    }
#endif
#endif

    return WLAN_STATUS_SUCCESS;
}   /* rftestSetAbortTestMode */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidRftestSetAutoTest (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    )
{
    P_PARAM_MTK_WIFI_TEST_STRUC_T  prRfATInfo = \
                            (P_PARAM_MTK_WIFI_TEST_STRUC_T)pvSetBuffer;

    DEBUGFUNC("wlanoidRftestSetAutoTest");

    ASSERT(prAdapter);
    ASSERT(pvSetBuffer);
    ASSERT(pu4SetInfoLen);

    if (!IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        return WLAN_STATUS_NOT_ACCEPTED;
    }

    *pu4SetInfoLen = 0;

    if (u4SetBufferLen != sizeof(PARAM_MTK_WIFI_TEST_STRUC_T)) {
        DBGLOG(REQ, ERROR, ("Invalid data. SetBufferLen: %ld.\n",
                  u4SetBufferLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    return arbFsmRunEventRftestSetAutoTest(prAdapter, prRfATInfo);
}   /* wlanoidRftestSetAutoTest */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanoidRftestQueryAutoTest (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuf,
    IN  UINT_32           u4QueryBufLen,
    OUT PUINT_32          pu4QueryInfoLen
    )
{
    P_PARAM_MTK_WIFI_TEST_STRUC_T  prRfATInfo;

    DEBUGFUNC("wlanoidRftestQueryAutoTest");

    ASSERT(prAdapter);
    ASSERT(pvQueryBuf);
    ASSERT(pu4QueryInfoLen);

    *pu4QueryInfoLen = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    if (u4QueryBufLen != sizeof(PARAM_MTK_WIFI_TEST_STRUC_T)) {
        DBGLOG(REQ, ERROR, ("Invalid data. QueryBufferLen: %ld.\n",
                  u4QueryBufLen));
        return WLAN_STATUS_INVALID_LENGTH;
    }

    prRfATInfo = (P_PARAM_MTK_WIFI_TEST_STRUC_T)pvQueryBuf;

    return arbFsmRunEventRftestQueryAutoTest(prAdapter, prRfATInfo);
}   /* wlanoidRftestQueryAutoTest */

