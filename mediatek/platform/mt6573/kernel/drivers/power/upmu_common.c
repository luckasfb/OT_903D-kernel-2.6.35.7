
//#include <linux/irqflags.h>

#include "upmu_hw.h"
#include "upmu_sw.h"

#include "pmu6573_sw.h"

//#define DRV_Reg(addr)               (*(volatile kal_uint16 *)(addr))
//#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))

#define DRV_ClearBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp &=~(data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp |= (data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetData(addr, bitmask, value)     {\
   kal_uint16 temp;\
   temp = (~(bitmask)) & DRV_Reg(addr);\
   temp |= (value);\
   DRV_WriteReg(addr,temp);\
}

#define PMU_DRV_ClearBits16(addr, data)             DRV_ClearBits(addr,data)
#define PMU_DRV_SetBits16(addr, data)               DRV_SetBits(addr,data)
#define PMU_DRV_WriteReg16(addr, data)              DRV_WriteReg(addr, data)
#define PMU_DRV_ReadReg16(addr)                     DRV_Reg(addr)
#define PMU_DRV_SetData16(addr, bitmask, value)     DRV_SetData(addr, bitmask, value)

//#define PMU_SaveAndSetIRQMask()    SaveAndSetIRQMask()
//#define PMU_RestoreIRQMask(n)      RestoreIRQMask(n)

//#define PMU_SaveAndSetIRQMask()	local_irq_save() 
//#define PMU_RestoreIRQMask(n)	local_irq_restore(n)


