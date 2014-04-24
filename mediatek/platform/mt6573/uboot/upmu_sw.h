

#ifndef __UPMU_SW_H__
#define __UPMU_SW_H__

#include "pmic_features.h"

#if defined(__DRV_UPMU_LDO_V1__)
// LDO S/W strcture definition
typedef enum
{
	LDO_LOCAL_SENSE = 0,
	LDO_REMOTE_SENSE = 1
}upmu_ldo_rs_enum;

typedef enum
{
	LDO_ENABLE_WITH_SRCLKEN = 0,
	LDO_ENABLE_LDO_EN_REGISTER = 1
}upmu_ldo_on_sel_enum;

typedef enum
{
	LDO_OC_TD_100_US = 0,
	LDO_OC_TD_200_US = 1,
	LDO_OC_TD_400_US = 2,
	LDO_OC_TD_800_US = 3
}upmu_ldo_oc_td_enum;

typedef enum
{
	LDO_STB_TD_200_US = 0,
	LDO_STB_TD_400_US = 1,
	LDO_STB_TD_600_US = 2,
	LDO_STB_TD_800_US = 3
}upmu_ldo_stb_td_enum;

typedef enum
{
	LDO_ICAL_X_1_0 = 0,
	LDO_ICAL_X_0_5 = 1,
	LDO_ICAL_X_2_0 = 2,
	LDO_ICAL_X_3_0 = 3
}upmu_ldo_ical_en_enum;

//AddForMT6573
typedef enum
{
	DRIVEN_BY_THE_ERROR_AMP = 0,
	DRIVEN_TO_VBAT_2_5 = 1
}upmu_pmt_passgate_ovrd_enum;

typedef enum
{
	CONTROL_BY_SYSTEM = 0,
	CONTROL_BY_LDO_MODE = 1
}upmu_ldo_mode_sel_enum;

typedef enum
{
	CONTROL_BY_SIM_CONTROLLER = 0,
	CONTROL_BY_VSIM_CON0 = 1
}upmu_ldo_gpldo_enum;

typedef enum
{
	SIM_APPLICATION = 0,
	GPIO_FUNCTION = 1
}upmu_ldo_vsimx_mode_enum;

#endif // #if defined(__DRV_UPMU_LDO_V1__)


#if defined(__DRV_UPMU_BUCK_V1__)
// BUCK S/W strcture definition
// TTTTTTT, TBD
typedef enum
{
	BUCK_ENABLE_WITH_SRCLKEN = 0,
	BUCK_ENABLE_LDO_EN_REGISTER = 1
}upmu_buck_on_sel_enum;

typedef enum
{
	BUCK_LOCAL_SENSE = 0,
	BUCK_REMOTE_SENSE = 1
}upmu_buck_rs_enum;

typedef enum
{
	BUCK_SOFT_START_DIRECT_SWITCH = 0,
	BUCK_SOFT_START
}upmu_buck_sf_str_mode_enum;

typedef enum
{
	BUCK_AUTO_MODE = 0,
	BUCK_FORCE_PWM_MODE
}upmu_buck_modeset_enum;

typedef enum
{
	BUCK_OC_TD_100_US = 0,
	BUCK_OC_TD_200_US = 1,
	BUCK_OC_TD_400_US = 2,
	BUCK_OC_TD_800_US = 3
}upmu_buck_oc_td_enum;

typedef enum
{
	BUCK_STB_TD_200_US = 0,
	BUCK_STB_TD_400_US = 1,
	BUCK_STB_TD_600_US = 2,
	BUCK_STB_TD_800_US = 3,

	BUCK_STB_TD_219_US = 0,
	BUCK_STB_TD_313_US = 0,
	BUCK_STB_TD_531_US = 1,
	BUCK_STB_TD_719_US = 2,
	BUCK_STB_TD_938_US = 3
}upmu_buck_stb_td_enum;

typedef enum
{
	BUCK_OC_THD_4_DIVIDED_BY_8 = 0,
	BUCK_OC_THD_3_DIVIDED_BY_8 = 1,
	BUCK_OC_THD_2_DIVIDED_BY_8 = 2,
	BUCK_OC_THD_1_DIVIDED_BY_8 = 3
}upmu_buck_oc_thd_enum;

typedef enum
{
	BUCK_OC_WND_4_US = 0,
	BUCK_OC_WND_8_US = 0,
	BUCK_OC_WND_16_US = 1,
	BUCK_OC_WND_32_US = 2,
	BUCK_OC_WND_64_US = 3
}upmu_buck_oc_wnd_enum;

typedef enum
{
	BUCK_ICAL_X_1_0 = 0,
	BUCK_ICAL_X_0_5 = 1,
	BUCK_ICAL_X_2_0 = 2,
	BUCK_ICAL_X_3_0 = 3
}upmu_buck_ical_en_enum;

