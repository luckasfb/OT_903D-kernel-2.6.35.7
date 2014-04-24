

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/i2c.h>
#include <linux/spinlock.h>
#include <mach/mt6516_typedefs.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include "pmic6326_hw.h"

#include "pmic6326_sw.h" // 20100722
//#include "MT6326PMIC_sw.h" // 20100319

// 20100722
//#ifdef CONFIG_PMIC_DCT
//#include "pmic_drv.h"
//#else
//#include "pmic_drv_nodct.h"
//#endif

#include <mach/mt6516_gpio.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include <cust_eint.h>
#include <cust_leds_def.h>

//#define CONFIG_TESTCASE_MSG

#define PMIC_DEVNAME "MT6326-pmic"

#define TEST_PMIC_PRINT 	_IO('k', 0)
#define PMIC_READ 			_IOW('k', 1, int)
#define PMIC_WRITE 			_IOW('k', 2, int)
#define SET_PMIC_LCDBK 		_IOW('k', 3, int)
#define ENABLE_VIBRATOR 	_IO('k', 4)
#define DISABLE_VIBRATOR 	_IO('k', 5)

static struct class *pmic_class = NULL;
static int pmic_major = 0;
static dev_t pmic_devno;
static struct cdev *pmic_cdev;

int pmic_in_data[2] = {1,1};
int pmic_out_data[2] = {1,1};
int pmic_lcdbk_data[1] = {1};

extern void MT6516_EINT_Registration (
    kal_uint8 eintno, 
    kal_bool Dbounce_En, 
    kal_bool ACT_Polarity,
    void (EINT_FUNC_PTR)(void), 
    kal_bool auto_umask
);
extern void MT6516_EINT_Set_Polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern kal_uint32 MT6516_EINT_Set_Sensitivity(kal_uint8 eintno, kal_bool sens);
extern void MT6516_EINT_Set_HW_Debounce(kal_uint8 eintno, kal_uint32 ms);
extern void hwPMMonitorInit(void);
extern int DVFS_thread_kthread(void *x);
extern void wake_up_bat (void);
extern struct wake_lock battery_suspend_lock;

extern int g_chr_event;

#define mt6326_SLAVE_ADDR_WRITE	0xC0
#define mt6326_SLAVE_ADDR_Read	0xC1

/* Addresses to scan */
static unsigned short normal_i2c[] = { mt6326_SLAVE_ADDR_WRITE,  I2C_CLIENT_END };
static unsigned short ignore = I2C_CLIENT_END;

#if 0
//20100827
static struct i2c_client_address_data addr_data = {
	.normal_i2c = normal_i2c,
	.probe	= &ignore,
	.ignore	= &ignore,
};
#endif

static struct i2c_client *new_client = NULL;

//static int mt6326_attach_adapter(struct i2c_adapter *adapter);
//static int mt6326_detect(struct i2c_adapter *adapter, int address, int kind);
//static int mt6326_detach_client(struct i2c_client *client);

#if 0
//20100827
/* This is the driver that will be inserted */
static struct i2c_driver mt6326_driver = {
	.attach_adapter	= mt6326_attach_adapter,
	.detach_client	= mt6326_detach_client,
	.driver = 	{
	    .name		    = "mt6326",
	},	
};
#endif

static const struct i2c_device_id mt6326_i2c_id[] = {{"mt6326_pmic",0},{}};   
static unsigned short force[] = {1, 0xC0, I2C_CLIENT_END, I2C_CLIENT_END};   
static const unsigned short * const forces[] = { force, NULL };              
static struct i2c_client_address_data addr_data = { .forces = forces,}; 

static int mt6326_driver_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int mt6326_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

struct i2c_driver mt6326_driver = {                       
    .probe = mt6326_driver_probe,                                   
    //.remove = mt6326_driver_remove,                           
    .detect = mt6326_driver_detect,                           
    .driver.name = "mt6326_pmic",                 
    .id_table = mt6326_i2c_id,                             
    .address_data = &addr_data,                        
};

#if 0  
static ssize_t mt6326_read(struct kobject *kobj, char *buf, loff_t off, size_t count)
{
    u8     cmd_buf[2]={0x81,0x82};
    u8     readData = 0;
    int     i=0,ret=0;

    printk(" mt6326_read start !!\n ");
    for (i = 0; i < sizeof(cmd_buf); i++) {
        ret = i2c_master_send(new_client, &cmd_buf[i], 1);
        if (ret < 0) {
            printk("sends command error!! \n");
            return 0;
        }
        ret = i2c_master_recv(new_client, &readData, 1);
        if (ret < 0) {
            printk("reads data error!! \n");
            return 0;
        }
        printk(" mt6326_detect : cmd %x . read data %x!!\n ", cmd_buf[i], readData);
    }
    printk(" mt6326_read end !!\n ");

    return 0;
}

static ssize_t mt6326_write(struct kobject *kobj, char *buf, loff_t off, size_t count)
{
    u8    write_data[2] = {0};
    int    ret=0;
    
    write_data[0] = 0x81;
    write_data[1] = 0x44;
    
    printk(" mt6326_write start !!\n ");
    ret = i2c_master_send(new_client, write_data, 2);
    if (ret < 0) {
        printk("sends command error!! \n");
        return 0;
    }
    printk(" mt6326_write end !!\n ");
    
    return 0;
}

static struct bin_attribute mt6326_attr_read = {
	.attr = {
		.name = "mt6326_read",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_read,
};

static struct bin_attribute mt6326_attr_write = {
	.attr = {
		.name = "mt6326_write",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_write,
};
#endif

ssize_t mt6326_read_byte(u8 cmd, u8 *returnData)
{
    char     cmd_buf[1]={0x00};
    char     readData = 0;
    int     ret=0;

    cmd_buf[0] = cmd;
    ret = i2c_master_send(new_client, &cmd_buf[0], 1);
    if (ret < 0) {
		//#ifdef CONFIG_TESTCASE_MSG
        printk("PMIC sends command error!! \n");
		//#endif
        return 0;
    }
    ret = i2c_master_recv(new_client, &readData, 1);
    if (ret < 0) {
		//#ifdef CONFIG_TESTCASE_MSG
        printk("PMIC reads data error!! \n");
		//#endif
        return 0;
    } 
    //printk("func mt6326_read_byte : 0x%x \n", readData);
    *returnData = readData;
    
    return 1;
}

ssize_t mt6326_write_byte(u8 cmd, u8 writeData)
{
    char    write_data[2] = {0};
    int    ret=0;
    
    write_data[0] = cmd;         // ex. 0x81
    write_data[1] = writeData;// ex. 0x44
    
    ret = i2c_master_send(new_client, write_data, 2);
    if (ret < 0) {
		#ifdef CONFIG_TESTCASE_MSG
        printk("sends command error!! \n");
		#endif
        return 0;
    }
    
    return 1;
}

//static spinlock_t mt6326_pmic_spinlock = SPIN_LOCK_UNLOCKED;
kal_uint8 pmic6326_eco_version = 0;
kal_uint8 pmic6326_reg[PMIC_REG_NUM] = {0};

// BL, Flash, OTG LDOs need higher voltage output, we also need to enable vboost1 when enabling the 3 LDOs
#define VBOOST1_SET_FLAG_BL			0x01		// Bit00
#define VBOOST1_SET_FLAG_FLASH		0x02		// Bit01
#define VBOOST1_SET_FLAG_OTG			0x04		// Bit02
#define VBOOST1_SET_FLAG_BOOST1		0x08		// Bit03
#define PMIC_DEFAULT_BL_DIM			0x8
#define PMIC_DEFAULT_KP_DIM			0x10
#define DEFAULT_VIBR_DUTY			31

kal_uint8 pmic6326_vboost1_set_flag = 0;
// Internal usage only, called insided PMIC6326 driver
// Do NOT need protection check, because it is called inside PMIC API
#define pmic_boost1_enable_internal(flag)		{\
	kal_uint8 enable;\
	enable = (flag == 0)?0:1;\
	pmic6326_reg[0x5D] &= ~(BOOST1_EN_MASK << BOOST1_EN_SHIFT);\
	pmic6326_reg[0x5D] |= (enable << BOOST1_EN_SHIFT);\
	mt6326_write_byte(0x5D, pmic6326_reg[0x5D]);\
}

// FLASH, Keypad LED, Vibrator LDOs need to control DIM, so we need to turn on 0x72 when any of them are turned on
#define DIM_CK_ON_FLAG_CK				0x01		// Bit00
#define DIM_CK_ON_FLAG_FLASH			0x02		// Bit01
#define DIM_CK_ON_FLAG_KEY				0x04		// Bit02
#define DIM_CK_ON_FLAG_VIB				0x08		// Bit03
// Internal usage only, called insided PMIC6326 driver
// Do NOT need protection check, because it is called inside PMIC API
kal_uint8 pmic6326_dim_ck_set_flag = 0;
#define pmic_dim_ck_force_on_internal(flag)		{\
	kal_uint8 enable;\
	enable = (flag == 0)?0:1;\
	pmic6326_reg[0x72] &= ~(DIM_CK_FORCE_ON_MASK << DIM_CK_FORCE_ON_SHIFT);\
	pmic6326_reg[0x72] |= (enable << DIM_CK_FORCE_ON_SHIFT);\
	mt6326_write_byte(0x72, pmic6326_reg[0x72]);\
}

#define VRF_CAL					0							/* 0 ~ 15 */
#define VTCXO_CAL					0							/* 0 ~ 15 */
#define V3GTX_CAL					0							/* 0 ~ 15 */
#define V3GTX_SEL					V3GTX_2_8				/* V3GTX_2_8/3_0/3_3/2_5 */
#define V3GRX_CAL					0							/* 0 ~ 15 */
#define V3GRX_SEL					V3GRX_2_8				/* V3GRX_2_8/3_0/3_3/2_5 */
#define VCAMA_CAL					0							/* 0 ~ 15 */
#define VCAMA_SEL					VCAMA_2_8				/* VCAMA_2_8/2_5/1_8/1_5 */
#define VWIFI3V3_CAL				0							/* 0 ~ 15 */
#define VWIFI3V3_SEL				VWIFI3V3_2_8			/* VWIFI3V3_2_8/3_0/3_3/2_5 */
#define VWIFI2V8_CAL				0							/* 0 ~ 15 */
#define VWIFI2V8_SEL				VWIFI2V8_2_8			/* VWIFI2V8_2_8/3_0/3_3/2_5 */
#define VSIM_CAL					0							/* 0 ~ 15 */
#define VBT_CAL					0							/* 0 ~ 15 */
#define VBT_SEL					VBT_1_3					/* VBT_1_3/1_5/1_8/2_5/2_8/3_0/3_3/1_2 */
#define VCAMD_CAL					0							/* 0 ~ 15 */
#define VCAMD_SEL					VCAMD_1_3				/* VCAMD_1_3/1_5/1_8/2_5/2_8/3_0/3_3/1_2 */
#define VGP_CAL					0							/* 0 ~ 15 */
#define VGP_SEL					VGP_1_3					/* VGP_1_3/1_5/1_8/2_5/2_8/3_0/3_3 */
#define VSDIO_CAL					0							/* 0 ~ 15 */
#define VSDIO_SEL					VSDIO_2_8				/* VSDIO_2_8/3_0 */
#define VGP2_SEL					VGP2_1_3					/* VGP2_1_3/1_5/1_8/2_5/2_8/3_0/3_3 */
#define VGP2_ON_SEL				VGP2_ENABLE_WITH_VGP2_EN	/* VGP2_ENABLE_WITH_SRCLKEN */
#define CHR_OFFSET				CHR_CURRENT_OFFSET_NO	/* CHR_CURRENT_OFFSET_PLUS_1_STEP/CHR_CURRENT_OFFSET_PLUS_2_STEP/
																		   CHR_CURRENT_OFFSET_MINUS_2_STEP/CHR_CURRENT_OFFSET_MINUS_1_STEP */
#define AC_CHR_CURRENT        CHR_CURRENT_650MA		/* CHR_CURRENT_50MA/90MA/150MA/225MA/300MA/450MA */
#define USB_CHR_CURRENT       CHR_CURRENT_450MA		/* CHR_CURRENT_50MA/90MA/150MA/225MA/300MA */
#define BOOST_MODE				BOOST_MODE_TYPE_I		/* BOOST_MODE_TYPE_II/BOOST_MODE_TYPE_III/BOOST_MODE_TYPE_IV */
#define BL_NUM						BL_NUM_4					/* BL_NUM_1/2/3/4/5/6/7/8 */
#define ASW_ASEL					ASW_ASEL_ISINK_6_8_AS	/* ASW_ASEL_ALL_ISINK_BL */
#define VBOOST1_TUNE				VBOOST1_VOL_3_20_V	/* 3_35_V/3_50_V/3_50_V/3_65_V3_80_V/3_95_V/4_10_V/4_25_V/4_40_V/
																	   4_55_V/4_70_V/4_85_V/5_00_V/5_15_V/5_30_V/5_45_V */
#define BL_I_COARSE_TUNE		BL_I_CORSE_TUNE_20MA	/* 4MA/8MA/12MA/16MA/20MA/24MA/28MA/32MA */
#define ASW_BSEL					HI_Z						/* RECEIVER/TWO_OF_RGB_DRIVER */
#define VCORE2_ON_SEL			VCORE2_ENABLE_WITH_EN_PASS		/* VCORE2_ENABLE_WITH_VCORE2_EN */
#define VCORE2_EN					KAL_TRUE					/* KAL_FALSE */
#define USE_SPKL					KAL_TRUE					/* KAL_FALSE */
#define USE_SPKR					KAL_TRUE					/* KAL_FALSE */

/* (0x09) STATUS 6 (RO) */
kal_bool pmic_boost2_oc_status(void){    
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (BOOST2_OC_STATUS_MASK << BOOST2_OC_STATUS_SHIFT)){
            return KAL_TRUE;
    }else{
            return KAL_FALSE;
    }
}

