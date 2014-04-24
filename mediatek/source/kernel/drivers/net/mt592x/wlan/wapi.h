
#ifndef _WAPI_H
#define _WAPI_H





#if SUPPORT_WAPI
/* Structure of WAPI Information */
typedef struct _WAPI_INFO_T {
    UINT_8          ucElemId;
    UCHAR           ucLength;
    UINT_16         u2Version;
    UINT_32         u4AuthKeyMgtSuiteCount;
    UINT_32         au4AuthKeyMgtSuite[MAX_NUM_SUPPORTED_AKM_SUITES];
    UINT_32         u4PairwiseKeyCipherSuiteCount;
    UINT_32         au4PairwiseKeyCipherSuite[MAX_NUM_SUPPORTED_CIPHER_SUITES];
    UINT_32         u4GroupKeyCipherSuite;
    UINT_16         u2WapiCap;
    UINT_16         u2Bkid;
    UINT_8          aucBkid[16][16];
} WAPI_INFO_T, *P_WAPI_INFO_T;

/* WAPI Information element format */
typedef struct _WAPI_INFO_ELEM_T {
    UCHAR       ucElemId;
    UCHAR       ucLength;
    UINT_16     u2Version;
    UINT_16     u2AuthKeyMgtSuiteCount;
    UCHAR       aucAuthKeyMgtSuite1[4];
} WAPI_INFO_ELEM_T, *P_WAPI_INFO_ELEM_T;

#endif



#define WAPI_CIPHER_SUITE_WPI           0x01721400 /* WPI_SMS4 */
#define WAPI_AKM_SUITE_802_1X           0x01721400 /* WAI */
#define WAPI_AKM_SUITE_PSK              0x02721400 /* WAI_PSK */
		
#define ELEM_ID_WAPI                    68 /* WAPI IE */
	
#define WAPI_IE(fp)                     ((P_WAPI_INFO_ELEM_T) fp)


BOOLEAN
wapiPerformPolicySelection (
    IN  P_ADAPTER_T         prAdapter,
    IN  P_BSS_DESC_T        prBss
    );


#endif /* _RSN_H */