typedef enum
{
	BUCK_ADJCKSEL_1_48_MHz = 0,
	BUCK_ADJCKSEL_1_77_MHz = 3,
	BUCK_ADJCKSEL_1_15_MHz = 4
}upmu_buck_adjcksel_enum;

typedef enum
{
	BUCK_SLEW_1_00_X = 0,
	BUCK_SLEW_0_75_X = 1,
	BUCK_SLEW_0_50_X = 2,
	BUCK_SLEW_0_25_X = 3
}upmu_buck_slew_enum;

typedef enum
{
	BUCK_SLEW_NMOS_1_00_X = 0,
	BUCK_SLEW_NMOS_0_75_X = 1,
	BUCK_SLEW_NMOS_0_50_X = 2,
	BUCK_SLEW_NMOS_0_25_X = 3
}upmu_buck_slew_nmos_enum;

#endif // #if defined(__DRV_UPMU_BUCK_V1__)

#if defined(__DRV_UPMU_BUCK_BOOST_V1__)

typedef enum
{
	BUCK_BOOST_AUTOMATIC_POWER_SAVING = 0,
	BUCK_BOOST_FORCE_PWM = 1
}upmu_buck_boost_fpwm_enum;

typedef enum
{
	BUCK_BOOST_NORMAL_MODE = 0,
	BUCK_BOOST_TEST_MODE = 1
}upmu_buck_boost_tm_enum;

typedef enum
{
	BUCK_BOOST_NORMAL = 0,
	BUCK_BOOST_1_DIVIDED_BY_2_SPEED = 1
}upmu_buck_boost_ss_spd_enum;

#endif // #if defined(__DRV_UPMU_BUCK_BOOST_V1__)

#if defined(__DRV_UPMU_BOOST_V1__)
typedef enum
{
	BOOST_TYPE_VOLTAGE_CONTROLLER = 0,
	BOOST_TYPE_CURRENT_CONVERTER
}upmu_boost_type_enum;

typedef enum
{
	BOOST_PWM_MODE = 0,
	BOOST_REGISTER_CTRL_MODE = 1
}upmu_boost_mode_enum;

typedef enum
{
	BOOST_OC_THD_4_DIVIDED_BY_8 = 0,
	BOOST_OC_THD_3_DIVIDED_BY_8 = 1,
	BOOST_OC_THD_2_DIVIDED_BY_8 = 2,
	BOOST_OC_THD_1_DIVIDED_BY_8 = 3
}upmu_boost_oc_thd_enum;

typedef enum
{
	BOOST_OC_WND_8_US = 0,
	BOOST_OC_WND_16_US = 1,
	BOOST_OC_WND_32_US = 2,
	BOOST_OC_WND_64_US = 3
}upmu_boost_oc_wnd_enum;

typedef enum
{
	BOOST_HW_SEL_ISINK = 0,
	BOOST_HW_SEL_BOOST
}upmu_boost_hw_sel_enum;


#endif // #if defined(__DRV_UPMU_BOOST_V1__)

#if defined(__DRV_UPMU_ISINK_V1__)
typedef enum
{
	ISINK_PWM_MODE = 0,
	ISINK_REGISTER_CTRL_MODE = 1
}upmu_isink_mode_enum;

#endif // #if defined(__DRV_UPMU_ISINK_V1__)

#if defined(__DRV_UPMU_KPLED_V1__)
typedef enum
{
	KPLED_PWM_MODE = 0,
	KPLED_REGISTER_CTRL_MODE = 1
}upmu_kpled_mode_enum;

typedef enum
{
	KPLED_SFSTRT_C_31US_X_1 = 0,
	KPLED_SFSTRT_C_31US_X_2 = 1,
	KPLED_SFSTRT_C_31US_X_3 = 2,
	KPLED_SFSTRT_C_31US_X_4 = 3
}upmu_kpled_sf_start_time_constane_enum;

#endif // #if defined(__DRV_UPMU_KPLED_V1__)

#if defined(__DRV_UPMU_SPK_V1__)
typedef enum
{
	SPK_OC_THD_4_DIVIDED_BY_8 = 0,
	SPK_OC_THD_3_DIVIDED_BY_8 = 1,
	SPK_OC_THD_2_DIVIDED_BY_8 = 2,
	SPK_OC_THD_1_DIVIDED_BY_8 = 3
}upmu_spk_oc_thd_enum;

typedef enum
{
	SPK_OC_WND_16_US = 0,
	SPK_OC_WND_32_US = 1,
	SPK_OC_WND_64_US = 2,
	SPK_OC_WND_128_US = 3
}upmu_spk_oc_wnd_enum;

typedef enum
{
	SPK_CLASS_D_MODE = 0,
	SPK_CLASS_AB_MODE
}upmu_spk_mode_enum;

#endif // #if defined(__DRV_UPMU_SPK_V1__)

