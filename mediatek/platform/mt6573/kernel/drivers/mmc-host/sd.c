

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/blkdev.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/dma-mapping.h>

#include <mach/dma.h>
#include <mach/board.h> /* FIXME */
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_boot.h>
//#include <mach/mt6573_gpt_sw.h>
#include <asm/tcm.h>

#include "mt6573_sd.h"

#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

//#define CONFIG_MMC_DEBUG
//#define CFG_PROFILING
//#define CFG_PROFILING_DUMP
#define DRV_NAME            "mt6573-sd"

#define DMA_MAX_COUNT       (512*1024)             /* Max. = 0xFFFFF but we set to 512KB */

#define HOST_MAX_NUM        (4)
//#define HOST_MAX_SCLK       (49150000)           /* 49.15MHz  and this define removes  kernel/include/linux/mmc/core.h */
#define HOST_MIN_SCLK       (260000)               /* 260kHz */
#define HOST_INI_SCLK       (HOST_MIN_SCLK)
#define HOST_MAX_BLKSZ      (2048)

/* Tuning Parameter */
#define DEFAULT_DEBOUNCE    (8)       /* 8 cycles */
#define DEFAULT_DLT         (1)       /* data latch timing */
#define DEFAULT_DTOC        (40)      /* data timeout counter. 65536x40 sclk. */
#define DEFAULT_WDOD        (0)       /* write data output delay. no delay. */
#define DEFAULT_BSYDLY      (8)       /* card busy delay. 8 extend sclk */

#define CMD_TIMEOUT         (100/(1000/HZ)) /* 100ms */
#define DAT_TIMEOUT         (500/(1000/HZ)) /* 500ms */

#define MAX_FIFOTHD         (1)       /* the maximun fifo threshold */
/* Jocelyn: for efficiency, suggest not use (16)*/
#define PRD_FIFOTHD         (1)       /* default fifo threshold of pio read */
#define PWR_FIFOTHD         (1)       /* default fifo threshold of pio write */

/* Debug message event */
#define DBG_EVT_NONE        (0)       /* No event */
#define DBG_EVT_DMA         (1 << 0)  /* DMA related event */
#define DBG_EVT_CMD         (1 << 1)  /* MSDC CMD related event */
#define DBG_EVT_RSP         (1 << 2)  /* MSDC CMD RSP related event */
#define DBG_EVT_INT         (1 << 3)  /* MSDC INT event */
#define DBG_EVT_CFG         (1 << 4)  /* MSDC CFG event */
#define DBG_EVT_FUC         (1 << 5)  /* Function event */
#define DBG_EVT_OPS         (1 << 6)  /* Read/Write operation event */
#define DBG_EVT_WRN         (1 << 7)  /* Warning event */
#define DBG_EVT_PWR         (1 << 8)  /* Power event */
#define DBG_EVT_ALL         (0xffffffff)

#define DBG_EVT_MASK        (DBG_EVT_ALL)

#define MT6573_SD_DEBUG
#ifdef CONFIG_MMC_DEBUG
#ifndef MT6573_SD_DEBUG
#define MT6573_SD_DEBUG
#endif
#endif

#ifdef MT6573_SD_DEBUG
static struct mt6573_sd_regs *mt6573_sd_reg[HOST_MAX_NUM];
static struct msdc_dma_regs *msdc_dma_reg[HOST_MAX_NUM];
static unsigned long mt6573_sd_debug_sw[4]={
	0,
	0,
	0,
	0
};

#define MSG(evt, fmt, args...) \
	do {    \
		if ((host!=NULL)&&((DBG_EVT_##evt) & mt6573_sd_debug_sw[host->id])) { \
			printk("[SD%d] %s: "fmt, host->id, __func__, ##args); \
		}   \
	} while(0)
#else
#define MSG(evt, fmt, args...)  do{}while(0)
#endif

#define msdc_clr_fifo()         sdr_set_bits(MSDC_STA, MSDC_STA_FIFOCLR)
#define msdc_is_fifo_empty()   (sdr_read16(MSDC_STA) & MSDC_STA_BE)
#define msdc_is_fifo_full()    (sdr_read16(MSDC_STA) & MSDC_STA_BF)
#define msdc_is_data_req()     (sdr_read32(MSDC_STA) & MSDC_STA_DRQ)
#define msdc_is_busy()         (sdr_read16(MSDC_STA) & MSDC_STA_BUSY)

#define msdc_get_fifo_cnt(c)    sdr_get_field(MSDC_STA, MSDC_STA_FIFOCNT, (c))
#define msdc_set_fifo_thd(v)    sdr_set_field(MSDC_CFG, MSDC_CFG_FIFOTHD, (v))
#define msdc_fifo_write(v)      sdr_write32(MSDC_DAT, (v))
#define msdc_fifo_read()        sdr_read32(MSDC_DAT)

#define msdc_dma_on()           sdr_set_bits(MSDC_CFG, MSDC_CFG_DMAEN)
#define msdc_dma_off()          sdr_clr_bits(MSDC_CFG, MSDC_CFG_DMAEN)

#define msdc_fifo_irq_on()      sdr_set_bits(MSDC_CFG, MSDC_CFG_DIRQE)
#define msdc_fifo_irq_off()     sdr_clr_bits(MSDC_CFG, MSDC_CFG_DIRQE)

#define msdc_reset() \
	do { \
		sdr_set_bits(MSDC_CFG, MSDC_CFG_RST); \
		dsb(); \
		udelay(10); /* must be. otherwise, msdc is unstable. */ \
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
		MSG(PWR, "[+]VMC ref. count: %d, LINE:%d\n", ++host->pwr_ref, __LINE__); \
		(void)hwPowerOn(MT65XX_POWER_LDO_VMC, VOL_3300, "SD"); \
	} while (0)

#define msdc_core_power_off(host) \
	do { \
		MSG(PWR, "[-]VMC ref. count: %d, LINE:%d\n", --host->pwr_ref, __LINE__); \
		(void)hwPowerDown(MT65XX_POWER_LDO_VMC, "SD"); \
	} while (0)

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

#define is_card_present(h)     (((struct mt6573_sd_host*)(h))->card_inserted)

static u32  ghclk_tab[]  = {98300000, 81900000, 70200000, 61400000};

#define FORCE_IN_DMA (0x11)
#define FORCE_IN_PIO (0x10)
#define FORCE_NOTHING (0x0)
static int    dma_force[4] = //used for sd ioctrol
{
	FORCE_NOTHING,
	FORCE_NOTHING,
	FORCE_NOTHING,
	FORCE_NOTHING
};
#define dma_is_forced(host_id) 	(dma_force[host_id] & 0x10)
#define get_forced_transfer_mode(host_id)  (dma_force[host_id] & 0x01)

static u32 clock_id[4] =	{
	MT65XX_PDN_PERI_MSDC,
	MT65XX_PDN_PERI_MSDC2,
	MT65XX_PDN_PERI_MSDC3,
	MT65XX_PDN_PERI_MSDC4
};
static u32   *msdc_multi_buffer = NULL;
static u32    msdc_base_addr[]  = {0,0,0,0};
static struct mt6573_sd_host *msdc_host[HOST_MAX_NUM]={NULL,NULL,NULL,NULL};
static void   mt6573_sd_start_dma(struct mt6573_sd_host *host);
static void   mt6573_sd_stop_dma(struct mt6573_sd_host *host);
static void   mt6573_sd_prepare_dma(struct mt6573_sd_host *host,struct scatterlist *sg);
static int    mt6573_sd_pio_read(struct mt6573_sd_host *host, struct mmc_data *data);
static int    mt6573_sd_pio_write(struct mt6573_sd_host* host, struct mmc_data *data);
static void   mt6573_sd_enable_cd_irq(struct mmc_host *mmc, int enable);
static unsigned int mt6573_sd_send_command(struct mt6573_sd_host *host,
		struct mmc_command    *cmd,
		int                    polling,
		unsigned long          timeout);
static void mt6573_sd_card_power(struct mt6573_sd_host *host, int on);
static void mt6573_sd_card_clock(struct mt6573_sd_host *host, int on);
static void mt6573_sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios);

static atomic_t    flag_sdio_complete = ATOMIC_INIT(1);
static inline void  disable_sdio_irq_process_data(void)
{
	atomic_set(&flag_sdio_complete, 0);
}

static inline void enable_sdio_irq_process_data(void)
{
	atomic_set(&flag_sdio_complete, 1);
}

static inline int can_sdio_irq_process_data(void)
{
	return  atomic_read(&flag_sdio_complete);
}

#define SD_FALSE             -1
#define SD_TRUE               0
#define BLK_SIZ_CNT_OF_512    1          /* it must be in [1,4] */
#define MSDC_SCLKF_NUM        4
#define MSDC_CLKSRC_NUM       4
#define PMU_VOSEL            (0x7<<4)
#define PMU_VMCEN            (0x1<<0)
//#define PMU_BASE              0xF702F000
//#define VMC_CON0             (PMU_BASE + 0x07C0)
//#define MSDC_CCTL             0xF700E140
#define MSDC_SEL             (0x7<<8)

#define MAX_RETRY_TIME 3
static int Is_In_ReTry = MAX_RETRY_TIME;

extern void clr_device_working_ability(UINT32 clockId, MT6573_STATE state);
extern void set_device_working_ability(UINT32 clockId, MT6573_STATE state);

#if  0
static int SD_PowerTuning(struct mt6573_sd_host *host)
{
	unsigned int reg_address=VMC_CON0;
	u32 base = host->base;
	u32 status=0;
	u32 sdc_status;
	unsigned long tmo;

	MSG(CFG,"MT6573 retry VMC tuning start!!\n");
	sdr_get_field(VMC_CON0,PMU_VOSEL,status);
	MSG(CFG,"MT6573 VMC output voltage:%d!!\n",status);
	switch(status)
	{
	case 0:
		MSG(CFG,"VMC output voltage: 1.3V\n");
		return SD_TRUE;
	case 1:
		MSG(CFG,"VMC output voltage: 1.5V\n");
		return SD_TRUE;
	case 2:
		MSG(CFG,"VMC output voltage: 1.8V\n");
		return SD_TRUE;
	case 3:
		MSG(CFG,"VMC output voltage: 2.5V\n");
		return SD_TRUE;
	case 4:
		MSG(CFG,"VMC output voltage: 2.8V\n");
		break;
	case 5:
		MSG(CFG,"VMC output voltage: 3.0V\n");
		break;
	case 6:
		MSG(CFG,"VMC output voltage: 3.3V\n");
		break;
	default:
		MSG(CFG,"VMC output voltage: invalid\n");
		return SD_FALSE;
	}
	msdc_reset();
	tmo=jiffies+5*CMD_TIMEOUT;
	while(1)
	{
		sdc_status=sdr_read16(SDC_STA);
		if((sdc_status&0x001F)==0)
		{
			break;
		}
		if(time_after(jiffies,tmo))
		{
			MSG(CFG,"VMC set timeout\n");
			return SD_FALSE;
		}
	}
	if(status==4||status ==5)
	{
		MSG(CFG,"Set VMC output voltage: 3.3V\n");
		status=6;
		sdr_set_field(VMC_CON0,PMU_VOSEL,status);

		sdr_set_bits(VMC_CON0,PMU_VMCEN);
		msdc_reset();
		udelay(100);

		mmc_remove_card(host->mmc->card);
		host->mmc->card = NULL;

		mmc_detect_change(host->mmc, msecs_to_jiffies(20));

	}
	else
	{
		MSG(CFG,"Set VMC output voltage: 2.8V\n");
		status=4;
		sdr_set_field(VMC_CON0,PMU_VOSEL,status);

		sdr_set_bits(VMC_CON0,PMU_VMCEN);

		msdc_reset();
		udelay(100);

		mmc_remove_card(host->mmc->card);
		host->mmc->card = NULL;
		mmc_detect_change(host->mmc, msecs_to_jiffies(20));
	}

	sdr_get_field(VMC_CON0,PMU_VOSEL,status);
	switch(status)
	{
	case 0:
		MSG(CFG,"After reset,VMC output voltage: 1.3V\n");
		return SD_TRUE;
	case 1:
		MSG(CFG,"After reset,VMC output voltage: 1.5V\n");
		return SD_TRUE;
	case 2:
		MSG(CFG,"After reset,VMC output voltage: 1.8V\n");
		return SD_TRUE;
	case 3:
		MSG(CFG,"After reset,VMC output voltage: 2.5V\n");
		return SD_TRUE;
	case 4:
		MSG(CFG,"After reset,VMC output voltage: 2.8V\n");
		break;
	case 5:
		MSG(CFG,"After reset,VMC output voltage: 3.0V\n");
		break;
	case 6:
		MSG(CFG,"After reset,VMC output voltage: 3.3V\n");
		break;
	default:
		MSG(CFG,"After reset,VMC output voltage: invalid\n");
		return SD_FALSE;
	}
	MSG(CFG,"MT6573 retry VMC tuning end!!\n");
	return SD_TRUE;
}
#endif

static int SD_retrySendCMD12(struct mt6573_sd_host* host)
{
	struct mmc_command cmd;

	memset(&cmd,0,sizeof(struct mmc_command));
	cmd.opcode=MMC_STOP_TRANSMISSION;
	cmd.flags=MMC_RSP_R1B|MMC_CMD_AC;

	if(mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT)!=0){
		printk("[SD%d] %s: RETRY -> send CMD12 failed!!\n", host->id, __func__);
		return -1;
	}
	else
		return 0;
}
static int SD_CanCmdBeLatched(struct mt6573_sd_host* host)
{
	struct mmc_command cmd;
	u32 base = host->base;
	int retries = 100;

	memset(&cmd,0,sizeof(struct mmc_command));
	cmd.opcode=MMC_SEND_STATUS;

	if (!host || !(host->mmc)) {
		printk("[SD*] %s: RETRY -> enter cmd check: host NULL \n", __func__);
		return SD_FALSE;
	}

	if (!is_card_present(host)) {
		return SD_FALSE;
	} else {
		if (NULL != host->mmc->card)
			cmd.arg = host->mmc->card->rca<<16;
		else if (0 != host->card_rca)
			cmd.arg = host->card_rca<<16;
		else
			return SD_FALSE;
	}

	MSG(CFG,"RETRY -> card RCA is 0x%x\n",cmd.arg);
	cmd.flags=MMC_RSP_R1|MMC_CMD_AC;

	while (retries--) {
		if (mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT)!=0)	{
			printk("[SD%d] %s: RETRY -> send CMD13 failed!\n",host->id, __func__);
			goto fail;
		}

		if ((cmd.resp[0] & CARD_READY_FOR_DATA) && (CARD_CURRENT_STATE(cmd.resp[0]) == 4)){
			MSG(CFG,"RETRY -> send CMD13 successfully, and the card is free for retry!\n");
			return SD_TRUE;
		}
	}

	MSG(CFG,"RETRY -> send CMD13 successfully, and the card is busy for retry!\n");
fail:
	msdc_reset();
	return SD_FALSE;
}

static int mt6573_sd_retry_setblksz(struct mt6573_sd_host* host, u16 blk_cnt)
{
	u32 base = host->base;
	struct mmc_command cmd;
	u16 blk_size = 0;

	/* Make sure blk_size is a multiple of 512
	*/
	blk_cnt = (blk_cnt==0)?(1):(blk_cnt);
	blk_cnt = (blk_cnt>4)?(4):(blk_cnt);
	blk_size = blk_cnt << 9;

	memset(&cmd,0,sizeof(struct mmc_command));
	cmd.opcode = MMC_SET_BLOCKLEN;
	cmd.arg    = blk_size;
	cmd.flags  = MMC_RSP_R1|MMC_CMD_AC;
	if (mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT)!=0) {
		MSG(CFG,"RETRY -> Block length set fail\n");
		return SD_FALSE;
	}

	msdc_set_bksz(blk_size);
	MSG(CFG,"RETRY -> Block size=%d, SDC_CFG = 0x%x\tCLKACB_CFG = 0x%x\n",
			blk_size, sdr_read32(SDC_CFG),sdr_read32(CLKACB_CFG));
	return SD_TRUE;
}

static int SD_CanSingleReadBeLatched(struct mt6573_sd_host* host, u32 address)
{
	u32 uLen       = BLK_SIZ_CNT_OF_512<<9;
	u32 TIMEOUT    = 2000*uLen*100/512;
	u32 tmo        = 0 ;
	u32 temp       = 0;
	u32 base       = host->base;
	u32 uWaitCount = 0x0;
	u16 sdc_datsta;
	struct mmc_command cmd;

	sdr_read32(SDC_CMDSTA);
	sdr_read32(BOOT_STA);
	sdr_read32(SDC_DATSTA);

	mt6573_sd_stop_dma(host);
	msdc_dma_off();

	if (SD_FALSE == mt6573_sd_retry_setblksz(host, BLK_SIZ_CNT_OF_512))
		return SD_FALSE;

	msdc_clr_fifo();
	msdc_reset();

	memset(&cmd,0,sizeof(struct mmc_command));
	cmd.opcode=MMC_READ_SINGLE_BLOCK;
	cmd.arg=address;
	cmd.flags=MMC_RSP_R1|MMC_CMD_ADTC;

	if (mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT)!=0) {
		printk("[SD%d] %s: RETRY -> send CMD17 fail\n",host->id, __func__);
		return SD_FALSE;
	}

	MSG(CFG,"RETRY -> Single block read:Polling, Data Len = 0x%04x\n", uLen);
	while (uLen > 0)
	{
		while ((!msdc_is_fifo_empty()) && (uLen >= 4)) {
			temp = sdr_read32(MSDC_DAT);
			//MSG(CFG,"RETRY -> MSDC_DAT = 0x%x\n",temp);
			uLen -= sizeof(u32);
		}

		tmo ++ ;
		udelay (1) ;

		if (tmo>=TIMEOUT) {
			if(!msdc_is_fifo_empty()) {
				tmo = 0;
				printk("[SD%d] %s: RETRY -> PollingRead() - Context TimeOut!!\n", host->id, __func__);
			} else { // This is real hardware timeout
				printk("[SD%d] %s: RETRY -> PollingRead() - TimeOut!,msdc_sta=0x%x\n", host->id, __func__, sdr_read16(MSDC_STA));
				printk("[SD%d] %s: RETRY -> PollingRead() - Remain %d bytes\n", host->id, __func__, uLen);
				break;
			}
		}
	}

	while (!(sdc_datsta = sdr_read16(SDC_DATSTA))) {
		uWaitCount++;
		if (uWaitCount > 0x200000) {
			printk("[SD%d] %s: RETRY -> PollingWaitDatRdy(): Timeout, sdc_datsta=0x%x,Remain %d bytes\n", host->id, __func__, sdc_datsta,uLen);
			goto fail;
		}
	}

	if(sdc_datsta & SDC_DATSTA_DATTO) {
		printk("[SD%d] %s: RETRY -> PollingWaitDatRdy(): DATTO! %x!\n", host->id, __func__,sdc_datsta);
		goto fail;
	} else if (sdc_datsta & SDC_DATSTA_DATCRCERR) {
		printk("[SD%d] %s: RETRY -> PollingWaitDatRdy(): CRC Error! %x!\n", host->id, __func__, sdc_datsta);
		goto fail;
	} else if(sdc_datsta & SDC_DATSTA_BLKDONE) {
		printk("[SD%d] %s: RETRY -> PollingWaitDatRdy(): BLOCK Done! %x!\n", host->id, __func__,sdc_datsta);
		goto done;
	}

