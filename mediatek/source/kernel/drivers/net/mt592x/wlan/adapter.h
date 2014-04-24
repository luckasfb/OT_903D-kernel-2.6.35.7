





#ifndef _ADAPTER_H
#define _ADAPTER_H



#define STA_RECORD_GUID                         0xA5A51461
#if DBG || BUILD_QA_DBG
#define STA_RECORD_SET_GUID(_prStaRec)          {(_prStaRec)->u4Magic = STA_RECORD_GUID;}
#define STA_RECORD_CHK_GUID(_prStaRec)          ASSERT((_prStaRec)->u4Magic == STA_RECORD_GUID)
#else
#define STA_RECORD_SET_GUID(_prStaRec)
#define STA_RECORD_CHK_GUID(_prStaRec)
#endif /* DBG || BUILD_QA_DBG */

#define STA_RECORD_HASH_BITS                    2
#define STA_RECORD_HASH_MASK                    BITS(0, (STA_RECORD_HASH_BITS - 1))
#define STA_RECORD_HASH_NUM                     BIT(STA_RECORD_HASH_BITS)

#define STA_RECORD_RM_POLICY_EXCLUDE_STATE_3    BIT(0) // Remove STA Record except the STATE_3 node(also imply the default entry for AdHoc Mode.)
#define STA_RECORD_RM_POLICY_TIMEOUT            BIT(1) // Remove the timeout one
#define STA_RECORD_RM_POLICY_OLDEST             BIT(2) // Remove the oldest one
#define STA_RECORD_RM_POLICY_ENTIRE             BIT(3) // Remove entire STA Record

/* The Lifetime of a Station Record if it is inactive */
#define STA_RECORD_TIMEOUT_SEC                  600 // Seconds


/* 11.3 STA Authentication and Association */
#define STA_STATE_1                             0 /* Accept Class 1 frames */
#define STA_STATE_2                             1 /* Accept Class 1 & 2 frames */
#define STA_STATE_3                             2 /* Accept Class 1,2 & 3 frames */



#if (CFG_DBG_BEACON_RCPI_MEASUREMENT || CFG_LP_PATTERN_SEARCH_SLT)
/* Beacon RCPI debug */
typedef struct _BEACON_RCPI_RECORED_T {
    UINT_32                 u4RcpiIndex;
    UINT_32                 u4RcpiMaxIndex;
    UINT_32                 u4RcpiMinIndex;
    INT_8                   acRcpiArray[MAX_NUM_RCPI_RECORDS];
    //INT_8                   cCurRcpi;        /*!< Current RCPI value (min. -200) */
    INT_8                   cCurAveRcpi;
    UINT_32                 u4BcnLostCnt;
    UINT_32                 u4BcnRecvCnt;    /*including false alarm one*/
    UINT_32                 u4RcpiFalseAlarmCnt;
    INT_8                   cRcpiMin;
    INT_8                   cRcpiMax;
    UINT_32                 u4Range0;
    UINT_32                 u4Range1;
    UINT_32                 u4Range2;
    INT_32                  i4AccuRCPI;
    UINT_32                 u4AccuNF;   /*max 2^32/2^10 packets*/
    ULARGE_INTEGER           rLastTbttCnt;
} BEACON_RCPI_RECORED_T, *P_BEACON_RCPI_RECORED_T;

/* SLT Testing Mode */
typedef enum _ENUM_SLT_MODE_T {
    SLT_MODE_NORMAL = 0,
    SLT_MODE_LP,
    SLT_MODE_PATTERN_SEARCH,
    SLT_MODE_NUM
} ENUM_SLT_MODE_T, *P_ENUM_SLT_MODE_T;

#endif

//3 /* Session for CONNECTION SETTINGS */
typedef enum _ENUM_CONNECTION_POLICY_T {
    CONNECT_BY_SSID = 0,
    CONNECT_BY_SSID_BEST_RSSI,
    CONNECT_BY_SSID_BEST_RSSI_MIN_CH_LOAD,
    CONNECT_BY_SSID_ANY,
    CONNECT_BY_CUSTOMIZED_RULE
} ENUM_CONNECTION_POLICY_T, *P_ENUM_CONNECTION_POLICY_T;


