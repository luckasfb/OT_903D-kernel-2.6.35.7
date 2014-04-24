

#ifndef __UPMU_COMMON_SW_H__
#define __UPMU_COMMON_SW_H__


// ==> Must be included after pwic.h


#if defined(__DRV_UPMU_LDO_V1__)
// LDO CON0
extern void upmu_ldo_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_on_sel(upmu_ldo_list_enum ldo, upmu_ldo_on_sel_enum sel);
extern void upmu_ldo_vol_sel(upmu_ldo_list_enum ldo, upmu_ldo_vol_enum vol);
extern void upmu_ldo_ndis_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_stb_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_oc_auto_off(upmu_ldo_list_enum ldo, kal_bool auto_off);
extern void upmu_ldo_ocfb_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern kal_bool upmu_ldo_oc_status(upmu_ldo_list_enum ldo);
extern kal_bool upmu_ldo_status(upmu_ldo_list_enum ldo);
extern kal_bool upmu_ldo_get_oc_status(upmu_ldo_list_enum ldo);
extern kal_bool upmu_ldo_get_status(upmu_ldo_list_enum ldo);

//AddForMT6573
extern void upmu_ldo_psclk_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_stb_force_enable(upmu_ldo_list_enum ldo, kal_bool enable);
//AddForMT6573 PMUA_CON0
extern void upmu_ldo_va28_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_va25_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_va12_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_vmic_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_pmt_passgate_ovrd_enable(upmu_ldo_list_enum ldo, upmu_pmt_passgate_ovrd_enum sel);

// LDO CON1
#if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON1_BIT0__)
extern void upmu_ldo_stb_td(upmu_ldo_list_enum ldo, upmu_ldo_stb_td_enum sel);
#endif // #if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON1_BIT0__)
extern void upmu_ldo_cal(upmu_ldo_list_enum ldo, kal_uint8 val);

//AddForMT6573
extern void upmu_ldo_force_low_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_mode_enable(upmu_ldo_list_enum ldo, kal_bool enable_low_power_mode);
extern void upmu_ldo_mode_select(upmu_ldo_list_enum ldo, upmu_ldo_mode_sel_enum sel);
extern void upmu_ldo_srclk_enable(upmu_ldo_list_enum ldo, kal_bool enable);
//AddForMT6573 PMUA_CON1
extern void upmu_ldo_vaudp_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_vtv_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable);

extern void upmu_thr_hwpdn_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_usbdl_enable(upmu_ldo_list_enum ldo, kal_bool enable);

// LDO CON2
extern void upmu_ldo_oc_td(upmu_ldo_list_enum ldo, upmu_ldo_oc_td_enum sel);
#if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON2_BIT6__)
extern void upmu_ldo_stb_td(upmu_ldo_list_enum ldo, upmu_ldo_stb_td_enum sel);
#endif // #if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON2_BIT6__)
extern void upmu_ldo_ical(upmu_ldo_list_enum ldo, kal_uint8 val);

//AddForMT6573
extern void upmu_ldo_en_force_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_gpldo_enable(upmu_ldo_list_enum ldo, upmu_ldo_gpldo_enum sel);
//AddForMT6573 PMUA_CON2
extern void upmu_ldo_lnibias_trimctrl(upmu_ldo_list_enum ldo, kal_uint8 val);
extern void upmu_ldo_abist_s2d_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_s2d2_bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_lnibias_trim(upmu_ldo_list_enum ldo, kal_uint8 val);
extern void upmu_ldo_abist_txbias2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_txvga2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_apc2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_afc2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_lnibias_enable(upmu_ldo_list_enum ldo, kal_bool enable);

