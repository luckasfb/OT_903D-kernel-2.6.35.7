






#include "precomp.h"

extern UINT_32 u4ScanInstCount;
extern BOOLEAN fgScanInited;


const UINT_32 au4RFInitTableForAiroha2236_24[] = {
    0x800116C7,
    0x8000B802,
    0x8000E7F3,
    0x800105A4,
    0x800F4DC5,
    0x800805B6,
    0x800A0688,
    0x800543B9,
    0x80001BBA,
    0x80000F9B,
    0x80039D8C,
    0x8008000D,
    0x8000587F
};


/* AL2236 channel table for 2.4 GHz band, 20 Mhz reference clock */
const RF_CHANNEL_PROG_ENTRY arRFChannelTableForAH2236[] = {
    {   0,  0, 0, 0, {0,0,0,0}},
    { CH_1,    2412000,     BAND_24G, 2,  { 0x8001F790, 0x80033331, 0, 0 } },
    { CH_2,    2417000,     BAND_24G, 2,  { 0x8001F790, 0x800B3331, 0, 0 } },
    { CH_3,    2422000,     BAND_24G, 2,  { 0x8001E790, 0x80033331, 0, 0 } },
    { CH_4,    2427000,     BAND_24G, 2,  { 0x8001E790, 0x800B3331, 0, 0 } },
    { CH_5,    2432000,     BAND_24G, 2,  { 0x8001F7A0, 0x80033331, 0, 0 } },
    { CH_6,    2437000,     BAND_24G, 2,  { 0x8001F7A0, 0x800B3331, 0, 0 } },
    { CH_7,    2442000,     BAND_24G, 2,  { 0x8001E7A0, 0x80033331, 0, 0 } },
    { CH_8,    2447000,     BAND_24G, 2,  { 0x8001E7A0, 0x800B3331, 0, 0 } },
    { CH_9,    2452000,     BAND_24G, 2,  { 0x8001F7B0, 0x80033331, 0, 0 } },
    { CH_10,   2457000,     BAND_24G, 2,  { 0x8001F7B0, 0x800B3331, 0, 0 } },
    { CH_11,   2462000,     BAND_24G, 2,  { 0x8001E7B0, 0x80033331, 0, 0 } },
    { CH_12,   2467000,     BAND_24G, 2,  { 0x8001E7B0, 0x800B3331, 0, 0 } },
    { CH_13,   2472000,     BAND_24G, 2,  { 0x8001F7C0, 0x80033331, 0, 0 } },
    { CH_14,   2484000,     BAND_24G, 2,  { 0x8001E7C0, 0x80066661, 0, 0 } }
};







VOID
halRFCalibrateAH2236(
    IN P_ADAPTER_T prAdapter
    );


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

