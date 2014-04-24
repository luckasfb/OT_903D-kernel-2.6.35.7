/*
 * (C) Copyright 2011
 * MediaTek <www.mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


/* for general operations */
#include "mt6573.h"

/* for storage device operations */
#include "nand.h"
#include "mt6573_partition.h"
#include "boot_device.h"

/* for crypto operations */
#include "cust_sec_ctrl.h"
#include "sec_error.h"
#include "sec.h"

#include "mt65xx.h"

/******************************************************************************
 * CONSTANT DEFINITIONS                                                       
 ******************************************************************************/
#define MOD                         "LIB"
#define SEC_WORKING_BUF_ADDR        SEC_WORKING_BUFFER_START
#define SEC_UTIL_BUF_ADDR           SEC_UTIL_BUFFER_START
#define SEC_IMG_BUF_ADDR            SEC_IMG_BUFFER_START
#define SEC_IMG_HEADER_MAX_LEN      0x2000
#define SEC_IMG_BUF_LEN             SEC_IMG_BUFFER_LENGTH


/******************************************************************************
 * DEBUG
 ******************************************************************************/
#define SEC_DEBUG                   (FALSE)
#define SMSG                        dbg_print
#if SEC_DEBUG
#define DMSG                        dbg_print
#else
#define DMSG 
#endif


/******************************************************************************
 *  EXTERNAL VARIABLES
 ******************************************************************************/
extern struct nand_chip             g_nand_chip;
extern storge_device_t              use_device;

/******************************************************************************
 *  RETURN AVAILABLE BUFFER FOR S-BOOT CHECK
 ******************************************************************************/
U8* sec_util_get_img_buf (void)
{
    return (U8*) SEC_IMG_BUF_ADDR;
}

U8* sec_util_get_working_buf (void)
{
    return (U8*) SEC_WORKING_BUF_ADDR;
}

/******************************************************************************
 *  READ IMAGE FOR S-BOOT CHECK (FROM NAND or eMMC DEVICE)
 ******************************************************************************/
U8* sec_util_read_image (U32 offset, U32 size)
{
    BOOL ret            = TRUE;
    U32 i               = 0;
    U32 cnt             = 0;

    U32 now_offset      = 0;
    U32 total_pages     = 0;
    U32 start_offset    = offset;
    const U32 mem_dst   = SEC_IMG_BUF_ADDR + SEC_IMG_HEADER_MAX_LEN;

    /* ======================== */
    /* read from eMMC           */
    /* ======================== */
    if(CFG_STORAGE_DEVICE == EMMC)
    {
        SMSG("[%s] Read from eMMC\n",MOD);
        use_device.storge_read((U8*)mem_dst, offset, size);
    }
    /* ======================== */
    /* read from NAND           */
    /* ======================== */
    else if(CFG_STORAGE_DEVICE == NAND)
    {
        /* ========================= */
        /* initialize nand parameter */
        /* ========================= */            
        const U32 page_size = g_nand_chip.page_size; 
        const U32 blk_size  = g_nand_chip.erasesize;

        SMSG("[%s] Read from NAND\n",MOD);
        /* ========================= */
        /* initialize buffer         */
        /* ========================= */           
        U32 tmp_dst         = mem_dst;

        /* clean buffer */
        memset((U8*)tmp_dst, 0x0, size);

        /* ========================= */
        /* calculate total page num  */
        /* ========================= */   
        total_pages = size / page_size;
        if((size % page_size) !=0)
            total_pages++;

        /* ========================= */
        /* check bad block           */
        /* ========================= */
        start_offset = nand_find_safe_block (start_offset);
     
        /* ========================= */
        /* read image                */
        /* ========================= */   
        for (cnt = total_pages, now_offset = start_offset ; cnt >= 1 ; cnt -- , now_offset += page_size ) 
        {        

            if((now_offset % blk_size) == 0)
                now_offset = nand_find_safe_block(now_offset);
     
            /* ========================= */
            /* read page                 */
            /* ========================= */
            use_device.storge_read((U8*)tmp_dst, now_offset, page_size);
            
            /* dump first 8 bytes for debugging */
            for(i=0;i<8;i++)
                DMSG("0x%x,",*((U8*)tmp_dst+i));
            DMSG ("\n");

            /* ========================= */
            /* adjust img buffer offset  */
            /* ========================= */
            tmp_dst += page_size;
        }    
    }
    else
    {
        SMSG ("[%s] unknown storage type\n",MOD);
        ASSERT(0);
    }

    
    return (U8*)mem_dst;
}
