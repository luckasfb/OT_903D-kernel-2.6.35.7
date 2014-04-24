






#include "precomp.h"

#if CFG_LP_IOT
extern UINT_32 u4AccumBcnTimeoutCnt;
#endif

extern const WMM_AC_PARAM_T arDefaultWmmParamFor11agSTA[];
extern const WMM_AC_PARAM_T arDefaultWmmParamFor11bSTA[];

extern PVOID pPreAllocedBSSCached;



static IST_EVENT_FUNCTION apfnEventFuncTable[] = {
    nicProcessAbnormalInterrupt,            /* INT_EVENT_ABNORMAL       */
    nicProcessWatchDogInterrupt,            /* INT_EVENT_WATCH_DOG      */
    nicProcessTsfDriftInterrupt,            /* INT_EVENT_TSF_DRIFT      */
    nicProcesScanDoneInterrupt,             /* INT_EVENT_SCAN_DONE      */
    nicProcessRxInterrupt,                  /* INT_EVENT_RX             */
    nicProcessTxInterrupt,                  /* INT_EVENT_TX             */
    nicProcesT0TimeupInterrupt,             /* INT_EVENT_T0_TIMEUP      */
    nicProcesT1TimeupInterrupt,             /* INT_EVENT_T1_TIMEUP      */
    nicProcesT2TimeupInterrupt,             /* INT_EVENT_T2_TIMEUP      */
    nicProcesT3TimeupInterrupt,             /* INT_EVENT_T3_TIMEUP      */
    nicProcessAdmitTimeMetInterrupt,        /* INT_EVENT_ADMIT_TIME_MET */
    nicProcessBeaconTimeoutInterrupt,       /* INT_EVENT_BEACON_TIMEOUT */
    nicProcessTxNullFailInterrupt,          /* INT_EVENT_TX_NULL_FAIL   */
    nicProcessTxTriggerFailInterrupt,       /* INT_EVENT_TX_TRIG_FAIL   */
    nicProcessTxPsPollFailInterrupt,        /* INT_EVENT_TX_PSPOLL_FAIL */
    nicProcessRxBmcTimeoutInterrupt,        /* INT_EVENT_BMC_TIMEOUT    */
    nicProcessApsdTimeoutInterrupt,         /* INT_EVENT_APSD_TIMEOUT   */
    nicProcessPsPollTimeoutInterrupt,       /* INT_EVENT_PSPOLL_TIMEOUT */
    nicProcessTbttInterrupt,                /* INT_EVENT_TBTT           */
    nicProcessBeaconTxRxOkInterrupt,        /* INT_EVENT_BCN_TR_OK      */
    nicProcessAtimWindowTimeoutInterrupt,   /* INT_EVENT_ATIM           */
    nicProcessBeaconTxOkInterrupt,          /* INT_EVENT_BCN_TX_OK      */
    nicProcessRcpiInterrupt,                /* INT_EVENT_RCPI           */
    nicProcessALCInterrupt,                 /* INT_EVENT_ALC            */
    NULL
};

#if CFG_ONLY_802_11A
/* MTK A60105 channel table for 5 GHz band, 20 Mhz reference clock */
CHANNEL_BAND_FREQ_MAP arChnlFreqMappingTable[] = {
    { CH_240,    BAND_5G,  4920000},
    { CH_244,    BAND_5G,  4940000},
    { CH_248,    BAND_5G,  4960000},
    { CH_252,    BAND_5G,  4980000},
    { CH_8,      BAND_5G,  5040000},
    { CH_12,     BAND_5G,  5060000},
    { CH_16,     BAND_5G,  5080000},
    { CH_34,     BAND_5G,  5170000},
    { CH_36,     BAND_5G,  5180000},
    { CH_38,     BAND_5G,  5190000},
    { CH_40,     BAND_5G,  5200000},
    { CH_42,     BAND_5G,  5210000},
    { CH_44,     BAND_5G,  5220000},
    { CH_46,     BAND_5G,  5230000},
    { CH_48,     BAND_5G,  5240000},
    { CH_52,     BAND_5G,  5260000},
    { CH_56,     BAND_5G,  5280000},
    { CH_58,     BAND_5G,  5300000},
    { CH_60,     BAND_5G,  5320000},
    { CH_100,    BAND_5G,  5500000},
    { CH_104,    BAND_5G,  5520000},
    { CH_108,    BAND_5G,  5540000},
    { CH_112,    BAND_5G,  5560000},
    { CH_116,    BAND_5G,  5580000},
    { CH_120,    BAND_5G,  5600000},
    { CH_124,    BAND_5G,  5620000},
    { CH_128,    BAND_5G,  5640000},
    { CH_132,    BAND_5G,  5660000},
    { CH_136,    BAND_5G,  5680000},
    { CH_140,    BAND_5G,  5700000},
    { CH_149,    BAND_5G,  5745000},
    { CH_153,    BAND_5G,  5765000},
    { CH_157,    BAND_5G,  5785000},
    { CH_161,    BAND_5G,  5805000}
};
#else
/* MTK A60105 channel table for 2.4 GHz band, 20 Mhz reference clock */
CHANNEL_BAND_FREQ_MAP arChnlFreqMappingTable[] = {
    { CH_1,     BAND_24G,   2412000 },
    { CH_2,     BAND_24G,   2417000 },
    { CH_3,     BAND_24G,   2422000 },
    { CH_4,     BAND_24G,   2427000 },
    { CH_5,     BAND_24G,   2432000 },
    { CH_6,     BAND_24G,   2437000 },
    { CH_7,     BAND_24G,   2442000 },
    { CH_8,     BAND_24G,   2447000 },
    { CH_9,     BAND_24G,   2452000 },
    { CH_10,    BAND_24G,   2457000 },
    { CH_11,    BAND_24G,   2462000 },
    { CH_12,    BAND_24G,   2467000 },
    { CH_13,    BAND_24G,   2472000 },
    { CH_14,    BAND_24G,   2484000 }
};
#endif

#if CFG_ONLY_802_11A

/*! \brief Default regulatory domain information, all channel are enabled. */
static DOMAIN_INFO_ENTRY rDefaultDomainInfo = {
    COUNTRY_CODE_NULL,
    {  36, 4, 16, 0 }
};

#else
/*! \brief Supported regulatory domain information table */
/*! These entries should be sorted by COUNTRY_CODE_x */
static DOMAIN_INFO_ENTRY arSupportedDomainInfoTable[] = {
    /* Austria */
    {
        COUNTRY_CODE_AT,
        { 1, 13, 20, 0 }
    },

    /* Australia */
    {
        COUNTRY_CODE_AU,
        { 1, 13, 20, 0 }
    },

    /* Belgium */
    {
        COUNTRY_CODE_BE,
        { 1, 13, 20, 0 }
    },

    /* Canada */
    {
        COUNTRY_CODE_CA,
        { 1, 11, 30, 0 }
    },

    /* Switzerland */
    {
        COUNTRY_CODE_CH,
        { 1, 13, 20, 0 }
    },

    /* Germany */
    {
        COUNTRY_CODE_DE,
        { 1, 13, 20, 0 }
    },

    /* Denmark */
    {
        COUNTRY_CODE_DK,
        { 1, 13, 20, 0 }
    },

    /* Spain */
    {
        COUNTRY_CODE_ES,
        { 10, 2, 20, 0 }
    },

    /* ETSI (Europe) */
    {
        COUNTRY_CODE_EU,
        { 1, 13, 20, 0 }
    },

    /* Finland */
    {
        COUNTRY_CODE_FI,
        { 1, 13, 20, 0 }
    },

    /* France */
    {
        COUNTRY_CODE_FR,
        { 10, 4,  20, 0 }
    },

    /* United Kingdom */
    {
        COUNTRY_CODE_GB,
        { 1,  13, 20, 0 }
    },

    /* Greece */
    {
        COUNTRY_CODE_GR,
        { 1, 13, 20, 0 }
    },

    /* Ireland */
    {
        COUNTRY_CODE_IE,
        { 1, 13, 20, 0 }
    },

    /* Israel */
    {
        COUNTRY_CODE_IL,
        { 5, 4,  20, 0 }
    },

    /* Iceland */
    {
        COUNTRY_CODE_IS,
        { 1, 13, 20, 0 }
    },

    /* Italy */
    {
        COUNTRY_CODE_IT,
        { 1,  13, 20, 0 }
    },

    /* Japan */
    {
        COUNTRY_CODE_JP,
        { 1, 14, 23, 0 }
    },

    /* Liechtenstein */
    {
        COUNTRY_CODE_LI,
        { 1, 13, 20, 0 }
    },

    /* Luxembourg */
    {
        COUNTRY_CODE_LU,
        { 1, 13, 20, 0 }
    },

    /* Netherlands */
    {
        COUNTRY_CODE_NL,
        { 1, 13, 20, 0 }
    },

    /* Norway */
    {
        COUNTRY_CODE_NO,
        { 1, 13, 20, 0 }
    },

    /* Portugal */
    {
        COUNTRY_CODE_PT,
        { 1, 13, 20, 0 }
    },

    /* Sweden */
    {
        COUNTRY_CODE_SE,
        { 1, 13, 20, 0 }
    },

    /* Taiwan */
    {
        COUNTRY_CODE_TW,
        { 1,  13, 30, 0 }
    },

    /* United States */
    {
        COUNTRY_CODE_US,
        { 1, 11, 30, 0 }
    }
};

/*! \brief Default regulatory domain information, all channel are enabled. */
static DOMAIN_INFO_ENTRY rDefaultDomainInfo = {
    COUNTRY_CODE_NULL,
    { 1, 14, 20, 0 }
};

#endif


#define LOCAL_NIC_ALLOCATE_MEMORY(pucMem, u4Size, pucComment) \
        { \
            DBGLOG(INIT, INFO, ("Allocating %ld bytes for %s.\n", (UINT_32)u4Size, pucComment)); \
            if ((pucMem = (PUINT_8)kalMemAlloc(u4Size)) == (PUINT_8)NULL) { \
                DBGLOG(INIT, ERROR, ("Could not allocate %ld bytes for %s.\n", (UINT_32)u4Size, pucComment)); \
                break; \
            } \
            ASSERT(((UINT_32)pucMem % 4) == 0); \
            DBGLOG(INIT, INFO, ("Virtual Address = 0x%08lx for %s.\n", (UINT_32)pucMem, pucComment)); \
        }


#define ALC_THRESHOLD_UPDATE_WITH_LIMIT(_oldTd, _DeltaTd, _newTd) \
    { \
        if(_DeltaTd < 0) {\
            if(_oldTd + _DeltaTd < 0) {\
                _newTd = 0;\
            }\
            else{\
                _newTd = _oldTd+ _DeltaTd;\
            }\
        }\
        else {\
            if(_oldTd + _DeltaTd > 0x3F) {\
                _newTd = 0x3F;\
            }\
            else{\
                _newTd = _oldTd+ _DeltaTd;\
            }\
        }\
    }



#if CFG_SDIO_DEVICE_DRIVER_WO_NDIS

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicAllocateAdapterMemory (
    IN P_ADAPTER_T prAdapter
    )
{
    WLAN_STATUS status = WLAN_STATUS_RESOURCES;
    P_TX_CTRL_T prTxCtrl;
    P_RX_CTRL_T prRxCtrl;
    P_MGT_BUF_INFO_T prMgtBufInfo;
    P_SCAN_INFO_T prScanInfo;
    P_STA_INFO_T prStaInfo;

    DEBUGFUNC("nicAllocateAdapterMemory");

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;
    prRxCtrl = &prAdapter->rRxCtrl;
    prMgtBufInfo = &prAdapter->rMgtBufInfo;
    prScanInfo = &prAdapter->rScanInfo;
    prStaInfo = &prAdapter->rStaInfo;

    do {
        //4 <0> Reset all Memory Handler
        prTxCtrl->pucTxCached = (PUINT_8)NULL;
        prRxCtrl->pucRxCached = (PUINT_8)NULL;

#if CFG_COALESCING_BUFFER_SIZE
        prAdapter->pucCoalescingBufCached = (PUINT_8)NULL;
#endif /* CFG_COALESCING_BUFFER_SIZE */

#if CFG_FRAGMENT_COALESCING_BUFFER_SIZE
        prAdapter->pucFragCoalescingBufCached = (PUINT_8)NULL;
#endif /* CFG_FRAGMENT_COALESCING_BUFFER_SIZE */

        prMgtBufInfo->pucMgtBufCached = (PUINT_8)NULL;
        prStaInfo->pucStaRecCached = (PUINT_8)NULL;
        prScanInfo->pucBSSCached = (PUINT_8)NULL;


        //4 <1> Memory for TX Descriptor
        /* Allocate memory for the transmit data structures. */
        prTxCtrl->u4TxCachedSize = (ALIGN_4(sizeof(MSDU_INFO_MEM_CTRL_T)) * \
                                    (CFG_MAX_NUM_MSDU_INFO_FOR_TC0 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TC1 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TC2 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TC3 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TS0 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TCM)) + \
                                   (ALIGN_4(sizeof(SW_TFCB_T)) * \
                                    (MAX_NUM_SW_TFCB_FOR_AC0 + \
                                     MAX_NUM_SW_TFCB_FOR_AC1 + \
                                     MAX_NUM_SW_TFCB_FOR_AC2 + \
                                     MAX_NUM_SW_TFCB_FOR_AC3 + \
                                     MAX_NUM_SW_TFCB_FOR_TS0 + \
                                     MAX_NUM_SW_TFCB_FOR_AC4));

        LOCAL_NIC_ALLOCATE_MEMORY(prTxCtrl->pucTxCached,
                                  prTxCtrl->u4TxCachedSize,
                                  "MSDU_INFO_MEM_CTRL_T & SW_TFCB_T");


        //4 <2> Memory for RX Descriptor
        /* Initialize the number of rx buffers we will have in our queue. */
        /* <TODO> We may setup ucRxPacketDescriptors by GLUE Layer, and using
         * this variable directly.
         */
        /* Allocate memory for the SW receive structures. */
        prRxCtrl->u4RxCachedSize = CFG_RX_MAX_PKT_NUM * \
                                   ALIGN_4(sizeof(SW_RFB_T));

        LOCAL_NIC_ALLOCATE_MEMORY(prRxCtrl->pucRxCached,
                                  prRxCtrl->u4RxCachedSize,
                                  "SW_RFB_T");


        //4 <3> Memory for Common Coalescing Buffer

        //4 <4> Memory for Management Frame Buffer

        //4 <5> Memory for STATION Records
        /* Allocate memory for storing the STA RECORD. */
        prStaInfo->u4StaRecCachedSize = CFG_MAX_NUM_STA_RECORD * \
                                        ALIGN_4(sizeof(STA_RECORD_T));

        LOCAL_NIC_ALLOCATE_MEMORY(prStaInfo->pucStaRecCached,
                                  prStaInfo->u4StaRecCachedSize,
                                  "STA Record List");

        //4 <6> Memory for BSS list

#if DBG
        {
            UINT_32 u4AllocatedSize;

            u4AllocatedSize =
                prTxCtrl->u4TxCachedSize +
                prRxCtrl->u4RxCachedSize;
            DBGLOG(INIT, TRACE, ("Total %ld bytes are allocated\n", u4AllocatedSize));
        }
#endif /* DBG */

        status = WLAN_STATUS_SUCCESS;

    }
    while (FALSE);


    return status;

}   /* end of nicAllocateAdapterMemory() */

