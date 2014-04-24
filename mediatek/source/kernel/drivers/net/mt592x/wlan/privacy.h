





#ifndef _PRIVACY_H
#define _PRIVACY_H



#define MAX_KEY_NUM                             4 
#define WEP_40_LEN                              5
#define WEP_104_LEN                             13
#define LEGACY_KEY_MAX_LEN                      16
#define CCMP_KEY_LEN                            16   
#define TKIP_KEY_LEN                            32
#define MAX_KEY_LEN                             32
#define MIC_RX_KEY_OFFSET                       16
#define MIC_TX_KEY_OFFSET                       24
#define MIC_KEY_LEN                             8 

#define WEP_KEY_ID_FIELD      BITS(0,29) 
#define KEY_ID_FIELD          BITS(0,7)

#define IS_TRANSMIT_KEY       BIT(31)
#define IS_UNICAST_KEY        BIT(30)
#define IS_AUTHENTICATOR      BIT(28)





VOID
privacyInitialize(
    IN P_ADAPTER_T          prAdapter
    );

VOID
privacyClearPmkid(
    IN P_ADAPTER_T          prAdapter
    );

BOOLEAN
privacyRsnKeyHandshakeEnabled(
    IN P_ADAPTER_T          prAdapter
    );

BOOLEAN
privacyTransmitKeyExist(
    IN P_ADAPTER_T          prAdapter
    );

VOID
privacyReloadDefaultWepKeys(
    IN P_ADAPTER_T          prAdapter
    );

BOOLEAN
privacyEnabled(
    IN P_ADAPTER_T          prAdapter
    );

VOID
privacySetCipherSuite(
    IN P_ADAPTER_T          prAdapter,
    IN UINT_32              u4CipherSuitesFlags
    );

WLAN_STATUS
privacySetKeyEntry(
    IN P_ADAPTER_T          prAdapter,
    IN P_PARAM_KEY_T        prNewKey
    );

VOID
privacyClearKeyEntry(
    IN P_ADAPTER_T          prAdapter,
    IN BOOLEAN              fgPairwise,
    IN PUINT_8              pucBssid,
    IN UINT_8               ucKeyId
    );


#endif /* _PRIVACY_H */

