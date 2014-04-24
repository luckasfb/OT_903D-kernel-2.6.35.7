







#include "gl_os.h"

#include "config.h"
#include "wlan_oid.h"

#include "gl_wext.h"
#include "gl_wext_priv.h"

#if SUPPORT_WAPI
#include "gl_sec.h"
#endif

/* compatibility to wireless extensions */
#ifdef WIRELESS_EXT

const long channel_freq[] = {
        2412, 2417, 2422, 2427, 2432, 2437, 2442,
        2447, 2452, 2457, 2462, 2467, 2472, 2484
};

#define NUM_CHANNELS (sizeof(channel_freq) / sizeof(channel_freq[0]))



/* NOTE: name in iwpriv_args only have 16 bytes */
static const struct iw_priv_args rIwPrivTable[] = {
    {IOCTL_SET_INT,             IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   ""},
    {IOCTL_GET_INT,             0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   ""},
    {IOCTL_SET_INT,             IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0,   ""},
    {IOCTL_GET_INT,             0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,   ""},
    /* added for set_oid and get_oid */
    {IOCTL_SET_STRUCT,          256,                                    0, ""},
    {IOCTL_GET_STRUCT,          0,                                      256, ""},
#if PTA_ENABLED
    {IOCTL_SET_STRUCT,          IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 6, 0,  ""},
#endif

    /* sub-ioctl definitions */
    {PRIV_CMD_REG_DOMAIN,       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_reg_domain" },
    {PRIV_CMD_REG_DOMAIN,       0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_reg_domain" },

    {PRIV_CMD_BEACON_PERIOD,    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_bcn_period" },
    {PRIV_CMD_BEACON_PERIOD,    0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_bcn_period" },

    {PRIV_CMD_ADHOC_MODE,       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_adhoc_mode" },
    {PRIV_CMD_ADHOC_MODE,       0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_adhoc_mode" },

#if CFG_TCP_IP_CHKSUM_OFFLOAD
    {PRIV_CMD_CSUM_OFFLOAD,     IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_tcp_csum" },
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

    {PRIV_CMD_ROAMING,          IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_roaming" },
    {PRIV_CMD_ROAMING,          0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_roaming" },

    {PRIV_CMD_VOIP_DELAY,       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_voip_delay" },
    {PRIV_CMD_VOIP_DELAY,       0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_voip_delay" },

    {PRIV_CMD_POWER_MODE,       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_power_mode" },
    {PRIV_CMD_POWER_MODE,       0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_power_mode" },

    {PRIV_CMD_WMM_PS,           IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0,   "set_wmm_ps" },
#if PTA_ENABLED
    {PRIV_CMD_BT_COEXIST,       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 6, 0,   "set_bt_coexist" },
#endif
    {PRIV_GPIO2_MODE,           IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_gpio2_mode" },

    {PRIV_CUSTOM_SET_PTA,       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_pta" },

    {PRIV_CUSTOM_SINGLE_ANTENNA,0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,   "get_single_ant" },

    {PRIV_CUSTOM_CONTINUOUS_POLL, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "set_poll" },

    {PRIV_CUSTOM_DISABLE_BEACON_DETECTION, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,   "down_bcn_lost" },

    /* SET STRUCT sub-ioctls commands */
    {PRIV_CMD_OID, 256, 0, "set_oid"},
    /* GET STRUCT sub-ioctls commands */
    {PRIV_CMD_OID, 0, 256, "get_oid"}
};

static const iw_handler rIwPrivHandler[] = {
    [IOCTL_SET_INT - SIOCIWFIRSTPRIV] = priv_set_int,
    [IOCTL_GET_INT - SIOCIWFIRSTPRIV] = priv_get_int,
    [IOCTL_SET_ADDRESS - SIOCIWFIRSTPRIV] = NULL,
    [IOCTL_GET_ADDRESS - SIOCIWFIRSTPRIV] = NULL,
    [IOCTL_SET_STR - SIOCIWFIRSTPRIV] = NULL,
    [IOCTL_GET_STR - SIOCIWFIRSTPRIV] = NULL,
    [IOCTL_SET_KEY - SIOCIWFIRSTPRIV] = NULL,
    [IOCTL_GET_KEY - SIOCIWFIRSTPRIV] = NULL,
    [IOCTL_SET_STRUCT - SIOCIWFIRSTPRIV] = priv_set_struct,
    [IOCTL_GET_STRUCT - SIOCIWFIRSTPRIV] = priv_get_struct,
    [IOCTL_SET_STRUCT_FOR_EM - SIOCIWFIRSTPRIV] = priv_set_struct,
};

const struct iw_handler_def wext_handler_def = {
    .num_standard   = 0,
    .num_private = (__u16)sizeof(rIwPrivHandler)/sizeof(iw_handler),
    .num_private_args = (__u16)sizeof(rIwPrivTable)/sizeof(struct iw_priv_args),
    .standard   = (iw_handler *)NULL,
    .private = rIwPrivHandler,
    .private_args = rIwPrivTable,
    .get_wireless_stats = wext_get_wireless_stats,
};





/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static BOOLEAN
wextSrchDesiredWPAIE (
    IN  PUINT_8         pucIEStart,
    IN  INT_32          i4TotalIeLen,
    IN  UINT_8          ucDesiredElemId,
    OUT PUINT_8         *ppucDesiredIE
    )
{
    INT_32 i4InfoElemLen;

    ASSERT(pucIEStart);
    ASSERT(ppucDesiredIE);

    while (i4TotalIeLen >= 2) {
        i4InfoElemLen = (INT_32) pucIEStart[1] + 2;

        if (pucIEStart[0] == ucDesiredElemId && i4InfoElemLen <= i4TotalIeLen) {
            if (ucDesiredElemId != 0xDD) {
                /* Non 0xDD, OK! */
                *ppucDesiredIE = &pucIEStart[0];
                return TRUE;
            }
            else {
                /* EID == 0xDD, check WPA IE */
                if (pucIEStart[1] >= 4) {
                    if (memcmp(&pucIEStart[2], "\x00\x50\xf2\x01", 4) == 0) {
                        *ppucDesiredIE = &pucIEStart[0];
                        return TRUE;
                    }
                } /* check WPA IE length */
            } /* check EID == 0xDD */
        } /* check desired EID */

        /* Select next information element. */
        i4TotalIeLen -= i4InfoElemLen;
        pucIEStart += i4InfoElemLen;
    }

    return FALSE;
} /* parseSearchDesiredWPAIE */


#if SUPPORT_WPS
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static BOOLEAN
wextSrchDesiredWPSIE (
    IN PUINT_8 pucIEStart,
    IN INT_32 i4TotalIeLen,
    IN UINT_8 ucDesiredElemId,
    OUT PUINT_8 *ppucDesiredIE)
{
    INT_32 i4InfoElemLen;

    ASSERT(pucIEStart);
    ASSERT(ppucDesiredIE);

    while (i4TotalIeLen >= 2) {
        i4InfoElemLen = (INT_32) pucIEStart[1] + 2;

        if (pucIEStart[0] == ucDesiredElemId && i4InfoElemLen <= i4TotalIeLen) {
            if (ucDesiredElemId != 0xDD) {
                /* Non 0xDD, OK! */
                *ppucDesiredIE = &pucIEStart[0];
                return TRUE;
            }
            else {
                /* EID == 0xDD, check WPS IE */
                if (pucIEStart[1] >= 4) {
                    if (memcmp(&pucIEStart[2], "\x00\x50\xf2\x04", 4) == 0) {
                        *ppucDesiredIE = &pucIEStart[0];
                        return TRUE;
                    }
                } /* check WPS IE length */
            } /* check EID == 0xDD */
        } /* check desired EID */

        /* Select next information element. */
        i4TotalIeLen -= i4InfoElemLen;
        pucIEStart += i4InfoElemLen;
    }

    return FALSE;
} /* parseSearchDesiredWPSIE */
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_name (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT char *pcName,
    IN char *pcExtra
    )
{
    ENUM_PARAM_NETWORK_TYPE_T  eNetWorkType;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(pcName);
    if (FALSE == GLUE_CHK_PR2(prNetDev, pcName)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (netif_carrier_ok(prNetDev)) {
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidQueryNetworkTypeInUse,
            &eNetWorkType,
            sizeof(eNetWorkType),
            TRUE,
            TRUE,
            &u4BufLen);
#else
        rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
            wlanoidQueryNetworkTypeInUse,
            &eNetWorkType,
            sizeof(eNetWorkType),
            &u4BufLen);
#endif
        /* strcpy(pucName, "IEEE 802.11bg"); */
        sprintf(pcName,
            "IEEE 802.11%c",
            (eNetWorkType == NETWORK_TYPE_OFDM24) ? 'g' : 'b');
    }
    else {
        strcpy(pcName, "Disconnected");
    }

    return (WLAN_STATUS_SUCCESS == rStatus) ? 0 : -EFAULT;
} /* wext_get_name */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_freq (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwReqInfo,
    IN struct iw_freq *prIwFreq,
    IN char *pcExtra
    )
{
    UINT_32 u4ChnlFreq; /* Store channel or frequency information */

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prIwFreq);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prIwFreq)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /*
    printk("set m:%d, e:%d, i:%d, flags:%d\n",
        prIwFreq->m, prIwFreq->e, prIwFreq->i, prIwFreq->flags);
    */

    /* If setting by frequency, convert to a channel */
    if ((prIwFreq->e == 1) &&
        (prIwFreq->m >= (int) 2.412e8) &&
        (prIwFreq->m <= (int) 2.484e8)) {

        /* Change to KHz format */
        u4ChnlFreq = (UINT_32)(prIwFreq->m / (1000 / 10));
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
                           wlanoidSetFrequency,
                           &u4ChnlFreq,
                           sizeof(u4ChnlFreq),
                           FALSE,
                           TRUE,
                           &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                           wlanoidSetFrequency,
                           &u4ChnlFreq,
                           sizeof(u4ChnlFreq),
                           &u4BufLen);
#endif

        if (WLAN_STATUS_SUCCESS != rStatus) {
            return -EINVAL;
        }
    }
    /* Setting by channel number */
    else if ((prIwFreq->m > 1000) || (prIwFreq->e > 0)) {
        return -EOPNOTSUPP;
    }
    else {
        /* Change to channel number format */
        u4ChnlFreq = (UINT_32)prIwFreq->m;

#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidSetChannel,
            &u4ChnlFreq,
            sizeof(u4ChnlFreq),
            FALSE,
            TRUE,
            &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
            wlanoidSetChannel,
            &u4ChnlFreq,
            sizeof(u4ChnlFreq),
            &u4BufLen);
#endif

        if (WLAN_STATUS_SUCCESS != rStatus) {
            return -EINVAL;
        }
    }

    return 0;

} /* wext_set_freq */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_freq (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_freq *prIwFreq,
    IN char *pcExtra
    )
{
    UINT_32 u4Channel = 0;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prIwFreq);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prIwFreq)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /* GeorgeKuo: TODO skip checking in IBSS mode */
    if (!netif_carrier_ok(prNetDev)) {
        return -ENOTCONN;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryFrequency,
        &u4Channel,
        sizeof(u4Channel),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryFrequency,
        &u4Channel,
        sizeof(u4Channel),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    prIwFreq->m = (int)u4Channel; /* freq in KHz */
    prIwFreq->e = 3;

    return 0;

} /* wext_get_freq */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_mode (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwReqInfo,
    IN unsigned int *pu4Mode,
    IN char *pcExtra
    )
{
    ENUM_PARAM_OP_MODE_T eOpMode;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(pu4Mode);
    if (FALSE == GLUE_CHK_PR2(prNetDev, pu4Mode)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    switch (*pu4Mode) {
    case IW_MODE_AUTO:
        eOpMode = NET_TYPE_AUTO_SWITCH;
        break;

    case IW_MODE_ADHOC:
        eOpMode = NET_TYPE_IBSS;
        break;

    case IW_MODE_INFRA:
        eOpMode = NET_TYPE_INFRA;
        break;

    default:
        printk(KERN_DEBUG "%s(): Set UNSUPPORTED Mode = %d.\n", __FUNCTION__, *pu4Mode);
        return -EOPNOTSUPP;
    }

    /*
    printk("%s(): Set Mode = %d\n", __FUNCTION__, *pu4Mode);
    */

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSetInfrastructureMode,
        &eOpMode,
        sizeof(eOpMode),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
        wlanoidSetInfrastructureMode,
        &eOpMode,
        sizeof(eOpMode),
        &u4BufLen);
#endif
    /* after set operation mode, key table are cleared */

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    /* reset wpa info */
    prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
    prGlueInfo->rWpaInfo.u4KeyMgmt = 0;
    prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_NONE;

    /*Don't reset it open*/
    //prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;

    return 0;
} /* wext_set_mode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_mode (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwReqInfo,
    OUT unsigned int *pu4Mode,
    IN char *pcExtra
    )
{
    ENUM_PARAM_OP_MODE_T eOpMode;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(pu4Mode);
    if (FALSE == GLUE_CHK_PR2(prNetDev, pu4Mode)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryInfrastructureMode,
        &eOpMode,
        sizeof(eOpMode),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryInfrastructureMode,
        &eOpMode,
        sizeof(eOpMode),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    switch (eOpMode){
    case NET_TYPE_IBSS:
        *pu4Mode = IW_MODE_ADHOC;
        break;

    case NET_TYPE_INFRA:
        *pu4Mode = IW_MODE_INFRA;
        break;

    case NET_TYPE_AUTO_SWITCH:
        *pu4Mode = IW_MODE_AUTO;
        break;

    default:
        printk(KERN_DEBUG "%s(): Get UNKNOWN Mode.\n", __FUNCTION__);
        return -EINVAL;
    }

    return 0;
} /* wext_get_mode */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_range (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_point *prData,
    OUT char *pcExtra
    )
{
    struct iw_range *prRange = NULL;
    PARAM_RATES_EX aucSuppRate = {0}; /* data buffers */
    int i = 0;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(pcExtra);
    if (FALSE == GLUE_CHK_PR2(prNetDev, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    prRange = (struct iw_range *)pcExtra;

    memset(prRange, 0, sizeof(*prRange));
    prRange->throughput = 20000000;  /* 20Mbps */
    prRange->min_nwid = 0;   /* not used */
    prRange->max_nwid = 0;   /* not used */

    /* scan_capa not implemented */

    /* event_capa[6]: kernel + driver capabilities */
    prRange->event_capa[0] = (IW_EVENT_CAPA_K_0
        | IW_EVENT_CAPA_MASK(SIOCGIWAP)
        | IW_EVENT_CAPA_MASK(SIOCGIWSCAN)
        /* can't display meaningful string in iwlist
        | IW_EVENT_CAPA_MASK(SIOCGIWTXPOW)
        | IW_EVENT_CAPA_MASK(IWEVMICHAELMICFAILURE)
        | IW_EVENT_CAPA_MASK(IWEVASSOCREQIE)
        | IW_EVENT_CAPA_MASK(IWEVPMKIDCAND)
        */
        );
    prRange->event_capa[1] = IW_EVENT_CAPA_K_1;

    /* report 2.4G channel and frequency only */
    prRange->num_channels = (__u16)NUM_CHANNELS;
    prRange->num_frequency = (__u8)NUM_CHANNELS;
    for (i = 0; i < NUM_CHANNELS; i++) {
        /* iwlib takes this number as channel number */
        prRange->freq[i].i = i + 1;
        prRange->freq[i].m = channel_freq[i];
        prRange->freq[i].e = 6;  /* Values in table in MHz */
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(
        prGlueInfo,
        wlanoidQuerySupportedRates,
        &aucSuppRate,
        sizeof(aucSuppRate),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(
        prGlueInfo->prAdapter,
        wlanoidQuerySupportedRates,
        &aucSuppRate,
        sizeof(aucSuppRate),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    for (i = 0; i < IW_MAX_BITRATES && i < PARAM_MAX_LEN_RATES_EX ; i++) {
        if (aucSuppRate[i] == 0) {
            break;
    }
        prRange->bitrate[i] = (aucSuppRate[i] & 0x7F) * 500000; /* 0.5Mbps */
    }
    prRange->num_bitrates = i;

    prRange->min_rts = 0;
    prRange->max_rts = 2347;
    prRange->min_frag = 256;
    prRange->max_frag = 2346;

    prRange->min_pmp = 0;    /* power management by driver */
    prRange->max_pmp = 0;    /* power management by driver */
    prRange->min_pmt = 0;    /* power management by driver */
    prRange->max_pmt = 0;    /* power management by driver */
    prRange->pmp_flags = IW_POWER_RELATIVE;    /* pm default flag */
    prRange->pmt_flags = IW_POWER_ON;    /* pm timeout flag */
    prRange->pm_capa = IW_POWER_ON;  /* power management by driver */

    prRange->encoding_size[0] = 5;   /* wep40 */
    prRange->encoding_size[1] = 16;   /* tkip */
    prRange->encoding_size[2] = 16;   /* ckip */
    prRange->encoding_size[3] = 16;   /* ccmp */
    prRange->encoding_size[4] = 13;  /* wep104 */
    prRange->encoding_size[5] = 16;  /* wep128 */
    prRange->num_encoding_sizes = 6;
    prRange->max_encoding_tokens = 6;    /* token? */

#if WIRELESS_EXT < 17
    prRange->txpower_capa = 0x0002; /* IW_TXPOW_RELATIVE */
#else
    prRange->txpower_capa = IW_TXPOW_RELATIVE;
#endif
    prRange->num_txpower = 5;
    prRange->txpower[0] = 0; /* minimum */
    prRange->txpower[1] = 25; /* 25% */
    prRange->txpower[2] = 50;    /* 50% */
    prRange->txpower[3] = 100;    /* 100% */

    prRange->we_version_compiled = WIRELESS_EXT;
    prRange->we_version_source = WIRELESS_EXT;

    prRange->retry_capa = IW_RETRY_LIMIT;
    prRange->retry_flags = IW_RETRY_LIMIT;
    prRange->min_retry = 7;
    prRange->max_retry = 7;
    prRange->r_time_flags = IW_RETRY_ON;
    prRange->min_r_time = 0;
    prRange->max_r_time = 0;

    /* signal strength and link quality */
    /* Just define range here, reporting value moved to wext_get_stats() */
    prRange->sensitivity = -83;  /* fixed value */
    prRange->max_qual.qual = 100;  /* max 100% */
    prRange->max_qual.level = (__u8)(0x100 - 0); /* max 0 dbm */
    prRange->max_qual.noise = (__u8)(0x100 - 0); /* max 0 dbm */

    /* enc_capa */
    prRange->enc_capa = IW_ENC_CAPA_WPA |
        IW_ENC_CAPA_WPA2 |
        IW_ENC_CAPA_CIPHER_TKIP |
        IW_ENC_CAPA_CIPHER_CCMP;

    /* min_pms; Minimal PM saving */
    /* max_pms; Maximal PM saving */
    /* pms_flags; How to decode max/min PM saving */

    /* modul_capa; IW_MODUL_* bit field */
    /* bitrate_capa; Types of bitrates supported */

    return 0;
} /* wext_get_range */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_ap (
    IN struct net_device *prDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct sockaddr *prAddr,
    IN char *pcExtra
    )
{
    return 0;
} /* wext_set_ap */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_ap (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct sockaddr *prAddr,
    IN char *pcExtra
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prAddr);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prAddr)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (!netif_carrier_ok(prNetDev)) {
        return -ENOTCONN;
    }

    if (prGlueInfo->eParamMediaStateIndicated == PARAM_MEDIA_STATE_DISCONNECTED) {
        memset(prAddr, 0, 6);
        return 0;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryBssid,
        prAddr->sa_data,
        ETH_ALEN,
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryBssid,
        prAddr->sa_data,
        ETH_ALEN,
        &u4BufLen);
#endif

    return (WLAN_STATUS_SUCCESS == rStatus) ? 0 : -EFAULT;
} /* wext_get_ap */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_mlme (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_point *prData,
    IN char *pcExtra
    )
{
    struct iw_mlme *prMlme = NULL;
    UINT_8 pucBssid[ETH_ALEN];

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(pcExtra);
    if (FALSE == GLUE_CHK_PR2(prNetDev, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    prMlme = (struct iw_mlme *)pcExtra;
    if (prMlme->cmd == IW_MLME_DEAUTH || prMlme->cmd == IW_MLME_DISASSOC) {
        if (!netif_carrier_ok(prNetDev)) {
            printk(KERN_DEBUG "[wifi] Set MLME Deauth/Disassoc, but netif_carrier_off\n");
            return 0;
        }
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidQueryBssid,
            pucBssid,
            ETH_ALEN,
            TRUE,
            TRUE,
            &u4BufLen);
#else
        rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
            wlanoidQueryBssid,
            pucBssid,
            ETH_ALEN,
            &u4BufLen);
#endif

        if (WLAN_STATUS_SUCCESS != rStatus) {
            return -EFAULT;
        }

        if (memcmp(pucBssid, prMlme->addr.sa_data, ETH_ALEN)) {
            printk(KERN_DEBUG "[wifi] Set MLME Fail, different bssid: ["
                MACSTR "], ["
                MACSTR "]\n",
                MAC2STR(pucBssid),
                MAC2STR(prMlme->addr.sa_data));
            return -EINVAL;
        }

        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidSetDisassociate,
            NULL,
            0,
            FALSE,
            TRUE,
            &u4BufLen);

        if (WLAN_STATUS_SUCCESS == rStatus) {
            prGlueInfo->fgRadioOn = FALSE;
            return 0;
        }
        else  {
            return -EFAULT;
        }
    }
    else {
        printk(KERN_DEBUG "[wifi] unsupported IW_MLME_ command :%d\n", prMlme->cmd);
        return -EOPNOTSUPP;
    }
} /* wext_set_mlme */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_scan (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN union iwreq_data *prData,
    IN char *pcExtra
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;
    int essid_len = 0;

    ENUM_PARAM_OP_MODE_T eOpMode;
    PARAM_SSID_T rNewSsid;

    ASSERT(prNetDev);
    if (FALSE == GLUE_CHK_DEV(prNetDev)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (prGlueInfo->fgRadioOn == FALSE) {
        // Set to infrastructure mode before set ssid
        eOpMode = NET_TYPE_INFRA;
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidSetInfrastructureMode,
            &eOpMode,
            sizeof(eOpMode),
            FALSE,
            TRUE,
            &u4BufLen);

        // To turn on RF power when scan
        rNewSsid.aucSsid[0] = '.';
        rNewSsid.aucSsid[1] = 0;
        rNewSsid.u4SsidLen = 1;
        if (sdio_io_ctrl(prGlueInfo,
                wlanoidSetSsid,
                (PVOID)&rNewSsid,
                sizeof(PARAM_SSID_T),
                FALSE,
                TRUE,
                &u4BufLen) != WLAN_STATUS_SUCCESS) {
            printk(KERN_WARNING "Fail to set ssid\n");
            return -EFAULT;
        }
        //kalUdelay(10);

        prGlueInfo->fgRadioOn = TRUE;
    }

    // TODO:  parse flags and issue different scan requests?



#if defined(_HIF_SDIO)

//mdy by mtk80743
#define MAX_SSID_LEN 32

	if(prData)
		essid_len = ((struct iw_scan_req *)(((struct iw_point*)prData)->pointer))->essid_len;

    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSetBssidListScan,
		(PVOID)pcExtra,
		essid_len,
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
        wlanoidSetBssidListScan,
        NULL,
        0,
        &u4BufLen);
#endif

    return (rStatus == WLAN_STATUS_SUCCESS) ? 0 : -EFAULT;
} /* wext_set_scan */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_scan (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN OUT struct iw_point *prData,
    IN char *pcExtra
    )
{
    UINT_32 i = 0;
    UINT_32 j = 0;
    P_PARAM_BSSID_LIST_EX_T prList = NULL;
    P_PARAM_BSSID_EX_T prBss = NULL;
    P_PARAM_VARIABLE_IE_T prDesiredIE = NULL;
    struct iw_event iwEvent; /* local iw_event buffer */

    /* write pointer of extra buffer */
    char *pcCur = NULL;
    /* pointer to the end of  last full entry in extra buffer */
    char *pcValidEntryEnd = NULL;
    char *pcEnd = NULL; /* end of extra buffer */

    UINT_32 u4AllocBufLen = 0;

    /* arrange rate information */
    UINT_32 u4HighestRate = 0;
    char aucRatesBuf[64];
    UINT_32 u4BufIndex;

    /* return value */
    int ret = 0;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prData);
    ASSERT(pcExtra);
    if (FALSE == GLUE_CHK_PR3(prNetDev, prData, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /* Initialize local variables */
    pcCur = pcExtra;
    pcValidEntryEnd = pcExtra;
    pcEnd = pcExtra + prData->length; /* end of extra buffer */

    /* Allocate another query buffer with the same size of extra buffer */
    u4AllocBufLen = prData->length;
    prList = kalMemAlloc(u4AllocBufLen);
    if (prList == NULL) {
        printk(KERN_NOTICE "[wifi] no memory for scan list:%d\n", prData->length);
        ret = -ENOMEM;
        goto error;
    }
    prList->u4NumberOfItems = 0;

    /* Do not wait scan done. Application should wait driver's SIOCGIWSCAN event
     * before query scan results.
    */

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryBssidList,
        prList,
        u4AllocBufLen,
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryBssidList,
        prList,
        u4AllocBufLen,
        &u4BufLen);
#endif

    if (rStatus == WLAN_STATUS_INVALID_LENGTH) {
        /* Buffer length is not large enough. */
        //printk(KERN_INFO "[wifi] buf:%d result:%ld\n", pData->length, u4BufLen);

#if WIRELESS_EXT >= 17
        /* This feature is supported in WE-17 or above, limited by iwlist.
        ** Return -E2BIG and iwlist will request again with a larger buffer.
        */
        ret = -E2BIG;
        /* Update length to give application a hint on result length */
        prData->length = (__u16)u4BufLen;
        goto error;
#else
        /* Realloc a larger query buffer here, but don't write too much to extra
        ** buffer when filling it later.
        */
        kalMemFree(prList, u4AllocBufLen);

        u4AllocBufLen = u4BufLen;
        prList = kalMemAlloc(u4AllocBufLen);
        if (prList == NULL) {
            printk(KERN_NOTICE "[wifi] no memory for larger scan list :%ld\n", u4BufLen);
            ret = -ENOMEM;
            goto error;
        }
        prList->NumberOfItems = 0;

    #if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidQueryBssidList,
            prList,
            u4AllocBufLen,
            TRUE,
            TRUE,
            &u4BufLen);
    #else
        rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
            wlanoidQueryBssidList,
            prList,
            u4AllocBufLen,
            &u4BufLen);
    #endif
        if (rStatus == WLAN_STATUS_INVALID_LENGTH) {
            printk(KERN_INFO "[wifi] larger buf:%d result:%ld\n", u4AllocBufLen, u4BufLen);
            ret = -E2BIG;
            prData->length = (__u16)u4BufLen;
            goto error;
        }
        else if (WLAN_STATUS_SUCCESS != rStatus) {
            ret = -EFAULT;
            goto error;
        }
#endif /* WIRELESS_EXT >= 17 */
    }
    else if (WLAN_STATUS_SUCCESS != rStatus) {
        ret = -EFAULT;
        goto error;
    }

    if (prList->u4NumberOfItems > CFG_MAX_NUM_BSS_LIST) {
        printk(KERN_INFO "[wifi] strange scan result count:%ld\n",
            prList->u4NumberOfItems);
        ret = -EFAULT;
        goto error;
    }

    /* Copy required data from pList to pcExtra */
    prBss = &prList->arBssid[0];    /* set to the first entry */
    for (i = 0; i < prList->u4NumberOfItems; ++i) {
        /* BSSID */
        iwEvent.cmd = SIOCGIWAP;
        iwEvent.len = IW_EV_ADDR_LEN;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.ap_addr.sa_family = ARPHRD_ETHER;
        memcpy(iwEvent.u.ap_addr.sa_data, prBss->arMacAddress, ETH_ALEN);
        memcpy(pcCur, &iwEvent, IW_EV_ADDR_LEN);
        pcCur += IW_EV_ADDR_LEN;

        /* SSID */
        iwEvent.cmd = SIOCGIWESSID;
        /* Modification to user space pointer(essid.pointer) is not needed. */
        iwEvent.u.essid.length = (__u16)prBss->rSsid.u4SsidLen;
        iwEvent.len = IW_EV_POINT_LEN + iwEvent.u.essid.length;

        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.essid.flags = 1;
        iwEvent.u.essid.pointer = NULL;

#if WIRELESS_EXT <= 18
        memcpy(pcCur, &iwEvent, iwEvent.len);
#else
        memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
        memcpy(pcCur + IW_EV_LCP_LEN,
            &iwEvent.u.data.length,
            sizeof(struct  iw_point) - IW_EV_POINT_OFF);
#endif
        memcpy(pcCur + IW_EV_POINT_LEN, prBss->rSsid.aucSsid, iwEvent.u.essid.length);
        pcCur += iwEvent.len;
        /* Frequency */
        iwEvent.cmd = SIOCGIWFREQ;
        iwEvent.len = IW_EV_FREQ_LEN;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.freq.m = prBss->rConfiguration.u4DSConfig;
        iwEvent.u.freq.e = 3;   /* (in KHz) */
        iwEvent.u.freq.i = 0;
        memcpy(pcCur, &iwEvent, IW_EV_FREQ_LEN);
        pcCur += IW_EV_FREQ_LEN;

        /* Operation Mode */
        iwEvent.cmd = SIOCGIWMODE;
        iwEvent.len = IW_EV_UINT_LEN;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        if (prBss->eOpMode == NET_TYPE_IBSS) {
            iwEvent.u.mode = IW_MODE_ADHOC;
        }
        else if (prBss->eOpMode == NET_TYPE_INFRA) {
            iwEvent.u.mode = IW_MODE_INFRA;
        }
        else {
            iwEvent.u.mode = IW_MODE_AUTO;
        }
        memcpy(pcCur, &iwEvent, IW_EV_UINT_LEN);
        pcCur += IW_EV_UINT_LEN;

        /* Quality */
        iwEvent.cmd = IWEVQUAL;
        iwEvent.len = IW_EV_QUAL_LEN;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.qual.qual = 0; /* Quality not available now */
        /* -100 < Rssi < -10, normalized by adding 0x100 */
        iwEvent.u.qual.level = 0x100 + prBss->rRssi;
        iwEvent.u.qual.noise = 0; /* Noise not available now */
        iwEvent.u.qual.updated = IW_QUAL_QUAL_INVALID | IW_QUAL_LEVEL_UPDATED \
            | IW_QUAL_NOISE_INVALID;
        memcpy(pcCur, &iwEvent, IW_EV_QUAL_LEN);
        pcCur += IW_EV_QUAL_LEN;

        /* Security Mode*/
        iwEvent.cmd = SIOCGIWENCODE;
        iwEvent.len = IW_EV_POINT_LEN;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.data.pointer = NULL;
        iwEvent.u.data.flags = 0;
        iwEvent.u.data.length = 0;
        if(!prBss->u4Privacy) {
            iwEvent.u.data.flags |=  IW_ENCODE_DISABLED;
        }
#if WIRELESS_EXT <= 18
        memcpy(pcCur, &iwEvent, IW_EV_POINT_LEN);
#else
        memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
        memcpy(pcCur + IW_EV_LCP_LEN,
            &iwEvent.u.data.length,
            sizeof(struct  iw_point) - IW_EV_POINT_OFF);
#endif
        pcCur += IW_EV_POINT_LEN;

        /* rearrange rate information */
        u4BufIndex = sprintf(aucRatesBuf, "Rates (Mb/s):");
        u4HighestRate = 0;
        for (j = 0; j < PARAM_MAX_LEN_RATES_EX; ++j) {
            UINT_8 curRate = prBss->rSupportedRates[j] & 0x7F;
            if (curRate == 0) {
                break;
            }

            if (curRate > u4HighestRate) {
                u4HighestRate = curRate;
            }

            if (curRate == RATE_5_5M) {
                u4BufIndex += sprintf(aucRatesBuf + u4BufIndex, " 5.5");
            }
            else {
                u4BufIndex += sprintf(aucRatesBuf + u4BufIndex, " %d", curRate / 2);
            }
    #if DBG
            if (u4BufIndex > sizeof(aucRatesBuf)) {
                printk("rate info too long\n");
                break;
            }
    #endif
        }
        /* Report Highest Rates */
        iwEvent.cmd = SIOCGIWRATE;
        iwEvent.len = IW_EV_PARAM_LEN;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.bitrate.value = u4HighestRate * 500000;
        iwEvent.u.bitrate.fixed = 0;
        iwEvent.u.bitrate.disabled = 0;
        iwEvent.u.bitrate.flags = 0;
        memcpy(pcCur, &iwEvent, iwEvent.len);
        pcCur += iwEvent.len;

    #if WIRELESS_EXT >= 15  /* IWEVCUSTOM is available in WE-15 or above */
        /* Report Residual Rates */
        iwEvent.cmd = IWEVCUSTOM;
        iwEvent.u.data.length = u4BufIndex;
        iwEvent.len = IW_EV_POINT_LEN + iwEvent.u.data.length;
        if ((pcCur + iwEvent.len) > pcEnd)
            break;
        iwEvent.u.data.flags = 0;
     #if WIRELESS_EXT <= 18
        memcpy(pcCur, &iwEvent, IW_EV_POINT_LEN);
     #else
        memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
        memcpy(pcCur + IW_EV_LCP_LEN,
            &iwEvent.u.data.length,
            sizeof(struct  iw_point) - IW_EV_POINT_OFF);
     #endif
        memcpy(pcCur + IW_EV_POINT_LEN, aucRatesBuf, u4BufIndex);
        pcCur += iwEvent.len;
    #endif /* WIRELESS_EXT >= 15 */


    if (wextSrchDesiredWPAIE(&prBss->aucIEs[sizeof(PARAM_FIXED_IEs)],
             prBss->u4IELength - sizeof(PARAM_FIXED_IEs),
             0xDD,
             (PUINT_8 *)&prDesiredIE)) {
            iwEvent.cmd = IWEVGENIE;
            iwEvent.u.data.flags = 1;
            iwEvent.u.data.length = 2 + (__u16)prDesiredIE->ucLength;
            iwEvent.len = IW_EV_POINT_LEN + iwEvent.u.data.length;
            if ((pcCur + iwEvent.len) > pcEnd)
                break;
#if WIRELESS_EXT <= 18
            memcpy(pcCur, &iwEvent, IW_EV_POINT_LEN);
#else
            memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
            memcpy(pcCur + IW_EV_LCP_LEN,
                   &iwEvent.u.data.length,
                   sizeof(struct  iw_point) - IW_EV_POINT_OFF);
#endif
            memcpy(pcCur + IW_EV_POINT_LEN, prDesiredIE, 2 + prDesiredIE->ucLength);
            pcCur += iwEvent.len;
    }

#if SUPPORT_WPS  /* search WPS IE (0xDD, 221, OUI: 0x0050f204 ) */
    if (wextSrchDesiredWPSIE(&prBss->aucIEs[sizeof(PARAM_FIXED_IEs)],
              prBss->u4IELength - sizeof(PARAM_FIXED_IEs),
              0xDD,
              (PUINT_8 *)&prDesiredIE)) {
                iwEvent.cmd = IWEVGENIE;
                iwEvent.u.data.flags = 1;
                iwEvent.u.data.length = 2 + (__u16)prDesiredIE->ucLength;
                iwEvent.len = IW_EV_POINT_LEN + iwEvent.u.data.length;
            if ((pcCur + iwEvent.len) > pcEnd)
                break;
#if WIRELESS_EXT <= 18
            memcpy(pcCur, &iwEvent, IW_EV_POINT_LEN);
#else
            memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
            memcpy(pcCur + IW_EV_LCP_LEN,
                &iwEvent.u.data.length,
                sizeof(struct  iw_point) - IW_EV_POINT_OFF);
#endif
                memcpy(pcCur + IW_EV_POINT_LEN, prDesiredIE, 2 + prDesiredIE->ucLength);
                pcCur += iwEvent.len;
            }
#endif


        /* Search RSN IE (0x30, 48). pBss->IEs starts from timestamp. */
        /* pBss->IEs starts from timestamp */
        if (wextSrchDesiredWPAIE(&prBss->aucIEs[sizeof(PARAM_FIXED_IEs)],
                prBss->u4IELength -sizeof(PARAM_FIXED_IEs),
                0x30,
                (PUINT_8 *)&prDesiredIE)) {

                iwEvent.cmd = IWEVGENIE;
                iwEvent.u.data.flags = 1;
                iwEvent.u.data.length = 2 + (__u16)prDesiredIE->ucLength;
                iwEvent.len = IW_EV_POINT_LEN + iwEvent.u.data.length;
            if ((pcCur + iwEvent.len) > pcEnd)
                break;
#if WIRELESS_EXT <= 18
            memcpy(pcCur, &iwEvent, IW_EV_POINT_LEN);
#else
            memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
            memcpy(pcCur + IW_EV_LCP_LEN,
                &iwEvent.u.data.length,
                sizeof(struct  iw_point) - IW_EV_POINT_OFF);
#endif
            memcpy(pcCur + IW_EV_POINT_LEN, prDesiredIE, 2 + prDesiredIE->ucLength);
            pcCur += iwEvent.len;
        }

#if SUPPORT_WAPI
             /*Search WAPI IE (0x44, 68)*/
	if (wextSrchDesiredWPAIE(&prBss->aucIEs[sizeof(PARAM_FIXED_IEs)],
                prBss->u4IELength -sizeof(PARAM_FIXED_IEs),
                0x44,
                (PUINT_8 *)&prDesiredIE)) {

                iwEvent.cmd = IWEVGENIE;
                iwEvent.u.data.flags = 1;
                iwEvent.u.data.length = 2 + (__u16)prDesiredIE->ucLength;
                iwEvent.len = IW_EV_POINT_LEN + iwEvent.u.data.length;
            if ((pcCur + iwEvent.len) > pcEnd)
                break;
#if WIRELESS_EXT <= 18
            memcpy(pcCur, &iwEvent, IW_EV_POINT_LEN);
#else
            memcpy(pcCur, &iwEvent, IW_EV_LCP_LEN);
            memcpy(pcCur + IW_EV_LCP_LEN,
                &iwEvent.u.data.length,
                sizeof(struct  iw_point) - IW_EV_POINT_OFF);
#endif
            memcpy(pcCur + IW_EV_POINT_LEN, prDesiredIE, 2 + prDesiredIE->ucLength);
            pcCur += iwEvent.len;
        }
#endif	
        /* Complete an entry. Update end of valid entry */
        pcValidEntryEnd = pcCur;
        /* Extract next bss */
        prBss = (P_PARAM_BSSID_EX_T)((char *)prBss + prBss->u4Length);
    }

    /* Update valid data length for caller function and upper layer
     * applications.
     */
    prData->length = (pcValidEntryEnd - pcExtra);
    //printk(KERN_INFO "[wifi] buf:%d result:%ld\n", pData->length, u4BufLen);

error:
    /* free local query buffer */
    if (prList) {
        kalMemFree(prList, u4AllocBufLen);
    }

    return ret;
} /* wext_get_scan */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_essid (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_point *prEssid,
    IN char *pcExtra
    )
{
    PARAM_SSID_T rNewSsid;
    UINT_32 cipher;
    ENUM_ENCRYPTION_STATUS_T eEncStatus;
    ENUM_PARAM_AUTH_MODE_T eAuthMode;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prEssid);
    ASSERT(pcExtra);
    if (FALSE == GLUE_CHK_PR3(prNetDev, prEssid, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (prEssid->length > (IW_ESSID_MAX_SIZE + 1)) {
        return -E2BIG;
    }

    /* set auth mode */
    if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_DISABLED) {
        eAuthMode = (prGlueInfo->rWpaInfo.u4AuthAlg == IW_AUTH_ALG_OPEN_SYSTEM) ?
            AUTH_MODE_OPEN : AUTH_MODE_AUTO_SWITCH;
        /*
        printk(KERN_INFO "IW_AUTH_WPA_VERSION_DISABLED->Param_AuthMode%s\n",
            (eAuthMode == AUTH_MODE_OPEN) ? "Open" : "Shared");
        */
    }
    else {
        /* set auth mode */
        switch(prGlueInfo->rWpaInfo.u4KeyMgmt) {
            case IW_AUTH_KEY_MGMT_802_1X:
                eAuthMode =
                    (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA) ?
                    AUTH_MODE_WPA : AUTH_MODE_WPA2;
                /*
                printk("IW_AUTH_KEY_MGMT_802_1X->AUTH_MODE_WPA%s\n",
                    (authMode == AUTH_MODE_WPA) ? "" : "2");
                */
                break;
            case IW_AUTH_KEY_MGMT_PSK:
                eAuthMode =
                    (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA) ?
                    AUTH_MODE_WPA_PSK: AUTH_MODE_WPA2_PSK;
                /*
                printk("IW_AUTH_KEY_MGMT_PSK->AUTH_MODE_WPA%sPSK\n",
                    (authMode == AUTH_MODE_WPA_PSK) ? "" : "2");
                */
                break;
#if defined (IW_AUTH_KEY_MGMT_WPA_NONE)
            case IW_AUTH_KEY_MGMT_WPA_NONE:
                eAuthMode = AUTH_MODE_WPA_NONE;
                /*
                printk("IW_AUTH_KEY_MGMT_WPA_NONE->AUTH_MODE_WPA_NONE\n");
                */
                break;
#endif
            default:
                printk(KERN_INFO DRV_NAME"strange IW_AUTH_KEY_MGMT : %ld set auto switch\n",
                    prGlueInfo->rWpaInfo.u4KeyMgmt);
                eAuthMode = AUTH_MODE_AUTO_SWITCH;
                break;
        }
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSetAuthMode,
        &eAuthMode,
        sizeof(eAuthMode),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
        wlanoidSetAuthMode,
        &eAuthMode,
        sizeof(eAuthMode),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    /* set encryption status */
    cipher = prGlueInfo->rWpaInfo.u4CipherGroup |
        prGlueInfo->rWpaInfo.u4CipherPairwise;
    if (cipher & IW_AUTH_CIPHER_CCMP) {
        /*
        printk("IW_AUTH_CIPHER_CCMP->ENUM_ENCRYPTION3_ENABLED\n");
        */
        eEncStatus = ENUM_ENCRYPTION3_ENABLED;
    }
    else if (cipher & IW_AUTH_CIPHER_TKIP) {
        /*
        printk("IW_AUTH_CIPHER_TKIP->ENUM_ENCRYPTION2_ENABLED\n");
        */
        eEncStatus = ENUM_ENCRYPTION2_ENABLED;
    }
    else if (cipher & (IW_AUTH_CIPHER_WEP104 | IW_AUTH_CIPHER_WEP40)) {
        /*
        printk("IW_AUTH_CIPHER_WEPx->ENUM_ENCRYPTION1_ENABLED\n");
        */
        eEncStatus = ENUM_ENCRYPTION1_ENABLED;
    }
    else if (cipher & IW_AUTH_CIPHER_NONE){
        /*
        printk("IW_AUTH_CIPHER_NONE->ENUM_ENCRYPTION_DISABLED\n");
        */
        if (prGlueInfo->rWpaInfo.fgPrivacyInvoke)
            eEncStatus = ENUM_ENCRYPTION1_ENABLED;
        else
            eEncStatus = ENUM_ENCRYPTION_DISABLED;
    }
    else {
        printk("unknown IW_AUTH_CIPHER->Param_EncryptionDisabled\n");
        eEncStatus = ENUM_ENCRYPTION_DISABLED;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSetEncryptionStatus,
        &eEncStatus,
        sizeof(eEncStatus),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
        wlanoidSetEncryptionStatus,
        &eEncStatus,
        sizeof(eEncStatus),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

#if WIRELESS_EXT < 21
    /* GeorgeKuo: a length error bug exists in (WE < 21) cases, kernel before
    ** 2.6.19. Cut the trailing '\0'.
    */
    rNewSsid.u4SsidLen = (prEssid->length) ? prEssid->length - 1 : 0;
#else
    rNewSsid.u4SsidLen = prEssid->length;
#endif
    kalMemCopy(rNewSsid.aucSsid, pcExtra, rNewSsid.u4SsidLen);

    /*
    rNewSsid.aucSsid[rNewSsid.u4SsidLen] = '\0';
    printk("set ssid(%lu): %s\n", rNewSsid.u4SsidLen, rNewSsid.aucSsid);
    */

#if defined(_HIF_SDIO)
    if (sdio_io_ctrl(prGlueInfo,
        wlanoidSetSsid,
        (PVOID)&rNewSsid,
        sizeof(PARAM_SSID_T),
        FALSE,
        TRUE,
        &u4BufLen) != WLAN_STATUS_SUCCESS) {
        printk(KERN_WARNING "Fail to set ssid\n");
        return -EFAULT;
    }
#else
    if (wlanSetInformation(prGlueInfo->prAdapter,
            wlanoidSetSsid,
            (PVOID)&rNewSsid,
            sizeof(PARAM_SSID_T),
            &u4BufLen) != WLAN_STATUS_SUCCESS) {
        printk(KERN_WARNING "Fail to set ssid\n");
        return -EFAULT;
    }
#endif
    // add for turn on RF power issue of Android Linux driver
    prGlueInfo->fgRadioOn = TRUE;

    return 0;
} /* wext_set_essid */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_essid (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_point *prEssid,
    OUT char *pcExtra
    )
{
    PARAM_SSID_T ssid;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prEssid);
    ASSERT(pcExtra);
    if (FALSE == GLUE_CHK_PR3(prNetDev, prEssid, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (!netif_carrier_ok(prNetDev)) {
        return -ENOTCONN;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQuerySsid,
        &ssid,
        sizeof(ssid),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQuerySsid,
        &ssid,
        sizeof(ssid),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    kalMemCopy(pcExtra, ssid.aucSsid, ssid.u4SsidLen);
    prEssid->length = ssid.u4SsidLen;
    prEssid->flags = 1;

    return 0;
} /* wext_get_essid */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int
wext_set_rate (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwReqInfo,
    IN struct iw_param *prRate,
    IN char *pcExtra
    )
{
    PARAM_RATES_EX aucSuppRate = {0};
    PARAM_RATES_EX aucNewRate = {0};
    UINT_32 u4NewRateLen = 0;
    UINT_32 i;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prRate);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prRate)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /*
    printk("value = %d, fixed = %d, disable = %d, flags = %d\n",
        prRate->value, prRate->fixed, prRate->disabled, prRate->flags);
    */
#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(
        prGlueInfo,
        wlanoidQuerySupportedRates,
        &aucSuppRate,
        sizeof(aucSuppRate),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(
        prGlueInfo->prAdapter,
        wlanoidQuerySupportedRates,
        &aucSuppRate,
        sizeof(aucSuppRate),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    /* Case: AUTO */
    if (prRate->value < 0)  {
        if (prRate->fixed == 0) {
            /* iwconfig wlan0 rate auto */

            /* set full supported rate to device */
            /* printk("wlanoidQuerySupportedRates():u4BufLen = %ld\n", u4BufLen); */
#if defined(_HIF_SDIO)
            rStatus = sdio_io_ctrl(
                prGlueInfo,
                wlanoidSetDesiredRates,
                &aucSuppRate,
                sizeof(aucSuppRate),
                FALSE,
                TRUE,
                &u4BufLen);
#else
            rStatus = wlanSetInformation(
                prGlueInfo->prAdapter,
                wlanoidSetDesiredRates,
                &aucSuppRate,
                sizeof(aucSuppRate),
                &u4BufLen);
#endif
            return (WLAN_STATUS_SUCCESS == rStatus) ? 0 : -EFAULT;
        }
        else {
            /* iwconfig wlan0 rate fixed */

            /* fix rate to what? DO NOTHING */
            return -EINVAL;
        }
    }


    aucNewRate[0] = prRate->value / 500000; /* In unit of 500k */

    for (i = 0; i < PARAM_MAX_LEN_RATES_EX; i++) {
        /* check the given value is supported */
        if (aucSuppRate[i] == 0) {
            break;
        }

        if (aucNewRate[0] == aucSuppRate[i]) {
            u4NewRateLen = 1;
            break;
        }
    }

    if (u4NewRateLen == 0) {
        /* the given value is not supported */
        /* return error or use given rate as upper bound? */
        return -EINVAL;
    }

    if (prRate->fixed == 0) {
        /* add all rates lower than desired rate */
        for (i = 0; i < PARAM_MAX_LEN_RATES_EX; ++i) {
            if (aucSuppRate[i] == 0) {
                break;
            }

            if (aucSuppRate[i] < aucNewRate[0]) {
                aucNewRate[u4NewRateLen++] = aucSuppRate[i];
            }
        }
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(
        prGlueInfo,
        wlanoidSetDesiredRates,
        &aucNewRate,
        sizeof(aucNewRate),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(
        prGlueInfo->prAdapter,
        wlanoidSetDesiredRates,
        &aucNewRate,
        sizeof(aucNewRate),
        &u4BufLen);
#endif

    return (WLAN_STATUS_SUCCESS == rStatus) ? 0 : -EFAULT;
} /* wext_set_rate */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_rate (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_param *prRate,
    IN char *pcExtra
    )
{
    UINT_32 u4Rate;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prRate);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prRate)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (!netif_carrier_ok(prNetDev)) {
        return -ENOTCONN;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryLinkSpeed,
        &u4Rate,
        sizeof(u4Rate),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryLinkSpeed,
        &u4Rate,
        sizeof(u4Rate),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    prRate->value = u4Rate * 100;   /* u4Rate is in unit of 100bps */
    prRate->fixed = 0;

    return 0;
} /* wext_get_rate */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_rts (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_param *prRts,
    IN char *pcExtra
    )
{
    PARAM_RTS_THRESHOLD u4RtsThresh;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prRts);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prRts)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (prRts->disabled == 1) {
        u4RtsThresh = 2347;
    }
    else if (prRts->value < 0 || prRts->value > 2347) {
        return -EINVAL;
    }
    else {
        u4RtsThresh = (PARAM_RTS_THRESHOLD)prRts->value;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSetRtsThreshold,
        &u4RtsThresh,
        sizeof(u4RtsThresh),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
        wlanoidSetRtsThreshold,
        &u4RtsThresh,
        sizeof(u4RtsThresh),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    prRts->value = (typeof(prRts->value ))u4RtsThresh;
    prRts->disabled = (prRts->value > 2347) ? 1 : 0;
    prRts->fixed = 1;

    return 0;
} /* wext_set_rts */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_rts (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_param *prRts,
    IN char *pcExtra
    )
{
    PARAM_RTS_THRESHOLD u4RtsThresh;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prRts);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prRts)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryRtsThreshold,
        &u4RtsThresh,
        sizeof(u4RtsThresh),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryRtsThreshold,
        &u4RtsThresh,
        sizeof(u4RtsThresh),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    prRts->value = (typeof(prRts->value))u4RtsThresh;
    prRts->disabled = (prRts->value > 2347 || prRts->value < 0) ? 1 : 0;
    prRts->fixed = 1;

    return 0;
} /* wext_get_rts */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_frag (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_param *prFrag,
    IN char *pcExtra
    )
{
    ASSERT(prFrag);

    prFrag->value = 2346;
    prFrag->fixed = 1;
    prFrag->disabled = 1;
    return 0;
} /* wext_get_frag */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_txpow (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_param *prTxPow,
    IN char *pcExtra
    )
{
    PARAM_DEVICE_POWER_STATE ePowerState;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prTxPow);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prTxPow)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    if (prTxPow->disabled) {
        ePowerState = ParamDeviceStateD3;
    }
    else {
        ePowerState = ParamDeviceStateD0;
    }

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSetAcpiDevicePowerState,
        &ePowerState,
        sizeof(ePowerState),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
        wlanoidSetAcpiDevicePowerState,
        &ePowerState,
        sizeof(ePowerState),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    wext_indicate_wext_event(prGlueInfo,
        SIOCGIWTXPOW,
        (unsigned char *)prTxPow,
        sizeof(struct iw_param));

    prGlueInfo->ePowerState = ePowerState;

    // add for turn on RF power issue of Android Linux driver
    if (ePowerState == ParamDeviceStateD3) {
        prGlueInfo->fgRadioOn = FALSE;
    }

    return 0;
} /* wext_set_txpow */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_txpow (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_param *prTxPow,
    IN char *pcExtra
    )
{
    PARAM_DEVICE_POWER_STATE ePowerState;

    P_GLUE_INFO_T prGlueInfo = NULL;

    ASSERT(prNetDev);
    ASSERT(prTxPow);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prTxPow)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /* GeorgeKuo: wlanoidQueryAcpiDevicePowerState() reports capability, not
     * current state. Use GLUE_INFO_T to store state.
    */
    ePowerState = prGlueInfo->ePowerState;

    /* TxPow parameters: Fixed at relative 100% */
#if WIRELESS_EXT < 17
    prTxPow->flags = 0x0002; /* IW_TXPOW_RELATIVE */
#else
    prTxPow->flags = IW_TXPOW_RELATIVE;
#endif
    prTxPow->value = 100;
    prTxPow->fixed = 1;
    prTxPow->disabled = (ePowerState != ParamDeviceStateD3) ? FALSE : TRUE;

    return 0;
} /* wext_get_txpow */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_encode (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_point *prEnc,
    IN char *pcExtra
    )
{
    ENUM_ENCRYPTION_STATUS_T eEncMode;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prEnc);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prEnc)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryEncryptionStatus,
        &eEncMode,
        sizeof(eEncMode),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryEncryptionStatus,
        &eEncMode,
        sizeof(eEncMode),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    switch(eEncMode) {
    case ENUM_WEP_DISABLED:
        prEnc->flags = IW_ENCODE_DISABLED;
        break;
    case ENUM_WEP_ENABLED:
        prEnc->flags = IW_ENCODE_ENABLED;
        break;
    case ENUM_WEP_KEY_ABSENT:
        prEnc->flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
        break;
    default:
        prEnc->flags = IW_ENCODE_ENABLED;
        break;
    }

    /* Cipher, Key Content, Key ID can't be queried */
    prEnc->flags |= IW_ENCODE_NOKEY;
    return 0;
} /* wext_get_encode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_encode (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_point *prEnc,
    IN char *pcExtra
    )
{
    ENUM_ENCRYPTION_STATUS_T eEncStatus;
    ENUM_PARAM_AUTH_MODE_T eAuthMode;
    UINT_8 wepBuf[48];
    P_PARAM_WEP_T prWepKey = (P_PARAM_WEP_T)wepBuf;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prEnc);
    ASSERT(pcExtra);
    if (FALSE == GLUE_CHK_PR3(prNetDev, prEnc, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /* reset to default mode */
    prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
    prGlueInfo->rWpaInfo.u4KeyMgmt = 0;
    prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;

    /* iwconfig wlan0 key off */
    if ( (prEnc->flags & IW_ENCODE_MODE) == IW_ENCODE_DISABLED ) {
        eAuthMode = AUTH_MODE_OPEN;
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidSetAuthMode,
            &eAuthMode,
            sizeof(eAuthMode),
            FALSE,
            TRUE,
            &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
            wlanoidSetAuthMode,
            &eAuthMode,
            sizeof(eAuthMode),
            &u4BufLen);
#endif

        if (WLAN_STATUS_SUCCESS != rStatus) {
            return -EFAULT;
        }

        eEncStatus = ENUM_ENCRYPTION_DISABLED;
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
            wlanoidSetEncryptionStatus,
            &eEncStatus,
            sizeof(eEncStatus),
            FALSE,
            TRUE,
            &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
            wlanoidSetEncryptionStatus,
            &eEncStatus,
            sizeof(eEncStatus),
            &u4BufLen);
#endif
        return (rStatus == WLAN_STATUS_SUCCESS) ? 0 : -EFAULT;
    }

    /* iwconfig wlan0 key 0123456789 */
    /* iwconfig wlan0 key s:abcde */
    /* iwconfig wlan0 key 0123456789 [1] */
    /* iwconfig wlan0 key 01234567890123456789012345 [1] */
    /* check key size for WEP */
    if (prEnc->length == 5 || prEnc->length == 13 || prEnc->length == 16) {
        /* prepare PARAM_WEP key structure */
        prWepKey->u4KeyIndex = (prEnc->flags & IW_ENCODE_INDEX) ?
            (prEnc->flags & IW_ENCODE_INDEX) -1 : 0;
        if (prWepKey->u4KeyIndex > 3) {
            /* key id is out of range */
            return -EINVAL;
        }
        prWepKey->u4KeyIndex |= 0x80000000;
        prWepKey->u4Length = 12 + prEnc->length;
        prWepKey->u4KeyLength = prEnc->length;
        kalMemCopy(prWepKey->aucKeyMaterial, pcExtra, prEnc->length);
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
                     wlanoidSetAddWep,
                     prWepKey,
                     prWepKey->u4Length,
                     FALSE,
                     TRUE,
                     &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                     wlanoidSetAddWep,
                     prWepKey,
                     prWepKey->u4Length,
                     &u4BufLen);
#endif
        if (rStatus != WLAN_STATUS_SUCCESS) {
            printk(KERN_INFO DRV_NAME"wlanoidSetAddWep fail 0x%lx\n", rStatus);
            return -EFAULT;
        }

        /* change to auto switch */
        prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_SHARED_KEY |
            IW_AUTH_ALG_OPEN_SYSTEM;
        eAuthMode = AUTH_MODE_AUTO_SWITCH;
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
                     wlanoidSetAuthMode,
                     &eAuthMode,
                     sizeof(eAuthMode),
                     FALSE,
                     TRUE,
                     &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                     wlanoidSetAuthMode,
                     &eAuthMode,
                     sizeof(eAuthMode),
                     &u4BufLen);
#endif
        if (rStatus != WLAN_STATUS_SUCCESS) {
            printk(KERN_INFO DRV_NAME"wlanoidSetAuthMode fail 0x%lx\n", rStatus);
            return -EFAULT;
        }

        prGlueInfo->rWpaInfo.u4CipherPairwise =
            IW_AUTH_CIPHER_WEP104 | IW_AUTH_CIPHER_WEP40;
        prGlueInfo->rWpaInfo.u4CipherGroup =
            IW_AUTH_CIPHER_WEP104 | IW_AUTH_CIPHER_WEP40;

        eEncStatus = ENUM_WEP_ENABLED;
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
                     wlanoidSetEncryptionStatus,
                     &eEncStatus,
                     sizeof(ENUM_ENCRYPTION_STATUS_T),
                     FALSE,
                     TRUE,
                     &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                     wlanoidSetEncryptionStatus,
                     &eEncStatus,
                     sizeof(ENUM_ENCRYPTION_STATUS_T),
                     &u4BufLen);