#else

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicAllocateAdapterMemory (
    IN P_ADAPTER_T prAdapter
    )
{
    WLAN_STATUS status = WLAN_STATUS_RESOURCES;
    P_TX_CTRL_T prTxCtrl;
    P_RX_CTRL_T prRxCtrl;
    P_MGT_BUF_INFO_T prMgtBufInfo;
    P_SCAN_INFO_T prScanInfo;
    P_STA_INFO_T prStaInfo;

    DEBUGFUNC("nicAllocateAdapterMemory");

    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;
    prRxCtrl = &prAdapter->rRxCtrl;
    prMgtBufInfo = &prAdapter->rMgtBufInfo;
    prScanInfo = &prAdapter->rScanInfo;
    prStaInfo = &prAdapter->rStaInfo;

    do {
        //4 <0> Reset all Memory Handler
        prTxCtrl->pucTxCached = (PUINT_8)NULL;
        prRxCtrl->pucRxCached = (PUINT_8)NULL;

#if CFG_COALESCING_BUFFER_SIZE
        prAdapter->pucCoalescingBufCached = (PUINT_8)NULL;
#endif /* CFG_COALESCING_BUFFER_SIZE */

#if CFG_FRAGMENT_COALESCING_BUFFER_SIZE
        prAdapter->pucFragCoalescingBufCached = (PUINT_8)NULL;
#endif /* CFG_FRAGMENT_COALESCING_BUFFER_SIZE */

        prMgtBufInfo->pucMgtBufCached = (PUINT_8)NULL;
        prStaInfo->pucStaRecCached = (PUINT_8)NULL;
        prScanInfo->pucBSSCached = (PUINT_8)NULL;


        //4 <1> Memory for TX Descriptor
        /* Allocate memory for the transmit data structures. */
        prTxCtrl->u4TxCachedSize = (ALIGN_4(sizeof(MSDU_INFO_MEM_CTRL_T)) * \
                                    (CFG_MAX_NUM_MSDU_INFO_FOR_TC0 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TC1 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TC2 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TC3 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TS0 + \
                                     CFG_MAX_NUM_MSDU_INFO_FOR_TCM)) + \
                                   (ALIGN_4(sizeof(SW_TFCB_T)) * \
                                    (MAX_NUM_SW_TFCB_FOR_AC0 + \
                                     MAX_NUM_SW_TFCB_FOR_AC1 + \
                                     MAX_NUM_SW_TFCB_FOR_AC2 + \
                                     MAX_NUM_SW_TFCB_FOR_AC3 + \
                                     MAX_NUM_SW_TFCB_FOR_TS0 + \
                                     MAX_NUM_SW_TFCB_FOR_AC4));

        LOCAL_NIC_ALLOCATE_MEMORY(prTxCtrl->pucTxCached,
                                  prTxCtrl->u4TxCachedSize,
                                  "MSDU_INFO_MEM_CTRL_T & SW_TFCB_T");


        //4 <2> Memory for RX Descriptor
        /* Initialize the number of rx buffers we will have in our queue. */
        /* <TODO> We may setup ucRxPacketDescriptors by GLUE Layer, and using
         * this variable directly.
         */
        /* Allocate memory for the SW receive structures. */
        prRxCtrl->u4RxCachedSize = CFG_RX_MAX_PKT_NUM * \
                                   ALIGN_4(sizeof(SW_RFB_T));

        LOCAL_NIC_ALLOCATE_MEMORY(prRxCtrl->pucRxCached,
                                  prRxCtrl->u4RxCachedSize,
                                  "SW_RFB_T");


        //4 <3> Memory for Common Coalescing Buffer
#if CFG_COALESCING_BUFFER_SIZE
        /* Allocate memory for the common coalescing buffer. */
        prAdapter->u4CoalescingBufCachedSize = CFG_COALESCING_BUFFER_SIZE;

        LOCAL_NIC_ALLOCATE_MEMORY(prAdapter->pucCoalescingBufCached,
                                  prAdapter->u4CoalescingBufCachedSize,
                                  "COMMON COALESCING BUFFER");
#endif /* CFG_COALESCING_BUFFER_SIZE */

#if CFG_FRAGMENT_COALESCING_BUFFER_SIZE
        prAdapter->u4FragCoalescingBufCachedSize = CFG_FRAGMENT_COALESCING_BUFFER_SIZE;

        LOCAL_NIC_ALLOCATE_MEMORY(prAdapter->pucFragCoalescingBufCached,
                                  prAdapter->u4FragCoalescingBufCachedSize,
                                  "FRAGMENT COALESCING BUFFER");
#endif /* CFG_FRAGMENT_COALESCING_BUFFER_SIZE */


        //4 <4> Memory for Management Frame Buffer
        /* Allocate memory for the common frame buffer of Management Protocol Stack. */
        prMgtBufInfo->u4MgtBufCachedSize = MGT_PACKET_DESCRIPTORS_SIZE + \
                                           MGT_BUFFER_SIZE;

        LOCAL_NIC_ALLOCATE_MEMORY(prMgtBufInfo->pucMgtBufCached,
                                  prMgtBufInfo->u4MgtBufCachedSize,
                                  "COMMON MGMT FRAME BUFFER");


        //4 <5> Memory for STATION Records
        /* Allocate memory for storing the STA RECORD. */
        prStaInfo->u4StaRecCachedSize = CFG_MAX_NUM_STA_RECORD * \
                                        ALIGN_4(sizeof(STA_RECORD_T));

        LOCAL_NIC_ALLOCATE_MEMORY(prStaInfo->pucStaRecCached,
                                  prStaInfo->u4StaRecCachedSize,
                                  "STA Record List");


        //4 <6> Memory for BSS list
        prScanInfo->u4BSSCachedSize = CFG_MAX_NUM_BSS_LIST * \
                                      ALIGN_4(sizeof(BSS_DESC_T));
                                          
        /* Renbang 20100702 use pre-allocated memory,
        * in case of that there is no availiable 32k physical memory when initialization
        */
        if(pPreAllocedBSSCached){
            prScanInfo->pucBSSCached = pPreAllocedBSSCached;
        }else {
            LOCAL_NIC_ALLOCATE_MEMORY(prScanInfo->pucBSSCached,
                                      prScanInfo->u4BSSCachedSize,
                                      "BSS List");
        }
#if DBG
        {
            UINT_32 u4AllocatedSize;

            u4AllocatedSize =
                prTxCtrl->u4TxCachedSize +
                prRxCtrl->u4RxCachedSize +
    #if CFG_COALESCING_BUFFER_SIZE
                prAdapter->u4CoalescingBufCachedSize +
    #endif /* CFG_COALESCING_BUFFER_SIZE */

    #if CFG_FRAGMENT_COALESCING_BUFFER_SIZE
                prAdapter->u4FragCoalescingBufCachedSize +
    #endif /* CFG_FRAGMENT_COALESCING_BUFFER_SIZE */
                prMgtBufInfo->u4MgtBufCachedSize +
                prStaInfo->u4StaRecCachedSize +
                prScanInfo->u4BSSCachedSize;
            DBGLOG(INIT, TRACE, ("Total %ld bytes are allocated\n", u4AllocatedSize));

        }
#endif /* DBG */

        status = WLAN_STATUS_SUCCESS;

    }
    while (FALSE);


    return status;

}   /* end of nicAllocateAdapterMemory() */

#endif /* CFG_SDIO_DEVICE_DRIVER_WO_NDIS */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicReleaseAdapterMemory (
    IN P_ADAPTER_T prAdapter
    )
{
    P_TX_CTRL_T prTxCtrl;
    P_RX_CTRL_T prRxCtrl;
    P_MGT_BUF_INFO_T prMgtBufInfo;
    P_SCAN_INFO_T prScanInfo;
    P_STA_INFO_T prStaInfo;


    ASSERT(prAdapter);
    prTxCtrl = &prAdapter->rTxCtrl;
    prRxCtrl = &prAdapter->rRxCtrl;
    prMgtBufInfo = &prAdapter->rMgtBufInfo;
    prScanInfo = &prAdapter->rScanInfo;
    prStaInfo = &prAdapter->rStaInfo;

    //4 <6> Memory for BSS list
    if (prScanInfo->pucBSSCached) {
        if(!pPreAllocedBSSCached){
            kalMemFree((PVOID)prScanInfo->pucBSSCached,
                       prScanInfo->u4BSSCachedSize);
        }    	
        prScanInfo->pucBSSCached = (PUINT_8)NULL;
        prScanInfo->u4BSSCachedSize = 0;        
    }

    //4 <5> Memory for STATION Records
    if (prStaInfo->pucStaRecCached) {
        kalMemFree((PVOID)prStaInfo->pucStaRecCached,
                   prStaInfo->u4StaRecCachedSize);
        prStaInfo->pucStaRecCached = (PUINT_8)NULL;
    }


    //4 <4> Memory for Management Frame Buffer
    if (prMgtBufInfo->pucMgtBufCached) {
        kalMemFree((PVOID)prMgtBufInfo->pucMgtBufCached,
                   prMgtBufInfo->u4MgtBufCachedSize);
        prMgtBufInfo->pucMgtBufCached = (PUINT_8)NULL;
    }

    //4 <3> Memory for Common Coalescing Buffer
#if CFG_COALESCING_BUFFER_SIZE
    if (prAdapter->pucCoalescingBufCached) {
        kalMemFree((PVOID)prAdapter->pucCoalescingBufCached,
                   prAdapter->u4CoalescingBufCachedSize);
        prAdapter->pucCoalescingBufCached = (PUINT_8)NULL;
    }
#endif /* CFG_COALESCING_BUFFER_SIZE */

#if CFG_FRAGMENT_COALESCING_BUFFER_SIZE
    if (prAdapter->pucFragCoalescingBufCached) {
        kalMemFree((PVOID)prAdapter->pucFragCoalescingBufCached,
                   prAdapter->u4FragCoalescingBufCachedSize);
        prAdapter->pucFragCoalescingBufCached = (PUINT_8)NULL;
    }
#endif /* CFG_FRAGMENT_COALESCING_BUFFER_SIZE */

    //4 <2> Memory for RX Descriptor
    if (prRxCtrl->pucRxCached) {
        kalMemFree((PVOID)prRxCtrl->pucRxCached,
                   prRxCtrl->u4RxCachedSize);
        prRxCtrl->pucRxCached = (PUINT_8)NULL;
    }


    //4 <1> Memory for TX Descriptor
    if (prTxCtrl->pucTxCached) {
        kalMemFree((PVOID)prTxCtrl->pucTxCached,
                   prTxCtrl->u4TxCachedSize);
        prTxCtrl->pucTxCached = (PUINT_8)NULL;
    }

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicDisableInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    HAL_INTR_DISABLE(prAdapter);

    prAdapter->fgIsIntEnable = FALSE;
}



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicEnableInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    BOOLEAN fgIsIntEnableCache;


    ASSERT(prAdapter);
    fgIsIntEnableCache = prAdapter->fgIsIntEnable;

    prAdapter->fgIsIntEnable = TRUE; // NOTE(Kevin): It must be placed before MCR GINT write.

    /* If need enable INT and also set LPOwn at the same time. */
    if (prAdapter->fgIsIntEnableWithLPOwnSet) {
        prAdapter->fgIsIntEnableWithLPOwnSet = FALSE; /* NOTE(Kevin): It's better to place it
                                                       * before MCR GINT write.
                                                       */
        /* If INT was enabled, only set LPOwn */
        if (fgIsIntEnableCache) {
            HAL_LP_OWN_SET(prAdapter);
        }
        /* If INT was not enabled, enable it and also set LPOwn now */
        else {
            HAL_INTR_ENABLE_AND_LP_OWN_SET(prAdapter);
        }
    }
    /* If INT was not enabled, enable it now */
    else if (!fgIsIntEnableCache) {
        HAL_INTR_ENABLE(prAdapter);
    }

    return;
} /* end of nicEnableInterrupt() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicProcessIST (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_8 ucIntCount = 0;
    P_INT_EVENT_MAP_T prIntEventMap = NULL;
    UINT_32 au4IntStatus[INT_NUM];
    UINT_32 i;
#if CFG_SDIO_STATUS_ENHANCE && CFG_WORKAROUND_HEC_5269
    BOOLEAN fgValidTxStatus = FALSE;
#endif

    DEBUGFUNC("nicProcessIST");

    ASSERT(prAdapter);

    if (prAdapter->rAcpiState == ACPI_STATE_D3) {
        DBGLOG(REQ, WARN, ("Fail in set nicProcessIST! (Adapter not ready). ACPI=D%d, Radio=%d\n",
                    prAdapter->rAcpiState, prAdapter->fgIsRadioOff));
        return WLAN_STATUS_ADAPTER_NOT_READY;
    }

    for (i = 0; (i < CFG_IST_LOOP_COUNT || NIC_IS_NEED_TO_REDO_PROCESS_IST(prAdapter)); i++) {

#if !CFG_SDIO_STATUS_ENHANCE
        halReadIntStatus(prAdapter, au4IntStatus);
#else
    #if CFG_WORKAROUND_HEC_5269
        halSDIOReadIntStatus(prAdapter, au4IntStatus, &fgValidTxStatus);
    #else
        halSDIOReadIntStatus(prAdapter, au4IntStatus);
    #endif

#endif /* CFG_SDIO_STATUS_ENHANCE */

        /* Sync SW pre-fetched interrupt status with HW reported interrupt status */
        for (ucIntCount = 0; ucIntCount < INT_NUM; ucIntCount++) {
            prAdapter->au4IntStatus[ucIntCount] |= au4IntStatus[ucIntCount];
        }

        for (ucIntCount = 0; ucIntCount < INT_NUM; ucIntCount++) {
            if (prAdapter->au4IntStatus[ucIntCount] &
                (~(prAdapter->au4IntEnable[ucIntCount])) ) {
                DBGLOG(INTR , WARN, ("au4IntStatus[%d] = %#lx, au4IntEnable[%d] = %#lx\n",
                    ucIntCount, prAdapter->au4IntStatus[ucIntCount],
                    ucIntCount, prAdapter->au4IntEnable[ucIntCount]));
                //4 Filter the bit which is not set.
                prAdapter->au4IntStatus[ucIntCount] &= prAdapter->au4IntEnable[ucIntCount];
                //ASSERT_REPORT(0, "Invalid INT bit is asserted\n");
            }
        }

        /* interrupt false alarm detection */
        if ((prAdapter->au4IntStatus[INT_HISR] == 0) &&
            (prAdapter->au4IntStatus[INT_HSCISR] == 0)
#if CFG_SDIO_STATUS_ENHANCE && CFG_WORKAROUND_HEC_5269
            && !fgValidTxStatus
#endif
            ) {
#if DBG
            if ((i == 0) && !prAdapter->fgIntrMayFalseAlarmFlag) {
                ucIntCount = INT_HISR;
                DBGLOG(INTR , WARN,
                    ("INTR false alarm! au4IntStatus[%d] = %#lx, au4IntEnable[%d] = %#lx\n",
                    ucIntCount, prAdapter->au4IntStatus[ucIntCount],
                    ucIntCount, prAdapter->au4IntEnable[ucIntCount]));
            }
            /* Clear the FLAG To check interrupt false alarm */
            prAdapter->fgIntrMayFalseAlarmFlag = FALSE;
#endif /* DBG */
            break;
        }

