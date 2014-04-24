/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver's
* applicable license agreements with MediaTek Inc.
*/

//#include <common.h>
#include <mt6573.h>
#include "mt6573_typedefs.h"
#include <mt6573_i2c.h>
#include <mt6573_pmic6326_hw.h>
#include <mt6573_pmic6326_sw.h>

/**********************************************************
*
*   [PMIC Customer Define] 
*
*********************************************************/
#define VRF_CAL                 0       /* 0 ~ 15 */
#define VTCXO_CAL               0       /* 0 ~ 15 */
#define V3GTX_CAL               0       /* 0 ~ 15 */
#define V3GTX_SEL               V3GTX_2_8       /* V3GTX_2_8/3_0/3_3/2_5 */
#define V3GRX_CAL               0       /* 0 ~ 15 */
#define V3GRX_SEL               V3GRX_2_8       /* V3GRX_2_8/3_0/3_3/2_5 */
#define VCAMA_CAL               0       /* 0 ~ 15 */
#define VCAMA_SEL               VCAMA_2_8       /* VCAMA_2_8/2_5/1_8/1_5 */
#define VWIFI3V3_CAL            0       /* 0 ~ 15 */
#define VWIFI3V3_SEL            VWIFI3V3_2_8    /* VWIFI3V3_2_8/3_0/3_3/2_5 */
#define VWIFI2V8_CAL            0       /* 0 ~ 15 */
#define VWIFI2V8_SEL            VWIFI2V8_2_8    /* VWIFI2V8_2_8/3_0/3_3/2_5 */
#define VSIM_CAL                0       /* 0 ~ 15 */
#define VBT_CAL                 0       /* 0 ~ 15 */
#define VBT_SEL                 VBT_1_3 /* VBT_1_3/1_5/1_8/2_5/2_8/3_0/3_3/1_2 */
#define VCAMD_CAL               0       /* 0 ~ 15 */
#define VCAMD_SEL               VCAMD_1_3       /* VCAMD_1_3/1_5/1_8/2_5/2_8/3_0/3_3/1_2 */
#define VGP_CAL                 0       /* 0 ~ 15 */
#define VGP_SEL                 VGP_1_3 /* VGP_1_3/1_5/1_8/2_5/2_8/3_0/3_3 */
#define VSDIO_CAL               0       /* 0 ~ 15 */
#define VSDIO_SEL               VSDIO_2_8       /* VSDIO_2_8/3_0 */
#define VGP2_SEL                VGP2_1_3        /* VGP2_1_3/1_5/1_8/2_5/2_8/3_0/3_3 */
#define VGP2_ON_SEL             VGP2_ENABLE_WITH_VGP2_EN        /* VGP2_ENABLE_WITH_SRCLKEN */
#define CHR_OFFSET              CHR_CURRENT_OFFSET_NO   /* CHR_CURRENT_OFFSET_PLUS_1_STEP/CHR_CURRENT_OFFSET_PLUS_2_STEP/
CHR_CURRENT_OFFSET_MINUS_2_STEP/CHR_CURRENT_OFFSET_MINUS_1_STEP */
#define AC_CHR_CURRENT  CHR_CURRENT_650MA       /* CHR_CURRENT_50MA/90MA/150MA/225MA/300MA/450MA */
#define USB_CHR_CURRENT CHR_CURRENT_450MA       /* CHR_CURRENT_50MA/90MA/150MA/225MA/300MA */
#define BOOST_MODE      BOOST_MODE_TYPE_I       /* BOOST_MODE_TYPE_II/BOOST_MODE_TYPE_III/BOOST_MODE_TYPE_IV */
#define BL_NUM          BL_NUM_4        /* BL_NUM_1/2/3/4/5/6/7/8 */
#define ASW_ASEL        ASW_ASEL_ISINK_6_8_AS   /* ASW_ASEL_ALL_ISINK_BL */
#define VBOOST1_TUNE    VBOOST1_VOL_3_20_V      /* 3_35_V/3_50_V/3_50_V/3_65_V3_80_V/3_95_V/4_10_V/4_25_V/4_40_V/
4_55_V/4_70_V/4_85_V/5_00_V/5_15_V/5_30_V/5_45_V */
#define BL_I_COARSE_TUNE        BL_I_CORSE_TUNE_20MA    /* 4MA/8MA/12MA/16MA/20MA/24MA/28MA/32MA */
#define ASW_BSEL                HI_Z    /* RECEIVER/TWO_OF_RGB_DRIVER */
#define VCORE2_ON_SEL           VCORE2_ENABLE_WITH_EN_PASS      /* VCORE2_ENABLE_WITH_VCORE2_EN */
#define VCORE2_EN               KAL_TRUE        /* KAL_FALSE */
#define USE_SPKL                KAL_TRUE        /* KAL_FALSE */
#define USE_SPKR                KAL_TRUE        /* KAL_FALSE */

