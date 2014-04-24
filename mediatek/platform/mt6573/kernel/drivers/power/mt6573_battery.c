
#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/wakelock.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <mach/hardware.h>
#include <mach/mt6573_gpt.h>
#include <mach/mt6573_boot.h>

//#include "mt6573_cust_adc.h"
#include <cust_adc.h>

//#include "mt6573_cust_battery.h"
#include <cust_battery.h>
#include "mt6573_battery.h"
#include "pmu6573_sw.h"
#include "upmu_sw.h"
#include "mt6573_udc.h"

//#define CONFIG_DEBUG_MSG

///////////////////////////////////////////////////////////////////////////////////////////
//// Extern Functions
///////////////////////////////////////////////////////////////////////////////////////////
extern int IMM_GetOneChannelValue(int dwChannel, int deCount);
extern kal_bool upmu_is_chr_det(upmu_chr_list_enum chr);
extern void upmu_adc_measure_vbat_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_adc_measure_vsen_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_adc_measure_vchr_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_chr_current(upmu_chr_list_enum chr, upmu_chr_current_enum current_val);
extern void upmu_cs_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_vbat_cc_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);
extern void upmu_vbat_cv_vth(upmu_chr_list_enum chr, upmu_chr_vol_enum vol);
extern void upmu_vbat_cc_det_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_vbat_cv_det_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_chrwdt_td(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_chrwdt_int_enable(upmu_chr_list_enum chr, kal_uint8 enable);
extern void upmu_chrwdt_enable(upmu_chr_list_enum chr, kal_uint8 enable);
extern void upmu_chrwdt_flag(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_csdac_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_chr_enable(upmu_chr_list_enum chr, kal_bool enable);
extern kal_bool upmu_is_vbat_cv_det(upmu_chr_list_enum chr);
extern CHARGER_TYPE mt_charger_type_detection(void);
extern kal_int32 FGADC_Get_BatteryCapacity_CoulombMothod(void);
extern kal_int32 FGADC_Get_BatteryCapacity_VoltageMothod(void);
extern kal_int32 FGADC_Get_FG_Voltage(void);
extern void FGADC_Reset_SW_Parameter(void);
extern void upmu_csdac_dly(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_csdac_stp(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_pchr_csdac_test_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_pchr_test_csdac_dat(upmu_chr_list_enum chr, kal_uint8 val);
extern void upmu_bc11_bb_crtl_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_bc11_reset_circuit(upmu_chr_list_enum chr, kal_uint8 val);

extern kal_int32 gFG_current;
extern kal_int32 gFG_voltage;
extern kal_int32 gFG_DOD0;
extern kal_int32 gFG_DOD1;
extern int gFG_15_vlot;

///////////////////////////////////////////////////////////////////////////////////////////
//// Smart Battery Structure
///////////////////////////////////////////////////////////////////////////////////////////
#define UINT32 unsigned long
#define UINT16 unsigned short
#define UINT8 unsigned char

//#define BATT_ID_CHECK_SUPPORT	1	/*add for IEEE1725 certification*/
#define BATT_VOLT_OV_CHECK_SUPPORT	0	/*add for IEEE1725 certification, not used now*/
#if BATT_VOLT_OV_CHECK_SUPPORT
	#define BATT_OVER_VOLTAGE_VALUE	4250
#endif

#if BATT_ID_CHECK_SUPPORT
	#define AUXADC_BATT_ID_CHANNEL	4
	#define CHG_MIN_VALID_BATT_ID	600	/*0.6V*/
	#define CHG_MAX_VALID_BATT_ID	900	/*0.9V*/
#endif

typedef struct 
{
    kal_bool   	bat_exist;
#if BATT_ID_CHECK_SUPPORT
    kal_bool   	bat_id_valid;	/*compatible with IEEE1725*/
#endif
    kal_bool   	bat_full;  
    kal_bool   	bat_low;  
    UINT32  	bat_charging_state;
    UINT32  	bat_vol;            
    kal_bool 	charger_exist;   
    UINT32  	pre_charging_current;
    UINT32  	charging_current;
    UINT32  	charger_vol;        
    UINT32   	charger_protect_status; 
    UINT32  	ISENSE;                
    UINT32  	ICharging;
    INT32   	temperature;
    UINT32  	total_charging_time;
    UINT32  	PRE_charging_time;
	UINT32  	CC_charging_time;
	UINT32  	TOPOFF_charging_time;
	UINT32  	POSTFULL_charging_time;
    UINT32   	charger_type;
    UINT32   	PWR_SRC;
    UINT32   	SOC;
    UINT32   	ADC_BAT_SENSE;
    UINT32   	ADC_I_SENSE;
} PMU_ChargerStruct;

typedef enum 
{
    PMU_STATUS_OK = 0,
    PMU_STATUS_FAIL = 1,
}PMU_STATUS;

///////////////////////////////////////////////////////////////////////////////////////////
//// Global Variable
///////////////////////////////////////////////////////////////////////////////////////////
static CHARGER_TYPE CHR_Type_num = CHARGER_UNKNOWN;
PMU_ChargerStruct BMT_status;

static unsigned short batteryVoltageBuffer[BATTERY_AVERAGE_SIZE];
static unsigned short batteryCurrentBuffer[BATTERY_AVERAGE_SIZE];
static unsigned short batterySOCBuffer[BATTERY_AVERAGE_SIZE];
static int batteryIndex = 0;
static int batteryVoltageSum = 0;
static int batteryCurrentSum = 0;
static int batterySOCSum = 0;
kal_bool g_bat_full_user_view = KAL_FALSE;
kal_bool g_Battery_Fail = KAL_FALSE;
kal_bool batteryBufferFirst = KAL_FALSE;

struct wake_lock battery_suspend_lock; 

//#if defined(CONFIG_BATTERY_E1000)
int g_BatTempProtectEn = 0; /*0:temperature measuring default off*/
//#else
//int g_BatTempProtectEn = 1; /*1:temperature measuring default on*/
//#endif

int g_PMIC_CC_VTH = PMIC_ADPT_VOLT_03_300000_V;
int g_PMIC_CV_VTH = PMIC_ADPT_VOLT_04_000000_V;
int V_PRE2CC_THRES = 3400;
//int V_CC2TOPOFF_THRES = 4100;
int V_CC2TOPOFF_THRES = 4050;
//int V_CC2TOPOFF_THRES = 4000;
int V_compensate_EVB = 80;

int g_HW_Charging_Done = 0;
int g_Charging_Over_Time = 0;

int g_SW_CHR_OUT_EN = 0;
int g_HW_stop_charging = 0;

// HW CV algorithm
unsigned int CHR_CON_0 = 0xF702FA00;
unsigned int CHR_CON_1 = 0xF702FA04;
unsigned int CHR_CON_2 = 0xF702FA08;
unsigned int CHR_CON_4 = 0xF702FA10;
unsigned int CHR_CON_6 = 0xF702FA18;
unsigned int CHR_CON_9 = 0xF702FA24;
unsigned int CHR_CON_10 = 0xF702FA28;
unsigned int PMIC_RESERVE_CON1 = 0xF702FE84;
volatile unsigned int read_value = 0x0;
volatile unsigned int write_value = 0x0;
volatile unsigned int save_value = 0x0;
volatile unsigned int CSDAC_DAT_MAX = 255;
volatile unsigned int CSDAC_DAT = 0;
volatile unsigned int VBAT_CV_DET = 0x0;
volatile unsigned int CS_DET = 0x0;
int g_sw_cv_enable=0;

#if defined(CONFIG_POWER_EXT)
int CHARGING_FULL_CURRENT=300;	// mA on EVB
#else
int CHARGING_FULL_CURRENT=120;	// mA on phone
#endif

// may eat the custom file
int gForceADCsolution=0;
//int gForceADCsolution=1;

int gSyncPercentage=0;

unsigned int g_BatteryNotifyCode=0x0000;
unsigned int g_BN_TestMode=0x0000;

kal_uint32 gFGsyncTimer=0;
//kal_uint32 DEFAULT_SYNC_TIME_OUT=30; //30s
//kal_uint32 DEFAULT_SYNC_TIME_OUT=180; //3mins
kal_uint32 DEFAULT_SYNC_TIME_OUT=60; //1mins

int g_Calibration_FG=0;

int g_XGPT_restart_flag=0;

#define CHR_OUT_CURRENT	50

int gSW_CV_prepare_flag=0;

int gBAT_counter_15=1;

int gADC_BAT_SENSE_temp=0;
int gADC_I_SENSE_temp=0;
int gADC_I_SENSE_offset=0;

int g_BAT_TemperatureR = 0;
int g_TempBattVoltage = 0;
int g_InstatVolt = 0;
int g_BatteryAverageCurrent = 0;
int g_BAT_BatterySenseVoltage = 0;
int g_BAT_ISenseVoltage = 0;
int g_BAT_ChargerVoltage = 0;

static int bat_thread_timeout = 0;
static int sw_cv_thread_timeout = 0;

static DEFINE_MUTEX(bat_mutex);
static DECLARE_WAIT_QUEUE_HEAD(bat_thread_wq);

static DEFINE_MUTEX(sw_cv_mutex);
static DECLARE_WAIT_QUEUE_HEAD(sw_cv_thread_wq);

int g_chr_event = 0;
int bat_volt_cp_flag = 0;
int bat_volt_check_point = 0;
int Enable_BATDRV_LOG = 0;
//int Enable_BATDRV_LOG = 1;

/// mark add for logout
#define BAT_LOG(...) \ 
{ \
    if (Enable_BATDRV_LOG ) \
    { \
        printk(__VA_ARGS__); \
    } \
}

void wake_up_bat (void)
{
#if defined(CONFIG_POWER_EXT)
//#if 0
	printk("[BATTERY] wake_up_bat. EVB: do nothing\r\n");
#else
	//if (Enable_BATDRV_LOG == 1) {
		printk("[BATTERY] wake_up_bat. Call FGADC_Reset_SW_Parameter.\r\n");
	//}
	g_Calibration_FG = 0;
    FGADC_Reset_SW_Parameter();	

    bat_thread_timeout = 1;
    wake_up(&bat_thread_wq);
#endif	
}
EXPORT_SYMBOL(wake_up_bat);

void wake_up_sw_cv (void)
{
    sw_cv_thread_timeout = 1;
    wake_up(&sw_cv_thread_wq);
}
EXPORT_SYMBOL(wake_up_sw_cv);

int g_usb_state = USB_UNCONFIGURED;
//int g_usb_state = USB_SUSPEND;

int g_temp_CC_value = Cust_CC_0MA;
int g_soft_start_delay = 1;

#if (CONFIG_USB_IF == 0)
int g_Support_USBIF = 0;
#else
int g_Support_USBIF = 1;
#endif

static struct proc_dir_entry *proc_entry;
static char proc_bat_data[32];  

ssize_t bat_log_write( struct file *filp, const char __user *buff,
                        unsigned long len, void *data )
{
	if (copy_from_user( &proc_bat_data, buff, len )) {
		printk("bat_log_write error.\n");
		return -EFAULT;
	}

	if (proc_bat_data[0] == '1') {
		printk("enable battery driver log system\n");
		Enable_BATDRV_LOG = 1;
	} else {
		printk("Disable battery driver log system\n");
		Enable_BATDRV_LOG = 0;
	}
	
	return len;
}

int init_proc_log(void)
{
	int ret=0;
	proc_entry = create_proc_entry( "batdrv_log", 0644, NULL );
	
	if (proc_entry == NULL) {
		ret = -ENOMEM;
	  	printk("init_proc_log: Couldn't create proc entry\n");
	} else {
		proc_entry->write_proc = bat_log_write;
		//proc_entry->owner = THIS_MODULE;
		printk("init_proc_log loaded.\n");
	}
  
	return ret;
}

#define ADC_CALI_DEVNAME "MT6516-adc_cali"

#define TEST_ADC_CALI_PRINT _IO('k', 0)
#define SET_ADC_CALI_Slop _IOW('k', 1, int)
#define SET_ADC_CALI_Offset _IOW('k', 2, int)
#define SET_ADC_CALI_Cal _IOW('k', 3, int)
#define ADC_CHANNEL_READ _IOW('k', 4, int)
#define BAT_STATUS_READ _IOW('k', 5, int)
#define Set_Charger_Current _IOW('k', 6, int)

static struct class *adc_cali_class = NULL;
static int adc_cali_major = 0;
static dev_t adc_cali_devno;
static struct cdev *adc_cali_cdev;

int adc_cali_slop[14] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
int adc_cali_offset[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int adc_cali_cal[1] = {0};

int adc_in_data[2] = {1,1};
int adc_out_data[2] = {1,1};

int battery_in_data[1] = {0};
int battery_out_data[1] = {0};    

int charging_level_data[1] = {0};

kal_bool g_ADC_Cali = KAL_FALSE;
kal_bool g_ftm_battery_flag = KAL_FALSE;

////////////////////////////////////////////////////////////////////////////////
// FOR ANDROID BATTERY SERVICE
////////////////////////////////////////////////////////////////////////////////
struct mt6573_ac_data {
    struct power_supply psy;
    int AC_ONLINE;    
};

struct mt6573_usb_data {
    struct power_supply psy;
    int USB_ONLINE;    
};

struct mt6573_battery_data {
    struct power_supply psy;
    int BAT_STATUS;
    int BAT_HEALTH;
    int BAT_PRESENT;
    int BAT_TECHNOLOGY;
    int BAT_CAPACITY;
    /* Add for Battery Service*/
    int BAT_batt_vol;
    int BAT_batt_temp;
    /* Add for EM */
    int BAT_TemperatureR;
    int BAT_TempBattVoltage;
    int BAT_InstatVolt;
    int BAT_BatteryAverageCurrent;
    int BAT_BatterySenseVoltage;
    int BAT_ISenseVoltage;
    int BAT_ChargerVoltage;
};

static enum power_supply_property mt6573_ac_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt6573_usb_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt6573_battery_props[] = {
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_PRESENT,
    POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_CAPACITY,
    /* Add for Battery Service */
    POWER_SUPPLY_PROP_batt_vol,
    POWER_SUPPLY_PROP_batt_temp,    
    /* Add for EM */
    POWER_SUPPLY_PROP_TemperatureR,
    POWER_SUPPLY_PROP_TempBattVoltage,
    POWER_SUPPLY_PROP_InstatVolt,
    POWER_SUPPLY_PROP_BatteryAverageCurrent,
    POWER_SUPPLY_PROP_BatterySenseVoltage,
    POWER_SUPPLY_PROP_ISenseVoltage,
    POWER_SUPPLY_PROP_ChargerVoltage,
};

static int mt6573_ac_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
    int ret = 0;
    struct mt6573_ac_data *data = container_of(psy, struct mt6573_ac_data, psy);    

    switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
	    val->intval = data->AC_ONLINE;
	    break;
	default:
	    ret = -EINVAL;
	    break;
    }
    return ret;
}

static int mt6573_usb_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
    int ret = 0;
#if defined(CONFIG_POWER_EXT)
//#if 0
#else
    struct mt6573_usb_data *data = container_of(psy, struct mt6573_usb_data, psy);    
#endif

    switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:                           
		#if defined(CONFIG_POWER_EXT)
		//#if 0
		val->intval = 1;
		#else
	    val->intval = data->USB_ONLINE;
		#endif
	    break;
	default:
	    ret = -EINVAL;
	    break;
    }
    return ret;
}

static int mt6573_battery_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
    int ret = 0;     
    struct mt6573_battery_data *data = container_of(psy, struct mt6573_battery_data, psy);

    switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
	    val->intval = data->BAT_STATUS;
	    break;
	case POWER_SUPPLY_PROP_HEALTH:
	    val->intval = data->BAT_HEALTH;
	    break;
	case POWER_SUPPLY_PROP_PRESENT:
	    val->intval = data->BAT_PRESENT;
	    break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
	    val->intval = data->BAT_TECHNOLOGY;
	    break;
	case POWER_SUPPLY_PROP_CAPACITY:
	    val->intval = data->BAT_CAPACITY;
	    break;        
	case POWER_SUPPLY_PROP_batt_vol:
	    val->intval = data->BAT_batt_vol;
	    break;
	case POWER_SUPPLY_PROP_batt_temp:
	    val->intval = data->BAT_batt_temp;
	    break;
	case POWER_SUPPLY_PROP_TemperatureR:
	    val->intval = data->BAT_TemperatureR;
	    break;	
	case POWER_SUPPLY_PROP_TempBattVoltage:	    
	    val->intval = data->BAT_TempBattVoltage;
	    break;	
	case POWER_SUPPLY_PROP_InstatVolt:
	    val->intval = data->BAT_InstatVolt;
	    break;	
	case POWER_SUPPLY_PROP_BatteryAverageCurrent:
	    val->intval = data->BAT_BatteryAverageCurrent;
	    break;	
	case POWER_SUPPLY_PROP_BatterySenseVoltage:
	    val->intval = data->BAT_BatterySenseVoltage;
	    break;	
	case POWER_SUPPLY_PROP_ISenseVoltage:
	    val->intval = data->BAT_ISenseVoltage;
	    break;	
	case POWER_SUPPLY_PROP_ChargerVoltage:
	    val->intval = data->BAT_ChargerVoltage;
	    break;

	default:
	    ret = -EINVAL;
	    break;
    }

    return ret;
}

