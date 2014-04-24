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


#include <mt6573.h>
#include <mt6573_typedefs.h>
#include <mt6573_pdn_sw.h>
#include <mt6573_i2c.h>
//#include <asm/mach-types.h>


#define MT6516_I2C_TIMEOUT_TH       200     // i2c wait for response timeout value

#define MT6516_MOD_I2C              "mt6516_i2c:"
/*=====================================================================*/
/*                     Macro Definitions                               */
/*=====================================================================*/
#define MT6516_I2C_DBG_LOG          0

#if MT6516_I2C_DBG_LOG
#ifndef I2C_BUG
#define I2C_BUG() do { \
    dbg_print("BUG at %s:%d!\n", __FILE__, __LINE__); \
            } while (0)
#define I2C_BUG_ON(condition) do { if (condition) I2C_BUG(); } while(0)
#endif /* I2C_BUG */
#else
#define I2C_BUG() do {) while (0)
#define I2C_BUG_ON(condition) do {} while(0)
#define dbg_print(a,...)
#endif

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


/*=====================================================================*/
/*                   Internal Local Functions                          */
/*=====================================================================*/
static int i2c_set_speed(unsigned long        clock,
                         MT6516_I2C_SPD_MODE  mode,
                         unsigned long        khz)
{
    int ret = 0;
    unsigned short sample_cnt_div, step_cnt_div;
    unsigned short max_step_cnt_div = (mode == HS_MODE) ? 
                    MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
    unsigned long tmp, sclk;

    {
        unsigned long diff, min_diff = I2C_CLK_RATE;
        unsigned short sample_div = MAX_SAMPLE_CNT_DIV;
        unsigned short step_div = max_step_cnt_div;
        for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
            for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
                sclk = (clock >> 1) / (sample_cnt_div * step_cnt_div);
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

    sclk = clock / (2 * sample_cnt_div * step_cnt_div);
    if (sclk > khz) {
        ret = -1;
        return ret;
    }
    step_cnt_div--;
    sample_cnt_div--;

    if (mode == HS_MODE) {          
        tmp  = __raw_readw(MT6516_I2C_HS) & ((0x7 << 12) | (0x7 << 8));
        tmp  = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
        __raw_writew(tmp, MT6516_I2C_HS);       
        I2C_SET_HS_MODE(1);
    }
    else {
        tmp  = __raw_readw(MT6516_I2C_TIMING) & ~((0x7 << 8) | (0x1f << 0));
        tmp  = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x1f) << 0 | tmp;
        __raw_writew(tmp, MT6516_I2C_TIMING);
        I2C_SET_HS_MODE(0);
    }
    dbg_print("%s set sclk to %ldkhz (orig: %ldkhz)\n", MT6516_MOD_I2C, sclk, khz);
    return ret;
}

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/
/*-----------------------------------------------------------------------
* Reset
*/
bool mt6516_i2c_reset(void)
{
    return KAL_TRUE;
}

/*-----------------------------------------------------------------------
* Initialization
*/
void mt6516_i2c_init (int speed, int slaveaddr)
{
    /*
    * WARNING: Do NOT save speed in a static variable: if the
    * I2C routines are called before RAM is initialized (to read
    * the DIMM SPD, for instance), RAM won't be usable and your
    * system will crash.
    */
    // i2c_status.state=I2C_READY_STATE;    
    PDN_Power_CONA_DOWN(PDN_PERI_I2C2, KAL_FALSE);

    if (PDN_Get_Peri_Status(PDN_PERI_I2C))
        PDN_Power_CONA_DOWN(PDN_PERI_I2C, KAL_FALSE);

    mt6516_i2c_reset();

    //DRV_SetReg32(MT6516_I2C_CONTROL, I2C_CTL_ACK_ERR_DET_BIT);
    //DRV_SetReg32(MT6516_I2C_CONTROL, I2C_HS_NACK_ERR_DET_EN_BIT);
    //*((volatile unsigned short int *) MT6516_I2C_CONTROL |= I2C_CTL_ACK_ERR_DET_BIT;
    //*((volatile unsigned short int *) MT6516_I2C_CONTROL |= I2C_HS_NACK_ERR_DET_EN_BIT;
    //ENABLE_I2C_ACK_ERR_DET; //Always enable ack error detection
    //ENABLE_I2C_NAKERR_DET;  //Always enable nack error detection

    // ENABLE_I2C_BUS_BUSY_DET;

    // set up the driving current
    // Jau comment for short
    // *((volatile unsigned short int *) (CONFIG_BASE + 0x804)) |= 0x66;

    // i2c_status.is_DMA_enabled=KAL_FALSE;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* set speed mode */
    i2c_set_speed(I2C_CLK_RATE, ST_MODE, MAX_ST_MODE_SPEED);

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    __raw_writel(0, MT6516_I2C_START);
}

