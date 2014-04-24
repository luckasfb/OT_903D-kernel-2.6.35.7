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

/******************************************************************************
*
* Filename:
* ---------
*     mt6573.h
*

*******************************************************************************/

#ifndef MT6573_H
#define MT6573_H

#include "preloader.h"


/* I/O mapping */
#define IO_PHYS                     0x70000000
#define IO_SIZE                     0x00100000


// TODO : remove following mt6516 setting
#define CONFIG_BASE                 (IO_PHYS + 0x00001000)
#define USB_BASE                    (IO_PHYS + 0x00050000)
#define USB11_BASE                  (IO_PHYS + 0x00060000)
#define DMA_BASE                    (IO_PHYS + 0x00022000)
#define AMCONFG_BASE                (IO_PHYS + 0x00039000)
#define RGU_BASE                    (IO_PHYS + 0x00003000)

/**************************************************************************
*  REGISTER BASE
**************************************************************************/


#define EMI_BASE                    0x70000000
#define FLEXL2_BASE                 0x70001000
#define AP_DMA_BASE                 0x70002000
#define UART1_BASE                  0x70003000
#define UART2_BASE                  0x70004000
#define UART3_BASE                  0x70005000
#define UART4_BASE                  0x70006000
#define APMCU_GPTIMER_BASE          0x70007000
#define EINT_BASE                   0x70008000
#define NFI_BASE                    0x70009000
#define NFIECC_BASE                 0x7000A000
#define I2C1_BASE                   0x7000B000
#define APMCU_LMU_BASE              0x7000C000
#define I2C2_BASE                   0x7000D000
#define MSDC0_BASE                  0x7000E000
#define MSDC1_BASE                  0x7000F000
#define MSDC2_BASE                  0x70010000
#define MSDC3_BASE                  0x70011000
#define KP_BASE                     0x70012000
#define SIM_BASE                    0x70013000
#define RTC_BASE                    0x70014000
#define PWM_BASE                    0x70015000
#define IRDA_BASE                   0x70016000
#define APMCU_CIRQ_BASE             0x70017000
#define DEVICE_APC0_BASE            0x70018000
                                    
#define AUXADC_BASE                 0x70020000
#define APMCU_OSTIMER_BASE          0x70021000
#define TOPSM_BASE                  0x70022000
#define GPIO_BASE                   0x70023000
#define EFUSE_BASE                  0x70024000
#define AP_RGU_BASE                 0x70025000
#define APCONFIG_BASE               0x70026000
#define HDQ_ONEWRITE_BASE           0x70027000
#define AP_CCIF1_BASE               0x70028000
#define AP_CCIF2_BASE               0x70029000
#define SEJ_BASE                    0x7002A000
#define ACCDET_BASE                 0x7002B000
#define APXGPT_BASE                 0x7002C000
#define SIM2_BASE                   0x7002D000
#define MIXEDSYS0_BASE              0x7002E000
#define MIXEDSYS1_BASE              0x7002F000
#define ATBBWL_BASE                 0x70030000
#define PERI_CLK_CTRL_BASE          0x70031000
#define TRNG_BASE                   0x70032000
#define DEVICE_APC1_BASE            0x70033000

#define PL310_BASE                  0x70040000
                                    
