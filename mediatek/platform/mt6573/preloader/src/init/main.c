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

/*******************************************************************************
*
* Filename:
* ---------
*      main.c 
 **************************/
#include "mt65xx.h"

#define MOD "PRELOADER"

// ==============================================
// MACRO
// ==============================================
#define RSA_MEM_HEAP_SIZE             (4096)
#define RECOVERY_MODE                 (0x0030)
#define MAIN_DBG_LOG                  (FALSE)

#if MAIN_DBG_LOG
#define MAIN_LOG                      dbg_print
#else
#define MAIN_LOG
#endif

// ==============================================
// GLOBAL VARIABLES
BOOTMODE g_boot_mode = NORMAL_BOOT;
BOOT_ARGUMENT g_boot_argument;
storge_device_t use_device;

// ====================
// print title
void pl_print_title (char *s)
{
	dbg_print ("\n[%s] %s\n", MOD, s);
	dbg_print ("---------------------------\n");
}

// ==============================================
// delay macro
// ==============================================
void delay (unsigned int loops)
{
	unsigned int i = 0;
	for (i = 0; i < loops; i++);
}
// ==============================================
// post process
static void pl_post_process (void)
{
	g_boot_argument.maggic_number = BOOT_ARGUMENT_MAGIC;

	dbg_print ("\nboot mode = 0x%x\n", g_boot_mode);
    g_boot_argument.e_flag = 0x0;
	g_boot_argument.boot_mode = g_boot_mode;
    g_boot_argument.e_flag = sp_check_platform();

    dbg_print ("\nboot e_flag = 0x%x\n", g_boot_argument.e_flag);
	memcpy (BOOT_ARGUMENT_LOCATION, &g_boot_argument,sizeof (g_boot_argument));    
}


// ==============================================
// main function
// ==============================================
void C_Main (void)
{

	SYS_BOOT_REASON reason;
	g_boot_mode = NORMAL_BOOT;

	// pre-process
	pl_sys_init();   

	// check boot reason
	pl_check_boot_reason();

	//boot security tactics
	pl_security_init();

	// clear SGX core clock
	pl_operate_clock();    

	// check cable in or not and charge type
	meta_detection_via_usb();

#ifndef CFG_UART_HANDSHAKE_DISABLE
	//via UART to detect tool 
	meta_detection_via_uart ();  
#endif

	pl_security_check();

	// post process
	pl_post_process ();

	// do other function
	pl_do_other_function();
    
	// load u-boot image to DRAM       
	pl_load_uboot_image ();

	// jump to DRAM to run u-boot image        
	pl_print_title ("Jump U-Boot");
	JumpCmd ((UBOOT_IMG_ADDR));

	while (1);
}
