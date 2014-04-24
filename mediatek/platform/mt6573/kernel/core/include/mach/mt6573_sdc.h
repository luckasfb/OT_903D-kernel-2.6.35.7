
#ifndef __MT6573_SDC_H__
#define __MT6573_SDC_H__

#include <asm/io.h>
#include <mach/mmc_core.h>
//#include <asm/arch/mt65xx_typedefs.h>

#define IO_PHYS            	0xF7020000 // 2010-07-03 Koshi: Modified for MT6573
//#define APCONFIG_BASE (IO_PHYS + 0x00026000)
//#define MIXEDSYS0_BASE 		(IO_PHYS + 0x0002E000)
#define PLL_CON5_REG            (MIXEDSYS0_BASE+0x114)

#define IO_PHYS2            	0xF7000000 // 2010-07-03 Koshi: Modified for MT6573
#define MSDC1_BASE              (IO_PHYS2 + 0x0000E000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
//#define MSDC2_BASE              (IO_PHYS2 + 0x0000F000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
//#define MSDC3_BASE              (IO_PHYS2 + 0x00010000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver
//#define MSDC4_BASE              (IO_PHYS2 + 0x00011000) // 2010-07-09 Koshi: Modified for MT6573 MSDC driver

#define MSDC_MAX_NUM                    (4)

/* MSDC registers */
#define MSDC_CFG                        ((volatile u32*)(base+0x0))
#define MSDC_STA                        ((volatile u32*)(base+0x04))
#define MSDC_INT                        ((volatile u32*)(base+0x08))
#define MSDC_PS                         ((volatile u32*)(base+0x0c))
#define MSDC_DAT                        ((volatile u32*)(base+0x10))
#define MSDC_IOCON0                     ((volatile u32*)(base+0x14))
#define MSDC_IOCON1                     ((volatile u32*)(base+0x18))
/* SDC registers */
#define SDC_CFG                         ((volatile u32*)(base+0x20))
#define SDC_CMD                         ((volatile u32*)(base+0x24))
#define SDC_ARG                         ((volatile u32*)(base+0x28))
#define SDC_STA                         ((volatile u32*)(base+0x2c)) // 16bits
#define SDC_RESP0                       ((volatile u32*)(base+0x30))
#define SDC_RESP1                       ((volatile u32*)(base+0x34))
#define SDC_RESP2                       ((volatile u32*)(base+0x38))
#define SDC_RESP3                       ((volatile u32*)(base+0x3c))
#define SDC_CMDSTA                      ((volatile u32*)(base+0x40)) // 16bits
#define SDC_DATSTA                      ((volatile u32*)(base+0x44)) // 16bits
#define SDC_CSTA                        ((volatile u32*)(base+0x48))
#define SDC_IRQMASK0                    ((volatile u32*)(base+0x4c))
#define SDC_IRQMASK1                    ((volatile u32*)(base+0x50))
/* SDIO registers */
#define SDIO_CFG                        ((volatile u32*)(base+0x54))
#define SDIO_STA                        ((volatile u32*)(base+0x58))
/* MMC boot up registers */
#define BOOT_CFG                        ((volatile u32*)(base+0x70))
#define BOOT_STA                        ((volatile u32*)(base+0x74))
#define BOOT_IOCON                      ((volatile u32*)(base+0x78))
/* Auto calibration registers */
#define CLKACB_CFG                      ((volatile u32*)(base+0x80))
#define CLKACB_STA                      ((volatile u32*)(base+0x84))
#define CLKACB_CRCSTA1                  ((volatile u32*)(base+0x88))
#define CLKACB_CRCSTA2                  ((volatile u32*)(base+0x8c))
 
/* MSDC DMA registers */
#define DMA_INT_FLAG                    ((volatile u32*)(base+0x0))
#define DMA_INT_EN                      ((volatile u32*)(base+0x04))
#define DMA_EN                          ((volatile u32*)(base+0x08))
#define DMA_RST                         ((volatile u32*)(base+0x0c))
#define DMA_STOP                        ((volatile u32*)(base+0x10))
#define DMA_FLUSH                       ((volatile u32*)(base+0x14))
#define DMA_CTRL                        ((volatile u32*)(base+0x18))
#define DMA_MEM_ADDR                    ((volatile u32*)(base+0x1c))
#define DMA_LEN                         ((volatile u32*)(base+0x24))
#define DMA_INTBUF_SZ                   ((volatile u32*)(base+0x38))
#define DMA_DBG_STS                     ((volatile u32*)(base+0x50))
 
