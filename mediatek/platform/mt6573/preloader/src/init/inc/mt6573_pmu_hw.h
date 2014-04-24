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


#ifndef _PMU_HW_H
#define _PMU_HW_H


#define PMU_CON0                    (PMU_BASE+0x000)
#define PMU_CON1                    (PMU_BASE+0x004)
#define PMU_CON2                    (PMU_BASE+0x008)
#define PMU_CON3                    (PMU_BASE+0x00C)
#define PMU_CON4                    (PMU_BASE+0x010)
#define PMU_CON5                    (PMU_BASE+0x014)
#define PMU_CON6                    (PMU_BASE+0x018)
#define PMU_CON7                    (PMU_BASE+0x01C)
#define PMU_CON8                    (PMU_BASE+0x020)
#define PMU_CON9                    (PMU_BASE+0x024)
#define PMU_CONA                    (PMU_BASE+0x028)
#define PMU_CONB                    (PMU_BASE+0x02C)
#define PMU_CONC                    (PMU_BASE+0x030)
#define PMU_COND                    (PMU_BASE+0x034)
#define PMU_CONE                    (PMU_BASE+0x038)
#define PMU_CONF                    (PMU_BASE+0x03C)

#define PMU_CON10                   (PMU_BASE+0x040)
#define PMU_CON11                   (PMU_BASE+0x044)
#define PMU_CON12                   (PMU_BASE+0x048)
#define PMU_CON13                   (PMU_BASE+0x04C)
#define PMU_CON14                   (PMU_BASE+0x050)
#define PMU_CON15                   (PMU_BASE+0x054)
#define PMU_CON16                   (PMU_BASE+0x058)
#define PMU_CON17                   (PMU_BASE+0x05C)
#define PMU_CON18                   (PMU_BASE+0x060)
#define PMU_CON19                   (PMU_BASE+0x064)
#define PMU_CON1A                   (PMU_BASE+0x068)
#define PMU_CON1B                   (PMU_BASE+0x06C)
#define PMU_CON1C                   (PMU_BASE+0x070)
#define PMU_CON1D                   (PMU_BASE+0x074)
#define PMU_CON1E                   (PMU_BASE+0x078)
#define PMU_CON1F                   (PMU_BASE+0x07C)

#define PMU_CON20                   (PMU_BASE+0x100)
#define PMU_CON21                   (PMU_BASE+0x104)
#define PMU_CON22                   (PMU_BASE+0x108)
#define PMU_CON23                   (PMU_BASE+0x10C)
#define PMU_CON24                   (PMU_BASE+0x110)
#define PMU_CON25                   (PMU_BASE+0x114)
#define PMU_CON26                   (PMU_BASE+0x118)
#define PMU_CON27                   (PMU_BASE+0x11C)   //reserved
#define PMU_CON28                   (PMU_BASE+0x120)
#define PMU_CON29                   (PMU_BASE+0x124)
#define PMU_CON2A                   (PMU_BASE+0x128)

#define PMU_F32K_CON0               (PMU_BASE+0xB04)
#define PMU_TEMPDET_CON0            (PMU_BASE+0xC00)
#define PMU_TEMPDET_CON1            (PMU_BASE+0xC04)
#define PMU_PWRRST_CODE             (PMU_BASE+0x800)
#define PMU_MISC                    (PMU_BASE+0x804)

/*PMU_CON0*/

#define RG_FORCE_PWM
#define RG_CL
#define RG_CS


/*PMU_CON1*/
#define RG_SLP
#define RG_RC
#define RG_SS
#define RG_SR_PMOS
#define RG_SR_NMOS
#define RG_SRC

/*PMU_CON2*/
#define RG_SEL
#define RG_SLP_12_INC


/*PMU_CON3*/
#define RG_ACC_OUT_INIT_VCORE1
#define RG_ADC_IN_EDGE_VCORE1
#define RG_ADJCKSEL_VCORE1
#define RG_DCVCKSEL_VCORE1              (0x1<<12)
#define RG_DCVTRIM_VBAT_VCORE1_MASK     0x1FFF

/*PMU_CON4*/
#define RG_DIRECT_CTRL_EN_VCORE1
#define RG_DUTY_INIT_VBAT_VCORE1
#define RG_FAST_SLOW_VCORE1
#define RG_FBEN_VCORE1
#define RG_GAIND_VCORE1

/*PMU_CON5*/
#define RG_GAINI_VCORE1
#define RG_GAINP_VCORE1
#define RG_IASEL_VBAT
#define RG_ISEL_VBAT
#define RG_MODECMP_VBAT
#define RG_MODEEN_VBAT
#define RG_MODESELIA_VBAT
#define RG_MODESET_VBAT

