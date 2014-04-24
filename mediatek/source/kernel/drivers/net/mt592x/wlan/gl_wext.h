





#ifndef _GL_WEXT_H
#define _GL_WEXT_H

#ifdef WIRELESS_EXT


#define RATE_5_5M     (11)  /* 5.5M */

typedef struct _PARAM_FIXED_IEs {
    UINT_8   aucTimestamp[8];
    UINT_16  u2BeaconInterval;
    UINT_16  u2Capabilities;
} PARAM_FIXED_IEs;

typedef struct _PARAM_VARIABLE_IE_T {
    UINT_8  ucElementID;
    UINT_8  ucLength;
    UINT_8  aucData[1];
} PARAM_VARIABLE_IE_T, *P_PARAM_VARIABLE_IE_T;

enum {
    IEEE80211_FILTER_TYPE_BEACON            = 1<<0,
    IEEE80211_FILTER_TYPE_PROBE_REQ         = 1<<1,
    IEEE80211_FILTER_TYPE_PROBE_RESP        = 1<<2,
    IEEE80211_FILTER_TYPE_ASSOC_REQ         = 1<<3,
    IEEE80211_FILTER_TYPE_ASSOC_RESP        = 1<<4,
    IEEE80211_FILTER_TYPE_AUTH              = 1<<5,
    IEEE80211_FILTER_TYPE_DEAUTH            = 1<<6,
    IEEE80211_FILTER_TYPE_DISASSOC          = 1<<7,
    IEEE80211_FILTER_TYPE_ALL               = 0xFF  /* used to check the valid filter bits */
};

#if SUPPORT_WAPI
#define IW_AUTH_WAPI_ENABLED     0x20
#define IW_ENCODE_ALG_SMS4  0x20
#endif


extern const struct iw_handler_def wext_handler_def;



/* wireless extensions' ioctls */
int
wext_support_ioctl(
    IN struct net_device *prDev,
    IN struct ifreq *prIfReq,
    IN int i4Cmd
    );

int
wext_set_rate (
    IN struct net_device *prNetDev,
    IN struct iw_request_info *prIwReqInfo,
    IN struct iw_param *prRate,
    IN char *pcExtra
    );

void
wext_indicate_wext_event(
    IN P_GLUE_INFO_T prGlueInfo,
    IN unsigned int u4Cmd,
    IN unsigned char *pucData,
    IN unsigned int u4DataLen
    );

struct iw_statistics *
wext_get_wireless_stats (
    struct net_device *prDev
    );

int
wext_get_priv (
    IN struct net_device *prNetDev,
    IN struct ifreq *prIfReq
    );


#endif /* WIRELESS_EXT */

#endif /* _GL_WEXT_H */

