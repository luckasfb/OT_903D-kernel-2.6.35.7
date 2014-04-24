




#ifndef _WLAN_LIB_H
#define _WLAN_LIB_H
#include "../platform.h"



#define MAX_NUM_GROUP_ADDR                      32      /* max number of group addresses */



#define TX_CS_TCP_UDP_GEN        BIT(1)
#define TX_CS_IP_GEN             BIT(0)
#if SUPPORT_WAPI
#define TX_WPI_OPEN              BIT(6) /* At WAPI, send data frame as 802.11 format and no encrypt */
#define TX_WPI_ENCRYPT           BIT(7)
#endif

#define CSUM_OFFLOAD_EN_TX_TCP      BIT(0)
#define CSUM_OFFLOAD_EN_TX_UDP      BIT(1)
#define CSUM_OFFLOAD_EN_TX_IP       BIT(2)
#define CSUM_OFFLOAD_EN_RX_TCP      BIT(3)
#define CSUM_OFFLOAD_EN_RX_UDP      BIT(4)
#define CSUM_OFFLOAD_EN_RX_IPv4     BIT(5)
#define CSUM_OFFLOAD_EN_RX_IPv6     BIT(6)
#define CSUM_OFFLOAD_EN_TX_MASK     BITS(0,2)
#define CSUM_OFFLOAD_EN_ALL         BITS(0,6)

/* TCP, UDP, IP Checksum */
#define RX_CS_TYPE_UDP           BIT(7)
#define RX_CS_TYPE_TCP           BIT(6)
#define RX_CS_TYPE_IPv6          BIT(5)
#define RX_CS_TYPE_IPv4          BIT(4)

#define RX_CS_STATUS_UDP         BIT(3)
#define RX_CS_STATUS_TCP         BIT(2)
#define RX_CS_STATUS_IP          BIT(0)

#define CSUM_NOT_SUPPORTED      0x0
typedef WLAN_STATUS (*PFN_OID_HANDLER_FUNC) (
    IN  P_ADAPTER_T prAdapter,
    IN  PVOID       pvBuf,
    IN  UINT_32     u4BufLen,
    OUT PUINT_32    pu4OutInfoLen
    );

typedef enum _ENUM_CSUM_TYPE_T {
    CSUM_TYPE_IPV4,
    CSUM_TYPE_IPV6,
    CSUM_TYPE_TCP,
    CSUM_TYPE_UDP,
    CSUM_TYPE_NUM
} ENUM_CSUM_TYPE_T, *P_ENUM_CSUM_TYPE_T;

typedef enum _ENUM_CSUM_RESULT_T {
    CSUM_RES_NONE,
    CSUM_RES_SUCCESS,
    CSUM_RES_FAILED,
    CSUM_RES_NUM
} ENUM_CSUM_RESULT_T, *P_ENUM_CSUM_RESULT_T;

