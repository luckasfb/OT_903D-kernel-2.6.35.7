/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>

#include "bootloader.h"
#include "common.h"
#include "mtdutils/mtdutils.h"
#include "roots.h"
#include "install.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#undef LOG_TAG
#include "cutils/log.h"
#define LOG_TAG "sec"

#include "sec.h"

/******************************************************************************
 *  DEBUG OPTIONS
 ******************************************************************************/
//#define SEC_UI_MESSAGE

/******************************************************************************
 *  INTERNAL DEFINITION
 ******************************************************************************/
#define MOD                                     "SEC"

/******************************************************************************
 *  INTERNAL DATA STRUCTURE
 ******************************************************************************/
#define RB_SEC_VERSION                          "2011/5/30"

/******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
bool bSecEnabled                                = false;

/******************************************************************************
 *  EXTERNAL FUNCTION
 ******************************************************************************/
extern void LOG_INFO(const char *msg, ...);
extern void LOG_ERROR(const char *msg, ...);
extern void LOG_HEX(const char *str, const char *p, int len);

/******************************************************************************
 *  SECURE INIT
 ******************************************************************************/
bool sec_init(void)
{
    int ret = SEC_OK;

#ifdef SEC_UI_MESSAGE
    ui_print("[%s] %s\n", __func__, RB_SEC_VERSION);
#endif    

    /* =================================== */
    /* init boot info                      */
    /* =================================== */
    ret = sec_boot_init(false,false);

    if((ret == SEC_SBOOT_NOT_ENABLED) || (ret == SEC_SUSBDL_NOT_ENABLED))
    {
        printf("[%s] no check. ret 0x%x\n",MOD,ret);
        bSecEnabled = false;
        goto _end;
    }
    
    if(SEC_OK != ret)
    {   
        goto _err;
    }

    bSecEnabled = true;

_end:        
    return ret;

_err:
    ui_print("[%s] init fail. ret '0x%x'\n",MOD,ret);
    //assert(0);    
    return ret;
}

/******************************************************************************
 *  MARK STATUS
 ******************************************************************************/
int sec_mark_status(bool bDebug)
{
    int ret = SEC_OK;
    
    if(bDebug) { ui_print("sec mark status\n"); }     


    /* =================================== */
    /* check if sec is enabled             */
    /* =================================== */
    if(false == bSecEnabled)
    {
        goto _end;
    }

    /* =================================== */
    /* mark status                         */
    /* =================================== */
    if(SEC_OK != (ret = sec_boot_mark_status()))
    {
        goto _err;
    }

_end:    
    return ret;

_err:
    ui_print("[%s] secure boot check fail '0x%x'\n",MOD,ret);
    //assert(0);        
    return ret;
}



/******************************************************************************
 *  SECURE UPDATE
 ******************************************************************************/
int sec_update(bool bDebug)
{
    int ret = SEC_OK;

    /* =================================== */
    /* check if sec is enabled             */
    /* =================================== */
    if(false == bSecEnabled)
    {
        goto _end;
    }

    /* =================================== */
    /* recovery update                     */
    /* =================================== */
    if(SEC_OK != (ret = sec_boot_recovery_update()))
    {
        goto _err;
    }     

_end:    
    return ret;

_err:
    ui_print("[%s] recovery update fail fail '0x%x'\n",MOD,ret);
    //assert(0);        
    return ret;
}

