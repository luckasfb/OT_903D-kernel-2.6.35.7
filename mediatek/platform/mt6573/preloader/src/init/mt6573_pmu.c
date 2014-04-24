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

//#include "mt6573_timer.h"
#include "mt6573_typedefs.h"
#include "mt6573_gpio.h"
#include "mt6573_pmu_sw.h"
#include "cust_usbdl_flow.h"


#define BUCK_VFBADJ_MASK          0x01F0
#define BUCK_VFBADJ_SHIFT         4

#define DRV_Reg(addr)              (*(volatile kal_uint16 *)(addr))
#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))
#define DRV_SetData(addr, bitmask, value)     {\
    kal_uint16 temp;\
    temp = (~(bitmask)) & DRV_Reg(addr);\
    temp |= (value);\
    DRV_WriteReg(addr,temp);\
}
#define PMU_DRV_SetData16(addr, bitmask, value)   DRV_SetData(addr, bitmask, value)

#define DRV_SetData32(addr, bitmask, value)     {\
    kal_uint32 temp;\
    temp = (~(bitmask)) & DRV_Reg32(addr);\
    temp |= (value);\
    DRV_WriteReg32(addr,temp);\
}

#define PMU_DRV_SetData32(addr, bitmask, value)   DRV_SetData32(addr, bitmask, value)

bool pl_pmu_init (void)
{
    *(volatile unsigned short *)0x7002F940 = 0xA861;

#ifdef ZPHN_PLATFORM
    /* Disable DRVBUS */
    mt_set_gpio_mode(GPIO2,GPIO_MODE_05);
    mt_set_gpio_dir(GPIO2,GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO2,GPIO_OUT_ZERO);
#endif

#if 1
    /*Solve L2 cache issue*/
    DRV_WriteReg(0x7002F900, 0x2C01);
    DRV_WriteReg(0x7002F914, 0x0306);
    DRV_WriteReg(0x7002F940, 0x2C61);
    DRV_WriteReg(0x7002F954, 0x0306);
    DRV_WriteReg(0x7002F920, 0x2C01);
    DRV_WriteReg(0x7002F934, 0x0306);
    DRV_WriteReg(0x7002F960, 0x2801);
    DRV_WriteReg(0x7002F974, 0x0307);
#endif

#if 1
    /*Solve HW timing issue*/
    DRV_WriteReg(0x7002F940, 0x2C71);
#endif
    return true;

}


BOOL PMU_IsUsbCableIn (void)
{
    kal_uint16 reg_val16;

    // wait 500 ms for pmu stable
    //gpt4_busy_wait_ms (500);

    // From designer 0x7002FA00[13] (MT6573)
    // 1: Charge source in (Include USB and Charger)
    // 0: Charge source NOT in

    reg_val16 = *((volatile kal_uint16 *) (0x7002FA00));
    if (reg_val16 & ((kal_uint16) (1 << 13)))
    {
        return TRUE;
    }
    return FALSE;
}