#endif
        if (rStatus != WLAN_STATUS_SUCCESS) {
            printk(KERN_INFO DRV_NAME"wlanoidSetEncryptionStatus fail 0x%lx\n", rStatus);
            return -EFAULT;
        }

        return 0;
    }

    return -EOPNOTSUPP;
} /* wext_set_encode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
extern UINT_8 PER_FLAG;

static int
wext_set_power (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_param *prPower,
    IN char *pcExtra
    )
{
    PARAM_POWER_MODE ePowerMode;
    INT_32 i4PowerValue;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prPower);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prPower)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    //printk(KERN_INFO "wext_set_power value(%d) disabled(%d) flag(0x%x)\n",
    //  prPower->value, prPower->disabled, prPower->flags);

    if (prPower->disabled) {
        ePowerMode = Param_PowerModeCAM;
    }
    else {
        i4PowerValue = prPower->value;
#if WIRELESS_EXT < 21
        i4PowerValue /= 1000000;
#endif
        if (i4PowerValue == 0) {
            ePowerMode = Param_PowerModeCAM;
        } else if (i4PowerValue == 1) {
            ePowerMode = Param_PowerModeMAX_PSP;
        } else if (i4PowerValue == 2) {
            ePowerMode = Param_PowerModeFast_PSP;
        }
        else {
            printk(KERN_DEBUG "%s(): unsupported power management mode value = %d.\n",
                __FUNCTION__,
                prPower->value);

            return -EINVAL;
        }
    }

    if(PER_FLAG){
	printk("[WIFI] PER_FLAG is set, wifi power can not be set Non-zero Number.\n");
	return 0;	
    }

    printk("[WIFI] set power mode : %d\n",ePowerMode);
#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidSet802dot11PowerSaveProfile,
        &ePowerMode,
        sizeof(ePowerMode),
        FALSE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                wlanoidSet802dot11PowerSaveProfile,
        &ePowerMode,
        sizeof(ePowerMode),
                &u4BufLen);
#endif

    if (rStatus != WLAN_STATUS_SUCCESS) {
        printk(KERN_INFO DRV_NAME"wlanoidSet802dot11PowerSaveProfile fail 0x%lx\n", rStatus);
        return -EFAULT;
    }

    return 0;
} /* wext_set_power */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_get_power (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    OUT struct iw_param *prPower,
    IN char *pcExtra
    )
{
    PARAM_POWER_MODE ePowerMode;

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prPower);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prPower)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQuery802dot11PowerSaveProfile,
        &ePowerMode,
        sizeof(ePowerMode),
        TRUE,
        TRUE,
        &u4BufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQuery802dot11PowerSaveProfile,
        &ePowerMode,
        sizeof(ePowerMode),
        &u4BufLen);