#if DBG
        /* Clear the FLAG To check interrupt false alarm */
        prAdapter->fgIntrMayFalseAlarmFlag = FALSE;
#endif
        /* Clear the FLAG To Redo IST */
        NIC_UNSET_REDO_PROCESS_IST_FLAG(prAdapter);

        /* Process each of the interrupt status consequently */
        prIntEventMap = &arIntEventMapTable[0];
        for (ucIntCount = 0; ucIntCount < ucIntEventMapSize; prIntEventMap++, ucIntCount++) {
            if (prIntEventMap->u4Int & prAdapter->au4IntStatus[prIntEventMap->ucRegNum]) {

                if (apfnEventFuncTable[prIntEventMap->u4Event] != NULL) {
                    apfnEventFuncTable[prIntEventMap->u4Event](prAdapter);
                }
                else {
                    DBGLOG(INTR , WARN,
                        ("Empty INTR handler! ISAR bit#: %ld, Reg#: %#x\n",
                        prIntEventMap->u4Int,
                        prIntEventMap->ucRegNum));

                    ASSERT(0); // to trap any NULL interrupt handler
                }
                prAdapter->au4IntStatus[prIntEventMap->ucRegNum] &= ~prIntEventMap->u4Int;
            }
        }

#if CFG_SDIO_STATUS_ENHANCE && CFG_WORKAROUND_HEC_5269
        if (fgValidTxStatus) {
            /* Need to process the TX done interrupt status due to
               TX status is already been read by the single command
               combined with reading interrupt status under
               SDIO enhance mode */
            nicProcessTxInterrupt(prAdapter);
            fgValidTxStatus = FALSE;
        }
#endif

    }

#if !CFG_STATISTICS_TIMER_EN
    statisticsIntEevnt(prAdapter);
#endif /* CFG_STATISTICS_TIMER_EN */

    return WLAN_STATUS_SUCCESS;
} /* end of nicProcessIST() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicReadConfig (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgEepromRecall
    )
{
    P_EEPROM_CTRL_T prEEPROMCtrl;

    DEBUGFUNC("nicReadConfig");

    ASSERT(prAdapter);
    prEEPROMCtrl = &prAdapter->rEEPROMCtrl;

    if (!halVerifyChipID(prAdapter)) {
        DBGLOG(INIT, ERROR, ("halVerifyChipID Error!\n"));
        return WLAN_STATUS_FAILURE;
    }

    //4 <0> Turn ON clock before HW access
    nicpmPowerOn(prAdapter);

    if (!halVerifyRevID(prAdapter)) {
        DBGLOG(INIT, ERROR, ("halVerifyRevID Error!\n"));
        return WLAN_STATUS_FAILURE;
    }

#if CFG_SUPPORT_EXT_CONFIG
    if (WLAN_STATUS_SUCCESS != nicExtReadCfg(prAdapter, prEEPROMCtrl)) {
        //DBGLOG(HAL, ERROR, ("Read external configuration fail.\n"));
        printk(KERN_WARNING "[MT5921][nicReadConfig] failed to read external cfg!!\n");
        printk(KERN_WARNING "[MT5921][nicReadConfig] loading defaule cfg...\n");        
			  /* Renbang : Alway let driver enter normal mode, even when read invalid config data from NVRAM */        
        prEEPROMCtrl->fgIsEepromValid = TRUE;
        //4 2007/07/02, mikewu, if there is no EEPROM present or cannot read EEPROM due to
        //4 some error, just load the default cofiguration of the EEPROM
        nicGetDefaultEEPROMCfg(prAdapter, prEEPROMCtrl);
    }
    else {
        prEEPROMCtrl->fgIsEepromValid = TRUE;
        //DBGLOG(INIT, TRACE, ("nicExtReadCfg success\n"));
        printk("[MT5921][nicReadConfig] success to read external cfg!!\n");        
    }
#else
    /* !CFG_SUPPORT_EXT_CONFIG && !CFG_SDIO_DEVICE_DRIVER_WO_NDIS: use EEPROM */

    /* Let chip reload entire EEPROM, including its size and checksum */
    if (fgEepromRecall) {
        halEepromRecall(prAdapter);
    }

    if (WLAN_STATUS_SUCCESS != nicEepromReadCfg(prAdapter, prEEPROMCtrl)) {
        DBGLOG(HAL, ERROR, ("Read EEPROM Configuration fail.\n"));
        prEEPROMCtrl->fgIsEepromValid = FALSE;
        //4 2007/07/02, mikewu, if there is no EEPROM present or cannot read EEPROM due to
        //4 some error, just load the default cofiguration of the EEPROM
        nicGetDefaultEEPROMCfg(prAdapter, prEEPROMCtrl);
    }
    else {
        prEEPROMCtrl->fgIsEepromValid = TRUE;
        DBGLOG(INIT, TRACE, ("nicEepromReadCfg success\n"));
    }
