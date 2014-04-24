

#ifndef __UPMU_HW_H__
#define __UPMU_HW_H__

#include "pmic_features.h"

#if defined(__DRV_UPMU_LDO_V1__)
#define LDO_CON0_OFFSET           0x00
#define LDO_CON1_OFFSET           0x04
#define LDO_CON2_OFFSET           0x08
#define LDO_CON3_OFFSET           0x0C
// LDO H/W register bitmap definition
  // LDO_XXX CON0
#define LDO_EN_MASK               0x0001
#define LDO_EN_SHIFT              0

#define LDO_ON_SEL_MASK           0x0002
#define LDO_ON_SEL_SHIFT          1

#define LDO_RS_MASK               0x0004
#define LDO_RS_SHIFT              2

#define LDO_VOL_SEL_MASK          0x01F0
#define LDO_VOL_SEL_SHIFT         4

#define LDO_NDIS_EN_MASK          0x0400
#define LDO_NDIS_EN_SHIFT         10

#define LDO_STB_EN_MASK           0x0800
#define LDO_STB_EN_SHIFT          11

#define LDO_OC_AUTO_OFF_MASK      0x1000
#define LDO_OC_AUTO_OFF_SHIFT     12

#define LDO_OCFB_EN_MASK          0x2000
#define LDO_OCFB_EN_SHIFT         13

#define LDO_OC_STATUS_MASK        0x4000
#define LDO_OC_STATUS_SHIFT       14

#define LDO_STATUS_MASK           0x8000
#define LDO_STATUS_SHIFT          15

 //AddForMT6573 LDO_XXX CON0, new
#define LDO_PSCLK_EN_MASK               0x0004
#define LDO_PSCLK_EN_SHIFT              2

#define LDO_STB_FORCE_MASK               0x0004
#define LDO_STB_FORCE_SHIFT              2

 //AddForMT6573 PMUA_CON0
#define VA28_BIST_EN_MASK               0x0001
#define VA28_BIST_EN_SHIFT              0

#define VA25_BIST_EN_MASK           0x0002
#define VA25_BIST_EN_SHIFT          1

#define VA12_BIST_EN_MASK               0x0004
#define VA12_BIST_EN_SHIFT              2

#define VMIC_BIST_EN_MASK               0x0008
#define VMIC_BIST_EN_SHIFT              3

#define PMT_PASSGATE_OVRD_MASK           0x8000
#define PMT_PASSGATE_OVRD_SHIFT          15

  // LDO_XXX CON1
#if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON1_BIT0__)
#define LDO_STB_TD_MASK           0x0003
#define LDO_STB_TD_SHIFT          0
#endif // #if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON1_BIT0__)

#define LDO_CAL_MASK              0x01F0
#define LDO_CAL_SHIFT             4

 //AddForMT6573
#define LDO_FORCE_LOW_MASK               0x0004
#define LDO_FORCE_LOW_SHIFT              2

#define LDO_MODE_MASK               0x0008
#define LDO_MODE_SHIFT              3

#define LDO_MODE_SEL_MASK               0x0010
#define LDO_MODE_SEL_SHIFT              4
 
#define LDO_SRCLK_EN_MASK               0x0100
#define LDO_SRCLK_EN_SHIFT              8

//AddForMT6573 PMUA_CON1
#define VAUDP_BIST_EN_MASK               0x0001
#define VAUDP_BIST_EN_SHIFT              0

#define VTV_BIST_EN_MASK           0x0002
#define VTV_BIST_EN_SHIFT          1

#define THR_HWPDN_EN_MASK           0x0010
#define THR_HWPDN_EN_SHIFT          4

#define USBDL_EN_MASK           0x0020
#define USBDL_EN_SHIFT          5

  // LDO_XXX CON2
#define LDO_OC_TD_MASK            0x0030
#define LDO_OC_TD_SHIFT           4

#if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON2_BIT6__)
#define LDO_STB_TD_MASK           0x00C0
#define LDO_STB_TD_SHIFT          6
#endif // #if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON2_BIT6__)

#define LDO_ICAL_EN_MASK          0x3000
#define LDO_ICAL_EN_SHIFT         12

 //AddForMT6573
#define LDO_EN_FORCE_MASK               0x0001
#define LDO_EN_FORCE_SHIFT              0 

#define LDO_GPLDO_EN_MASK           0x0002
#define LDO_GPLDO_EN_SHIFT          1

//AddForMT6573 PMUA_CON2
#define LNIBIAS_TRIMCTRL_MASK           0x0003
#define LNIBIAS_TRIMCTRL_SHIFT          0

#define ABIST_S2D_EN_MASK           0x0004
#define ABIST_S2D_EN_SHIFT          2

#define ABIST_S2D2_BBRX_MASK           0x0008
#define ABIST_S2D2_BBRX_SHIFT          3

#define LNIBIAS_TRIM_MASK           0x01F0
#define LNIBIAS_TRIM_SHIFT          4

#define ABIST_TXBIAS2BBRX_MASK           0x0200
#define ABIST_TXBIAS2BBRX_SHIFT          9

#define ABIST_TXVGA2BBRX_MASK           0x0400
#define ABIST_TXVGA2BBRX_SHIFT          10

#define ABIST_APC2BBRX_MASK           0x0800
#define ABIST_APC2BBRX_SHIFT          11

#define ABIST_AFC2BBRX_MASK           0x1000
#define ABIST_AFC2BBRX_SHIFT          12

#define LNIBIAS_ENABLE_MASK           0x2000
#define LNIBIAS_ENABLE_SHIFT          13

 // LDO_XXX CON3
 //AddForMT6573
