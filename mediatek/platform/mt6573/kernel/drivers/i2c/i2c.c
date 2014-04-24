

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <asm/scatterlist.h>
#include <linux/scatterlist.h>
#include <asm/io.h>
//#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_devs.h>
#include <mach/mt6573_pll.h>
#include <mach/dma.h>
#include <asm/tcm.h>
#include <mach/mt6573_gpio.h>


#define DRV_NAME                    "mt6573-i2c"



enum i2c_trans_st_rs {
        I2C_TRANS_STOP = 0,
        I2C_TRANS_REPEATED_START,
};

enum mt6573_trans_op {
	I2C_MASTER_NONE = 0,
	I2C_MASTER_WR = 1,
	I2C_MASTER_RD,
	I2C_MASTER_WRRD,
};

//CONTROL
#define I2C_CONTROL_RS			(0x1 << 1)	
#define	I2C_CONTROL_DMA_EN		(0x1 << 2)	
#define	I2C_CONTROL_CLK_EXT_EN		(0x1 << 3)
#define	I2C_CONTROL_DIR_CHANGE		(0x1 << 4)
#define	I2C_CONTROL_ACKERR_DET_EN 	(0x1 << 5)
#define	I2C_CONTROL_TRANSFER_LEN_CHANGE (0x1 << 6)



#define I2C_WRITE_REG(base, offset, value) \
	__raw_writel(value, (base) + (offset))
#define I2C_READ_REG(base, offset) \
		__raw_readl((base) + (offset))



//the offset is based on 32-bit width
enum I2C_REGS_OFFSET {
	OFFSET_DATA_PORT = 0x0,			//0x0
	OFFSET_SLAVE_ADDR = 0x04,		//0x04
	OFFSET_INTR_MASK = 0x08,		//0x08
	OFFSET_INTR_STAT = 0x0C,		//0x0C
	OFFSET_CONTROL = 0x10,			//0X10
	OFFSET_TRANSFER_LEN = 0x14,		//0X14
	OFFSET_TRANSAC_LEN = 0x18,		//0X18
	OFFSET_DELAY_LEN = 0x1C,		//0X1C
	OFFSET_TIMING = 0x20,			//0X20
	OFFSET_START = 0x24,			//0X24
	OFFSET_FIFO_STAT = 0x30,		//0X30
	OFFSET_FIFO_THRESH = 0x34,		//0X34
	OFFSET_FIFO_ADDR_CLR = 0x38,		//0X38
	OFFSET_IO_CONFIG = 0x40,		//0X40
	OFFSET_RSV_DEBUG = 0x44,		//0X44
	OFFSET_HS = 0x48,			//0X48
	OFFSET_SOFTRESET = 0x50,		//0X50
	OFFSET_DEBUGSTAT = 0x64,		//0X64
	OFFSET_DEBUGCTRL = 0x68,		//0x68
};
enum DMA_REGS_OFFSET {	
	OFFSET_INT_FLAG = 0x0,
	OFFSET_INT_EN = 0x04,
	OFFSET_EN = 0x08,
	OFFSET_CON = 0x18,
	OFFSET_MEM_ADDR = 0x1c,
	OFFSET_LEN = 0x24,
};

#define I2C_TRANSFER_LEN(len, aux)	(((len) & 0xFF) | (((aux) & 0x1F) << 8))
#define I2C_TRANSAC_LEN(num)		((num) & 0xFF)
#define	I2C_FIFO_STAT_LEN(n)		(((n) >> 4) & 0x0F)



#define I2C_CLK_RATE			    15360			/* khz for CPU_416MHZ_MCU_104MHZ*/

#define I2C_FIFO_SIZE  	  			8

#define MAX_ST_MODE_SPEED			100	 /* khz */
#define MAX_FS_MODE_SPEED			400	 /* khz */
#define MAX_HS_MODE_SPEED	   		3400 /* khz */

#define MAX_DMA_TRANS_SIZE			252	/* Max(255) aligned to 4 bytes = 252 */
#define MAX_DMA_TRANS_NUM			256

#define MAX_SAMPLE_CNT_DIV			8
#define MAX_STEP_CNT_DIV			64
#define MAX_HS_STEP_CNT_DIV			8

#define mt6573_I2C_DATA_PORT		((base) + 0x0000)
#define mt6573_I2C_SLAVE_ADDR		((base) + 0x0004)
#define mt6573_I2C_INTR_MASK		((base) + 0x0008)
#define mt6573_I2C_INTR_STAT		((base) + 0x000c)
#define mt6573_I2C_CONTROL			((base) + 0x0010)
#define mt6573_I2C_TRANSFER_LEN	    ((base) + 0x0014)
#define mt6573_I2C_TRANSAC_LEN	    ((base) + 0x0018)
#define mt6573_I2C_DELAY_LEN		((base) + 0x001c)
#define mt6573_I2C_TIMING			((base) + 0x0020)
#define mt6573_I2C_START			((base) + 0x0024)
#define mt6573_I2C_FIFO_STAT		((base) + 0x0030)
#define mt6573_I2C_FIFO_THRESH	    ((base) + 0x0034)
#define mt6573_I2C_FIFO_ADDR_CLR	((base) + 0x0038)
#define mt6573_I2C_IO_CONFIG		((base) + 0x0040)
#define mt6573_I2C_DEBUG			((base) + 0x0044)
#define mt6573_I2C_HS				((base) + 0x0048)
#define mt6573_I2C_DEBUGSTAT		((base) + 0x0064)
#define mt6573_I2C_DEBUGCTRL		((base) + 0x0068)

