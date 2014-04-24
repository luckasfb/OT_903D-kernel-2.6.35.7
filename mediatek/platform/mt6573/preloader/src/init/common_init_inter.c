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

/************************
*
* Filename:
* ---------
*    common_init_inter.c 
*************************/
#include "mt65xx.h"
#include "cust_kpd.h"
#include "mmc_common_inter.h"

DM_USBDL_FLOW g_usbdl_flow;
u32 g_uboot_size;
void register_storage_device(void);
/*******************
*  EXTERNAL VARIABLES
********************/
extern part_t mt6573_parts[];
extern void security_init(void);
extern storge_device_t use_device;
/*******************
*  MACRO
********************/
#define MOD                         "INIT"

/*******************
*  SEC DEBUG FLAG
********************/
#define PART_DEBUG_LOG              0
#define PART_DEBUG_LOADING_UBOOT    1

/*******************
*  DEBUG DEFINITION
********************/
#if PART_DEBUG_LOG
#define PART_LOG                    dbg_print
#else
#define PART_LOG
#endif

void primary_sys_init(void)
{
    //pl_wdt_init();
}

static bool middle_sys_init(void)
{
    DRV_WriteReg32 (0x70026308, 0x80);
    return true;
}

static void check_init_error_code(u32 value)
{
    if(value)
    {
        dbg_print("[Error]: (0x%x)\n",value); 
        ASSERT(0);
    }
    else
    {
        dbg_print ("\n[system_init] success!!\n\n");
    }
}

static void set_jtck_low_power(void)
{
    mt_set_gpio_mode(GPIO192, GPIO_MODE_01);
    mt_set_gpio_dir(GPIO192, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO192, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO192, GPIO_PULL_UP);

    mt_set_gpio_mode(GPIO193, GPIO_MODE_01);
    mt_set_gpio_dir(GPIO193, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO193, GPIO_PULL_ENABLE);

    mt_set_gpio_pull_select(GPIO193, GPIO_PULL_DOWN);
    mt_set_gpio_pull_select(GPIO193, GPIO_PULL_UP);
    mt_set_gpio_pull_select(GPIO193, GPIO_PULL_DOWN);
}
// ------------------------------------------------ 
// system module init at first time
// ------------------------------------------------
int pl_sys_init (void)
{
    int ret = false;
    u32 error_code = 0x0;
        
    ret = middle_sys_init();
    if(!ret)
    {
        error_code |= MIDDLE_INIT_ERROR; 
    }

    // init PMU
    ret = pl_pmu_init();
    if(!ret)
    {
        error_code |= PMU_INIT_ERROR;
    }

    // init Timer    
    ret = pl_timer_init();
    if(!ret)
    {
        error_code |= TIMER_INIT_ERROR;
    }

    // init PLL  
    ret = pl_pll_init();
    if(!ret)
    {
        error_code |= PLL_INIT_ERROR;
    }

    // init UART 
    ret = pl_uart_init();
    if(!ret)
    {
        error_code |= UART_INIT_ERROR;
    }

#ifndef CFG_UART_HANDSHAKE_DISABLE
    // send ready to meta tool
    meta_send_ready();
#endif

    // init wdt internal
    ret = pl_wdt_internal_init();
    if (!ret)
    {
        error_code |= WDT_INTERNAL_ERROR;
    }
    
    // init wdt (watch dog timer)
    ret = pl_wdt_init();
    if(!ret)
    {
        error_code |= WDT_INIT_ERROR;
    }

    // init mem (NAND,DRAM) 
    mt6573_mem_init();

    // init GPIO
    mt6573_gpio_init();

    check_init_error_code(error_code);

    set_jtck_low_power();

    register_storage_device();
}
// ------------------------------------------------ 
// storage device common Interface - Read Function
// ------------------------------------------------ 
u32 read_device_data (u8 *store_buf, u32 start, u32 img_size)
{
    u32 chksm = 0;
    
    if(CFG_STORAGE_DEVICE == EMMC)
    {
        chksm = mmc_read_data(store_buf, start, img_size);
    }
    else
    {
        chksm = nand_read_data(store_buf, start, img_size);
        //add other storage device read
    }
    return chksm;
}

