






#include "precomp.h"
#include "bb_mt5922.h"




REG_ENTRY_T arBBInitTableForMTK60105[] = {
    {   1, 0x00000631}, //
    {  16, 0x0000d160}, //
    {  17, 0x00009090}, //
    {  18, 0x00007766}, //
    {  48, 0x00001992}, //
    {  46, 0x0000143a}, //
    {  49, 0x000066ee}, //
    {  24, 0x00001e1d}, //
    {  25, 0x0000197f}, //
    { 126, 0x0000d0bf}, //
    { 127, 0x0000da57}, //
    {  26, 0x00006cb8}, //
    {  27, 0x0000d889}, //
    {  29, 0x00001339}, //
    {  15, 0x00003f09}, //
    {  23, 0x00000070}, //
    {  62, 0x00000303}, //
    {  28, 0x00006039}, //
    {  21, 0x000018d1}, //
    {  43, 0x0000b90d}, //
    {  44, 0x000007a2}, //
    {  72, 0x0000e8b2}, //
    {  63, 0x00009b9b}, //
    {  74, 0x00003665}, //
    {  75, 0x00008a53}, //
    { 121, 0x00000800}, //
    {  67, 0x00002080}, //
    {  64, 0x00005821}, //
    {  65, 0x000008a5}, //
    { 125, 0x00000537}, //
    {  47, 0x0000340a}, //
    {  19, 0x0000340D}, //
    { 103, 0x000000AB}, //
    {  13, 0x00004318}, //
    {  4, 0x00006596}, //
    {  5, 0x00005965}, //
    {  6, 0x00009659}, //
};

REG_ENTRY_T arBBInitTableForAiroha2236[] = {
    // left blank here, for solving compiler warning, take a dummy register here
    {0, 0x00}, //CR0 is write only
};

TABLE_ENTRY_T arBBInitTableByRF[] = {
    {arBBInitTableForAiroha2236, sizeof(arBBInitTableForAiroha2236)/sizeof(REG_ENTRY_T)},
    {arBBInitTableForMTK60105, sizeof(arBBInitTableForMTK60105)/sizeof(REG_ENTRY_T)}
};






/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBInit (
    IN P_ADAPTER_T        prAdapter
    )
{
    UINT_16 u2Idx = 0;
    TABLE_ENTRY_T rBBInitTable = arBBInitTableByRF[halGetRFType(prAdapter)];
    //UINT_16 u2Size = sizeof(arBBInitTableForAiroha2236)/sizeof(REG_ENTRY_T);
    UINT_16 u2Size = rBBInitTable.u2Size;
    P_REG_ENTRY_T parBBInitTable = rBBInitTable.pu4TablePtr;

    DEBUGFUNC("halBBInit");

    ASSERT(prAdapter);

    HAL_MCR_WR(prAdapter, MCR_IOPCR, IO_SET_TRAP_PIN_DEFAULT_ATTR | IO_SET_DBG_PORT_SEL);
    HAL_MCR_WR(prAdapter, 0x130, 0x001e);

    if (halGetRFType(prAdapter) == RF_TYPE_AL2236) {
        HAL_MCR_WR(prAdapter, MCR_RSCR, 0x11421222);
    }
    else if (halGetRFType(prAdapter) == RF_TYPE_MTK60105) {
        HAL_MCR_WR(prAdapter, MCR_RSCR, 0x1182F22F);
    }

    for(u2Idx = 0; u2Idx < u2Size; u2Idx++) {
        DBGLOG(INIT, TRACE, ("Set %#x = %#x\n", parBBInitTable[u2Idx].u4Offset, parBBInitTable[u2Idx].u4Value));
        DBGLOG(MIKE, TRACE, ("Set %#x = %#x\n", parBBInitTable[u2Idx].u4Offset, parBBInitTable[u2Idx].u4Value));
        HAL_BBCR_WR(prAdapter, parBBInitTable[u2Idx].u4Offset, parBBInitTable[u2Idx].u4Value);
    }

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBChangeBand (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BAND_T eBand
    )
{
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBAdoptTempChange (
    IN P_ADAPTER_T          prAdapter,
    IN ENUM_THERMO_STATE_T  rState
    )
{
 
    DEBUGFUNC("halBBAdoptTempChange");
}