#endif /* CFG_SUPPORT_EXT_CONFIG */

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicInitializeChip (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;

    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;

    /* Renbang (20100120): put all GPIO initialization here */
    //Set GPIO1 based on EEPROM.
    nicSetGPIO1Mode(prAdapter);
    
    //Set GPIO2 according to Connection Setting.
    nicSetGPIO2Mode(prAdapter, prConnSettings->eGPIO2_Mode);

#if PTA_ENABLED
#if !CFG_SDIO_DEVICE_DRIVER_WO_NDIS
    /* Workaround for single antenna because initial time is long
     * The antenna setting is restored back to HW control in
     * function nicPTASetConfig()
     *
     * Here we note the following initial functions shall keep RICR and IOPCR
     * to let antenna switched to BT. There is no good mechanism to make sure
     * it now. 2009/3/30.
     */
    if (prAdapter->rPtaInfo.fgSingleAntenna) {
        if( prAdapter->rArbInfo.fgTestMode == true ){
        /* Renbang (20090815) : Set Single ANT owner */
            nicPtaSetAnt( prAdapter, true );        	
        }
        else{    	
#if PTA_NEW_BOARD_DESIGN
            /* Set prefer ant to BT */
            //nicPtaSetAnt(prAdapter, FALSE);
            /* Renbang (20100416) : for Android open, alway set prefer ant to wifi */
	       nicPtaSetAnt( prAdapter, true );	   
#else       
            UINT_32     u4RegValue;
            /* GeorgeKuo(20090805): new single-antenna design */
            /* Set prefer ant to BT */
            nicPtaSetAnt(prAdapter, FALSE);
            /* IOPCR (Ouput mode) */
            HAL_MCR_RD(prAdapter, MCR_IOPCR, &u4RegValue);
            HAL_MCR_WR(prAdapter, MCR_IOPCR,
                u4RegValue | (IOPCR_IO_ANT_SEL_P_DIR | IOPCR_IO_ANT_SEL_N_DIR));
#endif
        }
    }
#endif
#endif /* end of PTA_ENABLED */

    //4 <1> MCR init
    halMCRInit(prAdapter);

#if CFG_SDIO_STATUS_ENHANCE
    halSDIOInit(prAdapter);
#endif /* CFG_SDIO_STATUS_ENHANCE */

    //4 <2> BB init
    halBBInit(prAdapter);

    //4 <3> RF Init
    halRFInit(prAdapter, &prAdapter->rEEPROMCtrl);

    //4 <4> Queue Memory init
    halHWQueueInit(prAdapter);

    //4 <5> Set Mac address
    nicSetUnicastAddr(prAdapter);

    //4 <6> Enable Interrupt
#if CFG_INT_WRITE_CLEAR
        //HAL_MCR_WR(prAdapter, MCR_HSCISR, pu4IntStatus[1]);
#endif

    HAL_MCR_IER_WR(prAdapter, prAdapter->au4IntEnable[INT_HISR]);
    HAL_MCR_SCIER_WR(prAdapter, prAdapter->au4IntEnable[INT_HSCISR]);

    //4 <7> Setup AC parameter
    //4 <8> Setup Tx CTRL resp rate
    nicARInit(prAdapter);
    nicAREnable(prAdapter, TRUE);
    //nicARSetParam(prAdapter, 0x3, 0x2);
    //nicARSetRate(prAdapter, 0xffff, TRUE, 0);


    //4 <9> Setup Slot Time /* TODO(Kevin) : Should alter to eScanPhyType later */
    nicSetSlotTime(prAdapter,
                   prAdapter->eCurrentPhyType,
                   prConnSettings->fgIsShortSlotTimeOptionEnable);

    //4 <10> Setup RTS threshold
    nicRateSetRTSThreshold(prAdapter,
                           prConnSettings->u2RTSThreshold);


    //4 <11> Setup AIFS, CWmin & CWmax, TXOP
    if (prAdapter->fgIsEnableWMM) {
        nicTxQoSUpdateTXQParameters(prAdapter,
            (P_WMM_AC_PARAM_T)(0 ? //(prAdapter->eCurrentPhyType == PHY_TYPE_HR_DSSS_INDEX) ?
                &arDefaultWmmParamFor11bSTA[0] : &arDefaultWmmParamFor11agSTA[0]));
    }
    else {
        nicTxNonQoSUpdateTXQParameters(prAdapter,
                                       prAdapter->eCurrentPhyType);
    }

#if CFG_ONLY_802_11A
    nicRateSetBasicRate(prAdapter,
                        RATE_6M_INDEX,
                        FALSE);
#endif

#if CFG_PEEK_RCPI_VALUE_PERIOD_SEC
    ARB_INIT_TIMER(prAdapter,
                   prAdapter->rRcpiDiagnostic,
                   arbFsmRunEventRcpiDiagnostic,
                   TRUE);
#endif /* CFG_PEEK_RCPI_VALUE_PERIOD_SEC */

#if defined(LINUX) & CFG_TCP_IP_CHKSUM_OFFLOAD
#if SUPPORT_WAPI
    if (!prAdapter->fgUseWapi) {
    nicSetCSUMOffload(prAdapter, CSUM_OFFLOAD_EN_ALL);
    }
#else
    nicSetCSUMOffload(prAdapter, CSUM_OFFLOAD_EN_ALL);
#endif
#endif /* LINUX & CFG_TCP_IP_CHKSUM_OFFLOAD */

    /* Configure RX status for optional group */
    halRxConfigRxStatusGroup(prAdapter);

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicGetDefaultEEPROMCfg (
    IN P_ADAPTER_T prAdapter,
    OUT P_EEPROM_CTRL_T prEEPROMCtrl
    )
{
    ASSERT(prAdapter);
    ASSERT(prEEPROMCtrl);

    halGetDefaultEEPROMCfg(prEEPROMCtrl);
    prEEPROMCtrl->aucMacAddress[0] = 0x00;
    prEEPROMCtrl->aucMacAddress[1] = 0x0C;
    prEEPROMCtrl->aucMacAddress[2] = 0xE7;
    prEEPROMCtrl->aucMacAddress[3] = 0x09;
    prEEPROMCtrl->aucMacAddress[4] = 0x00;
    prEEPROMCtrl->aucMacAddress[5] = 0x00;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicSetHwBySta (
    IN P_ADAPTER_T      prAdapter,
    IN P_STA_RECORD_T   prSta
    )
{
    UINT_8              ucWlanIdx;

    ASSERT(prAdapter);
    if (prSta == NULL)
        return FALSE;

    //get the QoS value and desired rate, notice, since use auto rate,
    //the rate set for nicPrivacyUpdateBySta is not meaningful
    nicPrivacyUpdateBySta(prAdapter,
                          (PUINT_8)&prSta->aucMacAddr[0],
                          prSta,
                          (PBOOLEAN)&prSta->fgIsQoS,
                          NULL,
                          NULL,
                     #if 1
                          NULL,
                          NULL,
                          NULL,
                          NULL);
                     #else /* Disable auto rate */
                          &ucRate[0],
                          &ucRate[1],
                          &ucRate[2],
                          &ucRate[3]);
                     #endif

    if (!nicPrivacyGetWlanIndexByAddr(prAdapter, &prSta->aucMacAddr[0], &ucWlanIdx)) {
        ASSERT(0);
    }

    //hw auto rate
    nicARSetRate(prAdapter,
                 prSta->u2DesiredRateSet,
                 prSta->fgIsShortPreambleOptionEnable,
                 prSta->ucCurrRate1Index,
                 ucWlanIdx,
                 TRUE);

    return TRUE;
} /* nicSettingQoS */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicSetQoSBySta (
    IN  P_ADAPTER_T prAdapter,
    IN  PUINT_8     pucBSSID
    )
{
    P_STA_RECORD_T         prSta;

    ASSERT(prAdapter);
    prSta = staRecGetStaRecordByAddr(prAdapter, pucBSSID);

    nicPrivacyUpdateBySta(prAdapter,
                          pucBSSID,
                          prSta,
                          (PBOOLEAN)&prSta->fgIsQoS,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL);

    return TRUE;
} /* nicSettingQoS */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicSetAntennaBySta (
    IN  P_ADAPTER_T prAdapter,
    IN  PUINT_8     pucBSSID,
    IN  BOOLEAN     fgAntenna
    )
{
    P_STA_RECORD_T         prSta;

    ASSERT(prAdapter);
    prSta = staRecGetStaRecordByAddr(prAdapter, pucBSSID);

    nicPrivacyUpdateBySta(prAdapter,
                          pucBSSID,
                          prSta,
                          NULL,
                          (PBOOLEAN)&fgAntenna,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL);

    return TRUE;
} /* nicSettingAntenna */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicSetRateBySta (
    IN  P_ADAPTER_T        prAdapter,
    IN  PUINT_8            pucBSSID,
    IN  UINT_8             ucRate1,
    IN  UINT_8             ucRate2,
    IN  UINT_8             ucRate3,
    IN  UINT_8             ucRate4
    )
{
    P_STA_RECORD_T         prSta;

    ASSERT(prAdapter);
    prSta = staRecGetStaRecordByAddr(prAdapter, pucBSSID);

    /* <Todo> Do the rate check */
    /* <Todo> Since the auto rate, the function may remove */
    nicPrivacyUpdateBySta(prAdapter,
                          pucBSSID,
                          prSta,
                          NULL,
                          NULL,
                          NULL,
                          &ucRate1,
                          &ucRate2,
                          &ucRate3,
                          &ucRate4);

    return TRUE;
} /* nicSettingRate */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicChipReset (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 i;

    ASSERT(prAdapter);
    halChipReset(prAdapter);

    prAdapter->fgIsIntEnable = FALSE;

    for (i = 0; i < INT_NUM; i ++) {
        prAdapter->au4IntStatus[i]= 0;
    }

#if CFG_SDIO_STATUS_ENHANCE
    {
        P_SDIO_CTRL_T prSDIOCtrl = &prAdapter->rSDIOCtrl;

        kalMemZero(prSDIOCtrl->au2RxLengthDW, sizeof(prSDIOCtrl->au2RxLengthDW));
        kalMemZero(prSDIOCtrl->arTxStatus, sizeof(prSDIOCtrl->arTxStatus));
    }
#endif /* CFG_SDIO_STATUS_ENHANCE */

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicInitializeConnectionSettings (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;
    UINT_8 aucAnyBSSID[] = BC_BSSID;
    UINT_8 aucZeroMacAddr[MAC_ADDR_LEN] = {0,0,0,0,0,0};


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;

    COPY_MAC_ADDR(prConnSettings->aucBSSID, aucAnyBSSID);
    prConnSettings->fgIsConnByBssidIssued = FALSE;

    prConnSettings->fgIsConnReqIssued = FALSE;

    prConnSettings->fgIsEnableRoaming = TRUE;

    prConnSettings->u2BeaconPeriod = DOT11_BEACON_PERIOD_DEFAULT;

    prConnSettings->eDesiredPhyType = PHY_TYPE_802_11ABG;

    prConnSettings->eOPMode = NET_TYPE_INFRA;

    prConnSettings->eAuthMode = AUTH_MODE_OPEN;

    prConnSettings->eEncStatus = ENUM_ENCRYPTION_DISABLED;

    mibInitializeAttributes(prAdapter);

    prConnSettings->ePreambleType = PREAMBLE_TYPE_AUTO;

    prConnSettings->fgIsShortSlotTimeOptionEnable = TRUE;

    prConnSettings->fgIsAdHocQoSEnable = FALSE;

    prConnSettings->u2DesiredRateSet = RATE_SET_ALL;

    prConnSettings->u2RTSThreshold = DOT11_RTS_THRESHOLD_DEFAULT;

#if CFG_TX_FRAGMENT
    prConnSettings->u2FragmentationThreshold = DOT11_FRAGMENTATION_THRESHOLD_DEFAULT;

    if (prConnSettings->u2FragmentationThreshold) {
        prConnSettings->fgIsEnableTxFragment = TRUE;
    }
    else {
        prConnSettings->fgIsEnableTxFragment = FALSE;
    }

    prConnSettings->fgIsEnableTxAutoFragmentForBT = FALSE; // Check with PTA after reset.

    prConnSettings->eBTCoexistWindowType = BT_COEXIST_WINDOW_2500;
#endif /* CFG_TX_FRAGMENT */

    /* Set ATIM window */
    prConnSettings->u2AtimWindow = CFG_IBSS_ATIM_WINDOW;

    /* Set VoIP interval */
    prConnSettings->u4VoipTrafficInterval= CFG_INIT_VOIP_INTERVAL;

    /* Set PS mode */
    prConnSettings->rPwrMode= (PARAM_POWER_MODE)CFG_INIT_POWER_SAVE_PROF;

    /* Set ARP filter enable */
    prConnSettings->fgArpFilterEn = CFG_L3_PATTERN_MATCH_WAKEUP;

    /* Set MAC address */
    COPY_MAC_ADDR(prConnSettings->aucMacAddress, aucZeroMacAddr);

    /* Set Ad hoc mode */
    prConnSettings->eAdHocMode = CFG_INIT_ADHOC_MODE;

    /* Set U-APSD AC */
    prConnSettings->bmfgApsdEnAc = CFG_INIT_UAPSD_AC;

#if PTA_ENABLED
    /* Set PTA to disabled mode */
    kalMemZero((PVOID)&prConnSettings->rPtaParam, sizeof(PTA_PARAM_T));
#endif

    /* Set Country code */
    prConnSettings->u2CountryCode = COUNTRY_CODE_NULL;

    /* Set LED Blink mode */
    prConnSettings->ucLedBlinkMode = 0;

#if CFG_SUPPORT_802_11D
    /* Set multi-domain capability */
    prConnSettings->fgMultiDomainCapabilityEnabled = FALSE;
#endif

    prConnSettings->eGPIO2_Mode = GPIO2_MODE_EEPROM;

    prConnSettings->ucViAifsnBias = CFG_INIT_VI_AIFS_BIAS;

    prConnSettings->u2ViMaxTxopLimit = CFG_INIT_VI_MAX_TXOP_LIMIT;

    return;
} /* end of nicInitializeConnectionSettings() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetConnectionSettingsByGlueParam (
    IN P_ADAPTER_T prAdapter,
    IN P_REG_INFO_T prRegInfo
    )
{
extern WIFI_CFG_DATA gPlatformCfg;
    P_CONNECTION_SETTINGS_T prConnSettings;
    P_EEPROM_CTRL_T         prEEPROMCtrl;
#if PTA_ENABLED
    P_PTA_PARAM_T           prPtaParam;
    P_PTA_INFO_T            prPtaInfo;
#endif

    ASSERT(prAdapter);
    prEEPROMCtrl = &prAdapter->rEEPROMCtrl;
    prConnSettings = &prAdapter->rConnSettings;

    if (!prRegInfo) {
    prAdapter->rPtaInfo.fgSingleAntenna = gPlatformCfg.rWifiCustom.u4SingleAntenna;		
#if PTA_ENABLED
    prPtaParam = &prConnSettings->rPtaParam;
    prPtaInfo = &prAdapter->rPtaInfo;
    /* Set BT-Coexist setting, including PTA, BT type and frag control */
    prPtaParam->u4BtCR0 = 0x0;//prRegInfo->u4BtCR0;
    prPtaParam->u4BtCR1 = 0x0;//prRegInfo->u4BtCR1;
    prPtaParam->u4BtCR2 = 0x0;//prRegInfo->u4BtCR2;
    prPtaParam->u4BtCR3 = 0x0;//prRegInfo->u4BtCR3;
    kalMemCopy(&prPtaInfo->rPtaParam, prPtaParam, sizeof(PTA_PARAM_T));
#if PTA_NEW_BOARD_DESIGN
    prPtaParam = &prConnSettings->rSingleAclPtaParam;
    prPtaParam->u4BtCR0 = 0x82048041;//prRegInfo->u4SingleAclBtCR0;
    prPtaParam->u4BtCR1 = 0x19040f00;//prRegInfo->u4SingleAclBtCR1;
    prPtaParam->u4BtCR2 = 0x0f0001d0;//prRegInfo->u4SingleAclBtCR2;
    prPtaParam->u4BtCR3 = 0x0200000a;//prRegInfo->u4SingleAclBtCR3;
    kalMemCopy(&prPtaInfo->rSingleAclPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    prPtaParam = &prConnSettings->rSingleMixPtaParam;
    prPtaParam->u4BtCR0 = 0x82040061;//prRegInfo->u4SingleMixBtCR0;
    prPtaParam->u4BtCR1 = 0x19040e00;//prRegInfo->u4SingleMixBtCR1;
    prPtaParam->u4BtCR2 = 0x0;//prRegInfo->u4SingleMixBtCR2;
    prPtaParam->u4BtCR3 = 0x0;//prRegInfo->u4SingleMixBtCR3;
    kalMemCopy(&prPtaInfo->rSingleMixPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    prPtaParam = &prConnSettings->rDualAclPtaParam;
    prPtaParam->u4BtCR0 = 0x82000061;//prRegInfo->u4DualAclBtCR0;
    prPtaParam->u4BtCR1 = 0x09040f00;//prRegInfo->u4DualAclBtCR1;
    prPtaParam->u4BtCR2 = 0x0a0001d0;//prRegInfo->u4DualAclBtCR2;
    prPtaParam->u4BtCR3 = 0x02000000;//prRegInfo->u4DualAclBtCR3;
    kalMemCopy(&prPtaInfo->rDualAclPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    prPtaParam = &prConnSettings->rDualMixPtaParam;
    prPtaParam->u4BtCR0 = 0x82000061;//prRegInfo->u4DualMixBtCR0;
    prPtaParam->u4BtCR1 = 0x09040f00;//prRegInfo->u4DualMixBtCR1;
    prPtaParam->u4BtCR2 = 0x0;//prRegInfo->u4DualMixBtCR2;
    prPtaParam->u4BtCR3 = 0x0;//prRegInfo->u4DualMixBtCR3;
    kalMemCopy(&prPtaInfo->rDualMixPtaParam, prPtaParam, sizeof(PTA_PARAM_T));
#endif
#endif    	
        return;
    }

    /* Set ATIM window */
    prConnSettings->u2AtimWindow = (UINT_16)prRegInfo->u4AtimWindow;

    /* Set Poll interval */
    prConnSettings->u4ContPollIntv = prRegInfo->u4PollInterval;

    /* Set Poll interval, under no Wi-Fi traffic */
    prConnSettings->u4ContPollIntvB = prRegInfo->u4PollIntervalB;

    /* Set VoIP interval */
    prConnSettings->u4VoipTrafficInterval = prRegInfo->u4VoipInterval;

    /* Set PS mode */
    prConnSettings->rPwrMode = (PARAM_POWER_MODE)prRegInfo->u4PowerMode;

    /* Set ARP filter enable */
    prConnSettings->fgArpFilterEn = (BOOLEAN)((prRegInfo->u4L3PktFilterEn) ? TRUE : FALSE);

    /* Set MAC address */
    COPY_MAC_ADDR(prConnSettings->aucMacAddress, prRegInfo->aucMacAddr);

    /* Set Ad hoc mode */
    prConnSettings->eAdHocMode = (ENUM_PARAM_AD_HOC_MODE_T)prRegInfo->u4AdHocMode;

    /* Set U-APSD AC */
    prConnSettings->bmfgApsdEnAc = (UINT_8)prRegInfo->u4UapsdAc;

    /* Set Roaming enable */
    prConnSettings->fgIsEnableRoaming = (BOOLEAN)((prRegInfo->u4RoamingEn) ? TRUE : FALSE);

#if PTA_ENABLED
    prPtaParam = &prConnSettings->rPtaParam;
    prPtaInfo = &prAdapter->rPtaInfo;
    /* Set BT-Coexist setting, including PTA, BT type and frag control */
    prPtaParam->u4BtCR0 = prRegInfo->u4BtCR0;
    prPtaParam->u4BtCR1 = prRegInfo->u4BtCR1;
    prPtaParam->u4BtCR2 = prRegInfo->u4BtCR2;
    prPtaParam->u4BtCR3 = prRegInfo->u4BtCR3;
    kalMemCopy(&prPtaInfo->rPtaParam, prPtaParam, sizeof(PTA_PARAM_T));
#if PTA_NEW_BOARD_DESIGN
    prPtaParam = &prConnSettings->rSingleAclPtaParam;
    prPtaParam->u4BtCR0 = prRegInfo->u4SingleAclBtCR0;
    prPtaParam->u4BtCR1 = prRegInfo->u4SingleAclBtCR1;
    prPtaParam->u4BtCR2 = prRegInfo->u4SingleAclBtCR2;
    prPtaParam->u4BtCR3 = prRegInfo->u4SingleAclBtCR3;
    kalMemCopy(&prPtaInfo->rSingleAclPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    prPtaParam = &prConnSettings->rSingleMixPtaParam;
    prPtaParam->u4BtCR0 = prRegInfo->u4SingleMixBtCR0;
    prPtaParam->u4BtCR1 = prRegInfo->u4SingleMixBtCR1;
    prPtaParam->u4BtCR2 = prRegInfo->u4SingleMixBtCR2;
    prPtaParam->u4BtCR3 = prRegInfo->u4SingleMixBtCR3;
    kalMemCopy(&prPtaInfo->rSingleMixPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    prPtaParam = &prConnSettings->rDualAclPtaParam;
    prPtaParam->u4BtCR0 = prRegInfo->u4DualAclBtCR0;
    prPtaParam->u4BtCR1 = prRegInfo->u4DualAclBtCR1;
    prPtaParam->u4BtCR2 = prRegInfo->u4DualAclBtCR2;
    prPtaParam->u4BtCR3 = prRegInfo->u4DualAclBtCR3;
    kalMemCopy(&prPtaInfo->rDualAclPtaParam, prPtaParam, sizeof(PTA_PARAM_T));

    prPtaParam = &prConnSettings->rDualMixPtaParam;
    prPtaParam->u4BtCR0 = prRegInfo->u4DualMixBtCR0;
    prPtaParam->u4BtCR1 = prRegInfo->u4DualMixBtCR1;
    prPtaParam->u4BtCR2 = prRegInfo->u4DualMixBtCR2;
    prPtaParam->u4BtCR3 = prRegInfo->u4DualMixBtCR3;
    kalMemCopy(&prPtaInfo->rDualMixPtaParam, prPtaParam, sizeof(PTA_PARAM_T));
#endif
    ASSERT(prRegInfo->u4BTCoexistWindowType < BT_COEXIST_WINDOW_TYPE_NUM);

    #if CFG_TX_FRAGMENT
    prConnSettings->fgTryTxAutoFragmentForBT =
        prRegInfo->u4EnableTxAutoFragmentForBT ? TRUE : FALSE;
    prConnSettings->eBTCoexistWindowType =
        (ENUM_PARAM_BT_COEXIST_WINDOW_T) prRegInfo->u4BTCoexistWindowType;
    #endif /* CFG_TX_FRAGMENT */

//4 2008/10/23, mikewu, these should not be initialized here, 'cause it doen't belong to connection setting
    prPtaInfo->fgSingleAntenna = prRegInfo->u4SingleAntenna? TRUE : FALSE;
    prPtaInfo->fgEnabled = prRegInfo->u4PtaEnabled? TRUE : FALSE;
    ASSERT(prRegInfo->u4BtProfile < BT_PROFILE_NUM);
    prPtaInfo->rBtProfile.eBtProfile= (ENUM_BT_PROFILE_T) prRegInfo->u4BtProfile;
    //kalMemCopy(&prPtaInfo->rPtaParam, prPtaParam, sizeof(PTA_PARAM_T));
    //kalMemCopy(&prPtaInfo->rBtProfile.eBtProfile.u.au4Btcr, prPtaParam, sizeof(PTA_PARAM_T));
#endif

    /* Set Country code */
    if (*((PUINT_16)prRegInfo->aucCountryCode)) {
        kalMemCopy(&prConnSettings->u2CountryCode, prRegInfo->aucCountryCode, 2);
        prConnSettings->u2CountryCode = NTOHS(prConnSettings->u2CountryCode);

    }
    else {
        prConnSettings->u2CountryCode = (UINT_16)prEEPROMCtrl->u4RegulationDomain;
    }

    /* Set LED Blink mode */
    prConnSettings->ucLedBlinkMode = (UINT_8)prRegInfo->u2LedBlinkMode;

#if CFG_SUPPORT_802_11D
    /* Set multi-domain capability */
    prConnSettings->fgMultiDomainCapabilityEnabled = (BOOLEAN)prRegInfo->u4MultiDomainCap;
#endif

    /* Get GPIO2 mode */
    prConnSettings->eGPIO2_Mode =
        (ENUM_PARAM_GPIO2_MODE_T) prRegInfo->u4GPIO2_Mode;

    prConnSettings->ucViAifsnBias = (UINT_8)prRegInfo->u4ViAifsnBias;

    prConnSettings->u2ViMaxTxopLimit = (UINT_16)prRegInfo->u4ViMaxTxopLimit;

    return;
} /* end of nicSetConnectionSettingsByGlueParam() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetupOpChnlList (
    IN P_ADAPTER_T prAdapter,
    IN UINT_16 u2CountryCode,
    IN BOOLEAN fgUpdateIbssChannelInfo
    )
{
    P_RF_CHANNEL_INFO_T pOpChList;
    P_DOMAIN_INFO_ENTRY prDomainInfo;
#if !CFG_ONLY_802_11A
    UINT_32 u4NumDomainInfoEntry;
    UINT_32 i;
#endif
    UINT_8 j;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("nicSetupOpChnlList");

    DBGLOG(NIC, INFO, ("nicSetupOpChnlList\n"));

    ASSERT(prAdapter);

    prConnSettings = &prAdapter->rConnSettings;

    /* Clear nicOperatingChannelList structure */
    kalMemZero(prAdapter->arNicOpChnList, sizeof(prAdapter->arNicOpChnList));
    prAdapter->u2NicOpChnlNum = 0;

#if CFG_ONLY_802_11A
    prDomainInfo = &rDefaultDomainInfo;
#else

    /* Initialize nicOperatingChannelList structure */
    u4NumDomainInfoEntry = sizeof(arSupportedDomainInfoTable) /
                           sizeof(DOMAIN_INFO_ENTRY);

    if (u2CountryCode) {
        for (i = 0; i < u4NumDomainInfoEntry; i++) {
            prDomainInfo = &arSupportedDomainInfoTable[i];
            if (prDomainInfo->u2CountryCode == u2CountryCode) {
                break;
            }
        }
        if (i == u4NumDomainInfoEntry) {
            prDomainInfo = &rDefaultDomainInfo;
        }
    }
    else {
        prDomainInfo = &rDefaultDomainInfo;
    }
#endif
    ASSERT(prDomainInfo->rBand24.ucNumChannels <=
           (sizeof(prAdapter->arNicOpChnList) / sizeof(RF_CHANNEL_INFO_T)));


    for (j = 0; (j < prDomainInfo->rBand24.ucNumChannels) &&
                (j < MAXIMUM_OPERATION_CHANNEL_LIST); j++) {

        pOpChList = &prAdapter->arNicOpChnList[j];

#if CFG_ONLY_802_11A
        pOpChList->ucChannelNum = prDomainInfo->rBand24.ucFirstChannelNum + j * 2;
        pOpChList->eBand = BAND_5G;
#else
        pOpChList->ucChannelNum = prDomainInfo->rBand24.ucFirstChannelNum + j;
        pOpChList->eBand = BAND_24G;
#endif

        prAdapter->u2NicOpChnlNum += 1;

        DBGLOG(NIC, INFO, ("Channel: %d\n", pOpChList->ucChannelNum));
    }

    if (fgUpdateIbssChannelInfo) {
        prConnSettings->ucChannelNum = prAdapter->arNicOpChnList[0].ucChannelNum;
        prConnSettings->ucChannelBand = prAdapter->arNicOpChnList[0].eBand;
    }

    kalMemCopy(&prAdapter->rNicOpDomainInfo, prDomainInfo, sizeof(prAdapter->rNicOpDomainInfo));
    DBGLOG(DOMAIN, INFO, ("u2CountryCode: 0x%x (%c%c)\n",
        prAdapter->rNicOpDomainInfo.u2CountryCode,
        prAdapter->rNicOpDomainInfo.u2CountryCode >> 8,
        prAdapter->rNicOpDomainInfo.u2CountryCode & 0xFF));
    DBGLOG(DOMAIN, INFO, ("ucFirstChannelNum: %d\n", prAdapter->rNicOpDomainInfo.rBand24.ucFirstChannelNum));
    DBGLOG(DOMAIN, INFO, ("ucNumChannels: %d\n", prAdapter->rNicOpDomainInfo.rBand24.ucNumChannels));
    DBGLOG(DOMAIN, INFO, ("cMaxTxPowerLevel: %d\n", prAdapter->rNicOpDomainInfo.rBand24.cMaxTxPowerLevel));
} /* nicSetupOpChnlList */



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicInitializeAdapter (
    IN P_ADAPTER_T prAdapter
    )
{
    P_BSS_INFO_T prBssinfo;
    UINT_8 aucZeroMacAddr[MAC_ADDR_LEN] = {0,0,0,0,0,0};


    ASSERT(prAdapter);
    prBssinfo = &prAdapter->rBssInfo;

    //4 <1> Default Driver Feature
    prAdapter->fgIsEnableWMM = TRUE;
    //prAdapter->fgIsEnableRoaming = TRUE; /* Move to rConnSettings */
    prAdapter->fgIsEnableRoamingGsm = FALSE;
    prAdapter->fgIsEnableJoinToHiddenSSID = FALSE;

    //4 <2> Default Channel RF

    //4 <3> Default IER & Flags
    prAdapter->au4IntEnable[INT_HISR] = HIER_DEFAULT;
    prAdapter->au4IntEnable[INT_HSCISR] = HSCIER_DEFAULT;
    prAdapter->fgIsIntEnableWithLPOwnSet = FALSE;

    //4 <4> Judge which MAC Address we should use for Adapter.
    /* If the input MAC Address is NULL, use the MAC Address from EEPROM. */
    if (EQUAL_MAC_ADDR(prAdapter->rConnSettings.aucMacAddress, aucZeroMacAddr)) {
        COPY_MAC_ADDR(prAdapter->aucMacAddress, prAdapter->rEEPROMCtrl.aucMacAddress);
    }
    else {
        COPY_MAC_ADDR(prAdapter->aucMacAddress, prAdapter->rConnSettings.aucMacAddress);
    }


    //4 <5> Protocol related default value (MIB value)
    nicSetAvailablePhyTypeSet(prAdapter);

    /* Choose eCurrentPhyType for load setup default e.g. BB/RF calibration */
#if CFG_ONLY_802_11A
    prAdapter->eCurrentPhyType = PHY_TYPE_OFDM_INDEX;
#else
    if (prAdapter->u2AvailablePhyTypeSet & PHY_TYPE_BIT_ERP) {
        prAdapter->eCurrentPhyType = PHY_TYPE_ERP_INDEX;
    }
    else if (prAdapter->u2AvailablePhyTypeSet & PHY_TYPE_BIT_HR_DSSS) {
        prAdapter->eCurrentPhyType = PHY_TYPE_HR_DSSS_INDEX;
    }
    else if (prAdapter->u2AvailablePhyTypeSet & PHY_TYPE_BIT_OFDM) {
        prAdapter->eCurrentPhyType = PHY_TYPE_OFDM_INDEX;
    }
    else {
        ASSERT(0);
    }
#endif

    //4 <6> Operation Mode & Media State (Indication Synchronization) default value.
    prAdapter->eCurrentOPMode = OP_MODE_INFRASTRUCTURE;

    /* NOTE(Kevin): for windows, we may delay the Disconnect Event for 10 sec.
     * During this period, the eConnectionState == MEDIA_STATE_DISCONNECTED.
     * But the eConnectionStateIndicated == MEDIA_STATE_CONNECTED.
     */
    prAdapter->eConnectionState = MEDIA_STATE_DISCONNECTED; /* Instantaneous Media State */
    prAdapter->eConnectionStateIndicated = MEDIA_STATE_DISCONNECTED; /* The Media State that report to OS */

    prAdapter->fgBypassPortCtrlForRoaming = FALSE;

    prAdapter->fgIsIBSSActive = FALSE;
    prAdapter->fgIsIBSSBeaconTimeoutAlarm = FALSE;

    /* Initialize in nicInitializeChip */
    //nicARInit(prAdapter);

    //4 2007/11/05, mikewu, do we need this?
    kalMemZero(prBssinfo, sizeof(BSS_INFO_T));

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicCheckAvailablePhyTypeSet (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PARAM_PHY_TYPE_T eNewNetworkType
    )
{
    UINT_16 u2AvailablePhyTypeSet;
    ASSERT(prAdapter);

    ASSERT(prAdapter->u2SupportedPhyTypeSet);

    nicEepromGetAvailablePhyTypeSet(
                    &prAdapter->rEEPROMCtrl,
                    &u2AvailablePhyTypeSet);

    switch (eNewNetworkType) {
    case PHY_TYPE_802_11BG:
        u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11BG;
        break;

    case PHY_TYPE_802_11G:
        u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11G;
        break;

    case PHY_TYPE_802_11A:
        u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11A;
        break;

    case PHY_TYPE_802_11B:
        u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11B;
        break;

    default:
        u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11ABG;
        break;
    }

    u2AvailablePhyTypeSet &= prAdapter->u2SupportedPhyTypeSet;

    if (!u2AvailablePhyTypeSet) {
        return WLAN_STATUS_FAILURE;
    }

    return WLAN_STATUS_SUCCESS;
} /* end of nicCheckAvailablePhyTypeSet() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetAvailablePhyTypeSet (
    IN P_ADAPTER_T prAdapter
    )
{
    P_CONNECTION_SETTINGS_T prConnSettings;


    ASSERT(prAdapter);
    prConnSettings = &prAdapter->rConnSettings;

    halGetSupportedPhyTypeSet(&prAdapter->u2SupportedPhyTypeSet);

    ASSERT(prAdapter->u2SupportedPhyTypeSet);

    nicEepromGetAvailablePhyTypeSet(
                    &prAdapter->rEEPROMCtrl,
                    &prAdapter->u2AvailablePhyTypeSet);

    switch (prConnSettings->eDesiredPhyType) {
    case PHY_TYPE_802_11ABG:
        prAdapter->u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11ABG;
        break;

    case PHY_TYPE_802_11BG:
        prAdapter->u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11BG;
        break;

    case PHY_TYPE_802_11G:
        prAdapter->u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11G;
        break;

    case PHY_TYPE_802_11A:
        prAdapter->u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11A;
        break;

    case PHY_TYPE_802_11B:
        prAdapter->u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11B;
        break;

    default:
        prAdapter->u2AvailablePhyTypeSet &= (UINT_16)PHY_TYPE_SET_802_11ABG;
        break;
    }

    prAdapter->u2AvailablePhyTypeSet &= prAdapter->u2SupportedPhyTypeSet;

#if CFG_ONLY_802_11A
    prAdapter->u2AvailablePhyTypeSet = (UINT_16)PHY_TYPE_SET_802_11ABG;
#endif

    if (!prAdapter->u2AvailablePhyTypeSet) {
        prAdapter->u2AvailablePhyTypeSet = prAdapter->u2SupportedPhyTypeSet;
    }

    return;
} /* end of nicSetAvailablePhyTypeSet() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetSlotTime (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PHY_TYPE_INDEX_T ePhyTypeIndex,
    IN BOOLEAN fgUseShortSlotTime
    )
{
    P_RF_INFO_T prRFInfo;
    UINT_8 ucaSlotTime;


    ASSERT(prAdapter);
    prRFInfo = &prAdapter->rRFInfo;

    if (ePhyTypeIndex >= PHY_TYPE_INDEX_NUM) {
        return;
    }


    switch (ePhyTypeIndex) {
    case PHY_TYPE_ERP_INDEX:
        ucaSlotTime = (fgUseShortSlotTime) ? SLOT_TIME_ERP_SHORT : SLOT_TIME_ERP_LONG;
        break;

    case PHY_TYPE_HR_DSSS_INDEX:
        ucaSlotTime = SLOT_TIME_HR_DSSS;
        break;

    case PHY_TYPE_OFDM_INDEX:
        ucaSlotTime = SLOT_TIME_OFDM;
        break;

    default:
        return;
    }

    prRFInfo->ucaSlotTime = ucaSlotTime; /* Save current value for reset in an emergency. */

    halSetSlotTime(prAdapter, ucaSlotTime);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSwitchChannel (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BAND_T eBand,
    IN UINT_8      ucChannelNum,
    IN UINT_8      ucTxPwrLimit
    )
{
    P_RF_INFO_T prRFInfo;
    BOOLEAN     fgTxFilterForJp;
#if DBG
    UINT_16 i;
#endif

    DEBUGFUNC("nicSwitchChannel");

    ASSERT(prAdapter);
    prRFInfo = &prAdapter->rRFInfo;

    DBGLOG(NIC, INFO, ("Switch to Band = %s, Channel = %d\n",
        ((eBand == BAND_24G) ? "2.4G" : "5G"), ucChannelNum));

#if DBG
    for (i = 0; i < prAdapter->u2NicOpChnlNum + 1; i++) {
        if ((ucChannelNum == prAdapter->arNicOpChnList[i].ucChannelNum) &&
            (eBand        == prAdapter->arNicOpChnList[i].eBand)) {
            break;
        }
        else if (i == prAdapter->u2NicOpChnlNum) {
            ASSERT(0);
        }
    }
#endif

    /* do this check to ensure the instruction for the same band will not been written duplicately */
    // TODO:

    // to ensure band is programmed at least once!?
    halBBChangeBand(prAdapter, eBand);
    fgTxFilterForJp = nicSetTxFilterForJP(prAdapter, ucChannelNum, eBand);
    halRFSwitchChannel(prAdapter, ucChannelNum, eBand, fgTxFilterForJp);
    halSetTxPowerGain(prAdapter, ucChannelNum, eBand, NULL, NULL);

    /* set the channel and band in SW */
    prRFInfo->ucChannelNum = ucChannelNum;
    prRFInfo->eBand = eBand;

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetupBSS (
    IN P_ADAPTER_T prAdapter,
    IN P_BSS_INFO_T prBSSInfo
    )
{
    ASSERT(prAdapter);
    ASSERT(prBSSInfo);

    halSetBSSID(prAdapter, prBSSInfo->aucBSSID);

    if (prBSSInfo->eBSSType == BSS_TYPE_IBSS) {

        halSetOPMode(prAdapter, OP_MODE_IBSS);

        //4 Open Rx Filter to allow "probe req" to be received
        NIC_SET_RX_FILTER(prAdapter, RXFILTER_RXPROREQ);
    }
    else {
        halSetOPMode(prAdapter, OP_MODE_INFRASTRUCTURE);

        NIC_UNSET_RX_FILTER(prAdapter, RXFILTER_RXPROREQ);
    }

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicStartBeacon (
    IN P_ADAPTER_T prAdapter
    )
{
    P_BSS_INFO_T prBssInfo;


    ASSERT(prAdapter);
    prBssInfo = &prAdapter->rBssInfo;

    halSetBeaconContent(prAdapter,
                        prBssInfo->aucBcnContent,
                        prBssInfo->u2BcnLen,
                        prBssInfo->ucBasicRateIndex,
                        prBssInfo->fgUseShortPreamble);

    nicTxStartQueues(prAdapter, TXQ_BEACON_MASK);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicStopBeacon (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);


    nicTxFlushStopQueues(prAdapter, 0x0, TXQ_BEACON_MASK);

    /* NOTE(Kevin): We have to set Infrastructure Mode in order to stop some
     * Service Period related functions..
     */
    halSetOPMode(prAdapter, OP_MODE_INFRASTRUCTURE);

    /* NOTE(Kevin): When leave IBSS ALONE if timeout, we should do SCAN in IBSS
     * Mode in order for correct RCPI value.
     */
    halSetOPMode(prAdapter, OP_MODE_IBSS);

    return;
} /* end of nicStopBeacon() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicResumeBeacon (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);


    nicTxStartQueues(prAdapter, TXQ_BEACON_MASK);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicPauseBeacon (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);


    nicTxFlushStopQueues(prAdapter, 0x0, TXQ_BEACON_MASK);

    /* NOTE(Kevin): We have to set Infrastructure Mode in order to stop some
     * Service Period related functions..
     */
    halSetOPMode(prAdapter, OP_MODE_INFRASTRUCTURE);

    halSetOPMode(prAdapter, OP_MODE_IBSS);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicExistIBSS (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);


    nicTxFlushStopQueues(prAdapter, 0x0, TXQ_BEACON_MASK);

    halSetOPMode(prAdapter, OP_MODE_INFRASTRUCTURE);

    /* NOTE(Kevin 2008/05/2): DE suggest to stop Beacon Queue again after change the
     * Operation Mode to Infrastructure Mode.
     * Use hal instead of nic.
     */
    halFlushStopQueues(prAdapter, 0, TXQ_BEACON_MASK);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessAbnormalInterrupt(
    IN P_ADAPTER_T      prAdapter
    )
{
#if DBG
    UINT_32 i;
#endif

    DEBUGFUNC("nicProcessAbnormalInterrupt");

    DBGLOG(NIC, ERROR, ("Abnormal Interrupt arose\n"));

    ASSERT(prAdapter);

#if DBG
    for (i = 0; i < INT_NUM; i++) {
        DBGLOG(NIC, ERROR, ("prAdapter->au4IntStatus[%ld] = %08lx\n",
            i, prAdapter->au4IntStatus[i]));
    }
#endif
    {
        UINT_32 u4RegValue = 0;

        HAL_MCR_RD(prAdapter, MCR_CIR, &u4RegValue);
        if (u4RegValue & HLPCR_LP_OWN_STATE) {
            HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_LP_OWN_CLR);
        }

        HAL_MCR_RD(prAdapter, MCR_ASR, &u4RegValue);
        DBGLOG(NIC, ERROR, ("MCR_ASR = %08lx\n", u4RegValue));
    }


    nicLogicReset(prAdapter);

    // Force to set a break point for watchdog event in this phase
    ASSERT(0);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessWatchDogInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
#if DBG
    UINT_32 i;
#endif

    DEBUGFUNC("nicProcessWatchDogInterrupt");

    DBGLOG(NIC, ERROR, ("Watch-dog Interrupt arose\n"));

    ASSERT(prAdapter);

#if DBG
    for (i = 0; i < INT_NUM; i++) {
        DBGLOG(NIC, ERROR, ("prAdapter->au4IntStatus[%ld] = %08lx\n",
            i, prAdapter->au4IntStatus[i]));
    }
#endif
    {
        UINT_32 u4RegValue = 0;

        HAL_MCR_RD(prAdapter, MCR_CIR, &u4RegValue);
        if (u4RegValue & HLPCR_LP_OWN_STATE) {
            HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_LP_OWN_CLR);
        }

        HAL_MCR_RD(prAdapter, MCR_ASR, &u4RegValue);
        DBGLOG(NIC, ERROR, ("MCR_ASR = %08lx\n", u4RegValue));

        /* CR1853: handle abnormal case for ship build. */
        if (u4RegValue & 0x1) {
            DBGLOG(LP, INFO, ("Change PS profile to ENUM_PSP_CONTINUOUS_ACTIVE\n"));
            pmFsmRunEventSetPowerSaveProfileWatchDog(prAdapter, ENUM_PSP_CONTINUOUS_ACTIVE);
        }
    }

    nicLogicReset(prAdapter);

    // Force to set a break point for watchdog event in this phase
    ASSERT(0);

}

#if CFG_LP_IOT
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTsfDriftInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
    DEBUGFUNC("nicProcessTsfDriftInterrupt");

    DBGLOG(LP_IOT, INFO, ("[LP-IOT] TSF drift interrupt!\n"));

    ASSERT(prAdapter);
}
#else
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTsfDriftInterrupt (
    IN P_ADAPTER_T      prAdapter
    )
{
    DEBUGFUNC("nicProcessTsfDriftInterrupt");

    DBGLOG(LP, WARN, ("TSF drift interrupt!\n"));

    ASSERT(prAdapter);
}
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessBeaconTxOkInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);
#if 0//juji, use HW mechanism directly
    prAdapter->rBssInfo.fgIbssActive = TRUE;
#endif
}


#if CFG_TCP_IP_CHKSUM_OFFLOAD
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetCSUMOffload (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4CSUMFlags
    )
{
    ASSERT(prAdapter);


    prAdapter->u4CSUMFlags = u4CSUMFlags;
    halSetCSUMOffload(prAdapter, prAdapter->u4CSUMFlags);

    return;

} /* end of nicSetCSUMOffload() */
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRRAverageEnable (
    IN P_ADAPTER_T  prAdapter
    )
{
    ASSERT(prAdapter);


    /* NOTE(Kevin 2008/05/15): Disable RR first and then Enable it will let RR reset
     * the initial value. After enable, RR will treat the first Beacon's RCPI as
     * initial value and won't be effected by the RCPI moving average parameter.
     */
    halRRDisable(prAdapter);

    halRREnable(prAdapter,
                RCPI_HIGH_BOUND,
                RCPI_LOW_BOUND);

    prAdapter->fgEnableAverageRcpiFunc = TRUE;

    return;
} /* end of nicRRAverageEnable() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRRAverageDisable (
    IN P_ADAPTER_T  prAdapter
    )
{
    ASSERT(prAdapter);


    halRRDisable(prAdapter);

    prAdapter->fgEnableAverageRcpiFunc = FALSE;

    return;
} /* end of nicRRAverageDisable() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRRTriggerEnable (
    IN P_ADAPTER_T  prAdapter,
    IN RCPI         rRCPIUpperThreshold,
    IN RCPI         rRCPILowerThreshold
    )
{
    ASSERT(prAdapter);


    if (!prAdapter->fgEnableAverageRcpiFunc) {
        nicRRAverageEnable(prAdapter);
    }

    halRREnable(prAdapter,
                rRCPIUpperThreshold,
                rRCPILowerThreshold);

    NIC_SET_INT_EVENT(prAdapter, INT_EVENT_RCPI);

    return;
} /* end of nicRRTriggerEnable() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRRTriggerDisable (
    IN P_ADAPTER_T  prAdapter
    )
{
    ASSERT(prAdapter);

    if (prAdapter->fgEnableAverageRcpiFunc) {

        halRREnable(prAdapter,
                    RCPI_HIGH_BOUND,
                    RCPI_LOW_BOUND);

        NIC_UNSET_INT_EVENT(prAdapter, INT_EVENT_RCPI);
    }

    return;
} /* end of nicRRTriggerDisable() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicRRGetRCPI (
    IN P_ADAPTER_T  prAdapter,
    OUT P_RCPI      prRCPI
    )
{
    ASSERT(prAdapter);
    ASSERT(prRCPI);

    halRRGetRCPI(prAdapter, prRCPI);

    if (*prRCPI == RCPI_MEASUREMENT_NOT_AVAILABLE) {
        ASSERT(0);
    }

    return;
} /* end of nicRRGetRCPI() */


#if CFG_LP_IOT
UINT_8 g_ucMaxRcpi = 0;
UINT_8 g_ucMinRcpi = 0;
UINT_8 g_ucOrgRcpi = 0;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessRcpiInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32     u4Mcr;
    UINT_8 ucCurRcpi;
    DEBUGFUNC("nicProcessRcpiInterrupt");

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_RR, &u4Mcr);

    ucCurRcpi = (UINT_8)(u4Mcr & BITS(0, 7));

    if (ucCurRcpi > g_ucMaxRcpi) {
        g_ucMaxRcpi = ucCurRcpi;
    }
    if (ucCurRcpi < g_ucMinRcpi) {
        g_ucMinRcpi = ucCurRcpi;
    }

    if (g_ucOrgRcpi != 0) {
        DBGLOG(LP_IOT, INFO, ("[LP-IOT] RCPI triggered!! value: 0x%x (%d dBm) (original: 0x%x, Max : 0x%x, Min: 0x%x) (MCR_RR=0x%08X)\n",
                                ucCurRcpi,
                                RCPI_TO_DBM(ucCurRcpi),
                                g_ucOrgRcpi,
                                g_ucMaxRcpi,
                                g_ucMinRcpi,
                                u4Mcr));
    }

    return;
} /* end of nicProcessRcpiInterrupt() */