#define PMIC6326_E4_CID_CODE        0x000B
#define PMIC6326_ECO_4_VERSION      0x04

#if defined(CFG_MMC_BOOT)
#define MT6516_PMIC_DBG_LOG     0
#else
#define MT6516_PMIC_DBG_LOG     0
#endif


#if MT6516_PMIC_DBG_LOG
#define PMIC_BUG() do { \
    dbg_print("BUG at %s:%d!\n", __FILE__, __LINE__); \
    } while (0)
#define PMIC_BUG_ON(condition) do { if ((condition)==FALSE) BUG(); } while(0)
#else
#define PMIC_BUG() do {} while (0)
#define PMIC_BUG_ON(condition) do {} while(0)
#define dbg_print(a,...)
#endif

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


#define MT6516_MOD_PMIC     "mt6516_pmic:"
/**********************************************************
*
*   [PMIC Variable] 
*
*********************************************************/
kal_uint8 pmic6326_eco_version = 0;
kal_uint8 pmic6326_reg[PMIC_REG_NUM] = { 0 };

// BL, Flash, OTG LDOs need higher voltage output, we also need to enable vboost1 when enabling the 3 LDOs
#define VBOOST1_SET_FLAG_BL         0x01    // Bit00
#define VBOOST1_SET_FLAG_FLASH      0x02    // Bit01
#define VBOOST1_SET_FLAG_OTG            0x04    // Bit02
#define VBOOST1_SET_FLAG_BOOST1     0x08    // Bit03
kal_uint8 pmic6326_vboost1_set_flag = 0;
// Internal usage only, called insided PMIC6326 driver
// Do NOT need protection check, because it is called inside PMIC API
#define pmic_boost1_enable_internal(flag)       {\
    kal_uint8 enable;\
    enable = (flag == 0)?0:1;\
    pmic6326_reg[0x5D] &= ~(BOOST1_EN_MASK << BOOST1_EN_SHIFT);\
    pmic6326_reg[0x5D] |= (enable << BOOST1_EN_SHIFT);\
    mt6516_i2c_write_byte(0x5D, pmic6326_reg[0x5D]);\
}

// FLASH, Keypad LED, Vibrator LDOs need to control DIM, so we need to turn on 0x72 when any of them are turned on
#define DIM_CK_ON_FLAG_CK               0x01    // Bit00
#define DIM_CK_ON_FLAG_FLASH            0x02    // Bit01
#define DIM_CK_ON_FLAG_KEY              0x04    // Bit02
#define DIM_CK_ON_FLAG_VIB              0x08    // Bit03

/**********************************************************
*
*   [I2C Function For Read/Write PMIC] 
*
*       i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
*       i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
*
*********************************************************/
// Jau comment these two function
// please call another two function instead mt6516_i2c_read_byte and mt6516_i2c_write_byte
#if 0
static kal_bool
mt6326_read_byte (u8 cmd, u8 * returnData)
{
    u8 readData = 0;
    int i = 0, ret = 0;

    ret = mt6516_i2c_write (0xc0, 0, 1, &cmd, 1);       // set register command
    if (ret != 0)
        return FALSE;

    ret = mt6516_i2c_read (0xc0, 0, 1, &readData, 1);

    *returnData = readData;

    return (ret == 0);
}

static kal_bool
mt6516_i2c_write_byte (u8 cmd, u8 writeData)
{
    u8 write_data[2];
    int ret = 0;

    write_data[0] = cmd;
    write_data[1] = writeData;

    ret = mt6516_i2c_write (0xc0, 0, 2, write_data, 2);

    return (ret == 0);
}
#endif
/**********************************************************
*
*   [PMIC Basic Function] 
*
*********************************************************/
typedef enum
{
    PMIC_INIT_CTRL_1 = 0x89,
    PMIC_INIT_CTRL_2 = 0x8A,
    PMIC_INIT_CTRL_3 = 0x8B
} PMIC_INIT_CTRL;