//AddForMT6573
#if defined(__DRV_UPMU_BC11_V2__)

typedef enum
{
	BC11_DISABLE_VSRC = 0,
	BC11_ENABLE_VSRC_TO_DM = 1,
	BC11_ENABLE_VSRC_TO_DP = 2,
	BC11_FORBIDDEN_VSRC = 3
}upmu_bc11_vsrc_en_enum;

typedef enum
{
	BC11_DISABLE_CMP = 0,
	BC11_ENABLE_CMP_TO_DM = 1,
	BC11_ENABLE_CMP_TO_DP = 2,
	BC11_FORBIDDEN_CMP = 3
}upmu_bc11_cmp_en_enum;

typedef enum
{
	BC11_DISABLE_IPD = 0,
	BC11_ENABLE_IPD_TO_DM = 1,
	BC11_ENABLE_IPD_TO_DP = 2,
	BC11_FORBIDDEN_IPD = 3
}upmu_bc11_ipd_en_enum;

typedef enum
{
	BC11_DISABLE_IPU = 0,
	BC11_ENABLE_IPU_TO_DM = 1,
	BC11_ENABLE_IPU_TO_DP = 2,
	BC11_FORBIDDEN_IPU = 3
}upmu_bc11_ipu_en_enum;

typedef enum
{
	BC11_VREF_VTH_0_325 = 0,
	BC11_VREF_VTH_1_200 = 1
}upmu_bc11_vref_vth_enum;

#endif // #if defined(__DRV_UPMU_BC11_V2__)

//AddForMT6573
#if defined(__DRV_UPMU_STRUP_V1__)

typedef enum
{
	DEFAULT = 0,
	ADD_VREF_10K = 1,
	ADD_VREF_20K = 2,
	ADD_VREF_30K = 3,
	REDUCE_VREF_40K = 4,
	REDUCE_VREF_30K = 5,
	REDUCE_VREF_20K = 6,
	REDUCE_VREF_10K = 7	
}upmu_strup_bias_current_adjust_enum;

typedef enum
{
	INITIAL_SETTING = 0,
	ADD_10_C = 1,
	REDUCE_20_C = 2,
	REDUCE_10_C = 3
}upmu_strup_thermal_shutdown_threshold_fine_tune_enum;

#endif // #if defined(__DRV_UPMU_STRUP_V1__)

//AddForMT6573
#if defined(__DRV_UPMU_LPOSC_V1__)

typedef enum
{
	CALI_1_0_X = 0,
	CALI_0_9_X = 1,
	CALI_0_8_X = 2,
	CALI_0_7_X = 3,
	CALI_0_6_X = 4,
	CALI_0_5_X = 5,
	CALI_1_2_X = 6,
	CALI_1_5_X = 7	
}upmu_lposc_current_bias_calibration_enum;

typedef enum
{
	DIVIDER_RATIO_8 = 0,
	DIVIDER_RATIO_6 = 1,
	DIVIDER_RATIO_4 = 2,
	DIVIDER_RATIO_2 = 3
}upmu_lposc_buck_boost_output_freq_enum;

typedef enum
{
	FREQ_2_07_MHz = 0,
	FREQ_2_00_MHz = 1,
	FREQ_1_80_MHz = 2,
	FREQ_1_60_MHz = 3,
	FREQ_1_40_MHz = 4,
	FREQ_1_20_MHz = 5,
	FREQ_1_00_MHz = 6,
	FREQ_0_09_MHz = 7
}upmu_lposc_buck_output_freq_enum;

typedef enum
{
	DURATION_2 = 0,
	DURATION_4 = 1,
	DURATION_8 = 2,
	DURATION_12 = 3,
	DURATION_16 = 4,
	DURATION_20 = 5,
	DURATION_24 = 6,
	DURATION_28 = 7
}upmu_lposc_ssc_code_dur_enum;

typedef enum
{
	AMP_4_CODE = 0,
	AMP_8_CODE = 1,
	AMP_16_CODE = 2,
	AMP_24_CODE = 3,
	AMP_32_CODE = 4,
	AMP_40_CODE = 5,
	AMP_48_CODE = 6,
	AMP_56_CODE = 7
}upmu_lposc_ssc_mod_amp_enum;

typedef enum
{
	DURATION_64 = 0,
	DURATION_128 = 1,
	DURATION_4X128 = 2,
	DURATION_8X128 = 3
}upmu_lposc_fd_dis_dur2_enum;

typedef enum
{
	NO_DISABLE = 0,
	CLK_PERIOD_32768 = 1,
	CLK_PERIOD_8X32768 = 2,
	CLK_PERIOD_16X32768 = 3
}upmu_lposc_fd_dis_dur1_enum;

