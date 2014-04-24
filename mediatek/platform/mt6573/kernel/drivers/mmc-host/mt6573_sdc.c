
/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/

//#include <config.h>
//#include <common.h>
//#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_sdc.h>
#include <mach/board.h>
//#include <mach/upmu_common_sw.h>
#include <mach/pmu6573_sw.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_devs.h>

#include <linux/timer.h>

//#include <cust_sdc.h>

#ifdef CONFIG_MMC
//#define USE_FIFO_READ 1
#define mt6573_E1 1


//#define SDC_DEBUG

#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100)        /* 100ms */
#define BLK_LEN            512

#define MAX_FIFOTHD        (16)         /* the maximun fifo threshold */
#define MAX_SCLK           (52000000)
#define NORMAL_SCLK        (25000000)
#define MIN_SCLK           (260000)
#define MAX_DMA_CNT        (512*1024)   /* Max. = 0xFFFFF but we set to 512KB */

/* Tuning Parameter */
#define DEFAULT_DEBOUNCE    8	/* 8 cycles */
#define DEFAULT_DLT         1   /* data latch timing */
#define DEFAULT_DTOC        40	/* data timeout counter. 65536x40 sclk. */
#define DEFAULT_WDOD        0	/* write data output delay. no delay. */
#define DEFAULT_BSYDLY      8	/* card busy delay. 8 extend sclk */
#define DEFAULT_RED         EDGE_FALLING
#define DEFAULT_CMDRE       EDGE_RISING

/* Debug message event */
#define DBG_EVT_NONE        0x00000000	/* No event */
#define DBG_EVT_DMA         0x00000001	/* DMA related event */
#define DBG_EVT_CMD         0x00000002	/* MSDC CMD related event */
#define DBG_EVT_RSP         0x00000004	/* MSDC CMD RSP related event */
#define DBG_EVT_INT         0x00000008	/* MSDC INT event */
#define DBG_EVT_CFG         0x00000010	/* MSDC CFG event */
#define DBG_EVT_FUC         0x00000020	/* Function event */
#define DBG_EVT_OPS         0x00000040	/* Read/Write operation event */
#define DBG_EVT_PWR         0x00000080  /* Power event */
#define DBG_EVT_INF         0x01000000  /* information event */
#define DBG_EVT_WRN         0x02000000  /* Warning event */
#define DBG_EVT_ERR         0x04000000  /* Error event */
#define DBG_EVT_ALL         0xffffffff

#define DBG_EVT_MASK       (DBG_EVT_ALL)

typedef int (*msdc_status_cb)(void *priv);

