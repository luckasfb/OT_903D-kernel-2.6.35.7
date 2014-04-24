
/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>

#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mt6573_gpio.h>
#include <asm/arch/mt6573_sdc.h>
#include <asm/arch/mmc_core.h>
#include "upmu_common_sw.h"
#include "pmu6573_sw.h"

#include <cust_sdc.h>

#ifdef CONFIG_MMC

//#define SDC_DEBUG
#define SDC_DMA_MODE

#define AP_DMA_BASE        (0x70002000)
#define WPLL_CON0          (0x7002E240)
#define APMCU_CG_SET0      (0x70026304)
#define APMCU_CG_CLR0      (0x70026308)
#define MIXEDSYS0_BASE     (0x7002E000)
#define APCONFIG_BASE      (0x70026000)
#define PLL_CON5_REG       (MIXEDSYS0_BASE+0x0114)
#define MSDC0_CCTL         (APCONFIG_BASE+0x0140)
#define MSDC1_CCTL         (APCONFIG_BASE+0x0144)
#define MSDC2_CCTL         (APCONFIG_BASE+0x0148)
#define MSDC3_CCTL         (APCONFIG_BASE+0x014C)

#define GPIO_PULL_DOWN     (0)
#define GPIO_PULL_UP       (1)

#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100)        /* 100ms */

#define MAX_FIFOTHD        (16)         /* the maximun fifo threshold */
#define MAX_SCLK           (52000000)
#define MIN_SCLK           (260000)
#define MAX_DMA_CNT        (512*1024)   /* Max. = 0xFFFFF but we set to 512KB */

/* Tuning Parameter */
#define DEFAULT_DEBOUNCE    8   /* 8 cycles */
#define DEFAULT_DLT         1   /* data latch timing */
#define DEFAULT_DTOC        40  /* data timeout counter. 65536x40 sclk. */
#define DEFAULT_WDOD        0   /* write data output delay. no delay. */
#define DEFAULT_BSYDLY      8   /* card busy delay. 8 extend sclk */
#define DEFAULT_RED         EDGE_FALLING
#define DEFAULT_CMDRE       EDGE_RISING
#define EMMC_RESET_PIN      GPIO60

typedef int (*msdc_status_cb)(void *priv);

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

#define msdc_core_power_on() \
    do { \
        MSG(PWR, "[VMC] ++ (LINE:%d)\n", __LINE__); \
        upmu_ldo_enable(LDO_VMC, KAL_TRUE);	\
    } while (0)