/* Masks for MSDC_CFG */
#define MSDC_CFG_MSDC                   (0x1 << 0)      /* R/W */
#define MSDC_CFG_RST                    (0x1 << 1)      /* W */
#define MSDC_CFG_NOCRC                  (0x1 << 2)      /* R/W */
#define MSDC_CFG_CLKSRC                 (0x3 << 3)      /* R/W */
#define MSDC_CFG_STDBY                  (0x1 << 5)      /* R/W */
#define MSDC_CFG_RED                    (0x1 << 6)      /* R/W */
#define MSDC_CFG_SCLKON                 (0x1 << 7)      /* R/W */
#define MSDC_CFG_SCLKF                  (0xff << 8)     /* R/W */
#define MSDC_CFG_INTEN                  (0x1 << 16)     /* R/W */
#define MSDC_CFG_DMAEN                  (0x1 << 17)     /* R/W */
#define MSDC_CFG_PINEN                  (0x1 << 18)     /* R/W */
#define MSDC_CFG_DIRQE                  (0x1 << 19)     /* R/W */
#define MSDC_CFG_RCDEN                  (0x1 << 20)     /* R/W */
#define MSDC_CFG_VDDPD                  (0x1 << 21)     /* R/W */
#define MSDC_CFG_FIFOTHD                (0x1f << 24)    /* R/W */
 
/* Masks for MSDC_STA */
#define MSDC_STA_BF                     (0x1 << 0)      /* RO */
#define MSDC_STA_BE                     (0x1 << 1)      /* RO */
#define MSDC_STA_DRQ                    (0x1 << 2)      /* RO */
#define MSDC_STA_INT                    (0x1 << 3)      /* RO */
#define MSDC_STA_FIFOCNT                (0x1f << 4)     /* RO */
#define MSDC_STA_FIFOCLR                (0x1 << 14)     /* W */
#define MSDC_STA_BUSY                   (0x1 << 15)     /* R */
 
/* Masks for MSDC_INT */
#define MSDC_INT_DIRQ                   (0x1 << 0)      /* R/C */
#define MSDC_INT_PINIRQ                 (0x1 << 1)      /* R/C */
#define MSDC_INT_SDCMDIRQ               (0x1 << 2)      /* R/C */
#define MSDC_INT_SDDATIRQ               (0x1 << 3)      /* R/C */
#define MSDC_INT_SDMCIRQ                (0x1 << 4)      /* R/C */
#define MSDC_INT_MSIFIRQ                (0x1 << 5)      /* R/C */
#define MSDC_INT_SDR1BIRQ               (0x1 << 6)      /* R/C */
#define MSDC_INT_SDIOIRQ                (0x1 << 7)      /* R/C */
 
/* Masks for MSDC_PS */
#define MSDC_PS_CDEN                    (0x1 << 0)      /* R/W */
#define MSDC_PS_PIEN0                   (0x1 << 1)      /* R/W */
#define MSDC_PS_POEN0                   (0x1 << 2)      /* R/W */
#define MSDC_PS_PIN0                    (0x1 << 3)      /* R */
#define MSDC_PS_PINCH                   (0x1 << 4)      /* R/C */
#define MSDC_PS_DEBOUNCE                (0xf << 12)     /* R/W */
#define MSDC_PS_DAT                     (0xff << 16)    /* RO */
#define MSDC_PS_CMD                     (0x1 << 24)     /* RO */
 
/* Masks for MSDC_IOCON0 */
#define MSDC_IOCON0_ODCCFG0             (0x7 << 0)      /* R/W */
#define MSDC_IOCON0_ODCCFG1             (0x7 << 3)      /* R/W */
#define MSDC_IOCON0_SRCFG0              (0x1 << 6)      /* R/W */
#define MSDC_IOCON0_SRCFG1              (0x1 << 7)      /* R/W */
#define MSDC_IOCON0_DMABRUST            (0x3 << 8)      /* R/W */
#define MSDC_IOCON0_CMDRE               (0x1 << 15)     /* R/W */
#define MSDC_IOCON0_DSW                 (0x1 << 16)     /* R/W */
#define MSDC_IOCON0_INTLH               (0x3 << 17)     /* R/W */
#define MSDC_IOCON0_CMDSEL              (0x1 << 19)     /* R/W */
#define MSDC_IOCON0_CRCDIS              (0x1 << 20)     /* R/W */
#define MSDC_IOCON0_SAMPON              (0x1 << 21)     /* R/W */
#define MSDC_IOCON0_FIXDLY              (0x3 << 22)     /* R/W */
#define MSDC_IOCON0_SAMPLEDLY           (0x3 << 24)     /* R/W */
 
/* Masks for MSDC_IOCON1 */
#define MSDC_IOCON1_PRVAL_INS           (0x3 << 0)      /* R/W */
#define MSDC_IOCON1_PRCFG_INS           (0x1 << 2)      /* R/W */
#define MSDC_IOCON1_PRVAL_DAT           (0x3 << 4)      /* R/W */
#define MSDC_IOCON1_PRCFG_DAT           (0x1 << 6)      /* R/W */
#define MSDC_IOCON1_PRVAL_CMD           (0x3 << 8)      /* R/W */
#define MSDC_IOCON1_PRCFG_CMD           (0x1 << 10)     /* R/W */
#define MSDC_IOCON1_PRVAL_CLK           (0x3 << 12)     /* R/W */
#define MSDC_IOCON1_PRCFG_CLK           (0x1 << 14)     /* R/W */
#define MSDC_IOCON1_PRVAL_RST_WP        (0x3 << 16)     /* R/W */
#define MSDC_IOCON1_PRCFG_RST_WP        (0x1 << 18)     /* R/W */
 
