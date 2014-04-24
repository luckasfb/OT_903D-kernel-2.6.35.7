
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <mach/mt6573_boot.h>


#include "tpd_custom_mcs6024.h"
#include "tpd.h"
#include <cust_eint.h>

#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif

//if the TP has external power with GPIO pin,need define TPD_HAVE_POWER_ON_OFF in tpd_custom_mcs6024.h
#define TPD_HAVE_POWER_ON_OFF

#define MAX_POINT 2

#define TP_UPDATE 1

#if TP_UPDATE
#include <linux/dma-mapping.h>

 #define FW_ADDR_MSG21XX   (0xC4)
#define FW_ADDR_MSG21XX_TP   (0x4C)
#define FW_UPDATE_ADDR_MSG21XX   (0x92)
#define TP_DEBUG	printk
volatile static u8 Fmr_Loader[1024];
extern char mtkfb_lcm_name_for_tp[256];

unsigned char AC_MSG_GLASS_FIRMWARE_JUNDA[94*1024] =
{
#include "ac-glass-jdc-v2-03.h"
};
unsigned char AC_MSG_PMMA_FIRMWARE_JUNDA[94*1024] =
{
#include "ac-pmma-jdc-vA2-02.h"
};
unsigned char AC_MSG_PMMA_FIRMWARE_MUDO[94*1024] =
{
#include "ac-pmma-mt-vA1-01.h"
};

unsigned char DC_MSG_GLASS_FIRMWARE_JUNDA[94*1024] =
{
#include "dc_glass_jdc_v102-03.h"
};
unsigned char DC_MSG_PMMA_FIRMWARE_JUNDA[94*1024] =
{
#include "dc-pmma-jdc-v1A2-02.h"
};
unsigned char DC_MSG_PMMA_FIRMWARE_MUDO[94*1024] =
{
#include "dc_pmma_mt_v1A1-01.h"
};


//add DMA for updating
static unsigned char *tpDMABuf_va = NULL;
static u32 tpDMABuf_pa = NULL;


#endif

struct touch_info {
    int x[MAX_POINT], y[MAX_POINT];
    int p[MAX_POINT];
	int TouchpointFlag;
    int VirtualKeyFlag;
};

static int raw_x[MAX_POINT], raw_y[MAX_POINT];
struct touch_info cinfo, sinfo;

//wangdongliang 2012 for mstar
#define u8         unsigned char
#define u32        unsigned int
#define s32        signed int

#define MAX_TOUCH_FINGER 2
typedef struct
{
    u16 X;
    u16 Y;
} TouchPoint_t;

typedef struct
{
    u8 nTouchKeyMode;
    u8 nTouchKeyCode;
    u8 nFingerNum;
    TouchPoint_t Point[MAX_TOUCH_FINGER];
} TouchScreenInfo_t;



#define REPORT_PACKET_LENGTH    (8)
#define MSG21XX_INT_GPIO       (42)
#define MSG21XX_RESET_GPIO     (22)
#define MS_TS_MSG21XX_X_MAX   (320)
#define MS_TS_MSG21XX_Y_MAX   (480)

//end wangdongliang


extern struct tpd_device *tpd;
extern int tpd_show_version;
//extern int tpd_debuglog;

static int boot_mode = 0;
static int tpd_flag = 0;
static int tpd_halt=0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"mtk-tpd",0},{}};
static unsigned short force[] = {0, 0x4c, I2C_CLIENT_END,I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
static struct i2c_client_address_data addr_data = { .forces = forces,};
struct i2c_driver tpd_i2c_driver = {                       
    .probe = tpd_i2c_probe,                                   
    .remove = tpd_i2c_remove,                           
    .detect = tpd_i2c_detect,                           
    .driver.name = "mtk-tpd", 
    .id_table = tpd_i2c_id,                             
    .address_data = &addr_data,                        
}; 

static void tpd_hw_enable(void)
{

    /* CTP_CE */
	mt_set_gpio_mode(GPIO21, 0);
	mt_set_gpio_dir(GPIO21, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO21, GPIO_OUT_ONE);
	
    mdelay(20);
}

static void tpd_hw_disable(void)
{
    /* CTP_EN */
    mt_set_gpio_mode(GPIO21, 0);
	mt_set_gpio_dir(GPIO21, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO21, GPIO_OUT_ZERO);

}

static void tpd_i2c_master_send(struct i2c_client *client,const char *buf ,int count)
{
	client->addr = client->addr & I2C_MASK_FLAG;
	i2c_master_send(client,(const char*)buf,count);
}

static  int tpd_i2c_master_rs_send(struct i2c_client *client,const char *buf ,int count)
{
	int ret;

	client->addr = (client->addr & I2C_MASK_FLAG) | I2C_WR_FLAG | I2C_RS_FLAG; 
	ret = i2c_master_send(client,(const char*)buf,count);
	return ret;
}


static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-tpd");
    return 0;
}
#if TP_UPDATE
//for updating