#ifdef SDC_DEBUG
#define MSG(evt, fmt, args...) \
do {	\
    if ((DBG_EVT_##evt) & DBG_EVT_MASK) { \
        printk(fmt, ##args); \
    } \
} while(0)

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

#define MSG_FUNC_ENTRY(f)	MSG(FUC, "<FUN_ENT>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...) do{}while(0)
#define MSG_FUNC_ENTRY(f)	   do{}while(0)
#endif

#define msdc_clr_fifo()         sdr_set_bits(MSDC_STA, MSDC_STA_FIFOCLR)	
#define msdc_is_fifo_empty()   (sdr_read16(MSDC_STA) & MSDC_STA_BE)
#define msdc_is_fifo_full()    (sdr_read16(MSDC_STA) & MSDC_STA_BF)
#define msdc_is_data_req()     (sdr_read32(MSDC_STA) & MSDC_STA_DRQ)	
#define msdc_is_busy()         (sdr_read16(MSDC_STA) & MSDC_STA_BUSY)

#define msdc_get_fifo_cnt(c)    sdr_get_field(MSDC_STA, MSDC_STA_FIFOCNT, (c))
#define msdc_set_fifo_thd(v)    sdr_set_field(MSDC_CFG, MSDC_CFG_FIFOTHD, (v))
#define msdc_get_fifo_thd(v)    sdr_get_field(MSDC_CFG, MSDC_CFG_FIFOTHD, (v))
#define msdc_fifo_write(v)      sdr_write32(MSDC_DAT, (v))
#define msdc_fifo_read()        sdr_read32(MSDC_DAT)	

#define msdc_dma_on()           sdr_set_bits(MSDC_CFG, MSDC_CFG_DMAEN)
#define msdc_dma_off()          sdr_clr_bits(MSDC_CFG, MSDC_CFG_DMAEN)

#define msdc_fifo_irq_on()      sdr_set_bits(MSDC_CFG, MSDC_CFG_DIRQE)
#define msdc_fifo_irq_off()     sdr_clr_bits(MSDC_CFG, MSDC_CFG_DIRQE)


#define msdc_reset() \
    do { \
        sdr_set_bits(MSDC_CFG, MSDC_CFG_RST); \
        while(sdr_read32(MSDC_CFG) & MSDC_CFG_RST); \
    } while(0)
   
    
#define msdc_lock_irqsave(val) \
    do { \
        sdr_get_field(MSDC_CFG, MSDC_CFG_INTEN, val); \
        sdr_clr_bits(MSDC_CFG, MSDC_CFG_INTEN); \
    } while(0)
	
#define msdc_unlock_irqrestore(val) \
    do { \
        sdr_set_field(MSDC_CFG, MSDC_CFG_INTEN, val); \
    } while(0)

#define msdc_set_bksz(sz)       sdr_set_field(SDC_CFG, SDC_CFG_BLKLEN, sz)

#define msdc_core_power_on(host) \
    do { \
        (void)hwPowerOn(MT65XX_POWER_LDO_VMC, VOL_3300, "SD"); \
    } while (0)

#define msdc_core_power_off(host) \
    do { \
        (void)hwPowerDown(MT65XX_POWER_LDO_VMC, "SD"); \
    } while (0)

#define sdc_get_cmdsta()        sdr_read32(SDC_CMDSTA)
#define sdc_get_datsta()        sdr_read32(SDC_DATSTA)
#define sdc_is_busy()          (sdr_read16(SDC_STA) & SDC_STA_BESDCBUSY)
#define sdc_is_cmd_busy()      (sdr_read16(SDC_STA) & SDC_STA_BECMDBUSY)
#define sdc_is_dat_busy()      (sdr_read16(SDC_STA) & SDC_STA_BEDATBUSY)
#define sdc_is_write_protect() (sdr_read16(SDC_STA) & SDC_STA_WP)
#define sdc_sclk_on()          sdr_set_bits(SDC_CFG, SDC_CFG_SIEN)
#define sdc_sclk_off()         sdr_clr_bits(SDC_CFG, SDC_CFG_SIEN)
#define sdc_send_cmd(cmd,arg) \
    do { \
        sdr_write32(SDC_ARG, (arg)); \
        sdr_write32(SDC_CMD, (cmd)); \
    } while(0)

static struct msdc_regs *msdc_reg[4];
static struct msdc_host g_host;
static struct msdc_host *host = &g_host;
static struct msdc_card card;

void msdc_dump_card_status(u32 card_status)
{
    static char *state[] = {
        "Idle",			/* 0 */
        "Ready",		/* 1 */
        "Ident",		/* 2 */
        "Stby",			/* 3 */
        "Tran",			/* 4 */
        "Data",			/* 5 */
        "Rcv",			/* 6 */
        "Prg",			/* 7 */
        "Dis",			/* 8 */
        "Btst",		    /* 9 */
        "Slp",		    /* 10 */
        "Reserved",		/* 11 */
        "Reserved",		/* 12 */
        "Reserved",		/* 13 */
        "Reserved",		/* 14 */
        "I/O mode",		/* 15 */
    };
    if (card_status & R1_OUT_OF_RANGE)
        printk("\t[CARD_STATUS] Out of Range\n");
    if (card_status & R1_ADDRESS_ERROR)
        printk("\t[CARD_STATUS] Address Error\n");
    if (card_status & R1_BLOCK_LEN_ERROR)
        printk("\t[CARD_STATUS] Block Len Error\n");
    if (card_status & R1_ERASE_SEQ_ERROR)
        printk("\t[CARD_STATUS] Erase Seq Error\n");
    if (card_status & R1_ERASE_PARAM)
        printk("\t[CARD_STATUS] Erase Param\n");
    if (card_status & R1_WP_VIOLATION)
        printk("\t[CARD_STATUS] WP Violation\n");
    if (card_status & R1_CARD_IS_LOCKED)
        printk("\t[CARD_STATUS] Card is Locked\n");
    if (card_status & R1_LOCK_UNLOCK_FAILED)
        printk("\t[CARD_STATUS] Lock/Unlock Failed\n");
    if (card_status & R1_COM_CRC_ERROR)
        printk("\t[CARD_STATUS] Command CRC Error\n");
    if (card_status & R1_ILLEGAL_COMMAND)
        printk("\t[CARD_STATUS] Illegal Command\n");
    if (card_status & R1_CARD_ECC_FAILED)
        printk("\t[CARD_STATUS] Card ECC Failed\n");
    if (card_status & R1_CC_ERROR)
        printk("\t[CARD_STATUS] CC Error\n");
    if (card_status & R1_ERROR)
        printk("\t[CARD_STATUS] Error\n");
    if (card_status & R1_UNDERRUN)
        printk("\t[CARD_STATUS] Underrun\n");
    if (card_status & R1_OVERRUN)
        printk("\t[CARD_STATUS] Overrun\n");
    if (card_status & R1_CID_CSD_OVERWRITE)
        printk("\t[CARD_STATUS] CID/CSD Overwrite\n");
    if (card_status & R1_WP_ERASE_SKIP)
        printk("\t[CARD_STATUS] WP Eraser Skip\n");
    if (card_status & R1_CARD_ECC_DISABLED)
        printk("\t[CARD_STATUS] Card ECC Disabled\n");
    if (card_status & R1_ERASE_RESET)
        printk("\t[CARD_STATUS] Erase Reset\n");
    if (card_status & R1_READY_FOR_DATA)
        printk("\t[CARD_STATUS] Ready for Data\n");
    if (card_status & R1_SWITCH_ERROR)
        printk("\t[CARD_STATUS] Switch error\n");
    if (card_status & R1_URGENT_BKOPS)
        printk("\t[CARD_STATUS] Urgent background operations\n");
    if (card_status & R1_APP_CMD)
        printk("\t[CARD_STATUS] App Command\n");

    printk("\t[CARD_STATUS] '%s' State\n", 
    state[R1_CURRENT_STATE(card_status)]);
}

void msdc_dump_ocr_reg(u32 resp)
{
    if (resp & (1 << 7))
        printk("\t[OCR] Low Voltage Range\n");
    if (resp & (1 << 15))
        printk("\t[OCR] 2.7-2.8 volt\n");
    if (resp & (1 << 16))
        printk("\t[OCR] 2.8-2.9 volt\n");
    if (resp & (1 << 17))
        printk("\t[OCR] 2.9-3.0 volt\n");
    if (resp & (1 << 18))
        printk("\t[OCR] 3.0-3.1 volt\n");
    if (resp & (1 << 19))
        printk("\t[OCR] 3.1-3.2 volt\n");
    if (resp & (1 << 20))
        printk("\t[OCR] 3.2-3.3 volt\n");
    if (resp & (1 << 21))
        printk("\t[OCR] 3.3-3.4 volt\n");
    if (resp & (1 << 22))
        printk("\t[OCR] 3.4-3.5 volt\n");
    if (resp & (1 << 23))
        printk("\t[OCR] 3.5-3.6 volt\n");
    if (resp & (1 << 24))
        printk("\t[OCR] Switching to 1.8V Accepted (S18A)\n");
    if (resp & (1 << 30))
        printk("\t[OCR] Card Capacity Status (CCS)\n");
    if (resp & (1 << 31))
        printk("\t[OCR] Card Power Up Status (Idle)\n");
    else
        printk("\t[OCR] Card Power Up Status (Busy)\n");
}

void msdc_dump_rca_resp(u32 resp)
{
    u32 card_status = (((resp >> 15) & 0x1) << 23) |
                      (((resp >> 14) & 0x1) << 22) |
                      (((resp >> 13) & 0x1) << 19) |
                        (resp & 0x1fff);

    printk("\t[RCA] 0x%x\n", resp >> 16);
    msdc_dump_card_status(card_status);
}

#ifdef SDC_DEBUG
void msdc_dump_regs(struct msdc_host *host)
{
    u32 base = host->base;

    printk("[SD%d] MSDC_CFG=0x%x\n", host->id, sdr_read32(MSDC_CFG));
    printk("[SD%d] MSDC_INT=0x%x\n", host->id, sdr_read32(MSDC_INT));
    printk("[SD%d] MSDC_STA=0x%x\n", host->id, sdr_read32(MSDC_STA));
    printk("[SD%d] MSDC_PS=0x%x\n", host->id, sdr_read32(MSDC_PS));
    printk("[SD%d] MSDC_IOCON0=0x%x\n", host->id, sdr_read32(MSDC_IOCON0));
    printk("[SD%d] MSDC_IOCON1=0x%x\n", host->id, sdr_read32(MSDC_IOCON1));
    printk("[SD%d] SDC_CFG=0x%x\n", host->id, sdr_read32(SDC_CFG));
    printk("[SD%d] SDC_CMD=0x%x\n", host->id, sdr_read32(SDC_CMD));
    printk("[SD%d] SDC_ARG=0x%x\n", host->id, sdr_read32(SDC_ARG));
    printk("[SD%d] SDC_STA=0x%x\n", host->id, sdr_read32(SDC_STA));
    printk("[SD%d] SDC_CMDSTA=0x%x\n", host->id, sdr_read32(SDC_CMDSTA));
    printk("[SD%d] SDC_DATSTA=0x%x\n", host->id, sdr_read32(SDC_DATSTA));
    printk("[SD%d] SDC_CSTA=0x%x\n", host->id, sdr_read32(SDC_CSTA));
    printk("[SD%d] BOOT_STA=0x%x\n", host->id, sdr_read32(BOOT_STA));
}
#endif

int msdc_cmd(struct msdc_host *host, u32 cmd, u32 raw, u32 arg, int rsptyp, u32*resp)
{
    u32 base   = host->base;
    u32 status;
    unsigned long tmo = 0 ;
    u32 error = MMC_ERR_NONE;

    MSG(CMD, "[SD%d] SND_CMD(%d): ARG(0x%x), RAW(0x%x)\n", host->id, cmd, arg, raw);

    if ((cmd != (MMC_CMD_STOP_TRANSMISSION ))&&(cmd != 13)) {
        if (sdc_is_busy()) {
            tmo = 0 ;
            while (sdc_is_busy()) {
                mdelay (1) ; 
                tmo ++ ;
                if (tmo >= CMD_TIMEOUT) {
                    error = MMC_ERR_TIMEOUT;
                    printk("[SD%d] SND_CMD(%d): SDC_IS_BUSY timeout\n", host->id, cmd);
                    goto end;
                }
            }
        }
    } else {
        if (sdc_is_cmd_busy()) {
            tmo = 0;
            while (sdc_is_cmd_busy()) {
                mdelay (1) ; 
                tmo ++ ;                   

                if (tmo >= CMD_TIMEOUT) { 
                    error = MMC_ERR_TIMEOUT;
                    printk("[SD%d] SND_CMD(%d): SDC_IS_CMD_BUSY timeout\n", host->id, cmd);
                    goto end;
                }
            }
        }
    }

    sdc_send_cmd(raw, arg);

    if (!(status = sdc_get_cmdsta())) {
        tmo = 0;
        while (!(status = sdc_get_cmdsta())) {
            mdelay (1) ; 
            tmo ++ ; 
            if (tmo >= CMD_TIMEOUT) {
                error = MMC_ERR_TIMEOUT;
                printk("[SD%d] SND_CMD(%d): SDC_GET_CMDSTA timeout\n", host->id, cmd);
                goto end;
            }
        }
    }
 
    if (status & SDC_CMDSTA_CMDRDY) {
        error = MMC_ERR_NONE;
        switch (rsptyp) {
        case RESP_NONE:
            MSG(RSP, "[SD%d] CMD_RSP: RSPTP(%d)\n", host->id, rsptyp);
            break;
        case RESP_R2:
        {
            MSG(RSP,"[SD%d] CMD_RSP: RSPTP(%d) = 0x%x 0x%x 0x%x 0x%x\n", 
                host->id, rsptyp, sdr_read32(SDC_RESP3),sdr_read32(SDC_RESP2), sdr_read32(SDC_RESP1),sdr_read32(SDC_RESP0));			
            break;
        }
        default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
            *resp = sdr_read32(SDC_RESP0);
#ifdef SDC_DEBUG
            switch(rsptyp) {
            case RESP_R1:
            case RESP_R1B:
                msdc_dump_card_status(*resp);
                break;
            case RESP_R3:
                msdc_dump_ocr_reg(*resp);
                break;
            case RESP_R6:
                msdc_dump_rca_resp(*resp);
                break;
            }
#endif
            if (rsptyp == RESP_R1B) {
                /* wait for not busy */
                do {
                    status = sdr_read32(MSDC_INT) & MSDC_INT_SDR1BIRQ;
                } while (!status);
            }
            MSG(RSP, "[SD%d] CMD_RSP: RSPTP(%d) = 0x%x\n", host->id, 
                rsptyp, *resp);

            break;
        }
    } else if (status & SDC_CMDSTA_RSPCRCERR) {
        error = MMC_ERR_BADCRC;
        printk("[SD%d] CMD_RSP(%d): ERR(BADCRC), RSPTP(%d)\n", 
            host->id, cmd, rsptyp);
    } else if (status & SDC_CMDSTA_CMDTO) {
        error = MMC_ERR_TIMEOUT;
        MSG(RSP, "[SD%d] CMD_RSP(%d): ERR(CMDTO), RSPTP(%d)\n", 
            host->id, cmd, rsptyp);
    }

end:
    return error;
}

void msdc_set_timeout(struct msdc_host *host, u32 ns, u32 clks)
{
    u32 base = host->base;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout >> 16; /* in 65536 sclk cycle unit */
    timeout = timeout > 1 ? timeout - 1 : 0;
    timeout = timeout > 255 ? 255 : timeout;
    
    sdr_set_field(SDC_CFG, SDC_CFG_DTOC, timeout);

    MSG(OPS, "[SD%d] Set read data timeout: %dns %dclks -> %d x 65536 cycles\n",
        host->id, ns, clks, timeout + 1);
}

void msdc_set_fifothd(struct msdc_host *host, int threshold)
{
    u32 base = host->base;

    msdc_set_fifo_thd(threshold);
}

void msdc_clear_fifo(struct msdc_host *host)
{
    u32 base = host->base;
    msdc_clr_fifo();
}

void msdc_card_detect(struct msdc_host *host, int on)
{
    u32 base = host->base;
    struct mt6573_sd_host_hw *msdc = (struct mt6573_sd_host_hw*)host->priv;

    if ((msdc->flags & MSDC_CD_PIN_EN) == 0) {
        /* Pull down card detection pin since it is not avaiable */
        if (msdc->config_gpio_pin)
            msdc->config_gpio_pin(MSDC_CD_PIN, GPIO_PULL_DOWN);
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_INS, MSDC_PULL_RES_NONE);
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_INS, PIN_PULL_DOWN);
        sdr_clr_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);
        sdr_clr_bits(MSDC_CFG, MSDC_CFG_PINEN);    
        return;
    }

    MSG(CFG, "[SD%d] CD IRQ Eanable(%d)\n", host->id, on);

    if (on) {
        if (msdc->enable_cd_eirq) {
            msdc->enable_cd_eirq();
        } else {
            /* card detection circuit relies on the core power so that the core power 
             * shouldn't be turned off. Here adds a reference count to keep 
             * the core power alive.
             */
            msdc_core_power_on();

            if (msdc->config_gpio_pin)
                msdc->config_gpio_pin(MSDC_CD_PIN, GPIO_PULL_UP);
            sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_INS, MSDC_PULL_RES_47K);
            sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_INS, PIN_PULL_UP);
            sdr_set_field(MSDC_PS, MSDC_PS_DEBOUNCE, DEFAULT_DEBOUNCE);
            sdr_set_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);
            sdr_set_bits(MSDC_CFG, MSDC_CFG_PINEN);
        }
    } else {
        if (msdc->disable_cd_eirq) {
            msdc->disable_cd_eirq();
        } else {
            if (msdc->config_gpio_pin)
                msdc->config_gpio_pin(MSDC_CD_PIN, GPIO_PULL_DOWN);
            sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_INS, MSDC_PULL_RES_NONE);
            sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_INS, PIN_PULL_DOWN);
            sdr_clr_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);
            sdr_clr_bits(MSDC_CFG, MSDC_CFG_PINEN);

            /* Here decreases a reference count to core power since card 
             * detection circuit is shutdown.
             */
            msdc_core_power_off();
        }
    }
}

