






#include "precomp.h"
#include "bb_mt5921.h"



REG_ENTRY_T arBBInitTableForMTK60105[] = {
    {    1, 0x0631},
    {   16, 0xd190},
    {   17, 0x80a0},
    {   18, 0x6699},
    {   48, 0x1992},
    {   46, 0x143a},
    {   49, 0x66ee},
    {   24, 0x1c1f},
    {   25, 0x38ff},
    {  126, 0xd0bf},
    {  127, 0xda57},
    {   26, 0x6c98},
    {   27, 0x1029},
    {   29, 0x123b},
    {   15, 0x3f09},
    {   23, 0x0070},
    {   62, 0x1046},
    {   28, 0x603b},
    {   21, 0x14d1},
    {   43, 0xb90a},
    {   44, 0x07a2},
    {   72, 0xd0b9},
    {   63, 0x9b9b},
    {   74, 0x3669},
    {    4, 0x6596},
    {    5, 0x5965},
    {   75, 0x8a53},
    {  123, 0x039f},
    {  121, 0xf55c},
    {   67, 0x2080},
    {   64, 0x5821},
    {   65, 0x08a5},
    {  125, 0x0637},
    {   47, 0x340a},
    {   19, 0x260d},
    {  103, 0x00b7},
    {    6, 0x9659},
    {   13, 0x4018},
    {   61, 0x9908},
    {  124, 0x56b2},
};

const REG_ENTRY_T arOFDMTXBackOff[] = {
	// left blank here, for solving compiler warning, take a dummy register here
	{0, 0x00}, //CR0 is write only
};

/* OFDM TX backoff setting for Japan */
const REG_ENTRY_T arOFDMTXBackOffforJapan[] = {
	// left blank here, for solving compiler warning, take a dummy register here
	{0, 0x00}, //CR0 is write only
};

/* High Temperature setting (70)*/
static REG_ENTRY_T arHighTempSetting[] = {
    {   29, 0x1339},
    {   28, 0x6039},
    {   19, 0x2B0D},
    {  103, 0x00B2},
};

/* Normal Temperature setting (25)*/
static REG_ENTRY_T arNormalTempSetting[] = {
    {   29, 0x133B},
    {   28, 0x603B},
    {   19, 0x260D},
    {  103, 0x00B7},
};

/* Low Temperature setting (-20)*/
static REG_ENTRY_T arLowTempSetting[] = {
    {   29, 0x133C},
    {   28, 0x603C},
    {   19, 0x240D},
    {  103, 0x00B9},
};






