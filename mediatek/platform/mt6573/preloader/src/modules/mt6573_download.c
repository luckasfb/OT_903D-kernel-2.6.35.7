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
#include "nand_common_inter.h"

#ifdef CFG_USB_DOWNLOAD

#include "nand.h"
#include "boot_device.h"
#include "mt6573_usbtty.h"
#include "mt6573_wdt_hw.h"
#include "mt6573_download.h"
#include "mt6573_partition.h"
#include "mt6573_pmic6326_hw.h"
#include "mt6573_pmic6326_sw.h"
#include "mt6573_rtc.h"
#include "pmt.h"
#include "bmt.h"
#define MOD                 "<DM>"


/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/
#if DM_DBG_LOG
#define DM_ASSERT(expr)      {  if ((expr)==FALSE){  \
    dbg_print("%s : [ASSERT] at %s #%d %s\n       %s\n       above expression is not TRUE\n", MOD, __FILE__, __LINE__, __FUNCTION__, #expr); \
    while(1){};  }                    \
                             }
#define DM_STATE_LOG(state)  dbg_print("%s : state %s\n", MOD, (state))
#define DM_ENTRY_LOG()       dbg_print("%s : enter %s\n", MOD, __FUNCTION__)
#define DM_LOG               dbg_print
#else
#define DM_ASSERT(expr)
#define DM_STATE_LOG(state)
#define DM_ENTRY_LOG()
#define DM_LOG
#endif

#define DM_NAND_ECC_PARITY_SZ(pagesz, sparesz)      ((sparesz) - ((((pagesz)>>9)*8)-4))
#define DM_NAND_NO_PARITY_PKTSZ(pagesz)             ((pagesz)  + ((((pagesz)>>9)*8)-4))

#ifdef MT6573_CFG_USB_DOWNLOAD
/**************************************************************************
*  LOCAL VARIABLE DECLARATION
**************************************************************************/
DM_CONTEXT dm_ctx = { 0 };

/**************************************************************************
*  GLOBAL VARIABLE DECLARATION
**************************************************************************/
u32 g_dl_safe_start_addr;
char dm_rx_buf[DM_BUF_MAX_SIZE];
BOOL g_cust_key_init;
BOOL g_end_user_flash_tool;
BOOL g_sec_img_patch_enable;
BOOL g_sec_img_patch_valid;

DM_PARTITION_INFO_PACKET g_img_dl_pt_info = {0};

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
extern int nand_curr_device;
extern u32 nand_maf_id;
extern u32 nand_dev_id;
extern struct nand_chip g_nand_chip;
extern int usbdl_init (void);

#if DM_TIME_ANALYSIS
u32 g_DM_TIME_Tmp;
u32 g_DM_TIME_ReadData;
u32 g_DM_TIME_FlashData;
u32 g_DM_TIME_Erase;
u32 g_DM_TIME_USBGet;
u32 g_DM_TIME_SkipBadBlock;
u32 g_DM_TIME_Checksum;
u32 g_DM_TIME_Total_Begin;
u32 g_DM_TIME_Total;

#define DM_TIME_BEGIN \
    g_DM_TIME_Tmp = get_timer(0);

#define DM_TIME_END_NAND_READ \
    g_DM_TIME_ReadData += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_NAND_WRITE \
    g_DM_TIME_FlashData += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_NAND_ERASE \
    g_DM_TIME_Erase += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_NAND_BAD_BLOCK \
    g_DM_TIME_SkipBadBlock += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_USB_READ \
    g_DM_TIME_USBGet += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_CHECKSM \
    g_DM_TIME_Checksum += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_TOTAL_BEGIN \
    g_DM_TIME_Total_Begin = get_timer(0);

#define DM_TIME_TOTAL_END \
    g_DM_TIME_Total = get_timer(g_DM_TIME_Total_Begin);

void dump_time_analysis (void)
{
    dbg_print ("\n%s : --------------------------------------\n", MOD);
    dbg_print ("%s : [TIME] NAND erase takes %d ms \n", MOD, g_DM_TIME_Erase);
    dbg_print ("%s : [TIME] NAND flash takes %d ms \n", MOD, g_DM_TIME_FlashData);
    dbg_print ("%s : [TIME] NAND skip bad block takes %d ms \n", MOD, g_DM_TIME_SkipBadBlock);
    dbg_print ("%s : [TIME] USB read takes %d ms \n", MOD, g_DM_TIME_USBGet);
    dbg_print ("%s : [TIME] DM check sum takes %d ms \n", MOD, g_DM_TIME_Checksum);
    dbg_print ("%s : --------------------------------------\n", MOD);
    dbg_print ("%s : [TIME] DM total takes %d ms \n", MOD, g_DM_TIME_Total);
    dbg_print ("%s : --------------------------------------\n\n", MOD);
}
#else
#define DM_TIME_BEGIN
#define DM_TIME_END_NAND_READ
#define DM_TIME_END_NAND_WRITE
#define DM_TIME_END_NAND_ERASE
#define DM_TIME_END_NAND_BAD_BLOCK
#define DM_TIME_END_USB_READ
#define DM_TIME_END_CHECKSM
#define DM_TIME_TOTAL_BEGIN
#define DM_TIME_TOTAL_END
#endif