/* mt6573_ac_data initialization */
static struct mt6573_ac_data mt6573_ac_main = {
    .psy = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = mt6573_ac_props,
	.num_properties = ARRAY_SIZE(mt6573_ac_props),
	.get_property = mt6573_ac_get_property,                
    },
    .AC_ONLINE = 0,
};

/* mt6573_usb_data initialization */
static struct mt6573_usb_data mt6573_usb_main = {
    .psy = {
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.properties = mt6573_usb_props,
	.num_properties = ARRAY_SIZE(mt6573_usb_props),
	.get_property = mt6573_usb_get_property,                
    },
    .USB_ONLINE = 0,
};

/* mt6573_battery_data initialization */
static struct mt6573_battery_data mt6573_battery_main = {
    .psy = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = mt6573_battery_props,
	.num_properties = ARRAY_SIZE(mt6573_battery_props),
	.get_property = mt6573_battery_get_property,                
    },
#if defined(CONFIG_POWER_EXT)
//#if 0
    .BAT_STATUS = POWER_SUPPLY_STATUS_FULL,    
    .BAT_HEALTH = POWER_SUPPLY_HEALTH_GOOD,
    .BAT_PRESENT = 1,
    .BAT_TECHNOLOGY = POWER_SUPPLY_TECHNOLOGY_LION,
    .BAT_CAPACITY = 100,
    .BAT_batt_vol = 4200,
    .BAT_batt_temp = 22,
#else
    .BAT_STATUS = POWER_SUPPLY_STATUS_UNKNOWN,    
    .BAT_HEALTH = POWER_SUPPLY_HEALTH_UNKNOWN,
    .BAT_PRESENT = 0,
    .BAT_TECHNOLOGY = POWER_SUPPLY_TECHNOLOGY_UNKNOWN,
    .BAT_CAPACITY = 50,
    .BAT_batt_vol = 0,
    .BAT_batt_temp = 0,
#endif
};

static void mt6573_ac_update(struct mt6573_ac_data *ac_data)
{
    struct power_supply *ac_psy = &ac_data->psy;

	if( upmu_is_chr_det(CHR) == KAL_TRUE )
    {        
		if ( (BMT_status.charger_type == NONSTANDARD_CHARGER) || 
			 (BMT_status.charger_type == STANDARD_CHARGER)		)
		{
		    ac_data->AC_ONLINE = 1;        
		    ac_psy->type = POWER_SUPPLY_TYPE_MAINS;
		}
    }
    else
    {
		ac_data->AC_ONLINE = 0;        
    }

    power_supply_changed(ac_psy);    
}

static void mt6573_usb_update(struct mt6573_usb_data *usb_data)
{
    struct power_supply *usb_psy = &usb_data->psy;

	if( upmu_is_chr_det(CHR) == KAL_TRUE )		
    {
		if ( (BMT_status.charger_type == STANDARD_HOST) ||
			 (BMT_status.charger_type == CHARGING_HOST)		)
		{
		    usb_data->USB_ONLINE = 1;            
		    usb_psy->type = POWER_SUPPLY_TYPE_USB;            
		}
    }
    else
    {
		usb_data->USB_ONLINE = 0;
    }

    power_supply_changed(usb_psy); 
}

static void mt6573_battery_update(struct mt6573_battery_data *bat_data)
{
    struct power_supply *bat_psy = &bat_data->psy;
	int i;

    bat_data->BAT_TECHNOLOGY = POWER_SUPPLY_TECHNOLOGY_LION;
//    bat_data->BAT_HEALTH = POWER_SUPPLY_HEALTH_GOOD;
    bat_data->BAT_batt_vol = BMT_status.bat_vol;
    bat_data->BAT_batt_temp= BMT_status.temperature * 10;

//#if BATT_ID_CHECK_SUPPORT
#if 0	/*don't power off for compatible with IEEE1725*/
    if(BMT_status.bat_id_valid == 0)	/*batt id error*/
	bat_data->BAT_HEALTH = POWER_SUPPLY_HEALTH_DEAD;
    else
	bat_data->BAT_HEALTH = POWER_SUPPLY_HEALTH_GOOD;	
#else
    bat_data->BAT_HEALTH = POWER_SUPPLY_HEALTH_GOOD;
#endif

    if (BMT_status.bat_exist)
        bat_data->BAT_PRESENT = 1;
    else
        bat_data->BAT_PRESENT = 0;

    /* Charger and Battery Exist */
    //if( (upmu_is_chr_det(CHR)==KAL_TRUE) && (!g_Battery_Fail) )
    if( (upmu_is_chr_det(CHR)==KAL_TRUE) && (!g_Battery_Fail) && (g_Charging_Over_Time==0))
    {     
        if ( BMT_status.bat_exist )                
        {
            /* Battery Full */
            if ( (BMT_status.bat_vol >= RECHARGING_VOLTAGE) && (BMT_status.bat_full == KAL_TRUE) )
            {
            	/*Use no gas gauge*/
				if( (get_chip_eco_ver()==CHIP_E1) || (gForceADCsolution==1) )
				{
	                bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_FULL;
	                bat_data->BAT_CAPACITY = Battery_Percent_100;
					
					/* For user view */
					for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
						batterySOCBuffer[i] = 100; 
						batterySOCSum = 100 * BATTERY_AVERAGE_SIZE; /* for user view */
					}
					bat_volt_check_point = 100;
				}
				/*Use gas gauge*/
				else
				{					
					gSyncPercentage=1;
					bat_volt_check_point++;
					if(bat_volt_check_point>=100)
					{
						bat_volt_check_point=100;
						bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_FULL;						
					}
					bat_data->BAT_CAPACITY = bat_volt_check_point;
					
					if (Enable_BATDRV_LOG == 1) {
						printk("[Battery] In FULL Range (%d)\r\n", bat_volt_check_point);
					}

					if(get_chip_eco_ver()!=CHIP_E1)
					{
						gSyncPercentage=1;

						if (Enable_BATDRV_LOG == 1) {
							printk("[Battery_SyncRecharge] In recharging state, do not sync FG\r\n");
						}
					}
				}
            }
            /* battery charging */
            else 
            {
                /* Do re-charging for keep battery soc */
                if (g_bat_full_user_view) 
                {
                    bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_FULL;
                    bat_data->BAT_CAPACITY = Battery_Percent_100;

					/* For user view */
					for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
						batterySOCBuffer[i] = 100; 
						batterySOCSum = 100 * BATTERY_AVERAGE_SIZE; /* for user view */
					}
					bat_volt_check_point = 100;

					gSyncPercentage=1;
					if (Enable_BATDRV_LOG == 1) {
						printk("[Battery_Recharging] Keep UI as 100. bat_volt_check_point=%d, BMT_status.SOC=%ld\r\n", 
						bat_volt_check_point, BMT_status.SOC);
					}
                }
                else
                {
                    bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_CHARGING;   
                     /*add for not showing charging icon while OVP*/
                    if ( BMT_status.bat_charging_state == CHR_ERROR )   
	            { 
                         bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_DISCHARGING; 
                    }               

					/*Use no gas gauge*/
					if( (get_chip_eco_ver()==CHIP_E1) || (gForceADCsolution==1) )
					{
	                    /* SOC only UP when charging */
	                    if ( BMT_status.SOC > bat_volt_check_point ) {						
							bat_volt_check_point = BMT_status.SOC;
	                    } 
						bat_data->BAT_CAPACITY = bat_volt_check_point;
					}
					/*Use gas gauge*/
					else
					{
						if(bat_volt_check_point >= 100 )
						{					
							bat_volt_check_point=99;
							//BMT_status.SOC=99;
							gSyncPercentage=1;

							//if (Enable_BATDRV_LOG == 1) {
								printk("[Battery] Use gas gauge : gas gague get 100 first (%d)\r\n", bat_volt_check_point);
							//}
						}
						else
						{
							if(bat_volt_check_point == BMT_status.SOC)
							{
								gSyncPercentage=0;

								if (Enable_BATDRV_LOG == 1) {
									printk("[Battery] Can sync due to bat_volt_check_point=%d, BMT_status.SOC=%ld\r\n", 
									bat_volt_check_point, BMT_status.SOC);
								}
							}
							else
							{
								if (Enable_BATDRV_LOG == 1) {
									printk("[Battery] Keep UI due to bat_volt_check_point=%d, BMT_status.SOC=%ld\r\n", 
									bat_volt_check_point, BMT_status.SOC);
								}
							}
						}
						bat_data->BAT_CAPACITY = bat_volt_check_point;						
					}
                }
            }
        }
        /* No Battery, Only Charger */
        else
        {
            bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_UNKNOWN;
            bat_data->BAT_CAPACITY = 0;
        }
        
    }
    /* Only Battery */
    else
    {
        bat_data->BAT_STATUS = POWER_SUPPLY_STATUS_NOT_CHARGING;

        /* If VBAT < CLV, then shutdown */
        if (BMT_status.bat_vol <= SYSTEM_OFF_VOLTAGE)
        {   
        	/*Use no gas gauge*/
			if( (get_chip_eco_ver()==CHIP_E1) || (gForceADCsolution==1) )
			{
	            printk(  "[BAT BATTERY] VBAT < %d mV : Android will Power Off System !!\r\n", SYSTEM_OFF_VOLTAGE);                              
	            bat_data->BAT_CAPACITY = 0;
			}
			/*Use gas gauge*/
			else
			{
				gSyncPercentage=1;				
				bat_volt_check_point--;
				if(bat_volt_check_point <= 0)
				{
					bat_volt_check_point=0;
				}
				bat_data->BAT_CAPACITY = bat_volt_check_point;
				printk("[Battery] VBAT < %d mV (%d)\r\n", SYSTEM_OFF_VOLTAGE, bat_volt_check_point);
			}
        }
		/* If FG_VBAT <= gFG_15_vlot, then run to 15% */
		else if ( (gFG_voltage <= gFG_15_vlot)&&(gForceADCsolution==0)&&(bat_volt_check_point>=15) )
		{
			/*Use gas gauge*/
			gSyncPercentage=1;			
			if(gBAT_counter_15==0)
			{
				bat_volt_check_point--;
				gBAT_counter_15=1;
			}		
			else
			{
				gBAT_counter_15=0;
			}
			g_Calibration_FG = 0;
    		FGADC_Reset_SW_Parameter();
			gFG_DOD0=100-bat_volt_check_point;
			gFG_DOD1=gFG_DOD0;
			BMT_status.SOC=bat_volt_check_point;
			bat_data->BAT_CAPACITY = bat_volt_check_point;
			printk("[Battery] FG_VBAT <= %d, then SOC run to 15. (SOC=%ld,Point=%d,D1=%d,D0=%d)\r\n", 
				gFG_15_vlot, BMT_status.SOC, bat_volt_check_point, gFG_DOD1, gFG_DOD0);
		}
        else
        {      	
        	gBAT_counter_15=1;
			/*Use no gas gauge*/
			if( (get_chip_eco_ver()==CHIP_E1) || (gForceADCsolution==1) )
			{
				/* SOC only Done when dis-charging */
	            if ( BMT_status.SOC < bat_volt_check_point ) {
					bat_volt_check_point = BMT_status.SOC;
	            }
				bat_data->BAT_CAPACITY = bat_volt_check_point;            
			}
			/*Use gas gauge : gas gague get 0% fist*/
			else
			{	
				if (Enable_BATDRV_LOG == 1) {
					printk("[Battery_OnlyBattery!] bat_volt_check_point=%d,BMT_status.SOC=%ld\r\n", 
					bat_volt_check_point, BMT_status.SOC);
				}
				
				//if(bat_volt_check_point != BMT_status.SOC)
				//if(bat_volt_check_point > BMT_status.SOC)
				if( (bat_volt_check_point>BMT_status.SOC) && ((bat_volt_check_point!=1)) )
				{		
					if (Enable_BATDRV_LOG == 1) {
						printk("[Battery_OnlyBattery] bat_volt_check_point=%d,BMT_status.SOC=%ld,gFGsyncTimer=%d(on %d)\r\n", 
						bat_volt_check_point, BMT_status.SOC, gFGsyncTimer, DEFAULT_SYNC_TIME_OUT);
					}
					
					//reduce after xxs
					if(gFGsyncTimer >= DEFAULT_SYNC_TIME_OUT)
					{
						gFGsyncTimer=0;
						bat_volt_check_point--;
						bat_data->BAT_CAPACITY = bat_volt_check_point;
					}
					else
					{
						gFGsyncTimer+=10;
					}
				}
				else
				{				
					if(bat_volt_check_point <= 0 )
					{					
						bat_volt_check_point=1;
						//BMT_status.SOC=1;
						gSyncPercentage=1;

						//if (Enable_BATDRV_LOG == 1) {
							printk("[Battery] Use gas gauge : gas gague get 0 first (%d)\r\n", bat_volt_check_point);
						//}
					}
					else
					{
						gSyncPercentage=0;
					}

					if(bat_volt_check_point > 100)
					{
						bat_volt_check_point=100;
					}
					
					bat_data->BAT_CAPACITY = bat_volt_check_point;
				}

				if(bat_volt_check_point == 100) {
        			g_bat_full_user_view = KAL_TRUE;
					if (Enable_BATDRV_LOG == 1) {
						printk("[Battery_Only] Set g_bat_full_user_view=KAL_TRUE\r\n");
					}
				}
			}
        }
    }    

	if (Enable_BATDRV_LOG == 1) {
		printk("[BATTERY:IntegrationFG:point,per_C,per_V,count,vbat_charger,CSDAC_DAT] %d,%ld,%d,%d,%ld,%d\r\n", 
		bat_volt_check_point, BMT_status.SOC, FGADC_Get_BatteryCapacity_VoltageMothod(), 
		BATTERY_AVERAGE_SIZE, BMT_status.bat_vol, CSDAC_DAT);	
	}
	
	/* Update for EM */
	bat_data->BAT_TemperatureR=g_BAT_TemperatureR;
	bat_data->BAT_TempBattVoltage=g_TempBattVoltage;
	bat_data->BAT_InstatVolt=g_InstatVolt;
	bat_data->BAT_BatteryAverageCurrent=g_BatteryAverageCurrent;
	bat_data->BAT_BatterySenseVoltage=g_BAT_BatterySenseVoltage;
	bat_data->BAT_ISenseVoltage=g_BAT_ISenseVoltage;
	bat_data->BAT_ChargerVoltage=g_BAT_ChargerVoltage;
    
    power_supply_changed(bat_psy);    
}

typedef struct{
    INT32 BatteryTemp;
    INT32 TemperatureR;
}BATT_TEMPERATURE;

/* convert register to temperature  */
INT16 BattThermistorConverTemp(INT32 Res)
{
    int i=0;
    INT32 RES1=0,RES2=0;
    INT32 TBatt_Value=-200,TMP1=0,TMP2=0;

#if (BAT_NTC_10 == 1)
    /// qiang_li_checked_for_ALPS00062960 , need not to modify
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,68237},
        {-15,53650},
        {-10,42506},
        { -5,33892},
        {  0,27219},
        {  5,22021},
        { 10,17926},
        { 15,14674},
        { 20,12081},
        { 25,10000},
        { 30,8315},
        { 35,6948},
        { 40,5834},
        { 45,4917},
        { 50,4161},
        { 55,3535},
        { 60,3014}
    };
