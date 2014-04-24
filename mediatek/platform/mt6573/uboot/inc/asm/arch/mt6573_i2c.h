
#ifndef __MT6573_I2C_H__
#define __MT6573_I2C_H__

#include <asm/arch/mt65xx.h>


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


#define PDN_CLR0 (0x70026318) 
#define PDN_SET0 (0x70026314) 


extern int mt6573_i2c_polling_read(int port, unsigned char addr, unsigned char *buffer, int len);
extern int mt6573_i2c_polling_write(int port, unsigned char addr, unsigned char *buffer, int len);

#endif /* __MT6573_I2C_H__ */

