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
#include "mt6573_download.h"
#include "mt65xx.h"
#include "boot_device.h"
#include "mmc_core.h"
#include "nand.h"

#ifdef CFG_USB_DOWNLOAD

#define BUF_BLK_NUM                 4   /* 4 * 512bytes = 2KB */

/**************************************************************************
*  DEBUG CONTROL
**************************************************************************/

/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
extern struct nand_chip g_nand_chip;
extern Device_vFunc g_dev_vfunc;

static device_info_t mmc_gdevice_info;

device_info_t *mmc_get_storge_info(void)
{
	mmc_gdevice_info.page_size = 65536;
	mmc_gdevice_info.pktsz = 65536;

	return &mmc_gdevice_info; 
}

// ==========================================================
// MMC Common Interface - Correct R/W Address
// ==========================================================
u32 mmc_find_safe_block (u32 offset)
{
    return offset;
}


// ==========================================================
// MMC Common Interface - Read Function
// ==========================================================
u32 mmc_read_data(u8 *store_buf, u32 start, u32 img_size)
{
    unsigned long blknr;
    u32 blks;

    blknr = start / 512;
    blks = (img_size + 511) / 512;
    if (blks != (u32)mmc_block_read(0, blknr, blks, (unsigned long *)store_buf)) 
    {
        printf("[SD0] block read 0x%.8x failed\n", start);
    }

    return start;
}

// ==========================================================
// MMC Common Interface - Write Function
// ==========================================================
u32 mmc_write_data (u8 * buf, u32 offset)
{
    unsigned long blknr;
    u32 blks;

    blknr = offset / 512;
    blks = BUF_BLK_NUM; /* 2K bytes */
    blks = (u32)mmc_block_write(0, blknr, blks, (unsigned long *)buf);
    if (blks != BUF_BLK_NUM) 
    {
        printf("[SD0] block write 0x%.8x failed\n", offset);
    }    
    return offset;
}

// ==========================================================
// MMC Common Interface - Erase Function
// ==========================================================
bool mmc_erase_data (u32 offset, u32 offset_limit, u32 size)
{
    /* Notice that the block size is different with different emmc.
    * Thus, it could overwrite other partitions while erasing data.
    * Don't implement it if you don't know the block size of emmc.
    */
    return TRUE;
}

// ==========================================================
// MMC Common Interface - Check If Device Is Ready To Use
// ==========================================================
void mmc_wait_ready (void)
{
    return;
}

// ==========================================================
// MMC Common Interface - Checksum Calculation Body
// ==========================================================
u32 mmc_chksum_body (u32 chksm, char *buf, u32 pktsz)
{
    u32 i;

    // TODO : Correct It    
    /* checksum algorithm body, simply exclusive or */
    for (i = 0; i < pktsz - DM_SZ_SPARE_OFFSET; i++)
        chksm ^= buf[i];

    return chksm;
}

// ==========================================================
// MMC Common Interface - Checksum Calculation
// ==========================================================
u32 mmc_chksum_per_file (u32 mmc_offset, u32 img_size)
{
    u32 now = 0, i = 0, chksm = 0, start_block = 0, total = 0;
    INT32 cnt;
    bool ret = TRUE;

    // TODO : Correct it. Don't use nand page size
    u32 start = mmc_offset;
    u32 pagesz = g_nand_chip.page_size;
    u32 pktsz = pagesz + g_nand_chip.oobsize;
    u8 *buf = (u8 *) g_common_buf;

    // clean the buffer
    memset (buf, 0x0, COMMON_BUFFER_MAX_SIZE);

    // calculate the number of page
    total = img_size / pagesz;
    if (img_size % pagesz != 0)
    {    
        total++;
    }

    // check the starting block is safe
    start_block = mmc_find_safe_block (start);
    if (start_block != start)
    {     
        start = start_block;
    }

    // copy data from msdc to memory
    for (cnt = total, now = start; cnt >= 0; cnt--, now += pagesz)
    {
        /* read a packet */
        mmc_read_data (buf, now, pagesz);
        chksm = mmc_chksum_body (chksm, buf, pktsz);
    }

    return chksm;
}

// ==========================================================
// MMC Common Interface - Init
// ==========================================================
void mmc_device_init(void)
{
    int ret;
    mt6573_nand_reset_descriptor ();
    mt6573_nand_init ();
    mt6573_part_init ();

    ret = mmc_legacy_init(1);
    if (ret != 0) 
    {
        printf("[SD0] init card failed\n");
    }
#ifdef MT6573_CFG_USB_DOWNLOAD
    if(CFG_STORAGE_DEVICE == EMMC) {
        g_dev_vfunc.dev_write_data = mmc_write_data;
        g_dev_vfunc.dev_erase_data = mmc_erase_data;
        g_dev_vfunc.dev_wait_ready = mmc_wait_ready;
        g_dev_vfunc.dev_cal_chksum_body = mmc_chksum_body;
        g_dev_vfunc.dev_chksum_per_file = mmc_chksum_per_file;
        g_dev_vfunc.dev_find_safe_block = mmc_find_safe_block;
    }
#endif
}

#endif /* CFG_USB_DOWNLOAD */
