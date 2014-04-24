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

#ifndef __MT6516_I2C_H__
#define __MT6516_I2C_H__

#include <mt6573.h>


//#define MT6516_IRQ_I2C_CODE               19    /* I2C 2 controller */

#define I2C_CLK_RATE                        13000       /* khz for CPU_416MHZ_MCU_104MHZ */

#define I2C_FIFO_SIZE                       8

#define MAX_ST_MODE_SPEED                   100     /* khz */
#define MAX_FS_MODE_SPEED                   400     /* khz */
#define MAX_HS_MODE_SPEED                   3400    /* khz */

#define MAX_DMA_TRANS_SIZE                  252     /* Max(255) aligned to 4 bytes = 252 */
#define MAX_DMA_TRANS_NUM                   256

#define MAX_SAMPLE_CNT_DIV                  8
#define MAX_STEP_CNT_DIV                    64
#define MAX_HS_STEP_CNT_DIV                 8

#define MT6516_I2C_DATA_PORT                (I2C2_BASE + 0x0000)
#define MT6516_I2C_SLAVE_ADDR               (I2C2_BASE + 0x0004)
#define MT6516_I2C_INTR_MASK                (I2C2_BASE + 0x0008)
#define MT6516_I2C_INTR_STAT                (I2C2_BASE + 0x000c)
#define MT6516_I2C_CONTROL                  (I2C2_BASE + 0x0010)
#define MT6516_I2C_TRANSFER_LEN             (I2C2_BASE + 0x0014)
#define MT6516_I2C_TRANSAC_LEN              (I2C2_BASE + 0x0018)
#define MT6516_I2C_DELAY_LEN                (I2C2_BASE + 0x001c)
#define MT6516_I2C_TIMING                   (I2C2_BASE + 0x0020)
#define MT6516_I2C_START                    (I2C2_BASE + 0x0024)
#define MT6516_I2C_FIFO_STAT                (I2C2_BASE + 0x0030)
#define MT6516_I2C_FIFO_THRESH              (I2C2_BASE + 0x0034)
#define MT6516_I2C_FIFO_ADDR_CLR            (I2C2_BASE + 0x0038)
#define MT6516_I2C_IO_CONFIG                (I2C2_BASE + 0x0040)
#define MT6516_I2C_DEBUG                    (I2C2_BASE + 0x0044)
#define MT6516_I2C_HS                       (I2C2_BASE + 0x0048)
#define MT6516_I2C_DEBUGSTAT                (I2C2_BASE + 0x0064)
#define MT6516_I2C_DEBUGCTRL                (I2C2_BASE + 0x0068)

#define MT6516_I2C_TRANS_LEN_MASK           (0xff)
#define MT6516_I2C_TRANS_AUX_LEN_MASK       (0x1f << 8)
#define MT6516_I2C_CONTROL_MASK             (0x3f << 1)

//----------- Register mask -------------------//
#define I2C_3_BIT_MASK                      0x07
#define I2C_4_BIT_MASK                      0x0f
#define I2C_6_BIT_MASK                      0x3f
#define I2C_8_BIT_MASK                      0xff
#define I2C_FIFO_THRESH_MASK                0x07
#define I2C_AUX_LEN_MASK                    0x1f00
#define I2C_MASTER_READ                     0x01
#define I2C_MASTER_WRITE                    0x00
#define I2C_CTL_RS_STOP_BIT                 0x02
#define I2C_CTL_DMA_EN_BIT                  0x04
#define I2C_CTL_CLK_EXT_EN_BIT              0x08
#define I2C_CTL_DIR_CHANGE_BIT              0x10
#define I2C_CTL_ACK_ERR_DET_BIT             0x20
#define I2C_CTL_TRANSFER_LEN_CHG_BIT        0x40
#define I2C_DATA_READ_ADJ_BIT               0x8000
#define I2C_SCL_MODE_BIT                    0x01
#define I2C_SDA_MODE_BIT                    0x02
#define I2C_BUS_DETECT_EN_BIT               0x04
#define I2C_ARBITRATION_BIT                 0x01
#define I2C_CLOCK_SYNC_BIT                  0x02
#define I2C_BUS_BUSY_DET_BIT                0x04
#define I2C_HS_EN_BIT                       0x01
#define I2C_HS_NACK_ERR_DET_EN_BIT          0x02
#define I2C_HS_MASTER_CODE_MASK             0x70
#define I2C_HS_STEP_CNT_DIV_MASK            0x700
#define I2C_HS_SAMPLE_CNT_DIV_MASK          0x7000
#define I2C_FIFO_FULL_STATUS                0x01
#define I2C_FIFO_EMPTY_STATUS               0x02


#define I2C_DEBUG                           (1 << 3)
#define I2C_HS_NACKERR                      (1 << 2)
#define I2C_ACKERR                          (1 << 1)
#define I2C_TRANSAC_COMP                    (1 << 0)

#define I2C_TX_THR_OFFSET                   8
#define I2C_RX_THR_OFFSET                   0

