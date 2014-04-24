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


#ifndef __MT6516_DMA_H__
#define __MT6516_DMA_H__

#include <mt6573.h>
#include <mt6573_typedefs.h>

#define DMA_MAX_COUNT           65535

/*
 *  Full-Size    Half-Size    Virtual FIFO
 *          -            -               -
 *        SRC            -               -
 *        DST            -               -
 *       WPPT         WPPT               -
 *       WPTO         WPTO               - 
 *      COUNT        COUNT           COUNT
 *        CON          CON             CON
 *      START        START           START
 *     INTSTA       INTSTA          INTSTA
 *     ACKINT       ACKINT          ACKINT
 *       RLCT         RLCT            RLCT
 *    LIMITER      LIMITER         LIMITER
 *          -      PGMADDR         PGMADDR
 *          -            -           WRPTR
 *          -            -           RDPTR
 *          -            -           FFCNT
 *          -            -           FFSTA
 *          -            -           ALTLEN
 *          -            -           FFSIZE
 */
#define DMA_BASE_CH(n)      (DMA_BASE + 0x0080*(n+1))

#define DMA_GLBSTA_L        (DMA_BASE + 0x0000)
#define DMA_GLBSTA_H        (DMA_BASE + 0x0004)

#define DMA_VPORT_BASE      (0x80110000)
#define DMA_VPORT_CH(ch)    (DMA_VPORT_BASE + (ch - 16) * 0x00002000)

#define DMA_SRC(base)       ((volatile u32*)(base+0x0))
#define DMA_DST(base)       ((volatile u32*)(base+0x4))
#define DMA_WPPT(base)      ((volatile u32*)(base+0x8))
#define DMA_WPTO(base)      ((volatile u32*)(base+0xc))
#define DMA_COUNT(base)     ((volatile u32*)(base+0x10))
#define DMA_CON(base)       ((volatile u32*)(base+0x14))
#define DMA_START(base)     ((volatile u32*)(base+0x18))
#define DMA_INTSTA(base)    ((volatile u32*)(base+0x1c))
#define DMA_ACKINT(base)    ((volatile u32*)(base+0x20))
#define DMA_RLCT(base)      ((volatile u32*)(base+0x24))
#define DMA_LIMITER(base)   ((volatile u32*)(base+0x28))
#define DMA_PGMADDR(base)   ((volatile u32*)(base+0x2c))
#define DMA_WRPTR(base)     ((volatile u32*)(base+0x30))
#define DMA_RDPTR(base)     ((volatile u32*)(base+0x34))
#define DMA_FFCNT(base)     ((volatile u32*)(base+0x38))
#define DMA_FFSTA(base)     ((volatile u32*)(base+0x3c))
#define DMA_ALTLEN(base)    ((volatile u32*)(base+0x40))
#define DMA_FFSIZE(base)    ((volatile u32*)(base+0x44))


#define DMA_GLBSTA_RUN(ch)  (0x00000001 << (2*(ch)))
#define DMA_GLBSTA_IT(ch)   (0x00000002 << (2*(ch)))

#define DMA_COUNT_MASK      0x0000ffff

#define DMA_CON_SINC        0x00000004
#define DMA_CON_DINC        0x00000008
#define DMA_CON_DRQ         0x00000010  /*1:hw, 0:sw handshake */
#define DMA_CON_B2W         0x00000020  /*word to byte or byte to word, only used in half size dma */
#define DMA_CON_ITEN        0x00008000  /*Interrupt enable */
#define DMA_CON_WPSD        0x00010000  /*0:at source, 1: at destination */
#define DMA_CON_WPEN        0x00020000  /*0:disable, 1: enable */
#define DMA_CON_DIR         0x00040000  /*Only valid when dma = 9 ~ 20 */

#define DMA_START_BIT       0x00008000
#define DMA_STOP_BIT        0x00000000
#define DMA_ACKINT_BIT      0x00008000
#define DMA_INTSTA_BIT      0x00008000

