
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/module.h>
#include <asm/tcm.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/dma.h"
#include "mach/sync_write.h"
#include "mach/mt6573_pll.h"

#define DMA_DEBUG   0
#if(DMA_DEBUG == 1)
#define dbgmsg printk
static void dbg_print_dma_info(struct mt_dma_conf *config);
#else
#define dbg_print_dma_info(...)
#define dbgmsg(...)
#endif


#define DMA_IRQ MT6573_APDMA_AP_IRQ_LINE
#define NR_GDMA_CHANNEL 2
#define NR_PDMA_CHANNEL 9
#define NR_VFFDMA_CHANNEL 8
#define GDMA_START 0
#define PDMA_START (GDMA_START + NR_GDMA_CHANNEL)
#define VFFDMA_START (PDMA_START + NR_PDMA_CHANNEL)
#define NR_DMA (NR_GDMA_CHANNEL + NR_PDMA_CHANNEL + NR_VFFDMA_CHANNEL)
#define UART0 (VFFDMA_START + 0)
#define UART1 (VFFDMA_START + 1)
#define UART2 (VFFDMA_START + 2)
#define UART3 (VFFDMA_START + 3)
#define UART4 (VFFDMA_START + 4)
#define UART5 (VFFDMA_START + 5)
    

#define DMA_BASE_CH(n) (AP_DMA_BASE + 0x0080*(n + 1))

#define DMA_GLOBAL_INT_FLAG (AP_DMA_BASE + 0x0000) 
#define DMA_GLOBAL_RUNNING_STATUS (AP_DMA_BASE + 0x00008) 
#define DMA_VPORT_BASE 0xF7002600
#define DMA_VPORT_CH(ch) (DMA_VPORT_BASE + (ch - VFFDMA_START) * 0x00000100)
#define DMA_SRC(base) (base + 0x001C)
#define DMA_DST(base) (base + 0x0020)
#define DMA_LEN2(base) (base + 0x0028)
#define DMA_JUMP_ADDR(base) (base + 0x002C)
#define DMA_LEN1(base) (base + 0x0024)
#define DMA_CON(base) (base + 0x0018)
#define DMA_START(base)	(base + 0x0008)
#define DMA_FLUSH(base) (base + 0x0014)
#define DMA_INT_FLAG(base) (base + 0x0000)
#define DMA_INT_EN(base) (base + 0x0004)
#define DMA_MEM_ADDR(base) (base + 0x001C)
#define DMA_VFF_WPT(base) (base + 0x002C)
#define DMA_VFF_RPT(base) (base + 0x0030)
#define DMA_VFF_VALID_SIZE(base) (base + 0x003C)
#define DMA_VFF_THRE(base) (base + 0x0028)
#define DMA_VFF_LEN(base) (base + 0x024)


#define DMA_GLBSTA_RUN(ch) (0x00000001 << ((ch)))
#define DMA_GLBSTA_IT(ch) (0x00000001 << ((ch)))

#define DMA_GDMA_LEN_MAX_MASK 0x000FFFFF
#define DMA_CON_SFIX 0x00000010
#define DMA_CON_DFIX 0x00000008
#define DMA_CON_WSIZE_1BYTE 0x00000000
#define DMA_CON_WSIZE_4BYTE 0x02000000
#define DMA_CON_RSIZE_1BYTE 0x00000000
#define DMA_CON_RSIZE_4BYTE 0x20000000
#define DMA_CON_BURST_MASK 0x00070000
#define DMA_CON_SLOW_EN 0x00000004
#define DMA_CON_SLOW_OFFSET 5
#define DMA_CON_SLOW_MAX_MASK 0x000003FF
#define DMA_CON_WPSD 0x00100000
#define DMA_CON_WPEN 0x00008000
#define DMA_CON_DIR 0x00000001

#define DMA_START_BIT (0x00000001)
#define DMA_STOP_BIT (0x00000000)
#define DMA_INT_FLAG_BIT (0x00000001)
#define DMA_INT_FLAG_CLR_BIT (0x00000000)
#define DMA_INT_EN_BIT (0x00000001)
#define DMA_INT_EN_CLR_BIT (0x00000000)
#define DMA_FLUSH_BIT (0x00000001)
#define DMA_FLUSH_CLR_BIT (0x00000000)
#define DMA_UART_RX_INT_EN_BIT (0x00000003)