typedef struct _CONNECTION_SETTINGS_T {

    UINT_8                          aucMacAddress[MAC_ADDR_LEN];

    /* power mode */
    PARAM_POWER_MODE                rPwrMode;

    /* ARP filter enable */
    BOOLEAN                         fgArpFilterEn;

    /* ATIM windows using for IBSS power saving function */
    UINT_16                         u2AtimWindow;

    /* b0~3: trigger-en AC0~3. b4~7: delivery-en AC0~3 */
    UINT_8                          bmfgApsdEnAc;

    UINT_8                          aucBSSID[MAC_ADDR_LEN];
    BOOLEAN                         fgIsConnByBssidIssued;

    UINT_8                          ucSSIDLen;
    UINT_8                          aucSSID[ELEM_MAX_LEN_SSID];
    BOOLEAN                         fgIsConnReqIssued;

    BOOLEAN                         fgIsEnableRoaming;

    UINT_16                         u2BeaconPeriod;

    UINT_8                          ucChannelNum;
    UINT_8                          ucChannelBand;
    UINT_32                         u4FreqInKHz; /* TODO */

    ENUM_PARAM_PHY_TYPE_T           eDesiredPhyType;

    ENUM_PARAM_OP_MODE_T            eOPMode;

    ENUM_PARAM_AUTH_MODE_T          eAuthMode;

    ENUM_PARAM_AD_HOC_MODE_T        eAdHocMode;

    ENUM_ENCRYPTION_STATUS_T        eEncStatus;

#if SUPPORT_WAPI /* Set by assoc info */
    UINT_32                         u4WapiSelectedGroupCipher;
    UINT_32                         u4WapiSelectedPairwiseCipher;
    UINT_32                         u4WapiSelectedAKMSuite;
#endif

    /*! \brief MTK WLAN NIC driver IEEE 802.11 MIB */
    IEEE_802_11_MIB_T               rMib;

//    UINT_32                         u4BssRsnSelectedGroupCipher;
//    UINT_32                         u4BssRsnSelectedUnicastCipher;

    ENUM_CONNECTION_POLICY_T        eConnectionPolicy;

    ENUM_PARAM_PREAMBLE_TYPE_T      ePreambleType;

                                    /* User desired setting, but will honor the capability of AP */
    BOOLEAN                         fgIsShortSlotTimeOptionEnable;

    BOOLEAN                         fgIsAdHocQoSEnable;

    UINT_16                         u2DesiredRateSet; /* User desired setting */

    UINT_16                         u2RTSThreshold; /* User desired setting */

#if CFG_TX_FRAGMENT
    UINT_16                         u2FragmentationThreshold; /* User desired setting */

    BOOLEAN                         fgIsEnableTxFragment; /* User desired setting */

    BOOLEAN                         fgIsEnableTxAutoFragmentForBT;

    BOOLEAN                         fgTryTxAutoFragmentForBT; /* User desired setting */
    ENUM_PARAM_BT_COEXIST_WINDOW_T  eBTCoexistWindowType;
#endif /* CFG_TX_FRAGMENT */

    BOOLEAN                         fgIsVoipConn;
    UINT_32                         u4VoipTrafficInterval;  /* 0: disable VOIP configuration */

#if PTA_ENABLED
    PTA_PARAM_T                     rPtaParam;   /* PTA setting */

    /* new board design take two set of PTA parameters */
#if PTA_NEW_BOARD_DESIGN
    PTA_PARAM_T	                    rSingleAclPtaParam;
    PTA_PARAM_T	                    rSingleMixPtaParam;
    PTA_PARAM_T	                    rDualAclPtaParam;
    PTA_PARAM_T	                    rDualMixPtaParam;
#else
    PTA_PARAM_T	                    rScoPtaParam;
    PTA_PARAM_T	                    rAclPtaParam;
    PTA_PARAM_T	                    rMixPtaParam;
#endif
#endif

    UINT_16                         u2CountryCode;

    UINT_8                          ucLedBlinkMode;

#if CFG_SUPPORT_802_11D
    BOOLEAN                         fgMultiDomainCapabilityEnabled;
#endif /* CFG_SUPPORT_802_11D*/

    ENUM_PARAM_GPIO2_MODE_T         eGPIO2_Mode;

    UINT_32                         u4ContPollIntv; // 0: disable poll, else: enable with specific interval

    UINT_32                         u4ContPollIntvB; // 0: disable poll, else: enable with specific interval

    UINT_8                          ucViAifsnBias;      /* Define VI AIFSN bias (Default: 0, 0~15) */
                                                        /* Actual AIFSN = AIFSN from AP + AIFSN bias */
    UINT_16                         u2ViMaxTxopLimit;   /* Define VI MAX TXOP LIMIT(A upper bound) (Default: 0xFFFF, 0~0xFFFF) */

} CONNECTION_SETTINGS_T, *P_CONNECTION_SETTINGS_T;