/* Masks for SDC_CFG */
#define SDC_CFG_BLKLEN                  (0xfff << 0)    /* R/W */
#define SDC_CFG_BSYDLY                  (0xf << 12)     /* R/W */
#define SDC_CFG_SIEN                    (0x1 << 16)     /* R/W */
#define SDC_CFG_MDLEN                   (0x1 << 17)     /* R/W */
#define SDC_CFG_MDLEN8                  (0x1 << 18)     /* R/W */
#define SDC_CFG_SDIO                    (0x1 << 19)     /* R/W */
#define SDC_CFG_WDOD                    (0xf << 20)     /* R/W */
#define SDC_CFG_DTOC                    (0xffUL << 24)  /* R/W */
 
/* Masks for SDC_CMD */
#define SDC_CMD_CMD                     (0x3f << 0)     /* R/W */
#define SDC_CMD_BREAK                   (0x1 << 6)      /* R/W */
#define SDC_CMD_RSPTYP                  (0x7 << 7)      /* R/W */
#define SDC_CMD_IDRT                    (0x1 << 10)     /* R/W */
#define SDC_CMD_DTYPE                   (0x3 << 11)     /* R/W */
#define SDC_CMD_RW                      (0x1 << 13)     /* R/W */
#define SDC_CMD_STOP                    (0x1 << 14)     /* R/W */
#define SDC_CMD_INTC                    (0x1 << 15)	    /* R/W */
#define SDC_CMD_CMDFAIL                 (0x1 << 16)     /* R/W */
 
/* Masks for SDC_STA */
#define SDC_STA_BESDCBUSY               (0x1 << 0)      /* RO */
#define SDC_STA_BECMDBUSY               (0x1 << 1)      /* RO */
#define SDC_STA_BEDATBUSY               (0x1 << 2)      /* RO */
#define SDC_STA_FECMDBUSY               (0x1 << 3)      /* RO */
#define SDC_STA_FEDATBUSY               (0x1 << 4)      /* RO */
#define SDC_STA_WP                      (0x1 << 15)     /* R */
 
/* Masks for SDC_CMDSTA */
#define SDC_CMDSTA_CMDRDY               (0x1 << 0)      /* R/C */
#define SDC_CMDSTA_CMDTO                (0x1 << 1)      /* R/C */
#define SDC_CMDSTA_RSPCRCERR            (0x1 << 2)      /* R/C */
#define SDC_CMDSTA_MMCIRQ               (0x1 << 3)      /* R/C */

/* Masks for SDC_DATSTA */
#define SDC_DATSTA_BLKDONE              (0x1 << 0)      /* R/C */
#define SDC_DATSTA_DATTO                (0x1 << 1)      /* R/C */
#define SDC_DATSTA_DATCRCERR            (0xff << 2)     /* R/C */

#define SDC_DATSTA_ERR                  (SDC_DATSTA_DATTO|SDC_DATSTA_DATCRCERR)

/* Masks for SDC_CSTA */
#define SDC_CSTA_AKE_SEQ_ERR            (0x1 << 3)      /* R/C */
#define SDC_CSTA_CID_CSD_OVW            (0x1 << 16)     /* R/C */
#define SDC_CSTA_OVERRUN                (0x1 << 17)     /* R/C */
#define SDC_CSTA_UNDERRUN               (0x1 << 18)     /* R/C */
#define SDC_CSTA_ERROR                  (0x1 << 19)     /* R/C */
#define SDC_CSTA_CC_ERR                 (0x1 << 20)     /* R/C */
#define SDC_CSTA_CARD_ECC_FAILED        (0x1 << 21)     /* R/C */
#define SDC_CSTA_ILLEGAL_CMD            (0x1 << 22)     /* R/C */
#define SDC_CSTA_COM_CRC_ERR            (0x1 << 23)     /* R/C */
#define SDC_CSTA_LK_ULK_FAILED          (0x1 << 24)     /* R/C */
#define SDC_CSTA_WP_VIOLATION           (0x1 << 26)     /* R/C */
#define SDC_CSTA_ERASE_PARM             (0x1 << 27)     /* R/C */
#define SDC_CSTA_ERASE_SEQ_ERR          (0x1 << 28)     /* R/C */
#define SDC_CSTA_BLK_LEN_ERR            (0x1 << 29)     /* R/C */
#define SDC_CSTA_ADDR_ERR               (0x1 << 30)     /* R/C */
#define SDC_CSTA_OUT_OF_RANGE           (0x1 << 31)     /* R/C */
 