void pmic_int_ctrl_enable (PMIC_INIT_CTRL ctrl_type, int_ctrl_1_enum sel,  kal_bool enable)
{
    PMIC_BUG_ON ((kal_uint32) sel <= 0xFF);
    if (enable)
    {
        pmic6326_reg[ctrl_type] |= (kal_uint8) sel;
    }
    else
    {
        pmic6326_reg[ctrl_type] &= ~((kal_uint8) sel);
    }
    mt6516_i2c_write_byte (ctrl_type, pmic6326_reg[ctrl_type]);
}

void pmic_config_interface (kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT)
{
    pmic6326_reg[RegNum] &= ~(MASK << SHIFT);
    pmic6326_reg[RegNum] |= (val << SHIFT);
    mt6516_i2c_write_byte (RegNum, pmic6326_reg[RegNum]);
}

/* (0x3D) LDO CTRL 36 VBT */
void pmic_vbt_sel (vbt_sel_enum sel)
{
    kal_uint8 val;
    kal_uint16 reg_addr;
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3A;
        // For E3, the voltage changed, we need to re-map VBT voltage selction value of E1, E2 to E3
        if (sel == VBT_1_3)
        {
            val = (kal_uint8) VBT_E3_1_3;
        }
        else if (sel == VBT_1_5)
        {
            val = (kal_uint8) VBT_E3_1_5;
        }
        else if (sel == VBT_1_8)
        {
            val = (kal_uint8) VBT_E3_1_8;
        }
        else if (sel == VBT_2_5)
        {
            val = (kal_uint8) VBT_E3_2_5;
        }
        else if (sel == VBT_2_8)
        {
            val = (kal_uint8) VBT_E3_2_8;
        }
        else if (sel == VBT_3_0)
        {
            val = (kal_uint8) VBT_E3_3_0;
        }
        else if (sel == VBT_3_3)
        {
            val = (kal_uint8) VBT_E3_3_3;
        }
        else
        {
            PMIC_BUG_ON (0);        // E3 VBT does NOT support 1.2V
        }
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3D;
        val = (kal_uint8) sel;
    }
    PMIC_BUG_ON (val <= 7);
    pmic6326_reg[reg_addr] &= ~(VBT_SEL_MASK << VBT_SEL_SHIFT);
    pmic6326_reg[reg_addr] |= (val << VBT_SEL_SHIFT);
    mt6516_i2c_write_byte (reg_addr, pmic6326_reg[reg_addr]);
}

/**********************************************************
*
*   [Charging function] 
*
*********************************************************/

/**********************************************************
*
*   [Backlight Setting function] 
*
*********************************************************/
typedef enum
{
    PMIC_BL_CTRL_NUMBER = 0x6D,
    PMIC_BL_CTRL_ASW = 0x83,
    PMIC_BL_CTRL_VBOOST1 = 0x5C,
    PMIC_BL_CTRL_VBOOST1_EN = 0x5D,     // 20091020
    PMIC_BL_CTRL_igen_drv_force = 0x64, // 20091020
    PMIC_BL_CTRL_I_CORSE = 0x68,
    PMIC_BL_CTRL_BOOST2_DIM = 0x5F,
    PMIC_BL_CTRL_BOOST2_MOD = 0x61,
    PMIC_BL_CTRL_BOOST2_EN = 0x60,
    PMIC_BL_CTRL_DIM_DUTY = 0x67,

} PMIC_BL_CTRL;

void pmic_backlight_control (PMIC_BL_CTRL ctrl_type, int val, unsigned int mask, unsigned int shift)
{
    pmic6326_reg[ctrl_type] &= ~(mask << shift);
    pmic6326_reg[ctrl_type] |= ((kal_uint8) val << shift);
    mt6516_i2c_write_byte (ctrl_type, pmic6326_reg[ctrl_type]);
}

void pmic_bl_enable (kal_bool enable)
{
    if (enable)
    {
        pmic6326_vboost1_set_flag |= VBOOST1_SET_FLAG_BL;
    }
    else
    {
        pmic6326_vboost1_set_flag &= ~VBOOST1_SET_FLAG_BL;
    }
    pmic_boost1_enable_internal (pmic6326_vboost1_set_flag);

    pmic6326_reg[0x67] &= ~(BL_EN_MASK << BL_EN_SHIFT);
    pmic6326_reg[0x67] |= (enable << BL_EN_SHIFT);
    mt6516_i2c_write_byte (0x67, pmic6326_reg[0x67]);
}