#endif

    if (WLAN_STATUS_SUCCESS != rStatus) {
        return -EFAULT;
    }

    if (Param_PowerModeCAM == ePowerMode) {
        prPower->value = 0;
        prPower->disabled = 1;
    }
    else if (Param_PowerModeMAX_PSP == ePowerMode ) {
        prPower->value = 1;
        prPower->disabled = 0;
    }
    else if (Param_PowerModeFast_PSP == ePowerMode ) {
        prPower->value = 2;
        prPower->disabled = 0;
    }

    prPower->flags = IW_POWER_PERIOD | IW_POWER_RELATIVE;
#if WIRELESS_EXT < 21
    prPower->value *= 1000000;
#endif

    //printk(KERN_INFO "wext_get_power value(%d) disabled(%d) flag(0x%x)\n",
    //    prPower->value, prPower->disabled, prPower->flags);

    return 0;
} /* wext_get_power */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_auth (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_param *prAuth,
    IN char *pcExtra
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;

    ASSERT(prNetDev);
    ASSERT(prAuth);
    if (FALSE == GLUE_CHK_PR2(prNetDev, prAuth)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    /* Save information to glue info and process later when ssid is set. */
    switch(prAuth->flags & IW_AUTH_INDEX) {
        case IW_AUTH_WPA_VERSION:
#if SUPPORT_WAPI
            if (wlanQueryWapiMode(prGlueInfo->prAdapter)){
                prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
                prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
            }
            else {
                prGlueInfo->rWpaInfo.u4WpaVersion = prAuth->value;
            }
#else
            prGlueInfo->rWpaInfo.u4WpaVersion = prAuth->value;
#endif
            break;

        case IW_AUTH_CIPHER_PAIRWISE:
            prGlueInfo->rWpaInfo.u4CipherPairwise = prAuth->value;
            break;

        case IW_AUTH_CIPHER_GROUP:
            prGlueInfo->rWpaInfo.u4CipherGroup = prAuth->value;
            break;

        case IW_AUTH_KEY_MGMT:
            prGlueInfo->rWpaInfo.u4KeyMgmt = prAuth->value;
            break;

        case IW_AUTH_80211_AUTH_ALG:
            prGlueInfo->rWpaInfo.u4AuthAlg = prAuth->value;
            break;

        case IW_AUTH_PRIVACY_INVOKED:
            prGlueInfo->rWpaInfo.fgPrivacyInvoke = prAuth->value;
            break;
#if SUPPORT_WAPI
        case IW_AUTH_WAPI_ENABLED:
            {
                UINT_32 u4BufLen;
                WLAN_STATUS rStatus;

    #if defined(_HIF_SDIO)
                rStatus = sdio_io_ctrl(prGlueInfo,
                    wlanoidSetWapiMode,
                    &prAuth->value,
                    sizeof(UINT_32),
                    FALSE,
                    TRUE,
                    &u4BufLen);
    #else
                rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                    wlanoidSetWapiMode,
                    &prAuth->value,
                    sizeof(UINT_32),
                    &u4BufLen);
    #endif
                if (WLAN_STATUS_SUCCESS != rStatus) {
                    return -EFAULT;
                }
            }
            printk(KERN_INFO DRV_NAME "IW_AUTH_WAPI_ENABLED :%d\n", prAuth->value);
            break;
#endif
        default:
            /*
            printk(KERN_INFO "[wifi] unsupported IW_AUTH_INDEX :%d\n", prAuth->flags);
            */
            break;
    }
    return 0;
} /* wext_set_auth */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int
wext_set_encode_ext (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwrInfo,
    IN struct iw_point *prEnc,
    IN char *pcExtra
    )
{
#if SUPPORT_WAPI
    UINT_8 keyStructBuf[640];	/* add/remove key shared buffer */
#else
    UINT_8 keyStructBuf[100];   /* add/remove key shared buffer */
#endif
    P_PARAM_REMOVE_KEY_T prRemoveKey = (P_PARAM_REMOVE_KEY_T)keyStructBuf;
    P_PARAM_KEY_T prKey = (P_PARAM_KEY_T)keyStructBuf;
    struct iw_encode_ext *prIWEncExt = (struct iw_encode_ext *)pcExtra;
#if SUPPORT_WAPI
    P_PARAM_WPI_KEY_T prWpiKey = (P_PARAM_WPI_KEY_T)keyStructBuf;
#endif

    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    UINT_32 u4BufLen = 0;

    ASSERT(prNetDev);
    ASSERT(prEnc);
    if (FALSE == GLUE_CHK_PR3(prNetDev, prEnc, pcExtra)) {
        return -EINVAL;
    }
    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prNetDev);

    memset(keyStructBuf, 0, sizeof(keyStructBuf));