/* Masks for SDC_IRQMASK0 */
#define SDC_IRQMASK0_CMDRDY             (0x1 << 0)
#define SDC_IRQMASK0_CMDTO              (0x1 << 1)
#define SDC_IRQMASK0_RSPCRCERR          (0x1 << 2)
#define SDC_IRQMASK0_MMCIRQ             (0x1 << 3)
#define SDC_IRQMASK0_BLKDONE            (0x1 << 16)
#define SDC_IRQMASK0_DATTO              (0x1 << 17)
#define SDC_IRQMASK0_DATACRCERR         (0xff << 18)    /* CHECKME!!! */
 
/* Masks for SDC_IRQMASK1 */
#define SDC_IRQMASK1_AKE_SEQ_ERR        (0x1 << 3)
#define SDC_IRQMASK1_CID_CSD_OVW        (0x1 << 16)
#define SDC_IRQMASK1_OVERRUN            (0x1 << 17)
#define SDC_IRQMASK1_UNDERRUN           (0x1 << 18)
#define SDC_IRQMASK1_ERROR              (0x1 << 19)
#define SDC_IRQMASK1_CC_ERR             (0x1 << 20)
#define SDC_IRQMASK1_CARD_ECC_FAILED    (0x1 << 21)
#define SDC_IRQMASK1_ILLEGAL_CMD        (0x1 << 22)
#define SDC_IRQMASK1_COM_CRC_ERR        (0x1 << 23)
#define SDC_IRQMASK1_LK_ULK_FAILED      (0x1 << 24)
#define SDC_IRQMASK1_WP_VIOLATION       (0x1 << 26)
#define SDC_IRQMASK1_ERASE_PARM         (0x1 << 27)
#define SDC_IRQMASK1_ERASE_SEQ_ERR      (0x1 << 28)
#define SDC_IRQMASK1_BLK_LEN_ERR        (0x1 << 29)
#define SDC_IRQMASK1_ADDR_ERR           (0x1 << 30)
#define SDC_IRQMASK1_OUT_OF_RANGE       (0x1 << 31)
 
/* Masks for SDIO_CFG */
#define SDIO_CFG_INTEN                  (0x1 << 0)  /* R/W */
#define SDIO_CFG_INTSEL                 (0x1 << 1)  /* R/W */
#define SDIO_CFG_DSBSEL                 (0x1 << 2)  /* R/W */
 
/* Masks for SDIO_STA */
#define SDIO_STA_IRQ                    (0x1 << 0)  /* RO */
 
/* Masks for BOOT_CFG */
#define BOOT_CFG_START                  (0x1 << 0)          /* W1C */
#define BOOT_CFG_STOP                   (0x1 << 1)          /* W1C */
#define BOOT_CFG_MODE                   (0x1 << 2)          /* R/W */
#define BOOT_CFG_ACKTOC                 (0xff << 4)         /* R/W */
#define BOOT_CFG_WAITDLY                (0x7 << 12)         /* R/W */
#define BOOT_CFG_SUPPORT                (0x1 << 15)         /* R/W */
#define BOOT_CFG_DATTOC                 (0xffffUL << 16)    /* R/W */
 
/* Masks for BOOT_STA */
#define BOOT_STA_CRCERR                 (0x1 << 0)  /* R/C */
#define BOOT_STA_ACKERR                 (0x1 << 1)  /* R/C */
#define BOOT_STA_DATTMO                 (0x1 << 2)  /* R/C */
#define BOOT_STA_ACKTMO                 (0x1 << 3)  /* R/C */
#define BOOT_STA_UPSTATE                (0x1 << 4)  /* R/O */
#define BOOT_STA_ACKRCV                 (0x1 << 5)  /* R/O */
#define BOOT_STA_WAITEXIT               (0x1 << 6)  /* R/O */
 
/* Masks for BOOT_IOCON */
#define BOOT_IOCON_RST                  (0x1 << 0)  /* R/W */
 
/* Masks for CLKACB_CFG */
#define CLKACB_CFG_TUN_ON               (0x1 << 0)  /* W1C */
#define CLKACB_CFG_COS_EN               (0x1 << 1)  /* R/W */
#define CLKACB_CFG_FINE_EN              (0x1 << 2)  /* R/W */
#define CLKACB_CFG_SCAN_MODE            (0x1 << 3)  /* R/W */
#define CLKACB_CFG_SMPL_PERIOD          (0x3 << 4)  /* R/W */
#define CLKACB_CFG_CLK_LAT              (0x1 << 6)  /* R/W */
#define CLKACB_CFG_CLK_RED              (0x1 << 7)  /* R/W */
#define CLKACB_CFG_DAT_PHA              (0x1f<< 8)  /* R/W */
#define CLKACB_CFG_DAT_PHA_RED          (0x1 << 13) /* R/W */
#define CLKACB_CFG_TUN_BLK_NUM          (0xff << 16)/* R/W */
#define CLKACB_CFG_CMD_PHA              (0x1f << 24)/* R/W */
#define CLKACB_CFG_CMD_PHA_RED          (0x1 << 29) /* R/W */
#define CLKACB_CFG_ACB_MODE             (0x1 << 31) /* R/W */
 