void msdc_config_clksrc(struct msdc_host *host, u8 clksrc)
{
    u32 base = host->base;
    u32 clkctl[] = {APCONFIG_BASE + 0x140, APCONFIG_BASE + 0x144, 
        APCONFIG_BASE + 0x148, APCONFIG_BASE + 0x14c};
    u32 hclk[] = { 98300000, 81900000, 70200000, 61400000 };

    host->clksrc = clksrc;
    host->clk    = hclk[clksrc];
    host->sclk   = MIN_SCLK;

    /* select 3G PLL 492MHz */
    sdr_set_field(PLL_CON5_REG, 0xf, 0x1);
    /* clock phase ctrl */
    sdr_set_field(clkctl[host->id], 0x7 << 8, clksrc);    
    sdr_set_field(MSDC_CFG, MSDC_CFG_CLKSRC, host->clksrc);
}

void msdc_config_bus(struct msdc_host *host, u32 width)
{
    u32 base = host->base;

    switch (width) {
    default:
        width = HOST_BUS_WIDTH_1;
    case HOST_BUS_WIDTH_1:
        sdr_clr_bits(SDC_CFG, SDC_CFG_MDLEN | SDC_CFG_MDLEN8);
        break;
    case HOST_BUS_WIDTH_4:
        sdr_set_bits(SDC_CFG, SDC_CFG_MDLEN);
        sdr_clr_bits(SDC_CFG, SDC_CFG_MDLEN8);	
        break;
    case HOST_BUS_WIDTH_8:
        sdr_set_bits(SDC_CFG, SDC_CFG_MDLEN | SDC_CFG_MDLEN8);
        break;
    }

    printk("[SD%d] Bus Width = %d\n", host->id, width);
}

