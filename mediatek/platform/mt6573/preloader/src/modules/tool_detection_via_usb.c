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

#include "nand.h"
#include "mt6573_usbtty.h"
#include "mt6573_wdt_hw.h"
#include "mt6573_pmu_sw.h"
#include "mt6573_download.h"
#include "mt6573_partition.h"
#include "mt6573_pmic6326_hw.h"
#include "mt6573_pmic6326_sw.h"
#include "mt6573_rtc.h"
#include "mt65xx_meta.h"
#include "boot_mode.h"


#define MOD                 "<TOOL>"

//======================================================================
// MACRO DEFINITION
//======================================================================
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


//======================================================================
//  EXTERNAL DECLARATION
//======================================================================
extern int nand_curr_device;
extern u32 nand_maf_id;
extern u32 nand_dev_id;
extern struct nand_chip g_nand_chip;
extern u8 g_nand_spare[64];
extern struct nand_oobinfo mt6516_nand_oob;
extern int usbdl_init (void);
extern DM_USBDL_FLOW g_usbdl_flow;

extern void service_interrupts (void);
extern int usbdl_configured (void);

//======================================================================
//  HANDSHAKE SYNC TIME
//======================================================================
#define DM_WAIT_SYNCH_TIME      2500    /* in ms */

//======================================================================
//  Tool Handshake
//======================================================================
bool listen_tool_via_usb (uint listen_byte_cnt, uint wait_time)
{
    u32 start_time = get_timer (0);

    do
    {
        if (tool_is_present ())
            mt6573_usbtty_puts (DM_STR_READY);        /* "READY" */

        udelay (20000);       /* 20ms */
        /* apply usbdl parameter */
        if (TRUE == g_usbdl_flow.handshake_timeout_enable)
        {    
            if (get_timer (start_time) > DM_WAIT_SYNCH_TIME)
                return FALSE;
        }
    }
    
    while (mt6573_usbtty_query_data_size () < DM_SZ_DWN_REQ_STR);
    dbg_print ("%s : synch time %d ms\n", MOD, get_timer (start_time));

    return TRUE;
}


//======================================================================
//  Tool Handshake
//======================================================================
bool handshake_protocol_via_usb (void)
{
    u8 hpbuf[DM_SZ_IMG_HPBUF] = { '\0' };       /* hpbuf = handshake protocol buffer */    

    /* send "READY", detect tool existence */
    if (FALSE == listen_tool_via_usb (DM_SZ_DWN_REQ_STR, DM_WAIT_SYNCH_TIME))
    {
        //dbg_print ("%s : cannot detect flash tool or meta tool !\n", MOD);
        return FALSE;
    }

    mt6573_usbtty_getcn (DM_SZ_DWN_REQ_STR, hpbuf);

    /* check "DOWNLOAD", judge whether download mode indicated */
    if(!strncmp ((const u8 *) hpbuf, DM_STR_DOWNLOAD_REQ, strlen(DM_STR_DOWNLOAD_REQ)))
    {
#ifdef MT6573_CFG_USB_DOWNLOAD
        /* return "DAOLNWOD" to ack tool */
        mt6573_usbtty_puts (DM_STR_DOWNLOAD_ACK);
        dbg_print ("%s : start download !\n", MOD);
        g_boot_mode = DOWNLOAD_BOOT;
        return TRUE;
#else
        /* return "Download Error" to ack tool */
        //mt6573_usbtty_puts (DM_STR_DOWNLOAD_ACK);
        g_boot_mode = NORMAL_BOOT;
#ifdef DBG_PRELOADER
        dbg_print ("***now pre_loader version not support download function***\n");
        dbg_print ("***now enter normal mode ...\n\n");
#endif
        return FALSE;
#endif

    }
    /* check "METAMETA", judge whether meta mode indicated */
    if(!strncmp ((const u8 *) hpbuf, META_STR_REQ, strlen(META_STR_REQ)))
    {
        /* return "ATEMATEM" to ack tool */
        mt6573_usbtty_puts (META_STR_ACK);
        dbg_print ("%s : meta detected !\n", MOD);
        g_boot_mode = META_BOOT;
        return TRUE;
    }
    /* check "ADVMETA", judge whether meta mode indicated */
    if(!strncmp ((const u8 *) hpbuf, META_ADV_REQ, strlen(META_ADV_REQ)))
    {
          /* return "ATEMVDA" to ack tool */
        mt6573_usbtty_puts (META_ADV_ACK);
        dbg_print ("%s : adv detected !\n", MOD);
        g_boot_mode = ADVMETA_BOOT;
        return TRUE;
    }

    /* check "FACTORYM", judge whether ATE mode indicated */
    if(!strncmp ((const u8 *) hpbuf, ATE_STR_REQ, strlen(ATE_STR_REQ)))            
    {            
        dbg_print ("%s : ate detected !\n", MOD);  
        /* return "MYROTCAF" to ack tool */
        mt6573_usbtty_puts (ATE_STR_ACK);
        g_boot_mode = ATE_FACTORY_BOOT;
        return TRUE;
    }    
    else
    {   
        dbg_print ("%s : unknown string %s !\n", MOD, hpbuf);
    }

    return FALSE;
}