done:
	printk("[SD%d] %s: RETRY -> mt6573 retry and single read data OK!!\n",host->id, __func__);
	return SD_TRUE;

fail:
	printk("[SD%d] %s: RETRY -> mt6573 retry and single read data failed!!\n",host->id, __func__);
	return SD_FALSE;
}

static int SD_CanMultiReadBeLatched(struct mt6573_sd_host* host, u32 address)
{
	u32 uLen       = 4*(BLK_SIZ_CNT_OF_512<<9);
	u32 TIMEOUT    = 2000*uLen*100/512;
	u32 tmo        = 0 ;
	u32 temp = 0;
	u32 base = host->base;
	u32 uWaitCount = 0x0;
	u16 sdc_datsta;
	struct mmc_command cmd;

	sdr_read32(SDC_CMDSTA);
	sdr_read32(BOOT_STA);
	sdr_read32(SDC_DATSTA);

	mt6573_sd_stop_dma(host);
	msdc_dma_off();

	if (SD_FALSE == mt6573_sd_retry_setblksz(host, BLK_SIZ_CNT_OF_512))
		return SD_FALSE;

	msdc_clr_fifo();
	msdc_reset();

	memset(&cmd,0,sizeof(struct mmc_command));
	cmd.opcode=MMC_READ_MULTIPLE_BLOCK;
	cmd.arg=address;
	cmd.flags=MMC_RSP_R1|MMC_CMD_ADTC;

	if(mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT)!=0) {
		printk("[SD%d] %s: RETRY -> mt6573 retry send CMD18 fail\n",host->id, __func__);
		return SD_FALSE;
	}

	MSG(CFG,"RETRY -> MultiPollingRead(), dwLen: 0x%04x\n", uLen);
	while(uLen > 0)
	{
		while ((!msdc_is_fifo_empty()) && (uLen >= 4)) {
			temp = sdr_read32(MSDC_DAT);
			//MSG(CFG,"RETRY -> MSDC_DAT = 0x%x\n",temp);
			uLen -= sizeof(u32);
		}

		tmo ++ ;
		udelay (1) ;
		if (tmo>=TIMEOUT) {
			if(!msdc_is_fifo_empty()) {
				tmo = 0;
				printk("[SD%d] %s: RETRY -> MultiPollingRead() - Context TimeOut!!\n", host->id, __func__);
			} else { // This is real hardware timeout
				printk("[SD%d] %s: RETRY -> MultiPollingRead() - TimeOut!,msdc_sta=0x%x\n", host->id, __func__, sdr_read16(MSDC_STA));
				printk("[SD%d] %s: RETRY -> MultiPollingRead() - Remain %d bytes\n", host->id, __func__, uLen);
				break;
			}
		}
	}

	while (!(sdc_datsta = sdr_read16(SDC_DATSTA))) {
		uWaitCount++;
		if (uWaitCount > 0x200000) {
			printk("[SD%d] %s: RETRY -> MultiPollingWaitDatRdy(): Timeout, sdc_datsta=0x%x,Remain %d bytes\n", host->id, __func__, sdc_datsta,uLen);
			goto fail;
		}
	}

	if(sdc_datsta & SDC_DATSTA_DATTO) {
		printk("[SD%d] %s: RETRY -> MultiPollingWaitDatRdy(): DATTO! SDC_DATSTA=0x%x!\n", host->id, __func__, sdc_datsta);
		goto fail;
	} else if (sdc_datsta & SDC_DATSTA_DATCRCERR) {
		printk("[SD%d] %s: RETRY -> MultiPollingWaitDatRdy(): CRC Error! SDC_DATSTA=0x%x!\n", host->id, __func__, sdc_datsta);
		goto fail;
	} else if(sdc_datsta & SDC_DATSTA_BLKDONE) {
		printk("[SD%d] %s: RETRY -> MultiPollingWaitDatRdy(): BLOCK Done! SDC_DATSTA=0x%x!\n", host->id, __func__, sdc_datsta);
		goto done;
	}

done:
	printk("[SD%d] %s: RETRY -> mt6573 retry and multiple read data OK!!\n",host->id, __func__);
	SD_retrySendCMD12(host);
	return SD_TRUE;

fail:
	SD_retrySendCMD12(host);
	printk("[SD%d] %s: RETRY -> mt6573 retry and multiple read data failed!!\n",host->id, __func__);
	return SD_FALSE;
}

static int mt6573_sd_clktuning_feedbackclk(struct mt6573_sd_host* host, u32 address)
{
	int i=0;
	u32 base = host->base;

	sdr_clr_bits(CLKACB_CFG, CLKACB_CFG_CLK_LAT); // feedback clock is used to latch data

	MSG(CFG,"RETRY -> enter clktuning, address=0x%x\n", address);

	for(i=0;i<2;i++)
	{
		/* First time, try reading at falling edge, and than try reading data at rising edge.*/
		MSG(CFG,"RETRY -> set clock %s edge to latch data!!\n", (!i)?("falling"):("rising"));
		sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_RED, !i);

		/* First step, check the CMD sending is OK after CLK changed.*/
		if (SD_FALSE == SD_CanCmdBeLatched(host)) {
			msdc_reset();
			continue;
		}

		/* Now, try to read data again.*/
		if (SD_TRUE == SD_CanSingleReadBeLatched(host, address)) {
			printk("[SD%d] %s: RETRY -> %s edge to single read successfully and start multiple read tuning!!\n",
					host->id, __func__, (!i)?("falling"):("rising"));

			if(SD_TRUE == SD_CanMultiReadBeLatched(host, address)) {
				printk("[SD%d] %s: RETRY -> %s edge to multiple read successfully!!\n", host->id, __func__, (!i)?("falling"):("rising"));
				return SD_TRUE;
			}
		}

		/* End of the loop, reset the host controller */
		msdc_reset();
		udelay(100);
	}

	MSG(CFG,"RETRY -> mt6573 feedback clock tuning failed!!\n");

	return SD_FALSE;
}

static inline u32 mt6573_sd_getcurclock(struct mt6573_sd_host* host)
{
	u32 base      = host->base;
	u32 cur_clock = 0;
	u32 status    = sdr_read32(MSDC_CFG);
	u8  i         = 0;
	u8  j         = 0;

	i = (status & 0x18)>>3;
	j = (status & 0xFF00)>>8;
	cur_clock=ghclk_tab[i]/((j==0)?(2):(4*j));

	host->hclk        = ghclk_tab[i];
	host->sclk        = cur_clock;
	host->hw->clk_src = i;
	printk("[SD%d] %s: Current clock of SD Bus is %d\n",host->id, __func__, cur_clock);
	return cur_clock;
}

static int mt6573_sd_until_oprable(struct mt6573_sd_host* host)
{
	u32 base          = host->base;
	unsigned long tmo = jiffies + CMD_TIMEOUT;
	u16 sdc_status;

	sdc_status=sdr_read16(SDC_STA);

	if (sdc_status & (SDC_STA_FECMDBUSY | SDC_STA_FEDATBUSY))
		SD_retrySendCMD12(host);

	while ((sdc_status=sdr_read16(SDC_STA)) & 0x001F) {
		msdc_reset();

		if (time_after(jiffies, tmo)) {
			printk(KERN_ERR "[SD%d] %s: RETRY -> host is busy, SDC_STA=0x%x\n",
					host->id, __func__, sdc_status);
			return SD_FALSE;
		}
	}

	return SD_TRUE;
}

static void prepare_ios(struct mt6573_sd_host* host){
	int bit;

	MSG(CFG,"prepare ios\r\n");
	if (host->mmc->ocr)
		bit = ffs(host->mmc->ocr) - 1;
	else
		bit = fls(host->mmc->ocr_avail) - 1;

	host->mmc->ios.vdd = bit;
	host->mmc->ios.chip_select = MMC_CS_DONTCARE;
	host->mmc->ios.bus_mode = MMC_BUSMODE_OPENDRAIN;
	host->mmc->ios.power_mode = MMC_POWER_UP;
	host->mmc->ios.bus_width = MMC_BUS_WIDTH_1;
	host->mmc->ios.timing = MMC_TIMING_LEGACY;
	mt6573_sd_set_ios(host->mmc, &host->mmc->ios);
	mdelay(1);

	host->mmc->ios.clock = host->mmc->f_min;
	host->mmc->ios.power_mode = MMC_POWER_ON;
	mt6573_sd_set_ios(host->mmc, &host->mmc->ios);
	mdelay(1);

}

static void restore_ios(struct mt6573_sd_host* host, struct mmc_ios old_ios)
{
	MSG(CFG,"restore ios\r\n");

	host->mmc->ios.chip_select = old_ios.chip_select;
	host->mmc->ios.bus_mode = old_ios.bus_mode;
	host->mmc->ios.power_mode = old_ios.power_mode;
	mt6573_sd_set_ios(host->mmc, &host->mmc->ios);
	mdelay(1);
}
//return 0 if success, else return cmd->error or 1(not sd card flow)
static int mt6573_host_init_sd(struct mt6573_sd_host* host)
{
	int ret = 0;
	struct mmc_command cmd;
	u32 ocr = host->mmc->ocr;
	u32 cid[4];
	int i = 0;
	struct mmc_ios old_ios = host->mmc->ios;

	if(!mmc_card_sd(host->mmc->card)){
		printk("[SD%d] [%s]: failed because init_flow != MMC_INIT_FLOW_OF_SD\r\n", host->id,  __func__);
		return 1;
	}
	if(host->mmc->caps & MMC_CAP_SPI){
		printk("[SD%d] [%s]: failed because do not support SPI protocal != MMC_INIT_FLOW_OF_SD\r\n", host->id, __func__);
		return 2;
	}

	prepare_ios(host);

	//CMD0: go_to_idle
	cmd.opcode = MMC_GO_IDLE_STATE;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_NONE | MMC_CMD_BC;
	ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
	if(ret != 0)
	{
		goto fail;
	}
	mdelay(1);

	//CMD8: mmc_send_if_cond
	cmd.opcode = SD_SEND_IF_COND; //MMC_SEND_EXT_CSD;
	cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | 0xAA;
	cmd.flags = MMC_RSP_SPI_R7 | MMC_RSP_R7 | MMC_CMD_BCR;
	ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
	if(ret != 0)
	{
		goto fail;
	}
	ocr |= 1 << 30;

	//ACMD41: mmc_send_app_op_cond
	for (i = 100; i; i--) {
		//CMD55
		cmd.opcode = MMC_APP_CMD;
		cmd.arg = 0;
		cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_BCR;
		ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
		if(ret != 0){
			printk("[SD%d] [%s]: failed at CMD%d, ocr=%d, ret=%d\r\n", host->id, __func__, cmd.opcode,ocr, ret);
			continue;
		}

		//CMD41
		cmd.opcode = SD_APP_OP_COND;
		cmd.arg = ocr;
		cmd.data = NULL;
		cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R3 | MMC_CMD_BCR;
		ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
		if(ret != 0){
			printk("[SD%d] [%s]: failed at CMD%d, ocr=%d, ret=%d\r\n", host->id, __func__, cmd.opcode,ocr, ret);
			continue;
		}

		/* otherwise wait until reset completes */
		if (cmd.resp[0] & MMC_CARD_BUSY){
			break;
		}

		ret = -ETIMEDOUT;
		mdelay(10); //patched by MTK
	}
	if(ret != 0)
	{
		goto fail;
	}

	//CMD2: mmc_all_send_cid
	cmd.opcode = MMC_ALL_SEND_CID;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R2 | MMC_CMD_BCR;
	ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
	if(ret != 0){
		goto fail;
	}
	memcpy(cid, cmd.resp, sizeof(u32) * 4);
	if (memcmp(cid, host->mmc->card->raw_cid, sizeof(cid)) != 0) {
		printk("[SD%d] [%s]: failed at CMD%d, CID is changed, ret=%d\r\n", host->id, __func__, cmd.opcode, ret);
		ret = -ENOENT;
		goto fail;
	}

	//CMD3: mmc_send_relative_addr
	cmd.opcode = SD_SEND_RELATIVE_ADDR;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R6 | MMC_CMD_BCR;
	ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
	if(ret != 0){
		goto fail;
	}
	MSG(CFG,"CMD%d, old_rca=0x%x, new_rca = 0x%x,ret=%d\r\n", cmd.opcode,host->mmc->card->rca, (cmd.resp[0] >> 16), ret);
	host->mmc->card->rca = cmd.resp[0] >> 16;

	//CMD7: mmc_select_card
	for(i=0; i<3; i++){
		cmd.opcode = MMC_SELECT_CARD;
		cmd.arg = host->mmc->card->rca << 16;
		cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
		ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
		if(ret == 0){
			break;
		}
	}
	if(ret != 0){
		goto fail;
	}

	//CMD6: swith_high_speed
	if(mmc_card_highspeed(host->mmc->card)){ /* host->mmc->card->state & MMC_STATE_HIGHSPEED) */
		cmd.opcode = SD_SWITCH;
		cmd.arg = 0x1 << 31 | 0x00FFFFFF;
		cmd.arg &= ~(0xF << (0 * 4));
		cmd.arg |= 0x1 << (0 * 4);
		cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

		ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
		if(ret != 0){
			goto fail;
		}
		host->mmc->ios.timing = MMC_TIMING_SD_HS;
		mt6573_sd_set_ios(host->mmc, &host->mmc->ios);

		/*cmd.opcode = MMC_SEND_STATUS;
        cmd.arg = host->mmc->card->rca << 16;
        cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;
        ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);	
        if (ret)
        	 goto fail;
        	 
        if(cmd.resp[0] ==0xb00)
        {
        	 ret = -1;
        	 goto fail;
        }*/
		host->mmc->ios.clock = HOST_MAX_SCLK;
		host->mmc->f_max     = HOST_MAX_SCLK;
	} else {
		host->mmc->ios.clock = 25000000;
		host->mmc->f_max     = 25000000;
	}
	host->hw->clk_src = 0;
	host->hclk        = ghclk_tab[0];

	mt6573_sd_set_ios(host->mmc, &host->mmc->ios);
	mdelay(5);

	//ACMD6: switch_bus_width
	if ((host->mmc->caps & MMC_CAP_4_BIT_DATA) &&(host->mmc->card->scr.bus_widths & SD_SCR_BUS_WIDTH_4)) {
		for(i=0; i<5; i++){
			//CMD55
			cmd.opcode = MMC_APP_CMD;
			cmd.arg = host->mmc->card->rca << 16;;
			cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
			ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
			if(ret != 0){
				continue;
			}

			//CMD6
			cmd.opcode = SD_APP_SET_BUS_WIDTH;
			cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
			cmd.arg = SD_BUS_WIDTH_4;

			ret = mt6573_sd_send_command(host,&cmd,1,CMD_TIMEOUT);
			if(ret == 0){
				break;
			}
		}
		if(ret !=0)
			goto fail;

		host->mmc->ios.bus_width = MMC_BUS_WIDTH_4;
		mt6573_sd_set_ios(host->mmc, &host->mmc->ios);
	}

	if(ret == 0)
		goto end;

fail:
	printk("[SD%d] [%s]: failed at CMD%d,cmd.error=%d, ret=%d\r\n", host->id, __func__, cmd.opcode, cmd.error,ret);

end:
	restore_ios(host, old_ios);
	return ret;
}

static int  mt6573_sd_power_tuning_feedback(struct mt6573_sd_host* host)
{
	int ret = -1;
	if(host->mmc->card == NULL){
		//Do not waste too much time at power tuning when sd retry is failed at system booting.
		//We do not know the card type, cannot accomplish the initialization
		printk("[%s]: [SD%d] host->mmc->card is NULL, cancel power tuning\r\n", __func__, host->id);
	}else{
		if(mmc_card_sd(host->mmc->card))
		{
			MSG(CFG, "SD card, start power tuning\r\n");
			mt6573_sd_card_power(host, 0);
			mdelay(2); //patched by MTK
			mt6573_sd_card_power(host, 1);
			mt6573_sd_card_clock(host, 1);
			ret = mt6573_host_init_sd(host);
			if(ret != 0){				
				Is_In_ReTry--; 
				printk("[SD%d] [%s]: failed to init sd card, remain retry time:%d\r\n", host->id, __func__, Is_In_ReTry);
			}else
				MSG(CFG,"complete retry power\r\n");
		}else{
			MSG(CFG, "!SD card, cancel power tuning\r\n");
		}
	}
	return ret;
}

