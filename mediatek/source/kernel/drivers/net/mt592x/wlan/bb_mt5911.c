






#include "precomp.h"
#include "bb_mt5911.h"




REG_ENTRY_T arBBInitTableForAiroha2236[] = {
    {81,  0x1b},  // bandgap current
    {83,  0xfb},  // TX DAC buffer current
    {84,  0x15},  // DAC common mode/gain mode
    {92,  0x41},  // RX ADC gain mode 1V
    {93,  0xa8},  // RX ADC bias current
    {94,  0x40},  // RX ADC common mode
    {45,  0x54},  // Target 1
    {46,  0x1c},
    {45,  0x55},  // Target 2
    {46,  0x1d},
    {45,  0x51},  // LNA thr  : G1 & G2 switch
    {46,  0x1f},
    {45,  0x52},  // LNA thr2 : low & middle switch
    {46,  0x1f},
    {45,  0x53},  // LNA thr3 : high & middle switch
    {46,  0x1b},
    {45,  0x56},  // G1 & G2
    {46,  0x90},
    {45,  0x57},  // G2 & G3
    {46,  0x10},
    {45,  0x58},  // G4 : for middle gain
    {46,  0x0b},
    {45,  0x00},  // op mode
    {46,  0x22},
    {45,  0x01},  // T1 , T2
    {46,  0xf4},
    {45,  0x02},  // T3 , T4
    {46,  0x60},
    {45,  0x03},  // T5 , T6
    {46,  0x64},
    {45,  0x04},  // T7 , T8
    {46,  0x64},
/* for adjacent channel */
    {49,  0x05},  // SQ1
    {50,  0x84},  // SQ2
    {77,  0x94},  // OFDM OSD

    {45,  0x4b},  // Ed thr
    {46,  0x18},
    {45,  0x78},  // CCK thr
    {46,  0x07},
    {45,  0x79},  // OFDM thr
    {46,  0x07},
    {45,  0x77},  // CCK AR & OFDM AR
    {46,  0xa7},
    {45,  0x83},  // jmp_down_thr & jump_up_thr
    {46,  0x63},
    {45,  0x7d},  // LNA gain
    {46,  0x11},
    {45,  0x84},  // LNA middle gain
    {46,  0xe7},
    {45,  0x5b},  // ZC threshold
    {46,  0x10},
    {45,  0x82},  // IQ accumulation time
    {46,  0x12},
    {03,  0x21}     // CR3, T/RX configure-2: (RX antenna: fixed antenna 0, Bit[0]: reserved).
};


const REG_ENTRY_T arOFDMTXBackOff[] = {
    {39, 0x00},
    {40, 0x65},
    {39, 0x01},
    {40, 0xf7},
    {39, 0x02},
    {40, 0xdf},
    {39, 0x03},
    {40, 0x65},
    {39, 0x04},
    {40, 0xf7},
    {39, 0x05},
    {40, 0xdf}
};

/* OFDM TX backoff setting for Japan */
const REG_ENTRY_T arOFDMTXBackOffforJapan[] = {
    {39, 0x00},
    {40, 0x65},
    {39, 0x01},
    {40, 0xf7},
    {39, 0x02},
    {40, 0xdf},
    {39, 0x03},
    {40, 0x65},
    {39, 0x04},
    {40, 0xf7},
    {39, 0x05},
    {40, 0xdf},
};


TABLE_ENTRY_T arBBInitTableByRF[] = {
    {arBBInitTableForAiroha2236, sizeof(arBBInitTableForAiroha2236)/sizeof(REG_ENTRY_T)}
};