#if SUPPORT_WAPI
    if (prIWEncExt->alg == IW_ENCODE_ALG_SMS4) {
        if (prEnc->flags & IW_ENCODE_DISABLED) {
            //printk(KERN_INFO "[wapi] IW_ENCODE_DISABLED\n");
            return 0;
        }
        /* KeyID */
        prWpiKey->ucKeyID = (prEnc->flags & IW_ENCODE_INDEX);
        prWpiKey->ucKeyID --;
        if (prWpiKey->ucKeyID > 1) {
            /* key id is out of range */
            //printk(KERN_INFO "[wapi] add key error: key_id invalid %d\n", prWpiKey->ucKeyID);
            return -EINVAL;
        }

        if (prIWEncExt->key_len != 32) {
            /* key length not valid */
            //printk(KERN_INFO "[wapi] add key error: key_len invalid %d\n", prIWEncExt->key_len);
            return -EINVAL;
        }

        //printk(KERN_INFO "[wapi] %d ext_flags %d\n", prEnc->flags, prIWEncExt->ext_flags);

        if (prIWEncExt->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
            prWpiKey->eKeyType = ENUM_WPI_GROUP_KEY;
            prWpiKey->eDirection = ENUM_WPI_RX;
        }
        else if (prIWEncExt->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
            prWpiKey->eKeyType = ENUM_WPI_PAIRWISE_KEY;
            prWpiKey->eDirection = ENUM_WPI_RX_TX;
        }
#if SUPPORT_WAPI
        /* msg handle fianl */
        handle_sec_msg_final(prIWEncExt->key, 32, prIWEncExt->key, NULL);
#endif
        /* PN */
        memcpy(prWpiKey->aucPN, prIWEncExt->tx_seq, IW_ENCODE_SEQ_MAX_SIZE * 2);

        /* BSSID */
        memcpy(prWpiKey->aucAddrIndex, prIWEncExt->addr.sa_data, 6);

        memcpy(prWpiKey->aucWPIEK, prIWEncExt->key, 16);
        prWpiKey->u4LenWPIEK = 16;

        memcpy(prWpiKey->aucWPICK, &prIWEncExt->key[16], 16);
        prWpiKey->u4LenWPICK = 16;
        
#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
                     wlanoidSetWapiKey,
                     prWpiKey,
                     sizeof(PARAM_WPI_KEY_T),
                     FALSE,
                     TRUE,
                     &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                     wlanoidSetWapiKey,
                     prWpiKey,
                     sizeof(PARAM_WPI_KEY_T),
                     &u4BufLen);