//3 /* Session for STA RECORD */
struct _STA_RECORD_T {
    LINK_ENTRY_T        rLinkEntry;

    UINT_8              aucMacAddr[MAC_ADDR_LEN];
    UINT_8              ucStaState;
    BOOLEAN             fgIsQoS;
    BOOLEAN             fgIsLegacy;

#if CFG_IBSS_POWER_SAVE
    QUE_T               arStaWaitQueue[TC_NUM];

    /* remote STA power state (used under IBSS power save) */
    BOOLEAN             fgIsAdhocStaAwake;
#endif /* CFG_IBSS_POWER_SAVE */

    UINT_8              ucPowerMgtMode;

    /* Received RCPI from each RX frame */
    RCPI                rRcpi;

    /* Desired Rate Set after match with Operational Rate Set */
    UINT_16             u2DesiredRateSet;

    /* Current Rate1Index get from AutoRate Table */
    UINT_8              ucCurrRate1Index;

    /* Used for updating Rate Field's Preamble Mode of WLAN Table or Auto Rate Table */
    BOOLEAN             fgIsShortPreambleOptionEnable; /* Depend on STA's CAP_INFO */


    /* last rx Sequence Control field */
    UINT_16             u2LastRxSeqCtrl;

    /* last rx Sequence control field for each TID */
    UINT_16             u2TIDLastRxSeqCtrl[TID_NUM];

    UINT_16             u2StatusCode; /* Status of Auth/Assoc Req */
    UINT_16             u2ReasonCode; /* Reason that been Deauth/Disassoc */

    UINT_8              ucJoinFailureCount;

    OS_SYSTIME          rLastJoinTime;

    OS_SYSTIME          rUpdateTime;

    FRAG_INFO_T         rFragInfo[MAX_NUM_CONCURRENT_FRAGMENTED_MSDUS];

#if DBG || BUILD_QA_DBG
    UINT_32             u4Magic;
#endif /* DBG || BUILD_QA_DBG */

};

typedef struct _STA_INFO_T {
    UINT_32             u4StaRecCachedSize;
    PUINT_8             pucStaRecCached;

#if CFG_IBSS_POWER_SAVE
    QUE_T               arGcStaWaitQueue[TC_NUM];

    BOOLEAN             fgIsAllAdhocStaAwake;
#endif /* CFG_IBSS_POWER_SAVE */

    UINT_8              ucValidStaRecNum;
    LINK_T              rFreeStaRecList;
    LINK_T              arValidStaRecList[STA_RECORD_HASH_NUM];
} STA_INFO_T, *P_STA_INFO_T;

#if CFG_SDIO_STATUS_ENHANCE
//3 /* Session for SDIO ENHANCE MODE */
typedef struct _SDIO_CTRL_T {

    UINT_32         u4Hisr;
    UINT_16         au2RxLengthDW[SDIO_MAXIMUM_RX_STATUS];
    TX_STATUS_T     arTxStatus[SDIO_MAXIMUM_TX_STATUS];

    #if DBG
    UINT_8          ucMaxNumOfRxLen;
    UINT_8          ucMaxNumOfTxStatus;
    #endif /* DBG */

} SDIO_CTRL_T, *P_SDIO_CTRL_T;
#endif /* CFG_SDIO_STATUS_ENHANCE */

//3 /* Session for Thermo function */
typedef struct _THERMO_INFO_T {

        TIMER_T                 rTimer;
#if CFG_FAKE_THERMO_VALUE_DBG_EN
        UINT_8                  ucDbgThermoVal;
#endif
        UINT_8                  ucHwAlcValue;
        BOOL                    fgUpdateTxGainFromAlcInt;
        BOOL                    fgUpdateLnaFromAlcInt;
        UINT_32                 u4AlcArParam;
        ENUM_THERMO_STATE_T     rState;
} THERMO_INFO_T, *P_THERMO_INFO_T;