#endif

#if (BAT_NTC_47 == 1)
		BATT_TEMPERATURE Batt_Temperature_Table[] = {
			{-20,483954},
			{-15,360850},
			{-10,271697},
			{ -5,206463},
			{  0,158214},
			{  5,122259},
			{ 10,95227},
			{ 15,74730},
			{ 20,59065},
			{ 25,47000},
			{ 30,37643},
			{ 35,30334},
			{ 40,24591},
			{ 45,20048},
			{ 50,16433},
			{ 55,13539},
			{ 60,11210}		
		};
#endif

    if(Res>=Batt_Temperature_Table[0].TemperatureR)
    {
        #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
        printk("Res>=%d\n", Batt_Temperature_Table[0].TemperatureR);
        #endif
        TBatt_Value = -20;
    }
    else if(Res<=Batt_Temperature_Table[16].TemperatureR)
    {
        #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
        printk("Res<=%d\n", Batt_Temperature_Table[16].TemperatureR);
        #endif
        TBatt_Value = 60;
    }
    else
    {
        RES1=Batt_Temperature_Table[0].TemperatureR;
        TMP1=Batt_Temperature_Table[0].BatteryTemp;

        for(i=0;i<=16;i++)
        {
            if(Res>=Batt_Temperature_Table[i].TemperatureR)
            {
                RES2=Batt_Temperature_Table[i].TemperatureR;
                TMP2=Batt_Temperature_Table[i].BatteryTemp;
                break;
            }
            else
            {
                RES1=Batt_Temperature_Table[i].TemperatureR;
                TMP1=Batt_Temperature_Table[i].BatteryTemp;
            }
        }
        
        TBatt_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))/(RES1-RES2);
    }

    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printk("BattThermistorConverTemp() : TBatt_Value = %d\n",TBatt_Value);
	printk("BattThermistorConverTemp() : Res = %d\n",Res);
	printk("BattThermistorConverTemp() : RES1 = %d\n",RES1);
	printk("BattThermistorConverTemp() : RES2 = %d\n",RES2);
	printk("BattThermistorConverTemp() : TMP1 = %d\n",TMP1);
	printk("BattThermistorConverTemp() : TMP2 = %d\n",TMP2);
    #endif

    return TBatt_Value;    
}

/* convert ADC_bat_temp_volt to register */
INT16 BattVoltToTemp(UINT32 dwVolt)
{
    INT32 TRes;
    INT32 dwVCriBat = (TBAT_OVER_CRITICAL_LOW*RBAT_PULL_UP_VOLT)/(TBAT_OVER_CRITICAL_LOW+RBAT_PULL_UP_R); //~2000mV
    INT32 sBaTTMP = -100;

    if(dwVolt > dwVCriBat)
        TRes = TBAT_OVER_CRITICAL_LOW;
    else
        TRes = (RBAT_PULL_UP_R*dwVolt)/(RBAT_PULL_UP_VOLT-dwVolt);        

	g_BAT_TemperatureR = TRes;

    /* convert register to temperature */
    sBaTTMP = BattThermistorConverTemp(TRes);

    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printk("BattVoltToTemp() : TBAT_OVER_CRITICAL_LOW = %d\n", TBAT_OVER_CRITICAL_LOW);
	printk("BattVoltToTemp() : RBAT_PULL_UP_VOLT = %d\n", RBAT_PULL_UP_VOLT);
    printk("BattVoltToTemp() : dwVolt = %d\n", dwVolt);
    printk("BattVoltToTemp() : TRes = %d\n", TRes);
    printk("BattVoltToTemp() : sBaTTMP = %d\n", sBaTTMP);
    #endif
    
    return sBaTTMP;
}

//void BAT_SetUSBState(int usb_state_value)
void BATTERY_SetUSBState(int usb_state_value)
{
	if ( (usb_state_value < USB_SUSPEND) || ((usb_state_value > USB_CONFIGURED))){
		printk("[BATTERY] BAT_SetUSBState Fail! Restore to default value\r\n");	
		usb_state_value = USB_UNCONFIGURED;
	} else {
		printk("[BATTERY] BAT_SetUSBState Success! Set %d\r\n", usb_state_value);	
		g_usb_state = usb_state_value;	
	}
}
//EXPORT_SYMBOL(BAT_SetUSBState);
EXPORT_SYMBOL(BATTERY_SetUSBState);

///////////////////////////////////////////////////////////////////////////////////////////
//// Pulse Charging Algorithm 
///////////////////////////////////////////////////////////////////////////////////////////
void select_charging_curret(void)
{
	if (g_ftm_battery_flag) 
	{
		printk("[BATTERY] FTM charging : %d\r\n", charging_level_data[0]);	
        g_temp_CC_value = charging_level_data[0];                
    }
	else 
	{    
        if ( BMT_status.charger_type == STANDARD_HOST ) 
		{

			if (g_Support_USBIF == 1)
			{
				if (g_usb_state == USB_SUSPEND)
				{
					g_temp_CC_value = USB_CHARGER_CURRENT_SUSPEND;
				}
				else if (g_usb_state == USB_UNCONFIGURED)
				{
					g_temp_CC_value = USB_CHARGER_CURRENT_UNCONFIGURED;
				}
				else if (g_usb_state == USB_CONFIGURED)
				{
					g_temp_CC_value = USB_CHARGER_CURRENT_CONFIGURED;
				}
				else
				{
					g_temp_CC_value = USB_CHARGER_CURRENT_UNCONFIGURED;
				}
				
				if (Enable_BATDRV_LOG == 1) {
					printk("[BATTERY] Support BC1.1\r\n");
	            	printk("[BATTERY] STANDARD_HOST CC mode charging : %d on %d state\r\n", g_temp_CC_value, g_usb_state);
	            }
			}
			else
			{	
				g_temp_CC_value = USB_CHARGER_CURRENT;			
				
				if (Enable_BATDRV_LOG == 1) {
					printk("[BATTERY] Not Support BC1.1\r\n");
	            	printk("[BATTERY] STANDARD_HOST CC mode charging : %d\r\n", USB_CHARGER_CURRENT);
	            }
			}
        } 
		else if (BMT_status.charger_type == NONSTANDARD_CHARGER) 
		{   
			g_temp_CC_value = USB_CHARGER_CURRENT;
			
            if (Enable_BATDRV_LOG == 1) {
               	printk("[BATTERY] NONSTANDARD_CHARGER CC mode charging : %d\r\n", USB_CHARGER_CURRENT); // USB HW limitation
            }
        } 
		else if (BMT_status.charger_type == STANDARD_CHARGER) 
        {
        	g_temp_CC_value = AC_CHARGER_CURRENT;
			
            if (Enable_BATDRV_LOG == 1) {
            	printk("[BATTERY] STANDARD_CHARGER CC mode charging : %d\r\n", AC_CHARGER_CURRENT);
            }           
		}
		else if (BMT_status.charger_type == CHARGING_HOST) 
        {
        	g_temp_CC_value = AC_CHARGER_CURRENT;
			
            if (Enable_BATDRV_LOG == 1) {
            	printk("[BATTERY] CHARGING_HOST CC mode charging : %d\r\n", AC_CHARGER_CURRENT);
            }           
		}
		else 
		{
        	g_temp_CC_value = Cust_CC_70MA;
			
            if (Enable_BATDRV_LOG == 1) {
            	printk("[BATTERY] Default CC mode charging : %d\r\n", Cust_CC_70MA);
            }            
        }
        
    }
}

void ChargerHwInit(void)
{
	//printk("[MT6573 BAT_probe] ChargerHwInit\n" );
	upmu_csdac_dly(CHR,0x3);
	upmu_csdac_stp(CHR,0x0);
	//printk("[MT6573 BAT_probe] REG[0xF702FA0C]=%x\r\n", INREG16(0xF702FA0C));
}

void pchr_turn_off_charging (void)
{
	if (Enable_BATDRV_LOG == 1) {
		printk("[BATTERY] pchr_turn_off_charging !\r\n");
	}
	
	upmu_chrwdt_int_enable(CHR, 0); 			// CHRWDT_INT_EN
	upmu_chrwdt_enable(CHR, 0); 				// CHRWDT_EN
	upmu_chrwdt_flag(CHR, 0);					// CHRWDT_FLAG
	upmu_csdac_enable(CHR, KAL_FALSE);			// CSDAC_EN
	upmu_chr_enable(CHR, KAL_FALSE); 			// CHR_EN	

	if(get_chip_eco_ver()!=CHIP_E1)
	{
		//for BC1.1 circuit
		upmu_bc11_bb_crtl_enable(CHR, KAL_TRUE);
		upmu_bc11_reset_circuit(CHR, 0x0);
	}
}

extern kal_int32 gFG_booting_counter_I_FLAG;

void pchr_turn_on_charging (void)
{
#if BATT_ID_CHECK_SUPPORT
	if (( BMT_status.bat_charging_state == CHR_ERROR ) || (BMT_status.bat_id_valid == 0)) 
#else
	if ( BMT_status.bat_charging_state == CHR_ERROR ) 
#endif
	{
		if (Enable_BATDRV_LOG == 1) {
	#if BATT_ID_CHECK_SUPPORT
			printk("[BATTERY] Charger Error,or batt_id:%d turn OFF charging !\r\n",BMT_status.bat_id_valid);
	#endif
		}
		pchr_turn_off_charging();
	}
	else if( (get_boot_mode()==META_BOOT) || (get_boot_mode()==ADVMETA_BOOT) )
	{
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] In meta or advanced meta mode, disable charging.\r\n");
		}
		pchr_turn_off_charging();
	}
	else
	{
		ChargerHwInit();
	
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] pchr_turn_on_charging !\r\n");
		}

		select_charging_curret();
		
		if( g_temp_CC_value == Cust_CC_0MA)
		{
			pchr_turn_off_charging();
			printk("[BATTERY] charging current is set 0mA !\r\n");
		}
		else
		{			
			upmu_chr_current(CHR, g_temp_CC_value); 	// CS_VTH
			
			upmu_cs_enable(CHR, KAL_TRUE);				// CS_EN
			upmu_vbat_cc_vth(CHR, g_PMIC_CC_VTH);		// CC_VTH
			if(get_chip_eco_ver()==CHIP_E1)
			{
				upmu_vbat_cv_vth(CHR, g_PMIC_CV_VTH);		// CV_VTH
			}
			else
			{
				upmu_vbat_cv_vth(CHR, PMIC_ADPT_VOLT_04_200000_V);	// VBAT_CV_VTH = 0x10 = 4.2V
			}			
			upmu_vbat_cc_det_enable(CHR, KAL_TRUE);		// CC_EN
			upmu_vbat_cv_det_enable(CHR, KAL_TRUE);		// CV_EN
			
			upmu_chrwdt_td(CHR, 0x3);					// CHRWDT_TD 32s
			upmu_chrwdt_int_enable(CHR, 1);				// CHRWDT_INT_EN
			upmu_chrwdt_enable(CHR, 1);					// CHRWDT_EN
			upmu_chrwdt_flag(CHR, 1);					// CHRWDT_FLAG

			upmu_csdac_enable(CHR, KAL_TRUE);			// CSDAC_EN
			//upmu_chr_enable(CHR, KAL_TRUE);				// CHR_EN

			if(get_chip_eco_ver()==CHIP_E1)
			{
				upmu_chr_enable(CHR, KAL_TRUE); // CHR_EN
			}
			else
			{
				if(gFG_booting_counter_I_FLAG == 2)
				{
					upmu_chr_enable(CHR, KAL_TRUE); // CHR_EN
				}
				else
				{
					pchr_turn_off_charging();
					printk("[BATTERY] wait gFG_booting_counter_I_FLAG==2 (%d)\r\n", gFG_booting_counter_I_FLAG);
				}
			}
		}	

	if(get_chip_eco_ver()!=CHIP_E1)
	{
		//for BC1.1 circuit
		upmu_bc11_bb_crtl_enable(CHR, KAL_TRUE);
		upmu_bc11_reset_circuit(CHR, 0x1);
	}
}

}

void SW_CV_Algo_prepare(void)
{
	if (Enable_BATDRV_LOG == 1) {
		printk(  "[BATTERY] SW_CV_Algo_prepare:%d\n\r",g_temp_CC_value);
	}

	if( g_temp_CC_value == Cust_CC_0MA)
	{
		pchr_turn_off_charging();
	}
	else
	{
		upmu_pchr_csdac_test_enable(CHR, KAL_TRUE); 		// CSDAC_TEST = 1
		upmu_pchr_test_csdac_dat(CHR, 0);          			// CSDAC_DAT = 0
		
		upmu_csdac_enable(CHR, KAL_TRUE);					// CSDAC_EN = 1
		upmu_chr_enable(CHR, KAL_TRUE);						// CHR_EN = 1

		upmu_chr_current(CHR, g_temp_CC_value); 			// CS_VTH

		//upmu_pchr_ft_ctrl(CHR,0x5);							// PCHR_FT_CTRL[6:4]=101
		SETREG16(PMIC_RESERVE_CON1,0x0001); 				// [0] CV_MODE=1
		//SETREG16(PMIC_RESERVE_CON1,0x0002); 				// [1] VCDT_MODE=1
		CLRREG16(PMIC_RESERVE_CON1,0x0002); 				// [1] VCDT_MODE=0		

		upmu_vbat_cv_vth(CHR, PMIC_ADPT_VOLT_04_200000_V);	// VBAT_CV_VTH = 0x10 = 4.2V
		upmu_vbat_cv_det_enable(CHR, KAL_TRUE);				// VBAT_CV_EN = 1
		 		
	}	
}

void SW_CV_Algo_task(void)
{
	if (Enable_BATDRV_LOG == 1) {
		//printk(  "[BATTERY] SW_CV_Algo_task---------------%x,%x\n\r", INREG16(CHR_CON_0), INREG16(CHR_CON_1));
	}

	upmu_chrwdt_td(CHR, 0x3);					// CHRWDT_TD 32s
	upmu_chrwdt_int_enable(CHR, 1);				// CHRWDT_INT_EN
	upmu_chrwdt_enable(CHR, 1);					// CHRWDT_EN
	upmu_chrwdt_flag(CHR, 1);					// CHRWDT_FLAG

	save_value = INREG16(CHR_CON_1);
	VBAT_CV_DET = (save_value & 0x4000) >> 14;

	if (Enable_BATDRV_LOG == 1) {
		//printk(  "[BATTERY] VBAT_CV_DET=%d\n\r", VBAT_CV_DET);
	}

	save_value = INREG16(CHR_CON_2);
	CS_DET = (save_value & 0x8000) >> 15;
	
	if (Enable_BATDRV_LOG == 1) {
		//printk(  "[BATTERY] CS_DET=%d\n\r", CS_DET);
	}

	if(VBAT_CV_DET==1)
	{
		if(CSDAC_DAT > 0)
		{
			CSDAC_DAT--;
		}							
	}
	else if(CS_DET==1)
	{
		save_value = INREG16(CHR_CON_4);
		CSDAC_DAT_MAX = (save_value & 0xFF00) >> 8;
		
		if (Enable_BATDRV_LOG == 1) {
			//printk(  "[CS_DET==1] CSDAC_DAT_MAX=%d(%x)\n\r", CSDAC_DAT_MAX, save_value);
		}

		if(CSDAC_DAT > 0)
		{
			CSDAC_DAT--;
		}
	}
	else if( (VBAT_CV_DET==0) && (CS_DET==0) )	
	{
		if(CSDAC_DAT < CSDAC_DAT_MAX)
		{
			CSDAC_DAT++;
		}
	}
	else
	{
	}	
	upmu_pchr_test_csdac_dat(CHR, CSDAC_DAT);	

	if (Enable_BATDRV_LOG == 1) {
		//dump register
		//printk("CHR_CON_0 = %x\n", INREG16(CHR_CON_0));	
		//printk("CHR_CON_1 = %x\n", INREG16(CHR_CON_1));
		//printk("CHR_CON_2 = %x\n", INREG16(CHR_CON_2));
		//printk("CHR_CON_4 = %x\n", INREG16(CHR_CON_4));
		//printk("PMIC_RESERVE_CON1 = %x\n", INREG16(PMIC_RESERVE_CON1));
	}
}