void handle_lock_cmd (void);
void handle_pt_cmd (void);

//======================================================================
//  fill the internal imgp data structure
//======================================================================
void fill_internal_imgp (DM_IMAGE_INFO_PACKET *imgp, DM_EXT_IMAGE_INFO_PACKET *ext_imgp)
{
    imgp->pattern = ext_imgp->pattern;
    imgp->img_info.img_format = ext_imgp->img_info.img_format;
    imgp->img_info.img_type = ext_imgp->img_info.img_type;    
    imgp->img_info.img_size = ext_imgp->img_info.img_size;       
    imgp->img_info.addr_off = ext_imgp->img_info.addr_off;  

#ifdef GET_IMG_BOUNDARY_FROM_TOOL
    // v1.1050.2 flash tool will send the partition information 
    // to pre-loader for download boundary check
    // * if GET_IMG_BOUNDARY_FROM_TOOL is defined,
    // * please check the flash tool version (must greater than v1.1050.2)
    imgp->img_info.addr_boundary = ext_imgp->img_info.addr_boundary;       
    //dbg_print ("get image boundary (0x%x) from tool\n",imgp->img_info.addr_boundary);    
#else    
    imgp->img_info.addr_boundary = imgp->img_info.addr_off + get_part_range (imgp->img_info.img_type);  
    //dbg_print ("get image boundary (0x%x) from partition table\n",imgp->img_info.addr_boundary);    
#endif    

    if (imgp->img_info.img_type == DM_IMG_TYPE_USRDATA)
    {   imgp->img_info.addr_boundary = g_nand_chip.chipsize;
    }

    imgp->img_info.pkt_size = ext_imgp->img_info.pkt_size;      

}


/**************************************************************************
*  handle nand flash erase
**************************************************************************/
u32 force_erase (DM_IMG_INFO * img_info, u32 pktsz)
{

    DM_TIME_BEGIN;
    // erase whole partition to keep data consistent
    {
        u32 erase_limit = 0;
        if (img_info->img_type == DM_IMG_TYPE_USRDATA)
        {
            erase_limit = g_nand_chip.chipsize;
            //dbg_print ("%s : erase limit address (0x%x)\n", MOD,erase_limit);
            //dbg_print ("%s : user data partition ! erase 0x%x ~ 0x%x\n",  MOD, img_info->addr_off, g_nand_chip.chipsize);
            if (FALSE ==  g_dev_vfunc.dev_erase_data (img_info->addr_off, g_nand_chip.chipsize, g_nand_chip.chipsize - img_info->addr_off))
            {
                DM_ASSERT (0);
                dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
                dm_ctx.dm_err = DM_ERR_NAND_ER_FAIL;
            }
        }
        else
        {
            erase_limit = dm_ctx.img_info.addr_off + dm_ctx.part_range;
            //dbg_print ("%s : erase limit address (0x%x)\n", MOD, erase_limit);
            if (FALSE == g_dev_vfunc.dev_erase_data (img_info->addr_off, erase_limit, dm_ctx.part_range))
            {
                DM_ASSERT (0);
                dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
                dm_ctx.dm_err = DM_ERR_NAND_ER_FAIL;
            }
        }
    }
    DM_TIME_END_NAND_ERASE;
}


/**************************************************************************
*  handle nand flash erase
**************************************************************************/
u32 handle_erase (DM_IMG_INFO * img_info, u32 pktsz)
{
    if ((FALSE == g_end_user_flash_tool) || (FALSE == g_sec_img_patch_enable))
    {
        return;
    }

    force_erase (img_info, pktsz);
}


/**************************************************************************
/* flag is to indicate to handle normal sequence data or error flow data
*      flag = TRUE  :  handle normal data, receive and flash it into nand
*      flag = FALSE :  handle error flow data, receive it bu just skip it
/**************************************************************************/