void pmic_boost1_enable (kal_bool enable)
{
    if (enable)
    {
        pmic6326_vboost1_set_flag |= VBOOST1_SET_FLAG_BOOST1;
    }
    else
    {
        pmic6326_vboost1_set_flag &= ~(VBOOST1_SET_FLAG_BOOST1);
    }
    pmic_boost1_enable_internal (pmic6326_vboost1_set_flag);
}

void pmic_vbus_enable (kal_bool enable)
{

    pmic6326_reg[0x63] |= (enable << 0x0);

    mt6516_i2c_write_byte (0x63, pmic6326_reg[0x63]);
}

/**********************************************************
*
*   [PMIC Initialize function] 
*
*********************************************************/
void pmic_init (void)
{
    /* Get PMIC6326 ECO version */
    kal_uint16 eco_version = 0;
    kal_uint8 tmp8;
    kal_bool result_tmp;
    kal_uint32 i;
    kal_bool result;
    kal_uint16 reg_addr;

    /* First read will error, todo debug */
    //result_tmp = mt6516_i2c_read_byte(CID_1_REG_INDEX, &tmp8);

    /* Low part of CID */
    result_tmp = mt6516_i2c_read_byte (CID_1_REG_INDEX, &tmp8);
    PMIC_BUG_ON (result_tmp);
    eco_version |= tmp8;

    /* High part of CID */
    result_tmp = mt6516_i2c_read_byte (CID_2_REG_INDEX, &tmp8);
    PMIC_BUG_ON (result_tmp);
    eco_version |= (tmp8 << 8);
    if (eco_version == PMIC6326_E1_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_1_VERSION;
        dbg_print ("%s versin %d!\n", MT6516_MOD_PMIC, 1);
    }
    else if (eco_version == PMIC6326_E2_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_2_VERSION;
        dbg_print ("%s versin %d!\n", MT6516_MOD_PMIC, 2);
    }
    else if (eco_version == PMIC6326_E3_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_3_VERSION;
        dbg_print ("%s versin %d!\n", MT6516_MOD_PMIC, 3);
    }
    else if (eco_version == PMIC6326_E4_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_4_VERSION;
        dbg_print ("%s versin %d!\n", MT6516_MOD_PMIC, 4);
    }
    else
    {
        dbg_print ("%s versin unknown\n", MT6516_MOD_PMIC);
    }

    dbg_print ("%s int_ctrl_enable!\n", MT6516_MOD_PMIC);
    pmic_int_ctrl_enable (PMIC_INIT_CTRL_1, INT1_EN_ALL, KAL_FALSE);
    pmic_int_ctrl_enable (PMIC_INIT_CTRL_2, INT2_EN_ALL, KAL_TRUE);
    pmic_int_ctrl_enable (PMIC_INIT_CTRL_3, INT3_EN_ALL, KAL_FALSE);
    pmic_int_ctrl_enable (PMIC_INIT_CTRL_3, INT_EN_CHRDET | INT_EN_OVP, KAL_TRUE);

    dbg_print ("%s vrf, vtcxo and  v3gtx!\n", MT6516_MOD_PMIC);
    pmic_config_interface (0x1C, VRF_MAX_SLEW_RATE, VRF_CALST_MASK, VRF_CALST_SHIFT);
    pmic_config_interface (0x1F, VTCXO_MAX_SLEW_RATE, VTCXO_CALST_MASK,  VTCXO_CALST_SHIFT);
    pmic_config_interface (0x22, V3GTX_MAX_SLEW_RATE, V3GTX_CALST_MASK,  V3GTX_CALST_SHIFT);
    pmic_config_interface (0x25, V3GRX_MAX_SLEW_RATE, V3GRX_CALST_MASK,  V3GRX_CALST_SHIFT);

    dbg_print ("%s vpa_oc!\n", MT6516_MOD_PMIC);
    pmic_config_interface (0x5A, 3, VPA_OC_TH_MASK, VPA_OC_TH_SHIFT);

    /* PMIC WDT setting is at beginnging with compile option
    * speaker settings are moved to customization init
    * Because in customization init we decide the number of speaker usage */
    dbg_print ("%s boost1_sync_enable!\n", MT6516_MOD_PMIC);
    pmic_config_interface (0x5E, KAL_TRUE, BOOST1_SYNC_EN_MASK, BOOST1_SYNC_EN_SHIFT);

    dbg_print ("%s flash_i_tune!\n", MT6516_MOD_PMIC);
    pmic_config_interface (0x65, 0xF, FLASH_I_TUNE_MASK, FLASH_I_TUNE_SHIFT);   //pmic_flash_i_tune(0xF); 

    dbg_print ("%s vcore1!\n", MT6516_MOD_PMIC);
    pmic_config_interface (0x48, 1, VCORE1_DVFS_STEP_INC_MASK, VCORE1_DVFS_STEP_INC_SHIFT);     //pmic_vcore1_dvfs_step_inc(1);
    pmic_config_interface (0x57, 4, VCORE1_DVFS_1_ECO3_MASK, VCORE1_DVFS_1_ECO3_SHIFT); //pmic_vcore1_dvfs_1_eco3(4);     // 1.32v
    pmic_config_interface (0x57, 0, VCORE1_SLEEP_1_ECO3_MASK, VCORE1_SLEEP_1_ECO3_SHIFT);       //pmic_vcore1_sleep_1_eco3(0);
    pmic_config_interface (0x4E, 4, VCORE1_DVFS_0_ECO3_MASK, VCORE1_DVFS_0_ECO3_SHIFT); //pmic_vcore1_dvfs_0_eco3(4);
    pmic_config_interface (0x4F, 0, VCORE1_SLEEP_0_ECO3_MASK, VCORE1_SLEEP_0_ECO3_SHIFT);       //pmic_vcore1_sleep_0_eco3(0);
    pmic_config_interface (0x4F, KAL_TRUE, VCORE1_DVFS_RAMP_EN_MASK, VCORE1_DVFS_RAMP_EN_SHIFT);        //pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_config_interface (0x4F, KAL_FALSE, VCORE1_DVFS_TARGET_UPDATE_MASK, VCORE1_DVFS_TARGET_UPDATE_SHIFT);   //pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_config_interface (0x4F, KAL_TRUE, VCORE1_DVFS_TARGET_UPDATE_MASK, VCORE1_DVFS_TARGET_UPDATE_SHIFT);    //pmic_vcore1_dvfs_target_update(KAL_TRUE);

    /* Turn on all LDO */
    dbg_print ("%s LDO!\n", MT6516_MOD_PMIC);
    pmic_config_interface (0x1B, VRF_CAL, VRF_CAL_MASK, VRF_CAL_SHIFT); //pmic_vrf_cal(VRF_CAL);
    pmic_config_interface (0x1B, KAL_TRUE, VRF_EN_MASK, VRF_EN_SHIFT);  //pmic_vrf_enable(KAL_TRUE);

    pmic_config_interface (0x1E, VTCXO_CAL, VTCXO_CAL_MASK, VTCXO_CAL_SHIFT);   //pmic_vtcxo_cal(VTCXO_CAL); 
    pmic_config_interface (0x1E, KAL_TRUE, VTCXO_EN_MASK, VTCXO_EN_SHIFT);      //pmic_vtcxo_enable(KAL_TRUE);

#if 1                           // Kelvin, 20090806, remove un-necessary PMIC LDO
    pmic_config_interface (0x21, V3GTX_CAL, V3GTX_CAL_MASK, V3GTX_CAL_SHIFT);   //pmic_v3gtx_cal(V3GTX_CAL);
    pmic_config_interface (0x21, V3GTX_SEL, V3GTX_SEL_MASK, V3GTX_SEL_SHIFT);   //pmic_v3gtx_sel(V3GTX_SEL);    
    pmic_config_interface (0x22, KAL_TRUE, V3GTX_ON_SEL_MASK, V3GTX_ON_SEL_SHIFT);      //pmic_v3gtx_enable(KAL_TRUE);
    pmic_config_interface (0x22, KAL_FALSE, V3GTX_EN_MASK, V3GTX_EN_SHIFT);     //pmic_v3gtx_enable(KAL_TRUE);

    pmic_config_interface (0x24, V3GRX_CAL, V3GRX_CAL_MASK, V3GRX_CAL_SHIFT);   //pmic_v3grx_cal(V3GRX_CAL);
    pmic_config_interface (0x24, V3GRX_SEL, V3GRX_SEL_MASK, V3GRX_SEL_SHIFT);   //pmic_v3grx_sel(V3GRX_SEL);
    pmic_config_interface (0x25, KAL_TRUE, V3GRX_ON_SEL_MASK, V3GRX_ON_SEL_SHIFT);      //pmic_v3grx_enable(KAL_TRUE);
    pmic_config_interface (0x25, KAL_FALSE, V3GRX_EN_MASK, V3GRX_EN_SHIFT);     //pmic_v3grx_enable(KAL_TRUE);
#endif

    //    pmic_config_interface(0x2E, VCAMA_CAL, VCAMA_CAL_MASK, VCAMA_CAL_SHIFT);    //pmic_vcama_cal(VCAMA_CAL);
    //    pmic_config_interface(0x2E, VCAMA_SEL, VCAMA_SEL_MASK, VCAMA_SEL_SHIFT);    //pmic_vcama_sel(VCAMA_SEL);
    //    pmic_config_interface(0x2F, KAL_TRUE, VCAMA_EN_MASK, VCAMA_EN_SHIFT);    //pmic_vcama_enable(KAL_TRUE);

    //    pmic_config_interface(0x31, VWIFI3V3_CAL, VWIFI3V3_CAL_MASK, VWIFI3V3_CAL_SHIFT);    //pmic_vwifi3v3_cal(VWIFI3V3_CAL);
    //    pmic_config_interface(0x31, VWIFI3V3_SEL, VWIFI3V3_SEL_MASK, VWIFI3V3_SEL_SHIFT);    //pmic_vwifi3v3_sel(VWIFI3V3_SEL);
    pmic_config_interface (0x32, KAL_FALSE, VWIFI2V8_CAL_MASK, VWIFI3V3_EN_SHIFT);      //pmic_vwifi3v3_enable(KAL_TRUE);

    //    pmic_config_interface(0x34, VWIFI2V8_CAL, VWIFI2V8_CAL_MASK, VWIFI2V8_CAL_SHIFT);    //pmic_vwifi2v8_cal(VWIFI2V8_CAL);
    //    pmic_config_interface(0x34, VWIFI2V8_SEL, VWIFI2V8_SEL_MASK, VWIFI2V8_SEL_SHIFT);    //pmic_vwifi2v8_sel(VWIFI2V8_SEL);
    pmic_config_interface (0x35, KAL_FALSE, VWIFI2V8_EN_MASK, VWIFI2V8_EN_SHIFT);      

#if 0                           // Kelvin, 20090806, remove un-necessary PMIC LDO
    pmic_config_interface (0x38, VSIM_CAL, VSIM_CAL_MASK, VSIM_CAL_SHIFT);      //pmic_vsim_cal(VSIM_CAL);
    pmic_config_interface (0x37, VSIM_3_0V, VSIM_SEL_MASK, VSIM_SEL_SHIFT);     //pmic_vsim_sel(VSIM_3_0V);
    pmic_config_interface (0x37, KAL_TRUE, VSIM_EN_MASK, VSIM_EN_SHIFT);        //pmic_vsim_enable(KAL_TRUE);
#endif

#if 0
    //pmic_vbt_cal(VBT_CAL);
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        reg_addr = 0x3B;      // E3, E4 version
    }
    else
    {
        reg_addr = 0x3E;      // E1, E2 version    
    }
    pmic_config_interface (reg_addr, VBT_CAL, VBT_CAL_MASK, VBT_CAL_SHIFT);
    pmic_vbt_sel (VBT_SEL);
    //pmic_vbt_enable(KAL_TRUE);
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        reg_addr = 0x3A;      // E3, E4 version
    }
    else
    {
        reg_addr = 0x3D;      // E1, E2 version
    }
    pmic_config_interface (reg_addr, KAL_TRUE, VBT_EN_MASK, VBT_EN_SHIFT);