#define mt6573_I2C_TRANS_LEN_MASK		(0xff)
#define mt6573_I2C_TRANS_AUX_LEN_MASK	(0x1f << 8)
#define mt6573_I2C_CONTROL_MASK			(0x3f << 1)

#define I2C_DEBUG					(1 << 3)
#define I2C_HS_NACKERR				(1 << 2)
#define I2C_ACKERR					(1 << 1)
#define I2C_TRANSAC_COMP			(1 << 0)

#define I2C_TX_THR_OFFSET			8
#define I2C_RX_THR_OFFSET			0

#define I2C_START_TRANSAC			__raw_writel(0x1,mt6573_I2C_START)
#define I2C_FIFO_CLR_ADDR			__raw_writel(0x1,mt6573_I2C_FIFO_ADDR_CLR)
#define I2C_FIFO_OFFSET				(__raw_readl(mt6573_I2C_FIFO_STAT)>>4&0xf)
#define I2C_FIFO_IS_EMPTY			(__raw_readw(mt6573_I2C_FIFO_STAT)>>0&0x1)

#define I2C_SET_BITS(BS,REG)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define I2C_CLR_BITS(BS,REG)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

#define I2C_SET_FIFO_THRESH(tx,rx)	\
	do { u32 tmp = (((tx) & 0x7) << I2C_TX_THR_OFFSET) | \
	               (((rx) & 0x7) << I2C_RX_THR_OFFSET); \
		 __raw_writel(tmp, mt6573_I2C_FIFO_THRESH); \
	} while(0)

#define I2C_SET_INTR_MASK(mask)		__raw_writel(mask, mt6573_I2C_INTR_MASK)

#define I2C_CLR_INTR_MASK(mask)		\
	do { u32 tmp = __raw_readl(mt6573_I2C_INTR_MASK); \
		 tmp &= ~(mask); \
		 __raw_writel(tmp, mt6573_I2C_INTR_MASK); \
	} while(0)

#define I2C_SET_SLAVE_ADDR(addr)	__raw_writel((addr)&0xFF, mt6573_I2C_SLAVE_ADDR)

#define I2C_SET_TRANS_LEN(len)	 	\
	do { u32 tmp = __raw_readl(mt6573_I2C_TRANSFER_LEN) & \
	                          ~mt6573_I2C_TRANS_LEN_MASK; \
		 tmp |= ((len) & mt6573_I2C_TRANS_LEN_MASK); \
		 __raw_writel(tmp, mt6573_I2C_TRANSFER_LEN); \
	} while(0)

#define I2C_SET_TRANS_AUX_LEN(len)	\
	do { u32 tmp = __raw_readl(mt6573_I2C_TRANSFER_LEN) & \
	                         ~(mt6573_I2C_TRANS_AUX_LEN_MASK); \
		 tmp |= (((len) << 8) & mt6573_I2C_TRANS_AUX_LEN_MASK); \
		 __raw_writel(tmp, mt6573_I2C_TRANSFER_LEN); \
	} while(0)

#define I2C_SET_TRANSAC_LEN(len)	__raw_writel(len, mt6573_I2C_TRANSAC_LEN)
#define I2C_SET_TRANS_DELAY(delay)	__raw_writel(delay, mt6573_I2C_DELAY_LEN)

#define I2C_SET_TRANS_CTRL(ctrl)	\
	do { u32 tmp = __raw_readl(mt6573_I2C_CONTROL) & ~mt6573_I2C_CONTROL_MASK; \
		tmp |= ((ctrl) & mt6573_I2C_CONTROL_MASK); \
		__raw_writel(tmp, mt6573_I2C_CONTROL); \
	} while(0)

#define I2C_SET_HS_MODE(on_off) \
	do { u32 tmp = __raw_readl(mt6573_I2C_HS) & ~0x1; \
	tmp |= (on_off & 0x1); \
	__raw_writel(tmp, mt6573_I2C_HS); \
	} while(0)

#define I2C_READ_BYTE(byte)		\
	do { byte = __raw_readb(mt6573_I2C_DATA_PORT); } while(0)
	
#define I2C_WRITE_BYTE(byte)	\
	do { __raw_writeb(byte, mt6573_I2C_DATA_PORT); } while(0)

#define I2C_CLR_INTR_STATUS(status)	\
		do { __raw_writew(status, mt6573_I2C_INTR_STAT); } while(0)

#define I2C_INTR_STATUS				__raw_readw(mt6573_I2C_INTR_STAT)

/* mt6573 i2c control bits */
#define TRANS_LEN_CHG 				(1 << 6)
#define ACK_ERR_DET_EN				(1 << 5)
#define DIR_CHG						(1 << 4)
#define CLK_EXT						(1 << 3)
#define	DMA_EN						(1 << 2)
#define	REPEATED_START_FLAG 		(1 << 1)
#define	STOP_FLAG					(0 << 1)

enum {
	ST_MODE,
	FS_MODE,
	HS_MODE,
};

struct mt6573_i2c {
	struct i2c_adapter	*adap;		/* i2c host adapter */
	struct device		*dev;		/* the device object of i2c host adapter */
	u32					base;		/* i2c base addr */
	u16					id;
	u16					irqnr;		/* i2c interrupt number */
	u16					irq_stat;	/* i2c interrupt status */
	spinlock_t			lock;		/* for mt6573_i2c struct protection */
	wait_queue_head_t	wait;		/* i2c transfer wait queue */

	atomic_t			trans_err;	/* i2c transfer error */
	atomic_t			trans_comp;	/* i2c transfer completion */
	atomic_t			trans_stop;	/* i2c transfer stop */

	unsigned long		clk;		/* host clock speed in khz */
	unsigned long		sclk;		/* khz */