void register_storage_device(void)
{       
    if(CFG_STORAGE_DEVICE == EMMC)
    {
        mmc_device_init();
        use_device.device_info = get_storge_info(); /* FIXME */
        use_device.storge_read = read_device_data;
    }
    else
    {
	nand_device_init();
        use_device.device_info = get_storge_info();
        use_device.storge_read = read_device_data;
    }
}

// ------------------------------------------------ 
// detect boot reason
// ------------------------------------------------ 
SYS_BOOT_REASON g_boot_reason = BR_UNKNOWN;

SYS_BOOT_REASON check_boot_reason(void)
{
    PL_TRACER;

    if (rtc_boot_check()) {
        dbg_print("RTC Boot !\n");
        g_boot_reason = BR_RTC;
        PL_TRACER;
        return BR_RTC;
    }

    /* check power key */
    if (key_boot_detect(8)) {
        dbg_print("Power Key Boot !\n");
        rtc_mark_bypass_pwrkey();
        g_boot_reason = BR_POWER_KEY;
        PL_TRACER;
        return BR_POWER_KEY;
    }

#ifndef EVB_PLATFORM
    if (PMU_IsUsbCableIn()) {
        dbg_print("Charger Exist !\n");
        g_boot_reason = BR_USB;
        PL_TRACER;
        return BR_USB;
    }

    dbg_print("\nNo Charger !\n");
    g_boot_reason = BR_UNKNOWN;
    pl_power_off();

    /* should nerver be reached */

    PL_TRACER;
#endif

    return BR_POWER_KEY;
}

bool check_uart_switch_keys(void)
{
    /* FIXME: Use dl keys temporarily since switch keys isn't provided currently */
    if (mt6573_detect_key (KPD_DL_KEY1) &&
        mt6573_detect_key (KPD_DL_KEY2) && mt6573_detect_key (KPD_DL_KEY3)) {
        dbg_print ("uart switch keys are pressed\n");
        return true;
    }
    return false;
}

void pl_check_boot_reason(void)
{
    SYS_BOOT_REASON reason;

    reason = check_boot_reason ();
    if (reason == BR_RTC || reason == BR_POWER_KEY || reason == BR_USB)
    {
        dbg_print ("Pull PWRBB HIGH\n\n");
        rtc_bbpu_power_on ();
    }

    if (check_uart_switch_keys()) {
        U16 owner = 1; /* 0: No log, 1: AP log, 2: MD log, 3: invalid(default) */        
        rtc_rdwr_uart_bits(&owner);
    }
}

void pl_security_init(void)
{
}

void pl_operate_clock(void)
{
    DRV_WriteReg32 (0x700B0500, 0x1);
}

void pl_security_check(void)
{
#ifndef MTK_EMMC_SUPPORT    
    sec_lib_init();
    sec_boot_check();
#endif
}
void pl_do_other_function(void)
{
    //mt6573_rosc_characterization();        
    device_APC_dom_setup();
}