void handle_data (u32 pktsz, u8 * buf)
{
    bool res = TRUE;
    unsigned int i = 0;
    u32 starting_block = 0;
    u32 spare_start, spare_offset, spare_len;
    bool first_page = TRUE;
    bool need_erase_nand = TRUE;
    bool invalid_addr = FALSE;

    // set image patch invalid
    g_sec_img_patch_valid = FALSE;
    DM_ENTRY_LOG ();

    //*****************************************************
    //* ERR_ONGOING : error occurs, just receive data from usb and skip it
    if (dm_ctx.dm_status == DM_STATUS_ERR_ONGOING)
    {
        while (dm_ctx.curr_cnt <= dm_ctx.pkt_cnt)
        {
            mt6573_usbtty_getcn (pktsz, buf);
            dm_ctx.curr_cnt++;
        };
        dm_ctx.dm_status = DM_STATUS_ERR_FINISHED;
        return;
    }

    //*****************************************************
    //* Make sure the starting block is good
    starting_block = g_dev_vfunc.dev_find_safe_block (dm_ctx.page_off);
    if (dm_ctx.page_off != starting_block)
    {
        dm_ctx.page_off = starting_block;
    }

    //*****************************************************
    // * SECT_ONGOING : dm_ctx.dm_status == DM_STATUS_SECT_ONGING
    do
    {
        //*****************************************************
        //* (1) fill USB buffer
        DM_TIME_BEGIN;
        mt6573_usbtty_getcn (pktsz, buf);
        DM_TIME_END_USB_READ;


        //*****************************************************
        //* (2) cal check sum of received buffer
#if (DM_CAL_CKSM_FROM_USB_BUFFER || DM_DBG_LOG)
#if DM_CAL_CKSM_FROM_USB_BUFFER
        DM_TIME_BEGIN;
        cal_chksum_per_pkt (buf, pktsz);
        DM_TIME_END_CHECKSM;
#endif
#endif

        //=============================
        // (3) check image boundary 
        //     always check image boundary at begining to ensure that
        //     "won't write any data to next partition"
        //=============================         
        if (dm_ctx.page_off >= dm_ctx.img_info.addr_boundary)
        {   
            //dbg_print ("current page_off (0%x) >= addr_boundary (0x%x)\n", dm_ctx.page_off, dm_ctx.img_info.addr_boundary);            
            invalid_addr = TRUE;            

        }        

        // if addr is invalid, skip the nand writing process
        if (invalid_addr == TRUE)
        {   goto _next;
        }


        if (TRUE == need_erase_nand)
        {
            //******************
            //* erase nand flash
            handle_erase (&dm_ctx.img_info, pktsz);
            need_erase_nand = FALSE;
        }

        //*****************************************************
        //* (4) skip NAND programing when image type is authentication file
        if (dm_ctx.img_info.img_type != DM_IMG_TYPE_AUTHEN_FILE)
        {

            //*****************************************************
            //* when the address is block alignment, check if this block is good
            DM_TIME_BEGIN;
            if (dm_ctx.page_off % BLOCK_SIZE == 0)
            {   dm_ctx.page_off = g_dev_vfunc.dev_find_safe_block (dm_ctx.page_off);
            }
            DM_TIME_END_NAND_BAD_BLOCK;

            //*****************************************************
            //* flash NAND
            DM_TIME_BEGIN;
            g_dev_vfunc.dev_write_data (buf, dm_ctx.page_off) ;
            DM_TIME_END_NAND_WRITE;
        }

_next:

        //*****************************************************
        //* (5) update the latest safe nand addr
        g_dl_safe_start_addr = dm_ctx.page_off;

        //*****************************************************
        //* (6) increase must after flash data
        dm_ctx.curr_cnt++;
        dm_ctx.curr_off += pktsz;
        dm_ctx.page_off += dm_ctx.page_size;

        //dbg_print ("dm_ctx.page_off = 0x%x\n",dm_ctx.page_off);
        delay (1000);

    }
    while ((dm_ctx.curr_cnt <= dm_ctx.pkt_cnt) && (dm_ctx.dm_status == DM_STATUS_SECT_ONGING));
    dm_ctx.dm_status = DM_STATUS_SECT_FINISHED;

    return;
}