void msdc_config_pin(struct msdc_host *host, int mode)
{
    struct mt6573_sd_host_hw *msdc = (struct mt6573_sd_host_hw *)host->priv;
    u32 base = host->base;
    int pull = (mode == PIN_PULL_UP) ? GPIO_PULL_UP : GPIO_PULL_DOWN;

    /* Config WP pin */
    if (msdc->flags & MSDC_WP_PIN_EN || msdc->flags & MSDC_RST_PIN_EN)
    {
        if (msdc->config_gpio_pin)
            msdc->config_gpio_pin(MSDC_WP_PIN, pull);
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_RST_WP, msdc->rst_wp_pull_res);
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_RST_WP, mode);
    }

    if (msdc->config_gpio_pin) {
        msdc->config_gpio_pin(MSDC_CMD_PIN, pull);
        msdc->config_gpio_pin(MSDC_DAT_PIN, pull);    
    }

    sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_CLK, msdc->clk_pull_res);
    sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_CMD, msdc->cmd_pull_res);
    sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_DAT, msdc->dat_pull_res);
    sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_CLK, mode);/* Config CLK pin */
    sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_CMD, mode);/* Config CMD pin */
    sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_DAT, mode);/* Config DATA pins */

    MSG(CFG, "[SD%d] Pins mode(%d), down(%d), up(%d)\n", 
        host->id, mode, PIN_PULL_DOWN, PIN_PULL_UP);
}

void msdc_config_clock(struct msdc_host *host, u32 hz)
{
    struct mt6573_sd_host_hw *msdc = (struct mt6573_sd_host_hw*)host->priv;
    u32 base = host->base;
    u32 div;
    u32 sclk;

    if (!hz) {
        msdc_reset();
        sdr_clr_bits(SDC_CFG, SDC_CFG_SIEN);
        return;
    }

    if (hz >= (host->clk >> 1)) {
    	div  = 0;			   /* mean div = 1/2 */
    	sclk = host->clk >> 1; /* sclk = clk / 2 */
    } else {
    	div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
    	sclk = (host->clk >> 2) / div;
    }

    host->sclk = sclk;
	
    sdr_clr_bits(SDC_CFG, SDC_CFG_SIEN);
    sdr_set_field(MSDC_CFG, MSDC_CFG_SCLKF, div);
    sdr_set_field(MSDC_CFG, MSDC_CFG_RED, msdc->data_edge); 
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDRE, msdc->cmd_edge);
    sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_LAT, 0);
    sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_RED, 1);
    sdr_set_bits(SDC_CFG, SDC_CFG_SIEN);
    //msdc_set_timeout(host, host->timeout_ns, host->timeout_clks);
    mdelay(1);

    printk("[SD%d] SET_CLK(%dkHz): SCLK(%dkHz) DIV(%d) CEDGE(%d) DEDGE(%d)\n",
        host->id, hz/1000, sclk/1000, div, msdc->cmd_edge, msdc->data_edge);
}
  void msdc_clock(struct msdc_host *host, int on)
{
//    unsigned int base = host->base;

    MSG(CFG, "[SD%d] Turn %s %s clock \n", host->id, on ? "on" : "off", "host");

    if (on) {
        /* MSDC_CLK_CTRL_SEL[9]=1, MSDC_CLK_EN[7]=1, 3GPLL_EN[0]=1 */
        sdr_set_bits(0xf702E240, (1 << 9) | (1 << 7) | (1 << 0));
        sdr_set_bits(0xf7026308, 1 << host->id); /* clr clock gating */
    } else {
        sdr_set_bits(0xf7026304, 1 << host->id); /* set clock gating */
    }
}

void msdc_host_power(struct msdc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "host");

    if (on) {
        msdc_core_power_on();
        mdelay(1);
        msdc_clock(host, 1);
    } else {
        msdc_clock(host, 0);
        msdc_core_power_off();
        mdelay(100);
    }
}

void msdc_card_power(struct msdc_host *host, int on)
{
    u32 base = host->base;
    struct mt6573_sd_host_hw *msdc = (struct mt6573_sd_host_hw *)host->priv;
    
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "card");

    if (on) {    
        msdc_config_pin(host, PIN_PULL_UP);
        if (msdc->ext_power_on) {
            msdc->ext_power_on();
        } else {
            sdr_set_bits(MSDC_CFG, MSDC_CFG_VDDPD);
        }
    } else {
        msdc_config_pin(host, PIN_PULL_DOWN);
        if (msdc->ext_power_off) {
            msdc->ext_power_off();
        } else {
            sdr_clr_bits(MSDC_CFG, MSDC_CFG_VDDPD);
        }
    }
    mdelay(50);
}