kal_bool pmic_spkr_oc_det_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (SPKR_OC_DET_MASK << SPKR_OC_DET_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_spkl_oc_det_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (SPKL_OC_DET_MASK << SPKL_OC_DET_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_pwrkey_deb_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (PWRKEY_DEB_MASK << PWRKEY_DEB_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_ovp_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (OVP_MASK << OVP_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_chrdet_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (CHRDET_MASK << CHRDET_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_bat_on_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (BAT_ON_MASK << BAT_ON_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_cv_status(void){
    mt6326_read_byte(0x09, &pmic6326_reg[0x09]);//pmic6326_reg[0x09] = pmic6326_reg_read(0x09);
    if (pmic6326_reg[0x09] & (CV_MASK << CV_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

/* (0x0D) INT STATUS 3 (RO) */
kal_uint8 pmic_int_status_3(void){
    mt6326_read_byte(0x0D, &pmic6326_reg[0x0D]);//pmic6326_reg[0x0D] = pmic6326_reg_read(0x0D);
    return pmic6326_reg[0x0D];
}

kal_bool pmic_vsdio_oc_int_status(void){
    mt6326_read_byte(0x0D, &pmic6326_reg[0x0D]);//pmic6326_reg[0x0D] = pmic6326_reg_read(0x0D);
    if (pmic6326_reg[0x0D] & (VSDIO_OC_FLAG_STATUS_MASK << VSDIO_OC_FLAG_STATUS_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_vgp_oc_int_status(void){
    mt6326_read_byte(0x0D, &pmic6326_reg[0x0D]);//pmic6326_reg[0x0D] = pmic6326_reg_read(0x0D);
    if (pmic6326_reg[0x0D] & (VGP_OC_FLAG_STATUS_MASK << VGP_OC_FLAG_STATUS_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_vusb_oc_int_status(void){
    mt6326_read_byte(0x0D, &pmic6326_reg[0x0D]);//pmic6326_reg[0x0D] = pmic6326_reg_read(0x0D);
    if (pmic6326_reg[0x0D] & (VUSB_OC_FLAG_STATUS_MASK << VUSB_OC_FLAG_STATUS_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_ovp_int_status(void){
    mt6326_read_byte(0x0D, &pmic6326_reg[0x0D]);//pmic6326_reg[0x0D] = pmic6326_reg_read(0x0D);
    if (pmic6326_reg[0x0D] & (OVP_INT_FLAG_STATUS_MASK << OVP_INT_FLAG_STATUS_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

kal_bool pmic_chrdet_int_status(void){
    mt6326_read_byte(0x0D, &pmic6326_reg[0x0D]);//pmic6326_reg[0x0D] = pmic6326_reg_read(0x0D);
    if (pmic6326_reg[0x0D] & (CHRDET_INT_FLAG_STATUS_MASK << CHRDET_INT_FLAG_STATUS_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

/* (0x0E) INT STATUS 4 (RO) */
kal_uint8 pmic_int_status_4(void){
    mt6326_read_byte(0x0E, &pmic6326_reg[0x0E]);//pmic6326_reg[0x0E] = pmic6326_reg_read(0x0E);
    return pmic6326_reg[0x0E];
}

kal_bool pmic_watchdog_int_status(void){
    mt6326_read_byte(0x0E, &pmic6326_reg[0x0E]);//pmic6326_reg[0x0E] = pmic6326_reg_read(0x0E);
    if (pmic6326_reg[0x0E] & (WATCHDOG_INT_FLAG_STATUS_MASK << WATCHDOG_INT_FLAG_STATUS_SHIFT)){
        return KAL_TRUE;
    }else{
        return KAL_FALSE;
    }
}

/* Write watchdog bit to clear watch dog timer */
void pmic_watchdog_clear(void){
    mt6326_write_byte(0x0E, (WATCHDOG_INT_FLAG_STATUS_MASK << WATCHDOG_INT_FLAG_STATUS_SHIFT));
}

/* (0x1B) LDO CTRL 2 VRF */
void pmic_vrf_ical_en(vrf_ical_en_enum sel){
    pmic6326_reg[0x1B] &= ~(VRF_ICAL_EN_MASK << VRF_ICAL_EN_SHIFT);
    pmic6326_reg[0x1B] |= ((kal_uint8)sel << VRF_ICAL_EN_SHIFT);	
    mt6326_write_byte(0x1B, pmic6326_reg[0x1B]);
}

void pmic_vrf_oc_auto_off(kal_bool auto_off){
    pmic6326_reg[0x1B] &= ~(VRF_OC_AUTO_OFF_MASK << VRF_OC_AUTO_OFF_SHIFT);
    pmic6326_reg[0x1B] |= ((kal_uint8)auto_off << VRF_OC_AUTO_OFF_SHIFT);
    mt6326_write_byte(0x1B, pmic6326_reg[0x1B]);
}

void pmic_vrf_enable(kal_bool enable){
    pmic6326_reg[0x1B] &= ~(VRF_EN_MASK << VRF_EN_SHIFT);
    pmic6326_reg[0x1B] |= ((kal_uint8)enable << VRF_EN_SHIFT);	
    mt6326_write_byte(0x1B, pmic6326_reg[0x1B]);
}

void pmic_vrf_sel(kal_uint8 val){
}

void pmic_vrf_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x1B] &= ~(VRF_CAL_MASK << VRF_CAL_SHIFT);
    pmic6326_reg[0x1B] |= (val << VRF_CAL_SHIFT);
    mt6326_write_byte(0x1B, pmic6326_reg[0x1B]);
}

/* (0x1C) LDO CTRL 3 VRF */
void pmic_vrf_calst(vrf_calst_enum sel){
    pmic6326_reg[0x1C] &= ~(VRF_CALST_MASK << VRF_CALST_SHIFT);
    pmic6326_reg[0x1C] |= ((kal_uint8)sel << VRF_CALST_SHIFT);
    mt6326_write_byte(0x1C, pmic6326_reg[0x1C]);
}

void pmic_vrf_caloc(vrf_caloc_enum sel){
    pmic6326_reg[0x1C] &= ~(VRF_CALOC_MASK << VRF_CALOC_SHIFT);
    pmic6326_reg[0x1C] |= ((kal_uint8)sel << VRF_CALOC_SHIFT);
    mt6326_write_byte(0x1C, pmic6326_reg[0x1C]);
}

void pmic_vrf_on_sel(vrf_on_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x1C] &= ~(VRF_ON_SEL_MASK << VRF_ON_SEL_SHIFT);
    pmic6326_reg[0x1C] |= (val << VRF_ON_SEL_SHIFT);
    mt6326_write_byte(0x1C, pmic6326_reg[0x1C]);
}

void pmic_vrf_en_force(kal_bool enable){
    pmic6326_reg[0x1C] &= ~(VRF_EN_FORCE_MASK << VRF_EN_FORCE_SHIFT);
    pmic6326_reg[0x1C] |= ((kal_uint8)enable << VRF_EN_FORCE_SHIFT);
    mt6326_write_byte(0x1C, pmic6326_reg[0x1C]);
}

void pmic_vrf_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x1C] &= ~(VRF_PLNMOS_DIS_MASK << VRF_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x1C] |= ((kal_uint8)disable << VRF_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x1C, pmic6326_reg[0x1C]);
}

void pmic_vrf_cm(vrf_cm_enum sel){
    pmic6326_reg[0x1C] &= ~(VRF_CM_MASK << VRF_CM_SHIFT);
    pmic6326_reg[0x1C] |= ((kal_uint8)sel << VRF_CM_SHIFT);	
    mt6326_write_byte(0x1C, pmic6326_reg[0x1C]);
}

/* (0x1E) LDO CTRL 5 VTCXO */
void pmic_vtcxo_ical_en(vtcxo_ical_en_enum sel){
    pmic6326_reg[0x1E] &= ~(VTCXO_ICAL_EN_MASK << VTCXO_ICAL_EN_SHIFT);
    pmic6326_reg[0x1E] |= ((kal_uint8)sel << VTCXO_ICAL_EN_SHIFT);
    mt6326_write_byte(0x1E, pmic6326_reg[0x1E]);
}

void pmic_vtcxo_oc_auto_off(kal_bool auto_off){
    pmic6326_reg[0x1E] &= ~(VTCXO_OC_AUTO_OFF_MASK << VTCXO_OC_AUTO_OFF_SHIFT);
    pmic6326_reg[0x1E] |= ((kal_uint8)auto_off << VTCXO_OC_AUTO_OFF_SHIFT);
    mt6326_write_byte(0x1E, pmic6326_reg[0x1E]);
}

void pmic_vtcxo_enable(kal_bool enable){
    pmic6326_reg[0x1E] &= ~(VTCXO_EN_MASK << VTCXO_EN_SHIFT);
    pmic6326_reg[0x1E] |= (enable << VTCXO_EN_SHIFT);
    mt6326_write_byte(0x1E, pmic6326_reg[0x1E]);
}

void pmic_vtcxo_sel(kal_uint8 val){
}

void pmic_vtcxo_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x1E] &= ~(VTCXO_CAL_MASK << VTCXO_CAL_SHIFT);
    pmic6326_reg[0x1E] |= (val << VTCXO_CAL_SHIFT);
    mt6326_write_byte(0x1E, pmic6326_reg[0x1E]);
}

/* (0x1F) LDO CTRL 6 VTCXO */
void pmic_vtcxo_calst(vtcxo_calst_enum sel){
    pmic6326_reg[0x1F] &= ~(VTCXO_CALST_MASK << VTCXO_CALST_SHIFT);
    pmic6326_reg[0x1F] |= ((kal_uint8)sel << VTCXO_CALST_SHIFT);
    mt6326_write_byte(0x1F, pmic6326_reg[0x1F]);
}

void pmic_vtcxo_caloc(vtcxo_caloc_enum sel){
    pmic6326_reg[0x1F] &= ~(VTCXO_CALOC_MASK << VTCXO_CALOC_SHIFT);
    pmic6326_reg[0x1F] |= ((kal_uint8)sel << VTCXO_CALOC_SHIFT);
    mt6326_write_byte(0x1F, pmic6326_reg[0x1F]);
}

void pmic_vtcxo_on_sel(vtcxo_on_sel_enum sel){
    kal_uint8 val;	
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x1F] &= ~(VTCXO_ON_SEL_MASK << VTCXO_ON_SEL_SHIFT);
    pmic6326_reg[0x1F] |= (val << VTCXO_ON_SEL_SHIFT);
    mt6326_write_byte(0x1F, pmic6326_reg[0x1F]);
}

void pmic_vtcxo_en_force(kal_bool enable){
    pmic6326_reg[0x1F] &= ~(VTCXO_EN_FORCE_MASK << VTCXO_EN_FORCE_SHIFT);
    pmic6326_reg[0x1F] |= ((kal_uint8)enable << VTCXO_EN_FORCE_SHIFT);
    mt6326_write_byte(0x1F, pmic6326_reg[0x1F]);
}

void pmic_vtcxo_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x1F] &= ~(VTCXO_PLNMOS_DIS_MASK << VTCXO_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x1F] |= ((kal_uint8)disable << VTCXO_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x1F, pmic6326_reg[0x1F]);
}

void pmic_vtcxo_cm(vtcxo_cm_enum sel){
    pmic6326_reg[0x1F] &= ~(VTCXO_CM_MASK << VTCXO_CM_SHIFT);
    pmic6326_reg[0x1F] |= ((kal_uint8)sel << VTCXO_CM_SHIFT);
    mt6326_write_byte(0x1F, pmic6326_reg[0x1F]);
}

/* (0x21) LDO CTRL 8 V3GTX */
void pmic_v3gtx_sel(v3gtx_vol vol){
    kal_uint8 val;
    val = (kal_uint8) vol;
    ASSERT(val<=3);
    pmic6326_reg[0x21] &= ~(V3GTX_SEL_MASK << V3GTX_SEL_SHIFT);
    pmic6326_reg[0x21] |= (val << V3GTX_SEL_SHIFT);
    mt6326_write_byte(0x21, pmic6326_reg[0x21]);
}

void pmic_v3gtx_ical_en(v3gtx_ical_en_enum sel){
    pmic6326_reg[0x21] &= ~(V3GTX_ICAL_EN_MASK << V3GTX_ICAL_EN_SHIFT);
    pmic6326_reg[0x21] |= ((kal_uint8)sel << V3GTX_ICAL_EN_SHIFT);
    mt6326_write_byte(0x21, pmic6326_reg[0x21]);
}

void pmic_v3gtx_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x21] &= ~(V3GTX_CAL_MASK << V3GTX_CAL_SHIFT);
    pmic6326_reg[0x21] |= (val << V3GTX_CAL_SHIFT);
    mt6326_write_byte(0x21, pmic6326_reg[0x21]);            
}

/* (0x22) LDO CTRL 9 V3GTX */
void pmic_v3gtx_calst(v3gtx_calst_enum sel){
    pmic6326_reg[0x22] &= ~(V3GTX_CALST_MASK << V3GTX_CALST_SHIFT);
    pmic6326_reg[0x22] |= ((kal_uint8)sel << V3GTX_CALST_SHIFT);
    mt6326_write_byte(0x22, pmic6326_reg[0x22]);
}

void pmic_v3gtx_caloc(v3gtx_caloc_enum sel){
    pmic6326_reg[0x22] &= ~(V3GTX_CALOC_MASK << V3GTX_CALOC_SHIFT);
    pmic6326_reg[0x22] |= ((kal_uint8)sel << V3GTX_CALOC_SHIFT);
    mt6326_write_byte(0x22, pmic6326_reg[0x22]);
}

void pmic_v3gtx_oc_auto_off(kal_bool auto_off){
    pmic6326_reg[0x22] &= ~(V3GTX_OC_AUTO_OFF_MASK << V3GTX_OC_AUTO_OFF_SHIFT);
    pmic6326_reg[0x22] |= ((kal_uint8)auto_off << V3GTX_OC_AUTO_OFF_SHIFT);
    mt6326_write_byte(0x22, pmic6326_reg[0x22]);
}

void pmic_v3gtx_enable(kal_bool enable){
    pmic6326_reg[0x22] &= ~(V3GTX_EN_MASK << V3GTX_EN_SHIFT);
    pmic6326_reg[0x22] |= (enable << V3GTX_EN_SHIFT);
    mt6326_write_byte(0x22, pmic6326_reg[0x22]);
}

void pmic_v3gtx_on_sel(v3gtx_on_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x22] &= ~(V3GTX_ON_SEL_MASK << V3GTX_ON_SEL_SHIFT);
    pmic6326_reg[0x22] |= (val << V3GTX_ON_SEL_SHIFT);
    mt6326_write_byte(0x22, pmic6326_reg[0x22]);
}

void pmic_v3gtx_en_force(kal_bool enable){
    pmic6326_reg[0x22] &= ~(V3GTX_EN_FORCE_MASK << V3GTX_EN_FORCE_SHIFT);
    pmic6326_reg[0x22] |= ((kal_uint8)enable << V3GTX_EN_FORCE_SHIFT);
    mt6326_write_byte(0x22, pmic6326_reg[0x22]);
}

/* (0x24) LDO CTRL 11 V3GRX */
void pmic_v3grx_sel(v3grx_vol vol){
    kal_uint8 val;
    val = (kal_uint8) vol;
    ASSERT(val<=3);

    // TODO, val may OR the read back value (Or backup value) of the register
    pmic6326_reg[0x24] &= ~(V3GRX_SEL_MASK << V3GRX_SEL_SHIFT);
    pmic6326_reg[0x24] |= (val << V3GRX_SEL_SHIFT);
    mt6326_write_byte(0x24, pmic6326_reg[0x24]);
}

void pmic_3grx_ical_en(v3grx_ical_en_enum sel){
    pmic6326_reg[0x24] &= ~(V3GRX_ICAL_EN_MASK << V3GRX_ICAL_EN_SHIFT);
    pmic6326_reg[0x24] |= ((kal_uint8)sel << V3GRX_ICAL_EN_SHIFT);
    mt6326_write_byte(0x24, pmic6326_reg[0x24]);
}

void pmic_v3grx_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x24] &= ~(V3GRX_CAL_MASK << V3GRX_CAL_SHIFT);
    pmic6326_reg[0x24] |= (val << V3GRX_CAL_SHIFT);
    mt6326_write_byte(0x24, pmic6326_reg[0x24]);
}

/* (0x25) LDO CTRL 12 V3GRX */
void pmic_v3grx_calst(v3grx_calst_enum sel){
    pmic6326_reg[0x25] &= ~(V3GRX_CALST_MASK << V3GRX_CALST_SHIFT);
    pmic6326_reg[0x25] |= ((kal_uint8)sel << V3GRX_CALST_SHIFT);
    mt6326_write_byte(0x25, pmic6326_reg[0x25]);
}

void pmic_v3grx_caloc(v3grx_caloc_enum sel){
    pmic6326_reg[0x25] &= ~(V3GRX_CALOC_MASK << V3GRX_CALOC_SHIFT);
    pmic6326_reg[0x25] |= ((kal_uint8)sel << V3GRX_CALOC_SHIFT);
    mt6326_write_byte(0x25, pmic6326_reg[0x25]);
}

void pmic_v3grx_oc_auto_off(kal_bool auto_off){
    pmic6326_reg[0x25] &= ~(V3GRX_OC_AUTO_OFF_MASK << V3GRX_OC_AUTO_OFF_SHIFT);
    pmic6326_reg[0x25] |= ((kal_uint8)auto_off << V3GRX_OC_AUTO_OFF_SHIFT);
    mt6326_write_byte(0x25, pmic6326_reg[0x25]);
}

void pmic_v3grx_enable(kal_bool enable){
    pmic6326_reg[0x25] &= ~(V3GRX_EN_MASK << V3GRX_EN_SHIFT);
    pmic6326_reg[0x25] |= (enable << V3GRX_EN_SHIFT);
    mt6326_write_byte(0x25, pmic6326_reg[0x25]);
}

void pmic_v3grx_on_sel(v3grx_on_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x25] &= ~(V3GRX_ON_SEL_MASK << V3GRX_ON_SEL_SHIFT);
    pmic6326_reg[0x25] |= (val << V3GRX_ON_SEL_SHIFT);
    mt6326_write_byte(0x25, pmic6326_reg[0x25]);
}

void pmic_v3grx_en_force(kal_bool enable){
    pmic6326_reg[0x25] &= ~(V3GRX_EN_FORCE_MASK << V3GRX_EN_FORCE_SHIFT);
    pmic6326_reg[0x25] |= ((kal_uint8)enable << V3GRX_EN_FORCE_SHIFT);
    mt6326_write_byte(0x25, pmic6326_reg[0x25]);
}

/* (0x2E) LDO CTRL 21 VCAMA */
void pmic_vcama_sel(vcama_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 3);
    pmic6326_reg[0x2E] &= ~(VCAMA_SEL_MASK << VCAMA_SEL_SHIFT);
    pmic6326_reg[0x2E] |= (val << VCAMA_SEL_SHIFT);
    mt6326_write_byte(0x2E, pmic6326_reg[0x2E]);
}

void pmic_vcama_ical_en(vcama_ical_en_enum sel){
    pmic6326_reg[0x2E] &= ~(VCAMA_ICAL_EN_MASK << VCAMA_ICAL_EN_SHIFT);
    pmic6326_reg[0x2E] |= (sel << VCAMA_ICAL_EN_SHIFT);
    mt6326_write_byte(0x2E, pmic6326_reg[0x2E]);
}

void pmic_vcama_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x2E] &= ~(VCAMA_CAL_MASK << VCAMA_CAL_SHIFT);
    pmic6326_reg[0x2E] |= (val << VCAMA_CAL_SHIFT);
    mt6326_write_byte(0x2E, pmic6326_reg[0x2E]);
}

/* (0x2F) LDO CTRL 22 VCAMA */
void pmic_vcama_calst(vcama_calst_enum sel){
    pmic6326_reg[0x2F] &= ~(VCAMA_CALST_MASK << VCAMA_CALST_SHIFT);
    pmic6326_reg[0x2F] |= ((kal_uint8)sel << VCAMA_CALST_SHIFT);
    mt6326_write_byte(0x2F, pmic6326_reg[0x2F]);
}

void pmic_vcama_caloc(vcama_caloc_enum sel){
    pmic6326_reg[0x2F] &= ~(VCAMA_CALOC_MASK << VCAMA_CALOC_SHIFT);
    pmic6326_reg[0x2F] |= ((kal_uint8)sel << VCAMA_CALOC_SHIFT);
    mt6326_write_byte(0x2F, pmic6326_reg[0x2F]);
}

void pmic_vcama_enable(kal_bool enable){
    pmic6326_reg[0x2F] &= ~(VCAMA_EN_MASK << VCAMA_EN_SHIFT);
    pmic6326_reg[0x2F] |= ((kal_uint8)enable << VCAMA_EN_SHIFT);
    mt6326_write_byte(0x2F, pmic6326_reg[0x2F]);
}

void pmic_vcama_en_force(kal_bool enable){
    pmic6326_reg[0x2F] &= ~(VCAMA_EN_FORCE_MASK << VCAMA_EN_FORCE_SHIFT);
    pmic6326_reg[0x2F] |= ((kal_uint8)enable << VCAMA_EN_FORCE_SHIFT);
    mt6326_write_byte(0x2F, pmic6326_reg[0x2F]);
}

void pmic_vcama_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x2F] &= ~(VCAMA_PLNMOS_DIS_MASK << VCAMA_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x2F] |= ((kal_uint8)disable << VCAMA_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x2F, pmic6326_reg[0x2F]);   
}

void pmic_vcama_cm(vcama_cm_enum sel){
    pmic6326_reg[0x2F] &= ~(VCAMA_CM_MASK << VCAMA_CM_SHIFT);
    pmic6326_reg[0x2F] |= ((kal_uint8)sel << VCAMA_CM_SHIFT);
    mt6326_write_byte(0x2F, pmic6326_reg[0x2F]);
}

/* (0x31) LDO CTRL 24 VWIFI3V3 */
void pmic_vwifi3v3_sel(vwifi3v3_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 3);
    pmic6326_reg[0x31] &= ~(VWIFI3V3_SEL_MASK << VWIFI3V3_SEL_SHIFT);
    pmic6326_reg[0x31] |= (val << VWIFI3V3_SEL_SHIFT);
    mt6326_write_byte(0x31, pmic6326_reg[0x31]);
}

void pmic_vwifi3v3_ical_en(vwifi3v3_ical_en_enum sel){
    pmic6326_reg[0x31] &= ~(VWIFI3V3_ICAL_EN_MASK << VWIFI3V3_ICAL_EN_SHIFT);
    pmic6326_reg[0x31] |= (sel << VWIFI3V3_ICAL_EN_SHIFT);
    mt6326_write_byte(0x31, pmic6326_reg[0x31]);
}

void pmic_vwifi3v3_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x31] &= ~(VWIFI3V3_CAL_MASK << VWIFI3V3_CAL_SHIFT);
    pmic6326_reg[0x31] |= (val << VWIFI3V3_CAL_SHIFT);
    mt6326_write_byte(0x31, pmic6326_reg[0x31]);
}

/* (0x32) LDO CTRL 25 VWIFI3V3 */
void pmic_vwifi3v3_calst(vwifi3v3_calst_enum sel){
    pmic6326_reg[0x32] &= ~(VWIFI3V3_CALST_MASK << VWIFI3V3_CALST_SHIFT);
    pmic6326_reg[0x32] |= ((kal_uint8)sel << VWIFI3V3_CALST_SHIFT);
    mt6326_write_byte(0x32, pmic6326_reg[0x32]);
}

void pmic_vwifi3v3_caloc(vwifi3v3_caloc_enum sel){
    pmic6326_reg[0x32] &= ~(VWIFI3V3_CALOC_MASK << VWIFI3V3_CALOC_SHIFT);
    pmic6326_reg[0x32] |= ((kal_uint8)sel << VWIFI3V3_CALOC_SHIFT);
    mt6326_write_byte(0x32, pmic6326_reg[0x32]);
}

void pmic_vwifi3v3_enable(kal_bool enable){
    pmic6326_reg[0x32] &= ~(VWIFI3V3_EN_MASK << VWIFI3V3_EN_SHIFT);
    pmic6326_reg[0x32] |= ((kal_uint8)enable << VWIFI3V3_EN_SHIFT);
    mt6326_write_byte(0x32, pmic6326_reg[0x32]);
}

void pmic_vwifi3v3_en_force(kal_bool enable){
    pmic6326_reg[0x32] &= ~(VWIFI3V3_EN_FORCE_MASK << VWIFI3V3_EN_FORCE_SHIFT);
    pmic6326_reg[0x32] |= ((kal_uint8)enable << VWIFI3V3_EN_FORCE_SHIFT);
    mt6326_write_byte(0x32, pmic6326_reg[0x32]);
}

void pmic_vwifi3v3_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x32] &= ~(VWIFI3V3_PLNMOS_DIS_MASK << VWIFI3V3_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x32] |= ((kal_uint8)disable << VWIFI3V3_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x32, pmic6326_reg[0x32]);
}

void pmic_vwifi3v3_cm(vwifi3v3_cm_enum sel){
    pmic6326_reg[0x32] &= ~(VWIFI3V3_CM_MASK << VWIFI3V3_CM_SHIFT);
    pmic6326_reg[0x32] |= ((kal_uint8)sel << VWIFI3V3_CM_SHIFT);
    mt6326_write_byte(0x32, pmic6326_reg[0x32]);
}

/* (0x34) LDO CTRL 27 VWIFI2V8 */
void pmic_vwifi2v8_sel(vwifi2v8_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 3);
    pmic6326_reg[0x34] &= ~(VWIFI2V8_SEL_MASK << VWIFI2V8_SEL_SHIFT);
    pmic6326_reg[0x34] |= (val << VWIFI2V8_SEL_SHIFT);
    mt6326_write_byte(0x34, pmic6326_reg[0x34]);
}

void pmic_vwifi2v8_ical_en(vwifi2v8_ical_en_enum sel){
    pmic6326_reg[0x34] &= ~(VWIFI2V8_ICAL_EN_MASK << VWIFI2V8_ICAL_EN_SHIFT);
    pmic6326_reg[0x34] |= (sel << VWIFI2V8_ICAL_EN_SHIFT);
    mt6326_write_byte(0x34, pmic6326_reg[0x34]);
}

void pmic_vwifi2v8_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x34] &= ~(VWIFI2V8_CAL_MASK << VWIFI2V8_CAL_SHIFT);
    pmic6326_reg[0x34] |= (val << VWIFI2V8_CAL_SHIFT);
    mt6326_write_byte(0x34, pmic6326_reg[0x34]);
}

/* (0x35) LDO CTRL 28 VWIFI2V8 */
void pmic_vwifi2v8_calst(vwifi2v8_calst_enum sel){
    pmic6326_reg[0x35] &= ~(VWIFI2V8_CALST_MASK << VWIFI2V8_CALST_SHIFT);
    pmic6326_reg[0x35] |= ((kal_uint8)sel << VWIFI2V8_CALST_SHIFT);
    mt6326_write_byte(0x35, pmic6326_reg[0x35]);
}

void pmic_vwifi2v8_caloc(vwifi2v8_caloc_enum sel){
    pmic6326_reg[0x35] &= ~(VWIFI2V8_CALOC_MASK << VWIFI2V8_CALOC_SHIFT);
    pmic6326_reg[0x35] |= ((kal_uint8)sel << VWIFI2V8_CALOC_SHIFT);
    mt6326_write_byte(0x35, pmic6326_reg[0x35]);
}

void pmic_vwifi2v8_enable(kal_bool enable){
    pmic6326_reg[0x35] &= ~(VWIFI2V8_EN_MASK << VWIFI2V8_EN_SHIFT);
    pmic6326_reg[0x35] |= ((kal_uint8)enable << VWIFI2V8_EN_SHIFT);
    mt6326_write_byte(0x35, pmic6326_reg[0x35]);
}

void pmic_vwifi2v8_en_force(kal_bool enable){
    pmic6326_reg[0x35] &= ~(VWIFI2V8_EN_FORCE_MASK << VWIFI2V8_EN_FORCE_SHIFT);
    pmic6326_reg[0x35] |= ((kal_uint8)enable << VWIFI2V8_EN_FORCE_SHIFT);
    mt6326_write_byte(0x35, pmic6326_reg[0x35]);
}

void pmic_vwifi2v8_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x35] &= ~(VWIFI2V8_PLNMOS_DIS_MASK << VWIFI2V8_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x35] |= ((kal_uint8)disable << VWIFI2V8_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x35, pmic6326_reg[0x35]);
}

void pmic_vwifi2v8_cm(vwifi2v8_cm_enum sel){
    pmic6326_reg[0x35] &= ~(VWIFI2V8_CM_MASK << VWIFI2V8_CM_SHIFT);
    pmic6326_reg[0x35] |= ((kal_uint8)sel << VWIFI2V8_CM_SHIFT);
    mt6326_write_byte(0x35, pmic6326_reg[0x35]);
}

/* (0x37) LDO CTRL 30 VSIM */
void pmic_vsim_sel(vsim_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 7);
    pmic6326_reg[0x37] &= ~(VSIM_SEL_MASK << VSIM_SEL_SHIFT);
    pmic6326_reg[0x37] |= (val << VSIM_SEL_SHIFT);
    mt6326_write_byte(0x37, pmic6326_reg[0x37]);
}

void pmic_vsim_enable(kal_bool enable){
    pmic6326_reg[0x37] &= ~(VSIM_EN_MASK << VSIM_EN_SHIFT);
    pmic6326_reg[0x37] |= (enable << VSIM_EN_SHIFT);
    mt6326_write_byte(0x37, pmic6326_reg[0x37]);
}

void pmic_vsim_ical_en(vsim_ical_en_enum sel){
    pmic6326_reg[0x37] &= ~(VSIM_ICAL_EN_MASK << VSIM_ICAL_EN_SHIFT);
    pmic6326_reg[0x37] |= (sel << VSIM_ICAL_EN_SHIFT);
    mt6326_write_byte(0x37, pmic6326_reg[0x37]);
}

void pmic_vsim_en_force(kal_bool enable){
    pmic6326_reg[0x37] &= ~(VSIM_EN_FORCE_MASK << VSIM_EN_FORCE_SHIFT);
    pmic6326_reg[0x37] |= ((kal_uint8)enable << VSIM_EN_FORCE_SHIFT);
    mt6326_write_byte(0x37, pmic6326_reg[0x37]);
}

void pmic_vsim_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x37] &= ~(VSIM_PLNMOS_DIS_MASK << VSIM_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x37] |= ((kal_uint8)disable << VSIM_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x37, pmic6326_reg[0x37]);
}

/* (0x38) LDO CTRL 31 VSIM */
void pmic_vsim_cal(kal_uint8 val){
    ASSERT(val <= 15);
    pmic6326_reg[0x38] &= ~(VSIM_CAL_MASK << VSIM_CAL_SHIFT);
    pmic6326_reg[0x38] |= ((kal_uint8)val << VSIM_CAL_SHIFT);
    mt6326_write_byte(0x38, pmic6326_reg[0x38]);
}

/* (0x3A) LDO CTRL 33 VUSB                  */
/* USB voltage is NOT opened for change */
void pmic_vusb_enable(kal_bool enable){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3D;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3A;
    }
	
#if 0	
    if (enable){
        pmic6326_vboost1_set_flag |= VBOOST1_SET_FLAG_OTG;
    }else{
        pmic6326_vboost1_set_flag &= ~(VBOOST1_SET_FLAG_OTG);
    }
    pmic_boost1_enable_internal(pmic6326_vboost1_set_flag);