#define SIMX_MODE_MASK           0x0004
#define SIMX_MODE_SHIFT          2

#define SIMX_SRN_MASK           0x0030
#define SIMX_SRN_SHIFT          4

#define SIMX_SRP_MASK           0x00C0
#define SIMX_SRP_SHIFT          6 

#define SIMX_BIAS_MASK           0x0300
#define SIMX_BIAS_SHIFT          8

#define SIMXIO_DRV_MASK           0x1C00
#define SIMXIO_DRV_SHIFT          10

#define GPIO_SRST2_MASK           0x2000
#define GPIO_SRST2_SHIFT          13

#define GPIO_SCLK2_MASK           0x4000
#define GPIO_SCLK2_SHIFT          14

#define GPIO_SIO2_MASK           0x8000
#define GPIO_SIO2_SHIFT          15

//AddForMT6573 PMUA_CON3
#define ABIST_APC2ACCDET_MASK           0x0001
#define ABIST_APC2ACCDET_SHIFT          0

#define ABIST_AFC2AUX_YM_MASK           0x0002
#define ABIST_AFC2AUX_YM_SHIFT          1

#define ABIST_AFC2AUX_YP_MASK           0x0004
#define ABIST_AFC2AUX_YP_SHIFT          2

#define ABIST_AFC2AUX_XP_MASK           0x0008
#define ABIST_AFC2AUX_XP_SHIFT          3

#define ABIST_AUD2AUXADC_MASK           0x0010
#define ABIST_AUD2AUXADC_SHIFT          4

#define ABIST_TVDAC2AUXADC_MASK           0x0020
#define ABIST_TVDAC2AUXADC_SHIFT          5

#define VREF_BISTEN_MASK           0x0100
#define VREF_BISTEN_SHIFT          8

#define ABIST_AFC2FGADC_MASK           0x0200
#define ABIST_AFC2FGADC_SHIFT          9

#define ABIST_GND2FGADC_MASK           0x0C00
#define ABIST_GND2FGADC_SHIFT          10

#endif // #if defined(__DRV_UPMU_LDO_V1__)

#if defined(__DRV_UPMU_BUCK_V1__)
#define BUCK_CON0_OFFSET          0x00
#define BUCK_CON1_OFFSET          0x04
#define BUCK_CON2_OFFSET          0x08
#define BUCK_CON3_OFFSET          0x0C
#define BUCK_CON4_OFFSET          0x10
#define BUCK_CON5_OFFSET          0x14
#define BUCK_CON6_OFFSET          0x18
// BUCK H/W register bitmap definition
  // BUCK_XXX CON0
#define BUCK_EN_MASK              0x0001
#define BUCK_EN_SHIFT             0

#define BUCK_ON_SEL_MASK          0x0002
#define BUCK_ON_SEL_SHIFT         1

#define BUCK_RS_MASK              0x0004
#define BUCK_RS_SHIFT             2

#define BUCK_ST_STR_MASK          0x0008
#define BUCK_ST_STR_SHIFT         3

#define BUCK_VFBADJ_MASK          0x01F0
#define BUCK_VFBADJ_SHIFT         4

#define BUCK_DIS_ANTIUNSH_MASK    0x0400
#define BUCK_DIS_ANTIUNSH_SHIFT   10

#define BUCK_STB_EN_MASK          0x0800
#define BUCK_STB_EN_SHIFT         11

#define BUCK_OC_AUTO_OFF_MASK     0x1000
#define BUCK_OC_AUTO_OFF_SHIFT    12

#define BUCK_OCFB_EN_MASK         0x2000
#define BUCK_OCFB_EN_SHIFT        13

 //AddForMT6573
#define BUCK_OC_STATUS_MASK        0x4000
#define BUCK_OC_STATUS_SHIFT       14

#define BUCK_STATUS_MASK           0x8000
#define BUCK_STATUS_SHIFT          15 

  // BUCK_XXX CON1
#define BUCK_MODESET_MASK         0x0001
#define BUCK_MODESET_SHIFT        0

#define BUCK_VFBADJ_SLEEP_MASK    0x01F0
#define BUCK_VFBADJ_SLEEP_SHIFT   4

#define BUCK_CLK_SRC_MASK         0x0400
#define BUCK_CLK_SRC_SHIFT        10

#define MODE_STATUS_MASK           0x8000
#define MODE_STATUS_SHIFT          15

  // BUCK_XXX CON2
#define BUCK_CAL_MASK             0x01F0
#define BUCK_CAL_SHIFT            4

 //AddForMT6573
#define BUCK_VOSEL_MASK             0x0007
#define BUCK_VOSEL_SHIFT            0

  // BUCK_XXX CON3
#define BUCK_OC_TD_MASK           0x0030
#define BUCK_OC_TD_SHIFT          4

#define BUCK_STB_TD_MASK          0x00C0
#define BUCK_STB_TD_SHIFT         6

#define BUCK_OC_THD_MASK          0x0300
#define BUCK_OC_THD_SHIFT         8

#define BUCK_OC_WND_MASK          0x0C00
#define BUCK_OC_WND_SHIFT         10

#define BUCK_ICAL_EN_MASK         0x3000
#define BUCK_ICAL_EN_SHIFT        12

 //AddForMT6573
#define BUCK_EN_FORCE_MASK             0x0001
#define BUCK_EN_FORCE_SHIFT            0

  // BUCK XXX CON4
  //AddForMT6573 
#define BUCK_SLEW_NMOS_MASK             0x0003
#define BUCK_SLEW_NMOS_SHIFT            0 

#define BUCK_SLEW_MASK             0x000C
#define BUCK_SLEW_SHIFT            2

