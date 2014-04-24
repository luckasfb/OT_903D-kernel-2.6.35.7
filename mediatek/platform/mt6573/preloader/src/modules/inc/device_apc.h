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


#ifndef _MTK_DEVICE_APC_H
#define _MTK_DEVICE_APC_H

#include "mt6573_typedefs.h"

#define AP_DEVAPC0_base             0x70018000
#define AP_DEVAPC1_base             0x70033000
#define MD_DEVAPC_base              0x60150000
#define DSP_DEVAPC_base             0xFF070000
#define MM1_DEVAPC0_base            0x700A1000
#define MM1_DEVAPC1_base            0x700A2000
#define MM2_DEVAPC0_base            0x700B5000

#define AP_DEVAPC0_D0_APC_0		   ((volatile unsigned int*)(AP_DEVAPC0_base+0x0000))
#define AP_DEVAPC0_D0_APC_1        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0004))
#define AP_DEVAPC0_D1_APC_0        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0008))
#define AP_DEVAPC0_D1_APC_1        ((volatile unsigned int*)(AP_DEVAPC0_base+0x000C))
#define AP_DEVAPC0_D2_APC_0        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0010))
#define AP_DEVAPC0_D2_APC_1        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0014))
#define AP_DEVAPC0_D0_VIO_MASK     ((volatile unsigned int*)(AP_DEVAPC0_base+0x0020))
#define AP_DEVAPC0_D1_VIO_MASK     ((volatile unsigned int*)(AP_DEVAPC0_base+0x0024))
#define AP_DEVAPC0_D2_VIO_MASK     ((volatile unsigned int*)(AP_DEVAPC0_base+0x0028))
#define AP_DEVAPC0_D0_VIO_STA      ((volatile unsigned int*)(AP_DEVAPC0_base+0x0030))
#define AP_DEVAPC0_D1_VIO_STA      ((volatile unsigned int*)(AP_DEVAPC0_base+0x0034))
#define AP_DEVAPC0_D2_VIO_STA      ((volatile unsigned int*)(AP_DEVAPC0_base+0x0038))
#define AP_DEVAPC0_VIO_DBG0        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0040))
#define AP_DEVAPC0_VIO_DBG1        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0044))
#define AP_DEVAPC0_APC_CON         ((volatile unsigned int*)(AP_DEVAPC0_base+0x0090))
#define AP_DEVAPC0_APC_LOCK        ((volatile unsigned int*)(AP_DEVAPC0_base+0x0094))

#define AP_DEVAPC1_D0_APC_0		   ((volatile unsigned int*)(AP_DEVAPC1_base+0x0000))
#define AP_DEVAPC1_D0_APC_1        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0004))
#define AP_DEVAPC1_D1_APC_0        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0008))
#define AP_DEVAPC1_D1_APC_1        ((volatile unsigned int*)(AP_DEVAPC1_base+0x000C))
#define AP_DEVAPC1_D2_APC_0        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0010))
#define AP_DEVAPC1_D2_APC_1        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0014))
#define AP_DEVAPC1_D0_VIO_MASK     ((volatile unsigned int*)(AP_DEVAPC1_base+0x0020))
#define AP_DEVAPC1_D1_VIO_MASK     ((volatile unsigned int*)(AP_DEVAPC1_base+0x0024))
#define AP_DEVAPC1_D2_VIO_MASK     ((volatile unsigned int*)(AP_DEVAPC1_base+0x0028))
#define AP_DEVAPC1_D0_VIO_STA      ((volatile unsigned int*)(AP_DEVAPC1_base+0x0030))
#define AP_DEVAPC1_D1_VIO_STA      ((volatile unsigned int*)(AP_DEVAPC1_base+0x0034))
#define AP_DEVAPC1_D2_VIO_STA      ((volatile unsigned int*)(AP_DEVAPC1_base+0x0038))
#define AP_DEVAPC1_VIO_DBG0        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0040))
#define AP_DEVAPC1_VIO_DBG1        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0044))
#define AP_DEVAPC1_DXS_VIO_MASK    ((volatile unsigned int*)(AP_DEVAPC1_base+0x0080))
#define AP_DEVAPC1_DXS_VIO_STA     ((volatile unsigned int*)(AP_DEVAPC1_base+0x0084))
#define AP_DEVAPC1_APC_CON         ((volatile unsigned int*)(AP_DEVAPC1_base+0x0090))
#define AP_DEVAPC1_APC_LOCK        ((volatile unsigned int*)(AP_DEVAPC1_base+0x0094))
#define AP_DEVAPC1_MAS_DOM         ((volatile unsigned int*)(AP_DEVAPC1_base+0x00A0))
#define AP_DEVAPC1_MAS_SEC         ((volatile unsigned int*)(AP_DEVAPC1_base+0x00A4))
#define AP_DEVAPC1_DEC_ERR_CON     ((volatile unsigned int*)(AP_DEVAPC1_base+0x00B4))
#define AP_DEVAPC1_DEC_ERR_ADDR    ((volatile unsigned int*)(AP_DEVAPC1_base+0x00B8))