// LDO_XXX CON3
//AddForMT6573
extern void upmu_ldo_simx_mode(upmu_ldo_list_enum ldo, upmu_ldo_vsimx_mode_enum sel);
extern void upmu_ldo_simx_srn(upmu_ldo_list_enum ldo, kal_uint8 val);
extern void upmu_ldo_simx_srp(upmu_ldo_list_enum ldo, kal_uint8 val);
extern void upmu_ldo_simx_bias(upmu_ldo_list_enum ldo, kal_uint8 val);
extern void upmu_ldo_simxio_drv(upmu_ldo_list_enum ldo, kal_uint8 val);
extern kal_uint8 upmu_ldo_get_gpio_srst2(upmu_ldo_list_enum ldo);
extern kal_uint8 upmu_ldo_get_gpio_sclk2(upmu_ldo_list_enum ldo);
extern kal_uint8 upmu_ldo_get_gpio_sio2(upmu_ldo_list_enum ldo);
//AddForMT6573 PMUA_CON3
extern void upmu_ldo_abist_apc2accdet_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_afc2aux_ym_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_afc2aux_yp_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_afc2aux_xp_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_aud2auxadc_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_tvdac2auxadc_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_abist_afc2fgadc_enable(upmu_ldo_list_enum ldo, kal_bool enable);

#endif //#if defined(__DRV_UPMU_LDO_V1__)

#if defined(__DRV_UPMU_BUCK_V1__)
// BUCK CON0
extern void upmu_buck_enable(upmu_buck_list_enum buck, kal_bool enable);
extern void upmu_buck_rs(upmu_buck_list_enum buck, upmu_buck_rs_enum sel);
extern void upmu_buck_sf_str_mode(upmu_buck_list_enum buck, upmu_buck_sf_str_mode_enum sel);
extern void upmu_buck_normal_voltage_adjust(upmu_buck_list_enum buck, upmu_buck_vol_enum vol);
extern void upmu_buck_disable_anti_undershoot(upmu_buck_list_enum buck, kal_bool disable);
extern void upmu_buck_oc_auto_off(upmu_buck_list_enum buck, kal_bool auto_off);
extern void upmu_buck_ocfb_enable(upmu_buck_list_enum buck, kal_bool enable);
//AddForMT6573
extern void upmu_buck_stb_enable(upmu_buck_list_enum buck, kal_bool enable);
extern kal_bool upmu_buck_get_oc_status(upmu_buck_list_enum buck);
extern kal_bool upmu_buck_get_status(upmu_buck_list_enum buck);

// BUCK CON1
extern void upmu_buck_modeset(upmu_buck_list_enum buck, upmu_buck_modeset_enum sel);
extern void upmu_buck_sleep_voltage_adjust(upmu_buck_list_enum buck, upmu_buck_vol_enum vol);
//AddForMT6573
extern void upmu_buck_cpmcksel(upmu_buck_list_enum buck, kal_uint8 val);
//AddForMT6573
extern kal_bool upmu_is_buck_mode(upmu_buck_list_enum buck);

// BUCK CON2
extern void upmu_buck_cal(upmu_buck_list_enum buck, kal_uint8 val);
//AddForMT6573
extern void upmu_buck_vosel(upmu_buck_list_enum buck, kal_uint8 val); // reserved

// BUCK CON3
extern void upmu_buck_oc_td(upmu_buck_list_enum buck, upmu_buck_oc_td_enum val);
extern void upmu_buck_stb_td(upmu_buck_list_enum buck, upmu_buck_stb_td_enum val);
extern void upmu_buck_oc_thd(upmu_buck_list_enum buck, upmu_buck_oc_thd_enum val);
extern void upmu_buck_oc_wnd(upmu_buck_list_enum buck, upmu_buck_oc_wnd_enum val);
extern void upmu_buck_ical(upmu_buck_list_enum buck, upmu_buck_ical_en_enum val);
//AddForMT6573
extern void upmu_buck_en_force(upmu_buck_list_enum buck, kal_bool enable);

// BUCK CON4
//AddForMT6573
extern void upmu_buck_slew_nmos(upmu_buck_list_enum buck, upmu_buck_slew_nmos_enum val);
extern void upmu_buck_slew_mos(upmu_buck_list_enum buck, upmu_buck_slew_enum val);
extern void upmu_buck_adjcksel(upmu_buck_list_enum buck, upmu_buck_adjcksel_enum val);