typedef enum
{
	PERCENT_0_0 = 0,	
	PERCENT_0_5 = 1,
	PERCENT_1_0 = 2,
	PERCENT_1_5 = 3,
	PERCENT_2_0 = 4,
	PERCENT_3_0 = 5,
	PERCENT_4_0 = 6,
	PERCENT_5_0 = 7
}upmu_lposc_fd_resolution_adjust_enum;

#endif // #if defined(__DRV_UPMU_LPOSC_V1__)

// Common S/W structure
typedef enum
{
	UPMU_VOLT_0_0_0_0_V = PMIC_ADPT_VOLT_0_0,
	UPMU_VOLT_0_1_0_0_V = PMIC_ADPT_VOLT_0_1,
	UPMU_VOLT_0_2_0_0_V = PMIC_ADPT_VOLT_0_2,
	UPMU_VOLT_0_3_0_0_V = PMIC_ADPT_VOLT_0_3,
	UPMU_VOLT_0_4_0_0_V = PMIC_ADPT_VOLT_0_4,
	UPMU_VOLT_0_5_0_0_V = PMIC_ADPT_VOLT_0_5,
	UPMU_VOLT_0_6_0_0_V = PMIC_ADPT_VOLT_0_6,
	UPMU_VOLT_0_7_0_0_V = PMIC_ADPT_VOLT_0_7,
	UPMU_VOLT_0_8_0_0_V = PMIC_ADPT_VOLT_0_8_0_0,
	UPMU_VOLT_0_8_2_5_V = PMIC_ADPT_VOLT_0_8_2_5,
	UPMU_VOLT_0_8_5_0_V = PMIC_ADPT_VOLT_0_8_5_0,
	UPMU_VOLT_0_8_7_5_V = PMIC_ADPT_VOLT_0_8_7_5,
	UPMU_VOLT_0_9_0_0_V = PMIC_ADPT_VOLT_0_9_0_0,
	UPMU_VOLT_0_9_2_5_V = PMIC_ADPT_VOLT_0_9_2_5,
	UPMU_VOLT_0_9_5_0_V = PMIC_ADPT_VOLT_0_9_5_0,
	UPMU_VOLT_0_9_7_5_V = PMIC_ADPT_VOLT_0_9_7_5,
	UPMU_VOLT_1_0_0_0_V = PMIC_ADPT_VOLT_1_0_0_0,
	UPMU_VOLT_1_0_2_5_V = PMIC_ADPT_VOLT_1_0_2_5,
	UPMU_VOLT_1_0_5_0_V = PMIC_ADPT_VOLT_1_0_5_0,
	UPMU_VOLT_1_0_7_5_V = PMIC_ADPT_VOLT_1_0_7_5,
	UPMU_VOLT_1_1_0_0_V = PMIC_ADPT_VOLT_1_1_0_0,
	UPMU_VOLT_1_1_2_5_V = PMIC_ADPT_VOLT_1_1_2_5,
	UPMU_VOLT_1_1_5_0_V = PMIC_ADPT_VOLT_1_1_5_0,
	UPMU_VOLT_1_1_7_5_V = PMIC_ADPT_VOLT_1_1_7_5,
	UPMU_VOLT_1_2_0_0_V = PMIC_ADPT_VOLT_1_2_0_0,
	UPMU_VOLT_1_2_2_5_V = PMIC_ADPT_VOLT_1_2_2_5,
	UPMU_VOLT_1_2_5_0_V = PMIC_ADPT_VOLT_1_2_5_0,
	UPMU_VOLT_1_2_7_5_V = PMIC_ADPT_VOLT_1_2_7_5,
	UPMU_VOLT_1_3_0_0_V = PMIC_ADPT_VOLT_1_3_0_0,
	UPMU_VOLT_1_3_2_5_V = PMIC_ADPT_VOLT_1_3_2_5,
	UPMU_VOLT_1_3_5_0_V = PMIC_ADPT_VOLT_1_3_5_0,
	UPMU_VOLT_1_3_7_5_V = PMIC_ADPT_VOLT_1_3_7_5,
	UPMU_VOLT_1_4_0_0_V = PMIC_ADPT_VOLT_1_4_0_0,
	UPMU_VOLT_1_4_2_5_V = PMIC_ADPT_VOLT_1_4_2_5,
	UPMU_VOLT_1_4_5_0_V = PMIC_ADPT_VOLT_1_4_5_0,
	UPMU_VOLT_1_4_7_5_V = PMIC_ADPT_VOLT_1_4_7_5,
	UPMU_VOLT_1_5_0_0_V = PMIC_ADPT_VOLT_1_5_0_0,
	UPMU_VOLT_1_5_2_5_V = PMIC_ADPT_VOLT_1_5_2_5,
	UPMU_VOLT_1_5_5_0_V = PMIC_ADPT_VOLT_1_5_5_0,
	UPMU_VOLT_1_5_7_5_V = PMIC_ADPT_VOLT_1_5_7_5,
	UPMU_VOLT_1_6_0_0_V = PMIC_ADPT_VOLT_1_6_0_0,
	UPMU_VOLT_1_6_2_5_V = PMIC_ADPT_VOLT_1_6_2_5,
	UPMU_VOLT_1_6_5_0_V = PMIC_ADPT_VOLT_1_6_5_0,
	UPMU_VOLT_1_6_7_5_V = PMIC_ADPT_VOLT_1_6_7_5,
	UPMU_VOLT_1_7_0_0_V = PMIC_ADPT_VOLT_1_7_0_0,
	UPMU_VOLT_1_7_2_5_V = PMIC_ADPT_VOLT_1_7_2_5,
	UPMU_VOLT_1_7_5_0_V = PMIC_ADPT_VOLT_1_7_5_0,
	UPMU_VOLT_1_7_7_5_V = PMIC_ADPT_VOLT_1_7_7_5,
	UPMU_VOLT_1_8_0_0_V = PMIC_ADPT_VOLT_1_8_0_0,
	UPMU_VOLT_1_8_2_5_V = PMIC_ADPT_VOLT_1_8_2_5,
	UPMU_VOLT_1_8_5_0_V = PMIC_ADPT_VOLT_1_8_5_0,
	UPMU_VOLT_1_8_7_5_V = PMIC_ADPT_VOLT_1_8_7_5,
	UPMU_VOLT_1_9_0_0_V = PMIC_ADPT_VOLT_1_9_0_0,
	UPMU_VOLT_1_9_2_5_V = PMIC_ADPT_VOLT_1_9_2_5,
	UPMU_VOLT_1_9_5_0_V = PMIC_ADPT_VOLT_1_9_5_0,
	UPMU_VOLT_1_9_7_5_V = PMIC_ADPT_VOLT_1_9_7_5,
	UPMU_VOLT_2_0_0_0_V = PMIC_ADPT_VOLT_2_0_0_0,
	UPMU_VOLT_2_0_2_5_V = PMIC_ADPT_VOLT_2_0_2_5,
	UPMU_VOLT_2_0_5_0_V = PMIC_ADPT_VOLT_2_0_5_0,
	UPMU_VOLT_2_0_7_5_V = PMIC_ADPT_VOLT_2_0_7_5,
	UPMU_VOLT_2_1_0_0_V = PMIC_ADPT_VOLT_2_1,
	UPMU_VOLT_2_2_0_0_V = PMIC_ADPT_VOLT_2_2,
	UPMU_VOLT_2_3_0_0_V = PMIC_ADPT_VOLT_2_3,
	UPMU_VOLT_2_4_0_0_V = PMIC_ADPT_VOLT_2_4,
	UPMU_VOLT_2_5_0_0_V = PMIC_ADPT_VOLT_2_5,
	UPMU_VOLT_2_6_0_0_V = PMIC_ADPT_VOLT_2_6,
	//UPMU_VOLT_2_7_0_0_V = PMIC_ADPT_VOLT_2_7,
	UPMU_VOLT_2_7_0_0_V = PMIC_ADPT_VOLT_2_7_0_0,
	UPMU_VOLT_2_7_2_5_V = PMIC_ADPT_VOLT_2_7_2_5,
	UPMU_VOLT_2_7_5_0_V = PMIC_ADPT_VOLT_2_7_5_0,
	UPMU_VOLT_2_7_7_5_V = PMIC_ADPT_VOLT_2_7_7_5,
	//UPMU_VOLT_2_8_0_0_V = PMIC_ADPT_VOLT_2_8,
	UPMU_VOLT_2_8_0_0_V = PMIC_ADPT_VOLT_2_8_0_0,
	UPMU_VOLT_2_8_2_5_V = PMIC_ADPT_VOLT_2_8_2_5,
	UPMU_VOLT_2_8_5_0_V = PMIC_ADPT_VOLT_2_8_5_0,
	UPMU_VOLT_2_8_7_5_V = PMIC_ADPT_VOLT_2_8_7_5,
	UPMU_VOLT_2_9_0_0_V = PMIC_ADPT_VOLT_2_9,
	UPMU_VOLT_3_0_0_0_V = PMIC_ADPT_VOLT_3_0,
	UPMU_VOLT_3_1_0_0_V = PMIC_ADPT_VOLT_3_1,
	UPMU_VOLT_3_2_0_0_V = PMIC_ADPT_VOLT_3_2,
	UPMU_VOLT_3_3_0_0_V = PMIC_ADPT_VOLT_3_3,
	UPMU_VOLT_3_4_0_0_V = PMIC_ADPT_VOLT_3_4,
	UPMU_VOLT_3_5_0_0_V = PMIC_ADPT_VOLT_3_5,
	UPMU_VOLT_3_6_0_0_V = PMIC_ADPT_VOLT_3_6,
	UPMU_VOLT_3_7_0_0_V = PMIC_ADPT_VOLT_3_7,
	UPMU_VOLT_3_8_0_0_V = PMIC_ADPT_VOLT_3_8,
	UPMU_VOLT_3_9_0_0_V = PMIC_ADPT_VOLT_3_9,
	UPMU_VOLT_4_0_0_0_V = PMIC_ADPT_VOLT_4_0,
	UPMU_VOLT_4_1_0_0_V = PMIC_ADPT_VOLT_4_1,
	UPMU_VOLT_4_2_0_0_V = PMIC_ADPT_VOLT_4_2,
	UPMU_VOLT_4_3_0_0_V = PMIC_ADPT_VOLT_4_3,
	UPMU_VOLT_4_4_0_0_V = PMIC_ADPT_VOLT_4_4,
	UPMU_VOLT_4_5_0_0_V = PMIC_ADPT_VOLT_4_5,
	UPMU_VOLT_4_6_0_0_V = PMIC_ADPT_VOLT_4_6,
	UPMU_VOLT_4_7_0_0_V = PMIC_ADPT_VOLT_4_7,
	UPMU_VOLT_4_8_0_0_V = PMIC_ADPT_VOLT_4_8,
	UPMU_VOLT_4_9_0_0_V = PMIC_ADPT_VOLT_4_9,
	UPMU_VOLT_5_0_0_0_V = PMIC_ADPT_VOLT_5_0,
	UPMU_VOLT_5_1_0_0_V = PMIC_ADPT_VOLT_5_1,
	UPMU_VOLT_5_2_0_0_V = PMIC_ADPT_VOLT_5_2,
	UPMU_VOLT_5_3_0_0_V = PMIC_ADPT_VOLT_5_3,
	UPMU_VOLT_5_4_0_0_V = PMIC_ADPT_VOLT_5_4,
	UPMU_VOLT_5_5_0_0_V = PMIC_ADPT_VOLT_5_5,
	UPMU_VOLT_5_6_0_0_V = PMIC_ADPT_VOLT_5_6,
	UPMU_VOLT_5_7_0_0_V = PMIC_ADPT_VOLT_5_7,
	UPMU_VOLT_5_8_0_0_V = PMIC_ADPT_VOLT_5_8,
	UPMU_VOLT_5_9_0_0_V = PMIC_ADPT_VOLT_5_9,
	UPMU_VOLT_6_0_0_0_V = PMIC_ADPT_VOLT_6_0,
	UPMU_VOLT_6_1_0_0_V = PMIC_ADPT_VOLT_6_1,
	UPMU_VOLT_6_2_0_0_V = PMIC_ADPT_VOLT_6_2,
	UPMU_VOLT_6_3_0_0_V = PMIC_ADPT_VOLT_6_3,
	UPMU_VOLT_6_4_0_0_V = PMIC_ADPT_VOLT_6_4,
	UPMU_VOLT_6_5_0_0_V = PMIC_ADPT_VOLT_6_5,
	UPMU_VOLT_6_6_0_0_V = PMIC_ADPT_VOLT_6_6,
	UPMU_VOLT_6_7_0_0_V = PMIC_ADPT_VOLT_6_7,
	UPMU_VOLT_6_8_0_0_V = PMIC_ADPT_VOLT_6_8,
	UPMU_VOLT_6_9_0_0_V = PMIC_ADPT_VOLT_6_9,
	UPMU_VOLT_7_0_0_0_V = PMIC_ADPT_VOLT_7_0,
	UPMU_VOLT_7_1_0_0_V = PMIC_ADPT_VOLT_7_1,
	UPMU_VOLT_7_2_0_0_V = PMIC_ADPT_VOLT_7_2,
	UPMU_VOLT_7_3_0_0_V = PMIC_ADPT_VOLT_7_3,
	UPMU_VOLT_7_4_0_0_V = PMIC_ADPT_VOLT_7_4,
	UPMU_VOLT_7_5_0_0_V = PMIC_ADPT_VOLT_7_5,
	UPMU_VOLT_7_6_0_0_V = PMIC_ADPT_VOLT_7_6,
	UPMU_VOLT_7_7_0_0_V = PMIC_ADPT_VOLT_7_7,
	UPMU_VOLT_7_8_0_0_V = PMIC_ADPT_VOLT_7_8,
	UPMU_VOLT_7_9_0_0_V = PMIC_ADPT_VOLT_7_9,
	UPMU_VOLT_8_0_0_0_V = PMIC_ADPT_VOLT_8_0,
	UPMU_VOLT_MAX = PMIC_ADPT_VOLT_MAX
}upmu_ldo_vol_enum, upmu_buck_vol_enum, upmu_buck_boost_vol_enum;