static int mt6573_sd_retry(struct mt6573_sd_host* host, u32 address)
{
	u32 base       = host->base;
	u32 clkctl[]   = {MSDC_CCTL, MSDC2_CCTL, MSDC3_CCTL, MSDC4_CCTL};
	struct clk{
		u32 clock;
		u8  sd_clksrc;
		u8  sd_sclkf;
	}clk_table[]={
		{49150000, 0, 0},
		{40950000, 1, 0},
		{30700000, 3, 0},
		{24575000, 0, 1},
		{20475000, 1, 1},
		{10237500, 1, 2},
	};
#define  MAX_CLK_TABLE   ARRAY_SIZE(clk_table)
	u32  cur_clock = 0;
	u8   index     = 0;
	u8   enable_power_tune = 0;

	/* check sd card exists or not */
	MSG(CFG,"RETRY -> enter sd_retry\n");
	if (!is_card_present(host) || host->power_mode == MMC_POWER_OFF) {
		MSG(CFG,"mmc_card is not present or power off\n");
		return SD_FALSE;
	}
	MSG(CFG,"RETRY -> mmc_card is present\n");

retry:
	if (SD_FALSE == mt6573_sd_until_oprable(host)) {
		printk("[SD%d] %s: RETRY -> Busy before clock tuning.\n", host->id, __func__);
		goto fail;
	}

	cur_clock = mt6573_sd_getcurclock(host);
	while (clk_table[index].clock > cur_clock) {
		index++;
		if (index >= MAX_CLK_TABLE) {
			printk("[SD%d] %s: RETRY -> Can't select lower clock!\n",
					host->id, __func__);
			goto fail;
		}
	}

	MSG(CFG,"start clock tuning\n");

	while (index < MAX_CLK_TABLE) {
		sdr_set_field(MSDC_CFG,  MSDC_CFG_CLKSRC, clk_table[index].sd_clksrc);
		sdr_set_field(clkctl[host->id], MSDC_SEL, clk_table[index].sd_clksrc);

		sdr_set_field(MSDC_CFG,  MSDC_CFG_SCLKF,  clk_table[index].sd_sclkf);

		mdelay(1);
		MSG(CFG,"RETRY -> Select retry clock: %d, MSDC_CFG=0x%x\n", mt6573_sd_getcurclock(host), sdr_read32(MSDC_CFG));

		if (SD_TRUE == mt6573_sd_clktuning_feedbackclk(host, address))
			goto done;

		if (SD_FALSE == mt6573_sd_until_oprable(host)) {
			printk("[SD%d] %s: RETRY -> Busy between clock tunings.\n", host->id, __func__);
			goto fail;
		}
		index++;
	}

	if (index >= MAX_CLK_TABLE)
		printk("[SD%d] %s: RETRY -> Can't select lower clock!\n", host->id, __func__);

	MSG(CFG,"end clock tuning\r\n");
fail:
	if(!enable_power_tune){
		enable_power_tune = 1;
		printk("[SD%d] %s: RETRY -> Clock tuning failed, MSDC_CFG=0x%x, CLKACB_CFG=0x%x,"
				"reset and reinit the card.\n",
				host->id, __func__, sdr_read32(MSDC_CFG), sdr_read32(CLKACB_CFG));
		if (mt6573_sd_power_tuning_feedback(host) == 0) {
			printk("[SD%d] %s: RETRY -> Reset and reinit the card successfully, "
					"and try clock tuning again.\n", host->id, __func__);
			index = 0;
			goto retry;
		}
	}else{
		Is_In_ReTry--; 
		printk("[SD%d] %s: RETRY -> remain retry time:%d\n", host->id, __func__, Is_In_ReTry);
	}

	printk("[SD%d] %s: RETRY -> mt6573 sd retry fails, MSDC_CFG=0x%x, CLKACB_CFG=0x%x\n",
			host->id, __func__, sdr_read32(MSDC_CFG), sdr_read32(CLKACB_CFG));
	mt6573_sd_getcurclock(host);
	return SD_FALSE;
done:
	if (host &&(host->mmc)) {
		host->mmc->f_max = mt6573_sd_getcurclock(host);
		printk("[SD%d] %s: RETRY -> Clock tuning successfully, MSDC_CFG=0x%x, CLKACB_CFG=0x%x.\n",
				host->id, __func__,sdr_read32(MSDC_CFG),sdr_read32(CLKACB_CFG));
	}
	return SD_TRUE;
}
//Re-Try--

#ifdef CFG_PROFILING
struct mmc_op_report {
	unsigned long count;          /* the count of this operation */
	unsigned long min_time;       /* the min. time of this operation */
	unsigned long max_time;       /* the max. time of this operation */
	unsigned long total_time;     /* the total time of this operation */
	unsigned long total_size;     /* the total size of this operation */
};

struct mmc_op_perf {
	struct mt6573_sd_host *host;
	struct mmc_op_report   single_blk_read;
	struct mmc_op_report   single_blk_write;
	struct mmc_op_report   multi_blks_read;
	struct mmc_op_report   multi_blks_write;
};

static struct mmc_op_perf sd_perf[HOST_MAX_NUM];
static int sd_perf_rpt[] = {1, 1, 0};

static void mmc_perf_report(struct mmc_op_report *rpt)
{
	printk(KERN_INFO "\tCount      : %ld\n", rpt->count);
	printk(KERN_INFO "\tMax. Time  : %ld counts\n", rpt->max_time);
	printk(KERN_INFO "\tMin. Time  : %ld counts\n", rpt->min_time);
	printk(KERN_INFO "\tTotal Size : %ld KB\n", rpt->total_size / 1024);
	printk(KERN_INFO "\tTotal Time : %ld counts\n", rpt->total_time);
	if (rpt->total_time) {
		printk(KERN_INFO "\tPerformance: %ld KB/sec\n",
				((rpt->total_size / 1024) * 32768) / rpt->total_time);
	}
}

static int mmc_perf_dump(int dev_id)
{
	struct mt6573_sd_host *host;
	struct mmc_op_perf *perf;
	u32 total_read_size, total_write_size;
	u32 total_read_time, total_write_time;

	perf = &sd_perf[dev_id];
	host = sd_perf[dev_id].host;

	total_read_size = total_write_size = 0;
	total_read_time = total_write_time = 0;

	printk(KERN_INFO "\n============== [SD Host %d] ==============\n", dev_id);
	printk(KERN_INFO " OP Clock Freq. : %d khz\n", host->hclk / 1000);
	printk(KERN_INFO " SD Clock Freq. : %d khz\n", host->sclk / 1000);

	if (perf->multi_blks_read.count) {
		printk(KERN_INFO " Multi-Blks-Read:\n");
		mmc_perf_report(&perf->multi_blks_read);
		total_read_size += perf->multi_blks_read.total_size;
		total_read_time += perf->multi_blks_read.total_time;
	}
	if (perf->multi_blks_write.count) {
		printk(KERN_INFO " Multi-Blks-Write:\n");
		mmc_perf_report(&perf->multi_blks_write);
		total_write_size += perf->multi_blks_write.total_size;
		total_write_time += perf->multi_blks_write.total_time;
	}
	if (perf->single_blk_read.count) {
		printk(KERN_INFO " Single-Blk-Read:\n");
		mmc_perf_report(&perf->single_blk_read);
		total_read_size += perf->single_blk_read.total_size;
		total_read_time += perf->single_blk_read.total_time;
	}
	if (perf->single_blk_write.count) {
		printk(KERN_INFO " Single-Blk-Write:\n");
		mmc_perf_report(&perf->single_blk_write);
		total_write_size += perf->single_blk_write.total_size;
		total_write_time += perf->single_blk_write.total_time;
	}
	if (total_read_time) {
		printk(KERN_INFO " Performance Read : %d KB/sec\n",
				((total_read_size / 1024) * 32768) / total_read_time);
	}
	if (total_write_time) {
		printk(KERN_INFO " Performance Write: %d KB/sec\n",
				((total_write_size / 1024) * 32768) / total_write_time);
	}

	printk(KERN_INFO "========================================\n\n");

	return 0;
}

static ssize_t mmc_perf_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev;
	struct mt6573_sd_host *host;
	struct mmc_host *mmc;

	pdev = container_of(dev, struct platform_device, dev);
	mmc  = platform_get_drvdata(pdev);
	host = mmc_priv(mmc);
	return mmc_perf_dump(host->id);
}

static ssize_t mmc_perf_reset(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device *pdev;
	struct mt6573_sd_host *host;
	struct mmc_host *mmc;

	pdev = container_of(dev, struct platform_device, dev);
	mmc  = platform_get_drvdata(pdev);
	host = mmc_priv(mmc);

	host = sd_perf[host->id].host;
	memset(&sd_perf[host->id], 0, sizeof(struct mmc_op_perf));
	sd_perf[host->id].host = host;

	return count;
}

DEVICE_ATTR(perf, S_IWUSR | S_IRUGO, mmc_perf_show, mmc_perf_reset);

static int mmc_perf_init(struct mt6573_sd_host *host, struct platform_device *pdev)
{
	memset(&sd_perf[host->id], 0, sizeof(struct mmc_op_perf));
	sd_perf[host->id].host = host;
	return device_create_file(&pdev->dev, &dev_attr_perf);
}

static void xgpt_timer_init(XGPT_NUM num)
{
	XGPT_CONFIG config;

	config.num = num; /* 32768Hz */
	config.mode = XGPT_KEEP_GO;
	config.bIrqEnable = FALSE;
	config.clkDiv = XGPT_CLK_DIV_1;
	config.u4Compare = 32768;
	XGPT_Config(config);
}

static void xgpt_timer_start(XGPT_NUM num)
{
	XGPT_Start(num);
}
static void xgpt_timer_stop(XGPT_NUM num)
{
	XGPT_Stop(num);
}
static void xgpt_timer_stop_clear(XGPT_NUM num)
{
	XGPT_Stop(num);
	XGPT_ClearCount(num);
}
static unsigned int xgpt_timer_get_count(XGPT_NUM num)
{
	return (unsigned int)XGPT_GetCounter(num);
}
#endif

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

#if 0
static void msdc_dma_reset(struct msdc_dma *dma)
{
	u32 base = dma->base;

	/* hard reset */
	sdr_set_bits(DMA_RST, 0x2);
	dsb();
	sdr_clr_bits(DMA_RST, 0x2);
	sdr_write32(DMA_MEM_ADDR, 0);
}
#endif

static void msdc_dma_init(struct msdc_dma *dma, int id, struct resource *res)
{
	memset(dma, 0, sizeof(struct msdc_dma));

	snprintf(dma->name, 16, "msdc%d-dma", id);
	dma->id   = id;
	dma->base = res->start;
	dma->res  = res;
	res = request_mem_region(res->start, res->end - res->start + 1, dma->name);

	BUG_ON(!res);
}

static void msdc_dma_deinit(struct msdc_dma *dma)
{
	struct resource *res = dma->res;
	release_mem_region(res->start, res->end - res->start + 1);
	memset(dma, 0, sizeof(struct msdc_dma));
}

#ifdef MT6573_SD_DEBUG
static void mt6573_sd_dump_card_status(struct mt6573_sd_host *host, u32 status)
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
		"Reserved",		/* 9 */
		"Reserved",		/* 10 */
		"Reserved",		/* 11 */
		"Reserved",		/* 12 */
		"Reserved",		/* 13 */
		"Reserved",		/* 14 */
		"I/O mode",		/* 15 */
	};
	if (status & R1_OUT_OF_RANGE)
		MSG(RSP, "\t[CARD_STATUS] Out of Range\n");
	if (status & R1_ADDRESS_ERROR)
		MSG(RSP, "\t[CARD_STATUS] Address Error\n");
	if (status & R1_BLOCK_LEN_ERROR)
		MSG(RSP, "\t[CARD_STATUS] Block Len Error\n");
	if (status & R1_ERASE_SEQ_ERROR)
		MSG(RSP, "\t[CARD_STATUS] Erase Seq Error\n");
	if (status & R1_ERASE_PARAM)
		MSG(RSP, "\t[CARD_STATUS] Erase Param\n");
	if (status & R1_WP_VIOLATION)
		MSG(RSP, "\t[CARD_STATUS] WP Violation\n");
	if (status & R1_CARD_IS_LOCKED)
		MSG(RSP, "\t[CARD_STATUS] Card is Locked\n");
	if (status & R1_LOCK_UNLOCK_FAILED)
		MSG(RSP, "\t[CARD_STATUS] Lock/Unlock Failed\n");
	if (status & R1_COM_CRC_ERROR)
		MSG(RSP, "\t[CARD_STATUS] Command CRC Error\n");
	if (status & R1_ILLEGAL_COMMAND)
		MSG(RSP, "\t[CARD_STATUS] Illegal Command\n");
	if (status & R1_CARD_ECC_FAILED)
		MSG(RSP, "\t[CARD_STATUS] Card ECC Failed\n");
	if (status & R1_CC_ERROR)
		MSG(RSP, "\t[CARD_STATUS] CC Error\n");
	if (status & R1_ERROR)
		MSG(RSP, "\t[CARD_STATUS] Error\n");
	if (status & R1_UNDERRUN)
		MSG(RSP, "\t[CARD_STATUS] Underrun\n");
	if (status & R1_OVERRUN)
		MSG(RSP, "\t[CARD_STATUS] Overrun\n");
	if (status & R1_CID_CSD_OVERWRITE)
		MSG(RSP, "\t[CARD_STATUS] CID/CSD Overwrite\n");
	if (status & R1_WP_ERASE_SKIP)
		MSG(RSP, "\t[CARD_STATUS] WP Eraser Skip\n");
	if (status & R1_CARD_ECC_DISABLED)
		MSG(RSP, "\t[CARD_STATUS] Card ECC Disabled\n");
	if (status & R1_ERASE_RESET)
		MSG(RSP, "\t[CARD_STATUS] Erase Reset\n");
	if (status & R1_READY_FOR_DATA)
		MSG(RSP, "\t[CARD_STATUS] Ready for Data\n");
	if (status & R1_APP_CMD)
		MSG(RSP, "\t[CARD_STATUS] App Command\n");

	MSG(RSP, "\t[CARD_STATUS] '%s' State\n",
			state[R1_CURRENT_STATE(status)]);
}

static void mt6573_sd_dump_ocr_reg(struct mt6573_sd_host *host, u32 resp)
{
	if (resp & (1 << 7))
		MSG(RSP, "\t[OCR] Low Voltage Range\n");
	if (resp & (1 << 15))
		MSG(RSP, "\t[OCR] 2.7-2.8 volt\n");
	if (resp & (1 << 16))
		MSG(RSP, "\t[OCR] 2.8-2.9 volt\n");
	if (resp & (1 << 17))
		MSG(RSP, "\t[OCR] 2.9-3.0 volt\n");
	if (resp & (1 << 18))
		MSG(RSP, "\t[OCR] 3.0-3.1 volt\n");
	if (resp & (1 << 19))
		MSG(RSP, "\t[OCR] 3.1-3.2 volt\n");
	if (resp & (1 << 20))
		MSG(RSP, "\t[OCR] 3.2-3.3 volt\n");
	if (resp & (1 << 21))
		MSG(RSP, "\t[OCR] 3.3-3.4 volt\n");
	if (resp & (1 << 22))
		MSG(RSP, "\t[OCR] 3.4-3.5 volt\n");
	if (resp & (1 << 23))
		MSG(RSP, "\t[OCR] 3.5-3.6 volt\n");
	if (resp & (1 << 30))
		MSG(RSP, "\t[OCR] Card Capacity Status (CCS)\n");
	if (resp & (1 << 31))
		MSG(RSP, "\t[OCR] Card Power Up Status (Idle)\n");
	else
		MSG(RSP, "\t[OCR] Card Power Up Status (Busy)\n");
}

static void mt6573_sd_dump_rca_resp(struct mt6573_sd_host *host, u32 resp)
{
	u32 card_status = (((resp >> 15) & 0x1) << 23) |
		(((resp >> 14) & 0x1) << 22) |
		(((resp >> 13) & 0x1) << 19) |
		(resp & 0x1fff);

	MSG(RSP, "\t[RCA] 0x%.4x\n", resp >> 16);
	mt6573_sd_dump_card_status(host, card_status);
}
#endif