/* Masks for CLKACB_STA */
#define CLKACB_STA_TUN_DONE             (0x1 << 0)  /* R/C */
#define CLKACB_STA_DET_DONE             (0x1 << 1)  /* R/C */
#define CLKACB_STA_COS_DONE             (0x1 << 2)  /* R/C */
#define CLKACB_STA_FIN_DONE             (0x1 << 3)  /* R/C */
#define CLKACB_STA_ACB_TMO              (0x1 << 4)  /* R/C */
#define CLKACB_STA_DET_CLK_PHA          (0xf << 8)  /* R/O */
 
#define MSDC_DSW_NODELAY                (0)
#define MSDC_DSW_DELAY                  (1)
 
#define MSDC_DMA_BURST_1BEAT            (0)
#define MSDC_DMA_BURST_4BEAT            (1)
#define MSDC_DMA_BURST_8BEAT            (2)

#define EMMC_BOOT_PULL_CMD_MODE         (0)
#define EMMC_BOOT_RST_CMD_MODE          (1)
 
enum {
    PIN_PULL_UP   = 0,
    PIN_PULL_DOWN = 1,
};

#define SDC_CMD_CMD17 0x0891
#define SDC_CMD_CMD18 0x1092
#define SDC_CMD_CMD24 0x2898
#define SDC_CMD_CMD25 0x3099
#define SDC_CMD_CMD12 0x438c
#define SDC_CMD_CMD13 0x8d

#define READ_BLOCK_TIMEOUT   100
#define WRITE_BLOCK_TIMEOUT  250
#define WAIT_TIME 0x200000