struct dma_ctrl
{
    int in_use;
    DMA_ISR_CALLBACK isr_cb;
    void *data;
};

typedef enum {
    DMA_FULL_CHANNEL = 0,
    DMA_HALF_CHANNEL,
    DMA_VIRTUAL_FIFO
} DMA_TYPE;

typedef struct DMA_CHAN {
    struct mt_dma_conf config;
    u32 baseAddr;
    DMA_TYPE type;
    unsigned char chan_num;
    unsigned int registered;
} DMA_CHAN;


static struct dma_ctrl dma_ctrl[NR_DMA];
static spinlock_t dma_drv_lock;

int mt65xx_req_peri_dma(int channel, DMA_ISR_CALLBACK cb, void *data)
{
    unsigned long flags;
    int ret = 0;

#if 0
    if (channel < PDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
    if (channel >= VFFDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
#endif

    spin_lock_irqsave(&dma_drv_lock, flags);

    if (dma_ctrl[channel].in_use == 0) {
        dma_ctrl[channel].in_use = 1;
    } else {
        ret = -DMA_ERR_CH_BUSY;
    }

    spin_unlock_irqrestore(&dma_drv_lock, flags);

    if (ret == 0) {
        dma_ctrl[channel].isr_cb = cb;
        dma_ctrl[channel].data = data;
    }

    return ret;
}

EXPORT_SYMBOL(mt65xx_req_peri_dma);

int mt65xx_req_vff_dma(int channel, DMA_ISR_CALLBACK cb, void *data)
{
    unsigned long flags;
    int ret = 0;

#if 0
    if (channel < VFFDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
    if (channel >= (VFFDMA_START + NR_VFFDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }
#endif

    spin_lock_irqsave(&dma_drv_lock, flags);

    if (dma_ctrl[channel].in_use == 0) {
        dma_ctrl[channel].in_use = 1;
    } else {
        ret = -DMA_ERR_CH_BUSY;
    }

    spin_unlock_irqrestore(&dma_drv_lock, flags);

    if (ret == 0) {
        dma_ctrl[channel].isr_cb = cb;
        dma_ctrl[channel].data = data;
    }

    return ret;
}

EXPORT_SYMBOL(mt65xx_req_vff_dma);

int mt65xx_req_gdma(DMA_ISR_CALLBACK cb, void *data)
{
    unsigned long flags;
    int i;

    spin_lock_irqsave(&dma_drv_lock, flags);

    for (i = GDMA_START; i < NR_GDMA_CHANNEL; i++) {
        if (dma_ctrl[i].in_use) {
            continue;
        } else {
            dma_ctrl[i].in_use = 1;
            break;
        }
    }

    spin_unlock_irqrestore(&dma_drv_lock, flags);

    if (i <NR_GDMA_CHANNEL) {
        dma_ctrl[i].isr_cb = cb;
        dma_ctrl[i].data = data;
        return i;
    } else {
        return -DMA_ERR_NO_FREE_CH;
    }
}

EXPORT_SYMBOL(mt65xx_req_gdma);

int mt65xx_start_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }
    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(DMA_BASE_CH(channel)));
    mt65xx_reg_sync_writel(DMA_START_BIT, DMA_START(DMA_BASE_CH(channel)));

    return 0;
}

EXPORT_SYMBOL(mt65xx_start_gdma);

int mt65xx_stop_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }
    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    writel(DMA_FLUSH_BIT, DMA_FLUSH(DMA_BASE_CH(channel)));
    while (readl(DMA_START(DMA_BASE_CH(channel))));
    writel(DMA_FLUSH_CLR_BIT, DMA_FLUSH(DMA_BASE_CH(channel)));
    mt65xx_reg_sync_writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(DMA_BASE_CH(channel)));

    return 0;
}

EXPORT_SYMBOL(mt65xx_stop_gdma);