	unsigned char		master_code;/* master code in HS mode */
	unsigned char		mode;		/* ST/FS/HS mode */

	enum  i2c_trans_st_rs st_rs;
	bool                dma_en;
	u32                 pdmabase;
	u16                 delay_len;
	enum mt6573_trans_op op;
	MT65XX_CLOCK        pdn;
};

typedef struct {
	u32 data:8;
	u32 reserved:24;
} i2c_data_reg;

typedef struct {
	u32 slave_addr:8;
	u32 reserved:24;
} i2c_slave_addr_reg;

typedef struct {
	u32 transac_comp:1;
	u32 ackerr:1;
	u32 hs_nackerr:1;
	u32 debug:1;
	u32 reserved:28;
} i2c_intr_mask_reg;

typedef struct {
	u32 transac_comp:1;
	u32 ackerr:1;
	u32 hs_nackerr:1;
	u32 reserved:29;
} i2c_intr_stat_reg;

typedef struct {
	u32 reserved1:1;
	u32 rs_stop:1;
	u32 dma_en:1;
	u32 clk_ext_en:1;
	u32 dir_chg:1;
	u32 ackerr_det_en:1;
	u32 trans_len_chg:1;
	u32 reserved2:25;
} i2c_control_reg;

typedef struct {
	u32 trans_len:8;
	u32 trans_aux_len:5;
	u32 reserved:19;
} i2c_trans_len_reg;

typedef struct {
	u32 transac_len:8;
	u32 reserved:24;
} i2c_transac_len_reg;

typedef struct {
	u32 delay_len:8;
	u32 reserved:24;
} i2c_delay_len_reg;

typedef struct {
	u32 step_cnt_div:6;
	u32 reserved1:2;
	u32 sample_cnt_div:3;
	u32 reserved2:1;
	u32 data_rd_time:3;
	u32 data_rd_adj:1;
	u32 reserved3:16;
} i2c_timing_reg;

typedef struct {
	u32 start:1;
	u32 reserved:31;
} i2c_start_reg;

typedef struct {
	u32 rd_empty:1;
	u32 wr_full:1;
	u32 reserved1:2;
	u32 fifo_offset:4;
	u32 wr_addr:4;
	u32 rd_addr:4;
	u32 reserved3:16;
} i2c_fifo_stat_reg;

typedef struct {
	u32 rx_trig:3;
	u32 reserved1:5;
	u32 tx_trig:3;
	u32 reserved2:21;
} i2c_fifo_thresh_reg;

typedef struct {
	u32 scl_io_cfg:1;
	u32 sda_io_cfg:1;
	u32 io_sync_en:1;
	u32 reserved2:29;
} i2c_io_config_reg;

typedef struct {
	u32 debug:3;
	u32 reserved2:29;
} i2c_debug_reg;

typedef struct {
	u32 hs_en:1;
	u32 hs_nackerr_det_en:1;
	u32 reserved1:2;
	u32 master_code:3;
	u32 reserved2:1;
	u32 hs_step_cnt_div:3;
	u32 reserved3:1;
	u32 hs_sample_cnt_div:3;
	u32 reserved4:17;
} i2c_hs_reg;

typedef struct {
	u32 master_stat:4;
	u32 master_rd:1;
	u32 master_wr:1;
	u32 bus_busy:1;	
	u32 reserved:25;
} i2c_dbg_stat_reg;

typedef struct {
	u32 fifo_apb_dbg:1;
	u32 apb_dbg_rd:1;
	u32 reserved:30;
} i2c_dbg_ctrl_reg;

typedef struct {
	i2c_data_reg		*data;
	i2c_slave_addr_reg  *slave_addr;
	i2c_intr_mask_reg   *intr_mask;
	i2c_intr_stat_reg   *intr_stat;
	i2c_control_reg     *control;
	i2c_trans_len_reg   *trans_len;
	i2c_transac_len_reg *transac_len;
	i2c_delay_len_reg   *delay_len;
	i2c_timing_reg      *timing;
	i2c_start_reg       *start;
	i2c_fifo_stat_reg   *fifo_stat;
	i2c_fifo_thresh_reg *fifo_thresh;
	i2c_io_config_reg   *io_config;
	i2c_debug_reg       *debug;
	i2c_hs_reg			*hs;
	i2c_dbg_stat_reg    *dbg_stat;
	i2c_dbg_ctrl_reg    *dbg_ctrl;
} i2c_regs;

//extern BOOL hwEnableClock(mt6573_CLOCK clockId);
//extern BOOL hwDisableClock(mt6573_CLOCK clockId);

static int mt6573_i2c_set_speed(struct mt6573_i2c *i2c, int mode, unsigned long khz);

inline static void mt6573_i2c_power_up(struct mt6573_i2c *i2c)
{
    //2010/07/27: mt6573, the I2C power on is controlled by APMCU_CG_CLR1
    //            Bit8: I2C
    //            Bit9: I2C2
    #define PDN_CLR0 (0xF7026318)  
    u32 pwrbit[] = {8, 9};
    unsigned int poweron = 1 << pwrbit[i2c->id];
    I2C_SET_BITS(poweron, PDN_CLR0);
}

inline static void mt6573_i2c_power_down(struct mt6573_i2c *i2c)
{
    //2010/07/27: mt6573, the I2C power on is controlled by APMCU_CG_SET1
    //            Bit8: I2C
    //            Bit9: I2C2
    #define PDN_SET0 (0xF7026314)  
    u32 pwrbit[] = {8, 9};
    unsigned int poweroff = 1 << pwrbit[i2c->id];
    I2C_SET_BITS(poweroff, PDN_SET0);
}
static u32 mt6573_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_EMUL;
}

