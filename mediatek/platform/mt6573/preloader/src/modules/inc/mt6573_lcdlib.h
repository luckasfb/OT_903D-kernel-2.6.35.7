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


#ifndef __MT6516_LCDLIB_H__
#define __MT6516_LCDLIB_H__

#include "mt6573_typedefs.h"

#define OPTIMIZE_LCDLIB_CODE_SIZE  1

#ifdef __cplusplus
extern "C"
{
#endif


    typedef enum
    {
        DISP_LAYER_0 = 0,
        DISP_LAYER_1 = 1,
        DISP_LAYER_2 = 2,
        DISP_LAYER_3 = 3,
        DISP_LAYER_4 = 4,
        DISP_LAYER_5 = 5,
        DISP_LAYER_NUM,
        DISP_LAYER_ALL = 0xFFFFFFFF,
    } DISP_LAYER_ID;


    typedef enum
    {
        DISP_LAYER_FORMAT_INDEX = 0,
        DISP_LAYER_FORMAT_8BPP = 0,
        DISP_LAYER_FORMAT_RGB565 = 1,
        DISP_LAYER_FORMAT_ARGB8888 = 2,
        DISP_LAYER_FORMAT_RGB888 = 3,
        DISP_LAYER_FORMAT_NUM,
    } DISP_LAYER_FORMAT;


    typedef enum
    {
        DISP_LAYER_ROTATE_NONE = 0,
        DISP_LAYER_ROTATE_0 = 0,
        DISP_LAYER_ROTATE_90 = 1,
        DISP_LAYER_ROTATE_180 = 2,
        DISP_LAYER_ROTATE_270 = 3,
        DISP_LAYER_ROTATE_MIRROR_0 = 4,
        DISP_LAYER_ROTATE_MIRROR_90 = 5,
        DISP_LAYER_ROTATE_MIRROR_180 = 6,
        DISP_LAYER_ROTATE_MIRROR_270 = 7,
    } DISP_LAYER_ROTATION;


    typedef enum
    {
        DISP_SW_TRIGGER = 0,
        DISP_HW_TRIGGER_BUFFERING,
        DISP_HW_TRIGGER_DIRECT_COUPLE,
    } DISP_LAYER_TRIGGER_MODE;


    typedef enum
    {
        DISP_HW_TRIGGER_SRC_IRT1 = 0,
        DISP_HW_TRIGGER_SRC_IBW1 = 1,
        DISP_HW_TRIGGER_SRC_IBW2 = 3,
    } DISP_HW_TRIGGER_SRC;


    typedef enum
    {
        LCD_STATUS_OK = 0,

        LCD_STATUS_ERROR,
    } LCD_STATUS;


    typedef enum
    {
        LCD_STATE_IDLE = 0,
        LCD_STATE_BUSY,
    } LCD_STATE;


    typedef enum
    {
        LCD_IF_PARALLEL_0 = 0,
        LCD_IF_PARALLEL_1 = 1,
        LCD_IF_PARALLEL_2 = 2,
        LCD_IF_SERIAL_0 = 3,
        LCD_IF_SERIAL_1 = 4,
    } LCD_IF_ID;


    typedef enum
    {
        LCD_IF_PARALLEL_8BITS = 0,
        LCD_IF_PARALLEL_9BITS = 1,
        LCD_IF_PARALLEL_16BITS = 2,
        LCD_IF_PARALLEL_18BITS = 3,
        LCD_IF_PARALLEL_24BITS = 4,
    } LCD_IF_PARALLEL_BITS;


    typedef enum
    {
        LCD_IF_SERIAL_8BITS = 0,
        LCD_IF_SERIAL_9BITS = 1,
    } LCD_IF_SERIAL_BITS;


    typedef enum
    {
        LCD_IF_PARALLEL_CLK_DIV_1 = 0,
        LCD_IF_PARALLEL_CLK_DIV_2,
        LCD_IF_PARALLEL_CLK_DIV_4,
    } LCD_IF_PARALLEL_CLK_DIV;


    typedef enum
    {
        LCD_IF_SERIAL_CLK_DIV_1 = 0,
        LCD_IF_SERIAL_CLK_DIV_2,
        LCD_IF_SERIAL_CLK_DIV_4,
        LCD_IF_SERIAL_CLK_DIV_8,
        LCD_IF_SERIAL_CLK_DIV_16,
    } LCD_IF_SERIAL_CLK_DIV;


    typedef enum
    {
        LCD_IF_A0_LOW = 0,
        LCD_IF_A0_HIGH = 1,
    } LCD_IF_A0_MODE;


    typedef enum
    {
        LCD_IF_MCU_WRITE_8BIT = 8,
        LCD_IF_MCU_WRITE_16BIT = 16,
        LCD_IF_MCU_WRITE_32BIT = 32,
    } LCD_IF_MCU_WRITE_BITS;


    typedef enum
    {
        LCD_IF_FORMAT_RGB332 = 0,
        LCD_IF_FORMAT_RGB444 = 1,
        LCD_IF_FORMAT_RGB565 = 2,
        LCD_IF_FORMAT_RGB666 = 3,
        LCD_IF_FORMAT_RGB888 = 4,
    } LCD_IF_FORMAT;


    typedef enum
    {
        LCD_IF_WIDTH_8_BITS = 0,
        LCD_IF_WIDTH_9_BITS = 2,
        LCD_IF_WIDTH_16_BITS = 1,
        LCD_IF_WIDTH_18_BITS = 3,
        LCD_IF_WIDTH_24_BITS = 4,
    } LCD_IF_WIDTH;


    typedef enum
    {
        LCD_CMDQ_0 = 0,
        LCD_CMDQ_1 = 1,
    } LCD_CMDQ_ID;


    typedef enum
    {
        LCD_FB_0 = 0,
        LCD_FB_1 = 1,
        LCD_FB_2 = 2,
        LCD_FB_NUM,
    } LCD_FB_ID;


    typedef enum
    {
        LCD_FB_FORMAT_RGB565 = 0,
        LCD_FB_FORMAT_RGB888 = 1,
        LCD_FB_FORMAT_ARGB8888 = 2,
        LCD_FB_FORMAT_NUM,
    } LCD_FB_FORMAT;


    typedef enum
    {
        LCD_OUTPUT_TO_LCM = (1 << 0),
        LCD_OUTPUT_TO_MEM = (1 << 1),
        LCD_OUTPUT_TO_LCM_MEM = LCD_OUTPUT_TO_LCM | LCD_OUTPUT_TO_MEM,
    } LCD_OUTPUT_MODE;


// Configurations
    LCD_STATUS LCD_Init (void);
    LCD_STATUS LCD_Deinit (void);

    LCD_STATUS LCD_PowerOn (void);
    LCD_STATUS LCD_PowerOff (void);

    LCD_STATUS LCD_WaitForNotBusy (void);

// LCD Controller Interface
    LCD_STATUS LCD_ConfigParallelIF (LCD_IF_ID id,
                                     LCD_IF_PARALLEL_BITS ifDataWidth,
                                     LCD_IF_PARALLEL_CLK_DIV clkDivisor,
                                     UINT32 writeSetup,
                                     UINT32 writeHold,
                                     UINT32 writeWait,
                                     UINT32 readSetup,
                                     UINT32 readLatency, UINT32 waitPeriod);

    LCD_STATUS LCD_ConfigSerialIF (LCD_IF_ID id,
                                   LCD_IF_SERIAL_BITS bits,
                                   BOOL clockPolarity,
                                   BOOL clockPhase,
                                   BOOL csPolarity,
                                   LCD_IF_SERIAL_CLK_DIV clkDivisor,
                                   BOOL mode);

    LCD_STATUS LCD_ConfigIfFormat (BOOL RGBOrder,
                                   BOOL MSBFirst,
                                   BOOL paddingOnLSB,
                                   LCD_IF_FORMAT format,
                                   LCD_IF_WIDTH busWidth);

    LCD_STATUS LCD_SetResetSignal (BOOL high);

// LCD Command Queue
    LCD_STATUS LCD_CmdQueueEnable (BOOL enabled);
    LCD_STATUS LCD_CmdQueueSelect (LCD_CMDQ_ID id);
    LCD_STATUS LCD_CmdQueueSetWaitPeriod (UINT32 period);
    LCD_STATUS LCD_CmdQueueWrite (LCD_CMDQ_ID id, UINT32 * cmds,
                                  UINT32 cmdCount);

// Layer Configurations
    LCD_STATUS LCD_LayerEnable (DISP_LAYER_ID id, BOOL enable);
    LCD_STATUS LCD_LayerSetAddress (DISP_LAYER_ID id, UINT32 address);
    LCD_STATUS LCD_LayerSetSize (DISP_LAYER_ID id, UINT32 width,
                                 UINT32 height);
    LCD_STATUS LCD_LayerSetOffset (DISP_LAYER_ID id, UINT32 x, UINT32 y);
    LCD_STATUS LCD_LayerSetFormat (DISP_LAYER_ID id,
                                   DISP_LAYER_FORMAT format);
    LCD_STATUS LCD_LayerSetRotation (DISP_LAYER_ID id,
                                     DISP_LAYER_ROTATION rotation);
    LCD_STATUS LCD_LayerSetAlphaBlending (DISP_LAYER_ID id, BOOL enable,
                                          UINT8 alpha);
    LCD_STATUS LCD_LayerSetSourceColorKey (DISP_LAYER_ID id, BOOL enable,
                                           UINT32 colorKey);

// HW Trigger Configurations
    LCD_STATUS LCD_LayerSetTriggerMode (DISP_LAYER_ID id,
                                        DISP_LAYER_TRIGGER_MODE mode);
    LCD_STATUS LCD_LayerSetHwTriggerSrc (DISP_LAYER_ID id,
                                         DISP_HW_TRIGGER_SRC src);
    LCD_STATUS LCD_EnableHwTrigger (BOOL enable);

// ROI Window Configurations
    LCD_STATUS LCD_SetBackgroundColor (UINT32 bgColor);
    LCD_STATUS LCD_SetRoiWindow (UINT32 x, UINT32 y, UINT32 width,
                                 UINT32 height);

// Output to Memory Configurations
    LCD_STATUS LCD_SetOutputMode (LCD_OUTPUT_MODE mode);
    LCD_STATUS LCD_WaitDPIIndication (BOOL enable);
    LCD_STATUS LCD_FBSetFormat (LCD_FB_FORMAT format);
    LCD_STATUS LCD_FBSetPitch (UINT32 pitchInByte);
    LCD_STATUS LCD_FBEnable (LCD_FB_ID id, BOOL enable);
    LCD_STATUS LCD_FBSetAddress (LCD_FB_ID id, UINT32 address);
    LCD_STATUS LCD_FBSetStartCoord (UINT32 x, UINT32 y);

// Operations
    LCD_STATUS LCD_SelectWriteIF (LCD_IF_ID id);
    LCD_STATUS LCD_WriteIF (LCD_IF_ID id, LCD_IF_A0_MODE a0, UINT32 value,
                            LCD_IF_MCU_WRITE_BITS bits);
    LCD_STATUS LCD_ReadIF (LCD_IF_ID id, LCD_IF_A0_MODE a0, UINT32 * value,
                           LCD_IF_MCU_WRITE_BITS bits);
    LCD_STATUS LCD_StartTransfer (BOOL blocking);

// Retrieve Information
    LCD_STATE LCD_GetState (void);

// Debug
    LCD_STATUS LCD_DumpRegisters (void);

#ifdef __cplusplus
}
#endif

#endif                          // __MT6516_LCDLIB_H__