int BAT_CheckPMUStatusReg(void)
{ 
    if( upmu_is_chr_det(CHR) == KAL_TRUE )
    {
        BMT_status.charger_exist = TRUE;
    }
    else
    {   
        BMT_status.charger_exist = FALSE;
		
		BMT_status.total_charging_time = 0;
		BMT_status.PRE_charging_time = 0;
		BMT_status.CC_charging_time = 0;
		BMT_status.TOPOFF_charging_time = 0;
		BMT_status.POSTFULL_charging_time = 0;

        BMT_status.bat_charging_state = CHR_PRE;        
		
        return PMU_STATUS_FAIL;
    }  

	return PMU_STATUS_OK;
}

unsigned long BAT_Get_Battery_Voltage(void)
{
	unsigned long ret_val = 0;
	
	upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	ret_val=IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,20) * R_BAT_SENSE;

	return ret_val;
}

#if 0
int g_Get_I_Charging(void)
{
	int ADC_I_SENSE=0;
	int ADC_BAT_SENSE=0;
	int ICharging=0;	
	
	//upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	//upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
	ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,20) * R_BAT_SENSE;
	ADC_I_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,20) * R_I_SENSE;
	if(ADC_I_SENSE > ADC_BAT_SENSE)
	{
    	ICharging = (ADC_I_SENSE - ADC_BAT_SENSE)*10/R_CURRENT_SENSE;
	}
	else
	{
    	ICharging = 0;
	}
	return ICharging;
}
#endif

//new adc sampling algo.
int g_Get_I_Charging(void)
{
	kal_int32 ADC_BAT_SENSE_tmp[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	kal_int32 ADC_BAT_SENSE_sum=0;
	kal_int32 ADC_BAT_SENSE=0;
	kal_int32 ADC_I_SENSE_tmp[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	kal_int32 ADC_I_SENSE_sum=0;
	kal_int32 ADC_I_SENSE=0;	
	int repeat=20;
	int i=0;
	int j=0;
	kal_int32 temp=0;
	int ICharging=0;	

	for(i=0 ; i<repeat ; i++)
	{
		ADC_BAT_SENSE_tmp[i] = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,1)*R_BAT_SENSE;
		ADC_I_SENSE_tmp[i] = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,1)*R_I_SENSE;
	
		ADC_BAT_SENSE_sum += ADC_BAT_SENSE_tmp[i];
		ADC_I_SENSE_sum += ADC_I_SENSE_tmp[i];	
	}

	//sorting	BAT_SENSE 
	for(i=0 ; i<repeat ; i++)
	{
		for(j=i; j<repeat ; j++)
		{
			if( ADC_BAT_SENSE_tmp[j] < ADC_BAT_SENSE_tmp[i] )
			{
				temp = ADC_BAT_SENSE_tmp[j];
				ADC_BAT_SENSE_tmp[j] = ADC_BAT_SENSE_tmp[i];
				ADC_BAT_SENSE_tmp[i] = temp;
			}
		}
	}
	#if 0
	if (Enable_BATDRV_LOG == 1) {
		printk("[g_Get_I_Charging:BAT_SENSE]\r\n");	
		for(i=0 ; i<repeat ; i++ )
		{
			printk("%d,", ADC_BAT_SENSE_tmp[i]);
		}
		printk("\r\n");
	}
	#endif

	//sorting	I_SENSE 
	for(i=0 ; i<repeat ; i++)
	{
		for(j=i ; j<repeat ; j++)
		{
			if( ADC_I_SENSE_tmp[j] < ADC_I_SENSE_tmp[i] )
			{
				temp = ADC_I_SENSE_tmp[j];
				ADC_I_SENSE_tmp[j] = ADC_I_SENSE_tmp[i];
				ADC_I_SENSE_tmp[i] = temp;
			}
		}
	}
	#if 0
	if (Enable_BATDRV_LOG == 1) {
		printk("[g_Get_I_Charging:I_SENSE]\r\n");	
		for(i=0 ; i<repeat ; i++ )
		{
			printk("%d,", ADC_I_SENSE_tmp[i]);
		}
		printk("\r\n");
	}
	#endif
		
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[0];
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[1];
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[18];
	ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[19];		
	ADC_BAT_SENSE = ADC_BAT_SENSE_sum / (repeat-4);

	#if 0
	if (Enable_BATDRV_LOG == 1) {
		printk("[g_Get_I_Charging] ADC_BAT_SENSE=%d\r\n", ADC_BAT_SENSE);
	}
	#endif

	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[0];
	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[1];
	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[18];
	ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[19];
	ADC_I_SENSE = ADC_I_SENSE_sum / (repeat-4);

	#if 0
	if (Enable_BATDRV_LOG == 1) {
		printk("[g_Get_I_Charging] ADC_I_SENSE(Before)=%d\r\n", ADC_I_SENSE);
	}
	#endif
	
	ADC_I_SENSE += gADC_I_SENSE_offset;

	#if 0
	if (Enable_BATDRV_LOG == 1) {
		printk("[g_Get_I_Charging] ADC_I_SENSE(After)=%d\r\n", ADC_I_SENSE);
	}
	#endif

	BMT_status.ADC_BAT_SENSE = ADC_BAT_SENSE;
	BMT_status.ADC_I_SENSE = ADC_I_SENSE;
	
	if(ADC_I_SENSE > ADC_BAT_SENSE)
	{
    	ICharging = (ADC_I_SENSE - ADC_BAT_SENSE)*10/R_CURRENT_SENSE;
	}
	else
	{
    	ICharging = 0;
	}

	return ICharging;
}

void BAT_GetVoltage(void)
{ 
	int bat_temperature_volt=0;
#if BATT_ID_CHECK_SUPPORT
	int bat_id_volt = 0;
#endif

	upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vchr_enable(CHR, KAL_TRUE);

	/* Get V_BAT_SENSE */
	if (g_chr_event == 0) 
	{    	
		BMT_status.ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,20) * R_BAT_SENSE;		
	} 
	else 
	{
		/* Just charger in/out event, same as I_sense */
		g_chr_event = 0;		
		BMT_status.ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,20) * R_I_SENSE;
	}
#if defined(CONFIG_POWER_EXT)	
	BMT_status.ADC_BAT_SENSE += V_compensate_EVB;
#endif	
	BMT_status.bat_vol = BMT_status.ADC_BAT_SENSE;

#if 0
	/* Get V_I_SENSE */	
	BMT_status.ADC_I_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,20) * R_I_SENSE;
#if defined(CONFIG_POWER_EXT)
	BMT_status.ADC_I_SENSE += V_compensate_EVB;
#endif
#endif

	/* Get V_Charger */
	BMT_status.charger_vol = IMM_GetOneChannelValue(AUXADC_CHARGER_VOLTAGE_CHANNEL,5) * (((R_CHARGER_1+R_CHARGER_2)*100)/R_CHARGER_2);
	BMT_status.charger_vol = BMT_status.charger_vol / 100;

	/* Get V_BAT_Temperature */
	bat_temperature_volt = IMM_GetOneChannelValue(AUXADC_TEMPERATURE_CHANNEL,5); 

#if BATT_ID_CHECK_SUPPORT
	/*Get battery id voltage*/
	bat_id_volt = IMM_GetOneChannelValue(AUXADC_BATT_ID_CHANNEL,5); 	/**/
	if((bat_id_volt < CHG_MIN_VALID_BATT_ID) || (bat_id_volt > CHG_MAX_VALID_BATT_ID))	/*if battery id isn't within valid range*/
		{
		  BMT_status.bat_id_valid = 0;
		}
	else
		BMT_status.bat_id_valid = 1;

    if (Enable_BATDRV_LOG == 1) {
        printk("jrd_enter %s [BATTERY:ADC] batt_temp_volt:%ld BAT_id_volt:%ld\n",__func__, bat_temperature_volt, bat_id_volt);
    }
#endif

#if defined(CONFIG_POWER_EXT)	
//#if 0
    BMT_status.temperature = 23;    
#else
	//if ( g_BatTempProtectEn == 1 ) 
	//{	
		BMT_status.temperature = BattVoltToTemp(bat_temperature_volt);        			
	//}
#endif	

    /* Data Calibration  */    
    if (g_ADC_Cali) {
		if (Enable_BATDRV_LOG == 1) {
        	printk("Before Cal : %ld(B) , %ld(I) \r\n", BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE);
		}
		
        //BMT_status.ADC_I_SENSE = ((BMT_status.ADC_I_SENSE * (*(adc_cali_slop+0)))+(*(adc_cali_offset+0)))/1000;
        //BMT_status.ADC_BAT_SENSE = ((BMT_status.ADC_BAT_SENSE * (*(adc_cali_slop+1)))+(*(adc_cali_offset+1)))/1000;

		BMT_status.ADC_I_SENSE = ((BMT_status.ADC_I_SENSE * (*(adc_cali_slop+1)))+(*(adc_cali_offset+1)))/1000;
        BMT_status.ADC_BAT_SENSE = ((BMT_status.ADC_BAT_SENSE * (*(adc_cali_slop+0)))+(*(adc_cali_offset+0)))/1000;
		
		if (Enable_BATDRV_LOG == 1) {
        	printk("After Cal : %ld(B) , %ld(I) \r\n", BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE);
		}
    }

#if 0
	/* Calculate the charging current */
	if(BMT_status.ADC_I_SENSE > BMT_status.ADC_BAT_SENSE)
        BMT_status.ICharging = (BMT_status.ADC_I_SENSE - BMT_status.ADC_BAT_SENSE)*10/R_CURRENT_SENSE;
    else
        BMT_status.ICharging = 0;    
#endif
	BMT_status.ICharging = g_Get_I_Charging();

    if (Enable_BATDRV_LOG == 1) {
    	printk("[BATTERY:ADC] VCHR:%ld BAT_SENSE:%ld I_SENSE:%ld Current:%ld CAL:%d\n", BMT_status.charger_vol,
            BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE, BMT_status.ICharging, g_ADC_Cali );
    }

	g_InstatVolt = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,20);
	g_BatteryAverageCurrent = BMT_status.ICharging;
	g_BAT_BatterySenseVoltage = BMT_status.ADC_BAT_SENSE;
	g_BAT_ISenseVoltage = BMT_status.ADC_I_SENSE;
	g_BAT_ChargerVoltage = BMT_status.charger_vol;
   
}

UINT32 BattVoltToPercent(UINT16 dwVoltage)
{
    UINT32 m=0;
    UINT32 VBAT1=0,VBAT2=0;
    UINT32 bPercntResult=0,bPercnt1=0,bPercnt2=0;

	if (Enable_BATDRV_LOG == 1) {
    	printk("###### 100 <-> voltage : %d ######\r\n", Batt_VoltToPercent_Table[10].BattVolt);
	}
    
    if(dwVoltage<=Batt_VoltToPercent_Table[0].BattVolt)
    {
        bPercntResult = Batt_VoltToPercent_Table[0].BattPercent;
        return bPercntResult;
    }
    else if (dwVoltage>=Batt_VoltToPercent_Table[10].BattVolt)
    {
        bPercntResult = Batt_VoltToPercent_Table[10].BattPercent;
        return bPercntResult;
    }
    else
    {        
        VBAT1 = Batt_VoltToPercent_Table[0].BattVolt;
        bPercnt1 = Batt_VoltToPercent_Table[0].BattPercent;
        for(m=1;m<=10;m++)
        {
            if(dwVoltage<=Batt_VoltToPercent_Table[m].BattVolt)
            {
                VBAT2 = Batt_VoltToPercent_Table[m].BattVolt;
                bPercnt2 = Batt_VoltToPercent_Table[m].BattPercent;
                break;
            }
            else
            {
                VBAT1 = Batt_VoltToPercent_Table[m].BattVolt;
                bPercnt1 = Batt_VoltToPercent_Table[m].BattPercent;    
            }
        }
    }
    
    bPercntResult = ( ((dwVoltage-VBAT1)*bPercnt2)+((VBAT2-dwVoltage)*bPercnt1) ) / (VBAT2-VBAT1);    

    return bPercntResult;
    
}

int getVoltFlag = 0;

int BAT_CheckBatteryStatus(void)
{
    int BAT_status = PMU_STATUS_OK;
    int i = 0;

	/* Get Battery Information */
    BAT_GetVoltage();

	/*Charging 9s and discharging 1s : start*/
	if(get_chip_eco_ver()==CHIP_E1)
	{
		if( (upmu_is_chr_det(CHR) == KAL_TRUE) && 
			//(BMT_status.bat_full == KAL_FALSE) &&
			(g_HW_Charging_Done == 0) &&
			(BMT_status.bat_charging_state != CHR_ERROR)	) 
		{
			g_HW_stop_charging = 1;
		
			if (Enable_BATDRV_LOG == 1) {
				printk("[BATTERY] Dis Charging 1s\n\r");
			}
			pchr_turn_off_charging();
			getVoltFlag = 1;
			msleep(g_free_bat_temp);
		}
	}
	else
	{
		if( (upmu_is_chr_det(CHR) == KAL_TRUE) && 
			//(BMT_status.bat_full == KAL_FALSE) &&
			(g_HW_Charging_Done == 0) &&
			(BMT_status.bat_charging_state != CHR_ERROR) &&
			(BMT_status.bat_charging_state != CHR_TOP_OFF)) 
		{
			g_HW_stop_charging = 1;
		
			if (Enable_BATDRV_LOG == 1) {
				printk("[BATTERY] Dis Charging 1s\n\r");
			}
			pchr_turn_off_charging();
			getVoltFlag = 1;
			msleep(g_free_bat_temp);
		}
	}

#if defined(CONFIG_POWER_EXT)
//#if 0	
	if (Enable_BATDRV_LOG == 1) {
		printk("[BATTERY] Delete at EVB !\n\r");
	}
#else
	upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
	BMT_status.ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,20) * R_BAT_SENSE;
	//printk("[BATTERY] ADC_BAT_SENSE=%d !\n\r", BMT_status.ADC_BAT_SENSE);
	#if defined(CONFIG_POWER_EXT)	
	BMT_status.ADC_BAT_SENSE += V_compensate_EVB;
	#endif
	BMT_status.bat_vol = BMT_status.ADC_BAT_SENSE;	

	//new adc sampling algo.		
	if ( BMT_status.bat_charging_state != CHR_TOP_OFF)
	{
		gADC_BAT_SENSE_temp = (int)BMT_status.bat_vol;
		gADC_I_SENSE_temp = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,1) * R_I_SENSE;	

		//workaround
		gADC_BAT_SENSE_temp = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,1) * R_BAT_SENSE;
		gADC_I_SENSE_temp = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,1) * R_I_SENSE;
		gADC_BAT_SENSE_temp = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,1) * R_BAT_SENSE;
		gADC_I_SENSE_temp = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,1) * R_I_SENSE;
		gADC_BAT_SENSE_temp = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,1) * R_BAT_SENSE;
		gADC_I_SENSE_temp = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,1) * R_I_SENSE;
		gADC_BAT_SENSE_temp = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,1) * R_BAT_SENSE;
		gADC_I_SENSE_temp = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,1) * R_I_SENSE;
		
		if (Enable_BATDRV_LOG == 1) {			
			printk("[BATTERY] gADC_BAT_SENSE_temp=%d, gADC_I_SENSE_temp=%d\n\r", gADC_BAT_SENSE_temp, gADC_I_SENSE_temp);
		}
		
		gADC_I_SENSE_offset = gADC_BAT_SENSE_temp - gADC_I_SENSE_temp;
	}