#define MD_DEVAPC_D0_APC_0		  ((volatile unsigned int*)(MD_DEVAPC_base+0x0000))
#define MD_DEVAPC_D0_APC_1        ((volatile unsigned int*)(MD_DEVAPC_base+0x0004))
#define MD_DEVAPC_D1_APC_0        ((volatile unsigned int*)(MD_DEVAPC_base+0x0008))
#define MD_DEVAPC_D1_APC_1        ((volatile unsigned int*)(MD_DEVAPC_base+0x000C))
#define MD_DEVAPC_D2_APC_0        ((volatile unsigned int*)(MD_DEVAPC_base+0x0010))
#define MD_DEVAPC_D2_APC_1        ((volatile unsigned int*)(MD_DEVAPC_base+0x0014))
#define MD_DEVAPC_D0_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC_base+0x0020))
#define MD_DEVAPC_D1_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC_base+0x0024))
#define MD_DEVAPC_D2_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC_base+0x0028))
#define MD_DEVAPC_D0_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC_base+0x0030))
#define MD_DEVAPC_D1_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC_base+0x0034))
#define MD_DEVAPC_D2_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC_base+0x0038))
#define MD_DEVAPC_VIO_DBG0        ((volatile unsigned int*)(MD_DEVAPC_base+0x0040))
#define MD_DEVAPC_VIO_DBG1        ((volatile unsigned int*)(MD_DEVAPC_base+0x0044))
#define MD_DEVAPC_DXS_VIO_MASK    ((volatile unsigned int*)(MD_DEVAPC_base+0x0080))
#define MD_DEVAPC_DXS_VIO_STA     ((volatile unsigned int*)(MD_DEVAPC_base+0x0084))
#define MD_DEVAPC_APC_CON         ((volatile unsigned int*)(MD_DEVAPC_base+0x0090))
#define MD_DEVAPC_APC_LOCK        ((volatile unsigned int*)(MD_DEVAPC_base+0x0094))
#define MD_DEVAPC_MAS_DOM         ((volatile unsigned int*)(MD_DEVAPC_base+0x00A0))
#define MD_DEVAPC_MAS_SEC         ((volatile unsigned int*)(MD_DEVAPC_base+0x00A4))
#define MD_DEVAPC_DEC_ERR_CON     ((volatile unsigned int*)(MD_DEVAPC_base+0x00B4))
#define MD_DEVAPC_DEC_ERR_ADDR    ((volatile unsigned int*)(MD_DEVAPC_base+0x00B8))

