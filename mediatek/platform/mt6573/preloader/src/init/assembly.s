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


;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@
;@ modification history
;# 20100412 : Shu-Hsin enlarge stack size from 0x800 to 0x1800
;@
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.section .text.start

#define Mode_USR   	0x10
#define Mode_FIQ   	0x11
#define Mode_IRQ   	0x12
#define Mode_SVC   	0x13
#define Mode_ABT   	0x17
#define Mode_UNDEF 	0x1B
#define Mode_SYS        0x1F    
#define I_F_Bit         0xC0
#define Stack_Ptr       0x800

#define C1_IBIT        &00001000
#define ISRAM_Base     0x40000000

   
#include "preloader.h"
#include "pll.h"
#include "mt6573.h"
      
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.section .text.start
      
.globl _start
_start : 
    b resethandler
    
.globl _bss_globl_start
_bss_globl_start:
    .word _bss_start

.globl _bss_end
_bss_end:
    .word _bss_end
  
    
resethandler :

    MOV     r0,  #0
    MOV     r1,  #0
    MOV     r2,  #0
    MOV     r3,  #0
    MOV     r4,  #0
    MOV     r5,  #0
    MOV     r6,  #0
    MOV     r7,  #0
    MOV     r8,  #0
    MOV     r9,  #0
    MOV     r10, #0
    MOV     r11, #0
    MOV     r12, #0
    MOV     r13, #0
    MOV     lr,  #0

    /* set the cpu to SVC32 mode */
    mrs	r0,cpsr
    bic	r0,r0,#0x1f
    orr	r0,r0,#0xd3
    msr	cpsr,r0

    /* disable interrupt */
    MRS     r0, cpsr
    MOV     r1, #(0x80 | 0x40)
    ORR     r0, r0, r1
    MSR     cpsr_cxsf, r0    
      
    /* TODO : enable Icache */
    mrc   p15, 0, ip, c1, c0, 0
    orr   ip, ip, #0x1800
    mcr   p15, 0, ip, c1, c0, 0    

    /* Set stack */     
    mov   r1, #0x40000000      
    add   r1, r1, #0x1800
    SUB   r1, r1, #0x04
    MOV   SP, r1

clear_bss :
    ldr	r0, _bss_globl_start  /* find start of bss segment */
    ldr	r1, _bss_end    /* stop here */
    mov	r2, #0x00000000 /* clear */
    
    cmp r0, r1
    beq entry
    
    /*  clear loop... */
clbss_l : 
    str	r2, [r0]    
    add	r0, r0, #4
    cmp	r0, r1
    bne	clbss_l

entry :
    /* Disable watch dog timer & Init CP15 setting */                 
    BL	  primary_sys_init
    B     C_Main


.globl JumpCmd   
JumpCmd:
      MOV  pc, r0