/**************************************************************************
*  cal checksum
**************************************************************************/
void handle_cksm (void)
{
    u32 u4cksum = 0;
    DM_CHKSUM_PACKET cksm = { DM_CHKSUM_PKT_PATN, 0 };

    DM_ENTRY_LOG ();

    //*****************************************************
    //* method 1 : calculate the image checksum from USB received buffer to 
    //* verify USB connection
#if DM_CAL_CKSM_FROM_USB_BUFFER
    //dbg_print ("%s : <CHKSUM> : calculate image checksum of USB received buffer .. ", MOD);
    dbg_print ("<CHKSUM> : USB buffer .. ", MOD);
    cksm.chk_sum = dm_ctx.chk_sum;
    dbg_print ("0x%x (=%d)\n", cksm.chk_sum, cksm.chk_sum);
#endif

    if (dm_ctx.img_info.img_type != DM_IMG_TYPE_AUTHEN_FILE)
    {
        //*****************************************************
        //* method 2 : calculate the image checksum from NAND flash to 
        //* verify R/W functions of NAND flash
#if DM_CAL_CKSM_FROM_NAND_FLASH
        //dbg_print ("%s : <CHKSUM> : calculate image checksum of NAND flash .. ", MOD);
        dbg_print ("<CHKSUM> : NAND flash .. ", MOD);
        u4cksum = g_dev_vfunc.dev_chksum_per_file (dm_ctx.img_info.addr_off, dm_ctx.img_info.img_size);
        dbg_print ("0x%x (=%d)\n", u4cksum, u4cksum);
#if DM_CAL_CKSM_FROM_USB_BUFFER
        if (u4cksum != cksm.chk_sum)
        {
            dbg_print ("ERROR : USB buffer != NAND flash content\n",MOD);
            ASSERT (0);
        }
#endif
        cksm.chk_sum = u4cksum;
#endif

    }

    mt6573_usbtty_putcn (DM_SZ_CHK_SUM_PKT, (u8 *) & cksm, TRUE);

    //*****************************************************
    //*  verify authentication file
    //*****************************************************
    if (dm_ctx.img_info.img_type == DM_IMG_TYPE_AUTHEN_FILE)
    {
        dm_ctx.dm_status = DM_STATUS_AUTH_VERIFY;
    }
    else if (TRUE == g_sec_img_patch_enable)
    {
        dm_ctx.dm_status = DM_STATUS_PATCH_VERIFY;
    }
    else
    {
        dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT;
    }

    return;
}

/**************************************************************************
*  check image information
**************************************************************************/
u32 check_imgp (DM_IMAGE_INFO_PACKET * imgp, u32 * pktsz)
{

    dbg_print ("\n%s : ------------------ IMG ---------------\n", MOD);
    dbg_print ("%s : IMG fmt    = %s   \t type = %s\n", MOD, get_img_fmt (imgp->img_info.img_format),  get_img_type (imgp->img_info.img_type));
    dbg_print ("%s : IMG sz     = 0x%x \t addr = 0x%x\n", MOD,(imgp->img_info.img_size), (imgp->img_info.addr_off));
    dbg_print ("%s : addr       = 0x%x \t boundary = 0x%x\n", MOD, (imgp->img_info.addr_off), (imgp->img_info.addr_boundary));
    dbg_print ("%s : pkt sz     = 0x%x \n", MOD, (imgp->img_info.pkt_size));
    //dbg_print ("%s : --------------------------------------\n", MOD);
    dbg_print ("%s : IMG range  = 0x%x ~ 0x%x\n", MOD, (imgp->img_info.addr_off), (imgp->img_info.addr_off) + (imgp->img_info.img_size));
    //dbg_print ("%s : --------------------------------------\n", MOD);

    // image type is end user flash tool inform
    if (DM_IMG_TYPE_EU_FT_INFORM == imgp->img_info.img_type)
    {
        //g_end_user_flash_tool = TRUE;
        dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT;
        return;
    }


    // image type is partition table inform
    if (DM_IMG_TYPE_PT_TABLE_INFORM == imgp->img_info.img_type)
    {
        dbg_print ("partition table inform\n");
        mt6573_usbtty_puts (DM_STR_START_REQ);
        handle_pt_cmd ();
        return;
    }
    
    // image type isn't authentication file
    // no need to check image boundary 
    // if current image is authentication file but the authentication file 
    // must smaller then USB transmission buffer size (0x840)
    if (DM_IMG_TYPE_AUTHEN_FILE != imgp->img_info.img_type)
    {
        // check image boundary  
        if (g_dl_safe_start_addr > imgp->img_info.addr_off)
        {
            //dbg_print ("can't flash this image !\n");
            //dbg_print ("current safe starting address for flashing (0x%x) > current image address (0x%x)\n", g_dl_safe_start_addr, imgp->img_info.addr_off);
            ASSERT (0);
        }

        DM_LOG ("%s : cur safe start addr for flashing (0x%x)\n", MOD, g_dl_safe_start_addr);
        g_dl_safe_start_addr = imgp->img_info.addr_off;
    }

    if (imgp->pattern != DM_IMAGE_INFO_PKT_PATN)
    {
        DM_ASSERT (imgp->pattern == DM_IMAGE_INFO_PKT_PATN);
        dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
        dm_ctx.dm_err = DM_ERR_WRONG_SEQ;
    }
    // start address offset must be block alignment
    else if (imgp->img_info.addr_off % dm_ctx.block_size)
    {
        DM_ASSERT (imgp->img_info.addr_off % dm_ctx.block_size == 0);
        dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
        dm_ctx.dm_err = DM_ERR_WRONG_ADDR;
        *pktsz = save_imgp (imgp);
    }
    // packet size must be page size + spare size
    else if (imgp->img_info.pkt_size != dm_ctx.page_size + dm_ctx.spare_size)
    {
        DM_ASSERT (imgp->img_info.pkt_size == dm_ctx.page_size + dm_ctx.spare_size);
        dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
        dm_ctx.dm_err = DM_ERR_WRONG_PKT_SZ;
        *pktsz = save_imgp (imgp);
    }
    else
    {
        *pktsz = save_imgp (imgp);
        // check if secure image patch is enabled or not
        if ((DM_IMG_TYPE_EU_FT_INFORM == imgp->img_info.img_type) && (TRUE == g_end_user_flash_tool))
        {
            dm_ctx.dm_status = DM_STATUS_PATCH_CMD;
        }
        else
        {
            dm_ctx.dm_status = DM_STATUS_SECT_ONGING;
            DM_TIME_BEGIN;
            // if secure image patch mechaism isn't built-in, erase nand first
            force_erase (&imgp->img_info, *pktsz);
            DM_TIME_END_NAND_ERASE;
        }

    }

    return *pktsz;
}