#if defined(__DRV_UPMU_LDO_V1__)
// LDO CON0
void upmu_ldo_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_EN_MASK, ((kal_uint16)enable << LDO_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_ldo_on_sel(upmu_ldo_list_enum ldo, upmu_ldo_on_sel_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_ON_SEL_MASK, ((kal_uint16)sel << LDO_ON_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// If the LDO does NOT support passed-in voltage, driver will assert
void upmu_ldo_vol_sel(upmu_ldo_list_enum ldo, upmu_ldo_vol_enum vol)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_ldo_vol_enum *p_ldo_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage >= 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num >= 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_ldo_vol = &p_upmu_entry->vol_list[0];

	// If LDO allowed configured voltage is only 1, we just need to compare the allowed voltage and configure voltage
	if (p_upmu_entry->vol_list_num == 1)
	{
		if (*p_ldo_vol != vol)
		{
			// TTTTTT TBD
			//ASSERT(0); // Expected voltage is NOT allowed to configured
		}
		return;
	}

	for (i=0;i<vol_list_num;i++)
	{
		if (*p_ldo_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
			                    LDO_VOL_SEL_MASK, ((kal_uint16)i << LDO_VOL_SEL_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_ldo_vol++;
	}
	// Unexpexted path
	//ASSERT(0);
}

void upmu_ldo_ndis_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_NDIS_EN_MASK, ((kal_uint16)enable << LDO_NDIS_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_ldo_stb_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_STB_EN_MASK, ((kal_uint16)enable << LDO_STB_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}



void upmu_ldo_oc_auto_off(upmu_ldo_list_enum ldo, kal_bool auto_off)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_OC_AUTO_OFF_MASK, ((kal_uint16)auto_off << LDO_OC_AUTO_OFF_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_ldo_ocfb_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_OCFB_EN_MASK, ((kal_uint16)enable << LDO_OCFB_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_ldo_get_oc_status(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & LDO_OC_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_ldo_get_status(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & LDO_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_ldo_oc_status(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & LDO_OC_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_ldo_status(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & LDO_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

//AddForMT6573
void upmu_ldo_psclk_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_PSCLK_EN_MASK, ((kal_uint16)enable << LDO_PSCLK_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_stb_force_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    LDO_STB_FORCE_MASK, ((kal_uint16)enable << LDO_STB_FORCE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_va28_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VA28_BIST_EN_MASK, ((kal_uint16)enable << VA28_BIST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_va25_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VA25_BIST_EN_MASK, ((kal_uint16)enable << VA25_BIST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_va12_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VA12_BIST_EN_MASK, ((kal_uint16)enable << VA12_BIST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_vmic_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VMIC_BIST_EN_MASK, ((kal_uint16)enable << VMIC_BIST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573_2
void upmu_ldo_vaudp_stb_force(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VAUDP_STB_FORCE_MASK, ((kal_uint16)enable << VAUDP_STB_FORCE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573_2
void upmu_ldo_vaudn_ck_mode(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VAUDN_CK_MODE_MASK, ((kal_uint16)enable << VAUDN_CK_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573_2
void upmu_ldo_vaudn_ck_force(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    VAUDN_CK_FORCE_MASK, ((kal_uint16)enable << VAUDN_CK_FORCE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_pmt_passgate_ovrd_enable(upmu_ldo_list_enum ldo, upmu_pmt_passgate_ovrd_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON0_OFFSET),
	                    PMT_PASSGATE_OVRD_MASK, ((kal_uint16)sel << PMT_PASSGATE_OVRD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LDO CON1
#if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON1_BIT0__)
void upmu_ldo_stb_td(upmu_ldo_list_enum ldo, upmu_ldo_stb_td_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    LDO_STB_TD_MASK, ((kal_uint16)sel << LDO_STB_TD_SHIFT));
	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON1_BIT0__)

void upmu_ldo_cal(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    LDO_CAL_MASK, ((kal_uint16)val << LDO_CAL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_force_low_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    LDO_FORCE_LOW_MASK, ((kal_uint16)enable << LDO_FORCE_LOW_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_mode_enable(upmu_ldo_list_enum ldo, kal_bool enable_low_power_mode)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    LDO_MODE_MASK, ((kal_uint16)enable_low_power_mode << LDO_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_mode_select(upmu_ldo_list_enum ldo, upmu_ldo_mode_sel_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    LDO_MODE_SEL_MASK, ((kal_uint16)sel << LDO_MODE_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_srclk_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    LDO_SRCLK_EN_MASK, ((kal_uint16)enable << LDO_SRCLK_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_vaudp_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    VAUDP_BIST_EN_MASK, ((kal_uint16)enable << VAUDP_BIST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_vtv_bist_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    VTV_BIST_EN_MASK, ((kal_uint16)enable << VTV_BIST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_thr_hwpdn_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    THR_HWPDN_EN_MASK, ((kal_uint16)enable << THR_HWPDN_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_usbdl_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON1_OFFSET),
	                    USBDL_EN_MASK, ((kal_uint16)enable << USBDL_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LDO CON2
void upmu_ldo_oc_td(upmu_ldo_list_enum ldo, upmu_ldo_oc_td_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LDO_OC_TD_MASK, ((kal_uint16)sel << LDO_OC_TD_SHIFT));
	//PMU_RestoreIRQMask(savedMask);
}

#if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON2_BIT6__)
void upmu_ldo_stb_td(upmu_ldo_list_enum ldo, upmu_ldo_stb_td_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LDO_STB_TD_MASK, ((kal_uint16)sel << LDO_STB_TD_SHIFT));
	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_UPMU_LDO_V1_STB_TD_AT_CON2_BIT6__)

void upmu_ldo_ical(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                   LDO_ICAL_EN_MASK, ((kal_uint16)val << LDO_ICAL_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_en_force_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LDO_EN_FORCE_MASK, ((kal_uint16)enable << LDO_EN_FORCE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_gpldo_enable(upmu_ldo_list_enum ldo, upmu_ldo_gpldo_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LDO_GPLDO_EN_MASK, ((kal_uint16)sel << LDO_GPLDO_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_lnibias_trimctrl(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LNIBIAS_TRIMCTRL_MASK, ((kal_uint16)val << LNIBIAS_TRIMCTRL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_s2d_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    ABIST_S2D_EN_MASK, ((kal_uint16)enable << ABIST_S2D_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_s2d2_bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    ABIST_S2D2_BBRX_MASK, ((kal_uint16)enable << ABIST_S2D2_BBRX_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_lnibias_trim(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LNIBIAS_TRIM_MASK, ((kal_uint16)val << LNIBIAS_TRIM_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_txbias2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    ABIST_TXBIAS2BBRX_MASK, ((kal_uint16)enable << ABIST_TXBIAS2BBRX_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_txvga2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    ABIST_TXVGA2BBRX_MASK, ((kal_uint16)enable << ABIST_TXVGA2BBRX_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_apc2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    ABIST_APC2BBRX_MASK, ((kal_uint16)enable << ABIST_APC2BBRX_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_afc2bbrx_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    ABIST_AFC2BBRX_MASK, ((kal_uint16)enable << ABIST_AFC2BBRX_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_lnibias_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON2_OFFSET),
	                    LNIBIAS_ENABLE_MASK, ((kal_uint16)enable << LNIBIAS_ENABLE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LDO CON3
//AddForMT6573
void upmu_ldo_simx_mode(upmu_ldo_list_enum ldo, upmu_ldo_vsimx_mode_enum sel)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    SIMX_MODE_MASK, ((kal_uint16)sel << SIMX_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_simx_srn(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    SIMX_SRN_MASK, ((kal_uint16)val << SIMX_SRN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_simx_srp(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    SIMX_SRP_MASK, ((kal_uint16)val << SIMX_SRP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_simx_bias(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    SIMX_BIAS_MASK, ((kal_uint16)val << SIMX_BIAS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_simxio_drv(upmu_ldo_list_enum ldo, kal_uint8 val)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    SIMXIO_DRV_MASK, ((kal_uint16)val << SIMXIO_DRV_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
kal_uint8 upmu_ldo_get_gpio_srst2(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & GPIO_SRST2_MASK) != 0)
	{
		return 1;
	}
	return 0;
}

//AddForMT6573
kal_uint8 upmu_ldo_get_gpio_sclk2(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & GPIO_SCLK2_MASK) != 0)
	{
		return 1;
	}
	return 0;
}

//AddForMT6573
kal_uint8 upmu_ldo_get_gpio_sio2(upmu_ldo_list_enum ldo)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+LDO_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & GPIO_SIO2_MASK) != 0)
	{
		return 1;
	}
	return 0;
}

//AddForMT6573
void upmu_ldo_abist_apc2accdet_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_APC2ACCDET_MASK, ((kal_uint16)enable << ABIST_APC2ACCDET_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_afc2aux_ym_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_AFC2AUX_YM_MASK, ((kal_uint16)enable << ABIST_AFC2AUX_YM_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_afc2aux_yp_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_AFC2AUX_YP_MASK, ((kal_uint16)enable << ABIST_AFC2AUX_YP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_afc2aux_xp_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_AFC2AUX_XP_MASK, ((kal_uint16)enable << ABIST_AFC2AUX_XP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_aud2auxadc_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_AUD2AUXADC_MASK, ((kal_uint16)enable << ABIST_AUD2AUXADC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_tvdac2auxadc_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_TVDAC2AUXADC_MASK, ((kal_uint16)enable << ABIST_TVDAC2AUXADC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_ldo_abist_afc2fgadc_enable(upmu_ldo_list_enum ldo, kal_bool enable)
{
	upmu_ldo_profile_entry *p_upmu_entry = &upmu_ldo_profile[(kal_uint16)ldo];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LDO_CON3_OFFSET),
	                    ABIST_AFC2FGADC_MASK, ((kal_uint16)enable << ABIST_AFC2FGADC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#endif // #if defined(__DRV_UPMU_LDO_V1__)


#if defined(__DRV_UPMU_BUCK_V1__)
// BUCK CON0
void upmu_buck_enable(upmu_buck_list_enum buck, kal_bool enable)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                   BUCK_EN_MASK, ((kal_uint16)enable << BUCK_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_rs(upmu_buck_list_enum buck, upmu_buck_rs_enum sel)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                    BUCK_RS_MASK, ((kal_uint16)sel << BUCK_RS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_sf_str_mode(upmu_buck_list_enum buck, upmu_buck_sf_str_mode_enum sel)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                    BUCK_ST_STR_MASK, ((kal_uint16)sel << BUCK_ST_STR_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_normal_voltage_adjust(upmu_buck_list_enum buck, upmu_buck_vol_enum vol)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_vol_enum *p_buck_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
			                    BUCK_VFBADJ_MASK, ((kal_uint16)i << BUCK_VFBADJ_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

void upmu_buck_disable_anti_undershoot(upmu_buck_list_enum buck, kal_bool disable)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                    BUCK_DIS_ANTIUNSH_MASK, ((kal_uint16)disable << BUCK_DIS_ANTIUNSH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_oc_auto_off(upmu_buck_list_enum buck, kal_bool auto_off)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                    BUCK_OC_AUTO_OFF_MASK, ((kal_uint16)auto_off << BUCK_OC_AUTO_OFF_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_ocfb_enable(upmu_buck_list_enum buck, kal_bool enable)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                    BUCK_OCFB_EN_MASK, ((kal_uint16)enable << BUCK_OCFB_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_stb_enable(upmu_buck_list_enum buck, kal_bool enable)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON0_OFFSET),
	                   BUCK_STB_EN_MASK, ((kal_uint16)enable << BUCK_STB_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
kal_bool upmu_buck_get_oc_status(upmu_buck_list_enum buck)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+BUCK_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & BUCK_OC_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

//AddForMT6573
kal_bool upmu_buck_get_status(upmu_buck_list_enum buck)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+BUCK_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & BUCK_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// BUCK CON1
void upmu_buck_modeset(upmu_buck_list_enum buck, upmu_buck_modeset_enum sel)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON1_OFFSET),
	                    BUCK_MODESET_MASK, ((kal_uint16)sel << BUCK_MODESET_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_sleep_voltage_adjust(upmu_buck_list_enum buck, upmu_buck_vol_enum vol)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_vol_enum *p_buck_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON1_OFFSET),
			                    BUCK_VFBADJ_SLEEP_MASK, ((kal_uint16)i << BUCK_VFBADJ_SLEEP_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

//AddForMT6573
void upmu_buck_cpmcksel(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON1_OFFSET),
	                    BUCK_CLK_SRC_MASK, ((kal_uint16)val << BUCK_CLK_SRC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
kal_bool upmu_is_buck_mode(upmu_buck_list_enum buck)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+BUCK_CON1_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & MODE_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// BUCK CON2
void upmu_buck_cal(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON2_OFFSET),
	                    BUCK_CAL_MASK, ((kal_uint16)val << BUCK_CAL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_vosel(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON2_OFFSET),
	                    BUCK_VOSEL_MASK, ((kal_uint16)val << BUCK_VOSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BUCK CON3
void upmu_buck_oc_td(upmu_buck_list_enum buck, upmu_buck_oc_td_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON3_OFFSET),
	                    BUCK_OC_TD_MASK, ((kal_uint16)val << BUCK_OC_TD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_stb_td(upmu_buck_list_enum buck, upmu_buck_stb_td_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON3_OFFSET),
	                    BUCK_STB_TD_MASK, ((kal_uint16)val << BUCK_STB_TD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_oc_thd(upmu_buck_list_enum buck, upmu_buck_oc_thd_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON3_OFFSET),
	                    BUCK_OC_THD_MASK, ((kal_uint16)val << BUCK_OC_THD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_oc_wnd(upmu_buck_list_enum buck, upmu_buck_oc_wnd_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON3_OFFSET),
	                    BUCK_OC_WND_MASK, ((kal_uint16)val << BUCK_OC_WND_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_ical(upmu_buck_list_enum buck, upmu_buck_ical_en_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON3_OFFSET),
	                    BUCK_ICAL_EN_MASK, ((kal_uint16)val << BUCK_ICAL_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_en_force(upmu_buck_list_enum buck, kal_bool enable)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON3_OFFSET),
	                   BUCK_EN_FORCE_MASK, ((kal_uint16)enable << BUCK_EN_FORCE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BUCK CON4

//AddForMT6573
void upmu_buck_slew_nmos(upmu_buck_list_enum buck, upmu_buck_slew_nmos_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON4_OFFSET),
	                    BUCK_SLEW_NMOS_MASK, ((kal_uint16)val << BUCK_SLEW_NMOS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_slew_mos(upmu_buck_list_enum buck, upmu_buck_slew_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON4_OFFSET),
	                    BUCK_SLEW_MASK, ((kal_uint16)val << BUCK_SLEW_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_adjcksel(upmu_buck_list_enum buck, upmu_buck_adjcksel_enum val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON4_OFFSET),
	                    BUCK_ADJCKSEL_MASK, ((kal_uint16)val << BUCK_ADJCKSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BUCK CON5
void upmu_buck_csl(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON5_OFFSET),
	                    BUCK_CSL_MASK, ((kal_uint16)val << BUCK_CSL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_buck_burst(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON5_OFFSET),
	                    BUCK_BURST_MASK, ((kal_uint16)val << BUCK_BURST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_csr(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON5_OFFSET),
	                    BUCK_CSR_MASK, ((kal_uint16)val << BUCK_CSR_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_rzsel(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON5_OFFSET),
	                    BUCK_RZSEL_MASK, ((kal_uint16)val << BUCK_RZSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_gmsel(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON5_OFFSET),
	                    BUCK_GMSEL_MASK, ((kal_uint16)val << BUCK_GMSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_zx_pdn(upmu_buck_list_enum buck, kal_uint8 val)
{
	upmu_buck_profile_entry *p_upmu_entry = &upmu_buck_profile[(kal_uint16)buck];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_CON5_OFFSET),
	                    BUCK_ZX_PDN_MASK, ((kal_uint16)val << BUCK_ZX_PDN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#endif // #if defined(__DRV_UPMU_BUCK_V1__)

#if defined(__DRV_UPMU_BUCK_BOOST_V1__)
// BUCK_BOOST CON0
//AddForMT6573
void upmu_buck_boost_cc(upmu_buck_boost_list_enum buck_boost, kal_uint8 val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_CC_MASK, ((kal_uint16)val << BUCK_BOOST_CC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_rc(upmu_buck_boost_list_enum buck_boost, kal_uint8 val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_RC_MASK, ((kal_uint16)val << BUCK_BOOST_RC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_sr2(upmu_buck_boost_list_enum buck_boost, kal_uint8 val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_SR2_MASK, ((kal_uint16)val << BUCK_BOOST_SR2_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_sr1(upmu_buck_boost_list_enum buck_boost, kal_uint8 val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_SR1_MASK, ((kal_uint16)val << BUCK_BOOST_SR1_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_cs_adj(upmu_buck_boost_list_enum buck_boost, kal_uint8 val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_CS_ADJ_MASK, ((kal_uint16)val << BUCK_BOOST_CS_ADJ_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_anti_ring(upmu_buck_boost_list_enum buck_boost, kal_bool enable)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_ANTI_RING_MASK, ((kal_uint16)enable << BUCK_BOOST_ANTI_RING_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_enable(upmu_buck_boost_list_enum buck_boost, kal_bool enable)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON0_OFFSET),
	                    BUCK_BOOST_EN_MASK, ((kal_uint16)enable << BUCK_BOOST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BUCK_BOOST CON1
//AddForMT6573
void upmu_buck_boost_fpwm(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_fpwm_enum val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON1_OFFSET),
	                    BUCK_BOOST_FPWM_MASK, ((kal_uint16)val << BUCK_BOOST_FPWM_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_tm(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_tm_enum val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON1_OFFSET),
	                    BUCK_BOOST_TM_MASK, ((kal_uint16)val << BUCK_BOOST_TM_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_buck_boost_ss_spd(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_ss_spd_enum val)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON1_OFFSET),
	                    BUCK_BOOST_SS_SPD_MASK, ((kal_uint16)val << BUCK_BOOST_SS_SPD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
kal_bool upmu_buck_boost_get_status(upmu_buck_boost_list_enum buck_boost)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+BUCK_BOOST_CON1_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & BUCK_BOOST_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// BUCK_BOOST CON2
//AddForMT6573
void upmu_buck_boost_voltage_tune_0(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON2_OFFSET),
			                    BUCK_BOOST_VOTUNE_0_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_0_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

//AddForMT6573
void upmu_buck_boost_voltage_tune_1(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON2_OFFSET),
			                    BUCK_BOOST_VOTUNE_1_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_1_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

// BUCK_BOOST CON3
//AddForMT6573
void upmu_buck_boost_voltage_tune_2(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON3_OFFSET),
			                    BUCK_BOOST_VOTUNE_2_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_2_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

//AddForMT6573
void upmu_buck_boost_voltage_tune_3(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON3_OFFSET),
			                    BUCK_BOOST_VOTUNE_3_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_3_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

// BUCK_BOOST CON4
//AddForMT6573
void upmu_buck_boost_voltage_tune_4(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON4_OFFSET),
			                    BUCK_BOOST_VOTUNE_4_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_4_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

//AddForMT6573
void upmu_buck_boost_voltage_tune_5(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON4_OFFSET),
			                    BUCK_BOOST_VOTUNE_5_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_5_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

// BUCK_BOOST CON5
//AddForMT6573
void upmu_buck_boost_voltage_tune_6(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON5_OFFSET),
			                    BUCK_BOOST_VOTUNE_6_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_6_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

//AddForMT6573
void upmu_buck_boost_voltage_tune_7(upmu_buck_boost_list_enum buck_boost, upmu_buck_boost_vol_enum vol)
{
	upmu_buck_boost_profile_entry *p_upmu_entry = &upmu_buck_boost_profile[(kal_uint16)buck_boost];
	kal_uint32 i;
	kal_uint32 vol_list_num;
	upmu_buck_boost_vol_enum *p_buck_boost_vol;
	//kal_uint32 savedMask;

	// Only when configurable voltage > 1 then we allow to configure voltage
	//ASSERT(p_upmu_entry->vol_list_num > 1);  // TTTT

	// Read value once here
	vol_list_num = p_upmu_entry->vol_list_num;

	// Use pointer operation to speed up
	p_buck_boost_vol = &p_upmu_entry->vol_list[0];



	for (i=0;i<vol_list_num;i++)
	{
		if (*p_buck_boost_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+BUCK_BOOST_CON5_OFFSET),
			                    BUCK_BOOST_VOTUNE_7_MASK, ((kal_uint16)i << BUCK_BOOST_VOTUNE_7_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_buck_boost_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

#endif // #if defined(__DRV_UPMU_BUCK_BOOST_V1__)

#if defined(__DRV_UPMU_BOOST_V1__)
// BOOST CON0
void upmu_boost_enable(upmu_boost_list_enum boost, kal_bool enable)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON0_OFFSET),
	                    BOOST_EN_MASK, ((kal_uint16)enable << BOOST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_type(upmu_boost_list_enum boost, upmu_boost_type_enum val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON0_OFFSET),
	                    BOOST_TYPE_MASK, ((kal_uint16)val << BOOST_TYPE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_mode(upmu_boost_list_enum boost, upmu_boost_mode_enum val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON0_OFFSET),
	                    BOOST_MODE_MASK, ((kal_uint16)val << BOOST_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_vrsel(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON0_OFFSET),
	                    BOOST_VRSEL_MASK, ((kal_uint16)val << BOOST_VRSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_oc_auto_off(upmu_boost_list_enum boost, kal_bool auto_off)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON0_OFFSET),
	                    BOOST_OC_AUTO_OFF_MASK, ((kal_uint16)auto_off << BOOST_OC_AUTO_OFF_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_boost_oc_status(upmu_boost_list_enum boost)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+BOOST_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	if ((val & BOOST_OC_FLAG_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// BOOST CON1
void upmu_boost_cl(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON1_OFFSET),
	                    BOOST_CL_MASK, ((kal_uint16)val << BOOST_CL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_cs(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON1_OFFSET),
	                    BOOST_CS_MASK, ((kal_uint16)val << BOOST_CS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_rc(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON1_OFFSET),
	                    BOOST_RC_MASK, ((kal_uint16)val << BOOST_RC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_ss(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON1_OFFSET),
	                    BOOST_SS_MASK, ((kal_uint16)val << BOOST_SS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BOOST CON2
void upmu_boost_cksel(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON2_OFFSET),
	                    BOOST_CKSEL_MASK, ((kal_uint16)val << BOOST_CKSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_sr_pmos(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON2_OFFSET),
	                    BOOST_SR_PMOS_MASK, ((kal_uint16)val << BOOST_SR_PMOS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_sr_nmos(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON2_OFFSET),
	                    BOOST_SR_NMOS_MASK, ((kal_uint16)val << BOOST_SR_NMOS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_slp(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON2_OFFSET),
	                    BOOST_SLP_MASK, ((kal_uint16)val << BOOST_SLP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}


// BOOST CON3
void upmu_boost_cks_prg(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON3_OFFSET),
	                    BOOST_CKS_PRG_MASK, ((kal_uint16)val << BOOST_CKS_PRG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BOOST CON4
void upmu_boost_oc_thd(upmu_boost_list_enum boost, upmu_boost_oc_thd_enum val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON4_OFFSET),
	                    BOOST_OC_THD_MASK, ((kal_uint16)val << BOOST_OC_THD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_oc_wnd(upmu_boost_list_enum boost, upmu_boost_oc_wnd_enum val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON4_OFFSET),
	                    BOOST_OC_WND_MASK, ((kal_uint16)val << BOOST_OC_WND_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_clk_cal(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//ASSERT(val < 8);

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON4_OFFSET),
	                    BOOST_CLK_CAL_MASK, ((kal_uint16)val << BOOST_CLK_CAL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// BOOST CON6
void upmu_boost_hw_sel(upmu_boost_list_enum boost, upmu_boost_hw_sel_enum val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON6_OFFSET),
	                    BOOST_HW_SEL_MASK, ((kal_uint16)val << BOOST_HW_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_boost_cc(upmu_boost_list_enum boost, kal_uint8 val)
{
	upmu_boost_profile_entry *p_upmu_entry = &upmu_boost_profile[(kal_uint16)boost];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+BOOST_CON6_OFFSET),
	                    BOOST_CC_MASK, ((kal_uint16)val << BOOST_CC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}



#endif // #if defined(__DRV_UPMU_BOOST_V1__)


#if defined(__DRV_UPMU_ISINK_V1__)
// ISINK CON0
void upmu_isink_enable(upmu_isink_list_enum isink, kal_bool enable)
{
	upmu_isink_profile_entry *p_upmu_entry = &upmu_isink_profile[(kal_uint16)isink];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+ISINK_CON0_OFFSET),
	                    ISINK_EN_MASK, ((kal_uint16)enable << ISINK_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_isink_mode(upmu_isink_list_enum isink, upmu_isink_mode_enum val)
{
	upmu_isink_profile_entry *p_upmu_entry = &upmu_isink_profile[(kal_uint16)isink];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+ISINK_CON0_OFFSET),
	                    ISINK_MODE_MASK, ((kal_uint16)val << ISINK_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_isink_step(upmu_isink_list_enum isink, kal_uint8 val)
{
	upmu_isink_profile_entry *p_upmu_entry = &upmu_isink_profile[(kal_uint16)isink];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+ISINK_CON0_OFFSET),
	                    ISINK_STEP_MASK, ((kal_uint16)val << ISINK_STEP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_isink_status(upmu_isink_list_enum isink)
{
	upmu_isink_profile_entry *p_upmu_entry = &upmu_isink_profile[(kal_uint16)isink];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+ISINK_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	if ((val & ISINK_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// ISINK CON1
void upmu_isinks_vref_cal(upmu_isink_list_enum isink, kal_uint8 val)
{
	upmu_isink_profile_entry *p_upmu_entry = &upmu_isink_profile[(kal_uint16)isink];
	//kal_uint32 savedMask;

	//ASSERT(isink == ISINK0); // The control register is only located in ISINK0

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+ISINK_CON1_OFFSET),
	                    ISINK_VREF_CAL_MASK, ((kal_uint16)val << ISINK_VREF_CAL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}


#endif // #if defined(__DRV_UPMU_ISINK_V1__)

#if defined(__DRV_UPMU_KPLED_V1__)
// KPLED CON0
void upmu_kpled_enable(upmu_kpled_list_enum kpled, kal_bool enable)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON0_OFFSET),
	                    KPLED_EN_MASK, ((kal_uint16)enable << KPLED_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_kpled_mode(upmu_kpled_list_enum kpled, upmu_kpled_mode_enum val)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON0_OFFSET),
	                    KPLED_MODE_MASK, ((kal_uint16)val << KPLED_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_kpled_sel(upmu_kpled_list_enum kpled, kal_uint8 val)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON0_OFFSET),
	                    KPLED_SEL_MASK, ((kal_uint16)val << KPLED_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_kpled_sfstrt_c(upmu_kpled_list_enum kpled, upmu_kpled_sf_start_time_constane_enum val)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON0_OFFSET),
	                    KPLED_SFSTRT_C_MASK, ((kal_uint16)val << KPLED_SFSTRT_C_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_kpled_status(upmu_kpled_list_enum kpled)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+KPLED_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	if ((val & KPLED_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_kpled_get_status(upmu_kpled_list_enum kpled)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+KPLED_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	if ((val & KPLED_STATUS_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

//AddForMT6573
void upmu_kpled_thermal_shutdown_enable(upmu_kpled_list_enum kpled, kal_bool enable)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON0_OFFSET),
	                    KPLED_THER_SHDN_EN_MASK, ((kal_uint16)enable << KPLED_THER_SHDN_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_kpled_sfstrt_en(upmu_kpled_list_enum kpled, kal_bool enable)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON0_OFFSET),
	                    KPLED_SFSTRT_EN_MASK, ((kal_uint16)enable << KPLED_SFSTRT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// KPLED CON1
//AddForMT6573
void upmu_kpled_force_off_enable(upmu_kpled_list_enum kpled, kal_bool enable)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON1_OFFSET),
	                    KPLED_FORCE_OFF_EN_MASK, ((kal_uint16)enable << KPLED_FORCE_OFF_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_kpled_test_mode_enable(upmu_kpled_list_enum kpled, kal_bool enable)
{
	upmu_kpled_profile_entry *p_upmu_entry = &upmu_kpled_profile[(kal_uint16)kpled];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+KPLED_CON1_OFFSET),
	                    KPLED_TEST_MODE_EN_MASK, ((kal_uint16)enable << KPLED_TEST_MODE_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#endif // #if defined(__DRV_UPMU_KPLED_V1__)

#if defined(__DRV_UPMU_SPK_V1__)
// SPK CON0
void upmu_spk_enable(upmu_spk_list_enum spk, kal_bool enable)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON0_OFFSET),
	                    SPK_EN_MASK, ((kal_uint16)enable << SPK_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_reset(upmu_spk_list_enum spk, kal_bool reset)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON0_OFFSET),
	                    SPK_RST_MASK, ((kal_uint16)reset << SPK_RST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_vol(upmu_spk_list_enum spk, upmu_spk_vol_enum vol)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	kal_uint32 i;
	kal_uint32 spk_vol_list_num;
	upmu_spk_vol_enum *p_spk_vol;
	//kal_uint32 savedMask;

	// Only when configurable volume > 1 then we allow to configure volume
	//ASSERT(p_upmu_entry->spk_vol_list_num > 1);  // TTTT

	// Read value once here
	spk_vol_list_num = p_upmu_entry->spk_vol_list_num;

	// Use pointer operation to speed up
	p_spk_vol = &p_upmu_entry->spk_vol_list[0];

	for (i=0;i<spk_vol_list_num;i++)
	{
		if (*p_spk_vol == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON0_OFFSET),
			                    SPK_VOL_MASK, ((kal_uint16)i << SPK_VOL_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_spk_vol++;
	}
	// Unexpexted path
	//ASSERT(0);

}

upmu_spk_vol_enum upmu_spk_get_vol(upmu_spk_list_enum spk)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;
	kal_uint16 val;

	// Only when configurable volume > 1 then we allow to configure volume
	//ASSERT(p_upmu_entry->spk_vol_list_num > 1);  // TTTT

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+SPK_CON0_OFFSET));

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON0_OFFSET),
	                    SPK_VOL_MASK, ((kal_uint16)val << SPK_VOL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
	val = (val & SPK_VOL_MASK) >> SPK_VOL_SHIFT;

	return p_upmu_entry->spk_vol_list[val];

}

void upmu_spk_oc_auto_off(upmu_spk_list_enum spk, kal_bool auto_off)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON0_OFFSET),
	                    SPK_OC_AUTO_OFF_MASK, ((kal_uint16)auto_off << SPK_OC_AUTO_OFF_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_ocfb_enable(upmu_spk_list_enum spk, kal_bool enable)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON0_OFFSET),
	                    SPK_OCFB_EN_MASK, ((kal_uint16)enable << SPK_OCFB_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_spk_oc_status(upmu_spk_list_enum spk)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+SPK_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	if ((val & SPK_OC_FLAG_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// SPK CON1
void upmu_spk_pfd_mode_enable(upmu_spk_list_enum spk, kal_bool enable)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON1_OFFSET),
	                    SPK_PFD_MODE_MASK, ((kal_uint16)enable << SPK_PFD_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_cmode(upmu_spk_list_enum spk, kal_uint8 val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON1_OFFSET),
	                    SPK_CMODE_MASK, ((kal_uint16)val << SPK_CMODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_ccode(upmu_spk_list_enum spk, kal_uint8 val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON1_OFFSET),
	                    SPK_CCODE_MASK, ((kal_uint16)val << SPK_CCODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// SPK CON2
void upmu_spk_oc_thd(upmu_spk_list_enum spk, upmu_spk_oc_thd_enum val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON2_OFFSET),
	                    SPK_OC_THD_MASK, ((kal_uint16)val << SPK_OC_THD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_oc_wnd(upmu_spk_list_enum spk, upmu_spk_oc_wnd_enum val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON2_OFFSET),
	                    SPK_OC_WND_MASK, ((kal_uint16)val << SPK_OC_WND_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// SPK CON3
void upmu_spk_osc_isel(upmu_spk_list_enum spk, kal_uint8 val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//ASSERT(val < 4);

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON3_OFFSET),
	                    SPK_OSC_ISEL_MASK, ((kal_uint16)val << SPK_OSC_ISEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_oc_enable(upmu_spk_list_enum spk, kal_bool enable)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON3_OFFSET),
	                    SPK_OC_EN_MASK, ((kal_uint16)enable << SPK_OC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// SPK CON7
void upmu_spk_mode(upmu_spk_list_enum spk, upmu_spk_mode_enum val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//ASSERT(val < 4);

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON7_OFFSET),
	                    SPK_MODE_MASK, ((kal_uint16)val << SPK_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_ab_obias(upmu_spk_list_enum spk, kal_uint8 val)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//ASSERT(val < 4);

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON7_OFFSET),
	                    SPK_AB_OBIAS_MASK, ((kal_uint16)val << SPK_AB_OBIAS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_spk_ab_oc_enable(upmu_spk_list_enum spk, kal_bool enable)
{
	upmu_spk_profile_entry *p_upmu_entry = &upmu_spk_profile[(kal_uint16)spk];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+SPK_CON7_OFFSET),
	                    SPK_AB_OC_EN_MASK, ((kal_uint16)enable << SPK_AB_OC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}


#endif // #if defined(__DRV_UPMU_SPK_V1__)


#if defined(__DRV_UPMU_CHARGER_V1__)
// CHR_CON0
#if 0
void upmu_vcdt_lv_vth(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
	                    VCDT_LV_VTH_MASK, ((kal_uint16)val << VCDT_LV_VTH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vcdt_hv_vth(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
	                    VCDT_HV_VTH_MASK, ((kal_uint16)val << VCDT_HV_VTH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif

void upmu_vcdt_lv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint32 i;
	kal_uint32 chr_det_lv_list_num;
	upmu_chr_vol_enum *p_voltage;
	//kal_uint32 savedMask;

	// Read value once here
	chr_det_lv_list_num = p_upmu_entry->chr_det_lv_list_num;

	// Use pointer operation to speed up
	p_voltage = &p_upmu_entry->chr_det_lv_list[0];

	for (i=0;i<chr_det_lv_list_num;i++)
	{
		if (*p_voltage == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
			                    VCDT_LV_VTH_MASK, ((kal_uint16)i << VCDT_LV_VTH_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_voltage++;
	}
	// Unexpexted path
	//ASSERT(0);

}

void upmu_vcdt_hv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint32 i;
	kal_uint32 chr_det_hv_list_num;
	upmu_chr_vol_enum *p_voltage;
	//kal_uint32 savedMask;

	// Read value once here
	chr_det_hv_list_num = p_upmu_entry->chr_det_hv_list_num;

	// Use pointer operation to speed up
	p_voltage = &p_upmu_entry->chr_det_hv_list[0];

	for (i=0;i<chr_det_hv_list_num;i++)
	{
		if (*p_voltage == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
			                    VCDT_HV_VTH_MASK, ((kal_uint16)i << VCDT_HV_VTH_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_voltage++;
	}
	// Unexpexted path
	//ASSERT(0);
}

void upmu_vcdt_hv_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
	                    VCDT_HV_EN_MASK, ((kal_uint16)enable << VCDT_HV_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_pchr_auto_enable(upmu_chr_list_enum chr, kal_bool auto_enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
	                    PCHR_AUTO_MASK, ((kal_uint16)auto_enable << PCHR_AUTO_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_csdac_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
	                    CSDAC_EN_MASK, ((kal_uint16)enable << CSDAC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

extern void upmu_chrwdt_clear_internal(upmu_chr_list_enum chr);
void upmu_chr_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	upmu_chrwdt_clear_internal(chr);

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON0_OFFSET),
	                    CHR_EN_MASK, ((kal_uint16)enable << CHR_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_chr_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & CHRDET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_vcdt_lv_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VCDT_LV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_vcdt_hv_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VCDT_HV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_chr_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & CHRDET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_vcdt_lv_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VCDT_LV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_vcdt_hv_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VCDT_HV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

//AddForMT6573
kal_bool upmu_is_chr_ldo_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & CHR_LDO_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// CHR_CON1
#if 0
void upmu_vbat_cv_vth(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON1_OFFSET),
	                    VBAT_CV_VTH_MASK, ((kal_uint16)val << VBAT_CV_VTH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vbat_cc_vth(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON1_OFFSET),
	                    VBAT_CC_VTH_MASK, ((kal_uint16)val << VBAT_CC_VTH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif

void upmu_vbat_cv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint32 i;
	kal_uint32 chr_vbat_cv_list_num;
	upmu_chr_vol_enum *p_voltage;
	//kal_uint32 savedMask;

	// Read value once here
	chr_vbat_cv_list_num = p_upmu_entry->chr_vbat_cv_list_num;

	// Use pointer operation to speed up
	p_voltage = &p_upmu_entry->chr_vbat_cv_list[0];

	for (i=0;i<chr_vbat_cv_list_num;i++)
	{
		if (*p_voltage == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON1_OFFSET),
			                    VBAT_CV_VTH_MASK, ((kal_uint16)i << VBAT_CV_VTH_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_voltage++;
	}
	// Unexpexted path
	//ASSERT(0);

}

void upmu_vbat_cc_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint32 i;
	kal_uint32 chr_vbat_cc_list_num;
	upmu_chr_vol_enum *p_voltage;
	//kal_uint32 savedMask;

	// Read value once here
	chr_vbat_cc_list_num = p_upmu_entry->chr_vbat_cc_list_num;

	// Use pointer operation to speed up
	p_voltage = &p_upmu_entry->chr_vbat_cc_list[0];

	for (i=0;i<chr_vbat_cc_list_num;i++)
	{
		if (*p_voltage == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON1_OFFSET),
			                    VBAT_CC_VTH_MASK, ((kal_uint16)i << VBAT_CC_VTH_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_voltage++;
	}
	// Unexpexted path
	//ASSERT(0);

}

void upmu_vbat_cv_det_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON1_OFFSET),
	                    VBAT_CV_EN_MASK, ((kal_uint16)enable << VBAT_CV_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vbat_cc_det_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON1_OFFSET),
	                    VBAT_CC_EN_MASK, ((kal_uint16)enable << VBAT_CC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_vbat_cv_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON1_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VBAT_CV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_vbat_cc_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON1_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VBAT_CC_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_vbat_cv_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON1_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VBAT_CV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_vbat_cc_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON1_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VBAT_CC_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// CHR_CON2
void upmu_pchr_tohtc(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON2_OFFSET),
	                    PCHR_TOHTC_MASK, ((kal_uint16)val << PCHR_TOHTC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_pchr_toltc(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON2_OFFSET),
	                    PCHR_TOLTC_MASK, ((kal_uint16)val << PCHR_TOLTC_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_cs_vth(upmu_chr_list_enum chr, upmu_chr_current_enum current_val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint32 i;
	kal_uint32 chr_current_list_num;
	upmu_chr_current_enum *p_chr_current;
	//kal_uint32 savedMask;

	// Read value once here
	chr_current_list_num = p_upmu_entry->chr_current_list_num;

	// Use pointer operation to speed up
	p_chr_current = &p_upmu_entry->chr_current_list[0];

	for (i=0;i<chr_current_list_num;i++)
	{
		if (*p_chr_current == current_val)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON2_OFFSET),
			                    CS_VTH_MASK, ((kal_uint16)i << CS_VTH_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_chr_current++;
	}
	// Unexpexted path
	//ASSERT(0);

}

// General API name, set charge current
void upmu_chr_current(upmu_chr_list_enum chr, upmu_chr_current_enum current_val)
{
	upmu_cs_vth(chr, current_val);
}

upmu_chr_current_enum upmu_get_cs_vth(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;

	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	val = ((val & CS_VTH_MASK) >> CS_VTH_SHIFT);

	//ASSERT(val < p_upmu_entry->chr_current_list_num);

	return p_upmu_entry->chr_current_list[val];

}

upmu_chr_current_enum upmu_get_chr_cs_vth(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;

	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	val = ((val & CS_VTH_MASK) >> CS_VTH_SHIFT);

	//ASSERT(val < p_upmu_entry->chr_current_list_num);

	return p_upmu_entry->chr_current_list[val];

}

// General API name, get charge current
upmu_chr_current_enum upmu_get_chr_current(upmu_chr_list_enum chr)
{
	//return upmu_get_cs_vth(chr);
	return upmu_get_chr_cs_vth(chr);
}

// Get the register index mapped to specific charge current (Passed-in parameter)
kal_uint32 upmu_get_cs_vth_idx(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;

	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	val = ((val & CS_VTH_MASK) >> CS_VTH_SHIFT);

	//ASSERT(val < p_upmu_entry->chr_current_list_num);

	return val;
}

kal_uint32 upmu_get_chr_cs_vth_idx(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;

	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	val = ((val & CS_VTH_MASK) >> CS_VTH_SHIFT);

	//ASSERT(val < p_upmu_entry->chr_current_list_num);

	return val;
}

// Get the register index mapped to specific charge current (Passed-in parameter)
kal_uint32 upmu_get_chr_current_idx(upmu_chr_list_enum chr)
{
	//return upmu_get_cs_vth_idx(chr);
	return upmu_get_chr_cs_vth_idx(chr);
}

void upmu_cs_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON2_OFFSET),
	                    CS_EN_MASK, ((kal_uint16)enable << CS_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#if 0
extern upmu_chr_current_enum upmu_cust_ret_current(pmic_adpt_chr_type chr_type);
upmu_chr_current_enum upmu_get_custom_charge_current(pmic_adpt_chr_type chr_type)
{
	return upmu_cust_ret_current(chr_type);
}
#endif

#if defined(__DRV_OTG_BVALID_DET_AT_CON2_BIT14__)
kal_bool upmu_otg_bvalid_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & OTG_BVALID_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}

	return KAL_FALSE;
}

kal_bool upmu_is_otg_bvalid_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & OTG_BVALID_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}

	return KAL_FALSE;
}

#endif //#if defined(__DRV_OTG_BVALID_DET_AT_CON2_BIT14__)

kal_bool upmu_cs_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & CS_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_chr_cs_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON2_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & CS_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

// CHR_CON3
void upmu_csdac_stp(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    CSDAC_STP_MASK, ((kal_uint16)val << CSDAC_STP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT3__)
void upmu_vbat_ov_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    VBAT_OV_EN_MASK, ((kal_uint16)enable << VBAT_OV_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT3__)

void upmu_csdac_dly(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    CSDAC_DLY_MASK, ((kal_uint16)val << CSDAC_DLY_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_ov_vth(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    VBAT_OV_VTH_MASK, ((kal_uint16)val << VBAT_OV_VTH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//__DRV_VBAT_OV_VTH_CON3_2BIT_WIDTH__
void upmu_vbat_ov_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint32 i;
	kal_uint32 chr_vbat_ov_list_num;
	upmu_chr_vol_enum *p_voltage;
	//kal_uint32 savedMask;

	// Read value once here
	chr_vbat_ov_list_num = p_upmu_entry->chr_vbat_ov_list_num;

	// Use pointer operation to speed up
	p_voltage = &p_upmu_entry->chr_vbat_ov_list[0];

	for (i=0;i<chr_vbat_ov_list_num;i++)
	{
		if (*p_voltage == vol)
		{
			//savedMask = PMU_SaveAndSetIRQMask();
			PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
			                    VBAT_OV_VTH_MASK, ((kal_uint16)i << VBAT_OV_VTH_SHIFT));
			//PMU_RestoreIRQMask(savedMask);
			return;
		}
		p_voltage++;
	}
	// Unexpexted path
	//ASSERT(0);

}

#if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT8__)
void upmu_vbat_ov_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    VBAT_OV_EN_MASK, ((kal_uint16)enable << VBAT_OV_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_VBAT_OV_EN_AT_CON3_BIT8__)

#if defined(__DRV_BATON_EN_AT_CON3_BIT9__)
void upmu_bat_on_det_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    BATON_EN_MASK, ((kal_uint16)enable << BATON_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_BATON_EN_AT_CON3_BIT9__)

#if defined(__DRV_BATON_EN_AT_CON3_BIT12__)
void upmu_bat_on_det_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    BATON_EN_MASK, ((kal_uint16)enable << BATON_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_BATON_EN_AT_CON3_BIT12__)


#if defined(__DRV_OTG_BVALID_EN_AT_CON3_BIT13__)
void upmu_otg_bvalid_det_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    OTG_BVALID_EN_MASK, ((kal_uint16)enable << OTG_BVALID_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_OTG_BVALID_EN_AT_CON3_BIT13__)

kal_bool upmu_vbat_ov(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VBAT_OV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_bat_on(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & BATON_UNDET_MASK) != 0)
	{
		// BATON_UNDET is 0 ==> Battery is attached
		// BATON_UNDET is 1 ==> Battery is NOT attached
		return KAL_FALSE;
	}
	return KAL_TRUE;
}

kal_bool upmu_is_vbat_ov(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & VBAT_OV_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}
	return KAL_FALSE;
}

kal_bool upmu_is_bat_on(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON3_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & BATON_UNDET_MASK) != 0)
	{
		// BATON_UNDET is 0 ==> Battery is attached
		// BATON_UNDET is 1 ==> Battery is NOT attached
		return KAL_FALSE;
	}
	return KAL_TRUE;
}

//AddForMT6573
void upmu_baton_ht_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    BATON_HTEN_MASK, ((kal_uint16)enable << BATON_HTEN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_vbat_ov_deg_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON3_OFFSET),
	                    VBAT_OV_DEG_MASK, ((kal_uint16)enable << VBAT_OV_DEG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// CHR_CON4
void upmu_pchr_test_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON4_OFFSET),
	                    PCHR_TEST_MASK, ((kal_uint16)enable << PCHR_TEST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_pchr_csdac_test_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON4_OFFSET),
	                    PCHR_CSDAC_TEST_MASK, ((kal_uint16)enable << PCHR_CSDAC_TEST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_pchr_test_reset(upmu_chr_list_enum chr, kal_bool reset)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON4_OFFSET),
	                    PCHR_RST_MASK, ((kal_uint16)reset << PCHR_RST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_pchr_test_csdac_dat(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON4_OFFSET),
	                    CSDAC_DAT_MASK, ((kal_uint16)val << CSDAC_DAT_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// CHR_CON5
void upmu_pchr_flag_sel(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON5_OFFSET),
	                    PCHR_FLAG_SEL_MASK, ((kal_uint16)val << PCHR_FLAG_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_pchr_flag_enable(upmu_chr_list_enum chr, kal_uint8 enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON5_OFFSET),
	                    PCHR_FLAG_EN_MASK, ((kal_uint16)enable << PCHR_FLAG_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_uint8 upmu_pchr_flag_out(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON5_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & PCHR_FLAG_OUT_MASK) >> PCHR_FLAG_OUT_SHIFT;

	return (kal_uint8)val;

}

kal_uint8 upmu_pchr_get_flag_out(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON5_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & PCHR_FLAG_OUT_MASK) >> PCHR_FLAG_OUT_SHIFT;

	return (kal_uint8)val;

}

#if defined(__DRV_OTG_BVALID_EN_AT_CON5_BIT12__)
void upmu_otg_bvalid_det_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON5_OFFSET),
	                    OTG_BVALID_EN_MASK, ((kal_uint16)enable << OTG_BVALID_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_OTG_BVALID_EN_AT_CON5_BIT12__)

#if defined(__DRV_OTG_BVALID_DET_AT_CON5_BIT15__)
kal_bool upmu_otg_bvalid_det(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON5_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & OTG_BVALID_DET_MASK) != 0)
	{
		return KAL_TRUE;
	}

	return KAL_FALSE;
}
#endif // #if defined(__DRV_OTG_BVALID_DET_AT_CON5_BIT15__)

//AddForMT6573
void upmu_pchr_ft_ctrl(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON5_OFFSET),
	                    PCHR_FT_CTRL_MASK, ((kal_uint16)val << PCHR_FT_CTRL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// CHR_CON6
void upmu_chrwdt_td(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON6_OFFSET),
	                    CHRWDT_TD_MASK, ((kal_uint16)val << CHRWDT_TD_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_chrwdt_enable(upmu_chr_list_enum chr, kal_uint8 enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON6_OFFSET),
	                    CHRWDT_EN_MASK, ((kal_uint16)enable << CHRWDT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//kal_uint32 wait_tick = 2;
void upmu_chrwdt_clear(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;
	//kal_uint32 tmp1, tmp2;

	// To reset Charger WDT counter, just WRITE any bit of "CHR_CON6"
	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON6_OFFSET));
	PMU_DRV_WriteReg16((p_upmu_entry->addr+CHR_CON6_OFFSET), val);
	//PMU_RestoreIRQMask(savedMask);

	// Need to wait 2 ms here (At least)
	// Before H/W fix the bug, need to put delay here
	// Here we use delay 200 tick is a experiment value on MT6255
	#if 0
	tmp1 = drv_get_current_time();
	#endif // #if 0
	#if defined(__DRV_CHR_WDT_CLEAR_WAIT_2MS__)
	{
		kal_uint32 tmp;
		tmp = drv_get_current_time();
		do{
			__nop();
		}while (200 > drv_get_duration_tick(tmp, drv_get_current_time()));
	}
	#endif // #if defined(__DRV_CHR_WDT_CLEAR_WAIT_2MS__)
	#if 0
	tmp2 = drv_get_current_time();

	if ((tmp2 - tmp1) > 10)
	{
		upmu_pchr_test_csdac_dat(CHR, (kal_uint8)tmp1);
		wait_tick = 3;
	}
	else
	{
		upmu_pchr_test_csdac_dat(CHR, (kal_uint8)tmp2);
		wait_tick = 4;
	}
	#endif // #if 0


	// CHRWDT_FLAG bit will gate CHR_EN, the way to clear CHRWDT_FLAG is to
	//    1. Write CHR_CON6
	//    2. Write CHR_CON7[1] (CHRWDT_FLAG bit)
	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON7_OFFSET));
	val |= CHRWDT_FLAG_WR_MASK;
	PMU_DRV_WriteReg16((p_upmu_entry->addr+CHR_CON7_OFFSET), val);
	//PMU_RestoreIRQMask(savedMask);
}

void upmu_chrwdt_clear_internal(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;

	// To reset Charger WDT counter, just WRITE any bit of "CHR_CON6"
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON6_OFFSET));
	PMU_DRV_WriteReg16((p_upmu_entry->addr+CHR_CON6_OFFSET), val);

	// Need to wait 2 ms here (At least)
	// Before H/W fix the bug, need to put delay here
	// Here we use delay 200 tick is a experiment value on MT6255
	#if 0
	tmp1 = drv_get_current_time();
	#endif // #if 0
	#if defined(__DRV_CHR_WDT_CLEAR_WAIT_2MS__)
	{
		kal_uint32 tmp;
		tmp = drv_get_current_time();
		do{
			__nop();
		}while (200 > drv_get_duration_tick(tmp, drv_get_current_time()));
	}
	#endif // #if defined(__DRV_CHR_WDT_CLEAR_WAIT_2MS__)
	#if 0
	tmp2 = drv_get_current_time();

	if ((tmp2 - tmp1) > 10)
	{
		upmu_pchr_test_csdac_dat(CHR, (kal_uint8)tmp1);
		wait_tick = 3;
	}
	else
	{
		upmu_pchr_test_csdac_dat(CHR, (kal_uint8)tmp2);
		wait_tick = 4;
	}
	#endif // #if 0

	// CHRWDT_FLAG bit will gate CHR_EN, the way to clear CHRWDT_FLAG is to
	//    1. Write CHR_CON6
	//    2. Write CHR_CON7[1] (CHRWDT_FLAG bit)
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON7_OFFSET));
	val |= CHRWDT_FLAG_WR_MASK;
	PMU_DRV_WriteReg16((p_upmu_entry->addr+CHR_CON7_OFFSET), val);

}


// CHR_CON7
void upmu_chrwdt_int_enable(upmu_chr_list_enum chr, kal_uint8 enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON7_OFFSET),
	                    CHRWDT_INT_EN_MASK, ((kal_uint16)enable << CHRWDT_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_chrwdt_flag(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON7_OFFSET),
	                    CHRWDT_FLAG_WR_MASK, ((kal_uint16)val << CHRWDT_FLAG_WR_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_bool upmu_chrwdt_timeout(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON7_OFFSET));
	//PMU_RestoreIRQMask(savedMask);
	if ((val & CHRWDT_OUT_MASK) != 0)
	{
		return KAL_TRUE;
	}

	return KAL_FALSE;
}

// CHR_CON8
void upmu_adc_measure_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val = 0;
	//kal_uint32 savedMask;

	if (KAL_TRUE == enable)
	{
		val = ADC_EN_MASK;
	}
	else
	{
		val = 0;
	}

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    ADC_EN_MASK, val);

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_adc_measure_vbat_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    ADCIN_VBAT_EN_MASK, ((kal_uint16)enable << ADCIN_VBAT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_adc_measure_vsen_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    ADCIN_VSEN_EN_MASK, ((kal_uint16)enable << ADCIN_VSEN_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_adc_measure_vchr_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    ADCIN_CHR_EN_MASK, ((kal_uint16)enable << ADCIN_CHR_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_bgr_rsel(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    BGR_RSEL_MASK, ((kal_uint16)val << BGR_RSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_bgr_unchop_ph(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    BGR_UNCHOP_PH_MASK, ((kal_uint16)val << BGR_UNCHOP_PH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_bgr_unchop(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    BGR_UNCHOP_MASK, ((kal_uint16)val << BGR_UNCHOP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_uvlo_vthl(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    UVLO_VTHL_MASK, ((kal_uint16)val << UVLO_VTHL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_usbdl_rst(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    USBDL_RST_MASK, ((kal_uint16)val << USBDL_RST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
void upmu_usbdl_set(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON8_OFFSET),
	                    USBDL_SET_MASK, ((kal_uint16)val << USBDL_SET_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

//AddForMT6573
#if defined(__DRV_UPMU_BC11_V2__)
// CHR_CON9 (CHR_BC11_CON0)
void upmu_bc11_vsrc_en_source(upmu_chr_list_enum chr, upmu_bc11_vsrc_en_enum val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON9_OFFSET),
	                    BC11_VSRC_EN_MASK, ((kal_uint16)val << BC11_VSRC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_bc11_reset_circuit(upmu_chr_list_enum chr, kal_uint8 val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON9_OFFSET),
	                    BC11_RST_MASK, ((kal_uint16)val << BC11_RST_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// CHR_CON10 (CHR_BC11_CON1)
void upmu_bc11_cmp_enable(upmu_chr_list_enum chr, upmu_bc11_cmp_en_enum val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON10_OFFSET),
	                    BC11_CMP_EN_MASK, ((kal_uint16)val << BC11_CMP_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_bc11_ipd_enable(upmu_chr_list_enum chr, upmu_bc11_ipd_en_enum val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON10_OFFSET),
	                    BC11_IPD_EN_MASK, ((kal_uint16)val << BC11_IPD_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_bc11_ipu_enable(upmu_chr_list_enum chr, upmu_bc11_ipu_en_enum val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON10_OFFSET),
	                    BC11_IPU_EN_MASK, ((kal_uint16)val << BC11_IPU_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_bc11_vref_vth(upmu_chr_list_enum chr, upmu_bc11_vref_vth_enum val)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON10_OFFSET),
	                    BC11_VREF_VTH_MASK, ((kal_uint16)val << BC11_VREF_VTH_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_bc11_bias_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON10_OFFSET),
	                    BC11_BIAS_EN_MASK, ((kal_uint16)enable << BC11_BIAS_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_bc11_bb_crtl_enable(upmu_chr_list_enum chr, kal_bool enable)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+CHR_CON10_OFFSET),
	                    BC11_BB_CTRL_MASK, ((kal_uint16)enable << BC11_BB_CTRL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_uint8 upmu_get_bc11_cmp_out(upmu_chr_list_enum chr)
{
	upmu_chr_profile_entry *p_upmu_entry = &upmu_chr_profile[(kal_uint16)chr];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+CHR_CON10_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & BC11_CMP_OUT_MASK) >> BC11_CMP_OUT_SHIFT;

	return (kal_uint8)val;

}

#endif // #if defined(__DRV_UPMU_BC11_V2__)

#endif // #if defined(__DRV_UPMU_CHARGER_V1__)


#if defined(__DRV_UPMU_OC_V1__)
// PMIC_OC_CON0
void upmu_vrf_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VRF_OC_INT_EN_MASK, ((kal_uint16)enable << VRF_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vcama_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VCAMA_OC_INT_EN_MASK, ((kal_uint16)enable << VCAMA_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vcamd_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VCAMD_OC_INT_EN_MASK, ((kal_uint16)enable << VCAMD_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vio_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VIO_OC_INT_EN_MASK, ((kal_uint16)enable << VIO_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vusb_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VUSB_OC_INT_EN_MASK, ((kal_uint16)enable << VUSB_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vsim_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VSIM_OC_INT_EN_MASK, ((kal_uint16)enable << VSIM_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vsim2_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VSIM2_OC_INT_EN_MASK, ((kal_uint16)enable << VSIM2_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vibr_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VIBR_OC_INT_EN_MASK, ((kal_uint16)enable << VIBR_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vmc_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VMC_OC_INT_EN_MASK, ((kal_uint16)enable << VMC_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vcama2_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VCAMA2_OC_INT_EN_MASK, ((kal_uint16)enable << VCAMA2_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vcamd2_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON0_OFFSET),
	                    VCAMD2_OC_INT_EN_MASK, ((kal_uint16)enable << VCAMD2_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// PMIC_OC_CON1
void upmu_vm12_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON1_OFFSET),
	                    VM12_OC_INT_EN_MASK, ((kal_uint16)enable << VM12_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vm12_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON1_OFFSET),
	                    VM12_INT_OC_INT_EN_MASK, ((kal_uint16)enable << VM12_INT_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// PMIC_OC_CON2
void upmu_vcore_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON2_OFFSET),
	                    VCORE_OC_INT_EN_MASK, ((kal_uint16)enable << VCORE_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vio1v8_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON2_OFFSET),
	                    VIO1V8_OC_INT_EN_MASK, ((kal_uint16)enable << VIO1V8_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vaproc_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON2_OFFSET),
	                    VAPROC_OC_INT_EN_MASK, ((kal_uint16)enable << VAPROC_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_vrf1v8_int_oc_int_enable(upmu_oc_list_enum oc, kal_bool enable)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON2_OFFSET),
	                    VRF1V8_OC_INT_EN_MASK, ((kal_uint16)enable << VRF1V8_OC_INT_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// PMIC_OC_CON4
kal_uint8 upmu_get_vrf_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VRF_OC_FLAG_MASK) >> VRF_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vcama_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VCAMA_OC_FLAG_MASK) >> VCAMA_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vcamd_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VCAMD_OC_FLAG_MASK) >> VCAMD_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vio_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VIO_OC_FLAG_MASK) >> VIO_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vusb_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VUSB_OC_FLAG_MASK) >> VUSB_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vsim_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VSIM_OC_FLAG_MASK) >> VSIM_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_sim2_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VSIM2_OC_FLAG_MASK) >> VSIM2_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vibr_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VBIR_OC_FLAG_MASK) >> VBIR_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vmc_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VMC_OC_FLAG_MASK) >> VMC_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vcama2_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VCAMA2_OC_FLAG_MASK) >> VCAMA2_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vcamd2_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON4_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VCAMD2_OC_FLAG_MASK) >> VCAMD2_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

void upmu_clear_vrf_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VRF_OC_FLAG_MASK, (1 << VRF_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vcama_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VCAMA_OC_FLAG_MASK, (1 << VCAMA_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vcamd_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VCAMD_OC_FLAG_MASK, (1 << VCAMD_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vio_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VIO_OC_FLAG_MASK, (1 << VIO_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vusb_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VUSB_OC_FLAG_MASK, (1 << VUSB_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vsim_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VSIM_OC_FLAG_MASK, (1 << VSIM_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_sim2_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VSIM2_OC_FLAG_MASK, (1 << VSIM2_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vibr_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VBIR_OC_FLAG_MASK, (1 << VBIR_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vmc_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VMC_OC_FLAG_MASK, (1 << VMC_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vcama2_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VCAMA2_OC_FLAG_MASK, (1 << VCAMA2_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vcamd2_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON4_OFFSET),
	                    VCAMD2_OC_FLAG_MASK, (1 << VCAMD2_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// PMIC_OC_CON5
kal_uint8 upmu_get_vm12_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON5_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VM12_OC_FLAG_MASK) >> VM12_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vm12_int_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON5_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VM12_INT_OC_FLAG_MASK) >> VM12_INT_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

void upmu_clear_vm12_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON5_OFFSET),
	                    VM12_OC_FLAG_MASK, (1 << VM12_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vm12_int_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON5_OFFSET),
	                    VM12_INT_OC_FLAG_MASK, (1 << VM12_INT_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// PMIC_OC_CON6
kal_uint8 upmu_get_vcore_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON6_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VCORE_OC_FLAG_MASK) >> VCORE_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vio1v8_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON6_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VIO1V8_OC_FLAG_MASK) >> VIO1V8_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vaproc_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON6_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VAPROC_OC_FLAG_MASK) >> VAPROC_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_vrf1v8_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+OC_CON6_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & VRF1V8_OC_FLAG_MASK) >> VRF1V8_OC_FLAG_SHIFT;

	return (kal_uint8)val;
}

void upmu_clear_vcore_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON6_OFFSET),
	                    VCORE_OC_FLAG_MASK, (1 << VCORE_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vio1v8_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON6_OFFSET),
	                    VIO1V8_OC_FLAG_MASK, (1 << VIO1V8_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_clear_vaproc_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON6_OFFSET),
	                    VAPROC_OC_FLAG_MASK, (1 << VAPROC_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}


void upmu_clear_vrf1v8_oc_flag(upmu_oc_list_enum oc)
{
	upmu_oc_profile_entry *p_upmu_entry = &upmu_oc_profile[(kal_uint16)oc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+OC_CON6_OFFSET),
	                    VRF1V8_OC_FLAG_MASK, (1 << VRF1V8_OC_FLAG_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#endif // #if defined(__DRV_UPMU_OC_V1__)


#if defined(__DRV_UPMU_STRUP_V1__)
// STRUP_CON0
void upmu_strup_iref_adj(upmu_strup_list_enum strup, upmu_strup_bias_current_adjust_enum val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON0_OFFSET),
	                    STRUP_IREF_ADJ_MASK, ((kal_uint16)val << STRUP_IREF_ADJ_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_strup_thr_sel(upmu_strup_list_enum strup, upmu_strup_thermal_shutdown_threshold_fine_tune_enum val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON0_OFFSET),
	                    STRUP_THR_SEL_MASK, ((kal_uint16)val << STRUP_THR_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

kal_uint8 upmu_get_strup_test_mode_status(upmu_strup_list_enum strup)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+STRUP_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & TEST_MODE_POR_MASK) >> TEST_MODE_POR_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_strup_pwrkey_vcore_status(upmu_strup_list_enum strup)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+STRUP_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & PWRKEY_VCORE_MASK) >> PWRKEY_VCORE_SHIFT;

	return (kal_uint8)val;
}

kal_uint8 upmu_get_strup_pwrkey_deb_status(upmu_strup_list_enum strup)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	kal_uint16 val;
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();
	val = PMU_DRV_ReadReg16((p_upmu_entry->addr+STRUP_CON0_OFFSET));
	//PMU_RestoreIRQMask(savedMask);

	val = (val & PWRKEY_DEB_MASK) >> PWRKEY_DEB_SHIFT;

	return (kal_uint8)val;
}

// STRUP_CON1
void upmu_strup_dig_test_en(upmu_strup_list_enum strup, kal_uint8 val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON1_OFFSET),
	                    DIG_TEST_EN_MASK, ((kal_uint16)val << DIG_TEST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_strup_rst_drvsel(upmu_strup_list_enum strup, kal_uint8 val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON1_OFFSET),
	                    RST_DRVSEL_MASK, ((kal_uint16)val << RST_DRVSEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_strup_pmu_lev_ungate(upmu_strup_list_enum strup, kal_uint8 val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON1_OFFSET),
	                    PMU_LEV_UNGATE_MASK, ((kal_uint16)val << PMU_LEV_UNGATE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_strup_flag_en(upmu_strup_list_enum strup, kal_uint8 val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON1_OFFSET),
	                    STRUP_FLAG_EN_MASK, ((kal_uint16)val << STRUP_FLAG_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_strup_bias_gen_force(upmu_strup_list_enum strup, kal_uint8 val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON1_OFFSET),
	                    BIAS_GEN_FORCE_MASK, ((kal_uint16)val << BIAS_GEN_FORCE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// STRUP_CON3
void upmu_strup_flag_sel(upmu_strup_list_enum strup, kal_uint8 val)
{
	upmu_strup_profile_entry *p_upmu_entry = &upmu_strup_profile[(kal_uint16)strup];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+STRUP_CON3_OFFSET),
	                    STRUP_FLAG_SEL_MASK, ((kal_uint16)val << STRUP_FLAG_SEL_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// STRUP_CON4
// reserved

#endif // #if defined(__DRV_UPMU_STRUP_V1__)


#if defined(__DRV_UPMU_LPOSC_V1__)
// LPOSC_CON0
void upmu_lposc_i_bias_cali(upmu_lposc_list_enum lposc, upmu_lposc_current_bias_calibration_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON0_OFFSET),
	                    LPOSC_IBIAS_CALI_MASK, ((kal_uint16)val << LPOSC_IBIAS_CALI_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_digtal_circuit_enable(upmu_lposc_list_enum lposc, kal_bool enable)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON0_OFFSET),
	                    LPOSC_EN_MASK, ((kal_uint16)enable << LPOSC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LPOSC_CON1
void upmu_lposc_output_freq_set(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON1_OFFSET),
	                    LPOSC_FREQ_SET_MASK, ((kal_uint16)val << LPOSC_FREQ_SET_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_output_freq_switching(upmu_lposc_list_enum lposc, upmu_lposc_buck_output_freq_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON1_OFFSET),
	                    LPOSC_BUCK_FREQ_MASK, ((kal_uint16)val << LPOSC_BUCK_FREQ_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_boost_freq_divider(upmu_lposc_list_enum lposc, upmu_lposc_buck_boost_output_freq_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON1_OFFSET),
	                    LPOSC_BBCLK_DIB_MASK, ((kal_uint16)val << LPOSC_BBCLK_DIB_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_auto_calibration_enable(upmu_lposc_list_enum lposc, kal_bool enable)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON1_OFFSET),
	                    LPOSC_ACALI_EN_MASK, ((kal_uint16)enable << LPOSC_ACALI_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LPOSC_CON2
void upmu_lposc_fd_resolution_adjust(upmu_lposc_list_enum lposc, upmu_lposc_fd_resolution_adjust_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_FD_RES_MASK, ((kal_uint16)val << LPOSC_FD_RES_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_fd_disable_duration_1_adjust(upmu_lposc_list_enum lposc, upmu_lposc_fd_dis_dur1_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_FD_DIS_DUR1_MASK, ((kal_uint16)val << LPOSC_FD_DIS_DUR1_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_fd_disable_duration_2_adjust(upmu_lposc_list_enum lposc, upmu_lposc_fd_dis_dur2_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_FD_DIS_DUR2_MASK, ((kal_uint16)val << LPOSC_FD_DIS_DUR2_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_ssc_mod_amp(upmu_lposc_list_enum lposc, upmu_lposc_ssc_mod_amp_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_SSC_MOD_AMP_MASK, ((kal_uint16)val << LPOSC_SSC_MOD_AMP_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_boost_enable(upmu_lposc_list_enum lposc, kal_bool enable)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_BUCK_BOOST_EN_MASK, ((kal_uint16)enable << LPOSC_BUCK_BOOST_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_ssc_code_duration(upmu_lposc_list_enum lposc, upmu_lposc_ssc_code_dur_enum val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_SSC_CODE_DUR_MASK, ((kal_uint16)val << LPOSC_SSC_CODE_DUR_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_phase_generation_block_enable(upmu_lposc_list_enum lposc, kal_bool enable)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON2_OFFSET),
	                    LPOSC_PG_EN_MASK, ((kal_uint16)enable << LPOSC_PG_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LPOSC_CON3
void upmu_lposc_buck_4_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON3_OFFSET),
	                    LPOSC_BUCK4_PS_MASK, ((kal_uint16)val << LPOSC_BUCK4_PS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_3_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON3_OFFSET),
	                    LPOSC_BUCK3_PS_MASK, ((kal_uint16)val << LPOSC_BUCK3_PS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_2_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON3_OFFSET),
	                    LPOSC_BUCK2_PS_MASK, ((kal_uint16)val << LPOSC_BUCK2_PS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_1_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON3_OFFSET),
	                    LPOSC_BUCK1_PS_MASK, ((kal_uint16)val << LPOSC_BUCK1_PS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

// LPOSC_CON4
void upmu_lposc_buck_6_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON4_OFFSET),
	                    LPOSC_BUCK6_PS_MASK, ((kal_uint16)val << LPOSC_BUCK6_PS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_buck_5_phase_selection(upmu_lposc_list_enum lposc, kal_uint8 val)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON4_OFFSET),
	                    LPOSC_BUCK5_PS_MASK, ((kal_uint16)val << LPOSC_BUCK5_PS_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

void upmu_lposc_init_dac_enable(upmu_lposc_list_enum lposc, kal_bool enable)
{
	upmu_lposc_profile_entry *p_upmu_entry = &upmu_lposc_profile[(kal_uint16)lposc];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+LPOSC_CON4_OFFSET),
	                    LPOSC_INIT_DAC_EN_MASK, ((kal_uint16)enable << LPOSC_INIT_DAC_EN_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

#endif // #if defined(__DRV_UPMU_LPOSC_V1__)


#if defined(__DRV_UPMU_Retention_V1__)
void upmu_data_retention(upmu_retention_list_enum ret, kal_uint8 val)
{
	upmu_retention_profile_entry *p_upmu_entry = &upmu_retention_profile[(kal_uint16)ret];
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16((p_upmu_entry->addr+RETENTION_CON0_OFFSET),
	                    DATA_RETENTION_MASK, ((kal_uint16)val << DATA_RETENTION_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}
#endif // #if defined(__DRV_UPMU_Retention_V1__)

//AddForMT6573_2
void upmu_lpocs_sw_mode(kal_bool enable)
{
	//kal_uint32 savedMask;

	//savedMask = PMU_SaveAndSetIRQMask();

	PMU_DRV_SetData16(0xF702FE88,
	                    LPOSC_SW_MODE_MASK, ((kal_uint16)enable << LPOSC_SW_MODE_SHIFT));

	//PMU_RestoreIRQMask(savedMask);
}