CHARGER_TYPE  hw_charger_type_detection(void)
{
    CHARGER_TYPE ret                = CHARGER_UNKNOWN;
    unsigned int CHR_CON_9          = 0x7002FA24;
    unsigned int CHR_CON_10         = 0x7002FA28;
    unsigned int USB_U2PHYACR3_2    = 0x7005081E;
    unsigned int MEM_ID_USB20       = 0x70026038;
    unsigned int PDN_ID_USB         = 0x70026308;
    unsigned int USBPHYRegs         = 0x70050800; //U2B20_Base+0x800
    unsigned short wChargerAvail    = 0;
    unsigned short bLineState_B     = 0;
    unsigned short bLineState_C     = 0;

    //msleep(400);
    //dbg_print("PMU : charger_type_detection\r\n");

    /********* Step 0.0 : enable USB memory and clock *********/
    //PMU_DRV_SetData32(MEM_ID_USB20,0x02,0); //MEM_ID_USB20
    //PMU_DRV_SetData32(MEM_ID_USB20,0x20,0); //MEM_ID_USB20 bit5
    //msleep(1);          

    //PMU_DRV_SetData32(PDN_ID_USB,0x80,0); //PDN_ID_USB bit 7
    PMU_DRV_SetData32(PDN_ID_USB,0x80,1); //PDN_ID_USB bit 7
    mdelay(1);

    /********* Step 1.0 : PMU_BC11_Detect_Init ***************/
    //PMU_DRV_SetBits16(USB_U2PHYACR3_2,0x04); //USB_U2PHYACR3_2[2]=1
    SETREG8(USB_U2PHYACR3_2,0x04); //USB_U2PHYACR3_2[2]=1

    SETREG16(CHR_CON_9,0x0100);
    CLRREG16(CHR_CON_9,0x0100); 

    SETREG16(CHR_CON_10,0x0180);//RG_BC11_BIAS_EN=1 
    CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
    CLRREG16(CHR_CON_10,0x0040);//RG_BC11_VREF_VTH = 0
    CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
    CLRREG16(CHR_CON_10,0x0030);//RG_BC11_IPU_EN[1.0] = 00
    CLRREG16(CHR_CON_10,0x000C);//RG_BC11_IPD_EN[1.0] = 00

    /********* Step A *************************************/
    //dbg_print("mt_charger_type_detection : step A\r\n");
    CLRREG16(CHR_CON_10,0x0030);//RG_BC11_IPU_EN[1.0] = 00

    SETREG8(USBPHYRegs+0x10,0x0010);//RG_PUPD_BIST_EN = 1
    CLRREG8(USBPHYRegs+0x14,0x0040);//RG_EN_PD_DM=0

    SETREG16(CHR_CON_9,0x0002);//RG_BC11_VSRC_EN[1.0] = 10 
    SETREG16(CHR_CON_10,0x0004);//RG_BC11_IPD_EN[1:0] = 01
    CLRREG16(CHR_CON_10,0x0040);//RG_BC11_VREF_VTH = 0
    SETREG16(CHR_CON_10,0x0001);//RG_BC11_CMP_EN[1.0] = 01
    mdelay(80);
    wChargerAvail = INREG16(CHR_CON_10);
    //dbg_print("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
    CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
    CLRREG16(CHR_CON_10,0x0004);//RG_BC11_IPD_EN[1.0] = 00
    CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
    mdelay(80);

    if(wChargerAvail & 0x0200)
    {
        /********* Step B *************************************/
        //dbg_print("mt_charger_type_detection : step B\r\n");

        SETREG16(CHR_CON_10,0x0020); //RG_BC11_IPU_EN[1:0]=10
        mdelay(80);

        bLineState_B = INREG8(USBPHYRegs+0x72);
        //dbg_print("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
        if(bLineState_B & 0x40)
        {
            ret = STANDARD_CHARGER;
            //dbg_print("mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
            dbg_print("STANDARD CHARGER!\r\n");
        }
        else
        {
            ret = CHARGING_HOST;
            //dbg_print("mt_charger_type_detection : step B : Charging Host!\r\n");
            dbg_print("Charging Host!\r\n");
        }
    }
    else
    {
        /********* Step C *************************************/
        //dbg_print("mt_charger_type_detection : step C\r\n");

        SETREG16(CHR_CON_10,0x0010); //RG_BC11_IPU_EN[1:0]=01
        SETREG16(CHR_CON_10,0x0001);//RG_BC11_CMP_EN[1.0] = 01
        mdelay(80);

        bLineState_C = INREG16(CHR_CON_10);
        //dbg_print("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
        if(bLineState_C & 0x0200)
        {
            ret = NONSTANDARD_CHARGER;
            //dbg_print("mt_charger_type_detection : step C : UNSTANDARD CHARGER!\r\n");
            dbg_print("UNSTANDARD CHARGER!\r\n");
        }
        else
        {
            ret = STANDARD_HOST;
            //dbg_print("mt_charger_type_detection : step C : Standard USB Host!\r\n");
            dbg_print("Standard USB Host!\r\n");
        }
    }
    /********* Finally setting *******************************/
    CLRREG16(CHR_CON_9,0x0003);//RG_BC11_VSRC_EN[1:0]=00
    CLRREG16(CHR_CON_10,0x0040);//RG_BC11_VREF_VTH = 0
    CLRREG16(CHR_CON_10,0x0003);//RG_BC11_CMP_EN[1.0] = 00
    CLRREG16(CHR_CON_10,0x0030);//RG_BC11_IPU_EN[1.0] = 00
    CLRREG16(CHR_CON_10,0x000C);//RG_BC11_IPD_EN[1.0] = 00
    CLRREG16(CHR_CON_10,0x0080);//RG_BC11_BIAS_EN=0

    CLRREG8(USB_U2PHYACR3_2,0x04); //USB_U2PHYACR3_2[2]=0

    //step4:done, ret the type
    return ret;

}

void hw_set_cc(int cc_val)
{
	unsigned int CHR_CON_0 = 0x7002FA00;
    unsigned int CHR_CON_2 = 0x7002FA08;

    switch(cc_val){
        case 70:			
			CLRREG16(CHR_CON_2,0x0700);
        	SETREG16(CHR_CON_2,0x1700);
	        SETREG16(CHR_CON_0,0x1800);	
			break;
	    case 450:
			CLRREG16(CHR_CON_2,0x0700);
        	SETREG16(CHR_CON_2,0x1400);
	        SETREG16(CHR_CON_0,0x1800);	
			break;
		case 650:
			CLRREG16(CHR_CON_2,0x0700);
        	SETREG16(CHR_CON_2,0x1200);
	        SETREG16(CHR_CON_0,0x1800);	
			break;	
		default:
			dbg_print("hw_set_cc: argument invalid!!\r\n");
			break;
	}

}