//======================================================================
//  check partition table info command
//======================================================================
void check_pt_cmd (void)
{
    DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };
    DM_PKT_TYPE pkt_type;
    char buffer[DM_CMD_MAX_SIZE];
    int i;

    dbg_print ("DM_PARTITION_INFO_PACKET\n");
    dbg_print (": pattern %x\n", g_img_dl_pt_info.pattern);
    dbg_print (": part_num = %d\n", g_img_dl_pt_info.part_num);
    for (i = 0; i < PART_MAX_COUNT; i++)
    {
        dbg_print (": part_info[%d].part_name = %s\n", i, g_img_dl_pt_info.part_info[i].part_name);
        dbg_print (": part_info[%d].start_addr = %x\n", i, g_img_dl_pt_info.part_info[i].start_addr);
        dbg_print (": part_info[%d].part_len = %x\n", i, g_img_dl_pt_info.part_info[i].part_len);
        dbg_print (": part_info[%d].part_visibility = %x\n", i, g_img_dl_pt_info.part_info[i].part_visibility);
        dbg_print (": part_info[%d].dl_selected = %x\n", i, g_img_dl_pt_info.part_info[i].dl_selected);
        dbg_print ("\n");
    }
    
    //==================================================
    // check if specified partitions can be downloaded
    //==================================================
    errp.err_code = new_part_tab ((u8 *) &g_img_dl_pt_info);

    set_bmt_reserve_region(SET_RESERVE_SET, g_img_dl_pt_info.part_info[2].start_addr);
    
    mt6573_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);

    if (errp.err_code != DM_ERR_OK)
    {

        dbg_print ("\n%s : the specified partitions can not be downloaded, err_code = %d\n", MOD, errp.err_code);
       
        //================================
        // receive REBOOT packet
        //================================
        reset_dm_descriptor ();
        mt6573_usbtty_getcn (DM_SZ_REBOOT_STR, buffer);
        pkt_type = judge_pkt_type ((const void *) buffer);
        
        if (pkt_type == DM_PKT_REBT)
        {
            //dbg_print ("%s : received REBOOT packet\n", MOD);
            dm_ctx.dm_status = DM_STATUS_REBOOT;
        } 
        else   
        {
            //dbg_print ("%s : do not received REBOOT packet\n", MOD);
            dm_ctx.dm_status = DM_STATUS_START;
        }
    }
    else
    {
        dm_ctx.dm_status = DM_STATUS_START;
    }
}

//======================================================================
//  handle partition table info command
//======================================================================
void  handle_pt_cmd (void)
{
    DM_ENTRY_LOG ();
    mt6573_usbtty_getcn (DM_SZ_PT_INFO_CMD_PKT, (u8 *) & g_img_dl_pt_info);
    check_pt_cmd ();
}

/**************************************************************************
*  handle image 
**************************************************************************/
u32 handle_imgp (u32 * pktsz)
{
    DM_EXT_IMAGE_INFO_PACKET ext_imgp = { 0 };
    DM_IMAGE_INFO_PACKET imgp = { 0 };

    DM_ENTRY_LOG ();
    mt6573_usbtty_getcn (DM_SZ_EXT_IMG_INFO_PKT, (u8 *) & ext_imgp);
    fill_internal_imgp(&imgp , &ext_imgp);

    check_imgp (&imgp, pktsz);
    mt6573_usbtty_puts (DM_STR_START_REQ);
    return *pktsz;
}