#define BUCK_ADJCKSEL_MASK             0x0070
#define BUCK_ADJCKSEL_SHIFT            4

  // BUCK XXX CON5
#define BUCK_CSL_MASK             0x0700
#define BUCK_CSL_SHIFT            8

#define BUCK_BURST_MASK           0x3000
#define BUCK_BURST_SHIFT          12

 //AddForMT6573
#define BUCK_CSR_MASK           0x000F
#define BUCK_CSR_SHIFT          0 

#define BUCK_RZSEL_MASK           0x0070
#define BUCK_RZSEL_SHIFT          4 

#define BUCK_GMSEL_MASK           0x4000
#define BUCK_GMSEL_SHIFT          14 

#define BUCK_ZX_PDN_MASK           0x8000
#define BUCK_ZX_PDN_SHIFT          15 

#endif // #if defined(__DRV_UPMU_BUCK_V1__)

#if defined(__DRV_UPMU_BUCK_BOOST_V1__)
#define BUCK_BOOST_CON0_OFFSET         0x00
#define BUCK_BOOST_CON1_OFFSET         0x04
#define BUCK_BOOST_CON2_OFFSET         0x08
#define BUCK_BOOST_CON3_OFFSET         0x0C
#define BUCK_BOOST_CON4_OFFSET         0x10
#define BUCK_BOOST_CON5_OFFSET         0x14

 // BUCK_BOOST_XXX CON0
#define BUCK_BOOST_CC_MASK           0x0003
#define BUCK_BOOST_CC_SHIFT          0

#define BUCK_BOOST_RC_MASK           0x000C
#define BUCK_BOOST_RC_SHIFT          2

#define BUCK_BOOST_SR2_MASK           0x0030
#define BUCK_BOOST_SR2_SHIFT          4

#define BUCK_BOOST_SR1_MASK           0x00C0
#define BUCK_BOOST_SR1_SHIFT          6

#define BUCK_BOOST_CS_ADJ_MASK           0x0700
#define BUCK_BOOST_CS_ADJ_SHIFT          8

#define BUCK_BOOST_ANTI_RING_MASK           0x0800
#define BUCK_BOOST_ANTI_RING_SHIFT          11

#define BUCK_BOOST_EN_MASK           0x1000
#define BUCK_BOOST_EN_SHIFT          12

 // BUCK_BOOST_XXX CON1
#define BUCK_BOOST_FPWM_MASK           0x0100
#define BUCK_BOOST_FPWM_SHIFT          8

#define BUCK_BOOST_TM_MASK           0x0200
#define BUCK_BOOST_TM_SHIFT          9

#define BUCK_BOOST_SS_SPD_MASK           0x0400
#define BUCK_BOOST_SS_SPD_SHIFT          10

#define BUCK_BOOST_STATUS_MASK           0x8000
#define BUCK_BOOST_STATUS_SHIFT          15

 // BUCK_BOOST_XXX CON2
#define BUCK_BOOST_VOTUNE_0_MASK           0x001F
#define BUCK_BOOST_VOTUNE_0_SHIFT          0

#define BUCK_BOOST_VOTUNE_1_MASK           0x1F00
#define BUCK_BOOST_VOTUNE_1_SHIFT          8

 // BUCK_BOOST_XXX CON3
#define BUCK_BOOST_VOTUNE_2_MASK           0x001F
#define BUCK_BOOST_VOTUNE_2_SHIFT          0

#define BUCK_BOOST_VOTUNE_3_MASK           0x1F00
#define BUCK_BOOST_VOTUNE_3_SHIFT          8

 // BUCK_BOOST_XXX CON4
#define BUCK_BOOST_VOTUNE_4_MASK           0x001F
#define BUCK_BOOST_VOTUNE_4_SHIFT          0

#define BUCK_BOOST_VOTUNE_5_MASK           0x1F00
#define BUCK_BOOST_VOTUNE_5_SHIFT          8

 // BUCK_BOOST_XXX CON5
#define BUCK_BOOST_VOTUNE_6_MASK           0x001F
#define BUCK_BOOST_VOTUNE_6_SHIFT          0

#define BUCK_BOOST_VOTUNE_7_MASK           0x1F00
#define BUCK_BOOST_VOTUNE_7_SHIFT          8

#endif // #if defined(__DRV_UPMU_BUCK_BOOST_V1__)

#if defined(__DRV_UPMU_BOOST_V1__)
#define BOOST_CON0_OFFSET         0x00
#define BOOST_CON1_OFFSET         0x04
#define BOOST_CON2_OFFSET         0x08
#define BOOST_CON3_OFFSET         0x0C
#define BOOST_CON4_OFFSET         0x10
#define BOOST_CON5_OFFSET         0x14
#define BOOST_CON6_OFFSET         0x18
// BOOST H/W register bitmap definition
  // BOOST_XXX CON0
#define BOOST_EN_MASK             0x0001
#define BOOST_EN_SHIFT            0

#define BOOST_TYPE_MASK           0x0002
#define BOOST_TYPE_SHIFT          1

#define BOOST_MODE_MASK           0x0004
#define BOOST_MODE_SHIFT          2

#define BOOST_VRSEL_MASK          0x01F0
#define BOOST_VRSEL_SHIFT         4

#define BOOST_OC_AUTO_OFF_MASK    0x1000
#define BOOST_OC_AUTO_OFF_SHIFT   12

#define BOOST_OC_FLAG_MASK        0x4000
#define BOOST_OC_FLAG_SHIFT       14

  // BOOST_XXX CON1
#define BOOST_CL_MASK             0x0007
#define BOOST_CL_SHIFT            0