static void mt6573_i2c_post_isr(struct mt6573_i2c *i2c, u16 addr)
{

	if (i2c->irq_stat & I2C_TRANSAC_COMP) {
		atomic_set(&i2c->trans_err, 0);
			atomic_set(&i2c->trans_comp, 1);
	}
	
	if (i2c->irq_stat & I2C_HS_NACKERR) {
		if (likely(!(addr & I2C_A_FILTER_MSG)))
			dev_err(i2c->dev, "I2C_HS_NACKERR\n");
		}
	if (i2c->irq_stat & I2C_ACKERR) {
		if (likely(!(addr & I2C_A_FILTER_MSG)))
			dev_err(i2c->dev, "I2C_ACKERR\n");
		}
	atomic_set(&i2c->trans_err, i2c->irq_stat & (I2C_HS_NACKERR | I2C_ACKERR));
}

static int mt6573_i2c_start_xfer(struct mt6573_i2c *i2c, struct i2c_msg *msg)
{
    u32 base = i2c->base;
	u16 addr = msg->addr;
	u16 flags = msg->flags;
	u16 read = (flags & I2C_M_RD);
	u16 len = msg->len;
	u8 *ptr = msg->buf;
	long tmo = i2c->adap->timeout;
	int speedret = 0;
	int ret = len;
	long old_ioconfig;

	u16 trans_len;
	u16 trans_auxlen;
	u16 trans_num;
	u16 data_size;
	u16 control;
	s32 scl_gpio;
	s32 sda_gpio;

	if(0 == (msg->timing & 0x0fff))
		speedret = mt6573_i2c_set_speed(i2c, ST_MODE, MAX_ST_MODE_SPEED);
    else{
		if (unlikely(msg->addr & I2C_HS_FLAG)) 
			speedret = mt6573_i2c_set_speed(i2c, HS_MODE, msg->timing & 0x0fff);
		else
			speedret = mt6573_i2c_set_speed(i2c, FS_MODE, msg->timing & 0x0fff);
    }
  if(-ENOTSUPP == speedret) 
  	dev_err(i2c->dev, "mt6573-i2c:(%x) speed set is invalid.\n", msg->addr);

	if(unlikely(msg->addr & I2C_RS_FLAG)) 
		i2c->st_rs = I2C_TRANS_REPEATED_START;
	else
		i2c->st_rs = I2C_TRANS_STOP;

	if(unlikely(msg->addr & I2C_DMA_FLAG)) 
			i2c->dma_en = true;
	else
		i2c->dma_en = false;
	i2c->delay_len = (msg->timing & 0xff000) >> 12;
	if(I2C_TRANS_STOP == i2c->st_rs) {
		if((i2c->delay_len) >> 8)
			dev_err(i2c->dev, "mt6573-i2c:(%x) the delay_len is too long.\n", msg->addr);
	}
	if(0 == i2c->delay_len)
		i2c->delay_len = 2;

	if (unlikely(msg->addr & I2C_WR_FLAG))
		i2c->op = I2C_MASTER_WRRD;

	else
	{
	   if(msg->flags & I2C_M_RD)
	   		i2c->op = I2C_MASTER_RD;
	   else
	   		i2c->op = I2C_MASTER_WR;
	}

	if(I2C_MASTER_WRRD != i2c->op && I2C_MASTER_WR != i2c->op && I2C_MASTER_RD != i2c->op)
		dev_err(i2c->dev, "mt6573-i2c:(%x) op is invalid.\n", msg->addr);

	if(!msg->buf){
		dev_err(i2c->dev, "mt6573-i2c:(%x) data buffer is NULL.\n", msg->addr);
	}

	if(false == i2c->dma_en) {
		if(I2C_MASTER_WRRD != i2c->op) {
			trans_len = (msg->len) & 0xFF;	
			trans_num = (msg->len >> 8) & 0xFF;	
			if(0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len*trans_num;
			if(!trans_len || !trans_num || trans_len*trans_num > 8) {
				dev_err(i2c->dev, "mt6573-i2c:(%x) non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", msg->addr, trans_len, trans_num, trans_auxlen);
			}			
		} else {
			trans_len = (msg->len) & 0xFF;	
			trans_auxlen = (msg->len >> 8) & 0xFF;	
			trans_num = 2;
			data_size = trans_len;
			if(!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8) {
				dev_err(i2c->dev, "mt6573-i2c:(%x) WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", msg->addr, trans_len, trans_num, trans_auxlen);
			}
		}
	} else {
		if(I2C_MASTER_WRRD != i2c->op) {
			trans_len = (msg->len) & 0xFF;	
			trans_num = (msg->len >> 8) & 0xFF;	
			if(0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len*trans_num;
			if(!trans_len || !trans_num || trans_len > 255 || trans_num > 255) {
				dev_err(i2c->dev, "mt6573-i2c:(%x) DMA non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", msg->addr, trans_len, trans_num, trans_auxlen);
			}			
		} else {
			dev_err(i2c->dev, "mt6573-i2c:(%x) DMA does not support WRRD.\n", msg->addr);
		}
	}

	atomic_set(&i2c->trans_stop, 0);
	atomic_set(&i2c->trans_comp, 0);
	atomic_set(&i2c->trans_err, 0);


	old_ioconfig = __raw_readw(mt6573_I2C_IO_CONFIG);
	old_ioconfig |= 1;
	__raw_writew(old_ioconfig, mt6573_I2C_IO_CONFIG);

	if(!addr)
		dev_err(i2c->dev, "mt6573-i2c:(%x) addr is invalid.\n", msg->addr);

	addr = read ? (addr | 0x1) : (addr & ~0x1);
	control = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN;

	if(i2c->dma_en) {
		control |= I2C_CONTROL_DMA_EN;
	}

	if(I2C_MASTER_WRRD == i2c->op)
		control |= I2C_CONTROL_DIR_CHANGE;

	if(HS_MODE == i2c->mode || (trans_num > 1 && I2C_TRANS_REPEATED_START == i2c->st_rs)) {
		control |= I2C_CONTROL_RS;
	}


	I2C_SET_SLAVE_ADDR(addr);
	I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
	I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_FIFO_CLR_ADDR;
	I2C_WRITE_REG(base, OFFSET_CONTROL, control);
	if(control & I2C_CONTROL_RS){	// bit is set to 1, i.e.,use repeated stop
		I2C_WRITE_REG(base, OFFSET_DELAY_LEN, i2c->delay_len);
	}
	I2C_WRITE_REG(base, OFFSET_TRANSFER_LEN, I2C_TRANSFER_LEN(trans_len, trans_auxlen));
	I2C_WRITE_REG(base, OFFSET_TRANSAC_LEN, I2C_TRANSAC_LEN(trans_num));

//	if(i2c->dma_en)
//		hwEnableClock(MT65XX_PDN_PERI_APDMA, "i2c");
	
//		I2C_WRITE_REG(APCONFIG_BASE, 0x318, 1<<4);
//		I2C_WRITE_REG(APCONFIG_BASE, 0x03c, 0<<3);
//	dev_err(i2c->dev, "Power register:\nCG %x\n",(__raw_readl(0xf7026310)));


	if (I2C_MASTER_RD == i2c->op) {
		if(i2c->dma_en) {
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_INT_FLAG, 0x0000);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_CON, 0x0001);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_MEM_ADDR, (u32)ptr);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_LEN, data_size);
//				dev_info(i2c->dev, "addr %.2x dma %.2X byte\n", addr, data_size);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_EN, 0x0001);
		}
	
		I2C_START_TRANSAC;
		tmo = wait_event_interruptible_timeout(
				i2c->wait,
				atomic_read(&i2c->trans_stop),
				tmo);
		mt6573_i2c_post_isr(i2c, addr);
		

		if(!i2c->dma_en) {
			if (atomic_read(&i2c->trans_comp)) {
				data_size = I2C_FIFO_STAT_LEN(I2C_READ_REG(base, OFFSET_FIFO_STAT));
				while (data_size--) {
					I2C_READ_BYTE(*ptr);
//					dev_info(i2c->dev, "addr %.2x read byte = 0x%.2X\n", addr, *ptr);
					ptr++;
				}
			}
		}
	}
	else if(I2C_MASTER_WR == i2c->op)
	{
		if(i2c->dma_en) {
			if((u32)ptr > 0xc0000000)
				dev_err(i2c->dev, "mt6573-i2c:(%x) DMA mode should use physical buffer address!\n", msg->addr);
			
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_INT_FLAG, 0x0000);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_CON, 0x0000);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_MEM_ADDR, (u32)ptr);
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_LEN, data_size);
//				dev_info(i2c->dev, "addr %.2x dma %.2X byte\n", addr, data_size);	
			I2C_WRITE_REG(i2c->pdmabase, OFFSET_EN, 0x0001);

		}			
		
		if(!i2c->dma_en) {
			while (data_size--) {
				I2C_WRITE_BYTE(*ptr);
//				dev_info(i2c->dev, "addr %.2x write byte = 0x%.2X\n", addr, *ptr);
				ptr++;
			}
		}

		I2C_START_TRANSAC;
		tmo = wait_event_interruptible_timeout(i2c->wait, 
		                               atomic_read(&i2c->trans_stop), tmo);
		mt6573_i2c_post_isr(i2c, addr);
	}
	else
	{
		while (data_size--) {
			I2C_WRITE_BYTE(*ptr);
//					dev_info(i2c->dev, "addr %.2x write byte = 0x%.2X\n", addr, *ptr);
			ptr++;
		}
		I2C_START_TRANSAC;
		tmo = wait_event_interruptible_timeout(i2c->wait, 
		                               atomic_read(&i2c->trans_stop), tmo);
		mt6573_i2c_post_isr(i2c, addr);
		
		ptr = msg->buf;

		if (atomic_read(&i2c->trans_comp)) {
			data_size = I2C_FIFO_STAT_LEN(I2C_READ_REG(base, OFFSET_FIFO_STAT));
			while (data_size--) {
				I2C_READ_BYTE(*ptr);
//						dev_info(i2c->dev, "addr %.2x read byte = 0x%.2X\n", addr, *ptr);
				ptr++;
			}
		}
	}

	if(i2c->dma_en)