/*PMU_CON6*/
#define RG_NCDOF_VBAT_VCORE1
#define RG_PFMISEL_VBAT_VCORE1
#define RG_PWMB_VCORE1
#define RG_RSEL_VBAT_VCORE1
#define RG_SDM_ORDER_VCORE1

/*PMU_CON7*/
#define RG_VFBADJ_12_VCORE1

#define OUTPUT_VCORE1_1_5 0xD
#define OUTPUT_VCORE1_1_2 0x7

typedef enum
{
    OUTPUT_VCORE1_160 = 0xF,
    OUTPUT_VCORE1_155 = 0xE,
    OUTPUT_VCORE1_150 = 0xD,
    OUTPUT_VCORE1_145 = 0xC,
    OUTPUT_VCORE1_140 = 0xB,
    OUTPUT_VCORE1_135 = 0xA,
    OUTPUT_VCORE1_130 = 0x9,
    OUTPUT_VCORE1_125 = 0x8,
    OUTPUT_VCORE1_120 = 0x7,
    OUTPUT_VCORE1_115 = 0x6,
    OUTPUT_VCORE1_110 = 0x5,
    OUTPUT_VCORE1_105 = 0x4,
    OUTPUT_VCORE1_100 = 0x3,
    OUTPUT_VCORE1_095 = 0x2,
    OUTPUT_VCORE1_090 = 0x1,
    OUTPUT_VCORE1_085 = 0x0
} VCORE1_VOL;


#define RG_VOSEL_VBAT_VCORE1
#define RG_DCV_TEST_EN_VBAT_VCORE1
#define RG_DCV_SLEW_CTRL_VBAT_VCORE1
#define RG_CLK_SOURCE_SEL_VCORE1
#define RG_VD_SENSE_VBAT_VCORE1         0x1000
#define RG_DCV_SLEW_CTRL_NMOS_VBAT_VCORE1
#define AUTO_UPDATE_EN                  0x8000

/*PMU_CON8*/
#define RG_CAL_LDO_VCORE1
#define RG_ICAL_LDO_VCORE1              0x0040
#define RG_EN_FORCE_LDO_VCORE1
#define RG_ANTIUNSH_DN_LDO_VCORE1

/*PMU_CON9*/
#define RG_ACC_OUT_INIT_VCORE2
#define RG_ADC_IN_EDGE_VCORE2
#define RG_ADJCKSEL_VCORE2
#define RG_DCVCKSEL_VCORE2              (0x1<<12)
#define RG_DCVTRIM_VBAT_VCORE2_MASK     0x1FFF

/*PMU_CONA*/
#define RG_DIRECT_CTRL_EN_VCORE2
#define RG_DUTY_INIT_VBAT_VCORE2
#define RG_FAST_SLOW_VCORE2
#define RG_FBEN_VCORE2
#define RG_GAINP_VCORE2

/*PMU_CONB*/
#define RG_GAINI_VCORE2
#define RG_GAIND_VCORE2
#define RG_IASEL_VBAT
#define RG_ISEL_VBAT
#define RG_MODECMP_VBAT
#define RG_MODEEN_VBAT
#define RG_MODESELIA_VBAT
#define RG_MODESET_VBAT

/*PMU_CONC*/
#define RG_NCDOF_VBAT_VCORE2
#define RG_PFMISEL_VBAT_VCORE2
#define RG_PWMB_VCORE2
#define RG_RSEL_VBAT_VCORE2
#define RG_SDM_ORDER_VCORE2

/*PMU_COND*/
#define RG_VFBADJ_12_VCORE2
typedef enum
{
    OUTPUT_VCORE2_160 = 0xF,
    OUTPUT_VCORE2_155 = 0xE,
    OUTPUT_VCORE2_150 = 0xD,
    OUTPUT_VCORE2_145 = 0xC,
    OUTPUT_VCORE2_140 = 0xB,
    OUTPUT_VCORE2_135 = 0xA,
    OUTPUT_VCORE2_130 = 0x9,
    OUTPUT_VCORE2_125 = 0x8,
    OUTPUT_VCORE2_120 = 0x7,
    OUTPUT_VCORE2_115 = 0x6,
    OUTPUT_VCORE2_110 = 0x5,
    OUTPUT_VCORE2_105 = 0x4,
    OUTPUT_VCORE2_100 = 0x3,
    OUTPUT_VCORE2_095 = 0x2,
    OUTPUT_VCORE2_090 = 0x1,
    OUTPUT_VCORE2_085 = 0x0
} VCORE2_VOL;