#else /* CFG_LP_IOT */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessRcpiInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    //UINT_32     u4Mcr;
    DEBUGFUNC("nicProcessBBInterrupt");

    ASSERT(prAdapter);

    arbFsmRunEventRoamingRCPI(prAdapter);

    return;
} /* end of nicProcessRcpiInterrupt() */

#endif /* CFG_LP_IOT */



#if CFG_LP_IOT
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessBeaconTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{

    OS_SYSTIME rCurrentSysTime;
    ULARGE_INTEGER rCurrentTsf;

    DEBUGFUNC("nicProcessBeaconTimeoutInterrupt");

    ASSERT(prAdapter);
    if (prAdapter->eConnectionState != MEDIA_STATE_CONNECTED) {
        return;
    }

    GET_CURRENT_SYSTIME(&rCurrentSysTime);
    NIC_GET_CURRENT_TSF(prAdapter, &rCurrentTsf);

    DBGLOG(LP_IOT, INFO, ("[LP-IOT] Beacon timeout INT !! (%ld)\n", ++u4AccumBcnTimeoutCnt));
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] current systime: %08lx\n", rCurrentSysTime));
    DBGLOG(LP_IOT, INFO, ("[LP-IOT] current TSF: 0x%08lx-0x%08lx\n",
                        rCurrentTsf.u.HighPart, rCurrentTsf.u.LowPart));

    pmFsmRunEventReadBeaconTimeoutCount(prAdapter);

    if (CHECK_FOR_TIMEOUT(rCurrentSysTime,
                          ARB_FSM_GET_LAST_RX_UC_DATA_FRAME_SYSTIME(prAdapter),
                          MSEC_TO_SYSTIME(BEACON_TIMEOUT_GUARD_TIME_FROM_UC_DATA_MSEC))) {

        DBGLOG(LP_IOT, INFO, ("[LP-IOT] Beacon lost... do disconnect diagnostic\n"));

        arbFsmRunEventConnectionTest(prAdapter);
    }
    else {
        DBGLOG(LP_IOT, INFO, ("[LP-IOT] Beacon timeout not triggered due to UC data is received in the guard time\n"));
    }

    return;
}