#define BOOST_CS_MASK             0x0070
#define BOOST_CS_SHIFT            4

#define BOOST_RC_MASK             0x0700
#define BOOST_RC_SHIFT            8

#define BOOST_SS_MASK             0x7000
#define BOOST_SS_SHIFT            12

  // BOOST_XXX CON2
#define BOOST_CKSEL_MASK          0x0002
#define BOOST_CKSEL_SHIFT         1

#define BOOST_SR_PMOS_MASK        0x0070
#define BOOST_SR_PMOS_SHIFT       4

#define BOOST_SR_NMOS_MASK        0x0700
#define BOOST_SR_NMOS_SHIFT       8

#define BOOST_SLP_MASK            0xC000
#define BOOST_SLP_SHIFT           14

  // BOOST_XXX CON3
#define BOOST_CKS_PRG_MASK        0x003F
#define BOOST_CKS_PRG_SHIFT       0

  // BOOST_XXX CON4
#define BOOST_OC_THD_MASK         0x0030
#define BOOST_OC_THD_SHIFT        4

#define BOOST_OC_WND_MASK         0x00C0
#define BOOST_OC_WND_SHIFT        6

#define BOOST_CLK_CAL_MASK        0x7000
#define BOOST_CLK_CAL_SHIFT       12

  // BOOST_XXX CON6
#define BOOST_HW_SEL_MASK         0x0001
#define BOOST_HW_SEL_SHIFT        0

#define BOOST_CC_MASK             0x0070
#define BOOST_CC_SHIFT            4

#endif // #if defined(__DRV_UPMU_BOOST_V1__)


#if defined(__DRV_UPMU_ISINK_V1__)
#define ISINK_CON0_OFFSET         0x00
#define ISINK_CON1_OFFSET         0x04
#define ISINK_CON2_OFFSET         0x08
// iSINK H/W register bitmap definition
  // ISINK_XXX CON0
#define ISINK_EN_MASK             0x0001
#define ISINK_EN_SHIFT            0

#define ISINK_MODE_MASK           0x0002
#define ISINK_MODE_SHIFT          1

#define ISINK_STEP_MASK           0x01F0
#define ISINK_STEP_SHIFT          4

#define ISINK_STATUS_MASK         0x8000
#define ISINK_STATUS_SHIFT        15

  // ISINK_XXX CON1
#define ISINK_VREF_CAL_MASK       0x1F00
#define ISINK_VREF_CAL_SHIFT      8

#endif // #if defined(__DRV_UPMU_ISINK_V1__)


#if defined(__DRV_UPMU_KPLED_V1__)
#define KPLED_CON0_OFFSET         0x00
#define KPLED_CON1_OFFSET         0x04
// KPLED H/W register bitmap definition
  // KPLED_XXX CON0
#define KPLED_EN_MASK             0x0001
#define KPLED_EN_SHIFT            0

#define KPLED_MODE_MASK           0x0002
#define KPLED_MODE_SHIFT          1

#define KPLED_SEL_MASK            0x0070
#define KPLED_SEL_SHIFT           4

#define KPLED_SFSTRT_C_MASK       0x0300
#define KPLED_SFSTRT_C_SHIFT      8

#define KPLED_SFSTRT_EN_MASK      0x0400
#define KPLED_SFSTRT_EN_SHIFT     10

#define KPLED_STATUS_MASK         0x8000
#define KPLED_STATUS_SHIFT        15

 //AddForMT6573
#define KPLED_THER_SHDN_EN_MASK         0x0800
#define KPLED_THER_SHDN_EN_SHIFT        11 

 // KPLED_XXX CON1
 //AddForMT6573
#define KPLED_FORCE_OFF_EN_MASK         0x0001
#define KPLED_FORCE_OFF_EN_SHIFT        0

#define KPLED_TEST_MODE_EN_MASK         0x0020
#define KPLED_TEST_MODE_EN_SHIFT        5   

#endif // #if defined(__DRV_UPMU_KPLED_V1__)


#if defined(__DRV_UPMU_SPK_V1__)
#define SPK_CON0_OFFSET           0x00
#define SPK_CON1_OFFSET           0x04
#define SPK_CON2_OFFSET           0x08
#define SPK_CON3_OFFSET           0x0C
#define SPK_CON4_OFFSET           0x10
#define SPK_CON5_OFFSET           0x14
#define SPK_CON6_OFFSET           0x18
#define SPK_CON7_OFFSET           0x1C
// SPK H/W register bitmap definition
  // SPK_XXX CON0
#define SPK_EN_MASK               0x0001
#define SPK_EN_SHIFT              0

#define SPK_RST_MASK              0x0002
#define SPK_RST_SHIFT             1

#define SPK_VOL_MASK              0x01F0
#define SPK_VOL_SHIFT             4

#define SPK_OC_AUTO_OFF_MASK      0x1000
#define SPK_OC_AUTO_OFF_SHIFT     12

#define SPK_OCFB_EN_MASK          0x2000
#define SPK_OCFB_EN_SHIFT         13

#define SPK_OC_FLAG_MASK          0x4000
#define SPK_OC_FLAG_SHIFT         14

  // SPK_XXX CON1
#define SPK_PFD_MODE_MASK         0x0001
#define SPK_PFD_MODE_SHIFT        0

#define SPK_CMODE_MASK            0x000C
#define SPK_CMODE_SHIFT           2

#define SPK_CCODE_MASK            0x00F0
#define SPK_CCODE_SHIFT           4

  // SPK_XXX CON2
#define SPK_OC_THD_MASK           0x0030
#define SPK_OC_THD_SHIFT          4

#define SPK_OC_WND_MASK           0x00C0
#define SPK_OC_WND_SHIFT          6

  // SPK_XXX CON3