//#define upmu_ldo_vol_enum_new pmic_adpt_voltage_enum
//#define upmu_buck_vol_enum_new pmic_adpt_voltage_enum
#define upmu_chr_vol_enum pmic_adpt_voltage_enum
//#define upmu_spk_vol_enum pmic_adpt_spk_vol_enum
//#define upmu_chr_current_enum pmic_adpt_chr_current_enum

typedef enum
{
	UPMU_SPK_VOL_0_dB = PMIC_ADPT_SPK_VOL_0_dB,
	UPMU_SPK_VOL_1_dB = PMIC_ADPT_SPK_VOL_1_dB,
	UPMU_SPK_VOL_2_dB = PMIC_ADPT_SPK_VOL_2_dB,
	UPMU_SPK_VOL_3_dB = PMIC_ADPT_SPK_VOL_3_dB,
	UPMU_SPK_VOL_4_dB = PMIC_ADPT_SPK_VOL_4_dB,
	UPMU_SPK_VOL_5_dB = PMIC_ADPT_SPK_VOL_5_dB,
	UPMU_SPK_VOL_6_dB = PMIC_ADPT_SPK_VOL_6_dB,
	UPMU_SPK_VOL_7_dB = PMIC_ADPT_SPK_VOL_7_dB,
	UPMU_SPK_VOL_8_dB = PMIC_ADPT_SPK_VOL_8_dB,
	UPMU_SPK_VOL_9_dB = PMIC_ADPT_SPK_VOL_9_dB,
	UPMU_SPK_VOL_10_dB = PMIC_ADPT_SPK_VOL_10_dB,
	UPMU_SPK_VOL_11_dB = PMIC_ADPT_SPK_VOL_11_dB,
	UPMU_SPK_VOL_12_dB = PMIC_ADPT_SPK_VOL_12_dB,
	UPMU_SPK_VOL_13_dB = PMIC_ADPT_SPK_VOL_13_dB,
	UPMU_SPK_VOL_14_dB = PMIC_ADPT_SPK_VOL_14_dB,
	UPMU_SPK_VOL_15_dB = PMIC_ADPT_SPK_VOL_15_dB,
	UPMU_SPK_VOL_16_dB = PMIC_ADPT_SPK_VOL_16_dB,
	UPMU_SPK_VOL_17_dB = PMIC_ADPT_SPK_VOL_17_dB,
	UPMU_SPK_VOL_18_dB = PMIC_ADPT_SPK_VOL_18_dB,
	UPMU_SPK_VOL_19_dB = PMIC_ADPT_SPK_VOL_19_dB,
	UPMU_SPK_VOL_20_dB = PMIC_ADPT_SPK_VOL_20_dB,
	UPMU_SPK_VOL_21_dB = PMIC_ADPT_SPK_VOL_21_dB,
	UPMU_SPK_VOL_22_dB = PMIC_ADPT_SPK_VOL_22_dB,
	UPMU_SPK_VOL_23_dB = PMIC_ADPT_SPK_VOL_23_dB,
	UPMU_SPK_VOL_24_dB = PMIC_ADPT_SPK_VOL_24_dB,
	UPMU_SPK_VOL_MAX = PMIC_ADPT_SPK_VOL_MAX
}upmu_spk_vol_enum;