#define I2C_START_TRANSAC           __raw_writel(0x1,MT6516_I2C_START)
#define I2C_FIFO_CLR_ADDR           __raw_writel(0x1,MT6516_I2C_FIFO_ADDR_CLR)
#define I2C_FIFO_OFFSET             (__raw_readl(MT6516_I2C_FIFO_STAT)>>4&0xf)
#define I2C_FIFO_IS_EMPTY           (__raw_readw(MT6516_I2C_FIFO_STAT)>>0&0x1)

#define I2C_SET_BITS(BS,REG)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define I2C_CLR_BITS(BS,REG)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

#define I2C_SET_FIFO_THRESH(tx,rx)  \
    do { u32 tmp = (((tx) & 0x7) << I2C_TX_THR_OFFSET) | \
    (((rx) & 0x7) << I2C_RX_THR_OFFSET); \
    __raw_writel(tmp, MT6516_I2C_FIFO_THRESH); \
    } while(0)

#define I2C_SET_INTR_MASK(mask)     __raw_writel(mask, MT6516_I2C_INTR_MASK)

#define I2C_CLR_INTR_MASK(mask)     \
    do { u32 tmp = __raw_readl(MT6516_I2C_INTR_MASK); \
    tmp &= ~(mask); \
    __raw_writel(tmp, MT6516_I2C_INTR_MASK); \
    } while(0)

#define I2C_SET_SLAVE_ADDR(addr)    __raw_writel(addr, MT6516_I2C_SLAVE_ADDR)

#define I2C_SET_TRANS_LEN(len)      \
    do { u32 tmp = __raw_readl(MT6516_I2C_TRANSFER_LEN) & \
    ~MT6516_I2C_TRANS_LEN_MASK; \
    tmp |= ((len) & MT6516_I2C_TRANS_LEN_MASK); \
    __raw_writel(tmp, MT6516_I2C_TRANSFER_LEN); \
    } while(0)

#define I2C_SET_TRANS_AUX_LEN(len)  \
    do { u32 tmp = __raw_readl(MT6516_I2C_TRANSFER_LEN) & \
    ~(MT6516_I2C_TRANS_AUX_LEN_MASK); \
    tmp |= (((len) << 8) & MT6516_I2C_TRANS_AUX_LEN_MASK); \
    __raw_writel(tmp, MT6516_I2C_TRANSFER_LEN); \
    } while(0)

#define I2C_SET_TRANSAC_LEN(len)    __raw_writel(len, MT6516_I2C_TRANSAC_LEN)
#define I2C_SET_TRANS_DELAY(delay)  __raw_writel(delay, MT6516_I2C_DELAY_LEN)

#define I2C_SET_TRANS_CTRL(ctrl)    \
    do { u32 tmp = __raw_readl(MT6516_I2C_CONTROL) & ~MT6516_I2C_CONTROL_MASK; \
    tmp |= ((ctrl) & MT6516_I2C_CONTROL_MASK); \
    __raw_writel(tmp, MT6516_I2C_CONTROL); \
    } while(0)

#define I2C_SET_HS_MODE(on_off) \
    do { u32 tmp = __raw_readl(MT6516_I2C_HS) & ~0x1; \
    tmp |= (on_off & 0x1); \
    __raw_writel(tmp, MT6516_I2C_HS); \
    } while(0)

#define I2C_READ_BYTE(byte)     \
    do { byte = __raw_readb(MT6516_I2C_DATA_PORT); } while(0)

#define I2C_WRITE_BYTE(byte)    \
    do { __raw_writeb(byte, MT6516_I2C_DATA_PORT); } while(0)

#define I2C_CLR_INTR_STATUS(status) \
    do { __raw_writew(status, MT6516_I2C_INTR_STAT); } while(0)

#define I2C_INTR_STATUS             __raw_readw(MT6516_I2C_INTR_STAT)

/* mt6516 i2c control bits */
#define TRANS_LEN_CHG               (1 << 6)
#define ACK_ERR_DET_EN              (1 << 5)
#define DIR_CHG                     (1 << 4)
#define CLK_EXT                     (1 << 3)
#define DMA_EN                      (1 << 2)
#define REPEATED_START_FLAG         (1 << 1)
#define STOP_FLAG                   (0 << 1)


typedef enum
{
    ST_MODE,
    FS_MODE,
    HS_MODE
} MT6516_I2C_SPD_MODE;



void mt6516_i2c_init (int speed, int slaveaddr);
bool mt6516_i2c_reset (void);
int mt6516_i2c_probe (uchar addr);
int mt6516_i2c_read (uchar chip, uint addr, int alen, uchar * buffer, int len);
int mt6516_i2c_write (uchar chip, uint addr, int alen, uchar * buffer, int len);
uchar mt6516_i2c_reg_read (uchar i2c_addr, uchar reg);
void mt6516_i2c_reg_write (uchar i2c_addr, uchar reg, uchar val);

extern kal_bool mt6516_i2c_read_byte (u8 cmd, u8 * returnData);
extern kal_bool mt6516_i2c_write_byte (u8 cmd, u8 writeData);

#endif /* __MT6516_I2C_H__ */
