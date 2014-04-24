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

/*************************************
*
* Filename:
* ---------
*     mt65xx.h
*************************************/
#ifndef MT65XX_H
#define MT65xx_H

#include "preloader.h"
#include "uart.h"
#include "nand.h"
#include "pll.h"
#include "nand.h"
#include "boot_device.h"

#include "mt6573.h"
#include "mt6573_gpio.h"
#include "mt6573_typedefs.h"
#include "mt6573_i2c.h"
#include "mt6573_rtc.h"
#include "mt6573_download.h"
#include "mt6573_usbd.h"
#include "mt6573_usbtty.h"
#include "mt6573_pmic6326_hw.h"
#include "mt6573_pmic6326_sw.h"
#include "mt6573_partition.h"
#include "mt6573_download.h"
#include "mt6573_timer.h"
#include "mt6573_bat.h"
#include "mt6573_pmu_sw.h"
#include "mt6573_key.h"
#include "mt6573_buffer.h"

#include "cust_sec_ctrl.h"
#include "sec.h"

#include "boot_mode.h"

#include "cust.h"


#define MIDDLE_INIT_ERROR        0x1
#define WDT_INIT_ERROR           0x2
#define PMU_INIT_ERROR           0x4
#define TIMER_INIT_ERROR         0x8
#define PLL_INIT_ERROR           0x10
#define UART_INIT_ERROR          0x20
#define MEM_INIT_ERROR           0x40
#define GPIO_INIT_ERROR          0x80
#define WDT_INTERNAL_ERROR  0x100
	
typedef U32     (*STORGE_READ)(u8 *buf, u32 start, u32 img_size);

typedef struct {
	u32 page_size;
	u32 pktsz;
} device_info_t;

typedef struct {
        part_t *part;
        device_info_t *device_info;
	u32 device_id;
	u32 part_start_addr;
    	STORGE_READ  storge_read;	
}storge_device_t;

#if 0
typedef enum {
    BR_POWER_KEY    = 0,
    BR_USB          = 1,
    BR_RTC          = 2,
    BR_UNKNOWN      = 3,
} SYS_BOOT_REASON;
#endif
#endif