//		hwDisableClock(MT65XX_PDN_PERI_APDMA, "i2c");
//		I2C_WRITE_REG(APCONFIG_BASE, 0x314, 1<<4);

	if (unlikely(msg->addr & 0x0200)) {
		__raw_writew(old_ioconfig, mt6573_I2C_IO_CONFIG);
	}
	
	I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

	if (tmo == 0) {
		if(0 == i2c->id)
		{

				scl_gpio = mt_get_gpio_in(GPIO_I2C0_SCA_PIN);
				sda_gpio = mt_get_gpio_in(GPIO_I2C0_SDA_PIN);
		}
		else			
		{

				scl_gpio = mt_get_gpio_in(GPIO_I2C1_SCA_PIN);
				sda_gpio = mt_get_gpio_in(GPIO_I2C1_SDA_PIN);
		}	
		if (likely(!(addr & I2C_A_FILTER_MSG)))
		{
			dev_err(i2c->dev, "addr: %.2x, transfer timeout\n", msg->addr);
			dev_err(i2c->dev, "I2C structure:\nMode %x\nSt_rs %x\nDma_en %x\nOp %x\nTrans_len %x\nTrans_num %x\nTrans_auxlen %x\nData_size %x\nIrq_stat %x\nTrans_stop %d\nTrans_comp %d\nTrans_error %d\n", i2c->mode, i2c->st_rs, i2c->dma_en, i2c->op, trans_len, trans_num, trans_auxlen, data_size, i2c->irq_stat, atomic_read(&i2c->trans_stop), atomic_read(&i2c->trans_comp), atomic_read(&i2c->trans_err));
			dev_err(i2c->dev, "I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n",(__raw_readl(mt6573_I2C_SLAVE_ADDR)),(__raw_readl(mt6573_I2C_INTR_MASK)),(__raw_readl(mt6573_I2C_INTR_STAT)),(__raw_readl(mt6573_I2C_CONTROL)),(__raw_readl(mt6573_I2C_TRANSFER_LEN)),(__raw_readl(mt6573_I2C_TRANSAC_LEN)),(__raw_readl(mt6573_I2C_DELAY_LEN)),(__raw_readl(mt6573_I2C_TIMING)),(__raw_readl(mt6573_I2C_START)),(__raw_readl(mt6573_I2C_FIFO_STAT)),(__raw_readl(mt6573_I2C_IO_CONFIG)),(__raw_readl(mt6573_I2C_HS)),(__raw_readl(mt6573_I2C_DEBUGSTAT)));
      dev_err(i2c->dev, "DMA register:\nINT_FLAG %x\nCON %x\nMEM_ADDR %x\nLEN %x\nINT_EN %x\nEN %x\n",(__raw_readl(i2c->pdmabase+OFFSET_INT_FLAG)),(__raw_readl(i2c->pdmabase+OFFSET_CON)),(__raw_readl(i2c->pdmabase+OFFSET_MEM_ADDR)),(__raw_readl(i2c->pdmabase+OFFSET_LEN)),(__raw_readl(i2c->pdmabase+OFFSET_INT_EN)),(__raw_readl(i2c->pdmabase+OFFSET_EN)));
			dev_err(i2c->dev, "Power register:\nCG %x\n",(__raw_readl(0xf7026310)));
			dev_err(i2c->dev, "GPIO status:\nCG scl is %x and sda is %x\n", scl_gpio, sda_gpio);
		}

		I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
		I2C_SET_SLAVE_ADDR(0x00);
		I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
//		I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
		I2C_FIFO_CLR_ADDR;

		ret = -ETIMEDOUT;
	}
	else if (atomic_read(&i2c->trans_err)) {
		if(0 == i2c->id)
		{
				scl_gpio = mt_get_gpio_in(GPIO_I2C0_SCA_PIN);
				sda_gpio = mt_get_gpio_in(GPIO_I2C0_SDA_PIN);
		}
		else			
		{
				scl_gpio = mt_get_gpio_in(GPIO_I2C1_SCA_PIN);
				sda_gpio = mt_get_gpio_in(GPIO_I2C1_SDA_PIN);
		}	
		if (likely(!(addr & I2C_A_FILTER_MSG)))
		{
			dev_err(i2c->dev, "addr: %.2x, transfer error\n", msg->addr);
			dev_err(i2c->dev, "I2C structure:\nMode %x\nSt_rs %x\nDma_en %x\nOp %x\nTrans_len %x\nTrans_num %x\nTrans_auxlen %x\nData_size %x\nIrq_stat %x\nTrans_stop %d\nTrans_comp %d\nTrans_error %d\n", i2c->mode, i2c->st_rs, i2c->dma_en, i2c->op, trans_len, trans_num, trans_auxlen, data_size, i2c->irq_stat, atomic_read(&i2c->trans_stop), atomic_read(&i2c->trans_comp), atomic_read(&i2c->trans_err));
			dev_err(i2c->dev, "I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n",(__raw_readl(mt6573_I2C_SLAVE_ADDR)),(__raw_readl(mt6573_I2C_INTR_MASK)),(__raw_readl(mt6573_I2C_INTR_STAT)),(__raw_readl(mt6573_I2C_CONTROL)),(__raw_readl(mt6573_I2C_TRANSFER_LEN)),(__raw_readl(mt6573_I2C_TRANSAC_LEN)),(__raw_readl(mt6573_I2C_DELAY_LEN)),(__raw_readl(mt6573_I2C_TIMING)),(__raw_readl(mt6573_I2C_START)),(__raw_readl(mt6573_I2C_FIFO_STAT)),(__raw_readl(mt6573_I2C_IO_CONFIG)),(__raw_readl(mt6573_I2C_HS)),(__raw_readl(mt6573_I2C_DEBUGSTAT)));
      dev_err(i2c->dev, "DMA register:\nINT_FLAG %x\nCON %x\nMEM_ADDR %x\nLEN %x\nINT_EN %x\nEN %x\n",(__raw_readl(i2c->pdmabase+OFFSET_INT_FLAG)),(__raw_readl(i2c->pdmabase+OFFSET_CON)),(__raw_readl(i2c->pdmabase+OFFSET_MEM_ADDR)),(__raw_readl(i2c->pdmabase+OFFSET_LEN)),(__raw_readl(i2c->pdmabase+OFFSET_INT_EN)),(__raw_readl(i2c->pdmabase+OFFSET_EN)));
			dev_err(i2c->dev, "Power register:\nCG %x\n",(__raw_readl(0xf7026310)));
			dev_err(i2c->dev, "GPIO status:\nCG scl is %x and sda is %x\n", scl_gpio, sda_gpio);
		}

		I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
		I2C_SET_SLAVE_ADDR(0x00);
		I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
//		I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
		I2C_FIFO_CLR_ADDR;

		ret = -EREMOTEIO;
	}

	if(ret < 0)
		return -EAGAIN;
	return ret;
}