#endif	

	//printk("[pmic_vusb_enable] only turn on/off VUSB\r\n");

    pmic6326_reg[reg_addr] &= ~(VUSB_EN_MASK << VUSB_EN_SHIFT);
    pmic6326_reg[reg_addr] |= (enable << VUSB_EN_SHIFT);
	
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vusb_ical_en(vusb_ical_en_enum sel){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3D;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3A;
    }
    pmic6326_reg[reg_addr] &= ~(VUSB_ICAL_EN_MASK << VUSB_ICAL_EN_SHIFT);
    pmic6326_reg[reg_addr] |= (sel << VUSB_ICAL_EN_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vusb_en_force(kal_bool enable){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3D;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3A;
    }
    pmic6326_reg[reg_addr] &= ~(VUSB_EN_FORCE_MASK << VUSB_EN_FORCE_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)enable << VUSB_EN_FORCE_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vusb_plnmos_dis(kal_bool disable){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3D;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3A;
    }
    pmic6326_reg[reg_addr] &= ~(VUSB_PLNMOS_DIS_MASK << VUSB_PLNMOS_DIS_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)disable << VUSB_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vusb_sel(kal_uint8 val){
}

/* (0x3B) LDO CTRL 34 VUSB */
void pmic_vusb_cal(kal_uint8 val){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3E;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3B;
    }
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[reg_addr] &= ~(VUSB_CAL_MASK << VUSB_CAL_SHIFT);
    pmic6326_reg[reg_addr] |= (val << VUSB_CAL_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vusb_calst(vusb_calst_enum sel){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3E;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3B;
    }
    pmic6326_reg[reg_addr] &= ~(VUSB_CALST_MASK << VUSB_CALST_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)sel << VUSB_CALST_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vusb_caloc(vusb_caloc_enum sel){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3E;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3B;
    }
    pmic6326_reg[reg_addr] &= ~(VUSB_CALOC_MASK << VUSB_CALOC_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)sel << VUSB_CALOC_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

/* (0x3D) LDO CTRL 36 VBT */
void pmic_vbt_sel(vbt_sel_enum sel){
    kal_uint8 val;
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)    
    {
        // E3, E4 version
        reg_addr = 0x3A;
        // For E3, the voltage changed, we need to re-map VBT voltage selction value of E1, E2 to E3
        if (sel == VBT_1_3) {val = (kal_uint8)VBT_E3_1_3;}
        else if	(sel == VBT_1_5) {val = (kal_uint8)VBT_E3_1_5;}
        else if	(sel == VBT_1_8) {val = (kal_uint8)VBT_E3_1_8;}
        else if	(sel == VBT_2_5) {val = (kal_uint8)VBT_E3_2_5;}
        else if	(sel == VBT_2_8) {val = (kal_uint8)VBT_E3_2_8;}
        else if	(sel == VBT_3_0) {val = (kal_uint8)VBT_E3_3_0;}
        else if (sel == VBT_3_3) {val = (kal_uint8)VBT_E3_3_3;}
        else
        {
            ASSERT(0);	// E3 VBT does NOT support 1.2V
        }
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3D;
        val = (kal_uint8)sel;
    }
    ASSERT(val <= 7);
    pmic6326_reg[reg_addr] &= ~(VBT_SEL_MASK << VBT_SEL_SHIFT);
    pmic6326_reg[reg_addr] |= (val << VBT_SEL_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vbt_enable(kal_bool enable){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3A;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3D;
    }
    pmic6326_reg[reg_addr] &= ~(VBT_EN_MASK << VBT_EN_SHIFT);
    pmic6326_reg[reg_addr] |= (enable << VBT_EN_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vbt_ical_en(vbt_ical_en_enum sel){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3A;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3D;
    }
    pmic6326_reg[0x3D] &= ~(VBT_ICAL_EN_MASK << VBT_ICAL_EN_SHIFT);
    pmic6326_reg[0x3D] |= (sel << VBT_ICAL_EN_SHIFT);
    mt6326_write_byte(0x3D, pmic6326_reg[0x3D]);
}

void pmic_vbt_en_force(kal_bool enable){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3A;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3D;
    }	
    pmic6326_reg[reg_addr] &= ~(VBT_EN_FORCE_MASK << VBT_EN_FORCE_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)enable << VBT_EN_FORCE_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vbt_plnmos_dis(kal_bool disable){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3A;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3D;
    }
    pmic6326_reg[reg_addr] &= ~(VBT_PLNMOS_DIS_MASK << VBT_PLNMOS_DIS_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)disable << VBT_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

/* (0x3E) LDO CTRL 37 VBT */
void pmic_vbt_cal(kal_uint8 val){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3B;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3E;
    }
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[reg_addr] &= ~(VBT_CAL_MASK << VBT_CAL_SHIFT);
    pmic6326_reg[reg_addr] |= (val << VBT_CAL_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vbt_calst(vbt_calst_enum sel){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3B;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3E;
    }
    pmic6326_reg[reg_addr] &= ~(VBT_CALST_MASK << VBT_CALST_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)sel << VBT_CALST_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

void pmic_vbt_caloc(vbt_caloc_enum sel){
    kal_uint16 reg_addr;
    //if (pmic6326_eco_version == PMIC6326_ECO_3_VERSION)
    if (pmic6326_eco_version >= PMIC6326_ECO_3_VERSION)
    {
        // E3, E4 version
        reg_addr = 0x3B;
    }
    else
    {
        // E1, E2 version
        reg_addr = 0x3E;
    }
    pmic6326_reg[reg_addr] &= ~(VBT_CALOC_MASK << VBT_CALOC_SHIFT);
    pmic6326_reg[reg_addr] |= ((kal_uint8)sel << VBT_CALOC_SHIFT);
    mt6326_write_byte(reg_addr, pmic6326_reg[reg_addr]);
}

/* (0x40) LDO CTRL 39 VCAMD */
void pmic_vcamd_sel(vcamd_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 7);
    pmic6326_reg[0x40] &= ~(VCAMD_SEL_MASK << VCAMD_SEL_SHIFT);
    pmic6326_reg[0x40] |= (val << VCAMD_SEL_SHIFT);
    mt6326_write_byte(0x40, pmic6326_reg[0x40]);
}

void pmic_vcamd_enable(kal_bool enable){
    pmic6326_reg[0x40] &= ~(VCAMD_EN_MASK << VCAMD_EN_SHIFT);
    pmic6326_reg[0x40] |= (enable << VCAMD_EN_SHIFT);
    mt6326_write_byte(0x40, pmic6326_reg[0x40]);
}

void pmic_vcamd_ical_en(vcamd_ical_en_enum sel){
    pmic6326_reg[0x40] &= ~(VCAMD_ICAL_EN_MASK << VCAMD_ICAL_EN_SHIFT);
    pmic6326_reg[0x40] |= (sel << VCAMD_ICAL_EN_SHIFT);
    mt6326_write_byte(0x40, pmic6326_reg[0x40]);
}

void pmic_vcamd_en_force(kal_bool enable){
    pmic6326_reg[0x40] &= ~(VCAMD_EN_FORCE_MASK << VCAMD_EN_FORCE_SHIFT);
    pmic6326_reg[0x40] |= ((kal_uint8)enable << VCAMD_EN_FORCE_SHIFT);
    mt6326_write_byte(0x40, pmic6326_reg[0x40]);
}

void pmic_vcamd_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x40] &= ~(VCAMD_PLNMOS_DIS_MASK << VCAMD_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x40] |= ((kal_uint8)disable << VCAMD_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x40, pmic6326_reg[0x40]);
}

/* (0x41) LDO CTRL 40 VCAMD */
void pmic_vcamd_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x41] &= ~(VCAMD_CAL_MASK << VCAMD_CAL_SHIFT);
    pmic6326_reg[0x41] |= (val << VCAMD_CAL_SHIFT);
    mt6326_write_byte(0x41, pmic6326_reg[0x41]);
}

void pmic_vcamd_calst(vcamd_calst_enum sel){
    pmic6326_reg[0x41] &= ~(VCAMD_CALST_MASK << VCAMD_CALST_SHIFT);
    pmic6326_reg[0x41] |= ((kal_uint8)sel << VCAMD_CALST_SHIFT);
    mt6326_write_byte(0x41, pmic6326_reg[0x41]);
}

void pmic_vcamd_caloc(vcamd_caloc_enum sel){
    pmic6326_reg[0x41] &= ~(VCAMD_CALOC_MASK << VCAMD_CALOC_SHIFT);
    pmic6326_reg[0x41] |= ((kal_uint8)sel << VCAMD_CALOC_SHIFT);
    mt6326_write_byte(0x41, pmic6326_reg[0x41]);
}

/* (0x43) LDO CTRL 42 VGP */
void pmic_vgp_sel(vgp_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8) sel;
    pmic6326_reg[0x43] &= ~(VGP_SEL_MASK << VGP_SEL_SHIFT);
    pmic6326_reg[0x43] |= (val << VGP_SEL_SHIFT);
    mt6326_write_byte(0x43, pmic6326_reg[0x43]);
}

void pmic_vgp_enable(kal_bool enable){
    pmic6326_reg[0x43] &= ~(VGP_EN_MASK << VGP_EN_SHIFT);
    pmic6326_reg[0x43] |= (enable << VGP_EN_SHIFT);
    mt6326_write_byte(0x43, pmic6326_reg[0x43]);
}

/* (0x44) LDO CTRL 43 VGP */
void pmic_vgp_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x44] &= ~(VGP_CAL_MASK << VGP_CAL_SHIFT);
    pmic6326_reg[0x44] |= (val << VGP_CAL_SHIFT);
    mt6326_write_byte(0x44, pmic6326_reg[0x44]);
}

/* (0x46) LDO CTRL 45 VSDIO */
void pmic_vsdio_ical_en(vsdio_ical_en_enum sel){
    pmic6326_reg[0x46] &= ~(VSDIO_ICAL_EN_MASK << VSDIO_ICAL_EN_SHIFT);
    pmic6326_reg[0x46] |= (sel << VSDIO_ICAL_EN_SHIFT);
    mt6326_write_byte(0x46, pmic6326_reg[0x46]);
}

void pmic_vsdio_enable(kal_bool enable){
    pmic6326_reg[0x46] &= ~(VSDIO_EN_MASK << VSDIO_EN_SHIFT);
    pmic6326_reg[0x46] |= (enable << VSDIO_EN_SHIFT);

    if (mt6326_write_byte(0x46, pmic6326_reg[0x46]) == 1) {
        kal_uint8 pmic6326_reg_7 = 0;
        kal_uint8 val = 0;
        /* The VSDIO is unstable sometimes. We make sure it is on/off
         * indeed before return.
         */
        do {
            if (mt6326_read_byte(7, &pmic6326_reg_7) == 1) {
                val = (pmic6326_reg_7>>5)&(0x01);
                if (((enable == KAL_TRUE) && (val == 1)) ||
                    ((enable == KAL_FALSE) && (val == 0)))
                    break;
            }
            printk("VSDIO : %d (%s)\n", val, enable == KAL_TRUE ? "enabled" : "disabled");
        } while(1);
    }
}

void pmic_vsdio_en_force(kal_bool enable){
    pmic6326_reg[0x46] &= ~(VSDIO_EN_FORCE_MASK << VSDIO_EN_FORCE_SHIFT);
    pmic6326_reg[0x46] |= ((kal_uint8)enable << VSDIO_EN_FORCE_SHIFT);
    mt6326_write_byte(0x46, pmic6326_reg[0x46]);
}

void pmic_vsdio_cal(kal_uint8 val){
    ASSERT(val<=15);		// val is 0000 ~ 1111
    pmic6326_reg[0x46] &= ~(VSDIO_CAL_MASK << VSDIO_CAL_SHIFT);
    pmic6326_reg[0x46] |= (val << VSDIO_CAL_SHIFT);
    mt6326_write_byte(0x46, pmic6326_reg[0x46]);
}

/* (0x47) LDO CTRL 46 VSDIO */
void pmic_vsdio_calst(vsdio_calst_enum sel){
    pmic6326_reg[0x47] &= ~(VSDIO_CALST_MASK << VSDIO_CALST_SHIFT);
    pmic6326_reg[0x47] |= ((kal_uint8)sel << VSDIO_CALST_SHIFT);
    mt6326_write_byte(0x47, pmic6326_reg[0x47]);
}

void pmic_vsdio_caloc(vsdio_caloc_enum sel){
    pmic6326_reg[0x47] &= ~(VSDIO_CALOC_MASK << VSDIO_CALOC_SHIFT);
    pmic6326_reg[0x47] |= ((kal_uint8)sel << VSDIO_CALOC_SHIFT);
    mt6326_write_byte(0x47, pmic6326_reg[0x47]);
}

void pmic_vsdio_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x47] &= ~(VSDIO_PLNMOS_DIS_MASK << VSDIO_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x47] |= ((kal_uint8)disable << VSDIO_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x47, pmic6326_reg[0x47]);
}

void pmic_vsdio_sel(vsdio_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8) sel;
    pmic6326_reg[0x47] &= ~(VSDIO_SEL_MASK << VSDIO_SEL_SHIFT);
    pmic6326_reg[0x47] |= (val << VSDIO_SEL_SHIFT);
    mt6326_write_byte(0x47, pmic6326_reg[0x47]);
}

void pmic_vsdio_cm(vsdio_cm_enum sel){
    pmic6326_reg[0x47] &= ~(VSDIO_CM_MASK << VSDIO_CM_SHIFT);
    pmic6326_reg[0x47] |= ((kal_uint8)sel << VSDIO_CM_SHIFT);
    mt6326_write_byte(0x47, pmic6326_reg[0x47]);
}

/* (0x48) LDO CTRL 47 VSDIO */
void pmic_vcore1_dvfs_step_inc(kal_uint8 val){
    ASSERT(val <= 0x1F);
    pmic6326_reg[0x48] &= ~(VCORE1_DVFS_STEP_INC_MASK << VCORE1_DVFS_STEP_INC_SHIFT);
    pmic6326_reg[0x48] |= ((kal_uint8)val << VCORE1_DVFS_STEP_INC_SHIFT);
    mt6326_write_byte(0x48, pmic6326_reg[0x48]);
}

/* (0x4E) BUCK CTRL 6 VCORE1 */
void pmic_vcore1_dvfs_0_eco3(kal_uint8 val){
    ASSERT(val <= 0xF);
    pmic6326_reg[0x4E] &= ~(VCORE1_DVFS_0_ECO3_MASK << VCORE1_DVFS_0_ECO3_SHIFT);
    pmic6326_reg[0x4E] |= ((kal_uint8)val << VCORE1_DVFS_0_ECO3_SHIFT);
    mt6326_write_byte(0x4E, pmic6326_reg[0x4E]);
}

/* (0x4F) BUCK CTRL 7 VCORE1 */
void pmic_vcore1_sleep_0_eco3(kal_uint8 val){
    ASSERT(val <= 0x1);
    pmic6326_reg[0x4F] &= ~(VCORE1_SLEEP_0_ECO3_MASK << VCORE1_SLEEP_0_ECO3_SHIFT);
    pmic6326_reg[0x4F] |= ((kal_uint8)val << VCORE1_SLEEP_0_ECO3_SHIFT);
    mt6326_write_byte(0x4F, pmic6326_reg[0x4F]);
}

void pmic_vcore1_dvfs_ramp_enable(kal_bool enable){
    pmic6326_reg[0x4F] &= ~(VCORE1_DVFS_RAMP_EN_MASK << VCORE1_DVFS_RAMP_EN_SHIFT);
    pmic6326_reg[0x4F] |= ((kal_uint8)enable << VCORE1_DVFS_RAMP_EN_SHIFT);
    mt6326_write_byte(0x4F, pmic6326_reg[0x4F]);
}

void pmic_vcore1_dvfs_target_update(kal_bool update){
    pmic6326_reg[0x4F] &= ~(VCORE1_DVFS_TARGET_UPDATE_MASK << VCORE1_DVFS_TARGET_UPDATE_SHIFT);
    pmic6326_reg[0x4F] |= ((kal_uint8)update << VCORE1_DVFS_TARGET_UPDATE_SHIFT);
    mt6326_write_byte(0x4F, pmic6326_reg[0x4F]);
}

/* (0x51) BUCK CTRL 9 VCORE2 */
void pmic_vcore2_dvfs_0_eco3(kal_uint8 val){
    ASSERT(val <=0xF);
    pmic6326_reg[0x51] &= ~(VCORE2_DVFS_0_ECO3_MASK << VCORE2_DVFS_0_ECO3_SHIFT);
    pmic6326_reg[0x51] |= ((kal_uint8)val << VCORE2_DVFS_0_ECO3_SHIFT);
    mt6326_write_byte(0x51, pmic6326_reg[0x51]);
}

// (0x52) BUCK CTRL 10 VCORE2
void pmic_vcore2_enable(kal_bool enable){
    pmic6326_reg[0x52] &= ~(VCORE2_EN_MASK << VCORE2_EN_SHIFT);
    pmic6326_reg[0x52] |= ((kal_uint8)enable << VCORE2_EN_SHIFT);
    mt6326_write_byte(0x52, pmic6326_reg[0x52]);
}

void pmic_vcore2_sleep_0_eco3(kal_uint8 val){
    pmic6326_reg[0x52] &= ~(VCORE2_SLEEP_0_ECO3_MASK << VCORE2_SLEEP_0_ECO3_SHIFT);
    pmic6326_reg[0x52] |= ((kal_uint8)val << VCORE2_SLEEP_0_ECO3_SHIFT);
    mt6326_write_byte(0x52, pmic6326_reg[0x52]);
}

/* (0x53) BUCK CTRL 11 VCORE2 */
void pmic_vcore2_on_sel(vcore2_on_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x53] &= ~(VCORE2_ON_SEL_MASK << VCORE2_ON_SEL_SHIFT);
    pmic6326_reg[0x53] |= ((kal_uint8)val << VCORE2_ON_SEL_SHIFT);
    mt6326_write_byte(0x53, pmic6326_reg[0x53]);
}

/* (0x54) BUCK CTRL 12 - VCORE 2 */
void pmic_vcore2_sleep_1_eco3(kal_uint8 val){
    ASSERT(val <= 1);
    pmic6326_reg[0x54] &= ~(VCORE2_SLEEP_1_ECO3_MASK << VCORE2_SLEEP_1_ECO3_SHIFT);
    pmic6326_reg[0x54] |= ((kal_uint8)val << VCORE2_SLEEP_1_ECO3_SHIFT);
    mt6326_write_byte(0x54, pmic6326_reg[0x54]);
}

void pmic_vcore2_dvfs_1_eco3(kal_uint8 val){
    ASSERT(val <= 15);
    pmic6326_reg[0x54] &= ~(VCORE2_DVFS_1_ECO3_MASK << VCORE2_DVFS_1_ECO3_SHIFT);
    pmic6326_reg[0x54] |= ((kal_uint8)val << VCORE2_DVFS_1_ECO3_SHIFT);
    mt6326_write_byte(0x54, pmic6326_reg[0x54]);    
}

void pmic_vcore2_dvfs_ramp_enable(kal_bool enable){
    pmic6326_reg[0x54] &= ~(VCORE2_DVFS_RAMP_EN_MASK << VCORE2_DVFS_RAMP_EN_SHIFT);
    pmic6326_reg[0x54] |= ((kal_uint8)enable << VCORE2_DVFS_RAMP_EN_SHIFT);
    mt6326_write_byte(0x54, pmic6326_reg[0x54]);
}

void pmic_vcore2_dvfs_target_update(kal_bool update){
    pmic6326_reg[0x54] &= ~(VCORE2_DVFS_TARGET_UPDATE_MASK << VCORE2_DVFS_TARGET_UPDATE_SHIFT);
    pmic6326_reg[0x54] |= ((kal_uint8)update << VCORE2_DVFS_TARGET_UPDATE_SHIFT);
    mt6326_write_byte(0x54, pmic6326_reg[0x54]);
}

/* (0x57) BUCK CTRL 15 VMEM */
void pmic_vcore1_sleep_1_eco3(kal_uint8 val){
    ASSERT(val <= 1);
    pmic6326_reg[0x57] &= ~(VCORE1_SLEEP_1_ECO3_MASK << VCORE1_SLEEP_1_ECO3_SHIFT);
    pmic6326_reg[0x57] |= ((kal_uint8)val << VCORE1_SLEEP_1_ECO3_SHIFT);
    mt6326_write_byte(0x57, pmic6326_reg[0x57]);
}

void pmic_vcore1_dvfs_1_eco3(kal_uint8 val){
    ASSERT(val <= 15);
    pmic6326_reg[0x57] &= ~(VCORE1_DVFS_1_ECO3_MASK << VCORE1_DVFS_1_ECO3_SHIFT);
    pmic6326_reg[0x57] |= ((kal_uint8)val << VCORE1_DVFS_1_ECO3_SHIFT);
    mt6326_write_byte(0x57, pmic6326_reg[0x57]);    
}

/* (0x58) BUCK CTRL 16 VPA */
void pmic_vpa_tuneh(kal_uint8 value){
    ASSERT(value <= 31);
    pmic6326_reg[0x58] &= ~(VPA_TUNEH_MASK << VPA_TUNEH_SHIFT);
    pmic6326_reg[0x58] |= (value << VPA_TUNEH_SHIFT);
    mt6326_write_byte(0x58, pmic6326_reg[0x58]);
}

void pmic_vpa_en_force(kal_bool enable){
    pmic6326_reg[0x58] &= ~(VPA_EN_FORCE_MASK << VPA_EN_FORCE_SHIFT);
    pmic6326_reg[0x58] |= ((kal_uint8)enable << VPA_EN_FORCE_SHIFT);
    mt6326_write_byte(0x58, pmic6326_reg[0x58]);
}

void pmic_vpa_plnmos_dis(kal_bool disable){
    pmic6326_reg[0x58] &= ~(VPA_PLNMOS_DIS_MASK << VPA_PLNMOS_DIS_SHIFT);
    pmic6326_reg[0x58] |= ((kal_uint8)disable << VPA_PLNMOS_DIS_SHIFT);
    mt6326_write_byte(0x58, pmic6326_reg[0x58]);
}

void pmic_vpa_enable(kal_bool enable){
    pmic6326_reg[0x58] &= ~(VPA_EN_MASK << VPA_EN_SHIFT);
    pmic6326_reg[0x58] |= ((kal_uint8)enable << VPA_EN_SHIFT);
    mt6326_write_byte(0x58, pmic6326_reg[0x58]);
}

/* (0x59) BUCK CTRL 17 VPA */
void pmic_vpa_tunel(kal_uint8 value){
    ASSERT(value <= 31);
    pmic6326_reg[0x59] &= ~(VPA_TUNEL_MASK << VPA_TUNEL_SHIFT);
    pmic6326_reg[0x59] |= (value << VPA_TUNEL_SHIFT);
    mt6326_write_byte(0x59, pmic6326_reg[0x59]);
}

/* (0x5A) BUCK CTRL 18 VPA */
void pmic_vpa_oc_tune(kal_uint8 val){
    ASSERT(val <= 7);
    pmic6326_reg[0x5A] &= ~(VPA_OC_TH_MASK << VPA_OC_TH_SHIFT);
    pmic6326_reg[0x5A] |= (val << VPA_OC_TH_SHIFT);
    mt6326_write_byte(0x5A, pmic6326_reg[0x5A]);
}

/* (0x5C) BOOST CTRL 1 BOOST1 */
void pmic_vboost1_tune(vboost1_tune_enum sel){
    pmic6326_reg[0x5C] &= ~(VBOOST1_TUNE_MASK << VBOOST1_TUNE_SHIFT);
    pmic6326_reg[0x5C] |= ((kal_uint8)sel << VBOOST1_TUNE_SHIFT);
    mt6326_write_byte(0x5C, pmic6326_reg[0x5C]);
}

void pmic_vboost1_tatt(kal_uint8 val){
    ASSERT(val <=15);
    pmic6326_reg[0x5C] &= ~(VBOOST1_TATT_MASK << VBOOST1_TATT_SHIFT);
    pmic6326_reg[0x5C] |= (val << VBOOST1_TATT_SHIFT);
    mt6326_write_byte(0x5C, pmic6326_reg[0x5C]);
}

/* (0x5D) BOOST CTRL 2 BOOST1 */
void pmic_boost1_oc_th(kal_uint8 val){
    ASSERT(val <= 7);
    pmic6326_reg[0x5D] &= ~(BOOST1_OC_TH_MASK << BOOST1_OC_TH_SHIFT);
    pmic6326_reg[0x5D] |= (val << BOOST1_OC_TH_SHIFT);
    mt6326_write_byte(0x5D, pmic6326_reg[0x5D]);
}

void pmic_boost1_enable(kal_bool enable){
    if (enable){
        pmic6326_vboost1_set_flag |= VBOOST1_SET_FLAG_BOOST1;
    }else{
        pmic6326_vboost1_set_flag &= ~(VBOOST1_SET_FLAG_BOOST1);
    }
    pmic_boost1_enable_internal(pmic6326_vboost1_set_flag);
}

void pmic_boost1_pre_sr_con(kal_uint8 val){
    ASSERT(val <= 7);
    pmic6326_reg[0x5D] &= ~(BOOST1_PRE_SR_CON_MASK << BOOST1_PRE_SR_CON_SHIFT);
    pmic6326_reg[0x5D] |= (val << BOOST1_PRE_SR_CON_SHIFT);
    mt6326_write_byte(0x5D, pmic6326_reg[0x5D]);
}

void pmic_boost1_soft_st_speed(boost1_soft_st_speed_enum sel){
    pmic6326_reg[0x5D] &= ~(BOOST1_SOFT_ST_SPEED_MASK << BOOST1_SOFT_ST_SPEED_SHIFT);
    pmic6326_reg[0x5D] |= ((kal_uint8)sel << BOOST1_SOFT_ST_SPEED_SHIFT);
    mt6326_write_byte(0x5D, pmic6326_reg[0x5D]);
}

/* (0x5E) BOOST CTRL 3 BOOST1 */
void pmic_boost1_dio_sr_con(kal_uint8 val){
    ASSERT(val <=7);
    pmic6326_reg[0x5E] &= ~(BOOST1_DIO_SR_CON_MASK << BOOST1_DIO_SR_CON_SHIFT);
    pmic6326_reg[0x5E] |= (val << BOOST1_DIO_SR_CON_SHIFT);
    mt6326_write_byte(0x5E, pmic6326_reg[0x5E]);
}

void pmic_boost1_sync_enable(kal_bool enable){
    pmic6326_reg[0x5E] &= ~(BOOST1_SYNC_EN_MASK << BOOST1_SYNC_EN_SHIFT);
    pmic6326_reg[0x5E] |= ((kal_uint8)enable << BOOST1_SYNC_EN_SHIFT);
    mt6326_write_byte(0x5E, pmic6326_reg[0x5E]);
}

/* (0x5F) BOOST CTRL 4 BOOST2 */
void pmic_boost2_tune(vboost2_tune_enum sel){
    pmic6326_reg[0x5F] &= ~(BOOST2_TUNE_MASK << BOOST2_TUNE_SHIFT);
    pmic6326_reg[0x5F] |= ((kal_uint8)sel << BOOST2_TUNE_SHIFT);
    mt6326_write_byte(0x5F, pmic6326_reg[0x5F]);
}

void pmic_boots2_oc_th(boost2_oc_th_enum sel){
    pmic6326_reg[0x5F] &= ~(BOOST2_OC_TH_MASK << BOOST2_OC_TH_SHIFT);
    pmic6326_reg[0x5F] |= ((kal_uint8)sel << BOOST2_OC_TH_SHIFT);
    mt6326_write_byte(0x5F, pmic6326_reg[0x5F]);
}