#define MMSYS1_CONFIG_BASE          0x70080000
#define GMC1_BASE                   0x70081000
#define GMC1_MMU_BASE               0x70082000
#define FDVT_BASE                   0x70083000
#define G2D_BASE                    0x70084000
#define GCMQ_BASE                   0x70085000
#define R_DMA0_BASE                 0x70086000
#define R_DMA1_BASE                 0x70087000
#define ROT_DMA0_BASE               0x70088000
#define ROT_DMA1_BASE               0x70089000
#define ROT_DMA2_BASE               0x7008A000
#define ROT_DMA3_BASE               0x7008B000
#define DPI_BASE                    0x70087008
#define BRZ_BASE                    0x7008D000
#define JPG_DMA_BASE                0x7008E000
#define OVL_BASE                    0x7008F000
#define CAM_BASE                    0x70090000
#define ISPMEM_BASE                 0x70091000
#define CSI2_BASE                   0x70092000
#define CRZ_BASE                    0x70093000
#define VRZ_BASE                    0x70094000
#define IMGPROC_BASE                0x70095000
#define EIS_BASE                    0x70096000
#define GIF_DECODER_BASE            0x70097000
#define PNG_DECODER_BASE            0x70098000
#define PRZ0_BASE                   0x70099000
#define PRZ1_BASE                   0x7009A000
#define JPG_CODEC_BASE              0x7009B000
#define DSI_BASE                    0x7009C000
#define TVC_BASE                    0x7009D000
#define TVE_BASE                    0x7009E000
#define TV_ROT_BASE                 0x7009F000
#define HIF_BASE                    0x7009A000
#define MM1_APC0_BASE               0x700A1000
#define MM1_APC1_BASE               0x700A2000
#define LCD_BASE                    0x700A3000

#define MPU_BASE                    0x70081100
#define MAU1_BASE                   0x70082400
#define MAU2_BASE                   0x700B1100

#define MMSYS2_CONFIG_BASE          0x700B0000
#define GMC2_BASE                   0x700B1000
#define SPI_BASE                    0x700B2000
#define MFV_BASE                    0x700B3000
#define MFG_BASE                    0x700B4000
#define MM2_APC_BASE                0x700B5000
                                    
#define DAPROM_BASE                 0x700F0000
#define ETB_BASE                    0x700F1000
#define CTI_BASE                    0x700F2000
#define TPIU_BASE                   0x700F3000
#define FUNNEL0_BASE                0x700F4000
#define FUNNEL1_BASE                0x700F5000
#define ARMCTI_BASE                 0x700F6000
#define FRIOCTI_BASE                0x700F7000
#define ETM_BASE                    0x700F8000
#define STP_BASE                    0x700F9000
#define DEM_BASE                    0x700FA000
#define SATA_BASE                   0x700FB000
#define APPMCUCTI_BASE              0x700FC000
                                    
#define SYSTEM_FLEXL2_BASE          0x79000000


//----------------------------------------------------------------------------
// GPT - Gerneral Purpose Timer Registers
#define MT6573_XGPT_IRQEN          (APXGPT_BASE + 0x00)
#define MT6573_XGPT_IRQSTA         (APXGPT_BASE + 0x04)
#define MT6573_XGPT_IRQACK         (APXGPT_BASE + 0x08)
#define MT6573_XGPT1_CON           (APXGPT_BASE + 0x10)
#define MT6573_XGPT1_PRESCALE      (APXGPT_BASE + 0x14)
#define MT6573_XGPT1_COUNT         (APXGPT_BASE + 0x18)
#define MT6573_XGPT1_COMPARE       (APXGPT_BASE + 0x1c)
#define MT6573_XGPT2_CON           (APXGPT_BASE + 0x20)
#define MT6573_XGPT2_PRESCALE      (APXGPT_BASE + 0x24)
#define MT6573_XGPT2_COUNT         (APXGPT_BASE + 0x28)
#define MT6573_XGPT2_COMPARE       (APXGPT_BASE + 0x2c)
#define MT6573_XGPT3_CON           (APXGPT_BASE + 0x30)
#define MT6573_XGPT3_PRESCALE      (APXGPT_BASE + 0x34)
#define MT6573_XGPT3_COUNT         (APXGPT_BASE + 0x38)
#define MT6573_XGPT3_COMPARE       (APXGPT_BASE + 0x3c)
#define MT6573_XGPT4_CON           (APXGPT_BASE + 0x40)
#define MT6573_XGPT4_PRESCALE      (APXGPT_BASE + 0x44)
#define MT6573_XGPT4_COUNT         (APXGPT_BASE + 0x48)
#define MT6573_XGPT4_COMPARE       (APXGPT_BASE + 0x4c)
#define MT6573_XGPT5_CON           (APXGPT_BASE + 0x50)
#define MT6573_XGPT5_PRESCALE      (APXGPT_BASE + 0x54)
#define MT6573_XGPT5_COUNT         (APXGPT_BASE + 0x58)
#define MT6573_XGPT5_COMPARE       (APXGPT_BASE + 0x5c)
#define MT6573_XGPT6_CON           (APXGPT_BASE + 0x60)
#define MT6573_XGPT6_PRESCALE      (APXGPT_BASE + 0x64)
#define MT6573_XGPT6_COUNT         (APXGPT_BASE + 0x68)
#define MT6573_XGPT6_COMPARE       (APXGPT_BASE + 0x6c)
#define MT6573_XGPT7_CON           (APXGPT_BASE + 0x70)
#define MT6573_XGPT7_PRESCALE      (APXGPT_BASE + 0x74)
#define MT6573_XGPT7_COUNT         (APXGPT_BASE + 0x78)
#define MT6573_XGPT7_COMPARE       (APXGPT_BASE + 0x7c)