static __tcmfunc irqreturn_t mt6573_i2c_irq(int irqno, void *dev_id)
{
	struct mt6573_i2c *i2c;
	u32 base;

	i2c = (struct mt6573_i2c*)dev_id;
	base = i2c->base;
	I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

	i2c->irq_stat = I2C_INTR_STATUS;
//	dev_info(i2c->dev, "I2C interrupt status 0x%04X\n", i2c->irq_stat);


	atomic_set(&i2c->trans_stop, 1);
	wake_up_interruptible(&i2c->wait);

	I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);

	return IRQ_HANDLED;
}

static int mt6573_i2c_do_transfer(struct mt6573_i2c *i2c, struct i2c_msg *msgs, int num)
{
	int ret = 0;
	int left_num = num;

	while (left_num--) {
			ret = mt6573_i2c_start_xfer(i2c, msgs++);
		if (ret < 0)
			return -EAGAIN;
	}
//    the return value is number of executed messages
	return num;
}

static int mt6573_i2c_transfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct mt6573_i2c *i2c = i2c_get_adapdata(adap);
	
	int	retry;
	int	ret = 0;
	

	hwEnableClock(i2c->pdn, "i2c");
//    mt6573_i2c_power_up(i2c);

	for (retry = 0; retry < adap->retries; retry++) {

		ret = mt6573_i2c_do_transfer(i2c, msgs, num);
		if (ret != -EAGAIN) {
			break;
		}

		dev_info(i2c->dev, "Retrying transmission (%d)\n", retry);
		udelay(100);
	}
	