typedef enum
{
	UPMU_CHARGE_CURRENT_0_MA = PMIC_ADPT_CHARGE_CURRENT_0_MA,
	UPMU_CHARGE_CURRENT_50_MA = PMIC_ADPT_CHARGE_CURRENT_50_MA,
	UPMU_CHARGE_CURRENT_100_MA = PMIC_ADPT_CHARGE_CURRENT_100_MA,
	UPMU_CHARGE_CURRENT_150_MA = PMIC_ADPT_CHARGE_CURRENT_150_MA,
	UPMU_CHARGE_CURRENT_200_MA = PMIC_ADPT_CHARGE_CURRENT_200_MA,
	UPMU_CHARGE_CURRENT_250_MA = PMIC_ADPT_CHARGE_CURRENT_250_MA,
	UPMU_CHARGE_CURRENT_300_MA = PMIC_ADPT_CHARGE_CURRENT_300_MA,
	UPMU_CHARGE_CURRENT_350_MA = PMIC_ADPT_CHARGE_CURRENT_350_MA,
	UPMU_CHARGE_CURRENT_400_MA = PMIC_ADPT_CHARGE_CURRENT_400_MA,
	UPMU_CHARGE_CURRENT_450_MA = PMIC_ADPT_CHARGE_CURRENT_450_MA,
	UPMU_CHARGE_CURRENT_500_MA = PMIC_ADPT_CHARGE_CURRENT_500_MA,
	UPMU_CHARGE_CURRENT_550_MA = PMIC_ADPT_CHARGE_CURRENT_550_MA,
	UPMU_CHARGE_CURRENT_600_MA = PMIC_ADPT_CHARGE_CURRENT_600_MA,
	UPMU_CHARGE_CURRENT_650_MA = PMIC_ADPT_CHARGE_CURRENT_650_MA,
	UPMU_CHARGE_CURRENT_700_MA = PMIC_ADPT_CHARGE_CURRENT_700_MA,
	UPMU_CHARGE_CURRENT_750_MA = PMIC_ADPT_CHARGE_CURRENT_750_MA,
	UPMU_CHARGE_CURRENT_800_MA = PMIC_ADPT_CHARGE_CURRENT_800_MA,
	UPMU_CHARGE_CURRENT_850_MA = PMIC_ADPT_CHARGE_CURRENT_850_MA,
	UPMU_CHARGE_CURRENT_900_MA = PMIC_ADPT_CHARGE_CURRENT_900_MA,
	UPMU_CHARGE_CURRENT_950_MA = PMIC_ADPT_CHARGE_CURRENT_950_MA,
	UPMU_CHARGE_CURRENT_1000_MA = PMIC_ADPT_CHARGE_CURRENT_1000_MA,
	UPMU_CHARGE_CURRENT_1200_MA = PMIC_ADPT_CHARGE_CURRENT_1200_MA,
	UPMU_CHARGE_CURRENT_1500_MA = PMIC_ADPT_CHARGE_CURRENT_1500_MA,
	UPMU_CHARGE_CURRENT_1800_MA = PMIC_ADPT_CHARGE_CURRENT_1800_MA,
	UPMU_CHARGE_CURRENT_2000_MA = PMIC_ADPT_CHARGE_CURRENT_2000_MA,
	UPMU_CHARGE_CURRENT_MAX = PMIC_ADPT_CHARGE_CURRENT_MAX
}upmu_chr_current_enum;

