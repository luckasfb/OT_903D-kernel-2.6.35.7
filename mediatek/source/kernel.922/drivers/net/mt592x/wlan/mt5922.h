





#ifndef _MT5922_H
#define _MT5922_H


#include "mt592x.h"
#include "rf_al2236.h"
#include "bb_mt5911.h"

#define MTK_CHIP_REV               0x00005921

#define MTK_CHIP_MP_REVERSION_ID   0x0
#define MTK_CHIP_ECO4_REVERSION_ID 0x1

#define DEFAULT_VAG_GAIN_SLOPE     0x02
#define DEFAULT_THERMO_SLOPE       0x0A





VOID
halMCRChipInit(
    IN P_ADAPTER_T prAdapter
    );

VOID
halGetDefaultEEPROMCfg(
    IN P_EEPROM_CTRL_T prEEPROMCtrl
    );

VOID
halGetSupportedPhyTypeSet(
    IN PUINT_16 pu2PhyTypeSet
    );


#endif /* _MT5922_H */


