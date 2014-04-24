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

divend        .req    r0
divsor        .req    r1
res           .req    r2
curb          .req    r3

        .text
        .globl   __udivsi3
        .type   __udivsi3 ,function
        .globl  __aeabi_uidiv
        .type   __aeabi_uidiv ,function
        .align  0

 __udivsi3:

 __aeabi_uidiv:
        cmp     divsor, #0
        beq     LD0
        mov     curb, #1
        mov     res, #0
        cmp     divend, divsor
        bcc     Lgot_res

L1:
        cmp     divsor, #0x10000000
        cmpcc   divsor, divend
        movcc   divsor, divsor, lsl #4
        movcc   curb, curb, lsl #4
        bcc     L1

LB:
        cmp     divsor, #0x80000000
        cmpcc   divsor, divend
        movcc   divsor, divsor, lsl #1
        movcc   curb, curb, lsl #1
        bcc     LB

L3:

        cmp     divend, divsor
        subcs   divend, divend, divsor
        orrcs   res, res, curb
        cmp     divend, divsor, lsr #1
        subcs   divend, divend, divsor, lsr #1
        orrcs   res, res, curb, lsr #1
        cmp     divend, divsor, lsr #2
        subcs   divend, divend, divsor, lsr #2
        orrcs   res, res, curb, lsr #2
        cmp     divend, divsor, lsr #3
        subcs   divend, divend, divsor, lsr #3
        orrcs   res, res, curb, lsr #3
        cmp     divend, #0                   
        movnes  curb, curb, lsr #4          
        movne   divsor, divsor, lsr #4
        bne     L3

Lgot_res:
        mov     r0, res
        mov     pc, lr

LD0:
        str     lr, [sp, #-4]!
        bl       __div0       (PLT)
        mov     r0, #0                  
        ldmia   sp!, {pc}
        .size  __udivsi3       , . -  __udivsi3

.globl __aeabi_uidivmod
__aeabi_uidivmod:
        stmfd   sp!, {r0, r1, ip, lr}
        bl      __aeabi_uidiv
        ldmfd   sp!, {r1, r2, ip, lr}
        mul     r3, r0, r2
        sub     r1, r1, r3
        mov     pc, lr

.globl __aeabi_idivmod

__aeabi_idivmod:

        stmfd   sp!, {r0, r1, ip, lr}
        bl      __aeabi_idiv
        ldmfd   sp!, {r1, r2, ip, lr}
        mul     r3, r0, r2
        sub     r1, r1, r3
	mov     pc, lr