#define msdc_core_power_off() \
    do { \
        MSG(PWR, "[VMC] -- (LINE:%d)\n", __LINE__); \
        upmu_ldo_enable(LDO_VMC, KAL_FALSE); \
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

static int msdc_rsp[] = {
    0,  /* RESP_NONE */
    1,  /* RESP_R1 */
    2,  /* RESP_R2 */
    3,  /* RESP_R3 */
    4,  /* RESP_R4 */
    5,  /* RESP_R5 */
    6,  /* RESP_R6 */
    7,  /* RESP_R7 */
    7,  /* RESP_R1b */
};

#ifdef SDC_DEBUG
static struct msdc_regs *msdc_reg[MSDC_MAX_NUM];
#endif

#ifdef SDC_DMA_MODE
static struct msdc_dma_regs *msdc_dma_reg[MSDC_MAX_NUM];
static struct msdc_dma msdc_dma_chnl[MSDC_MAX_NUM];

static int msdc_dma_data_status(void *priv)
{
    int err = MMC_ERR_NONE;
    struct mmc_host *host = (struct mmc_host *)priv;
    u32 base = host->base;
    u32 status;

    status = sdr_read32(MSDC_STA);
    if (status & MSDC_STA_INT) {
        status = sdc_get_datsta() & SDC_DATSTA_ERR;
        if (status) {
            printf("[SD%d] DATTO(%d) DATCRCERR(%d)\n", host->id, 
                (status & SDC_DATSTA_DATTO) >> 1, 
                (status & SDC_DATSTA_DATCRCERR) >> 2);
            if (status & SDC_DATSTA_DATTO)
                err = MMC_ERR_TIMEOUT;
            else
                err = MMC_ERR_BADCRC;
        }
    }
    return err;
}

static void msdc_dma_config(struct msdc_dma *dma, int burst, u32 addr, u32 len)
{
    u32 base = dma->base;

    sdr_write32(DMA_LEN, len & 0xFFFFF);
    sdr_write32(DMA_MEM_ADDR, addr);
    sdr_write32(DMA_CTRL, (burst << 16) | dma->dir);
}

static void msdc_dma_start(struct msdc_dma *dma)
{
    u32 base = dma->base;

    sdr_clr_bits(DMA_INT_FLAG, 0x1);
    sdr_set_bits(DMA_INT_EN, 0x1);
    dsb();
    sdr_write32(DMA_EN, 0x1);
}

static void msdc_dma_stop(struct msdc_dma *dma)
{
    u32 base = dma->base;

    /* disable dma interrupt to avoid disturb system */
    sdr_clr_bits(DMA_INT_EN, 0x1);
    dsb();
    sdr_set_bits(DMA_STOP, 0x1);
    while (sdr_read32(DMA_EN) & 0x1);
    sdr_clr_bits(DMA_INT_FLAG, 0x1);
    dsb();
}

static void msdc_dma_reset(struct msdc_dma *dma)
{
    u32 base = dma->base;

    /* hard reset */
    sdr_set_bits(DMA_RST, 0x2);
    dsb();
    sdr_clr_bits(DMA_RST, 0x2);
    sdr_write32(DMA_MEM_ADDR, 0);
}

static int msdc_dma_wait_done(struct msdc_dma *dma, msdc_status_cb cb, void *data)
{
    u32 base = dma->base;
    int err = MMC_ERR_NONE;

    /* TODO: timeout checking */
    while (!err) {
        if (sdr_read32(DMA_INT_FLAG)) {
            sdr_write32(DMA_INT_FLAG, 0); /* clear */
            break;
        } else if (cb) {
            err = cb(data);
        }    
    }

    return err;
}

static void msdc_dma_init(struct msdc_dma *dma, int id)
{
    memset(dma, 0, sizeof(struct msdc_dma));

    dma->id   = id;
    dma->base = AP_DMA_BASE + 0x180 + 0x80 * id;
    msdc_dma_reg[id] = (struct msdc_dma_regs *)dma->base;

    sdr_set_bits(0x70026318, 1 << 4); /* ungate APDMA */

    msdc_dma_reset(dma);
}
#endif

void msdc_dump_card_status(u32 card_status)
{
#ifdef SDC_DEBUG
    static char *state[] = {
        "Idle",         /* 0 */
        "Ready",        /* 1 */
        "Ident",        /* 2 */
        "Stby",         /* 3 */
        "Tran",         /* 4 */
        "Data",         /* 5 */
        "Rcv",          /* 6 */
        "Prg",          /* 7 */
        "Dis",          /* 8 */
        "Btst",         /* 9 */
        "Slp",          /* 10 */
        "Reserved",     /* 11 */
        "Reserved",     /* 12 */
        "Reserved",     /* 13 */
        "Reserved",     /* 14 */
        "I/O mode",     /* 15 */
    };
    if (card_status & R1_OUT_OF_RANGE)
        printf("\t[CARD_STATUS] Out of Range\n");
    if (card_status & R1_ADDRESS_ERROR)
        printf("\t[CARD_STATUS] Address Error\n");
    if (card_status & R1_BLOCK_LEN_ERROR)
        printf("\t[CARD_STATUS] Block Len Error\n");
    if (card_status & R1_ERASE_SEQ_ERROR)
        printf("\t[CARD_STATUS] Erase Seq Error\n");
    if (card_status & R1_ERASE_PARAM)
        printf("\t[CARD_STATUS] Erase Param\n");
    if (card_status & R1_WP_VIOLATION)
        printf("\t[CARD_STATUS] WP Violation\n");
    if (card_status & R1_CARD_IS_LOCKED)
        printf("\t[CARD_STATUS] Card is Locked\n");
    if (card_status & R1_LOCK_UNLOCK_FAILED)
        printf("\t[CARD_STATUS] Lock/Unlock Failed\n");
    if (card_status & R1_COM_CRC_ERROR)
        printf("\t[CARD_STATUS] Command CRC Error\n");
    if (card_status & R1_ILLEGAL_COMMAND)
        printf("\t[CARD_STATUS] Illegal Command\n");
    if (card_status & R1_CARD_ECC_FAILED)
        printf("\t[CARD_STATUS] Card ECC Failed\n");
    if (card_status & R1_CC_ERROR)
        printf("\t[CARD_STATUS] CC Error\n");
    if (card_status & R1_ERROR)
        printf("\t[CARD_STATUS] Error\n");
    if (card_status & R1_UNDERRUN)
        printf("\t[CARD_STATUS] Underrun\n");
    if (card_status & R1_OVERRUN)
        printf("\t[CARD_STATUS] Overrun\n");
    if (card_status & R1_CID_CSD_OVERWRITE)
        printf("\t[CARD_STATUS] CID/CSD Overwrite\n");
    if (card_status & R1_WP_ERASE_SKIP)
        printf("\t[CARD_STATUS] WP Eraser Skip\n");
    if (card_status & R1_CARD_ECC_DISABLED)
        printf("\t[CARD_STATUS] Card ECC Disabled\n");
    if (card_status & R1_ERASE_RESET)
        printf("\t[CARD_STATUS] Erase Reset\n");
    if (card_status & R1_READY_FOR_DATA)
        printf("\t[CARD_STATUS] Ready for Data\n");
    if (card_status & R1_SWITCH_ERROR)
        printf("\t[CARD_STATUS] Switch error\n");
    if (card_status & R1_URGENT_BKOPS)
        printf("\t[CARD_STATUS] Urgent background operations\n");
    if (card_status & R1_APP_CMD)
        printf("\t[CARD_STATUS] App Command\n");

    printf("\t[CARD_STATUS] '%s' State\n", 
    state[R1_CURRENT_STATE(card_status)]);
#endif
}

void msdc_dump_ocr_reg(u32 resp)
{
#ifdef SDC_DEBUG
    if (resp & (1 << 7))
        printf("\t[OCR] Low Voltage Range\n");
    if (resp & (1 << 15))
        printf("\t[OCR] 2.7-2.8 volt\n");
    if (resp & (1 << 16))
        printf("\t[OCR] 2.8-2.9 volt\n");
    if (resp & (1 << 17))
        printf("\t[OCR] 2.9-3.0 volt\n");
    if (resp & (1 << 18))
        printf("\t[OCR] 3.0-3.1 volt\n");
    if (resp & (1 << 19))
        printf("\t[OCR] 3.1-3.2 volt\n");
    if (resp & (1 << 20))
        printf("\t[OCR] 3.2-3.3 volt\n");
    if (resp & (1 << 21))
        printf("\t[OCR] 3.3-3.4 volt\n");
    if (resp & (1 << 22))
        printf("\t[OCR] 3.4-3.5 volt\n");
    if (resp & (1 << 23))
        printf("\t[OCR] 3.5-3.6 volt\n");
    if (resp & (1 << 24))
        printf("\t[OCR] Switching to 1.8V Accepted (S18A)\n");
    if (resp & (1 << 30))
        printf("\t[OCR] Card Capacity Status (CCS)\n");
    if (resp & (1 << 31))
        printf("\t[OCR] Card Power Up Status (Idle)\n");
    else
        printf("\t[OCR] Card Power Up Status (Busy)\n");
#endif
}

void msdc_dump_rca_resp(u32 resp)
{
#ifdef SDC_DEBUG
    u32 card_status = (((resp >> 15) & 0x1) << 23) |
                      (((resp >> 14) & 0x1) << 22) |
                      (((resp >> 13) & 0x1) << 19) |
                        (resp & 0x1fff);

    printf("\t[RCA] 0x%x\n", resp >> 16);
    msdc_dump_card_status(card_status);
#endif
}

#ifdef SDC_DEBUG
void msdc_dump_regs(struct mmc_host *host)
{
    u32 base = host->base;

    printf("[SD%d] MSDC_CFG=0x%x\n", host->id, sdr_read32(MSDC_CFG));
    printf("[SD%d] MSDC_INT=0x%x\n", host->id, sdr_read32(MSDC_INT));
    printf("[SD%d] MSDC_STA=0x%x\n", host->id, sdr_read32(MSDC_STA));
    printf("[SD%d] MSDC_PS=0x%x\n", host->id, sdr_read32(MSDC_PS));
    printf("[SD%d] MSDC_IOCON0=0x%x\n", host->id, sdr_read32(MSDC_IOCON0));
    printf("[SD%d] MSDC_IOCON1=0x%x\n", host->id, sdr_read32(MSDC_IOCON1));
    printf("[SD%d] SDC_CFG=0x%x\n", host->id, sdr_read32(SDC_CFG));
    printf("[SD%d] SDC_CMD=0x%x\n", host->id, sdr_read32(SDC_CMD));
    printf("[SD%d] SDC_ARG=0x%x\n", host->id, sdr_read32(SDC_ARG));
    printf("[SD%d] SDC_STA=0x%x\n", host->id, sdr_read32(SDC_STA));
    printf("[SD%d] SDC_CMDSTA=0x%x\n", host->id, sdr_read32(SDC_CMDSTA));
    printf("[SD%d] SDC_DATSTA=0x%x\n", host->id, sdr_read32(SDC_DATSTA));
    printf("[SD%d] SDC_CSTA=0x%x\n", host->id, sdr_read32(SDC_CSTA));
    printf("[SD%d] BOOT_STA=0x%x\n", host->id, sdr_read32(BOOT_STA));
}
#endif

int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base   = host->base;
    u32 opcode = cmd->opcode;
    u32 rsptyp = cmd->rsptyp;
    u32 rawcmd;
    u32 start;
    u32 timeout = cmd->timeout;
    u16 status = SDC_CMDSTA_CMDRDY;
    u32 error = MMC_ERR_NONE;

    rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) | msdc_rsp[rsptyp] << 7;

    /* cmd = opcode | rsptyp << 7 | idrt << 10 | dtype << 11 |  rw << 13 | stop << 14 */
    if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
        rawcmd |= ((2 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_WRITE_BLOCK) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK || 
               opcode == SD_ACMD_SEND_SCR ||
               opcode == SD_CMD_SWITCH ||
               opcode == MMC_CMD_SEND_EXT_CSD) {
        rawcmd |= (1 << 11);
    } else if (opcode == MMC_CMD_STOP_TRANSMISSION) {
        rawcmd |= (1 << 14);
    } else if (opcode == MMC_CMD_ALL_SEND_CID || 
               opcode == SD_ACMD_SEND_OP_COND) {
        rawcmd |= (1 << 10);
    }

    MSG(CMD, "[SD%d] SND_CMD(%d): ARG(0x%x), RAW(0x%x)\n", 
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), cmd->arg, rawcmd);

    if (rsptyp == RESP_R1B && opcode != MMC_CMD_STOP_TRANSMISSION) {
        if (sdc_is_busy()) {
            start = get_timer(0);
            while (sdc_is_busy()) {
                if (get_timer(start) > timeout) {
                    cmd->error = MMC_ERR_TIMEOUT;
                    printf("[SD%d] SND_CMD(%d): SDC_IS_BUSY timeout\n", 
                        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
                    goto end;
                }
            }
        }
    } else {
        if (sdc_is_cmd_busy()) {
            start = get_timer(0);
            while (sdc_is_cmd_busy()) {
                if (get_timer(start) > timeout) {
                    cmd->error = MMC_ERR_TIMEOUT;
                    printf("[SD%d] SND_CMD(%d): SDC_IS_CMD_BUSY timeout\n", 
                        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
                    goto end;
                }
            }
        }
    }

    sdc_send_cmd(rawcmd, cmd->arg);

    if (rsptyp == RESP_R1B) {
        /* wait for not busy */
        do {
            status = sdr_read32(MSDC_INT) & MSDC_INT_SDR1BIRQ;
        } while (!status);
    }

    if (!(status = sdc_get_cmdsta())) {
        timeout = cmd->timeout;
        start = get_timer(0);
        while (!(status = sdc_get_cmdsta())) {
            if (get_timer(start) > timeout) {
                error = MMC_ERR_TIMEOUT;
                printf("[SD%d] SND_CMD(%d): SDC_GET_CMDSTA timeout\n", 
                    host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
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
            u32 *resp = &cmd->resp[0];
            *resp++ = sdr_read32(SDC_RESP3);
            *resp++ = sdr_read32(SDC_RESP2);
            *resp++ = sdr_read32(SDC_RESP1);
            *resp++ = sdr_read32(SDC_RESP0);
            MSG(RSP, "[SD%d] CMD_RSP: RSPTP(%d) = 0x%x 0x%x 0x%x 0x%x\n", 
                host->id, cmd->rsptyp, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);			
            break;
        }
        default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
            cmd->resp[0] = sdr_read32(SDC_RESP0);
#ifdef SDC_DEBUG
            switch(rsptyp) {
            case RESP_R1:
            case RESP_R1B:
                msdc_dump_card_status(cmd->resp[0]);
                break;
            case RESP_R3:
                msdc_dump_ocr_reg(cmd->resp[0]);
                break;
            case RESP_R6:
                msdc_dump_rca_resp(cmd->resp[0]);
                break;
            }
#endif

            MSG(RSP, "[SD%d] CMD_RSP: RSPTP(%d) = 0x%x\n", host->id, 
                cmd->rsptyp, cmd->resp[0]);

            break;
        }
    } else if (status & SDC_CMDSTA_RSPCRCERR) {
        error = MMC_ERR_BADCRC;
        printf("[SD%d] CMD_RSP(%d): ERR(BADCRC), RSPTP(%d)\n", 
            host->id, opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT), cmd->rsptyp);
    } else if (status & SDC_CMDSTA_CMDTO) {
        error = MMC_ERR_TIMEOUT;
        MSG(RSP, "[SD%d] CMD_RSP(%d): ERR(CMDTO), RSPTP(%d)\n", 
            host->id, opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT), cmd->rsptyp);
    }

end:
    cmd->error = error;
    return error;
}