#endif
        if (WLAN_STATUS_SUCCESS != rStatus) {
            return -EFAULT;
        }
    }
    else
#endif
    {
        /* KeyID */
        prKey->u4KeyIndex = (prEnc->flags & IW_ENCODE_INDEX) ?
            (prEnc->flags & IW_ENCODE_INDEX) -1: 0;
        if (prKey->u4KeyIndex > 3) {
            printk(KERN_INFO DRV_NAME"key index error:0x%lx\n", prKey->u4KeyIndex);
            /* key id is out of range */
            return -EINVAL;
        }

        /* bit(31) and bit(30) are shared by pKey and pRemoveKey */
        /* Tx Key Bit(31)*/
        if (prIWEncExt->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
            prKey->u4KeyIndex |= 0x1UL << 31;
        }

        /* Pairwise Key Bit(30) */
        if (prIWEncExt->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
            /* group key */
        }
        else {
            /* pairwise key */
            prKey->u4KeyIndex |= 0x1UL << 30;
        }

        if ( (prEnc->flags & IW_ENCODE_MODE) == IW_ENCODE_DISABLED) {
            prRemoveKey->u4Length = sizeof(*prRemoveKey);
            memcpy(prRemoveKey->arBSSID, prIWEncExt->addr.sa_data, 6);
            /*
            printk("IW_ENCODE_DISABLED: ID:%d, Addr:[" MACSTR "]\n",
                prRemoveKey->KeyIndex, MAC2STR(prRemoveKey->BSSID));
            */
#if defined(_HIF_SDIO)
            rStatus = sdio_io_ctrl(prGlueInfo,
                         wlanoidSetRemoveKey,
                         prRemoveKey,
                         prRemoveKey->u4Length,
                         FALSE,
                         TRUE,
                         &u4BufLen);
#else
            rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                         wlanoidSetRemoveKey,
                         prRemoveKey,
                         prRemoveKey->u4Length,
                         &u4BufLen);
#endif
            if (rStatus != WLAN_STATUS_SUCCESS) {
                printk(KERN_INFO DRV_NAME"remove key error:%lx\n", rStatus);
            }
            return 0;
        }

        /* Rx SC Bit(29) */
        if (prIWEncExt->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID) {
            prKey->u4KeyIndex |= 0x1UL << 29;
            memcpy(&prKey->rKeyRSC, prIWEncExt->rx_seq, IW_ENCODE_SEQ_MAX_SIZE);
        }

        /* BSSID */
        memcpy(prKey->arBSSID, prIWEncExt->addr.sa_data, 6);

        /* switch tx/rx MIC key for sta */
        if (prIWEncExt->alg == IW_ENCODE_ALG_TKIP && prIWEncExt->key_len == 32) {
            memcpy(prKey->aucKeyMaterial, prIWEncExt->key, 16);
            memcpy(prKey->aucKeyMaterial + 16, prIWEncExt->key + 24, 8);
            memcpy(prKey->aucKeyMaterial + 24, prIWEncExt->key + 16, 8);
        }
        else {
            memcpy(prKey->aucKeyMaterial, prIWEncExt->key, prIWEncExt->key_len);
        }

        prKey->u4KeyLength = prIWEncExt->key_len;
        prKey->u4Length = ((UINT_32)&(((P_PARAM_KEY_T)0)->aucKeyMaterial)) + prKey->u4KeyLength;