typedef struct {
    u32 msdc:1;
    u32 rst:1;
    u32 nocrc:1;
    u32 clksrc:2;
    u32 stdby:1;
    u32 red:1;
    u32 sclkon:1;
    u32 sclkf:8;
    u32 inten:1;
    u32 dmaen:1;
    u32 pinen:1;
    u32 dirqen:1;
    u32 rcden:1;
    u32 vddpd:1;
    u32 pad1:2;
    u32 fifothd:5;
    u32 pad2:3;
} msdc_cfg_reg;
typedef struct {
    u32 bf:1;
    u32 be:1;
    u32 drq:1;
    u32 intr:1;
    u32 fifocnt:5;
    u32 pad1:5;
    u32 fifoclr:1;
    u32 busy:1;
    u32 pad2:16;
} msdc_sta_reg;
typedef struct {
    u32 dirq:1;
    u32 pinirq:1;
    u32 sdcmdirq:1;
    u32 sddatirq:1;
    u32 sdmcirq:1;
    u32 msifirq:1;
    u32 sdr1b:1;
    u32 sdioirq:1;
    u32 pad:24;
} msdc_int_reg;
typedef struct {
    u32 val;
} msdc_dat_reg;
typedef struct {
    u32 cden:1;
    u32 pien0:1;
    u32 poen0:1;
    u32 pin0:1;
    u32 pinchg:1;
    u32 pad1:7;
    u32 cddebounce:4;
    u32 dat:8;
    u32 cmd:1;
    u32 pad2:7;
} msdc_ps_reg;
typedef struct {
    u32 odccfg0:3;
    u32 odccfg1:3;
    u32 srcfg0:1;
    u32 srcfg1:1;
    u32 dmaburst:2;
    u32 pad1:5;
    u32 cmdre:1;
    u32 dsw:1;
    u32 intlh:2;
    u32 cmdsel:1;
    u32 crcdis:1;
    u32 smplon:1;
    u32 fixdly:2;
    u32 smpldly:2;
    u32 pad2:6;
} msdc_iocon0_reg;
typedef struct {
    u32 prvins:2;
    u32 prcfins:1;
    u32 pad1:1;
    u32 prvdat:2;
    u32 prcfdat:1;
    u32 pad2:1;
    u32 prvcmd:2;
    u32 prcfcmd:1;
    u32 pad3:1;
    u32 prvclk:2;
    u32 prcfclk:1;
    u32 pad4:1;
    u32 prvrstwp:2;
    u32 prcfrstwp:1;
    u32 pad5:13;
} msdc_iocon1_reg;
typedef struct {
    u32 blklen:12;
    u32 busydly:4;
    u32 sien:1;
    u32 mdlen:1;
    u32 mdlw8:1;
    u32 sdio:1;
    u32 wdod:4;
    u32 dtoc:8;
} sdc_cfg_reg;
typedef struct {
    u32 cmd:6;
    u32 brk:1;
    u32 rsptyp:3;
    u32 idrt:1;
    u32 dtype:2;
    u32 rw:1;
    u32 stop:1;
    u32 intc:1;
    u32 cmdfail:1;
    u32 pad:15;
} sdc_cmd_reg;
typedef struct {
    u32 arg;
} sdc_arg_reg;
typedef struct {
    u32 besdcbusy:1;
    u32 becmdbusy:1;
    u32 bedatbusy:1;
    u32 fecmdbusy:1;
    u32 fedatbusy:1;
    u32 pad1:10;
    u32 wp:1;
    u32 pad2:16;
} sdc_sta_reg;
typedef struct {
    u32 val;
} sdc_resp0_reg;
typedef struct {
    u32 val;    
} sdc_resp1_reg;
typedef struct {
    u32 val;    
} sdc_resp2_reg;
typedef struct {
    u32 val;    
} sdc_resp3_reg;
typedef struct {
    u32 cmdrdy:1;
    u32 cmtto:1;
    u32 rspcrcerr:1;
    u32 mmcirq:1;
    u32 pad:28;
} sdc_cmdsta_reg;
typedef struct {
    u32 blkdone:1;
    u32 datto:1;
    u32 datcrcerr:8;
    u32 pad:22;
} sdc_datsta_reg;
typedef struct {
    u32 csta;
} sdc_csta_reg;
typedef struct {
    u32 cmdrdy:1;
    u32 cmdto:1;
    u32 rspcrerr:1;
    u32 mmcirq:1;
    u32 pad1:12;
    u32 blkdone:1;
    u32 datto:1;
    u32 datcrcerr:8;
    u32 pad2:6;
} sdc_irqmask0_reg;
typedef struct {
    u32 pad1:3;
    u32 ake_seq_err:1;
    u32 pad2:12;
    u32 cid_csd_ovw:1;
    u32 overrun:1;
    u32 underrun:1;
    u32 error:1;
    u32 cc_err:1;
    u32 card_ecc_failed:1;
    u32 ill_cmd:1;
    u32 com_crc_err:1;
    u32 lk_ulk_failed:1;
    u32 pad3:1;
    u32 wp_violation:1;
    u32 erase_parm:1;
    u32 erase_seq_err:1;
    u32 blk_len_err:1;
    u32 addr_err:1;
    u32 out_of_range:1;
} sdc_irqmask1_reg;
typedef struct {
    u32 inten:1;
    u32 intsel:1;
    u32 dsbsel:1;
    u32 reserved:29;
} sdio_cfg_reg;
typedef struct {
    u32 irq:1;
    u32 reserved:31;
} sdio_sta_reg;
typedef struct {
    u32 start:1;
    u32 stoop:1;
    u32 mode:1;
    u32 pad1:1;
    u32 acktoc:8;
    u32 waitdly:3;
    u32 support:1;
    u32 dattoc:16;
} boot_cfg_reg;
typedef struct {
    u32 crcerr:1;
    u32 ackerr:1;
    u32 dattmo:1;
    u32 acktmo:1;
    u32 upstate:1;
    u32 ackrcv:1;
    u32 waitexit:1;
    u32 pad:25;
} boot_sta_reg;
typedef struct {
    u32 rst:1;
    u32 pad:31;
} boot_iocon_reg;
typedef struct {
    u32 clktunon:1;
    u32 coarseen:1;
    u32 fineen:1;
    u32 smplperiod:2;
    u32 clklatch:1;
    u32 clkred:1;
    u32 datphase:5;
    u32 datphasered:1;
    u32 pad1:2;
    u32 tunblknum:8;
    u32 cmdphase:5;
    u32 cmdphasered:1;
    u32 pad2:1;
    u32 acbmod:1;
} clkacb_cfg_reg;
typedef struct {
    u32 clktundone:1;
    u32 detdone:1;
    u32 coarsedone:1;
    u32 finedone:1;
    u32 acbtmo:1;
    u32 pad1:3;
    u32 detclkphase:4;
    u32 pad2:20;
} clkacb_sta_reg;
typedef struct {
    u32 ph0crc:1;
    u32 ph1crc:1;
    u32 ph2crc:1;
    u32 ph3crc:1;
    u32 ph4crc:1;
    u32 ph5crc:1;
    u32 ph6crc:1;
    u32 ph7crc:1;
    u32 ph8crc:1;
    u32 ph9crc:1;
    u32 ph10crc:1;
    u32 ph11crc:1;
    u32 ph12crc:1;
    u32 ph13crc:1;
    u32 ph14crc:1;
    u32 ph15crc:1;
    u32 ph0acb:1;
    u32 ph1acb:1;
    u32 ph2acb:1;
    u32 ph3acb:1;
    u32 ph4acb:1;
    u32 ph5acb:1;
    u32 ph6acb:1;
    u32 ph7acb:1;
    u32 ph8acb:1;
    u32 ph9acb:1;
    u32 ph10acb:1;
    u32 ph11acb:1;
    u32 ph12acb:1;
    u32 ph13acb:1;
    u32 ph14acb:1;
    u32 ph15acb:1;
} clkacb_crcsta1_reg;
typedef struct {
    u32 ph0crc:1;
    u32 ph1crc:1;
    u32 ph2crc:1;
    u32 ph3crc:1;
    u32 ph4crc:1;
    u32 ph5crc:1;
    u32 ph6crc:1;
    u32 ph7crc:1;
    u32 ph8crc:1;
    u32 ph9crc:1;
    u32 ph10crc:1;
    u32 ph11crc:1;
    u32 ph12crc:1;
    u32 ph13crc:1;
    u32 ph14crc:1;
    u32 ph15crc:1;
    u32 ph0acb:1;
    u32 ph1acb:1;
    u32 ph2acb:1;
    u32 ph3acb:1;
    u32 ph4acb:1;
    u32 ph5acb:1;
    u32 ph6acb:1;
    u32 ph7acb:1;
    u32 ph8acb:1;
    u32 ph9acb:1;
    u32 ph10acb:1;
    u32 ph11acb:1;
    u32 ph12acb:1;
    u32 ph13acb:1;
    u32 ph14acb:1;
    u32 ph15acb:1;
} clkacb_crcsta2_reg;