static u32 msdc_cal_timeout(struct mmc_host *host, u32 ns, u32 clks, u32 clkunit)
{
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout / clkunit;
    return timeout;
}

void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks)
{
    u32 base = host->base;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout >> 16; /* in 65536 sclk cycle unit */
    timeout = timeout > 1 ? timeout - 1 : 0;
    timeout = timeout > 255 ? 255 : timeout;

    host->timeout_clks = clks;
    host->timeout_ns = ns;

    sdr_set_field(SDC_CFG, SDC_CFG_DTOC, timeout);

    MSG(OPS, "[SD%d] Set read data timeout: %dns %dclks -> %d x 65536 cycles\n",
        host->id, ns, clks, timeout + 1);
}

void msdc_set_fifothd(struct mmc_host *host, int threshold)
{
    u32 base = host->base;

    msdc_set_fifo_thd(threshold);
}

void msdc_set_blklen(struct mmc_host *host, u32 blklen)
{
    u32 base = host->base;

    host->blklen = blklen;
    msdc_set_bksz(blklen);
    msdc_clr_fifo();
}

void msdc_set_blknum(struct mmc_host *host, u32 blknum)
{
    return; /* no need */
}

int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size)
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
        printf("[SD%d] PIO Read Error (%d)\n", host->id, err);
    }    
    return err;
}