int mt65xx_config_gdma(int channel, struct mt65xx_gdma_conf *config, DMA_CONF_FLAG flag)
{
    unsigned int dma_con = 0x0, limiter = 0;

    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }
    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }
    if (!config) {
        return -DMA_ERR_INV_CONFIG;
    }
    if (!(config->sinc) && ((config->src) % 8)) {
        printk("GDMA fixed address mode requires 8-bytes aligned address\n");
        return -DMA_ERR_INV_CONFIG;
    }
    if (!(config->dinc) && ((config->dst) % 8)) {
        printk("GDMA fixed address mode requires 8-bytes aligned address\n");
        return -DMA_ERR_INV_CONFIG;
    }

    switch (flag) {
    case ALL:
        writel(config->src, DMA_SRC(DMA_BASE_CH(channel)));
        writel(config->dst, DMA_DST(DMA_BASE_CH(channel)));
        writel((config->wplen) & DMA_GDMA_LEN_MAX_MASK, DMA_LEN2(DMA_BASE_CH(channel)));
        writel(config->wpto, DMA_JUMP_ADDR(DMA_BASE_CH(channel)));
        writel((config->count) & DMA_GDMA_LEN_MAX_MASK, DMA_LEN1(DMA_BASE_CH(channel)));

        if (config->wpen)
            dma_con |= DMA_CON_WPEN;
        if (config->wpsd)
            dma_con |= DMA_CON_WPSD;
        if (config->iten) {
            writel(DMA_INT_EN_BIT, DMA_INT_EN(DMA_BASE_CH(channel)));
        } else {
            writel(DMA_INT_EN_CLR_BIT, DMA_INT_EN(DMA_BASE_CH(channel)));
        }

        dma_con |= (config->burst & DMA_CON_BURST_MASK);

        if (!(config->dinc)) {
            dma_con |= DMA_CON_DFIX;
            dma_con |= DMA_CON_WSIZE_1BYTE;
            dma_con &= ~(DMA_CON_BURST_MASK);
            dma_con |= DMA_CON_BURST_SINGLE;
        }
        if (!(config->sinc)) {
            dma_con |= DMA_CON_SFIX;
            dma_con |= DMA_CON_RSIZE_1BYTE;
            dma_con &= ~(DMA_CON_BURST_MASK);
            dma_con |= DMA_CON_BURST_SINGLE;
        }
        if (config->limiter) {
            limiter = (config->limiter) & DMA_CON_SLOW_MAX_MASK;
            dma_con |= limiter << DMA_CON_SLOW_OFFSET;
            dma_con |= DMA_CON_SLOW_EN;
        }

        writel(dma_con, DMA_CON(DMA_BASE_CH(channel)));

        break;

    case SRC:
        writel(config->src, DMA_SRC(DMA_BASE_CH(channel)));
        break;
        
    case DST:
        writel(config->dst, DMA_DST(DMA_BASE_CH(channel)));
        break;

    case SRC_AND_DST:
        writel(config->src, DMA_SRC(DMA_BASE_CH(channel)));
        writel(config->dst, DMA_DST(DMA_BASE_CH(channel)));
        break;

    default:
        break;
    }

    /* use the data synchronization barrier to ensure that all writes are completed */
    dsb();

    return 0;
}

EXPORT_SYMBOL(mt65xx_config_gdma);

int mt65xx_free_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }
    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }
    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    mt65xx_stop_gdma(channel);

    dma_ctrl[channel].isr_cb = NULL;
    dma_ctrl[channel].data = NULL;
    dma_ctrl[channel].in_use = 0;

    return 0;
}

EXPORT_SYMBOL(mt65xx_free_gdma);


void mt65xx_dma_running_status(void)
{
  unsigned int dma_running_status;
  int i=0;
  char DMA_name [19][10] = {"G_DMA1", "G_DMA2", "MSDC1", "MSDC2", "MSDC3", "MSDC4", "SIM1", "SIM2", "I2C1", "I2C2", 
                            "IRDA", "UART1_TX", "UART1_RX", "UART2_TX", "UART2_RX", "UART3_TX", "UART3_RX", "UART4_TX", "UART4_RX"};
  
  dma_running_status = readl(DMA_GLOBAL_RUNNING_STATUS);
  for(i=0; i<19; i++)
  {
    if(((dma_running_status>>i) & 0x01) == 1)
    {
      printk("DMA %s is running\n", DMA_name[i]);
    }
  }
}

EXPORT_SYMBOL(mt65xx_dma_running_status);