//add DMA
ssize_t mt6573_dma_write_m_byte(unsigned char*returnData_va, u32 returnData_pa, int  len)
{

    int     ret=0;
	 i2c_client->addr = 0x92;

	 i2c_client->addr = (i2c_client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
    if (len > 0){
        ret = i2c_master_send(i2c_client, returnData_pa, len);
        if (ret < 0) {
            printk(KERN_ERR"xxxxfocal write data error!! xxxx\n");
            return 0;
        }
    }
	//printk("xxxxwrite transfer ok!!!!!!xxxx\n");
    return 1;
}
U8 mt6573_dma_read_m_byte(U8 n, U8* returnData_va,  u32 returnData_pa)    //First it needs send 0x11 to notify we want to get flash data back.
{
    U8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0};
	 i2c_client->addr = 0x92;
    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
    i2c_master_send(i2c_client, &Read_cmd, 1);     //**i2c_client
 //	   ret = i2c_master_send(i2c_client, returnData_pa, len);
     i2c_client->addr = (i2c_client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
     i2c_master_recv(i2c_client, returnData_pa, n);
}

static void HalTscrCReadI2CSeq(u8 addr, u8* read_data, u8 size)
{
	i2c_client->addr = addr;
	i2c_master_recv(i2c_client, &read_data[0], size);
}

static void HalTscrCDevWriteI2CSeq(u8 addr, u8* data, u16 size)
{
	i2c_client->addr = addr;
	i2c_master_send(i2c_client, &data[0], size);
}

static void dbbusDWIICEnterSerialDebugMode(void)
{
    u8 data[5];

    // Enter the Serial Debug Mode
    data[0] = 0x53;
    data[1] = 0x45;
    data[2] = 0x52;
    data[3] = 0x44;
    data[4] = 0x42;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 5);
}

static void dbbusDWIICStopMCU(void)
{
    u8 data[1];

    // Stop the MCU
    data[0] = 0x37;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICIICUseBus(void)
{
    u8 data[1];

    // IIC Use Bus
    data[0] = 0x35;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICIICReshape(void)
{
    u8 data[1];

    // IIC Re-shape
    data[0] = 0x71;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICIICNotUseBus(void)
{
    u8 data[1];

    // IIC Not Use Bus
    data[0] = 0x34;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICNotStopMCU(void)
{
    u8 data[1];

    // Not Stop the MCU
    data[0] = 0x36;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICExitSerialDebugMode(void)
{
    u8 data[1];

    // Exit the Serial Debug Mode
    data[0] = 0x45;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);

    // Delay some interval to guard the next transaction
    //udelay ( 200 );        // delay about 0.2ms
}

static void drvISP_EntryIspMode(void)
{
    u8 bWriteData[5] =
    {
        0x4D, 0x53, 0x54, 0x41, 0x52
    };

    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);
}

static u8 drvISP_Read(u8 n, u8* pDataToRead)    //First it needs send 0x11 to notify we want to get flash data back.
{
    u8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0};
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &Read_cmd, 1);
    udelay(200);        // delay about 100us*****
    udelay(200); 
    if (n == 1)
    {

        HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
        *pDataToRead = dbbus_rx_data[1];
    }
    else
    {
        HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG21XX, &pDataToRead[0], n);
    }

    return 0;
}

static void drvISP_WriteEnable(void)
{
    u8 bWriteData[2] =
    {
        0x10, 0x06
    };
    u8 bWriteData1 = 0x12;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
}


static void drvISP_ExitIspMode(void)
{
    u8 bWriteData = 0x24;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData, 1);
}

static u8 drvISP_ReadStatus(void)
{
    u8 bReadData = 0;
    u8 bWriteData[2] =
    {
        0x10, 0x05
    };
    u8 bWriteData1 = 0x12;

    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    udelay(100);        // delay about 100us*****
    udelay(100); 
    drvISP_Read(1, &bReadData);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
    return bReadData;
}


