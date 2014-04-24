
#ifndef _CUST_BAT_H_
#define _CUST_BAT_H_

//#include <asm/arch/mt6573.h>
#include <asm/arch/mt65xx_typedefs.h>


typedef enum
{
	Cust_CC_800MA = 800,
	Cust_CC_700MA = 700,
	Cust_CC_650MA = 600,
	Cust_CC_550MA = 500,
	Cust_CC_450MA = 400,
	Cust_CC_400MA = 300,
	Cust_CC_200MA = 200,
	Cust_CC_70MA  = 100,
	Cust_CC_0MA	  = 0
}cust_charging_current_enum;

typedef struct{	
	UINT32 BattVolt;
	UINT32 BattPercent;
}VBAT_TO_PERCENT;

#define CUST_BAT_NAME	 	"CUST_BAT_EVB"

/* Battery Temperature Protection */
#define MAX_CHARGE_TEMPERATURE  45
#define MIN_CHARGE_TEMPERATURE  0
#define ERR_CHARGE_TEMPERATURE  0xFF

/* Recharging Battery Voltage */
#define RECHARGING_VOLTAGE      4110


/* Charging Current Setting */
#define USB_CHARGER_CURRENT		Cust_CC_450MA      
#define AC_CHARGER_CURRENT		Cust_CC_650MA

/* Battery Meter Solution */
#define CONFIG_ADC_SOLUTION 	1

/* Battery Voltage and Percentage Mapping Table */
VBAT_TO_PERCENT Batt_VoltToPercent_Table[] = {
	/*BattVolt,BattPercent*/
	{3400,0},
	{3641,10},
	{3708,20},
	{3741,30},
	{3765,40},
	{3793,50},
	{3836,60},
	{3891,70},
	{3960,80},
	{4044,90},
	{4183,100},
};

/* Precise Tunning */
//#define BATTERY_AVERAGE_SIZE 	600
//#define BATTERY_AVERAGE_SIZE 	60
#define BATTERY_AVERAGE_SIZE 	300

#define CHARGING_IDLE_MODE	 1

#define CHARGING_PICTURE	 1

/* Common setting */
#define R_CURRENT_SENSE 2	// 0.2 Ohm
#define R_BAT_SENSE 2		// times of voltage
#define R_I_SENSE 2			// times of voltage
#define R_CHARGER_1 270
#define R_CHARGER_2 39
#define R_CHARGER_SENSE ((R_CHARGER_1+R_CHARGER_2)/R_CHARGER_2) // times of voltage
#define V_CHARGER_MAX 6000	// 6 V
#define V_CHARGER_MIN 4400	// 4.4 V
#define V_CHARGER_ENABLE 0	// 1:ON , 0:OFF
#define BACKLIGHT_KEY 10	// middle key  // change to 10 -> home key for proto

/* ADC channel */
#define AUXADC_BATTERY_VOLTAGE_CHANNEL 0
#define AUXADC_REF_CURRENT_CHANNEL 1
#define AUXADC_CHARGER_VOLTAGE_CHANNEL 2
#define AUXADC_TEMPERATURE_CHANNEL 5 /// 3 /// qiang_li_checked_for_ALPS00062960, use ADC 5 to measure NTC resister

/* Teperature related setting */
#define RBAT_PULL_UP_R             24000
#define RBAT_PULL_UP_VOLT          2500
#define TBAT_OVER_CRITICAL_LOW     68237
#define BAT_TEMP_PROTECT_ENABLE    1
#define BAT_NTC_10 1
#define BAT_NTC_47 0

#endif /* _CUST_BAT_H_ */ 