static DMA_CHAN dma_chan[NR_DMA];
static unsigned char free_list_head_fs;
static unsigned char free_list_fs[NR_GDMA_CHANNEL];
static unsigned char free_list_head_hs;
static unsigned char free_list_hs[NR_PDMA_CHANNEL];


struct mt_dma_conf *__allocate_dma(unsigned char *free_list, unsigned char *free_list_head, DMA_CHAN *dma_chan_buf, int nr_dma)
{
    struct mt_dma_conf *config = NULL;
    unsigned long irq_flag;

    /* grab dma_drv_lock */
    spin_lock_irqsave(&dma_drv_lock, irq_flag);
    
    if (*free_list_head != nr_dma) {
         config = &(dma_chan_buf[*free_list_head].config);
         dma_chan_buf[*free_list_head].registered = DMA_TRUE;
         *free_list_head = free_list[*free_list_head];
    } else {
        printk(KERN_ERR"DMA Module - All DMA channel were used!\n");
    }

    dbg_print_dma_info(config);
        
    /* release dma_drv_lock */
    spin_unlock_irqrestore(&dma_drv_lock, irq_flag);    
    
    return config;
}

struct mt_dma_conf *mt_request_full_size_dma(void)
{   
    int ch;
 
    dbgmsg("Full-Size DMA Channel\n");
    
#if 0
    return __allocate_dma(free_list_fs, &free_list_head_fs, &dma_chan[GDMA_START], NR_GDMA_CHANNEL);
#endif
    ch = mt65xx_req_gdma(NULL, NULL);
    if (ch >= 0) {
        dma_chan[ch].registered = DMA_TRUE;
        return &(dma_chan[ch].config);
    } else {
        return NULL;
    }
}

struct mt_dma_conf *mt_request_half_size_dma(void)
{
    dbgmsg("Half-Size DMA Channel\n");
    
    return __allocate_dma(free_list_hs, &free_list_head_hs, &dma_chan[PDMA_START], NR_PDMA_CHANNEL);
}

struct mt_dma_conf *mt_request_virtual_fifo_dma(unsigned int vf_dma_ch)
{
    struct mt_dma_conf *config = NULL;
    unsigned long irq_flag;
    unsigned ch = VFFDMA_START + vf_dma_ch;
    
    dbgmsg("Virtual fifo DMA channel %d\n", vf_dma_ch);
    
    /* grab dma_drv_lock */
    spin_lock_irqsave(&dma_drv_lock, irq_flag);

    if (ch >= NR_DMA)
    {
        printk(KERN_ERR"DMA Module - The requested virtual fifo %d DMA channel is not exist!!\n", vf_dma_ch);        
        return NULL;
    }        
    
    if (dma_chan[ch].registered == DMA_FALSE) {
        config = &(dma_chan[ch].config);
        dma_chan[ch].registered = DMA_TRUE;
    } else {
        printk(KERN_ERR"DMA Module - The DMA channel %d was used!\n", ch);
    }
        
    dbg_print_dma_info(config);
        
    /* release dma_drv_lock */
    spin_unlock_irqrestore(&dma_drv_lock, irq_flag);

    return config;
}

void mt_free_dma(struct mt_dma_conf *config)
{
    DMA_CHAN *chan = (DMA_CHAN *)config;
    unsigned char ch = chan->chan_num;
    unsigned long irq_flag;
    
    dbgmsg("DMA Module - Free DMA Channel %d - START\n", chan->chan_num);
    
    if ((ch >= GDMA_START) && (ch < (GDMA_START + NR_GDMA_CHANNEL))) {
        dma_chan[ch].registered = DMA_FALSE;
        mt_reset_dma(config);
        mt65xx_free_gdma(ch);
    } else {
        /* grab dma_drv_lock */
        spin_lock_irqsave(&dma_drv_lock, irq_flag);

        if (chan->chan_num < NR_GDMA_CHANNEL) {
            free_list_fs[ch] = free_list_head_fs;
            free_list_head_fs = ch;        
        } else if (chan->chan_num < (NR_GDMA_CHANNEL + NR_PDMA_CHANNEL)) {
            ch -= PDMA_START;
            free_list_hs[ch] = free_list_head_hs;
            free_list_head_hs = ch;    
        } else if (chan->chan_num < (NR_GDMA_CHANNEL + NR_PDMA_CHANNEL + NR_VFFDMA_CHANNEL)) {
        } else {
            printk(KERN_ERR"DMA Module - The DMA channel %d is not exist!\n", chan->chan_num);
        }
    
        dma_chan[chan->chan_num].registered = DMA_FALSE;

        mt_reset_dma(config);

        /* release dma_drv_lock */
        spin_unlock_irqrestore(&dma_drv_lock, irq_flag);
    }

    /* ===============DEBUG-START================ */
    #if(DMA_DEBUG == 1)
        switch(chan->type){
            case DMA_FULL_CHANNEL:
                break;
            case DMA_HALF_CHANNEL:
                ch += PDMA_START;
                break;
            case DMA_VIRTUAL_FIFO:
                break;
            default:
                break;
        }
        printk("DMA Module - Free DMA Channel %d - END\n", chan->chan_num);
    #endif
    /* ===============DEBUG-END================ */

    return;
}

