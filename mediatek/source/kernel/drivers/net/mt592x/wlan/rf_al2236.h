






#ifndef _RF_AL2236_H
#define _RF_AL2236_H



typedef enum _ENUM_RF_FREQ_T {
    RF_FREQ_NUM
} ENUM_RF_FREQ_T, *P_ENUM_RF_FREQ_T;


/* NIC RF channel programming entry structure */
typedef struct _RF_CHANNEL_PROG_ENTRY {
    UINT_8      ucChannelNum;
    UINT_32     u4ChannelFreq;
    ENUM_BAND_T eBand;
    UINT_32     u4NumSynthProgWords;
    UINT_32     au4SynthProgWords[4];
} RF_CHANNEL_PROG_ENTRY, *P_RF_CHANNEL_PROG_ENTRY;





VOID
halRFInit(
    IN P_ADAPTER_T prAdapter,
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    );

VOID
halRFSetInitTable(
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    );

WLAN_STATUS
halRFSwitchChannel(
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucChannelNum,
    IN ENUM_BAND_T eBand
    );

ENUM_RF_TYPE_T
halGetRFType(
    IN P_ADAPTER_T prAdapter
    );

P_RF_CHANNEL_PROG_ENTRY
halRFGetRFChnlProgEntryFromChannelFreq(
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4ChannelFreq
    );

VOID
halRFAdoptTempChange(
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_THERMO_STATE_T  rState
    );

#endif /* _RF_AL2236_H */