void pmic_boost2_dim_source(boost2_dim_source_enum sel){
    pmic6326_reg[0x5F] &= ~(BOOST2_DIM_SOURCE_MASK << BOOST2_DIM_SOURCE_SHIFT);
    pmic6326_reg[0x5F] |= ((kal_uint8)sel << BOOST2_DIM_SOURCE_SHIFT);
    mt6326_write_byte(0x5F, pmic6326_reg[0x5F]);
}

/* (0x60) BOOST CTRL 5 BOOST2 */
void pmic_boost2_pre_sr_con(kal_uint8 val){
    ASSERT(val <=7);
    pmic6326_reg[0x60] &= ~(BOOST2_PRE_SR_CON_MASK << BOOST2_PRE_SR_CON_SHIFT);
    pmic6326_reg[0x60] |= ((kal_uint8)val << BOOST2_PRE_SR_CON_SHIFT);
    mt6326_write_byte(0x60, pmic6326_reg[0x60]);
}

void pmic_boost2_enable(kal_bool enable){
    pmic6326_reg[0x60] &= ~(BOOST2_EN_MASK << BOOST2_EN_SHIFT);
    pmic6326_reg[0x60] |= ((kal_uint8)enable << BOOST2_EN_SHIFT);
    mt6326_write_byte(0x60, pmic6326_reg[0x60]);        
}

/* (0x61) BOOST CTRL 6 BOOST2 and BOOST */
void pmic_boost_mode(boost_mode_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 3);
    pmic6326_reg[0x61] &= ~(BOOST_MODE_MASK << BOOST_MODE_SHIFT);
    pmic6326_reg[0x61] |= ((kal_uint8)val << BOOST_MODE_SHIFT);
    mt6326_write_byte(0x61, pmic6326_reg[0x61]);
}

/* (0x62)  DRIVER CTRL 1 - RGB*/
void pmic_vcore2_dvfs_step_inc(kal_uint8 val){
    ASSERT(val <= 0x1F);
    pmic6326_reg[0x62] &= ~(VCORE2_DVFS_STEP_INC_MASK << VCORE2_DVFS_STEP_INC_SHIFT);
    pmic6326_reg[0x62] |= ((kal_uint8)val << VCORE2_DVFS_STEP_INC_SHIFT);
    mt6326_write_byte(0x62, pmic6326_reg[0x62]);  
}

void pmic_RGB_1_enable(kal_bool enable){
    
    pmic6326_reg[0x62] |= (enable << 0x0);
	
    mt6326_write_byte(0x62, pmic6326_reg[0x62]);
}

void pmic_RGB_2_enable(kal_bool enable){
    
    pmic6326_reg[0x62] |= (enable << 0x1);
	
    mt6326_write_byte(0x62, pmic6326_reg[0x62]);
}


/* (0x63) DRIVER CTRL 2 BUS */
void pmic_vbus_enable(kal_bool enable){
    
    pmic6326_reg[0x63] |= (enable << 0x0);
	
    mt6326_write_byte(0x63, pmic6326_reg[0x63]);
}

/* (0x64) DRIVER CTRL 3 GEN */
void pmic_igen_drv_isel(kal_uint8 sel){
    ASSERT(sel <=3);
    pmic6326_reg[0x64] &= ~(IGEN_DRV_ISEL_MASK << IGEN_DRV_ISEL_SHIFT);
    pmic6326_reg[0x64] |= (sel << IGEN_DRV_ISEL_SHIFT);
    mt6326_write_byte(0x64, pmic6326_reg[0x64]);
}

void pmic_igen_drv_force(kal_bool force){
    pmic6326_reg[0x64] &= ~(IGEN_DRV_FORCE_MASK << IGEN_DRV_FORCE_SHIFT);
    pmic6326_reg[0x64] |= (force << IGEN_DRV_FORCE_SHIFT);
    mt6326_write_byte(0x64, pmic6326_reg[0x64]);
}

void pmic_vgen_drv_bgsel(kal_uint8 sel){
    ASSERT(sel <=7);
    pmic6326_reg[0x64] &= ~(VGEN_DRV_BGSEL_MASK << VGEN_DRV_BGSEL_SHIFT);
    pmic6326_reg[0x64] |= ((kal_uint8)sel << VGEN_DRV_BGSEL_SHIFT);
    mt6326_write_byte(0x64, pmic6326_reg[0x64]);
}

/* (0x65) DRIVER CTRL 4 FLASH */
void pmic_flash_i_tune(kal_uint8 val){
    ASSERT((kal_uint8)val <= 15);
    pmic6326_reg[0x65] &= ~(FLASH_I_TUNE_MASK << FLASH_I_TUNE_SHIFT);
    pmic6326_reg[0x65] |= ((kal_uint8)val << FLASH_I_TUNE_SHIFT);
    mt6326_write_byte(0x65, pmic6326_reg[0x65]);
}

void pmic_flash_dim_div(kal_uint8 val){
    ASSERT(val<=15);
    pmic6326_reg[0x65] &= ~(FLASH_DIM_DIV_MASK << FLASH_DIM_DIV_SHIFT);
    pmic6326_reg[0x65] |= ((kal_uint8)val << FLASH_DIM_DIV_SHIFT);
    mt6326_write_byte(0x65, pmic6326_reg[0x65]);
}

/* (0x66) DRIVER CTRL 5 FLASH */
void pmic_flash_dim_duty(kal_uint8 duty){
    ASSERT(duty <=31);
    pmic6326_reg[0x66] &= ~(FLASH_DIM_DUTY_MASK << FLASH_DIM_DUTY_SHIFT);
    pmic6326_reg[0x66] |= (duty << FLASH_DIM_DUTY_SHIFT);
    mt6326_write_byte(0x66, pmic6326_reg[0x66]);
}

void pmic_flash_enable(kal_bool enable){
    if (enable){
        pmic6326_vboost1_set_flag |= VBOOST1_SET_FLAG_FLASH;
        pmic6326_dim_ck_set_flag |= DIM_CK_ON_FLAG_FLASH;
    }else{
        pmic6326_vboost1_set_flag &= ~VBOOST1_SET_FLAG_FLASH;
        pmic6326_dim_ck_set_flag &= ~DIM_CK_ON_FLAG_FLASH;
    }
    //pmic_boost1_enable_internal(pmic6326_vboost1_set_flag);
    //pmic_dim_ck_force_on_internal(pmic6326_dim_ck_set_flag);

    pmic6326_reg[0x66] &= ~(FLASH_EN_MASK << FLASH_EN_SHIFT);
    pmic6326_reg[0x66] |= (enable << FLASH_EN_SHIFT);
    mt6326_write_byte(0x66, pmic6326_reg[0x66]);
}

// If bypass divisor, the frequency of BL is 50K/(div+1)
// If no bypass divisor, the frequency of BL is 2K/(div+1)
void pmic_flash_bypass(kal_bool bypass){
    // bypass = TRUE ==> Bypass divisor 25 for FLASH PWM
    // bypass = FALSE ==> No bypass for FLASH PWM
    pmic6326_reg[0x66] &= ~(FLASH_BYPASS_MASK << FLASH_BYPASS_SHIFT);
    pmic6326_reg[0x66] |= (bypass << FLASH_BYPASS_SHIFT);
    mt6326_write_byte(0x66, pmic6326_reg[0x66]);
}

/* (0x67) DRIVER CTRL 6 BL */
void pmic_bl_dim_duty(kal_uint8 duty){
    ASSERT(duty <=31);
    pmic6326_reg[0x67] &= ~(BL_DIM_DUTY_MASK << BL_DIM_DUTY_SHIFT);
    pmic6326_reg[0x67] |= (duty << BL_DIM_DUTY_SHIFT);
    mt6326_write_byte(0x67, pmic6326_reg[0x67]);
}

void pmic_bl_enable(kal_bool enable){
    if (enable){
        pmic6326_vboost1_set_flag |= VBOOST1_SET_FLAG_BL;
    }else{
        pmic6326_vboost1_set_flag &= ~VBOOST1_SET_FLAG_BL;
    }
    pmic_boost1_enable_internal(pmic6326_vboost1_set_flag);

    pmic6326_reg[0x67] &= ~(BL_EN_MASK << BL_EN_SHIFT);
    pmic6326_reg[0x67] |= (enable << BL_EN_SHIFT);
    mt6326_write_byte(0x67, pmic6326_reg[0x67]);
}

void pmic_bl_i_cal_enable(kal_bool enable){
    pmic6326_reg[0x67] &= ~(BL_I_CAL_EN_MASK << BL_I_CAL_EN_SHIFT);
    pmic6326_reg[0x67] |= (enable << BL_I_CAL_EN_SHIFT);
    mt6326_write_byte(0x67, pmic6326_reg[0x67]);
}

// If bypass divisor, the frequency of BL is 50K/(div+1)
// If no bypass divisor, the frequency of BL is 2K/(div+1)
void pmic_bl_bypass(kal_bool bypass){
    // bypass = TRUE ==> Bypass divisor 25 for BL PWM
    // bypass = FALSE ==> No bypass for BL PWM
    pmic6326_reg[0x67] &= ~(BL_BYPASS_MASK << BL_BYPASS_SHIFT);
    pmic6326_reg[0x67] |= (bypass << BL_BYPASS_SHIFT);
    mt6326_write_byte(0x67, pmic6326_reg[0x67]);
}

/* (0x68) DRIVER CTRL 7 BL */
void pmic_bl_i_corse_tune(bl_i_corse_tune_enum sel){
    ASSERT((kal_uint8)sel <= 7);
    pmic6326_reg[0x68] &= ~(BL_I_CORSE_TUNE_MASK << BL_I_CORSE_TUNE_SHIFT);
    pmic6326_reg[0x68] |= ((kal_uint8)sel << BL_I_CORSE_TUNE_SHIFT);
    mt6326_write_byte(0x68, pmic6326_reg[0x68]);
}

void pmic_bl_i_fine_tune(bl_i_fine_tune_enum sel){
    ASSERT((kal_uint8)sel <= 7);
    pmic6326_reg[0x68] &= ~(BL_I_FINE_TUNE_MASK << BL_I_FINE_TUNE_SHIFT);
    pmic6326_reg[0x68] |= ((kal_uint8)sel << BL_I_FINE_TUNE_SHIFT);
    mt6326_write_byte(0x68, pmic6326_reg[0x68]);
}

/* (0x6D) DRIVER CTRL 12 BL */
void pmic_bl_dim_div(kal_uint8 val){
    ASSERT(val<=15);
    pmic6326_reg[0x6D] &= ~(BL_DIM_DIV_MASK << BL_DIM_DIV_SHIFT);
    pmic6326_reg[0x6D] |= ((kal_uint8)val << BL_DIM_DIV_SHIFT);
    mt6326_write_byte(0x6D, pmic6326_reg[0x6D]);
}

void pmic_bl_number(bl_number_enum num){
    ASSERT((kal_uint8)num<=7);
    pmic6326_reg[0x6D] &= ~(BL_NUMBER_MASK << BL_NUMBER_SHIFT);
    pmic6326_reg[0x6D] |= ((kal_uint8)num << BL_NUMBER_SHIFT);
    mt6326_write_byte(0x6D, pmic6326_reg[0x6D]);
}

/* (0x6E) DRIVER CTRL 13 KP */
void pmic_kp_dim_div(kal_uint8 val){
    ASSERT(val<=15);
    pmic6326_reg[0x6E] &= ~(KP_DIM_DIV_MASK << KP_DIM_DIV_SHIFT);
    pmic6326_reg[0x6E] |= ((kal_uint8)val << KP_DIM_DIV_SHIFT);
    mt6326_write_byte(0x6E, pmic6326_reg[0x6E]);
}

void pmic_kp_enable(kal_bool enable){
    if (enable){
        pmic6326_dim_ck_set_flag |= DIM_CK_ON_FLAG_KEY;
    }else{
        pmic6326_dim_ck_set_flag &= ~DIM_CK_ON_FLAG_KEY;
    }
    pmic_dim_ck_force_on_internal(pmic6326_dim_ck_set_flag);

    pmic6326_reg[0x6E] &= ~(KP_EN_MASK << KP_EN_SHIFT);
    pmic6326_reg[0x6E] |= (enable << KP_EN_SHIFT);
	
    mt6326_write_byte(0x6E, pmic6326_reg[0x6E]);
}

/* (0x6F) DRIVER CTRL 14 KP */
void pmic_kp_dim_duty(kal_uint8 duty){
    ASSERT(duty <=31);
    pmic6326_reg[0x6F] &= ~(KP_DIM_DUTY_MASK << KP_DIM_DUTY_SHIFT);
    pmic6326_reg[0x6F] |= (duty << KP_DIM_DUTY_SHIFT);
    mt6326_write_byte(0x6F, pmic6326_reg[0x6F]);
}

/* (0x70) DRIVER CTRL 15 VIBR */
void pmic_vibr_dim_div(kal_uint8 val){
    ASSERT(val<=15);
    pmic6326_reg[0x70] &= ~(VIBR_DIM_DIV_MASK << VIBR_DIM_DIV_SHIFT);
    pmic6326_reg[0x70] |= ((kal_uint8)val << VIBR_DIM_DIV_SHIFT);
    mt6326_write_byte(0x70, pmic6326_reg[0x70]);
}

void pmic_vibr_enable(kal_bool enable){
    if (enable){
        pmic6326_dim_ck_set_flag |= DIM_CK_ON_FLAG_VIB;
    }else{
        pmic6326_dim_ck_set_flag &= ~DIM_CK_ON_FLAG_VIB;
    }
    pmic_dim_ck_force_on_internal(pmic6326_dim_ck_set_flag);

    pmic6326_reg[0x70] &= ~(VIBR_EN_MASK << VIBR_EN_SHIFT);
    pmic6326_reg[0x70] |= (enable << VIBR_EN_SHIFT);
    mt6326_write_byte(0x70, pmic6326_reg[0x70]);
}

/* (0x71) DRIVER CTRL 16 VIBR */
void pmic_vibr_dim_duty(kal_uint8 duty){
    ASSERT(duty <=31);
    pmic6326_reg[0x71] &= ~(VIBR_DIM_DUTY_MASK << VIBR_DIM_DUTY_SHIFT);
    pmic6326_reg[0x71] |= (duty << VIBR_DIM_DUTY_SHIFT);
    mt6326_write_byte(0x71, pmic6326_reg[0x71]);
}

/* (0x73) CLASS_D CTRL 3 SPKL */
void pmic_spkl_dtin(kal_uint8 val){
    pmic6326_reg[0x73] &= ~(SPKL_DTIN_MASK << SPKL_DTIN_SHIFT);
    pmic6326_reg[0x73] |= (val << SPKL_DTIN_SHIFT);
    mt6326_write_byte(0x73, pmic6326_reg[0x73]);
}

void pmic_spkl_dtip(kal_uint8 val){
    pmic6326_reg[0x73] &= ~(SPKL_DTIP_MASK << SPKL_DTIP_SHIFT);
    pmic6326_reg[0x73] |= (val << SPKL_DTIP_SHIFT);
    mt6326_write_byte(0x73, pmic6326_reg[0x73]);
}

/* (0x74) CLASS_D CTRL 4 SPKL */
void pmic_spkl_dmode(spkl_dmode_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 3);
    pmic6326_reg[0x74] &= ~(SPKL_DMODE_MASK << SPKL_DMODE_SHIFT);
    pmic6326_reg[0x74] |= ((kal_uint8)val << SPKL_DMODE_SHIFT);
    mt6326_write_byte(0x74, pmic6326_reg[0x74]);
}

void pmic_spkl_enable(kal_bool enable){
    pmic6326_reg[0x74] &= ~(SPKL_EN_MASK << SPKL_EN_SHIFT);
    pmic6326_reg[0x74] |= ((kal_uint8)enable << SPKL_EN_SHIFT);
    mt6326_write_byte(0x74, pmic6326_reg[0x74]);
}

void pmic_spkl_dtcal(spkl_dtcal_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x74] &= ~(SPKL_DTCAL_MASK << SPKL_DTCAL_SHIFT);
    pmic6326_reg[0x74] |= ((kal_uint8)val << SPKL_DTCAL_SHIFT);
    mt6326_write_byte(0x74, pmic6326_reg[0x74]);
}

/* (0x76) CLASS_D CTRL 6 SPKL */
void pmic_spkl_vol(kal_uint8 val){
    pmic6326_reg[0x76] &= ~(SPKL_VOL_MASK << SPKL_VOL_SHIFT);
    pmic6326_reg[0x76] |= (val << SPKL_VOL_SHIFT);
    mt6326_write_byte(0x76, pmic6326_reg[0x76]);
}

/* (0x78) CLASS_D CTRL 8 SPKR */
void pmic_spkr_dtin(kal_uint8 val){
    pmic6326_reg[0x78] &= ~(SPKR_DTIN_MASK << SPKR_DTIN_SHIFT);
    pmic6326_reg[0x78] |= (val << SPKR_DTIN_SHIFT);
    mt6326_write_byte(0x78, pmic6326_reg[0x78]);
}

void pmic_spkr_dtip(kal_uint8 val){
    pmic6326_reg[0x78] &= ~(SPKR_DTIP_MASK << SPKR_DTIP_SHIFT);
    pmic6326_reg[0x78] |= (val << SPKR_DTIP_SHIFT);
    mt6326_write_byte(0x78, pmic6326_reg[0x78]);
}

/* (0x79) CLASS_D CTRL 9 SPKR */
void pmic_spkr_dmode(spkr_dmode_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 3);
    pmic6326_reg[0x79] &= ~(SPKR_DMODE_MASK << SPKR_DMODE_SHIFT);
    pmic6326_reg[0x79] |= ((kal_uint8)val << SPKR_DMODE_SHIFT);
    mt6326_write_byte(0x79, pmic6326_reg[0x79]);
}

void pmic_spkr_enable(kal_bool enable){
    pmic6326_reg[0x79] &= ~(SPKR_EN_MASK << SPKR_EN_SHIFT);
    pmic6326_reg[0x79] |= ((kal_uint8)enable << SPKR_EN_SHIFT);
    mt6326_write_byte(0x79, pmic6326_reg[0x79]);
}

void pmic_spkr_dtcal(spkr_dtcal_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x79] &= ~(SPKR_DTCAL_MASK << SPKR_DTCAL_SHIFT);
    pmic6326_reg[0x79] |= ((kal_uint8)val << SPKR_DTCAL_SHIFT);
    mt6326_write_byte(0x79, pmic6326_reg[0x79]);
}

/* (0x7B) CLASS_D CTRL 11 SPKR */
void pmic_spkr_vol(kal_uint8 val){
    pmic6326_reg[0x7B] &= ~(SPKR_VOL_MASK << SPKR_VOL_SHIFT);
    pmic6326_reg[0x7B] |= (val << SPKR_VOL_SHIFT);
    mt6326_write_byte(0x7B, pmic6326_reg[0x7B]);
}

/* (0x81) CHARGER CTRL 1 */
void pmic_chr_offset(cht_chr_offset_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <=7);
    pmic6326_reg[0x81] &= ~(CHR_CHOFST_MASK << CHR_CHOFST_SHIFT);
    pmic6326_reg[0x81] |= ((kal_uint8)val << CHR_CHOFST_SHIFT);
    mt6326_write_byte(0x81, pmic6326_reg[0x81]);
}

void pmic_chr_ov_th_high(void){
    // TODO
    ASSERT(0);
}

void pmic_chr_chr_enable(kal_bool enable);

void pmic_chr_current(chr_chr_current_enum curr){
    //ASSERT((kal_uint8)curr <= 7);
    if(curr >= 7)
    {
    	pmic_chr_chr_enable(KAL_FALSE);
    }
	else 
	{	
	    pmic6326_reg[0x81] &= ~(CHR_CHR_CURRENT_MASK << CHR_CHR_CURRENT_SHIFT);
	    pmic6326_reg[0x81] |= ((kal_uint8)curr << CHR_CHR_CURRENT_SHIFT);
	    mt6326_write_byte(0x81, pmic6326_reg[0x81]);
	}
}

/* (0x82) CHARGER CTRL 2 */
void pmic_chr_cv_rt(void){
    // TODO
    ASSERT(0);
}

void pmic_chr_force(kal_bool force){
    pmic6326_reg[0x82] &= ~(CHR_CHRON_FORCE_MASK << CHR_CHRON_FORCE_SHIFT);
    pmic6326_reg[0x82] |= ((kal_uint8)force << CHR_CHRON_FORCE_SHIFT);
    mt6326_write_byte(0x82, pmic6326_reg[0x82]);
}

void pmic_chr_chr_enable(kal_bool enable){
    pmic6326_reg[0x82] &= ~(CHR_CHR_EN_MASK << CHR_CHR_EN_SHIFT);
    pmic6326_reg[0x82] |= ((kal_uint8)enable << CHR_CHR_EN_SHIFT);

    mt6326_write_byte(0x82, pmic6326_reg[0x82]);
}

void pmic_chr_cv_tune(void){
    // TODO
}

/* (0x83) TESTMODE CTRL 3 Analog Switch */
void pmic_asw_asel(asw_asel_enum sel){
    pmic6326_reg[0x83] &= ~(ASW_ASEL_MASK << ASW_ASEL_SHIFT);
    pmic6326_reg[0x83] |= ((kal_uint8)sel << ASW_ASEL_SHIFT);
    mt6326_write_byte(0x83, pmic6326_reg[0x83]);
}

void pmic_asw_bsel(asw_bsel_enum sel){
    //ASSERT((kal_uint8)sel <= 2);		// 0x3 is reserved
    pmic6326_reg[0x83] &= ~(ASW_BSEL_MASK << ASW_BSEL_SHIFT);
    pmic6326_reg[0x83] |= ((kal_uint8)sel << ASW_BSEL_SHIFT);
    mt6326_write_byte(0x83, pmic6326_reg[0x83]);
}

void pmic_asw_a1sel(kal_uint8 sel){
    ASSERT(sel <= 1);
    pmic6326_reg[0x83] &= ~(ASW_A1_SEL_MASK << ASW_A1_SEL_SHIFT);
    pmic6326_reg[0x83] |= (sel << ASW_A1_SEL_SHIFT);
    mt6326_write_byte(0x83, pmic6326_reg[0x83]);
}

void pmic_asw_a2sel(kal_uint8 sel){
    ASSERT(sel <= 1);
    pmic6326_reg[0x83] &= ~(ASW_A2_SEL_MASK << ASW_A2_SEL_SHIFT);
    pmic6326_reg[0x83] |= (sel << ASW_A2_SEL_SHIFT);
    mt6326_write_byte(0x83, pmic6326_reg[0x83]);
}

/* (0x86) TESTMODE CTRL 6 BB AUXADC Related */
void pmic_adc_isense_enable(kal_bool enable){
    pmic6326_reg[0x86] &= ~(ADC_ISENSE_OUT_EN_MASK << ADC_ISENSE_OUT_EN_SHIFT);
    pmic6326_reg[0x86] |= ((kal_uint8)enable << ADC_ISENSE_OUT_EN_SHIFT);
    mt6326_write_byte(0x86, pmic6326_reg[0x86]);
}

void pmic_adc_vbat_enable(kal_bool enable){
    pmic6326_reg[0x86] &= ~(ADC_VBAT_OUT_EN_MASK << ADC_VBAT_OUT_EN_SHIFT);
    pmic6326_reg[0x86] |= ((kal_uint8)enable << ADC_VBAT_OUT_EN_SHIFT);
    mt6326_write_byte(0x86, pmic6326_reg[0x86]);
}

void pmic6326_adc_meas_on(kal_bool on){
    if (on){
        pmic6326_reg[0x86] |= (((kal_uint8)ADC_VBAT_OUT_EN_MASK << ADC_VBAT_OUT_EN_SHIFT) | ((kal_uint8)ADC_ISENSE_OUT_EN_MASK << ADC_ISENSE_OUT_EN_SHIFT));
    }else{
        pmic6326_reg[0x86] &= ~(((kal_uint8)ADC_VBAT_OUT_EN_MASK << ADC_VBAT_OUT_EN_SHIFT) | ((kal_uint8)ADC_ISENSE_OUT_EN_MASK << ADC_ISENSE_OUT_EN_SHIFT));
    }
    mt6326_write_byte(0x86, pmic6326_reg[0x86]);
}

