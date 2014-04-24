/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef AT91_TC_H
#define AT91_TC_H

typedef struct at91_tcc {
	u32		ccr;	/* 0x00 Channel Control Register */
	u32		cmr;	/* 0x04 Channel Mode Register */
	u32		reserved1[2];
	u32		cv;	/* 0x10 Counter Value */
	u32		ra;	/* 0x14 Register A */
	u32		rb;	/* 0x18 Register B */
	u32		rc;	/* 0x1C Register C */
	u32		sr;	/* 0x20 Status Register */
	u32		ier;	/* 0x24 Interrupt Enable Register */
	u32		idr;	/* 0x28 Interrupt Disable Register */
	u32		imr;	/* 0x2C Interrupt Mask Register */
	u32		reserved3[4];
} __attribute__ ((packed)) at91_tcc_t;

#define AT91_TC_CCR_CLKEN		0x00000001
#define AT91_TC_CCR_CLKDIS		0x00000002
#define AT91_TC_CCR_SWTRG		0x00000004

#define AT91_TC_CMR_CPCTRG		0x00004000

#define AT91_TC_CMR_TCCLKS_CLOCK1	0x00000000
#define AT91_TC_CMR_TCCLKS_CLOCK2	0x00000001
#define AT91_TC_CMR_TCCLKS_CLOCK3	0x00000002
#define AT91_TC_CMR_TCCLKS_CLOCK4	0x00000003
#define AT91_TC_CMR_TCCLKS_CLOCK5	0x00000004
#define AT91_TC_CMR_TCCLKS_XC0		0x00000005
#define AT91_TC_CMR_TCCLKS_XC1		0x00000006
#define AT91_TC_CMR_TCCLKS_XC2		0x00000007

typedef struct at91_tc {
	at91_tcc_t	tc[3];	/* 0x00 TC Channel 0-2 */
	u32		bcr;	/* 0xC0 TC Block Control Register */
	u32		bmr;	/* 0xC4 TC Block Mode Register */
} __attribute__ ((packed)) at91_tc_t;

#define AT91_TC_BMR_TC0XC0S_TCLK0	0x00000000
#define AT91_TC_BMR_TC0XC0S_NONE	0x00000001
#define AT91_TC_BMR_TC0XC0S_TIOA1	0x00000002
#define AT91_TC_BMR_TC0XC0S_TIOA2	0x00000003

#define AT91_TC_BMR_TC1XC1S_TCLK1	0x00000000
#define AT91_TC_BMR_TC1XC1S_NONE	0x00000004
#define AT91_TC_BMR_TC1XC1S_TIOA0	0x00000008
#define AT91_TC_BMR_TC1XC1S_TIOA2	0x0000000C

#define AT91_TC_BMR_TC2XC2S_TCLK2	0x00000000
#define AT91_TC_BMR_TC2XC2S_NONE	0x00000010
#define AT91_TC_BMR_TC2XC2S_TIOA0	0x00000020
#define AT91_TC_BMR_TC2XC2S_TIOA1	0x00000030

#endif