VOID
halRFSetInitTable (
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    )
{
    prEEPROMCtrl->pu4RFInitTable_24 = (PUINT_32)&au4RFInitTableForAiroha2236_24[0];
    prEEPROMCtrl->u4RFInitTable_24_Size = sizeof(au4RFInitTableForAiroha2236_24) / sizeof(UINT_32);

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFInit (
    IN P_ADAPTER_T     prAdapter,
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    )
{
    UINT_32 u4Index = 0;
    BOOLEAN fgStatus = FALSE;
    
    ASSERT(prAdapter);
    ASSERT(prEEPROMCtrl);

    for (u4Index = 0; u4Index < prEEPROMCtrl->u4RFInitTable_24_Size; u4Index++) {
        HAL_MCR_WR_AND_WAIT(prAdapter, MCR_RSDR, prEEPROMCtrl->pu4RFInitTable_24[u4Index],
            RSDR_SYNT_PROG_START, 2, 5, fgStatus);

        kalUdelay(20);
    }

    /* post process for some RF init */
    halRFCalibrateAH2236(prAdapter);

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalibrateAH2236 (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32    i;
    BOOLEAN fgStatus;
    UINT_32 u4BBCR84 = 0, u4BBCR82 = 0, u4BBCR88 = 0;

    ASSERT(prAdapter);

    /* power on calibration */
    HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_RF_SX_EN);

    for (i = 0; i < 3; i++) {
        kalUdelay(50);
    }

    HAL_BBCR_RD(prAdapter, 84, &u4BBCR84);
    HAL_BBCR_RD(prAdapter, 82, &u4BBCR82);
    HAL_BBCR_RD(prAdapter, 88, &u4BBCR88);

    HAL_BBCR_WR(prAdapter, 85, 0x00);
    HAL_BBCR_WR(prAdapter, 8, 0x80);
    HAL_BBCR_WR(prAdapter, 82, 0x80);
    HAL_BBCR_WR(prAdapter, 84, u4BBCR84 | BIT(7));
    HAL_BBCR_WR(prAdapter, 88, u4BBCR88 | BIT(7));
    HAL_BBCR_WR(prAdapter, 88, u4BBCR88);

    /* TXDCOC */
    HAL_MCR_WR_AND_WAIT(prAdapter, MCR_RSDR, 0x8000D80F,
        RSDR_SYNT_PROG_START, 2, 5, fgStatus);
    kalUdelay(50);

    /* IQ imbalance */
    HAL_MCR_WR_AND_WAIT(prAdapter, MCR_RSDR, 0x8000780F,
        RSDR_SYNT_PROG_START, 2, 5, fgStatus);
    kalUdelay(50);

    HAL_MCR_WR_AND_WAIT(prAdapter, MCR_RSDR, 0x8000580F,
        RSDR_SYNT_PROG_START, 2, 5, fgStatus);

    HAL_BBCR_WR(prAdapter, 84, u4BBCR84);
    HAL_BBCR_WR(prAdapter, 82, u4BBCR82);

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_RF_CHANNEL_PROG_ENTRY
halRFGetRFChnlProgEntry (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    )
{
    UINT_32 u4Idx = 0;
    BOOLEAN fgFound = FALSE;

    for(u4Idx = 0; u4Idx < sizeof(arRFChannelTableForAH2236)/sizeof(RF_CHANNEL_PROG_ENTRY); u4Idx++) {
        if ( ucChannelNum == arRFChannelTableForAH2236[u4Idx].ucChannelNum &&
            arRFChannelTableForAH2236[u4Idx].eBand == eBand ){
            fgFound = TRUE;
            break;
        }

    }

    if (fgFound) {
        return (P_RF_CHANNEL_PROG_ENTRY)&arRFChannelTableForAH2236[u4Idx];
    } 
    else {
        return NULL;
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFSetProg (
    IN P_ADAPTER_T prAdapter,
    IN PUINT_32    pu4SynthProgWords,
    IN UINT_32     u4NumWords
    )
{
    UINT_32 u4Idx;
    BOOLEAN fgStatus = FALSE;

    for (u4Idx = 0; u4Idx < u4NumWords; u4Idx++) {
        HAL_MCR_WR_AND_WAIT(prAdapter, MCR_RSDR, pu4SynthProgWords[u4Idx],
            RSDR_SYNT_PROG_START, 2, 5, fgStatus);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
halRFSwitchChannel (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    )
{
    P_RF_CHANNEL_PROG_ENTRY prRfChnlProgEntry;

    ASSERT(prAdapter);

    if ((prRfChnlProgEntry = halRFGetRFChnlProgEntry(prAdapter, ucChannelNum, eBand)) != NULL) {
        halRFSetProg(prAdapter,
                     prRfChnlProgEntry->au4SynthProgWords,
                     prRfChnlProgEntry->u4NumSynthProgWords);
        return WLAN_STATUS_SUCCESS;
    } 
    else {
        return WLAN_STATUS_NOT_ACCEPTED;
    }

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
halSetRFSwitchChnlInst (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucChannelNum,
    IN ENUM_BAND_T eBand
    )
{
    P_RF_CHANNEL_PROG_ENTRY prChProgEntry;
    UINT_32 i;

    ASSERT(prAdapter);

    if ((prChProgEntry = halRFGetRFChnlProgEntry(prAdapter, ucChannelNum, eBand)) != NULL) {
        /* Program the synthesizer. */
        for (i = 0; i < prChProgEntry->u4NumSynthProgWords; i++) {
             HAL_HW_SCAN_SET_INST_RFCR_WR(prAdapter,
                                         prChProgEntry->au4SynthProgWords[i]);
        }
        return WLAN_STATUS_SUCCESS;
    } 
    else {
        return WLAN_STATUS_NOT_ACCEPTED;
    }

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
ENUM_RF_TYPE_T
halGetRFType (
    IN P_ADAPTER_T prAdapter
    )
{
    return RF_TYPE_AL2236;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_RF_CHANNEL_PROG_ENTRY
halRFGetRFChnlProgEntryFromChannelFreq (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4ChannelFreq
    )
{
    UINT_32 u4Idx = 0;
    BOOLEAN fgFound = FALSE;

    DEBUGFUNC("nicGetChannelBandFromFreq");

    ASSERT(prAdapter);

    for(u4Idx = 0; u4Idx < sizeof(arRFChannelTableForAH2236)/sizeof(RF_CHANNEL_PROG_ENTRY); u4Idx++) {
        if ( u4ChannelFreq == arRFChannelTableForAH2236[u4Idx].u4ChannelFreq  ){
            fgFound = TRUE;
            break;
        }

    }

    if (fgFound) {
        return (P_RF_CHANNEL_PROG_ENTRY)&arRFChannelTableForAH2236[u4Idx];
    } 
    else {
        return NULL;
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFAdoptTempChange (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_THERMO_STATE_T  rState
    )
{
 
    DEBUGFUNC("halRFAdoptTempChange");
}