static void drvISP_ChipErase(void)
{
    u8 bWriteData[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    u8 bWriteData1 = 0x12;
		u32 timeOutCount=0;
    drvISP_WriteEnable();

    //Enable write status register
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x50;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);

    //Write Status
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x01;
    bWriteData[2] = 0x00;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 3);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);

    //Write disable
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x04;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
		udelay(100);        // delay about 100us*****
		udelay(100); 
    timeOutCount=0;
		while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
		{
		   timeOutCount++;
	     if ( timeOutCount >= 100000 ) 
	     	{
	     	printk("xxxxxxxxxx timeout xxxxxxxxxx\n");
		 break; /* around 1 sec timeout */
	     	}
		 }
    drvISP_WriteEnable();

//    bWriteData[0] = 0x10;
 //   bWriteData[1] = 0xC7;

 // 	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
     bWriteData[0] = 0x10;
    bWriteData[1] = 0xD8;        //Block Erase
    bWriteData[2] = 0;
    bWriteData[3] = 0;
    bWriteData[4] = 0 ;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData[0], 5);

    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
		udelay(100);        // delay about 100us*****
		udelay(100); 
		timeOutCount=0;
		while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
		{
		   timeOutCount++;
	     if ( timeOutCount >= 500000 ) {
				     	printk("xxxxxxxxxx timeout 222xxxxxxxxxx\n");
	
			break; /* around 5 sec timeout */
			}
	  }
}

static void drvISP_Program(u16 k, u8* pDataToWrite)
{
    u16 i = 0;
    u16 j = 0;
    //u16 n = 0;
    u8 TX_data[133];
    u8 bWriteData1 = 0x12;
    u32 addr = k * 1024;
		u32 timeOutCount=0;
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
//        TX_data[0] = 0x10;
//        TX_data[1] = 0x02;// Page Program CMD
//        TX_data[2] = (addr + 128 * j) >> 16;
//        TX_data[3] = (addr + 128 * j) >> 8;
//        TX_data[4] = (addr + 128 * j);
	tpDMABuf_va[0] = 0x10;
	tpDMABuf_va[1] = 0x02;
	tpDMABuf_va[2] = ((addr + 128 * j) >> 16) & 0xFF;
	tpDMABuf_va[3] = ((addr + 128 * j) >> 8) & 0xFF;
	tpDMABuf_va[4] = (addr + 128 * j) & 0xFF;

		
        for (i = 0; i < 128; i++)
        {
        	tpDMABuf_va[5 + i] = pDataToWrite[j * 128 + i];

//            TX_data[5 + i] = pDataToWrite[j * 128 + i];
        }
        udelay(100);        // delay about 100us*****
        udelay(100); 
       
        timeOutCount=0;
				while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
				{
		   			timeOutCount++;
	     			if ( timeOutCount >= 100000 ) break; /* around 1 sec timeout */
	  		}
        
        
        
        drvISP_WriteEnable();
			mt6573_dma_write_m_byte(tpDMABuf_va, tpDMABuf_pa, 133);

//        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, TX_data, 133);   //write 133 byte per cycle
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
    }
}