#else

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessBeaconTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    OS_SYSTIME rCurrentSysTime;

    DEBUGFUNC("nicProcessBeaconTimeoutInterrupt");

    DBGLOG(LP, INFO, ("nicProcessBeaconTimeoutInterrupt\n"));

    ASSERT(prAdapter);
    /* Here we check the beacon timeout event combined with the last RX UC data frame timestamp.
       This is for covering few AP that the beacon will not transmitted during heavy traffic load,
       but in this way, we cannot cover the heavy traffic is not target to our STA
    */
    GET_CURRENT_SYSTIME(&rCurrentSysTime);

    if (CHECK_FOR_TIMEOUT(rCurrentSysTime,
                          ARB_FSM_GET_LAST_RX_UC_DATA_FRAME_SYSTIME(prAdapter),
                          MSEC_TO_SYSTIME(BEACON_TIMEOUT_GUARD_TIME_FROM_UC_DATA_MSEC))) {

        DBGLOG(LP, INFO, ("Beacon lost... do disconnect diagnostic\n"));

        arbFsmRunEventConnectionTest(prAdapter);
    }
    else {
        DBGLOG(LP, INFO, ("Beacon timeout not triggered due to UC data is received in the guard time\n"));
    }

    return;

}
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTxNullFailInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("nicProcessTxNullFailInterrupt");


    DBGLOG(LP, INFO, ("nicProcessTxNullFailInterrupt\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prConnSettings = &prAdapter->rConnSettings;

    /* Handle the same process as beacon timeout handler, under continuous polling mode.
       Which is due to beacon timeout is disabled, and the jobs is moved to HW transmission
       packet (error) handlers */
    if (prPmInfo->fgIsContinousPollingEnabled) {
        prPmInfo->u4ContiHwTxFail++;

        if (prPmInfo->u4ContiHwTxFail > DEFAULT_CONTI_HW_TX_FAIL) {
            prPmInfo->u4ContiHwTxFail = 0;
            nicProcessBeaconTimeoutInterrupt(prAdapter);
        } else {
            /* Enable continuos polling check timer */
            ARB_SET_TIMER(prAdapter,
                           prPmInfo->rContiHwTxFailCheckTimer,
                           (prPmInfo->fgContiPollIsStepUp ? prConnSettings->u4ContPollIntv + 100 : prConnSettings->u4ContPollIntvB + 100)
                           );
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTxTriggerFailInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("nicProcessTxTriggerFailInterrupt");


    DBGLOG(LP, INFO, ("nicProcessTxTriggerFailInterrupt\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prConnSettings = &prAdapter->rConnSettings;

    /* Handle the same process as beacon timeout handler, under continuous polling mode.
       Which is due to beacon timeout is disabled, and the jobs is moved to HW transmission
       packet (error) handlers */
    if (prPmInfo->fgIsContinousPollingEnabled) {
        prPmInfo->u4ContiHwTxFail++;

        if (prPmInfo->u4ContiHwTxFail > DEFAULT_CONTI_HW_TX_FAIL) {
            prPmInfo->u4ContiHwTxFail = 0;
            nicProcessBeaconTimeoutInterrupt(prAdapter);
        } else {
            /* Enable continuos polling check timer */
            ARB_SET_TIMER(prAdapter,
                           prPmInfo->rContiHwTxFailCheckTimer,
                           (prPmInfo->fgContiPollIsStepUp ? prConnSettings->u4ContPollIntv + 100 : prConnSettings->u4ContPollIntvB + 100)
                           );
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessTxPsPollFailInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    P_PM_INFO_T                 prPmInfo;
    P_CONNECTION_SETTINGS_T prConnSettings;

    DEBUGFUNC("nicProcessTxPsPollFailInterrupt");


    DBGLOG(LP, INFO, ("nicProcessTxPsPollFailInterrupt\n"));

    ASSERT(prAdapter);

    prPmInfo = &prAdapter->rPmInfo;
    prConnSettings = &prAdapter->rConnSettings;

    /* Handle the same process as beacon timeout handler, under continuous polling mode.
       Which is due to beacon timeout is disabled, and the jobs is moved to HW transmission
       packet (error) handlers */
    if (prPmInfo->fgIsContinousPollingEnabled) {
        prPmInfo->u4ContiHwTxFail++;

        if (prPmInfo->u4ContiHwTxFail > DEFAULT_CONTI_HW_TX_FAIL) {
            prPmInfo->u4ContiHwTxFail = 0;
            nicProcessBeaconTimeoutInterrupt(prAdapter);
        } else {
            /* Enable continuos polling check timer */
            ARB_SET_TIMER(prAdapter,
                           prPmInfo->rContiHwTxFailCheckTimer,
                           (prPmInfo->fgContiPollIsStepUp ? prConnSettings->u4ContPollIntv + 100 : prConnSettings->u4ContPollIntvB + 100)
                           );
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessRxBmcTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    DEBUGFUNC("nicProcessRxBmcTimeoutInterrupt");


    DBGLOG(LP, INFO, ("nicProcessRxBmcTimeoutInterrupt\n"));

    ASSERT(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessApsdTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    DEBUGFUNC("nicProcessApsdTimeoutInterrupt");


    DBGLOG(LP, INFO, ("nicProcessApsdTimeoutInterrupt\n"));

    ASSERT(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessPsPollTimeoutInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    DEBUGFUNC("nicProcessPsPollTimeoutInterrupt");


    DBGLOG(LP, INFO, ("nicProcessPsPollTimeoutInterrupt\n"));

    ASSERT(prAdapter);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicLogicReset (
    IN P_ADAPTER_T prAdapter
    )
{
    P_BSS_INFO_T prBssInfo;
    UINT_32 i;

    ASSERT(prAdapter);
    prBssInfo = &prAdapter->rBssInfo;
    halLogicReset(prAdapter);

    for (i = 0; i < INT_NUM; i ++) {
        prAdapter->au4IntStatus[i]= 0;
    }

#if CFG_SDIO_STATUS_ENHANCE
    {
        P_SDIO_CTRL_T prSDIOCtrl = &prAdapter->rSDIOCtrl;

        kalMemZero(prSDIOCtrl->au2RxLengthDW, sizeof(prSDIOCtrl->au2RxLengthDW));
        kalMemZero(prSDIOCtrl->arTxStatus, sizeof(prSDIOCtrl->arTxStatus));
    }
#endif /* CFG_SDIO_STATUS_ENHANCE */


    /* juji: we do not invoke scanFsmRunEventScanAbort() for it might
       also change the SCAN_STATE */
    NIC_RESET_INT_EVENT(prAdapter, INT_EVENT_SCAN_DONE);

    /* TODO(Kevin 08/10/2008): check if connected first */
    if (prBssInfo->eBSSType == BSS_TYPE_IBSS) {
        nicStartBeacon(prAdapter);
    }

    nicTxRefreshQueues(prAdapter);

    /* Pull down more packets */
    nicTxRetransmitOfSendWaitQue(prAdapter);

    if (prAdapter->rTxCtrl.fgIsPacketInOsSendQueue) {
        nicTxRetransmitOfOsSendQue(prAdapter);
    }

    #if CFG_SDIO_TX_ENHANCE
    halTxLeftClusteredMpdu(prAdapter);
    #endif /* CFG_SDIO_TX_ENHANCE */

    /* To info the sec fsm the AC4 FIFO empty event */
    nicTxSignalFifoEmptyEvent(prAdapter);

    /* timer recovery for reset (timeout value will be reloaded by logic reset) */
#if !CFG_USE_SW_ROOT_TIMER && !CFG_SDIO_DEVICE_DRIVER_WO_NDIS
    timerDoTimeOutCheckOnLogicReset(prAdapter);
#endif

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetUnicastAddr (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    halSetMACAddr(prAdapter,
                  0,
                  prAdapter->aucMacAddress);

    return;
} /* end of nicSetUnicastAddr() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetMulticastAddrList (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 paucMulticastAddrList,
    IN UINT_8 ucNum
    )
{
    PUINT_8 pucAddrPtr = paucMulticastAddrList;
    UINT_8 i = 0;

    DEBUGFUNC("nicSetMulticastAddrList");


    ASSERT(prAdapter);
    ASSERT(ucNum <= MAX_NUM_GROUP_ADDR);

    prAdapter->ucMulticastAddrNum =
        (ucNum > MAX_NUM_GROUP_ADDR) ? MAX_NUM_GROUP_ADDR : ucNum;

    if (ucNum == 0) { /* Clear all multicast list */
        halClearMulticastAddrList(prAdapter);
        return;
    }
#if DBG
    else {
        ASSERT(pucAddrPtr);
    }
#endif /* DBG */

    for (i = 0; i < MAX_NUM_GROUP_ADDR;  i++, pucAddrPtr += MAC_ADDR_LEN) {
        if (i < prAdapter->ucMulticastAddrNum) {
            COPY_MAC_ADDR(prAdapter->aucMulticastAddr[i], pucAddrPtr);
        }
        else {
            kalMemZero(prAdapter->aucMulticastAddr[i], MAC_ADDR_LEN);
        }

        DBGLOG(RX, LOUD, ("Set MCAST[%d]: "MACSTR"\n", i, MAC2STR(prAdapter->aucMulticastAddr[i])));
        halSetMACAddr(prAdapter, (i + 1), prAdapter->aucMulticastAddr[i]);
    }

    return;
} /* end of nicSetMulticastAddrList() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
nicGetChannelBandFromFreq (
    IN  P_ADAPTER_T   prAdapter,
    IN  UINT_32       u4ChannelFreq,
    OUT P_ENUM_BAND_T peBand,
    OUT PUINT_8      pucChannelNum
    )
{
    P_RF_CHANNEL_PROG_ENTRY prChannelEntry = NULL;

    DEBUGFUNC("nicGetChannelBandFromFreq");

    ASSERT(prAdapter);
    ASSERT(peBand);
    ASSERT(pucChannelNum);

    prChannelEntry =
        halRFGetRFChnlProgEntryFromChannelFreq( prAdapter,  u4ChannelFreq);

    if(prChannelEntry == (P_RF_CHANNEL_PROG_ENTRY)NULL) {
        DBGLOG(PHY, ERROR, ("Can not get channel number with Freq = %ld\n", u4ChannelFreq));

        return WLAN_STATUS_FAILURE;
    }
    /* get channel and band info */
    *pucChannelNum = prChannelEntry->ucChannelNum;
    *peBand = prChannelEntry->eBand;

    return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicEepromWrite16 (
    IN P_ADAPTER_T      prAdapter,
    IN UINT_8           ucEepromWordOffset,
    IN UINT_16              u2EepromData
    )
{
    ASSERT(prAdapter);
    return halEepromWrite16(
                    prAdapter,
                    ucEepromWordOffset,
                    u2EepromData
                    );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicEepromRead16 (
    IN      P_ADAPTER_T     prAdapter,
    IN      UINT_8          ucEepromWordOffset,
    OUT     PUINT_16        pu2EepromData
    )
{
    ASSERT(prAdapter);
    return halEepromRead16(
                    prAdapter,
                    ucEepromWordOffset,
                    pu2EepromData
                    );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_16
nicEepromGetSize (
    IN      P_ADAPTER_T     prAdapter
    )
{
    ASSERT(prAdapter);
    return halEepromGetSize(
                    prAdapter
                    );
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetHwTxByBasicRate (
    IN  P_ADAPTER_T prAdapter,
    IN BOOLEAN fgUseBasicRate
    )
{
    ASSERT(prAdapter);
    halSetHwTxByBasicRate(prAdapter, fgUseBasicRate);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicGetRandomNumber (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_16 pu2Number
    )
{
    ASSERT(prAdapter);
    ASSERT(pu2Number);

    halGetRandomNumber(prAdapter, pu2Number);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
UINT_32
nicGetFreqByChannelNum (
    IN UINT_8          ucChannelNum,
    IN ENUM_BAND_T     eBand
    )
{
    P_CHANNEL_BAND_FREQ_MAP parChnlFreqMappingEntry;
    UINT_32 i;

    for (i = 0;
         i < (sizeof(arChnlFreqMappingTable) / sizeof(CHANNEL_BAND_FREQ_MAP));
         i++) {

        parChnlFreqMappingEntry = &arChnlFreqMappingTable[i];

        if (parChnlFreqMappingEntry->eBand == eBand &&
            parChnlFreqMappingEntry->ucChannelNum == ucChannelNum) {
            return parChnlFreqMappingEntry->u4ChannelFreq;
        }
    }

    return 0;
} /* nicGetFreqByChannelNum */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicGetChannelNumByFreq (
    IN UINT_32      u4FreqInkHz,
    OUT UINT_8      *pucChannelNum,
    OUT ENUM_BAND_T *peBand
    )
{
    P_CHANNEL_BAND_FREQ_MAP parChnlFreqMappingEntry;
    UINT_32 i;

    ASSERT(pucChannelNum);
    ASSERT(peBand);

    for (i = 0;
         i < (sizeof(arChnlFreqMappingTable) / sizeof(CHANNEL_BAND_FREQ_MAP));
         i++) {

        parChnlFreqMappingEntry = &arChnlFreqMappingTable[i];

        if (parChnlFreqMappingEntry->u4ChannelFreq == u4FreqInkHz) {
            *pucChannelNum = parChnlFreqMappingEntry->ucChannelNum;
            *peBand = parChnlFreqMappingEntry->eBand;
            return TRUE;
        }
    }

    return FALSE;
} /* nicGetChannelNumByFreq */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_THERMO_INFO_T     prThermoInfo;

    DEBUGFUNC("nicThermoInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prThermoInfo = &prAdapter->rThermoInfo;

#if CFG_THERMO_TIMER_EN
    nicThermoTimerInit(prAdapter);
#endif

#if CFG_THERMO_INT_EN
    nicThermoIntInit(prAdapter);
#endif
    prThermoInfo->rState = UNDEF_TEMP;
    DBGLOG(INIT, TRACE, ("Current Thermo State : %d\n",\
            prThermoInfo->rState));

    return;
}


#if CFG_THERMO_TIMER_EN
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoTimerInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_THERMO_INFO_T     prThermoInfo;
    P_EEPROM_CTRL_T     prEEPROMCtrl;
    ALC_VAL             rAlcInitVal;

    DEBUGFUNC("nicThermoTimerInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prThermoInfo = &prAdapter->rThermoInfo;
    prEEPROMCtrl = &prAdapter->rEEPROMCtrl;

#if CFG_FAKE_THERMO_VALUE_DBG_EN
    NIC_ALC_DISABLE(prAdapter);
#else
    //Disable Interrupt.
    halALCREnable(prAdapter, prThermoInfo->u4AlcArParam, 0x00, 0x3F, 0x00);
    halALCRTriggerALC(prAdapter);
    halALCRGetRawValue(prAdapter, &rAlcInitVal);
    halALCREnable(prAdapter, ALCR_AR_PARM_1_OF_4, 0x3F, 0x00, rAlcInitVal);
#endif


    ARB_INIT_TIMER(prAdapter,
                   prThermoInfo->rTimer,
                   nicThermoTimerTimeOut,
                   TRUE);

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        prThermoInfo->fgUpdateTxGainFromAlcInt = TRUE;
        prThermoInfo->fgUpdateLnaFromAlcInt = TRUE;
    }
    else{

        if (prEEPROMCtrl->fgAlcUseThermoEn == FALSE && \
            prEEPROMCtrl->fgLnaUseThermoEn == FALSE) {
            prThermoInfo->fgUpdateTxGainFromAlcInt = FALSE;
            prThermoInfo->fgUpdateLnaFromAlcInt = FALSE;
            return;
        }
        if (prEEPROMCtrl->fgAlcUseThermoEn == TRUE ) {
            prThermoInfo->fgUpdateTxGainFromAlcInt = TRUE;
        }

        if (prEEPROMCtrl->fgLnaUseThermoEn == TRUE ) {
            prThermoInfo->fgUpdateLnaFromAlcInt= TRUE;
        }

    }

    ARB_SET_TIMER(prAdapter,
                  prThermoInfo->rTimer,
                  CFG_PEEK_THERMO_PERIOD_MSEC);
    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoTimerUnInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_THERMO_INFO_T     prThermoInfo;

    DEBUGFUNC("nicThermoTimerUnInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prThermoInfo = &prAdapter->rThermoInfo;

    ARB_CANCEL_TIMER(prAdapter, prThermoInfo->rTimer);
    NIC_ALC_DISABLE(prAdapter);

    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoTimerTimeOut (
    IN P_ADAPTER_T prAdapter
    )
{
    P_THERMO_INFO_T     prThermoInfo;

#if CFG_FAKE_THERMO_VALUE_DBG_EN
#else
    ALC_VAL rCalAlc;
#endif

    DEBUGFUNC("nicThermoTimerTimeOut");

    ASSERT(prAdapter);
    DBGLOG(REQ, TRACE, ("\n"));
    prThermoInfo = &prAdapter->rThermoInfo;

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        if(prAdapter->rRFTestInfo.rSetting.fgALCEn == FALSE) {
            prThermoInfo->fgUpdateTxGainFromAlcInt = FALSE;
            prThermoInfo->fgUpdateLnaFromAlcInt = FALSE;
            return;
        }
    }
    else{

        if(prAdapter->rEEPROMCtrl.fgAlcUseThermoEn == FALSE &&
           prAdapter->rEEPROMCtrl.fgLnaUseThermoEn == FALSE ) {
            prThermoInfo->fgUpdateTxGainFromAlcInt = FALSE;
            prThermoInfo->fgUpdateLnaFromAlcInt = FALSE;
            return;
        }

        /*Tx*/

        if(prAdapter->rEEPROMCtrl.fgAlcUseThermoEn == TRUE ) {

        #if CFG_FAKE_THERMO_VALUE_DBG_EN
            halThermoUpdateTxGain(
                prAdapter,
                prAdapter->rRFInfo.ucChannelNum,
                prAdapter->rRFInfo.eBand,
                prThermoInfo->ucDbgThermoVal++);
            prThermoInfo->ucDbgThermoVal %= BIT(6);
        #else
            halALCRGetCalValue( prAdapter, &rCalAlc);
            halThermoUpdateTxGain(
                 prAdapter,
                 prAdapter->rRFInfo.ucChannelNum,
                 prAdapter->rRFInfo.eBand,
                 rCalAlc);
        #endif

        }
        /*Rx*/
        if( prAdapter->rEEPROMCtrl.fgLnaUseThermoEn == TRUE ) {
            halThermoUpdateRxSetting(
            prAdapter,
            rCalAlc);

        }

    }
    ARB_SET_TIMER(prAdapter,
                  prThermoInfo->rTimer,
                  CFG_PEEK_THERMO_PERIOD_MSEC);
    return;
}

#endif //CFG_THERMO_TIMER_EN

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoIntInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_THERMO_INFO_T     prThermoInfo;
    P_EEPROM_CTRL_T     prEEPROMCtrl;
    //ALC_VAL             rAlcIniVal;

    DEBUGFUNC("nicThermoIntInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prEEPROMCtrl = &prAdapter->rEEPROMCtrl;
    prThermoInfo = &prAdapter->rThermoInfo;

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        prThermoInfo->fgUpdateTxGainFromAlcInt = TRUE;
        prThermoInfo->fgUpdateLnaFromAlcInt = TRUE;
        prThermoInfo->u4AlcArParam = ALCR_AR_PARM_1_OF_4;

    }
    else{

        if (prEEPROMCtrl->fgAlcUseThermoEn == FALSE && \
            prEEPROMCtrl->fgLnaUseThermoEn == FALSE) {
            prThermoInfo->fgUpdateTxGainFromAlcInt = FALSE;
            prThermoInfo->fgUpdateLnaFromAlcInt = FALSE;
            return;
        }
        if (prEEPROMCtrl->fgAlcUseThermoEn == TRUE ) {
            prThermoInfo->fgUpdateTxGainFromAlcInt = TRUE;
        }

        if (prEEPROMCtrl->fgLnaUseThermoEn == TRUE ) {
            prThermoInfo->fgUpdateLnaFromAlcInt= TRUE;
        }
        prThermoInfo->u4AlcArParam = ALCR_AR_PARM_1_OF_32;

    }

    //halALCREnable(prAdapter, prThermoInfo->u4AlcArParam, 0x00, 0x3F, 0x00);
    //halALCRTriggerALC(prAdapter);
    //halALCRGetRawValue(prAdapter, &rAlcIniVal);
    //halALCREnable(prAdapter, prThermoInfo->u4AlcArParam, 0x00, 0x3F, rAlcIniVal);

    //Enable Int mask, default enableed.


    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoIntUnInit (
    IN P_ADAPTER_T prAdapter
    )
{

    DEBUGFUNC("nicThermoIntUnInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    NIC_ALC_DISABLE(prAdapter);

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicProcessALCInterrupt (
    IN P_ADAPTER_T prAdapter
    )
{
    ALC_VAL         rCalAlc;
    ALC_VAL         rMinThreshold;
    ALC_VAL         rMaxThreshold;
    INT_8           cDeltaAlc;
    INT_8           cCurTmp;
    P_EEPROM_CTRL_T prEEPROMCtrl;
    P_THERMO_INFO_T prThermoInfo;

    DEBUGFUNC("nicProcessALCInterrupt");

    ASSERT(prAdapter);

    prEEPROMCtrl = &prAdapter->rEEPROMCtrl;
    prThermoInfo = &prAdapter->rThermoInfo;


    cDeltaAlc = prEEPROMCtrl->ucThermoSensorSlop / 2; /*+10~-10*/

    halALCRGetCalValue(prAdapter, &rCalAlc);
    DBGLOG(INTR, TRACE, ("Calculated ALC = 0x%02X\n"\
            , rCalAlc));

    /*tune CalALC to 25+- n*(10)*/
    HAL_THERMO_2_DEGREE(prEEPROMCtrl, rCalAlc, cCurTmp);

    DBGLOG(INTR, TRACE, ("Current Temperature = %d\n", cCurTmp));
    if(cCurTmp>0) {
        cCurTmp /= 10;
        cCurTmp *= 10;
        cCurTmp += 5;
    }
    else{
        cCurTmp /= 10;
        cCurTmp *= 10;
        cCurTmp -= 5;

    }
    DBGLOG(INTR, TRACE, ("Updated Temperature = %d\n", cCurTmp));
    HAL_DEGREE_2_THERMO(prEEPROMCtrl, cCurTmp, rCalAlc)

    if (IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        //Dealy setting update until new tx/rx start
    }
    else{
        if(prThermoInfo->fgUpdateTxGainFromAlcInt == TRUE) {
            halThermoUpdateTxGain(
         prAdapter,
         prAdapter->rRFInfo.ucChannelNum,
         prAdapter->rRFInfo.eBand,
         rCalAlc);
        }
        if (prThermoInfo->fgUpdateLnaFromAlcInt== TRUE) {
            halThermoUpdateRxSetting(
            prAdapter,
            rCalAlc);

        }
    }

    /*set max min threshold*/
    ALC_THRESHOLD_UPDATE_WITH_LIMIT(rCalAlc, cDeltaAlc, rMaxThreshold);
    ALC_THRESHOLD_UPDATE_WITH_LIMIT(rCalAlc, (-cDeltaAlc), rMinThreshold);

    halALCREnable(prAdapter, prThermoInfo->u4AlcArParam, rMaxThreshold, rMinThreshold, \
            (rCalAlc));
    DBGLOG(INTR, TRACE, ("Updated calculated ALC = 0x%02X, new Max:0x%02X, Min:0x%02X\n"\
            , rCalAlc, rMaxThreshold, rMinThreshold));

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicThermoUnInit (
    IN P_ADAPTER_T prAdapter
    )
{
    P_THERMO_INFO_T     prThermoInfo;

    DEBUGFUNC("nicThermoUnInit");

    ASSERT(prAdapter);

    DBGLOG(INIT, TRACE, ("\n"));

    prThermoInfo = &prAdapter->rThermoInfo;

#if CFG_THERMO_TIMER_EN
    nicThermoTimerUnInit(prAdapter);
#endif

#if CFG_THERMO_INT_EN
    nicThermoIntUnInit(prAdapter);
#endif
    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetGPIO1Mode (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32     u4Scr;

    ASSERT(prAdapter);

    HAL_MCR_RD(prAdapter, MCR_SCR, &u4Scr);
    u4Scr &= ~(SCR_GPIO0_ENABLE_OUTPUT_MODE | SCR_GPIO0_CHAIN_SEL |
               SCR_GPIO1_ENABLE_OUTPUT_MODE | SCR_GPIO1_CHAIN_SEL);/* gpi */
    u4Scr |= SCR_GPIO0_POLAR_HIGH | SCR_GPIO1_POLAR_HIGH; /* Active high */

    if ((prAdapter->rEEPROMCtrl.ucDaisyChain & EEPROM_DAISY_GPIO1_MASK)
        == EEPROM_DAISY_GPIO1_DAISY) {
        u4Scr |= SCR_GPIO1_CHAIN_SEL; /* Input, daisy */
    }

    HAL_MCR_WR(prAdapter, MCR_SCR , u4Scr);
    return;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicSetGPIO2Mode (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_PARAM_GPIO2_MODE_T eGPIO2_Mode
    )
{
    UINT_32     u4Hcr, u4Scr;

    ASSERT(prAdapter);
    ASSERT(eGPIO2_Mode <= GPIO2_MODE_DAISY_INPUT);

    HAL_MCR_RD(prAdapter, MCR_HCR, &u4Hcr);
    HAL_MCR_RD(prAdapter, MCR_SCR, &u4Scr);
    u4Scr &= ~(SCR_GPIO2_ENABLE_OUTPUT_MODE | SCR_GPIO2_CHAIN_SEL);/* gpi */
    u4Scr |=SCR_GPIO2_POLAR_HIGH; /* Active high */
    u4Hcr &= ~HCR_INT_OUT_SELECT;

    switch (eGPIO2_Mode) {
    case GPIO2_MODE_INT:
        /* It is not necessary to set GPIO2 to output mode by MCR_SCR */
        u4Hcr |= HCR_INT_OUT_SELECT;
        break;

    case GPIO2_MODE_DAISY_INPUT:
        u4Scr |= SCR_GPIO2_CHAIN_SEL; /* Input, daisy */
        break;

    default:    /* GPIO2_MODE_EEPROM */
        /* Follow EEPROM configuration about GPIO2 */
        if ((prAdapter->rEEPROMCtrl.ucDaisyChain & EEPROM_DAISY_GPIO2_MASK)
            == EEPROM_DAISY_GPIO2_DAISY) {
            u4Scr |= SCR_GPIO2_CHAIN_SEL; /* Input, daisy */
        }
        break;
    }

    HAL_MCR_WR(prAdapter, MCR_HCR , u4Hcr);
    HAL_MCR_WR(prAdapter, MCR_SCR , u4Scr);
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
nicSetTxFilterForJP (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    )
{
    ASSERT(prAdapter);

    /* Setting the BCCMR1 bit 6 for countery code is JP and at CH 14, bit 6 set 0 others */
    if (!IS_ARB_IN_RFTEST_STATE(prAdapter)) {
        if ((eBand == BAND_24G) && (ucChannelNum == CH_14) &&
            (prAdapter->rConnSettings.u2CountryCode == COUNTRY_CODE_JP)) {
            return TRUE;
        }
    }
    return FALSE;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
nicCompose802_11DataFrame (
    IN  P_ADAPTER_T     prAdapter,
    IN  PUINT_8         pucBuffer,
    IN  UINT_8          ucTid,
    IN  BOOLEAN         fgPrivacy,
    IN  P_NATIVE_PACKET pvWlanBody,
    OUT PUINT_8         pucWlanHeaderLen
    )
{
    P_WLAN_MAC_HEADER_QOS_T     prMacHeader;
    UINT_16                     u2FrameCtrl;
    UINT_16                     u2QosControl = 0;
    P_BSS_INFO_T                prBssInfo;
    PUINT_8                     pucRA;
    UINT_8                      ucOffset;

    UINT_8 aucRfc1042Encap[6] = {
        0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00
    };
    DEBUGFUNC("nicCompose802_11DataFrame");

    ASSERT(prAdapter);
    ASSERT(pucBuffer);
    ASSERT(pvWlanBody);
    ASSERT(pucWlanHeaderLen);

    prBssInfo = &prAdapter->rBssInfo;

    /* Compose the MAC header of the Null frame. */
    prMacHeader =(P_WLAN_MAC_HEADER_QOS_T) pucBuffer;

    /* Notice the header is used for msdu to take this as 802.11 */
    if (prBssInfo->fgIsWmmAssoc) {
        ucOffset = WLAN_MAC_HEADER_QOS_LEN + LLC_LEN - ETH_HLEN;

        /* Copy the total packet before compose header. */
        kalCopyFrame(prAdapter->prGlueInfo,
                     pvWlanBody,
                     pucBuffer + ucOffset);
        kalMemSet(pucBuffer, 0x0, ucOffset - ETHER_TYPE_LEN);

        kalMemCopy(pucBuffer + WLAN_MAC_HEADER_QOS_LEN, aucRfc1042Encap, LLC_LEN - ETHER_TYPE_LEN);
        u2FrameCtrl = (UINT_16) MAC_FRAME_QOS_DATA;

        u2QosControl = (UINT_16)ucTid;

        WLAN_SET_FIELD_16(&prMacHeader->u2QosCtrl, u2QosControl);

        *pucWlanHeaderLen = WLAN_MAC_HEADER_QOS_LEN;

    }
    else {
        ucOffset = WLAN_MAC_HEADER_LEN + LLC_LEN - ETH_HLEN;

        /* Copy the total packet before compose header. */
        kalCopyFrame(prAdapter->prGlueInfo,
                     pvWlanBody,
                     pucBuffer + (ucOffset));
        kalMemSet(pucBuffer, 0x0, ucOffset - ETHER_TYPE_LEN );
        kalMemCopy(pucBuffer + WLAN_MAC_HEADER_LEN, aucRfc1042Encap, LLC_LEN - ETHER_TYPE_LEN);

        /* Fill the Frame Control field. */
        u2FrameCtrl = MAC_FRAME_DATA;

        *pucWlanHeaderLen = sizeof(WLAN_MAC_HEADER_T);
    }

    /* Set the To DS and From DS bits in the Frame Control field. */
    /* Update the Frame Control field by current Operation Mode. */
    u2FrameCtrl |= MASK_FC_TO_DS;

    if (fgPrivacy)
        u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;

    pucRA = &prAdapter->rBssInfo.aucBSSID[0];

    WLAN_SET_FIELD_16(&prMacHeader->u2FrameCtrl, u2FrameCtrl);

    /* Fill the Address 1 field with the Receive Address. */
    COPY_MAC_ADDR(prMacHeader->aucAddr1, pucRA);

    /* Fill the Address 2 field with our own MAC address. */
    COPY_MAC_ADDR(prMacHeader->aucAddr2, prAdapter->aucMacAddress);

    /* Fill the Address 3 field with the destination address. */
    COPY_MAC_ADDR(prMacHeader->aucAddr3, prAdapter->rBssInfo.aucBSSID);

    /* Clear the SEQ/FRAG_NO field(HW won't overide the FRAG_NO, so we need to clear it). */
    prMacHeader->u2SeqCtrl = 0;

    return;
}   /* nicCompose802_11DataFrame */


#if defined(_HIF_SPI)
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void
nicRestoreSpiDefMode (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    HAL_MCR_WR(prAdapter, MCR_SPICSR, SPICSR_8BIT_MODE_DATA);

}
#endif
