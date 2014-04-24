
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
#include <linux/dma-mapping.h>


#include "tpd_custom_mcs6024.h"
#include "tpd.h"
#include <cust_eint.h>

#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif

//if the TP has external power with GPIO pin,need define TPD_HAVE_POWER_ON_OFF in tpd_custom_mcs6024.h
#define TPD_HAVE_POWER_ON_OFF

#define MAX_POINT 2

struct touch_info {
    unsigned int x[MAX_POINT], y[MAX_POINT];
	signed int distx, disty;
    int p[MAX_POINT];
	int pointnum;
	unsigned char key_point;
	int TouchpointFlag;
    int VirtualKeyFlag;
};

struct class *firmware_class;
struct device *firmware_cmd_dev;

static  char *fw_version;

static U8 temp[94][1024];  
static int FwDataCnt;

//add DMA
static unsigned char *tpDMABuf_va = NULL;
static u32 tpDMABuf_pa = NULL;


static struct touch_info cinfo;

int  __flag_first;
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

void change_i2c_addr_ISP(void)
{
	i2c_client->addr = 0x92;
}

void change_i2c_addr_ini(void)
{
	i2c_client->addr = 0x4c;
}

//add DMA
ssize_t mt6573_dma_write_m_byte(unsigned char*returnData_va, u32 returnData_pa, int  len)
{

    int     ret=0;

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

static void tpd_hw_enable(void)
{
    /* CTP_LDO */
        mt_set_gpio_mode(GPIO38, 0);
	mt_set_gpio_dir(GPIO38, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO38, GPIO_OUT_ONE);
   
   	/* CTP_EN */
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
    
    /* CTP_LDO */
        mt_set_gpio_mode(GPIO38, 0);
	mt_set_gpio_dir(GPIO38, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO38, GPIO_OUT_ZERO);
}


static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-tpd");
    return 0;
}

void dbbusDWIICEnterSerialDebugMode()
{
    U8 data[5];

    // Enter the Serial Debug Mode
    data[0] = 0x53;
    data[1] = 0x45;
    data[2] = 0x52;
    data[3] = 0x44;
    data[4] = 0x42;
    i2c_master_send(i2c_client, data, 5);
}

void dbbusDWIICStopMCU()
{
    U8 data[1];

    // Stop the MCU
    data[0] = 0x37;
    i2c_master_send(i2c_client, data, 1);
}

void dbbusDWIICIICUseBus()
{
    U8 data[1];

    // IIC Use Bus
    data[0] = 0x35;
    i2c_master_send(i2c_client, data, 1);
}

void dbbusDWIICIICReshape()
{
    U8 data[1];

    // IIC Re-shape
    data[0] = 0x71;
    i2c_master_send(i2c_client, data, 1);
}


void Get_Chip_Version(void)
{

    unsigned char dbbus_tx_data[3];
    unsigned char dbbus_rx_data[2];

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0xCE;
    i2c_master_send(i2c_client, &dbbus_tx_data[0], 3);
    i2c_master_recv(i2c_client, &dbbus_rx_data[0], 2);
    if (dbbus_rx_data[1] == 0)
    {
        // it is Catch2
       // DBG("*** Catch2 ***\n");
        //FwVersion  = 2;// 2 means Catch2
    }
    else
    {
        // it is catch1
        //DBG("*** Catch1 ***\n");
        //FwVersion  = 1;// 1 means Catch1
    }

}


static ssize_t firmware_version_show(struct device *dev,
                                     struct device_attribute *attr, char *buf)
{
   // DBG("*** firmware_version_show fw_version = %s***\n", fw_version);
    return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_version_store(struct device *dev,
                                      struct device_attribute *attr, const char *buf, size_t size)
{
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();

    unsigned char dbbus_tx_data[3];
    unsigned char dbbus_rx_data[4] ;
    unsigned short major=0, minor=0;
    fw_version = kzalloc(sizeof(char), GFP_KERNEL);