static void drvISP_Verify(u16 k, u8* pDataToVerify)
{
    u16 i = 0, j = 0;
    u8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    u8 RX_data[256];
    u8 bWriteData1 = 0x12;
    u32 addr = k * 1024;
    u8 index=0;
    u32 timeOutCount;
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        bWriteData[2] = (u8)((addr + j * 128) >> 16);
        bWriteData[3] = (u8)((addr + j * 128) >> 8);
        bWriteData[4] = (u8)(addr + j * 128);
        udelay(100);        // delay about 100us*****
        udelay(100); 
        
        timeOutCount=0;
	while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )

	{
		timeOutCount++;
		if ( timeOutCount >= 100000 ) break; /* around 1 sec timeout */
	}
        
        
        
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);    //write read flash addr
        udelay(100);        // delay about 100us*****
        udelay(100); 
        //drvISP_Read(128, RX_data);
        mt6573_dma_read_m_byte(128, tpDMABuf_va, tpDMABuf_pa);

        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);    //cmd end
        for (i = 0; i < 128; i++)   //log out if verify error
        {
        if((RX_data[i]!=0)&&index<10)
		{
        //TP_DEBUG("j=%d,RX_data[%d]=0x%x\n",j,i,RX_data[i]);
        index++;
		}
            if (tpDMABuf_va[i] != pDataToVerify[128 * j + i])
            {
                TP_DEBUG("k=%d,j=%d,i=%d===============Update Firmware Error================",k,j,i);
            }
        }
    }
}
static void _HalTscrHWReset(void)
{

	mt_set_gpio_mode(GPIO17, 0);
	mt_set_gpio_dir(GPIO17, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO17, GPIO_OUT_ONE);
	mdelay(10);
	mt_set_gpio_out(GPIO17, GPIO_OUT_ZERO);
	mdelay(10);
	mt_set_gpio_out(GPIO17, GPIO_OUT_ONE);
	mdelay(50);
}
void masterBUT_LoadFwToTarget(unsigned char *pfirmware)
{
	u8 i;
	unsigned short j;
	u8 dbbus_tx_data[4];
	unsigned char dbbus_rx_data[2] = {0};
	i2c_client->timing = 50;
	//		_HalTscrHWReset();
	//1.Erase TP Flash first

	dbbusDWIICEnterSerialDebugMode();
	dbbusDWIICStopMCU();
	dbbusDWIICIICUseBus();
	dbbusDWIICIICReshape();
	mdelay(300);
		
				
	// Disable the Watchdog
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x60;
	dbbus_tx_data[3] = 0x55;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x61;
	dbbus_tx_data[3] = 0xAA;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	//Stop MCU
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x0F;
	dbbus_tx_data[2] = 0xE6;
	dbbus_tx_data[3] = 0x01;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	//set FRO to 50M
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x11;
	dbbus_tx_data[2] = 0xE2;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
	dbbus_rx_data[0] = 0;
	dbbus_rx_data[1] = 0;
	HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
	TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);



	//set MCU clock,SPI clock =FRO
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x22;
	dbbus_tx_data[3] = 0x00;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x23;
	dbbus_tx_data[3] = 0x00;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	// Enable slave's ISP ECO mode
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x08;
	dbbus_tx_data[2] = 0x0c;
	dbbus_tx_data[3] = 0x08;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	//Enable SPI Pad
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x02;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
	HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
	TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	//WP overwrite
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x0E;
	dbbus_tx_data[3] = 0x02;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	//set pin high
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x10;
	dbbus_tx_data[3] = 0x08;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	dbbusDWIICIICNotUseBus();
	dbbusDWIICNotStopMCU();
	dbbusDWIICExitSerialDebugMode();


	drvISP_EntryIspMode();
	drvISP_ChipErase();
//    _HalTscrHWReset();
//    mdelay(300);
	for (i = 0; i < 94; i++)   // total  94 KB : 1 byte per R/W
	{
		printk("xxxxx i = %d xxxx\n", i);
		for ( j = 0; j < 1024; j++ )        //Read 1k bytes
		{
			Fmr_Loader[j] = pfirmware[(i*1024)+j]; // Read the bin files of slave firmware from the baseband file system
		}

		drvISP_Program(i, Fmr_Loader);    // program to slave's flash
		//drvISP_Verify ( i, Fmr_Loader ); //verify data
	}
	printk("xxxxxxxxxxxxxxxx update end and please set timing to 100 and addr to 0x4cxxxxxxxxxxxxxxxxxxxxx\n");
	drvISP_ExitIspMode();
	i2c_client->timing = 100;
	i2c_client->addr = 0x4c;			//back to normal addr

}

 //end for updating
#endif


static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {             
	int err = 0,i;
//	uint8_t Firmware_version[3] = {0x20,0x00,0x00};
	i2c_client = client;
       unsigned char tpd_buf[8] = {0};
#if TP_UPDATE
	unsigned char dbbus_tx_data[3];
	unsigned char dbbus_rx_data[4] ;
	unsigned short current_major_version=0, current_minor_version=0;
	unsigned short wanted_major_version=0,wanted_minor_version=0;
	char version[3][20] = {"glass JUNDA", "pmma JUNDA", "pmma MUTTO"};
	char *pversion;
	char *lcd_ic_name[2] = {"r61526", "ili9340"};
	unsigned char *FIRMWARE_OF_TP = NULL;

#endif

//	#ifdef TPD_HAVE_POWER_ON_OFF
	//for power on sequence
	tpd_hw_enable();
	mt_set_gpio_mode(GPIO21, 0);
	mt_set_gpio_dir(GPIO21, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO21, GPIO_OUT_ONE);

    	mdelay(200);


//eint config to gpio
	    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);	
//	#endif

	msleep(20);

#if TP_UPDATE
/**************enter update****************/

