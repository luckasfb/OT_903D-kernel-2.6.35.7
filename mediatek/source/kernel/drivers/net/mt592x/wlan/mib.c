






#include "precomp.h"





/* Initial values for the NIC's 802.11 MIB */
static const IEEE_802_11_MIB_T initIeee80211Mib = {

    /* dot11PrivacyTable (dot11smt 5) */
    0,                      /* dot11WEPDefaultKeyID */
    FALSE,                  /* dot11PariwiseKeyAvailable */
    0,                      /* dot11WEPICVErrorCount */
    0,                      /* dot11WEPExcludedCount */

    /*! dot11RSNAConfigTable (dot11smt 8) */
    WPA_CIPHER_SUITE_NONE,      /* dot11RSNAConfigGroupCipher */

    {
        { WPA_CIPHER_SUITE_WEP40,    FALSE },     /* WPA WEP-40         */
        { WPA_CIPHER_SUITE_TKIP,     FALSE },     /* WPA TKIP           */
        { WPA_CIPHER_SUITE_CCMP,     FALSE },     /* WPA CCMP           */
        { WPA_CIPHER_SUITE_WEP104,   FALSE },     /* WPA WEP-104        */
        { RSN_CIPHER_SUITE_WEP40,    FALSE },     /* WPA2 WEP-40        */
        { RSN_CIPHER_SUITE_TKIP,     FALSE },     /* WPA2 TKIP          */
        { RSN_CIPHER_SUITE_CCMP,     FALSE },     /* WPA2 CCMP          */
        { RSN_CIPHER_SUITE_WEP104,   FALSE }      /* WPA2 WEP-104       */
    },

    {
        { WPA_AKM_SUITE_NONE,        FALSE },    /* None        */
        { WPA_AKM_SUITE_802_1X,      FALSE },    /* 802.1X      */
        { WPA_AKM_SUITE_PSK,         FALSE },    /* PSK         */
        { RSN_AKM_SUITE_NONE,        FALSE },    /* None        */
        { RSN_AKM_SUITE_802_1X,      FALSE },    /* 802.1X      */
        { RSN_AKM_SUITE_PSK,         FALSE },    /* PSK         */
    }
};


PHY_ATTRIBUTE_T rPhyAttributes[PHY_TYPE_INDEX_NUM] = {
    {RATE_SET_ERP,          TRUE,           TRUE},  /* For PHY_TYPE_ERP_INDEX(0) */
    {RATE_SET_HR_DSSS,      TRUE,           FALSE}, /* For PHY_TYPE_HR_DSSS_INDEX(1) */
    {RATE_SET_OFDM,         FALSE,          FALSE}, /* For PHY_TYPE_OFDM_INDEX(2) */
};


ADHOC_MODE_ATTRIBUTE_T rAdHocModeAttributes[AD_HOC_MODE_NUM] = {
    {PHY_TYPE_HR_DSSS_INDEX,    BASIC_RATE_SET_HR_DSSS},        /* For AD_HOC_MODE_11B(0) */
    {PHY_TYPE_ERP_INDEX,        BASIC_RATE_SET_HR_DSSS_ERP},    /* For AD_HOC_MODE_MIXED_11BG(1) */
    {PHY_TYPE_ERP_INDEX,        BASIC_RATE_SET_ERP},            /* For AD_HOC_MODE_11G(2) */
    {PHY_TYPE_OFDM_INDEX,       BASIC_RATE_SET_OFDM},           /* For AD_HOC_MODE_11A(3) */
};




/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
mibInitializeAttributes (
    IN P_ADAPTER_T    prAdapter
    )
{
    ASSERT(prAdapter);
    
    /* Initialize the NIC's IEEE 802.11 MIB */
    kalMemCopy((PVOID)&prAdapter->rConnSettings.rMib,
        (PVOID)&initIeee80211Mib,
        sizeof(IEEE_802_11_MIB_T));

} /* mibInitializeAttributes */