struct msdc_regs {
    msdc_cfg_reg        msdc_cfg;       /* base+0x00h */
    msdc_sta_reg        msdc_sta;       /* base+0x04h */
    msdc_int_reg        msdc_int;       /* base+0x08h */
    msdc_ps_reg         msdc_ps;        /* base+0x0Ch */    
    msdc_dat_reg        msdc_dat;       /* base+0x10h */    
    msdc_iocon0_reg     msdc_iocon0;    /* base+0x14h */
    msdc_iocon1_reg     msdc_iocon1;    /* base+0x18h */
    u32                 reserved1[1];
    sdc_cfg_reg         sdc_cfg;        /* base+0x20h */
    sdc_cmd_reg         sdc_cmd;        /* base+0x24h */
    sdc_arg_reg         sdc_arg;        /* base+0x28h */
    sdc_sta_reg         sdc_sta;        /* base+0x2Ch */
    sdc_resp0_reg       sdc_resp0;      /* base+0x30h */
    sdc_resp1_reg       sdc_resp1;      /* base+0x34h */
    sdc_resp2_reg       sdc_resp2;      /* base+0x38h */
    sdc_resp3_reg       sdc_resp3;      /* base+0x3Ch */
    sdc_cmdsta_reg      sdc_cmdsta;     /* base+0x40h */
    sdc_datsta_reg      sdc_datsta;     /* base+0x44h */
    sdc_csta_reg        sdc_csta;       /* base+0x48h */
    sdc_irqmask0_reg    sdc_irqmask0;   /* base+0x4Ch */
    sdc_irqmask1_reg    sdc_irqmask1;   /* base+0x50h */
    sdio_cfg_reg        sdio_cfg;       /* base+0x54h */
    sdio_sta_reg        sdio_sta;       /* base+0x58h */
    u32                 reserved2[5];
    boot_cfg_reg        boot_cfg;       /* base+0x70h */
    boot_sta_reg        boot_sta;       /* base+0x74h */
    u32                 reserved3[2];
    clkacb_cfg_reg      clkacb_cfg;     /* base+0x80h */
    clkacb_sta_reg      clkacb_sta;     /* base+0x84h */
    clkacb_crcsta1_reg  clkacb_crcsta1; /* base+0x88h */
    clkacb_crcsta2_reg  clkacb_crcsta2; /* base+0x8Ch */
};

typedef struct
{
    u32 flag:1;
    u32 rsv:31;
} dma_flag_reg;

typedef struct
{
    u32 inten:1;
    u32 rsv:31;
} dma_inten_reg;

typedef struct
{
    u32 en:1;
    u32 rsv:31;
} dma_en_reg;

typedef struct
{
    u32 warm_rst:1;
    u32 hard_rst:1;
    u32 rsv:30;
} dma_rst_reg;

typedef struct
{
    u32 stop:1;
    u32 pause:1;
    u32 rsv:30;
} dma_stop_reg;

typedef struct
{
    u32 flush:1;
    u32 rsv:31;
} dma_flush_reg;

typedef struct
{
    u32 dir:1;
    u32 fixen:1;
    u32 slowen:1;
    u32 rsv1:2;
    u32 slowcnt:10;
    u32 rsv2:1;
    u32 burstlen:3;    
    u32 rsv3:13;
} dma_con_reg;

typedef struct
{
    u32 addr;
} dma_addr_reg;

typedef struct
{
    u32 len:20;
    u32 rsv:12;
} dma_len_reg;

typedef struct
{
    u32 len:8;
    u32 rsv:24;
} dma_inbuf_reg;