// 1st step: get current firmware version
	dbbus_tx_data[0] = 0x53;
	dbbus_tx_data[1] = 0x00;
	dbbus_tx_data[2] = 0x74;
	i2c_master_send(i2c_client, &dbbus_tx_data[0], 3);
	i2c_master_recv(i2c_client, &dbbus_rx_data[0], 4);


	current_major_version = (dbbus_rx_data[1]<<8)+dbbus_rx_data[0];
	current_minor_version = (dbbus_rx_data[3]<<8)+dbbus_rx_data[2];
	if((current_major_version == 0x0001) || (current_major_version == 0x0002) ||(current_major_version == 0x0102))
		pversion = version[0];		//glass JUNDA
	else if((current_major_version == 0x00a2) || (current_major_version == 0x01a2))
		pversion = version[1];		//pmma JUNDA
	else if((current_major_version == 0x00a1) || (current_major_version == 0x01a1))
		pversion = version[2];		//pmma MUTTO
	printk("%s 's current version is  0x%4x.0x%4x\n",pversion, current_major_version, current_minor_version);
	printk("lcm is %s\n", mtkfb_lcm_name_for_tp);


//2nd step: decide which firmware to be used
	if(strcmp(&mtkfb_lcm_name_for_tp, lcd_ic_name[0]) == 0)			//AC LCD
	{
		printk("ac LCM is %s\n", mtkfb_lcm_name_for_tp);
		if((current_major_version == 0x0001) || (current_major_version == 0x0002))					//now we are using  ac glass JUNDA's TP
			FIRMWARE_OF_TP = AC_MSG_GLASS_FIRMWARE_JUNDA;
		else if(current_major_version == 0x00a2)
			FIRMWARE_OF_TP = AC_MSG_PMMA_FIRMWARE_JUNDA;
		else if(current_major_version == 0x00a1)
			FIRMWARE_OF_TP = AC_MSG_PMMA_FIRMWARE_MUDO;
	}else if(strcmp(&mtkfb_lcm_name_for_tp, lcd_ic_name[1]) == 0){	//DC LCD
		printk("dc LCM is %s\n", mtkfb_lcm_name_for_tp);
		if((current_major_version == 0x0001) || (current_major_version == 0x0002) ||(current_major_version == 0x0102)||(current_major_version == 0x0101) )					//now we are using  dc glass JUNDA's TP
			FIRMWARE_OF_TP = DC_MSG_GLASS_FIRMWARE_JUNDA;
		else if((current_major_version == 0x00a2)||(current_major_version == 0x01a2))
			FIRMWARE_OF_TP = DC_MSG_PMMA_FIRMWARE_JUNDA;
		else if((current_major_version == 0x00a1)||(current_major_version == 0x01a1))
			FIRMWARE_OF_TP = DC_MSG_PMMA_FIRMWARE_MUDO;
	}

//3rd step: update or not!
	if(FIRMWARE_OF_TP != NULL){
		printk("decide update or not\n");
		wanted_major_version= (FIRMWARE_OF_TP[0x3076] << 8) + FIRMWARE_OF_TP[0x3077];
		wanted_minor_version= (FIRMWARE_OF_TP[0x3074] << 8) + FIRMWARE_OF_TP[0x3075];
		printk("needed version is  0x%4x.0x%4x\n", wanted_major_version, wanted_minor_version);
		if(wanted_major_version == current_major_version)	//same TP make sure
		{
			printk("same TP, both AC, or DC\n");
			if(current_minor_version < wanted_minor_version){//minor version diff , we updating
				printk("needed version 0x%2x is bigger than current version ,now updating...\n", wanted_minor_version);
			masterBUT_LoadFwToTarget(FIRMWARE_OF_TP);
			}
			else if(current_minor_version == wanted_minor_version)
				printk("needed version 0x%2x is same to current version. over\n", wanted_minor_version);
		}
		else if((wanted_major_version != current_major_version)&&((wanted_major_version&0xff00) == 0x0100)){	//from AC to DC
			printk("now, from AC to DC\n");	
			masterBUT_LoadFwToTarget(FIRMWARE_OF_TP);		//mast update!
		}
	}else 
		printk("FIRMWARE_OF_TP is NULL ,means no TP, please notice error!\n");

/****************end update****************/
#endif


		
	  thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread)) { 
        err = PTR_ERR(thread);
        TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", err);
    }    
    

    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	
   tpd_load_status = 1;
    return 0;
}

void tpd_eint_interrupt_handler(void) { 
	//if(tpd_em_debuglog==1) 
		//TPD_DMESG("[mtk-tpd], %s\n", __FUNCTION__);
//		printk("xxxxxxxxx %s xxxxxxxxxxx\n", __FUNCTION__);
    TPD_DEBUG_PRINT_INT; tpd_flag=1; wake_up_interruptible(&waiter);
} 
static int tpd_i2c_remove(struct i2c_client *client) {return 0;}