#endif

	g_TempBattVoltage = BMT_status.bat_vol;
	if ( getVoltFlag == 1 )
	{
		if (Enable_BATDRV_LOG == 1) {
			//printk("......................\n\r");
			printk("[BATTERY] Charging 9s\n\r");
		}
		//pchr_turn_on_charging();
		getVoltFlag = 0;
	}	
	/*Charging 9s and discharging 1s : end*/

	/*Use no gas gauge*/
	if( (get_chip_eco_ver()==CHIP_E1) || (gForceADCsolution==1) )
	{

		/* Re-calculate Battery Percentage (SOC) */    
		BMT_status.SOC = BattVoltToPercent(BMT_status.bat_vol);
			    
		/* User smooth View when discharging : start */
		if( upmu_is_chr_det(CHR) == KAL_FALSE )
		{
			if (BMT_status.bat_vol >= RECHARGING_VOLTAGE) {
				BMT_status.SOC = 100;	
				BMT_status.bat_full = KAL_TRUE;
			}		
		}		
		if (bat_volt_cp_flag == 0) 
		{
			bat_volt_cp_flag = 1;		
			bat_volt_check_point = BMT_status.SOC;
		}
		/* User smooth View when discharging : end */

	    /**************** Averaging : START ****************/        
	    if (!batteryBufferFirst)
	    {
	        batteryBufferFirst = KAL_TRUE;
	        
	        for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
	            batteryVoltageBuffer[i] = BMT_status.bat_vol;            
	            batteryCurrentBuffer[i] = BMT_status.ICharging;            
	            batterySOCBuffer[i] = BMT_status.SOC;
	        }

	        batteryVoltageSum = BMT_status.bat_vol * BATTERY_AVERAGE_SIZE;
	        batteryCurrentSum = BMT_status.ICharging * BATTERY_AVERAGE_SIZE;        
	        batterySOCSum = BMT_status.SOC * BATTERY_AVERAGE_SIZE;
	    }

	    batteryVoltageSum -= batteryVoltageBuffer[batteryIndex];
	    batteryVoltageSum += BMT_status.bat_vol;
	    batteryVoltageBuffer[batteryIndex] = BMT_status.bat_vol;

	    batteryCurrentSum -= batteryCurrentBuffer[batteryIndex];
	    batteryCurrentSum += BMT_status.ICharging;
	    batteryCurrentBuffer[batteryIndex] = BMT_status.ICharging;
	    
	    if (BMT_status.bat_full)
	        BMT_status.SOC = 100;
	    if (g_bat_full_user_view)
	        BMT_status.SOC = 100;
		
	    batterySOCSum -= batterySOCBuffer[batteryIndex];
	    batterySOCSum += BMT_status.SOC;
	    batterySOCBuffer[batteryIndex] = BMT_status.SOC;
	    
	    BMT_status.bat_vol = batteryVoltageSum / BATTERY_AVERAGE_SIZE;
	    BMT_status.ICharging = batteryCurrentSum / BATTERY_AVERAGE_SIZE;    
	    BMT_status.SOC = batterySOCSum / BATTERY_AVERAGE_SIZE;

	    batteryIndex++;
	    if (batteryIndex >= BATTERY_AVERAGE_SIZE)
	        batteryIndex = 0;
	    /**************** Averaging : END ****************/
		
		if( BMT_status.SOC == 100 ) {
			BMT_status.bat_full = KAL_TRUE;   
		}
	}
	/*Use gas gauge*/
	else
	{
		/* Re-calculate Battery Percentage (SOC) */    
		BMT_status.SOC = FGADC_Get_BatteryCapacity_CoulombMothod();
		//BMT_status.bat_vol = FGADC_Get_FG_Voltage();

		/* Sync FG's percentage */
		if(gSyncPercentage==0)
		{			
			if( (upmu_is_chr_det(CHR)==KAL_TRUE) && (!g_Battery_Fail) && (g_Charging_Over_Time==0))
			{
				/* SOC only UP when charging */
	            if ( BMT_status.SOC > bat_volt_check_point ) {						
					bat_volt_check_point = BMT_status.SOC;
	            }
			}
			else
			{
				/* SOC only Done when dis-charging */
		        if ( BMT_status.SOC < bat_volt_check_point ) {
					bat_volt_check_point = BMT_status.SOC;
		        }
			}		
			//bat_volt_check_point = BMT_status.SOC;
		}

		/* User smooth View when discharging : start */
		//if (bat_volt_cp_flag == 0) 
		//{
		//	if(BMT_status.SOC != 0)
		//	{
		//		bat_volt_cp_flag = 1;		
		//	}
		//	bat_volt_check_point = BMT_status.SOC;
		//}
		/* User smooth View when discharging : end */
		
		/**************** Averaging : START ****************/
		if (!batteryBufferFirst)
	    {
	        batteryBufferFirst = KAL_TRUE;
	        
	        for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
	            batteryVoltageBuffer[i] = BMT_status.bat_vol;
				batteryCurrentBuffer[i] = BMT_status.ICharging;
	        }

	        batteryVoltageSum = BMT_status.bat_vol * BATTERY_AVERAGE_SIZE;
			batteryCurrentSum = BMT_status.ICharging * BATTERY_AVERAGE_SIZE;
	    }

		if( (batteryCurrentSum==0) && (BMT_status.ICharging!=0) )
		{
			for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {	        
				batteryCurrentBuffer[i] = BMT_status.ICharging;
	        }
			batteryCurrentSum = BMT_status.ICharging * BATTERY_AVERAGE_SIZE;
		}

	    batteryVoltageSum -= batteryVoltageBuffer[batteryIndex];
	    batteryVoltageSum += BMT_status.bat_vol;
	    batteryVoltageBuffer[batteryIndex] = BMT_status.bat_vol;

	    batteryCurrentSum -= batteryCurrentBuffer[batteryIndex];
	    batteryCurrentSum += BMT_status.ICharging;
	    batteryCurrentBuffer[batteryIndex] = BMT_status.ICharging;
	
	    //if (g_bat_full_user_view)
	    //    BMT_status.SOC = 100;
	    
	    BMT_status.bat_vol = batteryVoltageSum / BATTERY_AVERAGE_SIZE;
		BMT_status.ICharging = batteryCurrentSum / BATTERY_AVERAGE_SIZE;

	    batteryIndex++;
	    if (batteryIndex >= BATTERY_AVERAGE_SIZE)
	        batteryIndex = 0;
		/**************** Averaging : END ****************/
	}

	if (Enable_BATDRV_LOG == 1) {
    	printk("[BATTERY:AVG] BatTemp:%d Vbat:%ld VBatSen:%ld SOC:%ld ChrDet:%d Vchrin:%ld Icharging:%ld ChrType:%d USBstate:%d\r\n", 
       	BMT_status.temperature ,BMT_status.bat_vol, BMT_status.ADC_BAT_SENSE, BMT_status.SOC, 
       	upmu_is_chr_det(CHR), BMT_status.charger_vol, BMT_status.ICharging, CHR_Type_num, g_usb_state );            
		printk(  "[BATTERY] CON_9:%x, CON10:%x, V_CC2TOPOFF_THRES=%d\n\r", 
			INREG16(CHR_CON_9), INREG16(CHR_CON_10), V_CC2TOPOFF_THRES);
	}           

    if (Enable_BATDRV_LOG == 1) {
		//printk("[BATTERY:FG] BatTemp,Vbat,VBatSen,SOC,ChrDet,Vchrin,Icharging,ChrType,SOC_C,SOC_V\r\n");
		printk("[BATTERY:FG] %d,%ld,%ld,%ld,%d,%ld,%ld,%d,%d,%d,%d\r\n", 
       	BMT_status.temperature ,BMT_status.bat_vol, BMT_status.ADC_BAT_SENSE, BMT_status.SOC, 
       	upmu_is_chr_det(CHR), BMT_status.charger_vol, BMT_status.ICharging, CHR_Type_num,
       	FGADC_Get_BatteryCapacity_CoulombMothod(), FGADC_Get_BatteryCapacity_VoltageMothod(), BATTERY_AVERAGE_SIZE );
    }

	/* Protection Check : start*/
    BAT_status = BAT_CheckPMUStatusReg();
    if(BAT_status != PMU_STATUS_OK)
        return PMU_STATUS_FAIL;                  

	#if (BAT_TEMP_PROTECT_ENABLE == 1)
    if ((BMT_status.temperature <= MIN_CHARGE_TEMPERATURE) || 
        (BMT_status.temperature == ERR_CHARGE_TEMPERATURE))
    {
        printk(  "[BATTERY] Battery Under Temperature or NTC fail !!\n\r");                
		BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL;       
    }
	#endif		    
	if (BMT_status.temperature >= MAX_CHARGE_TEMPERATURE)
    {
        printk(  "[BATTERY] Battery Over Temperature !!\n\r");                
		BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL;       
    }

	if( upmu_is_chr_det(CHR) == KAL_TRUE)
    {
    	#if (V_CHARGER_ENABLE == 1)
        if (BMT_status.charger_vol <= V_CHARGER_MIN )
        {
            printk(  "[BATTERY]Charger under voltage!!\r\n");                    
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;        
        }
		#endif
        if ( BMT_status.charger_vol >= V_CHARGER_MAX )
        {
            printk(  "[BATTERY]Charger over voltage !!\r\n");                    
            BMT_status.charger_protect_status = charger_OVER_VOL;
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;        
        }		
	}
	/* Protection Check : end*/

    if( upmu_is_chr_det(CHR) == KAL_TRUE)
    {        
		if((BMT_status.bat_vol < RECHARGING_VOLTAGE) && (BMT_status.bat_full) && (g_HW_Charging_Done == 1) && (!g_Battery_Fail) )	
        {
            if (Enable_BATDRV_LOG == 1) {
            	printk("[BATTERY] Battery Re-charging !!\n\r");                
            }
            BMT_status.bat_full = KAL_FALSE;    
            g_bat_full_user_view = KAL_TRUE;
			//BMT_status.bat_charging_state = CHR_CC;
			BMT_status.bat_charging_state = CHR_PRE;

			g_HW_Charging_Done = 0;
			g_Calibration_FG = 0;

			//if (Enable_BATDRV_LOG == 1) {
			//	printk("[BATTERY] Battery Re-charging. Call FGADC_Reset_SW_Parameter.\n\r");	
			//}
			//FGADC_Reset_SW_Parameter();

			CSDAC_DAT_MAX=255;
			if(get_chip_eco_ver()!=CHIP_E1)
			{
				upmu_pchr_csdac_test_enable(CHR, KAL_FALSE); 		// CSDAC_TEST = 0
			}
        }		
    }
        
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryStatusFailAction(void)
{
    if (Enable_BATDRV_LOG == 1) {
    	printk(  "[BATTERY] BAD Battery status... Charging Stop !!\n\r");            
    }

    BMT_status.total_charging_time = 0;
	BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time = 0;
	BMT_status.POSTFULL_charging_time = 0;

    /*  Disable charger */
    pchr_turn_off_charging();

	g_sw_cv_enable=0;

    return PMU_STATUS_OK;
}

PMU_STATUS BAT_ChargingOTAction(void)
{    
    printk(  "[BATTERY] Charging over %d hr stop !!\n\r", MAX_CHARGING_TIME);            
 
    //BMT_status.bat_full = KAL_TRUE;
	BMT_status.total_charging_time = 0;
	BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time = 0;
	BMT_status.POSTFULL_charging_time = 0;
    
	g_HW_Charging_Done = 1;	
	g_Charging_Over_Time = 1;

    /*  Disable charger*/
    pchr_turn_off_charging();

	g_sw_cv_enable=0;
  
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryFullAction(void)
{
	if (Enable_BATDRV_LOG == 1) {    
    	printk(  "[BATTERY] Battery full !!\n\r");            
	}
    
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
	BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time = 0;
	BMT_status.POSTFULL_charging_time = 0;
	
	g_HW_Charging_Done = 1;
	g_Calibration_FG = 1;

    /*  Disable charger */
    pchr_turn_off_charging();

	g_sw_cv_enable=0;
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_PreChargeModeAction(void)
{
    if (Enable_BATDRV_LOG == 1) {
    	printk(  "[BATTERY] Pre-CC mode charge, timer=%ld on %ld !!\n\r",
    	BMT_status.PRE_charging_time, BMT_status.total_charging_time);    
    }

	BMT_status.PRE_charging_time += BAT_TASK_PERIOD;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time = 0;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;                    

	//reset
	upmu_pchr_test_csdac_dat(CHR, 0);   // CSDAC_DAT = 0
	CLRREG16(0xF702FA10,0x0002); 		// CSDAC_TEST = 0
	if (Enable_BATDRV_LOG == 1) 
	{
		printk("[BAT_PreChargeModeAction] Reg[0xF702FA10]=%d\r\n", INREG16(0xF702FA10));		
	}

	/*  Enable charger */
	pchr_turn_on_charging();	        

	if ( BMT_status.bat_vol > V_PRE2CC_THRES )
	{
		BMT_status.bat_charging_state = CHR_CC;
	}

	save_value = 0x0;
	CSDAC_DAT_MAX = 255;
	CSDAC_DAT = 0;
	VBAT_CV_DET = 0x0;
	CS_DET = 0x0;
	g_sw_cv_enable=0;
	
    return PMU_STATUS_OK;        
} 

PMU_STATUS BAT_ConstantCurrentModeAction(void)
{
	int i=0;

    if (Enable_BATDRV_LOG == 1) {
    	printk(  "[BATTERY] CC mode charge, timer=%ld on %ld !!\n\r",
    	BMT_status.CC_charging_time, BMT_status.total_charging_time);    
    }

    BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time += BAT_TASK_PERIOD;
	BMT_status.TOPOFF_charging_time = 0;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;                    

	//reset
	upmu_pchr_test_csdac_dat(CHR, 0);   // CSDAC_DAT = 0
	CLRREG16(0xF702FA10,0x0002); 		// CSDAC_TEST = 0
	if (Enable_BATDRV_LOG == 1) 
	{
		printk("[BAT_ConstantCurrentModeAction] Reg[0xF702FA10]=%d\r\n", INREG16(0xF702FA10));		
	}

	/*  Enable charger */
	pchr_turn_on_charging();	        

	if ( BMT_status.bat_vol > V_CC2TOPOFF_THRES )
	{
		BMT_status.bat_charging_state = CHR_TOP_OFF;
		
		if(get_chip_eco_ver()!=CHIP_E1)
		{
			gSW_CV_prepare_flag=1;
			
			SW_CV_Algo_prepare();
		
			if(g_temp_CC_value == AC_CHARGER_CURRENT)
			{
				for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {	            
					batteryCurrentBuffer[i] = 650;
	        	}
				batteryCurrentSum = 650 * BATTERY_AVERAGE_SIZE;
			}
			else
			{
				for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {	            
					batteryCurrentBuffer[i] = 450;
	        	}
				batteryCurrentSum = 450 * BATTERY_AVERAGE_SIZE;
			}
		}
	}

	save_value = 0x0;
	CSDAC_DAT_MAX = 255;
	CSDAC_DAT = 0;
	VBAT_CV_DET = 0x0;
	CS_DET = 0x0;
	g_sw_cv_enable=0;
	
    return PMU_STATUS_OK;        
}    

PMU_STATUS BAT_TopOffModeAction(void)
{
	//int i=0;
	//int CV_counter=0;
	int ret_check_I_charging=0;	

    if (Enable_BATDRV_LOG == 1) {
    	printk(  "[BATTERY] Top Off mode charge, timer=%ld on %ld !!\n\r",
    	BMT_status.TOPOFF_charging_time, BMT_status.total_charging_time);    
    }

    BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time += BAT_TASK_PERIOD;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;                    

#if 0
	for(i=0;i<100;i++)
	{
		if ( upmu_is_vbat_cv_det(CHR) == KAL_TRUE )
		{
			CV_counter++;
		}
	}
	
	printk("TOPOFF:CV_counter = %d\r\n", CV_counter);
	
	if ( CV_counter >= 90 )
	{
		BMT_status.bat_charging_state = CHR_POST_FULL;
	}
#endif 

	if(BMT_status.TOPOFF_charging_time >= 20)
	{
		ret_check_I_charging = g_Get_I_Charging();
		if(ret_check_I_charging > CHR_OUT_CURRENT)
		{
			/*  Enable charger */
			pchr_turn_on_charging();
		}
		if (Enable_BATDRV_LOG == 1) {
    		printk(  "[BAT_TopOffModeAction] ret_check_I_charging=%dmA\n\r",ret_check_I_charging);    
    	}
	}
	else
	{
	/*  Enable charger */
	pchr_turn_on_charging();
	}

	//if(get_chip_eco_ver()!=CHIP_E1)
	//{
	//	SW_CV_Algo_prepare();
	//}

	g_sw_cv_enable=1;
	
    return PMU_STATUS_OK;        
} 

int POSTFULL_safety_timer=0;

PMU_STATUS BAT_PostFullModeAction(void)
{
#if 0
	int i=0;
	int CV_counter=0;

    if (Enable_BATDRV_LOG == 1) {
    	printk(  "[BATTERY] Post Full mode charge, timer=%d on %d (%d) !!\n\r",
    	BMT_status.POSTFULL_charging_time, BMT_status.total_charging_time, POSTFULL_safety_timer);    
    }

    BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time = 0;
	BMT_status.POSTFULL_charging_time += BAT_TASK_PERIOD;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;                    
	POSTFULL_safety_timer += BAT_TASK_PERIOD;

	if ( POSTFULL_safety_timer > MAX_POSTFULL_SAFETY_TIME )
	{
		BMT_status.bat_charging_state = CHR_BATFULL;
		BMT_status.bat_full = KAL_TRUE;
		POSTFULL_safety_timer = 0;
		printk("POSTFULL_safety_timer timeout !\r\n");
		return PMU_STATUS_OK;
	}

	if (BMT_status.POSTFULL_charging_time > 90)
	{
		for(i=0;i<100;i++)
		{
			if ( upmu_is_vbat_cv_det(CHR) == KAL_TRUE )
			{
				CV_counter++;
			}
		}
		
		printk("POSTFULL:CV_counter = %d\r\n", CV_counter);
		
		if ( CV_counter >= 90 )
		{
			BMT_status.bat_charging_state = CHR_BATFULL;
			BMT_status.bat_full = KAL_TRUE;
			POSTFULL_safety_timer = 0;
			return PMU_STATUS_OK;
		}
	
		BMT_status.POSTFULL_charging_time = 0;
	}	

	/*  Enable charger */
	pchr_turn_on_charging();	        
#endif

	g_sw_cv_enable=0;

    return PMU_STATUS_OK;        
} 

void mt_battery_notify_check(void)
{
    static uint temp_over_count = 0;    

	g_BatteryNotifyCode = 0x0000;
	
	if(g_BN_TestMode == 0x0000)
	{
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] mt_battery_notify_check\n");
		}

#if defined(BATTERY_NOTIFY_CASE_0000)	
		BAT_LOG("[BATTERY] BATTERY_NOTIFY_CASE_0000\n");
#endif

#if defined(BATTERY_NOTIFY_CASE_0001)
		if(BMT_status.charger_vol > V_CHARGER_MAX)
		//if(BMT_status.charger_vol > 3000) //test
		{
			g_BatteryNotifyCode |= 0x0001;
			printk("[BATTERY] BMT_status.charger_vol(%ld) > 7000mV\n", BMT_status.charger_vol);
		}
		BAT_LOG("[BATTERY] BATTERY_NOTIFY_CASE_0001\n");
#endif

#if defined(BATTERY_NOTIFY_CASE_0002)
		if(BMT_status.temperature > MAX_CHARGE_TEMPERATURE)
		//if(BMT_status.temperature > 20) //test
		{

            temp_over_count++;
            if (temp_over_count >= 3)
            {
                g_BatteryNotifyCode |= 0x0002;
                printk("[BATTERY] warning : temp over count = %d\n", temp_over_count);
            }
			printk("[BATTERY] bat_temp(%d) > 50'C\n", BMT_status.temperature);
		}
        else
        {
            temp_over_count = 0;
		}	
        
        BAT_LOG("[BATTERY] BATTERY_NOTIFY_CASE_0002\n");
#endif

#if defined(BATTERY_NOTIFY_CASE_0003)
		//if(BMT_status.ICharging > 1000)
		if( (BMT_status.ICharging > 1000) &&
			(BMT_status.total_charging_time > 300)
			)		
		//if(BMT_status.ICharging > 200) //test
		{
			g_BatteryNotifyCode |= 0x0004;
			printk("[BATTERY] I_charging(%ld) > 1000mA\n", BMT_status.ICharging);
		}
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] BATTERY_NOTIFY_CASE_0003\n");
		}	