// BUCK CON5
extern void upmu_buck_csl(upmu_buck_list_enum buck, kal_uint8 val);
extern void upmu_buck_burst(upmu_buck_list_enum buck, kal_uint8 val);
//AddForMT6573
extern void upmu_buck_csr(upmu_buck_list_enum buck, kal_uint8 val);
extern void upmu_buck_rzsel(upmu_buck_list_enum buck, kal_uint8 val);
extern void upmu_buck_gmsel(upmu_buck_list_enum buck, kal_uint8 val);
extern void upmu_buck_zx_pdn(upmu_buck_list_enum buck, kal_uint8 val);

#endif // #if defined(__DRV_UPMU_BUCK_V1__)

#if defined(__DRV_UPMU_BUCK_BOOST_V1__)
// BUCK_BOOST CON0
extern void upmu_buck_boost_cc(upmu_buck_boost_list_enum buck_boost, kal_uint8 val);
extern void upmu_buck_boost_rc(upmu_buck_boost_list_enum buck_boost, kal_uint8 val);
extern void upmu_buck_boost_sr2(upmu_buck_boost_list_enum buck_boost, kal_uint8 val);
extern void upmu_buck_boost_sr1(upmu_buck_boost_list_enum buck_boost, kal_uint8 val);
extern void upmu_buck_boost_cs_adj(upmu_buck_boost_list_enum buck_boost, kal_uint8 val);
extern void upmu_buck_boost_anti_ring(upmu_buck_boost_list_enum buck_boost, kal_bool enable);
extern void upmu_buck_boost_enable(upmu_buck_boost_list_enum buck_boost, kal_bool enable);
 
// BUCK_BOOST CON1
extern void upmu_buck_boost_fpwm(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_fpwm_enum val);
extern void upmu_buck_boost_tm(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_tm_enum val);
extern void upmu_buck_boost_ss_spd(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_ss_spd_enum val);
extern kal_bool upmu_buck_boost_get_status(upmu_buck_boost_list_enum buck_boost);

// BUCK_BOOST CON2
extern void upmu_buck_boost_voltage_tune_0(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);
extern void upmu_buck_boost_voltage_tune_1(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);

// BUCK_BOOST CON3
extern void upmu_buck_boost_voltage_tune_2(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);
extern void upmu_buck_boost_voltage_tune_3(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);

// BUCK_BOOST CON4
extern void upmu_buck_boost_voltage_tune_4(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);
extern void upmu_buck_boost_voltage_tune_5(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);

// BUCK_BOOST CON5
extern void upmu_buck_boost_voltage_tune_6(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);
extern void upmu_buck_boost_voltage_tune_7(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol);

#endif // #if defined(__DRV_UPMU_BUCK_BOOST_V1__)