void tpd_down(int raw_x, int raw_y, int x, int y, int p) {	
    input_report_abs(tpd->dev, ABS_PRESSURE, p);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, p);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    TPD_DEBUG("D[%4d %4d %4d] ", x, y, p);
    input_mt_sync(tpd->dev);
    TPD_DOWN_DEBUG_TRACK(x,y);
    TPD_EM_PRINT(raw_x, raw_y, x, y, p, 1);
}

int tpd_up(int raw_x, int raw_y, int x, int y) {
        input_report_abs(tpd->dev, ABS_PRESSURE, 0);
        input_report_key(tpd->dev, BTN_TOUCH, 0);
        input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
        input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
        input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
        TPD_DEBUG("U[%4d %4d %4d] ", x, y, 0);
        input_mt_sync(tpd->dev);
        TPD_UP_DEBUG_TRACK(x,y);
        TPD_EM_PRINT(raw_x, raw_y, x, y, 0, 0);
        return 1;
}


int tpd_gettouchinfo(struct touch_info *cinfo) {
	uint8_t status = 0x10;
	uint8_t  address_p1[4] = {0x11,0x00,0x00,0x00};
	uint8_t  address_p2[4] = {0x19,0x00,0x00,0x00};
	uint8_t i;

    sinfo.VirtualKeyFlag = cinfo->VirtualKeyFlag;
	cinfo->TouchpointFlag = 0;
    cinfo->VirtualKeyFlag= 0;
		
	tpd_i2c_master_rs_send(i2c_client,&status,(1<<8)|1);
	cinfo->TouchpointFlag = status&0x03;   //For multi-touch(three point)
	TPD_DEBUG("TouchpointFlag = %x\n",status);

    status = 0x18;
    tpd_i2c_master_rs_send(i2c_client,&status,(1<<8)|1);
    cinfo->VirtualKeyFlag = status&0x0F;
    TPD_DEBUG("VirtualKeyFlag = %x\n",status);
    
	if((cinfo->TouchpointFlag == 0) && ((cinfo->VirtualKeyFlag & 0x03) == 0))
		return 1;  //No touch

	if(cinfo->TouchpointFlag == 0x01) // 1 point
	{
		tpd_i2c_master_rs_send(i2c_client,address_p1,(4<<8)|1);
		cinfo->x[0] =address_p1[1]|((address_p1[0]&0x3)<<8);
		cinfo->y[0]= address_p1[3]|((address_p1[2]&0x3)<<8);
	}
	if(cinfo->TouchpointFlag == 0x02) 
	{
		tpd_i2c_master_rs_send(i2c_client,address_p2,(4<<8)|1);
		cinfo->x[1] =address_p2[1]|((address_p2[0]&0x3)<<8);
		cinfo->y[1]= address_p2[3]|((address_p2[2]&0x3)<<8);
	}
	if(cinfo->TouchpointFlag == 0x03) 
	{

		tpd_i2c_master_rs_send(i2c_client,address_p1,(4<<8)|1);
		tpd_i2c_master_rs_send(i2c_client,address_p2,(4<<8)|1);
		cinfo->x[0] =address_p1[1]|((address_p1[0]&0x3)<<8);
		cinfo->y[0]= address_p1[3]|((address_p1[2]&0x3)<<8);
		cinfo->x[1] =address_p2[1]|((address_p2[0]&0x3)<<8);
		cinfo->y[1]= address_p2[3]|((address_p2[2]&0x3)<<8);

	}

	//if(tpd_debuglog==1) {
		//TPD_DMESG("point data [%d %d %d  %d]\n",cinfo->x[0],cinfo->y[0],cinfo->x[1],cinfo->y[1]);
	//}
	
	TPD_DEBUG("point data [%d %d %d  %d]\n",address_p1[0],address_p1[1],address_p1[2],address_p1[3]);	
	TPD_DEBUG("point data [%d %d %d  %d]\n",cinfo->x[0],cinfo->y[0],cinfo->x[1],cinfo->y[1]);
		
	return 0;	
}


 static u8 Calculate_8BitsChecksum( u8 *msg, s32 s32Length )
 {
	 s32 s32Checksum = 0;
	 s32 i;
 
	 for ( i = 0 ; i < s32Length; i++ )
	 {
		 s32Checksum += msg[i];
	 }
 
	 return (u8)( ( -s32Checksum ) & 0xFF );
 }
 
 static void msg21xx_data_disposal(void)
 {
       u8 val[8] = {0};
       u8 Checksum = 0;
	u8 i;
	u32 delta_x = 0, delta_y = 0;
	u32 u32X = 0;
	u32 u32Y = 0;
	u8 touchkeycode = 0;
	TouchScreenInfo_t  touchData;
	static u32 preKeyStatus=0;
	//static u32 preFingerNum=0;
//	printk("xxxxxxxxxxx %s xxxxxxxxx\n", __FUNCTION__);
#define SWAP_X_Y   (1)
//#define REVERSE_X  (1)
//#define REVERSE_Y  (1)
#ifdef SWAP_X_Y
	int tempx;
	int tempy;
#endif



	i2c_master_recv(i2c_client,&val[0],REPORT_PACKET_LENGTH);
     Checksum = Calculate_8BitsChecksum(&val[0], 7); //calculate checksum
    if ((Checksum == val[7]) && (val[0] == 0x52))   //check the checksum  of packet
    {
        u32X = (((val[1] & 0xF0) << 4) | val[2]);         //parse the packet to coordinates
        u32Y = (((val[1] & 0x0F) << 8) | val[3]);

        delta_x = (((val[4] & 0xF0) << 4) | val[5]);
        delta_y = (((val[4] & 0x0F) << 8) | val[6]);

    #ifdef SWAP_X_Y
		tempy = u32X;
		tempx = u32Y;
        u32X = tempx;
        u32Y = tempy;

		tempy = delta_x;
		tempx = delta_y;
        delta_x = tempx;
        delta_y = tempy;
	#endif
	  #ifdef REVERSE_X
	  u32X = 2047 - u32X;
	  delta_x = 4095 - delta_x;
	  #endif
	  #ifdef REVERSE_Y
	  u32Y = 2047 - u32Y;
	  delta_y = 4095 - delta_y;
	  #endif
        //DBG("[HAL] u32X = %x, u32Y = %x", u32X, u32Y);
        //DBG("[HAL] delta_x = %x, delta_y = %x", delta_x, delta_y);

        if ((val[1] == 0xFF) && (val[2] == 0xFF) && (val[3] == 0xFF) && (val[4] == 0xFF) && (val[6] == 0xFF))
        {
            touchData.Point[0].X = 0; // final X coordinate
            touchData.Point[0].Y = 0; // final Y coordinate

           if((val[5]==0x0)||(val[5]==0xFF))
            {
                touchData.nFingerNum = 0; //touch end
                touchData.nTouchKeyCode = 0; //TouchKeyMode
                touchData.nTouchKeyMode = 0; //TouchKeyMode
            }
            else
            {
			touchData.nTouchKeyMode = 1; //TouchKeyMode
			touchData.nTouchKeyCode = val[5]; //TouchKeyCode
			touchData.nFingerNum = 1;
            }
        }
		else
		{
		    touchData.nTouchKeyMode = 0; //Touch on screen...

			if ((delta_x == 0) && (delta_y == 0))
			{
				touchData.nFingerNum = 1; //one touch
				touchData.Point[0].X = (u32X * MS_TS_MSG21XX_X_MAX) / 2048;
				touchData.Point[0].Y = (u32Y * MS_TS_MSG21XX_Y_MAX) / 2048;
//				printk("====x = %d, y = %d ====\n", touchData.Point[0].X, touchData.Point[0].Y);

			}
			else
			{
				u32 x2, y2;

				touchData.nFingerNum = 2; //two touch

				/* Finger 1 */
				touchData.Point[0].X = (u32X * MS_TS_MSG21XX_X_MAX) / 2048;
				touchData.Point[0].Y = (u32Y * MS_TS_MSG21XX_Y_MAX) / 2048;

				/* Finger 2 */
				if (delta_x > 2048)     //transform the unsigh value to sign value
				{
					delta_x -= 4096;
				}
				if (delta_y > 2048)
				{
					delta_y -= 4096;
				}

				x2 = (u32)(u32X + delta_x);
				y2 = (u32)(u32Y + delta_y);

				touchData.Point[1].X = (x2 * MS_TS_MSG21XX_X_MAX) / 2048;
				touchData.Point[1].Y = (y2 * MS_TS_MSG21XX_Y_MAX) / 2048;
			}
		}
		
		//report...
		if(touchData.nTouchKeyMode)
		{
			if (touchData.nTouchKeyCode == 1)
				touchkeycode = KEY_HOME;
			if (touchData.nTouchKeyCode == 2)
				touchkeycode = KEY_MENU;
			if (touchData.nTouchKeyCode == 4)
				touchkeycode = KEY_BACK;
			if (touchData.nTouchKeyCode == 8)
				touchkeycode = KEY_SEARCH;
			

			if(preKeyStatus!=touchkeycode)
			{
				preKeyStatus=touchkeycode;
				input_report_key(tpd->dev, touchkeycode, 1);
				//printk("&&&&&&&&useful key code report touch key code = %d\n",touchkeycode);
			}
			input_sync(tpd->dev);
		}
        else
        {
		    preKeyStatus=0; //clear key status..

            if((touchData.nFingerNum) == 0)   //touch end
            {
				//preFingerNum=0;
				input_report_key(tpd->dev, KEY_MENU, 0);
				input_report_key(tpd->dev, KEY_HOME, 0);
				input_report_key(tpd->dev, KEY_BACK, 0);
				input_report_key(tpd->dev, KEY_SEARCH, 0);

				input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(tpd->dev);
				input_sync(tpd->dev);
            }
            else //touch on screen
            {
			    /*
				if(preFingerNum!=touchData.nFingerNum)   //for one touch <--> two touch issue
				{
					printk("langwenlong number has changed\n");
					preFingerNum=touchData.nFingerNum;
					input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
				    input_mt_sync(tpd->dev);
				    input_sync(tpd->dev);
				}*/

				for(i = 0;i < (touchData.nFingerNum);i++)
				{
					input_report_abs(tpd->dev, ABS_MT_PRESSURE, 255);
					input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 255);
					input_report_abs(tpd->dev, ABS_MT_POSITION_X, touchData.Point[i].X);
					input_report_abs(tpd->dev, ABS_MT_POSITION_Y, touchData.Point[i].Y);
					input_mt_sync(tpd->dev);
				}

				input_sync(tpd->dev);
			}
		}
    }
    else
    {
        //DBG("Packet error 0x%x, 0x%x, 0x%x", val[0], val[1], val[2]);
        //DBG("             0x%x, 0x%x, 0x%x", val[3], val[4], val[5]);
        //DBG("             0x%x, 0x%x, 0x%x", val[6], val[7], Checksum);
		printk(KERN_ERR "err status in tp\n");
    }
 }