#define SPK_OC_EN_MASK            0x0400
#define SPK_OC_EN_SHIFT           10
#define SPK_OSC_ISEL_MASK         0x00C0
#define SPK_OSC_ISEL_SHIFT        6

  // SPK_XXX CON7
#define SPK_AB_OBIAS_MASK         0x0030
#define SPK_AB_OBIAS_SHIFT        4
#define SPK_AB_OC_EN_MASK         0x0100
#define SPK_AB_OC_EN_SHIFT        8
#define SPK_MODE_MASK             0x0001
#define SPK_MODE_SHIFT            0


#endif // #if defined(__DRV_UPMU_SPK_V1__)


#if defined(__DRV_UPMU_CHARGER_V1__)
#define CHR_CON0_OFFSET           0x00
#define CHR_CON1_OFFSET           0x04
#define CHR_CON2_OFFSET           0x08
#define CHR_CON3_OFFSET           0x0C
#define CHR_CON4_OFFSET           0x10
#define CHR_CON5_OFFSET           0x14
#define CHR_CON6_OFFSET           0x18
#define CHR_CON7_OFFSET           0x1C
#define CHR_CON8_OFFSET           0x20
#define CHR_CON9_OFFSET           0x24
#define CHR_BC11_CON0_OFFSET      CHR_CON9_OFFSET
#define CHR_CON10_OFFSET          0x28
#define CHR_BC11_CON1_OFFSET      CHR_CON10_OFFSET
// CHARGER H/W register bitmap definition
  // CHR_XXX CON0
#define VCDT_LV_VTH_MASK          0x000F
#define VCDT_LV_VTH_SHIFT         0

#define VCDT_HV_VTH_MASK          0x00F0
#define VCDT_HV_VTH_SHIFT         4

#define VCDT_HV_EN_MASK           0x0100
#define VCDT_HV_EN_SHIFT          8

#define CHR_LDO_DET_MASK           0x0200
#define CHR_LDO_DET_SHIFT          9

#define PCHR_AUTO_MASK            0x0400
#define PCHR_AUTO_SHIFT           10

#define CSDAC_EN_MASK             0x0800
#define CSDAC_EN_SHIFT            11

#define CHR_EN_MASK               0x1000
#define CHR_EN_SHIFT              12

#define CHRDET_MASK               0x2000
#define CHRDET_SHIFT              13

#define VCDT_LV_DET_MASK          0x4000
#define VCDT_LV_DET_SHIFT         14

#define VCDT_HV_DET_MASK          0x8000
#define VCDT_HV_DET_SHIFT         15

  // CHR_XXX CON1
#define VBAT_CV_VTH_MASK          0x001F
#define VBAT_CV_VTH_SHIFT         0

#define VBAT_CC_VTH_MASK          0x00C0
#define VBAT_CC_VTH_SHIFT         6

#define VBAT_CV_EN_MASK           0x0100
#define VBAT_CV_EN_SHIFT          8

#define VBAT_CC_EN_MASK           0x0200
#define VBAT_CC_EN_SHIFT          9

#define VBAT_CV_DET_MASK          0x4000
#define VBAT_CV_DET_SHIFT         14

#define VBAT_CC_DET_MASK          0x8000
#define VBAT_CC_DET_SHIFT         15

  // CHR_XXX CON2
#define PCHR_TOHTC_MASK           0x0007
#define PCHR_TOHTC_SHIFT          0

#define PCHR_TOLTC_MASK           0x0070
#define PCHR_TOLTC_SHIFT          4

#define CS_VTH_MASK               0x0700
#define CS_VTH_SHIFT              8

#define CS_EN_MASK                0x1000
#define CS_EN_SHIFT               12

#if defined(__DRV_OTG_BVALID_DET_AT_CON2_BIT14__)
#define OTG_BVALID_DET_MASK       0x4000
#define OTG_BVALID_DET_SHIFT      14
#endif // #if defined(__DRV_OTG_BVALID_DET_AT_CON2_BIT14__)

#define CS_DET_MASK               0x8000
#define CS_DET_SHIFT              15

  // CHR_XXX CON3
#define CSDAC_STP_MASK            0x0003
#define CSDAC_STP_SHIFT           0

#if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT3__)
#define VBAT_OV_EN_MASK           0x0008
#define VBAT_OV_EN_SHIFT          3
#endif // #if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT3__)

#define CSDAC_DLY_MASK            0x0030
#define CSDAC_DLY_SHIFT           4

#define VBAT_OV_VTH_MASK          0x00C0
#define VBAT_OV_VTH_SHIFT         6

#if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT8__)
#define VBAT_OV_EN_MASK           0x0100
#define VBAT_OV_EN_SHIFT          8
#endif // #if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT8__)

#if defined(__DRV_BATON_EN_AT_CON3_BIT9__)
#define BATON_EN_MASK             0x0200
#define BATON_EN_SHIFT            9
#endif // #if defined(__DRV_BATON_EN_AT_CON3_BIT9__)

#if defined(__DRV_BATON_EN_AT_CON3_BIT12__)
#define BATON_EN_MASK             0x1000
#define BATON_EN_SHIFT            12
#endif // #if defined(__DRV_BATON_EN_AT_CON3_BIT12__)

#if defined(__DRV_OTG_BVALID_EN_AT_CON3_BIT13__)
#define OTG_BVALID_EN_MASK        0x2000
#define OTG_BVALID_EN_SHIFT       13
#endif // #if defined(__DRV_OTG_BVALID_EN_AT_CON3_BIT13__)

#define VBAT_OV_DET_MASK          0x4000
#define VBAT_OV_DET_SHIFT         14