#if defined(__DRV_UPMU_BOOST_V1__)
// BOOST CON0
extern void upmu_boost_enable(upmu_boost_list_enum boost, kal_bool enable);
extern void upmu_boost_type(upmu_boost_list_enum boost, upmu_boost_type_enum val);
extern void upmu_boost_mode(upmu_boost_list_enum boost, upmu_boost_mode_enum val);
extern void upmu_boost_vrsel(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_oc_auto_off(upmu_boost_list_enum boost, kal_bool auto_off);
extern kal_bool upmu_boost_oc_status(upmu_boost_list_enum boost);
// BOOST CON1
extern void upmu_boost_cl(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_cs(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_rc(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_ss(upmu_boost_list_enum boost, kal_uint8 val);
// BOOST CON2
extern void upmu_boost_cksel(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_sr_pmos(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_sr_nmos(upmu_boost_list_enum boost, kal_uint8 val);
extern void upmu_boost_slp(upmu_boost_list_enum boost, kal_uint8 val);
// BOOST CON3
extern void upmu_boost_cks_prg(upmu_boost_list_enum boost, kal_uint8 val);
// BOOST CON4
extern void upmu_boost_oc_thd(upmu_boost_list_enum boost, upmu_boost_oc_thd_enum val);
extern void upmu_boost_oc_wnd(upmu_boost_list_enum boost, upmu_boost_oc_wnd_enum val);
extern void upmu_boost_clk_cal(upmu_boost_list_enum boost, kal_uint8 val);
// BOOST CON6
extern void upmu_boost_hw_sel(upmu_boost_list_enum boost, upmu_boost_hw_sel_enum val);
extern void upmu_boost_cc(upmu_boost_list_enum boost, kal_uint8 val);
#endif // #if defined(__DRV_UPMU_BOOST_V1__)

#if defined(__DRV_UPMU_ISINK_V1__)
// ISINK CON0
extern void upmu_isink_enable(upmu_isink_list_enum isink, kal_bool enable);
extern void upmu_isink_mode(upmu_isink_list_enum isink, upmu_isink_mode_enum val);
extern void upmu_isink_step(upmu_isink_list_enum isink, kal_uint8 val);
extern kal_bool upmu_isink_status(upmu_isink_list_enum isink);
// ISINK CON1
extern void upmu_isinks_vref_cal(upmu_isink_list_enum isink, kal_uint8 val);
#endif // #if defined(__DRV_UPMU_ISINK_V1__)

#if defined(__DRV_UPMU_KPLED_V1__)
// KPLED CON0
extern void upmu_kpled_enable(upmu_kpled_list_enum kpled, kal_bool enable);
extern void upmu_kpled_mode(upmu_kpled_list_enum kpled, upmu_kpled_mode_enum val);
extern void upmu_kpled_sel(upmu_kpled_list_enum kpled, kal_uint8 val);
extern void upmu_kpled_sfstrt_c(upmu_kpled_list_enum kpled, upmu_kpled_sf_start_time_constane_enum val);
extern kal_bool upmu_kpled_status(upmu_kpled_list_enum kpled);
extern kal_bool upmu_kpled_get_status(upmu_kpled_list_enum kpled);
//AddForMT6573
extern void upmu_kpled_thermal_shutdown_enable(upmu_kpled_list_enum kpled, kal_bool enable);
extern void upmu_kpled_sfstrt_en(upmu_kpled_list_enum kpled, kal_bool enable);

// KPLED CON1
extern void upmu_kpled_force_off_enable(upmu_kpled_list_enum kpled, kal_bool enable);
extern void upmu_kpled_test_mode_enable(upmu_kpled_list_enum kpled, kal_bool enable);

#endif // #if defined(__DRV_UPMU_KPLED_V1__)

#if defined(__DRV_UPMU_SPK_V1__)
// SPK CON0
extern void upmu_spk_enable(upmu_spk_list_enum spk, kal_bool enable);
extern void upmu_spk_reset(upmu_spk_list_enum spk, kal_bool reset);
extern void upmu_spk_vol(upmu_spk_list_enum spk, upmu_spk_vol_enum vol);
extern upmu_spk_vol_enum upmu_spk_get_vol(upmu_spk_list_enum spk);
extern void upmu_spk_oc_auto_off(upmu_spk_list_enum spk, kal_bool auto_off);
extern void upmu_spk_ocfb_enable(upmu_spk_list_enum spk, kal_bool enable);
extern kal_bool upmu_spk_oc_status(upmu_spk_list_enum spk);
// SPK CON1
extern void upmu_spk_pfd_mode_enable(upmu_spk_list_enum spk, kal_bool enable);
extern void upmu_spk_cmode(upmu_spk_list_enum spk, kal_uint8 val);
extern void upmu_spk_ccode(upmu_spk_list_enum spk, kal_uint8 val);
// SPK CON2
extern void upmu_spk_oc_thd(upmu_spk_list_enum spk, upmu_spk_oc_thd_enum val);
extern void upmu_spk_oc_wnd(upmu_spk_list_enum spk, upmu_spk_oc_wnd_enum val);
// SPK CON3
extern void upmu_spk_osc_isel(upmu_spk_list_enum spk, kal_uint8 val);
extern void upmu_spk_oc_enable(upmu_spk_list_enum spk, kal_bool enable);
// SPK CON7
extern void upmu_spk_ab_obias(upmu_spk_list_enum spk, kal_uint8 val);
extern void upmu_spk_ab_oc_enable(upmu_spk_list_enum spk, kal_bool enable);
extern void upmu_spk_mode(upmu_spk_list_enum spk, upmu_spk_mode_enum val);
#endif // #if defined(__DRV_UPMU_SPK_V1__)


#if defined(__DRV_UPMU_CHARGER_V1__)
// CHR_CON0
//extern void upmu_vcdt_lv_vth(upmu_chr_list_enum chr, kal_uint8 val); // remove
//extern void upmu_vcdt_hv_vth(upmu_chr_list_enum chr, kal_uint8 val); // remove
extern void upmu_vcdt_lv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);
extern void upmu_vcdt_hv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);

extern void upmu_vcdt_hv_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_pchr_auto_enable(upmu_chr_list_enum chr, kal_bool auto_enable);
extern void upmu_csdac_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_chr_enable(upmu_chr_list_enum chr, kal_bool enable);

extern kal_bool upmu_chr_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_vcdt_lv_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_vcdt_hv_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_is_chr_det(upmu_chr_list_enum chr);
extern kal_bool upmu_is_vcdt_lv_det(upmu_chr_list_enum chr);
extern kal_bool upmu_is_vcdt_hv_det(upmu_chr_list_enum chr);

//AddForMT6573
extern kal_bool upmu_is_chr_ldo_det(upmu_chr_list_enum chr);

// CHR_CON1
//extern void upmu_vbat_cv_vth(upmu_chr_list_enum chr, kal_uint8 val); // remove
//extern void upmu_vbat_cc_vth(upmu_chr_list_enum chr, kal_uint8 val); // remove
extern void upmu_vbat_cv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);
extern void upmu_vbat_cc_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);

extern void upmu_vbat_cv_det_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_vbat_cc_det_enable(upmu_chr_list_enum chr, kal_bool enable);

extern kal_bool upmu_vbat_cv_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_vbat_cc_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_is_vbat_cv_det(upmu_chr_list_enum chr);
extern kal_bool upmu_is_vbat_cc_det(upmu_chr_list_enum chr);

// CHR_CON2
extern void upmu_pchr_tohtc(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_pchr_toltc(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_cs_vth(upmu_chr_list_enum chr, upmu_chr_current_enum current);
// General API name, set charge current
extern void upmu_chr_current(upmu_chr_list_enum chr, upmu_chr_current_enum current);

extern upmu_chr_current_enum upmu_get_cs_vth(upmu_chr_list_enum chr); // remove
extern upmu_chr_current_enum upmu_get_chr_cs_vth(upmu_chr_list_enum chr);

// General API name, get charge current
extern upmu_chr_current_enum upmu_get_chr_current(upmu_chr_list_enum chr);

// Get the register index mapped to specific charge current (Passed-in parameter)
extern kal_uint32 upmu_get_cs_vth_idx(upmu_chr_list_enum chr); // remove
extern kal_uint32 upmu_get_chr_cs_vth_idx(upmu_chr_list_enum chr);

// Get the register index mapped to specific charge current (Passed-in parameter)
extern kal_uint32 upmu_get_chr_current_idx(upmu_chr_list_enum chr);
extern void upmu_cs_enable(upmu_chr_list_enum chr, kal_bool enable);
extern upmu_chr_current_enum upmu_get_custom_charge_current(pmic_adpt_chr_type chr_type);

#if defined(__DRV_OTG_BVALID_DET_AT_CON2_BIT14__)
extern kal_bool upmu_otg_bvalid_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_is_otg_bvalid_det(upmu_chr_list_enum chr);
#endif //#if defined(__DRV_OTG_BVALID_DET_AT_CON2_BIT14__)

extern kal_bool upmu_cs_det(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_is_chr_cs_det(upmu_chr_list_enum chr);

// CHR_CON3
extern void upmu_csdac_stp(upmu_chr_list_enum chr, kal_uint8 val);
#if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT3__)
extern void upmu_vbat_ov_enable(upmu_chr_list_enum chr, kal_bool enable);
#endif // #if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT3__)
extern void upmu_csdac_dly(upmu_chr_list_enum chr, kal_uint8 val);

//extern void upmu_ov_vth(upmu_chr_list_enum chr, kal_uint8 val); // remove
extern void upmu_vbat_ov_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);

#if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT8__)
extern void upmu_vbat_ov_enable(upmu_chr_list_enum chr, kal_bool enable);
#endif // #if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT8__)
#if defined(__DRV_BATON_EN_AT_CON3_BIT9__)
extern void upmu_bat_on_det_enable(upmu_chr_list_enum chr, kal_bool enable);
#endif // #if defined(__DRV_BATON_EN_AT_CON3_BIT9__)
#if defined(__DRV_BATON_EN_AT_CON3_BIT12__)
extern void upmu_bat_on_det_enable(upmu_chr_list_enum chr, kal_bool enable);
#endif // #if defined(__DRV_BATON_EN_AT_CON3_BIT12__)
#if defined(__DRV_OTG_BVALID_EN_AT_CON3_BIT13__)
extern void upmu_otg_bvalid_det_enable(upmu_chr_list_enum chr, kal_bool enable);
#endif // #if defined(__DRV_OTG_BVALID_EN_AT_CON3_BIT13__)

extern kal_bool upmu_vbat_ov(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_bat_on(upmu_chr_list_enum chr); // remove
extern kal_bool upmu_is_vbat_ov(upmu_chr_list_enum chr);
extern kal_bool upmu_is_bat_on(upmu_chr_list_enum chr);

//AddForMT6573
extern void upmu_baton_ht_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_vbat_ov_deg_enable(upmu_chr_list_enum chr, kal_bool enable);

// CHR_CON4
extern void upmu_pchr_test_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_pchr_csdac_test_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_pchr_test_reset(upmu_chr_list_enum chr, kal_bool reset);
extern void upmu_pchr_test_csdac_dat(upmu_chr_list_enum chr, kal_uint8 val);

// CHR_CON5
extern void upmu_pchr_flag_sel(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_pchr_flag_enable(upmu_chr_list_enum chr, kal_uint8 enable);

extern kal_uint8 upmu_pchr_flag_out(upmu_chr_list_enum chr); // remove
extern kal_uint8 upmu_pchr_get_flag_out(upmu_chr_list_enum chr);

#if defined(__DRV_OTG_BVALID_EN_AT_CON5_BIT12__)
extern void upmu_otg_bvalid_det_enable(upmu_chr_list_enum chr, kal_bool enable);
#endif // #if defined(__DRV_OTG_BVALID_EN_AT_CON5_BIT12__)

#if defined(__DRV_OTG_BVALID_DET_AT_CON5_BIT15__)
extern kal_bool upmu_otg_bvalid_det(upmu_chr_list_enum chr);
#endif // #if defined(__DRV_OTG_BVALID_DET_AT_CON5_BIT15__)

//AddForMT6573
extern void upmu_pchr_ft_ctrl(upmu_chr_list_enum chr, kal_uint8 val);

// CHR_CON6
extern void upmu_chrwdt_td(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_chrwdt_enable(upmu_chr_list_enum chr, kal_uint8 enable);
extern void upmu_chrwdt_clear(upmu_chr_list_enum chr);

// CHR_CON7
extern void upmu_chrwdt_int_enable(upmu_chr_list_enum chr, kal_uint8 enable);
extern void upmu_chrwdt_flag(upmu_chr_list_enum chr, kal_uint8 val);
extern kal_bool upmu_chrwdt_timeout(upmu_chr_list_enum chr); // remove

// CHR_CON8
extern void upmu_adc_measure_enable(upmu_chr_list_enum chr, kal_bool enable); // remove

//AddForMT6573
extern void upmu_bgr_rsel(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_bgr_unchop_ph(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_bgr_unchop(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_uvlo_vthl(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_usbdl_rst(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_usbdl_set(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_adc_measure_vbat_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_adc_measure_vsen_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_adc_measure_vchr_enable(upmu_chr_list_enum chr, kal_bool enable);

//AddForMT6573
#if defined(__DRV_UPMU_BC11_V2__)
// CHR_CON9 (CHR_BC11_CON0)
extern void upmu_bc11_vsrc_en_source(upmu_chr_list_enum chr, upmu_bc11_vsrc_en_enum val);
extern void upmu_bc11_reset_circuit(upmu_chr_list_enum chr, kal_uint8 val);

// CHR_CON10 (CHR_BC11_CON1)
extern void upmu_bc11_cmp_enable(upmu_chr_list_enum chr, upmu_bc11_cmp_en_enum val);
extern void upmu_bc11_ipd_enable(upmu_chr_list_enum chr, upmu_bc11_ipd_en_enum val);
extern void upmu_bc11_ipu_enable(upmu_chr_list_enum chr, upmu_bc11_ipu_en_enum val);
extern void upmu_bc11_vref_vth(upmu_chr_list_enum chr, upmu_bc11_vref_vth_enum val);
extern void upmu_bc11_bias_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_bc11_bb_crtl_enable(upmu_chr_list_enum chr, kal_bool enable);
extern kal_uint8 upmu_get_bc11_cmp_out(upmu_chr_list_enum chr);

#endif // #if defined(__DRV_UPMU_BC11_V2__)

#endif // #if defined(__DRV_UPMU_CHARGER_V1__


#if defined(__DRV_UPMU_OC_V1__)
// PMIC_OC_CON0
extern void upmu_vrf_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vcama_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vcamd_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vio_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vusb_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vsim_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vsim2_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vibr_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vmc_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vcama2_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vcamd2_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);

// PMIC_OC_CON1
extern void upmu_vm12_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vm12_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);

// PMIC_OC_CON2
extern void upmu_vcore_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vio1v8_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vaproc_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);
extern void upmu_vrf1v8_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable);

// PMIC_OC_CON4
extern kal_uint8 upmu_get_vrf_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vcama_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vcamd_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vio_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vusb_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vsim_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_sim2_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vibr_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vmc_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vcama2_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vcamd2_oc_flag(upmu_oc_list_enum oc);

extern void upmu_clear_vrf_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vcama_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vcamd_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vio_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vusb_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vsim_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_sim2_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vibr_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vmc_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vcama2_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vcamd2_oc_flag(upmu_oc_list_enum oc);

// PMIC_OC_CON5
extern kal_uint8 upmu_get_vm12_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vm12_int_oc_flag(upmu_oc_list_enum oc);

extern void upmu_clear_vm12_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vm12_int_oc_flag(upmu_oc_list_enum oc);

// PMIC_OC_CON6
extern kal_uint8 upmu_get_vcore_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vio1v8_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vaproc_oc_flag(upmu_oc_list_enum oc);
extern kal_uint8 upmu_get_vrf1v8_oc_flag(upmu_oc_list_enum oc);

extern void upmu_clear_vcore_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vio1v8_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vaproc_oc_flag(upmu_oc_list_enum oc);
extern void upmu_clear_vrf1v8_oc_flag(upmu_oc_list_enum oc);

#endif // #if defined(__DRV_UPMU_OC_V1__)


#if defined(__DRV_UPMU_STRUP_V1__)
// STRUP_CON0
extern void upmu_strup_iref_adj(upmu_strup_list_enum strup, upmu_strup_bias_current_adjust_enum val);
extern void upmu_strup_thr_sel(upmu_strup_list_enum strup, upmu_strup_thermal_shutdown_threshold_fine_tune_enum val);
extern kal_uint8 upmu_get_strup_test_mode_status(upmu_strup_list_enum strup);
extern kal_uint8 upmu_get_strup_pwrkey_vcore_status(upmu_strup_list_enum strup);
extern kal_uint8 upmu_get_strup_pwrkey_deb_status(upmu_strup_list_enum strup);

// STRUP_CON1
extern void upmu_strup_dig_test_en(upmu_strup_list_enum strup, kal_uint8 val);
extern void upmu_strup_rst_drvsel(upmu_strup_list_enum strup, kal_uint8 val);
extern void upmu_strup_pmu_lev_ungate(upmu_strup_list_enum strup, kal_uint8 val);
extern void upmu_strup_flag_en(upmu_strup_list_enum strup, kal_uint8 val);
extern void upmu_strup_bias_gen_force(upmu_strup_list_enum strup, kal_uint8 val);

// STRUP_CON3
extern void upmu_strup_flag_sel(upmu_strup_list_enum strup, kal_uint8 val);

// STRUP_CON4
//reserved

#endif // #if defined(__DRV_UPMU_STRUP_V1__)


#if defined(__DRV_UPMU_LPOSC_V1__)
// LPOSC_CON0
extern void upmu_lposc_i_bias_cali(upmu_lposc_list_enum lposc, upmu_lposc_current_bias_calibration_enum val);
extern void upmu_lposc_digtal_circuit_enable(upmu_lposc_list_enum lposc, kal_bool enable);

// LPOSC_CON1
extern void upmu_lposc_output_freq_set(upmu_lposc_list_enum lposc, kal_uint8 val);
extern void upmu_lposc_buck_output_freq_switching(upmu_lposc_list_enum lposc, upmu_lposc_buck_output_freq_enum val);
extern void upmu_lposc_buck_boost_freq_divider(upmu_lposc_list_enum lposc, upmu_lposc_buck_boost_output_freq_enum val);
extern void upmu_lposc_auto_calibration_enable(upmu_lposc_list_enum lposc, kal_bool enable);

// LPOSC_CON2
extern void upmu_lposc_fd_resolution_adjust(upmu_lposc_list_enum lposc, upmu_lposc_fd_resolution_adjust_enum val);
extern void upmu_lposc_fd_disable_duration_1_adjust(upmu_lposc_list_enum lposc, upmu_lposc_fd_dis_dur1_enum val);
extern void upmu_lposc_fd_disable_duration_2_adjust(upmu_lposc_list_enum lposc, upmu_lposc_fd_dis_dur2_enum val);
extern void upmu_lposc_ssc_mod_amp(upmu_lposc_list_enum lposc, upmu_lposc_ssc_mod_amp_enum val);
extern void upmu_lposc_buck_boost_enable(upmu_lposc_list_enum lposc, kal_bool enable);
extern void upmu_lposc_ssc_code_duration(upmu_lposc_list_enum lposc, upmu_lposc_ssc_code_dur_enum val);
extern void upmu_lposc_phase_generation_block_enable(upmu_lposc_list_enum lposc, kal_bool enable);

// LPOSC_CON3
extern void upmu_lposc_buck_4_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val);
extern void upmu_lposc_buck_3_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val);
extern void upmu_lposc_buck_2_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val);
extern void upmu_lposc_buck_1_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val);

// LPOSC_CON4
extern void upmu_lposc_buck_6_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val);
extern void upmu_lposc_buck_5_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val);
extern void upmu_lposc_init_dac_enable(upmu_lposc_list_enum lposc, kal_bool enable);

#endif // #if defined(__DRV_UPMU_LPOSC_V1__)


#if defined(__DRV_UPMU_Retention_V1__)
extern void upmu_data_retention(upmu_retention_list_enum chr, kal_uint8 val);
#endif // #if defined(__DRV_UPMU_Retention_V1__)

extern void upmu_lpocs_sw_mode(kal_bool enable);

#endif // #ifndef __UPMU_COMMON_SW_H__