static unsigned int mt6573_sd_send_command(struct mt6573_sd_host *host,
		struct mmc_command    *cmd,
		int                    polling,
		unsigned long          timeout)
{
	u32 base = host->base;
	u32 opcode = cmd->opcode;
	u32 rawcmd;
	u32 resp;
	u32 status;
	unsigned long tmo;

	/* Protocol layer does not provide response type, but our hardware needs
	 * to know exact type, not just size!
	 */

	if (opcode == MMC_SEND_OP_COND || opcode == SD_APP_OP_COND)
		resp = RESP_R3;
	else if (opcode == MMC_SET_RELATIVE_ADDR || opcode == SD_SEND_RELATIVE_ADDR)
		resp = (mmc_cmd_type(cmd) == MMC_CMD_BCR) ? RESP_R6 : RESP_R1;
	else if (opcode == MMC_FAST_IO)
		resp = RESP_R4;
	else if (opcode == MMC_GO_IRQ_STATE)
		resp = RESP_R5;
	else if (opcode == MMC_SELECT_CARD)
		resp = (cmd->arg != 0) ? RESP_R1B : RESP_NONE;
	else if (opcode == SD_IO_RW_DIRECT || opcode == SD_IO_RW_EXTENDED)
		resp = RESP_R1; /* SDIO workaround. */
	else if (opcode == SD_SEND_IF_COND && (mmc_cmd_type(cmd) == MMC_CMD_BCR))
		resp = RESP_R1;
	else {
		switch (mmc_resp_type(cmd)) {
		case MMC_RSP_R1:
			resp = RESP_R1;
			break;
		case MMC_RSP_R1B:
			resp = RESP_R1B;
			break;
		case MMC_RSP_R2:
			resp = RESP_R2;
			break;
		case MMC_RSP_R3:
			resp = RESP_R3;
			break;
		case MMC_RSP_NONE:
		default:
			resp = RESP_NONE;
			break;
		}
	}

	cmd->error = 0;
	rawcmd = opcode | resp << 7;

	/* cmd = opc | rtyp << 7 | idt << 10 | dtype << 11 | rw << 13 | stp << 14 */

	if (opcode == MMC_READ_MULTIPLE_BLOCK) {
		rawcmd |= (2 << 11);
	} else if (opcode == MMC_READ_SINGLE_BLOCK) {
		rawcmd |= (1 << 11);
	} else if (opcode == SD_IO_RW_EXTENDED) {
		if (cmd->data->flags & MMC_DATA_WRITE)
			rawcmd |= (1 << 13);
		if (cmd->data->blocks > 1)
			rawcmd |= (2 << 11);
		else
			rawcmd |= (1 << 11);
	} else if (opcode == MMC_WRITE_MULTIPLE_BLOCK) {
		rawcmd |= ((2 << 11) | (1 << 13));
	} else if (opcode == MMC_WRITE_BLOCK) {
		rawcmd |= ((1 << 11) | (1 << 13));
	} else if (opcode == SD_IO_RW_DIRECT && cmd->flags == (unsigned int)-1) {
		rawcmd |= (1 << 14);
	} else if ((opcode == SD_APP_SEND_SCR) ||
			(opcode == SD_APP_SEND_NUM_WR_BLKS) ||
			(opcode == SD_SWITCH && (mmc_cmd_type(cmd) == MMC_CMD_ADTC)) ||
			(opcode == SD_APP_SD_STATUS && (mmc_cmd_type(cmd) == MMC_CMD_ADTC)) ||
			(opcode == MMC_SEND_EXT_CSD && (mmc_cmd_type(cmd) == MMC_CMD_ADTC))) {
		rawcmd |= (1 << 11);
	} else if (opcode == MMC_STOP_TRANSMISSION) {
		rawcmd |= (1 << 14);
	} else if (opcode == MMC_ALL_SEND_CID) {
		rawcmd |= (1 << 10);
	}

	MSG(CMD, "SND_CMD(%d): ARG(0x%.8x), RAW(0x%.8x), RSP(%d), Polling(%d)\n",
			opcode, cmd->arg, rawcmd, resp, polling);
	/*
	    printk("[SD%d] %s: MSDC_CFG(0x%x): SDC_CFG(0x%x), CLKACB_CFG(0x%x),MSDC_IOCON0(0x%x),MSDC_IOCON1(0x%x),SDC_IRQMASK0(0x%x)\n",
	    host->id, __func__, sdr_read32(MSDC_CFG),sdr_read32(SDC_CFG),sdr_read32(CLKACB_CFG),sdr_read32(MSDC_IOCON0),sdr_read32(MSDC_IOCON1),sdr_read32(SDC_IRQMASK0));
	    */
	tmo = jiffies + timeout;
	if ((resp == RESP_R1B || cmd->data) && (opcode != MMC_STOP_TRANSMISSION)) {
		/* command issued with data line */
		for (;;) {
			if (!sdc_is_busy())
				break;
			if (time_after(jiffies, tmo)) {
				printk(KERN_ERR "[SD%d] %s: polling data line is busy\n", host->id, __func__);
				cmd->error = (unsigned int)-ETIMEDOUT;
				msdc_reset();
				goto end;
			}
		}
	} else {
		/* command issued without data line */
		for (;;) {
			if (!sdc_is_cmd_busy())
				break;
			if (time_after(jiffies, tmo)) {
				printk(KERN_ERR "[SD%d] %s: cmd line is busy\n", host->id, __func__);
				cmd->error = (unsigned int)-ETIMEDOUT;
				msdc_reset();
				goto end;
			}
		}
	}

	if (resp != RESP_NONE && polling == 0) {
		BUG_ON(in_interrupt());
		init_completion(&host->cmd_done);
		host->cmd = cmd;
		host->cmd_rsp = resp;
		sdr_clr_bits(SDC_IRQMASK0,
				SDC_IRQMASK0_CMDRDY|SDC_IRQMASK0_CMDTO|SDC_IRQMASK0_RSPCRCERR);
	}
	sdr_read32(SDC_CMDSTA);
	sdc_send_cmd(rawcmd, cmd->arg);

	if (resp != RESP_NONE) {
		if (polling) {
			for (;;) {
				if ((status = sdr_read32(SDC_CMDSTA)) != 0)
					break;
				if (time_after(jiffies, tmo)) {
					printk(KERN_ERR "[SD%d] %s: cmd[%d] polling timeout\n", host->id, __func__, opcode);
					status = SDC_CMDSTA_CMDTO;
					break;
				}
			}
			if (status & SDC_CMDSTA_CMDRDY) {
				u32 *rsp = &cmd->resp[0];
				cmd->error = 0;
				if (unlikely(resp == RESP_R2)) {
					*rsp++ = sdr_read32(SDC_RESP3);
					*rsp++ = sdr_read32(SDC_RESP2);
					*rsp++ = sdr_read32(SDC_RESP1);
					*rsp++ = sdr_read32(SDC_RESP0);
				} else {
					*rsp = sdr_read32(SDC_RESP0); /* Resp: 1, 3, 4, 5, 6, 7(1b) */
				}
			} else if (status & SDC_CMDSTA_RSPCRCERR) {
				printk(KERN_ERR "[SD%d] %s: cmd[%d] resp crc error\n", host->id, __func__, opcode);
				cmd->error = (unsigned int)-EIO;
				msdc_reset();
			} else if(status & SDC_CMDSTA_CMDTO){
				printk(KERN_WARNING "[SD%d] %s: cmd[%d] resp timeout and status is 0x%x \n", host->id, __func__, opcode, status);
				cmd->error = (unsigned int)-ETIMEDOUT;
				msdc_reset();
			}
		} else {
			spin_unlock(&host->lock);
			if(!wait_for_completion_timeout(&host->cmd_done, 10*timeout)){
				printk(KERN_ERR "[SD%d] %s:CMD[%d] irq timeout\n", host->id, __func__, cmd->opcode);
				cmd->error = (unsigned int)-ETIMEDOUT;
				msdc_reset();
			}
			//wait_for_completion(&host->cmd_done);
			spin_lock(&host->lock);
			sdr_set_bits(SDC_IRQMASK0,
					SDC_IRQMASK0_CMDRDY|SDC_IRQMASK0_CMDTO|SDC_IRQMASK0_RSPCRCERR);
		}
	}

end:
#ifdef MT6573_SD_DEBUG
	switch (resp) {
	case RESP_NONE:
		MSG(RSP, "CMD(%d) ERR(%d) RSP(%d)\n",
				opcode, cmd->error, resp);
		break;
	case RESP_R2:
		MSG(RSP, "CMD:(%d) ERR(%d) RSP(%d)= %.8x %.8x %.8x %.8x\n",
				opcode, cmd->error, resp, cmd->resp[0], cmd->resp[1],
				cmd->resp[2], cmd->resp[3]);
		break;
	default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
		MSG(RSP, "CMD(%d) ERR(%d) RSP(%d)= 0x%.8x\n",
				opcode, cmd->error, resp, cmd->resp[0]);
		if (cmd->error == 0) {
			switch (resp) {
			case RESP_R1:
			case RESP_R1B:
				mt6573_sd_dump_card_status(host, cmd->resp[0]);
				break;
			case RESP_R3:
				mt6573_sd_dump_ocr_reg(host, cmd->resp[0]);
				break;
			case RESP_R6:
				mt6573_sd_dump_rca_resp(host, cmd->resp[0]);
				break;
			}
		}
		break;
	}
#endif
	if ((opcode == SD_SEND_RELATIVE_ADDR) && (cmd->error == 0))
		host->card_rca = cmd->resp[0]>>16;
	return cmd->error;
}

static int mt6573_sd_pio_read(struct mt6573_sd_host *host, struct mmc_data *data)
{
	struct scatterlist *sg = data->sg;
	u32  base = host->base;
	u32  num = data->sg_len;
	u32 *ptr;
	u32  left;
	u32  size = 0;
	u32  status;

	while (num) {
		left = sg->length;
		ptr = sg_virt(sg);
		while (left) {
			status = sdr_read32(MSDC_STA);
			if (status & MSDC_STA_DRQ) {
				if (likely(left > 3)) {
					*ptr++ = msdc_fifo_read();
					left -= 4;
				} else {
					u32 val = msdc_fifo_read();
					memcpy(ptr, &val, left);
					left = 0;
				}
			}
			if (atomic_read(&host->abort))
				goto end;
		}
		size += sg->length;
		sg = sg_next(sg); num--;
	}
end:
	data->bytes_xfered += size;

	if (data->error)
		printk(KERN_ERR "[SD%d] %s: pio read error %d\n", host->id, __func__, data->error);

	MSG(OPS, "DATA_RD: %d/%d bytes\n", size, host->xfer_size);

	return data->error;
}

static int mt6573_sd_pio_write(struct mt6573_sd_host* host, struct mmc_data *data)
{
	u32  base = host->base;
	struct scatterlist *sg = data->sg;
	u32  num = data->sg_len;
	u32 *ptr;
	u32  left;
	u32  size = 0;
	u32  status;

	while (num) {
		left = sg->length;
		ptr = sg_virt(sg);
		while (left) {
			status = sdr_read32(MSDC_STA);
			if (status & MSDC_STA_DRQ) {
				if (likely(left > 3)) {
					msdc_fifo_write(*ptr); ptr++;
					left -= 4;
				} else {
					u32 val = 0;
					memcpy(&val, ptr, left);
					msdc_fifo_write(val);
					left = 0;
				}
			}
			if (atomic_read(&host->abort))
				goto end;
		}
		size += sg->length;
		sg = sg_next(sg); num--;
	}
end:
	data->bytes_xfered = size;

	if (data->error)
		printk(KERN_ERR "[SD%d] %s: pio read error %d\n", host->id, __func__, data->error);

	MSG(OPS, "DATA_WR: %d/%d bytes\n", size, host->xfer_size);

	return data->error;
}

static void mt6573_sd_setup_dma(struct mt6573_sd_host *host,
		struct scatterlist *sg)
{
	u32 base = host->base;
	u32 left_size;
	u32 xfer_size;
	u32 dma_addr = host->dma_addr;
	u32 dma_burst, sdc_burst, fifo_thd = 1;

	MSG(DMA, "DMA_SET: Addr:0x%.8x Size:%d bytes\n",
			host->dma_addr, host->dma_left_size);

	left_size = host->xfer_size - host->data->bytes_xfered;
	xfer_size = left_size > host->dma_left_size ? host->dma_left_size : left_size;

	BUG_ON(!xfer_size);

	if (xfer_size > DMA_MAX_COUNT)
		xfer_size = DMA_MAX_COUNT;

	/* FIXME { */
	dma_burst = DMA_CON_BURST_SINGLE;
	sdc_burst = MSDC_DMA_BURST_1BEAT;

	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_DMABRUST, sdc_burst);
	msdc_set_fifo_thd(fifo_thd);
	/* FIXME } */
	msdc_dma_config(&host->dma, dma_burst, dma_addr, xfer_size);

	host->dma_xfer_size = xfer_size;
}

static void mt6573_sd_prepare_dma(struct mt6573_sd_host *host,
		struct scatterlist *sg)
{
	host->dma_addr = sg_dma_address(sg);
	host->dma_left_size = sg_dma_len(sg);

	/* 1(RX): Mem <- Dev, 0(TX): Mem -> Dev */
	host->dma.dir = (host->dma_dir == DMA_FROM_DEVICE) ? 1 : 0;

	mt6573_sd_setup_dma(host, sg);
}

static void mt6573_sd_start_dma(struct mt6573_sd_host *host)
{
	msdc_dma_start(&host->dma);
}

static void mt6573_sd_stop_dma(struct mt6573_sd_host *host)
{
	msdc_dma_stop(&host->dma);
}

static void mt6573_sd_dma_callback(void *d)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host*)d;

	tasklet_hi_schedule(&host->dma_tasklet);
}

static void mt6573_sd_abort_data(struct mt6573_sd_host *host, struct mmc_data *data)
{
	u32 base = host->base;

	printk(KERN_ERR "[SD%d] %s: abort data. dma(%d)\n", host->id, __func__, host->dma_xfer);

	atomic_set(&host->abort, 1);

	if (data->stop)
		(void)mt6573_sd_send_command(host, data->stop, 1, CMD_TIMEOUT);

	if (host->dma_xfer) {
		MSG(DMA, "mt6573_sd_abort_data call dma tasklet now!!\n");
		tasklet_hi_schedule(&host->dma_tasklet);
	} else {
		MSG(CFG,"mt6573_sd_abort_data turn off fifo irq  now!!\n");
		msdc_fifo_irq_off();
		complete(&host->xfer_done);
	}
}

static void mt6573_sd_set_timeout(struct mt6573_sd_host *host, u32 ns, u32 clks)
{
	u32 base = host->base;
	u32 timeout, clk_ns;

	host->timeout_ns   = ns;
	host->timeout_clks = clks;

	clk_ns  = 1000000000UL / host->sclk;
	timeout = ns / clk_ns + clks;
	timeout = timeout >> 16; /* in 65536 sclk cycle unit */
	timeout = timeout > 1 ? timeout - 1 : 0;
	timeout = timeout > 255 ? 255 : timeout;

	sdr_set_field(SDC_CFG, SDC_CFG_DTOC, timeout);

	MSG(OPS, "Set read data timeout: %dns %dclks -> %d x 65536 cycles\n",
			ns, clks, timeout + 1);
}

static __tcmfunc irqreturn_t mt6573_sd_irq(int irq, void *dev_id)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)dev_id;
	struct mmc_data       *data = host->data;
	struct mmc_command    *cmd  = host->cmd;
	int cmd_done = 0;
	u32 base     = host->base;
	u32 intsts   = sdr_read32(MSDC_INT);

	if (intsts & MSDC_INT_DIRQ && data != NULL)
		tasklet_hi_schedule(&host->fifo_tasklet);

	if (intsts & MSDC_INT_SDIOIRQ)
		mmc_signal_sdio_irq(host->mmc);

	if (intsts & MSDC_INT_SDMCIRQ)
		printk(KERN_INFO "\n[SD%d] %s: MCIRQ: SDC_CSTA=0x%.8x\n",
				host->id, __func__, sdr_read32(SDC_CSTA));

	if (intsts & MSDC_INT_SDCMDIRQ && cmd != NULL) {
		u32 status = sdr_read32(SDC_CMDSTA);
		if (status & SDC_CMDSTA_CMDRDY) {
			u32 *rsp = &cmd->resp[0];
			cmd->error = 0;
			if (unlikely(host->cmd_rsp == RESP_R2)) {
				*rsp++ = sdr_read32(SDC_RESP3);
				*rsp++ = sdr_read32(SDC_RESP2);
				*rsp++ = sdr_read32(SDC_RESP1);
				*rsp++ = sdr_read32(SDC_RESP0);
			} else {
				*rsp = sdr_read32(SDC_RESP0); /* Resp: 1, 3, 4, 5, 6, 7(1b) */
			}
		} else if (status & SDC_CMDSTA_RSPCRCERR) {
			printk(KERN_ERR "[SD%d] %s: cmd resp crc error\n", host->id, __func__);
			cmd->error = (unsigned int)-EIO;
			msdc_reset();
		} else if (status & SDC_CMDSTA_CMDTO) {
			printk(KERN_ERR "[SD%d] %s: sd_irq cmd(%d) resp timeout and status is 0x%x\n", host->id, __func__,cmd->opcode,status);
			cmd->error = (unsigned int)-ETIMEDOUT;
			msdc_reset();
		}

		if (status) {
			host->cmd_rsp_done = 1;
			cmd_done = 1;
		}
	}

	if (intsts & MSDC_INT_SDR1BIRQ && cmd != NULL) {
		host->cmd_r1b_done = 1;
		cmd_done = 1;
	}

	if (cmd_done) {
		if (((host->cmd_rsp != RESP_R1B) && (host->cmd_rsp_done == 1)) ||
				((host->cmd_rsp == RESP_R1B) && (host->cmd_rsp_done == 1) &&
				 (host->cmd_r1b_done == 1))) {
			complete(&host->cmd_done);
			host->cmd = NULL;
			host->cmd_rsp_done = 0;
			host->cmd_r1b_done = 0;
		}
	}

	if (intsts & MSDC_INT_SDDATIRQ && data != NULL) {
		u32 status1 = 0x0;
		u32 status = 0x0;

		if(get_chip_eco_ver() == CHIP_E1){
			status1 = sdr_read32(BOOT_STA);
			status = sdr_read32(SDC_DATSTA);
		}
		else
			status = sdr_read32(SDC_DATSTA);

		if (status & SDC_DATSTA_DATTO){
			if ((host->id != 1)|| (can_sdio_irq_process_data())) {
				printk("[SD%d] %s: Data timeout is(0x%x)!!!\n",host->id, __func__,status);
				data->error = (unsigned int)-ETIMEDOUT;
			} else {
				MSG(INT, "SDIO data timeout after transfer finished, status=0x%x\n", status);
			}
		}

		if(get_chip_eco_ver() == CHIP_E1){
			if(!(data->flags & MMC_DATA_WRITE)){
				if (status & SDC_DATSTA_DATCRCERR)
					data->error = (unsigned int)-EIO;
			}
			else{
				if (status1 & BOOT_STA_CRCERR)
					data->error = (unsigned int)-EIO;
			}
		}
		else{
			if (status & SDC_DATSTA_DATCRCERR)
				data->error = (unsigned int)-EIO;
		}

		if (data->error) {
			if(get_chip_eco_ver() == CHIP_E1){
				if(data->flags & MMC_DATA_WRITE){
					printk(KERN_ERR "\n[SD%d] %s: SDDATIRQ: data error(%d) flags(0x%x), Write CRC(0x%x)\n",
							host->id, __func__, data->error, data->flags, status1 & BOOT_STA_CRCERR);
				}
				else {
					printk(KERN_ERR "\n[SD%d] %s: SDDATIRQ: data error(%d) flags(0x%x), Read CRC(0x%x)\n",
							host->id, __func__, data->error, data->flags, status & SDC_DATSTA_DATCRCERR);
				}
			}
			else{
				printk(KERN_ERR "\n[SD%d] %s: SDDATIRQ: data error(%d) flags(0x%x),CRC(0x%x)\n",
						host->id, __func__, data->error, data->flags, status & SDC_DATSTA_DATCRCERR);
			}

			mt6573_sd_abort_data(host, data);
		}
	}

#ifdef MT6573_SD_DEBUG
	{
		msdc_int_reg *int_reg = (msdc_int_reg*)&intsts;
		MSG(INT, "IRQ(%d) INTSTS(0x%.8x): SDIO(%d) R1B(%d), DAT(%d), CMD(%d), PIN(%d), DIRQ(%d)\n",
				irq,
				intsts,
				int_reg->sdioirq,
				int_reg->sdr1b,
				int_reg->sddatirq,
				int_reg->sdcmdirq,
				int_reg->pinirq,
				int_reg->dirq);
	}
#endif
	return IRQ_HANDLED;
}

static __tcmfunc irqreturn_t mt6573_sd_cd_irq(int irq, void *dev_id)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)dev_id;

	tasklet_hi_schedule(&host->card_tasklet);

#ifdef MT6573_SD_DEBUG
	{
		u32 base = host->base;
		u32 intsts = sdr_read32(MSDC_INT);
		msdc_int_reg *int_reg = (msdc_int_reg*)&intsts;
		MSG(INT, "IRQ(%d) INTSTS(0x%.8x): SDIO(%d) R1B(%d), DAT(%d), CMD(%d), PIN(%d), DIRQ(%d)\n",
				irq,
				intsts,
				int_reg->sdioirq,
				int_reg->sdr1b,
				int_reg->sddatirq,
				int_reg->sdcmdirq,
				int_reg->pinirq,
				int_reg->dirq);
	}
#endif

	return IRQ_HANDLED;
}

static void mt6573_sd_sdio_eirq(void *data)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)data;

	MSG(INT, "SDIO EINT\n");

	mmc_signal_sdio_irq(host->mmc);
}

static void mt6573_sd_cd_eirq(void *data)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)data;

	MSG(INT, "CD EINT\n");

	tasklet_hi_schedule(&host->card_tasklet);
}

static void mt6573_sd_init_dma(struct mt6573_sd_host *host, struct resource *res)
{
	int dma_channel[] = {P_DMA_MSDC_1, P_DMA_MSDC_2, P_DMA_MSDC_3, P_DMA_MSDC_4};
	int ret;

	ret = mt65xx_req_peri_dma(dma_channel[host->id], mt6573_sd_dma_callback,
			(void *)host);

	BUG_ON(ret);

	msdc_dma_init(&host->dma, host->id, res);
}

static void mt6573_sd_deinit_dma(struct mt6573_sd_host *host)
{
	msdc_dma_stop(&host->dma);
	msdc_dma_deinit(&host->dma);
}

