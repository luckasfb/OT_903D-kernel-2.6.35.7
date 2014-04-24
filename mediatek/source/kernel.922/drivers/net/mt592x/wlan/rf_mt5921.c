






#include "precomp.h"

extern UINT_32 u4ScanInstCount;
extern BOOLEAN fgScanInited;


#if CFG_ONLY_802_11A
RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_5G_13M[] = {
    {   0,  0, 0, 0, {0,0,0,0}},
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_5G_26M[] = {
    {   0,  0, 0, 0, {0,0,0,0}},
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_5G_20M[] = {
    {   0,  0, 0, 0, {0,0,0,0}},
    { CH_240,    4920000,     BAND_5G, 1,  { 0x45EC0000, 0, 0, 0 } },
    { CH_244,    4940000,     BAND_5G, 1,  { 0x49EE0000, 0, 0, 0 } },
    { CH_248,    4960000,     BAND_5G, 1,  { 0x4DF00000, 0, 0, 0 } },
    { CH_252,    4980000,     BAND_5G, 1,  { 0x51F20000, 0, 0, 0 } },
    { CH_8,      5040000,     BAND_5G, 1,  { 0x55F80000, 0, 0, 0 } },
    { CH_12,     5060000,     BAND_5G, 1,  { 0x59FA0000, 0, 0, 0 } },
    { CH_16,     5080000,     BAND_5G, 1,  { 0x5DFC0000, 0, 0, 0 } },
    { CH_34,     5170000,     BAND_5G, 1,  { 0x62050000, 0, 0, 0 } },
    { CH_36,     5180000,     BAND_5G, 1,  { 0x62060000, 0, 0, 0 } },
    { CH_38,     5190000,     BAND_5G, 1,  { 0x66070000, 0, 0, 0 } },
    { CH_40,     5200000,     BAND_5G, 1,  { 0x66080000, 0, 0, 0 } },
    { CH_42,     5210000,     BAND_5G, 1,  { 0x6A090000, 0, 0, 0 } },
    { CH_44,     5220000,     BAND_5G, 1,  { 0x6A0A0000, 0, 0, 0 } },
    { CH_46,     5230000,     BAND_5G, 1,  { 0x6E0B0000, 0, 0, 0 } },
    { CH_48,     5240000,     BAND_5G, 1,  { 0x6E0C0000, 0, 0, 0 } },
    { CH_52,     5260000,     BAND_5G, 1,  { 0x720E0000, 0, 0, 0 } },
    { CH_56,     5280000,     BAND_5G, 1,  { 0x76100000, 0, 0, 0 } },
    { CH_58,     5300000,     BAND_5G, 1,  { 0x7A120000, 0, 0, 0 } },
    { CH_60,     5320000,     BAND_5G, 1,  { 0x7E140000, 0, 0, 0 } },
    { CH_100,    5500000,     BAND_5G, 1,  { 0x82260000, 0, 0, 0 } },
    { CH_104,    5520000,     BAND_5G, 1,  { 0x86280000, 0, 0, 0 } },
    { CH_108,    5540000,     BAND_5G, 1,  { 0x8A2A0000, 0, 0, 0 } },
    { CH_112,    5560000,     BAND_5G, 1,  { 0x8E2C0000, 0, 0, 0 } },
    { CH_116,    5580000,     BAND_5G, 1,  { 0x922E0000, 0, 0, 0 } },
    { CH_120,    5600000,     BAND_5G, 1,  { 0x96300000, 0, 0, 0 } },
    { CH_124,    5620000,     BAND_5G, 1,  { 0x9A320000, 0, 0, 0 } },
    { CH_128,    5640000,     BAND_5G, 1,  { 0x9E340000, 0, 0, 0 } },
    { CH_132,    5660000,     BAND_5G, 1,  { 0xA2360000, 0, 0, 0 } },
    { CH_136,    5680000,     BAND_5G, 1,  { 0xA6380000, 0, 0, 0 } },
    { CH_140,    5700000,     BAND_5G, 1,  { 0xAA3A0000, 0, 0, 0 } },
    { CH_149,    5745000,     BAND_5G, 1,  { 0xAE3E8000, 0, 0, 0 } },
    { CH_153,    5765000,     BAND_5G, 1,  { 0xB2408000, 0, 0, 0 } },
    { CH_157,    5785000,     BAND_5G, 1,  { 0xB6428000, 0, 0, 0 } },
    { CH_161,    5805000,     BAND_5G, 1,  { 0xBA448000, 0, 0, 0 } },

};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_5G_40M[] = {
    {   0,  0, 0, 0, {0,0,0,0}},
    { CH_240,    4920000,     BAND_5G, 1,  { 0x45EC0000, 0, 0, 0 } },
    { CH_244,    4940000,     BAND_5G, 1,  { 0x49EE0000, 0, 0, 0 } },
    { CH_248,    4960000,     BAND_5G, 1,  { 0x4DF00000, 0, 0, 0 } },
    { CH_252,    4980000,     BAND_5G, 1,  { 0x51F20000, 0, 0, 0 } },
    { CH_8,      5040000,     BAND_5G, 1,  { 0x55F80000, 0, 0, 0 } },
    { CH_12,     5060000,     BAND_5G, 1,  { 0x59FA0000, 0, 0, 0 } },
    { CH_16,     5080000,     BAND_5G, 1,  { 0x5DFC0000, 0, 0, 0 } },
    { CH_34,     5170000,     BAND_5G, 1,  { 0x62050000, 0, 0, 0 } },
    { CH_36,     5180000,     BAND_5G, 1,  { 0x62060000, 0, 0, 0 } },
    { CH_38,     5190000,     BAND_5G, 1,  { 0x66070000, 0, 0, 0 } },
    { CH_40,     5200000,     BAND_5G, 1,  { 0x66080000, 0, 0, 0 } },
    { CH_42,     5210000,     BAND_5G, 1,  { 0x6A090000, 0, 0, 0 } },
    { CH_44,     5220000,     BAND_5G, 1,  { 0x6A0A0000, 0, 0, 0 } },
    { CH_46,     5230000,     BAND_5G, 1,  { 0x6E0B0000, 0, 0, 0 } },
    { CH_48,     5240000,     BAND_5G, 1,  { 0x6E0C0000, 0, 0, 0 } },
    { CH_52,     5260000,     BAND_5G, 1,  { 0x720E0000, 0, 0, 0 } },
    { CH_56,     5280000,     BAND_5G, 1,  { 0x76100000, 0, 0, 0 } },
    { CH_58,     5300000,     BAND_5G, 1,  { 0x7A120000, 0, 0, 0 } },
    { CH_60,     5320000,     BAND_5G, 1,  { 0x7E140000, 0, 0, 0 } },
    { CH_100,    5500000,     BAND_5G, 1,  { 0x82260000, 0, 0, 0 } },
    { CH_104,    5520000,     BAND_5G, 1,  { 0x86280000, 0, 0, 0 } },
    { CH_108,    5540000,     BAND_5G, 1,  { 0x8A2A0000, 0, 0, 0 } },
    { CH_112,    5560000,     BAND_5G, 1,  { 0x8E2C0000, 0, 0, 0 } },
    { CH_116,    5580000,     BAND_5G, 1,  { 0x922E0000, 0, 0, 0 } },
    { CH_120,    5600000,     BAND_5G, 1,  { 0x96300000, 0, 0, 0 } },
    { CH_124,    5620000,     BAND_5G, 1,  { 0x9A320000, 0, 0, 0 } },
    { CH_128,    5640000,     BAND_5G, 1,  { 0x9E340000, 0, 0, 0 } },
    { CH_132,    5660000,     BAND_5G, 1,  { 0xA2360000, 0, 0, 0 } },
    { CH_136,    5680000,     BAND_5G, 1,  { 0xA6380000, 0, 0, 0 } },
    { CH_140,    5700000,     BAND_5G, 1,  { 0xAA3A0000, 0, 0, 0 } },
    { CH_149,    5745000,     BAND_5G, 1,  { 0xAE3E8000, 0, 0, 0 } },
    { CH_153,    5765000,     BAND_5G, 1,  { 0xB2408000, 0, 0, 0 } },
    { CH_157,    5785000,     BAND_5G, 1,  { 0xB6428000, 0, 0, 0 } },
    { CH_161,    5805000,     BAND_5G, 1,  { 0xBA448000, 0, 0, 0 } }
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_5G_19_2M[] = {
    {   0,  0, 0, 0, {0,0,0,0}},
};

/* Notice the map to the ENUM_RF_FREQ_T */
TABLE_RF_ENTRY_T arRFInitTableByFreq[] = {
    {arRFChannelTableForA60105_5G_20M, sizeof(arRFChannelTableForA60105_5G_20M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_5G_26M, sizeof(arRFChannelTableForA60105_5G_26M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_5G_13M, sizeof(arRFChannelTableForA60105_5G_13M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_5G_40M, sizeof(arRFChannelTableForA60105_5G_40M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_5G_19_2M, sizeof(arRFChannelTableForA60105_5G_19_2M)/sizeof(RF_CHANNEL_PROG_ENTRY)}
};
#endif


/* MTK A60105 channel table for 2.4 GHz band, 20 Mhz reference clock */
RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_13M[] = {
    {    0,          0,(ENUM_BAND_T)0, 0, {          0, 0, 0, 0 } },
    { CH_1,    2412000,     BAND_24G, 1,  { 0x0AE62762, 0, 0, 0 } },
    { CH_2,    2417000,     BAND_24G, 1,  { 0x0EE7B13B, 0, 0, 0 } },
    { CH_3,    2422000,     BAND_24G, 1,  { 0x12E93B13, 0, 0, 0 } },
    { CH_4,    2427000,     BAND_24G, 1,  { 0x16EAC4EC, 0, 0, 0 } },
    { CH_5,    2432000,     BAND_24G, 1,  { 0x1AEC4EC4, 0, 0, 0 } },
    { CH_6,    2437000,     BAND_24G, 1,  { 0x1EEDD89D, 0, 0, 0 } },
    { CH_7,    2442000,     BAND_24G, 1,  { 0x22EF6276, 0, 0, 0 } },
    { CH_8,    2447000,     BAND_24G, 1,  { 0x26F0EC4E, 0, 0, 0 } },
    { CH_9,    2452000,     BAND_24G, 1,  { 0x2AF27627, 0, 0, 0 } },
    { CH_10,   2457000,     BAND_24G, 1,  { 0x2EF40000, 0, 0, 0 } },
    { CH_11,   2462000,     BAND_24G, 1,  { 0x32F589D8, 0, 0, 0 } },
    { CH_12,   2467000,     BAND_24G, 1,  { 0x36F713B1, 0, 0, 0 } },
    { CH_13,   2472000,     BAND_24G, 1,  { 0x3AF89D89, 0, 0, 0 } },
    { CH_14,   2484000,     BAND_24G, 1,  { 0x42FC4EC4, 0, 0, 0 } }
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_26M[] = {
    {    0,          0,(ENUM_BAND_T)0, 0, {          0, 0, 0, 0 } },
    { CH_1,    2412000,     BAND_24G, 1,  { 0x0AE62762, 0, 0, 0 } },
    { CH_2,    2417000,     BAND_24G, 1,  { 0x0EE7B13B, 0, 0, 0 } },
    { CH_3,    2422000,     BAND_24G, 1,  { 0x12E93B13, 0, 0, 0 } },
    { CH_4,    2427000,     BAND_24G, 1,  { 0x16EAC4EC, 0, 0, 0 } },
    { CH_5,    2432000,     BAND_24G, 1,  { 0x1AEC4EC4, 0, 0, 0 } },
    { CH_6,    2437000,     BAND_24G, 1,  { 0x1EEDD89D, 0, 0, 0 } },
    { CH_7,    2442000,     BAND_24G, 1,  { 0x22EF6276, 0, 0, 0 } },
    { CH_8,    2447000,     BAND_24G, 1,  { 0x26F0EC4E, 0, 0, 0 } },
    { CH_9,    2452000,     BAND_24G, 1,  { 0x2AF27627, 0, 0, 0 } },
    { CH_10,   2457000,     BAND_24G, 1,  { 0x2EF40000, 0, 0, 0 } },
    { CH_11,   2462000,     BAND_24G, 1,  { 0x32F589D8, 0, 0, 0 } },
    { CH_12,   2467000,     BAND_24G, 1,  { 0x36F713B1, 0, 0, 0 } },
    { CH_13,   2472000,     BAND_24G, 1,  { 0x3AF89D89, 0, 0, 0 } },
    { CH_14,   2484000,     BAND_24G, 1,  { 0x42FC4EC4, 0, 0, 0 } }
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_20M[] = {
    {    0,          0,(ENUM_BAND_T)0, 0, {          0, 0, 0, 0 } },
    { CH_1,    2412000,     BAND_24G, 1,  { 0x09E26666, 0, 0, 0 } },
    { CH_2,    2417000,     BAND_24G, 1,  { 0x0DE36666, 0, 0, 0 } },
    { CH_3,    2422000,     BAND_24G, 1,  { 0x11E46666, 0, 0, 0 } },
    { CH_4,    2427000,     BAND_24G, 1,  { 0x15E56666, 0, 0, 0 } },
    { CH_5,    2432000,     BAND_24G, 1,  { 0x19E66666, 0, 0, 0 } },
    { CH_6,    2437000,     BAND_24G, 1,  { 0x1DE76666, 0, 0, 0 } },
    { CH_7,    2442000,     BAND_24G, 1,  { 0x21E86666, 0, 0, 0 } },
    { CH_8,    2447000,     BAND_24G, 1,  { 0x25E96666, 0, 0, 0 } },
    { CH_9,    2452000,     BAND_24G, 1,  { 0x29EA6666, 0, 0, 0 } },
    { CH_10,   2457000,     BAND_24G, 1,  { 0x2DEB6666, 0, 0, 0 } },
    { CH_11,   2462000,     BAND_24G, 1,  { 0x31EC6666, 0, 0, 0 } },
    { CH_12,   2467000,     BAND_24G, 1,  { 0x35ED6666, 0, 0, 0 } },
    { CH_13,   2472000,     BAND_24G, 1,  { 0x39EE6666, 0, 0, 0 } },
    { CH_14,   2484000,     BAND_24G, 1,  { 0x41F0CCCC, 0, 0, 0 } }
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_40M[] = {
    {    0,          0,(ENUM_BAND_T)0, 0, {          0, 0, 0, 0 } },
    { CH_1,    2412000,     BAND_24G, 1,  { 0x09E26666, 0, 0, 0 } },
    { CH_2,    2417000,     BAND_24G, 1,  { 0x0DE36666, 0, 0, 0 } },
    { CH_3,    2422000,     BAND_24G, 1,  { 0x11E46666, 0, 0, 0 } },
    { CH_4,    2427000,     BAND_24G, 1,  { 0x15E56666, 0, 0, 0 } },
    { CH_5,    2432000,     BAND_24G, 1,  { 0x19E66666, 0, 0, 0 } },
    { CH_6,    2437000,     BAND_24G, 1,  { 0x1DE76666, 0, 0, 0 } },
    { CH_7,    2442000,     BAND_24G, 1,  { 0x21E86666, 0, 0, 0 } },
    { CH_8,    2447000,     BAND_24G, 1,  { 0x25E96666, 0, 0, 0 } },
    { CH_9,    2452000,     BAND_24G, 1,  { 0x29EA6666, 0, 0, 0 } },
    { CH_10,   2457000,     BAND_24G, 1,  { 0x2DEB6666, 0, 0, 0 } },
    { CH_11,   2462000,     BAND_24G, 1,  { 0x31EC6666, 0, 0, 0 } },
    { CH_12,   2467000,     BAND_24G, 1,  { 0x35ED6666, 0, 0, 0 } },
    { CH_13,   2472000,     BAND_24G, 1,  { 0x39EE6666, 0, 0, 0 } },
    { CH_14,   2484000,     BAND_24G, 1,  { 0x41F0CCCC, 0, 0, 0 } }
};

RF_CHANNEL_PROG_ENTRY arRFChannelTableForA60105_19_2M[] = {
    {    0,          0,(ENUM_BAND_T)0, 0, {          0, 0, 0, 0 } },
};

/* Notice the map to the ENUM_RF_FREQ_T */
TABLE_RF_ENTRY_T arRFInitTableByFreq[] = {
    {arRFChannelTableForA60105_20M, sizeof(arRFChannelTableForA60105_20M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_26M, sizeof(arRFChannelTableForA60105_26M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_13M, sizeof(arRFChannelTableForA60105_13M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_40M, sizeof(arRFChannelTableForA60105_40M)/sizeof(RF_CHANNEL_PROG_ENTRY)},
    {arRFChannelTableForA60105_19_2M, sizeof(arRFChannelTableForA60105_19_2M)/sizeof(RF_CHANNEL_PROG_ENTRY)}
};


/* High Temperature setting (70)*/
static REG_ENTRY_T arHighTempSetting[] = {
    {   RFCR_10, 0x49279E1F},
    {   RFCR_9,  0x00650000},
};

/* Normal Temperature setting (25)*/
static REG_ENTRY_T arNormalTempSetting[] = {
    {   RFCR_10, 0x49279E1F},
    {   RFCR_9,  0x00640000},
};

/* Low Temperature setting (-20)*/
static REG_ENTRY_T arLowTempSetting[] = {
    {   RFCR_10, 0x49279E07},
    {   RFCR_9,  0x00640000},
};






VOID
halRFCal60105_2412_2484(
    IN P_ADAPTER_T prAdapter
    );

#if CFG_ONLY_802_11A
VOID
halRFCal60105_5G(
    IN P_ADAPTER_T prAdapter
    );
#endif

VOID
halRFCalPreStep(
    IN P_ADAPTER_T prAdapter
    );

VOID
halRFCalPostStep(
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucFreqRange
    );


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFInit (
    IN P_ADAPTER_T     prAdapter,
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    )
{
    UINT_32 u4RFCR1 = 0;
    UINT_32 u4tmp, u4RFCR4 = 0;
    UINT_32 u4bbcr1;
    UINT_32 u4cr0tmp, u4cr12tmp, u4cr6tmp;
    UINT_32 u4cr7tmp, u4cr2tmp;
    UINT_32 u4mcr98tmp, u4mcr60tmp;

    DEBUGFUNC("halRFInit");

    ASSERT(prAdapter);
    ASSERT(prEEPROMCtrl);

    //HAL_MCR_WR(prAdapter, MCR_HLPCR, HLPCR_RF_SX_EN | HLPCR_ADC_BUFFER_EN);

#if CFG_ONLY_802_11A
    HAL_MCR_RD(prAdapter,  MCR_ACDR4, &u4tmp);
    switch ((u4tmp & ACDR4_RG_DIVS) >> 16) {
    case ACDR4_RG_DIVS_20M:
        prAdapter->eRfFreqSet = RF_FREQ_20M;
        u4RFCR1 = 0x701990B6;
        u4RFCR4 = 0x080301DC;
        break;
    case ACDR4_RG_DIVS_40M:
        prAdapter->eRfFreqSet = RF_FREQ_40M;
        u4RFCR1 = 0x701D90B6;
        u4RFCR4 = 0x080301DD;
        break;
    case ACDR4_RG_DIVS_26M:
        prAdapter->eRfFreqSet = RF_FREQ_26M;
        u4RFCR1 = 0x701D90B6;
        u4RFCR4 = 0x080301D9;
        break;
    case ACDR4_RG_DIVS_13M:
        prAdapter->eRfFreqSet = RF_FREQ_13M;
        u4RFCR1 = 0x701990B6;
        u4RFCR4 = 0x080301D8;
        break;
    case ACDR4_RG_DIVS_19_2M:
        prAdapter->eRfFreqSet = RF_FREQ_19_2M;
        u4RFCR1 = 0x701990B6;
        u4RFCR4 = 0x080301DB;
        break;
    default:
        ASSERT(0);
    }

    HAL_MCR_WR(prAdapter, RFCR_1, 0xF01990B6);
    HAL_MCR_WR(prAdapter, RFCR_0, 0x9B6D509C);
    HAL_MCR_WR(prAdapter, RFCR_1, u4RFCR1);
    HAL_MCR_WR(prAdapter, RFCR_2, 0x4068F349);
    HAL_MCR_WR(prAdapter, RFCR_6, 0x0524927C);
    HAL_MCR_WR(prAdapter, RFCR_7, 0x7E7E293F);
    HAL_MCR_WR(prAdapter, RFCR_4, u4RFCR4);
    HAL_MCR_WR(prAdapter, RFCR_11, 0x92490600);
    HAL_MCR_WR(prAdapter, RFCR_10, 0xF927FFFF);

#else
    HAL_MCR_RD(prAdapter,  MCR_ACDR4, &u4tmp);
    switch ((u4tmp & ACDR4_RG_DIVS) >> 16) {
    case ACDR4_RG_DIVS_20M:
        prAdapter->eRfFreqSet = RF_FREQ_20M;
        u4RFCR1 = 0x701991B6;
        u4RFCR4 = 0x080301DC;
        break;
    case ACDR4_RG_DIVS_40M:
        prAdapter->eRfFreqSet = RF_FREQ_40M;
        u4RFCR1 = 0x701D91B6;
        u4RFCR4 = 0x080301DD;
        break;
    case ACDR4_RG_DIVS_26M:
        prAdapter->eRfFreqSet = RF_FREQ_26M;
        u4RFCR1 = 0x701D91B6;
        u4RFCR4 = 0x080301D9;
        break;
    case ACDR4_RG_DIVS_13M:
        prAdapter->eRfFreqSet = RF_FREQ_13M;
        u4RFCR1 = 0x701991B6;
        u4RFCR4 = 0x080301D8;
        break;
    case ACDR4_RG_DIVS_19_2M:
        prAdapter->eRfFreqSet = RF_FREQ_19_2M;
        ASSERT(0);
        break;
    default:
        ASSERT(0);
    }

    /* Write the per-band RF initial table to the RF chipset. */
    /* BG band */
    HAL_MCR_WR(prAdapter, RFCR_1, 0xF01990B6);
    HAL_MCR_WR(prAdapter, RFCR_0, 0x9B6D10BC);
    HAL_MCR_WR(prAdapter, RFCR_1, u4RFCR1);
    HAL_MCR_WR(prAdapter, RFCR_2, 0x42680349);
    HAL_MCR_WR(prAdapter, RFCR_6, 0x0524923B);
    HAL_MCR_WR(prAdapter, RFCR_7, 0x7E7E293F);

    /* RFCR4_XO_TRIM_MASK use setting from EEPROM */
    u4RFCR4 = (u4RFCR4 &~ RFCR4_XO_TRIM_MASK) | \
              ((UINT_32)((prEEPROMCtrl->ucXtalTrim)<<RFCR4_XO_TRIM_OFFSET) \
               & RFCR4_XO_TRIM_MASK ) ;
    HAL_MCR_WR(prAdapter, RFCR_4, u4RFCR4);
    HAL_MCR_WR(prAdapter, RFCR_13, 0x02114238);
    HAL_MCR_WR(prAdapter, RFCR_9, 0x00640000);
    HAL_MCR_WR(prAdapter, RFCR_12, 0xFFF04387);
    HAL_MCR_WR(prAdapter, RFCR_10, 0x49279e1f);

#endif

    kalUdelay(40);

#if CFG_ONLY_802_11A
    {
        UINT_32     u4RegValue;

        /* Keep antenna switch setting for single antenna application */
        /* Note: MCR_RFICR is MCR_RICR in MT5921 IC */
        HAL_MCR_RD(prAdapter, MCR_RFICR, &u4RegValue);
        u4RegValue &= (RICR_ANT_SEL_P_SW_MODE | RICR_ANT_SEL_N_SW_MODE |
                   RICR_SWANT_SEL_P_HIGH | RICR_SWANT_SEL_N_HIGH);
        u4RegValue |= 0x001A0F18;
        HAL_MCR_WR(prAdapter, MCR_RFICR, u4RegValue);
    }
#else
    {
        UINT_32     u4RegValue;

        /* Keep antenna switch setting for single antenna application */
        /* Note: MCR_RFICR is MCR_RICR in MT5921 IC */
        HAL_MCR_RD(prAdapter, MCR_RFICR, &u4RegValue);
        u4RegValue &= (RICR_ANT_SEL_P_SW_MODE | RICR_ANT_SEL_N_SW_MODE |
                   RICR_SWANT_SEL_P_HIGH | RICR_SWANT_SEL_N_HIGH);
        u4RegValue |= 0x001C0F18;
        HAL_MCR_WR(prAdapter, MCR_RFICR, u4RegValue);
    }
#endif

    //4 Initialize default channel to operational channel index 0
    nicSwitchChannel(prAdapter,
                     prAdapter->arNicOpChnList[0].eBand,
                     prAdapter->arNicOpChnList[0].ucChannelNum,
                     CFG_INIT_TX_POWER_LIMIT);

    //Before TX DC/gain/phase calibration
    HAL_MCR_RD(prAdapter, RFCR_0, &u4cr0tmp)
    HAL_MCR_RD(prAdapter, RFCR_2, &u4cr2tmp)
    HAL_MCR_RD(prAdapter, RFCR_6, &u4cr6tmp)
    HAL_MCR_RD(prAdapter, RFCR_7, &u4cr7tmp)
    HAL_MCR_RD(prAdapter, RFCR_12, &u4cr12tmp)
    DBGLOG(PHY, TRACE,("RFCR0 is read as: 0x%lx\n", u4cr0tmp));
    DBGLOG(PHY, TRACE,("RFCR2 is read as: 0x%lx\n", u4cr2tmp));
    DBGLOG(PHY, TRACE,("RFCR6 is read as: 0x%lx\n", u4cr6tmp));
    DBGLOG(PHY, TRACE,("RFCR7 is read as: 0x%lx\n", u4cr7tmp));
    DBGLOG(PHY, TRACE,("RFCR12 is read as: 0x%lx\n", u4cr12tmp));

    HAL_BBCR_RD(prAdapter, 1, &u4bbcr1);

    halRFCalPreStep(prAdapter);

#if CFG_ONLY_802_11A
    {
        UINT_32     u4RegValue;

        /* Keep antenna switch setting for single antenna application */
        /* Note: MCR_RFICR is MCR_RICR in MT5921 IC */
        HAL_MCR_RD(prAdapter, MCR_RFICR, &u4RegValue);
        u4RegValue &= (RICR_ANT_SEL_P_SW_MODE | RICR_ANT_SEL_N_SW_MODE |
                   RICR_SWANT_SEL_P_HIGH | RICR_SWANT_SEL_N_HIGH);
        u4RegValue |= 0x001A0E00;
        HAL_MCR_WR(prAdapter, MCR_RFICR, u4RegValue);
    }
#else
    {
        UINT_32     u4RegValue;

        /* Keep antenna switch setting for single antenna application */
        /* Note: MCR_RFICR is MCR_RICR in MT5921 IC */
        HAL_MCR_RD(prAdapter, MCR_RFICR, &u4RegValue);
        u4RegValue &= (RICR_ANT_SEL_P_SW_MODE | RICR_ANT_SEL_N_SW_MODE |
                   RICR_SWANT_SEL_P_HIGH | RICR_SWANT_SEL_N_HIGH);
        u4RegValue |= 0x001C0E00;
        HAL_MCR_WR(prAdapter, MCR_RFICR, u4RegValue);
    }
#endif

    HAL_MCR_RD(prAdapter, MCR_RICR, &u4mcr98tmp);
    DBGLOG(PHY, TRACE,("MCR 98 is read as: 0x%lx\n", u4mcr98tmp));
    HAL_MCR_RD(prAdapter, MCR_CCR, &u4mcr60tmp);
    DBGLOG(PHY, TRACE,("MCR 60 is read as: 0x%lx\n", u4mcr60tmp));

#if CFG_ONLY_802_11A
    halRFCal60105_5G(prAdapter);
#else
    halRFCal60105_2412_2484(prAdapter);
#endif

    //Restore
    HAL_MCR_WR(prAdapter, RFCR_0, u4cr0tmp)
    DBGLOG(PHY, TRACE, ("The RFCR 0 store as 0x%08lx\n", u4cr0tmp));
    HAL_MCR_WR(prAdapter, RFCR_2, u4cr2tmp)
    DBGLOG(PHY, TRACE, ("The RFCR 2 store as 0x%08lx\n", u4cr2tmp));
    HAL_MCR_WR(prAdapter, RFCR_6, u4cr6tmp)
    DBGLOG(PHY, TRACE, ("The RFCR 6 store as 0x%08lx\n", u4cr6tmp));
    HAL_MCR_WR(prAdapter, RFCR_7, u4cr7tmp)
    DBGLOG(PHY, TRACE, ("The RFCR 7 store as 0x%08lx\n", u4cr7tmp));
    HAL_MCR_WR(prAdapter, RFCR_12, u4cr12tmp)
    DBGLOG(PHY, TRACE, ("The RFCR 12 store as 0x%08lx\n", u4cr12tmp));

    HAL_MCR_WR(prAdapter, MCR_RICR, u4mcr98tmp)
    DBGLOG(PHY, TRACE, ("The MCR_RICR store as 0x%08lx\n", u4mcr98tmp));

    HAL_MCR_WR(prAdapter, MCR_CCR, u4mcr60tmp)
    DBGLOG(PHY, TRACE, ("The MCR_CCR store as 0x%08lx\n", u4mcr60tmp));

    HAL_BBCR_WR(prAdapter, 1, u4bbcr1);

#if CFG_ONLY_802_11A
    halRFCalPostStep(prAdapter, 1);
#else
    halRFCalPostStep(prAdapter, 0);
#endif
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalPreStep (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4tmp;

    DEBUGFUNC("halRFCalPreStep");
    DBGLOG(PHY, TRACE,("\n"));

    ASSERT(prAdapter);

    /* T2 */
    DBGLOG(PHY, TRACE,("T2 Phase\n"));
    HAL_MCR_RD(prAdapter, RFCR_0, &u4tmp);
    u4tmp = (u4tmp & (~BITS(2, 5))) | BITS(3,4);
    HAL_MCR_WR(prAdapter, RFCR_0, u4tmp);
    DBGLOG(PHY, TRACE,("RFCR 0 is writen as: 0x%lx\n", u4tmp));
    kalUdelay(40);

    /* T3 */
    DBGLOG(PHY, TRACE,("T3 Phase\n"));
    HAL_MCR_RD(prAdapter, RFCR_0, &u4tmp);
    u4tmp = u4tmp | BIT(15);
    HAL_MCR_WR(prAdapter, RFCR_0, u4tmp);
    DBGLOG(PHY, TRACE,("RFCR 0 is writen as: 0x%lx\n", u4tmp));
    kalUdelay(40);

    /* T4 */
    DBGLOG(PHY, TRACE,("T4 Phase\n"));
    HAL_MCR_RD(prAdapter, RFCR_0, &u4tmp);
    u4tmp = (u4tmp & (~BITS(2, 5))) | BIT(3);
    HAL_MCR_WR(prAdapter, RFCR_0, u4tmp);
    DBGLOG(PHY, TRACE,("RFCR 0 is writen as: 0x%lx\n", u4tmp));
    kalUdelay(80);

    /* T5 */
    DBGLOG(PHY, TRACE,("T5 Phase\n"));
    HAL_MCR_RD(prAdapter, RFCR_2, &u4tmp);
    u4tmp = u4tmp | BIT(31);
    HAL_MCR_WR(prAdapter, RFCR_2, u4tmp);
    DBGLOG(PHY, TRACE,("RFCR 2 is writen as: 0x%lx\n", u4tmp));
    kalMdelay(7);
    kalUdelay(500);

}   /* halRFCalPreStep */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalTxSet (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucFreqRange
    )
{
    UINT_32     u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_WR(prAdapter, RFCR_0,  0x9B6D1353);
    HAL_MCR_WR(prAdapter, RFCR_6,  0x05249E3B);
    HAL_MCR_WR(prAdapter, RFCR_12, 0xF7004087);

    //FORCE RXHP high
    HAL_BBCR_WR(prAdapter, 97, 0x0008);
    HAL_BBCR_WR(prAdapter, 98, 0x0080);

    HAL_BBCR_WR(prAdapter, 1, 0x8601);

#if 1
    /* Keep antenna switch setting for single antenna application */
    HAL_MCR_RD(prAdapter, MCR_RICR, &u4RegValue);
    u4RegValue &= (RICR_ANT_SEL_P_SW_MODE | RICR_ANT_SEL_N_SW_MODE |
                   RICR_SWANT_SEL_P_HIGH | RICR_SWANT_SEL_N_HIGH);
    u4RegValue |= 0x00180100;
    HAL_MCR_WR(prAdapter, MCR_RICR, u4RegValue);
#else
    HAL_MCR_WR(prAdapter, MCR_RICR, 0x00180100);
#endif
    HAL_MCR_WR(prAdapter, MCR_CCR,  0xE0200100);

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalRxSet (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucFreqRange
    )
{
    UINT_32     u4RegValue;

    ASSERT(prAdapter);

    HAL_MCR_WR(prAdapter, RFCR_0,  0x9B6D1357);
    HAL_MCR_WR(prAdapter, RFCR_6,  0x05249E3B);
    HAL_MCR_WR(prAdapter, RFCR_12, 0xF6404087);

    //FORCE RXHP high
    HAL_BBCR_WR(prAdapter, 97, 0x0008);
    HAL_BBCR_WR(prAdapter, 98, 0x0080);

    HAL_BBCR_WR(prAdapter, 1, 0x0601);

#if 1
    /* Keep antenna switch setting for single antenna application */
    HAL_MCR_RD(prAdapter, MCR_RICR, &u4RegValue);
    u4RegValue &= (RICR_ANT_SEL_P_SW_MODE | RICR_ANT_SEL_N_SW_MODE |
                   RICR_SWANT_SEL_P_HIGH | RICR_SWANT_SEL_N_HIGH);
    u4RegValue |= 0x00180100;
    HAL_MCR_WR(prAdapter, MCR_RICR, u4RegValue);
#else
    HAL_MCR_WR(prAdapter, MCR_RICR, 0x00180100);
#endif
    HAL_MCR_WR(prAdapter, MCR_CCR,  0xE0200100);

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalTxIDC (
    IN P_ADAPTER_T     prAdapter
    )
{
    UINT_8             temp8;
    UINT_32            u4tmp, loopcount;
    INT_32             result;

    DEBUGFUNC("halRFCalTxIDC");

    ASSERT(prAdapter);

    HAL_BBCR_WR(prAdapter, 77, 0xc180);

    kalMdelay(1);
    loopcount = 0;
    /* wait result */
    do {
        /* read BB CR 78 */
        HAL_BBCR_RD(prAdapter, 78, &u4tmp);
        temp8 = (UINT_8)u4tmp;

        if ((temp8 & 0x80) != 0) {
            temp8 = temp8 & 0x7f;
            result = 0;
            break;
        } else {
            loopcount ++;
            result = -1;
        }
    } while (loopcount < 10);

    if (result == 0) {
        //Write compensation value in BBCR79 D6-D0
        HAL_BBCR_RD(prAdapter, 79, &u4tmp);
        u4tmp &= 0x7f00;
        u4tmp |= temp8;
        HAL_BBCR_WR(prAdapter, 79, u4tmp);
        DBGLOG(PHY, TRACE,("The calibrated TX I DC mis. is: (%d) 0x%04lx\n", temp8, u4tmp));
    } else {
        ERRORLOG(("The calibration is failed in TX I DC.\n"));
    }
    HAL_BBCR_WR(prAdapter, 77, 0x0);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalTxQDC (
    IN P_ADAPTER_T     prAdapter
    )
{
    UINT_8             temp8;
    UINT_32            u4tmp, loopcount;
    INT_32             result;

    DEBUGFUNC("halRFCalTxQDC");

    ASSERT(prAdapter);

    //Do TX Q-DC calibration
    HAL_BBCR_WR(prAdapter, 77, 0xa180);
    kalMdelay(1);

    loopcount = 0;
    /* wait result */
    do {
        /* read BB CR 78 */
        HAL_BBCR_RD(prAdapter, 78, &u4tmp);
        temp8 = (UINT_8)u4tmp;

        if ((temp8 & 0x80) != 0) {
            temp8 = temp8 & 0x7f;
            result = 0;
            break;
        } else {
            loopcount ++;
            result = -1;
        }
    } while (loopcount < 2);

    if (result == 0) {
        //Write compensation value in BBCR79 D14-D8
        HAL_BBCR_RD(prAdapter, 79, &u4tmp);
        u4tmp &= 0x7f;
        u4tmp |= (temp8 << 8);
        HAL_BBCR_WR(prAdapter, 79, u4tmp);
        DBGLOG(PHY, TRACE,("The calibrated TX Q DC mis. is: (%d) 0x%04lx\n", temp8, u4tmp));
    } else {
        ERRORLOG(("The calibration is failed in TX Q DC.\n"));
    }
    HAL_BBCR_WR(prAdapter, 77, 0x0);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalTxGain (
    IN P_ADAPTER_T     prAdapter
    )
{
    UINT_8             temp8;
    UINT_32            u4tmp, loopcount;
    INT_32             result;

    DEBUGFUNC("halRFCalTxGain");

    ASSERT(prAdapter);

    //Do Tx gain calibration
    HAL_BBCR_WR(prAdapter, 77, 0x91c0);
    kalMdelay(1);

    loopcount = 0;
    /* wait result */
    do {
        HAL_BBCR_RD(prAdapter, 78, &u4tmp);
        temp8 = (UINT_8)u4tmp;

        if ((temp8 & 0x80) != 0) {
            result = 0;
            temp8 = temp8 & 0x7f;
            break;
        } else {
            loopcount ++;
            result = -1;
        }
    } while (loopcount < 2);

    if (result != 0) {
        ERRORLOG(("The calibration is failed due to TX Gain calculation.\n"));
    }

    if (result == 0) {
        HAL_BBCR_RD(prAdapter, 80, &u4tmp);
        u4tmp &= 0x7f00;
        u4tmp |= temp8;
        HAL_BBCR_WR(prAdapter, 80, u4tmp);
        DBGLOG(PHY, TRACE, ("The calibrated TX Gain: (%d) 0x%04lx\n", temp8, u4tmp));
    }
    HAL_BBCR_WR(prAdapter, 77, 0x0);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalTxPhase (
    IN P_ADAPTER_T     prAdapter
    )
{
    UINT_8             temp8;
    UINT_32            u4tmp, loopcount;
    INT_32             result;

    DEBUGFUNC("halRFCalTxPhase");

    ASSERT(prAdapter);

    //Do Tx Phase calibration
    HAL_BBCR_WR(prAdapter, 77, 0x89c0);
    kalMdelay(1);

    loopcount = 0;
    /* wait result */
    do {
        /* read BB CR 78 */
        HAL_BBCR_RD(prAdapter, 78, &u4tmp);
        temp8 = (UINT_8)u4tmp;

        if ((temp8 & 0x80) != 0) {
            result = 0;
            temp8 = temp8 & 0x7f;
            break;
        } else {
            loopcount ++;
            result = -1;
        }
    } while (loopcount < 2);

    if (result != 0) {
        ERRORLOG(("The calibration is failed due to TX Phase calculation.\n"));
    }

    if (result == 0) {
        HAL_BBCR_RD(prAdapter, 80, &u4tmp);
        u4tmp &= 0x7f;
        u4tmp |= (temp8 << 8);
        HAL_BBCR_WR(prAdapter, 80, u4tmp);
        DBGLOG(PHY, TRACE, ("The calibrated TX Phase: (%d) 0x%04lx\n", temp8, u4tmp));
    }
    HAL_BBCR_WR(prAdapter, 77, 0x0);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalRxGain (
    IN P_ADAPTER_T     prAdapter
    )
{
    UINT_8             temp8;
    UINT_32            u4tmp, loopcount;
    INT_32             result;

    DEBUGFUNC("halRFCalRxGain");

    ASSERT(prAdapter);

    //Do Rx gain calibration
    HAL_BBCR_WR(prAdapter, 77, 0x85C0);
    kalMdelay(1);

    loopcount = 0;
    /* wait result */
    do {
        HAL_BBCR_RD(prAdapter, 78, &u4tmp);
        temp8 = (UINT_8)u4tmp;

        if ((temp8 & 0x80) != 0) {
            result = 0;
            temp8 = temp8 & 0x7f;
            break;
        } else {
            loopcount ++;
            result = -1;
        }

    } while (loopcount < 2);

    if (result != 0) {
        ERRORLOG(("The calibration is failed due to Rx Gain calculation.\n"));
    }

    if (result == 0) {
        HAL_BBCR_RD(prAdapter, 81, &u4tmp);
        u4tmp &= 0x7f00;
        u4tmp |= temp8;
        HAL_BBCR_WR(prAdapter, 81, u4tmp);
        DBGLOG(PHY, TRACE, ("The calibrated RX Gain: (%d) 0x%04lx\n", temp8, u4tmp));
    }
    HAL_BBCR_WR(prAdapter, 77, 0x0);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalRxPhase (
    IN P_ADAPTER_T     prAdapter
    )
{
    UINT_8             temp8;
    UINT_32            u4tmp, loopcount;
    INT_32             result;

    DEBUGFUNC("halRFCalRxPhase");

    ASSERT(prAdapter);

    //Do Rx phase calibration
    HAL_BBCR_WR(prAdapter, 77, 0x83C0);
    kalMdelay(1);

    loopcount = 0;
    /* wait result */
    do {
        HAL_BBCR_RD(prAdapter, 78, &u4tmp);
        temp8 = (UINT_8)u4tmp;
        if ((temp8 & 0x80) != 0) {
            result = 0;
            temp8 = temp8 & 0x7f;
            break;
        } else {
            loopcount ++;
            result = -1;
        }
    } while (loopcount < 2);

    if (result != 0) {
        ERRORLOG(("The calibration is failed due to Rx Phase calculation.\n"));
    }

    if (result == 0) {
        HAL_BBCR_RD(prAdapter, 81, &u4tmp);
        u4tmp &= 0x7f;
        u4tmp |= (temp8 << 8);
        HAL_BBCR_WR(prAdapter, 81, u4tmp);
        DBGLOG(PHY, TRACE, ("The calibrated RX Phase: (%d) 0x%04lx\n", temp8, u4tmp));
    }
    HAL_BBCR_WR(prAdapter, 77, 0x0);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCalPostStep (
    IN P_ADAPTER_T     prAdapter,
    IN UINT_8          ucFreqRange
    )
{
    UINT_32 u4tmp;

    DEBUGFUNC("halRFCal60105_Post");
    DBGLOG(PHY, TRACE,("\n"));

    ASSERT(prAdapter);

    /* back to normal operation bit5~2 0x0111 */
    HAL_MCR_RD(prAdapter, RFCR_0, &u4tmp);
    u4tmp |= BITS(2,4);
    u4tmp &= ~BIT(5);
    /* The BG/AJ selection */
    if (ucFreqRange ==0) {
        u4tmp &= ~BIT(14);
    }
    else {
        u4tmp |= BIT(14);
    }
    HAL_MCR_WR(prAdapter, RFCR_0, u4tmp);
    DBGLOG(PHY, TRACE, ("The RFCR0 store as 0x%08lx\n", u4tmp));

    HAL_BBCR_WR(prAdapter, 77, 0x0000);
    DBGLOG(PHY, TRACE, ("The BBCR BRFCCR clear as 0x0000\n"));

    HAL_BBCR_WR(prAdapter, 97, 0x0000);
    DBGLOG(PHY, TRACE, ("The BBCR BTMR clear as 0x0000\n"));

    HAL_BBCR_WR(prAdapter, 98, 0x0000);
    DBGLOG(PHY, TRACE, ("The BBCR 98 clear as 0x0000\n"));

}   /* halRFCalPostStep */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCal60105_2412_2484 (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 i;

    DEBUGFUNC("halRFCal60105_2412_2484");

    DBGLOG(PHY, TRACE,("\n"));

    ASSERT(prAdapter);

    halRFCalTxSet(prAdapter, 0);

    //Do TX I-DC calibration
    halRFCalTxIDC(prAdapter);

    halRFCalTxSet(prAdapter, 0);

    //Do TX Q-DC calibration
    halRFCalTxQDC(prAdapter);

    for (i=0; i<2; i++) {

        halRFCalTxSet(prAdapter, 0);

        //Do Tx gain calibration
        halRFCalTxGain(prAdapter);

        halRFCalTxSet(prAdapter, 0);

        //Do Tx Phase calibration
       halRFCalTxPhase(prAdapter);
    }

    halRFCalRxSet(prAdapter, 0);

    halRFCalRxGain(prAdapter);

    halRFCalRxSet(prAdapter, 0);

    halRFCalRxPhase(prAdapter);

}   /* halRFCal60105_2412_2484 */


#if CFG_ONLY_802_11A
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFCal60105_5G (
    IN P_ADAPTER_T prAdapter
    )
{
    ASSERT(prAdapter);

    HAL_BBCR_WR(prAdapter, 80, 0x0602);
}
#endif


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halRFSetProg (
    IN P_ADAPTER_T     prAdapter,
    IN PUINT_32        pu4SynthProgWords,
    IN UINT_32         u4NumWords
    )
{
    UINT_32 i;

    DEBUGFUNC("halRFSetProg");

    ASSERT(prAdapter);
    ASSERT(pu4SynthProgWords);

    /* Program the synthesizer. */
    for (i = 0; i < u4NumWords; i++) {
        DBGLOG(PHY, TRACE, ("Channel code: 0x%lx\n", pu4SynthProgWords[i]));

        HAL_MCR_WR(prAdapter, RFCR_3, pu4SynthProgWords[i]);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_RF_CHANNEL_PROG_ENTRY
halRFGetRFChnlProgEntry (
    IN P_ADAPTER_T     prAdapter,
    IN UINT_8          ucChannelNum,
    IN ENUM_BAND_T     eBand
    )
{
    UINT_32            u4Idx = 0;
    BOOLEAN            fgFound = FALSE;
    TABLE_RF_ENTRY_T   rRfChTable;
    UINT_16            u2Size;
    P_RF_CHANNEL_PROG_ENTRY parRFChTable;

    ASSERT(prAdapter);

    rRfChTable = arRFInitTableByFreq[prAdapter->eRfFreqSet];
    parRFChTable = rRfChTable.pu4TableRfPtr;
    u2Size = rRfChTable.u2Size;
    ASSERT(parRFChTable);

    for (u4Idx = 0; u4Idx < u2Size; u4Idx++) {
        if (ucChannelNum == parRFChTable[u4Idx].ucChannelNum &&
            parRFChTable[u4Idx].eBand == eBand ){
            fgFound = TRUE;
            break;
        }
    }

    if (fgFound) {
        return (P_RF_CHANNEL_PROG_ENTRY)&parRFChTable[u4Idx];
    }
    else {
        return NULL;
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
halRFSwitchChannel (
    IN P_ADAPTER_T       prAdapter,
    IN UINT_8            ucChannelNum,
    IN ENUM_BAND_T       eBand,
    IN BOOLEAN           fgSetBBTXfilterForJp
    )
{
    P_RF_CHANNEL_PROG_ENTRY prRfChnlProgEntry;
    UINT_32 u4Tmp;

    DEBUGFUNC("halRFSwitchChannel");

    ASSERT(prAdapter);

    /* Setting the BCCMR1 bit 6 for countery code is JP and at CH 14, bit 6 set 0 others */
            HAL_BBCR_RD(prAdapter, BBCR_CR65 , &u4Tmp);
    if (fgSetBBTXfilterForJp) {
            u4Tmp |= BCR_BBCMR1_TXF_JP_CH;
    }
    else {
        u4Tmp &= ~BCR_BBCMR1_TXF_JP_CH;
    }
    HAL_BBCR_WR(prAdapter, BBCR_CR65, u4Tmp);

    if ((prRfChnlProgEntry = halRFGetRFChnlProgEntry(prAdapter, ucChannelNum, eBand)) != NULL) {
        DBGLOG(HAL, TRACE, ("ch=%d (%lu) 0x%08lx \n",
            ucChannelNum,
            prRfChnlProgEntry->u4NumSynthProgWords,
            prRfChnlProgEntry->au4SynthProgWords[0] ));
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
    IN ENUM_BAND_T eBand,
    IN BOOLEAN     fgSetBBTXfilterForJp
    )
{
    P_RF_CHANNEL_PROG_ENTRY prChProgEntry;
    UINT_32 i;
    UINT_32 u4Tmp;

    DEBUGFUNC("halSetRFSwitchChnlInst");
    ASSERT(prAdapter);

    /* Setting the BCCMR1 bit 6 for countery code is JP and at CH 14, bit 6 set 0 others */
    HAL_BBCR_RD(prAdapter, BBCR_CR65 , &u4Tmp);
    if (fgSetBBTXfilterForJp) {
        u4Tmp |= BCR_BBCMR1_TXF_JP_CH;
    }
    else {
        u4Tmp &= ~BCR_BBCMR1_TXF_JP_CH;
    }
    HAL_HW_SCAN_SET_INST_MCR_WR(prAdapter, BBCR_CR65, u4Tmp, FALSE, TRUE);

    if ((prChProgEntry = halRFGetRFChnlProgEntry(prAdapter, ucChannelNum, eBand)) != NULL) {
        DBGLOG(HAL, TRACE, ("ch=%d 0x%08lx \n",
            ucChannelNum,
            prChProgEntry->au4SynthProgWords[0] ));
        /* Program the synthesizer. */
        for (i = 0; i < prChProgEntry->u4NumSynthProgWords; i++) {
            HAL_HW_SCAN_SET_INST_MCR_WR(prAdapter, RFCR_3, prChProgEntry->au4SynthProgWords[i],TRUE,TRUE);
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
    return RF_TYPE_MTK60105;
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
    TABLE_RF_ENTRY_T rRfChTable;
    UINT_16 u2Size;
    P_RF_CHANNEL_PROG_ENTRY parRFChTable;

    DEBUGFUNC("nicGetChannelBandFromFreq");

    ASSERT(prAdapter);
    rRfChTable = arRFInitTableByFreq[prAdapter->eRfFreqSet];
    u2Size = rRfChTable.u2Size;
    parRFChTable = rRfChTable.pu4TableRfPtr;
    ASSERT(parRFChTable);

    for (u4Idx = 0; u4Idx < u2Size; u4Idx++) {
        if ( u4ChannelFreq == parRFChTable[u4Idx].u4ChannelFreq  ){
            fgFound = TRUE;
            break;
        }
    }

    if (fgFound) {
        return (P_RF_CHANNEL_PROG_ENTRY)&parRFChTable[u4Idx];
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
#if CFG_ONLY_802_11A
#else
    UINT_16         u2Idx = 0;
    UINT_16         u2Size;
    P_REG_ENTRY_T   prTable;

    DEBUGFUNC("halRFAdoptTempChange");

    ASSERT(prAdapter);

    switch (rState) {
    case LOW_TEMP:
        u2Size = sizeof(arLowTempSetting)/sizeof(REG_ENTRY_T);
        prTable = arLowTempSetting;
        break;

    case NORMAL_TEMP:
        u2Size = sizeof(arNormalTempSetting)/sizeof(REG_ENTRY_T);
        prTable = arNormalTempSetting;
        break;

    case HIGH_TEMP:
        u2Size = sizeof(arHighTempSetting)/sizeof(REG_ENTRY_T);
        prTable = arHighTempSetting;
        break;
    default:
        DBGLOG(HAL, ERROR, ("Temperature %d is not supported\n", rState));
        return;
    }

    for(u2Idx = 0; u2Idx < u2Size; u2Idx++) {
         DBGLOG(HAL, INFO, ("Set %#lx = %#lx\n", prTable[u2Idx].u4Offset, prTable[u2Idx].u4Value));
         HAL_MCR_WR(prAdapter, prTable[u2Idx].u4Offset, prTable[u2Idx].u4Value);
     }
#endif
}