/**************************************************************************
*  this function will also return packet size per transmission
**************************************************************************/
u32 save_imgp (DM_IMAGE_INFO_PACKET * imgp)
{
    u32 cnt;
    u32 extra_sz_for_patch_sig = 0;
    u32 extra_pk_cnt_for_patch_sig = 0;
    
    DM_ENTRY_LOG ();
    /* save image information to main descriptor */
    dm_ctx.img_info.img_format = imgp->img_info.img_format;
    dm_ctx.img_info.img_type = imgp->img_info.img_type;
    dm_ctx.img_info.img_size = imgp->img_info.img_size;
    dm_ctx.img_info.addr_off = imgp->img_info.addr_off;
    dm_ctx.img_info.addr_boundary = imgp->img_info.addr_boundary;
    dm_ctx.img_info.pkt_size = dm_ctx.page_size + dm_ctx.spare_size;    //imgp->img_info.pkt_size;

    dm_ctx.curr_cnt = 0;
    dm_ctx.curr_off = dm_ctx.img_info.addr_off;
    dm_ctx.page_off = dm_ctx.img_info.addr_off;


    //************************
    //* no need to check image boundary 
    //* if current image is authentication file
    //************************
    if (dm_ctx.img_info.img_type != DM_IMG_TYPE_AUTHEN_FILE)
    {
#ifdef GET_IMG_BOUNDARY_FROM_TOOL    
        // v1.1050.2 flash tool will send the partition information 
        // to pre-loader for download boundary check
        // * if GET_IMG_BOUNDARY_FROM_TOOL is defined, 
        // * please check the flash tool version (must greater than v1.1050.2)
        dm_ctx.part_range = dm_ctx.img_info.addr_boundary - dm_ctx.img_info.addr_off;
#else
        dm_ctx.part_range = get_part_range (dm_ctx.img_info.img_type);
#endif // #ifdef GET_IMG_BOUNDARY_FROM_TOOL
    }
    else
    {                         
        dm_ctx.part_range = 0x0;
    }


    if (dm_ctx.img_info.img_format == DM_IMG_FORMAT_YAFFS2)
    {
        if ((dm_ctx.img_info.img_size % dm_ctx.img_info.pkt_size))
        {
            dbg_print ("[ERROR] image size not align YAFFS2 format, image size = 0x%x, packet size = 0x%x\n", dm_ctx.img_info.img_size, dm_ctx.img_info.pkt_size);
            //dbg_print ("[ERROR] please check the image format\n");
            ASSERT (0);
        }
        else
        {
            cnt = (dm_ctx.img_info.img_size / dm_ctx.img_info.pkt_size);
        }
        dm_ctx.pkt_cnt = cnt - 1;   /* because curr_cnt count from 0 */
    }
    else                        
    {
        cnt = (dm_ctx.img_info.img_size % dm_ctx.page_size) ?
            (dm_ctx.img_info.img_size / dm_ctx.page_size + 1) : (dm_ctx.img_info.img_size / dm_ctx.page_size);
        dm_ctx.pkt_cnt = cnt - 1;   /* because curr_cnt count from 0 */
    }


#if DM_DBG_LOG
    dump_dm_descriptor ();
#endif


    return dm_ctx.img_info.pkt_size;
}

/**************************************************************************
*  handle flash information sent by flash tool
**************************************************************************/
void handle_pl_info (void)
{
    DM_PL_INFO_PACKET plip = {0};
    DM_ENTRY_LOG ();
    plip.pattern = DM_PL_INFO_PKT_PATN;
    plip.chip = DRV_Reg16 (APHW_CODE);

    // fill _DM_FLASH_INFO according to NAND exported info.         
    plip.flash_info.man_code = nand_maf_id;
    plip.flash_info.dev_id = g_nand_chip.id;
    plip.flash_info.dev_code = nand_dev_id;
    plip.flash_info.dev_size = g_nand_chip.chipsize;
    plip.flash_info.page_size = g_nand_chip.page_size;
    plip.flash_info.spare_size = g_nand_chip.oobsize;
    
    // fill dm descriptor according to NAND exported info.          
    dm_ctx.block_size = g_nand_chip.erasesize;
    dm_ctx.page_size = g_nand_chip.page_size;
    dm_ctx.spare_size = g_nand_chip.oobsize;
    
    DM_LOG ("----------------------------------------\n");
    DM_LOG ("g_nand_chip.page_shift    = %x\n", g_nand_chip.page_shift);
    DM_LOG ("g_nand_chip.page_size     = %x\n", g_nand_chip.page_size);
    DM_LOG ("g_nand_chip.ChipID        = %x\n", g_nand_chip.ChipID);
    DM_LOG ("g_nand_chip.chips_name    = %x\n", g_nand_chip.chips_name);
    DM_LOG ("g_nand_chip.chipsize      = %x\n", g_nand_chip.chipsize);
    DM_LOG ("g_nand_chip.erasesize     = %x\n", g_nand_chip.erasesize);
    DM_LOG ("g_nand_chip.mfr           = %x\n", g_nand_chip.mfr);
    DM_LOG ("g_nand_chip.id            = %x\n", g_nand_chip.id);
    DM_LOG ("g_nand_chip.name          = %x\n", g_nand_chip.name);
    DM_LOG ("g_nand_chip.numchips      = %x\n", g_nand_chip.numchips);
    DM_LOG ("g_nand_chip.oobblock      = %x\n", g_nand_chip.oobblock);
    DM_LOG ("g_nand_chip.oobsize       = %x\n", g_nand_chip.oobsize);
    DM_LOG ("g_nand_chip.eccsize       = %x\n", g_nand_chip.eccsize);
    DM_LOG ("g_nand_chip.bus16         = %x\n", g_nand_chip.bus16);
    DM_LOG ("g_nand_chip.nand_ecc_mode = %x\n", g_nand_chip.nand_ecc_mode);
    DM_LOG ("----------------------------------------\n");

    dm_ctx.img_info.pkt_size = dm_ctx.page_size + dm_ctx.spare_size;
    mt6573_usbtty_putcn (DM_SZ_PL_INFO_PKT, (u8 *) & plip, TRUE);
}