int msdc_pio_write(struct mmc_host *host, u32 *ptr, u32 size)
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
            status = sdc_get_datsta() & SDC_DATSTA_ERR;
            if (status) {
                printf("[SD%d] DATTO(%d) DATCRCERR(%d)\n", host->id, 
                    (status & SDC_DATSTA_DATTO) >> 1, 
                    (status & SDC_DATSTA_DATCRCERR) >> 2);
                if (status & SDC_DATSTA_DATTO)
                    err = MMC_ERR_TIMEOUT;
                else
                    err = MMC_ERR_BADCRC;
                break;
            }
        }
    }

    if (err == MMC_ERR_NONE) {        
        while(!msdc_is_fifo_empty()); /* wait fifo flush to card */
    } else {
        printf("[SD%d] PIO Write Error (%d)\n", host->id, err);
    }

    return err;
}

#ifndef SDC_DMA_MODE
int msdc_pio_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    int multi;
    struct mmc_command cmd;
    u32 *ptr = (u32 *)dst;

    MSG(OPS, "[SD%d] Read data %d bytes from 0x%x\n", host->id, nblks * blksz, src);

    multi = nblks > 1 ? 1 : 0;

    msdc_set_bksz(blksz);
    msdc_set_fifo_thd(1);
    msdc_clr_fifo();

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = msdc_pio_read(host, ptr, nblks * blksz);

    if (multi) {
        cmd.opcode  = MMC_CMD_STOP_TRANSMISSION;
        cmd.rsptyp  = RESP_R1B;
        cmd.arg     = 0;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &cmd);
    }

done:
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Read data error %d\n", host->id, err);
    }

    return err;
}

int msdc_pio_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    u32 base = host->base;
    int err = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    struct mmc_command cmd;
    u32 *ptr = (u32 *)src;

    MSG(OPS, "[SD%d] Write data %d bytes to 0x%x\n", host->id, nblks * blksz, dst);

    multi = nblks > 1 ? 1 : 0;

    msdc_set_bksz(blksz);
    msdc_set_fifo_thd(1);
    msdc_clr_fifo();

    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = msdc_pio_write(host, ptr, nblks * blksz);

    if (multi) {
        cmd.opcode  = MMC_CMD_STOP_TRANSMISSION;
        cmd.rsptyp  = RESP_R1B;
        cmd.arg     = 0;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &cmd);
    }

done:
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Write data error %d\n", host->id, err);
    }

    return err;
}
#endif