static void mt6573_sd_tasklet_card(unsigned long arg)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)arg;
	struct mt6573_sd_host_hw *hw = host->hw;
	u32 base = host->base;
	u32 inserted;
	u32 status = 0;
	u32 change = 0;

	spin_lock(&host->lock);

	if (hw->get_cd_status) {
		inserted = hw->get_cd_status();
	} else {
		status = sdr_read32(MSDC_PS);
		inserted = status & MSDC_PS_PIN0 ? 0 : 1;
	}

	change = host->card_inserted ^ inserted;
	host->card_inserted = inserted;

	if (change && !host->suspend){
		Is_In_ReTry = MAX_RETRY_TIME;
		mmc_detect_change(host->mmc, msecs_to_jiffies(20));
	}
	spin_unlock(&host->lock);

	printk("[SD%d] %s: PS(0x%.8x), inserted(%d), change(%d)\n",
			host->id, __func__, status, host->card_inserted, change);
}

static void mt6573_sd_tasklet_fifo(unsigned long arg)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)arg;
	struct mmc_data *data = host->data;
	u32 base = host->base;

	if (!data) return;

	spin_lock(&host->lock);

	/* disable DREQ interrupt to avoid disturb pio ops. */
	msdc_fifo_irq_off();

	if (data->flags & MMC_DATA_READ) {
		(void)mt6573_sd_pio_read(host, data);
	} else {
		(void)mt6573_sd_pio_write(host, data);
	}
	complete(&host->xfer_done);
	disable_sdio_irq_process_data();

	spin_unlock(&host->lock);
}

static void mt6573_sd_tasklet_dma(unsigned long arg)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host*)arg;
	struct mmc_data *data = host->data;
	u32 base = host->base;

	if (!data || data->bytes_xfered == host->xfer_size) {
		WARN_ON(1); /* FIXME */
		return;
	}

	spin_lock(&host->lock);
	mt6573_sd_stop_dma(host);

	data->bytes_xfered += host->dma_xfer_size;

	if (data->error || data->bytes_xfered == host->xfer_size) {
		MSG(CFG,"DMA Transfer finished!!\n");
		msdc_dma_off();
		complete(&host->xfer_done);
		disable_sdio_irq_process_data();
		spin_unlock(&host->lock);
		return;
	}

	host->dma_addr += host->dma_xfer_size;

	if (host->dma_addr == (sg_dma_address(host->cur_sg) + sg_dma_len(host->cur_sg))) {
		host->cur_sg++;
		host->num_sg--;
		host->dma_addr = sg_dma_address(host->cur_sg);
		host->dma_left_size = sg_dma_len(host->cur_sg);
	} else {
		host->dma_left_size -= host->dma_xfer_size;
	}

	mt6573_sd_setup_dma(host, host->cur_sg);
	mt6573_sd_start_dma(host);
	spin_unlock(&host->lock);
}

static void mt6573_sd_card_clock(struct mt6573_sd_host *host, int on)
{
	u32 base = host->base;

	MSG(CFG, "Turn %s %s clock (clkon: %d -> %d)\n",
			on ? "on" : "off", "card", host->card_clkon, on);

	if (on && !host->card_clkon) {
		host->card_clkon = 1;
		sdc_sclk_on();
		mdelay(1);
	} else if (!on && host->card_clkon) {
		host->card_clkon = 0;
		sdc_sclk_off();
	}
}

static void mt6573_sd_host_clock(struct mt6573_sd_host *host, int clksrc, int on)
{
	u32 base = host->base;
	//u32 clksrc_div;
	u32 clkctl[] = {MSDC_CCTL, MSDC2_CCTL, MSDC3_CCTL, MSDC4_CCTL};

	MSG(CFG, "Turn %s %s clock (clkon: %d -> %d, clksrc: %d)\n",
			on ? "on" : "off", "host", host->core_clkon, on, clksrc);

	if (on && !host->core_clkon) {
		host->core_clkon = 1;

		/* select 3G PLL 492MHz */
		sdr_set_field(PLL_CON5_REG, 0xf, 0x1);
		/* clock phase ctrl */
		sdr_set_field(clkctl[host->id], 0x7 << 8, clksrc);
		sdr_set_field(MSDC_CFG, MSDC_CFG_CLKSRC, clksrc);

		if (host->id == 0x1) { /* Jocelyn: */
			sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_LAT, 0);
			/* jocelyn:
			    HS &&  fbclk shorter than data: 1(falling),
			    HS &&  fbclk longer than data: 0 (rising),
			    nonHS-RED:1 (falling)
			    */
			sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_RED, 1);
			MSG(CFG, "set CLKACB_CFG_CLK_LAT(0) CLKACB_CFG_CLK_RED(1) \n");
		}

		/* FIXME. MSDC_CLK_EN is not set */
		//sdr_write32(0xF702E240, 0x4ED);
		//sdr_write32(0xF702E240, 0x4ED|sdr_read32(0xF702E240));
		(void)hwEnableClock(clock_id[host->id],"SD");
	} else if (!on && host->core_clkon) {
		host->core_clkon = 0;
		(void)hwDisableClock(clock_id[host->id],"SD");
	}
}

static void mt6573_sd_config_clock(struct mt6573_sd_host *host, u32 hz)
{
	struct mt6573_sd_host_hw *hw = host->hw;
	u32 base = host->base;
	u32 flags;
	u32 cred = hw->cmd_edge;
	u32 dred = hw->data_edge;
	u32 sclk_div;
	u32 sclk;
	u32 hclk = host->hclk;
	u8  clksrc = hw->clk_src;

	if (!hz) {
		msdc_reset();
		mt6573_sd_card_clock(host, 0);
		return;
	}

	msdc_lock_irqsave(flags);
	mt6573_sd_card_clock(host, 0);
	mt6573_sd_host_clock(host, clksrc, 0);

	if (hz >= (hclk >> 1)) {
		sclk_div = 0;         /* mean div = 1/2 */
		sclk     = hclk >> 1; /* sclk = hclk / 2 */
	} else {
		sclk_div = (hclk + ((hz << 2) - 1)) / (hz << 2);
		sclk     = (hclk >> 2) / sclk_div;
	}
	if(sclk > HOST_INI_SCLK && host->sclk <= HOST_INI_SCLK){
		sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDSEL, 1);
		printk("sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDSEL, 1)\n");
	}
	if(sclk <= HOST_INI_SCLK && host->sclk > HOST_INI_SCLK){
		sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDSEL, 0);
		printk("sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDSEL, 0)\n");
	}

	host->sclk  = sclk;
	host->hclk  = hclk;
	hw->clk_src = clksrc;

	sdr_set_field(MSDC_CFG, MSDC_CFG_SCLKF, sclk_div);
	sdr_set_field(MSDC_CFG, MSDC_CFG_RED, dred);
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_CMDRE, cred);
	mt6573_sd_host_clock(host, clksrc, 1);
	mt6573_sd_card_clock(host, 1);
	mt6573_sd_set_timeout(host, host->timeout_ns, host->timeout_clks);

	msdc_unlock_irqrestore(flags);

	//MSG(CFG, "SET_CLK(%d): SCLK(%dkHz) DIV(%d) CRED(%d) DRED(%d)\n",
	printk(KERN_INFO "[SD%d] %s: SET_CLK(%d): SCLK(%dkHz) DIV(%d) CRED(%d) DRED(%d)\n",
			host->id, __func__, clksrc, sclk/1000, sclk_div, cred, dred);
}

static void mt6573_sd_config_bus(struct mt6573_sd_host *host, u32 width)
{
	u32 base = host->base;
	u32 flags;

	msdc_lock_irqsave(flags);

	switch (width) {
	default:
	case 1:
		width = 1;
		sdr_clr_bits(SDC_CFG, SDC_CFG_MDLEN | SDC_CFG_MDLEN8);
		break;
	case 4:
		sdr_set_bits(SDC_CFG, SDC_CFG_MDLEN);
		sdr_clr_bits(SDC_CFG, SDC_CFG_MDLEN8);
		break;
	case 8:
		sdr_set_bits(SDC_CFG, SDC_CFG_MDLEN | SDC_CFG_MDLEN8);
		break;
	}

	msdc_unlock_irqrestore(flags);

	MSG(CFG, "Bus Width = %d\n", width);
}

static void mt6573_sd_config_pin(struct mt6573_sd_host *host, int mode)
{
	struct mt6573_sd_host_hw *hw = host->hw;
	u32 base = host->base;
	int pull = (mode == PIN_PULL_UP) ? GPIO_PULL_UP : GPIO_PULL_DOWN;

	/* Config WP pin */
	if (hw->flags & MSDC_WP_PIN_EN) {
		if (hw->config_gpio_pin)
			hw->config_gpio_pin(MSDC_WP_PIN, pull);
		/* TODO */
		sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_RST_WP, hw->rst_wp_pull_res);
		sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_RST_WP, mode);
	}

	if (hw->config_gpio_pin) {
		hw->config_gpio_pin(MSDC_CMD_PIN, pull);
		hw->config_gpio_pin(MSDC_DAT_PIN, pull);
	}

	/* TODO */
	sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_CLK, hw->clk_pull_res);
	sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_CMD, hw->cmd_pull_res);
	sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_DAT, hw->dat_pull_res);
	sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_CLK, mode);/* Config CMD pin */
	sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_CMD, mode);/* Config CMD pin */
	sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_DAT, mode);/* Config DATA pins */

	MSG(CFG, "Pins mode(%d), down(%d), up(%d)\n",
			mode, PIN_PULL_DOWN, PIN_PULL_UP);
}

static void mt6573_sd0_card_power(struct mt6573_sd_host *host, int on)
{
	MSG(CFG, "Turn %s %s power (copower: %d -> %d)\n",
			on ? "on" : "off", "core", host->core_power, on);

	if (on && host->core_power == 0) {
		msdc_core_power_on(host);
		host->core_power = 1;
		udelay(1000);
	} else if (!on && host->core_power == 1) {
		msdc_core_power_off(host);
		host->core_power = 0;
		udelay(1000);
	}
}

static void mt6573_sd_host_power(struct mt6573_sd_host *host, int on)
{
	MSG(CFG, "Turn %s %s power \n", on ? "on" : "off", "host");

	if (on) {
		mt6573_sd_host_clock(host, host->hw->clk_src, 1);
	} else {
		mt6573_sd_host_clock(host, host->hw->clk_src, 0);
	}
}

static void mt6573_sd_card_power(struct mt6573_sd_host *host, int on)
{
	u32 base = host->base;

	MSG(CFG, "Turn %s %s power \n", on ? "on" : "off", "card");

	if (on) {
		if(host->id == 0x1){
			mt_set_gpio_mode(GPIO62,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO63,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO64,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO65,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO66,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO67,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO68,GPIO_MODE_01);
		}
#if 0
		else if(host->id == 0x0){
			mt_set_gpio_mode(GPIO71,GPIO_MODE_02);
			mt_set_gpio_mode(GPIO72,GPIO_MODE_02);
			mt_set_gpio_mode(GPIO73,GPIO_MODE_02);
			mt_set_gpio_mode(GPIO74,GPIO_MODE_02);
			printk("SD0 to 8 bit emmc\n");
		}
#endif
		mt6573_sd_config_pin(host, PIN_PULL_UP);
		if (host->hw->ext_power_on) {
			host->hw->ext_power_on();
		} else {
            if(host->id == 0)
                mt6573_sd0_card_power(host, 1);
		}
		mdelay(1);
	} else {
		if (host->hw->ext_power_off) {
			host->hw->ext_power_off();
		} else {
            if(host->id == 0)
                mt6573_sd0_card_power(host, 0);
		}
		mt6573_sd_config_pin(host, PIN_PULL_DOWN);
		if(host->id == 0x1){
			mt_set_gpio_mode(GPIO62,GPIO_MODE_00);
			mt_set_gpio_mode(GPIO63,GPIO_MODE_00);
			mt_set_gpio_mode(GPIO64,GPIO_MODE_00);
			mt_set_gpio_mode(GPIO65,GPIO_MODE_00);
			mt_set_gpio_mode(GPIO66,GPIO_MODE_00);
			mt_set_gpio_mode(GPIO67,GPIO_MODE_00);
			mt_set_gpio_mode(GPIO68,GPIO_MODE_00);
		}
#if 0
		else if(host->id == 0x0){
			mt_set_gpio_mode(GPIO71,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO72,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO73,GPIO_MODE_01);
			mt_set_gpio_mode(GPIO74,GPIO_MODE_01);
			printk("SD0 to 4 bit emmc\n");
		}
#endif
		mdelay(1);
	}
}

static void mt6573_sd_set_power(struct mt6573_sd_host *host, u8 mode)
{
	MSG(CFG, "Set power mode(%d)\n", mode);

	if (host->power_mode == MMC_POWER_OFF && mode != MMC_POWER_OFF) {
		mt6573_sd_host_power(host, 1);
		mt6573_sd_card_power(host, 1);
	} else if (host->power_mode != MMC_POWER_OFF && mode == MMC_POWER_OFF) {
		mt6573_sd_card_power(host, 0);
		mt6573_sd_host_power(host, 0);
	}
	host->power_mode = mode;
}

static void mt6573_sd_init_hw(struct mt6573_sd_host *host, struct resource *dma)
{
	struct mt6573_sd_host_hw *hw = host->hw;
	u32 base = host->base;

#ifdef MT6573_SD_DEBUG
	mt6573_sd_reg[host->id] = (struct mt6573_sd_regs *)host->base;
	msdc_dma_reg[host->id] =
		(struct msdc_dma_regs *)(AP_DMA_BASE + 0x180 + 0x80 * host->id);
#endif

	/* Power on */
	mt6573_sd_card_power(host, 1);
	printk("[SD%d] %s: PMU VMC register value is :0x%x \n",host->id, __func__,sdr_read16(0xF702F7C0));

	mt6573_sd_card_power(host, 0);
	printk("[SD%d] %s: PMU VMC register value is :0x%x \n",host->id, __func__,sdr_read16(0xF702F7C0));

	mt6573_sd_card_power(host, 1);
	printk("[SD%d] %s: PMU VMC register value is :0x%x \n",host->id, __func__,sdr_read16(0xF702F7C0));

	/* Reset */
	msdc_reset();

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
	if(get_chip_eco_ver() == CHIP_E1)
		sdr_read16(BOOT_STA);

	/* Mask command interrups since we use pio mode for command req/rsp. */
	sdr_set_bits(SDC_IRQMASK0,
			SDC_IRQMASK0_CMDRDY|SDC_IRQMASK0_CMDTO|SDC_IRQMASK0_RSPCRCERR);

	/* Mask data done interrupt to reduce interrupt latency */
	sdr_set_bits(SDC_IRQMASK0, SDC_IRQMASK0_BLKDONE);

	/* Mask csta interrupt since it'll be handled by linux mmc subsystem (CHECKME) */
	sdr_set_bits(SDC_IRQMASK1, 0xFFFFFFFF);

	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_ODCCFG0, hw->cmd_odc);
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_ODCCFG1, hw->data_odc);
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_SRCFG0, hw->cmd_slew_rate);
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_SRCFG1, hw->data_slew_rate);
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_DSW, MSDC_DSW_NODELAY);

	/* CHECKME. It's MUST. Otherwise, data crc occurs */
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_SAMPON, 0x1);

#if 1 /*Jocelyn: suggest set this field to 0 by default */
	sdr_set_field(MSDC_IOCON0, MSDC_IOCON0_DMABRUST, 0);
#endif

	sdr_set_bits(MSDC_STA, MSDC_STA_FIFOCLR);

	sdr_set_field(SDC_CFG, SDC_CFG_DTOC, DEFAULT_DTOC);
	sdr_set_field(SDC_CFG, SDC_CFG_WDOD, DEFAULT_WDOD);
	sdr_set_field(SDC_CFG, SDC_CFG_BSYDLY, DEFAULT_BSYDLY);

	if (host->id == 0x0) { /* Jocelyn: */
		sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_LAT, 0);
		/* jocelyn:
		    HS &&  fbclk shorter than data: 1(falling),
		    HS &&  fbclk longer than data: 0 (rising),
		    nonHS-RED:1 (falling)
		    */
		sdr_set_field(CLKACB_CFG, CLKACB_CFG_CLK_RED, 1);
		MSG(CFG, "set CLKACB_CFG_CLK_LAT(0) CLKACB_CFG_CLK_RED(1) \n");
	}

	mt6573_sd_config_bus(host, 1);
	mt6573_sd_init_dma(host, dma);

	MSG(FUC, "init hardware done!\n");
}

static void mt6573_sd_deinit_hw(struct mt6573_sd_host *host)
{
	u32 base = host->base;

	/* Disable all interrupts */
	sdr_clr_bits(MSDC_CFG, MSDC_CFG_RCDEN|MSDC_CFG_DIRQE|MSDC_CFG_PINEN|
			MSDC_CFG_DMAEN|MSDC_CFG_INTEN);

	/* Disable card detection */
	mt6573_sd_enable_cd_irq(host->mmc, 0);
	mt6573_sd_deinit_dma(host);
	mt6573_sd_set_power(host, MMC_POWER_OFF);   /* make sure power down */
}

static void mt6573_sd_pm_change(pm_message_t state, void *data)
{
	struct mt6573_sd_host *host = (struct mt6573_sd_host *)data;
	int evt = state.event;

	if (evt == PM_EVENT_SUSPEND || evt == PM_EVENT_USER_SUSPEND) {

		if (host->suspend)
			return;

		if (evt == PM_EVENT_SUSPEND && host->power_mode == MMC_POWER_OFF)
			return;

		if(host->hw->flags & MSDC_SYS_SUSPEND)
			(void)mmc_suspend_host(host->mmc);
		else {
			host->mmc->pm_flags |= MMC_PM_IGNORE_PM_NOTIFY;
			mmc_remove_host(host->mmc);
		}
		host->suspend = 1;
		host->pm_state = state;

		printk(KERN_INFO "[SD%d] %s: %s Suspend\n", host->id, __func__,
				evt == PM_EVENT_SUSPEND ? "PM" : "USR");

	} else if (evt == PM_EVENT_RESUME || evt == PM_EVENT_USER_RESUME) {

		if (!host->suspend)
			return;

		/* don't resume from system when it's suspended by user */
		if (evt == PM_EVENT_RESUME && host->pm_state.event == PM_EVENT_USER_SUSPEND) {
			printk(KERN_INFO "[SD%d] %s: No PM Resume(USR Suspend)\n", host->id, __func__);
			return;
		}
		printk(KERN_INFO "[SD%d] %s: %s Resume\n", host->id, __func__,
				evt == PM_EVENT_RESUME ? "PM" : "USR");

		if(host->hw->flags & MSDC_SYS_SUSPEND)
			(void)mmc_resume_host(host->mmc);
		else {
			host->mmc->pm_flags |= MMC_PM_IGNORE_PM_NOTIFY;
			mmc_add_host(host->mmc);
		}
		host->suspend = 0;
		host->pm_state = state;
	}
}