    Get_Chip_Version();
    dbbus_tx_data[0] = 0x53;
    dbbus_tx_data[1] = 0x00;
    dbbus_tx_data[2] = 0x74;
    i2c_master_send(i2c_client, &dbbus_tx_data[0], 3);                             
    i2c_master_recv(i2c_client, &dbbus_rx_data[0], 4);
    major = (dbbus_rx_data[1]<<8)+dbbus_rx_data[0];
    minor = (dbbus_rx_data[3]<<8)+dbbus_rx_data[2];
    printk("***major =0x %x ***\n", major);
    printk("***minor = 0x%x ***\n", minor);
    sprintf(fw_version,"%03d%03d", major, minor);
    printk("***fw_version = 0x%x ***\n", fw_version);

    return size;
}

static DEVICE_ATTR(version, 0664, firmware_version_show, firmware_version_store);

void dbbusDWIICIICNotUseBus()
{
    U8 data[1];

    // IIC Not Use Bus
    data[0] = 0x34;
    i2c_master_send(i2c_client, data, 1);
}

void dbbusDWIICNotStopMCU()
{
    U8 data[1];

    // Not Stop the MCU
    data[0] = 0x36;
    i2c_master_send(i2c_client, data, 1);
}

void dbbusDWIICExitSerialDebugMode()
{
    U8 data[1];

    // Exit the Serial Debug Mode
    data[0] = 0x45;
    i2c_master_send(i2c_client, data, 1);

    // Delay some interval to guard the next transaction
    udelay ( 200 );        // delay about 0.2ms
}

void drvISP_EntryIspMode(void)
{
    U8 bWriteData[5] =
    {
        0x4D, 0x53, 0x54, 0x41, 0x52
    };
	
    i2c_master_send(i2c_client, bWriteData, 5);     //**i2c_clent

    //udelay ( 1000 );        // delay about 1ms
}

U8 drvISP_Read(U8 n, U8* pDataToRead)    //First it needs send 0x11 to notify we want to get flash data back.
{
    U8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0};

    i2c_master_send(i2c_client, &Read_cmd, 1);     //**i2c_client
    if (n == 1)
    {
        i2c_master_recv(i2c_client, &dbbus_rx_data[0], 2);
        *pDataToRead = dbbus_rx_data[0];
    }
    else
        i2c_master_recv(i2c_client, pDataToRead, n);
}


U8 mt6573_dma_read_m_byte(U8 n, U8* returnData_va,  u32 returnData_pa)    //First it needs send 0x11 to notify we want to get flash data back.
{
    U8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0};
    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
    i2c_master_send(i2c_client, &Read_cmd, 1);     //**i2c_client
 //	   ret = i2c_master_send(i2c_client, returnData_pa, len);
     i2c_client->addr = (i2c_client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
     i2c_master_recv(i2c_client, returnData_pa, n);
}


void drvISP_WriteEnable(void)
{
    U8 bWriteData[2] =
    {
        0x10, 0x06
    };
    U8 bWriteData1 = 0x12;
	i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
    i2c_master_send(i2c_client, bWriteData, 2);     //**i2c_client
    i2c_master_send(i2c_client, &bWriteData1, 1);
}

U8 drvISP_ReadStatus()
{
    U8 bReadData = 0;
    U8 bWriteData[2] =
    {
        0x10, 0x05
    };
    U8 bWriteData1 = 0x12;
 	i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
    i2c_master_send(i2c_client, bWriteData, 2);   //**i2c_client
    drvISP_Read(1, &bReadData);
    i2c_master_send(i2c_client, &bWriteData1, 1);

    return bReadData;
}