// ------------------------------------------------
//  Tool Detection via USB
// ------------------------------------------------
int meta_detection_via_usb(void) 
{           
    int ret = 0;
    usb_phy_init();

    g_usbdl_flow.enum_timeout_enable = TRUE;
    g_usbdl_flow.handshake_timeout_enable = TRUE;

    if (g_boot_reason == BR_USB || TRUE == PMU_IsUsbCableIn ())
    {
        dbg_print ("PMU - cable in\n");
        tool_detection_via_usb ();
    }   
}
// ------------------------------------------------
//  Tool Detection via UART
// ------------------------------------------------
void meta_detection_via_uart (void)
{

    // META  Tool - META Mode
    if (g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT)
    {   
        //dbg_print ("\n%s : aleady detected meta tool!\n\n",MOD); 
    }
    else
    {
        if (FALSE == meta_check_pc_trigger ())
        {
            dbg_print ("%s : cannot detect meta tool!\n",MOD); 
            //dbg_print ("%s : META Tool handshake fail!\n\n",MOD);
        }    
    }	
}      
#ifdef PRINT_PARTITION_MAP 
// ------------------------------------------------
//  print storge partition base info
// ------------------------------------------------
void part_dump_map (void)
{
    u32 index = 0, part_offset = 0;
    switch(use_device.device_id)
    {
        case NAND_DEVICE:
            use_device.part = get_device_part();
            break;
        default:
            break;
    }

    while (use_device.part->flags != PART_FLAG_END)
    {
        // get partition information              
        PART_LOG ("[%s] %s (0x%x)\n", MOD, use_device.part->name, part_offset);
        part_offset += LOGICAL_ADDR (use_device.part->pgnum);

        // increment index
        index++;
        use_device.part = &mt6573_parts[index];
    }
}
#endif
// ------------------------------------------------
//  get partition info by partition name
// ------------------------------------------------
part_t *get_partition_info (char *name)
{
    part_t *part = NULL;
    part = mt6573_part_get_partition (PART_UBOOT);
    use_device.part_start_addr = part->startblk * use_device.device_info->page_size;
    return part;
}
// ------------------------------------------------
// check nand partition
// ------------------------------------------------
BOOL check_partition_info (part_t *part, U32 magic_number)
{

    u32 info_addr = 0;
    part_hdr_t *uboot_hdr = NULL;
    u8 *buf = (u8 *) g_common_buf;

    info_addr = use_device.part_start_addr;
    /* read header from NAND */
    use_device.storge_read(buf, info_addr, 512); /* FIXME */

    uboot_hdr = (part_hdr_t *) (buf);
    
    /* read partition header */
    //dbg_print ("[%s] u-boot magic number : 0x%x\n", MOD, uboot_hdr->info.magic);
    
    /* read partition name */
    uboot_hdr->info.name[31] = '\0';    //append end char
    dbg_print ("[%s] u-boot name : %s\n", MOD, uboot_hdr->info.name);
    
    /* read partition size */
    dbg_print ("[%s] u-boot size : %d\n", MOD, uboot_hdr->info.dsize);
    g_uboot_size = uboot_hdr->info.dsize;

    /* check partition magic */
    if (uboot_hdr->info.magic != magic_number)
    {
        //dbg_print ("\n[%s] u-boot magic error\n", MOD);
        return FALSE;
    }
    
    /* check partition magic */
    if (strcmp (uboot_hdr->info.name, PART_UBOOT))
    {
        //dbg_print ("\n[%s] u-boot name error\n", MOD);
        return FALSE;
    }

    return TRUE;
}


// ------------------------------------------------
// load partition data to DRAM
// ------------------------------------------------

u32 load_partition_image (part_t *part, U32 mem_addr, u32 img_size)
{
    u32 start = 0, read_length = 0;
    BOOL decrypt_img;

    start = use_device.part_start_addr;
//    read_length = part->pgnum * use_device.device_info->page_size;

#if PART_DEBUG_LOADING_UBOOT
    u32 original_mem_addr = mem_addr;
    u32 mem_chksm = 0;
#endif

    use_device.storge_read((u8 *)mem_addr, start, img_size);
}
// ====================
// Load u-boot to DRAM
void pl_load_uboot_image (void)
{
    use_device.part = get_partition_info (PART_UBOOT);
    if (!use_device.part)
    {   
        dbg_print ("u-boot not found\n");
        ASSERT (0);
    }   
    if (check_partition_info (use_device.part, PART_MAGIC) == FALSE)
    {   
        dbg_print ("wrong u-boot header !\n");
        ASSERT (0);
    }
    // notes : part_load will read the u-boot header
    BUG_ON(UBOOT_IMG_ADDR < UBOOT_HEADER_SIZE);
    load_partition_image (use_device.part, UBOOT_IMG_ADDR - UBOOT_HEADER_SIZE, g_uboot_size + UBOOT_HEADER_SIZE);
}