#endif

#if defined(BATTERY_NOTIFY_CASE_0004)
		if(BMT_status.bat_vol > 4350)
		//if(BMT_status.bat_vol > 3000) //test
		{
			g_BatteryNotifyCode |= 0x0008;
			printk("[BATTERY] bat_vlot(%ld) > 4350mV\n", BMT_status.bat_vol);
		}
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] BATTERY_NOTIFY_CASE_0004\n");
		}
#endif

#if defined(BATTERY_NOTIFY_CASE_0005)
		if(BMT_status.total_charging_time >= MAX_CHARGING_TIME)
		//if(BMT_status.total_charging_time >= 60) //test
		{
			g_BatteryNotifyCode |= 0x0010;
			printk("[BATTERY] Charging Over Time\n");
		}
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] BATTERY_NOTIFY_CASE_0005\n");
		}
#endif

	}
	else if(g_BN_TestMode == 0x0001)
	{
		g_BatteryNotifyCode = 0x0001;
		printk("[BATTERY_TestMode] BATTERY_NOTIFY_CASE_0001\n");
	}
	else if(g_BN_TestMode == 0x0002)
	{
		g_BatteryNotifyCode = 0x0002;
		printk("[BATTERY_TestMode] BATTERY_NOTIFY_CASE_0002\n");
	}
	else if(g_BN_TestMode == 0x0003)
	{
		g_BatteryNotifyCode = 0x0004;
		printk("[BATTERY_TestMode] BATTERY_NOTIFY_CASE_0003\n");
	}
	else if(g_BN_TestMode == 0x0004)
	{
		g_BatteryNotifyCode = 0x0008;
		printk("[BATTERY_TestMode] BATTERY_NOTIFY_CASE_0004\n");
	}
	else if(g_BN_TestMode == 0x0005)
	{
		g_BatteryNotifyCode = 0x0010;
		printk("[BATTERY_TestMode] BATTERY_NOTIFY_CASE_0005\n");
	}
	else
	{
		printk("[BATTERY] Unknown BN_TestMode Code : %x\n", g_BN_TestMode);
	}
}

int gFG_can_reset_flag = 1;

void BAT_thread(void)
{
    int BAT_status = 0;

	if (Enable_BATDRV_LOG == 1) {
    	printk("[BATTERY_TOP] LOG. %d,%d,%d,%d----------------------------\n", 
			BATTERY_AVERAGE_SIZE, CHARGING_FULL_CURRENT, RECHARGING_VOLTAGE, gFG_15_vlot);
	}

    /* If charger exist, than get the charger type */    
    if( upmu_is_chr_det(CHR) == KAL_TRUE )
    {
        wake_lock(&battery_suspend_lock);        

		if(BMT_status.charger_type == CHARGER_UNKNOWN)    		
		{
	   	    CHR_Type_num = mt_charger_type_detection();      
			BMT_status.charger_type = CHR_Type_num;
			
			if( (CHR_Type_num==STANDARD_HOST) || (CHR_Type_num==CHARGING_HOST) )
			{
			    mt_usb_connect();
			}
    	}

		if(g_XGPT_restart_flag==1)
		{
			XGPT_Start(XGPT3);
			g_XGPT_restart_flag=0;			
			if (Enable_BATDRV_LOG == 1) {
				printk("Do XGPT_Start (%d)\n", g_XGPT_restart_flag);
			}
		}
    }
    else 
    {   
		wake_unlock(&battery_suspend_lock);
	
        BMT_status.charger_type = CHARGER_UNKNOWN;
        BMT_status.bat_full = KAL_FALSE;
		
		/*Use no gas gauge*/
		if( gForceADCsolution == 1 )
		{
			g_bat_full_user_view = KAL_FALSE;
		}
		/*Use gas gauge*/
		else
		{
		if(bat_volt_check_point != 100) {
        	g_bat_full_user_view = KAL_FALSE;
			if (Enable_BATDRV_LOG == 1) {
				printk("[Battery_Only] Set g_bat_full_user_view=KAL_FALSE\r\n");
			}
		}
		}

		g_usb_state = USB_UNCONFIGURED;
		//g_usb_state = USB_SUSPEND;

		g_HW_Charging_Done = 0;
		g_Charging_Over_Time = 0;
		g_Calibration_FG = 0;

		CSDAC_DAT_MAX=255;

		mt_usb_disconnect();

		gSW_CV_prepare_flag=0;

		XGPT_Stop(XGPT3);
		g_XGPT_restart_flag=1;
		if (Enable_BATDRV_LOG == 1) {
			printk("Do XGPT_Stop (%d)\n", g_XGPT_restart_flag);
		}
		
    }

    /* Check Battery Status */
    BAT_status = BAT_CheckBatteryStatus();
    if( BAT_status == PMU_STATUS_FAIL )
        g_Battery_Fail = KAL_TRUE;
    else
        g_Battery_Fail = KAL_FALSE;

	if( BMT_status.bat_charging_state == CHR_ERROR )
        g_Battery_Fail = KAL_TRUE;
    else
        g_Battery_Fail = KAL_FALSE;

	/* Battery Notify Check */	
	mt_battery_notify_check();

	if(get_chip_eco_ver()==CHIP_E1)
	{
		gFG_booting_counter_I_FLAG=2;
		printk("get_chip_eco_ver()==CHIP_E1\r\n");
	}
//#if BATT_ID_CHECK_SUPPORT
#if 0
	if((gFG_booting_counter_I_FLAG == 2) || 
				((BMT_status.bat_id_valid == 0) && (mt6573_battery_main.BAT_HEALTH != POWER_SUPPLY_HEALTH_DEAD)))	/*batt id error*/
#else
	if(gFG_booting_counter_I_FLAG == 2)
#endif
	{		
#if BATT_ID_CHECK_SUPPORT
           if (Enable_BATDRV_LOG == 1) {
                        printk("jrd_enter %s:counter_I_FLAG:%d.id:%d, start to update ac/usb/battery!=========================!\n",
						 __func__, gFG_booting_counter_I_FLAG, BMT_status.bat_id_valid);
                }
#endif
	    /* AC/USB/Battery information update for Android */
	    mt6573_ac_update(&mt6573_ac_main);
	    mt6573_usb_update(&mt6573_usb_main);
	    mt6573_battery_update(&mt6573_battery_main);   
	}

    /* No Charger */
    if(BAT_status == PMU_STATUS_FAIL || g_Battery_Fail)    
    {
    	gFG_can_reset_flag = 1;
		
        BAT_BatteryStatusFailAction();
    }
	
    /* Battery Full */
	//else if (BMT_status.bat_full)
   	/* HW charging done, real stop charging */
	else if (g_HW_Charging_Done == 1)
    {   
    	if (Enable_BATDRV_LOG == 1) {
    		printk("[BATTERY] Battery real full. \n");
    	}
        BAT_BatteryFullAction();		

		if(gFG_can_reset_flag == 1)
		{
			gFG_can_reset_flag = 0;

			if (Enable_BATDRV_LOG == 1) {
				printk("[BATTERY] Battery real full. Call FGADC_Reset_SW_Parameter.\n");
			}
			FGADC_Reset_SW_Parameter();
		}
    }

	/* Charging Overtime, can not charging */
	else if (g_Charging_Over_Time == 1)
	{
		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] Charging Over Time. \n");
		}
		pchr_turn_off_charging();

		if(gFG_can_reset_flag == 1)
		{
			gFG_can_reset_flag = 0;
			//FGADC_Reset_SW_Parameter();
		}
    }
	
    /* Battery Not Full and Charger exist : Do Charging */
    else
    {
		gFG_can_reset_flag = 1;

		if (Enable_BATDRV_LOG == 1) {
			printk("[BATTERY] state=%ld, I=%d, CSDAC_DAT_MAX=%d, CSDAC_DAT=%d\n", 
				BMT_status.bat_charging_state, g_BatteryAverageCurrent, CSDAC_DAT_MAX, CSDAC_DAT);
		}
	
        /* Charging OT */
        if(BMT_status.total_charging_time >= MAX_CHARGING_TIME)
        {
        	//BMT_status.bat_charging_state = CHR_BATFULL;
            BAT_ChargingOTAction();
			return;
        }

#if BATT_VOLT_OV_CHECK_SUPPORT
	if(BMT_status.TOPOFF_charging_time >= MAX_CV_CHARGING_TIME) || (BMT_status.bat_vol > BATT_OVER_VOLTAGE_VALUE)
#else
	if ( BMT_status.TOPOFF_charging_time >= MAX_CV_CHARGING_TIME )
#endif
		{
			if (Enable_BATDRV_LOG == 1) {
				printk("BMT_status.TOPOFF_charging_time >= %d, or vbat:%d \r\n", MAX_CV_CHARGING_TIME, BMT_status.bat_vol);
			}
			BMT_status.bat_charging_state = CHR_BATFULL;
		    BAT_BatteryFullAction();										
			return;
        }

		if(get_chip_eco_ver()==CHIP_E1)
		{
			if ( (BMT_status.bat_charging_state == CHR_TOP_OFF) &&
	             (BMT_status.SOC == 100) && 
	             (BMT_status.bat_vol >= Batt_VoltToPercent_Table[10].BattVolt) )
			{
				if (Enable_BATDRV_LOG == 1) {
					printk("[BATTERY] Battery real full(%ld,%d) and disable charging !\n", 
							BMT_status.SOC, Batt_VoltToPercent_Table[10].BattVolt); 
				}
				BMT_status.bat_charging_state = CHR_BATFULL;
			    BAT_BatteryFullAction();
				return;
	        }	
		}
		else
		{
			/* charging full condition when charging current < CHARGING_FULL_CURRENT mA on CHR_TOP_OFF mode*/
			if ( (BMT_status.bat_charging_state == CHR_TOP_OFF ) 
				 && (BMT_status.TOPOFF_charging_time > 60)
				 && (BMT_status.ICharging <= CHARGING_FULL_CURRENT)
				 //&& (g_BatteryAverageCurrent <= CHARGING_FULL_CURRENT)	 				 
				 //&& (CSDAC_DAT_MAX != 255)
				 //&& (CSDAC_DAT == 0)
				 )
			{
				BMT_status.bat_charging_state = CHR_BATFULL;
				BAT_BatteryFullAction();				
				//printk("[BATTERY] Battery real full and disable charging on %d mA \n", g_BatteryAverageCurrent); 
				printk("[BATTERY] Battery real full and disable charging on %ld mA \n", BMT_status.ICharging); 
				return;
			}
		}
		
        /* Charging flow begin */
        switch(BMT_status.bat_charging_state)
        {            
            case CHR_PRE :
				BAT_PreChargeModeAction();
                break;    
                
            case CHR_CC :
                BAT_ConstantCurrentModeAction();
                break;    
                
            case CHR_TOP_OFF :
                BAT_TopOffModeAction();
                break;
				
            case CHR_POST_FULL :
                BAT_PostFullModeAction();
                break;				

            case CHR_BATFULL:
				BAT_BatteryFullAction();
                break;
				
            case CHR_ERROR:
				BAT_BatteryStatusFailAction();
                break;				
        }    
    }

	g_SW_CHR_OUT_EN = 1;
	g_HW_stop_charging = 0;

	if (Enable_BATDRV_LOG == 1) {
    	printk(  "[BATTERY] CON_9:%x, CON10:%x\n\r", INREG16(CHR_CON_9), INREG16(CHR_CON_10));
	}

}

///////////////////////////////////////////////////////////////////////////////////////////
//// Internal API 
///////////////////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_POWER_EXT)
//#if 0
#else
static int bat_thread_kthread(void *x)
{
	//set_user_nice(current, 8);
	
    /* Run on a process content */  
    while (1) {           
        mutex_lock(&bat_mutex);
        BAT_thread();                      
        mutex_unlock(&bat_mutex);
		if (Enable_BATDRV_LOG == 1) {
			printk("******** MT6573 battery : bat_thread_kthread : 1 ********\n" );
		}

		wait_event(bat_thread_wq, bat_thread_timeout);

		if (Enable_BATDRV_LOG == 1) {
			printk("******** MT6573 battery : bat_thread_kthread : 2 ********\n" );
		}
        bat_thread_timeout=0;
    }

    return 0;
}
#endif

void bat_thread_wakeup(UINT16 i)
{
    bat_thread_timeout = 1;
	if (Enable_BATDRV_LOG == 1) {
		printk("******** MT6573 battery : bat_thread_wakeup : 1 ********\n" );
	}
	
    wake_up(&bat_thread_wq);
	
	if (Enable_BATDRV_LOG == 1) {
		printk("******** MT6573 battery : bat_thread_wakeup : 2 ********\n" );
	}
}