#define RG_VOSEL_VBAT_VCORE2
#define RG_DCV_TEST_EN_VBAT_VCORE2
#define RG_DCV_SLEW_CTRL_VBAT_VCORE2
#define RG_CLK_SOURCE_SEL_VCORE2
#define RG_VD_SENSE_VBAT_VCORE2             0x1000
#define RG_DCV_SLEW_CTRL_NMOS_VBAT_VCORE2

/*PMU_CONE*/
#define RG_CAL_LDO_VCORE2
#define RG_ICAL_LDO_VCORE2
#define RG_EN_FORCE_LDO_VCORE2
#define RG_ANTIUNSH_DN_LDO_VCORE2

/*PMU_CONF*/
#define RG_ACC_OUT_INIT_VM
#define RG_ADC_IN_EDGE_VM
#define RG_ADJCKSEL_VM
#define RG_DCVCKSEL_VM                      (0x1<<12)
#define RG_DCVTRIM_VBAT_VM_MASK             0x1FFF


/*PMU_CON10*/
#define RG_DIRECT_CTRL_EN_VM                (1<<0)
#define RG_DUTY_INIT_VBAT_VM                (1<<7)
#define RG_FAST_SLOW_VM_EN                  (1<<8)
#define RG_FBEN_VM_EN                       (1<<9)
#define RG_GAINP_VM

/*PMU_CON11*/
#define RG_GAINI_VM
#define RG_GAIND_VM
#define RG_IASEL_VBAT
#define RG_ISEL_VBAT
#define RG_MODECMP_VBAT
#define RG_MODEEN_VBAT
#define RG_MODESELIA_VBAT
#define RG_MODESET_VBAT

/*PMU_CON12*/
#define RG_NCDOF_VBAT_VM
#define RG_PFMISEL_VBAT_VM
#define RG_PWMB_VM
#define RG_RSEL_VBAT_VM
#define RG_SDM_ORDER_VM

/*PMU_CON13*/
#define RG_VFBADJ_12_VM
#define RG_VOSEL_VBAT_VM
#define RG_DCV_TEST_EN_VBAT_VM
#define RG_DCV_SLEW_CTRL_VBAT_VM
#define RG_CLK_SOURCE_SEL_VM
#define RG_VD_SENSE_VBAT_VM
#define RG_DCV_SLEW_CTRL_NMOS_VBAT_VM

/*PMU_CON14*/
#define RG_CAL_LDO_VM
#define RG_ICAL_LDO_VM                  0x0040
#define RG_EN_FORCE_LDO_VM
#define RG_ANTIUNSH_DN_LDO_VM

/*PMU_CON15*/
#define RG_VIO_EN_FORCE
#define RG_ICALIO_EN
#define RG_ANTIUDSH_IO_DN
#define RG_VIO_CAL
#define RG_VRTC_EN_FORCE
#define RG_VRTC1_CAL
#define RG_VRTC2_CAL
#define RG_VRTCCAL_LATCH_EN

/*PMU_CON16*/
#define RG_VUSB_EN                      0x0001
#define RG_VUSB_EN_FORCE
#define RG_ICALUSB_EN
#define RG_ANTIUDSH_USB_DN
#define RG_VUSB_CAL
#define RG_TESTMODE_IO

/*PMU_CON17*/
#define RG_VCAMD_EN                     0x0001
#define RG_VCAMD_EN_FORCE
#define RG_ICALCAMD_EN
#define RG_ANTIUDSH_CAMD_DN
#define RG_VCAMD_CAL
#define RG_VCAMD_SEL_MASK               0x0C00
typedef enum
{
    RG_VCAMD_SEL_1_3V = (00 << 10),
    RG_VCAMD_SEL_1_5V = (01 << 10),
    RG_VCAMD_SEL_1_8V = (02 << 10),
    RG_VCAMD_SEL_2_8V = (03 << 10)
} RG_VCAMD_SEL;

/*PMU_CON18*/
#define RG_VTCXO_EN                     0x0001
#define RG_VTCXO_EN_FORCE
#define RG_ICALTCXO_EN
#define RG_VTCXO_ON_SEL
#define RG_VTCXO_CAL
#define RG_VA_EN_FORCE
#define RG_ICALA_EN

/*PMU_CON19*/
#define RG_VA_CAL
#define RG_VBT_EN                       0x0010
#define RG_VBT_EN_FORCE
#define RG_ICALBT_EN
#define RG_ANTIUDSH_BT_DN
#define RG_VBT_CAL
#define RG_VBT_SEL
#define RG_TESTMODE_A