/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBInit (
    IN P_ADAPTER_T          prAdapter
    )
{
    UINT_16 u2Idx = 0;
    UINT_16 u2Size = sizeof(arBBInitTableForMTK60105)/sizeof(REG_ENTRY_T);
    UINT_32 u4Tmp;

    DEBUGFUNC("halBBInit");

    ASSERT(prAdapter);

#if 1
    /* Keep antenna switch setting for single antenna application */
    HAL_MCR_RD(prAdapter, MCR_IOPCR, &u4Tmp);
    u4Tmp &= (IOPCR_IO_ANT_SEL_N_DIR | IOPCR_IO_ANT_SEL_P_DIR);
    HAL_MCR_WR(prAdapter, MCR_IOPCR,
        u4Tmp | IO_SET_TRAP_PIN_DEFAULT_ATTR | IO_SET_DBG_PORT_SEL);
#else
    HAL_MCR_WR(prAdapter, MCR_IOPCR, IO_SET_TRAP_PIN_DEFAULT_ATTR | IO_SET_DBG_PORT_SEL);
#endif

    for(u2Idx = 0; u2Idx < u2Size; u2Idx++) {
        DBGLOG(INIT, TRACE, ("Set %#lx = %#lx\n", arBBInitTableForMTK60105[u2Idx].u4Offset, arBBInitTableForMTK60105[u2Idx].u4Value));
        HAL_BBCR_WR(prAdapter, arBBInitTableForMTK60105[u2Idx].u4Offset, arBBInitTableForMTK60105[u2Idx].u4Value);
    }

    //CR 265 [BUG] TX RESP Antenna mode wrongly set to Antenna 1
    //Now use Rx Antenna 0
    HAL_BBCR_RD(prAdapter, 1, &u4Tmp);
    u4Tmp &= ~BITS(4,5);
    u4Tmp |= BIT(5);
    HAL_BBCR_WR(prAdapter, 1, u4Tmp);
    DBGLOG(HAL, TRACE,("BB Set Rx Antenna to 0!\n"));

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBChangeBand (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BAND_T eBand
    )
{

    DEBUGFUNC("halBBChangeBand");
    DBGLOG(HAL, TRACE,("BB change Band not support !!\n"));
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBAdoptTempChange (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_THERMO_STATE_T  rState
    )
{
#if CFG_ONLY_802_11A
#else
    UINT_16         u2Idx = 0;
    UINT_16         u2Size;
    P_REG_ENTRY_T   prTable;

    DEBUGFUNC("halBBAdoptTempChange");

    ASSERT(prAdapter);

    switch (rState) {
    case LOW_TEMP:
        u2Size = sizeof(arLowTempSetting)/sizeof(REG_ENTRY_T);
        prTable = &arLowTempSetting[0];
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
         DBGLOG(HAL, LOUD, ("Set BBCR_%ld = %#lx\n", prTable[u2Idx].u4Offset, prTable[u2Idx].u4Value));
         HAL_BBCR_WR(prAdapter, prTable[u2Idx].u4Offset, prTable[u2Idx].u4Value);
    }

    DBGLOG(HAL, TRACE, ("Temperature State is changed to %d\n", rState));
#endif
}

#if PTA_ENABLED
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBEnableFixedRxGain (
    IN P_ADAPTER_T          prAdapter,
    IN BOOL                 fgEnableFixedRxGain
    )
{
    UINT_32 u4Size = sizeof(arBBInitTableForMTK60105)/sizeof(REG_ENTRY_T);
    UINT_32 u4CR24Index = 0xFFFFFFFF, u4CR25Index = 0xFFFFFFFF, index = 0;
    
    DEBUGFUNC("halBBEnableFixedRxGain");

    DBGLOG(HAL, TRACE, ("Use fixed RX gain? %d\r\n", fgEnableFixedRxGain));

    for(index = 0 ; index < u4Size ; index ++ ){
        if(arBBInitTableForMTK60105[index].u4Offset == BBCR_CR24){
            u4CR24Index = index;
        }
        if(arBBInitTableForMTK60105[index].u4Offset == BBCR_CR25){
            u4CR25Index = index;
        }
    }

    ASSERT(0xFFFFFFFF != u4CR24Index);
    ASSERT(0xFFFFFFFF != u4CR25Index);

    if((0xFFFFFFFF == u4CR24Index) || (0xFFFFFFFF == u4CR25Index)){
        return;
    }

    DBGLOG(HAL, TRACE, ("BBCR24 index %d BBCR25 index %d\r\n", u4CR24Index, u4CR25Index));

    DBGLOG(HAL, TRACE, ("CR24 value 0x%08x CR25 value 0x%08x\r\n", 
        arBBInitTableForMTK60105[u4CR24Index].u4Value,
        arBBInitTableForMTK60105[u4CR25Index].u4Value));

    if(fgEnableFixedRxGain){
        UINT_32 u4Value = 0;

        /* middle RX gain */
        u4Value = 0x001F;
        HAL_BBCR_WR(prAdapter, BBCR_CR24, u4Value);

        u4Value = arBBInitTableForMTK60105[u4CR25Index].u4Value;
        u4Value &= ~BITS(8, 12);//AGC_LNA_THR3
        u4Value &= ~BITS(5, 6);//Mid_low check en, High_mid check en 
        HAL_BBCR_WR(prAdapter, BBCR_CR25, u4Value);        

    }else{
        HAL_BBCR_WR(prAdapter, BBCR_CR24, 
            arBBInitTableForMTK60105[u4CR24Index].u4Value);

        HAL_BBCR_WR(prAdapter, BBCR_CR25, 
            arBBInitTableForMTK60105[u4CR25Index].u4Value);
    }
}
#endif