#define BATON_UNDET_MASK          0x8000
#define BATON_UNDET_SHIFT         15

 //AddForMT6573
#define BATON_HTEN_MASK          0x0400
#define BATON_HTEN_SHIFT         10

#define VBAT_OV_DEG_MASK          0x0800
#define VBAT_OV_DEG_SHIFT         11

  // CHR_XXX CON4
#define PCHR_TEST_MASK            0x0001
#define PCHR_TEST_SHIFT           0

#define PCHR_CSDAC_TEST_MASK      0x0002
#define PCHR_CSDAC_TEST_SHIFT     1

#define PCHR_RST_MASK             0x0004
#define PCHR_RST_SHIFT            2

#define CSDAC_DAT_MASK            0xFF00
#define CSDAC_DAT_SHIFT           8

  // CHR_XXX CON5
#define PCHR_FLAG_SEL_MASK        0x000F
#define PCHR_FLAG_SEL_SHIFT       0

#define PCHR_FLAG_EN_MASK         0x0080
#define PCHR_FLAG_EN_SHIFT        7

#define PCHR_FLAG_OUT_MASK        0x0F00
#define PCHR_FLAG_OUT_SHIFT       8

#if defined(__DRV_OTG_BVALID_EN_AT_CON5_BIT12__)
#define OTG_BVALID_EN_MASK        0x1000
#define OTG_BVALID_EN_SHIFT       12
#endif // #if defined(__DRV_OTG_BVALID_EN_AT_CON5_BIT12__)

#if defined(__DRV_OTG_BVALID_DET_AT_CON5_BIT15__)
#define OTG_BVALID_DET_MASK       0x8000
#define OTG_BVALID_DET_SHIFT      15
#endif // #if defined(__DRV_OTG_BVALID_DET_AT_CON5_BIT15__)

 //AddForMT6573
#define PCHR_FT_CTRL_MASK       0x0070
#define PCHR_FT_CTRL_SHIFT      4 

  // CHR_XXX CON6
#define CHRWDT_TD_MASK            0x000F  // TTTTTTTTT
#define CHRWDT_TD_SHIFT           0

#define CHRWDT_EN_MASK            0x0010
#define CHRWDT_EN_SHIFT           4

  // CHR_XXX CON7
#define CHRWDT_INT_EN_MASK        0x0001
#define CHRWDT_INT_EN_SHIFT       0

#define CHRWDT_FLAG_WR_MASK       0x0002
#define CHRWDT_FLAG_WR_SHIFT      1

#define CHRWDT_OUT_MASK           0x8000
#define CHRWDT_OTU_SHIFT          15

  // CHR_XXX CON8
#define BGR_RSEL_MASK             0x0007
#define BGR_RSEL_SHIFT            0

#define BGR_UNCHOP_PH_MASK        0x0010
#define BGR_UNCHOP_PH_SHIFT       4

#define BGR_UNCHOP_MASK           0x0020
#define BGR_UNCHOP_SHIFT          5

#define UVLO_VTHL_MASK            0x0300
#define UVLO_VTHL_SHIFT           8

#define ADC_EN_MASK               0x7000   // All ADC channels are enabled at same time
#define ADC_EN_SHIFT              12

  //AddForMT6573
#define USBDL_RST_MASK            0x0400
#define USBDL_RST_SHIFT           10  

#define USBDL_SET_MASK            0x0800
#define USBDL_SET_SHIFT           11  

#define ADCIN_VBAT_EN_MASK            0x1000    // VBAT
#define ADCIN_VBAT_EN_SHIFT           12

#define ADCIN_VSEN_EN_MASK            0x2000    // VSEN
#define ADCIN_VSEN_EN_SHIFT           13

#define ADCIN_CHR_EN_MASK            0x4000    // VCHR
#define ADCIN_CHR_EN_SHIFT           14

  // CHR_XXX CON9 (CHR_BC11_CON0)
#if defined(__DRV_UPMU_BC11_V1__)
#define BC11_VREF_VTH_MASK        0x0001
#define BC11_VREF_VTH_SHIFT       0
#define BC11_CMP_EN_MASK          0x0006
#define BC11_CMP_EN_SHIFT         1
#define BC11_IPD_EN_MASK          0x0018
#define BC11_IPD_EN_SHIFT         3
#define BC11_IPU_EN_MASK          0x0060
#define BC11_IPU_EN_SHIFT         5
#define BC11_BIAS_EN_MASK         0x0080
#define BC11_BIAS_EN_SHIFT        7
#define BC11_BB_CTRL_MASK         0x0100
#define BC11_BB_CTRL_SHIFT        8
#define BC11_RST_MASK             0x0200
#define BC11_RST_SHIFT            9
#define BC11_CMP_OUT_MASK         0x1000
#define BC11_CMP_OUT_SHIFT        15
#endif // #if defined(__DRV_UPMU_BC11_V1__)

 //AddForMT6573
#if defined(__DRV_UPMU_BC11_V2__)
// CHR_XXX CON9 (CHR_BC11_CON0)
#define BC11_VSRC_EN_MASK        0x0003
#define BC11_VSRC_EN_SHIFT       0

//#define PCHR_FLAG_SEL_MASK        0x0004
//#define PCHR_FLAG_SEL_SHIFT       2

#define BC11_REV_MASK        0x00F8
#define BC11_REV_SHIFT       3

#define BC11_RST_MASK        0x0100
#define BC11_RST_SHIFT       8

// CHR_XXX CON10 (CHR_BC11_CON1)
#define BC11_CMP_EN_MASK        0x0003
#define BC11_CMP_EN_SHIFT       0