#if defined(_HIF_SDIO)
        rStatus = sdio_io_ctrl(prGlueInfo,
                     wlanoidSetAddKey,
                     prKey,
                     prKey->u4Length,
                     FALSE,
                     TRUE,
                     &u4BufLen);
#else
        rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                     wlanoidSetAddKey,
                     prKey,
                     prKey->u4Length,
                     &u4BufLen);
#endif
        if (rStatus != WLAN_STATUS_SUCCESS) {
            printk(KERN_INFO DRV_NAME"add key error:%lx\n", rStatus);
            return -EFAULT;
        }
    }

    return 0;
} /* wext_set_encode_ext */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int
wext_support_ioctl (
    IN struct net_device *prDev,
    IN struct ifreq *prIfReq,
    IN int i4Cmd
    )
{
    /* prIfReq is verified in the caller function wlanDoIOCTL() */
    struct iwreq *iwr = (struct iwreq*)prIfReq;
    struct iw_request_info rIwReqInfo;
    int ret = 0;
    char *prExtraBuf = NULL;
    UINT_32 u4ExtraSize = 0;

    /* prDev is verified in the caller function wlanDoIOCTL() */

    //printk("%d CMD:0x%x\n", jiffies_to_msecs(jiffies), i4Cmd);

    /* Prepare the call */
    rIwReqInfo.cmd = (__u16)i4Cmd;
    rIwReqInfo.flags = 0;
    	
    switch (i4Cmd) {
    case SIOCGIWNAME:   /* 0x8B01, get wireless protocol name */
        ret = wext_get_name(prDev, &rIwReqInfo, (char *)&iwr->u.name, NULL);
        break;

    /* case SIOCSIWNWID: 0x8B02, deprecated */
    /* case SIOCGIWNWID: 0x8B03, deprecated */

    case SIOCSIWFREQ:   /* 0x8B04, set channel */
        ret = wext_set_freq(prDev, NULL, &iwr->u.freq, NULL);
        break;

    case SIOCGIWFREQ:   /* 0x8B05, get channel */
        ret = wext_get_freq(prDev, NULL, &iwr->u.freq, NULL);
        break;

    case SIOCSIWMODE:   /* 0x8B06, set operation mode */
        ret = wext_set_mode(prDev, NULL, &iwr->u.mode, NULL);
        break;

    case SIOCGIWMODE:   /* 0x8B07, get operation mode */
        ret = wext_get_mode(prDev, NULL, &iwr->u.mode, NULL);
        break;

    /* case SIOCSIWSENS: 0x8B08, unsupported */
    /* case SIOCGIWSENS: 0x8B09, unsupported */

    /* case SIOCSIWRANGE: 0x8B0A, unused */
    case SIOCGIWRANGE: /* 0x8B0B, get range of parameters */
        if (iwr->u.data.pointer != NULL) {
            /* Buffer size shoule be large enough */
            if (iwr->u.data.length < sizeof(struct iw_range)) {
                ret = -E2BIG;
                break;
            }

            prExtraBuf = kalMemAlloc(sizeof(struct iw_range));
            if (!prExtraBuf) {
                ret = - ENOMEM;
                break;
            }
            
            /* prExtraBuf should be 4-byte aligned */
            ASSERT((((unsigned long)prExtraBuf) & 0x3) == 0);

            /* reset all fields */
            memset(prExtraBuf, 0, sizeof(struct iw_range));
            iwr->u.data.length = sizeof(struct iw_range);

            ret = wext_get_range(prDev, NULL, &iwr->u.data, prExtraBuf);

            /* Push up to the caller */
            if (!ret && copy_to_user(iwr->u.data.pointer,
                prExtraBuf,
                iwr->u.data.length)) {
                ret = -EFAULT;
            }

            kalMemFree(prExtraBuf, sizeof(struct iw_range));
            prExtraBuf = NULL;
        }
        else {
            ret = -EINVAL;
        }
        break;

    /* case SIOCSIWPRIV: 0x8B0C, unused */
    /* case SIOCGIWPRIV: 0x8B0D, handled in wlan_do_ioctl() */
    /* caes SIOCSIWSTATS: 0x8B0E, unused */
    /* case SIOCGIWSTATS:
            get statistics, intercepted by wireless_process_ioctl() in wireless.c,
            redirected to dev_iwstats(), dev->get_wireless_stats().
    */
    /* case SIOCSIWSPY: 0x8B10, unsupported */
    /* case SIOCGIWSPY: 0x8B11, unsupported*/
    /* case SIOCSIWTHRSPY: 0x8B12, unsupported */
    /* case SIOCGIWTHRSPY: 0x8B13, unsupported*/

    case SIOCSIWAP: /* 0x8B14, set access point MAC addresses (BSSID) */
        if (iwr->u.ap_addr.sa_data[0] == 0 &&
            iwr->u.ap_addr.sa_data[1] == 0 &&
            iwr->u.ap_addr.sa_data[2] == 0 &&
            iwr->u.ap_addr.sa_data[3] == 0 &&
            iwr->u.ap_addr.sa_data[4] == 0 &&
            iwr->u.ap_addr.sa_data[5] == 0) {
            /* WPA Supplicant will set 000000000000 in
            ** wpa_driver_wext_deinit(), do nothing here or disassoc again?
            */
            ret = 0;
            break;
        }
        else {
            ret = wext_set_ap(prDev, NULL, &iwr->u.ap_addr, NULL);
        }
        break;

    case SIOCGIWAP: /* 0x8B15, get access point MAC addresses (BSSID) */
        ret = wext_get_ap(prDev, NULL, &iwr->u.ap_addr, NULL);
        break;

    case SIOCSIWMLME: /* 0x8B16, request MLME operation */
        /* Fixed length structure */
        if (iwr->u.data.length != sizeof(struct iw_mlme)) {
            printk(KERN_INFO "MLME buffer strange:%d\n", iwr->u.data.length);
            ret = -EINVAL;
            break;
        }

        if (!iwr->u.data.pointer) {
            ret = -EINVAL;
            break;
        }

        prExtraBuf = kalMemAlloc(sizeof(struct iw_mlme));
        if (!prExtraBuf) {
            ret = - ENOMEM;
            break;
        }

        if (copy_from_user(prExtraBuf, iwr->u.data.pointer, sizeof(struct iw_mlme))) {
            ret = -EFAULT;
        }
        else {
            ret = wext_set_mlme(prDev, NULL, &(iwr->u.data), prExtraBuf);
        }

        kalMemFree(prExtraBuf, sizeof(struct iw_mlme));
        prExtraBuf = NULL;
        break;

    /* case SIOCGIWAPLIST: 0x8B17, deprecated */
    case SIOCSIWSCAN: /* 0x8B18, scan request */
	//mdy by mtk 80743

	if(iwr->u.data.pointer == NULL)
        ret = wext_set_scan(prDev, NULL, NULL, NULL);
	else if(iwr->u.data.length == sizeof(struct iw_scan_req)){	   
		prExtraBuf = kalMemAlloc(MAX_SSID_LEN);
		if(!prExtraBuf){
			ret = -ENOMEM;
			break;
	}

	if(copy_from_user(prExtraBuf, ((struct iw_scan_req*)(iwr->u.data.pointer))->essid, 
		((struct iw_scan_req*)(iwr->u.data.pointer))->essid_len)){
			ret = -EFAULT;
	}else{
		ret = wext_set_scan(prDev, NULL, &(iwr->u.data), prExtraBuf);
	}

	kalMemFree(prExtraBuf, MAX_SSID_LEN);
	prExtraBuf = NULL;
	}else
	
	ret = -EINVAL;
		
        break;

    case SIOCGIWSCAN: /* 0x8B19, get scan results */
        if (!iwr->u.data.pointer|| !iwr->u.essid.pointer) {
            ret = -EINVAL;
            break;
        }

        u4ExtraSize = iwr->u.data.length;
        /* allocate the same size of kernel buffer to store scan results. */
        prExtraBuf = kalMemAlloc(u4ExtraSize);
        if (!prExtraBuf) {
            ret = - ENOMEM;
            break;
        }

        /* iwr->u.data.length may be updated by wext_get_scan() */
        ret = wext_get_scan(prDev, NULL, &iwr->u.data, prExtraBuf);
        if (ret != 0) {
            if (ret == -E2BIG) {
                printk(KERN_INFO "[wifi] wext_get_scan -E2BIG\n");
            }
        }
        else {
            /* check updated length is valid */
            ASSERT(iwr->u.data.length <= u4ExtraSize);
            if (iwr->u.data.length > u4ExtraSize) {
                printk(KERN_WARNING DRV_NAME
                    "Updated result length is larger than allocated (%d > %ld)\n",
                    iwr->u.data.length, u4ExtraSize);
                iwr->u.data.length = u4ExtraSize;
            }

            if (copy_to_user(iwr->u.data.pointer,
                            prExtraBuf,
                            iwr->u.data.length)) {
                ret = -EFAULT;
            }
        }

        kalMemFree(prExtraBuf, u4ExtraSize);
        prExtraBuf = NULL;

        break;

    case SIOCSIWESSID: /* 0x8B1A, set SSID (network name) */
        if (iwr->u.essid.length > (IW_ESSID_MAX_SIZE + 1)) {
            ret = -E2BIG;
            break;
        }
        if (!iwr->u.essid.pointer) {
            ret = -EINVAL;
            break;
        }

        prExtraBuf = kalMemAlloc(IW_ESSID_MAX_SIZE + 4);
        if (!prExtraBuf) {
            ret = - ENOMEM;
            break;
        }

        if (copy_from_user(prExtraBuf,
                           iwr->u.essid.pointer,
                           iwr->u.essid.length)) {
            ret = -EFAULT;
        }
        else {
            /* Add trailing '\0' for printk */
            //prExtraBuf[iwr->u.essid.length] = 0;
            //printk(KERN_INFO "wext_set_essid: %s (%d)\n", prExtraBuf, iwr->u.essid.length);
            ret = wext_set_essid(prDev, NULL, &iwr->u.essid, prExtraBuf);
        }

        kalMemFree(prExtraBuf, IW_ESSID_MAX_SIZE + 4);
        prExtraBuf = NULL;
        break;

    case SIOCGIWESSID: /* 0x8B1B, get SSID */
        if (!iwr->u.essid.pointer) {
            ret = -EINVAL;
            break;
        }

        if (iwr->u.essid.length < IW_ESSID_MAX_SIZE) {
            printk(KERN_INFO "[wifi] iwr->u.essid.length:%d too small\n",
                iwr->u.essid.length);
            ret = -E2BIG;   /* let caller try larger buffer */
            break;
        }

        prExtraBuf = kalMemAlloc(IW_ESSID_MAX_SIZE);
        if (!prExtraBuf) {
            ret = -ENOMEM;
            break;
        }

        /* iwr->u.essid.length is updated by wext_get_essid() */
        ret = wext_get_essid(prDev, NULL, &iwr->u.essid, prExtraBuf);
        if (ret == 0) {
            if (copy_to_user(iwr->u.essid.pointer, prExtraBuf, iwr->u.essid.length)) {
                ret = -EFAULT;
            }
        }
        kalMemFree(prExtraBuf, IW_ESSID_MAX_SIZE);
        prExtraBuf = NULL;
        break;

    /* case SIOCSIWNICKN: 0x8B1C, not supported */
    /* case SIOCGIWNICKN: 0x8B1D, not supported */

    case SIOCSIWRATE: /* 0x8B20, set default bit rate (bps) */
        ret = wext_set_rate(prDev, &rIwReqInfo, &iwr->u.bitrate, NULL);
        break;

    case SIOCGIWRATE: /* 0x8B21, get current bit rate (bps) */
        ret = wext_get_rate(prDev, NULL, &iwr->u.bitrate, NULL);
        break;

    case SIOCSIWRTS: /* 0x8B22, set rts/cts threshold */
        ret = wext_set_rts(prDev, NULL, &iwr->u.rts, NULL);
        break;

    case SIOCGIWRTS: /* 0x8B23, get rts/cts threshold */
        ret = wext_get_rts(prDev, NULL, &iwr->u.rts, NULL);
        break;

    /* case SIOCSIWFRAG: 0x8B24, unsupported */
    case SIOCGIWFRAG: /* 0x8B25, get frag threshold */
        ret = wext_get_frag(prDev, NULL, &iwr->u.frag, NULL);
        break;

    case SIOCSIWTXPOW: /* 0x8B26, set relative tx power (in %) */
        ret = wext_set_txpow(prDev, NULL, &iwr->u.txpower, NULL);
        break;

    case SIOCGIWTXPOW: /* 0x8B27, get relative tx power (in %) */
        ret = wext_get_txpow(prDev, NULL, &iwr->u.txpower, NULL);
        break;

    /* case SIOCSIWRETRY: 0x8B28, unsupported */
    /* case SIOCGIWRETRY: 0x8B29, unsupported */

    case SIOCSIWENCODE: /* 0x8B2A, set encoding token & mode */
        /* Only DISABLED case has NULL pointer and length == 0 */
        if (iwr->u.encoding.pointer) {
            if (iwr->u.encoding.length > 16) {
                ret = -E2BIG;
                break;
            }

            u4ExtraSize = iwr->u.encoding.length;
            prExtraBuf = kalMemAlloc(u4ExtraSize);
            if (!prExtraBuf) {
                ret = -ENOMEM;
                break;
            }

            if (copy_from_user(prExtraBuf,
                                iwr->u.encoding.pointer,
                                iwr->u.encoding.length)) {
                ret = -EFAULT;
            }
        }
        else if (iwr->u.encoding.length != 0) {
            ret = -EINVAL;
            //break; /* MTK, Infinity, 20090814, Fix memory leak */
        }

        if (ret == 0) {
            ret = wext_set_encode(prDev, NULL, &iwr->u.encoding, prExtraBuf);
        }

        if (prExtraBuf) {
            kalMemFree(prExtraBuf, u4ExtraSize);
            prExtraBuf = NULL;
        }
        break;

    case SIOCGIWENCODE: /* 0x8B2B, get encoding token & mode */
        /* check pointer */
        ret = wext_get_encode(prDev, NULL, &iwr->u.encoding, NULL);
        break;

    case SIOCSIWPOWER: /* 0x8B2C, set power management */
        ret = wext_set_power(prDev, NULL, &iwr->u.power, NULL);
        break;

    case SIOCGIWPOWER: /* 0x8B2D, get power management */
        ret = wext_get_power(prDev, NULL, &iwr->u.power, NULL);
        break;

    case SIOCSIWGENIE: /* 0x8B30, set gen ie */
    	{
#if SUPPORT_WAPI
      P_GLUE_INFO_T  prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
      // Close NvRam confige WAPI function
#if 0
      if(gPlatformCfg.rWifiCustom.u4UseWapi !=0){ 
#endif
       if ((iwr->u.data.pointer) && (iwr->u.data.length != 0)) {
           u4ExtraSize = iwr->u.data.length;
           prExtraBuf = kalMemAlloc(u4ExtraSize);
				
           if (!prExtraBuf) {
               ret = -ENOMEM;
               break;
           }

           if (copy_from_user(prExtraBuf, iwr->u.data.pointer, iwr->u.data.length)) {
                    ret = -EFAULT;
                }
                else {
              /*WAPI Information Element ID*/
		if(*prExtraBuf == 0x44){			
                  WLAN_STATUS    rStatus;
                   UINT_32        u4BufLen;

		if (u4ExtraSize > 42 /* The max wapi ie buffer */) {
                    ret = -EINVAL;
                    break;
                }
		  printk(KERN_INFO "[wifi] Operate in WAPI Mode!\n");
		  wlanSetWapiMode(prGlueInfo->prAdapter, TRUE);	
    #if defined(_HIF_SDIO)
                    rStatus = sdio_io_ctrl(prGlueInfo,
                                  wlanoidSetWapiAssocInfo,
                                  prExtraBuf,
                                  u4ExtraSize,
                                  FALSE,
                                  TRUE,
                                  &u4BufLen);
    #else
                    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                                  wlanoidSetWapiAssocInfo,
                                  prExtraBuf,
                                  u4ExtraSize,
                                  &u4BufLen);
    #endif

                    if (rStatus != WLAN_STATUS_SUCCESS) {
						
                        //printk(KERN_INFO "[wapi] set wapi assoc info error:%lx\n", rStatus);
                    }
		}
		else{
			printk(KERN_INFO "[wifi] Operate in WLAN Mode!\n");
			wlanSetWapiMode(prGlueInfo->prAdapter, FALSE);		
		}
		}
                kalMemFree(prExtraBuf, u4ExtraSize);
                prExtraBuf = NULL;
		break;
            }
#if 0
    	}
#endif
#endif
    }
       break;

    case SIOCGIWGENIE: /* 0x8B31, get gen ie, unused */
        break;

    case SIOCSIWAUTH: /* 0x8B32, set auth mode params */
        ret = wext_set_auth(prDev, NULL, &iwr->u.param, NULL);
        break;

    /* case SIOCGIWAUTH: 0x8B33, unused? */
    case SIOCSIWENCODEEXT: /* 0x8B34, set extended encoding token & mode */
        if (iwr->u.encoding.pointer) {
            u4ExtraSize = iwr->u.encoding.length;
            prExtraBuf = kalMemAlloc(u4ExtraSize);
            if (!prExtraBuf) {
                ret = -ENOMEM;
                break;
            }

            if (copy_from_user(prExtraBuf,
                    iwr->u.encoding.pointer,
                    iwr->u.encoding.length)) {
                ret = -EFAULT;
            }
        }
        else if (iwr->u.encoding.length != 0) {
            ret = -EINVAL;
            break;
        }

        if (ret == 0) {
            ret = wext_set_encode_ext(prDev, NULL, &iwr->u.encoding, prExtraBuf);
        }

        if (prExtraBuf) {
            kalMemFree(prExtraBuf, u4ExtraSize);
            prExtraBuf = NULL;
        }
        break;

    /* case SIOCGIWENCODEEXT: 0x8B35, unused? */

    case SIOCSIWPMKSA: /* 0x8B36, pmksa cache operation */
        if (iwr->u.data.pointer) {
            /* Fixed length structure */
            if (iwr->u.data.length != sizeof(struct iw_pmksa)) {
                ret = -EINVAL;
                break;
            }

            u4ExtraSize = sizeof(struct iw_pmksa);
            prExtraBuf = kalMemAlloc(u4ExtraSize);
            if (!prExtraBuf) {
                ret = -ENOMEM;
                break;
            }

            if (copy_from_user(prExtraBuf,
                                iwr->u.data.pointer,
                                sizeof(struct iw_pmksa))) {
                ret = -EFAULT;
            }
            else {
                switch(((struct iw_pmksa *)prExtraBuf)->cmd) {
                case IW_PMKSA_ADD:
                    /*
                    printk(KERN_INFO "IW_PMKSA_ADD [" MACSTR "]\n",
                        MAC2STR(((struct iw_pmksa *)pExtraBuf)->bssid.sa_data));
                    */
                    {
                    P_GLUE_INFO_T  prGlueInfo =
                        (P_GLUE_INFO_T)netdev_priv(prDev);
                    WLAN_STATUS    rStatus;
                    UINT_32        u4BufLen;
                    P_PARAM_PMKID_T  prPmkid;

                    prPmkid = (P_PARAM_PMKID_T)kalMemAlloc(8 + sizeof(PARAM_BSSID_INFO_T));
                    if (!prPmkid) {
                        printk(KERN_WARNING DRV_NAME
                            "Can not alloc memory for IW_PMKSA_ADD\n");
                        ret = -ENOMEM;
                        break;
                    }

                    prPmkid->u4Length = 8 + sizeof(PARAM_BSSID_INFO_T);
                    prPmkid->u4BSSIDInfoCount = 1;
                    kalMemCopy(prPmkid->arBSSIDInfo->arBSSID,
                        ((struct iw_pmksa *)prExtraBuf)->bssid.sa_data,
                        6);
                    kalMemCopy(prPmkid->arBSSIDInfo->arPMKID,
                        ((struct iw_pmksa *)prExtraBuf)->pmkid,
                        IW_PMKID_LEN);

#if defined(_HIF_SDIO)
                    rStatus = sdio_io_ctrl(prGlueInfo,
                             wlanoidSetPmkid,
                             prPmkid,
                             sizeof(PARAM_PMKID_T),
                             FALSE,
                             TRUE,
                             &u4BufLen);
#else
                    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                                 wlanoidSetPmkid,
                                 prPmkid,
                                 sizeof(PARAM_PMKID_T),
                                 &u4BufLen);
#endif
                    if (rStatus != WLAN_STATUS_SUCCESS) {
                        printk(KERN_INFO DRV_NAME"add pmkid error:%lx\n", rStatus);
                        ret = -EFAULT;
                    }
                    kalMemFree(prPmkid, 8 + sizeof(PARAM_BSSID_INFO_T));
                    }
                    break;
                case IW_PMKSA_REMOVE:
                    /*
                    printk(KERN_INFO "IW_PMKSA_REMOVE [" MACSTR "]\n",
                        MAC2STR(((struct iw_pmksa *)buf)->bssid.sa_data));
                    */
                    break;
                case IW_PMKSA_FLUSH:
                    /*
                    printk(KERN_INFO "IW_PMKSA_FLUSH\n");
                    */
                    {
                    P_GLUE_INFO_T  prGlueInfo =
                            (P_GLUE_INFO_T)netdev_priv(prDev);
                    WLAN_STATUS    rStatus;
                    UINT_32        u4BufLen;
                    P_PARAM_PMKID_T  prPmkid;

                    prPmkid = (P_PARAM_PMKID_T)kalMemAlloc(8);
                    if (!prPmkid) {
                        printk(KERN_WARNING DRV_NAME
                            "Can not alloc memory for IW_PMKSA_FLUSH\n");
                        ret = -ENOMEM;
                        break;
                    }

                    prPmkid->u4Length = 8;
                    prPmkid->u4BSSIDInfoCount = 0;

#if defined(_HIF_SDIO)
                    rStatus = sdio_io_ctrl(prGlueInfo,
                             wlanoidSetPmkid,
                             prPmkid,
                             sizeof(PARAM_PMKID_T),
                             FALSE,
                             TRUE,
                             &u4BufLen);
#else
                    rStatus = wlanSetInformation(prGlueInfo->prAdapter,
                                 wlanoidSetPmkid,
                                 prPmkid,
                                 sizeof(PARAM_PMKID_T),
                                 &u4BufLen);
#endif
                    if (rStatus != WLAN_STATUS_SUCCESS) {
                        printk(KERN_INFO DRV_NAME
                            "flush pmkid error:%lx\n", rStatus);
                        ret = -EFAULT;
                    }
                    kalMemFree(prPmkid, 8);
                    }
                    break;
                default:
                    printk(KERN_INFO DRV_NAME"UNKNOWN iw_pmksa command:%d\n",
                        ((struct iw_pmksa *)prExtraBuf)->cmd);
                    ret = -EINVAL;
                    break;
                }
            }

            if (prExtraBuf) {
                kalMemFree(prExtraBuf, u4ExtraSize);
                prExtraBuf = NULL;
            }
        }
        else if (iwr->u.data.length != 0) {
            ret = -EINVAL;
            break;
        }
        break;

    default:
        /* printk(KERN_NOTICE "unsupported IOCTL: 0x%x\n", i4Cmd); */
        ret = -EOPNOTSUPP;
        break;
    }

    //printk("%ld CMD:0x%x ret:%d\n", jiffies_to_msecs(jiffies), i4Cmd, ret);

    return ret;
} /* wext_support_ioctl */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void
wext_indicate_wext_event (
    IN P_GLUE_INFO_T prGlueInfo,
    IN unsigned int u4Cmd,
    IN unsigned char *pucData,
    IN unsigned int u4dataLen
    )
{
    union iwreq_data wrqu;
    unsigned char *pucExtraInfo = NULL;
#if WIRELESS_EXT >= 15
    unsigned char *pucDesiredIE = NULL;
    unsigned char aucExtraInfoBuf[200];
#endif

    memset(&wrqu, 0, sizeof(wrqu));

    switch (u4Cmd) {
    case SIOCGIWTXPOW:
        memcpy(&wrqu.power, pucData, u4dataLen);
        break;
    case SIOCGIWSCAN:
        break;

    case SIOCGIWAP:
        if (pucData) {
            memcpy(&wrqu.ap_addr.sa_data, pucData, ETH_ALEN);
        }
        else {
            //prGlueInfo->fgRadioOn = FALSE;
            memset(&wrqu.ap_addr.sa_data, 0, ETH_ALEN);
        }
        break;

    case IWEVASSOCREQIE:
#if WIRELESS_EXT < 15
        /* under WE-15, no suitable Event can be used */
        goto skip_indicate_event;
#else
        /* do supplicant a favor, parse to the start of WPA/RSN IE */
        if (wextSrchDesiredWPAIE(pucData, u4dataLen, 0x30, &pucDesiredIE)) {
            /* RSN IE found */
        }
#if SUPPORT_WPS
        else if (wextSrchDesiredWPSIE(pucData, u4dataLen, 0xDD, &pucDesiredIE)) {
            /* WPS IE found */
        }
#endif
        else if (wextSrchDesiredWPAIE(pucData, u4dataLen, 0xDD, &pucDesiredIE)) {
            /* WPA IE found */
        }
        else {
            /* no WPA/RSN IE found, skip this event */
            goto skip_indicate_event;
        }

    #if WIRELESS_EXT < 18
        /* under WE-18, only IWEVCUSTOM can be used */
        u4Cmd = IWEVCUSTOM;
        pucExtraInfo = aucExtraInfoBuf;
        pucExtraInfo += sprintf(pucExtraInfo, "ASSOCINFO(ReqIEs=");
        infoElemLen = pucDesiredIE[1] + 2;
        /* printk(KERN_DEBUG "assoc info buffer size needed:%d\n", infoElemLen * 2 + 17); */
        /* translate binary string to hex string, requirement of IWEVCUSTOM */
        for (i = 0; i < infoElemLen; ++i) {
            pucExtraInfo += sprintf(pucExtraInfo, "%02x", pucDesiredIE[i]);
        }
        pucExtraInfo = aucExtraInfoBuf;
        wrqu.data.length = 17 + infoElemLen * 2;
    #else
         /* IWEVASSOCREQIE, indicate binary string */
        pucExtraInfo = pucDesiredIE;
        wrqu.data.length = pucDesiredIE[1] + 2;
    #endif
#endif  /* WIRELESS_EXT < 15 */
        break;

    case IWEVMICHAELMICFAILURE:
#if WIRELESS_EXT < 15
        /* under WE-15, no suitable Event can be used */
        goto skip_indicate_event;
#else
        if (pucData) {
            P_PARAM_AUTH_REQUEST_T pAuthReq = (P_PARAM_AUTH_REQUEST_T)pucData;
            /* under WE-18, only IWEVCUSTOM can be used */
            u4Cmd = IWEVCUSTOM;
            pucExtraInfo = aucExtraInfoBuf;
            pucExtraInfo += sprintf(pucExtraInfo,
                    "MLME-MICHAELMICFAILURE.indication ");
            pucExtraInfo += sprintf(pucExtraInfo,
                                "%s",
                                (pAuthReq->u4Flags == PARAM_AUTH_REQUEST_GROUP_ERROR) ?
                                "groupcast " : "unicast ");

            wrqu.data.length = pucExtraInfo - aucExtraInfoBuf;
            pucExtraInfo = aucExtraInfoBuf;
        }
#endif /* WIRELESS_EXT < 15 */
        break;

    case IWEVPMKIDCAND:
        if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA2 &&
            prGlueInfo->rWpaInfo.u4KeyMgmt == IW_AUTH_KEY_MGMT_802_1X) {

            /* only used in WPA2 */
#if WIRELESS_EXT >= 18
            P_PARAM_PMKID_CANDIDATE_T prPmkidCand = (P_PARAM_PMKID_CANDIDATE_T)pucData;

            struct  iw_pmkid_cand rPmkidCand;
            pucExtraInfo = aucExtraInfoBuf;

            rPmkidCand.flags = prPmkidCand->u4Flags;
            rPmkidCand.index = 0;
            kalMemCopy(rPmkidCand.bssid.sa_data, prPmkidCand->arBSSID, 6);

            kalMemCopy(pucExtraInfo, (PUINT_8)&rPmkidCand, sizeof(struct iw_pmkid_cand));
            wrqu.data.length = sizeof(struct iw_pmkid_cand);

            /* pmkid canadidate list is supported after WE-18 */
            /* indicate struct iw_pmkid_cand */
#else
            /* printk(KERN_INFO "IWEVPMKIDCAND event skipped, WE < 18\n"); */
            goto skip_indicate_event;
#endif
        }
        else {
            /* printk(KERN_INFO "IWEVPMKIDCAND event skipped, NOT WPA2\n"); */
            goto skip_indicate_event;
        }
        break;

        default:
            /* printk(KERN_INFO "Unsupported wext event:%x\n", cmd); */
            goto skip_indicate_event;
    }

    /* Send event to user space */
    wireless_send_event(prGlueInfo->prDevHandler, u4Cmd, &wrqu, pucExtraInfo);