/* (0x89) INT CTRL 1 */
void pmic_int_ctrl_1_enable(int_ctrl_1_enum sel, kal_bool enable){
    ASSERT((kal_uint32)sel <= 0xFF);
    if (enable){
        pmic6326_reg[0x89] |= (kal_uint8)sel;
    }else{
        pmic6326_reg[0x89] &= ~((kal_uint8)sel);
    }
    mt6326_write_byte(0x89, pmic6326_reg[0x89]);
}

/* (0x8A) INT CTRL 2 */
void pmic_int_ctrl_2_enable(int_ctrl_2_enum sel, kal_bool enable){
    ASSERT((kal_uint32)sel <= 0xFF);
    if (enable){
        pmic6326_reg[0x8A] |= (kal_uint8)sel;
    }else{
        pmic6326_reg[0x8A] &= ~((kal_uint8)sel);
    }
    mt6326_write_byte(0x8A, pmic6326_reg[0x8A]);
}

/*(0x8B) INT CTRL 2 */
void pmic_int_ctrl_3_enable(int_ctrl_3_enum sel, kal_bool enable){
    ASSERT((kal_uint32)sel <= 0xFF);
    if (enable){
        pmic6326_reg[0x8B] |= (kal_uint8)sel;
    }else{
        pmic6326_reg[0x8B] &= ~((kal_uint8)sel);
    }
    mt6326_write_byte(0x8B, pmic6326_reg[0x8B]);
}

/* (0x96) WATCHDOG CTRL and INT CTRL 4 */
void pmic_wdt_timeout(wdt_timout_enum sel){
    ASSERT((kal_uint8)sel <=3);
    pmic6326_reg[0x96] &= ~(WDT_TIMEOUT_MASK << WDT_TIMEOUT_SHIFT);
    pmic6326_reg[0x96] |= ((kal_uint8)sel << WDT_TIMEOUT_SHIFT);
    mt6326_write_byte(0x96, pmic6326_reg[0x96]);
}

void pmic_intr_polarity(kal_bool assert){
    pmic6326_reg[0x96] &= ~(INTR_POLARITY_MASK << INTR_POLARITY_SHIFT);
    // 1. Flag assert when intr occur
    // 0. Flag de-assert when intr occur
    pmic6326_reg[0x96] |= ((kal_uint8)assert << INTR_POLARITY_SHIFT);
    mt6326_write_byte(0x96, pmic6326_reg[0x96]);
}

void pmic_wdt_enable(kal_bool enable){
    pmic6326_reg[0x96] &= ~(WDT_DISABLE_MASK << WDT_DISABLE_SHIFT);
    // 1: Disable WDT
    // 0: Enable WDT
    pmic6326_reg[0x96] |= ((kal_uint8)(~enable) << WDT_DISABLE_SHIFT);
    mt6326_write_byte(0x96, pmic6326_reg[0x96]);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void pmic_vgp2_enable(kal_bool enable){
    pmic6326_reg[0x1A] &= ~(VGP2_EN_MASK << VGP2_EN_SHIFT);
    pmic6326_reg[0x1A] |= (enable << VGP2_EN_SHIFT);
    mt6326_write_byte(0x1A, pmic6326_reg[0x1A]);
}

void pmic_vgp2_on_sel(vgp2_on_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 1);
    pmic6326_reg[0x84] &= ~(VGP2_ON_SEL_MASK << VGP2_ON_SEL_SHIFT);
    pmic6326_reg[0x84] |= (val << VGP2_ON_SEL_SHIFT);
    mt6326_write_byte(0x84, pmic6326_reg[0x84]);
}

void pmic_vgp2_sell(kal_uint8 value){
    ASSERT(value <=3);
    pmic6326_reg[0x49] &= ~(VGP2_SELL_MASK << VGP2_SELL_SHIFT);
    pmic6326_reg[0x49] |= (value << VGP2_SELL_SHIFT);
    mt6326_write_byte(0x49, pmic6326_reg[0x49]);
}

void pmic_vgp2_selh(kal_uint8 value){
    ASSERT(value <=1);
    pmic6326_reg[0x5E] &= ~(VGP2_SELH_MASK << VGP2_SELH_SHIFT);
    pmic6326_reg[0x5E] |= (value << VGP2_SELH_SHIFT);
    mt6326_write_byte(0x5E, pmic6326_reg[0x5E]);    
}

void pmic_vgp2_sel(vgp2_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 6);
    pmic_vgp2_selh(val >> 2);	// Extract bit2 to write to selh
    pmic_vgp2_sell(val & 0x3);	// Extract bit[1..0] to write to sell
}

void pmic_vsim2_enable(kal_bool enable){
    pmic_vgp2_enable(enable);
    // Always to VGP2_EN_SEL to VGP2_ENABLE_WITH_VGP2_EN
    pmic_vgp2_on_sel(VGP2_ENABLE_WITH_VGP2_EN);
}

void pmic_vsim2_sel(vsim_sel_enum sel){
    kal_uint8 val;
    val = (kal_uint8)sel;
    ASSERT(val <= 6);
    pmic_vgp2_selh(val >> 2);	// Extract bit2 to write to selh
    pmic_vgp2_sell(val & 0x3);	// Extract bit[1..0] to write to sell
}

void pmic_spk_enable(kal_bool enable){
	// Empty function, not used anymore
}

// Exported for EM used
void pmic6326_EM_reg_write(kal_uint8 reg, kal_uint8 val){
    ASSERT(reg <= PMIC_MAX_REG_IDX);
    pmic6326_reg[reg] = val;
    mt6326_write_byte(reg, pmic6326_reg[reg]);
}

kal_uint8 pmic6326_EM_reg_read(kal_uint8 reg){
    ASSERT(reg <= PMIC_MAX_REG_IDX);
    mt6326_read_byte(reg, &pmic6326_reg[reg]);
    return pmic6326_reg[reg];
}

//#if defined(ENABLE_KICK_PMIC6326_CHARGER_WATCHDOG_TIMER)
#if 1
//void pmic6326_kick_charger_wdt(void *parameter){
void pmic6326_kick_charger_wdt(void){
	pmic_watchdog_clear();
}

void pmic_kick_wdt(kal_bool enable){
    
    if (enable){
        pmic6326_kick_charger_wdt();
    }
   
}
#endif

void pmic6326_customization_init(void)
{
	/*customization*/
#ifdef __FUE__	
       printk("pmic_init :  __FUE__ !!\n "); 
	pmic_boost_mode(BOOST_MODE);
	pmic_bl_number(BL_NUM);
	pmic_vboost1_tune(VBOOST1_TUNE);
	pmic_bl_i_corse_tune(BL_I_COARSE_TUNE);
	pmic_vcore2_on_sel(VCORE2_ON_SEL);
	pmic_vcore2_enable(VCORE2_EN);
	// Init BL (Request from MT6268 reference phone)
	//5F->40    Analog Dimming
	pmic_boost2_dim_source(BOOST2_ANALOG_DIMING);
	//61->10    Type II Config
	pmic_boost_mode(BOOST_MODE_TYPE_II);
	//60->10    Boost2 Enable (Disable to turn off BL)
	pmic_boost2_enable(KAL_TRUE);
	//67->3F    BLEN and Max Duty, Control duty to control dimming
	//pmic_bl_dim_duty(PMIC_DEFAULT_BL_DIM);
	//pmic_bl_enable(KAL_TRUE);
#else
       printk("pmic_init :  Normal !!\n ");  
	pmic_vrf_cal(VRF_CAL);
	pmic_vtcxo_cal(VTCXO_CAL);
	pmic_v3gtx_cal(V3GTX_CAL);
	pmic_v3gtx_sel(V3GTX_SEL);
	pmic_v3grx_cal(V3GRX_CAL);
	pmic_v3grx_sel(V3GRX_SEL);
	pmic_vcama_cal(VCAMA_CAL);
	pmic_vcama_sel(VCAMA_SEL);
	pmic_vwifi3v3_cal(VWIFI3V3_CAL);
	pmic_vwifi3v3_sel(VWIFI3V3_SEL);
	pmic_vwifi2v8_cal(VWIFI2V8_CAL);
	pmic_vwifi2v8_sel(VWIFI2V8_SEL);
	pmic_vsim_cal(VSIM_CAL);
	pmic_vbt_cal(VBT_CAL);
	pmic_vbt_sel(VBT_SEL);
	pmic_vcamd_cal(VCAMD_CAL);
	pmic_vcamd_sel(VCAMD_SEL);
	pmic_vgp_cal(VGP_CAL);
	pmic_vgp_sel(VGP_SEL);
	pmic_vsdio_cal(VSDIO_CAL);
	pmic_vsdio_sel(VSDIO_SEL);
	pmic_vgp2_sel(VGP2_SEL);
	pmic_vgp2_on_sel(VGP2_ON_SEL);
	pmic_chr_offset(CHR_OFFSET);
	pmic_boost_mode(BOOST_MODE);
	pmic_bl_number(BL_NUM);
	pmic_asw_asel(ASW_ASEL);
	pmic_vboost1_tune(VBOOST1_TUNE);
	pmic_bl_i_corse_tune(BL_I_COARSE_TUNE);
	pmic_asw_bsel(ASW_BSEL);  

	
       #if 0
	pmic_boost_mode(BOOST_MODE);
	pmic_bl_number(BL_NUM);
	pmic_asw_asel(ASW_ASEL);
	pmic_vboost1_tune(VBOOST1_TUNE);
	pmic_bl_i_corse_tune(BL_I_COARSE_TUNE);
	pmic_asw_bsel(ASW_BSEL);
	#endif
	
       #if 0
	pmic_vcore2_on_sel(VCORE2_ON_SEL);
	pmic_vcore2_enable(VCORE2_EN);
       #endif

	// Init SPKL
	if (USE_SPKL == KAL_TRUE){
		pmic_spkl_enable(KAL_TRUE);
		pmic_spkl_dtip(0xF);
		pmic_spkl_dtin(0xF);
		pmic_spkl_dtcal(SPKL_DTCAL_DISABLE_CLASS_D_R_READ_TIME_CAL);
		pmic_spkl_dmode(SPKL_FF_AUTO_CAL_DTCN_DTCP);
		pmic_spkl_dtcal(SPKL_DTCAL_ENABLE_CLASS_D_R_READ_TIME_CAL);
		pmic_spkl_enable(KAL_FALSE);
	}
	// Init SPKR
	if (USE_SPKR == KAL_TRUE){
		pmic_spkr_enable(KAL_TRUE);
		pmic_spkr_dtip(0xF);
		pmic_spkr_dtin(0xF);
		pmic_spkr_dtcal(SPKR_DTCAL_DISABLE_CLASS_D_R_READ_TIME_CAL);
		pmic_spkr_dmode(SPKR_FF_AUTO_CAL_DTCN_DTCP);
		pmic_spkr_dtcal(SPKR_DTCAL_ENABLE_CLASS_D_R_READ_TIME_CAL);
		pmic_spkr_enable(KAL_FALSE);
	}

       /* Backlight Setting */
       
	// Init BL (Request from MT6268 reference phone)
	//5F->40    Analog Dimming
	pmic_boost2_dim_source(BOOST2_ANALOG_DIMING);
	//61->10    Type II Config
	pmic_boost_mode(BOOST_MODE_TYPE_II);
	//60->10    Boost2 Enable (Disable to turn off BL)
	pmic_boost2_enable(KAL_TRUE);
	//67->3F    BLEN and Max Duty, Control duty to control dimming
	pmic_bl_dim_duty(PMIC_DEFAULT_BL_DIM);
	pmic_bl_enable(KAL_FALSE);

	 /* set VSDIO output voltage for MSDC slot */
	 pmic_vsdio_enable(KAL_TRUE);
#endif
	/*end of customization*/
}   