#define BC11_IPD_EN_MASK        0x000C
#define BC11_IPD_EN_SHIFT       2

#define BC11_IPU_EN_MASK        0x0030
#define BC11_IPU_EN_SHIFT       4

#define BC11_VREF_VTH_MASK        0x0040
#define BC11_VREF_VTH_SHIFT       6

#define BC11_BIAS_EN_MASK        0x0080
#define BC11_BIAS_EN_SHIFT       7

#define BC11_BB_CTRL_MASK        0x0100
#define BC11_BB_CTRL_SHIFT       8

#define BC11_CMP_OUT_MASK        0x0200
#define BC11_CMP_OUT_SHIFT       9

#endif // #if defined(__DRV_UPMU_BC11_V2__)

#endif // #if defined(__DRV_UPMU_CHARGER_V1__)


 //AddForMT6573
#if defined(__DRV_UPMU_OC_V1__)
#define OC_CON0_OFFSET           0x00
#define OC_CON1_OFFSET           0x04
#define OC_CON2_OFFSET           0x08
//#define OC_CON3_OFFSET           0x0C
#define OC_CON4_OFFSET           0x10
#define OC_CON5_OFFSET           0x14
#define OC_CON6_OFFSET           0x18

// PMIC_OC_CON0
#define VRF_OC_INT_EN_MASK        0x0001
#define VRF_OC_INT_EN_SHIFT       0

#define VCAMA_OC_INT_EN_MASK        0x0008
#define VCAMA_OC_INT_EN_SHIFT       3

#define VCAMD_OC_INT_EN_MASK        0x0010
#define VCAMD_OC_INT_EN_SHIFT       4

#define VIO_OC_INT_EN_MASK        0x0020
#define VIO_OC_INT_EN_SHIFT       5

#define VUSB_OC_INT_EN_MASK        0x0040
#define VUSB_OC_INT_EN_SHIFT       6

#define VSIM_OC_INT_EN_MASK        0x0100
#define VSIM_OC_INT_EN_SHIFT       8

#define VSIM2_OC_INT_EN_MASK        0x0200
#define VSIM2_OC_INT_EN_SHIFT       9

#define VIBR_OC_INT_EN_MASK        0x0800
#define VIBR_OC_INT_EN_SHIFT       11

#define VMC_OC_INT_EN_MASK        0x1000
#define VMC_OC_INT_EN_SHIFT       12

#define VCAMA2_OC_INT_EN_MASK        0x2000
#define VCAMA2_OC_INT_EN_SHIFT       13

#define VCAMD2_OC_INT_EN_MASK        0x4000
#define VCAMD2_OC_INT_EN_SHIFT       14

// PMIC_OC_CON1
#define VM12_OC_INT_EN_MASK        0x0001
#define VM12_OC_INT_EN_SHIFT       0

#define VM12_INT_OC_INT_EN_MASK        0x0002
#define VM12_INT_OC_INT_EN_SHIFT       1

// PMIC_OC_CON2
#define VCORE_OC_INT_EN_MASK        0x0001
#define VCORE_OC_INT_EN_SHIFT       0

#define VIO1V8_OC_INT_EN_MASK        0x0002
#define VIO1V8_OC_INT_EN_SHIFT       1

#define VAPROC_OC_INT_EN_MASK        0x0004
#define VAPROC_OC_INT_EN_SHIFT       2

#define VRF1V8_OC_INT_EN_MASK        0x0008
#define VRF1V8_OC_INT_EN_SHIFT       3

// PMIC_OC_CON4
#define VRF_OC_FLAG_MASK        0x0001
#define VRF_OC_FLAG_SHIFT       0

#define VCAMA_OC_FLAG_MASK        0x0008
#define VCAMA_OC_FLAG_SHIFT       3

#define VCAMD_OC_FLAG_MASK        0x0010
#define VCAMD_OC_FLAG_SHIFT       4

#define VIO_OC_FLAG_MASK        0x0020
#define VIO_OC_FLAG_SHIFT       5

#define VUSB_OC_FLAG_MASK        0x0040
#define VUSB_OC_FLAG_SHIFT       6

#define VSIM_OC_FLAG_MASK        0x0100
#define VSIM_OC_FLAG_SHIFT       8

#define VSIM2_OC_FLAG_MASK        0x0200
#define VSIM2_OC_FLAG_SHIFT       9

#define VBIR_OC_FLAG_MASK        0x0800
#define VBIR_OC_FLAG_SHIFT       11

#define VMC_OC_FLAG_MASK        0x1000
#define VMC_OC_FLAG_SHIFT       12

#define VCAMA2_OC_FLAG_MASK        0x2000
#define VCAMA2_OC_FLAG_SHIFT       13

#define VCAMD2_OC_FLAG_MASK        0x4000
#define VCAMD2_OC_FLAG_SHIFT       14

// PMIC_OC_CON5
#define VM12_OC_FLAG_MASK        0x0001
#define VM12_OC_FLAG_SHIFT       0

#define VM12_INT_OC_FLAG_MASK        0x0002
#define VM12_INT_OC_FLAG_SHIFT       1

// PMIC_OC_CON6
#define VCORE_OC_FLAG_MASK        0x0001
#define VCORE_OC_FLAG_SHIFT       0

#define VIO1V8_OC_FLAG_MASK        0x0002
#define VIO1V8_OC_FLAG_SHIFT       1

#define VAPROC_OC_FLAG_MASK        0x0004
#define VAPROC_OC_FLAG_SHIFT       2

#define VRF1V8_OC_FLAG_MASK        0x0008
#define VRF1V8_OC_FLAG_SHIFT       3