//======================================================================
//  USB connection
//======================================================================
bool usb_connect (void)
{
    unsigned int start_time = get_timer (0);
    mt_usb_disconnect ();
    mt_usb_connect ();
#ifdef DBG_PRELOADER
    dbg_print ("Enum (Start)\n");
#endif
    do
    {
        service_interrupts ();

        /* apply usbdl parameter */
        if (TRUE == g_usbdl_flow.enum_timeout_enable)
        {
            /* enable timeout mechanism */
            if (get_timer (start_time) > USB_ENUM_TIMEOUT)
            {
                //dbg_print ("timeout\n");
                return FALSE;
            }
        }
    }
    while (!usbdl_configured ());

	hw_set_cc(450);
#ifdef DBG_PRELOADER
    dbg_print ("Enum (End)\n");
#endif
    return TRUE;
}

//======================================================================
//  USB disconnect
//======================================================================
void mt6573_usb_disconnect (void)
{
    mt_usb_disconnect ();
    return;
}

//======================================================================
//  Tool Detection via USB
//======================================================================
void tool_detection_via_usb (void)
{
    bool usb_enum = FALSE;
    CHARGER_TYPE chr_type = hw_charger_type_detection();

    if((chr_type == STANDARD_HOST) || (chr_type == CHARGING_HOST))
        usb_enum = TRUE;

    /* if not plugged in, leave here and enter normal booting mode */
    if (usb_enum == FALSE)
    {
        //dbg_print ("PMU - USB charger doesn't exist. Cannot start download and META!\n");
        dbg_print ("PMU - USB charger doesn't exist!\n");
		hw_set_cc(450);
        return;
    }

    usbdl_init ();
    udelay (1000);
    mt6573_usb_disconnect ();
    if (usb_connect () == FALSE)
    {    
        dbg_print ("%s : USB enum timeout !\n", MOD);
        usb_service_offline ();
        return;
    }

    // Flash Tool - USB Download
    // META  Tool - META Mode
    udelay (1000);
    if (FALSE == handshake_protocol_via_usb ())
    {    
        dbg_print ("%s : handshake fail!\n",MOD);
        usb_service_offline ();
        return;
    }

#ifdef MT6573_CFG_USB_DOWNLOAD 
    // USB download
    if (g_boot_mode == DOWNLOAD_BOOT)
    {    
#ifdef DBG_PRELOADER
        dbg_print ("Please check the VBAT > 3.6V for download image successfully\n");
        //dbg_print ("Because the power consumption is too high\n");
#endif
        transmission_main ();
    }
#endif
        
    usb_service_offline ();
    return;

}

