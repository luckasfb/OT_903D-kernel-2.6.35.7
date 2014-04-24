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

#include "mt6573_typedefs.h"

/* import rom_info variable */
#include "sec_rom_info.h"

/* import partition table */
#include "partition_define.h"

/* import custom info */
#include "cust_sec_ctrl.h"
#include "KEY_IMAGE_AUTH.h"
#include "KEY_SML_AUTH.h"
#include "KEY_SML_ENCODE.h"

/**************************************************************************
*  MACRO
**************************************************************************/ 
#define MOD                             "ROM_INFO"

/******************************************************************************
 * DEBUG
 ******************************************************************************/
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

/**************************************************************************
 *  EXTERNAL VARIABLES
 **************************************************************************/

/**************************************************************************
 *  EXTERNAL FUNCTIONS
 **************************************************************************/

/**************************************************************************
 *  DO NOT MODIFY THIS !
 **************************************************************************/
__attribute__((section(".text.rom_info")))
AND_ROMINFO_T  g_ROM_INFO = 
{
    /* ============================= */
    /* ROM_INFO                      */
    /* ============================= */
                                                            /* MTK */    
    .m_identifier                       = ROM_INFO_NAME                           

                                                            /* MTK */
    ,.m_rom_info_ver                    = ROM_INFO_VER               

                                                            /* CUSTOMER */
    ,.m_platform_id                     = PLATFORM_NAME                             

                                                            /* CUSTOMER */
    ,.m_project_id                      = PROJECT_NAME                    

    /* bit to indicate whether SEC_INFO does exist */       /* MTK */
    ,.m_sec_ro_exist                    = ROM_INFO_SEC_RO_EXIST                

                                                            /* MTK */     
    ,.m_sec_ro_offset                   = sizeof(AND_ROMINFO_T) - sizeof(AND_SECRO_T)
    
    /* sec_ro_length */                                     /* MTK */    
    ,.m_sec_ro_length                   = sizeof(AND_SECRO_T)                 

    /* ac_offset from ROM_INFO */                           /* MTK */
    ,.m_ac_offset                       = ROM_INFO_ANTI_CLONE_OFFSET 

    /* ac_length */                                         /* MTK */
    ,.m_ac_length                       = ROM_INFO_ANTI_CLONE_LENGTH 

    /* sec_cfg_offset */                                    /* MTK */
    ,.m_sec_cfg_offset                  = PART_SIZE_PRELOADER+PART_SIZE_DSP_BL+PART_SIZE_NVRAM 

    /* sec_cfg_length */                                    /* MTK */
    ,.m_sec_cfg_length                  = PART_SIZE_SECCFG           

    ,.m_reserve1                        = 0x0
   
    /* ============================= */
    /* AND_SECCTRL_T                 */
    /* ============================= */
                                                            /* MTK */
    ,.m_SEC_CTRL.m_identifier           = ROM_INFO_SEC_CTRL_ID        

                                                            /* MTK */
    ,.m_SEC_CTRL.m_sec_cfg_ver          = ROM_INFO_SEC_CTRL_VER      

                                                            /* CUSTOMER */
    ,.m_SEC_CTRL.m_sec_usb_dl           = SEC_USBDL_CFG               

                                                            /* CUSTOMER */
    ,.m_SEC_CTRL.m_sec_boot             = SEC_BOOT_CFG                  

                                                            /* MTK */
    ,.m_reserve2                        = 0x0                                      


    /* ============================= */
    /* AND_SECBOOT_UPDATEABLE_PART_T */
    /* ============================= */
    #if VERIFY_PART_UBOOT                                   /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[0]     = SBOOT_PART_UBOOT 
    #else
    ,.m_SEC_BOOT_CHECK_PART.name[0]     = 0x0
    #endif

    #if VERIFY_PART_LOGO                                    /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[1]     = SBOOT_PART_LOGO  
    #else
    ,.m_SEC_BOOT_CHECK_PART.name[1]     = 0x0
    #endif

    #if VERIFY_PART_BOOTIMG                                 /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[2]     = SBOOT_PART_BOOTIMG  
    #else
    ,.m_SEC_BOOT_CHECK_PART.name[2]     = 0x0
    #endif

    #if VERIFY_PART_RECOVERY                                /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[3]     = SBOOT_PART_RECOVERY 
    #else
    ,.m_SEC_BOOT_CHECK_PART.name[3]     = 0x0
    #endif

    #if VERIFY_PART_ANDSYSIMG                               /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[4]     = SBOOT_PART_ANDSYSIMG 
    #else
    ,.m_SEC_BOOT_CHECK_PART.name[4]     = 0x0
    #endif

    #if VERIFY_PART_SECSTATIC                               /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[5]     = SBOOT_PART_SECSTATIC      
    #else
    ,.m_SEC_BOOT_CHECK_PART.name[5]     = 0x0
    #endif

    ,.m_SEC_BOOT_CHECK_PART.name[6]     = 0x0               /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[7]     = 0x0               /* CUSTOMER */
    ,.m_SEC_BOOT_CHECK_PART.name[8]     = 0x0               /* CUSTOMER */

    /* ============================= */
    /* AND_SECRO_T                   */
    /* ============================= */
                                                            /* MTK */
    ,.m_SEC_RO.m_identifier             = ROM_INFO_SEC_RO_ID            

                                                            /* MTK */
    ,.m_SEC_RO.m_sec_ro_ver             = ROM_INFO_SEC_RO_VER           

                                                            /* CUSTOMER */
    ,.m_SEC_RO.root_rsa_n               = IMG_CUSTOM_RSA_N                

                                                            /* CUSTOMER */
    ,.m_SEC_RO.root_rsa_e               = IMG_CUSTOM_RSA_E                

                                                            /* CUSTOMER */
    ,.m_SEC_RO.sml_aes256               = SML_CUSTOM_AES_256              

                                                            /* CUSTOMER */
    ,.m_SEC_RO.crypto_seed              = CUSTOM_CRYPTO_SEED             

                                                            /* CUSTOMER */
    ,.m_SEC_RO.sml_auth_rsa_n           = SML_CUSTOM_RSA_N            

                                                            /* CUSTOMER */
    ,.m_SEC_RO.sml_auth_rsa_e           = SML_CUSTOM_RSA_E            
    
};

void sec_rom_info_init (void)
{
    COMPILE_ASSERT(AND_ROM_INFO_SIZE == sizeof(AND_ROMINFO_T));
    //SMSG("sizeof(AND_ROMINFO_T) = 0x%x\n",sizeof(AND_ROMINFO_T));
    SMSG("[%s] 'v%d','0x%x','0x%x'\n",MOD,  g_ROM_INFO.m_rom_info_ver,
                                            g_ROM_INFO.m_sec_cfg_offset,
                                            g_ROM_INFO.m_sec_cfg_length);

    //SMSG("[%s] PART_SIZE_PRELOADER '0x%x'\n",MOD,PART_SIZE_PRELOADER);
    //SMSG("[%s] PART_SIZE_DSP_BL    '0x%x'\n",MOD,PART_SIZE_DSP_BL);    
    //SMSG("[%s] PART_SIZE_NVRAM     '0x%x'\n",MOD,PART_SIZE_NVRAM);       
    //SMSG("[%s] PART_SIZE_SECCFG    '0x%x'\n",MOD,PART_SIZE_SECCFG);           
}