static int sg_panic_dump = 0;
static void mt6573_sd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct mt6573_sd_host *host = mmc_priv(mmc);
	struct mmc_command    *cmd;
	struct mmc_data       *data;
	u32 base = host->base;
	int  dma           = 0;
	int  read          = 1;
	int  polling       = 0;
	int  send_type     = 0;/* if send_type is true, then mmc_data is NULL */
	int  ret           = 0;

	if (!sg_panic_dump){
		polling = 0;
	} else {
		polling = 1;
	}

	spin_lock(&host->lock);

	cmd  = mrq->cmd;
	data = mrq->cmd->data;

	if (!is_card_present(host) || host->power_mode == MMC_POWER_OFF) {
		MSG(WRN, "REQ fail. no present/power off\n");
		cmd->error = (unsigned int)-ENOMEDIUM;
		mrq->done(mrq);
		spin_unlock(&host->lock);
		return;
	}

	if ((host->id ==0) || (host->id == 1)) {
		MSG(OPS, "Enable SD host/card working ability!\n");
		clr_device_working_ability(clock_id[host->id], SLOW_IDLE_STATE);
		clr_device_working_ability(clock_id[host->id], DEEP_IDLE_STATE);
	}

	if (data != NULL) {
		BUG_ON(data->blksz > HOST_MAX_BLKSZ);
		MSG(OPS, "PRE_DAT: blksz(%d), blks(%d), size(%d)\n",
				data->blksz, data->blocks, data->blocks * data->blksz);

#ifdef CFG_PROFILING
		if (sd_perf_rpt[host->id]) {
			xgpt_timer_init(host->id + XGPT5);
			xgpt_timer_start(host->id + XGPT5);
		}
#endif

		host->data = data;
		host->xfer_size = data->blocks * data->blksz;
		host->cur_sg = data->sg;
		host->num_sg = data->sg_len;

		/* NOTE:
		 * For performance consideration, may use pio mode for smaller size
		 * transferring and use dma mode for larger size one.
		 */
		if (dma_is_forced(host->id)){
			host->dma_xfer = dma = get_forced_transfer_mode(host->id);
			printk("Force the transfer in %s mode \r\n", (dma? "DMA" : "PIO"));
		} else {
			if(host->xfer_size >= 512) {
				if (!sg_panic_dump) {
					host->dma_xfer = dma = 1;
				}
			}
			MSG(CFG, "Do transfer in %s mode \r\n", (dma? "DMA" : "PIO"));
		}

		if (cmd->opcode == MMC_READ_SINGLE_BLOCK ||
				cmd->opcode == MMC_WRITE_BLOCK ||
				cmd->opcode == MMC_READ_MULTIPLE_BLOCK ||
				cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) {
			host->error_address = cmd->arg;
		}else{
			if(cmd->opcode != MMC_STOP_TRANSMISSION)
				host->error_address = 0;
		}

		init_completion(&host->xfer_done);
		read = data->flags & MMC_DATA_READ ? 1 : 0;

		if (read) {
			polling = 1;
			if ((host->timeout_ns != data->timeout_ns) ||
					(host->timeout_clks != data->timeout_clks)) {
				mt6573_sd_set_timeout(host, data->timeout_ns, data->timeout_clks);
			}
		}

		msdc_set_bksz(data->blksz);
		msdc_clr_fifo();

		/* Jocelyn: read data content may error */
		msdc_reset();

		if (dma) {
			msdc_dma_on();
			msdc_fifo_irq_off();
			host->dma_dir = read ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
			(void)dma_map_sg(mmc_dev(mmc), data->sg, data->sg_len, host->dma_dir);
			mt6573_sd_prepare_dma(host, host->cur_sg);
			mt6573_sd_start_dma(host);
		} else {
			msdc_dma_off();
			if (!sg_panic_dump){
				msdc_fifo_irq_on();
			}
			if (read) {
				u32 xfersz = host->xfer_size;
				if ((xfersz >> 2) < MAX_FIFOTHD) {
					if((xfersz >> 2)== 0x0)
						msdc_set_fifo_thd(PWR_FIFOTHD);
					else
						msdc_set_fifo_thd(xfersz >> 2);
				} else {
					msdc_set_fifo_thd(PRD_FIFOTHD);
				}
			} else {
				msdc_set_fifo_thd(PWR_FIFOTHD);
			}
		}
	}
	else
		send_type = 1; /* There is no data needed to transfer */

	if(get_chip_eco_ver() == CHIP_E1){
		/*
		//keep SDC_DATSTA value is new
		printk("keep BOOT_STA value new and value is 0x%x\n",sdr_read32(BOOT_STA));
		//keep SDC_DATSTA value is new
		printk("keep SDC_DATSTA value new and value is 0x%x\n",sdr_read32(SDC_DATSTA));
		*/
		sdr_read32(BOOT_STA);
		sdr_read32(SDC_DATSTA);
	} else {
		/*
		//keep SDC_DATSTA value is new
		printk("keep SDC_DATSTA value new and value is 0x%x\n",sdr_read32(SDC_DATSTA));
		*/
		sdr_read32(SDC_DATSTA);
	}

	if (mt6573_sd_send_command(host, cmd, polling, CMD_TIMEOUT) != 0)
		goto done;

	if (data != NULL) {
		if (!sg_panic_dump){
			spin_unlock(&host->lock);
			if(!wait_for_completion_timeout(&host->xfer_done, 5*DAT_TIMEOUT)){
				printk(KERN_ERR "[SD%d] %s: data transfer timeout \n", host->id, __func__);
				data->error = (unsigned int)-ETIMEDOUT;
				if (data->stop)
					(void)mt6573_sd_send_command(host, data->stop, 1, CMD_TIMEOUT);
			}
			spin_lock(&host->lock);
		} else {
			/* pio, not to use interrupt to trigger data recv and send */
			tasklet_hi_schedule(&host->fifo_tasklet);

			spin_unlock(&host->lock);
			if (!wait_for_completion_timeout(&host->xfer_done, 5*DAT_TIMEOUT)) {
				printk(KERN_ERR "[SD%d] %s: data transfer timeout \n", host->id, __func__);
				data->error = (unsigned int)-ETIMEDOUT;
				if (data->stop)
					(void)mt6573_sd_send_command(host, data->stop, 1, CMD_TIMEOUT);
			}
			spin_lock(&host->lock);
		}

#if 0
		/* Jocelyn: only applied to E1 ,use interrupt and not need this code*/
		do {
			u32 u4_boot_sta, u4_sdc_datsta;
			if (data->flags & MMC_DATA_WRITE) {
				u4_boot_sta = sdr_read32(BOOT_STA);
				if (u4_boot_sta & BOOT_STA_CRCERR) {
					data->error = (unsigned int)-EIO;
					/* SDC_DATSTA read clear */
					u4_sdc_datsta = sdr_read32(SDC_DATSTA);
					printk(KERN_ERR "\n[SD%d] %s: BOOT_STA: data error(%d) flags(0x%x), BOOT_STA(0x%x), SDC_DATSTA(0x%x),CMD_NUM(%d)\n",
							host->id, __func__, data->error, data->flags, u4_boot_sta, u4_sdc_datsta,cmd->opcode);
				}
			}
		} while (0);
#endif
		if (!data->error) {
			/* make sure contents in fifo flushed to device after a dma write */
			if (!read) {
				int count;
				do {
					count = (sdr_read32(MSDC_STA) >> 4) & 0xf;
					if (count)
						printk(KERN_WARNING "[SD%d] %s: FIFOCNT: %d\n", host->id, __func__, count);
				} while(0 != count);
			}
			if (data->stop)
				(void)mt6573_sd_send_command(host, data->stop, 0, CMD_TIMEOUT);
		}
		if (cmd->opcode == SD_IO_RW_EXTENDED) {
			if (cmd->arg & 0x08000000) {
				/* SDIO workaround for CMD53 multiple block transfer */
				if (data->blocks > 1) {
					struct mmc_command abort;
					memset(&abort, 0, sizeof(struct mmc_command));
					abort.opcode = SD_IO_RW_DIRECT;
					abort.arg    = 0x80000000;            /* write */
					abort.arg   |= 0 << 28;               /* function 0 */
					abort.arg   |= SDIO_CCCR_ABORT << 9;  /* address */
					abort.arg   |= 0;                     /* abort function 0 */
					abort.flags  = (unsigned int)-1;
					(void)mt6573_sd_send_command(host, &abort, 1, CMD_TIMEOUT);
				}
			} else {
				/* SDIO workaround for CMD53 multiple byte transfer, which
				 * is not 4 byte-alignment
				 */
				if (data->blksz % 4) {
					/* The delay is required and tunable. The delay time must
					 * be not too small. Currently, it is tuned to 25us.(CHECKME)
					 */
					udelay(25);
					msdc_reset();
				}
			}
			enable_sdio_irq_process_data();
		}
	}
done:
#ifdef CFG_PROFILING
	if (data && (data->error == 0) && sd_perf_rpt[host->id]) {
		struct mmc_op_report *rpt;
		unsigned int counts = xgpt_timer_get_count(host->id + XGPT5);
		xgpt_timer_stop_clear(host->id + XGPT5);

		if (data->stop) {
			if (read)
				rpt = &sd_perf[host->id].multi_blks_read;
			else
				rpt = &sd_perf[host->id].multi_blks_write;
		} else {
			if (read)
				rpt = &sd_perf[host->id].single_blk_read;
			else
				rpt = &sd_perf[host->id].single_blk_write;
		}
		rpt->count++;
		rpt->total_size += host->xfer_size;
		rpt->total_time += counts;

		if (counts < rpt->min_time || rpt->min_time == 0)
			rpt->min_time = counts;
		if (counts > rpt->max_time || rpt->max_time == 0)
			rpt->max_time = counts;

#ifdef CFG_PROFILING_DUMP
		printk(KERN_INFO "[SD%d] %s: %s(%.8x): %6d bytes, %6d counts, %6d us, %5d KB/s\n",
				host->id, __func__, read ? "READ " : "WRITE",
				cmd->arg, host->xfer_size,
				counts, counts * 30 + counts * 16960 / 32768,
				host->xfer_size * 32 / (counts ? counts : 1));
#endif
	}
#endif

	host->data = NULL;

	if (dma != 0 && data != NULL) {
		host->dma_xfer = 0;
		dma_force[host->id] = FORCE_NOTHING;
		mt6573_sd_stop_dma(host);
		dma_unmap_sg(mmc_dev(mmc), data->sg, data->sg_len, host->dma_dir);
	}
	if (atomic_read(&host->abort)) {
		msdc_clr_fifo();
		atomic_set(&host->abort, 0);
	}

	/* if error occurred, do some retry job.
	 * NOTE: Current retry is in the environment of spin_lock!
	 * It is forbidden that add spin_lock again!
	 */
	if ((data != NULL)&&(data->error)) {
		if ((host->id == 0x0) && (Is_In_ReTry>0))
		{
			ret = mt6573_sd_retry(host, host->error_address);
			printk(KERN_ERR "[SD%d] %s: mt6573 retry %s.\n", host->id, __func__,
					(SD_TRUE == ret)?("successfully"):("failed"));
		}
	}

	/* Do some jobs after cmd sent or data transferred. */
	if ((host->id ==0) || (host->id == 1)) {
		if (send_type) {
			MSG(OPS,"This request has not data transferred!\n");

			if(cmd->opcode == MMC_SEND_STATUS){
				if((cmd->resp[0] & CARD_READY_FOR_DATA) ||(CARD_CURRENT_STATE(cmd->resp[0]) != 7)){
					MSG(OPS,"Disable SD host/card working ability when card left prg state by the RSP13!\n");
					set_device_working_ability(clock_id[host->id], SLOW_IDLE_STATE);
					set_device_working_ability(clock_id[host->id], DEEP_IDLE_STATE);
				}
			} else {
				MSG(OPS,"Disable SD host/card working ability when the request is not CMD13 !\n");
				set_device_working_ability(clock_id[host->id], SLOW_IDLE_STATE);
				set_device_working_ability(clock_id[host->id], DEEP_IDLE_STATE);
			}
		} else {
			MSG(OPS,"This request is for %s operation.\n", read?("read"):("write"));

			if(read){
				MSG(OPS,"Disable SD host/card working ability after CMD(%d) read completed!\n", cmd->opcode);
				set_device_working_ability(clock_id[host->id], SLOW_IDLE_STATE);
				set_device_working_ability(clock_id[host->id], DEEP_IDLE_STATE);
			}
		}
	}

	spin_unlock(&host->lock);
	mmc_request_done(mmc, mrq);
	return;
}

static void mt6573_sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct mt6573_sd_host *host = mmc_priv(mmc);
#ifdef MT6573_SD_DEBUG
	static char *vdd[] = {
		"1.50v", "1.55v", "1.60v", "1.65v", "1.70v", "1.80v", "1.90v",
		"2.00v", "2.10v", "2.20v", "2.30v", "2.40v", "2.50v", "2.60v",
		"2.70v", "2.80v", "2.90v", "3.00v", "3.10v", "3.20v", "3.30v",
		"3.40v", "3.50v", "3.60v"
	};
	static char *power_mode[] = {
		"OFF", "UP", "ON"
	};
	static char *bus_mode[] = {
		"UNKNOWN", "OPENDRAIN", "PUSHPULL"
	};
	static char *timing[] = {
		"LEGACY", "MMC_HS", "SD_HS"
	};

	MSG(CFG, "SET_IOS: CLK(%dkHz), BUS(%s), BW(%u), PWR(%s), VDD(%s), TIMING(%s)\n",
			ios->clock / 1000, bus_mode[ios->bus_mode],
			(ios->bus_width == MMC_BUS_WIDTH_4) ? 4 : 1,
			power_mode[ios->power_mode], vdd[ios->vdd], timing[ios->timing]);
#endif
	/* Bus width select */
	switch (ios->bus_width) {
	case MMC_BUS_WIDTH_1:
		mt6573_sd_config_bus(host, 1);
		break;
	case MMC_BUS_WIDTH_4:
		mt6573_sd_config_bus(host, 4);
		break;
	case MMC_BUS_WIDTH_8:
		mt6573_sd_config_bus(host, 8);
		break;
	default:
		break;
	}

	/* Power control */
	switch (ios->power_mode) {
	case MMC_POWER_OFF:
	case MMC_POWER_UP:
		mt6573_sd_set_power(host, ios->power_mode);
		break;
	case MMC_POWER_ON:
		host->power_mode = MMC_POWER_ON;
		break;
	default:
		break;
	}

	/* Clock control */
	if (host->mclk != ios->clock) {
		host->mclk = ios->clock;
		mt6573_sd_config_clock(host, ios->clock);
		mdelay(10);
	}
}

static int mt6573_sd_card_readonly(struct mmc_host *mmc)
{
	struct mt6573_sd_host *host = mmc_priv(mmc);
	u32 base = host->base;
	unsigned long flags;
	int ro = 0;

	if (host->hw->flags & MSDC_WP_PIN_EN) {
		spin_lock_irqsave(&host->lock, flags);
		ro = sdc_is_write_protect();
		spin_unlock_irqrestore(&host->lock, flags);
	}
	return ro;
}

static int mt6573_sd_card_inserted(struct mmc_host *mmc)
{
	struct mt6573_sd_host *host = mmc_priv(mmc);
	unsigned long flags;
	int present = 1;

	if (!(host->hw->flags & MSDC_REMOVABLE))
		return 1;

	if (host->hw->flags & MSDC_CD_PIN_EN) {
		spin_lock_irqsave(&host->lock, flags);
		present = host->card_inserted;
		spin_unlock_irqrestore(&host->lock, flags);
	} else {
		present = 0; /* TODO? Check DAT3 pins for card detection */
	}

	return present;
}

static void mt6573_sd_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct mt6573_sd_host *host = mmc_priv(mmc);
	u32 base = host->base;
	u32 tmp;

	if (host->hw->flags & MSDC_EXT_SDIO_IRQ) {
		if (enable)
			host->hw->enable_sdio_eirq();
		else
			host->hw->disable_sdio_eirq();
	} else {
		tmp = sdr_read32(SDIO_CFG);
		if (enable) {
			tmp |= SDIO_CFG_INTEN;
			sdr_set_bits(SDC_CFG, SDC_CFG_SDIO);
			sdr_write32(SDIO_CFG, tmp);
		} else {
			tmp &= ~SDIO_CFG_INTEN;
			sdr_write32(SDIO_CFG, tmp);
			sdr_clr_bits(SDC_CFG, SDC_CFG_SDIO);
		}
	}
}

static void mt6573_sd_enable_cd_irq(struct mmc_host *mmc, int enable)
{
	struct mt6573_sd_host *host = mmc_priv(mmc);
	struct mt6573_sd_host_hw *hw = host->hw;
	u32 base = host->base;

	if ((hw->flags & MSDC_CD_PIN_EN) == 0) {
		/* Pull down card detection pin since it is not avaiable */
		if (hw->config_gpio_pin)
			hw->config_gpio_pin(MSDC_CD_PIN, GPIO_PULL_DOWN);
		sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_INS, MSDC_PULL_RES_NONE);
		sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_INS, PIN_PULL_DOWN);
		sdr_clr_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);
		sdr_clr_bits(MSDC_CFG, MSDC_CFG_PINEN);
		return;
	}

	MSG(CFG, "CD IRQ Eanable(%d)\n", enable);

	if (enable) {
		if (hw->enable_cd_eirq) {
			hw->enable_cd_eirq();
		} else {
			/* card detection circuit relies on the core power so that the core power
			 * shouldn't be turned off. Here adds a reference count to keep
			 * the core power alive.
			 */
			// msdc_core_power_on(host);

			if (hw->config_gpio_pin)
				hw->config_gpio_pin(MSDC_CD_PIN, GPIO_PULL_UP);
			sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_INS, MSDC_PULL_RES_47K);
			sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_INS, PIN_PULL_UP);
			sdr_set_field(MSDC_PS, MSDC_PS_DEBOUNCE, DEFAULT_DEBOUNCE);
			sdr_set_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);
			sdr_set_bits(MSDC_CFG, MSDC_CFG_PINEN);
		}
	} else {
		if (hw->disable_cd_eirq) {
			hw->disable_cd_eirq();
		} else {
			if (hw->config_gpio_pin)
				hw->config_gpio_pin(MSDC_CD_PIN, GPIO_PULL_DOWN);
			sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRVAL_INS, MSDC_PULL_RES_NONE);
			sdr_set_field(MSDC_IOCON1, MSDC_IOCON1_PRCFG_INS, PIN_PULL_DOWN);
			sdr_clr_bits(MSDC_PS, MSDC_PS_PIEN0|MSDC_PS_POEN0|MSDC_PS_CDEN);
			sdr_clr_bits(MSDC_CFG, MSDC_CFG_PINEN);

			/* Here decreases a reference count to core power since card
			 * detection circuit is shutdown.
			 */
			// msdc_core_power_off(host);
		}
	}
}