void msdc_power(struct msdc_host *host, u8 mode)
{
    
    sdr_clr_bits(0xf702F7C0, 0x1);
    mdelay(5);
    sdr_set_bits(0xf702F7C0, 0x1);
    sdr_set_bits(0xf702E240, (1 << 9) | (1 << 7) | (1 << 0));   
    sdr_set_bits(0xf7026308, 1 << host->id); /* clr clock gating */
    sdr_set_field(0xf702E114, 0xf, 0x1);
    mdelay(5);
    //sdr_write16(0xf702F7C0, 0x8c61);
    //sdr_set_bits(0xf702E240, 0x280);
    //sdr_write32(0xf702E240, 0x6ED);
//    sdr_write32(0xf702E240, 0x8000|sdr_read32(0xf702E240));
    //sdr_set_bits(0xf7026308, 0x1);
    
    printk("0xf702F7C0,0xf702E240,0xf7026300 is 0x%x,0x%x,0x%x",sdr_read16(0xf702F7C0),sdr_read16(0xf702E240),sdr_read16(0xf7026300)) ;
    /*
    if (mode == MMC_POWER_ON || mode == MMC_POWER_UP) {
        msdc_host_power(host, 1);
        msdc_card_power(host, 1);
    } else {
        msdc_card_power(host, 0);
        msdc_host_power(host, 0);
    }
    */
}

int msdc_card_avail(struct msdc_host *host)
{
    u32 base = host->base;
    struct mt6573_sd_host_hw *msdc = (struct mt6573_sd_host_hw*)host->priv;
    int sts, avail = 0;

    if ((msdc->flags & MSDC_REMOVABLE) == 0)
        return 1;

    if (msdc->flags & MSDC_CD_PIN_EN) {
        if (msdc->get_cd_status) {
            avail = msdc->get_cd_status();
        } else {
            sts = sdr_read32(MSDC_PS);
            avail = sts & MSDC_PS_PIN0 ? 0 : 1;
        }
    }

    return avail;
}

int msdc_card_write_prot(struct msdc_host *host)
{
    int prot;
    u32 base = host->base;
    struct mt6573_sd_host_hw *msdc = (struct mt6573_sd_host_hw*)host->priv;

    if (msdc->flags & MSDC_WP_PIN_EN) {
        prot = sdc_is_write_protect();
    } else {
        prot = 0;
    }

    return prot;
}

void msdc_hard_reset(void)
{
    msdc_core_power_off();
    mdelay(100);

    msdc_core_power_on();
    mdelay(1);
}

void msdc_soft_reset(void)
{
    u32 base = host->base;

    msdc_reset();
    msdc_clr_fifo();
}

int SD_initialize(struct msdc_host *host)
{
	int bRet = 0;
//	int bIsSupport20=0;
	u32 Response;
	u32 cmd_41_arg;
	u32 temp;
	int i;
	u32 rca;
	u32 base = host->base;	
	
	MSG(CFG, "[SD%d] sd_initialize \n", host->id);
	
	sdr_clr_bits(SDC_CFG, SDC_CFG_MDLEN | SDC_CFG_MDLEN8);
	temp = sdr_read32(SDC_CFG);
	MSG(CFG, "[SD%d] SDC_CFG is 0x%08x \n", host->id, temp); 
	
	MSG(CFG, "[SD%d] SEND CMD:0 \n", host->id);	
	if(msdc_cmd(host, 0, 0, 0, RESP_NONE, &Response))
		goto EXIT;
	
	MSG(CFG, "[SD%d] SEND CMD:8 \n", host->id);	
	if(msdc_cmd(host, 8, 0x88, 0x15a, RESP_R7, &Response))
		goto EXIT;
	if(Response == 0x15a) 
		cmd_41_arg = 0x40ff8000;
	else 
		cmd_41_arg = 0xff8000;
	i=0;
	while(i<20)
	{
		i++;
		MSG(CFG, "[SD%d] SEND ACMD41 \n", host->id);
		if(msdc_cmd(host, 55, 0xb7, 0, RESP_R3, &Response))
			goto EXIT;
					
		if(msdc_cmd(host, 41, 0x1a9,cmd_41_arg, RESP_R3, &Response))
			goto EXIT;
		
		if((Response & 0x80000000)==0x80000000)
		{ 
    		if ((Response & 0x40000000)==0x40000000){ 
    			host->card->card_cap = high_capacity;
    			MSG(CFG, "[SD%d] card is high_capacity \n", host->id);
    		}	
    		else {
    			host->card->card_cap = standard_capacity;
    			MSG(CFG, "[SD%d] card is standard_capacity \n", host->id);
    		}	
    		break;
    	}	
		mdelay(50);
	}
	
	MSG(CFG, "[SD%d] SEND CMD:2 \n", host->id);	
	if(msdc_cmd(host, 2, 0x502, 0, RESP_R2, &Response))
		goto EXIT;
	    
	MSG(CFG, "[SD%d] SEND CMD:3 \n", host->id);	
	if(msdc_cmd(host, 3, 0x303, 0, RESP_R6, &Response))
		goto EXIT;		
	rca = (Response & 0xffff0000);
	host->card->rca = rca;
	
	MSG(CFG, "[SD%d] SEND CMD:9 \n", host->id);	
	if(msdc_cmd(host, 9, 0x109, rca, RESP_R2, &Response))
		goto EXIT;	
		
	MSG(CFG, "[SD%d] SEND CMD:13 \n", host->id);	
	if(msdc_cmd(host, 13, 0x8d, rca, RESP_R1, &Response))
		goto EXIT;			
	
	MSG(CFG, "[SD%d] SEND CMD:7 \n", host->id);	
	if(msdc_cmd(host, 7, 0x387, rca, RESP_R1, &Response))
		goto EXIT;		    

	MSG(CFG, "[SD%d] SEND ACMD42 for clr card detect\n", host->id);	
	if(msdc_cmd(host, 55, 0xb7, rca, RESP_R3, &Response))
		goto EXIT;	
	if(msdc_cmd(host, 42, 0xaa, 0, RESP_R1, &Response))
		goto EXIT;	
	
	MSG(CFG, "[SD%d] SEND ACMD6 for setting 4bit mode\n", host->id);	
	if(msdc_cmd(host, 55, 0xb7, rca, RESP_R3, &Response))
		goto EXIT;	
	if(msdc_cmd(host, 6, 0x86, 0x2, RESP_R1, &Response))
		goto EXIT;
	msdc_config_bus(host, HOST_BUS_WIDTH_4);
	
	MSG(CFG, "[SD%d] set SDC_CFG_BLKLEN= 0x200.\n", host->id);	
	sdr_set_field(SDC_CFG, SDC_CFG_BLKLEN, 0x200); 
	        		
	MSG(CFG, "[SD%d] sd initialize finished\n", host->id);	
	bRet = 1;
EXIT:
	 return bRet;
}