#endif

    //    pmic_config_interface(0x41, VCAMD_CAL, VCAMD_CAL_MASK, VCAMD_CAL_SHIFT);    //pmic_vcamd_cal(VCAMD_CAL);
    //    pmic_config_interface(0x40, VCAMD_SEL, VCAMD_SEL_MASK, VCAMD_SEL_SHIFT);    //pmic_vcamd_sel(VCAMD_SEL);
    //    pmic_config_interface(0x40, KAL_TRUE, VCAMD_EN_MASK, VCAMD_EN_SHIFT);    //pmic_vcamd_enable(KAL_TRUE);

#if 1                           // Kelvin, 20090806, remove un-necessary PMIC LDO
    pmic_config_interface (0x44, VGP_CAL, VGP_CAL_MASK, VGP_CAL_SHIFT); //pmic_vgp_cal(VGP_CAL);
    pmic_config_interface (0x43, VGP_2_8, VGP_SEL_MASK, VGP_SEL_SHIFT); //pmic_vgp_sel(VGP_2_8); // for reference phone
    pmic_config_interface (0x43, KAL_TRUE, VGP_EN_MASK, VGP_EN_SHIFT);  //pmic_vgp_enable(KAL_TRUE);
#endif

#if defined(CFG_MMC_BOOT)       // Kelvin, 20090806, remove un-necessary PMIC LDO
    pmic_config_interface (0x46, VSDIO_CAL, VSDIO_CAL_MASK, VSDIO_CAL_SHIFT);   //pmic_vsdio_cal(VSDIO_CAL);
    pmic_config_interface (0x47, VSDIO_SEL, VSDIO_SEL_MASK, VSDIO_SEL_SHIFT);   //pmic_vsdio_sel(VSDIO_SEL);
    pmic_config_interface (0x46, KAL_TRUE, VSDIO_EN_MASK, VSDIO_EN_SHIFT);      //pmic_vsdio_enable(KAL_TRUE);