//3 /* BSS Descriptor structure */
struct _BSS_DESC_T {
    LINK_ENTRY_T            rLinkEntry;

    UINT_8                  aucBSSID[MAC_ADDR_LEN];
    UINT_8                  aucSrcAddr[MAC_ADDR_LEN]; /* For IBSS, the SrcAddr is different from BSSID */

    BOOLEAN                 fgIsConnecting; /* If we are going to connect to this BSS
                                             * (JOIN or ROAMING to another BSS), don't
                                             * remove this record from BSS List.
                                             */
    BOOLEAN                 fgIsConnected; /* If we have connected to this BSS (NORMAL_TR),
                                            * don't removed this record from BSS list.
                                            */

    BOOLEAN                 fgIsHiddenSSID; /* When this flag is TRUE, means the SSID
                                             * of this BSS is not known yet.
                                             */
    UINT_8                  ucSSIDLen;
    UINT_8                  aucSSID[ELEM_MAX_LEN_SSID];

    OS_SYSTIME              rUpdateTime;

    ENUM_BSS_TYPE_T         eBSSType;

    ENUM_PHY_TYPE_INDEX_T   ePhyType; /* PHY Type of this BSS_DESC_T */

    RCPI                    rRcpi;


    UINT_8                  ucWmmFlag; /* A flag to indicate this BSS's WMM capability */

    BOOLEAN                 fgPreAuthEnabled;

    BOOLEAN                 fgIsERPPresent;
    UINT_8                  ucERP;

    BOOLEAN                 fgIsLargerTSF; /* This BSS's TimeStamp is larger than us(TCL == 1 in RX_STATUS_T) */

    BOOLEAN                 fgIsUnknownBssBasicRate; /* Have unknown data rate which is not list in aucDataRate[] */

    UINT_16                 u2OperationalRateSet;
    UINT_16                 u2BSSBasicRateSet;

    UINT_8                  ucDTIMPeriod;

    UINT_8                  ucSupportedRatesLen;
    UINT_8                  aucSupportedRates[RATE_NUM];

    UINT_16                 u2CapInfo;
    UINT_16                 u2BeaconInterval;
    UINT_16                 u2ATIMWindow;

    P_WPA_INFO_ELEM_T       prIEWPA;
    P_RSN_INFO_ELEM_T       prIERSN;
#if SUPPORT_WAPI
    P_WAPI_INFO_ELEM_T	    prIEWAPI;
#endif

    P_IE_WMM_PARAM_ELEM_T   prIeWmmParamElem;

#if CFG_SUPPORT_802_11D
    P_IE_COUNTRY_T          prIECountry;
#endif

#if 0 // NOTE(Kevin): Useless for reporting SCAN Result, remove later.
    P_IE_COUNTRY_T                  prIECountry;
    P_IE_BSS_LOAD_T                 prIEBSSLoad;
    P_IE_POWER_CONSTRAINT_T         prIEPowerConstraint;
    P_IE_TPC_REPORT_T               prIETPCReport;
    P_IE_CHNL_SWITCH_ANNOUNCEMENT_T prIEChnlSwitchAnnouncement;
    P_IE_QUIET_T                    prIEQuiet;
    P_IE_IBSS_DFS_T                 prIEIBSSDFS;
#endif

    UINT_16                 u2IELength;

    UINT_8                  aucIEBuf[CFG_IE_BUFFER_SIZE];

//    UINT_16             u2WPAIEOffset;
//    UINT_16             u2RSNIEOffset;
//    UINT_16             u2WMMIEOffset;


    ULARGE_INTEGER          u8TimeStamp;

    ENUM_BAND_T             eBand;

    UINT_8                  ucChannelNum;


//    INT_32              rssi;
//    UINT_32             linkQuality;
//    UINT_16             lastMgtRespStatusCode;
//    UINT_16             requestRefusedStatusCode;
//    UINT                joinTimeoutCount;


