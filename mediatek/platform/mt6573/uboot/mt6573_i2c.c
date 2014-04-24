

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
//#include <asm/arch/mt65xx_pdn_sw.h>
#include <asm/arch/mt6573_i2c.h>
#include <asm/mach-types.h>


int mt6573_i2c_polling_read(int port, unsigned char addr, unsigned char *buffer, int len)
{
  u32   base;
	u8   *ptr  = buffer;
	int   ret  = len;
	long  tmo = 0xffff;    
  unsigned short status; 
  unsigned long tmp;
  
  if(port > 1)
  	printf("mt6573-i2c polling: No this port.\n");
  else if(0 == port)
  	base = 0x7000B000;
  else
  	base = 0x7000D000;
	
	if(len > I2C_FIFO_SIZE)
		printf("mt6573-i2c polling: len is too long.\n");

    /* CHECKME. mt6573 doesn't support len = 0. */
	if (!len)
	{
		printf("mt6573-i2c polling: data buffer is NULL.\n");
		return 0;
	}

    /* bit 0 is to indicate read REQ or write REQ */
	addr = (addr | 0x1);
	
	if(0 == port)
		I2C_SET_BITS(0x1<<0x8, PDN_CLR0);
	else
		I2C_SET_BITS(0x1<<0x9, PDN_CLR0);
		
	tmp  = __raw_readw(mt6573_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
	tmp  = (6 & 0x7) << 8 | (10 & 0x3f) << 0 | tmp;
	__raw_writew(tmp, mt6573_I2C_TIMING);
	I2C_SET_HS_MODE(0);

    /* control registers */
	I2C_SET_SLAVE_ADDR(addr);
	I2C_SET_TRANS_LEN(len);
	I2C_SET_TRANSAC_LEN(1);
	I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_FIFO_CLR_ADDR;

	I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start trnasfer transaction */
	I2C_START_TRANSAC;

    /* see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP && (!I2C_FIFO_IS_EMPTY) ) {
            ret = 0;
            break;
        }
        else if ( status & I2C_HS_NACKERR) {
            ret = 1;
            printf("mt6573-i2c polling: transaction nack error %d\n", ret);            
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret = 2;
            printf("mt6573-i2c polling: transaction ack error %d\n", ret);
						printf("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n",(__raw_readl(mt6573_I2C_SLAVE_ADDR)),(__raw_readl(mt6573_I2C_INTR_MASK)),(__raw_readl(mt6573_I2C_INTR_STAT)),(__raw_readl(mt6573_I2C_CONTROL)),(__raw_readl(mt6573_I2C_TRANSFER_LEN)),(__raw_readl(mt6573_I2C_TRANSAC_LEN)),(__raw_readl(mt6573_I2C_DELAY_LEN)),(__raw_readl(mt6573_I2C_TIMING)),(__raw_readl(mt6573_I2C_START)),(__raw_readl(mt6573_I2C_FIFO_STAT)),(__raw_readl(mt6573_I2C_IO_CONFIG)),(__raw_readl(mt6573_I2C_HS)),(__raw_readl(mt6573_I2C_DEBUGSTAT)));
            I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
						I2C_SET_SLAVE_ADDR(0x00);
						I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
						I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
						I2C_FIFO_CLR_ADDR;
            break;
        }
        else if (0 == tmo) {
            ret = 3;
            printf("mt6573-i2c polling: transaction timeout\n");
            printf("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n",(__raw_readl(mt6573_I2C_SLAVE_ADDR)),(__raw_readl(mt6573_I2C_INTR_MASK)),(__raw_readl(mt6573_I2C_INTR_STAT)),(__raw_readl(mt6573_I2C_CONTROL)),(__raw_readl(mt6573_I2C_TRANSFER_LEN)),(__raw_readl(mt6573_I2C_TRANSAC_LEN)),(__raw_readl(mt6573_I2C_DELAY_LEN)),(__raw_readl(mt6573_I2C_TIMING)),(__raw_readl(mt6573_I2C_START)),(__raw_readl(mt6573_I2C_FIFO_STAT)),(__raw_readl(mt6573_I2C_IO_CONFIG)),(__raw_readl(mt6573_I2C_HS)),(__raw_readl(mt6573_I2C_DEBUGSTAT)));
            I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
						I2C_SET_SLAVE_ADDR(0x00);
						I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
						I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
						I2C_FIFO_CLR_ADDR;
            break;
        }
        tmo --;
    };

	I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
	
    if (!ret) {
        while (len--) {
		    I2C_READ_BYTE(*ptr);
//			printf("mt6573-i2c polling: read byte = 0x%.2X\n", *ptr);
			ptr++;
		}
    }

    /* clear bit mask */
	I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	
	if(0 == port)
		I2C_SET_BITS(0x1<<0x8, PDN_SET0);
	else
		I2C_SET_BITS(0x1<<0x9, PDN_SET0);
    
	return ret;
}


int mt6573_i2c_polling_write(int port, unsigned char addr, unsigned char *buffer, int len)
{
  u32    base;
	u8    *ptr = buffer;
	int    ret = len;
	long   tmo = 0xffff;    
  unsigned short status; 
  unsigned long tmp;
  
  if(port > 1)
  	printf("mt6573-i2c polling: No this port.\n");
  else if(0 == port)
  	base = 0x7000B000;
  else
  	base = 0x7000D000;

	if(len > I2C_FIFO_SIZE)
		printf("mt6573-i2c polling: len is too long.\n");

    /* CHECKME. mt6573 doesn't support len = 0. */
	if (!len)
	{
		printf("mt6573-i2c polling: data buffer is NULL.\n");
		return 0;
	}

    /* bit 0 is to indicate read REQ or write REQ */
	addr = (addr & ~0x1);
	
	if(0 == port)
		I2C_SET_BITS(0x1<<0x8, PDN_CLR0);
	else
		I2C_SET_BITS(0x1<<0x9, PDN_CLR0);
		
	tmp  = __raw_readw(mt6573_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
	tmp  = (6 & 0x7) << 8 | (10 & 0x3f) << 0 | tmp;
	__raw_writew(tmp, mt6573_I2C_TIMING);
	I2C_SET_HS_MODE(0);

    /* control registers */
	I2C_SET_SLAVE_ADDR(addr);
	I2C_SET_TRANS_LEN(len);
	I2C_SET_TRANSAC_LEN(1);
	I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_FIFO_CLR_ADDR;

	I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start to write data */
    while (len--) {
	    I2C_WRITE_BYTE(*ptr);
//		printf("mt6573-i2c polling: write byte = 0x%.2X\n", *ptr);
		ptr++;
    }

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP ) {
            ret = 0;
            break;
        }
        else if ( status & I2C_HS_NACKERR) {
            ret = 1;
            printf("mt6573-i2c polling: transaction nack error %d\n", ret);            
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret = 2;
            printf("mt6573-i2c polling: transaction ack error %d\n", ret);
						printf("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n",(__raw_readl(mt6573_I2C_SLAVE_ADDR)),(__raw_readl(mt6573_I2C_INTR_MASK)),(__raw_readl(mt6573_I2C_INTR_STAT)),(__raw_readl(mt6573_I2C_CONTROL)),(__raw_readl(mt6573_I2C_TRANSFER_LEN)),(__raw_readl(mt6573_I2C_TRANSAC_LEN)),(__raw_readl(mt6573_I2C_DELAY_LEN)),(__raw_readl(mt6573_I2C_TIMING)),(__raw_readl(mt6573_I2C_START)),(__raw_readl(mt6573_I2C_FIFO_STAT)),(__raw_readl(mt6573_I2C_IO_CONFIG)),(__raw_readl(mt6573_I2C_HS)),(__raw_readl(mt6573_I2C_DEBUGSTAT)));
            I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
						I2C_SET_SLAVE_ADDR(0x00);
						I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
						I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
						I2C_FIFO_CLR_ADDR;
            break;
        }
        else if (0 == tmo) {
            ret = 3;
            printf("mt6573-i2c polling: transaction timeout\n");
            printf("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n",(__raw_readl(mt6573_I2C_SLAVE_ADDR)),(__raw_readl(mt6573_I2C_INTR_MASK)),(__raw_readl(mt6573_I2C_INTR_STAT)),(__raw_readl(mt6573_I2C_CONTROL)),(__raw_readl(mt6573_I2C_TRANSFER_LEN)),(__raw_readl(mt6573_I2C_TRANSAC_LEN)),(__raw_readl(mt6573_I2C_DELAY_LEN)),(__raw_readl(mt6573_I2C_TIMING)),(__raw_readl(mt6573_I2C_START)),(__raw_readl(mt6573_I2C_FIFO_STAT)),(__raw_readl(mt6573_I2C_IO_CONFIG)),(__raw_readl(mt6573_I2C_HS)),(__raw_readl(mt6573_I2C_DEBUGSTAT)));
            I2C_WRITE_REG(base, OFFSET_SOFTRESET, 0x1);
						I2C_SET_SLAVE_ADDR(0x00);
						I2C_CLR_INTR_STATUS(I2C_HS_NACKERR|I2C_ACKERR|I2C_TRANSAC_COMP);
						I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
						I2C_FIFO_CLR_ADDR;
            break;
        }
        tmo --;
  	};

	I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
	I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	
	if(0 == port)
		I2C_SET_BITS(0x1<<0x8, PDN_SET0);
	else
		I2C_SET_BITS(0x1<<0x9, PDN_SET0);

	return ret;
}

