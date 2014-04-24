





#ifndef _RF_A60105_H
#define _RF_A60105_H









ENUM_RF_TYPE_T
halGetRFType (
    IN P_ADAPTER_T prAdapter
    );

P_RF_CHANNEL_PROG_ENTRY
halRFGetRFChnlProgEntryFromChannelFreq (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4ChannelFreq
    );

VOID
halRFAdoptTempChange (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_THERMO_STATE_T  rState
    );

#endif /* _RF_A60105_H */