skip_indicate_event:
    return;
} /* wext_indicate_wext_event */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
struct iw_statistics *
wext_get_wireless_stats (
    struct net_device *prDev
    )
{
    WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
    P_GLUE_INFO_T prGlueInfo = NULL;
    struct iw_statistics *pStats = NULL;
    INT_32 i4Rssi;
    UINT_32 bufLen;

    if (!prDev || !netif_carrier_ok(prDev)) {
        /* network not connected */
        goto stat_out;
    }

    prGlueInfo = (P_GLUE_INFO_T)netdev_priv(prDev);
    ASSERT(prGlueInfo);
    if (!prGlueInfo) {
        goto stat_out;
    }
    pStats = (struct iw_statistics *)(&(prGlueInfo->rIwStats));

#if defined(_HIF_SDIO)
    rStatus = sdio_io_ctrl(prGlueInfo,
        wlanoidQueryRssi,
        &i4Rssi,
        sizeof(i4Rssi),
        TRUE,
        TRUE,
        &bufLen);
#else
    rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
        wlanoidQueryRssi,
        &i4Rssi,
        sizeof(i4Rssi),
        &bufLen);
#endif

    /* no quality, no noise info */
    pStats->qual.qual = 0; /* not available */
    pStats->qual.noise = 0; /* not available */
    pStats->qual.updated = IW_QUAL_QUAL_INVALID | IW_QUAL_NOISE_INVALID;