#define DSP_DEVAPC_D0_APC_0		   ((volatile unsigned int*)(DSP_DEVAPC_base+0x0000))
#define DSP_DEVAPC_D0_APC_1        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0004))
#define DSP_DEVAPC_D1_APC_0        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0008))
#define DSP_DEVAPC_D1_APC_1        ((volatile unsigned int*)(DSP_DEVAPC_base+0x000C))
#define DSP_DEVAPC_D2_APC_0        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0010))
#define DSP_DEVAPC_D2_APC_1        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0014))
#define DSP_DEVAPC_D0_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC_base+0x0020))
#define DSP_DEVAPC_D1_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC_base+0x0024))
#define DSP_DEVAPC_D2_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC_base+0x0028))
#define DSP_DEVAPC_D0_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC_base+0x0030))
#define DSP_DEVAPC_D1_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC_base+0x0034))
#define DSP_DEVAPC_D2_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC_base+0x0038))
#define DSP_DEVAPC_VIO_DBG0        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0040))
#define DSP_DEVAPC_VIO_DBG1        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0044))
#define DSP_DEVAPC_DXS_VIO_MASK    ((volatile unsigned int*)(DSP_DEVAPC_base+0x0080))
#define DSP_DEVAPC_DXS_VIO_STA     ((volatile unsigned int*)(DSP_DEVAPC_base+0x0084))
#define DSP_DEVAPC_APC_CON         ((volatile unsigned int*)(DSP_DEVAPC_base+0x0090))
#define DSP_DEVAPC_APC_LOCK        ((volatile unsigned int*)(DSP_DEVAPC_base+0x0094))
#define DSP_DEVAPC_MAS_DOM         ((volatile unsigned int*)(DSP_DEVAPC_base+0x00A0))
#define DSP_DEVAPC_MAS_SEC         ((volatile unsigned int*)(DSP_DEVAPC_base+0x00A4))
#define DSP_DEVAPC_DEC_ERR_CON     ((volatile unsigned int*)(DSP_DEVAPC_base+0x00B4))
#define DSP_DEVAPC_DEC_ERR_ADDR    ((volatile unsigned int*)(DSP_DEVAPC_base+0x00B8))

#define MM1_DEVAPC0_D0_APC_0 	    ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0000))
#define MM1_DEVAPC0_D0_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0004))
#define MM1_DEVAPC0_D1_APC_0        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0008))
#define MM1_DEVAPC0_D1_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x000C))
#define MM1_DEVAPC0_D2_APC_0        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0010))
#define MM1_DEVAPC0_D2_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0014))
#define MM1_DEVAPC0_D0_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0020))
#define MM1_DEVAPC0_D1_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0024))
#define MM1_DEVAPC0_D2_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0028))
#define MM1_DEVAPC0_D0_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0030))
#define MM1_DEVAPC0_D1_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0034))
#define MM1_DEVAPC0_D2_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0038))
#define MM1_DEVAPC0_VIO_DBG0        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0040))
#define MM1_DEVAPC0_VIO_DBG1        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0044))
#define MM1_DEVAPC0_APC_CON         ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0090))
#define MM1_DEVAPC0_APC_LOCK        ((volatile unsigned int*)(MM1_DEVAPC0_base+0x0094))
#define MM1_DEVAPC0_DEC_ERR_CON     ((volatile unsigned int*)(MM1_DEVAPC0_base+0x00B4))
#define MM1_DEVAPC0_DEC_ERR_ADDR    ((volatile unsigned int*)(MM1_DEVAPC0_base+0x00B8))

#define MM1_DEVAPC1_D0_APC_0 	    ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0000))
#define MM1_DEVAPC1_D1_APC_0        ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0008))
#define MM1_DEVAPC1_D2_APC_0        ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0010))
#define MM1_DEVAPC1_D0_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0020))
#define MM1_DEVAPC1_D1_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0024))
#define MM1_DEVAPC1_D2_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0028))
#define MM1_DEVAPC1_D0_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0030))
#define MM1_DEVAPC1_D1_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0034))
#define MM1_DEVAPC1_D2_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0038))
#define MM1_DEVAPC1_VIO_DBG0        ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0040))
#define MM1_DEVAPC1_VIO_DBG1        ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0044))
#define MM1_DEVAPC1_APC_CON         ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0090))
#define MM1_DEVAPC1_APC_LOCK        ((volatile unsigned int*)(MM1_DEVAPC1_base+0x0094))