#ifdef SDC_DMA_MODE
int msdc_dma_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    int multi;
    struct mmc_command cmd;
    struct msdc_dma *dma = &msdc_dma_chnl[host->id];
    ulong bytes, trans = 0, left = 0, dma_size;
    u32 *ptr = (u32 *)dst;
    u32 status;

    MSG(OPS, "[SD%d] Read data %d bytes from 0x%x\n", host->id, nblks * blksz, src);

    multi = nblks > 1 ? 1 : 0;

    msdc_dma_on();
    msdc_set_bksz(blksz);
    msdc_set_fifo_thd(4);
    msdc_clr_fifo();

    bytes = nblks * blksz;
    trans = bytes / MAX_DMA_CNT;
    left  = bytes % MAX_DMA_CNT;
    if (trans) {
        dma_size = MAX_DMA_CNT;
        trans--;
    } else {
        dma_size = left;
        left = 0;
    }

    dma->dir = 1; /* 1: RX */

    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_DMABRUST, MSDC_DMA_BURST_4BEAT);
    msdc_dma_config(dma, 3, (unsigned int)ptr, dma_size);
    msdc_dma_start(dma);

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    for (;;) { /* FIXME. Need a timeout detect */
        err = msdc_dma_wait_done(dma, msdc_dma_data_status, (void*)host);
        msdc_dma_stop(dma);
        if (err) {
            printf("[SD%d] Wait dma done error (%d)\n", host->id, err);
            break;
        }
        status = sdc_get_datsta() & SDC_DATSTA_ERR;
        if (status) {
            printf("[SD%d] DATTO(%d) DATCRCERR(%d)\n", host->id, 
                (status & SDC_DATSTA_DATTO) >> 1, 
                (status & SDC_DATSTA_DATCRCERR) >> 2);
            if (status & SDC_DATSTA_DATTO)
                err = MMC_ERR_TIMEOUT;
            else
                err = MMC_ERR_BADCRC;
            break;
        }
        if (trans) {
            dma_size = MAX_DMA_CNT;
            trans--;
        } else if (left) {
            dma_size = left;
            left = 0;
        } else {
            break;
        }
        ptr += MAX_DMA_CNT;
        msdc_dma_config(dma, 3, (unsigned int)ptr, dma_size);
        msdc_dma_start(dma);
    }
    if (multi) {
        cmd.opcode  = MMC_CMD_STOP_TRANSMISSION;
        cmd.rsptyp  = RESP_R1B;
        cmd.arg     = 0;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &cmd);
    }

done:
    msdc_dma_off();
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Read data error %d\n", host->id, err);
    }

    return err;
}

int msdc_dma_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    int multi;
    struct mmc_command cmd;
    struct msdc_dma *dma = &msdc_dma_chnl[host->id];
    ulong bytes, trans = 0, left = 0, dma_size;
    u32 *ptr = (u32 *)src;
    u32 status;

    MSG(OPS, "[SD%d] Write data %d bytes to 0x%x\n", host->id, nblks * blksz, dst);

    multi = nblks > 1 ? 1 : 0;

    msdc_dma_on();
    msdc_set_bksz(blksz);
    msdc_set_fifo_thd(4);
    msdc_clr_fifo();

    bytes = nblks * blksz;
    trans = bytes / MAX_DMA_CNT;
    left  = bytes % MAX_DMA_CNT;
    if (trans) {
        dma_size = MAX_DMA_CNT;
        trans--;
    } else {
        dma_size = left;
        left = 0;
    }    

    dma->dir = 0; /* 0: TX */

    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_DMABRUST, MSDC_DMA_BURST_4BEAT);
    msdc_dma_config(dma, 3, (unsigned int)ptr, dma_size);
    msdc_dma_start(dma);

    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    for (;;) { /* FIXME. Need a timeout detect */
        err = msdc_dma_wait_done(dma, msdc_dma_data_status, (void*)host);
        msdc_dma_stop(dma);
        if (err) {
            printf("[SD%d] Wait dma done error (%d)\n", host->id, err);
            break;
        }
        status = sdc_get_datsta() & SDC_DATSTA_ERR;
        if (status) {
            printf("[SD%d] DATTO(%d) DATCRCERR(%d)\n", host->id, 
                (status & SDC_DATSTA_DATTO) >> 1, 
                (status & SDC_DATSTA_DATCRCERR) >> 2);
            if (status & SDC_DATSTA_DATTO)
                err = MMC_ERR_TIMEOUT;
            else
                err = MMC_ERR_BADCRC;
            break;
        }
        if (trans) {
            dma_size = MAX_DMA_CNT;
            trans--;
        } else if (left) {
            dma_size = left;
            left = 0;
        } else {
            break;
        }
        ptr += MAX_DMA_CNT;
        msdc_dma_config(dma, 3, (unsigned int)ptr, dma_size);
        msdc_dma_start(dma);
    }
    if (!err) {
        while(!msdc_is_fifo_empty()); /* wait fifo flush to card */
    }
    if (multi) {
        cmd.opcode  = MMC_CMD_STOP_TRANSMISSION;
        cmd.rsptyp  = RESP_R1B;
        cmd.arg     = 0;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &cmd);
    }

done:
    msdc_dma_off();
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Write data error %d\n", host->id, err);
    }

    return err;
}
#endif

void msdc_clear_fifo(struct mmc_host *host)
{
    u32 base = host->base;
    msdc_clr_fifo();
}

