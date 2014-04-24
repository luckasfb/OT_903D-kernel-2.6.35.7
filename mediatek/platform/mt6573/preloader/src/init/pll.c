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

#include "mt6573.h"
#include "mt6573_typedefs.h"
#include "mt6573_timer.h"
#include "uart.h"
#include "pll.h"
#include "preloader.h"

#define CHIP_VER_E1                     0x8A00
#define CHIP_VER_E2                     0xCA10

/* APCONFIG*/
#define APHW_VER                        (APCONFIG_BASE+0x0000)
#define APSW_VER                        (APCONFIG_BASE+0x0004)
#define APHW_CODE                       (APCONFIG_BASE+0x0008)
#define TOPSM_DMY0                      (APCONFIG_BASE+0x0060)
#define TOPSM_DMY1                      (APCONFIG_BASE+0x0064)
#define AP_SAPD_WAYEN                   (APCONFIG_BASE+0x00B0)
#define AP_SLPPRT_BUS                   (APCONFIG_BASE+0x00C0)
#define APPER_SLP_CON                   (APCONFIG_BASE+0x00C8)

/* MTCMOS */
#define PWR_RST_B                       0
#define PWR_ISO                         1
#define PWR_ON                          2
#define PWR_MEM_OFF                     3
#define PWR_CLK_DIS                     4
#define PWR_REQ_EN                      6
#define PWR_CTRL                        7
#define PWRON_SETTLE_SHIFT              8

#define RM_PWR_CON0                     (TOPSM_BASE + 0x0800)	/* 16 bits */
#define RM_PWR_CON1                     (TOPSM_BASE + 0x0804)	/* 16 bits */
#define RM_PWR_CON2                     (TOPSM_BASE + 0x0808)	/* 16 bits */
#define RM_PWR_CON3                     (TOPSM_BASE + 0x080c)	/* 16 bits */
#define RM_PWR_CON4                     (TOPSM_BASE + 0x0810)	/* 16 bits */
#define RM_PWR_CON5                     (TOPSM_BASE + 0x0814)	/* 16 bits */
#define RM_PWR_CON6                     (TOPSM_BASE + 0x0818)	/* 16 bits */
#define RM_PWR_CON7                     (TOPSM_BASE + 0x081c)	/* 16 bits */
#define RM_PWR_STA                      (TOPSM_BASE + 0x0820)

typedef enum MT65XX_SUBSYS_TAG {
    MD2G_SUBSYS,
    HSPA_SUBSYS,
    FC4_SUBSYS, 
    AUDIO_SUBSYS,
    MM1_SUBSYS,
    MM2_SUBSYS,
    MD_SUBSYS, 
    AP_SUBSYS,
    MT65XX_SUBSYS_COUNT_END,
} MT65XX_SUBSYS;

void MTCMOS_Dis(UINT32 subsys)
{
    volatile UINT16 reg_val = 0,i;

    /* AP slave i/f way disable setting */
    if (subsys == AUDIO_SUBSYS) 
        DRV_ClrReg32(AP_SAPD_WAYEN, 1<< 11) ;
    if (subsys == MM1_SUBSYS) 
        DRV_ClrReg32(AP_SAPD_WAYEN, 1<< 8) ;
    if (subsys == MM2_SUBSYS) 
        DRV_ClrReg32(AP_SAPD_WAYEN, 1<< 9) ;
    /* Assert Power on reset */
    DRV_ClrReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_RST_B)); 

    /* Subsys Clock off and ISO en*/
    DRV_SetReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_CLK_DIS)|(1<<PWR_ISO) ); 

    /* MMSYS power */
    if (subsys == MM1_SUBSYS)
        DRV_WriteReg32(TOPSM_DMY0,0xFFFFFFFF);
    if (subsys == MM2_SUBSYS)
        DRV_WriteReg32(TOPSM_DMY1,0xFFFFFFFF);

    /* Subsys Mem Power off and Subsys Power off    */
    reg_val = DRV_Reg32(TOPSM_BASE+0x800+subsys*4);
    reg_val = ((reg_val | (1<<PWR_MEM_OFF)) & ~(1<<PWR_ON));
    DRV_WriteReg32((TOPSM_BASE+0x800+subsys*4), reg_val);

    /* delay*/
    for (i = 0; i < 1000; i++);
}

/*
Enable the Following PLL
1. AP_MCU_PLL
2. MD_MCU_PLL
3. DSP_PLL
4. EMI_PLL
5. 3G_PLL :  
6. 2G_PLL
*/

