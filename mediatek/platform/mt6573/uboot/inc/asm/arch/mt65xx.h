

#ifndef __MT6573_H__
#define __MT6573_H__

#include <asm/sizes.h>

/* MAX DRAM bank */
#define MAX_NR_BANK   4
#define DRAM_BANKS_NR get_bank_nr()

/* I/O mapping */
#define IO_PHYS            	0x70000000 // 2010-07-03 Koshi: Modified for MT6573
#define IO_SIZE            	0x00100000

#define MT6573_SDRAM_PA    	0x00000000

#define APCONFIG_BASE (IO_PHYS + 0x00026000)

/* IO register definitions */
#define EFUSE_BASE      	(IO_PHYS + 0x00000000)
#define CONFIG_BASE      	(IO_PHYS + 0x00026000)     
#define GPIO_BASE        	(IO_PHYS + 0x00023000)
#define RGU_BASE         	(IO_PHYS + 0x00025000)   //2010-09-15 
#define DVFS_BASE         	(IO_PHYS + 0x00004000)

/* APCONFIG */
#define APHW_VER        (CONFIG_BASE + 0x0000)

#define CIRQ_BASE        	(IO_PHYS + 0x00017000)
#define OST_BASE        	(IO_PHYS + 0x00021000)
#define TOPSM_BASE       	(IO_PHYS + 0x00022000)

#define AP_DMA_BASE      	(IO_PHYS + 0x00002000)
#define UART1_BASE 		    (IO_PHYS + 0x00003000)
#define UART2_BASE 		    (IO_PHYS + 0x00004000)
#define UART3_BASE 		    (IO_PHYS + 0x00005000)
#define UART4_BASE 		    (IO_PHYS + 0x00006000)
#define APMCU_GPTIMER_BASE  (IO_PHYS + 0x00007000)

#define HDQ_BASE         	(IO_PHYS + 0x00027000) // 6573 only    
#define KP_BASE         	(IO_PHYS + 0x00012000)
#define RTC_BASE         	(IO_PHYS + 0x00014000)
#define SEJ_BASE         	(IO_PHYS + 0x0002D000) // 6573 only
#define I2C3_BASE         	(IO_PHYS + 0x0002E000) // 6573
#define PMU_BASE       	(IO_PHYS + 0x0002F000) //(IO_PHYS + 0x0002F000)     

#define MIXEDSYS0_BASE 		(IO_PHYS + 0x0002E000) //

#define I2C_BASE         	(IO_PHYS + 0x00030000) // (IO_PHYS + 0x0002D000)
#define NFI_BASE         	(IO_PHYS + 0x00009000) // 2010-07-03 Koshi: Modified for MT6573 NNAND driver
#define NFIECC_BASE	        (IO_PHYS + 0x0000A000) // 2010-07-03 Koshi: Modified for MT6573 NNAND driver
#define MSDC0_BASE              (IO_PHYS + 0x0000E000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
#define MSDC1_BASE              (IO_PHYS + 0x0000F000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
#define MSDC2_BASE              (IO_PHYS + 0x00010000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
#define MSDC3_BASE              (IO_PHYS + 0x00011000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
#define PWM_BASE         	(IO_PHYS + 0x00015000)
#define SIM_BASE 	      	(IO_PHYS + 0x00033000) // 6573
#define I2C2_BASE        	(IO_PHYS + 0x00035000) // 6573
#define CCIF_BASE        	(IO_PHYS + 0x00036000) // 6573
#define AMCONFG_BASE     	(IO_PHYS + 0x00039000)
#define AP2MD_BASE	     	(IO_PHYS + 0x0003A000)
#define APVFE_BASE	     	(IO_PHYS + 0x0003B000)
#define APSLP_BASE	     	(IO_PHYS + 0x0003C000)
#define AUXADC_BASE	     	(IO_PHYS + 0x0003D000) // (IO_PHYS + 0x00034000)
#define APXGPT_BASE	     	(IO_PHYS + 0x0002C000)

#define CSDBG_BASE			(IO_PHYS + 0x00040000)

#define PLL_BASE        	(IO_PHYS + 0x00060000) // same
#define DSI_PHY_BASE        (IO_PHYS + 0x00080800)