    /*! \brief RSN parameters selected for connection */
    /*! \brief The Select score for final AP selection,
               0, no sec, 1,2,3 group cipher is WEP, TKIP, CCMP */
    UINT_8                  ucEncLevel;
    /*! \brief The srbiter Search State will matched the scan result,
               and saved the selected cipher and akm, and report the score,
               for arbiter join state, join module will carry this target BSS
               to rsn generate ie function, for gen wpa/rsn ie */
    UINT_32                 u4RsnSelectedGroupCipher;
    UINT_32                 u4RsnSelectedPairwiseCipher;
    UINT_32                 u4RsnSelectedAKMSuite;

    BOOLEAN                 fgWpaCapPresent;
    UINT_16                 u2WpaCap;
    UINT_16                 u2RsnCap;

#if DBG || BUILD_QA_DBG
    UINT_32                 u4Magic;
#endif /* DBG || BUILD_QA_DBG */

};


//3 /* Peer BSS' Information structure which we are going to JOIN with or MERGE into */
struct _PEER_BSS_INFO_T {

    UINT_8              aucBSSID[MAC_ADDR_LEN];


    UINT_16             u2AssocId;

    UINT_16             u2OperationalRateSet; /* Operational Rate Set of peer BSS */
    UINT_16             u2BSSBasicRateSet; /* Basic Rate Set of peer BSS */
    BOOLEAN             fgIsUnknownBSSBasicRate;

    BOOLEAN             fgIsShortPreambleAllowed; /* From Capability Info. */

    BOOLEAN             fgUseShortSlotTime; /* From Capability Info. */

    BOOLEAN             fgIsPrivacyEnabled; /* From Capability Info. */

    /* Save the Challenge Text from Auth Seq 2 Frame, before sending Auth Seq 3 Frame */
    IE_CHALLENGE_TEXT_T rIeChallengeText;

    WMM_INFO_T          rWmmInfo; /* Current WMM capability after associated with target AP */

};




//3 /* Current BSS' Information structure which we belong in this moment */
struct _BSS_INFO_T {

    ENUM_PHY_TYPE_INDEX_T   ePhyType;

    ENUM_BSS_TYPE_T     eBSSType;

    UINT_8              aucBSSID[MAC_ADDR_LEN];

    BOOLEAN             fgIsWmmAssoc; /* This flag is used to indicate that WMM is enable in current BSS */

    UINT_8              ucSSIDLen;
    UINT_8              aucSSID[ELEM_MAX_LEN_SSID];

    UINT_16             u2CapInfo;
    UINT_16             u2AssocId;

    BOOLEAN             fgIsERPPresent; /* This flag is used to indicate that we need sync the ERP with AP */
    UINT_8              ucERP;
    BOOLEAN             fgIsProtection;



    UINT_8              ucDtimPeriod;

    UINT_16             u2OperationalRateSet; /* Operational Rate Set of current BSS */
    UINT_16             u2BSSBasicRateSet; /* Basic Rate Set of current BSS */

    UINT_8              ucBasicRateIndex;

    UINT_8              ucRTSProtectionRateIndex; /* This is also the Rate Index of CTS_to_self */
    UINT_8              ucRTSRateIndex; /* This is also the Rate Index of CTS_to_self */

    BOOLEAN             fgIsShortPreambleAllowed; /* From Capability Info. of AssocResp Frame AND of Beacon/ProbeResp Frame */

    BOOLEAN             fgUseShortPreamble; /* Short Preamble is enabled in current BSS. */

    BOOLEAN             fgUseShortSlotTime; /* Short Slot Time is enabled in current BSS. */

    BOOLEAN             fgIsWPAorWPA2Enabled; /* WPA/WPA2 is enabled in current BSS. */

    /*! \brief RSN saved cipher for oid add key after associate and key handshake  */
    UINT_32             u4RsnSelectedGroupCipher;
    UINT_32             u4RsnSelectedPairwiseCipher;
    UINT_32             u4RsnSelectedAKMSuite;

    RCPI                rRcpi;
    PARAM_RSSI          rRssi;
    OS_SYSTIME          rRssiLastUpdateTime; /* The last time when update RSSI */


    //4 2007/10/01, mikewu, add for compose beacon
    UINT_16             u2ATIMWindow;
    UINT_8              ucChnl;
    ENUM_BAND_T         eBand;
    UINT_16             u2BeaconInterval;