static int touch_event_handler(void *unused) {
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };

    int index;
	uint8_t Firmware_version[3] = {0x20,0x00,0x00};

    sched_setscheduler(current, SCHED_RR, &param);
    do {
        set_current_state(TASK_INTERRUPTIBLE);
        if (!kthread_should_stop()) {
            TPD_DEBUG_CHECK_NO_RESPONSE;
            do {
				while (tpd_halt) {tpd_flag = 0;sinfo.TouchpointFlag=0; msleep(20);}
               		wait_event_interruptible(waiter,tpd_flag!=0);
					tpd_flag = 0;
            } while(0);

            TPD_DEBUG_SET_TIME;
        }
       		 set_current_state(TASK_RUNNING);
		 msg21xx_data_disposal();

		
    } while (!kthread_should_stop());
    return 0;
}


int tpd_local_init(void) 
{
	//if(tpd_debuglog==1) {
		//TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__); 
	//}
    boot_mode = get_boot_mode();
    // Software reset mode will be treated as normal boot
    if(boot_mode==3) boot_mode = NORMAL_BOOT;
#if TP_UPDATE

	tpDMABuf_va = (unsigned char *)dma_alloc_coherent(NULL, 4096, &tpDMABuf_pa, GFP_KERNEL);
    if(!tpDMABuf_va){
		printk(KERN_ERR"xxxx Allocate DMA I2C Buffer failed!xxxx\n");
		return -1;
    	}

#endif

	
     if(i2c_add_driver(&tpd_i2c_driver)!=0) {
      TPD_DMESG("unable to add i2c driver.\n");
      return -1;
    }
#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		tpd_type_cap = 1;
    return 0;
}

/* Function to manage low power suspend */
void tpd_suspend(struct early_suspend *h)
{
	//if(tpd_debuglog==1) {
		//TPD_DMESG("[mtk-tpd] %s\n", __FUNCTION__); 
	//}
	 tpd_halt = 1;
   	 mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    tpd_hw_disable();

}

/* Function to manage power-on resume */
void tpd_resume(struct early_suspend *h) 
{

    tpd_hw_enable();

mdelay(200);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
    tpd_halt = 0;
}

static struct tpd_driver_t tpd_device_driver = {
		.tpd_device_name = "mcs6024",
		.tpd_local_init = tpd_local_init,
		.suspend = tpd_suspend,
		.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		.tpd_have_button = 1,
#else
		.tpd_have_button = 0,
#endif		
};
/* called when loaded into kernel */
static int __init tpd_driver_init(void) {
    printk("MediaTek mcs6024 touch panel driver init\n");
		if(tpd_driver_add(&tpd_device_driver) < 0)
			TPD_DMESG("add generic driver failed\n");
    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) {
    TPD_DMESG("MediaTek mcs6024 touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