/*PMU_CON1A*/
#define RG_VCAMA_EN                     0x0001
#define RG_VCAMA_EN_FORCE
#define RG_ICALCAMA_EN
#define RG_VCAMA_CAL
#define RG_VCAMA_SEL_MASK               0x0600
typedef enum
{
    RG_VCAMA_SEL_1_5V = (0 << 9),
    RG_VCAMA_SEL_1_8V = (1 << 9),
    RG_VCAMA_SEL_2_5V = (2 << 9),
    RG_VCAMA_SEL_2_8V = (3 << 9)
} RG_VCAMA_SEL;
/*PMU_CON1B*/
#define RG_VGPS_EN              (1<<0)
#define RG_VGPS_EN_FORCE        (1<<1)
#define RG_ICALGPS_EN           (3<<2)
#define RG_VGPS_CAL             (0xf<<3)

/*PMU_CON1C*/
#define RG_VGP_EN                       0x0001
#define RG_VCAMA_EN_FORCE
#define RG_ICALGP_EN
#define RG_VGP_CAL
#define RG_VGP_SEL_3_3                  0x0600
#define RG_VGP_SEL_MASK                 0x0600

typedef enum
{
    RG_VGP_SEL_1_5V = (0 << 9),
    RG_VGP_SEL_1_8V = (1 << 9),
    RG_VGP_SEL_2_8V = (2 << 9),
    RG_VGP_SEL_3_3V = (3 << 9)
} RG_VGP_SEL;


/*PMU_CON1D*/
#define RG_VSDIO_EN                     0x0001
#define RG_VSDIO_EN_FORCE
#define RG_ICALSDIO_EN
#define RG_VSDIO_CAL
#define RG_VSDIO_SEL2_8                 0x0200
#define RG_VSDIO_SEL3_3                 0x0600
#define RG_ANTIUDSH_SDIO_DN
#define RG_VSDIO_SEL_MASK               0x0600
typedef enum
{
    RG_VSDIO_SEL_1_5V = (0 << 9),
    RG_VSDIO_SEL_1_8V = (1 << 9),
    RG_VSDIO_SEL_2_8V = (2 << 9),
    RG_VSDIO_SEL_3_3V = (3 << 9)
} RG_VSDIO_SEL;

/*PMU_CON1E*/
#define RG_CRON_FORCE                   0x0001
#define RG_CR_EN                        0x0002
#define RG_CHOFST_MASK                  0x001C
#define RG_CLASS_D_MASK                 0x01E0
typedef enum RG_CLASS_D
{
    CC_50mA = (0 << 5),
    CC_87_5mA = (1 << 5),
    CC_150mA = (2 << 5),
    CC_225mA = (3 << 5),
    CC_300mA = (4 << 5),
    CC_450mA = (5 << 5),
    CC_650mA = (6 << 5),
    CC_800mA = (7 << 5),
    CC_900mA = (8 << 5),
    CC_1000mA = (9 << 5),
} CC_MODE_CHAGE_LEVEL;


#define RG_CV_RT

typedef enum CV_TUNE
{
    VBG_1200 = (0 << 11),
    VBG_1205 = (1 << 11),
    VBG_1210 = (2 << 11),
    VBG_1215 = (3 << 11),
    VBG_1180 = (4 << 11),
    VBG_1185 = (5 << 11),
    VBG_1190 = (6 << 11),
    VBG_1195 = (7 << 11),
} RG_CV_TUNE;

#define RG_OV_TH_FREEZE             (1<<14)
#define RG_OV_TH_HIGH               (1<<15)

/*PMU_CON1F*/
typedef enum UV_SEL
{
    UV_SEL_2900,
    UV_SEL_2750,
    UV_SEL_2600,
    UV_SEL_DDLO
} RG_UV_SEL;

typedef enum THR_SEL
{
    INIT_VAL = (0 << 12),
    PLUS_10_DRGREE = (1 << 12),
    MINUS_20_DRGREE = (2 << 12),
    MINUS_10_DRGREE = (3 << 12)
} RG_THR_SEL;

#define RG_OSC_ENB                  (1<<7)



/*PMU_CON20*/
#define RG_IBIASSEL
#define RG_BIAS_GEN_FORCE
#define RG_VA_EN_SEL
#define RG_NI_26M_SRC
#define RG_CLK26M_DIV_SET

/*PMU_CON21*/
#define RG_LED_R_EN
#define RG_LED_R_ICAL
#define RG_LED_G_EN
#define RG_LED_G_ICAL
#define RG_LED_B_EN
#define RG_LED_B_ICAL
#define RG_TP_LED