DMA_STATUS mt_config_dma(struct mt_dma_conf *config, DMA_CONF_FLAG flag)
{
    DMA_CHAN *chan = (DMA_CHAN *)config;
    unsigned int dma_con = 0x0;
    DMA_STATUS ret = DMA_OK;

    if (!(config->sinc) && ((config->src) % 8)) {
        printk("General DMA fixed address mode requires 8-bytes aligned address\n");
        return DMA_FAIL;
    }
    if (!(config->dinc) && ((config->dst) % 8)) {
        printk("General DMA fixed address mode requires 8-bytes aligned address\n");
        return DMA_FAIL;
    }

    switch (flag) {
    case ALL:
        switch (chan->type) {
        case DMA_FULL_CHANNEL:
            writel(config->src, DMA_SRC(chan->baseAddr));
            writel(config->dst, DMA_DST(chan->baseAddr));
            writel(config->wppt, DMA_LEN2(chan->baseAddr));
            writel(config->wpto, DMA_JUMP_ADDR(chan->baseAddr));
            writel(config->count, DMA_LEN1(chan->baseAddr));

            if (config->wpen)
                dma_con |= DMA_CON_WPEN;

            if (config->wpsd)
                dma_con |= DMA_CON_WPSD;

            if (config->iten) {
                writel(DMA_INT_EN_BIT, DMA_INT_EN(chan->baseAddr));
            }

            dma_con |= config->burst;

            if (!(config->dinc)) {
                dma_con |= DMA_CON_DFIX;
                dma_con |= DMA_CON_WSIZE_1BYTE;
                dma_con &= ~(DMA_CON_BURST_MASK);
                dma_con |= DMA_CON_BURST_SINGLE;
            }
            if (!(config->sinc)) {
                dma_con |= DMA_CON_SFIX;
                dma_con |= DMA_CON_RSIZE_1BYTE;
                dma_con &= ~(DMA_CON_BURST_MASK);
                dma_con |= DMA_CON_BURST_SINGLE;
            }

            if (config->limiter) {
                dma_con |= (config->limiter) << DMA_CON_SLOW_OFFSET;
                dma_con |= DMA_CON_SLOW_EN;
            }

            writel(dma_con, DMA_CON(chan->baseAddr));

            break;
            
        case DMA_HALF_CHANNEL:
            writel(config->pgmaddr, DMA_MEM_ADDR(chan->baseAddr));
            writel(config->wppt, DMA_LEN2(chan->baseAddr));
            writel(config->wpto, DMA_JUMP_ADDR(chan->baseAddr));
            writel(config->count, DMA_LEN1(chan->baseAddr));

            if (config->dir)
                dma_con |= DMA_CON_DIR;

            if (config->iten) {
                writel(DMA_INT_EN_BIT, DMA_INT_EN(chan->baseAddr));
            }

            dma_con |= config->burst;

            if (config->limiter) {
                dma_con |= (config->limiter) << DMA_CON_SLOW_OFFSET;
                dma_con |= DMA_CON_SLOW_EN;
            }

            writel(dma_con, DMA_CON(chan->baseAddr));

            break;

        case DMA_VIRTUAL_FIFO:
            writel(config->pgmaddr, DMA_MEM_ADDR(chan->baseAddr));
            writel(config->count, DMA_LEN1(chan->baseAddr));
            writel(config->altlen, DMA_VFF_THRE(chan->baseAddr));
            writel(config->ffsize, DMA_VFF_LEN(chan->baseAddr));
            break;
            
        default:
            printk(KERN_ERR"DMA Module - Unknown DMA Channel Type!!\n");
            ret = DMA_FAIL;
            break;
        }

        break;

    case SRC:
        if (chan->type == DMA_FULL_CHANNEL) {
            writel(config->src, DMA_SRC(chan->baseAddr));
        } else {
            writel(config->pgmaddr, DMA_MEM_ADDR(chan->baseAddr));
            if (config->dir == 1) {
                config->dir = 0;
            }
        }
        break;
        
    case DST:
        if (chan->type == DMA_FULL_CHANNEL){
            writel(config->dst, DMA_DST(chan->baseAddr));
        } else {
            writel(config->pgmaddr, DMA_MEM_ADDR(chan->baseAddr));
            if (config->dir == 0) {
                config->dir = 1;
            }
        }
        break;

    case SRC_AND_DST:
        if (chan->type != DMA_FULL_CHANNEL){
            printk(KERN_ERR"DMA Module - Operation Not Supported For The \
            Given Channel Type");
            ret = DMA_FAIL;
            break;
        }
        writel(config->src, DMA_SRC(chan->baseAddr));
        writel(config->dst, DMA_DST(chan->baseAddr));
        break;

    default:
        break;
    }

    /* use the data synchronization barrier to ensure that all writes are completed */
    dsb();

    return ret;
}