/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBInit (
    IN P_ADAPTER_T       prAdapter
    )
{
    UINT_16              u2Idx = 0;
    TABLE_ENTRY_T        rBBInitTable = arBBInitTableByRF[halGetRFType(prAdapter)];
    UINT_16              u2Size = rBBInitTable.u2Size;
    P_REG_ENTRY_T        parBBInitTable = rBBInitTable.pu4TablePtr;

    DEBUGFUNC("halBBInit");

    ASSERT(prAdapter);
    
    if (halGetRFType(prAdapter) == RF_TYPE_AL2236) {
        HAL_MCR_WR(prAdapter, MCR_IOPCR, IO_SET_TRAP_PIN_DEFAULT_ATTR | IO_SET_DBG_PORT_SEL);
        HAL_MCR_WR(prAdapter, 0x130, 0x001e);
        HAL_MCR_WR(prAdapter, MCR_RSCR, 0x11421222);
    }

    for(u2Idx = 0; u2Idx < u2Size; u2Idx++) {
        DBGLOG(INIT, TRACE, ("Set %#08lx = %#08lx\n", parBBInitTable[u2Idx].u4Offset, parBBInitTable[u2Idx].u4Value));
        HAL_BBCR_WR(prAdapter, parBBInitTable[u2Idx].u4Offset, parBBInitTable[u2Idx].u4Value);
    }

    //4 2007/07/04, mikewu, we just give it a default value
    halBBSetTxBackOff(prAdapter, prAdapter->rEEPROMCtrl.u4RegulationDomain);

    //4 2007/07/04, mikewu, now we use the value in the default config
    halBBSetDACOffset(prAdapter,
        prAdapter->rEEPROMCtrl.ucID,
        prAdapter->rEEPROMCtrl.ucQD);


}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBSetTxBackOff (
    IN P_ADAPTER_T      prAdapter,
    IN UINT_32          u4RegulationDomain
    )
{
    UINT_16 u2Idx = 0;
    UINT_16 u2Size = 0;

    ASSERT(prAdapter);

    if (u4RegulationDomain == REGULATION_DOMAIN_JAPAN) {
        u2Size = sizeof(arOFDMTXBackOffforJapan)/sizeof(REG_ENTRY_T);
        for(u2Idx = 0; u2Idx < u2Size; u2Idx++) {
            HAL_BBCR_WR(prAdapter, arOFDMTXBackOffforJapan[u2Idx].u4Offset, arOFDMTXBackOffforJapan[u2Idx].u4Value);
        }
    }
    else {
        u2Size = sizeof(arOFDMTXBackOff)/sizeof(REG_ENTRY_T);
        for(u2Idx = 0; u2Idx < u2Size; u2Idx++) {
            HAL_BBCR_WR(prAdapter, arOFDMTXBackOff[u2Idx].u4Offset, arOFDMTXBackOff[u2Idx].u4Value);
        }
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBSetDACOffset (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8      ucID,
    IN UINT_8      ucQD
    )
{
    UINT_32 u4CR81, u4CR83, u4CR84;

    ASSERT(prAdapter);

    HAL_BBCR_RD(prAdapter, 81, &u4CR81);
    HAL_BBCR_RD(prAdapter, 83, &u4CR83);
    HAL_BBCR_RD(prAdapter, 84, &u4CR84);

    u4CR81 &= ~BITS(0,1);
    u4CR83 &= ~BITS(0,1);
    u4CR83 &= ~BITS(4,7);
    u4CR84 &= ~BITS(0,1);

    if (ucID & BIT(4)) { u4CR84 |= BIT(1); }
    if (ucID & BIT(3)) { u4CR81 |= BIT(0); }
    if (ucID & BIT(2)) { u4CR81 |= BIT(1); }
    if (ucID & BIT(1)) { u4CR83 |= BIT(6); }
    if (ucID & BIT(0)) { u4CR83 |= BIT(7); }
    if (ucQD & BIT(4)) { u4CR84 |= BIT(0); }
    if (ucQD & BIT(3)) { u4CR83 |= BIT(0); }
    if (ucQD & BIT(2)) { u4CR83 |= BIT(1); }
    if (ucQD & BIT(1)) { u4CR83 |= BIT(4); }
    if (ucQD & BIT(0)) { u4CR83 |= BIT(5); }

    HAL_BBCR_WR(prAdapter, 81, u4CR81);
    HAL_BBCR_WR(prAdapter, 83, u4CR83);
    HAL_BBCR_WR(prAdapter, 84, u4CR84);

}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halBBChangeBand (
    IN P_ADAPTER_T prAdapter,
    IN ENUM_BAND_T eBand
    )
{
    UINT_32 u4BBCR3 = 0;

    ASSERT(prAdapter);

    HAL_BBCR_RD(prAdapter, BBCR_CR3, &u4BBCR3);
    if(eBand == BAND_5G) {
        u4BBCR3 |=  CR3_BAND_5G_EN | CR3_RX_CCK_DIS;
    }
    else { /* BAND 2.4G */
        u4BBCR3 &= ~(CR3_BAND_5G_EN | CR3_RX_CCK_DIS);
    }

    HAL_BBCR_WR(prAdapter, BBCR_CR3, u4BBCR3);
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
