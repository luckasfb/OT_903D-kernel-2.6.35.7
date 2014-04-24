

#ifndef __PMU6573_HW_H__
#define __PMU6573_HW_H__
#include <mach/pmic_features.h>
#include <mach/mt6573_reg_base.h>

#if defined(PMIC_6573_REG_API)

//#define PMU_BASE        0x7002F000
#define PMU_BASE		MIXEDSYS1_BASE
#define PMU_END         (PMU_BASE+0x1000)

///////////////////////////////////////////////////////////////////////////////
// 1. Data Retention group
#define RETENTION_CON0    (PMU_BASE + 0x0080)

///////////////////////////////////////////////////////////////////////////////
// 2. Low Power Oscillator group
#define LPOSC_CON0    (PMU_BASE + 0x0100)

///////////////////////////////////////////////////////////////////////////////
// 3. STRUP  group
#define STRUP_CON0    (PMU_BASE + 0x0200)

///////////////////////////////////////////////////////////////////////////////
// 4. LDO 1 group
#define VA28_CON0    (PMU_BASE + 0x0710)
#define VA25_CON0    (PMU_BASE + 0x0720)
#define VA12_CON0    (PMU_BASE + 0x0620)
#define VRTC_CON0    (PMU_BASE + 0x0628)
#define VMIC_CON0    (PMU_BASE + 0x0630)
#define VTV_CON0    (PMU_BASE + 0x0638)
#define VAUDN_CON0    (PMU_BASE + 0x0640)
#define VAUDP_CON0    (PMU_BASE + 0x0650)
#define PMUA_CON0    (PMU_BASE + 0x0680)

///////////////////////////////////////////////////////////////////////////////
// 5. LDO 2 group
#define VRF_CON0    (PMU_BASE + 0x0700)
#define VCAMA_CON0    (PMU_BASE + 0x0730)
#define VCAMD_CON0    (PMU_BASE + 0x0740)
#define VIO_CON0    (PMU_BASE + 0x0750)
#define VUSB_CON0    (PMU_BASE + 0x0760)
#define VSIM_CON0    (PMU_BASE + 0x0780)
#define VSIM2_CON0    (PMU_BASE + 0x0790)
#define VIBR_CON0    (PMU_BASE + 0x07B0)
#define VMC_CON0    (PMU_BASE + 0x07C0)
#define VCAMA2_CON0    (PMU_BASE + 0x07D0)
#define VCAMD2_CON0    (PMU_BASE + 0x07E0)
#define VM12_CON0    (PMU_BASE + 0x0800)
#define VM12_INT_CON0    (PMU_BASE + 0x0810)

///////////////////////////////////////////////////////////////////////////////
// 6. BUCK group
#define VCORE_CON0    (PMU_BASE + 0x0900)
#define VIO1V8_CON0    (PMU_BASE + 0x0920)
#define VAPROC_CON0    (PMU_BASE + 0x0940)
#define VRF1V8_CON0    (PMU_BASE + 0x0960)

///////////////////////////////////////////////////////////////////////////////
// 7. BOOST group
#define PMIC_BB_CON0    (PMU_BASE + 0x0B00)

///////////////////////////////////////////////////////////////////////////////
// 8. KPLED group
#define KPLED_CON0    (PMU_BASE + 0x0C80)

///////////////////////////////////////////////////////////////////////////////
// 9. OC group
#define PMIC_OC_CON0    (PMU_BASE + 0x0E00)

///////////////////////////////////////////////////////////////////////////////
// 10. CHR
#define CHR_CON0    (PMU_BASE + 0x0A00)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif // #if defined(PMIC_6573_REG_API)

#endif // #ifndef __PMU6573_HW_H__