#if 0
/* For storing driver initialization value from glue layer */
typedef struct _REG_INFO_T {
    UINT_32     u4SdBlockSize;      /* SDIO block size */
    UINT_32     u4SdBusWidth;       /* SDIO bus width. 1 or 4 */
    UINT_32     u4SdClockRate;      /* SDIO clock rate. (in unit of HZ) */
#if PTA_ENABLED
    UINT_32     u4BTCoexistWindowType; /*  */
    UINT_32     u4EnableTxAutoFragmentForBT; /*  */
    UINT_32     u4BtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4BtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4BtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4BtCR3;            /* BTCR3 value (correcspondent to register setting) */
#if PTA_NEW_BOARD_DESIGN
    UINT_32     u4SingleAclBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4SingleAclBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4SingleAclBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4SingleAclBtCR3;            /* BTCR3 value (correcspondent to register setting) */

    UINT_32     u4SingleMixBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4SingleMixBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4SingleMixBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4SingleMixBtCR3;            /* BTCR3 value (correcspondent to register setting) */

    UINT_32     u4DualAclBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4DualAclBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4DualAclBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4DualAclBtCR3;            /* BTCR3 value (correcspondent to register setting) */

    UINT_32     u4DualMixBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4DualMixBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4DualMixBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4DualMixBtCR3;            /* BTCR3 value (correcspondent to register setting) */

    UINT_32     u4BTSetting;        /* useless for BT only */
#else
    UINT_32     u4ScoBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4ScoBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4ScoBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4ScoBtCR3;            /* BTCR3 value (correcspondent to register setting) */
    
    UINT_32     u4AclBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4AclBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4AclBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4AclBtCR3;            /* BTCR3 value (correcspondent to register setting) */
    
    UINT_32     u4MixBtCR0;            /* BTCR0 value (correcspondent to register setting) */
    UINT_32     u4MixBtCR1;            /* BTCR1 value (correcspondent to register setting) */
    UINT_32     u4MixBtCR2;            /* BTCR2 value (correcspondent to register setting) */
    UINT_32     u4MixBtCR3;            /* BTCR3 value (correcspondent to register setting) */
#endif
    UINT_32     u4SingleAntenna;    /* 0: Not single antenna, 1: Single antenna */
    UINT_32     u4BtProfile;        /* 0: No PTA, 1: SCO, 2: A2DP(ACL), 3: See btcr0-3 */
    UINT_32     u4PtaEnabled;
#endif
    UINT_8      aucMacAddr[6];      /* MAC address */
    UINT_16     aucCountryCode[4];  /* Country code (in ISO 3166-1 expression, ex: "US", "TW")  */
    UINT_32     u4UapsdAc;          /* UAPSD AC (in bitwise OR of: a. 0x11-AC0, b. 0x22-AC1, a. 0x44-AC0, a. 0x88-AC0) */
    UINT_32     u4PowerMode;        /* Power mode (0: CAM, 1: MAX, 2: FAST) */
    UINT_32     u4AtimWindow;       /* ATIM Window value (in unit of TU) */
    UINT_32     u4VoipInterval;     /* VoIP interval (in unit of ms) */
    UINT_32     u4PollInterval;     /* Poll interval (in unit of ms) */
    UINT_32     u4PollIntervalB;    /* Poll interval, if Wi-Fi no traffic (in unit of ms) */
    UINT_32     u4L3PktFilterEn;    /* Enable pattern search for broadcast ARP filter */
    UINT_32     u4AdHocMode;        /* Adhoc mode */
    UINT_32     u4RoamingEn;        /* Enable Roaming function */
    UINT_32     u4LedBlinkMode;     /* LED blink mode (0: disable) */
#if CFG_SUPPORT_802_11D
    UINT_32     u4MultiDomainCap;   /* Multi-domain capability */
#endif
    UINT_32     u4GPIO2_Mode;       /* GPIO2 mode (0: GPI, 1: INT, 2: Daisy) */

    UINT_32     u4ViAifsnBias;      /* Define VI AIFSN BIAS (Default: 0, 0~15) */
                                        /* Actual AIFSN = AIFSN from AP + AIFSN Bias */
    UINT_32     u4ViMaxTxopLimit;   /* Define VI MAX TXOP LIMIT(A upper bound) (Default: 0xFFFF, 0~0xFFFF) */

    UINT_32     u4InitDelayInd;     /* Define how many MS to indicate DISCONNECTION after initialized */

#if SUPPORT_WAPI
    UINT_32     u4UseWapi;          /* Define use at wapi mode, control by wapi ui (Default: 0 disable, 1~0xFFFFFFFF: enable) */
#endif

} REG_INFO_T, *P_REG_INFO_T;
#endif




P_ADAPTER_T
wlanAdapterCreate (
    IN P_GLUE_INFO_T prGlueInfo
    );

VOID
wlanAdapterDestroy (
    IN P_ADAPTER_T prAdapter
    );

VOID
wlanCardEjected(
    IN P_ADAPTER_T         prAdapter
    );

WLAN_STATUS
wlanSendPacket (
    IN P_ADAPTER_T          prAdapter,
    IN P_PACKET_INFO_T      prPacketInfo
    );

VOID
wlanIST (
    IN P_ADAPTER_T prAdapter
    );

BOOL
wlanISR (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgGlobalIntrCtrl
    );

VOID
wlanReturnPacket (
    IN P_ADAPTER_T prAdapter,
    IN PVOID pvPacket
    );

WLAN_STATUS
wlanQueryInformation (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_OID_HANDLER_FUNC pfOidQryHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    OUT PUINT_32            pu4QryInfoLen
    );

WLAN_STATUS
wlanSetInformation (
    IN P_ADAPTER_T          prAdapter,
    IN PFN_OID_HANDLER_FUNC pfOidSetHandler,
    IN PVOID                pvInfoBuf,
    IN UINT_32              u4InfoBufLen,
    OUT PUINT_32            pu4SetInfoLen
    );

VOID
wlanoidSetTxStatistics (
    IN P_ADAPTER_T prAdapter
    );


VOID
wlanDoTimeOutCheck (
    IN P_ADAPTER_T  prAdapter
    );


WLAN_STATUS
wlanAdapterStart (
    IN P_ADAPTER_T  prAdapter,
    IN P_REG_INFO_T prRegInfo
    );

WLAN_STATUS
wlanAdapterStop (
    IN P_ADAPTER_T prAdapter
    );

VOID
wlanReturnRxPacket (
    IN PVOID pvAdapter,
    IN PVOID pvPacket
    );

VOID
wlanSetPromiscuousMode (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnablePromiscuousMode
    );

VOID
wlanRxSetBroadcast (
    IN P_ADAPTER_T  prAdapter,
    IN BOOLEAN      fgEnableBroadcast
    );

#if SUPPORT_WAPI
VOID
wlanSetWapiMode(
    IN P_ADAPTER_T          prAdapter,
    IN UINT_32              u4UseWapi
    );

BOOLEAN
wlanQueryWapiMode(
    IN P_ADAPTER_T          prAdapter
    );
#if 0
WLAN_STATUS
wlanoidQueryWapiCap (
    IN  P_ADAPTER_T prAdapter,
    OUT PVOID       pvQueryBuffer,
    IN  UINT_32     u4QueryBufferLen,
    OUT PUINT_32    pu4QueryInfoLen
    );
#endif
#endif

#endif /* _WLAN_LIB_H */