#endif

#if 0                           // Kelvin, 20090806, remove un-necessary PMIC LDO
    pmic_config_interface (0x5E, (VGP2_SEL >> 2), VGP2_SELH_MASK, VGP2_SELH_SHIFT);     //pmic_vgp2_sel(VGP2_SEL);    //pmic_vgp2_selh(val >> 2); // Extract 
bit2 to write to selh
    pmic_config_interface (0x49, (VGP2_SEL & 0x3), VGP2_SELL_MASK, VGP2_SELL_SHIFT);    //pmic_vgp2_sell(val & 0x3);     // Extract bit[1..0] to write to sell
    pmic_config_interface (0x84, VGP2_ON_SEL, VGP2_ON_SEL_MASK, VGP2_ON_SEL_SHIFT);     //pmic_vgp2_on_sel(VGP2_ON_SEL);
    pmic_config_interface (0x1A, KAL_TRUE, VGP2_EN_MASK, VGP2_EN_SHIFT);        //pmic_vgp2_enable(KAL_TRUE);
#endif

    pmic_vbus_enable (KAL_TRUE);

    //dbg_print("Open backlight!\n");
    //pmic_backlight_on();

    //    pmic_config_interface(0x53, VCORE2_ENABLE_WITH_VCORE2_EN, VCORE2_ON_SEL_MASK, VCORE2_ON_SEL_SHIFT); //pmic_vcore2_on_sel(VCORE2_ENABLE_WITH_VCORE2_EN);
    //    pmic_config_interface(0x52, KAL_TRUE, VCORE2_EN_MASK, VCORE2_EN_SHIFT); //pmic_vcore2_enable(KAL_TRUE);