#define DMA_FFSTA_FULL      0x00000001
#define DMA_FFSTA_EMPTY     0x00000002
#define DMA_FFSTA_ALT       0x00000004

/* MASTER */
#define DMA_CON_MASTER_SIM2         (0  << 20)
#define DMA_CON_MASTER_MSDC0        (1  << 20)
#define DMA_CON_MASTER_MSDC1        (2  << 20)
#define DMA_CON_MASTER_IRDATX       (3  << 20)
#define DMA_CON_MASTER_IRDARX       (4  << 20)
#define DMA_CON_MASTER_UART0TX      (5  << 20)
#define DMA_CON_MASTER_UART0RX      (6  << 20)
#define DMA_CON_MASTER_UART1TX      (7  << 20)
#define DMA_CON_MASTER_UART1RX      (8  << 20)
#define DMA_CON_MASTER_UART2TX      (9  << 20)
#define DMA_CON_MASTER_UART2RX      (10 << 20)
#define DMA_CON_MASTER_NFITX        (11 << 20)
#define DMA_CON_MASTER_NFIRX        (12 << 20)
#define DMA_CON_MASTER_VFE          (13 << 20)
#define DMA_CON_MASTER_I2CTX        (14 << 20)
#define DMA_CON_MASTER_I2CRX        (15 << 20)
#define DMA_CON_MASTER_UART3TX      (16 << 20)
#define DMA_CON_MASTER_UART3RX      (17 << 20)
#define DMA_CON_MASTER_MSDC2        (18 << 20)


/* burst */
#define DMA_CON_BURST_SINGLE    0x00000000      /*without burst mode */
#define DMA_CON_BURST_4BEAT     0x00000200      /*4-beat incrementing burst */
#define DMA_CON_BURST_8BEAT     0x00000400      /*8-beat incrementing burst */
#define DMA_CON_BURST_16BEAT    0x00000600      /*16-beat incrementing burst */

/* size */
#define DMA_CON_SIZE_BYTE       0x00000000
#define DMA_CON_SIZE_SHORT      0x00000001
#define DMA_CON_SIZE_LONG       0x00000002

#define NR_DMA_FULL_CHANNEL     8
#define NR_DMA_HALF_CHANNEL     8
#define NR_DMA_VFIFO_CHANNEL    8
#define DMA_FS_START            0
#define DMA_HS_START            (DMA_FS_START + NR_DMA_FULL_CHANNEL)
#define DMA_VF_START            (DMA_HS_START + NR_DMA_HALF_CHANNEL)

#define USB_FULL_DMA_CH0        (DMA_FS_START + 0)
#define USB_FULL_DMA_CH1        (DMA_FS_START + 1)
#define USB_FULL_DMA_CH2        (DMA_FS_START + 2)

#define USB_FULL_DMA0_BASE      (DMA_BASE_CH(USB_FULL_DMA_CH0))
#define USB_FULL_DMA1_BASE      (DMA_BASE_CH(USB_FULL_DMA_CH1))
#define USB_FULL_DMA2_BASE      (DMA_BASE_CH(USB_FULL_DMA_CH2))

#define MSDC_HALF_DMA_CH0       (DMA_HS_START + 0)
#define MSDC_HALF_DMA_CH1       (DMA_HS_START + 1)
#define MSDC_HALF_DMA_CH2       (DMA_HS_START + 2)

#define MSDC_HALF_DMA0_BASE     (DMA_BASE_CH(MSDC_HALF_DMA_CH0))
#define MSDC_HALF_DMA1_BASE     (DMA_BASE_CH(MSDC_HALF_DMA_CH1))
#define MSDC_HALF_DMA2_BASE     (DMA_BASE_CH(MSDC_HALF_DMA_CH2))

#endif /* __MT6516_DMA_H__ */
