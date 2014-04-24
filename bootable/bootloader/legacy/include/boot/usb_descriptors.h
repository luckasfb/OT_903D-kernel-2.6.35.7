/* Copyright Statement:
 * 
 * This software/firmware and related documentation ("MediaTek Software") are 
 * protected under relevant copyright laws. The information contained herein is 
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any 
 * reproduction, modification, use or disclosure of MediaTek Software, and 
 * information contained herein, in whole or in part, shall be strictly 
 * prohibited.  
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") 
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON 
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER 
 * DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF 
 * ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE 
 * MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR 
 * ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT 
 * IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER 
 * LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE 
 * RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO RECEIVER'S 
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM. 
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE 
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 *  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, 
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO 
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static unsigned short manufacturer_string[] = {
    (TYPE_STRING << 8) | (12 * 2),
    'G', 'o', 'o', 'g', 'l', 'e', ',', ' ', 'I', 'n', 'c',
};

static unsigned short product_string[] = {
    (TYPE_STRING << 8) | (12 * 2),
    'A', 'n', 'd', 'r', 'o', 'i', 'd', ' ', '1', '.', '0',
};

static unsigned short default_string[] = {
    (TYPE_STRING << 8) | (8 * 2),
    'd', 'e', 'f', 'a', 'u', 'l', 't',
};

static unsigned short language_table[] = {
    (TYPE_STRING << 8) | 4,
    0x0409, // LANGID for US English
};

static unsigned char device_desc[] = {
    18,              // length
    TYPE_DEVICE,     // type
    0x10, 0x02,      // usb spec rev 1.00
    0x00,            // class
    0x00,            // subclass
    0x00,            // protocol
    0x40,            // max packet size
    0xD1, 0x18,      // vendor id
    0x0D, 0xD0,      // product id
    0x00, 0x01,      // version 1.0
    0x01,            // manufacturer str idx
    0x02,            // product str idx
    0x00,            // serial number index
    0x01,            // number of configs,
};    

static unsigned char config_desc[] = {
    0x09,            // length
    TYPE_CONFIGURATION,
    0x20, 0x00,      // total length
    0x01,            // # interfaces
    0x01,            // config value
    0x00,            // config string
    0x80,            // attributes
    0x80,            // XXX max power (250ma)

    0x09,            // length
    TYPE_INTERFACE,
    0x00,            // interface number
    0x00,            // alt number
    0x02,            // # endpoints
    0xFF,
    0x42,
    0x03,
    0x00,            // interface string

    0x07,            // length
    TYPE_ENDPOINT,
    0x81,            // in, #1
    0x02,            // bulk
    0x00, 0x02,      // max packet 512
    0x00,            // interval
    
    0x07,            // length
    TYPE_ENDPOINT,
    0x01,            // out, #1
    0x02,            // bulk
    0x00, 0x02,      // max packet 512
    0x01,            // interval
};
    
static unsigned char config_desc_fs[] = {
    0x09,            // length
    TYPE_CONFIGURATION,
    0x20, 0x00,      // total length
    0x01,            // # interfaces
    0x01,            // config value
    0x00,            // config string
    0x80,            // attributes
    0x80,            // XXX max power (250ma)

    0x09,            // length
    TYPE_INTERFACE,
    0x00,            // interface number
    0x00,            // alt number
    0x02,            // # endpoints
    0xFF,
    0x42,
    0x03,
    0x00,            // interface string

    0x07,            // length
    TYPE_ENDPOINT,
    0x81,            // in, #1
    0x02,            // bulk
    0x40, 0x00,      // max packet 64
    0x00,            // interval
    
    0x07,            // length
    TYPE_ENDPOINT,
    0x01,            // out, #1
    0x02,            // bulk
    0x40, 0x00,      // max packet 64
    0x00,            // interval
};
    
typedef struct 
{
    void *data;
    unsigned short length;
    unsigned short id;
} dtable;

#define ID(type,num) ((type << 8) | num)

static dtable descr_hs[] = {
    { device_desc, sizeof(device_desc), ID(TYPE_DEVICE, 0) },
    { config_desc, sizeof(config_desc), ID(TYPE_CONFIGURATION, 0) },
    { manufacturer_string, sizeof(manufacturer_string), ID(TYPE_STRING, 1) },
    { product_string, sizeof(product_string), ID(TYPE_STRING, 2) },
    { default_string, sizeof(default_string), ID(TYPE_STRING, 4) },
    { language_table, sizeof(language_table), ID(TYPE_STRING, 0) },
    { 0, 0, 0 },
};    

static dtable descr_fs[] = {
    { device_desc, sizeof(device_desc), ID(TYPE_DEVICE, 0) },
    { config_desc_fs, sizeof(config_desc), ID(TYPE_CONFIGURATION, 0) },
    { manufacturer_string, sizeof(manufacturer_string), ID(TYPE_STRING, 1) },
    { product_string, sizeof(product_string), ID(TYPE_STRING, 2) },
    { default_string, sizeof(default_string), ID(TYPE_STRING, 4) },
    { language_table, sizeof(language_table), ID(TYPE_STRING, 0) },
    { 0, 0, 0 },
};    