void BatThread_XGPTConfig(void)
{
    GPT_CONFIG config;
    GPT_NUM  gpt_num = GPT1;    
    GPT_CLK_DIV clkDiv = GPT_CLK_DIV_128;

	printk("******** MT6573 battery : BatThread_XGPTConfig ********\n" );

    GPT_Init (gpt_num, bat_thread_wakeup);
    config.num = gpt_num;
    config.mode = GPT_REPEAT;
    config.clkDiv = clkDiv;
    config.u4Timeout = 10*128;
    
    if (GPT_Config(config) == FALSE )
        return;                       
        
    GPT_Start(gpt_num);  

    return ;
}

#if defined(CONFIG_POWER_EXT)
//#if 0
#else
extern int g_charger_in_flag;
int g_vchr_kthread_index=25;

void BatChargerOutDetect_SWworkaround_v3(void)
{
	int ret_check_I_charging=0;
	int j=0;	
	int sw_chr_out_flag=0;	
	int repeat_times = 5;
#if BATT_ID_CHECK_SUPPORT
	BAT_GetVoltage();
	if(BMT_status.bat_id_valid == 0)
		pchr_turn_off_charging();
#endif
	ret_check_I_charging = g_Get_I_Charging();	
	
	if( g_charger_in_flag == 1)
	{
		//printk("[BATTERY] check %d mA !\n", ret_check_I_charging);
	
		if( (ret_check_I_charging < CHR_OUT_CURRENT) && (g_HW_stop_charging==0) 		
			&& (g_SW_CHR_OUT_EN==1) && (g_HW_Charging_Done==0) )	
		{
			sw_chr_out_flag = 1;
			
			for(j=0 ; j<repeat_times ; j++)
			{
				ret_check_I_charging = g_Get_I_Charging();			
				if (Enable_BATDRV_LOG == 1) {
					printk("[BATTERY] double check %d mA(%d) (%d on %d)\n", 
						ret_check_I_charging, CHR_OUT_CURRENT, j, repeat_times);
				}
				if(ret_check_I_charging > CHR_OUT_CURRENT)
				{
					sw_chr_out_flag = 0;
					break;
				}			
			}

			if(sw_chr_out_flag == 1)
			{
				if( (ret_check_I_charging < CHR_OUT_CURRENT) && (g_HW_stop_charging==0)	
					&& (g_SW_CHR_OUT_EN==1) && (g_HW_Charging_Done==0) )
				{
					if(gSW_CV_prepare_flag == 0)
					{
						pchr_turn_off_charging();
						if (Enable_BATDRV_LOG == 1) {
							printk("[BatChargerOutDetect_SWworkaround_v3] %d < %d\r\n", ret_check_I_charging, CHR_OUT_CURRENT);
							printk("[BatChargerOutDetect_SWworkaround_v3] %d,%d,%d,%d\r\n", 
								g_HW_stop_charging, g_SW_CHR_OUT_EN, g_HW_Charging_Done, gSW_CV_prepare_flag);
						}
					}
				}
				else
				{
					if (Enable_BATDRV_LOG == 1) {
						printk("[BatChargerOutDetect_SWworkaround_v3] ignore\r\n");
						printk("[BatChargerOutDetect_SWworkaround_v3] %d,%d,%d,%d\r\n", 
							g_HW_stop_charging, g_SW_CHR_OUT_EN, g_HW_Charging_Done, gSW_CV_prepare_flag);
					}
				}
			}
		}		
	}	
}

static int sw_cv_thread_kthread(void *x)
{
    /* Run on a process content */  
    while (1) {           

		//printk("******** MT6573 battery : sw_cv_thread_kthread ********\n" );
		//printk("*");

		if( g_sw_cv_enable==1 )
		{
			if( BMT_status.bat_charging_state == CHR_TOP_OFF )
			{
				SW_CV_Algo_task();
			}
		}

		if(gSW_CV_prepare_flag == 0)
		{
			if(g_vchr_kthread_index <= 0)
			{
				//printk("-");			
				BatChargerOutDetect_SWworkaround_v3();
				g_vchr_kthread_index=25; // *20ms			
			}
			g_vchr_kthread_index--;
		}
		else
		{
			//printk("$");
			if(BMT_status.TOPOFF_charging_time >= 20)
			{
				if (Enable_BATDRV_LOG == 1) {
					printk("[sw_cv_thread_kthread] gSW_CV_prepare_flag=0\r\n");
				}
				gSW_CV_prepare_flag=0;
			}
		}
		
		wait_event(sw_cv_thread_wq, sw_cv_thread_timeout);		
        sw_cv_thread_timeout = 0;		
    }

    return 0;
}
#endif

void sw_cv_thread_wakeup(UINT16 i)
{
    sw_cv_thread_timeout = 1;	
    wake_up(&sw_cv_thread_wq);	
}

void SW_CV_Thread_XGPTConfig(void)
{
    XGPT_CONFIG config;
    XGPT_NUM  gpt_num = XGPT3;    
    XGPT_CLK_DIV clkDiv = XGPT_CLK_DIV_128;

	printk("******** MT6573 battery : SW_CV_Thread_XGPTConfig ********\n" );

    XGPT_Init (gpt_num, sw_cv_thread_wakeup);
    config.num = gpt_num;
    config.mode = XGPT_REPEAT;
    config.clkDiv = clkDiv;
    config.u4Compare = 5; // 20ms
    //config.u4Compare = 256; // 1s
    
    if (XGPT_Config(config) == FALSE )
        return;                       
        
    XGPT_Start(gpt_num);  

    return ;
}

///////////////////////////////////////////////////////////////////////////////////////////
//// fop API 
///////////////////////////////////////////////////////////////////////////////////////////
static int adc_cali_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int *user_data_addr;
    int *naram_data_addr;
    int i = 0;
    int ret = 0;

    mutex_lock(&bat_mutex);

    switch(cmd)
    {
        case TEST_ADC_CALI_PRINT :
            g_ADC_Cali = KAL_FALSE;
            break;
        
        case SET_ADC_CALI_Slop:            
            naram_data_addr = (int *)arg;
            ret = copy_from_user(adc_cali_slop, naram_data_addr, 36);
            g_ADC_Cali = KAL_FALSE; /* enable calibration after setting ADC_CALI_Cal */            
            /* Protection */
            for (i=0;i<14;i++) 
            { 
                if ( (*(adc_cali_slop+i) == 0) || (*(adc_cali_slop+i) == 1) ) {
                    *(adc_cali_slop+i) = 1000;
                }
            }
            for (i=0;i<14;i++) printk("adc_cali_slop[%d] = %d\n",i , *(adc_cali_slop+i));
            printk("**** MT6573 adc_cali ioctl : SET_ADC_CALI_Slop Done!\n");            
            break;    
            
        case SET_ADC_CALI_Offset:            
            naram_data_addr = (int *)arg;
            ret = copy_from_user(adc_cali_offset, naram_data_addr, 36);
            g_ADC_Cali = KAL_FALSE; /* enable calibration after setting ADC_CALI_Cal */
            for (i=0;i<14;i++) printk("adc_cali_offset[%d] = %d\n",i , *(adc_cali_offset+i));
            printk("**** MT6573 adc_cali ioctl : SET_ADC_CALI_Offset Done!\n");            
            break;
            
        case SET_ADC_CALI_Cal :            
            naram_data_addr = (int *)arg;
            ret = copy_from_user(adc_cali_cal, naram_data_addr, 4);
            g_ADC_Cali = KAL_TRUE;
            if ( adc_cali_cal[0] == 1 ) {
                g_ADC_Cali = KAL_TRUE;
            } else {
                g_ADC_Cali = KAL_FALSE;
            }            
            for (i=0;i<1;i++) printk("adc_cali_cal[%d] = %d\n",i , *(adc_cali_cal+i));
            printk("**** MT6573 adc_cali ioctl : SET_ADC_CALI_Cal Done!\n");            
            break;    

        case ADC_CHANNEL_READ:            
            g_ADC_Cali = KAL_FALSE; /* 20100508 Infinity */
            user_data_addr = (int *)arg;
            ret = copy_from_user(adc_in_data, user_data_addr, 8); /* 2*int = 2*4 */
            /*ChannelNUm, Counts*/
            //adc_out_data[0] = GetOneChannelValue(adc_in_data[0], adc_in_data[1]);            
			upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
			upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
			upmu_adc_measure_vchr_enable(CHR, KAL_TRUE);
            if( adc_in_data[0] == 0 ) // I_SENSE
            {
            	adc_out_data[0] = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL,adc_in_data[1]) * R_I_SENSE * adc_in_data[1];
            }
			else if( adc_in_data[0] == 1 ) // BAT_SENSE
			{
				adc_out_data[0] = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,adc_in_data[1]) * R_BAT_SENSE * adc_in_data[1];
			}
			else if( adc_in_data[0] == 3 ) // V_Charger
			{
				adc_out_data[0] = IMM_GetOneChannelValue(AUXADC_CHARGER_VOLTAGE_CHANNEL,adc_in_data[1]) * (((R_CHARGER_1+R_CHARGER_2)*100)/R_CHARGER_2) * adc_in_data[1];			
				adc_out_data[0] = adc_out_data[0] / 100;
			}	
			else if( adc_in_data[0] == 30 ) // V_Bat_temp magic number
			{
				adc_out_data[0] = IMM_GetOneChannelValue(AUXADC_TEMPERATURE_CHANNEL,adc_in_data[1]) * adc_in_data[1];
			}
			else
			{
				adc_out_data[0] = IMM_GetOneChannelValue(adc_in_data[0],adc_in_data[1]) * adc_in_data[1];
			}
            
            if (adc_out_data[0]<0)
                adc_out_data[1]=1; /* failed */
            else
                adc_out_data[1]=0; /* success */
            ret = copy_to_user(user_data_addr, adc_out_data, 8);
            printk("**** ioctl : Channel %d * %d times = %d\n", adc_in_data[0], adc_in_data[1], adc_out_data[0]);            
            break;

        case BAT_STATUS_READ:            
            user_data_addr = (int *)arg;
            ret = copy_from_user(battery_in_data, user_data_addr, 4); 
            /* [0] is_CAL */
            if (g_ADC_Cali) {
                battery_out_data[0] = 1;
            } else {
                battery_out_data[0] = 0;
            }
            ret = copy_to_user(user_data_addr, battery_out_data, 4); 
            printk("**** ioctl : CAL:%d\n", battery_out_data[0]);                        
            break;        

        case Set_Charger_Current: /* For Factory Mode*/
            user_data_addr = (int *)arg;
            ret = copy_from_user(charging_level_data, user_data_addr, 4);
            g_ftm_battery_flag = KAL_TRUE;            
			if( charging_level_data[0] == 0 )
			{
				charging_level_data[0] = Cust_CC_70MA; 
			}
			else if ( charging_level_data[0] == 1 )
			{
				charging_level_data[0] = Cust_CC_200MA; 
			}
			else if ( charging_level_data[0] == 2 )
			{
				charging_level_data[0] = Cust_CC_400MA;
			}
			else if ( charging_level_data[0] == 3 )
			{
				charging_level_data[0] = Cust_CC_450MA; 
			}
			else if ( charging_level_data[0] == 4 )
			{
				charging_level_data[0] = Cust_CC_550MA;
			}
			else if ( charging_level_data[0] == 5 )
			{
				charging_level_data[0] = Cust_CC_650MA;
			}
			else if ( charging_level_data[0] == 6 )
			{
				charging_level_data[0] = Cust_CC_700MA;
			}
			else if ( charging_level_data[0] == 7 )
			{
				charging_level_data[0] = Cust_CC_800MA;
			}
			else 
			{
				charging_level_data[0] = Cust_CC_450MA;
			}
            wake_up_bat();
            printk("**** ioctl : set_Charger_Current:%d\n", charging_level_data[0]);
            break;
          
        default:
            g_ADC_Cali = KAL_FALSE;
            break;
    }

    mutex_unlock(&bat_mutex);
    
    return 0;
}

static int adc_cali_open(struct inode *inode, struct file *file)
{ 
   return 0;
}

static int adc_cali_release(struct inode *inode, struct file *file)
{
    return 0;
}