#define GMC1_BASE			(IO_PHYS + 0x00080000)
#define G2D_BASE			(IO_PHYS + 0x00081000)
#define GCMQ_BASE			(IO_PHYS + 0x00082000)
#define GIFDEC_BASE			(IO_PHYS + 0x00083000)
#define IMGDMA_BASE			(IO_PHYS + 0x00084000)
#define PNGDEC_BASE			(IO_PHYS + 0x00085000)
#define MTVSPI_BASE			(IO_PHYS + 0x00087000)
#define TVCON_BASE			(IO_PHYS + 0x00088000)
#define TVENC_BASE			(IO_PHYS + 0x00089000)
#define CAM_BASE			(IO_PHYS + 0x0008A000)
#define CAM_ISP_BASE		        (IO_PHYS + 0x0008B000)
#define BLS_BASE			(IO_PHYS + 0x0008C000)
#define CRZ_BASE			(IO_PHYS + 0x0008D000)
#define DRZ_BASE			(IO_PHYS + 0x0008E000)
#define ASM_BASE			(IO_PHYS + 0x0008F000)

#define WT_BASE				 (IO_PHYS + 0x00090000)
#define IMG_BASE			 (IO_PHYS + 0x00091000)
#define MMSYS1_CONFIG_BASE             (IO_PHYS + 0x00080000)

#define GMC2_BASE			 (IO_PHYS + 0x000A0000)
#define JPEG_BASE			 (IO_PHYS + 0x000A1000)
#define M3D_BASE			 (IO_PHYS + 0x000A2000)
#define PRZ_BASE			 (IO_PHYS + 0x000A3000)
#define IMGDMA1_BASE		         (IO_PHYS + 0x000A4000)
#define MP4_DEBLK_BASE		         (IO_PHYS + 0x000A5000)
#define FAKE_ENG2_BASE		         (IO_PHYS + 0x000A6000)
#define GRAPH2SYS_BASE		         (IO_PHYS + 0x000A7000)

 #define MP4_BASE			 (IO_PHYS + 0x000C0000)
 #define H264_BASE			 (IO_PHYS + 0x000C1000)

#define USB_BASE            (IO_PHYS + 0x00100000) // Jau leave the definition to make compile pass

#define LCD_BASE           	(IO_PHYS + 0x000A3000) // same

#define DPI_BASE            (IO_PHYS + 0x0008C000) // same

#define DSI_BASE            (IO_PHYS + 0x0009C000) // same

/* EMI Registers */
#define EMI_CON0 					(EMI_BASE+0x0000) /* Bank 0 configuration */
#define EMI_CON1 					(EMI_BASE+0x0004) /* Bank 1 configuration */
#define EMI_CON2 					(EMI_BASE+0x0008) /* Bank 2 configuration */
#define EMI_CON3 					(EMI_BASE+0x000C) /* Bank 3 configuration */
#define EMI_CON4 					(EMI_BASE+0x0010) /* Boot Mapping config  */
#define	EMI_CON5 					(EMI_BASE+0x0014)
#define SDRAM_MODE 					(EMI_BASE+0x0020)
#define SDRAM_COMD 					(EMI_BASE+0x0024)
#define SDRAM_SET 					(EMI_BASE+0x0028)
#define SDRAM_BASE					0x00000000

/* AHB MCU-DSP SHARE RAM */
#define MCU_DSP_SHARE_RAM_BASE		(0xA0000000)
#define MCU_DSP_SHARE_RAM_SIZE      (0x01A0)        // 208*2
#define IDMA_PORT_BASE              (0xA2000000)

//----------------------------------------------------------------------------
#define MT6573_AP_MCU_SYS           (AMCONFG_BASE + 0x340)


//----------------------------------------------------------------------------
/* Powerdown module watch dog timer registers */
#define REG_RW_WATCHDOG_EN  		0x0100      // Watchdog Timer Control Register
#define REG_RW_WATCHDOG_TMR 		0x0104      // Watchdog Timer Register
#define REG_RW_WAKE_RSTCNT  		0x0108      // Wakeup Reset Counter Register