void mt_start_dma(struct mt_dma_conf *config)
{
    DMA_CHAN *chan = (DMA_CHAN *)config;

    dbgmsg("DMA Module - Start DMA Transfer (channel %d)\n", chan->chan_num);

    writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(chan->baseAddr));
    mt65xx_reg_sync_writel(DMA_START_BIT, DMA_START(chan->baseAddr));

    /* ===============DEBUG-START================ */
    #if(DMA_DEBUG == 1)
        if(readl(DMA_START(chan->baseAddr))){
            printk("DMA Module - Start DMA Transfer: Success (channel %d)\n",chan->chan_num);
        }
        else{
            printk("DMA Module - Start DMA Transfer: Failed\n");
        }
    #endif
    /* ===============DEBUG-END================ */

    return;
}

void mt_stop_dma(struct mt_dma_conf *config)
{
    DMA_CHAN *chan = (DMA_CHAN *)config;
         
    dbgmsg("DMA Module - Stop DMA Transfer (channel %d)\n", chan->chan_num);

    writel(DMA_FLUSH_BIT, DMA_FLUSH(chan->baseAddr));
    while (readl(DMA_START(chan->baseAddr)));
    writel(DMA_FLUSH_CLR_BIT, DMA_FLUSH(chan->baseAddr));
    mt65xx_reg_sync_writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(chan->baseAddr));

    dbgmsg("DMA Module - Stop DMA Transfer: Success (channel %d)\n", chan->chan_num);
   
    return;
}

void mt_reset_dma(struct mt_dma_conf *config){

    memset(config, 0, sizeof(struct mt_dma_conf));

    if (mt_config_dma(config, ALL) != 0){
        return;
    }

    return;
}

DMA_STATUS mt_get_info(struct mt_dma_conf *config, INFO_TYPE type, INFO *info){

    DMA_CHAN *chan = (DMA_CHAN *)config;

    switch (type) {
    case REMAINING_LENGTH:
        if (chan->type == DMA_VIRTUAL_FIFO) {
            break;
        }
        *info = readl(DMA_LEN1(chan->baseAddr));
        return DMA_OK;
        
    case VF_READPTR:
        if (chan->type != DMA_VIRTUAL_FIFO) {
            break;
        }
        *info = readl(DMA_VFF_RPT(chan->baseAddr));
        return DMA_OK;
        
    case VF_WRITEPTR:
        if (chan->type != DMA_VIRTUAL_FIFO) {
            break;
        }
        *info = readl(DMA_VFF_WPT(chan->baseAddr));
        return DMA_OK;
        
    case VF_FFCNT:
        if (chan->type != DMA_VIRTUAL_FIFO) {
            break;
        }
        *info = readl(DMA_VFF_VALID_SIZE(chan->baseAddr));
        return DMA_OK;
        
    case VF_ALERT:
        /* fall through */
    case VF_EMPTY:
        /* fall through */
    case VF_FULL:
        return DMA_FAIL;

    case VF_PORT:
        if(chan->type != DMA_VIRTUAL_FIFO){
            break;
        }
        *info = DMA_VPORT_CH(chan->chan_num);
        return DMA_OK;
        
    default:
        break;
    }

    printk(KERN_ERR"DMA Module - Information Type Not Available!!\n");

    return DMA_FAIL;
}