/**************************************************************************
*  handle error
**************************************************************************/
void handle_errp (void)
{
    DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };
    DM_ENTRY_LOG ();
    errp.err_code = dm_ctx.dm_err;
    mt6573_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);
    dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT; //DM_STATUS_REBOOT;
    return;
}

//======================================================================
//  update partition table 
//======================================================================
void  handle_update (void)
{
    DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };

    //==================================================
    // check if specified partitions can be updated
    //==================================================
    errp.err_code = update_part_tab ();
    mt6573_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);
    dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT;
}    

/**************************************************************************
*  reboot state
**************************************************************************/
void handle_reboot (void)
{
    // check if device Is ready to use
    g_dev_vfunc.dev_wait_ready ();
    // check if device Is ready to use
    g_dev_vfunc.dev_wait_ready ();
    DM_TIME_TOTAL_END;
#if DM_TIME_ANALYSIS
    dump_time_analysis ();
#endif
    mt6573_usb_disconnect ();
    do_reboot ();
}

/**************************************************************************
*  auto boot state
**************************************************************************/
void handle_autoboot (void)
{
    // check if device Is ready to use
    g_dev_vfunc.dev_wait_ready ();
    // check if device Is ready to use
    g_dev_vfunc.dev_wait_ready ();

    DM_TIME_TOTAL_END;
#if DM_TIME_ANALYSIS
    dump_time_analysis ();
#endif

    mt6573_usb_disconnect ();

    //--------------------------------------------------------
    // saving the reset status in rtc spare register to bypass power 
    // key check in u-boot
    //--------------------------------------------------------
    rtc_mark_recovery_bits (0x0020);
    dbg_print ("RTC_PDN1 = 0x%x\n", DRV_Reg16 (RTC_PDN1));
    do_reboot ();
}


/**************************************************************************
*  middle state
**************************************************************************/
void handle_midle_state (u8 * buf)
{
    DM_PKT_TYPE pkt_type;
    DM_EXT_IMAGE_INFO_PACKET *ext_imgp;
    DM_PATCH_CMD_PACKET *patch_cmd;
    DM_IMAGE_INFO_PACKET imgp = { 0 };

    u32 pktsz = 0;
    part_t *part;
    u8 *name = NULL;
    
    DM_ENTRY_LOG ();

    reset_dm_descriptor ();

    /* receive a little bit data, check if it is REBOOT */
    mt6573_usbtty_getcn (DM_SZ_REBOOT_STR, buf);
    pkt_type = judge_pkt_type ((const void *) buf);

    if (pkt_type == DM_PKT_REBT)
    {
        //dbg_print ("Reboot after download !!!\n");
        dm_ctx.dm_status = DM_STATUS_REBOOT;
        return;
    }

    if (pkt_type == DM_PKT_AUTO)
    {
        //dbg_print ("autoboot mode !!!\n");
        dm_ctx.dm_status = DM_STATUS_ATBOOT;
        return;
    }
    
    if (pkt_type == DM_PKT_UPDT)
    {
        dbg_print ("update partition table\n");
        dm_ctx.dm_status = DM_STATUS_UPDATE;
        return;
    }
    
    // pcaket type is IMGP
    if (pkt_type == DM_PKT_IMGP)
    {
        //dbg_print ("\nCASE 2 : pcaket type is DM_PKT_IMGP\n");
        mt6573_usbtty_getcn (DM_SZ_EXT_IMG_INFO_PKT - DM_SZ_REBOOT_STR, (u8 *) (buf + DM_SZ_REBOOT_STR));
        ext_imgp = (DM_EXT_IMAGE_INFO_PACKET *) buf;
        fill_internal_imgp (&imgp , &(*ext_imgp));
        check_imgp (&imgp, &pktsz);
        mt6573_usbtty_puts (DM_STR_START_REQ);
    }
    else
    {
        //dbg_print ("\nCASE 3 : others\n");
        if (pkt_type != DM_PKT_IMGP)
        {
            dbg_print ("pkt type = 0x%x\n", pkt_type);
        }
        ASSERT (0);
    }

    return;
}