extern int mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value);
extern int mmc_send_ext_csd(struct mmc_card *card, u8 *ext_csd);
#define DEBUG_MMC_IOCTL (0)
static int mt6573_sd_ioctl_single_rw(struct msdc_ioctl* msdc_ctl)
{
	char l_buf[512];
	struct scatterlist msdc_sg;
	struct mmc_data    msdc_data;
	struct mmc_command msdc_cmd;
	struct mmc_request msdc_mrq;
	struct mt6573_sd_host *host_ctl;

	host_ctl = msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);

	mmc_claim_host(host_ctl->mmc);

#if DEBUG_MMC_IOCTL
	printk("user want access %d partition\n",msdc_ctl->partition);
#endif
	switch (msdc_ctl->partition){
	case BOOT_PARTITION_1:
		mmc_send_ext_csd(host_ctl->mmc->card,l_buf);

		/* change to access boot partition 1 */
		l_buf[179] &= ~0x7;
		l_buf[179] |= 0x1;
		mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179]);
		break;
	case BOOT_PARTITION_2:
		mmc_send_ext_csd(host_ctl->mmc->card,l_buf);

		/* change to access boot partition 2 */
		l_buf[179] &= ~0x7;
		l_buf[179] |= 0x2;
		mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179]);
		break;
	default:
		break;
	}

	if(msdc_ctl->total_size > 512){
		msdc_ctl->result = -1;
		return  msdc_ctl->result;
	}

#if DEBUG_MMC_IOCTL
	printk("start MSDC_SINGLE_READ_WRITE !!\n");
#endif    
	memset(&msdc_data, 0, sizeof(struct mmc_data));
	memset(&msdc_mrq, 0, sizeof(struct mmc_request));
	memset(&msdc_cmd, 0, sizeof(struct mmc_command));

	msdc_mrq.cmd = &msdc_cmd;
	msdc_mrq.data = &msdc_data;

	if(msdc_ctl->trans_type)
		dma_force[host_ctl->id] = FORCE_IN_DMA;
	else
		dma_force[host_ctl->id] = FORCE_IN_PIO;

	if (msdc_ctl->iswrite){
		msdc_data.flags = MMC_DATA_WRITE;
		msdc_cmd.opcode = MMC_WRITE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;
		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
            if (copy_from_user(msdc_multi_buffer, msdc_ctl->buffer, 512))
                return -EFAULT;
		} else {
			/* called from other kernel module */
			memcpy(msdc_multi_buffer, msdc_ctl->buffer, 512);
		}
	} else {
		msdc_data.flags = MMC_DATA_READ;
		msdc_cmd.opcode = MMC_READ_SINGLE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;

		memset(msdc_multi_buffer, 0 , 512);
	}

	msdc_cmd.arg = msdc_ctl->address;

	BUG_ON(!host_ctl->mmc->card);
	if (!mmc_card_blockaddr(host_ctl->mmc->card)){
		printk("the device is used byte address!\n");
		msdc_cmd.arg <<= 9;
	}

	msdc_cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	msdc_data.stop = NULL;
	msdc_data.blksz = 512;
	msdc_data.sg = &msdc_sg;
	msdc_data.sg_len = 1;

#if DEBUG_MMC_IOCTL
	printk("single block: ueser buf address is 0x%p!\n",msdc_ctl->buffer);
#endif    
	sg_init_one(&msdc_sg, msdc_multi_buffer, msdc_ctl->total_size);
	mmc_set_data_timeout(&msdc_data, host_ctl->mmc->card);
	mmc_wait_for_req(host_ctl->mmc, &msdc_mrq);

	if (!msdc_ctl->iswrite){
		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
            if (copy_to_user(msdc_ctl->buffer,msdc_multi_buffer,512))
                return -EFAULT;
		} else {
			/* called from other kernel module */
			memcpy(msdc_ctl->buffer,msdc_multi_buffer,512);
		}
	}

	if (msdc_ctl->partition){
		mmc_send_ext_csd(host_ctl->mmc->card,l_buf);

		if (l_buf[179] & 0x8) {
			/* set back to access user area */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x0;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179]);
		}
	}

	mmc_release_host(host_ctl->mmc);

	if (msdc_cmd.error)
		msdc_ctl->result= msdc_cmd.error;

	if (msdc_data.error)
		msdc_ctl->result= msdc_data.error;
	else
		msdc_ctl->result= 0;

	return msdc_ctl->result;
}

static int mt6573_sd_ioctl_multi_rw(struct msdc_ioctl* msdc_ctl)
{
	char l_buf[512];
	struct scatterlist msdc_sg;
	struct mmc_data  msdc_data;
	struct mmc_command msdc_cmd;
	struct mmc_command msdc_stop;
	struct mmc_request  msdc_mrq;
	struct mt6573_sd_host *host_ctl;

	host_ctl = msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);

	mmc_claim_host(host_ctl->mmc);

#if DEBUG_MMC_IOCTL
	printk("user want access %d partition\n",msdc_ctl->partition);
#endif
	switch (msdc_ctl->partition){
	case BOOT_PARTITION_1:
		mmc_send_ext_csd(host_ctl->mmc->card,l_buf);
		printk("extend CSD: partition config is 0x%x\n", l_buf[179]);

		/* change to access boot partition 1 */
		l_buf[179] &= ~0x7;
		l_buf[179] |= 0x1;
		mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179]);
		break;
	case BOOT_PARTITION_2:
		mmc_send_ext_csd(host_ctl->mmc->card,l_buf);
		printk("extend CSD: partition config is 0x%x\n", l_buf[179]);

		/* change to access boot partition 2 */
		l_buf[179] &= ~0x7;
		l_buf[179] |= 0x2;
		mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179]);
		break;
	default:
		break;
	}

	if(msdc_ctl->total_size > 64*1024){
		msdc_ctl->result = -1;
		return  msdc_ctl->result;
	}

	memset(&msdc_data, 0, sizeof(struct mmc_data));
	memset(&msdc_mrq, 0, sizeof(struct mmc_request));
	memset(&msdc_cmd, 0, sizeof(struct mmc_command));
	memset(&msdc_stop, 0, sizeof(struct mmc_command));

	msdc_mrq.cmd = &msdc_cmd;
	msdc_mrq.data = &msdc_data;

	if(msdc_ctl->trans_type)
		dma_force[host_ctl->id] = FORCE_IN_DMA;
	else
		dma_force[host_ctl->id] = FORCE_IN_PIO;

	if (msdc_ctl->iswrite){
		msdc_data.flags = MMC_DATA_WRITE;
		msdc_cmd.opcode = MMC_WRITE_MULTIPLE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;
		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
            if (copy_from_user(msdc_multi_buffer, msdc_ctl->buffer, msdc_ctl->total_size))
                return -EFAULT;
		} else {
			/* called from other kernel module */
			memcpy(msdc_multi_buffer, msdc_ctl->buffer, msdc_ctl->total_size);
		}
	} else {
		msdc_data.flags = MMC_DATA_READ;
		msdc_cmd.opcode = MMC_READ_MULTIPLE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;
		memset(msdc_multi_buffer, 0 , msdc_ctl->total_size);
	}

	msdc_cmd.arg = msdc_ctl->address;

	BUG_ON(!host_ctl->mmc->card);
	if (!mmc_card_blockaddr(host_ctl->mmc->card)){
		printk("this device use byte address!!\n");
		msdc_cmd.arg <<= 9;
	}
	msdc_cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	msdc_stop.opcode = MMC_STOP_TRANSMISSION;
	msdc_stop.arg = 0;
	msdc_stop.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;

	msdc_data.stop = &msdc_stop;
	msdc_data.blksz = 512;
	msdc_data.sg = &msdc_sg;
	msdc_data.sg_len = 1;

#if DEBUG_MMC_IOCTL
	printk("total size is %d\n",msdc_ctl->total_size);
#endif
	sg_init_one(&msdc_sg, msdc_multi_buffer, msdc_ctl->total_size);
	mmc_set_data_timeout(&msdc_data, host_ctl->mmc->card);
	mmc_wait_for_req(host_ctl->mmc, &msdc_mrq);

	if (!msdc_ctl->iswrite){
		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
            if (copy_to_user(msdc_ctl->buffer, msdc_multi_buffer, msdc_ctl->total_size))
                return -EFAULT;
		} else {
			/* called from other kernel module */
			memcpy(msdc_ctl->buffer, msdc_multi_buffer, msdc_ctl->total_size);
		}
	}

	if (msdc_ctl->partition){
		mmc_send_ext_csd(host_ctl->mmc->card,l_buf);

		if (l_buf[179] & 0x8) {
			/* set back to access user area */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x0;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179]);
		}
	}

	mmc_release_host(host_ctl->mmc);

	if (msdc_cmd.error)
		msdc_ctl->result = msdc_cmd.error;

	if (msdc_data.error){
		msdc_ctl->result = msdc_data.error;
	} else {
		msdc_ctl->result = 0;
	}

	return msdc_ctl->result;
}

static int mt6573_sd_ioctl_get_cid(struct msdc_ioctl* msdc_ctl)
{
	struct mt6573_sd_host *host_ctl;

	host_ctl = msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

#if DEBUG_MMC_IOCTL
	printk("user want the cid in msdc slot%d \n",msdc_ctl->host_num);
#endif

    if (copy_to_user(msdc_ctl->buffer, &host_ctl->mmc->card->raw_cid, 16))
        return -EFAULT;

#if DEBUG_MMC_IOCTL
	printk("cid:0x%x,0x%x,0x%x,0x%x\n",host_ctl->mmc->card->raw_cid[0],
			host_ctl->mmc->card->raw_cid[1],
			host_ctl->mmc->card->raw_cid[2],
			host_ctl->mmc->card->raw_cid[3]);
#endif
	return 0;
}

static int mt6573_sd_ioctl_get_csd(struct msdc_ioctl* msdc_ctl)
{
	struct mt6573_sd_host *host_ctl;

	host_ctl = msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

#if DEBUG_MMC_IOCTL
	printk("user want the csd in msdc slot%d \n",msdc_ctl->host_num);
#endif

    if (copy_to_user(msdc_ctl->buffer, &host_ctl->mmc->card->raw_csd, 16))
        return -EFAULT;

#if DEBUG_MMC_IOCTL
	printk("csd:0x%x,0x%x,0x%x,0x%x\n",host_ctl->mmc->card->raw_csd[0],
			host_ctl->mmc->card->raw_csd[1],
			host_ctl->mmc->card->raw_csd[2],
			host_ctl->mmc->card->raw_csd[3]);
#endif
	return 0;
}

static int mt6573_sd_ioctl_get_excsd(struct msdc_ioctl* msdc_ctl)
{
	char l_buf[512];
	struct mt6573_sd_host *host_ctl;

	host_ctl = msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

	mmc_claim_host(host_ctl->mmc);

#if DEBUG_MMC_IOCTL
	printk("user want the extend csd in msdc slot%d \n",msdc_ctl->host_num);
#endif
	mmc_send_ext_csd(host_ctl->mmc->card,l_buf);
    if (copy_to_user(msdc_ctl->buffer, l_buf, 512))
        return -EFAULT;

#if DEBUG_MMC_IOCTL
	int i;
	for (i = 0; i < 512; i++)
	{
		printk("%x", l_buf[i]);
		if (0 == ((i + 1) % 16)){
			printk("\n");
		}
	}
#endif

    if (copy_to_user(msdc_ctl->buffer, l_buf, 512))
        return -EFAULT;

	mmc_release_host(host_ctl->mmc);

	return 0;
}



static long mt6573_sd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 base;
	struct msdc_ioctl *msdc_ctl = (struct msdc_ioctl *)arg;

	switch (msdc_ctl->opcode){
	case MSDC_SINGLE_READ_WRITE:
		msdc_ctl->result = mt6573_sd_ioctl_single_rw(msdc_ctl);
		break;
	case MSDC_MULTIPLE_READ_WRITE:
		msdc_ctl->result = mt6573_sd_ioctl_multi_rw(msdc_ctl);
		break;
	case MSDC_GET_CID:
		msdc_ctl->result = mt6573_sd_ioctl_get_cid(msdc_ctl);
		break;
	case MSDC_GET_CSD:
		msdc_ctl->result = mt6573_sd_ioctl_get_csd(msdc_ctl);
		break;
	case MSDC_GET_EXCSD:
		msdc_ctl->result = mt6573_sd_ioctl_get_excsd(msdc_ctl);
		break;
	case MSDC_DRIVING_SETTING:
		base = msdc_base_addr[msdc_ctl->host_num];
#if DEBUG_MMC_IOCTL
		printk("[%s]: before MSDC_DRIVING_SETTING(cmd_drv(%d), dat_drv(%d))! isWrite=%d, MSDC_IOCON0=0x%x\r\n",__func__, msdc_ctl->cmd_driving,msdc_ctl->dat_driving, msdc_ctl->iswrite, sdr_read32(MSDC_IOCON0));
#endif            
		if(msdc_ctl->iswrite){
			sdr_set_field(MSDC_IOCON0,MSDC_IOCON0_ODCCFG0,msdc_ctl->cmd_driving);
			sdr_set_field(MSDC_IOCON0,MSDC_IOCON0_ODCCFG1,msdc_ctl->dat_driving);
		}else{
			sdr_get_field(MSDC_IOCON0,MSDC_IOCON0_ODCCFG0,msdc_ctl->cmd_driving);
			sdr_get_field(MSDC_IOCON0,MSDC_IOCON0_ODCCFG1,msdc_ctl->dat_driving);
		}
#if DEBUG_MMC_IOCTL
		printk("[%s]: after MSDC_DRIVING_SETTING(cmd_drv(%d), dat_drv(%d))! isWrite=%d, MSDC_IOCON0=0x%x\r\n",__func__, msdc_ctl->cmd_driving,msdc_ctl->dat_driving, msdc_ctl->iswrite,sdr_read32(MSDC_IOCON0));
#endif 
		msdc_ctl->result = 0;
		break;
	default:
		printk("mt6573_sd_ioctl:this opcode value is illegal!!\n");
		return -EINVAL;
	}

	return msdc_ctl->result;
}

static int mt6573_sd_open(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations msdc_em_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= mt6573_sd_ioctl,
	.open		= mt6573_sd_open,
};

static struct miscdevice msdc_em_dev[] = {
	{
		.minor	= MISC_DYNAMIC_MINOR,
		.name	= "mt6573-sd0",
		.fops	= &msdc_em_fops,
	},
	{
		.minor	= MISC_DYNAMIC_MINOR,
		.name	= "mt6573-sd1",
		.fops	= &msdc_em_fops,
	},
	{
		.minor	= MISC_DYNAMIC_MINOR,
		.name	= "mt6573-sd2",
		.fops	= &msdc_em_fops,
	},
	{
		.minor	= MISC_DYNAMIC_MINOR,
		.name	= "mt6573-sd3",
		.fops	= &msdc_em_fops,
	},
};

static struct mmc_host_ops mt6573_sd_ops = {
	.request         = mt6573_sd_request,
	.set_ios         = mt6573_sd_set_ios,
	.get_ro          = mt6573_sd_card_readonly,
	.get_cd          = mt6573_sd_card_inserted,
	.enable_sdio_irq = mt6573_sd_enable_sdio_irq,
};

static int mt6573_sd_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct resource *reg, *dma;
	struct mt6573_sd_host *host;
	struct mt6573_sd_host_hw *hw;
	unsigned long base;
	int ret, irq, cirq;

	if ((pdev->id < 0) || (pdev->id >= HOST_MAX_NUM)) {
		printk("MSDC platform device id is %d, out of range!\n", pdev->id);
		return -EINVAL;
	}

	/* Allocate MMC host for this device */
	mmc = mmc_alloc_host(sizeof(struct mt6573_sd_host), &pdev->dev);
	if (!mmc)
		return -ENOMEM;

	hw   = (struct mt6573_sd_host_hw*)pdev->dev.platform_data;
	reg  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dma  = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	irq  = platform_get_irq(pdev, 0);
	cirq = platform_get_irq(pdev, 1);
	base = reg->start;

	BUG_ON(!hw);
	BUG_ON(!reg);
	BUG_ON(!dma);
	BUG_ON(irq < 0);
	BUG_ON(cirq < 0);

	/* for msdc engineer mode */
	msdc_base_addr[pdev->id] = base;

	reg = request_mem_region(reg->start, reg->end - reg->start + 1, DRV_NAME);
	if (reg == NULL) {
		mmc_free_host(mmc);
		return -EBUSY;
	}

	/* Set host parameters */
	mmc->ops        = &mt6573_sd_ops;
	mmc->f_min      = HOST_MIN_SCLK;
	mmc->f_max      = HOST_MAX_SCLK;
	if((pdev->id ==0x1)&&(!(hw->flags & MSDC_HIGHSPEED)))
		mmc->f_max      = HOST_MAX_SCLK/2;
	mmc->ocr_avail  = MMC_VDD_32_33 | MMC_VDD_33_34;
	if (hw->flags & MSDC_HIGHSPEED) {
		mmc->caps   = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED;
	}
	if (hw->data_pins == 4) {
		mmc->caps  |= MMC_CAP_4_BIT_DATA;
	} else if (hw->data_pins == 8) {
		mmc->caps  |= MMC_CAP_8_BIT_DATA;
	}
	if ((hw->flags & MSDC_SDIO_IRQ) || (hw->flags & MSDC_EXT_SDIO_IRQ))
		mmc->caps |= MMC_CAP_SDIO_IRQ;

	/* MMC core transfer sizes tunable parameters */
	mmc->max_hw_segs   = 128;
	mmc->max_phys_segs = 128;
	mmc->max_seg_size  = 128 * 512;
	mmc->max_blk_size  = HOST_MAX_BLKSZ;
	mmc->max_blk_count = 131071;//65535;
	mmc->max_req_size  = 131072;//65536;

	host = mmc_priv(mmc);
	host->hw        = hw;
	host->mmc       = mmc;
	host->id        = pdev->id;
	host->base      = base;
	host->sd_irq    = irq;
	host->cd_irq    = cirq;
	host->mclk      = 0;
	host->hclk      = ghclk_tab[hw->clk_src];
	host->sclk      = HOST_INI_SCLK;
	host->pm_state  = PMSG_RESUME;
	host->suspend   = 0;
	host->core_clkon = 0;
	host->card_clkon = 0;
	host->core_power = 0;
	if(host->id ==0)
		host->power_mode = MMC_POWER_UP;
	else
		host->power_mode = MMC_POWER_OFF;

	host->card_inserted = hw->flags & MSDC_REMOVABLE ? 0 : 1;
	host->timeout_ns = 0;
	host->timeout_clks = DEFAULT_DTOC * 65536;
	host->error_address = 0;

	/* for debug */
	msdc_host[pdev->id]      = host;
	spin_lock_init(&host->lock);

	tasklet_init(&host->card_tasklet, mt6573_sd_tasklet_card, (ulong)host);
	tasklet_init(&host->fifo_tasklet, mt6573_sd_tasklet_fifo, (ulong)host);
	tasklet_init(&host->dma_tasklet, mt6573_sd_tasklet_dma, (ulong)host);

	mt6573_sd_init_hw(host, dma);

	mt65xx_irq_set_sens(irq, MT65xx_LEVEL_SENSITIVE);
	ret = request_irq((unsigned int)irq,(irq_handler_t)mt6573_sd_irq, 0, DRV_NAME, host);
	if (ret)
		goto release;

	if (hw->flags & MSDC_CD_PIN_EN) {
		if (hw->request_cd_eirq) {
			hw->request_cd_eirq(mt6573_sd_cd_eirq, (void*)host);
		} else {
			mt65xx_irq_set_sens(cirq, MT65xx_EDGE_SENSITIVE);
			ret = request_irq((unsigned int)cirq, (irq_handler_t)mt6573_sd_cd_irq, 0, DRV_NAME, host);
			if (ret)
				goto free_irq;
		}
	}

	if (hw->request_sdio_eirq)
		hw->request_sdio_eirq(mt6573_sd_sdio_eirq, (void*)host);

	if (hw->register_pm){
		hw->register_pm(mt6573_sd_pm_change, (void*)host);
		if(hw->flags & MSDC_SYS_SUSPEND)
			printk("[SD%d] %s: MSDC_SYS_SUSPEND and register_pm both set\n", host->id, __func__);
		mmc->pm_flags |= MMC_PM_IGNORE_PM_NOTIFY;
	}

	platform_set_drvdata(pdev, mmc);

