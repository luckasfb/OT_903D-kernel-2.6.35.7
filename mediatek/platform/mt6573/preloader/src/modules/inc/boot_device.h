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

#ifndef __BOOT_DEVICE_H__
#define __BOOT_DEVICE_H__

#include "mt6573_typedefs.h"

// ===================================================
// prototypes of device relevant callback function 
// ===================================================

typedef U32     (*CB_DEV_INIT)(void);
typedef U32     (*CB_DEV_READ)(u8 * buf, u32 offset);
#ifdef MT6573_CFG_USB_DOWNLOAD
typedef U32     (*CB_DEV_WRITE)(u8 * buf, u32 offset);
typedef bool    (*CB_DEV_ERASE)(u32 offset, u32 offset_limit, u32 size);
typedef void    (*CB_DEV_WAIT_READY)(void);
typedef U32     (*CB_DEV_FIND_SAFE_BLOCK)(u32 offset);
typedef U32     (*CB_DEV_CHKSUM_PER_FILE)(u32 offset, u32 img_size);
typedef U32     (*CB_DEV_CHKSUM)(u32 chksm, char *buf, u32 pktsz);
#endif

typedef struct {

// ===================================================
// provided by driver owner
// ===================================================
    CB_DEV_INIT                 dev_init;
    CB_DEV_READ                 dev_read_data;
#ifdef MT6573_CFG_USB_DOWNLOAD
    CB_DEV_WRITE                dev_write_data;    
    CB_DEV_ERASE                dev_erase_data;    
    CB_DEV_WAIT_READY           dev_wait_ready; // check if device is ready to use
    CB_DEV_FIND_SAFE_BLOCK      dev_find_safe_block; // correct R/W address
    CB_DEV_CHKSUM               dev_cal_chksum_body; 
    CB_DEV_CHKSUM_PER_FILE      dev_chksum_per_file;
#endif
} Device_vFunc;


// ===================================================
// external 
// ===================================================
extern Device_vFunc g_dev_vfunc;

#endif /* __BOOT_DEVICE_H__ */
