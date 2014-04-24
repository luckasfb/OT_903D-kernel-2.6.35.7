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

#ifndef PLL_H
#define PLL_H

#if 0
#define AP_WDT_MODE_REG                                 *((P_U32)(RGU_base +
0x00))
#define AP_WDT_MODE_REG_ENABLE_WDT                      (0x0001)
#define CLEARBIT_AP_WDT_MODE_REG(config)                AP_WDT_MODE_REG = (
    AP_WDT_MODE_REG_KEY | (AP_WDT_MODE_REG & (~config)))
#endif
#if 0
#define PMU_VCORE_CON0_REG             *((volatile UINT16*)(MIXEDSYS1_BASE+0x900))
#define PMU_VAPROC_CON0_REG            *((volatile UINT16*)(MIXEDSYS1_BASE+0x940))
#endif
#define PMU_VCORE_CON0_REG             (MIXEDSYS1_BASE+0x900)
#define PMU_VAPROC_CON0_REG            (MIXEDSYS1_BASE+0x940)
#define VBUCK_08                       (0x10<<4)
#define VBUCK_09                       (0x14<<4)
#define VBUCK_10                       (0x18<<4)
#define VBUCK_105                      (0x1A<<4)
#define VBUCK_11                       (0x1C<<4)
#define VBUCK_115                      (0x1E<<4)
#define VBUCK_12                       (0x00<<4)
#define VBUCK_125                      (0x02<<4)
#define VBUCK_13                       (0x04<<4)
#define VBUCK_135                      (0x06<<4)
#define VBUCK_1375                     (0x07<<4)
#if 0
#define PLL_CON0_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x100))
#define PLL_CON1_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x104))
#define PLL_CON2_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x108))
//#define PLL_CON3_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x10C))
#define PLL_CON4_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x110))
#define PLL_CON5_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x114))
#define PLL_CON6_REG                   *((volatile UINT16*)(MIXEDSYS0_BASE+0x118))
#define CLKSW_PLLDIV_CON0              *((volatile UINT16*)(MIXEDSYS0_BASE+0x11C))
#define CLKSW_PLLDIV_CON1              *((volatile UINT16*)(MIXEDSYS0_BASE+0x120))
#define CLKSW_PLLDIV_CON3              *((volatile UINT16*)(MIXEDSYS0_BASE+0x128))
#define CLKSW_PLLCNTEN_CON             *((volatile UINT16*)(MIXEDSYS0_BASE+0x12C))
#define MPLL_CON0_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x140))
#define MPLL_CON1_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x144))
//#define MPLL_CON2_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x148))   
#define MPLL_CON3_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x14C))
#define AMPLL_CON0_REG                 *((volatile UINT16*)(MIXEDSYS0_BASE+0x160))
#define AMPLL_CON1_REG                 *((volatile UINT16*)(MIXEDSYS0_BASE+0x164))
//#define AMPLL_CON2_REG                 *((volatile UINT16*)(MIXEDSYS0_BASE+0x168))   
#define AMPLL_CON3_REG                 *((volatile UINT16*)(MIXEDSYS0_BASE+0x16C))
#define DPLL_CON0_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x180))
#define DPLL_CON1_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x184))
//#define DPLL_CON2_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x188))   
#define DPLL_CON3_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x18C))
#define EPLL_CON0_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x1C0))
#define EPLL_CON1_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x1C4))
//#define EPLL_CON2_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x1C8))   
#define EPLL_CON3_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x1CC))
#define CPLL_CON0_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x200))
#define WPLL_CON0_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x240))
#define GPLL_CON0_REG                  *((volatile UINT16*)(MIXEDSYS0_BASE+0x280))
#define THREEDPLL_CON0_REG            *((volatile UINT16*)(MIXEDSYS0_BASE+0x2C0))
#define TVPLL_CON0_REG                *((volatile UINT16*)(MIXEDSYS0_BASE+0x300))
#define FGPLL_CON0_REG                *((volatile UINT16*)(MIXEDSYS0_BASE+0x340))
#define AUXPLL_CON0_REG               *((volatile UINT16*)(MIXEDSYS0_BASE+0x380))
#endif
#define PLL_CON0_REG                   (MIXEDSYS0_BASE+0x100)
#define PLL_CON1_REG                   (MIXEDSYS0_BASE+0x104)
#define PLL_CON2_REG                   (MIXEDSYS0_BASE+0x108)
//#define PLL_CON3_REG                 (MIXEDSYS0_BASE+0x10C)
#define PLL_CON4_REG                   (MIXEDSYS0_BASE+0x110)
#define PLL_CON5_REG                   (MIXEDSYS0_BASE+0x114)
#define PLL_CON6_REG                   (MIXEDSYS0_BASE+0x118)
#define CLKSW_PLLDIV_CON0              (MIXEDSYS0_BASE+0x11C)
#define CLKSW_PLLDIV_CON1              (MIXEDSYS0_BASE+0x120)
#define CLKSW_PLLDIV_CON3              (MIXEDSYS0_BASE+0x128)
#define CLKSW_PLLCNTEN_CON             (MIXEDSYS0_BASE+0x12C)
#define MPLL_CON0_REG                  (MIXEDSYS0_BASE+0x140)
#define MPLL_CON1_REG                  (MIXEDSYS0_BASE+0x144)
//#define MPLL_CON2_REG                (MIXEDSYS0_BASE+0x148)
#define MPLL_CON3_REG                  (MIXEDSYS0_BASE+0x14C)
#define AMPLL_CON0_REG                 (MIXEDSYS0_BASE+0x160)
#define AMPLL_CON1_REG                 (MIXEDSYS0_BASE+0x164)
//#define AMPLL_CON2_REG               (MIXEDSYS0_BASE+0x168)
#define AMPLL_CON3_REG                 (MIXEDSYS0_BASE+0x16C)
#define DPLL_CON0_REG                  (MIXEDSYS0_BASE+0x180)
#define DPLL_CON1_REG                  (MIXEDSYS0_BASE+0x184)
//#define DPLL_CON2_REG                (MIXEDSYS0_BASE+0x188)
#define DPLL_CON3_REG                  (MIXEDSYS0_BASE+0x18C)
#define EPLL_CON0_REG                  (MIXEDSYS0_BASE+0x1C0)
#define EPLL_CON1_REG                  (MIXEDSYS0_BASE+0x1C4)
//#define EPLL_CON2_REG                (MIXEDSYS0_BASE+0x1C8)
#define EPLL_CON3_REG                  (MIXEDSYS0_BASE+0x1CC)
#define CPLL_CON0_REG                  (MIXEDSYS0_BASE+0x200)
#define WPLL_CON0_REG                  (MIXEDSYS0_BASE+0x240)
#define GPLL_CON0_REG                  (MIXEDSYS0_BASE+0x280)
#define THREEDPLL_CON0_REG             (MIXEDSYS0_BASE+0x2C0)
#define TVPLL_CON0_REG                 (MIXEDSYS0_BASE+0x300)
#define FGPLL_CON0_REG                 (MIXEDSYS0_BASE+0x340)
#define AUXPLL_CON0_REG                (MIXEDSYS0_BASE+0x380)
#define MPLL_LOCKED                    0x20
#define DPLL_LOCKED                    0x20
#define WPLL_LOCKED                    0x400
#define GPLL_LOCKED                    0x8000
#define EPLL_LOCKED                    0x20
#define AMPLL_LOCKED                   0x20
#define CPLL_LOCKED                    0x8000
#define THREEDPLL_LOCKED               0x8000
#define TVPLL_LOCKED                   0x8000
#define FGPLL_LOCKED                   0x8000
#define AUXPLL_LOCKED                  0x8000
#define RG_OVRD_SYS_CLK                0x2000
#define FSEL_SHIFT                     8
#define EMI_201_5_MHZ                  (0x70<<(FSEL_SHIFT))     // EPLL = 403.0 MHz
#define EMI_195_0_MHZ                  (0x6F<<(FSEL_SHIFT))     // EPLL = 390.0 MHz
#define EMI_165_75_MHZ                 (0x00<<(FSEL_SHIFT))     // EPLL = 331.5 MHz
#define EMI_152_75_MHZ                 (0x65<<(FSEL_SHIFT))     // EPLL = 305.5 MHz
#define EMI_101_85_MHZ                 (0x53<<(FSEL_SHIFT))     // EPLL = 203.7 MHz
#define EMI_13_MHZ                     (0x02<<(FSEL_SHIFT))     // EPLL = 26.0  MHz
#define APMCU_650_MHZ                  (0x00<<(FSEL_SHIFT))     // AMPLL = 650.0 MHz
#define APMCU_520_MHZ                  (0x69<<(FSEL_SHIFT))     // AMPLL = 520.0 MHz
#define APMCU_390_MHZ                  (0x5F<<(FSEL_SHIFT))     // AMPLL = 390.0 MHz
#define APMCU_260_MHZ                  (0x4E<<(FSEL_SHIFT))     // AMPLL = 260.0 MHz
#define APMCU_130_MHZ                  (0x30<<(FSEL_SHIFT))     // AMPLL = 130.0 MHz
#define APMCU_26_MHZ                   (0x02<<(FSEL_SHIFT))     // AMPLL = 26.0 MHz
#define DSP_312_MHZ                    (0x00<<(FSEL_SHIFT))     // DPLL = 312.0 MHz
#define DSP_260_MHZ                    (0x31<<(FSEL_SHIFT))     // DPLL = 260.0 MHz
#define DSP_130_MHZ                    (0x1C<<(FSEL_SHIFT))     // DPLL = 130.0 MHz
#define DSP_26_MHZ                     (0x02<<(FSEL_SHIFT))     // DPLL = 26.0 MHz
#define MDMCU_520_MHZ                  (0x00<<(FSEL_SHIFT))     // MPLL = 520.0 MHz
#define MDMCU_390_MHZ                  (0x5F<<(FSEL_SHIFT))     // MPLL = 390.0 MHz
#define MDMCU_260_MHZ                  (0x4E<<(FSEL_SHIFT))     // MPLL = 260.0 MHz
#define MDMCU_130_MHZ                  (0x30<<(FSEL_SHIFT))     // MPLL = 130.0 MHz
#define MDMCU_26_MHZ                   (0x02<<(FSEL_SHIFT))     // MPLL = 26.0MHz
     extern void mt6516_pll_init (void);
     extern void JumpCmd (unsigned int);
     extern void C_Main (void);
     extern void disable_i_cache (void);

#endif