bool pl_pll_init (void)
{
#if 1
    UINT32 i = 0;
    UINT32 ret_val, chip_ver, turbo_mode = 0;
    
    chip_ver = DRV_Reg32(APHW_VER);
    
    *((volatile UINT32 *) 0x70026040) = 0x00001031;
    
    // CLKSQ using hw control mode
    *((volatile UINT16 *) 0x7002E080) = 0x0000;
    
    // All clock source restore to 26Mhz 
    *((volatile UINT16 *) 0x7002E114) = 0x0000;
    *((volatile UINT16 *) 0x7002E110) = 0x0000;
    *((volatile UINT16 *) 0x7002E118) = 0x0000;
    
    // MD PLL control by MPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E144) = 0x0100;
    *((volatile UINT16 *) 0x7002E140) = 0x0000;
    
    // DSP PLL control by DPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E184) = 0x0100;
    *((volatile UINT16 *) 0x7002E180) = 0x0000;
    
    // 3G PLL disable
    *((volatile UINT16 *) 0x7002E240) = 0x0300;
    
    // 2G PLL disable
    *((volatile UINT16 *) 0x7002E280) = 0x0300;
    
    // EMI PLL control by EPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E1C4) = 0x0100;
    *((volatile UINT16 *) 0x7002E1C0) = 0x0000;
    
    // AP PLL control by AMPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E164) = 0x0100;
    *((volatile UINT16 *) 0x7002E160) = 0x0000;
    
    // CAM PLL disable
    *((volatile UINT16 *) 0x7002E200) = 0x0000;
    
    // 3D PLL disable
    *((volatile UINT16 *) 0x7002E2C0) = 0x0000;
    
    // TV PLL disable
    *((volatile UINT16 *) 0x7002E300) = 0x0000;
    
    // FG PLL disable
    *((volatile UINT16 *) 0x7002E340) = 0x0000;
    
    // AUX PLL disable
    *((volatile UINT16 *) 0x7002E380) = 0x0000;
    
    if (chip_ver == CHIP_VER_E2)
    {
        // MD PLL en, 481Mhz
        *((volatile UINT16 *) 0x7002E140) = 0x664C;
        *((volatile UINT16 *) 0x7002E140) = 0x664D;
    }
    else
    {
        // MD PLL en, 520Mhz
        *((volatile UINT16 *) 0x7002E140) = 0x004C;
        *((volatile UINT16 *) 0x7002E140) = 0x004D;
    }

    // DSP PLL en, 253.5Mhz
    *((volatile UINT16 *) 0x7002E180) = 0x304C;
    *((volatile UINT16 *) 0x7002E180) = 0x304D;

    // 3G PLL en, MSDC en
    *((volatile UINT16 *) 0x7002E240) = 0x02CC;
    *((volatile UINT16 *) 0x7002E240) = 0x02CD;
    
    // 2G PLL en, USB en
    *((volatile UINT16 *) 0x7002E280) = 0x02CC;
    *((volatile UINT16 *) 0x7002E280) = 0x02CD;
    
    turbo_mode = (*((volatile UINT32 *) 0x70024104) & 0x10000000) >> 28;
    if (turbo_mode)
    {
        // AP PLL en, 806Mhz
#if defined(PROJECT_SMART_PHONE_CPU_806M)
	//huyanwei modify it .
        // default code.AP PLL en, 806Mhz
        *((volatile UINT16 *) 0x7002E160) = 0x7F4C;
        *((volatile UINT16 *) 0x7002E160) = 0x7F4D;
#else
        //huyanwei modify it for AP PLL 676MHz.
        *((volatile UINT16 *) 0x7002E160) = 0x754C;
        *((volatile UINT16 *) 0x7002E160) = 0x754D;
#endif

    }
    else
    {
        // AP PLL en, 676Mhz
        *((volatile UINT16 *) 0x7002E160) = 0x754C;
        *((volatile UINT16 *) 0x7002E160) = 0x754D;
    }
    
    // CAM PLL dis, 143Mhz
    *((volatile UINT16 *) 0x7002E200) = 0x004C;
    //*((volatile UINT16 *) 0x7002E200) = 0x004D;
    
    // 3D PLL dis, 195Mhz
    *((volatile UINT16 *) 0x7002E2C0) = 0x004C;
    //*((volatile UINT16*) 0x7002E2C0) = 0x004D;    
    
    // TV PLL dis
    *((volatile UINT16 *) 0x7002E300) = 0x004C;
    //*((volatile UINT16*) 0x7002E300) = 0x004D;            
    
    // FG PLL dis
    *((volatile UINT16 *) 0x7002E340) = 0x0240;
    //*((volatile UINT16*)   0x7002E340) = 0x0241;                    
    
    // AUX PLL dis,  48Mhz
    *((volatile UINT16 *) 0x7002E380) = 0x034C;
    //*((volatile UINT16*) 0x7002E380) = 0x034D;    
    
    // EMI PLL en, 390Mhz
    *((volatile UINT16 *) 0x7002E1C0) = 0x6D4C;
    *((volatile UINT16 *) 0x7002E1C0) = 0x6D4D;
    
    for (i = 0; i < 1000; i++);
    
    // system control MD PLL        
    *((volatile UINT16 *) 0x7002E144) = 0x0000;
    
    // system control DSP PLL
    *((volatile UINT16 *) 0x7002E184) = 0x0000;
    
    // system control EMI PLL
    *((volatile UINT16 *) 0x7002E1C4) = 0x0000;
    
    // system control AP MCU PLL
    *((volatile UINT16 *) 0x7002E164) = 0x0000;
    
    // MD:MD PLL, DSP:DSP use 2G PLL, EMI:EMI PLL, CAM:CAM PLL 
    *((volatile UINT16 *) 0x7002E110) = 0x1111;
    
    // MSDC:3G PLL, GSM:2G PLL, 3G:3GPLL, AP:AP PLL
    *((volatile UINT16 *) 0x7002E114) = 0x1111;
    
    // Audio clock source is from 26Mhz
    *((volatile UINT16 *) 0x7002E118) = 0x0145;

    // enable auido pll
    // should not using AUX_PLL to replace 26Mhz , will affect MD behavior
    *((volatile UINT16 *) 0x7002E108) = 0x0000;

    // Disable WDT 
    //*(volatile UINT16 *) 0x70025000) = 0x2200;
    *((volatile UINT16 *) 0x70025020) = 0x2200;
    *((volatile UINT16 *) 0x70025030) = 0x2200;
    
    // un-gate APMCU CG bit     
    *((volatile UINT32 *) 0x70026308) = 0xffffffff;
    *((volatile UINT32 *) 0x70026318) = 0xffffffff;
    
    // Enable MSDC,AD2G,TP,2GTX,USB,USB11,UART1,UART4,AD3G
    *((volatile UINT32 *) 0x70026304) = 0x0C0E;
    
    // Enable SIM0,SIM1,DMA,PWM,MIX,NFI,I2C,I2C2
    *((volatile UINT32 *) 0x70026314) = 0x1C09;

    if (chip_ver == CHIP_VER_E2)
    {
        /* HW Power down Sequence MM1 SYS */
        // Clear power ack status before using it, write one clear
        DRV_WriteReg32(RM_PWR_STA, 0xffffffff);
    
        // Keep software control first and enable hw control power on
        ret_val = DRV_Reg32(RM_PWR_CON4) | 0x00C4; 
        DRV_WriteReg32(RM_PWR_CON4, ret_val);
        
        /* Sync for 32K base HW power on status */
        while (1)
        {
            ret_val = DRV_Reg32(RM_PWR_STA) & 0x100000;
            if (ret_val) 
            {
                /* MM1 Power ACK is sync to high now */
                break;
            }
        }
        
        /* No going to turn off MM1 Power in HW mode */
        DRV_WriteReg32(RM_PWR_STA, 0xffffffff); 
            
        // Switch off software control mode and turn off power req
        ret_val = DRV_Reg32(RM_PWR_CON4) & 0xFF7F; 
        DRV_WriteReg32(RM_PWR_CON4, ret_val);
    }
    
    // un-gate MMSYS1 CG bit, enable DPI, LCD, GMC    
    *((volatile UINT32 *) 0x70080340) = 0xffffffff;
    *((volatile UINT32 *) 0x70080320) = 0xFE7FFFFE;
    
    // PMU work around keypad 
    *((volatile UINT32 *) 0x7002F62C) = 0x00000010;
    
    // power off uncessary MTCMOS
    MTCMOS_Dis(MM2_SUBSYS);
    MTCMOS_Dis(AUDIO_SUBSYS);

#else //SMT BK

    int i = 0;
    
    *((volatile UINT32 *) 0x70026040) = 0x00001031;
    
    // CLKSQ using hw control mode
    *((volatile UINT16 *) 0x7002E080) = 0x0000;
    
    // All clock source restore to 26Mhz 
    *((volatile UINT16 *) 0x7002E114) = 0x0000;
    *((volatile UINT16 *) 0x7002E110) = 0x0000;
    *((volatile UINT16 *) 0x7002E118) = 0x0000;
    
    // MD PLL control by MPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E144) = 0x0100;
    *((volatile UINT16 *) 0x7002E140) = 0x0000;
    
    // DSP PLL control by DPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E184) = 0x0100;
    *((volatile UINT16 *) 0x7002E180) = 0x0000;
    
    // 3G PLL disable
    *((volatile UINT16 *) 0x7002E240) = 0x0300;
    
    // 2G PLL disable
    *((volatile UINT16 *) 0x7002E280) = 0x0300;
    
    // EMI PLL control by EPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E1C4) = 0x0100;
    *((volatile UINT16 *) 0x7002E1C0) = 0x0000;
    
    // AP PLL control by AMPLL_CON0, disable 
    *((volatile UINT16 *) 0x7002E164) = 0x0100;
    *((volatile UINT16 *) 0x7002E160) = 0x0000;
    
    // CAM PLL disable
    *((volatile UINT16 *) 0x7002E200) = 0x0000;
    
    // 3D PLL disable
    *((volatile UINT16 *) 0x7002E2C0) = 0x0000;
    
    // TV PLL disable
    *((volatile UINT16 *) 0x7002E300) = 0x0000;
    
    // FG PLL disable
    *((volatile UINT16 *) 0x7002E340) = 0x0000;
    
    // AUX PLL disable
    *((volatile UINT16 *) 0x7002E380) = 0x0000;
    
    // MD PLL en, 312Mhz
    *((volatile UINT16 *) 0x7002E140) = 0x564C;
    *((volatile UINT16 *) 0x7002E140) = 0x564D;
    
    // DSP PLL en, 280Mhz
    *((volatile UINT16 *) 0x7002E180) = 0x344C;
    *((volatile UINT16 *) 0x7002E180) = 0x344D;
    
    // 3G PLL en, MSDC en
    *((volatile UINT16 *) 0x7002E240) = 0x02CC;
    *((volatile UINT16 *) 0x7002E240) = 0x02CD;
    
    // 2G PLL en, USB en
    *((volatile UINT16 *) 0x7002E280) = 0x02CC;
    *((volatile UINT16 *) 0x7002E280) = 0x02CD;
    
    // AP PLL en, 520Mhz
    *((volatile UINT16 *) 0x7002E160) = 0x694C;
    *((volatile UINT16 *) 0x7002E160) = 0x694D;
    
    // CAM PLL dis, 143Mhz
    *((volatile UINT16 *) 0x7002E200) = 0x004C;
    //*((volatile UINT16 *) 0x7002E200) = 0x004D;
    
    // 3D PLL dis, 195Mhz
    *((volatile UINT16 *) 0x7002E2C0) = 0x004C;
    //*((volatile UINT16*) 0x7002E2C0 = 0x004D;    
    
    // TV PLL dis, 
    *((volatile UINT16 *) 0x7002E300) = 0x004C;
    //*((volatile UINT16*) 0x7002E300 = 0x004D;            
    
    // FG PLL dis
    *((volatile UINT16 *) 0x7002E340) = 0x0240;
    //*((volatile UINT16*) 0x7002E340) = 0x0241;                    
    
    // AUX PLL dis,  48Mhz
    *((volatile UINT16 *) 0x7002E380) = 0x034C;
    //*((volatile UINT16*) 0x7002E380) = 0x034D;    
    
    // EMI PLL en, 332Mhz
    *((volatile UINT16 *) 0x7002E1C0) = 0x694C;
    *((volatile UINT16 *) 0x7002E1C0) = 0x694D;
    
    for (i = 0; i < 1000; i++);
    
    // system control MD PLL        
    *((volatile UINT16 *) 0x7002E144) = 0x0000;
    
    // system control DSP PLL
    *((volatile UINT16 *) 0x7002E184) = 0x0000;
    
    // system control EMI PLL
    *((volatile UINT16 *) 0x7002E1C4) = 0x0000;
    
    // system control AP MCU PLL
    *((volatile UINT16 *) 0x7002E164) = 0x0000;
    
    // MD:MD PLL, DSP:DSP PLL, EMI:EMI PLL, CAM:CAM PLL 
    *((volatile UINT16 *) 0x7002E110) = 0x1111;
    
    // MSDC:3G PLL, GSM:2G PLL, 3G:3GPLL, AP:AP PLL
    *((volatile UINT16 *) 0x7002E114) = 0x1111;
    
    // Audio clock source is from 26Mhz
    *((volatile UINT16 *) 0x7002E118) = 0x0145;

    // enable auido pll
    // should not using AUX_PLL to replace 26Mhz , will affect MD behavior
    *((volatile UINT16 *) 0x7002E108) = 0x0320;

    // Disable WDT 
    //*((volatile UINT16 *) 0x70025000) = 0x2200;
    *((volatile UINT16 *) 0x70025020) = 0x2200;
    *((volatile UINT16 *) 0x70025030) = 0x2200;
    
    // un-gate APMCU CG bit     
    *((volatile UINT32 *) 0x70026308) = 0xffffffff;
    *((volatile UINT32 *) 0x70026318) = 0xffffffff;
    
    // un-gate MMSYS1 CG bit    
    *((volatile UINT32 *) 0x70080340) = 0xffffffff;
    
    // PMU work around keypad 
    *((volatile UINT32 *) 0x7002F62C) = 0x00000010;
#endif
    return true;
}