//static ssize_t  mt6326_enable_VRF(void){
ssize_t  mt6326_enable_VRF(void){

    pmic_vrf_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VRF  = {
	.attr = {
		.name = "mt6326_enable_VRF",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VRF,
};
#endif

//static ssize_t  mt6326_disable_VRF(void){
ssize_t  mt6326_disable_VRF(void){

    pmic_vrf_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VRF  = {
	.attr = {
		.name = "mt6326_disable_VRF",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VRF,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VTCXO(void){
ssize_t  mt6326_enable_VTCXO(void){

    pmic_vtcxo_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VTCXO  = {
	.attr = {
		.name = "mt6326_enable_VTCXO",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VTCXO,
};
#endif

//static ssize_t  mt6326_disable_VTCXO(void){
ssize_t  mt6326_disable_VTCXO(void){

    pmic_vtcxo_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VTCXO  = {
	.attr = {
		.name = "mt6326_disable_VTCXO",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VTCXO,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_V3GTX(void){
ssize_t  mt6326_enable_V3GTX(void){

    pmic_v3gtx_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_V3GTX  = {
	.attr = {
		.name = "mt6326_enable_V3GTX",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_V3GTX,
};
#endif

//static ssize_t  mt6326_disable_V3GTX(void){
ssize_t  mt6326_disable_V3GTX(void){

    pmic_v3gtx_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_V3GTX  = {
	.attr = {
		.name = "mt6326_disable_V3GTX",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_V3GTX,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_V3GRX(void){
ssize_t  mt6326_enable_V3GRX(void){

    pmic_v3grx_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_V3GRX  = {
	.attr = {
		.name = "mt6326_enable_V3GRX",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_V3GRX,
};
#endif

//static ssize_t  mt6326_disable_V3GRX(void){
ssize_t  mt6326_disable_V3GRX(void){

    pmic_v3grx_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_V3GRX  = {
	.attr = {
		.name = "mt6326_disable_V3GRX",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_V3GRX,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VCAM_A(void){
ssize_t  mt6326_enable_VCAM_A(void){

    pmic_vcama_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VCAM_A  = {
	.attr = {
		.name = "mt6326_enable_VCAM_A",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VCAM_A,
};
#endif

//static ssize_t  mt6326_disable_VCAM_A(void){
ssize_t  mt6326_disable_VCAM_A(void){

    pmic_vcama_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VCAM_A  = {
	.attr = {
		.name = "mt6326_disable_VCAM_A",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VCAM_A,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VWIFI3V3(void){
ssize_t  mt6326_enable_VWIFI3V3(void){

    pmic_vwifi3v3_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VWIFI3V3  = {
	.attr = {
		.name = "mt6326_enable_VWIFI3V3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VWIFI3V3,
};
#endif

//static ssize_t  mt6326_disable_VWIFI3V3(void){
ssize_t  mt6326_disable_VWIFI3V3(void){

    pmic_vwifi3v3_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VWIFI3V3  = {
	.attr = {
		.name = "mt6326_disable_VWIFI3V3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VWIFI3V3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VWIFI2V8(void){
ssize_t  mt6326_enable_VWIFI2V8(void){

    pmic_vwifi2v8_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VWIFI2V8  = {
	.attr = {
		.name = "mt6326_enable_VWIFI2V8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VWIFI2V8,
};
#endif

//static ssize_t  mt6326_disable_VWIFI2V8(void){
ssize_t  mt6326_disable_VWIFI2V8(void){

    pmic_vwifi2v8_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VWIFI2V8  = {
	.attr = {
		.name = "mt6326_disable_VWIFI2V8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VWIFI2V8,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VSIM(void){
ssize_t  mt6326_enable_VSIM(void){

    pmic_vsim_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VSIM  = {
	.attr = {
		.name = "mt6326_enable_VSIM",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VSIM,
};
#endif

//static ssize_t  mt6326_disable_VSIM(void){
ssize_t  mt6326_disable_VSIM(void){

    pmic_vsim_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VSIM  = {
	.attr = {
		.name = "mt6326_disable_VSIM",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VSIM,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VUSB(void){
ssize_t  mt6326_enable_VUSB(void){

    pmic_vusb_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VUSB  = {
	.attr = {
		.name = "mt6326_enable_VUSB",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VUSB,
};
#endif

//static ssize_t  mt6326_disable_VUSB(void){
ssize_t  mt6326_disable_VUSB(void){

    pmic_vusb_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VUSB  = {
	.attr = {
		.name = "mt6326_disable_VUSB",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VUSB,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VBT(void){
ssize_t  mt6326_enable_VBT(void){

    pmic_vbt_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VBT  = {
	.attr = {
		.name = "mt6326_enable_VBT",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VBT,
};
#endif

//static ssize_t  mt6326_disable_VBT(void){
ssize_t  mt6326_disable_VBT(void){

    pmic_vbt_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VBT  = {
	.attr = {
		.name = "mt6326_disable_VBT",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VBT,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VCAM_D(void){
ssize_t  mt6326_enable_VCAM_D(void){

    pmic_vcamd_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VCAM_D  = {
	.attr = {
		.name = "mt6326_enable_VCAM_D",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VCAM_D,
};
#endif

//static ssize_t  mt6326_disable_VCAM_D(void){
ssize_t  mt6326_disable_VCAM_D(void){

    pmic_vcamd_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VCAM_D  = {
	.attr = {
		.name = "mt6326_disable_VCAM_D",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VCAM_D,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VGP(void){
ssize_t  mt6326_enable_VGP(void){

    pmic_vgp_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VGP  = {
	.attr = {
		.name = "mt6326_enable_VGP",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VGP,
};
#endif

//static ssize_t  mt6326_disable_VGP(void){
ssize_t  mt6326_disable_VGP(void){

    pmic_vgp_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VGP  = {
	.attr = {
		.name = "mt6326_disable_VGP",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VGP,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VGP2(void){
ssize_t  mt6326_enable_VGP2(void){

    pmic_vgp2_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VGP2  = {
	.attr = {
		.name = "mt6326_enable_VGP2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VGP2,
};
#endif

//static ssize_t  mt6326_disable_VGP2(void){
ssize_t  mt6326_disable_VGP2(void){

    pmic_vgp2_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VGP2  = {
	.attr = {
		.name = "mt6326_disable_VGP2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VGP2,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VSDIO(void){
ssize_t  mt6326_enable_VSDIO(void){

    pmic_vsdio_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VSDIO  = {
	.attr = {
		.name = "mt6326_enable_VSDIO",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VSDIO,
};
#endif

//static ssize_t  mt6326_disable_VSDIO(void){
ssize_t  mt6326_disable_VSDIO(void){

    pmic_vsdio_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VSDIO  = {
	.attr = {
		.name = "mt6326_disable_VSDIO",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VSDIO,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_enable_VCORE_2(void){
ssize_t  mt6326_enable_VCORE_2(void){

    pmic_vcore2_on_sel(VCORE2_ENABLE_WITH_VCORE2_EN);
    pmic_vcore2_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VCORE_2  = {
	.attr = {
		.name = "mt6326_enable_VCORE_2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VCORE_2,
};
#endif

//static ssize_t  mt6326_disable_VCORE_2(void){
ssize_t  mt6326_disable_VCORE_2(void){

    pmic_vcore2_on_sel(VCORE2_ENABLE_WITH_VCORE2_EN);
    pmic_vcore2_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VCORE_2  = {
	.attr = {
		.name = "mt6326_disable_VCORE_2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VCORE_2,
};
#endif

#if 0
static ssize_t  mt6326_0x2F_read(void){
    mt6326_read_byte(0x2F, &pmic6326_reg[0x2F]);
    printk("cmd 0x%x : 0x%x\n", 0x2F, pmic6326_reg[0x2F]);
    return 0;
}

static struct bin_attribute mt6326_attr_0x2F_read = {
	.attr = {
		.name = "mt6326_0x2F_read",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_0x2F_read,
};
#endif

/***********************************************************/

ssize_t  mt6326_enable_VPA(void){
    
    pmic_vpa_enable(KAL_TRUE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_enable_VPA  = {
	.attr = {
		.name = "mt6326_enable_VPA",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_enable_VPA,
};
#endif

ssize_t  mt6326_disable_VPA(void){

    pmic_vpa_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_VPA  = {
	.attr = {
		.name = "mt6326_disable_VPA",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_VPA,
};
#endif


//static ssize_t  mt6326_V3GTX_set_2_5(void){
ssize_t  mt6326_V3GTX_set_2_5(void){

    pmic_v3gtx_cal(V3GTX_CAL);
    pmic_v3gtx_sel(V3GTX_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GTX_set_2_5  = {
	.attr = {
		.name = "mt6326_V3GTX_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GTX_set_2_5,
};
#endif

//static ssize_t  mt6326_V3GTX_set_2_8(void){
ssize_t  mt6326_V3GTX_set_2_8(void){

    pmic_v3gtx_cal(V3GTX_CAL);
    pmic_v3gtx_sel(V3GTX_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GTX_set_2_8  = {
	.attr = {
		.name = "mt6326_V3GTX_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GTX_set_2_8,
};
#endif

//static ssize_t  mt6326_V3GTX_set_3_0(void){
ssize_t  mt6326_V3GTX_set_3_0(void){

    pmic_v3gtx_cal(V3GTX_CAL);
    pmic_v3gtx_sel(V3GTX_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GTX_set_3_0  = {
	.attr = {
		.name = "mt6326_V3GTX_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GTX_set_3_0,
};
#endif

//static ssize_t  mt6326_V3GTX_set_3_3(void){
ssize_t  mt6326_V3GTX_set_3_3(void){

    pmic_v3gtx_cal(V3GTX_CAL);
    pmic_v3gtx_sel(V3GTX_3_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GTX_set_3_3  = {
	.attr = {
		.name = "mt6326_V3GTX_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GTX_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_V3GRX_set_2_5(void){
ssize_t  mt6326_V3GRX_set_2_5(void){

    pmic_v3grx_cal(V3GRX_CAL);
    pmic_v3grx_sel(V3GRX_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GRX_set_2_5  = {
	.attr = {
		.name = "mt6326_V3GRX_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GRX_set_2_5,
};
#endif

//static ssize_t  mt6326_V3GRX_set_2_8(void){
ssize_t  mt6326_V3GRX_set_2_8(void){

    pmic_v3grx_cal(V3GRX_CAL);
    pmic_v3grx_sel(V3GRX_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GRX_set_2_8  = {
	.attr = {
		.name = "mt6326_V3GRX_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GRX_set_2_8,
};
#endif

//static ssize_t  mt6326_V3GRX_set_3_0(void){
ssize_t  mt6326_V3GRX_set_3_0(void){

    pmic_v3grx_cal(V3GRX_CAL);
    pmic_v3grx_sel(V3GRX_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GRX_set_3_0  = {
	.attr = {
		.name = "mt6326_V3GRX_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GRX_set_3_0,
};
#endif

//static ssize_t  mt6326_V3GRX_set_3_3(void){
ssize_t  mt6326_V3GRX_set_3_3(void){

    pmic_v3grx_cal(V3GRX_CAL);
    pmic_v3grx_sel(V3GRX_3_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_V3GRX_set_3_3  = {
	.attr = {
		.name = "mt6326_V3GRX_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_V3GRX_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VCAM_A_set_2_8(void){
ssize_t  mt6326_VCAM_A_set_2_8(void){

    pmic_vcama_sel(VCAMA_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAM_A_set_2_8  = {
	.attr = {
		.name = "mt6326_VCAM_A_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAM_A_set_2_8,
};
#endif

//static ssize_t  mt6326_VCAM_A_set_2_5(void){
ssize_t  mt6326_VCAM_A_set_2_5(void){

    pmic_vcama_sel(VCAMA_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAM_A_set_2_5  = {
	.attr = {
		.name = "mt6326_VCAM_A_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAM_A_set_2_5,
};
#endif

//static ssize_t  mt6326_VCAM_A_set_1_8(void){
ssize_t  mt6326_VCAM_A_set_1_8(void){

    pmic_vcama_sel(VCAMA_1_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAM_A_set_1_8  = {
	.attr = {
		.name = "mt6326_VCAM_A_set_1_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAM_A_set_1_8,
};
#endif

//static ssize_t  mt6326_VCAM_A_set_1_5(void){
ssize_t  mt6326_VCAM_A_set_1_5(void){

    pmic_vcama_sel(VCAMA_1_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAM_A_set_1_5  = {
	.attr = {
		.name = "mt6326_VCAM_A_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAM_A_set_1_5,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VWIFI3V3_set_2_5(void){
ssize_t  mt6326_VWIFI3V3_set_2_5(void){

    pmic_vwifi3v3_cal(VWIFI3V3_CAL);
    pmic_vwifi3v3_sel(VWIFI3V3_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI3V3_set_2_5  = {
	.attr = {
		.name = "mt6326_VWIFI3V3_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI3V3_set_2_5,
};
#endif

//static ssize_t  mt6326_VWIFI3V3_set_2_8(void){
ssize_t  mt6326_VWIFI3V3_set_2_8(void){

    pmic_vwifi3v3_cal(VWIFI3V3_CAL);
    pmic_vwifi3v3_sel(VWIFI3V3_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI3V3_set_2_8  = {
	.attr = {
		.name = "mt6326_VWIFI3V3_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI3V3_set_2_8,
};
#endif

//static ssize_t  mt6326_VWIFI3V3_set_3_0(void){
ssize_t  mt6326_VWIFI3V3_set_3_0(void){

    pmic_vwifi3v3_cal(VWIFI3V3_CAL);
    pmic_vwifi3v3_sel(VWIFI3V3_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI3V3_set_3_0  = {
	.attr = {
		.name = "mt6326_VWIFI3V3_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI3V3_set_3_0,
};
#endif

//static ssize_t  mt6326_VWIFI3V3_set_3_3(void){
ssize_t  mt6326_VWIFI3V3_set_3_3(void){

    pmic_vwifi3v3_cal(VWIFI3V3_CAL);
    pmic_vwifi3v3_sel(VWIFI3V3_3_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI3V3_set_3_3  = {
	.attr = {
		.name = "mt6326_VWIFI3V3_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI3V3_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VWIFI2V8_set_2_5(void){
ssize_t  mt6326_VWIFI2V8_set_2_5(void){

    pmic_vwifi2v8_cal(VWIFI2V8_CAL);
    pmic_vwifi2v8_sel(VWIFI2V8_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI2V8_set_2_5  = {
	.attr = {
		.name = "mt6326_VWIFI2V8_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI2V8_set_2_5,
};
#endif

//static ssize_t  mt6326_VWIFI2V8_set_2_8(void){
ssize_t  mt6326_VWIFI2V8_set_2_8(void){

    pmic_vwifi2v8_cal(VWIFI2V8_CAL);
    pmic_vwifi2v8_sel(VWIFI2V8_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI2V8_set_2_8  = {
	.attr = {
		.name = "mt6326_VWIFI2V8_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI2V8_set_2_8,
};
#endif

//static ssize_t  mt6326_VWIFI2V8_set_3_0(void){
ssize_t  mt6326_VWIFI2V8_set_3_0(void){

    pmic_vwifi2v8_cal(VWIFI2V8_CAL);
    pmic_vwifi2v8_sel(VWIFI2V8_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI2V8_set_3_0  = {
	.attr = {
		.name = "mt6326_VWIFI2V8_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI2V8_set_3_0,
};
#endif

//static ssize_t  mt6326_VWIFI2V8_set_3_3(void){
ssize_t  mt6326_VWIFI2V8_set_3_3(void){

    pmic_vwifi2v8_cal(VWIFI2V8_CAL);
    pmic_vwifi2v8_sel(VWIFI2V8_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VWIFI2V8_set_3_3  = {
	.attr = {
		.name = "mt6326_VWIFI2V8_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VWIFI2V8_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VSIM_set_1_3(void){
ssize_t  mt6326_VSIM_set_1_3(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_1_3);
    pmic_vsim_sel(VSIM_1_3V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_1_3  = {
	.attr = {
		.name = "mt6326_VSIM_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_1_3,
};
#endif

//static ssize_t  mt6326_VSIM_set_1_5(void){
ssize_t  mt6326_VSIM_set_1_5(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_1_5);
    pmic_vsim_sel(VSIM_1_5V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_1_5  = {
	.attr = {
		.name = "mt6326_VSIM_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_1_5,
};
#endif

//static ssize_t  mt6326_VSIM_set_1_8(void){
ssize_t  mt6326_VSIM_set_1_8(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_1_8);
    pmic_vsim_sel(VSIM_1_8V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_1_8  = {
	.attr = {
		.name = "mt6326_VSIM_set_1_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_1_8,
};
#endif

//static ssize_t  mt6326_VSIM_set_2_5(void){
ssize_t  mt6326_VSIM_set_2_5(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_2_5);
    pmic_vsim_sel(VSIM_2_5V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_2_5  = {
	.attr = {
		.name = "mt6326_VSIM_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_2_5,
};
#endif

//static ssize_t  mt6326_VSIM_set_2_8(void){
ssize_t  mt6326_VSIM_set_2_8(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_2_8);
    pmic_vsim_sel(VSIM_2_8V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_2_8  = {
	.attr = {
		.name = "mt6326_VSIM_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_2_8,
};
#endif

//static ssize_t  mt6326_VSIM_set_3_0(void){
ssize_t  mt6326_VSIM_set_3_0(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_3_0);
    pmic_vsim_sel(VSIM_3_0V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_3_0  = {
	.attr = {
		.name = "mt6326_VSIM_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_3_0,
};
#endif

//static ssize_t  mt6326_VSIM_set_3_3(void){
ssize_t  mt6326_VSIM_set_3_3(void){

    pmic_vsim_cal(VSIM_CAL);
    //pmic_vsim_sel(VSIM_3_3);
    pmic_vsim_sel(VSIM_3_3V);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSIM_set_3_3  = {
	.attr = {
		.name = "mt6326_VSIM_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSIM_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VBT_set_1_3(void){
ssize_t  mt6326_VBT_set_1_3(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_1_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_1_3  = {
	.attr = {
		.name = "mt6326_VBT_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_1_3,
};
#endif

//static ssize_t  mt6326_VBT_set_1_5(void){
ssize_t  mt6326_VBT_set_1_5(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_1_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_1_5  = {
	.attr = {
		.name = "mt6326_VBT_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_1_5,
};
#endif

//static ssize_t  mt6326_VBT_set_1_8(void){
ssize_t  mt6326_VBT_set_1_8(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_1_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_1_8  = {
	.attr = {
		.name = "mt6326_VBT_set_1_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_1_8,
};
#endif

//static ssize_t  mt6326_VBT_set_2_5(void){
ssize_t  mt6326_VBT_set_2_5(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_2_5  = {
	.attr = {
		.name = "mt6326_VBT_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_2_5,
};
#endif

//static ssize_t  mt6326_VBT_set_2_8(void){
ssize_t  mt6326_VBT_set_2_8(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_2_8  = {
	.attr = {
		.name = "mt6326_VBT_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_2_8,
};
#endif

//static ssize_t  mt6326_VBT_set_3_0(void){
ssize_t  mt6326_VBT_set_3_0(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_3_0  = {
	.attr = {
		.name = "mt6326_VBT_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_3_0,
};
#endif

//static ssize_t  mt6326_VBT_set_3_3(void){
ssize_t  mt6326_VBT_set_3_3(void){

    pmic_vbt_cal(VBT_CAL);
    pmic_vbt_sel(VBT_3_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VBT_set_3_3  = {
	.attr = {
		.name = "mt6326_VBT_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VBT_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VCAMD_set_1_3(void){
ssize_t  mt6326_VCAMD_set_1_3(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_1_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_1_3  = {
	.attr = {
		.name = "mt6326_VCAMD_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_1_3,
};
#endif

//static ssize_t  mt6326_VCAMD_set_1_5(void){
ssize_t  mt6326_VCAMD_set_1_5(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_1_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_1_5  = {
	.attr = {
		.name = "mt6326_VCAMD_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_1_5,
};
#endif

//static ssize_t  mt6326_VCAMD_set_1_8(void){
ssize_t  mt6326_VCAMD_set_1_8(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_1_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_1_8  = {
	.attr = {
		.name = "mt6326_VCAMD_set_1_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_1_8,
};
#endif

//static ssize_t  mt6326_VCAMD_set_2_5(void){
ssize_t  mt6326_VCAMD_set_2_5(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_2_5  = {
	.attr = {
		.name = "mt6326_VCAMD_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_2_5,
};
#endif

//static ssize_t  mt6326_VCAMD_set_2_8(void){
ssize_t  mt6326_VCAMD_set_2_8(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_2_8  = {
	.attr = {
		.name = "mt6326_VCAMD_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_2_8,
};
#endif

//static ssize_t  mt6326_VCAMD_set_3_0(void){
ssize_t  mt6326_VCAMD_set_3_0(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_3_0  = {
	.attr = {
		.name = "mt6326_VCAMD_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_3_0,
};
#endif

//static ssize_t  mt6326_VCAMD_set_3_3(void){
ssize_t  mt6326_VCAMD_set_3_3(void){

    pmic_vcamd_cal(VCAMD_CAL);
    pmic_vcamd_sel(VCAMD_3_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCAMD_set_3_3  = {
	.attr = {
		.name = "mt6326_VCAMD_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCAMD_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VGP_set_1_3(void){
ssize_t  mt6326_VGP_set_1_3(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_1_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_1_3  = {
	.attr = {
		.name = "mt6326_VGP_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_1_3,
};
#endif

//static ssize_t  mt6326_VGP_set_1_5(void){
ssize_t  mt6326_VGP_set_1_5(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_1_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_1_5  = {
	.attr = {
		.name = "mt6326_VGP_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_1_5,
};
#endif

//static ssize_t  mt6326_VGP_set_1_8(void){
ssize_t  mt6326_VGP_set_1_8(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_1_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_1_8  = {
	.attr = {
		.name = "mt6326_VGP_set_1_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_1_8,
};
#endif

//static ssize_t  mt6326_VGP_set_2_5(void){
ssize_t  mt6326_VGP_set_2_5(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_2_5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_2_5  = {
	.attr = {
		.name = "mt6326_VGP_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_2_5,
};
#endif

//static ssize_t  mt6326_VGP_set_2_8(void){
ssize_t  mt6326_VGP_set_2_8(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_2_8  = {
	.attr = {
		.name = "mt6326_VGP_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_2_8,
};
#endif

//static ssize_t  mt6326_VGP_set_3_0(void){
ssize_t  mt6326_VGP_set_3_0(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_3_0  = {
	.attr = {
		.name = "mt6326_VGP_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_3_0,
};
#endif

//static ssize_t  mt6326_VGP_set_3_3(void){
ssize_t  mt6326_VGP_set_3_3(void){

    pmic_vgp_cal(VGP_CAL);
    pmic_vgp_sel(VGP_3_3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP_set_3_3  = {
	.attr = {
		.name = "mt6326_VGP_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VGP2_set_1_3(void){
ssize_t  mt6326_VGP2_set_1_3(void){

   pmic_vgp2_sel(VGP2_1_3);
   pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_1_3  = {
	.attr = {
		.name = "mt6326_VGP2_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_1_3,
};
#endif

//static ssize_t  mt6326_VGP2_set_1_5(void){
ssize_t  mt6326_VGP2_set_1_5(void){

    pmic_vgp2_sel(VGP2_1_5);
    pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_1_5  = {
	.attr = {
		.name = "mt6326_VGP2_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_1_5,
};
#endif

//static ssize_t  mt6326_VGP2_set_1_8(void){
ssize_t  mt6326_VGP2_set_1_8(void){

    pmic_vgp2_sel(VGP2_1_8);
    pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_1_8  = {
	.attr = {
		.name = "mt6326_VGP2_set_1_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_1_8,
};
#endif

//static ssize_t  mt6326_VGP2_set_2_5(void){
ssize_t  mt6326_VGP2_set_2_5(void){

    pmic_vgp2_sel(VGP2_2_5);
    pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_2_5  = {
	.attr = {
		.name = "mt6326_VGP2_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_2_5,
};
#endif

//static ssize_t  mt6326_VGP2_set_2_8(void){
ssize_t  mt6326_VGP2_set_2_8(void){

    pmic_vgp2_sel(VGP2_2_8);
    pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_2_8  = {
	.attr = {
		.name = "mt6326_VGP2_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_2_8,
};
#endif

//static ssize_t  mt6326_VGP2_set_3_0(void){
ssize_t  mt6326_VGP2_set_3_0(void){

    pmic_vgp2_sel(VGP2_3_0);
    pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_3_0  = {
	.attr = {
		.name = "mt6326_VGP2_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_3_0,
};
#endif

//static ssize_t  mt6326_VGP2_set_3_3(void){
ssize_t  mt6326_VGP2_set_3_3(void){

    pmic_vgp2_sel(VGP2_3_3);
    pmic_vgp2_on_sel(VGP2_ON_SEL);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VGP2_set_3_3  = {
	.attr = {
		.name = "mt6326_VGP2_set_3_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VGP2_set_3_3,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VSDIO_set_2_8(void){
ssize_t  mt6326_VSDIO_set_2_8(void){

    pmic_vsdio_cal(VSDIO_CAL);
    pmic_vsdio_sel(VSDIO_2_8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSDIO_set_2_8  = {
	.attr = {
		.name = "mt6326_VSDIO_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSDIO_set_2_8,
};
#endif

//static ssize_t  mt6326_VSDIO_set_3_0(void){
ssize_t  mt6326_VSDIO_set_3_0(void){

    pmic_vsdio_cal(VSDIO_CAL);
    pmic_vsdio_sel(VSDIO_3_0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VSDIO_set_3_0  = {
	.attr = {
		.name = "mt6326_VSDIO_set_3_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VSDIO_set_3_0,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_Dump_Register(void){
ssize_t  mt6326_Dump_Register(void){

    int i = 0;

     for (i=0;i<PMIC_REG_NUM;i++){
        pmic6326_reg[i] = 0;        
    }

    printk("mt6326_Dump_Register !!\n "); 
    for (i=0;i<PMIC_REG_NUM;i++){
        // We skip intr state read back
        // If there is intr asserted, after enable EINT intr, pmic hisr will handle the intr
        //if ( (i!=0x0B) && (i!=0x0C) && (i!=0x0D) && (i!=0x0E) ){
            mt6326_read_byte(i, &pmic6326_reg[i]);
            printk("0x%X,0x%X\n", i, pmic6326_reg[i]);
        //}
    }
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_Dump_Register  = {
	.attr = {
		.name = "mt6326_Dump_Register",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_Dump_Register,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_VCORE_1_set_0_80(void){
ssize_t  mt6326_VCORE_1_set_0_80(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(0); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_0_80  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_0_80",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_0_80,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_0_85(void){
ssize_t  mt6326_VCORE_1_set_0_85(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(2); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_0_85  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_0_85",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_0_85,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_0_90(void){
ssize_t  mt6326_VCORE_1_set_0_90(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(4); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_0_90  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_0_90",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_0_90,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_0_95(void){
ssize_t  mt6326_VCORE_1_set_0_95(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(6); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_0_95  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_0_95",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_0_95,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_0(void){
ssize_t  mt6326_VCORE_1_set_1_0(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(8); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_0  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_0,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_05(void){
ssize_t  mt6326_VCORE_1_set_1_05(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(10); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    //printk("pmic_init : update bit : 0 -> 1 !!\n ");
    //printk("1.05-");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_05  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_05",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_05,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_10(void){
ssize_t  mt6326_VCORE_1_set_1_10(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(12); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    //printk("pmic_init : update bit : 0 -> 1 !!\n ");
    //printk("1.0-");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_10  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_10",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_10,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_15(void){
ssize_t  mt6326_VCORE_1_set_1_15(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(14); 
    pmic_vcore1_sleep_1_eco3(1);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_15  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_15",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_15,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_2(void){
ssize_t  mt6326_VCORE_1_set_1_2(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(0); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_2  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_2,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_25(void){
ssize_t  mt6326_VCORE_1_set_1_25(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(2); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_25  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_25",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_25,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_3(void){
ssize_t  mt6326_VCORE_1_set_1_3(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(4); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    //printk("pmic_init : update bit : 0 -> 1 !!\n ");
    //printk("1.3-");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_3  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_3,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_35(void){
ssize_t  mt6326_VCORE_1_set_1_35(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(6); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_35  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_35",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_35,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_4(void){
ssize_t  mt6326_VCORE_1_set_1_4(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(8); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_4  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_4",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_4,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_45(void){
ssize_t  mt6326_VCORE_1_set_1_45(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(10); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_45  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_45",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_45,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_5(void){
ssize_t  mt6326_VCORE_1_set_1_5(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(12); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_5  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_5,
};
#endif

//static ssize_t  mt6326_VCORE_1_set_1_55(void){
ssize_t  mt6326_VCORE_1_set_1_55(void){

    pmic_vcore1_dvfs_step_inc(1);
    pmic_vcore1_dvfs_1_eco3(14); 
    pmic_vcore1_sleep_1_eco3(0);
    //pmic_vcore1_dvfs_0_eco3(0);
    //pmic_vcore1_sleep_0_eco3(0);
    pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore1_dvfs_target_update(KAL_FALSE);
    pmic_vcore1_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_1_set_1_55  = {
	.attr = {
		.name = "mt6326_VCORE_1_set_1_55",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_1_set_1_55,
};
#endif

/***********************************************************/

/***********************************************************/

//static ssize_t  mt6326_VCORE_2_set_0_80(void){
ssize_t  mt6326_VCORE_2_set_0_80(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(0); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_0_80  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_0_80",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_0_80,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_0_85(void){
ssize_t  mt6326_VCORE_2_set_0_85(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(2); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_0_85  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_0_85",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_0_85,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_0_90(void){
ssize_t  mt6326_VCORE_2_set_0_90(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(4); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_0_90  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_0_90",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_0_90,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_0_95(void){
ssize_t  mt6326_VCORE_2_set_0_95(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(6); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_0_95  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_0_95",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_0_95,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_0(void){
ssize_t  mt6326_VCORE_2_set_1_0(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(8); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_0  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_0,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_05(void){
ssize_t  mt6326_VCORE_2_set_1_05(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(10); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_05  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_05",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_05,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_10(void){
ssize_t  mt6326_VCORE_2_set_1_10(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(12); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_10  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_10",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_10,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_15(void){
ssize_t  mt6326_VCORE_2_set_1_15(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(14); 
    pmic_vcore2_sleep_1_eco3(1);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_15  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_15",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_15,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_2(void){
ssize_t  mt6326_VCORE_2_set_1_2(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(0); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_2  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_2,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_25(void){
ssize_t  mt6326_VCORE_2_set_1_25(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(2); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_25  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_25",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_25,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_3(void){
ssize_t  mt6326_VCORE_2_set_1_3(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(4); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_3  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_3,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_35(void){
ssize_t  mt6326_VCORE_2_set_1_35(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(6); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_35  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_35",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_35,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_4(void){
ssize_t  mt6326_VCORE_2_set_1_4(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(8); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_4  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_4",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_4,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_45(void){
ssize_t  mt6326_VCORE_2_set_1_45(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(10); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_45  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_45",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_45,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_5(void){
ssize_t  mt6326_VCORE_2_set_1_5(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(12); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_5  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_5,
};
#endif

//static ssize_t  mt6326_VCORE_2_set_1_55(void){
ssize_t  mt6326_VCORE_2_set_1_55(void){

    pmic_vcore2_dvfs_step_inc(1);
    pmic_vcore2_dvfs_1_eco3(14); 
    pmic_vcore2_sleep_1_eco3(0);
    //pmic_vcore2_dvfs_0_eco3(0);
    //pmic_vcore2_sleep_0_eco3(0);
    pmic_vcore2_dvfs_ramp_enable(KAL_TRUE);
    /* update bit : 0 -> 1 */
    pmic_vcore2_dvfs_target_update(KAL_FALSE);
    pmic_vcore2_dvfs_target_update(KAL_TRUE);
    printk("pmic_init : update bit : 0 -> 1 !!\n ");
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VCORE_2_set_1_55  = {
	.attr = {
		.name = "mt6326_VCORE_2_set_1_55",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VCORE_2_set_1_55,
};
#endif

/***********************************************************/

ssize_t  mt6326_VPA_set_1_075(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(0);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_075  = {
	.attr = {
		.name = "mt6326_VPA_set_1_075",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_075,
};
#endif

ssize_t  mt6326_VPA_set_1_15(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(1);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_15  = {
	.attr = {
		.name = "mt6326_VPA_set_1_15",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_15,
};
#endif

ssize_t  mt6326_VPA_set_1_225(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(2);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_225  = {
	.attr = {
		.name = "mt6326_VPA_set_1_225",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_225,
};
#endif

ssize_t  mt6326_VPA_set_1_3(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(3);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_3  = {
	.attr = {
		.name = "mt6326_VPA_set_1_3",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_3,
};
#endif

ssize_t  mt6326_VPA_set_1_375(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(4);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_375  = {
	.attr = {
		.name = "mt6326_VPA_set_1_375",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_375,
};
#endif

ssize_t  mt6326_VPA_set_1_45(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(5);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_45  = {
	.attr = {
		.name = "mt6326_VPA_set_1_45",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_45,
};
#endif

ssize_t  mt6326_VPA_set_1_525(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(6);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_525  = {
	.attr = {
		.name = "mt6326_VPA_set_1_525",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_525,
};
#endif

ssize_t  mt6326_VPA_set_1_6(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(7);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_6  = {
	.attr = {
		.name = "mt6326_VPA_set_1_6",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_6,
};
#endif

ssize_t  mt6326_VPA_set_1_675(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(8);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_675  = {
	.attr = {
		.name = "mt6326_VPA_set_1_675",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_675,
};
#endif

ssize_t  mt6326_VPA_set_1_75(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(9);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_75  = {
	.attr = {
		.name = "mt6326_VPA_set_1_75",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_75,
};
#endif

ssize_t  mt6326_VPA_set_1_825(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(10);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_825  = {
	.attr = {
		.name = "mt6326_VPA_set_1_825",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_825,
};
#endif

ssize_t  mt6326_VPA_set_1_9(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(11);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_9  = {
	.attr = {
		.name = "mt6326_VPA_set_1_9",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_9,
};
#endif

ssize_t  mt6326_VPA_set_1_975(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(12);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_1_975  = {
	.attr = {
		.name = "mt6326_VPA_set_1_975",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_1_975,
};
#endif

ssize_t  mt6326_VPA_set_2_05(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(13);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_05  = {
	.attr = {
		.name = "mt6326_VPA_set_2_05",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_05,
};
#endif

ssize_t  mt6326_VPA_set_2_125(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(14);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_125  = {
	.attr = {
		.name = "mt6326_VPA_set_2_125",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_125,
};
#endif

ssize_t  mt6326_VPA_set_2_2(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(15);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_2  = {
	.attr = {
		.name = "mt6326_VPA_set_2_2",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_2,
};
#endif

ssize_t  mt6326_VPA_set_2_275(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(16);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_275  = {
	.attr = {
		.name = "mt6326_VPA_set_2_275",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_275,
};
#endif

ssize_t  mt6326_VPA_set_2_35(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(17);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_35  = {
	.attr = {
		.name = "mt6326_VPA_set_2_35",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_35,
};
#endif

ssize_t  mt6326_VPA_set_2_425(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(18);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_425  = {
	.attr = {
		.name = "mt6326_VPA_set_2_425",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_425,
};
#endif

ssize_t  mt6326_VPA_set_2_5(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(19);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_5  = {
	.attr = {
		.name = "mt6326_VPA_set_2_5",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_5,
};
#endif

ssize_t  mt6326_VPA_set_2_575(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(20);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_575  = {
	.attr = {
		.name = "mt6326_VPA_set_2_575",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_575,
};
#endif

ssize_t  mt6326_VPA_set_2_65(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(21);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_65  = {
	.attr = {
		.name = "mt6326_VPA_set_2_65",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_65,
};
#endif

ssize_t  mt6326_VPA_set_2_725(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(22);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_725  = {
	.attr = {
		.name = "mt6326_VPA_set_2_725",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_725,
};
#endif

ssize_t  mt6326_VPA_set_2_8(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(23);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_8  = {
	.attr = {
		.name = "mt6326_VPA_set_2_8",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_8,
};
#endif

ssize_t  mt6326_VPA_set_2_875(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(24);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_875  = {
	.attr = {
		.name = "mt6326_VPA_set_2_875",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_875,
};
#endif

ssize_t  mt6326_VPA_set_2_95(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(25);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_2_95  = {
	.attr = {
		.name = "mt6326_VPA_set_2_95",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_2_95,
};
#endif

ssize_t  mt6326_VPA_set_3_025(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(26);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_3_025  = {
	.attr = {
		.name = "mt6326_VPA_set_3_025",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_3_025,
};
#endif

ssize_t  mt6326_VPA_set_3_1(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(27);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_3_1  = {
	.attr = {
		.name = "mt6326_VPA_set_3_1",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_3_1,
};
#endif

ssize_t  mt6326_VPA_set_3_175(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(28);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_3_175  = {
	.attr = {
		.name = "mt6326_VPA_set_3_175",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_3_175,
};
#endif

ssize_t  mt6326_VPA_set_3_25(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(29);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_3_25  = {
	.attr = {
		.name = "mt6326_VPA_set_3_25",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_3_25,
};
#endif

ssize_t  mt6326_VPA_set_3_325(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(30);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_3_325  = {
	.attr = {
		.name = "mt6326_VPA_set_3_325",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_3_325,
};
#endif

ssize_t  mt6326_VPA_set_3_4(void){

    pmic_vpa_oc_tune(0);
    pmic_vpa_tunel(31);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_VPA_set_3_4  = {
	.attr = {
		.name = "mt6326_VPA_set_3_4",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_VPA_set_3_4,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_en_charg_450(void){
ssize_t  mt6326_en_charg_450(void){

    /* 1. Set CC mode charge current */
    pmic_chr_current(CHR_CURRENT_450MA);

    /* 2. Enable watchdog timer */
    pmic_wdt_enable(KAL_TRUE);

    /* 3. Set watchdog timeout period */
    pmic_wdt_timeout(WDT_TIMEOUT_32_SEC);    

    /* 4. Enable charger */
    pmic_chr_chr_enable(KAL_TRUE);

    /* 5. Enable VBOUT pin */
    pmic_adc_vbat_enable(KAL_TRUE);

    /* 6. Enable ISENSE_OUT pin */
    pmic_adc_isense_enable(KAL_TRUE);

    printk("mt6326_enable_charging_450mA done !!\n ");
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_en_charg_450  = {
	.attr = {
		.name = "mt6326_en_charg_450",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_en_charg_450,
};
#endif

/***********************************************************/

ssize_t  mt6326_disable_charg(void){

    pmic_chr_chr_enable(KAL_FALSE);

    printk("mt6326_disable_charging done !!\n ");
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_disable_charg  = {
	.attr = {
		.name = "mt6326_disable_charg",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_disable_charg,
};
#endif

/***********************************************************/

//static ssize_t  mt6326_kick_charger_wdt(void){
ssize_t  mt6326_kick_charger_wdt(void){

    pmic6326_kick_charger_wdt();
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_kick_charger_wdt  = {
	.attr = {
		.name = "mt6326_kick_charger_wdt",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_kick_charger_wdt,
};
#endif

/***********************************************************/

ssize_t  mt6326_FL_Enable(void){
	
	pmic_vboost1_tune(VBOOST1_VOL_4_25_V);
	pmic_boost1_enable(KAL_TRUE);
	pmic_flash_dim_div(0);
	//pmic_flash_dim_duty(30);
	pmic_flash_enable(KAL_TRUE);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_FL_Enable  = {
	.attr = {
		.name = "mt6326_FL_Enable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_FL_Enable,
};
#endif

ssize_t  mt6326_FL_Disable(void){

	pmic_flash_enable(KAL_FALSE);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_FL_Disable  = {
	.attr = {
		.name = "mt6326_FL_Disable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_FL_Disable,
};
#endif

ssize_t  pmic_FL_dim_duty(kal_uint8 duty){

	if(duty > 32)	duty = 31;
	//if(duty < 0)	duty = 0;	

	pmic_flash_dim_duty(duty);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_FL_dim_duty  = {
	.attr = {
		.name = "pmic_FL_dim_duty",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = pmic_FL_dim_duty,
};
#endif


/***********************************************************/

ssize_t  mt6326_bl_Enable(void){

    pmic_boost2_enable(KAL_TRUE);    
    pmic_bl_enable(KAL_TRUE);        
    pmic_vbus_enable(KAL_TRUE);  
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_bl_Enable  = {
	.attr = {
		.name = "mt6326_bl_Enable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_bl_Enable,
};
#endif

ssize_t  mt6326_bl_Disable(void){

    pmic_boost2_enable(KAL_FALSE);    
    pmic_bl_enable(KAL_FALSE);        
    pmic_vbus_enable(KAL_FALSE);  
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_bl_Disable  = {
	.attr = {
		.name = "mt6326_bl_Disable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_bl_Disable,
};
#endif

ssize_t  mt6326_bl_dim_duty_Full(void){

    pmic_bl_dim_duty(31);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_bl_dim_duty_Full  = {
	.attr = {
		.name = "mt6326_bl_dim_duty_Full",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_bl_dim_duty_Full,
};
#endif

ssize_t  mt6326_bl_dim_duty_0(void){

    pmic_bl_dim_duty(0);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_bl_dim_duty_0  = {
	.attr = {
		.name = "mt6326_bl_dim_duty_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_bl_dim_duty_0,
};
#endif

/***********************************************************/

ssize_t  mt6326_kpled_Enable(void){
   
    pmic_kp_enable(KAL_TRUE);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_kpled_Enable  = {
	.attr = {
		.name = "mt6326_kpled_Enable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_kpled_Enable,
};
#endif

ssize_t  mt6326_kpled_Disable(void){

    pmic_kp_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_kpled_Disable  = {
	.attr = {
		.name = "mt6326_kpled_Disable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_kpled_Disable,
};
#endif

ssize_t  mt6326_kpled_dim_duty_Full(void){

    pmic_kp_dim_duty(PMIC_DEFAULT_KP_DIM);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_kpled_dim_duty_Full  = {
	.attr = {
		.name = "mt6326_kpled_dim_duty_Full",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_kpled_dim_duty_Full,
};
#endif

ssize_t  mt6326_kpled_dim_duty_0(void){

    pmic_kp_dim_duty(0);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_kpled_dim_duty_0  = {
	.attr = {
		.name = "mt6326_kpled_dim_duty_0",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_kpled_dim_duty_0,
};
#endif

/***********************************************************/

ssize_t  mt6326_vibr_Enable(void){
   
    pmic_vibr_dim_duty(DEFAULT_VIBR_DUTY);
    pmic_vibr_enable(KAL_TRUE);
        
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_vibr_Enable  = {
	.attr = {
		.name = "mt6326_vibr_Enable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_vibr_Enable,
};
#endif

ssize_t  mt6326_vibr_Disable(void){

    pmic_vibr_enable(KAL_FALSE);
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_vibr_Disable  = {
	.attr = {
		.name = "mt6326_vibr_Disable",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_vibr_Disable,
};
#endif

ssize_t  mt6326_check_power(void){

    kal_uint8 pmic6326_reg_4 = 0;
    kal_uint8 pmic6326_reg_5 = 0;
    kal_uint8 pmic6326_reg_6 = 0;
    kal_uint8 pmic6326_reg_7 = 0;
    kal_uint8 pmic6326_reg_8 = 0;
    kal_uint8 pmic6326_reg_1A = 0;
	kal_uint8 pmic6326_reg_ALL = 0;
	int index = 0;

    mt6326_read_byte(4, &pmic6326_reg_4);
    mt6326_read_byte(5, &pmic6326_reg_5);
    mt6326_read_byte(6, &pmic6326_reg_6);
    mt6326_read_byte(7, &pmic6326_reg_7);
    mt6326_read_byte(8, &pmic6326_reg_8);
    mt6326_read_byte(0x1A, &pmic6326_reg_1A);

    printk("In mt6326_check_power *******\n");
    //printk("0x%X,0x%X\n", 4, pmic6326_reg_4);
    //printk("0x%X,0x%X\n", 5, pmic6326_reg_5);
    //printk("0x%X,0x%X\n", 6, pmic6326_reg_6);
    //printk("0x%X,0x%X\n", 7, pmic6326_reg_7);
    //printk("0x%X,0x%X\n", 8, pmic6326_reg_8);
    
    printk("******* CHeck BUCK *******\n");
    printk("VCORE 1 : %d\n", (pmic6326_reg_7>>7)&(0x01));
    printk("VCORE 2 : %d\n", (pmic6326_reg_8>>3)&(0x01));
    printk("VPA : %d\n", (pmic6326_reg_8>>5)&(0x01));    
    printk("VM : %d\n", (pmic6326_reg_8>>1)&(0x01));
    
    printk("******* Check Analog LDO *******\n");
    printk("V3GTX : %d\n", (pmic6326_reg_4>>4)&(0x01));    
    printk("V3GRX : %d\n", (pmic6326_reg_4>>6)&(0x01));
    printk("VRF : %d\n", (pmic6326_reg_4>>0)&(0x01));    
    printk("VTCXO : %d\n", (pmic6326_reg_4>>2)&(0x01));
    printk("VA : %d\n", (pmic6326_reg_5>>0)&(0x01));
    printk("VCAMA : %d\n", (pmic6326_reg_5>>5)&(0x01));
    printk("VWIFI3V3 : %d\n", (pmic6326_reg_5>>7)&(0x01));
    printk("VWIFI2V8 : %d\n", (pmic6326_reg_6>>1)&(0x01));

    printk("******* Check Digital LDO *******\n");
    printk("VIO : %d\n", (pmic6326_reg_5>>2)&(0x01));
    printk("VSIM : %d\n", (pmic6326_reg_6>>3)&(0x01));
    printk("VUSB : %d\n", (pmic6326_reg_6>>7)&(0x01));
    printk("VBT : %d\n", (pmic6326_reg_6>>5)&(0x01));    
    printk("VCAMD : %d\n", (pmic6326_reg_7>>1)&(0x01));
    printk("VSDIO : %d\n", (pmic6326_reg_7>>5)&(0x01));
    printk("VGP : %d\n", (pmic6326_reg_7>>3)&(0x01));
    printk("VGP2 : %d\n", (pmic6326_reg_1A>>2)&(0x01));
    
    printk("VRTC : %d\n", (pmic6326_reg_5>>4)&(0x01));

	for (index = 0 ; index < 0x97 ; index++)
	{
		mt6326_read_byte(index, &pmic6326_reg_ALL);
		printk("reg%x: %x\n", index ,pmic6326_reg_ALL);
	}
    
    return 0;
}

#ifdef CONFIG_TESTCASE_MSG
static struct bin_attribute mt6326_attr_check_power  = {
	.attr = {
		.name = "mt6326_check_power",
		.mode = S_IRUGO,
		.owner = THIS_MODULE,
	},
	.read = mt6326_check_power,
};
#endif

/***********************************************************/

void  mt6326_test_con(void){
    printk("mt6326_test_con !!\n ");
}

#if defined(CONFIG_MTK_SMART_BATTERY)
void mt6326_pmic_eint_irq(void)
{
	wake_lock(&battery_suspend_lock);	

	#if defined(CONFIG_MTK_SMART_BATTERY)
    wake_up_bat ();
    #endif

	g_chr_event = 1;	
  
    return ;
}
#endif

void Yusu_Sound_AMP_Switch(kal_bool enAMP)
{
    #ifdef CONFIG_TESTCASE_MSG
    printk("EXTERNAL INTERRUPT: Yusu_Sound_AMP_Switch !!\n");
    #endif  
    pmic_spkl_enable(enAMP);
    pmic_spkr_enable(enAMP);
}

kal_bool PMIC_Ready=KAL_FALSE;

kal_bool Check_PMIC_Ready(void)
{
	return PMIC_Ready;
}

/* reg,value,mask,shift */
void pmic_config_interface(kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT){
    pmic6326_reg[RegNum] &= ~(MASK << SHIFT);
    pmic6326_reg[RegNum] |= (val << SHIFT);
	mt6326_write_byte(RegNum, pmic6326_reg[RegNum]);
}

void SPK_Dead_Time_Calibration(void)
{
	pmic_spkr_dmode(2);									// set FB auto calibration dead time mode
	pmic_spkr_dtcal(0);									// enable audio class-D dead time calibration
	mdelay(2);
	pmic_spkr_dtcal(1);									// disable audio class-D dead time calibration
	
	pmic_spkl_dmode(2);									// set FB auto calibration dead time mode
	pmic_spkl_dtcal(0);									// enable audio class-D dead time calibration
	mdelay(2);
	pmic_spkl_dtcal(1);									// disable audio class-D dead time calibration
}

void pmic_driver_init(void)
{
    /* Get PMIC6326 ECO version */
    kal_uint16 eco_version = 0;
    kal_uint8 tmp8;
    kal_bool result_tmp;
    kal_uint32 i;    
	
    /* First read will error, todo debug */
    result_tmp = mt6326_read_byte(CID_1_REG_INDEX, &tmp8);
    
    /* Low part of CID */
    result_tmp = mt6326_read_byte(CID_1_REG_INDEX, &tmp8);
    eco_version |= tmp8;

    /* High part of CID */
    result_tmp = mt6326_read_byte(CID_2_REG_INDEX, &tmp8);
    eco_version |= (tmp8 << 8);
    if (eco_version == PMIC6326_E1_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_1_VERSION;
        printk("pmic_init :  PMIC6326_ECO_1_VERSION !!\n ");
    }
    else if (eco_version == PMIC6326_E2_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_2_VERSION;
        printk("pmic_init :  PMIC6326_ECO_2_VERSION !!\n ");
    }
    else if (eco_version == PMIC6326_E3_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_3_VERSION;
        printk("pmic_init :  PMIC6326_ECO_3_VERSION !!\n ");
    }
    else if (eco_version == PMIC6326_E4_CID_CODE)
    {
        pmic6326_eco_version = PMIC6326_ECO_4_VERSION;
        printk("pmic_init :  PMIC6326_ECO_4_VERSION !!\n ");
    }
    else
    {
        printk("pmic_init :  UNKNOWN VERSION !!\n ");
    }

    #ifdef CONFIG_TESTCASE_MSG
	printk("pmic_init : before setting !!\n ");
    for (i=0;i<PMIC_REG_NUM;i++){
            mt6326_read_byte(i, &pmic6326_reg[i]);
            printk("0x%X,0x%X\n", i, pmic6326_reg[i]);        
    }
    #endif

	/* old basic setting */
#if 1
	//printk("pmic_init : pmic_int_ctrl_disable !!\n ");
	pmic_int_ctrl_1_enable(INT1_EN_ALL, KAL_FALSE);
    pmic_int_ctrl_2_enable(INT2_EN_ALL, KAL_FALSE);	
    pmic_int_ctrl_3_enable(INT3_EN_ALL, KAL_FALSE);
    pmic_int_ctrl_3_enable(INT_EN_CHRDET, KAL_TRUE);

	//printk("pmic_init :  pmic_vcore1 !!\n ");
	pmic_vcore1_dvfs_step_inc(1);
	pmic_vcore1_dvfs_1_eco3(4); 	// 1.3v
	pmic_vcore1_sleep_1_eco3(0);
	pmic_vcore1_dvfs_0_eco3(4);
	pmic_vcore1_sleep_0_eco3(1);
	pmic_vcore1_dvfs_ramp_enable(KAL_TRUE);
	/* update bit : 0 -> 1 */
	pmic_vcore1_dvfs_target_update(KAL_FALSE);
	pmic_vcore1_dvfs_target_update(KAL_TRUE);

    //printk("pmic_init :  pmic_flash_i_tune !!\n ");
    pmic_flash_i_tune(0xF); 

    /* CHARGER OFFSET */ 
    pmic_chr_offset(CHR_OFFSET);
#endif	

    /*  Backlight Setting for V2/V3 phone */
#if defined(CUST_LEDS_BACKLIGHT_PMIC_SERI)
    printk("pmic_init : series backlight\n ");
    pmic_bl_number(BL_NUM);
    pmic_asw_asel(ASW_ASEL);
    pmic_vboost1_tune(VBOOST1_TUNE);
    pmic_bl_i_corse_tune(BL_I_COARSE_TUNE);
    pmic_asw_bsel(ASW_BSEL);  
    pmic_boost2_dim_source(BOOST2_ANALOG_DIMING);    
    pmic_boost_mode(BOOST_MODE_TYPE_II);
    pmic_boost2_enable(KAL_TRUE); 
    pmic_bl_dim_duty(PMIC_DEFAULT_BL_DIM);
    pmic_bl_enable(KAL_TRUE);        
    pmic_vbus_enable(KAL_TRUE);    
#endif

	/*  Backlight Setting for Gemini phone */
#if defined(CUST_LEDS_BACKLIGHT_PMIC_PARA)
	printk("pmic_init : parallel backlight\n ");
	/* boost setting */
	pmic_boost_mode(BOOST_MODE_TYPE_I);
	pmic_vboost1_tune(VBOOST1_VOL_4_25_V);
	pmic_boost1_enable(KAL_TRUE);
	/* BL setting */
	pmic_igen_drv_force(KAL_TRUE);
	pmic_asw_asel(ASW_ASEL_ALL_ISINK_BL);
	pmic_bl_number(BL_NUM_6);
	pmic_bl_i_corse_tune(BL_I_CORSE_TUNE_16MA);
	pmic_bl_dim_duty(PMIC_DEFAULT_BL_DIM);
    pmic_bl_enable(KAL_TRUE);
	/* Flashlight setting */
	/* VBUS setting */
	pmic_vbus_enable(KAL_TRUE);
#endif

	/* 20100425 update the init setting */
	if (eco_version == PMIC6326_E4_CID_CODE)
	{ 
		pmic_wdt_timeout(WDT_TIMEOUT_32_SEC);				// set 32 sec
		//pmic_intr_polarity(KAL_TRUE);							// issue INT
		pmic_wdt_enable(KAL_TRUE); 							// enable wdt
			
		pmic_vcore1_dvfs_0_eco3(0xC);						// set vcore=1.1V, [4] is reserve, 0x6 for [7:5]
		pmic_vcore1_sleep_0_eco3(1);						// set as sleep mode voltage
		pmic_vcore1_dvfs_1_eco3(4); 						// set vcore=1.3v, [4] is reserve, 0x2 for [7:5]
		pmic_vcore1_sleep_1_eco3(0);						// set as normal mode voltage
		pmic_vcore1_dvfs_step_inc(1);						// 2 step for each voltage change
		pmic_vcore1_dvfs_ramp_enable(1);					// switch vcore1 by ramping
		pmic_vcore1_dvfs_target_update(1);					// switch out voltage when setting DVFS

		pmic_vpa_oc_tune(3);								// set overcurrent Th to 850mA
		
		pmic_vrf_calst(2);									// set the VRF soft-star to max slew rate
		pmic_vtcxo_calst(2);								// set the VTCXO soft-star to max slew rate
		pmic_v3gtx_calst(2);								// set the V3GTX soft-star to max slew rate
		pmic_v3grx_calst(2);								// set the V3GRX soft-star to max slew rate
		
		pmic_config_interface(0x8F,3,0x3,0x2);				// set VWIFI2V8 soft star internel = 800us
		pmic_config_interface(0x8F,3,0x3,0x0);				// set VWIFI3V3 soft star internel = 800us
		pmic_config_interface(0x90,3,0x3,0x2);				// set VSDIO soft star internel = 800us

		pmic_config_interface(0x92,3,0x3,0x2);				// set VWIFI2V8 soft star internel = 800us
		pmic_config_interface(0x92,3,0x3,0x0);				// set VWIFI3V3 soft star internel = 800us
		pmic_config_interface(0x93,3,0x3,0x2);				// set VSDIO soft star internel = 800us
#if 0
		pmic_spkr_enable(KAL_TRUE);							// enable class-D function
		pmic_spkr_dmode(2);									// set FB auto calibration dead time mode
		pmic_spkr_dtcal(0);									// enable audio class-D dead time calibration
		mdelay(2);
		pmic_spkr_dtcal(1);									// disable audio class-D dead time calibration
		pmic_spkr_enable(KAL_FALSE);						// disable class-D function

		pmic_spkl_enable(KAL_TRUE);							// enable class-D function
		pmic_spkl_dmode(2);									// set FB auto calibration dead time mode
		pmic_spkl_dtcal(0);									// enable audio class-D dead time calibration
		mdelay(2);
		pmic_spkl_dtcal(1);									// disable audio class-D dead time calibration
		pmic_spkl_enable(KAL_FALSE);						// disable class-D function
#endif
		//pmic_int_ctrl_1_enable(INT1_EN_ALL, KAL_FALSE);
	    //pmic_int_ctrl_2_enable(INT2_EN_ALL, KAL_TRUE);	

		pmic_boost1_sync_enable(KAL_TRUE);					// enable internal sync
		pmic_flash_i_tune(0xF); 							// set flashlight Ron
		pmic_config_interface(0x54,1,0x1,0x0);				// disable NMOS pull low
	}
	else
	{
		/* Kelvin : suspend power optimize, Vcore1 downgrade from 1.3V to 0.9V when enter suspend */
		pmic_vcore1_dvfs_step_inc(1);
		pmic_vcore1_sleep_0_eco3(1);
		pmic_vcore1_dvfs_ramp_enable(1);
		pmic_vcore1_dvfs_target_update(1);
		pmic_vtcxo_calst(VTCXO_MAX_SLEW_RATE);
		pmic_v3gtx_calst(V3GTX_MAX_SLEW_RATE);
	}

    hwPMMonitorInit();


	// 20100329 For DCT in o4
	//printk("Call DCT pmic_init----------");
	//pmic_init();


//#ifdef CONFIG_TESTCASE_MSG
	printk("pmic_init : after setting !!\n "); 
	for (i=0;i<PMIC_REG_NUM;i++){
		mt6326_read_byte(i, &pmic6326_reg[i]);
		printk("0x%X,0x%X\n", i, pmic6326_reg[i]);
	}
//#endif

	PMIC_Ready = KAL_TRUE;

	kthread_run(DVFS_thread_kthread, NULL, "DVFS_thread_kthread");
	
}

static int pmic_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int *user_data_addr;
	//int i = 0;
	int ret = 0;
	u8 returnData = 0;

    switch(cmd)
    {
        case TEST_PMIC_PRINT :
			printk("**** MT6326 pmic ioctl : test\n");
            break;
		
		case PMIC_READ:			
			user_data_addr = (int *)arg;
			ret = copy_from_user(pmic_in_data, user_data_addr, 8);
			//mt6326_read_byte(pmic_in_data[0], &pmic_out_data[0]);
			mt6326_read_byte(pmic_in_data[0], &returnData);
			pmic_out_data[0] = returnData;
			ret = copy_to_user(user_data_addr, pmic_out_data, 8);
			printk("**** MT6326 pmic ioctl : read reg[%d] = %d\n", pmic_in_data[0], pmic_out_data[0]);			
            break;	
			
		case PMIC_WRITE:			
			user_data_addr = (int *)arg;
			ret = copy_from_user(pmic_in_data, user_data_addr, 8);
			mt6326_write_byte(pmic_in_data[0], pmic_in_data[1]);			
			printk("**** MT6326 pmic ioctl : write %d to reg[%d]\n",pmic_in_data[1], pmic_in_data[0]);			
			/* check */
			mt6326_read_byte(pmic_in_data[0], &pmic_out_data[0]);
			printk("**** MT6326 pmic ioctl : read reg[%d] = %d\n", pmic_in_data[0], pmic_out_data[0]);
            break;
	
		case SET_PMIC_LCDBK:
			/* Gemini Phone */
			user_data_addr = (int *)arg;
			ret = copy_from_user(pmic_lcdbk_data, user_data_addr, 4);
			pmic_bl_dim_duty(pmic_lcdbk_data[0]);
			printk("**** MT6326 pmic ioctl : LCDBK set %d\n",pmic_lcdbk_data[0]);						
			break;
 
		case ENABLE_VIBRATOR:
			/* Gemini Phone */
			mt6326_vibr_Enable();
			printk("**** MT6326 pmic ioctl : Enable vibrator\n");						
			break;

		case DISABLE_VIBRATOR:
			/* Gemini Phone */
			mt6326_vibr_Disable();
			printk("**** MT6326 pmic ioctl : Disable vibrator\n");						
			break;			
 
        default:
            break;
    }

    return 0;
}

static int pmic_open(struct inode *inode, struct file *file)
{ 
   return 0;
}

static int pmic_release(struct inode *inode, struct file *file)
{
    return 0;
}

static struct file_operations pmic_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= pmic_ioctl,
	.open		= pmic_open,
	.release	= pmic_release,	
};

#if 0
//20100827
/* This function is called by i2c_detect */
int mt6326_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct class_device *class_dev = NULL;
    int err=0;
	int ret=0;

    printk("mt6326_detect !!\n ");

	/* Integrate with META TOOL : START */
	ret = alloc_chrdev_region(&pmic_devno, 0, 1, PMIC_DEVNAME);
	if (ret) 
		printk("Error: Can't Get Major number for pmic \n");
	pmic_cdev = cdev_alloc();
    pmic_cdev->owner = THIS_MODULE;
    pmic_cdev->ops = &pmic_fops;
    ret = cdev_add(pmic_cdev, pmic_devno, 1);
	if(ret)
	    printk("pmic Error: cdev_add\n");
	pmic_major = MAJOR(pmic_devno);
	pmic_class = class_create(THIS_MODULE, PMIC_DEVNAME);
    class_dev = (struct class_device *)device_create(pmic_class, 
													NULL, 
													pmic_devno, 
													NULL, 
													PMIC_DEVNAME);
	printk("PMIC META Prepare : Done !!\n ");
	/* Integrate with META TOOL : END */

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        goto exit;

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }	
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client->addr = address;
    new_client->adapter = adapter;
    new_client->driver = &mt6326_driver;
    new_client->flags = 0;
    strncpy(new_client->name, "mt6326", I2C_NAME_SIZE);

    if ((err = i2c_attach_client(new_client)))
        goto exit_kfree;

    /* Test vcore2 by cmd */
    /* path : sys/bus/i2c/devices/0-00c0/yyyy */
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x81_read);
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x81_write);
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x53_read);
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x2F_read);    

    pmic_driver_init();

#if defined(CONFIG_MTK_SMART_BATTERY)
	printk("mt6326_detect : set mt6326_pmic_eint_irq !\n ");
	/* 20090806 */	
	//MT6516_EINT_Set_Sensitivity(0,1); 		// set EIN0 to LEVEL trigger	
	//printk("mt6326_detect : MT6516_EINT_Set_Sensitivity(0,0) !\n ");
	MT6516_EINT_Set_Sensitivity(CUST_EINT_MT6326_PMIC_NUM,
			            CUST_EINT_MT6326_PMIC_SENSITIVE);
	MT6516_EINT_Set_Polarity(CUST_EINT_MT6326_PMIC_NUM,
			         CUST_EINT_MT6326_PMIC_POLARITY);// set positive polarity
	MT6516_EINT_Set_HW_Debounce(CUST_EINT_MT6326_PMIC_NUM,CUST_EINT_MT6326_PMIC_DEBOUNCE_CN); 	// set debounce time
	mt_set_gpio_mode(GPIO_PMIC_EINT_PIN,0x0);
	mt_set_gpio_pull_enable(GPIO_PMIC_EINT_PIN,true);
	mt_set_gpio_pull_select(GPIO_PMIC_EINT_PIN,false);
	mt_set_gpio_mode(GPIO_PMIC_EINT_PIN,0x01);	
	MT6516_EINT_Registration(CUST_EINT_MT6326_PMIC_NUM,
			         CUST_EINT_MT6326_PMIC_DEBOUNCE_EN,
				 CUST_EINT_MT6326_PMIC_POLARITY,
				 mt6326_pmic_eint_irq,
				 0);
   	
    /* mt6326 :  mt6326_pmic_eint_irq */
    //MT6516_EINT_Set_Sensitivity(0,0);
    //MT6516_EINT_Registration(0, 1, 1, mt6326_pmic_eint_irq, 1);
#endif
	
    #ifdef CONFIG_TESTCASE_MSG
    /* Test LDO ON/OFF and voltage setting */
    /* path : sys/bus/i2c/devices/1-00c0/yyyy */    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VRF);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VRF);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VTCXO);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VTCXO);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_V3GTX);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_V3GTX);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_V3GRX);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_V3GRX);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VCAM_A);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VCAM_A);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VWIFI3V3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VWIFI3V3);    

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VWIFI2V8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VWIFI2V8);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VSIM);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VSIM);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VUSB);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VUSB);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VBT);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VBT);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VCAM_D);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VCAM_D);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VGP);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VGP); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VGP2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VGP2); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VSDIO);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VSDIO); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VCORE_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VCORE_2); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VPA);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VPA); 
        
    /* Test LDO voltage setting */
    /* path : sys/bus/i2c/devices/1-00c0/yyyy */
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_3_3);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_1_5);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_3_3);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSDIO_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSDIO_set_3_0);
    
    /* VCORE 1 setting */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_80);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_85);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_90);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_95);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_05);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_10);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_15);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_25);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_35);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_4);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_45);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_55);

    /* VCORE 2 setting */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_80);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_85);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_90);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_95);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_05);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_10);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_15);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_25);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_35);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_4);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_45);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_55);

    /* VPA setting */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_075);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_15);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_225);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_375);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_45);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_525);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_6);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_675);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_75);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_825);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_9);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_975);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_05);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_125);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_275);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_35);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_425);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_575);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_65);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_725);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_875);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_95);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_025);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_1);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_175);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_25);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_325);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_4);

    /* Test Charging Function */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kick_charger_wdt);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_en_charg_450);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_charg);

    /* Backlight Test */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_Enable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_Disable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_dim_duty_Full);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_dim_duty_0);        

    /* KPD LED Test */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_Enable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_Disable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_dim_duty_Full);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_dim_duty_0);      

    /* Vibr Test */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_vibr_Enable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_vibr_Disable);   

    /* Dump all registers */    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_Dump_Register);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_check_power);
    
    #endif

    #if 0
    /* Test Function  mt6326_read_byte and mt6326_write_byte*/
    ret =  mt6326_read_byte(0x00, &pmic6326_reg[0x00]);
    ret =  mt6326_read_byte(0x00, &pmic6326_reg[0x00]);
    if (!ret) {
        printk("mt6326_read_byte error !!\n");
        return 0;
    } 
    printk("mt6326_read_byte (0x00) : 0x%x \n", pmic6326_reg[0x00]);
    #endif

    #if 0
    /* Test read/write by cmd */
    /* path : sys/bus/i2c/devices/1-00c0/yyyy */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_read);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_write);
    #endif     

    #if 0 
    /* Test Function  mt6326_read_byte and mt6326_write_byte*/

    /* First read will error, todo debug */
    /* 20090506 : ignore this error      */
    ret =  mt6326_read_byte(0x00, &readData);
    
    ret =  mt6326_read_byte(0x00, &readData);
    if (!ret) {
        printk("mt6326_read_byte error !!\n");
        return 0;
    } 
    printk("mt6326_read_byte : 0x%x \n", readData);

    ret =  mt6326_write_byte(0x81, 0x44);
    if (!ret) {
        printk("mt6326_write_byte error !!\n");
        return 0;
    } 
    printk("mt6326_write_byte done \n");

    ret =  mt6326_read_byte(0x81, &readData);
    if (!ret) {
        printk("mt6326_read_byte error !!\n");
        return 0;
    } 
    printk("mt6326_read_byte : 0x%x \n", readData);
    #endif

    return 0;

exit_kfree:
    kfree(new_client);
exit:
    return err;
}

static int mt6326_attach_adapter(struct i2c_adapter *adapter)
{
    if (adapter->id == 1)
    	return i2c_probe(adapter, &addr_data, mt6326_detect);
    return -1;
}

static int mt6326_detach_client(struct i2c_client *client)
{
	int err;

	err = i2c_detach_client(client);
	if (err) {
		dev_err(&client->dev, "Client deregistration failed, client not detached.\n");
		return err;
	}

	kfree(i2c_get_clientdata(client));
	
	return 0;
}
#endif

static int mt6326_driver_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, "mt6326_pmic");                                                         

	printk("mt6326_driver_detect !!\n ");
	
    return 0;                                                                                       
}

static int mt6326_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) {             
	struct class_device *class_dev = NULL;
    int err=0;
	int ret=0;

    printk("mt6326_driver_probe !!\n ");

	/* Integrate with META TOOL : START */
	ret = alloc_chrdev_region(&pmic_devno, 0, 1, PMIC_DEVNAME);
	if (ret) 
		printk("Error: Can't Get Major number for pmic \n");
	pmic_cdev = cdev_alloc();
    pmic_cdev->owner = THIS_MODULE;
    pmic_cdev->ops = &pmic_fops;
    ret = cdev_add(pmic_cdev, pmic_devno, 1);
	if(ret)
	    printk("pmic Error: cdev_add\n");
	pmic_major = MAJOR(pmic_devno);
	pmic_class = class_create(THIS_MODULE, PMIC_DEVNAME);
    class_dev = (struct class_device *)device_create(pmic_class, 
													NULL, 
													pmic_devno, 
													NULL, 
													PMIC_DEVNAME);
	printk("PMIC META Prepare : Done !!\n ");
	/* Integrate with META TOOL : END */

	if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }	
    memset(new_client, 0, sizeof(struct i2c_client));

	new_client = client;

#if 0
    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        goto exit;

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }	
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client->addr = address;
    new_client->adapter = adapter;
    new_client->driver = &mt6326_driver;
    new_client->flags = 0;
    strncpy(new_client->name, "mt6326", I2C_NAME_SIZE);

    if ((err = i2c_attach_client(new_client)))
        goto exit_kfree;
#endif 

    /* Test vcore2 by cmd */
    /* path : sys/bus/i2c/devices/0-00c0/yyyy */
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x81_read);
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x81_write);
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x53_read);
    //sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_0x2F_read);    

    pmic_driver_init();

#if defined(CONFIG_MTK_SMART_BATTERY)
	printk("mt6326_detect : set mt6326_pmic_eint_irq !!\n ");
	/* 20090806 */	
	//MT6516_EINT_Set_Sensitivity(0,1); 		// set EIN0 to LEVEL trigger	
	//printk("mt6326_detect : MT6516_EINT_Set_Sensitivity(0,0) !\n ");
	MT6516_EINT_Set_Sensitivity(CUST_EINT_MT6326_PMIC_NUM,
			            CUST_EINT_MT6326_PMIC_SENSITIVE);
	MT6516_EINT_Set_Polarity(CUST_EINT_MT6326_PMIC_NUM,
			         CUST_EINT_MT6326_PMIC_POLARITY);// set positive polarity
	MT6516_EINT_Set_HW_Debounce(CUST_EINT_MT6326_PMIC_NUM,CUST_EINT_MT6326_PMIC_DEBOUNCE_CN); 	// set debounce time
	mt_set_gpio_mode(GPIO_PMIC_EINT_PIN,0x0);
	mt_set_gpio_pull_enable(GPIO_PMIC_EINT_PIN,true);
	mt_set_gpio_pull_select(GPIO_PMIC_EINT_PIN,false);
	mt_set_gpio_mode(GPIO_PMIC_EINT_PIN,0x01);	
	MT6516_EINT_Registration(CUST_EINT_MT6326_PMIC_NUM,
			         CUST_EINT_MT6326_PMIC_DEBOUNCE_EN,
				 CUST_EINT_MT6326_PMIC_POLARITY,
				 mt6326_pmic_eint_irq,
				 0);
   	
    /* mt6326 :  mt6326_pmic_eint_irq */
    //MT6516_EINT_Set_Sensitivity(0,0);
    //MT6516_EINT_Registration(0, 1, 1, mt6326_pmic_eint_irq, 1);
#endif

#if 0
    #ifdef CONFIG_TESTCASE_MSG
    /* Test LDO ON/OFF and voltage setting */
    /* path : sys/bus/i2c/devices/1-00c0/yyyy */    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VRF);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VRF);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VTCXO);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VTCXO);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_V3GTX);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_V3GTX);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_V3GRX);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_V3GRX);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VCAM_A);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VCAM_A);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VWIFI3V3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VWIFI3V3);    

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VWIFI2V8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VWIFI2V8);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VSIM);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VSIM);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VUSB);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VUSB);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VBT);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VBT);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VCAM_D);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VCAM_D);  

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VGP);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VGP); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VGP2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VGP2); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VSDIO);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VSDIO); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VCORE_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VCORE_2); 

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_enable_VPA);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_VPA); 
        
    /* Test LDO voltage setting */
    /* path : sys/bus/i2c/devices/1-00c0/yyyy */
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GTX_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_V3GRX_set_3_3);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAM_A_set_1_5);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI3V3_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VWIFI2V8_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSIM_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VBT_set_3_3);
    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCAMD_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_1_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_3_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VGP2_set_3_3);

    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSDIO_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VSDIO_set_3_0);
    
    /* VCORE 1 setting */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_80);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_85);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_90);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_0_95);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_05);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_10);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_15);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_25);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_35);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_4);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_45);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_1_set_1_55);

    /* VCORE 2 setting */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_80);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_85);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_90);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_0_95);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_0);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_05);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_10);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_15);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_25);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_35);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_4);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_45);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VCORE_2_set_1_55);

    /* VPA setting */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_075);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_15);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_225);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_3);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_375);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_45);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_525);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_6);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_675);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_75);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_825);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_9);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_1_975);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_05);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_125);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_2);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_275);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_35);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_425);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_5);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_575);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_65);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_725);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_8);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_875);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_2_95);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_025);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_1);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_175);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_25);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_325);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_VPA_set_3_4);

    /* Test Charging Function */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kick_charger_wdt);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_en_charg_450);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_disable_charg);

    /* Backlight Test */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_Enable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_Disable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_dim_duty_Full);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_bl_dim_duty_0);        

    /* KPD LED Test */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_Enable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_Disable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_dim_duty_Full);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_kpled_dim_duty_0);      

    /* Vibr Test */
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_vibr_Enable);    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_vibr_Disable);   

    /* Dump all registers */    
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_Dump_Register);
    sysfs_create_bin_file(&new_client->dev.kobj, &mt6326_attr_check_power);
    
    #endif
#endif	
	
    return 0;                                                                                       

exit_kfree:
    kfree(new_client);
exit:
    return err;

}

static int __init mt6326_init(void)
{
	//20100827
	//return i2c_add_driver(&mt6326_driver);
	
	if(i2c_add_driver(&mt6326_driver)!=0)
	{
		printk("[mt6516-pmic] failed to register pmic i2c driver.\n");
	}
	else
	{
		printk("[mt6516-pmic] Success to register pmic i2c driver.\n");
	}
	
	return 0;		
}

static void __exit mt6326_exit(void)
{
	i2c_del_driver(&mt6326_driver);
}

module_init(mt6326_init);
module_exit(mt6326_exit);

/* charging function for battery driver */
EXPORT_SYMBOL(mt6326_test_con);
EXPORT_SYMBOL(pmic_ovp_status);
EXPORT_SYMBOL(pmic_chrdet_status);
EXPORT_SYMBOL(pmic_bat_on_status);
EXPORT_SYMBOL(pmic_cv_status);
EXPORT_SYMBOL(pmic_chr_current);
EXPORT_SYMBOL(pmic_wdt_enable);
EXPORT_SYMBOL(pmic_wdt_timeout);
EXPORT_SYMBOL(pmic_chr_chr_enable);
EXPORT_SYMBOL(pmic_adc_vbat_enable);
EXPORT_SYMBOL(pmic_adc_isense_enable);
EXPORT_SYMBOL(pmic6326_kick_charger_wdt);

/* 14 LDO Turn on / off */
/* 4 Always ON : VA, VIO, VRTC, VBAT_BACKUP */
EXPORT_SYMBOL(mt6326_enable_VRF);     
EXPORT_SYMBOL(mt6326_disable_VRF);    
EXPORT_SYMBOL(mt6326_enable_VTCXO);   
EXPORT_SYMBOL(mt6326_disable_VTCXO);  
EXPORT_SYMBOL(mt6326_enable_V3GTX);   
EXPORT_SYMBOL(mt6326_disable_V3GTX);  
EXPORT_SYMBOL(mt6326_enable_V3GRX);   
EXPORT_SYMBOL(mt6326_disable_V3GRX);  
EXPORT_SYMBOL(mt6326_enable_VCAM_A);  
EXPORT_SYMBOL(mt6326_disable_VCAM_A); 
EXPORT_SYMBOL(mt6326_enable_VWIFI3V3);
EXPORT_SYMBOL(mt6326_disable_VWIFI3V3);
EXPORT_SYMBOL(mt6326_enable_VWIFI2V8);
EXPORT_SYMBOL(mt6326_disable_VWIFI2V8);
EXPORT_SYMBOL(mt6326_enable_VSIM);    
EXPORT_SYMBOL(mt6326_disable_VSIM);   
EXPORT_SYMBOL(mt6326_enable_VUSB);    
EXPORT_SYMBOL(mt6326_disable_VUSB);   
EXPORT_SYMBOL(mt6326_enable_VBT);     
EXPORT_SYMBOL(mt6326_disable_VBT);    
EXPORT_SYMBOL(mt6326_enable_VCAM_D);  
EXPORT_SYMBOL(mt6326_disable_VCAM_D); 
EXPORT_SYMBOL(mt6326_enable_VGP);     
EXPORT_SYMBOL(mt6326_disable_VGP);    
EXPORT_SYMBOL(mt6326_enable_VGP2);    
EXPORT_SYMBOL(mt6326_disable_VGP2);   
EXPORT_SYMBOL(mt6326_enable_VSDIO);   
EXPORT_SYMBOL(mt6326_disable_VSDIO);     
   
/* 14 LDO voltage setting */
EXPORT_SYMBOL(mt6326_V3GTX_set_2_8);    
EXPORT_SYMBOL(mt6326_V3GTX_set_2_5);    
EXPORT_SYMBOL(mt6326_V3GTX_set_3_0);    
EXPORT_SYMBOL(mt6326_V3GTX_set_3_3);    
EXPORT_SYMBOL(mt6326_V3GRX_set_2_8);    
EXPORT_SYMBOL(mt6326_V3GRX_set_2_5);    
EXPORT_SYMBOL(mt6326_V3GRX_set_3_0);    
EXPORT_SYMBOL(mt6326_V3GRX_set_3_3);    
EXPORT_SYMBOL(mt6326_VCAM_A_set_2_5);   
EXPORT_SYMBOL(mt6326_VCAM_A_set_2_8);   
EXPORT_SYMBOL(mt6326_VCAM_A_set_1_8);   
EXPORT_SYMBOL(mt6326_VCAM_A_set_1_5);   
EXPORT_SYMBOL(mt6326_VWIFI3V3_set_2_5); 
EXPORT_SYMBOL(mt6326_VWIFI3V3_set_2_8); 
EXPORT_SYMBOL(mt6326_VWIFI3V3_set_3_0); 
EXPORT_SYMBOL(mt6326_VWIFI3V3_set_3_3); 
EXPORT_SYMBOL(mt6326_VWIFI2V8_set_2_5); 
EXPORT_SYMBOL(mt6326_VWIFI2V8_set_2_8); 
EXPORT_SYMBOL(mt6326_VWIFI2V8_set_3_0); 
EXPORT_SYMBOL(mt6326_VWIFI2V8_set_3_3); 
EXPORT_SYMBOL(mt6326_VSIM_set_1_3);     
EXPORT_SYMBOL(mt6326_VSIM_set_1_5);     
EXPORT_SYMBOL(mt6326_VSIM_set_1_8);     
EXPORT_SYMBOL(mt6326_VSIM_set_2_5);     
EXPORT_SYMBOL(mt6326_VSIM_set_2_8);     
EXPORT_SYMBOL(mt6326_VSIM_set_3_0);     
EXPORT_SYMBOL(mt6326_VSIM_set_3_3);     
EXPORT_SYMBOL(mt6326_VBT_set_1_3);      
EXPORT_SYMBOL(mt6326_VBT_set_1_5);      
EXPORT_SYMBOL(mt6326_VBT_set_1_8);      
EXPORT_SYMBOL(mt6326_VBT_set_2_5);      
EXPORT_SYMBOL(mt6326_VBT_set_2_8);      
EXPORT_SYMBOL(mt6326_VBT_set_3_0);      
EXPORT_SYMBOL(mt6326_VBT_set_3_3);      
EXPORT_SYMBOL(mt6326_VCAMD_set_1_3);    
EXPORT_SYMBOL(mt6326_VCAMD_set_1_5);    
EXPORT_SYMBOL(mt6326_VCAMD_set_1_8);    
EXPORT_SYMBOL(mt6326_VCAMD_set_2_5);    
EXPORT_SYMBOL(mt6326_VCAMD_set_2_8);    
EXPORT_SYMBOL(mt6326_VCAMD_set_3_0);    
EXPORT_SYMBOL(mt6326_VCAMD_set_3_3);    
EXPORT_SYMBOL(mt6326_VGP_set_1_3);      
EXPORT_SYMBOL(mt6326_VGP_set_1_5);      
EXPORT_SYMBOL(mt6326_VGP_set_1_8);      
EXPORT_SYMBOL(mt6326_VGP_set_2_5);      
EXPORT_SYMBOL(mt6326_VGP_set_2_8);      
EXPORT_SYMBOL(mt6326_VGP_set_3_0);      
EXPORT_SYMBOL(mt6326_VGP_set_3_3);      
EXPORT_SYMBOL(mt6326_VGP2_set_1_3);     
EXPORT_SYMBOL(mt6326_VGP2_set_1_5);     
EXPORT_SYMBOL(mt6326_VGP2_set_1_8);     
EXPORT_SYMBOL(mt6326_VGP2_set_2_5);     
EXPORT_SYMBOL(mt6326_VGP2_set_2_8);     
EXPORT_SYMBOL(mt6326_VGP2_set_3_0);     
EXPORT_SYMBOL(mt6326_VGP2_set_3_3);     
EXPORT_SYMBOL(mt6326_VSDIO_set_2_8);    
EXPORT_SYMBOL(mt6326_VSDIO_set_3_0);       
   
/* VCORE 1 DVFS */
EXPORT_SYMBOL(mt6326_VCORE_1_set_0_80);
EXPORT_SYMBOL(mt6326_VCORE_1_set_0_85);
EXPORT_SYMBOL(mt6326_VCORE_1_set_0_90);
EXPORT_SYMBOL(mt6326_VCORE_1_set_0_95);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_0); 
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_05);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_10);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_15);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_2); 
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_25);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_3); 
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_35);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_4); 
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_45);
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_5); 
EXPORT_SYMBOL(mt6326_VCORE_1_set_1_55);   

/* VCORE 2 DVFS */
EXPORT_SYMBOL(mt6326_VCORE_2_set_0_80);
EXPORT_SYMBOL(mt6326_VCORE_2_set_0_85);
EXPORT_SYMBOL(mt6326_VCORE_2_set_0_90);
EXPORT_SYMBOL(mt6326_VCORE_2_set_0_95);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_0); 
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_05);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_10);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_15);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_2); 
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_25);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_3); 
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_35);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_4); 
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_45);
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_5); 
EXPORT_SYMBOL(mt6326_VCORE_2_set_1_55); 

/* Vibrator driver */
EXPORT_SYMBOL(mt6326_vibr_Enable); 
EXPORT_SYMBOL(mt6326_vibr_Disable); 

/* Sound driver */
EXPORT_SYMBOL(Yusu_Sound_AMP_Switch);
EXPORT_SYMBOL(pmic_asw_bsel);
EXPORT_SYMBOL(pmic_spkl_vol);
EXPORT_SYMBOL(pmic_spkr_vol);
EXPORT_SYMBOL(pmic_spkl_enable);
EXPORT_SYMBOL(pmic_spkr_enable);
EXPORT_SYMBOL(SPK_Dead_Time_Calibration);

/* Backlight pmic driver */
EXPORT_SYMBOL(mt6326_bl_Enable);
EXPORT_SYMBOL(mt6326_bl_Disable);
EXPORT_SYMBOL(pmic_bl_dim_duty);

/* flashlight pmic driver */
EXPORT_SYMBOL(mt6326_FL_Enable);
EXPORT_SYMBOL(mt6326_FL_Disable);
EXPORT_SYMBOL(pmic_FL_dim_duty);

/* CCCI PMIC driver */
EXPORT_SYMBOL(pmic_vsim_cal);
EXPORT_SYMBOL(pmic_vsim_sel);
EXPORT_SYMBOL(pmic_vsim_enable);
EXPORT_SYMBOL(pmic_vsim2_sel);
EXPORT_SYMBOL(pmic_vsim2_enable);

/* Battery driver */
EXPORT_SYMBOL(pmic_int_status_3);   

/* Power Management Check */
EXPORT_SYMBOL(mt6326_check_power);
   
/* Check PMIC Ready */
EXPORT_SYMBOL(Check_PMIC_Ready);
   
MODULE_LICENSE("GPL");
//MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("MediaTek I2C mt6326 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");

