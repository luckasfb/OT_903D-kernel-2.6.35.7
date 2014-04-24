




#ifndef _WLAN_OID_RFTEST_H
#define _WLAN_OID_RFTEST_H






/*--------------------------------------------------------------*/
/*! \brief Struct definition to indicate specific event.        */
/*--------------------------------------------------------------*/

typedef struct _PARAM_RFTEST_INFO_T {
    UINT_32                 u4Length;             /* Length of structure,
                                                     including myself */
    UINT_32                 u4NicInfoContentLen;  /* Include following content
                                                     field and myself */
    UINT_8                  aucNicInfoContent[1];
} PARAM_RFTEST_INFO_T, *P_PARAM_RFTEST_INFO_T;


typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    UINT_32                 u4FuncIndex;
    UINT_32                 u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;





#define RF_AT_PKT_CONTENT_GET_OFFSET_BYTE(_funcdata) \
            ( (UINT_8)(((_funcdata) & BITS(24, 31))>>24) )
#define RF_AT_PKT_CONTENT_GET_OFFSET_PLUS1_BYTE(_funcdata) \
                ( (UINT_8)(((_funcdata) & BITS(16, 23))>>16) )
#define RF_AT_PKT_CONTENT_GET_OFFSET(_funcdata) \
                ( (UINT_16)(((_funcdata) & BITS(0, 15))>>0) )

#define RF_AT_PKT_CONTENT_GET_OFFSET(_funcdata) \
                ( (UINT_16)(((_funcdata) & BITS(0, 15))>>0) )


/*--------------------------------------------------------------*/
/* Routines to set parameters or query information.             */
/*--------------------------------------------------------------*/
/***** Routines in wlan_oid_rftest.c *****/

WLAN_STATUS
wlanoidRftestSetTestMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidRftestSetAbortTestMode (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidRftestSetAutoTest (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidRftestSetAutoTest (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvSetBuffer,
    IN  UINT_32           u4SetBufferLen,
    OUT PUINT_32          pu4SetInfoLen
    );

WLAN_STATUS
wlanoidRftestQueryAutoTest (
    IN  P_ADAPTER_T       prAdapter,
    IN  PVOID             pvQueryBuf,
    IN  UINT_32           u4QueryBufLen,
    OUT PUINT_32          pu4QueryInfoLen
    );


#endif /* _WLAN_OID_RFTEST_H */