static struct file_operations adc_cali_fops = {
    .owner        = THIS_MODULE,
    .ioctl        = adc_cali_ioctl,
    .open        = adc_cali_open,
    .release    = adc_cali_release,    
};

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Charger_Voltage
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Charger_Voltage(struct device *dev,struct device_attribute *attr, char *buf)
{
	printk("[EM] show_ADC_Charger_Voltage : %ld\n", BMT_status.charger_vol);
	return sprintf(buf, "%ld\n", BMT_status.charger_vol);
}
static ssize_t store_ADC_Charger_Voltage(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Charger_Voltage, 0664, show_ADC_Charger_Voltage, store_ADC_Charger_Voltage);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_0_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_0_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+0));
	printk("[EM] ADC_Channel_0_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_0_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_0_Slope, 0664, show_ADC_Channel_0_Slope, store_ADC_Channel_0_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_1_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_1_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+1));
	printk("[EM] ADC_Channel_1_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_1_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_1_Slope, 0664, show_ADC_Channel_1_Slope, store_ADC_Channel_1_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_2_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_2_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+2));
	printk("[EM] ADC_Channel_2_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_2_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_2_Slope, 0664, show_ADC_Channel_2_Slope, store_ADC_Channel_2_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_3_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_3_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+3));
	printk("[EM] ADC_Channel_3_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_3_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_3_Slope, 0664, show_ADC_Channel_3_Slope, store_ADC_Channel_3_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_4_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_4_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+4));
	printk("[EM] ADC_Channel_4_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_4_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_4_Slope, 0664, show_ADC_Channel_4_Slope, store_ADC_Channel_4_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_5_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_5_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+5));
	printk("[EM] ADC_Channel_5_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_5_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_5_Slope, 0664, show_ADC_Channel_5_Slope, store_ADC_Channel_5_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_6_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_6_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+6));
	printk("[EM] ADC_Channel_6_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_6_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_6_Slope, 0664, show_ADC_Channel_6_Slope, store_ADC_Channel_6_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_7_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_7_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+7));
	printk("[EM] ADC_Channel_7_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_7_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_7_Slope, 0664, show_ADC_Channel_7_Slope, store_ADC_Channel_7_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_8_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_8_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+8));
	printk("[EM] ADC_Channel_8_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_8_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_8_Slope, 0664, show_ADC_Channel_8_Slope, store_ADC_Channel_8_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_9_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_9_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+9));
	printk("[EM] ADC_Channel_9_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_9_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_9_Slope, 0664, show_ADC_Channel_9_Slope, store_ADC_Channel_9_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_10_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_10_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+10));
	printk("[EM] ADC_Channel_10_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_10_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_10_Slope, 0664, show_ADC_Channel_10_Slope, store_ADC_Channel_10_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_11_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_11_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+11));
	printk("[EM] ADC_Channel_11_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_11_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_11_Slope, 0664, show_ADC_Channel_11_Slope, store_ADC_Channel_11_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_12_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_12_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+12));
	printk("[EM] ADC_Channel_12_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_12_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_12_Slope, 0664, show_ADC_Channel_12_Slope, store_ADC_Channel_12_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_13_Slope
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_13_Slope(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_slop+13));
	printk("[EM] ADC_Channel_13_Slope : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_13_Slope(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_13_Slope, 0664, show_ADC_Channel_13_Slope, store_ADC_Channel_13_Slope);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_0_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_0_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+0));
	printk("[EM] ADC_Channel_0_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_0_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_0_Offset, 0664, show_ADC_Channel_0_Offset, store_ADC_Channel_0_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_1_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_1_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+1));
	printk("[EM] ADC_Channel_1_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_1_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_1_Offset, 0664, show_ADC_Channel_1_Offset, store_ADC_Channel_1_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_2_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_2_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+2));
	printk("[EM] ADC_Channel_2_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_2_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_2_Offset, 0664, show_ADC_Channel_2_Offset, store_ADC_Channel_2_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_3_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_3_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+3));
	printk("[EM] ADC_Channel_3_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_3_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_3_Offset, 0664, show_ADC_Channel_3_Offset, store_ADC_Channel_3_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_4_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_4_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+4));
	printk("[EM] ADC_Channel_4_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_4_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_4_Offset, 0664, show_ADC_Channel_4_Offset, store_ADC_Channel_4_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_5_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_5_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+5));
	printk("[EM] ADC_Channel_5_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_5_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_5_Offset, 0664, show_ADC_Channel_5_Offset, store_ADC_Channel_5_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_6_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_6_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+6));
	printk("[EM] ADC_Channel_6_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_6_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_6_Offset, 0664, show_ADC_Channel_6_Offset, store_ADC_Channel_6_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_7_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_7_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+7));
	printk("[EM] ADC_Channel_7_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_7_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_7_Offset, 0664, show_ADC_Channel_7_Offset, store_ADC_Channel_7_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_8_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_8_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+8));
	printk("[EM] ADC_Channel_8_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_8_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_8_Offset, 0664, show_ADC_Channel_8_Offset, store_ADC_Channel_8_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_9_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_9_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+9));
	printk("[EM] ADC_Channel_9_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_9_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_9_Offset, 0664, show_ADC_Channel_9_Offset, store_ADC_Channel_9_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_10_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_10_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+10));
	printk("[EM] ADC_Channel_10_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_10_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_10_Offset, 0664, show_ADC_Channel_10_Offset, store_ADC_Channel_10_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_11_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_11_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+11));
	printk("[EM] ADC_Channel_11_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_11_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_11_Offset, 0664, show_ADC_Channel_11_Offset, store_ADC_Channel_11_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_12_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_12_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+12));
	printk("[EM] ADC_Channel_12_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_12_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_12_Offset, 0664, show_ADC_Channel_12_Offset, store_ADC_Channel_12_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_13_Offset
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_13_Offset(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = (*(adc_cali_offset+13));
	printk("[EM] ADC_Channel_13_Offset : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_13_Offset(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_13_Offset, 0664, show_ADC_Channel_13_Offset, store_ADC_Channel_13_Offset);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : ADC_Channel_Is_Calibration
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_ADC_Channel_Is_Calibration(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=2;
	ret_value = g_ADC_Cali;
	printk("[EM] ADC_Channel_Is_Calibration : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_ADC_Channel_Is_Calibration(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(ADC_Channel_Is_Calibration, 0664, show_ADC_Channel_Is_Calibration, store_ADC_Channel_Is_Calibration);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : Power_On_Voltage
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_Power_On_Voltage(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = Batt_VoltToPercent_Table[0].BattVolt;
	printk("[EM] Power_On_Voltage : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_Power_On_Voltage(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(Power_On_Voltage, 0664, show_Power_On_Voltage, store_Power_On_Voltage);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : Power_Off_Voltage
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_Power_Off_Voltage(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = Batt_VoltToPercent_Table[0].BattVolt;
	printk("[EM] Power_Off_Voltage : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_Power_Off_Voltage(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(Power_Off_Voltage, 0664, show_Power_Off_Voltage, store_Power_Off_Voltage);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : Charger_TopOff_Value
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_Charger_TopOff_Value(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = Batt_VoltToPercent_Table[10].BattVolt;
	printk("[EM] Charger_TopOff_Value : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_Charger_TopOff_Value(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(Charger_TopOff_Value, 0664, show_Charger_TopOff_Value, store_Charger_TopOff_Value);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : FG_Battery_CurrentConsumption
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_FG_Battery_CurrentConsumption(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_value=1;
	ret_value = gFG_current;
	printk("[EM] FG_Battery_CurrentConsumption : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_FG_Battery_CurrentConsumption(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(FG_Battery_CurrentConsumption, 0664, show_FG_Battery_CurrentConsumption, store_FG_Battery_CurrentConsumption);

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : FG_SW_CoulombCounter
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_FG_SW_CoulombCounter(struct device *dev,struct device_attribute *attr, char *buf)
{
	kal_int32 ret_value=1;
	ret_value = FGADC_Get_BatteryCapacity_CoulombMothod();
	printk("[EM] FG_SW_CoulombCounter : %d\n", ret_value);
	return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_FG_SW_CoulombCounter(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	printk("[EM] Not Support Write Function\n");	
	return size;
}
static DEVICE_ATTR(FG_SW_CoulombCounter, 0664, show_FG_SW_CoulombCounter, store_FG_SW_CoulombCounter);

#if 0
///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For EM : PMU_Register
///////////////////////////////////////////////////////////////////////////////////////////
unsigned int g_PMU_value=0;
static ssize_t show_PMU_Register(struct device *dev,struct device_attribute *attr, char *buf)
{
	printk("[EM] show_PMU_Register : %x\n", g_PMU_value);
	return sprintf(buf, "%u\n", g_PMU_value);
}
static ssize_t store_PMU_Register(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	printk("[EM] store_PMU_Register\n");
	if(buf != NULL && size != 0)
	{
		printk("[EM] buf is %s and size is %d \n",buf,size);
		reg_address = simple_strtoul(buf,&pvalue,16);
		
		if(size > 9)
		{		
			reg_value = simple_strtoul((pvalue+1),NULL,16);		
			printk("[EM] write PMU reg %x with value %x \n",reg_address,reg_value);
			OUTREG32(reg_address,reg_value);
		}
		else
		{	
			g_PMU_value = INREG32(reg_address);
			printk("[EM] read PMU reg %x with value %x \n",reg_address,g_PMU_value);
			printk("[EM] Please use \"cat PMU_Register\" to get value\r\n");
		}		
	}		
	return size;
}
static DEVICE_ATTR(PMU_Register, 0664, show_PMU_Register, store_PMU_Register);
#endif

///////////////////////////////////////////////////////////////////////////////////////////
//// platform_driver API 
///////////////////////////////////////////////////////////////////////////////////////////
static int mt6573_battery_probe(struct platform_device *dev)	
{	
	struct class_device *class_dev = NULL;
    int ret=0;
	int i=0;
	int ret_device_file=0;

    printk("******** MT6573 battery driver probe!! ********\n" );
	
	/* Integrate with NVRAM */
	ret = alloc_chrdev_region(&adc_cali_devno, 0, 1, ADC_CALI_DEVNAME);
	if (ret) 
	   printk("Error: Can't Get Major number for adc_cali \n");
	adc_cali_cdev = cdev_alloc();
	adc_cali_cdev->owner = THIS_MODULE;
	adc_cali_cdev->ops = &adc_cali_fops;
	ret = cdev_add(adc_cali_cdev, adc_cali_devno, 1);
	if(ret)
	   printk("adc_cali Error: cdev_add\n");
	adc_cali_major = MAJOR(adc_cali_devno);
	adc_cali_class = class_create(THIS_MODULE, ADC_CALI_DEVNAME);
	class_dev = (struct class_device *)device_create(adc_cali_class, 
												   NULL, 
												   adc_cali_devno, 
												   NULL, 
												   ADC_CALI_DEVNAME);
	printk("[MT6573 BAT_probe] NVRAM prepare : done !!\n ");

	/* Integrate with Android Battery Service */
    ret = power_supply_register(&(dev->dev), &mt6573_ac_main.psy);
    if (ret)
    {            
	printk("[MT6573 BAT_probe] power_supply_register AC Fail !!\n");                    
	return ret;
    }             
    printk("[MT6573 BAT_probe] power_supply_register AC Success !!\n");

    ret = power_supply_register(&(dev->dev), &mt6573_usb_main.psy);
    if (ret)
    {            
	printk("[MT6573 BAT_probe] power_supply_register USB Fail !!\n");                    
	return ret;
    }             
    printk("[MT6573 BAT_probe] power_supply_register USB Success !!\n");

    ret = power_supply_register(&(dev->dev), &mt6573_battery_main.psy);
    if (ret)
    {
	printk("[MT6573 BAT_probe] power_supply_register Battery Fail !!\n");
	return ret;
    }
    printk("[MT6573 BAT_probe] power_supply_register Battery Success !!\n");

	wake_lock_init(&battery_suspend_lock, WAKE_LOCK_SUSPEND, "battery wakelock");

	/* For EM */
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Charger_Voltage);
	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_0_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_1_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_2_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_3_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_4_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_5_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_6_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_7_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_8_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_9_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_10_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_11_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_12_Slope);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_13_Slope);

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_0_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_1_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_2_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_3_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_4_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_5_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_6_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_7_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_8_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_9_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_10_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_11_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_12_Offset);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_13_Offset);

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_ADC_Channel_Is_Calibration);

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_Power_On_Voltage);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_Power_Off_Voltage);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_Charger_TopOff_Value);
	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Battery_CurrentConsumption);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_SW_CoulombCounter);
	
	//device_create_file(&(dev->dev), &dev_attr_PMU_Register);	
	
    /* Initialization BMT Struct */
    for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
        batteryCurrentBuffer[i] = 0;
        batteryVoltageBuffer[i] = 0; 
		batterySOCBuffer[i] = 0;
    }
    batteryVoltageSum = 0;
    batteryCurrentSum = 0;
	batterySOCSum = 0;

    BMT_status.bat_exist = 1;       /* phone must have battery */
#if BATT_ID_CHECK_SUPPORT
    BMT_status.bat_id_valid = 1;
#endif
    BMT_status.charger_exist = 0;     /* for default, no charger */
    BMT_status.bat_vol = 0;
    BMT_status.ICharging = 0;
    BMT_status.temperature = 0;
    BMT_status.charger_vol = 0;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
	BMT_status.CC_charging_time = 0;
	BMT_status.TOPOFF_charging_time = 0;
	BMT_status.POSTFULL_charging_time = 0;

	BMT_status.bat_charging_state = CHR_PRE;

#if defined(CONFIG_POWER_EXT)
//#if 0
	printk("******** MT6573 battery for EVB !! (POWER_EXT)********\n" );

	if(get_chip_eco_ver()!=CHIP_E1)
	{
		printk("[Battery] EVB : set VCDT_MODE=1\n" );
		//SETREG16(PMIC_RESERVE_CON1,0x0002); 				// [1] VCDT_MODE=1
		CLRREG16(PMIC_RESERVE_CON1,0x0002); 				// [1] VCDT_MODE=0
	}
#else
    /* Run Battery Thread Use GPT timer */ 
    BatThread_XGPTConfig();
    /* battery kernel thread for 10s check and charger in/out event */
    kthread_run(bat_thread_kthread, NULL, "bat_thread_kthread");

	if(get_chip_eco_ver()!=CHIP_E1)
	{
		printk("[BATTERY] Run SW CV Thread Use XGPT timer\r\n");
	    /* Run SW CV Thread Use GPT timer */ 
	    SW_CV_Thread_XGPTConfig();
	    kthread_run(sw_cv_thread_kthread, NULL, "sw_cv_thread_kthread");
		//SETREG16(PMIC_RESERVE_CON1,0x0002); 				// [1] VCDT_MODE=1
		CLRREG16(PMIC_RESERVE_CON1,0x0002); 				// [1] VCDT_MODE=0
		printk("REG[PMIC_RESERVE_CON1]=%x\r\n", INREG16(PMIC_RESERVE_CON1));
	}

	/*LOG System Set*/
	init_proc_log();	
#endif

	printk("DCT-ADC:AUXADC_BATTERY_VOLTAGE_CHANNEL=%d\r\n",AUXADC_BATTERY_VOLTAGE_CHANNEL);
	printk("DCT-ADC:AUXADC_REF_CURRENT_CHANNEL=%d\r\n",AUXADC_REF_CURRENT_CHANNEL);
	printk("DCT-ADC:AUXADC_CHARGER_VOLTAGE_CHANNEL=%d\r\n",AUXADC_CHARGER_VOLTAGE_CHANNEL);
	printk("DCT-ADC:AUXADC_TEMPERATURE_CHANNEL=%d\r\n",AUXADC_TEMPERATURE_CHANNEL);

    return 0;
}

static int mt6573_battery_remove(struct platform_device *dev)	
{
    printk("******** MT6573 battery driver remove!! ********\n" );

    return 0;
}

static void mt6573_battery_shutdown(struct platform_device *dev)	
{
    printk("******** MT6573 battery driver shutdown!! ********\n" );

}

static int mt6573_battery_suspend(struct platform_device *dev, pm_message_t state)	
{
    printk("[mt6573_battery_suspend]\n" );

    return 0;
}

static int mt6573_battery_resume(struct platform_device *dev)
{    
#if defined(CONFIG_POWER_EXT)
//#if 0
        printk("[mt6573_battery_resume] EVB: do nothing\n" );
#else

	#if 1
	printk("[mt6573_battery_resume] Disable sleep mode !!\n");
	#else 
	printk("[mt6573_battery_resume] AC/USB/Battery information update for Android !!\n" );

	if(get_chip_eco_ver()!=CHIP_E1)
	{
	    /* AC/USB/Battery information update for Android */
	    mt6573_ac_update(&mt6573_ac_main);
	    mt6573_usb_update(&mt6573_usb_main);
	    mt6573_battery_update(&mt6573_battery_main); 
	}
#endif	

#endif	

    return 0;
}

static struct platform_driver mt6573_battery_driver = {
    .probe		= mt6573_battery_probe,
    .remove		= mt6573_battery_remove,
    .shutdown	= mt6573_battery_shutdown,
    //#ifdef CONFIG_PM
    .suspend	= mt6573_battery_suspend,
    .resume		= mt6573_battery_resume,
    //#endif
    .driver     = {
        .name = "mt6573-battery",
    },
};

///////////////////////////////////////////////////////////////////////////////////////////
//// Battery Notify API 
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_BatteryNotify(struct device *dev,struct device_attribute *attr, char *buf)
{
	if (Enable_BATDRV_LOG == 1) {
	printk("[Battery] show_BatteryNotify : %x\n", g_BatteryNotifyCode);
	}
	return sprintf(buf, "%u\n", g_BatteryNotifyCode);
}
static ssize_t store_BatteryNotify(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_BatteryNotifyCode = 0;
	printk("[Battery] store_BatteryNotify\n");
	if(buf != NULL && size != 0)
	{
		printk("[Battery] buf is %s and size is %d \n",buf,size);
		reg_BatteryNotifyCode = simple_strtoul(buf,&pvalue,16);
		g_BatteryNotifyCode = reg_BatteryNotifyCode;
		printk("[Battery] store code : %x \n",g_BatteryNotifyCode);		
	}		
	return size;
}
static DEVICE_ATTR(BatteryNotify, 0664, show_BatteryNotify, store_BatteryNotify);

static ssize_t show_BN_TestMode(struct device *dev,struct device_attribute *attr, char *buf)
{
	printk("[Battery] show_BN_TestMode : %x\n", g_BN_TestMode);
	return sprintf(buf, "%u\n", g_BN_TestMode);
}
static ssize_t store_BN_TestMode(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_BN_TestMode = 0;
	printk("[Battery] store_BN_TestMode\n");
	if(buf != NULL && size != 0)
	{
		printk("[Battery] buf is %s and size is %d \n",buf,size);
		reg_BN_TestMode = simple_strtoul(buf,&pvalue,16);
		g_BN_TestMode = reg_BN_TestMode;
		printk("[Battery] store g_BN_TestMode : %x \n",g_BN_TestMode);		
	}		
	return size;
}
static DEVICE_ATTR(BN_TestMode, 0664, show_BN_TestMode, store_BN_TestMode);

///////////////////////////////////////////////////////////////////////////////////////////
//// platform_driver API 
///////////////////////////////////////////////////////////////////////////////////////////
static int mt_batteryNotify_probe(struct platform_device *dev)	
{	
	int ret_device_file = 0;

    printk("******** mt_batteryNotify_probe!! ********\n" );
	
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BatteryNotify);
	ret_device_file = device_create_file(&(dev->dev), &dev_attr_BN_TestMode);
	
    return 0;
}

static struct platform_driver mt_batteryNotify_driver = {
    .probe		= mt_batteryNotify_probe,
    .driver     = {
        .name = "mt-battery",
    },
};

static int __init mt6573_battery_init(void)
{
    int ret;

    ret = platform_driver_register(&mt6573_battery_driver);
    if (ret) {
	printk("****[mt6573_battery_driver] Unable to register driver (%d)\n", ret);
	return ret;
    }

	// battery notofy UI
    ret = platform_driver_register(&mt_batteryNotify_driver);
    if (ret) {
		printk("****[mt_batteryNotify] Unable to register driver (%d)\n", ret);
		return ret;
    }

    printk("****[mt6573_battery_driver] Initialization : DONE \n");

    return 0;
}

static void __exit mt6573_battery_exit (void)
{
}

module_init(mt6573_battery_init);
module_exit(mt6573_battery_exit);

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("MT6573 Battery Device Driver");
MODULE_LICENSE("GPL");