/*-----------------------------------------------------------------------
* Read/Write interface: Read bytes
*   chip:    I2C chip address, range 0..127 (slave address?)
*   addr:    Memory (register) address within the chip
*   alen:    Number of bytes to use for addr (typically 1, 2 for larger
*              memories, 0 for register type devices with only one
*              register)
*   buffer:  Where to read/write the data
*   len:     How many bytes to read/write
*
*   Returns : 0 on success, not 0 on failure
*   Example : i2c_read(i2c_addr, reg, 1, &buf, 1);
*/

int  mt6516_i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    u32   base = I2C2_BASE;
    u8   *ptr  = buffer;
    int   ret  = len;
    long  tmo;    
    unsigned short status;

    I2C_BUG_ON(len > I2C_FIFO_SIZE);

    /* CHECKME. mt6516 doesn't support len = 0. */
    if (!len)
        return 0;

    /* bit 0 is to indicate read REQ or write REQ */
    chip = (chip | 0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    tmo = get_timer(0);

    /* see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP && (!I2C_FIFO_IS_EMPTY) ) {
            ret = 0;
            break;
        }
        else if ( status & I2C_HS_NACKERR) {
            ret = 1;
            dbg_print("%s transaction error %d\n", MT6516_MOD_I2C, ret);
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret = 2;
            dbg_print("%s transaction error %d\n", MT6516_MOD_I2C, ret);
            break;
        }
        else if (get_timer(tmo) > MT6516_I2C_TIMEOUT_TH /* ms */) {
            ret = 3;
            dbg_print("%s transaction timeout\n", MT6516_MOD_I2C);
            break;
        }
    };

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    if (!ret) {
        while (len--) {
            I2C_READ_BYTE(*ptr);
            dbg_print("%s read byte = 0x%.2X\n", MT6516_MOD_I2C, *ptr);
            ptr++;
        }
    }

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

    return ret;
}

/*-----------------------------------------------------------------------
* Read/Write interface: Write bytes
*   chip:    I2C chip address, range 0..127
*   addr:    Memory (register) address within the chip
*   alen:    Number of bytes to use for addr (typically 1, 2 for larger
*              memories, 0 for register type devices with only one
*              register)
*   buffer:  Where to read/write the data
*   len:     How many bytes to read/write
*
*   Returns: 0 on success, not 0 on failure
*/

int  mt6516_i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    u32    base = I2C2_BASE;
    u8    *ptr = buffer;
    int    ret = len;
    long   tmo;    
    unsigned short status;

    I2C_BUG_ON(len > I2C_FIFO_SIZE);

    /* CHECKME. mt6516 doesn't support len = 0. */
    if (!len)   
        return 0;

    /* bit 0 is to indicate read REQ or write REQ */
    chip = (chip & ~0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start to write data */
    while (len--) {
        I2C_WRITE_BYTE(*ptr);
        dbg_print("%s write byte = 0x%.2X\n", MT6516_MOD_I2C, *ptr);
        ptr++;
    }

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    tmo = get_timer(0);

    /* see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP) {
            ret = 0;
            break;
        }
        else if ( status & I2C_HS_NACKERR) {
            ret = 1;
            dbg_print("%s transaction error %d\n", MT6516_MOD_I2C, ret);
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret = 2;
            dbg_print("%s transaction error %d\n", MT6516_MOD_I2C, ret);
            break;
        }
        else if (get_timer(tmo) > MT6516_I2C_TIMEOUT_TH /* ms */ ) {
            ret = 3;
            dbg_print("%s transaction timeout\n", MT6516_MOD_I2C);
            break;
        }
    };

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

    return ret;
}

kal_bool mt6516_i2c_read_byte(u8 cmd, u8 *returnData)
{
    int    ret=0;

    PL_TRACER;
    ret = mt6516_i2c_write(0xc0, 0, 1, &cmd, 1);    // set register command
    if (ret != 0)
        return FALSE ;

    PL_TRACER;
    ret = mt6516_i2c_read(0xc0, 0, 1, returnData, 1);

    return (ret == 0);
}

kal_bool mt6516_i2c_write_byte(u8 cmd, u8 writeData)
{
    u8    write_data[2];
    int   ret=0;

    write_data[0] = cmd;
    write_data[1] = writeData;

    ret = mt6516_i2c_write( 0xc0, 0, 2, write_data, 2);

    return (ret == 0);
}


