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


#ifndef __MT6516_LCM_H__
#define __MT6516_LCM_H__

#include "mt6573.h"
#include "mt6573_typedefs.h"
#include "mt6573_dpilib.h"
#include "mt6573_lcdlib.h"

#define CFG_DISPLAY_PITCH       (CFG_DISPLAY_WIDTH * CFG_DISPLAY_BPP / 8)
#define CFG_DISPLAY_FB_SIZE     (CFG_DISPLAY_HEIGHT * CFG_DISPLAY_PITCH)
#define CFG_DISPLAY_FB_PAGES    (2)

// ---------------------------------------------------------------------------
//  LCM Driver Functions
// ---------------------------------------------------------------------------

typedef void (*LCM_INIT) (DWORD fbPhysAddr);
typedef void (*LCM_DEINIT) (void);
typedef void (*LCM_PANEL_ENABLE) (BOOL enable);
typedef void (*LCM_SET_FRAME_BUFFER_ADDR) (DWORD fbPhysAddr);
typedef void (*LCM_UPDATE) (BOOL blocking, UINT32 x, UINT32 y, UINT32 width,
                            UINT32 height);
typedef UINT32 (*LCM_GET_VRAM_SIZE) (void);


typedef struct
{
    LCM_INIT init;
    LCM_DEINIT deinit;
    LCM_PANEL_ENABLE panel;
    LCM_SET_FRAME_BUFFER_ADDR set_framebuffer_addr;
    LCM_UPDATE update;
    LCM_GET_VRAM_SIZE get_vram_size;
} LCM_DRIVER;

typedef struct
{
    DWORD frameAddress;
    UINT32 frameWidth;
    UINT32 frameHeight;
    DPI_FB_FORMAT frameFormat;

    // DPI controller timing settings
    //
    BOOL pclkPolarity;
    UINT32 pclkDivisor;
    UINT32 pclkDuty;
    BOOL dePolarity;
    BOOL vsyncPolarity;
    UINT32 vsgyncPulseWidth;
    UINT32 vsyncBackPorch;
    UINT32 vsyncFrontPorch;
    BOOL hsyncPolarity;
    UINT32 hsgyncPulseWidth;
    UINT32 hsyncBackPorch;
    UINT32 hsyncFrontPorch;

    // DPI controller output color
    //
    BOOL inverseRGBOrder;

} LCM_RGB_IF_INIT;


typedef struct
{
    DWORD frameAddress;
    UINT32 frameWidth;
    UINT32 frameHeight;
    DISP_LAYER_FORMAT frameFormat;

} LCM_HOST_IF_INIT, *PLCM_HOST_IF_INIT;


// ---------------------------------------------------------------------------
//  LCM Driver Functions
// ---------------------------------------------------------------------------
extern const LCM_DRIVER *mt6516_lcm_get_drv (void);

extern void LCM_RGB_IF_Init (const LCM_RGB_IF_INIT * param);
extern void LCM_RGB_IF_Deinit (void);
extern void LCM_RGB_IF_PanelEnable (BOOL enable);
extern void LCM_RGB_IF_SetFBAddr (DWORD fbPhysAddr);
extern void LCM_RGB_IF_Update (BOOL blocking, UINT32 x, UINT32 y,
                               UINT32 width, UINT32 height);
extern UINT32 LCM_RGB_IF_GetVRamSize (void);

extern void LCM_HOST_IF_Init (const LCM_HOST_IF_INIT * param);
extern void LCM_HOST_IF_Deinit (void);
extern void LCM_HOST_IF_PanelEnable (BOOL enable);
extern void LCM_HOST_IF_SetFBAddr (DWORD fbPhysAddr);
extern void LCM_HOST_IF_Update (BOOL blocking, UINT32 x, UINT32 y,
                                UINT32 width, UINT32 height);
extern UINT32 LCM_HOST_IF_GetVRamSize (void);

#endif /* __MT6516_LCM_H__ */