//----------------------------------------------------------------------------
// GPT - Gerneral Purpose Timer Registers
#define MT6573_XGPT_IRQEN	    (APXGPT_BASE + 0x00)
#define MT6573_XGPT_IRQSTA      (APXGPT_BASE + 0x04)
#define MT6573_XGPT_IRQACK      (APXGPT_BASE + 0x08)
#define MT6573_XGPT1_CON        (APXGPT_BASE + 0x10)
#define MT6573_XGPT1_PRESCALE   (APXGPT_BASE + 0x14)
#define MT6573_XGPT1_COUNT      (APXGPT_BASE + 0x18)
#define MT6573_XGPT1_COMPARE    (APXGPT_BASE + 0x1c)
#define MT6573_XGPT2_CON        (APXGPT_BASE + 0x20)
#define MT6573_XGPT2_PRESCALE   (APXGPT_BASE + 0x24)
#define MT6573_XGPT2_COUNT      (APXGPT_BASE + 0x28)
#define MT6573_XGPT2_COMPARE    (APXGPT_BASE + 0x2c)
#define MT6573_XGPT3_CON        (APXGPT_BASE + 0x30)
#define MT6573_XGPT3_PRESCALE   (APXGPT_BASE + 0x34)
#define MT6573_XGPT3_COUNT      (APXGPT_BASE + 0x38)
#define MT6573_XGPT3_COMPARE    (APXGPT_BASE + 0x3c)
#define MT6573_XGPT4_CON        (APXGPT_BASE + 0x40)
#define MT6573_XGPT4_PRESCALE   (APXGPT_BASE + 0x44)
#define MT6573_XGPT4_COUNT      (APXGPT_BASE + 0x48)
#define MT6573_XGPT4_COMPARE    (APXGPT_BASE + 0x4c)
#define MT6573_XGPT5_CON        (APXGPT_BASE + 0x50)
#define MT6573_XGPT5_PRESCALE   (APXGPT_BASE + 0x54)
#define MT6573_XGPT5_COUNT      (APXGPT_BASE + 0x58)
#define MT6573_XGPT5_COMPARE    (APXGPT_BASE + 0x5c)
#define MT6573_XGPT6_CON        (APXGPT_BASE + 0x60)
#define MT6573_XGPT6_PRESCALE   (APXGPT_BASE + 0x64)
#define MT6573_XGPT6_COUNT      (APXGPT_BASE + 0x68)
#define MT6573_XGPT6_COMPARE    (APXGPT_BASE + 0x6c)
#define MT6573_XGPT7_CON        (APXGPT_BASE + 0x70)
#define MT6573_XGPT7_PRESCALE   (APXGPT_BASE + 0x74)
#define MT6573_XGPT7_COUNT      (APXGPT_BASE + 0x78)
#define MT6573_XGPT7_COMPARE    (APXGPT_BASE + 0x7c)

#define PLL_CON5_REG            (MIXEDSYS0_BASE+0x114)

/* HW's reserved for bonding option or parameters */
#define M_HW_RES0               (IO_PHYS + 0x24100)

/* MT6573 chip version */
#define MT6573_A68351B              0
#define MT6573_B68351B              1
#define MT6573_B68351D              2
#define MT6573_B68351E              3
#define MT6573_UNKNOWN_VER          0xFF

/* MT6573 EMI freq. definition */
#define EMI_52MHZ                   52000000
#define EMI_58_5MHZ                 58500000
#define EMI_104MHZ                  104000000
#define EMI_117MHZ                  117000000
#define EMI_130MHZ                  130000000

/* MT6573 clock definitions */
#define CPU_468MHZ_EMI_117MHZ       1
#define CPU_234MHZ_EMI_117MHZ       2
#define CPU_416MHZ_EMI_104MHZ       3
#define CPU_208MHZ_EMI_104MHZ       4
#define CPU_468MHZ_EMI_58_5MHZ      5
#define CPU_234MHZ_EMI_58_5MHZ      6
#define CPU_416MHZ_EMI_52MHZ        7
#define CPU_208MHZ_EMI_52MHZ        8
#define CPU_390MHZ_EMI_130MHZ       9

/* MT6573 board definitions */
#define MT6573_EVB                  1
#define MT6573_PHN                  2
#define MT6573_GEMINI               3
#define MT6573_OPPO                 4

/* MT6573 storage type */
#define DEV_UNKNOWN                 0
#define DEV_NAND                    1
#define DEV_NOR                     2
#define DEV_INAND                   3
#define DEV_MOVINAND                4
#define DEV_ENAND                   5
#define DEV_MMC_SD                  6

/* MT6573 storage boot type definitions */
#define NON_BOOTABLE                0
#define RAW_BOOT                    1
#define FAT_BOOT                    2

#define CONFIG_STACKSIZE	    (128*1024)	  /* regular stack */

// xuecheng, define this because we use zlib for boot logo compression
#define CONFIG_ZLIB 	1

// =======================================================================
// UBOOT DEBUG CONTROL
// =======================================================================
#define UBOOT_DEBUG_TRACER			(0)


/* debug u-boot by tracing its execution path */
#if UBOOT_DEBUG_TRACER
#define UBOOT_TRACER dbg_print("UBOOT TRACER: at %s #%d %s\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define UBOOT_TRACER 
#endif

#define dsb()	\
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")

#endif  /* !__MT6573_H__ */
