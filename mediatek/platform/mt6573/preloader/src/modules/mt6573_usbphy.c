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
#include "mt6573_usbd.h"
#include "mt6573_pdn_hw.h"
#include "mt6573_pdn_sw.h"

#define USBPHY_READ8(offset)          __raw_readb(USB_BASE+0x800+offset)
#define USBPHY_WRITE8(offset, value)  __raw_writeb(value, USB_BASE+0x800+offset)
#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, USBPHY_READ8(offset) | mask)
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, USBPHY_READ8(offset) & ~mask)

void mt6573_usb_phy_poweron(void){

    udelay(50);

    USBPHY_CLR8(0x6b, 0x04);  
    USBPHY_CLR8(0x66, 0x40);  
    USBPHY_SET8(0x66, 0x20); 
    USBPHY_CLR8(0x60, 0x40); 
    USBPHY_SET8(0x60, 0x80); 
    USBPHY_CLR8(0x1e, 0x04); 
    USBPHY_CLR8(0x15, 0x07);
    USBPHY_SET8(0x15, 0x01);
    USBPHY_CLR8(0x10, 0x03);  
    USBPHY_SET8(0x10, 0x02);
    USBPHY_CLR8(0x10, 0x0c);  
    USBPHY_SET8(0x10, 0x08);
    USBPHY_CLR8(0x18, 0x07);  
    USBPHY_SET8(0x18, 0x04);
    USBPHY_SET8(0x18, 0x40);  
    USBPHY_SET8(0x18, 0x20);  
    USBPHY_CLR8(0x18, 0x10);  
    USBPHY_CLR8(0x16, 0x03);  
    USBPHY_SET8(0x16, 0x02);
    USBPHY_CLR8(0x02, 0x3f);  
    USBPHY_SET8(0x02, 0x0a);
    USBPHY_CLR8(0x1b, 0x01);  
    USBPHY_CLR8(0x1b, 0x02);  
    USBPHY_CLR8(0x6a, 0x04);  
    USBPHY_SET8(0x1b, 0x04);  

    udelay(100);
    
    return;
}

void mt6573_usb_phy_savecurrent(void){

    USBPHY_CLR8(0x6b, 0x04);  
    USBPHY_CLR8(0x66, 0x40);  
    USBPHY_SET8(0x66, 0x20);  
    USBPHY_CLR8(0x6a, 0x04);  
    USBPHY_SET8(0x68, 0x40);  
    USBPHY_SET8(0x68, 0x80);  
    USBPHY_CLR8(0x68, 0x30);  
    USBPHY_SET8(0x68, 0x10);
    USBPHY_SET8(0x68, 0x04);  
    USBPHY_CLR8(0x69, 0x3c);  
    USBPHY_SET8(0x6a, 0x10);  
    USBPHY_SET8(0x6a, 0x20);  
    USBPHY_SET8(0x6a, 0x08);  
    USBPHY_SET8(0x6a, 0x02);  
    USBPHY_SET8(0x6a, 0x80);  
    USBPHY_CLR8(0x1b, 0x04);  

    udelay(100);

    USBPHY_SET8(0x63, 0x02);  
    
    udelay(1);

    USBPHY_SET8(0x6a, 0x04);  

    udelay(1);
    
    return;
}

void mt6573_usb_phy_recover(void){

    udelay(50);

    USBPHY_CLR8(0x1e, 0x04);  
    USBPHY_CLR8(0x10, 0x10);  
    USBPHY_CLR8(0x6b, 0x04);  
    USBPHY_CLR8(0x66, 0x40);  
    USBPHY_SET8(0x66, 0x20);  
    USBPHY_CLR8(0x6a, 0x04);  
    USBPHY_CLR8(0x68, 0x40);  
    USBPHY_CLR8(0x68, 0x80);  
    USBPHY_CLR8(0x68, 0x30);  
    USBPHY_CLR8(0x68, 0x04);  
    USBPHY_CLR8(0x69, 0x3c);  
    USBPHY_CLR8(0x6a, 0x10);  
    USBPHY_CLR8(0x6a, 0x20);  
    USBPHY_CLR8(0x6a, 0x08);  
    USBPHY_CLR8(0x6a, 0x02);  
    USBPHY_CLR8(0x6a, 0x80);  
    USBPHY_SET8(0x1b, 0x04);  

    udelay(100);

    return;
}

void usb_phy_init(void)
{   
    mt6573_usb_disconnect();
    mt6573_usb_phy_savecurrent();
    DRV_WriteReg32 (0x70026308, 0x80);
#ifdef DBG_PRELOADER        
    dbg_print ("usb phy power on\n");
#endif
    mt6573_usb_phy_poweron ();
#ifdef DBG_PRELOADER        
    dbg_print ("usb phy save current\n");
#endif
    mt6573_usb_phy_savecurrent ();
}   
