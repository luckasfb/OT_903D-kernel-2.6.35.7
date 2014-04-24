

#ifndef __PMU6573_SW_H__
#define __PMU6573_SW_H__
#include "pmic_features.h"

#if defined(PMIC_6573_REG_API)

#include "upmu_sw.h"

/* Charger external interrupt is fixed. */
#define PMU_CHR_EINT_PIN         15
#define PMU_OC_EINT_PIN          16
#define PMU_CHR_OVP_EINT_PIN     17
#define PMU_CHR_LDO_EINT_PIN     19

/* adc number for measuring VBAT/VISENSE/VCHARGER is fixed internally. */
#define PMU_ADC_VBAT_CH_NUM      0
#define PMU_ADC_VISENSE_CH_NUM   1
#define PMU_ADC_VCHARGER_CH_NUM  2

/* adc factor for VBAT/VISENSE/VCHARGER */
#define PMU_ADC_FACTOR_VBAT      100
#define PMU_ADC_FACTOR_VISENSE   100
#define PMU_ADC_FACTOR_VCHARGER  294  // 300/51 * 50

///////////////////////////////////////////////////////////////////////////////
// Chip specific LDOs list
typedef enum
{
	// LDO 1
	LDO_VA28 = 0,
	LDO_VA25,	
	LDO_VA12,
	LDO_VRTC,
	LDO_VMIC,
	LDO_VTV,
	LDO_VAUDN,
	LDO_VAUDP,
	LDO_PMUA,
	
	// LDO 2
	LDO_VRF,
	LDO_VCAMA,
	LDO_VCAMD,
	LDO_VIO,
	LDO_VUSB,
	LDO_VSIM,
	LDO_VSIM2,
	LDO_VIBR,
	LDO_VMC,
	LDO_VCAMA2,
	LDO_VCAMD2,
	LDO_VM12,
	LDO_VM12_INT,

	//need remove
	LDO_VBT,
	LDO_VFM,
	LDO_VTCXO,
	LDO_VA,
	LDO_VIO28,	
	
	LDO_MAX
}upmu_ldo_list_enum;
#if 0
typedef enum
{
	LDO_VRF = 0,
	LDO_VTCXO,
	LDO_VA,
	LDO_VCAMA,
	LDO_VCAMD,
	LDO_VIO28,
	LDO_VUSB,
	LDO_VBT,
	LDO_VSIM,
	LDO_VSIM2,
	LDO_VIBR,
	LDO_VMC,
	LDO_VFM,
	LDO_VM12,
	LDO_MAX
}upmu_ldo_list_enum;
#endif

extern upmu_ldo_profile_entry upmu_ldo_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific BUCKs list
typedef enum
{
	BUCK_VCORE = 0,
	BUCK_VIO1V8,
	BUCK_VAPROC,
	BUCK_VRF1V8,

	//need remove
	BUCK_VIO18,
	
	BUCK_MAX
}upmu_buck_list_enum;

extern upmu_buck_profile_entry upmu_buck_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific BOOSTs list
typedef enum
{
	BUCK_BOOST = 0,

	BUCK_BOOST_MAX
}upmu_buck_boost_list_enum;

extern upmu_buck_boost_profile_entry upmu_buck_boost_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific BOOSTs list
typedef enum
{
	BOOST = 0,

	BOOST_MAX
}upmu_boost_list_enum;

extern upmu_boost_profile_entry upmu_boost_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific ISINKs list
typedef enum
{
	ISINK0 = 0,
	ISINK1,
	ISINK2,
	ISINK3,

	ISINK_MAX
}upmu_isink_list_enum;

extern upmu_isink_profile_entry upmu_isink_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific KPLEDs list
typedef enum
{
	KPLED = 0,

	KPLED_MAX
}upmu_kpled_list_enum;

extern upmu_kpled_profile_entry upmu_kpled_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific SPKs list
typedef enum
{
	SPK = 0,

	SPK_MAX
}upmu_spk_list_enum;

extern upmu_spk_profile_entry upmu_spk_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific CHRs list
typedef enum
{
	CHR = 0,

	CHR_MAX
}upmu_chr_list_enum;

extern upmu_chr_profile_entry upmu_chr_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific OC list
typedef enum
{
	OC = 0,

	OC_MAX
}upmu_oc_list_enum;

extern upmu_oc_profile_entry upmu_oc_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific STRUP list
typedef enum
{
	STRUP = 0,

	STRUP_MAX
}upmu_strup_list_enum;

extern upmu_strup_profile_entry upmu_strup_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific LPOCS list
typedef enum
{
	LPOCS = 0,

	LPOCS_MAX
}upmu_lposc_list_enum;

extern upmu_lposc_profile_entry upmu_lposc_profile[];

///////////////////////////////////////////////////////////////////////////////
// Chip specific Retention list
typedef enum
{
	RETENTION = 0,

	RETENTION_MAX
}upmu_retention_list_enum;

extern upmu_retention_profile_entry upmu_retention_profile[];

typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    // AC : 450mA~1A 
    STANDARD_CHARGER,       // AC : ~1A
} CHARGER_TYPE;

// =====================================================================================

// MoDIS parser skip start
void pmu6573_init(void);
// MoDIS parser skip end

#endif  // #if defined(PMIC_6573_REG_API)

#endif // #ifndef __PMU6573_SW_H__