void msdc_card_detect(struct mmc_host *host, int on)
{
    u32 base = host->base;
    struct msdc_hw *msdc = (struct msdc_hw*)host->priv;

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

void msdc_config_clksrc(struct mmc_host *host, u8 clksrc)
{
    u32 base = host->base;
    u32 clkctl[] = {MSDC0_CCTL, MSDC1_CCTL, MSDC2_CCTL, MSDC3_CCTL};
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

void msdc_config_bus(struct mmc_host *host, u32 width)
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

    printf("[SD%d] Bus Width = %d\n", host->id, width);
}

void msdc_config_pin(struct mmc_host *host, int mode)
{
    struct msdc_hw *msdc = (struct msdc_hw *)host->priv;
    u32 base = host->base;
    int pull = (mode == PIN_PULL_UP) ? GPIO_PULL_UP : GPIO_PULL_DOWN;

    /* Config WP pin */
    if (msdc->flags & MSDC_WP_PIN_EN) {
        BUG_ON(msdc->flags & MSDC_RST_PIN_EN);
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

void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz)
{
    struct msdc_hw *msdc = (struct msdc_hw*)host->priv;
    u32 base = host->base;
    u32 div;
    u32 sclk;

    if (!hz) {
        sdr_clr_bits(SDC_CFG, SDC_CFG_SIEN);
        return;
    }

    if (hz >= (host->clk >> 1)) {
        div  = 0;              /* mean div = 1/2 */
        sclk = host->clk >> 1; /* sclk = clk / 2 */
    } else {
        div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
        sclk = (host->clk >> 2) / div;
    }

    host->sclk = sclk;

    sdr_clr_bits(SDC_CFG, SDC_CFG_SIEN);
    if (host->id == 0 || host->id == 1) {
        /* Jocelyn: 
         * HS && fbclk shorter than data: 1 (falling)
         * HS && fbclk longer than data: 0 (rising
         * non HS-RED: 1 (falling)
         */
        sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_LAT, 0);
        sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_RED, 1);
    }
    sdr_set_field(MSDC_CFG, MSDC_CFG_SCLKF, div);
    sdr_set_field(MSDC_CFG, MSDC_CFG_RED, msdc->data_edge); 
    sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDRE, msdc->cmd_edge);
    sdr_set_bits(SDC_CFG, SDC_CFG_SIEN);
    msdc_set_timeout(host, host->timeout_ns, host->timeout_clks);
    mdelay(1);

    printf("[SD%d] SET_CLK(%dkHz): SCLK(%dkHz) DDR(%d) DIV(%d) CEDGE(%d) DEDGE(%d)\n",
        host->id, hz/1000, sclk/1000, ddr, div, msdc->cmd_edge, msdc->data_edge);
}

void msdc_reset_pin(struct mmc_host *host, int mode)
{
    struct msdc_hw *msdc = (struct msdc_hw *)host->priv;
    u32 base = host->base;
    int pull = (mode == PIN_PULL_UP) ? GPIO_PULL_UP : GPIO_PULL_DOWN;

    /* Config reset pin */
    if (msdc->flags & MSDC_RST_PIN_EN) {
        BUG_ON(msdc->flags & MSDC_WP_PIN_EN);
        if (msdc->config_gpio_pin)
            msdc->config_gpio_pin(MSDC_RST_PIN, pull);

        #if 1
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_RST_WP, msdc->rst_wp_pull_res);

        if (mode == PIN_PULL_UP) {
            sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_RST_WP, mode);
            sdr_set_bits(BOOT_IOCON, BOOT_IOCON_RST);
        } else {
            sdr_clr_bits(BOOT_IOCON, BOOT_IOCON_RST);
            sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_RST_WP, mode);
        }
        #else
        /* eMMC HW reset pin testing on EVB */
        base = MSDC1_BASE;
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_DAT, msdc->dat_pull_res);
        sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_DAT, mode);/* Config CMD pin */
        #endif
    }
}

int msdc_switch_volt(struct mmc_host *host, int volt)
{
    return MMC_ERR_INVALID;
}

int msdc_tune_uhs1(struct mmc_host *host, struct mmc_card *card)
{
    return MMC_ERR_INVALID;
}

void msdc_clock(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s clock \n", host->id, on ? "on" : "off", "host");

    if (on) {
        /* MSDC_CLK_CTRL_SEL[9]=1, MSDC_CLK_EN[7]=1, 3GPLL_EN[0]=1 */
        sdr_set_bits(WPLL_CON0, (1 << 9) | (1 << 7) | (1 << 0));
        sdr_set_bits(APMCU_CG_CLR0, 1 << host->id); /* clr clock gating */
    } else {
        sdr_set_bits(APMCU_CG_SET0, 1 << host->id); /* set clock gating */
    }
}

void msdc_host_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "host");

    if (on) {
        msdc_core_power_on();
        mdelay(5);
        msdc_clock(host, 1);
        msdc_reset_pin(host, PIN_PULL_UP);
    } else {
        msdc_reset_pin(host, PIN_PULL_DOWN);
        msdc_clock(host, 0);
        msdc_core_power_off();
        mdelay(100);
    }
}

void msdc_card_power(struct mmc_host *host, int on)
{
    u32 base = host->base;
    struct msdc_hw *msdc = (struct msdc_hw *)host->priv;

    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "card");

    if (on) {    
        msdc_config_pin(host, PIN_PULL_UP);
        if (msdc->ext_power_on) {
            msdc->ext_power_on();
        } else {
            sdr_set_bits(MSDC_CFG, MSDC_CFG_VDDPD);
        }
    } else {
        if (msdc->ext_power_off) {
            msdc->ext_power_off();
        } else {
            sdr_clr_bits(MSDC_CFG, MSDC_CFG_VDDPD);
        }
        msdc_config_pin(host, PIN_PULL_DOWN);        
    }
    mdelay(50);
}