    UINT_16             u2BcnLen;
    UINT_8              aucBcnContent[CFG_IE_BUFFER_SIZE] __KAL_ATTRIB_ALIGN_4__;
    //4 End by Mike

#if CFG_TX_FRAGMENT

    BOOLEAN             fgIsPrivacyEnable;
    BOOLEAN             fgRequireMICForFrag; /* Depend on TKIP Cipher Key was set */
    UINT_8              ucWlanDataFrameHeaderLen; /* Depend on WMM association */
    UINT_16             u2PayloadFragmentationThreshold; /* Current Fragmentation Threshold for payload which has
                                                              */
#endif /* CFG_TX_FRAGMENT */

    WMM_INFO_T          rWmmInfo;

    OS_SYSTIME          rRoamingStableExpirationTime;
    UINT_8              ucRoamingAuthTypes;

#if CFG_SUPPORT_802_11D
    BOOLEAN             fgIsCountryInfoPresent;
    DOMAIN_INFO_ENTRY   rDomainInfo;
#endif

};



//3 /* Major ADAPTER structure */
/* Major data structure for driver operation */
struct _ADAPTER_T {
    UINT_8                  ucRevID;

    RF_CHANNEL_INFO_T       arNicOpChnList[MAXIMUM_OPERATION_CHANNEL_LIST]; /*!< operation channel list for normal operation */
    UINT_16                 u2NicOpChnlNum;

    DOMAIN_INFO_ENTRY       rNicOpDomainInfo;

    LINK_T                  rTimerList;         /*!< Timer List for mgmt_timer.c */
    OS_SYSTIME              rNextExpiredSysTime;

    PM_CONN_SETUP_INFO_T    rPmConnSetupInfo;
//    BOOLEAN                 fgPowerControlAcquired;
    /* Driver feature control */

    BOOLEAN                 fgIsEnableWMM;
    //BOOLEAN                 fgIsEnableRoaming; /* Move to rConnSettings */
    BOOLEAN                 fgIsEnableRoamingGsm;
    BOOLEAN                 fgIsEnableJoinToHiddenSSID;

    UINT_32                 u4OsPacketFilter;     // packet filter used by OS

#if CFG_TCP_IP_CHKSUM_OFFLOAD
    UINT_32                 u4CSUMFlags;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */


    /* ADAPTER flags */
    UINT_32                 u4Flags;

    ARB_INFO_T              rArbInfo;

    JOIN_INFO_T             rJoinInfo;

    ROAMING_INFO_T          rRoamingInfo;

    SEC_INFO_T              rSecInfo;
#if PTA_ENABLED
    PTA_INFO_T              rPtaInfo;
#endif
    TIMER_INFO_T            rTimerInfo;

    MGT_BUF_INFO_T          rMgtBufInfo;

    WLAN_ENTRY_CTRL_T       arWlanCtrl[WLAN_TABLE_SIZE];

    /*! \brief Global flag to let arbiter stay at standby and not connect to any network */
    BOOLEAN                 fgCounterMeasure;

    BOOLEAN                 fgIsRadioOff;

    BOOLEAN                 fgIsEnterD3ReqIssued;

    ENUM_ACPI_STATE_T       rAcpiState;

#if !CFG_SUPPORT_SSID_RECOVER_STATE
    BOOLEAN fgRemoveBGScan;
#endif

    CONNECTION_SETTINGS_T   rConnSettings;

    /* Element for SCAN(BSS description) management */
    SCAN_INFO_T             rScanInfo;

    /* Element for power management */
    PM_INFO_T               rPmInfo;

    /* Element for STA record management */
    STA_INFO_T              rStaInfo;

    RF_INFO_T               rRFInfo;

    /* RF Test mode*/
    RFTEST_INFO_T           rRFTestInfo;

    STATISTICS_INFO_T       rStatisticsInfo;


    /* Temporary data collected from Peer BSS while doing JOIN proccess, such as Challenge Text */
    PEER_BSS_INFO_T         rPeerBssInfo;

    /* Current associated BSS or created / merged IBSS */
    BSS_INFO_T              rBssInfo;


    ENUM_OP_MODE_T          eCurrentOPMode;