void drvISP_BlockErase(U32 addr)
{
    U8 bWriteData[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    U8 bWriteData1 = 0x12;

    drvISP_WriteEnable();

    //Enable write status register
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x50;
    i2c_master_send(i2c_client, bWriteData, 2);            //**i2c_client
    i2c_master_send(i2c_client, &bWriteData1, 1);

    //Write Status
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x01;
    bWriteData[2] = 0x00;
    i2c_master_send(i2c_client, bWriteData, 3);
    i2c_master_send(i2c_client, &bWriteData1, 1);

    //Write disable
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x04;
    i2c_master_send(i2c_client, bWriteData, 2);
    i2c_master_send(i2c_client, &bWriteData1, 1);

    while ((drvISP_ReadStatus() & 0x01) == 0x01);

    drvISP_WriteEnable();

    bWriteData[0] = 0x10;
    bWriteData[1] = 0xD8;        //Block Erase
    bWriteData[2] = ((addr >> 16) & 0xFF);
    bWriteData[3] = ((addr >> 8) & 0xFF);
    bWriteData[4] = (addr & 0xFF) ;
    i2c_master_send(i2c_client, bWriteData, 5);
    i2c_master_send(i2c_client, &bWriteData1, 1);

    while ((drvISP_ReadStatus() & 0x01) == 0x01);
}

void drvISP_Program(U16 k, U8* pDataToWrite)
{
    U16 i = 0;
    U16 j = 0;

    U8 TX_data[8];
    U8 bWriteData1 = 0x12;
    U32 addr = k * 1024;

    for (j = 0; j < 8; j++)
    {
	tpDMABuf_va[0] = 0x10;
	tpDMABuf_va[1] = 0x02;
	tpDMABuf_va[2] = ((addr + 128 * j) >> 16) & 0xFF;
	tpDMABuf_va[3] = ((addr + 128 * j) >> 8) & 0xFF;
	tpDMABuf_va[4] = (addr + 128 * j) & 0xFF;
	
	 for (i = 0; i < 128; i++)
        {
		tpDMABuf_va[5 + i] = pDataToWrite[j * 128 + i];
        }
	while((drvISP_ReadStatus() & 0x01) == 0x01);
	drvISP_WriteEnable();

	mt6573_dma_write_m_byte(tpDMABuf_va, tpDMABuf_pa, 133);

	i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
	i2c_master_send(i2c_client, &bWriteData1, 1);
    }
}


void drvISP_Verify(U16 k, U8* pDataToVerify)
{
    U16 i = 0, j = 0, m = 0;
    U8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    U8 RX_data[256];
    U8 bWriteData1 = 0x12;
    U32 addr = k * 1024;
	i2c_client->addr = (i2c_client->addr & I2C_MASK_FLAG);
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        bWriteData[2] = (U8)((addr + j * 128) >> 16);
        bWriteData[3] = (U8)((addr + j * 128) >> 8);
        bWriteData[4] = (U8)(addr + j * 128);
        while ((drvISP_ReadStatus() & 0x01) == 0x01);     //wait until not in write operation
        i2c_master_send(i2c_client, bWriteData, 5);    //write read flash addr         

	mt6573_dma_read_m_byte(128, tpDMABuf_va, tpDMABuf_pa);

        for (i = 0; i < 128; i++)   //log out if verify error
        {
            if (tpDMABuf_va[i] != pDataToVerify[128 * j + i])
            {
                printk("===============Update Firmware Error================\n");
            }
        }
    }
}

void drvISP_ExitIspMode(void)
{
    U8 bWriteData = 0x24;
i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
    i2c_master_send(i2c_client, &bWriteData, 1);      //**i2c_client
}

static ssize_t firmware_update_show(struct device *dev,
                                    struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "###############%s\n", fw_version);
}



static ssize_t firmware_update_store(struct device *dev,
                                     struct device_attribute *attr, const char *buf, size_t size)
{
    U8 i;
    U8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};
    int fd;
    //msctpc_LoopDelay ( 100 );        // delay about 100ms*****

    // Enable slave's ISP ECO mode
    /*dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();*/

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x08;
    dbbus_tx_data[2] = 0x0c;
    dbbus_tx_data[3] = 0x08;

    // Disable the Watchdog
    i2c_master_send(i2c_client, dbbus_tx_data, 4);



    //Get_Chip_Version();

        dbbus_tx_data[0] = 0x10;
        dbbus_tx_data[1] = 0x11;
        dbbus_tx_data[2] = 0xE2;
        dbbus_tx_data[3] = 0x00;

    i2c_master_send(i2c_client, dbbus_tx_data, 4);

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    i2c_master_send(i2c_client, dbbus_tx_data, 4);
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    i2c_master_send(i2c_client, dbbus_tx_data, 4);

//Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    i2c_master_send(i2c_client, dbbus_tx_data, 4);

//Enable SPI Pad
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x02;
    i2c_master_send(i2c_client, dbbus_tx_data, 3);
    i2c_master_recv(i2c_client, &dbbus_rx_data[0], 2);
    //DBG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
    dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
    i2c_master_send(i2c_client, dbbus_tx_data, 4);

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x25;
    i2c_master_send(i2c_client, dbbus_tx_data, 3);
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    i2c_master_recv(i2c_client, &dbbus_rx_data[0], 2);
    //DBG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xFC;  //Clear Bit 1,0
    i2c_master_send(i2c_client, dbbus_tx_data, 4);

//WP overwrite
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x0E;
    dbbus_tx_data[3] = 0x02;
    i2c_master_send(i2c_client, dbbus_tx_data, 4);

//set pin high
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x10;
    dbbus_tx_data[3] = 0x08;
    i2c_master_send(i2c_client, dbbus_tx_data, 4);

    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();

    ///////////////////////////////////////
    // Start to load firmware
    ///////////////////////////////////////
    change_i2c_addr_ISP();
    drvISP_EntryIspMode();
    drvISP_BlockErase(0x00000);
    for (i = 0; i < 94; i++)   // total  94 KB : 1 byte per R/W
    {
        drvISP_Program(i, temp[i]);    // program to slave's flash
        drvISP_Verify ( i, temp[i] ); //verify data
    }
    drvISP_ExitIspMode();
    printk("--------->ISP finished.\n\r");
    change_i2c_addr_ini();
    FwDataCnt = 0;
    return size;
}

static DEVICE_ATTR(update, 0664, firmware_update_show, firmware_update_store);

static ssize_t firmware_data_show(struct device *dev,
                                  struct device_attribute *attr, char *buf)
{
    return FwDataCnt;
}

static ssize_t firmware_data_store(struct device *dev,
                                   struct device_attribute *attr, const char *buf, size_t size)
{
    int i;
    for (i = 0; i < 1024; i++)
    {
        memcpy(temp[FwDataCnt], buf, 1024);
    }
    FwDataCnt++;
    return size;
}
static DEVICE_ATTR(data, 0664, firmware_data_show, firmware_data_store);

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {             
	int err = 0,i;
	unsigned char tpd_buf[8] = {0};
	i2c_client = client;

	printk("**********%s**********\n", __FUNCTION__);

	 firmware_class = class_create(THIS_MODULE, "ms-touchscreen-msg20xx");               
	 if (IS_ERR(firmware_class))
        pr_err("Failed to create class(firmware)!\n");

        firmware_cmd_dev = device_create(firmware_class,  NULL, 0, NULL, "device");
	if (IS_ERR(firmware_cmd_dev))
        pr_err("Failed to create device(firmware_cmd_dev)!\n");

     // version
   	 if (device_create_file(firmware_cmd_dev, &dev_attr_version) < 0)
        pr_err("Failed to create device file(%s)!\n", dev_attr_version.attr.name);
    // update
 	  if (device_create_file(firmware_cmd_dev, &dev_attr_update) < 0)
        pr_err("Failed to create device file(%s)!\n", dev_attr_update.attr.name);
    // data
   	 if (device_create_file(firmware_cmd_dev, &dev_attr_data) < 0)
     	  pr_err("Failed to create device file(%s)!\n", dev_attr_data.attr.name);

  	  dev_set_drvdata(firmware_cmd_dev, NULL);

	  
	#ifdef TPD_HAVE_POWER_ON_OFF
	tpd_hw_enable();

	//for power on sequence
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	
	#endif

	msleep(200);
	
	if(i2c_master_recv( i2c_client , &tpd_buf[0], 8) < 0)
	{	
		printk("**********i2c transfer error!\n");
		return -1;
	}

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
   printk("**********%s OK!**********\n", __FUNCTION__);
	__flag_first = 1;   
    return 0;
}