/**************************************************************************
*  allocate buffer
**************************************************************************/
u8 * prepare_data_buf (void)
{
    dm_ctx.data_buf = (u8 *) dm_rx_buf;

    if (dm_ctx.data_buf)
        return dm_ctx.data_buf;

    DM_ASSERT (dm_ctx.data_buf);

    {
        DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };
        errp.err_code = DM_ERR_NOMEM;   //TODO
        mt6573_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);
        return NULL;
    }
}

/**************************************************************************
*  download main function
**************************************************************************/
void transmission_main (void)
{
    u32 pktsz = 0;
    u8 *buf = NULL;

    handle_pl_info ();
    buf = prepare_data_buf ();

    // init global vars
    g_dl_safe_start_addr = 0;
    g_cust_key_init = FALSE;
    g_end_user_flash_tool = FALSE;
    g_sec_img_patch_enable = FALSE;
    g_sec_img_patch_valid = FALSE;

#if DM_CAL_CKSM_FROM_USB_BUFFER
    dm_ctx.chk_sum = 0;
#endif
#if DM_TIME_ANALYSIS
    g_DM_TIME_Tmp = 0;
    g_DM_TIME_ReadData = 0;
    g_DM_TIME_FlashData = 0;
    g_DM_TIME_Erase = 0;
    g_DM_TIME_USBGet = 0;
    g_DM_TIME_SkipBadBlock = 0;
    g_DM_TIME_Checksum = 0;
    g_DM_TIME_Total_Begin = 0;
    g_DM_TIME_Total = 0;
#endif
    reset_dm_descriptor ();

    DM_TIME_TOTAL_BEGIN;

    if (!buf)
        return;

    dm_ctx.dm_status = DM_STATUS_START;

    while (1)
    {
        if (dm_ctx.dm_status == DM_STATUS_SECT_ONGING)
        {
            DM_STATE_LOG ("SECT_ONGOING");
            handle_data (pktsz, buf);
        }
        else if (dm_ctx.dm_status == DM_STATUS_START)
        {
            DM_STATE_LOG ("START");
            handle_imgp (&pktsz);
        }
        else if (dm_ctx.dm_status == DM_STATUS_SECT_FINISHED)
        {
            DM_STATE_LOG ("SECT_FINISHED");
            handle_cksm ();
        }
        else if (dm_ctx.dm_status == DM_STATUS_SECT_WAIT_NXT)
        {
            DM_STATE_LOG ("WAIT_NEXT");
            handle_midle_state (buf);
        }
        else if (dm_ctx.dm_status == DM_STATUS_REBOOT)
        {
            DM_STATE_LOG ("REBOOT");
            handle_reboot ();
            break;
        }
        else if (dm_ctx.dm_status == DM_STATUS_ATBOOT)
        {
            DM_STATE_LOG ("ATBOOT");
            handle_autoboot ();
            break;
        }
        else if (dm_ctx.dm_status == DM_STATUS_UPDATE)
        {
            DM_STATE_LOG ("UPDATE");
            handle_update ();
        }
        else if (dm_ctx.dm_status == DM_STATUS_ERR_ONGOING)
        {
            DM_STATE_LOG ("ERR_ONGOING");
            handle_data (pktsz, buf);
        }
        else if (dm_ctx.dm_status == DM_STATUS_ERR_FINISHED)
        {
            DM_STATE_LOG ("ERR_FINISHED");
            handle_errp ();
        }
        else if (dm_ctx.dm_status == DM_STATUS_AUTH_VERIFY)
        {
            DM_STATE_LOG ("AUTH_VERIFY");
        }
        else if (dm_ctx.dm_status == DM_STATUS_PATCH_CMD)
        {
            DM_STATE_LOG ("PATCH_CMD");
        }
        else if (dm_ctx.dm_status == DM_STATUS_PATCH_VERIFY)
        {
            DM_STATE_LOG ("PATCH_VERIFY");
        }
    }
    return;
}
#endif
#endif /* CFG_USB_DOWNLOAD */