#ifdef CFG_PROFILING
	mmc_perf_init(host, pdev);
#endif

	ret = mmc_add_host(mmc);
	if (ret)
		goto free_cirq;

	/* Config card detection pin and enable interrupts */
	if (hw->flags & MSDC_CD_PIN_EN) {
		mt6573_sd_enable_cd_irq(mmc, 1);
	} else {
		mt6573_sd_enable_cd_irq(mmc, 0);
	}
	/* temp solution */
	//sdr_write32(MSDC_IOCON0, 0x1280036);
	//sdr_write32(MSDC_IOCON1, 0x23332);

	sdr_set_bits(MSDC_CFG, MSDC_CFG_INTEN);

	if (msdc_multi_buffer != NULL) {
		ret = misc_register(&msdc_em_dev[host->id]);
		if (ret) {
			printk("[SD%d] %s: register misc driver failed (%d)\n",host->id, __func__, ret);
		}
	}

	return 0;

free_cirq:
	free_irq(cirq, host);
free_irq:
	free_irq(irq, host);
release:
	platform_set_drvdata(pdev, NULL);
	mmc_free_host(mmc);
	mt6573_sd_deinit_hw(host);

	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->fifo_tasklet);
	tasklet_kill(&host->dma_tasklet);

	if (reg)
		release_mem_region(reg->start, reg->end - reg->start + 1);

	return ret;
}

static int mt6573_sd_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc  = platform_get_drvdata(pdev);
	struct mt6573_sd_host *host = mmc_priv(mmc);
	struct mt6573_sd_host_hw *hw = host->hw;
	struct resource *res;

	BUG_ON(!mmc || !host);

	printk(KERN_INFO "[SD%d] %s: removed !!!\n", host->id, __func__);

	platform_set_drvdata(pdev, NULL);
	mmc_remove_host(host->mmc);
	mmc_free_host(host->mmc);
	mt6573_sd_deinit_hw(host);

	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->fifo_tasklet);
	tasklet_kill(&host->dma_tasklet);
	free_irq(host->sd_irq, host);
	if ((hw->flags & MSDC_CD_PIN_EN) && !hw->request_cd_eirq)
		free_irq(host->cd_irq, host);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (res)
		release_mem_region(res->start, res->end - res->start + 1);

	if (msdc_multi_buffer != NULL) {
		misc_deregister(&msdc_em_dev[host->id]);
	}
	return 0;
}

#ifdef CONFIG_PM
static int mt6573_sd_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct mt6573_sd_host *host = mmc_priv(mmc);

	if (mmc && state.event == PM_EVENT_SUSPEND &&
			(host->hw->flags & MSDC_SYS_SUSPEND))
		mt6573_sd_pm_change(state, (void*)host);

	return ret;
}

static int mt6573_sd_resume(struct platform_device *pdev)
{
	int ret = 0;
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct mt6573_sd_host *host = mmc_priv(mmc);
	struct pm_message state;

	state.event = PM_EVENT_RESUME;
	if (mmc && (host->hw->flags & MSDC_SYS_SUSPEND))
		mt6573_sd_pm_change(state, (void*)host);

	return ret;
}
#endif

static struct platform_driver mt6573_sd_driver = {
	.probe   = mt6573_sd_probe,
	.remove  = mt6573_sd_remove,
#ifdef CONFIG_PM
	.suspend = mt6573_sd_suspend,
	.resume  = mt6573_sd_resume,
#endif
	.driver  = {
		.name  = DRV_NAME,
		.owner = THIS_MODULE,
	},
};

#ifndef  CONFIG_MMC_DEBUG
static ssize_t mt6573_sd_debug_sw_read(struct file *file, char *data, size_t len, loff_t *offset)
{
	unsigned char host_id=0;
	char* pll_src_name[5]={"26MHz", "3G PLL", "2G PLL", "N/A", "EMI PLL"};
	char* hclk_name[4] = { "(/5)98.3MHz", "(/6)81.9MHz", "(/7)70.2MHz", "(/8)61.4MHz" };
	int pll_src_id = 1;

	pll_src_id = sdr_read32(PLL_CON5_REG)&0xf;

	for (host_id=0; host_id<4; host_id++)
	{
		printk("=======================Host[%d]=======================\n", host_id);
		printk("      opened log: %s%s%s%s%s%s%s%s%s\n",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_DMA)? " DBG_EVT_DMA":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_CMD)? " DBG_EVT_CMD":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_RSP)? " DBG_EVT_RSP":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_INT)? " DBG_EVT_INT":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_CFG)? " DBG_EVT_CFG":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_FUC)? " DBG_EVT_FUC":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_OPS)? " DBG_EVT_OPS":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_WRN)? " DBG_EVT_WRN":"",
				(mt6573_sd_debug_sw[host_id] & DBG_EVT_PWR)? " DBG_EVT_PWR":"");
		if(msdc_host[host_id]){
			printk("      insert card: %d\r\n",is_card_present(msdc_host[host_id]));
			printk("      pll_src=%s, clock_src=%s, clock_fre[card:%uHz, host:%uHz], clock_on[card:%d, host:%d]\r\n",
					pll_src_name[pll_src_id],
					hclk_name[msdc_host[host_id]->hw->clk_src],
					msdc_host[host_id]->sclk,
					msdc_host[host_id]->hclk,
					msdc_host[host_id]->card_clkon,
					msdc_host[host_id]->core_clkon);
		}else{
			printk("      Has not been initialized!\r\n");
		}
	}

	printk("=========================================\r\n");
	return 0;
}

static ssize_t mt6573_sd_debug_sw_write(struct file *file, const char *data, size_t len, loff_t *offset)
{
	unsigned char  debug_sw[8];
	unsigned long  input = 0;
	unsigned char  host_id = 0, command = 0;

	if (len > 8)
		return -EFAULT;
	if (copy_from_user(debug_sw, data, 8))
		return -EFAULT;

	input = simple_strtol(debug_sw, NULL, 0);

	host_id = (input>>12)&0x3;
	command = (input>>16);

	switch(command){
	case 0:
		printk("===============CMD%d: Modify debug flag of host[%d]============\r\n", command, host_id);
		if(host_id < HOST_MAX_NUM){
			mt6573_sd_debug_sw[host_id] = input & 0x1FF;
			printk("    [SUCESS]: Host[%d]'s opened log: %s%s%s%s%s%s%s%s%s\n", host_id,
					(input & DBG_EVT_DMA)? " DBG_EVT_DMA":"",
					(input & DBG_EVT_CMD)? " DBG_EVT_CMD":"",
					(input & DBG_EVT_RSP)? " DBG_EVT_RSP":"",
					(input & DBG_EVT_INT)? " DBG_EVT_INT":"",
					(input & DBG_EVT_CFG)? " DBG_EVT_CFG":"",
					(input & DBG_EVT_FUC)? " DBG_EVT_FUC":"",
					(input & DBG_EVT_OPS)? " DBG_EVT_OPS":"",
					(input & DBG_EVT_WRN)? " DBG_EVT_WRN":"",
					(input & DBG_EVT_PWR)? " DBG_EVT_PWR":"");
		}else{
			printk("    [FAIL]: Bad host id\r\n");
		}
		break;
	case 1:
		printk("===============CMD%d: Test the basic function of the card in host[%d]============\r\n",command, host_id);
		if(msdc_host[host_id]){
			if(!is_card_present(msdc_host[host_id])){
				printk(" No card inserted\n");
				break;
			}
			clr_device_working_ability(clock_id[host_id], SLOW_IDLE_STATE);
			clr_device_working_ability(clock_id[host_id], DEEP_IDLE_STATE);

			printk(" **Test latch command:\n");
			if(SD_CanCmdBeLatched(msdc_host[host_id]) == SD_TRUE)
				printk("   **[SUCESS]: latch command\n");
			else
				printk("   **[FAIL]: latch command\n");

			printk(" **Test single read:\n");
			if(SD_CanSingleReadBeLatched(msdc_host[host_id], 0) == SD_TRUE)
				printk("   **[SUCESS]: single read\n");
			else
				printk("   **[FAIL]: single read\n");

			printk(" **Test multiple read:\n");
			if(SD_CanMultiReadBeLatched(msdc_host[host_id], 0) == SD_TRUE)
				printk("   **[SUCESS]: multiple read\n");
			else
				printk("   **[FAIL]: multiple read\n");

			set_device_working_ability(clock_id[host_id], SLOW_IDLE_STATE);
			set_device_working_ability(clock_id[host_id], DEEP_IDLE_STATE);
		}else{
			printk(" Has not been initialized!\n");
		}
		break;
	case 2:
		printk("===============CMD%d: do sd_retry of the card in host[%d]============\r\n",command, host_id);
		if(msdc_host[host_id]){
			if(!is_card_present(msdc_host[host_id])){
				printk(" No card inserted\n");
				break;
			}
			clr_device_working_ability(clock_id[host_id], SLOW_IDLE_STATE);
			clr_device_working_ability(clock_id[host_id], DEEP_IDLE_STATE);
			mt6573_sd_retry(msdc_host[host_id], 0);
			set_device_working_ability(clock_id[host_id], SLOW_IDLE_STATE);
			set_device_working_ability(clock_id[host_id], DEEP_IDLE_STATE);
		}
		break;

	default:
		printk("===============Bad command: %d============\r\n", command);
		break;
	}

	printk("=========================================\r\n");
	return  len;
}
static ssize_t mt6573_sd_dump_sw_read(struct file *file, char *data, size_t len, loff_t *offset)
{
	unsigned char host_id=0;
	u32 base;
	for (host_id=0; host_id<4; host_id++)
	{
		printk("=======================SD[%d]=========================\n", host_id);
		if(msdc_host[host_id]){
			base = msdc_host[host_id]->base;
			printk("  MSDC register:\n");
			printk("    MSDC_CFG		0x%x\n",sdr_read32(MSDC_CFG));
			printk("    MSDC_STA		0x%x\n",sdr_read32(MSDC_STA));
			printk("    MSDC_INT		0x%x\n",sdr_read32(MSDC_INT));
			printk("    MSDC_PS		0x%x\n",sdr_read32(MSDC_PS));
			printk("    MSDC_DAT		0x%x\n",sdr_read32(MSDC_DAT));
			printk("    MSDC_IOCON0		0x%x\n",sdr_read32(MSDC_IOCON0));
			printk("    MSDC_IOCON1		0x%x\n",sdr_read32(MSDC_IOCON1));

			printk("  SDC register:\n");
			printk("    SDC_CFG		0x%x\n",sdr_read32(SDC_CFG));
			printk("    SDC_CMD		0x%x\n",sdr_read32(SDC_CMD));
			printk("    SDC_ARG		0x%x\n",sdr_read32(SDC_ARG));
			printk("    SDC_STA		0x%x\n",sdr_read32(SDC_STA));
			printk("    SDC_RESP0		0x%x\n",sdr_read32(SDC_RESP0));
			printk("    SDC_RESP1		0x%x\n",sdr_read32(SDC_RESP1));
			printk("    SDC_RESP2		0x%x\n",sdr_read32(SDC_RESP2));
			printk("    SDC_RESP3		0x%x\n",sdr_read32(SDC_RESP3));
			printk("    SDC_CMDSTA		0x%x\n",sdr_read32(SDC_CMDSTA));
			printk("    SDC_DATSTA		0x%x\n",sdr_read32(SDC_DATSTA));
			printk("    SDC_CSTA		0x%x\n",sdr_read32(SDC_CSTA));
			printk("    SDC_IRQMASK0	0x%x\n",sdr_read32(SDC_IRQMASK0));
			printk("    SDC_IRQMASK1	0x%x\n",sdr_read32(SDC_IRQMASK1));

			printk("  SDIO register:\n");
			printk("    SDIO_CFG		0x%x\n",sdr_read32(SDIO_CFG));
			printk("    SDIO_STA		0x%x\n",sdr_read32(SDIO_STA));

			printk("  MMC boot up register:\n");
			printk("    BOOT_CFG		0x%x\n",sdr_read32(BOOT_CFG));
			printk("    BOOT_STA		0x%x\n",sdr_read32(BOOT_STA));
			printk("    BOOT_IOCON		0x%x\n",sdr_read32(BOOT_IOCON));

			printk("  Auto calibration register:\n");
			printk("    CLKACB_CFG		0x%x\n",sdr_read32(CLKACB_CFG));
			printk("    CLKACB_STA		0x%x\n",sdr_read32(CLKACB_STA));
			printk("    CLKACB_CRCSTA1	0x%x\n",sdr_read32(CLKACB_CRCSTA1));
			printk("    CLKACB_CRCSTA2	0x%x\n",sdr_read32(CLKACB_CRCSTA2));

			base = msdc_host[host_id]->dma.base;
			printk("  MSDC DMA register:\n");
			printk("    DMA_INT_FLAG	0x%x\n",sdr_read32(DMA_INT_FLAG));
			printk("    DMA_INT_EN		0x%x\n",sdr_read32(DMA_INT_EN));
			printk("    DMA_EN		0x%x\n",sdr_read32(DMA_EN));
			printk("    DMA_RST		0x%x\n",sdr_read32(DMA_RST));
			printk("    DMA_STOP		0x%x\n",sdr_read32(DMA_STOP));
			printk("    DMA_FLUSH		0x%x\n",sdr_read32(DMA_FLUSH));
			printk("    DMA_CTRL		0x%x\n",sdr_read32(DMA_CTRL));
			printk("    DMA_MEM_ADDR	0x%x\n",sdr_read32(DMA_MEM_ADDR));
			printk("    DMA_LEN		0x%x\n",sdr_read32(DMA_LEN));
			printk("    DMA_INTBUF_SZ	0x%x\n",sdr_read32(DMA_INTBUF_SZ));
			printk("    DMA_DBG_STS		0x%x\n",sdr_read32(DMA_DBG_STS));
		}else{
			printk("  Has not been initialized!\r\n");
		}

	}
	printk("=========================================\r\n");
	return 0;
}
static struct file_operations mt6573_sd_debug_sw_fops = {
	.read  = mt6573_sd_debug_sw_read,
	.write = mt6573_sd_debug_sw_write,
};
static struct file_operations mt6573_sd_dump_sw_fops = {
	.read  = mt6573_sd_dump_sw_read,
	.write = NULL,
};
static struct proc_dir_entry *mt6573_debug_entry=NULL;
static struct proc_dir_entry *mt6573_dump_entry=NULL;

static int mt6573_sd_dump_register(void)
{
	mt6573_dump_entry = create_proc_entry("sd_reg", S_IFREG|S_IRUGO, NULL);
	if (mt6573_dump_entry == NULL)
	{
		printk("Can't create /proc/sd_reg\n");
		return -ENOMEM;
	}
	mt6573_dump_entry->proc_fops= &mt6573_sd_dump_sw_fops;
	return 1;
}

static int mt6573_sd_dump_unregister(void)
{
	remove_proc_entry("sd_reg", NULL);

	return 1;
}

static int mt6573_sd_debug_register(void)
{
	mt6573_debug_entry = create_proc_entry("sd_debug", S_IFREG|S_IRUGO, NULL);
	if (mt6573_debug_entry == NULL)
	{
		printk("Can't create /proc/sd_debug\n");
		return -ENOMEM;
	}

	mt6573_debug_entry->proc_fops= &mt6573_sd_debug_sw_fops;
	return 1;
}

static int mt6573_sd_debug_unregister(void)
{
	remove_proc_entry("sd_debug", NULL);

	return 1;
}
#endif

static int __init mt6573_sd_init(void)
{
	int ret;

#ifndef  CONFIG_MMC_DEBUG
	ret = mt6573_sd_debug_register();
	if (ret != 1)
		return ret;

	ret = mt6573_sd_dump_register();
	if (ret != 1)
		return ret;
#endif

	msdc_multi_buffer =(u32*)kmalloc(64*1024,GFP_KERNEL);
	if( msdc_multi_buffer ==NULL)
		printk("allock 64KB memory failed\n");

	ret = platform_driver_register(&mt6573_sd_driver);
	if (ret) {
		printk(KERN_ERR DRV_NAME ": Can't register driver\n");
		return ret;
	}
	printk(KERN_INFO DRV_NAME ": MediaTek MT6573 SD/MMC Card Driver\n");

	return 0;
}

static void __exit mt6573_sd_exit(void)
{
	platform_driver_unregister(&mt6573_sd_driver);

	if (msdc_multi_buffer != NULL)
		kfree(msdc_multi_buffer);

#ifndef  CONFIG_MMC_DEBUG
	mt6573_sd_debug_unregister();
	mt6573_sd_dump_unregister();
#endif
}

module_init(mt6573_sd_init);
module_exit(mt6573_sd_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek MT6573 SD/MMC Card Driver");
MODULE_AUTHOR("Infinity Chen <infinity.chen@mediatek.com>");