void tpd_eint_interrupt_handler(void) { 
	//if(tpd_em_debuglog==1) 
		//TPD_DMESG("[mtk-tpd], %s\n", __FUNCTION__);
    TPD_DEBUG_PRINT_INT; 
	tpd_flag=1; 
	wake_up_interruptible(&waiter);
} 

static int tpd_i2c_remove(struct i2c_client *client) {return 0;}

unsigned char checksum(unsigned char *msg, uint8_t num)
 {
	uint8_t i;
 	signed long checksum_1 = 0;
 	for(i=0; i<num; i++)
 	{
		checksum_1 += msg[i];
	}
	return ((unsigned char)(-checksum_1)&0xff);
}


static int touch_event_handler(void *unused) {
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
    unsigned char mstar[8] = {0};
	int i=0;
	unsigned short mid=0;
	unsigned short Position_X=0,Position_Y=0,Distance_X=0,Distance_Y=0;

    sched_setscheduler(current, SCHED_RR, &param);
    do {
        set_current_state(TASK_INTERRUPTIBLE);
        if (!kthread_should_stop()) {
//            TPD_DEBUG_CHECK_NO_RESPONSE;
            do {
				while (tpd_halt) {tpd_flag = 0; msleep(20);}
               		wait_event_interruptible(waiter,tpd_flag!=0);
					tpd_flag = 0;
            } while(0);

            TPD_DEBUG_SET_TIME;
        }
       		 set_current_state(TASK_RUNNING);


		i2c_master_recv( i2c_client , &mstar[0], 8);

		if((mstar[0] == 0x52)&&(mstar[7] == checksum(mstar, 7)))
		{
			mid = (unsigned short)mstar[1];
			Position_X = (mid<<4) & 0x0F00;
			Position_X = Position_X | ((unsigned short)mstar[2]);
			Position_Y = (mid<<8) & 0x0F00;
			Position_Y = Position_Y |( (unsigned short)mstar[3]);

			mid = (unsigned short)mstar[4];
			Distance_X = (mid<<4) & 0x0F00;
			Distance_X = Distance_X |((unsigned short)mstar[5]);
			Distance_Y = (mid<<8) & 0x0F00;
			Distance_Y = Distance_Y |((unsigned short) mstar[6]);

			cinfo.x[0] = Position_X;
			cinfo.y[0] = Position_Y;
			cinfo.distx = Distance_X;
			cinfo.disty = Distance_Y;

			if(cinfo.distx > 2048)
				cinfo.distx -= 4096;
			cinfo.x[1] = cinfo.x[0] + cinfo.distx;

			cinfo.disty <<= 4;
			cinfo.disty /= 16;

			if(cinfo.disty > 2048)
				cinfo.disty -= 4096;
			cinfo.y[1] = cinfo.y[0] + cinfo.disty;

			if((cinfo.x[0] == 0xfff) && (cinfo.y[0] == 0xfff)){
				cinfo.pointnum = 0;
			}
			else if((cinfo.distx == 0) && (cinfo.disty ==0)){
				cinfo.pointnum = 1;
				cinfo.x[0] = cinfo.x[0] * 320 /2048;
				cinfo.y[0] = cinfo.y[0] * 480 /2048;
			}
			else{
				cinfo.pointnum = 2;
				cinfo.x[0] = cinfo.x[0] * 320 /2048;
				cinfo.y[0] = cinfo.y[0] * 480 /2048;
				cinfo.x[1] = cinfo.x[1] * 320 /2048;
				cinfo.y[1] = cinfo.y[1] * 480 /2048;
			}

			for(i =0; i<cinfo.pointnum; i++)
			{	
				input_report_abs(tpd->dev, ABS_PRESSURE, 128);
				input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 255);
				input_report_abs(tpd->dev, ABS_MT_POSITION_X, cinfo.x[i]);
				input_report_abs(tpd->dev, ABS_MT_POSITION_Y, cinfo.y[i]);
				input_mt_sync(tpd->dev);
			}

			if(cinfo.pointnum == 0)
			{
				input_report_abs(tpd->dev, ABS_PRESSURE, 0);
				input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_key(tpd->dev, KEY_MENU, 0);
				input_report_key(tpd->dev, KEY_SEARCH, 0);
				input_report_key(tpd->dev, KEY_BACK, 0);
				input_mt_sync(tpd->dev);
			}
		
    		}
		else if((mstar[0] == 0x53)&&(mstar[7] == checksum(mstar, 7))){
			if((mstar[1] == 0xff)&&(mstar[2] == 0xff)&&(mstar[3] == 0xff)&&(mstar[4] == 0xff)&&(mstar[6] ==0xff))
			{
				if(mstar[5] & 0x07){
					cinfo.key_point = mstar[5]&0x07;
					input_report_abs(tpd->dev, ABS_PRESSURE, 128);
					input_report_key(tpd->dev, BTN_TOUCH, 1);
					input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 128);
					if(cinfo.key_point == 1){
						//input_report_key(tpd->dev, KEY_BACK, 1);
						input_report_abs(tpd->dev, ABS_MT_POSITION_X, tpd_keys_dim_local[2][0]);
						input_report_abs(tpd->dev, ABS_MT_POSITION_Y, tpd_keys_dim_local[2][1]);
						}
					else if(cinfo.key_point == 2){
						//input_report_key(tpd->dev, KEY_MENU, 1);
						input_report_abs(tpd->dev, ABS_MT_POSITION_X, tpd_keys_dim_local[0][0]);
						input_report_abs(tpd->dev, ABS_MT_POSITION_Y, tpd_keys_dim_local[0][1]);
						}
					else if(cinfo.key_point == 4){
						//input_report_key(tpd->dev, KEY_SEARCH, 1);
						input_report_abs(tpd->dev, ABS_MT_POSITION_X, tpd_keys_dim_local[1][0]);
						input_report_abs(tpd->dev, ABS_MT_POSITION_Y, tpd_keys_dim_local[1][1]);
						}	
					input_mt_sync(tpd->dev);
				}
				else{
					input_report_abs(tpd->dev, ABS_PRESSURE, 0);
					input_report_key(tpd->dev, BTN_TOUCH, 0);
					input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
					input_report_abs(tpd->dev, ABS_MT_POSITION_X, 0);
					input_report_abs(tpd->dev, ABS_MT_POSITION_Y, 0);
					input_mt_sync(tpd->dev);
			}

		    }
		
		}
		input_sync(tpd->dev);
		
			
    } while (!kthread_should_stop());
    return 0;
}