    /* NOTE(Kevin): for windows, we may delay the Disconnect Event for 10 sec.
     * During this period, the eConnectionState == MEDIA_STATE_DISCONNECTED.
     * But the eConnectionStateIndicated == MEDIA_STATE_CONNECTED.
     */
    ENUM_MEDIA_STATE        eConnectionState;
    ENUM_MEDIA_STATE        eConnectionStateIndicated; /* The Media State that report to OS */

    BOOLEAN                 fgBypassPortCtrlForRoaming;
    /* NOTE(Kevin): Once we've create a new IBSS or merge with other IBSS, following
     * flag will be set to TRUE regardless of Connection State.
     */
    BOOLEAN                 fgIsIBSSActive;
    BOOLEAN                 fgIsIBSSBeaconTimeoutAlarm;
    OS_SYSTIME              rIBSSLastBeaconTimeout;


    UINT_8                  aucMacAddress[MAC_ADDR_LEN];
    UINT_8                  aucMulticastAddr[MAX_NUM_GROUP_ADDR][MAC_ADDR_LEN];
    UINT_8                  ucMulticastAddrNum;


    /* Driver default value according to HW capability */
    UINT_16                 u2SupportedPhyTypeSet; /* Supported PHY Types - HW dependent */

    UINT_16                 u2AvailablePhyTypeSet; /* Available PHY Types - HW dependent & User desired */

    ENUM_PHY_TYPE_INDEX_T   eCurrentPhyType; /* Current selection basing on the set of Available PHY Types */


    /* RF frequency set, only availabe at MT60105 ASIC  */
    ENUM_RF_FREQ_T          eRfFreqSet;


    PARAM_RSSI              rRssiTriggerValue;

#if CFG_COALESCING_BUFFER_SIZE
    UINT_32                 u4CoalescingBufCachedSize;
    PUINT_8                 pucCoalescingBufCached;
#endif /* CFG_COALESCING_BUFFER_SIZE */

#if CFG_FRAGMENT_COALESCING_BUFFER_SIZE
    UINT_32                 u4FragCoalescingBufCachedSize;
    PUINT_8                 pucFragCoalescingBufCached;
#endif /* CFG_FRAGMENT_COALESCING_BUFFER_SIZE */


    /* Element for TX PATH */
    TX_CTRL_T               rTxCtrl;

    /* Element for RX PATH */
    RX_CTRL_T               rRxCtrl;

#if CFG_SDIO_STATUS_ENHANCE || CFG_SDIO_TX_ENHANCE
    SDIO_CTRL_T             rSDIOCtrl;
#endif /* CFG_SDIO_STATUS_ENHANCE || CFG_SDIO_TX_ENHANCE */

    EEPROM_CTRL_T           rEEPROMCtrl;

    /* Buffer for Authentication Event */
    /* <Todo> Move to glue layer and refine the kal function */
    /* Reference to rsnGeneratePmkidIndication function at rsn.c */
    UINT_8                  aucIndicationEventBuffer[CFG_MAX_PMKID_CACHE * 20  + 8 ];

    //4 2007/06/20, mikewu, interrupt status
    UINT_32                 au4IntStatus[INT_NUM];
    UINT_32                 au4IntEnable[INT_NUM];
    BOOLEAN                 fgIsIntEnable;
    BOOLEAN                 fgIsIntEnableWithLPOwnSet;

#if DBG
    BOOLEAN                 fgIntrMayFalseAlarmFlag;
#endif
    BOOLEAN                 fgRedoProcessISTFlag;

    BOOLEAN                 fgEnableAverageRcpiFunc;

    //4 2007/09/20, mikewu, Autorate-related variables
    AR_CTRL_T               rARCtrl;

    RATE_INFO_T             rRateInfo;

    PARAM_CUSTOM_MCR_RW_STRUC_T rMcrRdWrInfo;

    P_GLUE_INFO_T           prGlueInfo;

#if SUPPORT_WPS
    BOOLEAN                 fgIndMgt;
    UINT_8                  aucAppIE[IEEE80211_APPIE_NUM_OF_FRAME][300];
    UINT_8                  aucAppIELen[IEEE80211_APPIE_NUM_OF_FRAME];
#endif