int msdc_init(void)
{
    int id = 0;
    u32 baddr[] = {MSDC1_BASE, MSDC2_BASE, MSDC3_BASE, MSDC4_BASE};
    u32 base = baddr[id];
    struct mt6573_sd_host_hw *msdc = &mt6573_sd0_hw;  //if id!=0, mt6573_sdx_hw 
    u32 ret = 0;

    BUG_ON(!msdc); 
    msdc_reg[id] = (struct msdc_regs*)base;  
    memset(host, 0, sizeof(struct msdc_host));    
    host->id     = id;
    host->base   = base;
    printk("[SD%d] base is 0x%08x\n", host->id, host->base);
    host->priv   = (void*)msdc;
    
    memset(&card, 0, sizeof(struct msdc_card));    
    card.type = MMC_TYPE_SD;  /*now only support SD*/
    card.file_system = FAT32;    
    host->card = &card;

    msdc_power(host, MMC_POWER_ON);

    /* Reset */
    msdc_reset();

    /* Disable card power */
    //sdr_clr_bits(MSDC_CFG, MSDC_CFG_VDDPD);

    /* Disable card detection */
    sdr_clr_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);

    /* Disable all interrupts */
    sdr_clr_bits(MSDC_CFG, MSDC_CFG_RCDEN|MSDC_CFG_DIRQE|MSDC_CFG_PINEN|
        MSDC_CFG_DMAEN|MSDC_CFG_INTEN);

    sdr_set_bits(MSDC_CFG, MSDC_CFG_MSDC);   /* SD mode */

    /* Clear interrupts and status bits */
    sdr_read32(MSDC_INT);
    sdr_read32(SDC_CMDSTA);
    sdr_read32(SDC_DATSTA);

    /* Mask command interrups since we use pio mode for command req/rsp. */
 /*    sdr_set_bits(SDC_IRQMASK0,
        SDC_IRQMASK0_CMDRDY|SDC_IRQMASK0_CMDTO|SDC_IRQMASK0_RSPCRCERR);

    Mask data done interrupt to reduce interrupt latency 
    sdr_set_bits(SDC_IRQMASK0, SDC_IRQMASK0_BLKDONE);*/

     /*Mask csta interrupt since it'll be handled by linux mmc subsystem (CHECKME) */
    sdr_set_bits(SDC_IRQMASK0, 0xFFFFFFFF);
    sdr_set_bits(SDC_IRQMASK1, 0xFFFFFFFF);

    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_ODCCFG0, msdc->cmd_odc);
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_ODCCFG1, msdc->data_odc);
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_SRCFG0, msdc->cmd_slew_rate);
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_SRCFG1, msdc->data_slew_rate);
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_DSW, MSDC_DSW_NODELAY);

    /* CHECKME. It's MUST. Otherwise, data crc occurs */
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_SAMPON, 0x1);

    sdr_set_bits(MSDC_STA, MSDC_STA_FIFOCLR);

    sdr_set_field(SDC_CFG, SDC_CFG_DTOC, DEFAULT_DTOC);
    sdr_set_field(SDC_CFG, SDC_CFG_WDOD, DEFAULT_WDOD);
    sdr_set_field(SDC_CFG, SDC_CFG_BSYDLY, DEFAULT_BSYDLY);

    msdc_config_clksrc(host, msdc->clk_src);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_set_timeout(host, 100000000, 0);
    msdc_config_clock(host, host->sclk);
    msdc_card_detect(host, 1);
    msdc_set_fifo_thd(4);
    
	if(!msdc_card_avail(host)){
	    printk("[SD%d] no card in the slot!!!!!!!!!!!\n", host->id);
	    return 0;
	}
	if(msdc_card_write_prot(host)){
	    printk("[SD%d] card write protect!!!!!!!!!!!!\n", host->id);
	    return 0;
	}
	  	
    ret = SD_initialize(host);
    if(ret){
    	msdc_config_clock(host, NORMAL_SCLK);
    }

    return ret;
}

int msdc_pio_read(u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    int count;
    u32 base = host->base;
    u32 status;
    u32 dwords = size >> 2;

    while (dwords) {
        status = sdr_read32(MSDC_STA);
        count  = (status & MSDC_STA_FIFOCNT) >> 4;
        if (count) {
            count = dwords > count ? count : dwords;
            dwords -= count;
            while (count--) {
                *ptr++ = msdc_fifo_read();
            }
        } 
        if (status & MSDC_STA_INT) {
            if (!(sdr_read32(MSDC_INT) & MSDC_INT_SDDATIRQ))
                continue;
            status = sdc_get_datsta() & SDC_DATSTA_ERR;
            if (status) {
                if (status & SDC_DATSTA_DATTO)
                    err = MMC_ERR_TIMEOUT;
                else
                    err = MMC_ERR_BADCRC;
                break;
            }
        }
    }

    if (err != MMC_ERR_NONE) {
        printk("[SD%d] PIO Read Error (%d)\n", host->id, err);
    }    
    return err;
}

int msdc_pio_write(u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    int count;
    u32 base = host->base;
    u32 status;
    u32 dwords = size >> 2;

	while (dwords) {            
        status = sdr_read32(MSDC_STA);
        if (((status & MSDC_STA_FIFOCNT) >> 4) == 0) {
            count = dwords > MAX_FIFOTHD ? MAX_FIFOTHD : dwords;
            dwords -= count;
            while (count--) {
                msdc_fifo_write(*ptr); ptr++;
            }
        }
        if (status & MSDC_STA_INT) { 
            if (!(sdr_read32(MSDC_INT) & MSDC_INT_SDDATIRQ))
                continue;
            #if 1 /* E1 workaround */
            status = sdr_read32(BOOT_STA);
            if (status & (BOOT_STA_CRCERR | BOOT_STA_DATTMO)) {
                printk("[SD%d] DATTO(%d) DATCRCERR(%d)\n", host->id, 
                    (status & BOOT_STA_DATTMO) >> 2, 
                    (status & BOOT_STA_CRCERR));
                if (status & BOOT_STA_DATTMO)
                    err = MMC_ERR_TIMEOUT;
                else
                    err = MMC_ERR_BADCRC;
                break;
            }            
            #else
            status = sdc_get_datsta() & SDC_DATSTA_ERR;
            if (status) {
                printk("[SD%d] DATTO(%d) DATCRCERR(%d)\n", host->id, 
                    (status & SDC_DATSTA_DATTO) >> 1, 
                    (status & SDC_DATSTA_DATCRCERR) >> 2);
                if (status & SDC_DATSTA_DATTO)
                    err = MMC_ERR_TIMEOUT;
                else
                    err = MMC_ERR_BADCRC;
                break;
            }
            #endif
        }
	}

    if (err == MMC_ERR_NONE) {        
        while(!msdc_is_fifo_empty()); /* wait fifo flush to card */
    } else {
        printk("[SD%d] PIO Write Error (%d)\n", host->id, err);
    }

    return err;
}