#define APHW_CODE                  (APCONFIG_BASE + 0x0008)

/* MT6573 chip version */
#define MT6573_A68351B             0
#define MT6573_B68351B             1
#define MT6573_B68351D             2
#define MT6573_B68351E             3
#define MT6573_UNKNOWN_VER         0xFF

/* MT6573 EMI freq. definition */
#define EMI_52MHZ                  52000000
#define EMI_58_5MHZ                58500000
#define EMI_104MHZ                 104000000
#define EMI_117MHZ                 117000000
#define EMI_130MHZ                 130000000

/* MT6573 clock definitions */
#define CPU_468MHZ_EMI_117MHZ      1
#define CPU_234MHZ_EMI_117MHZ      2
#define CPU_416MHZ_EMI_104MHZ      3
#define CPU_208MHZ_EMI_104MHZ      4
#define CPU_468MHZ_EMI_58_5MHZ     5
#define CPU_234MHZ_EMI_58_5MHZ     6
#define CPU_416MHZ_EMI_52MHZ       7
#define CPU_208MHZ_EMI_52MHZ       8
#define CPU_390MHZ_EMI_130MHZ      9


/*=======================================================================*/
/* POWER Control                                                         */
/*=======================================================================*/
#define APMCUSYS_PDN_SET0          (AMCONFG_BASE+0x0320)
#define APMCUSYS_PDN_CLR0          (AMCONFG_BASE+0x0340)

/*=======================================================================*/
/* CPU/MEM CLK and TIMER                                                 */
/*=======================================================================*/
#define CFG_BOARD_FREQ             (CPU_416MHZ_EMI_104MHZ)


/*=======================================================================*/
/* POWER CONTROL                                                         */
/*=======================================================================*/
#if (CFG_BOARD_ID != MT6516_EVB)
#define CFG_POWER_CHARGING
#endif

/*=======================================================================*/
/* Partition Information                                                 */
/*=======================================================================*/

/* PCB Config Partition Information */
#define PART_PCB_MAGIC             0x58881688
#define PART_PCB_NAME              "PCB_CONFIG"
#define PCB_NAND_START_PAGE        PAGE_NUM(128*KB)
#define PCB_NAND_BLKS              PAGE_NUM(128*KB)

/* U-Boot Partition Information */
#define PART_UBOOT_MAGIC           0x58881688
#define PART_UBOOT_NAME            "U-BOOT"

/*=======================================================================*/
/* NAND Control                                                                 
/*=======================================================================*/
/* NAND Parameter */
#define CFG_MT6516_NAND_ID         0x4E414E44

#define NAND_PAGE_SIZE             (2048)  // (Bytes)
#define NAND_BLOCK_BLKS            (64)    // 64 nand pages = 128KB
#define NAND_PAGE_SHIFT            (9)
#define NAND_LARGE_PAGE            (11)    // large page
#define NAND_SMALL_PAGE            (9)     // small page
#define NAND_BUS_WIDTH_8           (8)
#define NAND_BUS_WIDTH_16          (16)
#define NAND_FDM_SIZE              (8)
#define NAND_ECC_SW                (0)
#define NAND_ECC_HW                (1)