stat_out:
    if (WLAN_STATUS_SUCCESS == rStatus) {
        pStats->qual.level = 0x100 + i4Rssi;
        pStats->qual.updated |= IW_QUAL_LEVEL_UPDATED;
    }
    else if (NULL != pStats) {
        pStats->qual.level = 0;
        pStats->qual.updated |= IW_QUAL_LEVEL_UPDATED | IW_QUAL_LEVEL_INVALID;
    }

    return pStats;
} /* wlan_get_wireless_stats */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int
wext_get_priv (
    IN struct net_device *prNetDev,
    IN struct ifreq *prIfReq
    )
{
    /* prIfReq is verified in the caller function wlanDoIOCTL() */
    struct iwreq *prIwReq = (struct iwreq *)prIfReq;
    struct iw_point *prData = (struct iw_point *)&prIwReq->u.data;
    UINT_16 u2BufferSize = 0;

    u2BufferSize = prData->length;

    /* update our private table size */
    prData->length = (__u16)sizeof(rIwPrivTable)/sizeof(struct iw_priv_args);

    if (u2BufferSize < prData->length) {
        return -E2BIG;
    }

    if (prData->length) {
        if (copy_to_user(prData->pointer, rIwPrivTable, sizeof(rIwPrivTable))) {
            return -EFAULT;
        }
    }

    return 0;
} /* wext_get_priv */

#endif /* WIRELESS_EXT */


