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


#include <mt6573.h>
#include "mt6573_typedefs.h"
#include <mt6573_wdt_hw.h>

bool pl_wdt_init (void)
{
    /* TODO */
    //Disable watch dog timer
    DRV_WriteReg32 (0x70025020, 0x2200);        // DSP
    DRV_WriteReg32 (0x70025030, 0x2200);        // MD	
    WDT_init();
    return true;	
}

bool pl_wdt_internal_init(void)
{
#ifdef DBG_PRELOADER    
    dbg_print ("\nWDT register dump:\n");
    dbg_print ("==================\n");
    dbg_print ("WDT_MODE = 0x%x\n", *(volatile unsigned short *) 0x70025000);
    dbg_print ("WDT_STA = 0x%x\n", *(volatile unsigned short *) 0x7002500C);
    dbg_print ("WDT_INTERNAL = 0x%x\n", *(volatile unsigned short *) 0x70025018);
    dbg_print ("WDT_DSP_MODE = 0x%x\n", *(volatile unsigned short *) 0x70025020);
    dbg_print ("WDT_DSP_STA = 0x%x\n", *(volatile unsigned short *) 0x7002502C);
    dbg_print ("WDT_MD_MODE = 0x%x\n", *(volatile unsigned short *) 0x70025030);
    dbg_print ("WDT_MD_STA = 0x%x\n", *(volatile unsigned short *) 0x7002503C);
    dbg_print ("WDT_MD_INTERNAL = 0x%x\n", *(volatile unsigned short *) 0x70025040);
    dbg_print ("WDT_DSP_RSTN = 0x%x\n", *(volatile unsigned short *) 0x70025014);
    dbg_print ("WDT_PERIPH_RSTN = 0x%x\n", *(volatile unsigned short *) 0x70025010);
    dbg_print ("\n");
    
    dbg_print ("PMU setting for solve L2 cache issue\n");
    dbg_print ("(0x7002F900) = 0x%x\n", DRV_Reg32 (0x7002F900));
    dbg_print ("(0x7002F914) = 0x%x\n", DRV_Reg32 (0x7002F914));
    dbg_print ("(0x7002F940) = 0x%x\n", DRV_Reg32 (0x7002F940));
    dbg_print ("(0x7002F954) = 0x%x\n", DRV_Reg32 (0x7002F954));
    dbg_print ("(0x7002F920) = 0x%x\n", DRV_Reg32 (0x7002F920));
    dbg_print ("(0x7002F934) = 0x%x\n", DRV_Reg32 (0x7002F934));
    dbg_print ("(0x7002F960) = 0x%x\n", DRV_Reg32 (0x7002F960));
    dbg_print ("(0x7002F974) = 0x%x\n", DRV_Reg32 (0x7002F974));
    
    dbg_print ("VAPROC_CON0(0x7002F940) = 0x%x\n", DRV_Reg32 (0x7002F940));
    dbg_print ("AMPLL_CON0(0x7002E160) = 0x%x\n", DRV_Reg32 (0x7002E160));
    dbg_print ("APP_MEM_PD(0x70026020) = 0x%x\n", DRV_Reg32 (0x70026020)); 
#endif  

    *(volatile unsigned short *) 0x70025018 = 0x0FFA;
    //*(volatile unsigned short *) 0x70025040 = 0x0FFA;
    
    dbg_print ("\n\nMT6573 (%s)\n\n", BUILD_TIME);
    return true;
}
#ifdef CONFIG_HW_WATCHDOG
static unsigned short timeout;

void hw_watchdog_disable(void)
{
    u16 tmp;

    tmp = DRV_Reg(MT6573_WDT_MODE);
    tmp &= ~MT6573_WDT_MODE_ENABLE;       /* disable watchdog */
    tmp |= (MT6573_WDT_MODE_KEY);       /* need key then write is allowed */
    DRV_WriteReg(MT6573_WDT_MODE,tmp);
}

void MT6573_sw_watchdog_reset(void)
{
    /* Watchdog Rest */
    DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY); 
    DRV_WriteReg(MT6573_WDT_MODE, (MT6573_WDT_MODE_KEY|MT6573_WDT_MODE_EXTEN|MT6573_WDT_MODE_ENABLE));
    //DRV_WriteReg(MT6573_WDT_LENGTH, MT6573_WDT_LENGTH_KEY);
    DRV_WriteReg(MT6573_WDT_SWRST, MT6573_WDT_SWRST_KEY);
}


