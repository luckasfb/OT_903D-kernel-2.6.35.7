


#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "CFG_Wifi_File.h"

typedef struct {
#if  !BUILD_USE_EEPROM
    MT5921_CFG_PARAM_STRUCT rWifiNvram;
    unsigned long u4Cfglen;
#endif
#if 0
    bool fgSingleAnt;
    unsigned char ucDaisyChain;   
    unsigned long u4LedSetting;
#endif
	MT5921_CUSTOM_PARAM_STRUCT rWifiCustom;
    unsigned long u4Customlen;
}WIFI_CFG_DATA, P_WIFI_CFG_FATA;



#endif /* _DOMAIN_H */


