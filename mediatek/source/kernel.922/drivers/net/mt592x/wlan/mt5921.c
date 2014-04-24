






#include "precomp.h"



REG_ENTRY_T arMACInitValue[] = {
    {MCR_BCWR,         0x0000001f}, /* NOTE(2008/03/27): CR412 - Set to 0x1f for WiFi */
    {MCR_OFPR,         0x22222222},
    {MCR_CFPR,         0x00000020},
    {MCR_ACDR2,        0xC0000000}
};

extern WIFI_CFG_DATA gPlatformCfg;





/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halMCRChipInit (
    IN P_ADAPTER_T       prAdapter
    )
{
    UINT_16 u2Idx = 0;
    UINT_16 u2Size = sizeof(arMACInitValue)/sizeof(REG_ENTRY_T);

    ASSERT(prAdapter);

    for (u2Idx = 0; u2Idx < u2Size; u2Idx++) {
        HAL_MCR_WR(prAdapter, arMACInitValue[u2Idx].u4Offset, arMACInitValue[u2Idx].u4Value);
    }
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halGetDefaultEEPROMCfg (
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    )
{
    ASSERT(prEEPROMCtrl);

    prEEPROMCtrl->u4RegulationDomain = REGULATION_DOMAIN_FCC;
    prEEPROMCtrl->ucBandSelect = EEPROM_PHY_MODE_G;
    prEEPROMCtrl->ucVgaGainSlop = DEFAULT_VAG_GAIN_SLOPE;
    prEEPROMCtrl->ucThermoSensorSlop = DEFAULT_THERMO_SLOPE;
    prEEPROMCtrl->cAbsTemp = EEPROM_V3_THERMO_INFO_ABS_TEMP_DEFAULT;
    prEEPROMCtrl->ucThermoValue = EEPROM_V3_THERMO_INFO_THERMO_VAL_DEFAULT;
    prEEPROMCtrl->ucXtalTrim = EEPROM_XTAL_TRIM_VAL_DEFAULT;
    prEEPROMCtrl->u2OscStableTimeUs = EEPROM_OSC_STABLE_TIME_VAL_DEFAULT;
    prEEPROMCtrl->fgAlcUseThermoEn = EEPROM_ALC_USE_THERMO_EN_VAL_DEFAULT;
    prEEPROMCtrl->fgLnaUseThermoEn = EEPROM_LNA_USE_THERMO_EN_VAL_DEFAULT;
    prEEPROMCtrl->ucDaisyChain = gPlatformCfg.rWifiCustom.u4DaisyChainEnable;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
halGetSupportedPhyTypeSet (
    IN PUINT_16 pu2PhyTypeSet
    )
{
    ASSERT(pu2PhyTypeSet);

    *pu2PhyTypeSet = (UINT_16)(PHY_TYPE_BIT_HR_DSSS | PHY_TYPE_BIT_ERP | PHY_TYPE_BIT_OFDM);

    return;
}