int PollingRead(unsigned char* dst,u32 dataLen)
{   
    int bRet = 1;
    unsigned long tmo;
    u32 TIMEOUT;
    unsigned char* pBuff = dst;
    u32 dwLen = dataLen ;
    u32 base = host->base;
    
    TIMEOUT = 2000*dwLen*READ_BLOCK_TIMEOUT/BLK_LEN;
    
    MSG(CFG, "[SD%d]  PollingRead(), dwLen: 0x%04x\n", host->id, dwLen);
    tmo = 0 ;
    while(dwLen > 0)
    {       
        while((!msdc_is_fifo_empty()) && (dwLen >= 4))
        {
            *((u32 *)pBuff) = msdc_fifo_read();
            /*            
            MSG(CFG, "[SD%d] PollingRead() - Data: 0x%x, 0x%x, 0x%x, 0x%x!\n", 
                host->id, *pBuff, *(pBuff+1), *(pBuff+2), *(pBuff+3));*/
            dwLen -= sizeof(u32);
            pBuff += sizeof(u32);
            
        }
        
        tmo ++ ;
        udelay (1) ;   
                
        if (tmo>=TIMEOUT)    
        {
            // The timeout is caused by context switch for there stilll remain
            // data in fifo.
            if(!msdc_is_fifo_empty())
            {
                tmo = 0;
                printk("[SD%d] PollingRead() - Context TimeOut!!\n", host->id);
            }
            else // This is real hardware timeout
            {
                printk("[SD%d] PollingRead() - TimeOut!,msdc_sta=0x%x\n", host->id, sdr_read16(MSDC_STA));
                printk("[SD%d] PollingRead() - Remain %d bytes\n", host->id, dwLen);      
                bRet = 0;
                break;
            }
        }

        
    }
    MSG(CFG, "[SD%d]  PollingRead() Done,\n", host->id);
   
    return bRet;
    
}

int PollingWrite(unsigned char* dst, u32 dataLen)
{   
	int bRet = 1;
	unsigned long tmo;
	u32 TIMEOUT;
	unsigned char* pBuff = dst;
	u32 dwLen = dataLen ;
	u32 dwData;
	u32 base = host->base;	
    
	TIMEOUT = 2000*dwLen*WRITE_BLOCK_TIMEOUT/BLK_LEN;	
    
	MSG(CFG, "[SD%d]  PollingWrite(), dwLen: 0x%04x\n", host->id, dwLen);
	tmo = 0; 
	while(dwLen > 0)
	{       
		while(!msdc_is_fifo_full() && (dwLen >= 4)) {
			dwData = *((u32*)pBuff);
			msdc_fifo_write(dwData);
			dwLen -= sizeof(u32);
			pBuff += sizeof(u32);
		}

        tmo ++ ;
        udelay (1) ;
		if (tmo >= TIMEOUT)
		{
			// The timeout is caused by context switch for there stilll remain
			// data in fifo.
			if(!msdc_is_fifo_full())
			{
				tmo = 0 ;
				printk("[SD%d] PollingWrite() - Context TimeOut!!\n", host->id);
			}
			else // This is real hardware timeout.
			{
				printk("[SD%d] PollingWrite() - TimeOut!\n", host->id);
                printk("[SD%d] PollingWrite() - Remain %d bytes\n", host->id, dwLen);  
				bRet = 0;
				break;
			}
		}
	}
	MSG(CFG, "[SD%d]  PollingWrite() Done,\n", host->id);
   
	return bRet;
}

int PollingWaitDatRdy(void)
{
    u16 sdc_datsta;
    int status = 0;
    u32 dwWaitCount = 0;
    u32 base = host->base;
    while(!(sdc_datsta = sdr_read16(SDC_DATSTA)))
    {
        dwWaitCount++;
        if(dwWaitCount > WAIT_TIME)
        {
            printk("[SD%d] PollingWaitDatRdy(): Timeout, sdc_datsta=0x%x\n", host->id, sdc_datsta);
            return 0;            
        } 
    #ifdef mt6573_E1 /* E1 workaround */
        if (sdr_read32(BOOT_STA) & BOOT_STA_CRCERR ) {
            printk("[SD%d] PollingWaitDatRdy DATCRCERR\n", host->id);
            return 0;
        } 
    #endif       
    }

    if(sdc_datsta & SDC_DATSTA_BLKDONE)
    {        
        status = 1;
        MSG(CFG, "[SD%d] PollingWaitDatRdy(): BLOCK Done \n", host->id);                                    
    }    
    else if(sdc_datsta & SDC_DATSTA_DATTO)
    {
        printk("[SD%d] PollingWaitDatRdy(): DATTO!\n", host->id);
        status = 0;
        
    }
    else     
    {   
        printk("[SD%d] PollingWaitDatRdy(): CRC Error! %d!\n", host->id, sdc_datsta);
        status = 0;            
    }
    return status;
}
 