static irqreturn_t __tcmfunc dma_irq_handler(int irq, void *dev_id)
{
    int i;
    DMA_CHAN *chan = NULL;
    unsigned glbsta = readl(DMA_GLOBAL_INT_FLAG);
    
    dbgmsg("DMA Module - ISR Start\n");      
    dbgmsg("DMA MODULE - GLBSTA0 = %x\n", glbsta0);
    dbgmsg("DMA MODULE - GLBSTA1 = %x\n", glbsta1);
 
    for (i = 0; i < NR_DMA; i++) {
        if (glbsta & DMA_GLBSTA_IT(i)){
            dbgmsg("DMA Module - Interrupt Issued: %d\n", i);
            chan = &dma_chan[i];
            if (dma_ctrl[i].isr_cb) {
                dma_ctrl[i].isr_cb(dma_ctrl[i].data);
            } else if (chan->config.callback) {
                chan->config.callback(chan->config.data);
            }
            mt65xx_reg_sync_writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(chan->baseAddr));
#if(DMA_DEBUG == 1)
            glbsta = readl(DMA_GLOBAL_INT_FLAG);
            printk("DMA_GLOBAL_INT_FLAG after ack: %x\n", glbsta);
#endif
        }    
    }

    dbgmsg("DMA Module - ISR END\n");
    
    return IRQ_HANDLED;
}

static int __init mt_init_dma(void)
{
    int i;
    struct DMA_CHAN *chan;

    spin_lock_init(&dma_drv_lock);

    for (i = 0; i < NR_DMA; i++) {
        chan = &(dma_chan[i]);
        chan->baseAddr = DMA_BASE_CH(i);
        chan->chan_num = i;
        chan->registered = DMA_FALSE;
          
        if (i < NR_GDMA_CHANNEL) {
            chan->type = DMA_FULL_CHANNEL;
        } else if (i < NR_GDMA_CHANNEL + NR_PDMA_CHANNEL) {
            chan->type = DMA_HALF_CHANNEL;
        } else {
            chan->type = DMA_VIRTUAL_FIFO;
        }

        mt_reset_dma(&chan->config);
        mt_stop_dma(&chan->config);
    }

    /* initialize free lists */
    free_list_head_fs = 0;
    for (i = 0; i < NR_GDMA_CHANNEL; i++) {
        free_list_fs[i] = i + 1;
    }
    free_list_head_hs = 0;
    for (i = 0; i < NR_PDMA_CHANNEL; i++) {
        free_list_hs[i] = i + 1;
    }

    if (request_irq(DMA_IRQ, (irq_handler_t)dma_irq_handler, IRQF_TRIGGER_LOW, "DMA",  NULL)) {
        printk(KERN_ERR"fail to request DMA IRQ\n");
    }

    #ifdef POWER_FEATURE
    hwEnableClock(MT65XX_PDN_PERI_APDMA, "APDMA");
    #else  
    mt65xx_reg_sync_writel(0x00000010, APCONFIG_BASE + 0x0318);
    #endif       

    return 0;
}

arch_initcall(mt_init_dma);

EXPORT_SYMBOL(mt_request_half_size_dma);
EXPORT_SYMBOL(mt_request_full_size_dma);
EXPORT_SYMBOL(mt_request_virtual_fifo_dma);
EXPORT_SYMBOL(mt_free_dma);
EXPORT_SYMBOL(mt_config_dma);
EXPORT_SYMBOL(mt_start_dma);
EXPORT_SYMBOL(mt_stop_dma);
EXPORT_SYMBOL(mt_reset_dma);
EXPORT_SYMBOL(mt_get_info);