/*PMU_CON22*/
#define RG_TP_BUCK
#define RG_INT_NODE_MUX
#define RG_ISENSE_OUT_EN            0x0020
#define RG_VBAT_OUT_EN              0x0040
#define RG_PMU_TMSEL                0x0F80
#define RG_PMU_TMR_EN               0x1000
#define RG_PWRKEY_RSTB_SEL          0x6000
#define RG_PWRKEY_RSTB_SEL_MASK     0x6000
#define RG_PWRKEY_RSTB_SEL_SHIFT    13
#define RG_PWRKEY_RSTB_EN           0x8000
#define RG_PWRKEY_RSTB_EN_MASK      0x8000
#define RG_PWRKEY_RSTB_EN_SHIFT     15



/*PMU_CON23*/
#define RG_TM_RSV
#define RG_TPSEL
#define RG_TPSEL_E

/*PMU_CON24*/
typedef enum CAL_PRECC
{
    PRE_CC_50mA,
    PRE_CC_87_5mA,
    PRE_CC_150mA,
    PRE_CC_225mA
} RG_CAL_PRECC;

#define RG_CAL_CM
#define RG_CAL_CM2
#define RG_PS_SEL
#define RG_PS_SET       0x40
#define RG_CHR_RSV
typedef enum PS_SET
{
    PS_SET_VBAT = 0,
    PS_SET_AC
} RG_PS_SOURCE;



/*PMU_CON25*/
#define RG_CHR_TIMEOUT_EN           0x0001
#define RG_CR_TIMEOUT_SEL_MASK      0x0006
#define RG_CR_TIMEOUT_SEL_KEY       0xA250

typedef enum CHR_TIMEOUT_SEL
{
    CHR_TIMEOUT_SEL_4sec = (0 << 1),
    CHR_TIMEOUT_SEL_8sec = (1 << 1),
    CHR_TIMEOUT_SEL_16sec = (2 << 1),
    CHR_TIMEOUT_SEL_32sec = (3 << 1),
} RG_CHR_TIMEOUT_SEL;

/*PMU_CON26*/
#define RG_CHR_EN_CODE              0xAF26

/*PMU_CON27*/

/*PMU_CON28, read only */
#define ADC_INVALID                 (1<<15)
#define CHG_STATUS                  (1<<14)
#define UV_VBAT22                   (1<<13)
#define UV_VBAT32                   (1<<12)
#define EXT_RST_STATUS              0x0400
#define PMU_RSTB                    0x0100
#define PWRKEY_PREDEB               0x0080
#define BAD_BATT                    0x0040
#define CRDET_DIS                   0x0020
#define CV_MODE                     0x0010
#define CHR_DET                     0x0008
#define BAT_ONB                     0x0004
#define OVP                         0x0002
#define PWRKEY_DEB                  0x0001
#define PWRKEY_DEB_NOT_PRESSED      (1<<0)
#define BATTERY_REMOVED             (1<<2)
#define CHARGER_DETECTED            (1<<3)

/*PMU_CON29*/
#define VUSB_STATUS
#define VCAMA_STATUS
#define VCAMD_STATUS
#define VTCXO_STATUS
#define VSDIO_STATUS
#define VBT_STATUS
#define VGPS_STATUS
#define VGP_STATUS
#define VIO_STATUS
#define VA_STATUS
#define VM_STATUS
#define VCORE2_STATUS
#define VCORE1_STATUS
#define VRTC_STATUS


/*PMU_CON2A*/
typedef enum CHR_TEMPTIMEOUT
{
    CHR_WDT_TOUT = 2,
    CHR_UT_OR_HT = 3,
} CHR_TEMPTIMEOUT_STATUS;

#define F32K_FAIL_STATUS            (1<<2)
#define F32K_PROTECT_STATUS         (1<<3)
#define TEMP_PROTECT_STATUS         (1<<4)

/*PMU_F32K_CON0*/
#define F32K_PROTECT_CODE           0x22

/*PMU_TEMPDET_CON0*/
#define TEMP_PROTECT_CODE           0xA4
#define TEMP_SET_CODE               0x55


/*PMU_TEMPDET_CON1*/
#define TEMP_UNDER_THR_MASK         0x7F00
#define TEMP_OVER_THR_MASK          0x003F

/*PMU_PWRRST_CODE*/
//#define PMU_PWRRST_CODE         0x0033 /* CHECKME. Redefinition */

/*PMU_MISC*/
#define PMU_CHET                    (1<<0)
#define FORCE_VM_NMOS               (1<<1)
#define FORCE_VC2_NOT_DVFS          (1<<2)
#define FORCE_VC1_VFBADJ_SYNC       (1<<3)


#endif