unsigned int MT6573_wdt_CheckStatus(void)
{
    unsigned int status;

    status = DRV_Reg16(MT6573_WDT_STATUS);

    return status;
}

void MT6573_wdt_ModeSelection(kal_bool en, kal_bool auto_rstart, kal_bool IRQ )
{
    unsigned short tmp;

    tmp = DRV_Reg16(MT6573_WDT_MODE);
    tmp |= MT6573_WDT_MODE_KEY;

    // Bit 0 : Whether enable watchdog or not
    if(en == KAL_TRUE)
        tmp |= MT6573_WDT_MODE_ENABLE;
    else
        tmp &= ~MT6573_WDT_MODE_ENABLE;

    // Bit 4 : Whether enable auto-restart or not for counting value
    if(auto_rstart == KAL_TRUE)
        tmp |= MT6573_WDT_MODE_AUTORST;
    else
        tmp &= ~MT6573_WDT_MODE_AUTORST;

    // Bit 3 : TRUE for generating Interrupt (False for generating Reset) when WDT timer reaches zero
    if(IRQ == KAL_TRUE)
        tmp |= MT6573_WDT_RESET_IRQ;
    else
        tmp &= ~MT6573_WDT_RESET_IRQ;

    DRV_WriteReg16(MT6573_WDT_MODE,tmp);
}

void MT6573_wdt_SetTimeOutValue(unsigned short value)
{
    /*
    * TimeOut = BitField 15:5
    * Key      = BitField  4:0 = 0x08
    */

    // sec * 32768 / 512 = sec * 64 = sec * 1 << 6
    timeout = (unsigned short)(value * ( 1 << 6) );
    timeout = timeout << 5; 
    DRV_WriteReg16(MT6573_WDT_LENGTH, (timeout | MT6573_WDT_LENGTH_KEY) );  
}

void MT6573_wdt_Restart(void)
{
    // Reset WatchDogTimer's counting value to default value
    // ie., keepalive()

    DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY);
}

void
WDT_SW_Reset (void)
{
    dbg_print ("WDT SW RESET\n");
    DRV_WriteReg16 (0x70025000, 0x2201);
    DRV_WriteReg16 (0x70025008, 0x1971);
    DRV_WriteReg16 (0x7002501C, 0x1209);

    // 4. system will reset 

    while (1)
    {
        dbg_print ("SW reset fail ... \n");
    };
}

void WDT_HW_Reset(void)
{
    //dbg_print("WDT_HW_Reset_test\n");

    // 1. set WDT timeout 1 secs, 1*64*512/32768 = 1sec
    MT6573_wdt_SetTimeOutValue(1);

    // 2. enable WDT reset, auto-restart disable, disable intr
    MT6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_FALSE);

    // 3. reset the watch dog timer to the value set in WDT_LENGTH register
    MT6573_wdt_Restart();

    // 4. system will reset
    while(1);
}

void
hw_watchdog_reset (void)
{
}

void
mt6573_watchdog_init (void)
{
    /* TODO */
}

void mt6573_arch_reset(char mode)
{
    dbg_print("mt6573_arch_reset\n");

    WDT_HW_Reset();

    while (1);
}

#define WDT_INGORE_POWER_KEY (0x0ffe)
static unsigned int wstatus = 0;
static unsigned int wmode = 0;
//read and save status, disable WDT
void WDT_init(void)
{
	 wstatus = DRV_Reg16(MT6573_WDT_STATUS);
	 wmode = DRV_Reg16(MT6573_WDT_MODE);
	 hw_watchdog_disable();
}

bool WDT_boot_check(void)
{
	if ((wstatus & MT6573_WDT_STATUS_HWWDT) &&
  	 (wmode & MT6573_WDT_MODE_AUTORST)) // HW reset
  	{
  		// this method will be used to save status to uboot phase
  		//DRV_WriteReg16(MT6573_WDT_INTERNAL, WDT_INGORE_POWER_KEY);
  		return true;
  	}
       if ((wstatus & MT6573_WDT_STATUS_SWWDT) &&
          (wmode & MT6573_WDT_MODE_DEBUG))
       {
              return true;
       }
       return false; 
}
void WDT_arch_reset(char mode)
{
    WDT_HW_Reset();

    while (1);
}

#endif