typedef struct
{
    u32 rqclr:1;
    u32 wqclr:1;
    u32 rreq:1;
    u32 wreq:1;
    u32 rdact:1;
    u32 wdact:1;
    u32 rsv1:2;
    u32 waddrd:8;
    u32 waddrdlh:8;
    u32 radddrd:8;
} dma_dbg_reg;

struct msdc_dma_regs {
    dma_flag_reg    flag;       /* base+0x00h */ 
    dma_inten_reg   inten;      /* base+0x04h */ 
    dma_en_reg      en;         /* base+0x08h */ 
    dma_rst_reg     rst;        /* base+0x0ch */ 
    dma_stop_reg    stop;       /* base+0x10h */
    dma_flush_reg   flush;      /* base+0x14h */
    dma_con_reg     con;        /* base+0x18h */
    dma_addr_reg    addr;       /* base+0x1ch */
    u32             rsv1[1];    /* base+0x20h */
    dma_len_reg     len;        /* base+0x24h */
    u32             rsv2[4];
    dma_inbuf_reg   inbuf;      /* base+0x38h */
    u32             rsv3[5];
    dma_dbg_reg     dbg;        /* base+0x50h */
};

struct msdc_dma {
    u32 base;                   /* dma channel base */
    u32 dir;                    /* dma channel direction */
    u32 id;                     /* dma channel id */
};


struct msdc_host
{
    struct msdc_card *card;
    u32 base;         /* host base address */
    u8  id;           /* host id number */
    u32 clk;          /* host clock speed */
    u32 sclk;         /* SD/MS clock speed */
    u8  clksrc;       /* clock source */
    void *priv;       /* private data */
};

#define MMC_TYPE_UNKNOWN    (0)          /* Unknown card */
#define MMC_TYPE_MMC        (0x00000001) /* MMC card */
#define MMC_TYPE_SD         (0x00000002) /* SD card */
#define MMC_TYPE_SDIO       (0x00000004) /* SDIO card */

enum {
    standard_capacity = 0,
    high_capacity = 1,
    extended_capacity = 2,
};
enum {
    FAT16 = 0,
    FAT32 = 1,
    exFAT = 2,
};

/* MMC device */
struct msdc_card {
    u32                     rca;		  /* relative card address of device */
    unsigned int            type;		  /* card type */
    unsigned short          state;		  /* (our) card state */
    unsigned short          file_system;   /* FAT16/FAT32 */ 
    unsigned short          card_cap;      /* High Capcity/standard*/          
};

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
 
#define sdr_read16(reg)          __raw_readw(reg)
#define sdr_read32(reg)          __raw_readl(reg)
#define sdr_write16(reg,val)     __raw_writew(val,reg)
#define sdr_write32(reg,val)     __raw_writel(val,reg)
 
#define sdr_set_bits(reg,bs)     ((*(volatile u32*)(reg)) |= (u32)(bs))
#define sdr_clr_bits(reg,bs)     ((*(volatile u32*)(reg)) &= ~((u32)(bs)))
 
#define sdr_set_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = sdr_read32(reg); \
         tv &= ~(field); \
         tv |= ((val) << (uffs((unsigned int)field) - 1)); \
         sdr_write32(reg,tv); \
     } while(0)
#define sdr_get_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = sdr_read32(reg); \
         val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
     } while(0)
     
#ifdef MMC_PROFILING
static inline void msdc_timer_init(void)
{
    /* MODE[5:4]=3(32678HZ RTC free run). CLR[1]=1, EN[0]=0 */
    __raw_writel(0x32, MT6573_XGPT3_CON);    
    __raw_writel(0, MT6573_XGPT3_PRESCALE);
    __raw_writel(32768, MT6573_XGPT3_COMPARE);
}
static inline void msdc_timer_start(void)
{
    *(volatile unsigned int*)MT6573_XGPT3_CON |= (1 << 0);
}
static inline void msdc_timer_stop(void)
{
    *(volatile unsigned int*)MT6573_XGPT3_CON &= ~(1 << 0);
}
static inline void msdc_timer_stop_clear(void)
{
    *(volatile unsigned int*)MT6573_XGPT3_CON &= ~(1 << 0); /* stop  */
    *(volatile unsigned int*)MT6573_XGPT3_CON |= (1 << 1);  /* clear */
}
static inline unsigned int msdc_timer_get_count(void)
{
    return __raw_readl(MT6573_XGPT3_COUNT);
}
#else
#define msdc_timer_init()       do{}while(0)
#define msdc_timer_start()      do{}while(0)
#define msdc_timer_stop()       do{}while(0)
#define msdc_timer_stop_clear() do{}while(0)
#define msdc_timer_get_count()  0
#endif



extern int msdc_init(void);
extern void msdc_soft_reset(void);

extern int msdc_write_sectors(unsigned char* dst, u32 blk_num, u32 nblks);
extern int msdc_read_sectors(unsigned char* dst, u32 blk_num, u32 nblks);


#endif /* __MT6573_SDC_H__ */