int msdc_read_sectors(unsigned char* dst, u32 blk_num, u32 nblks)
{
	int i;
    int status = 0;
    u32 Response;
    u32 totalLen = BLK_LEN * nblks;
    u32 dwdataLen= totalLen;
    u32 readLen;
    u32 base = host->base;
//    u32 *ptr = (u32 *)dst;
    
    if(host->card->card_cap == standard_capacity) 
    {
        blk_num = BLK_LEN * blk_num;
    }
    MSG(CFG, "[SD%d] read from %dth block, n_blk=%d \n", host->id, blk_num, nblks);
    ////read file,if failed try again 3 time.	
    for(i=0; i<3; i++)
    {		
		msdc_clr_fifo();
		msdc_reset();
		
		if(nblks == 1)
		{	
			MSG(CFG, "[SD%d] SEND CMD:17 %dth times\n", host->id, i);		
		    if(msdc_cmd(host, 17, SDC_CMD_CMD17, blk_num, RESP_R1, &Response))
			{
				printk("[SD%d] SEND CMD:17 failed %dth times\n", host->id, i);
				continue;
			}
		}
		else
		{	
			MSG(CFG, "[SD%d] SEND CMD:18\n", host->id);		
		    if(msdc_cmd(host, 18, SDC_CMD_CMD18, blk_num, RESP_R1, &Response))
			{
				printk("[SD%d] SEND CMD:18 failed %dth times\n", host->id, i);
				continue;
			}
		}
#ifdef USE_FIFO_READ 		
		msdc_fifo_irq_off();
#endif		
		while(dwdataLen > 0)
		{
			readLen = (dwdataLen > BLK_LEN) ? BLK_LEN : dwdataLen;
#ifndef	USE_FIFO_READ		    
			status = PollingRead(dst + (BLK_LEN * nblks - dwdataLen), readLen);
			if(!status)
			{
				printk( "[SD%d] read failed %dth times\n", host->id, i);
				break;
			}
			status = PollingWaitDatRdy();
#else			
			status = msdc_pio_read(ptr, readLen);
			status = !status;
#endif			
			if(!status)
			{
			    printk( "[SD%d] read PollingWaitDatRdy from %dth block,%d blocks, failed %dth times\n", host->id, blk_num,nblks, i);
			    break;
			}
			else
			{
			   dwdataLen -= BLK_LEN;
			}
		}

	    // Need to send STOP command(CMD12) for multi block.
        if((nblks > 1)||(!status))
	    {
	        for(i=0; i<2; i++)
		    {
		        if(msdc_cmd(host, 12, SDC_CMD_CMD12, 0, RESP_R1B, &Response))
				{
					printk( "[SD%d] send CMD12 failed, %dth times\n", host->id, i);
					continue;
				}
	            break;
		    }
		}
	        
	    if (dwdataLen <= 0) {
	        break;	// data read success
	    }
    }

    if (dwdataLen > 0 || !status)
    {
        printk("[SD%d] msdc_read_sectors failed!!!!!!!!!!!!!!!!!!!!from %dth block, blk_num = %d\n", host->id, blk_num, nblks);
    }
    else
    {
        MSG(CFG, "[SD%d] msdc read block done, from %dth block, blk_num = %d\n", host->id, blk_num, nblks);
            
    }    
	
    return status;
} 

int SendCMD13CheckStatues(void)
{
    u32 dwWaitCount = 0;
    u32 Response = 0;
    u32 SD_State = 0;
//    u32 base = host->base;

    while (1)
    {
        dwWaitCount++;
        if (dwWaitCount > 1000)
        {
            printk("[SD%d]: PollingWaitProgram timeout!\r\n", host->id);
            return 0;
        }
        if(msdc_cmd(host, 13, SDC_CMD_CMD13, host->card->rca, RESP_R1, &Response))
		{
			MSG(CFG, "[SD%d] send CMD13 failed\n", host->id);
			continue;
		}        
        SD_State = (Response >> 9) & 0x0F;
        MSG(CFG,"[SD%d]: sd state is 0x%x \n", host->id, SD_State);
        if (SD_State == 4)
        {
            return 1;
        }
    }
}

int msdc_write_sectors(unsigned char* dst, u32 blk_num, u32 nblks)
{
	int i;
    int status = 0;
    u32 Response;
    u32 totalLen = BLK_LEN * nblks;
	u32 dwdataLen;
	u32 writeLen; 
	u32 base = host->base;
//	u32 *ptr = (u32 *)dst;

	if(nblks == 0)
	{
        printk("[SD%d] error nblks=0\n", host->id);   
		return status;
    }
    
    if(host->card->card_cap == standard_capacity) 
    {
        blk_num = BLK_LEN * blk_num;		
        MSG(CFG,"[SD%d] stan_capa write from %dth block, n_blk=%d %\n", host->id, blk_num, nblks);         
    }
    else
    {
        MSG(CFG,"[SD%d] high_capa write from %dth block, n_blk=%d\n", host->id, blk_num, nblks);   
    }
	
	////write sd,if failed try again 3 time.
	for(i=0; i<3; i++)
	{
		msdc_clr_fifo(); 
		msdc_reset();
		
		dwdataLen= totalLen;
        
		if(nblks == 1)
		{	
			MSG(CFG, "[SD%d] SEND CMD:24 %dth times\n", host->id, i);					
		    if(msdc_cmd(host, 24, SDC_CMD_CMD24, blk_num, RESP_R1, &Response))
			{
				printk("[SD%d] SEND CMD:24 failed %dth times\n", host->id, i);
				continue;
			}
		}
		else
		{	
			MSG(CFG, "[SD%d] SEND CMD:25 %dth times\n", host->id, i);		
		    if(msdc_cmd(host, 25, SDC_CMD_CMD25, blk_num, RESP_R1, &Response))
			{
				printk("[SD%d] SEND CMD:25 failed %dth times\n", host->id, i);
				continue;
			}
		}		
		
		while(dwdataLen > 0)
		{
			writeLen = (dwdataLen > BLK_LEN) ? BLK_LEN : dwdataLen;			
#ifndef	USE_FIFO_READ
			status = PollingWrite(dst+(totalLen-dwdataLen), writeLen);
			if (!status) 
			{
			    MSG(CFG, "[SD%d] write failed %dth times\n", host->id, i);
				break;
			}
			status = PollingWaitDatRdy(); 
#else			
			status = msdc_pio_write(ptr, writeLen);
			status = !status;
#endif			
			if(!status)
			{
				printk("[SD%d] write PollingWaitDatRdy Failed! from %dth block,%d blocks, failed %dth times\n", host->id, blk_num,nblks, i);
				break;
			}
			else 
			{
				dwdataLen -= BLK_LEN;
			}
		}

		// Need to send STOP command(CMD12) for multi block.
		if((nblks > 1)||(!status))
	    {
	        for(i=0; i<2; i++)
		    {
		        if(msdc_cmd(host, 12, SDC_CMD_CMD12, 0, RESP_R1B, &Response))
				{
					MSG(CFG, "[SD%d] send CMD12 failed %dth times\n", host->id, i);
					continue;
				}
	            break;
		    }
		}
		else {
		    MSG(CFG, "[SD%d] send CMD13 to wait pram ready \n", host->id);
		    if (!SendCMD13CheckStatues())
            {
                printk( "[SD%d] wait pram ready failed\n", host->id);
            }   
		}

		if (dwdataLen <= 0) {
			break;	// data write success
		}
	}

	if (dwdataLen > 0 || !status)
	{
		printk("[SD%d] msdc_write_sectors failed!!!!!!!!!!!!!!!!!!!!!from %dth block, blk_num = %d\n", host->id, blk_num, nblks);	
	}
	else{
	    MSG(CFG,"[SD%d] msdc_write_sectors done,from %dth block, blk_num = %d\n", host->id, blk_num, nblks); 
	}	
    
	return status;
}

#endif	/* CONFIG_MMC */

EXPORT_SYMBOL(msdc_read_sectors) ;
EXPORT_SYMBOL(msdc_write_sectors) ;
EXPORT_SYMBOL(msdc_soft_reset) ;
EXPORT_SYMBOL(msdc_init) ;