#define MM2_DEVAPC_D0_APC_0		    ((volatile unsigned int*)(MM2_DEVAPC_base+0x0000))
#define MM2_DEVAPC_D1_APC_0         ((volatile unsigned int*)(MM2_DEVAPC_base+0x0008))
#define MM2_DEVAPC_D2_APC_0         ((volatile unsigned int*)(MM2_DEVAPC_base+0x0010))
#define MM2_DEVAPC_D0_VIO_MASK      ((volatile unsigned int*)(MM2_DEVAPC_base+0x0020))
#define MM2_DEVAPC_D1_VIO_MASK      ((volatile unsigned int*)(MM2_DEVAPC_base+0x0024))
#define MM2_DEVAPC_D2_VIO_MASK      ((volatile unsigned int*)(MM2_DEVAPC_base+0x0028))
#define MM2_DEVAPC_D0_VIO_STA       ((volatile unsigned int*)(MM2_DEVAPC_base+0x0030))
#define MM2_DEVAPC_D1_VIO_STA       ((volatile unsigned int*)(MM2_DEVAPC_base+0x0034))
#define MM2_DEVAPC_D2_VIO_STA       ((volatile unsigned int*)(MM2_DEVAPC_base+0x0038))
#define MM2_DEVAPC_VIO_DBG0         ((volatile unsigned int*)(MM2_DEVAPC_base+0x0040))
#define MM2_DEVAPC_VIO_DBG1         ((volatile unsigned int*)(MM2_DEVAPC_base+0x0044))
#define MM2_DEVAPC_APC_CON          ((volatile unsigned int*)(MM2_DEVAPC_base+0x0090))
#define MM2_DEVAPC_APC_LOCK         ((volatile unsigned int*)(MM2_DEVAPC_base+0x0094))
#define MM2_DEVAPC_DEC_ERR_CON      ((volatile unsigned int*)(MM2_DEVAPC_base+0x00B4))
#define MM2_DEVAPC_DEC_ERR_ADDR     ((volatile unsigned int*)(MM2_DEVAPC_base+0x00B8))

/* DOMAIN_SETUP */
#define DOMAIN_AP						0
#define DOMAIN_FCORE					1
#define DOMAIN_MD						2

/* Masks for Domain Control for AP DEVAPC1 */
#define APPER_USB_1_DOM_CTRL            (0x3 << 10)  /* R/W */
#define APPER_USB_2_DOM_CTRL            (0x3 << 8)   /* R/W */
#define APPER_PWM_DOM_CTRL              (0x3 << 6)   /* R/W */
#define APPER_NFI_DOM_CTRL              (0x3 << 4)   /* R/W */
#define DEBUG_DOM_CTRL                  (0x3 << 2)   /* R/W */
#define AUDIO_DOM_CTRL                  (0x3 << 0)   /* R/W */

/* Masks for Domain Control for MD DEVAPC */
#define MDPER_LOGACC_DOM_CTRL           (0x3 << 8)   /* R/W */
#define MDPER_PFC_DOM_CTRL              (0x3 << 6)   /* R/W */
#define MD_DMA_DOM_CTRL                 (0x3 << 4)   /* R/W */
#define MD_3G_DOM_CTRL                  (0x3 << 2)   /* R/W */
#define MD_2G_DOM_CTRL                  (0x3 << 0)   /* R/W */

/* Masks for Domain Control for DSP DEVAPC */
#define FCORE_D1_PORT_DOM_CTRL          (0x3 << 4)   /* R/W */
#define FCORE_D0_PORT_DOM_CTRL          (0x3 << 2)   /* R/W */
#define FCORE_I_PORT_DOM_CTRL           (0x3 << 0)   /* R/W */

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#define reg_read16(reg)          __raw_readw(reg)
#define reg_read32(reg)          __raw_readl(reg)
#define reg_write16(reg,val)     __raw_writew(val,reg)
#define reg_write32(reg,val)     __raw_writel(val,reg)
 
#define reg_set_bits(reg,bs)     ((*(volatile u32*)(reg)) |= (u32)(bs))
#define reg_clr_bits(reg,bs)     ((*(volatile u32*)(reg)) &= ~((u32)(bs)))
 
#define reg_set_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = reg_read32(reg); \
         tv &= ~(field); \
         tv |= ((val) << (uffs((unsigned int)field) - 1)); \
         reg_write32(reg,tv); \
     } while(0)
     
#define reg_get_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = reg_read32(reg); \
         val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
     } while(0)

#endif