#define NFI_MAX_FDM_SIZE           (8)
#define NFI_MAX_FDM_SEC_NUM        (8)
#define NFI_MAX_LOCK_CHANNEL       (16)

#define ECC_MAX_CORRECTABLE_BITS   (12)
#define ECC_MAX_PARITY_SIZE        (20)    /* in bytes */

#define ECC_ERR_LOCATION_MASK      (0x1FFF)
#define ECC_ERR_LOCATION_SHIFT     (16)

#define NAND_FFBUF_SIZE            (2048+64)

/*=======================================================================*/
/* PCB_CONFIG Image Address                                              
/*=======================================================================*/
#define PCB_IMG_ADDR               (0x40010000)

/*=======================================================================*/
/* U-Boot Image Address                                                  
/*=======================================================================*/
#define UBOOT_IMG_ADDR             (0x01e00000)
#define UBOOT_HEADER_SIZE          (0x00000200)

/*=======================================================================*/
/* Preloader Address and Size
/*=======================================================================*/
#define PL_ADDR                    (0x40002000)
#define PL_SIZE                    (0x00016000)
#define PL_RELOC_ADDR              (0x00600000)


/* MT6516 EMI freq. definition */
#define EMI_52MHZ                  52000000
#define EMI_58_5MHZ                58500000
#define EMI_104MHZ                 104000000
#define EMI_117MHZ                 117000000
#define EMI_130MHZ                 130000000

/* MT6516 clock definitions */
#define CPU_468MHZ_EMI_117MHZ      1
#define CPU_234MHZ_EMI_117MHZ      2
#define CPU_416MHZ_EMI_104MHZ      3
#define CPU_208MHZ_EMI_104MHZ      4
#define CPU_468MHZ_EMI_58_5MHZ     5
#define CPU_234MHZ_EMI_58_5MHZ     6
#define CPU_416MHZ_EMI_52MHZ       7
#define CPU_208MHZ_EMI_52MHZ       8
#define CPU_390MHZ_EMI_130MHZ      9

/*=======================================================================*/
/* USB Control                                                              
*=======================================================================*/
#ifndef MTK_EMMC_SUPPORT
#define MT6573_CFG_USB_DOWNLOAD
#endif

#define CFG_USB_DOWNLOAD
#define CONFIG_USBD_LANG           "0409"
#define USBD_MANUFACTURER          "MediaTek"
#define USBD_PRODUCT_NAME          "MT65xx Preloader"
#if 1                              //MT65xx VCOM
#define USBD_VENDORID              0x0E8D
#define USBD_PRODUCTID             0x2000
#endif
/*=======================================================================*/
/* DBG PRINT Control                                                              
*=======================================================================*/
#define DBG_PRELOADER
/*=======================================================================*/
/* WDT Control                                                              
*=======================================================================*/
/*=======================================================================*/
#define CONFIG_HW_WATCHDOG


/*=======================================================================*/
/* BOOT Check Control                                                               
/*=======================================================================*/
#define CFG_CHECK_BOOT_STATUS

typedef enum {
    BR_POWER_KEY    = 0,
    BR_USB          = 1,
    BR_RTC          = 2,
    BR_UNKNOWN      = 3,
} SYS_BOOT_REASON;

/*=======================================================================*/
/* COMMON BUFFER CONTROL                                                        
/*=======================================================================*/
#define COMMON_BUFFER_MAX_SIZE      (4500)
extern char g_common_buf[];

/*=======================================================================*/
/* BUFFER
/*=======================================================================*/
#define g_common_buf               (0x80000)


/*=======================================================================*/
/* TCM and CACHE CONTROL
/*=======================================================================*/

/*=======================================================================*/
/* PRELOADER DEBUG CONTROL
/*=======================================================================*/
#define PL_DEBUG_TRACER            (0)


/* debug pre-loader by tracing its execution path */
#if PL_DEBUG_TRACER
#define PL_TRACER dbg_print("PL TRACER: at %s #%d %s\n", __FILE__, __LINE__,
__FUNCTION__)
#else
#define PL_TRACER
#endif

#define dsb()   \
    __asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")

#endif