    OS_SYSTIME              rLastRxUcDataSysTime;
    OS_SYSTIME              rLastConnectedTime;

    BOOLEAN                 fgBeaconReceivedAfterConnected;
    BOOLEAN                 fgIsBlockToScan;


    THERMO_INFO_T           rThermoInfo;

#if CFG_PEEK_RCPI_VALUE_PERIOD_SEC
    TIMER_T                 rRcpiDiagnostic;
#endif /* CFG_PEEK_RCPI_VALUE_PERIOD_SEC */

#if (CFG_DBG_BEACON_RCPI_MEASUREMENT || CFG_LP_PATTERN_SEARCH_SLT)
    /* Beacon RCPI debug */
    BEACON_RCPI_RECORED_T   rBcnRCPIRecord;

    /* SLT Mode Selection */
    ENUM_SLT_MODE_T eSLTModeSel;

    BOOLEAN                 fgPatternSearchMatch;
#endif

    BOOLEAN                 fgWaitOneBeaconForTsfRecovery;

#if SUPPORT_WAPI
    BOOLEAN                 fgUseWapi; /* TRUE, The registry to set the driver init as wapi driver */
    BOOLEAN                 fgWapiMode; /* At WAI, the open set this value 0 and wapi mode set 1,
                                           and the oid encryption status is always ENUM_ENCRYPTION_DISABLED 
                                           needed this value to identify */ 
#if SUPPORT_WPI_AVOID_LOCAL_BUFFER
    UINT_8                  ucTemp[1600];
#endif
#endif
#if CFG_TEST_IO_PIN
    /* IO Pin Test */
    BOOLEAN fgIntVarified;
    UINT_32 u4IntIORslt;
#endif
};/* end of _ADAPTER_T */



#define STA_RECORD_HASH_FUNC(pucAddr)   \
    ((pucAddr[MAC_ADDR_LEN - 2]) & STA_RECORD_HASH_MASK)

#define STA_RECORD_SET_EXPIRATION_TIME(prStaRec, u4TimeOutSecond) \
    SET_EXPIRATION_TIME((prStaRec)->rExpirationTime, \
        SEC_TO_SYSTIME(u4TimeOutSecond))

#define RCPI_TO_DBM(_rcpi)    ((INT_8)((_rcpi)/2 - 110))

#define ABS(_x)  ( (_x)>0 ? (_x):-(_x))



/*----------------------------------------------------------------------------*/
/* Routines in sta_record.c                                                   */
/*----------------------------------------------------------------------------*/
VOID
staRecInitialize (
    IN P_ADAPTER_T prAdapter
    );

P_STA_RECORD_T
staRecGetStaRecordByAddr (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucStaAddr
    );

P_STA_RECORD_T
staRecGetStaRecordByAddrOrAddIfNotExist (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucStaAddr
    );

VOID
staRecRemoveStaRecordByPolicy (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4RemovePolicy
    );

VOID
staRecRemoveStaRecordForIBSS (
    IN P_ADAPTER_T prAdapter
    );

VOID
staRecRemoveStateFlagOfAllStaRecords (
    IN P_ADAPTER_T prAdapter
    );

VOID
staRecClearStatusAndReasonCode (
    IN P_ADAPTER_T prAdapter
    );

VOID
staRecCheckDefragBufOfAllStaRecords (
    IN P_ADAPTER_T prAdapter
    );

#if CFG_DBG_STA_RECORD
VOID
staRecQueryStatus (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_8 pucBuffer,
    OUT PUINT_32 pu4Count
    );
#endif /* CFG_DBG_STA_RECORD */

__KAL_INLINE__ VOID
adapterDataTypeCheck (
    VOID
    )
{
#if CFG_SDIO_STATUS_ENHANCE
    #if DBG
    DATA_STRUC_INSPECTING_ASSERT(OFFSET_OF(SDIO_CTRL_T, ucMaxNumOfRxLen)
        == sizeof(ISAR_BIND_STATUS_T));
    #else
    DATA_STRUC_INSPECTING_ASSERT(sizeof(SDIO_CTRL_T)
        == sizeof(ISAR_BIND_STATUS_T));
    #endif /* DBG */
#endif /* CFG_SDIO_STATUS_ENHANCE */

    return;
}

#endif /* _ADAPTER_H */