#if 0
    dbg_print ("pmic_init : after setting !!\n ");
    for (i = 0; i < PMIC_REG_NUM; i++)
    {
        // We skip intr state read back
        // If there is intr asserted, after enable EINT intr, pmic hisr will handle the intr
        if ((i != 0x0B) && (i != 0x0C) && (i != 0x0D) && (i != 0x0E))
        {
            mt6516_i2c_read_byte (i, &pmic6326_reg[i]);
            dbg_print ("0x%X,0x%X\n", i, pmic6326_reg[i]);
        }
    }
#endif
}

#if defined(CONFIG_MMC)
void pmic_vsdio_power (int on)
{
    if (on)
    {
        pmic_config_interface (0x46, VSDIO_CAL, VSDIO_CAL_MASK, VSDIO_CAL_SHIFT);     //pmic_vsdio_cal(VSDIO_CAL);
        pmic_config_interface (0x47, VSDIO_SEL, VSDIO_SEL_MASK, VSDIO_SEL_SHIFT);     //pmic_vsdio_sel(VSDIO_SEL);
        pmic_config_interface (0x46, KAL_TRUE, VSDIO_EN_MASK, VSDIO_EN_SHIFT);        //pmic_vsdio_enable(KAL_TRUE);
    }
    else
    {
        pmic_config_interface (0x46, KAL_FALSE, VSDIO_EN_MASK, VSDIO_EN_SHIFT);       //pmic_vsdio_enable(KAL_TRUE);    
    }
}
#endif