#define UPMU_MAX_LDO_VOL_SEL_NUM   16
typedef struct
{
	kal_uint32 addr;
	kal_uint32 vol_list_num;  // 1: Means the voltage is fixed, not allow to configure
	upmu_ldo_vol_enum vol_list[UPMU_MAX_LDO_VOL_SEL_NUM];
}upmu_ldo_profile_entry;

#define UPMU_MAX_BUCK_VOL_SEL_NUM   32
typedef struct
{
	kal_uint32 addr;
	kal_uint32 vol_list_num;  // 1: Means the voltage is fixed, not allow to configure
	upmu_buck_vol_enum vol_list[UPMU_MAX_BUCK_VOL_SEL_NUM];
}upmu_buck_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_boost_profile_entry;

#define UPMU_MAX_BUCK_BOOST_VOL_SEL_NUM   32
typedef struct
{
	kal_uint32 addr;
	kal_uint32 vol_list_num;  // 1: Means the voltage is fixed, not allow to configure
	upmu_buck_vol_enum vol_list[UPMU_MAX_BUCK_BOOST_VOL_SEL_NUM];
}upmu_buck_boost_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_isink_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_kpled_profile_entry;

#define UPMU_MAX_SPK_VOL_NUM    16
typedef struct
{
	kal_uint32 addr;
	kal_uint32 spk_vol_list_num;
	upmu_spk_vol_enum spk_vol_list[UPMU_MAX_SPK_VOL_NUM];

}upmu_spk_profile_entry;

