/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2011. All rights reserved.
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

/* for general operations */
#include "mt6573_typedefs.h"
#include "mt6573_timer.h"

/* import customer configuration */
#include "sec_cust.h"

/* import sec cfg partition info */
#include "sec_rom_info.h"

/* customer key */
#include "cust_sec_ctrl.h"
#include "KEY_IMAGE_AUTH.h"
#include "KEY_SML_ENCODE.h"

/* for crypto operations */
#include "sec.h"
#include "sec_error.h"

/* for storage device operations */
#include "mt6573_partition.h"
#include "boot_device.h"
#include "mt65xx.h"

/**************************************************************************
*  MACRO
**************************************************************************/ 
#define MOD                             "SEC"

/**************************************************************************
 * DEBUG
 **************************************************************************/ 
#define SEC_DEBUG                       (FALSE)
#define SMSG                            dbg_print
#if SEC_DEBUG
#define DMSG                            dbg_print
#else
#define DMSG 
#endif

/**************************************************************************
 *  GLOBAL VARIABLES
 **************************************************************************/

/**************************************************************************
 *  LOCAL VARIABLES
 **************************************************************************/
SECURE_CFG_INFO                         sec_cfg_info;

/**************************************************************************
 *  EXTERNAL VARIABLES
 **************************************************************************/
extern struct nand_chip                 g_nand_chip;
extern storge_device_t                  use_device;
extern AND_ROMINFO_T                    g_ROM_INFO;

/**************************************************************************
 *  EXTERNAL FUNCTIONS
 **************************************************************************/
 
U8* sec_cfg_load (void)
{
    U32 i       = 0;
    U8 *buf     = (U8*)SEC_WORKING_BUFFER_START;

    /* ===================== */
    /* initialize buffer     */
    /* ===================== */
    memset(buf, 0x0, SEC_WORKING_BUFFER_LENGTH);

    /* ===================== */
    /* read sec cfg          */
    /* ===================== */
    SMSG("\n\n[%s] read '0x%x'\n",MOD,sec_cfg_info.addr);
    use_device.storge_read(buf, sec_cfg_info.addr, SEC_CFG_READ_SIZE);

    /* dump first 8 bytes for debugging */
    for(i=0;i<8;i++)
        SMSG("0x%x,",buf[i]);
    SMSG("\n");
    
    return buf;
}

/**************************************************************************
 * [SECURE LIBRARY INITIALIZATION] 
 **************************************************************************/
void sec_lib_init (void)
{

#if SEC_ENV_ENABLE

    part_t *part;
    U32 err;
    CUSTOM_SEC_CFG cus_sec_cfg;
    const U32 page_size = g_nand_chip.page_size; 

    /* ====================== */
    /* check status           */
    /* ====================== */
    /* check customer configuration data structure */
    COMPILE_ASSERT(CUSTOM_SEC_CFG_SIZE == sizeof(CUSTOM_SEC_CFG));


    /* ====================== */
    /* initialize variables   */
    /* ====================== */
    /* initialize customer configuration buffer */
    memset (&cus_sec_cfg, 0x0, sizeof(cus_sec_cfg));

    /* initialize customer configuration for security library */
    cus_sec_cfg.sec_usb_dl = SEC_USBDL_CFG;
    cus_sec_cfg.sec_boot = SEC_BOOT_CFG;
    memcpy (cus_sec_cfg.root_rsa_n, IMG_CUSTOM_RSA_N, sizeof(cus_sec_cfg.root_rsa_n));    
    memcpy (cus_sec_cfg.root_rsa_e, IMG_CUSTOM_RSA_E, sizeof(cus_sec_cfg.root_rsa_e));
    memcpy (cus_sec_cfg.root_aes256, SML_CUSTOM_AES_256, sizeof(cus_sec_cfg.root_aes256));    
    memcpy (cus_sec_cfg.crypto_seed, CUSTOM_CRYPTO_SEED, sizeof(cus_sec_cfg.crypto_seed));        

    /* ====================== */
    /* find sec cfg part info */
    /* ====================== */
    /* check if sec cfg is defined in partition table */
    part = mt6573_part_get_partition (PART_SECURE);

    if (!part)
    {
        SMSG ("[%s] part not found\n", MOD);
        ASSERT (0);
    }

    /* apply the rom info's sec cfg part info since tool also refer to this setting*/
    sec_cfg_info.addr = (unsigned int) g_ROM_INFO.m_sec_cfg_offset;
    sec_cfg_info.len = (unsigned int) g_ROM_INFO.m_sec_cfg_length;

    /* ====================== */
    /* initialize library     */
    /* ====================== */
    /* starting to initialze security library */
    if(SEC_OK != (err = seclib_init (&cus_sec_cfg, sec_cfg_load(), SEC_CFG_READ_SIZE, TRUE)))
    {
        SMSG("[%s] init fail '0x%x'\n",MOD,err);
        ASSERT (0);
    }

#endif

}