//	mt6573_i2c_power_down(i2c);
	hwDisableClock(i2c->pdn, "i2c");
	if (ret != -EAGAIN)
		return ret;
	else
		return -EREMOTEIO;
}


static void mt6573_i2c_free(struct mt6573_i2c *i2c)
{
	if (!i2c)
		return;

	free_irq(i2c->irqnr, i2c);
	if (i2c->adap)
		i2c_del_adapter(i2c->adap);
	kfree(i2c);
}

static int mt6573_i2c_set_speed(struct mt6573_i2c *i2c, int mode, unsigned long khz)
{
	u32 base = i2c->base;
	int ret = 0;
	unsigned short sample_cnt_div = 0;
	unsigned short step_cnt_div = 0;
	unsigned short max_step_cnt_div = (mode == HS_MODE) ? 
	                                  MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
	unsigned long tmp, sclk, hclk = i2c->clk;
	
	if((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) || 
		   (mode == HS_MODE && khz > MAX_HS_MODE_SPEED))
	{
		dev_err(i2c->dev, "mt6573-i2c: the speed is too fast for this mode.\n");
		ret = -ENOTSUPP;
		goto end;
	}

  if((mode == i2c->mode) && (khz == i2c->sclk))
  	return 0;

	spin_lock(&i2c->lock);

	{
		unsigned long diff, min_diff = I2C_CLK_RATE;
		unsigned short sample_div = MAX_SAMPLE_CNT_DIV;
		unsigned short step_div = max_step_cnt_div;
		for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
			for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
				sclk = (hclk >> 1) / (sample_cnt_div * step_cnt_div);
				if (sclk > khz) 
					continue;
				diff = khz - sclk;
				if (diff < min_diff) {
					min_diff = diff;
					sample_div = sample_cnt_div;
					step_div   = step_cnt_div;
				}											
			}
		}
		sample_cnt_div = sample_div;
		step_cnt_div   = step_div;
	}

	sclk = hclk / (2 * sample_cnt_div * step_cnt_div);
	if (sclk > khz) {
		dev_err(i2c->dev, "%s mode: unsupported speed (%ldkhz)\n", 
	           (mode == HS_MODE) ? "HS" : "ST/FT", khz);
		ret = -ENOTSUPP;
		goto end;
	}
	step_cnt_div--;
	sample_cnt_div--;

	if (mode == HS_MODE) {
		
		tmp  = __raw_readw(mt6573_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp  = (0 & 0x7) << 8 | (16 & 0x3f) << 0 | tmp;
		__raw_writew(tmp, mt6573_I2C_TIMING);

		tmp  = __raw_readw(mt6573_I2C_HS) & ~((0x7 << 12) | (0x7 << 8));
		tmp  = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
		__raw_writew(tmp, mt6573_I2C_HS);
		I2C_SET_HS_MODE(1);
	}
	else {
		tmp  = __raw_readw(mt6573_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp  = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0 | tmp;
		__raw_writew(tmp, mt6573_I2C_TIMING);
		I2C_SET_HS_MODE(0);
	}
	i2c->mode = mode;
	i2c->sclk = khz;
//	dev_info(i2c->dev, "mt6573-i2c: set sclk to %ldkhz (orig: %ldkhz), sample=%d, step=%d\n", sclk, khz, sample_cnt_div, step_cnt_div);
end:
	spin_unlock(&i2c->lock);
	return ret;
}

static void mt6573_i2c_init_hw(struct mt6573_i2c *i2c)
{
    u32 base = i2c->base;
//    if (i2c->id == 2) {
//	dev_dbg(i2c->dev, "mt6573_i2c_init_hw(1):setup SCL_IO_CONFIG\n");
//	__raw_writew(0x0001, mt6573_I2C_IO_CONFIG);     //setup SCL_IO_CONFIG
//    }
	I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
}

static struct i2c_algorithm mt6573_i2c_algorithm = {    
    .master_xfer   = mt6573_i2c_transfer,
	.smbus_xfer    = NULL,
	.functionality = mt6573_i2c_functionality,
};

static struct i2c_adapter mt6573_i2c_adaptor[] = {
    {
        .id                = 0,
     	.owner             = THIS_MODULE,
    	.name			   = "mt6573-i2c",
    	.algo			   = &mt6573_i2c_algorithm,
    	.algo_data         = NULL,
//    	.client_register   = NULL,
//    	.client_unregister = NULL,
    	.timeout           = 2 * HZ,
    	.retries		   = 1, 
    },
    {
        .id                = 1,
     	.owner             = THIS_MODULE,
    	.name			   = "mt6573-i2c",
    	.algo			   = &mt6573_i2c_algorithm,
    	.algo_data         = NULL,
//    	.client_register   = NULL,
//    	.client_unregister = NULL,
    	.timeout           = 2 * HZ,
    	.retries		   = 1, 
    },    
};

static int mt6573_i2c_probe(struct platform_device *pdev)
{
	int ret, irq;

	struct mt6573_i2c *i2c = NULL;

	

	/* Request IO memory */
	if (!request_mem_region(pdev->resource[0].start,
				            pdev->resource[0].end - pdev->resource[0].start + 1, 
				            pdev->name)) {
		return -EBUSY;
	}
	
	if (NULL == (i2c = kzalloc(sizeof(struct mt6573_i2c), GFP_KERNEL))) 
		return -ENOMEM;


	/* initialize mt6573_i2c structure */
    irq = pdev->resource[1].start;
    i2c->id = pdev->id;
    i2c->base = pdev->resource[0].start;
    i2c->irqnr = irq;
	i2c->clk    = I2C_CLK_RATE;	
	i2c->adap   = &mt6573_i2c_adaptor[pdev->id];
	i2c->dev    = &mt6573_i2c_adaptor[pdev->id].dev;
	i2c->adap->dev.parent = &pdev->dev;
	
	if(0 == i2c->id)
	{
		i2c->pdmabase = AP_DMA_BASE + 0x0480;
		i2c->pdn = MT65XX_PDN_PERI_I2C;
	}
	else
	{
		i2c->pdmabase = AP_DMA_BASE + 0x0500;
		i2c->pdn = MT65XX_PDN_PERI_I2C2;
	}
	spin_lock_init(&i2c->lock);	
	init_waitqueue_head(&i2c->wait);

	ret = request_irq(irq, (irq_handler_t)mt6573_i2c_irq, 0, DRV_NAME, i2c);

	if (ret){
		dev_err(&pdev->dev, "Can Not request I2C IRQ %d\n", irq);
		goto free;
	}

    mt6573_i2c_init_hw(i2c);

	i2c_set_adapdata(i2c->adap, i2c);

	ret = i2c_add_adapter(i2c->adap);

	if (ret){
		dev_err(&pdev->dev, "failed to add i2c bus to i2c core\n");
		goto free;
	}

	platform_set_drvdata(pdev, i2c);
	

	return ret;
	
free:
	mt6573_i2c_free(i2c);
	return ret;
}

static int mt6573_i2c_remove(struct platform_device *pdev)
{
	struct mt6573_i2c *i2c = platform_get_drvdata(pdev);
	
	if (i2c) {
		platform_set_drvdata(pdev, NULL);
		mt6573_i2c_free(i2c);
	}

	return 0;
}

#ifdef CONFIG_PM
static int mt6573_i2c_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct mt6573_i2c *i2c = platform_get_drvdata(pdev); 
    dev_dbg(i2c->dev,"[I2C %d] Suspend!\n", i2c->id);
    
    #if 0
    struct mt6573_i2c *i2c = dev_get_drvdata(pdev);    
    
    if (i2c) {
        dev_dbg(i2c->dev,"[I2C] Suspend!\n");
        /* Check if i2c bus is already in used by other modules or not.
         * Parent module should be stopped or wait for i2c bus completed before
         * entering i2c suspend mode.
         */
        WARN_ON(PDN_Get_Peri_Status(PDN_PERI_I2C) == KAL_FALSE);        
		while (PDN_Get_Peri_Status(PDN_PERI_I2C) == KAL_FALSE)
			msleep(10);
    }
    #endif
    return 0;
}