#endif // #if defined(__DRV_UPMU_OC_V1__)


 //AddForMT6573
#if defined(__DRV_UPMU_STRUP_V1__)
#define STRUP_CON0_OFFSET           0x00
#define STRUP_CON1_OFFSET           0x04
//#define STRUP_CON2_OFFSET           0x08
#define STRUP_CON3_OFFSET           0x0C
#define STRUP_CON4_OFFSET           0x10

// STRUP_CON0
#define STRUP_IREF_ADJ_MASK        0x0007
#define STRUP_IREF_ADJ_SHIFT       0

#define STRUP_THR_SEL_MASK        0x0300
#define STRUP_THR_SEL_SHIFT       8

#define TEST_MODE_POR_MASK        0x2000
#define TEST_MODE_POR_SHIFT       13

#define PWRKEY_VCORE_MASK        0x4000
#define PWRKEY_VCORE_SHIFT       14

#define PWRKEY_DEB_MASK        0x8000
#define PWRKEY_DEB_SHIFT       15

// STRUP_CON1
#define DIG_TEST_EN_MASK        0x0010
#define DIG_TEST_EN_SHIFT       4

#define RST_DRVSEL_MASK        0x0020
#define RST_DRVSEL_SHIFT       5

#define PMU_LEV_UNGATE_MASK        0x0100
#define PMU_LEV_UNGATE_SHIFT       8

#define STRUP_FLAG_EN_MASK        0x1000
#define STRUP_FLAG_EN_SHIFT       12

#define BIAS_GEN_FORCE_MASK        0x4000
#define BIAS_GEN_FORCE_SHIFT       14

// STRUP_CON3
#define STRUP_FLAG_SEL_MASK        0xF000
#define STRUP_FLAG_SEL_SHIFT       12

// STRUP_CON4

#endif // #if defined(__DRV_UPMU_STRUP_V1__)


 //AddForMT6573
#if defined(__DRV_UPMU_LPOSC_V1__)
#define LPOSC_CON0_OFFSET           0x00
#define LPOSC_CON1_OFFSET           0x04
#define LPOSC_CON2_OFFSET           0x08
//#define LPOSC_CON3_OFFSET           0x0C
#define LPOSC_CON3_OFFSET           0x10
//#define LPOSC_CON4_OFFSET           0x10
#define LPOSC_CON4_OFFSET           0x14


// LPOSC_CON0
#define LPOSC_IBIAS_CALI_MASK        0x0700
#define LPOSC_IBIAS_CALI_SHIFT       8

#define LPOSC_EN_MASK        0x1000
#define LPOSC_EN_SHIFT       12

// LPOSC_CON1
#define LPOSC_FREQ_SET_MASK        0x00FF
#define LPOSC_FREQ_SET_SHIFT       0

#define LPOSC_BUCK_FREQ_MASK        0x0700
#define LPOSC_BUCK_FREQ_SHIFT       8

#define LPOSC_BBCLK_DIB_MASK        0x3000
#define LPOSC_BBCLK_DIB_SHIFT       12

#define LPOSC_ACALI_EN_MASK        0x4000
#define LPOSC_ACALI_EN_SHIFT       14

// LPOSC_CON2
#define LPOSC_FD_RES_MASK        0x0007
#define LPOSC_FD_RES_SHIFT       0

#define LPOSC_FD_DIS_DUR1_MASK        0x0030
#define LPOSC_FD_DIS_DUR1_SHIFT       4

#define LPOSC_FD_DIS_DUR2_MASK        0x00C0
#define LPOSC_FD_DIS_DUR2_SHIFT       6

#define LPOSC_SSC_MOD_AMP_MASK        0x0700
#define LPOSC_SSC_MOD_AMP_SHIFT       8

#define LPOSC_BUCK_BOOST_EN_MASK        0x0800
#define LPOSC_BUCK_BOOST_EN_SHIFT       11

#define LPOSC_SSC_CODE_DUR_MASK        0x7000
#define LPOSC_SSC_CODE_DUR_SHIFT       12

#define LPOSC_PG_EN_MASK        0x8000
#define LPOSC_PG_EN_SHIFT       15

// LPOSC_CON3
#define LPOSC_BUCK4_PS_MASK        0x0007
#define LPOSC_BUCK4_PS_SHIFT       0

#define LPOSC_BUCK3_PS_MASK        0x0070
#define LPOSC_BUCK3_PS_SHIFT       4

#define LPOSC_BUCK2_PS_MASK        0x0700
#define LPOSC_BUCK2_PS_SHIFT       8

#define LPOSC_BUCK1_PS_MASK        0x7000
#define LPOSC_BUCK1_PS_SHIFT       12

// LPOSC_CON4
#define LPOSC_BUCK6_PS_MASK        0x0007
#define LPOSC_BUCK6_PS_SHIFT       0

#define LPOSC_BUCK5_PS_MASK        0x0070
#define LPOSC_BUCK5_PS_SHIFT       4

#define LPOSC_INIT_DAC_EN_MASK        0x0100
#define LPOSC_INIT_DAC_EN_SHIFT       8

#endif // #if defined(__DRV_UPMU_LPOSC_V1__)


 //AddForMT6573
#if defined(__DRV_UPMU_Retention_V1__)
#define RETENTION_CON0_OFFSET           0x00
#define DATA_RETENTION_MASK        0xFFFF
#define DATA_RETENTION_SHIFT       0
#endif // #if defined(__DRV_UPMU_Retention_V1__)

//AddForMT6573_2
#define LPOSC_SW_MODE_MASK        0x0080
#define LPOSC_SW_MODE_SHIFT       7

#endif // #ifndef __UPMU_HW_H__