#define UPMU_MAX_CHARGE_CURRENT_NUM    16
#define UPMU_MAX_CHARGER_DET_LV_NUM    32
#define UPMU_MAX_CHARGER_DET_HV_NUM    32
#define UPMU_MAX_VBAT_CC_DET_NUM       16
#define UPMU_MAX_VBAT_CV_DET_NUM       32
#define UPMU_MAX_VBAT_OV_NUM           8
typedef struct
{
	kal_uint32 addr;
	kal_uint32 chr_current_list_num;
	upmu_chr_current_enum chr_current_list[UPMU_MAX_CHARGE_CURRENT_NUM];
	kal_uint32 chr_det_lv_list_num;
	upmu_chr_vol_enum chr_det_lv_list[UPMU_MAX_CHARGER_DET_HV_NUM];
	kal_uint32 chr_det_hv_list_num;
	upmu_chr_vol_enum chr_det_hv_list[UPMU_MAX_CHARGER_DET_HV_NUM];
	kal_uint32 chr_vbat_cc_list_num;
	upmu_chr_vol_enum chr_vbat_cc_list[UPMU_MAX_VBAT_CC_DET_NUM];
	kal_uint32 chr_vbat_cv_list_num;
	upmu_chr_vol_enum chr_vbat_cv_list[UPMU_MAX_VBAT_CV_DET_NUM];
	kal_uint32 chr_vbat_ov_list_num;
	upmu_chr_vol_enum chr_vbat_ov_list[UPMU_MAX_VBAT_OV_NUM];
}upmu_chr_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_oc_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_strup_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_lposc_profile_entry;

typedef struct
{
	kal_uint32 addr;
}upmu_retention_profile_entry;

#endif // #ifndef __UPMU_SW_H__