static int mt6573_i2c_resume(struct platform_device *pdev)
{
    struct mt6573_i2c *i2c = platform_get_drvdata(pdev); 
    dev_dbg(i2c->dev,"[I2C %d] Resume!\n", i2c->id);
    
    #if 0
    struct mt6573_i2c *i2c = dev_get_drvdata(pdev);

    if (i2c) {
        dev_dbg(i2c->dev,"[I2C] Resume!\n");
        mt6573_i2c_init_hw(i2c);
    }
    #endif
    return 0;
}
#endif

/* device driver for platform bus bits */
static struct platform_driver mt6573_i2c_driver = {
	.probe	 = mt6573_i2c_probe,
	.remove	 = mt6573_i2c_remove,
#ifdef CONFIG_PM
	.suspend = mt6573_i2c_suspend,
	.resume	 = mt6573_i2c_resume,
#endif
	.driver  = {
        .name  = DRV_NAME,
        .owner = THIS_MODULE,
    },
};

static int __init mt6573_i2c_init(void)
{
	int ret;

	ret = platform_driver_register(&mt6573_i2c_driver);

	return ret;
}

static void __exit mt6573_i2c_exit(void)
{
	platform_driver_unregister(&mt6573_i2c_driver);
}

module_init(mt6573_i2c_init);
module_exit(mt6573_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek mt6573 I2C Bus Driver");
MODULE_AUTHOR("Infinity Chen <infinity.chen@mediatek.com>");