void msdc_power(struct mmc_host *host, u8 mode)
{
    if (mode == MMC_POWER_ON || mode == MMC_POWER_UP) {
        msdc_host_power(host, 1);
        msdc_card_power(host, 1);
    } else {
        msdc_card_power(host, 0);
        msdc_host_power(host, 0);
    }
}

int msdc_card_avail(struct mmc_host *host)
{
    u32 base = host->base;
    struct msdc_hw *msdc = (struct msdc_hw*)host->priv;
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

int msdc_card_protected(struct mmc_host *host)
{
    int prot;
    u32 base = host->base;
    struct msdc_hw *msdc = (struct msdc_hw*)host->priv;

    if (msdc->flags & MSDC_WP_PIN_EN) {
        prot = sdc_is_write_protect();
    } else {
        prot = 0;
    }

    return prot;
}

void msdc_emmc_boot_reset(struct mmc_host *host, int reset)
{
    u32 base = host->base;
    struct msdc_hw *msdc = (struct msdc_hw*)host->priv;

    switch (reset) {
    case EMMC_BOOT_PWR_RESET:
        msdc_hard_reset();
        break;
    case EMMC_BOOT_RST_N_SIG:
        if (msdc->flags & MSDC_RST_PIN_EN) {
            /* set reset pin to low */
            //msdc_reset_pin(host, PIN_PULL_DOWN);
            mt_set_gpio_out(EMMC_RESET_PIN,GPIO_OUT_ZERO);   

            /* tRSTW (RST_n pulse width) at least 1us */
            mdelay(1);

            /* set reset pin to high */
            //msdc_reset_pin(host, PIN_PULL_UP);
            mt_set_gpio_out(EMMC_RESET_PIN,GPIO_OUT_ONE);
            sdr_set_bits(MSDC_CFG, MSDC_CFG_SCLKON);
            /* tRSCA (RST_n to command time) at least 200us, 
               tRSTH (RST_n high period) at least 1us */
            mdelay(1);
            sdr_clr_bits(MSDC_CFG, MSDC_CFG_SCLKON);
        }
        break;
    case EMMC_BOOT_PRE_IDLE_CMD:
        /* bring emmc to pre-idle mode by software reset command. (MMCv4.41)*/
        sdc_send_cmd(0x0, 0xF0F0F0F0);
        break;
    }
}

void msdc_emmc_boot_stop(struct mmc_host *host, int mode)
{
    u32 base = host->base;

    if (mode == EMMC_BOOT_RST_CMD_MODE)
        sdc_send_cmd(0, 0);

    sdr_set_field(BOOT_CFG, BOOT_CFG_WAITDLY, 1);
    sdr_set_bits(BOOT_CFG, BOOT_CFG_STOP);

    while (sdr_read32(BOOT_STA) & BOOT_STA_WAITEXIT);

    sdr_clr_bits(BOOT_CFG, BOOT_CFG_SUPPORT);
}

int msdc_emmc_boot_start(struct mmc_host *host, u32 hz, int ddr, int mode, int ackdis)
{
    int err = MMC_ERR_NONE;
    u32 sts;
    u32 base = host->base;
    u32 tmo = 1000; /* 1sec */
    u32 start = get_timer(0);
    u32 acktmo, dattmo;

    acktmo = msdc_cal_timeout(host, 50 * 1000 * 1000, 0, 256) & 0xFF;     /* 50ms */
    dattmo = msdc_cal_timeout(host, 1000 * 1000 * 1000, 0, 256) & 0xFFFFF; /* 1sec */

    printf("[SD%d] EMMC BOOT ACK timeout: %d ms (clkcnt: %d)\n", host->id, 
        (acktmo * 256) / (host->sclk / 1000), acktmo);
    printf("[SD%d] EMMC BOOT DAT timeout: %d ms (clkcnt: %d)\n", host->id,
        (dattmo * 256) / (host->sclk / 1000), dattmo);

    /* configure msdc and clock frequency */    
    msdc_clr_fifo();
    msdc_set_fifothd(host, 4);
    msdc_set_blklen(host, 512);
    msdc_clear_fifo(host);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_config_clock(host, ddr, hz);

    /* requires 74 clocks/1ms before CMD0 */
    sdr_set_bits(MSDC_CFG, MSDC_CFG_SCLKON);
    mdelay(2);
    sdr_clr_bits(MSDC_CFG, MSDC_CFG_SCLKON);

    /* configure boot timeout value */
    sdr_set_field(BOOT_CFG, BOOT_CFG_ACKTOC, acktmo);
    sdr_set_field(BOOT_CFG, BOOT_CFG_DATTOC, dattmo);

    /* bring emmc to boot mode */
    sdr_set_bits(BOOT_CFG, BOOT_CFG_SUPPORT);

    if (mode == EMMC_BOOT_RST_CMD_MODE) {
        sdr_set_bits(BOOT_CFG, BOOT_CFG_MODE);
        sdc_send_cmd(0x1000, 0xFFFFFFFA);
    } else {
        sdr_clr_bits(BOOT_CFG, BOOT_CFG_MODE);
        sdc_send_cmd(0x1000, 0x0);
    }
    sdr_set_bits(BOOT_CFG, BOOT_CFG_START);

    do {
        sts = sdr_read32(BOOT_STA);
        if (sts == 0)
            continue;
        if (sts & BOOT_STA_ACKRCV) {
            printf("[SD%d] EMMC_STS(%x): boot ack received\n", host->id, sts);
            break;
        } else if (sts & BOOT_STA_ACKERR) {
            printf("[SD%d] EMMC_STS(%x): boot up ack error\n", host->id, sts);
            err = MMC_ERR_BADCRC;
            break;
        } else if (sts & BOOT_STA_ACKTMO) {
            printf("[SD%d] EMMC_STS(%x): boot up ack timeout\n", host->id, sts);
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (sts & BOOT_STA_UPSTATE) {
            printf("[SD%d] EMMC_STS(%x): in boot up state\n", host->id, sts);
        } else {
            printf("[SD%d] EMMC_STS(%x): boot up unexpected\n", host->id, sts);
            err = MMC_ERR_INVALID;
            break;
        }
        if (get_timer(start) > tmo) {
            err = MMC_ERR_TIMEOUT;
            break;
        }
    } while (1);

    if (err)
        msdc_emmc_boot_stop(host, mode);
    return err;    
}

int msdc_emmc_boot_read(struct mmc_host *host, u32 size, u32 *to)
{
    int err = MMC_ERR_NONE;
    int count;
    u32 status;
    u32 dwords;
    u32 base = host->base;

    dwords = (size + 3) >> 2;

    while (dwords) {
        status = sdr_read32(BOOT_STA);
        if (status & BOOT_STA_CRCERR) {
            err = MMC_ERR_BADCRC;
            goto out;
        } else if (status & BOOT_STA_DATTMO) {
            err = MMC_ERR_TIMEOUT;
            goto out;
        }
        status = sdr_read32(MSDC_STA);
        count  = (status & MSDC_STA_FIFOCNT) >> 4;
        if (count) {
            count = dwords > count ? count : dwords;
            dwords -= count;
            while (count--) {
                *to++ = msdc_fifo_read();
            }
        } 
    }

out:
    if (err) {
        printf("[SD%d] EMMC_BOOT: read boot code fail(%d)\n", host->id, err);
    } else {
        printf("[SD%d] EMMC_BOOT: read boot code done\n", host->id);
    }
    return err;
}

void msdc_hard_reset(void)
{
    msdc_core_power_off();
    mdelay(100);

    msdc_core_power_on();
    mdelay(1);
}

void msdc_soft_reset(struct mmc_host *host)
{
    u32 base = host->base;

    msdc_reset();
    msdc_clr_fifo();
}

int msdc_init(struct mmc_host *host, int id)
{
    u32 baddr[] = {MSDC0_BASE, MSDC1_BASE, MSDC2_BASE, MSDC3_BASE};
    u32 base = baddr[id];
    struct msdc_hw *msdc = cust_msdc_hw[id];

    BUG_ON(!msdc);

    mt_set_gpio_mode(EMMC_RESET_PIN,GPIO_MODE_00);
    mt_set_gpio_dir(EMMC_RESET_PIN,GPIO_DIR_OUT);
    mt_set_gpio_out(EMMC_RESET_PIN,GPIO_OUT_ONE);
		
#ifdef SDC_DEBUG
    msdc_reg[id] = (struct msdc_regs*)base;
#endif
    host->id     = id;
    host->base   = base;
    host->f_max  = MAX_SCLK;
    host->f_min  = MIN_SCLK;
    host->blkbits= MMC_BLOCK_BITS;
    host->blklen = 512;
    host->priv   = (void*)msdc;
    host->ocr_avail = MMC_VDD_32_33;
    host->boot_type = msdc->boot_type;
    host->caps   = MMC_CAP_4_BIT_DATA | MMC_CAP_MULTIWRITE;
    if (msdc->flags & MSDC_HIGHSPEED)
        host->caps |= MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED;
    if (msdc->data_pins == 8)
        host->caps |= MMC_CAP_8_BIT_DATA;

    host->max_hw_segs   = 128;
    host->max_phys_segs = 128;
    host->max_seg_size  = 128 * 512;
    host->max_blk_size  = 2048;
    host->max_blk_count = 65535;

#ifdef SDC_DMA_MODE
    msdc_dma_init(&msdc_dma_chnl[id], id);
    host->blk_read  = msdc_dma_bread;
    host->blk_write = msdc_dma_bwrite;
#else
    host->blk_read  = msdc_pio_bread;
    host->blk_write = msdc_pio_bwrite;
#endif

    /* Reset power */
    msdc_power(host, MMC_POWER_OFF);
    msdc_host_power(host, 1);

    /* Reset */
    msdc_reset();

    /* Workaround. Reset fifo threshold */
    msdc_set_fifo_thd(1);

    /* Disable card detection */
    sdr_clr_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);

    /* Disable all interrupts */
    sdr_clr_bits(MSDC_CFG, MSDC_CFG_RCDEN|MSDC_CFG_DIRQE|MSDC_CFG_PINEN|
        MSDC_CFG_DMAEN|MSDC_CFG_INTEN);

    /* Set to SD mode */
    sdr_set_bits(MSDC_CFG, MSDC_CFG_MSDC);

    /* Clear interrupts and status bits */
    sdr_read32(MSDC_INT);
    sdr_read32(SDC_CMDSTA);
    sdr_read32(SDC_DATSTA);

    /* Mask command interrups since we use pio mode for command req/rsp. */
    sdr_set_bits(SDC_IRQMASK0,
        SDC_IRQMASK0_CMDRDY|SDC_IRQMASK0_CMDTO|SDC_IRQMASK0_RSPCRCERR);

    /* Mask data done interrupt to reduce interrupt latency */
    sdr_set_bits(SDC_IRQMASK0, SDC_IRQMASK0_BLKDONE);

    /* Mask csta interrupt since it'll be handled by linux mmc subsystem (CHECKME) */
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
    msdc_config_clock(host, 0, host->sclk);
    msdc_card_power(host, 1);    
    msdc_card_detect(host, 1);

    return 0;
}

#endif	/* CONFIG_MMC */