int tpd_local_init(void) 
{
	printk("**********%s**********\n", __FUNCTION__);

    boot_mode = get_boot_mode();
    // Software reset mode will be treated as normal boot
    if(boot_mode==3) boot_mode = NORMAL_BOOT;

//add DMA
	tpDMABuf_va = (unsigned char *)dma_alloc_coherent(NULL, 4096, &tpDMABuf_pa, GFP_KERNEL);
    if(!tpDMABuf_va){
		printk(KERN_ERR"xxxx Allocate DMA I2C Buffer failed!xxxx\n");
		return -1;
    }
    
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
	printk("**********%s OK!**********\n", __FUNCTION__);

    return 0;
}

/* Function to manage low power suspend */
void tpd_suspend(struct early_suspend *h)
{
	 tpd_halt = 1;
   	 mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
#ifdef TPD_HAVE_POWER_ON_OFF
    tpd_hw_disable();
#endif

}

/* Function to manage power-on resume */
void tpd_resume(struct early_suspend *h) 
{
	
#ifdef TPD_HAVE_POWER_ON_OFF
	tpd_hw_enable();
	msleep(90);
#endif

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	tpd_halt = 0;
}

static struct tpd_driver_t tpd_device_driver = {
		.tpd_device_name = "mstar_tp",
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
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

