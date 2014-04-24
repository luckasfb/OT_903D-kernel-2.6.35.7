
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
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/rtc.h>

#include <asm/uaccess.h>

#include <mach/mt6573_typedefs.h>
#include <mach/hardware.h>
#include <mach/mt6573_gpt.h>
#include <mach/mt6573_boot.h>

#include "mt6573_fuel_gauge_hw.h"
#include "mt6573_fuel_gauge_sw.h"
//#include "custom_fuel_gauge.h"
#include <custom_fuel_gauge.h>

#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"

//#include "mt6573_cust_adc.h"
#include <cust_adc.h>
//#include <cust_battery.h>

static DEFINE_MUTEX(FGCURRENT_mutex);

#define MAX_V_CHARGER 4000
#define CHR_OUT_CURRENT	100

int Enable_FGADC_LOG = 0;
//int Enable_FGADC_LOG = 1;

///////////////////////////////////////////////////////////////////////////////////////////
//// Extern Functions
///////////////////////////////////////////////////////////////////////////////////////////
extern int IMM_GetOneChannelValue(int dwChannel, int deCount);
extern void upmu_adc_measure_vbat_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_adc_measure_vsen_enable(upmu_chr_list_enum chr, kal_bool enable);
extern void upmu_adc_measure_vchr_enable(upmu_chr_list_enum chr, kal_bool enable);
extern INT16 BattVoltToTemp(UINT32 dwVolt);
extern kal_bool upmu_is_chr_det(upmu_chr_list_enum chr);
extern void pchr_turn_off_charging (void);

extern int g_charger_in_flag;
extern int g_SW_CHR_OUT_EN;
extern int g_HW_Charging_Done;
extern int g_HW_stop_charging;
extern int bat_volt_check_point;
extern int gForceADCsolution;
extern kal_bool batteryBufferFirst;

///////////////////////////////////////////////////////////////////////////////////////////
//// Define
///////////////////////////////////////////////////////////////////////////////////////////
#define DRV_ClearBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp &=~(data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetBits(addr,data)     {\
   kal_uint16 temp;\
   temp = DRV_Reg(addr);\
   temp |= (data);\
   DRV_WriteReg(addr,temp);\
}

#define DRV_SetData(addr, bitmask, value)     {\
   kal_uint16 temp;\
   temp = (~(bitmask)) & DRV_Reg(addr);\
   temp |= (value);\
   DRV_WriteReg(addr,temp);\
}

#define FG_DRV_ClearBits16(addr, data)           DRV_ClearBits(addr,data)
#define FG_DRV_SetBits16(addr, data)             DRV_SetBits(addr,data)
#define FG_DRV_WriteReg16(addr, data)            DRV_WriteReg(addr, data)
#define FG_DRV_ReadReg16(addr)                   DRV_Reg(addr)
#define FG_DRV_SetData16(addr, bitmask, value)   DRV_SetData(addr, bitmask, value)

///////////////////////////////////////////////////////////////////////////////////////////
// Common API
///////////////////////////////////////////////////////////////////////////////////////////
kal_int32 FGADC_E1_SW_Simulate_Coulomb_Counter(void);
kal_int32 fgauge_read_r_bat_by_v(kal_int32 voltage);
kal_int32 fgauge_get_Q_max(kal_int16 temperature);
kal_int32 fgauge_get_Q_max_high_current(kal_int16 temperature);


void MTKFG_PLL_Control(kal_bool en)	   //True means turn on, False means turn off
{  
	kal_uint16 Temp_Reg=0;
	
    if(en == KAL_TRUE)
    {
	    //Temp_Reg = INREG16(PLL_CON6_BASE_ADDR) | 0x40;		
	    //OUTREG16(PLL_CON6_BASE_ADDR, Temp_Reg);
		
	    Temp_Reg = INREG16(FG_PLL_CON0_BASE_ADDR) | 0x201;	
	    OUTREG16(FG_PLL_CON0_BASE_ADDR, Temp_Reg);
		
	    printk("[MT6573-FGDBG] MTKFG_PLL_Control ---ON (0x%x) !\r\n", INREG16(FG_PLL_CON0_BASE_ADDR)); 
    }
    else
    {
	    //Temp_Reg = INREG16(PLL_CON6_BASE_ADDR) & (~0x40);
	    //OUTREG16(PLL_CON6_BASE_ADDR, Temp_Reg);

	    //Temp_Reg = INREG16(FG_PLL_CON0_BASE_ADDR) & (~0x201);
	    Temp_Reg = INREG16(FG_PLL_CON0_BASE_ADDR) & (~0x0001);
	    OUTREG16(FG_PLL_CON0_BASE_ADDR, Temp_Reg);
		
	    printk("[MT6573-FGDBG] MTKFG_PLL_Control ---OFF (0x%x) !\r\n", INREG16(FG_PLL_CON0_BASE_ADDR)); 
    }
   
}

void fg_reset_sw_control(fg_reset_sw_control_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_SW_RSTCLR_MASK, ((kal_uint16)val << FG_SW_RSTCLR_SHIFT));
}

void fg_sw_clear(fg_sw_clear_bit_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_SW_CLEAR_MASK, ((kal_uint16)val << FG_SW_CLEAR_SHIFT));
}

kal_uint16 fg_get_data_ready_status(void)
{
	kal_uint16 val;
	
	val = FG_DRV_ReadReg16(FGADC_CON0);	
	val = (val & FG_LATCHDATA_ST_MASK) >> FG_LATCHDATA_ST_SHIFT;

	return (kal_uint16)val;
}

void fg_sw_read_command(fg_sw_read_command_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_SW_READ_PRE_MASK, ((kal_uint16)val << FG_SW_READ_PRE_SHIFT));
}

void fg_sw_control_fg_behavior(fg_sw_control_fg_behavior_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_SW_CR_MASK, ((kal_uint16)val << FG_SW_CR_SHIFT));
}

void fg_sw_reset_fg_time(fg_sw_reset_fg_time_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_TIME_RST_MASK, ((kal_uint16)val << FG_TIME_RST_SHIFT));
}

void fg_sw_reset_fg_charge(fg_sw_reset_fg_charge_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_CHARGE_RST_MASK, ((kal_uint16)val << FG_CHARGE_RST_SHIFT));
}

void fg_interrupt_enable(kal_bool enable)
{
	FG_DRV_SetData16(FGADC_CON0, FG_INT_EN_MASK, ((kal_uint16)enable << FG_INT_EN_SHIFT));
}

void fg_set_auto_calibration_rate(fg_calibration_rate_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_AUTOCALRATE_MASK, ((kal_uint16)val << FG_AUTOCALRATE_SHIFT));
}

void fg_set_calibration_type(fg_calibration_type_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_CAL_MASK, ((kal_uint16)val << FG_CAL_SHIFT));
}

void fg_set_hw_active(fg_active_enum val)
{
	FG_DRV_SetData16(FGADC_CON0, FG_ON_MASK, ((kal_uint16)val << FG_ON_SHIFT));
}

kal_uint16 fg_get_car(void)
{
	kal_uint16 val;
	
	val = FG_DRV_ReadReg16(FGADC_CON1);	
	if (Enable_FGADC_LOG == 1){
		printk("[fg_get_car] 0x%x \r\n", val);
	}	
	
	val = (val & FG_CAR_MASK) >> FG_CAR_SHIFT;	

	return (kal_uint16)val;
}

kal_uint16 fg_get_nter(void)
{
	kal_uint16 val;
	
	val = FG_DRV_ReadReg16(FGADC_CON2);	
	val = (val & FG_NTER_MASK) >> FG_NTER_SHIFT;

	return (kal_uint16)val;
}

void fg_set_battery_low_threshold(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON3, FG_BLTR_MASK, ((kal_uint16)val << FG_BLTR_SHIFT));
}

void fg_set_battery_full_threshold(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON4, FG_BFTR_MASK, ((kal_uint16)val << FG_BFTR_SHIFT));
}

kal_uint16 fg_get_current(void)
{
	kal_uint16 val;
	
	val = FG_DRV_ReadReg16(FGADC_CON5);	
	val = (val & FG_CURRENT_OUT_MASK) >> FG_CURRENT_OUT_SHIFT;

	return (kal_uint16)val;
}

void fg_set_adjust_offset_value(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON6, FG_ADJUST_OFFSET_VALUE_MASK, ((kal_uint16)val << FG_ADJUST_OFFSET_VALUE_SHIFT));
}

fg_interrupt_check_enum fg_get_higher_or_lower_thd(void)
{
	kal_uint16 val;
	
	val = FG_DRV_ReadReg16(FGADC_CON7);	
	val = (val & FG_ISR_MASK) >> FG_ISR_SHIFT;

	if ( val == 1 )	
	{
		return HIGHER_THAN_THD;
	} 
	else if ( val == 2) 
	{
		return LOWER_THAN_THD;
	} 
	else 
	{
		return HW_ERROR;
	}
}

int fgauge_get_saddles(void)
{
    return sizeof(battery_profile_t2) / sizeof(BATTERY_PROFILE_STRUC);
}

int fgauge_get_saddles_r_table(void)
{
    return sizeof(r_profile_t2) / sizeof(R_PROFILE_STRUC);
}

BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature)
{
    switch (temperature)
    {
        case TEMPERATURE_T0:
            return &battery_profile_t0[0];
            break;    
        case TEMPERATURE_T1:
            return &battery_profile_t1[0];
            break;
        case TEMPERATURE_T2:
            return &battery_profile_t2[0];
            break;
        case TEMPERATURE_T3:
            return &battery_profile_t3[0];
            break;
        case TEMPERATURE_T:
            return &battery_profile_temperature[0];
            break;
        default:
            return NULL;
            break;
    }
}

R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature)
{
    switch (temperature)
    {
    	case TEMPERATURE_T0:
            return &r_profile_t0[0];
            break;
        case TEMPERATURE_T1:
            return &r_profile_t1[0];
            break;
        case TEMPERATURE_T2:
            return &r_profile_t2[0];
            break;
        case TEMPERATURE_T3:
            return &r_profile_t3[0];
            break;
        case TEMPERATURE_T:
            return &r_profile_temperature[0];
            break;
        default:
            return NULL;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// HW Test Mode
///////////////////////////////////////////////////////////////////////////////////////////
kal_uint16 fg_get_reset_detect_debug(void)
{
	kal_uint16 val;
	
	val = FG_DRV_ReadReg16(FGADC_CON7);	
	val = (val & FG_ADC_RSTDETECT_MASK) >> FG_ADC_RSTDETECT_MASK;

	return (kal_uint16)val;
}

void fg_adc_auto_reset(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON7, FG_ADC_AUTORST_MASK, ((kal_uint16)val << FG_ADC_AUTORST_SHIFT));
}

void fg_dig_test(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON7, FG_DIG_TEST_MASK, ((kal_uint16)val << FG_DIG_TEST_SHIFT));
}

void fg_fir_2_bypass(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON7, FG_FIR2BYPASS_MASK, ((kal_uint16)val << FG_FIR2BYPASS_SHIFT));
}

void fg_fir_1_bypass(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON7, FG_FIR1BYPASS_MASK, ((kal_uint16)val << FG_FIR1BYPASS_SHIFT));
}

void fg_set_over_sampling_rate(kal_uint16 val)
{
	FG_DRV_SetData16(FGADC_CON7, FG_OSR_MASK, ((kal_uint16)val << FG_OSR_SHIFT));
}

///////////////////////////////////////////////////////////////////////////////////////////
// Global Variable
///////////////////////////////////////////////////////////////////////////////////////////
kal_int8 gFG_DOD0_update = 0;
kal_int32 gFG_DOD0 = 0;
kal_int32 gFG_DOD1 = 0;
kal_int32 gFG_DOD1_return = 0;
kal_int32 gFG_columb = 0;
kal_int32 gFG_voltage = 0;
kal_int32 gFG_voltage_pre = -500;
kal_int32 gFG_current = 0;
kal_int32 gFG_capacity = 0;
kal_int32 gFG_capacity_by_c = 0;
kal_int32 gFG_capacity_by_c_init = 0;
kal_int32 gFG_capacity_by_v = 0;
kal_int32 gFG_columb_init = 0;
kal_int32 gFG_inner_R = 0;
kal_int16 gFG_temp= 100;
kal_int16 gFG_pre_temp=100;
kal_int16 gFG_T_changed=5;
kal_int32 gEstBatCapacity = 0;
kal_int32 gFG_SW_CoulombCounter = 0;
kal_bool gFG_Is_Charging = KAL_FALSE;
kal_int32 gFG_bat_temperature = 0;
kal_int32 gFG_resistance_bat = 0;
kal_int32 gFG_compensate_value = 0;
kal_int32 gFG_ori_voltage = 0;
kal_int32 gFG_booting_counter_I = 0;
kal_int32 gFG_booting_counter_I_FLAG = 0;
int i_kthread_index=0;
kal_int32 gFG_BATT_CAPACITY = 0;
int vchr_kthread_index=0;
kal_int32 gFG_voltage_init=0;
kal_int32 gFG_current_auto_detect_R_fg_total=0;
kal_int32 gFG_current_auto_detect_R_fg_count=0;
kal_int32 gFG_current_auto_detect_R_fg_result=0;
kal_int32 gFG_current_inout_battery = 0;
int gFG_15_vlot=3700;

void FGADC_dump_parameter(void)
{
	//printk("[FGADC] FG Columb  : %d mAh \r\n", gFG_columb);
    printk("[FGADC] FG Voltage : %d mV \r\n", gFG_voltage);
    //printk("[FGADC] FG Current : %d mA \r\n", gFG_current);
    //printk("[FGADC] FG inner R : %d mOhm \r\n", gFG_inner_R);
    printk("[FGADC] FG Capacity by Voltage: %d percent\r\n", gFG_capacity_by_v);
    printk("[FGADC] FG Capacity by Columb : %d percent\r\n", gFG_capacity_by_c);
    //printk("[FGADC] FG Estimate Capacity  : %d percent\r\n", gEstBatCapacity);    
    printk("[FGADC] FG gFG_capacity_by_c_init    : %d percent \r\n", gFG_capacity_by_c_init);
	//printk("[FGADC] FG gFG_DOD0_update    : %d \r\n", gFG_DOD0_update);
}

///////////////////////////////////////////////////////////////////////////////////////////
// SW algorithm
///////////////////////////////////////////////////////////////////////////////////////////

kal_int32 fgauge_read_temperature(void)
{
	int bat_temperature_volt=0;
	int bat_temperature=0;
		
	bat_temperature_volt = IMM_GetOneChannelValue(AUXADC_TEMPERATURE_CHANNEL,5);
	bat_temperature = BattVoltToTemp(bat_temperature_volt);
	gFG_bat_temperature = bat_temperature;

    return bat_temperature;
}

kal_int32 fgauge_read_columb(void)
{
	kal_uint16 uvalue16_CAR = 0;
	kal_uint16 uvalue16_NTER = 0;
    kal_int32 dvalue_CAR = 0;
    kal_int32 dvalue_NTER = 0;	
	int m = 0;
	kal_uint16 Temp_Reg = 0;
	int Temp_Value = 0;

    // 0. clear LATCHDATA_ST
    //fg_sw_clear(CLEAR_BIT);
	//fg_set_hw_active(FG_ACTIVE);
	//Temp_Reg = INREG16(FGADC_CON0) & 0x1002;
    //OUTREG16(FGADC_CON0, Temp_Reg);

	// 1. SW execute the read cmd	
    //fg_sw_read_command(CMD_READ_FG_DATA);
	//fg_set_hw_active(FG_ACTIVE);
	
	//Temp_Reg = 0x1008;
	Temp_Reg = 0x1888;
    OUTREG16(FGADC_CON0, Temp_Reg);

    // 2. wait the data ready	
    while ( fg_get_data_ready_status() == 0 )
    {
        Temp_Reg = INREG16(FGADC_CON0);
        printk("0x%d,",Temp_Reg);
		m++;
		if(m>1000)
		{
			printk("fg_get_data_ready_status timeout 1 !\r\n");
			break;
		}
    }    

    // 3. read row data 	
    uvalue16_CAR = fg_get_car();
    uvalue16_NTER = fg_get_nter();
	//printk("[FGADC] fgauge_read_columb : FG_CAR = %x\r\n", uvalue16_CAR);
    //printk("[FGADC] fgauge_read_columb : FG_NTER = %x\r\n", uvalue16_NTER);    	

    // 4. calculate the real world data	
    dvalue_CAR = (kal_int32) uvalue16_CAR;	
	if( dvalue_CAR == 0 )
	{
		//dis-charging
		Temp_Value = (int) dvalue_CAR;
	}
	else if( dvalue_CAR > 32767 ) // > 0x8000
	{
		//dis-charging
		Temp_Value = dvalue_CAR - 65535; // keep negative value
		//Temp_Value = Temp_Value - (Temp_Value*2);	
	}
	else
	{
		//charging
		Temp_Value = (int) dvalue_CAR;
	}
	//Temp_Value = ( ( (Temp_Value * UNIT_FGCHARGE) / 1000000000 ) * 16384 ) / 1000; //mAh
	Temp_Value = ((((Temp_Value*1161)/10)/100)+(5))/10;	
	dvalue_CAR = Temp_Value;

	if (Enable_FGADC_LOG == 1) {
		printk("[FGADC] fgauge_read_columb : dvalue_CAR = %d\r\n", dvalue_CAR);
	}
	
	#if (OSR_SELECT_7 == 1)
		dvalue_CAR = dvalue_CAR * 8;
		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] fgauge_read_columb : dvalue_CAR update to %d\r\n", dvalue_CAR);
		}
	#endif	
		
    dvalue_NTER = (kal_int32) uvalue16_NTER;
	dvalue_NTER = (dvalue_NTER * UNIT_FGTIME) / 100; // s
	
	/* Auto adjust value */
	if(R_FG_VALUE != 20)
	{
		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] Auto adjust value deu to the Rfg is %d\n Ori CAR=%d, ", R_FG_VALUE, dvalue_CAR);			
		}
		dvalue_CAR = (dvalue_CAR*20)/R_FG_VALUE;
		if (Enable_FGADC_LOG == 1) {
			printk("new CAR=%d\n", dvalue_CAR);			
		}

		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] Auto adjust value deu to the Rfg is %d\n Ori NTER=%d, ", R_FG_VALUE, dvalue_NTER);			
		}
		dvalue_NTER = (dvalue_NTER*20)/R_FG_VALUE;
		if (Enable_FGADC_LOG == 1) {
			printk("new NTER=%d\n", dvalue_NTER);			
		}
	}
	
	//printk("[FGADC] fgauge_read_columb : dvalue_CAR = %d mAh\r\n", dvalue_CAR);
    //printk("[FGADC] fgauge_read_columb : dvalue_NTER = %d\r\n", dvalue_NTER);

    // 5. clear the FG_LATCHDATA_ST bit
    //fg_sw_clear(CLEAR_BIT);
	//fg_set_hw_active(FG_ACTIVE);

	//Temp_Reg = 0x1002;
	Temp_Reg = 0x1882;
    OUTREG16(FGADC_CON0, Temp_Reg);

    return dvalue_CAR;
}


kal_int32 fgauge_read_columb_reset(void)
{
	kal_uint16 uvalue16_CAR = 0;
	kal_uint16 uvalue16_NTER = 0;
    kal_int32 dvalue_CAR = 0;
    kal_int32 dvalue_NTER = 0;	
	int m = 0;
	kal_uint16 Temp_Reg = 0;
	int Temp_Value = 0;

    // 0. clear LATCHDATA_ST
    //fg_sw_clear(CLEAR_BIT);
	//fg_set_hw_active(FG_ACTIVE);
	//Temp_Reg = INREG16(FGADC_CON0) & 0x1002;
    //OUTREG16(FGADC_CON0, Temp_Reg);

	// 1. SW execute the read cmd	
    //fg_sw_read_command(CMD_READ_FG_DATA);
	//fg_set_hw_active(FG_ACTIVE);
	
	//Temp_Reg = 0x1008;
	Temp_Reg = 0x18F8;
    OUTREG16(FGADC_CON0, Temp_Reg);

    // 2. wait the data ready	
    while ( fg_get_data_ready_status() == 0 )
    {
        Temp_Reg = INREG16(FGADC_CON0);
        printk("0x%d,",Temp_Reg);
		m++;
		if(m>1000)
		{
			printk("fg_get_data_ready_status timeout 1 !\r\n");
			break;
		}
    }    

    // 3. read row data 	
    uvalue16_CAR = fg_get_car();
    uvalue16_NTER = fg_get_nter();
	//printk("[FGADC] fgauge_read_columb : FG_CAR = %x\r\n", uvalue16_CAR);
    //printk("[FGADC] fgauge_read_columb : FG_NTER = %x\r\n", uvalue16_NTER);    	

    // 4. calculate the real world data	
    dvalue_CAR = (kal_int32) uvalue16_CAR;	
	if( dvalue_CAR == 0 )
	{
		//dis-charging
		Temp_Value = (int) dvalue_CAR;
	}
	else if( dvalue_CAR > 32767 ) // > 0x8000
	{
		//dis-charging
		Temp_Value = dvalue_CAR - 65535; // keep negative value
		//Temp_Value = Temp_Value - (Temp_Value*2);	
	}
	else
	{
		//charging
		Temp_Value = (int) dvalue_CAR;
	}
	//Temp_Value = ( ( (Temp_Value * UNIT_FGCHARGE) / 1000000000 ) * 16384 ) / 1000; //mAh
	Temp_Value = ((((Temp_Value*1161)/10)/100)+(5))/10;	
	dvalue_CAR = Temp_Value;

	if (Enable_FGADC_LOG == 1) {
		printk("[FGADC] fgauge_read_columb : dvalue_CAR = %d\r\n", dvalue_CAR);
	}
	
	#if (OSR_SELECT_7 == 1)
		dvalue_CAR = dvalue_CAR * 8;
		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] fgauge_read_columb : dvalue_CAR update to %d\r\n", dvalue_CAR);
		}
	#endif	
		
    dvalue_NTER = (kal_int32) uvalue16_NTER;
	dvalue_NTER = (dvalue_NTER * UNIT_FGTIME) / 100; // s
	
	/* Auto adjust value */
	if(R_FG_VALUE != 20)
	{
		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] Auto adjust value deu to the Rfg is %d\n Ori CAR=%d, ", R_FG_VALUE, dvalue_CAR);			
		}
		dvalue_CAR = (dvalue_CAR*20)/R_FG_VALUE;
		if (Enable_FGADC_LOG == 1) {
			printk("new CAR=%d\n", dvalue_CAR);			
		}

		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] Auto adjust value deu to the Rfg is %d\n Ori NTER=%d, ", R_FG_VALUE, dvalue_NTER);			
		}
		dvalue_NTER = (dvalue_NTER*20)/R_FG_VALUE;
		if (Enable_FGADC_LOG == 1) {
			printk("new NTER=%d\n", dvalue_NTER);			
		}
	}
	
	//printk("[FGADC] fgauge_read_columb : dvalue_CAR = %d mAh\r\n", dvalue_CAR);
    //printk("[FGADC] fgauge_read_columb : dvalue_NTER = %d\r\n", dvalue_NTER);

    // 5. clear the FG_LATCHDATA_ST bit
    //fg_sw_clear(CLEAR_BIT);
	//fg_set_hw_active(FG_ACTIVE);

	//Temp_Reg = 0x1002;
	Temp_Reg = 0x1882;
    OUTREG16(FGADC_CON0, Temp_Reg);

    return dvalue_CAR;
}

kal_int32 current_get_ori=0;

kal_int32 fgauge_read_current(void)
{
    //kal_int16 ivalue16 = 0;
    kal_uint16 uvalue16 = 0;
    kal_int32 dvalue = 0; 
	int m = 0;
	kal_uint16 Temp_Reg = 0;
	int Temp_Value = 0;
	//kal_int32 Current_Compensate_Value=300;
	kal_int32 Current_Compensate_Value=0;

	mutex_lock(&FGCURRENT_mutex);

    // 0. clear LATCHDATA_ST
    //fg_sw_clear(CLEAR_BIT);
	//fg_set_hw_active(FG_ACTIVE);
	//Temp_Reg = INREG16(FGADC_CON0) & 0x1002;
    //OUTREG16(FGADC_CON0, Temp_Reg);

	// 1. SW execute the read cmd	
    //fg_sw_read_command(CMD_READ_FG_DATA);
	//fg_set_hw_active(FG_ACTIVE);

	//Temp_Reg = 0x1008;
	Temp_Reg = 0x1888;
    OUTREG16(FGADC_CON0, Temp_Reg);

    // 2. wait the data ready	
    while ( fg_get_data_ready_status() == 0 )
    {
        Temp_Reg = INREG16(FGADC_CON0);
        //printk("0x%d,",Temp_Reg);
		m++;
		if(m>1000)
		{
			printk("fg_get_data_ready_status timeout 2 !\r\n");
			break;
		}
    }

	// 3. read row data 
    uvalue16 = fg_get_current();
    //printk("[FGADC] fgauge_read_current : FG_CURRENT = %x\r\n", uvalue16);

	// 4. calculate the real world data	
    dvalue = (kal_uint32) uvalue16;
	if( dvalue == 0 )
	{
		Temp_Value = (int) dvalue;
		gFG_Is_Charging = KAL_FALSE;
	}
	else if( dvalue > 32767 ) // > 0x8000
	{
		Temp_Value = dvalue - 65535;
		Temp_Value = Temp_Value - (Temp_Value*2);
		gFG_Is_Charging = KAL_FALSE;
	}
	else
	{
		Temp_Value = (int) dvalue;
		gFG_Is_Charging = KAL_TRUE;
	}
	//dvalue = (kal_uint32) ((Temp_Value * UNIT_FGCURRENT) / 1000000); //mA
	dvalue = (kal_uint32) ((Temp_Value * UNIT_FGCURRENT) / 100000);   	

	current_get_ori = dvalue;

#if 0	
	if( gFG_Is_Charging == KAL_TRUE )
	{
		printk("[FGADC] fgauge_read_current : current(charging) = %d mA\r\n", dvalue);
	}
	else
	{
		printk("[FGADC] fgauge_read_current : current(discharging) = %d mA\r\n", dvalue);
	}
#endif	

    // 5. clear the FG_LATCHDATA_ST bit
    //fg_sw_clear(CLEAR_BIT);
	//fg_set_hw_active(FG_ACTIVE);
	
	//Temp_Reg = 0x1002;
	Temp_Reg = 0x1882;
    OUTREG16(FGADC_CON0, Temp_Reg);

	/* Auto adjust value */
	if(R_FG_VALUE != 20)
	{
		if (Enable_FGADC_LOG == 1) {
			printk("[FGADC] Auto adjust value deu to the Rfg is %d\n Ori current=%d, ", R_FG_VALUE, dvalue);			
		}
		dvalue = (dvalue*20)/R_FG_VALUE;
		if (Enable_FGADC_LOG == 1) {
			printk("new current=%d\n", dvalue);			
		}
	}

	/* K current */
	if(R_FG_BOARD_SLOPE != R_FG_BOARD_BASE)
	{
		dvalue = ( (dvalue*R_FG_BOARD_BASE) + (R_FG_BOARD_SLOPE/2) ) / R_FG_BOARD_SLOPE;
	}

	/* current compensate */
	if(gFG_Is_Charging==KAL_TRUE)
	{
		dvalue = dvalue + Current_Compensate_Value;
	}
	else
	{
		dvalue = dvalue - Current_Compensate_Value;
	}

#if defined(CONFIG_POWER_EXT)
	// do nothing
#else
	dvalue = ((dvalue*94)/100);
#endif

	mutex_unlock(&FGCURRENT_mutex);

    return dvalue;
}

kal_int32 fgauge_read_voltage(void)
{	
    int vol_battery;

	upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
	vol_battery = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,200) * 2;

	if(gFG_voltage_pre == -500)
	{
		gFG_voltage_pre = vol_battery; // for init
		
		return vol_battery;
	}

    return vol_battery;
}

kal_int32 fgauge_compensate_battery_voltage(kal_int32 ori_voltage)
{
	kal_int32 ret_compensate_value = 0;

	gFG_ori_voltage = ori_voltage;

	gFG_resistance_bat = fgauge_read_r_bat_by_v(ori_voltage); // Ohm

	ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE)) / 1000;
	//ret_compensate_value = (gFG_current * (gFG_resistance_bat - R_FG_VALUE)) / 1000;
	ret_compensate_value = (ret_compensate_value+(10/2)) / 10; // 20101103

    if (gFG_Is_Charging == KAL_TRUE) 
    {
    	/* charging, COMPASATE_OCV is negitive */
        //return 0;

		ret_compensate_value = ret_compensate_value - (ret_compensate_value*2);
    }
    else
    {
        /* discharging, COMPASATE_OCV is positive */		
        //return COMPASATE_OCV;		
    }

	gFG_compensate_value = ret_compensate_value;

	//printk("[CompensateVoltage] Ori_voltage:%d, compensate_value:%d, gFG_resistance_bat:%d, gFG_current:%d\r\n", 
	//	ori_voltage, ret_compensate_value, gFG_resistance_bat, gFG_current);

    return ret_compensate_value;
}

kal_int32 fgauge_compensate_battery_voltage_recursion(kal_int32 ori_voltage, kal_int32 recursion_time)
{
	kal_int32 ret_compensate_value = 0;
	kal_int32 temp_voltage_1 = ori_voltage;
	kal_int32 temp_voltage_2 = temp_voltage_1;
	int i = 0;

	for(i=0 ; i < recursion_time ; i++) 
	{
		gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); // Ohm
		ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE)) / 1000;
		//ret_compensate_value = (gFG_current * (gFG_resistance_bat - R_FG_VALUE)) / 1000;
		ret_compensate_value = (ret_compensate_value+(10/2)) / 10; // 20101103
		
	    if (gFG_Is_Charging == KAL_TRUE) 
	    {
			ret_compensate_value = ret_compensate_value - (ret_compensate_value*2);
	    }
		temp_voltage_2 = temp_voltage_1 + ret_compensate_value;

		//if(gFG_booting_counter_I_FLAG != 2)
		if (Enable_FGADC_LOG == 1)
		{
		printk("[fgauge_compensate_battery_voltage_recursion] %d,%d,%d,%d\r\n", 
			temp_voltage_1, temp_voltage_2, gFG_resistance_bat, ret_compensate_value);
		}
		
		//temp_voltage_1 = temp_voltage_2;		
	}
	
	gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); // Ohm
	//ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE)) / 1000;
	ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE + FG_METER_RESISTANCE)) / 1000;	
	//ret_compensate_value = (gFG_current * (gFG_resistance_bat - R_FG_VALUE)) / 1000;
	ret_compensate_value = (ret_compensate_value+(10/2)) / 10; // 20101103
	
    if (gFG_Is_Charging == KAL_TRUE) 
    {
		ret_compensate_value = ret_compensate_value - (ret_compensate_value*2);
    }

	gFG_compensate_value = ret_compensate_value;

	//if(gFG_booting_counter_I_FLAG != 2)
	if (Enable_FGADC_LOG == 1)
	{
	printk("[fgauge_compensate_battery_voltage_recursion] %d,%d,%d,%d\r\n", 
			temp_voltage_1, temp_voltage_2, gFG_resistance_bat, ret_compensate_value);
	}

	//printk("[CompensateVoltage] Ori_voltage:%d, compensate_value:%d, gFG_resistance_bat:%d, gFG_current:%d\r\n", 
	//	ori_voltage, ret_compensate_value, gFG_resistance_bat, gFG_current);

    return ret_compensate_value;
}

void fgauge_construct_battery_profile(kal_int32 temperature, BATTERY_PROFILE_STRUC_P temp_profile_p)
{
    BATTERY_PROFILE_STRUC_P low_profile_p, high_profile_p;
    kal_int32 low_temperature, high_temperature;
    int i, saddles;
	kal_int32 temp_v_1 = 0, temp_v_2 = 0;

	if (temperature <= TEMPERATURE_T1)
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T0);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T1);
        low_temperature  = (-10);
        high_temperature = TEMPERATURE_T1;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T1);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T2);
        low_temperature  = TEMPERATURE_T1;
        high_temperature = TEMPERATURE_T2;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
    else
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T2);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T3);
        low_temperature  = TEMPERATURE_T2;
        high_temperature = TEMPERATURE_T3;
		
		if(temperature > high_temperature)
		{
			temperature = high_temperature;
		}
    }

    saddles = fgauge_get_saddles();

    for (i = 0; i < saddles; i++)
    {
		if( ((high_profile_p + i)->voltage) > ((low_profile_p + i)->voltage) )
		{
			temp_v_1 = (high_profile_p + i)->voltage;
			temp_v_2 = (low_profile_p + i)->voltage;	

			(temp_profile_p + i)->voltage = temp_v_2 +
			(
				(
					(temperature - low_temperature) * 
					(temp_v_1 - temp_v_2)
				) / 
				(high_temperature - low_temperature)				
			);
		}
		else
		{
			temp_v_1 = (low_profile_p + i)->voltage;
			temp_v_2 = (high_profile_p + i)->voltage;

			(temp_profile_p + i)->voltage = temp_v_2 +
			(
				(
					(high_temperature - temperature) * 
					(temp_v_1 - temp_v_2)
				) / 
				(high_temperature - low_temperature)				
			);
		}
	
        (temp_profile_p + i)->percentage = (high_profile_p + i)->percentage;
#if 0		
		(temp_profile_p + i)->voltage = temp_v_2 +
			(
				(
					(temperature - low_temperature) * 
					(temp_v_1 - temp_v_2)
				) / 
				(high_temperature - low_temperature)				
			);
#endif
    }

	
	// Dumpt new battery profile
	for (i = 0; i < saddles ; i++)
	{
		printk("<DOD,Voltage> at %d = <%d,%d>\r\n",temperature, (temp_profile_p+i)->percentage, (temp_profile_p+i)->voltage);
	}
	
}

void fgauge_construct_r_table_profile(kal_int32 temperature, R_PROFILE_STRUC_P temp_profile_p)
{
    R_PROFILE_STRUC_P low_profile_p, high_profile_p;
    kal_int32 low_temperature, high_temperature;
    int i, saddles;
	kal_int32 temp_v_1 = 0, temp_v_2 = 0;
	kal_int32 temp_r_1 = 0, temp_r_2 = 0;

	if (temperature <= TEMPERATURE_T1)
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T0);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T1);
        low_temperature  = (-10);
        high_temperature = TEMPERATURE_T1;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T1);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T2);
        low_temperature  = TEMPERATURE_T1;
        high_temperature = TEMPERATURE_T2;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
    else
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T2);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T3);
        low_temperature  = TEMPERATURE_T2;
        high_temperature = TEMPERATURE_T3;
		
		if(temperature > high_temperature)
		{
			temperature = high_temperature;
		}
    }

    saddles = fgauge_get_saddles_r_table();

	/* Interpolation for V_BAT */
    for (i = 0; i < saddles; i++)
    {
		if( ((high_profile_p + i)->voltage) > ((low_profile_p + i)->voltage) )
		{
			temp_v_1 = (high_profile_p + i)->voltage;
			temp_v_2 = (low_profile_p + i)->voltage;	

			(temp_profile_p + i)->voltage = temp_v_2 +
			(
				(
					(temperature - low_temperature) * 
					(temp_v_1 - temp_v_2)
				) / 
				(high_temperature - low_temperature)				
			);
		}
		else
		{
			temp_v_1 = (low_profile_p + i)->voltage;
			temp_v_2 = (high_profile_p + i)->voltage;

			(temp_profile_p + i)->voltage = temp_v_2 +
			(
				(
					(high_temperature - temperature) * 
					(temp_v_1 - temp_v_2)
				) / 
				(high_temperature - low_temperature)				
			);
		}

#if 0	
        //(temp_profile_p + i)->resistance = (high_profile_p + i)->resistance;
		
		(temp_profile_p + i)->voltage = temp_v_2 +
			(
				(
					(temperature - low_temperature) * 
					(temp_v_1 - temp_v_2)
				) / 
				(high_temperature - low_temperature)				
			);
#endif
    }

	/* Interpolation for R_BAT */
    for (i = 0; i < saddles; i++)
    {
		if( ((high_profile_p + i)->resistance) > ((low_profile_p + i)->resistance) )
		{
			temp_r_1 = (high_profile_p + i)->resistance;
			temp_r_2 = (low_profile_p + i)->resistance;	

			(temp_profile_p + i)->resistance = temp_r_2 +
			(
				(
					(temperature - low_temperature) * 
					(temp_r_1 - temp_r_2)
				) / 
				(high_temperature - low_temperature)				
			);
		}
		else
		{
			temp_r_1 = (low_profile_p + i)->resistance;
			temp_r_2 = (high_profile_p + i)->resistance;

			(temp_profile_p + i)->resistance = temp_r_2 +
			(
				(
					(high_temperature - temperature) * 
					(temp_r_1 - temp_r_2)
				) / 
				(high_temperature - low_temperature)				
			);
		}

#if 0	
        //(temp_profile_p + i)->voltage = (high_profile_p + i)->voltage;
		
		(temp_profile_p + i)->resistance = temp_r_2 +
			(
				(
					(temperature - low_temperature) * 
					(temp_r_1 - temp_r_2)
				) / 
				(high_temperature - low_temperature)				
			);
#endif
    }

	// Dumpt new r-table profile
	for (i = 0; i < saddles ; i++)
	{
		printk("<Rbat,VBAT> at %d = <%d,%d>\r\n",temperature, (temp_profile_p+i)->resistance, (temp_profile_p+i)->voltage);
	}
	
}


kal_int32 fgauge_get_dod0(kal_int32 voltage, kal_int32 temperature, kal_bool bOcv)
{
    kal_int32 dod0 = 0;
    int i, saddles;
    BATTERY_PROFILE_STRUC_P profile_p;
	R_PROFILE_STRUC_P profile_p_r_table;

/* R-Table (First Time) */	
    // Re-constructure r-table profile according to current temperature
    profile_p_r_table = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p_r_table == NULL)
    {
		printk("[FGADC] fgauge_get_profile_r_table : create table fail !\r\n");
    }
    fgauge_construct_r_table_profile(temperature, profile_p_r_table);

/* <DOD,VBAT> Table (First Time) */
    // handle invalid battery voltage
    //if (voltage < BATTERY_VOLTAGE_MINIMUM)
    //{
    //    return 100;
    //}
    //if (voltage > BATTERY_VOLTAGE_MAXIMUM)
    //{
    //    return 0;
    //}

    // Re-constructure battery profile according to current temperature
    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
		printk("[FGADC] fgauge_get_profile : create table fail !\r\n");
        return 100;
    }
    fgauge_construct_battery_profile(temperature, profile_p);

    // Get total saddle points from the battery profile
    saddles = fgauge_get_saddles();

    // If the input voltage is not OCV, compensate to ZCV due to battery loading
    // Compasate battery voltage from current battery voltage
    if (bOcv == KAL_FALSE)
    {        
        //voltage = voltage + fgauge_compensate_battery_voltage(voltage); //mV
        voltage = voltage + fgauge_compensate_battery_voltage_recursion(voltage,5); //mV
        printk("[FGADC] compensate_battery_voltage, voltage=%d\r\n", voltage);
    }
	
    // If battery voltage is less then mimimum profile voltage, then return 100
    // If battery voltage is greater then maximum profile voltage, then return 0
	if (voltage > (profile_p+0)->voltage)
    {
        return 0;
    }    
    if (voltage < (profile_p+saddles-1)->voltage)
    {
        return 100;
    }

    // get DOD0 according to current temperature
    for (i = 0; i < saddles - 1; i++)
    {
		//printk("Try <%d,%d> on %d\r\n", (profile_p+i)->voltage, (profile_p+i)->percentage, voltage);
	
        if ((voltage <= (profile_p+i)->voltage) && (voltage >= (profile_p+i+1)->voltage))
        {
            dod0 = (profile_p+i)->percentage +
				(
					(
						( ((profile_p+i)->voltage) - voltage ) * 
						( ((profile_p+i+1)->percentage) - ((profile_p + i)->percentage) ) 
					) /
					( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
				);

			//printk("DOD=%d\r\n", dod0);
			
            break;
        }
    }

#if 0
	// Dumpt new battery profile
	for (i = 0; i < saddles ; i++)
	{
		printk("<Voltage,DOD> at %d = <%d,%d>\r\n",gFG_bat_temperature, (profile_p+i)->voltage, (profile_p+i)->percentage);
	}
#endif

    return dod0;
}

kal_int32 fgauge_update_dod(void)
{
    kal_int32 FG_dod_1 = 0;
	int adjust_coulomb_counter=CAR_TUNE_VALUE;

	if(gFG_DOD0 > 100)
	{
		gFG_DOD0=100;
		printk("[fgauge_update_dod] gFG_DOD0 set to 100, gFG_columb=%d\r\n", gFG_columb);
	}
	else if(gFG_DOD0 < 0)
	{
		gFG_DOD0=0;
		printk("[fgauge_update_dod] gFG_DOD0 set to 0, gFG_columb=%d\r\n", gFG_columb);
	}
	else
	{
	}	

	gFG_temp = fgauge_read_temperature();
	gFG_BATT_CAPACITY = fgauge_get_Q_max(gFG_temp);
	

	//FG_dod_1 =  gFG_DOD0 - ((gFG_columb*100)/BATT_CAPACITY);
	//FG_dod_1 =  gFG_DOD0 - ((gFG_columb*100)/gFG_BATT_CAPACITY);
	//FG_dod_1 =  gFG_DOD0 - (((gFG_columb*1000)/gFG_BATT_CAPACITY)+5)/10;
	FG_dod_1 =  gFG_DOD0 - ((( (gFG_columb*1000*adjust_coulomb_counter)/100 )/gFG_BATT_CAPACITY)+5)/10;
	
	if (Enable_FGADC_LOG == 1){
	printk("[fgauge_update_dod] FG_dod_1=%d, adjust_coulomb_counter=%d, gFG_columb=%d, gFG_DOD0=%d, gFG_temp=%d, gFG_BATT_CAPACITY=%d\r\n", 
		FG_dod_1, adjust_coulomb_counter, gFG_columb, gFG_DOD0, gFG_temp, gFG_BATT_CAPACITY);
	}

	if(FG_dod_1 > 100)
	{
		FG_dod_1=100;
		printk("[fgauge_update_dod] FG_dod_1 set to 100, gFG_columb=%d\r\n", gFG_columb);
	}
	else if(FG_dod_1 < 0)
	{
		FG_dod_1=0;
		printk("[fgauge_update_dod] FG_dod_1 set to 0, gFG_columb=%d\r\n", gFG_columb);
	}
	else
	{
	}

    return FG_dod_1;
}

kal_int32 fgauge_read_capacity(kal_int32 type)
{
    kal_int32 voltage;
    kal_int32 temperature;
    kal_int32 dvalue = 0;
	
	kal_int32 C_0mA=0;
	kal_int32 C_400mA=0;
	kal_int32 dvalue_new=0;

    if (type == 0) // for initialization
    {
        // Use voltage to calculate capacity
        voltage = fgauge_read_voltage(); // in unit of mV
        temperature = fgauge_read_temperature();		
        //dvalue = fgauge_get_dod0(voltage, temperature, KAL_TRUE); // need not compensate
        dvalue = fgauge_get_dod0(voltage, temperature, KAL_FALSE); // need compensate vbat
    }
    else
    {
        // Use DOD0 and columb counter to calculate capacity
        dvalue = fgauge_update_dod(); // DOD1 = DOD0 + (-CAR)/Qmax
    }
	//printk("[fgauge_read_capacity] %d\r\n", dvalue);

	gFG_DOD1 = dvalue;
	
	//User View on HT~LT----------------------------------------------------------
	gFG_temp = fgauge_read_temperature();
	C_0mA = fgauge_get_Q_max(gFG_temp);
	C_400mA = fgauge_get_Q_max_high_current(gFG_temp);
	if(C_0mA > C_400mA)
	{
		dvalue_new = (100-dvalue) - ( ( (C_0mA-C_400mA) * (dvalue) ) / C_400mA );
		dvalue = 100 - dvalue_new;
	}
	if (Enable_FGADC_LOG == 1){
		printk("[fgauge_read_capacity] %d,%d,%d,%d,%d,D1=%d,D0=%d\r\n", 
			gFG_temp, C_0mA, C_400mA, dvalue, dvalue_new, gFG_DOD1, gFG_DOD0);
	}
	//----------------------------------------------------------------------------

    //gFG_DOD1 = dvalue;	
    //dvalue = 100 - dvalue;

	gFG_DOD1_return = dvalue;
	dvalue = 100 - gFG_DOD1_return;

	if(dvalue <= 1)
	{
		dvalue=1;
		if (Enable_FGADC_LOG == 1){
			printk("[fgauge_read_capacity] dvalue<=1 and set dvalue=1 !!\r\n");
		}
	}

    return dvalue;
}

kal_int32 fgauge_read_capacity_by_v(void)
{	
	int i = 0, saddles = 0;
	BATTERY_PROFILE_STRUC_P profile_p;
	kal_int32 ret_percent = 0;

	profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
		printk("[FGADC] fgauge get ZCV profile : fail !\r\n");
        return 100;
    }

	saddles = fgauge_get_saddles();

	if (gFG_voltage > (profile_p+0)->voltage)
    {
    	//printk("[fgauge_read_capacity_by_v] 100:%d,%d\r\n", gFG_voltage, (profile_p+0)->voltage);
        return 100; // battery capacity, not dod
        //return 0;
    }    
    if (gFG_voltage < (profile_p+saddles-1)->voltage)
    {
    	//printk("[fgauge_read_capacity_by_v] 0:%d,%d\r\n", gFG_voltage, (profile_p+saddles-1)->voltage);
        return 0; // battery capacity, not dod
        //return 100;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((gFG_voltage <= (profile_p+i)->voltage) && (gFG_voltage >= (profile_p+i+1)->voltage))
        {
            ret_percent = (profile_p+i)->percentage +
				(
					(
						( ((profile_p+i)->voltage) - gFG_voltage ) * 
						( ((profile_p+i+1)->percentage) - ((profile_p + i)->percentage) ) 
					) /
					( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
				);		 
			
            break;
        }
		
		//printk("[fgauge_read_capacity_by_v] gFG_voltage=%d\r\n", gFG_voltage);
		//printk("[fgauge_read_capacity_by_v] (profile_p+i)->percentag=%d\r\n", (profile_p+i)->percentage);
		//printk("[fgauge_read_capacity_by_v] ((profile_p+i+1)->percentage)=%d\r\n", ((profile_p+i+1)->percentage));
		//printk("[fgauge_read_capacity_by_v] ((profile_p+i)->voltage)=%d\r\n", ((profile_p+i)->voltage));
		//printk("[fgauge_read_capacity_by_v] ((profile_p+i+1)->voltage) =%d\r\n", ((profile_p+i+1)->voltage));
    }
	ret_percent = 100 - ret_percent;

	return ret_percent;
}

kal_int32 fgauge_read_r_bat_by_v(kal_int32 voltage)
{	
	int i = 0, saddles = 0;
	R_PROFILE_STRUC_P profile_p;
	kal_int32 ret_r = 0;

	profile_p = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p == NULL)
    {
		printk("[FGADC] fgauge get R-Table profile : fail !\r\n");
        return (profile_p+0)->resistance;
    }

	saddles = fgauge_get_saddles_r_table();

	if (voltage > (profile_p+0)->voltage)
    {
        return (profile_p+0)->resistance; 
    }    
    if (voltage < (profile_p+saddles-1)->voltage)
    {
        return (profile_p+saddles-1)->resistance; 
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage >= (profile_p+i+1)->voltage))
        {
            ret_r = (profile_p+i)->resistance +
				(
					(
						( ((profile_p+i)->voltage) - voltage ) * 
						( ((profile_p+i+1)->resistance) - ((profile_p + i)->resistance) ) 
					) /
					( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
				);
            break;
        }
    }

	return ret_r;
}

kal_int32 fgauge_read_v_by_capacity(int bat_capacity)
{	
	int i = 0, saddles = 0;
	BATTERY_PROFILE_STRUC_P profile_p;
	kal_int32 ret_volt = 0;

	profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
		printk("[fgauge_read_v_by_capacity] fgauge get ZCV profile : fail !\r\n");
        return 3700;
    }

	saddles = fgauge_get_saddles();

	if (bat_capacity < (profile_p+0)->percentage)
    {    	
        return 3700;         
    }    
    if (bat_capacity > (profile_p+saddles-1)->percentage)
    {    	
        return 3700;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((bat_capacity >= (profile_p+i)->percentage) && (bat_capacity <= (profile_p+i+1)->percentage))
        {
            ret_volt = (profile_p+i)->voltage -
				(
					(
						( bat_capacity - ((profile_p+i)->percentage) ) * 
						( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) ) 
					) /
					( ((profile_p+i+1)->percentage) - ((profile_p+i)->percentage) )
				);		 

			printk("[fgauge_read_v_by_capacity] ret_volt=%d\r\n", ret_volt);
			printk("[fgauge_read_v_by_capacity] (profile_p+i)->percentag=%d\r\n", (profile_p+i)->percentage);
			printk("[fgauge_read_v_by_capacity] ((profile_p+i+1)->percentage)=%d\r\n", ((profile_p+i+1)->percentage));
			printk("[fgauge_read_v_by_capacity] ((profile_p+i)->voltage)=%d\r\n", ((profile_p+i)->voltage));
			printk("[fgauge_read_v_by_capacity] ((profile_p+i+1)->voltage) =%d\r\n", ((profile_p+i+1)->voltage));
			
            break;
        }		
    }	

	return ret_volt;
}

#define FG_VBAT_AVERAGE_SIZE 36 // 36*5s=180s=3mins
//#define MinErrorOffset 30 //30mA
//#define MinErrorOffset 50 //50mA
//#define MinErrorOffset 200 //200mV
#define MinErrorOffset 1000 //1000mV
kal_bool gFGvbatBufferFirst = KAL_FALSE;
static unsigned short FGvbatVoltageBuffer[FG_VBAT_AVERAGE_SIZE];
static int FGbatteryIndex = 0;
static int FGbatteryVoltageSum = 0;
kal_int32 gFG_voltage_AVG = 0;
kal_int32 gFG_vbat_offset=0;
kal_int32 gFG_voltageVBAT=0;

void FG_booting_protection(void)
{
	gFG_current = fgauge_read_current();
    gFG_voltage = fgauge_read_voltage();
	gFG_voltage_init = gFG_voltage;
		
	if (gFG_Is_Charging == KAL_TRUE)
	{
		gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,1); //mV
	}
	else
	{
		gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,5); //mV
	}
		
	gFG_voltage = gFG_voltage + OCV_BOARD_COMPESATE;
	
	gFG_capacity_by_v = fgauge_read_capacity_by_v();

	printk("[FG_booting_protection] gFG_capacity_by_v=%d, gFG_voltage=%d, gFG_current=%d, gFG_Is_Charging=%d\r\n", 
		gFG_capacity_by_v, gFG_voltage, gFG_current, gFG_Is_Charging);
}

void fgauge_Normal_Mode_Work(void)
{
	int i=0;
	
//1. Get Raw Data  
	gFG_current = fgauge_read_current();
    gFG_voltage = fgauge_read_voltage();
	gFG_voltage_init = gFG_voltage;
	
	if (gFG_Is_Charging == KAL_TRUE)
	{
		gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,1); //mV
	}
	else
	{
		gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,5); //mV
	}
	
	gFG_voltage = gFG_voltage + OCV_BOARD_COMPESATE;

	if(get_chip_eco_ver()==CHIP_E1)
	{
		gFG_columb = FGADC_E1_SW_Simulate_Coulomb_Counter(); // SW workaround on E1
	}
	else
	{		
		gFG_current = fgauge_read_current();
		gFG_columb = fgauge_read_columb();		
		//gFG_columb -= gFG_columb_init; // - the init value (this value is exist before SW start)	
	}

//1.1 Average FG_voltage
	/**************** Averaging : START ****************/
	if(gFG_booting_counter_I_FLAG != 0)
	{
	    if (!gFGvbatBufferFirst)
	    {	        	        
	        for (i=0; i<FG_VBAT_AVERAGE_SIZE; i++) {
	            FGvbatVoltageBuffer[i] = gFG_voltage;            
	        }

	        FGbatteryVoltageSum = gFG_voltage * FG_VBAT_AVERAGE_SIZE;

			gFG_voltage_AVG = gFG_voltage;

			gFGvbatBufferFirst = KAL_TRUE;
	    }

		if(gFG_voltage >= gFG_voltage_AVG)
		{
			gFG_vbat_offset = (gFG_voltage - gFG_voltage_AVG);
		}
		else
		{
			gFG_vbat_offset = (gFG_voltage_AVG - gFG_voltage);
		}

		if(gFG_vbat_offset <= MinErrorOffset)
		{
		    FGbatteryVoltageSum -= FGvbatVoltageBuffer[FGbatteryIndex];
		    FGbatteryVoltageSum += gFG_voltage;
		    FGvbatVoltageBuffer[FGbatteryIndex] = gFG_voltage;
		    
		    gFG_voltage_AVG = FGbatteryVoltageSum / FG_VBAT_AVERAGE_SIZE;
			gFG_voltage = gFG_voltage_AVG;

		    FGbatteryIndex++;
		    if (FGbatteryIndex >= FG_VBAT_AVERAGE_SIZE)
		        FGbatteryIndex = 0;

			if (Enable_FGADC_LOG == 1)
			{
				printk("[FG_BUFFER] ");
				for (i=0; i<FG_VBAT_AVERAGE_SIZE; i++) {
		            printk("%d,", FGvbatVoltageBuffer[i]);            
		        }
				printk("\r\n");
			}
		}
		else
		{
			if (Enable_FGADC_LOG == 1){
				printk("[FG] Over MinErrorOffset:V=%d,Avg_V=%d, ", gFG_voltage, gFG_voltage_AVG);
			}
			
			gFG_voltage = gFG_voltage_AVG;
			
			if (Enable_FGADC_LOG == 1){
				printk("Avg_V need write back to V : V=%d,Avg_V=%d.\r\n", gFG_voltage, gFG_voltage_AVG);
			}
		}
	}
    /**************** Averaging : END ****************/
	gFG_voltageVBAT = gFG_voltage;

//2. Calculate battery capacity by VBAT	
	gFG_capacity_by_v = fgauge_read_capacity_by_v();

//3. Calculate battery capacity by Coulomb Counter
	gFG_capacity_by_c = fgauge_read_capacity(1);
	gEstBatCapacity = gFG_capacity_by_c;	

//4. update DOD0 after booting Xs
	if(gFG_booting_counter_I_FLAG == 1)
	{
		if(gFG_capacity_by_v <= 1)
		{
			printk("[FGADC] gFG_capacity_by_v (%d) <= 1, run FG_booting_protection\r\n", gFG_capacity_by_v);
			FG_booting_protection();
		}
	
		gFG_booting_counter_I_FLAG = 2;
		
		gFG_capacity = gFG_capacity_by_v;
		
	    gFG_capacity_by_c_init = gFG_capacity;
	    gFG_capacity_by_c = gFG_capacity;
	    gFG_pre_temp = gFG_temp;
	    
	    gFG_DOD0 = 100 - gFG_capacity;
		gFG_DOD1=gFG_DOD0;

		i_kthread_index = 0;

		bat_volt_check_point = gFG_capacity;

		printk("[FGADC] update DOD0 after booting %d s\r\n", (MAX_BOOTING_TIME_FGCURRENT/6));

		gFG_15_vlot = fgauge_read_v_by_capacity(87); //13%
		printk("[FGADC] gFG_15_vlot = %dmV\r\n", gFG_15_vlot);
		if( (gFG_15_vlot > 3800) || (gFG_15_vlot < 3600) )
		{
			printk("[FGADC] gFG_15_vlot(%d) over range, reset to 3700\r\n", gFG_15_vlot);
			gFG_15_vlot = 3700;
		}

		printk("[FGADC] bat_volt_check_point=%d, gFG_DOD0=%d, gFG_capacity_by_v=%d\r\n",
			bat_volt_check_point, gFG_DOD0, gFG_capacity_by_v);

		gFG_current_auto_detect_R_fg_result = gFG_current_auto_detect_R_fg_total / gFG_current_auto_detect_R_fg_count;
		if(gFG_current_auto_detect_R_fg_result <= CURRENT_DETECT_R_FG)
		{
			gForceADCsolution=1;
			batteryBufferFirst = KAL_FALSE; // for init array values when measuring by AUXADC 
			
			printk("[FGADC] Detect NO Rfg, use AUXADC report. (%d=%d/%d)(%d)\r\n", 
				gFG_current_auto_detect_R_fg_result, gFG_current_auto_detect_R_fg_total,
				gFG_current_auto_detect_R_fg_count, gForceADCsolution);			
		}
		else
		{
			if(gForceADCsolution == 0)
			{
			gForceADCsolution=0;
		
			printk("[FGADC] Detect Rfg, use FG report. (%d=%d/%d)(%d)\r\n", 
				gFG_current_auto_detect_R_fg_result, gFG_current_auto_detect_R_fg_total,
				gFG_current_auto_detect_R_fg_count, gForceADCsolution);
		}
			else
			{
				printk("[FGADC] Detect Rfg, but use AUXADC report. due to gForceADCsolution=%d \r\n", 
					gForceADCsolution);
			}
		}
	}

}

kal_int32 fgauge_get_Q_max(kal_int16 temperature)
{
	kal_int32 ret_Q_max=0;
	kal_int32 low_temperature = 0, high_temperature = 0;
	kal_int32 low_Q_max = 0, high_Q_max = 0;

	if (temperature <= TEMPERATURE_T1)
    {
        low_temperature = (-10);
		low_Q_max = Q_MAX_NEG_10;
        high_temperature = TEMPERATURE_T1;
		high_Q_max = Q_MAX_POS_0;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
	else if (temperature <= TEMPERATURE_T2)
    {
        low_temperature = TEMPERATURE_T1;
		low_Q_max = Q_MAX_POS_0;
        high_temperature = TEMPERATURE_T2;
		high_Q_max = Q_MAX_POS_25;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
    else
    {
    	low_temperature  = TEMPERATURE_T2;
		low_Q_max = Q_MAX_POS_25;
        high_temperature = TEMPERATURE_T3;
		high_Q_max = Q_MAX_POS_50;
		
		if(temperature > high_temperature)
		{
			temperature = high_temperature;
		}
    }

	ret_Q_max = low_Q_max +
	(
		(
			(temperature - low_temperature) * 
			(high_Q_max - low_Q_max)
		) / 
		(high_temperature - low_temperature)				
	);

	if (Enable_FGADC_LOG == 1){
		printk("[fgauge_get_Q_max] Q_max = %d\r\n", ret_Q_max);
	}

	return ret_Q_max;
}

kal_int32 fgauge_get_Q_max_high_current(kal_int16 temperature)
{
	kal_int32 ret_Q_max=0;
	kal_int32 low_temperature = 0, high_temperature = 0;
	kal_int32 low_Q_max = 0, high_Q_max = 0;

	if (temperature <= TEMPERATURE_T1)
    {
        low_temperature = (-10);
		low_Q_max = Q_MAX_NEG_10_H_CURRENT;
        high_temperature = TEMPERATURE_T1;
		high_Q_max = Q_MAX_POS_0_H_CURRENT;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
	else if (temperature <= TEMPERATURE_T2)
    {
        low_temperature = TEMPERATURE_T1;
		low_Q_max = Q_MAX_POS_0_H_CURRENT;
        high_temperature = TEMPERATURE_T2;
		high_Q_max = Q_MAX_POS_25_H_CURRENT;
		
		if(temperature < low_temperature)
		{
			temperature = low_temperature;
		}
    }
    else
    {
    	low_temperature  = TEMPERATURE_T2;
		low_Q_max = Q_MAX_POS_25_H_CURRENT;
        high_temperature = TEMPERATURE_T3;
		high_Q_max = Q_MAX_POS_50_H_CURRENT;
		
		if(temperature > high_temperature)
		{
			temperature = high_temperature;
		}
    }

	ret_Q_max = low_Q_max +
	(
		(
			(temperature - low_temperature) * 
			(high_Q_max - low_Q_max)
		) / 
		(high_temperature - low_temperature)				
	);

	if (Enable_FGADC_LOG == 1){
		printk("[fgauge_get_Q_max_high_current] Q_max = %d\r\n", ret_Q_max);
	}

	return ret_Q_max;
}


void fgauge_initialization(void)
{
	kal_uint16 Temp_Reg = 0;
	int i = 0;

// 1. HW initialization    

#if (ENABLE_SW_COULOMB_COUNTER == 1)
// 1.0 set VCORE=1.35V on E1
	printk("******** [fgauge_initialization] Set VCORE=1.375V for enable FG analog circuit !!\n" );
	upmu_buck_normal_voltage_adjust(BUCK_VCORE, UPMU_VOLT_1_3_7_5_V);
	upmu_buck_rs(BUCK_VCORE,BUCK_REMOTE_SENSE);
#endif

// 1.1. Power on FGPLL
	MTKFG_PLL_Control(KAL_TRUE);
	
// 1.2. Initial setting
    //fg_dig_test(1);	
    //fg_interrupt_enable(KAL_TRUE);
	//fg_set_calibration_type(AUTO_CALIBRATION);
	//fg_set_hw_active(FG_ACTIVE);
	//Temp_Reg = INREG16(FGADC_CON7) | 0x0010;
    //OUTREG16(FGADC_CON7, Temp_Reg);    

	if(get_chip_eco_ver()==CHIP_E1)
	{
		fg_set_over_sampling_rate(0x7); //V4
	}
	else
	{
		#if (OSR_SELECT_7 == 1)
			fg_set_over_sampling_rate(0x7); // and CAR need * 8			
		#else
			fg_set_over_sampling_rate(0x0); //E2
		#endif
	}

    //Temp_Reg = 0x1000;
    Temp_Reg = 0x1888;
    OUTREG16(FGADC_CON0, Temp_Reg);
	
	mdelay(100);

// 2. SW algorithm initialization
    gFG_voltage = fgauge_read_voltage();        
	
    //gFG_current = fgauge_read_current();
	while( gFG_current == 0 )
	{
		gFG_current = fgauge_read_current();
		if(i > 10)
			break;
		i++;
	}

	if(get_chip_eco_ver()==CHIP_E1)
	{
		gFG_columb = FGADC_E1_SW_Simulate_Coulomb_Counter(); // SW workaround on E1
	}
	else
	{
		gFG_columb = fgauge_read_columb();
	}
	
    gFG_capacity = fgauge_read_capacity(0);
    gFG_temp = fgauge_read_temperature(); 	

    gFG_columb_init = gFG_columb;
    gFG_capacity_by_c_init = gFG_capacity;
    gFG_capacity_by_c = gFG_capacity;
    gFG_capacity_by_v = gFG_capacity;
    gFG_pre_temp = gFG_temp;

    gFG_inner_R = (COMPASATE_OCV/1000) / gFG_current; // mOhm
    gFG_DOD0 = 100 - gFG_capacity;

	//V4 interpolation Q_max
	gFG_BATT_CAPACITY = fgauge_get_Q_max(gFG_temp);

	FGADC_dump_parameter();
	
	printk("******** [fgauge_initialization] Done!\n" );

}

void fgauge_main(void)
{
    // 1. Get Row Data ( CAR / VBAT / I / T ) 
    gFG_columb = fgauge_read_columb();          
    gFG_voltage = fgauge_read_voltage();         
    gFG_current = fgauge_read_current();         
    gFG_temp = fgauge_read_temperature();          
	
    // 2. Check T changed 
    if ( ((gFG_temp-gFG_pre_temp)>gFG_T_changed) || ((gFG_pre_temp-gFG_temp)>gFG_T_changed) )
    {
        // 2.1. Yes : 
        // 2.1.1. Re-Construct Battery Profile Table
        gFG_capacity_by_v = fgauge_read_capacity(0);
        gFG_capacity = gFG_capacity_by_v;	
		
        // 2.1.2. Update DOD0 & Reset HW FG
        gFG_DOD0 = 100 - gFG_capacity;
        fg_sw_control_fg_behavior(BEHAVIOR_RESET_TO_ZERO);
        gFG_columb_init = 0;		
        
        // 2.1.3. Return the ( 100 - DOD0 ) as the Battery Percentage
        printk("[FGADC] fgauge_main - gFG_capacity : %d \r\n", (kal_int32)gFG_capacity);
        
    }
    else
    {
        // 2.2. No : 
        // 2.2.1. Calculate the Battery Percentage ( 100 ¡V DOD1 ) and return this value
        gFG_columb -= gFG_columb_init; // - the init value (this value is exist before SW start)
        gFG_capacity_by_c = fgauge_read_capacity(1); 
        gFG_capacity = gFG_capacity_by_c;		
		
        printk("[FGADC] fgauge_main - gFG_capacity : %d\r\n", (kal_int32)gFG_capacity);
    }

    gFG_pre_temp = gFG_temp;		
    
}

///////////////////////////////////////////////////////////////////////////////////////////
//// External API 
///////////////////////////////////////////////////////////////////////////////////////////
kal_int32 FGADC_Get_BatteryCapacity_CoulombMothod(void)
{
	return gFG_capacity_by_c;
}

kal_int32 FGADC_Get_BatteryCapacity_VoltageMothod(void)
{
	return gFG_capacity_by_v;
}

kal_int32 FGADC_Get_FG_Voltage(void)
{
	return gFG_voltageVBAT;
}

extern int g_Calibration_FG;

void FGADC_Reset_SW_Parameter(void)
{	
	volatile kal_uint16 Temp_Reg = 0;
	volatile kal_uint16 val_car = 1;	
	
	if(get_chip_eco_ver()==CHIP_E1)
	{
#if (ENABLE_SW_COULOMB_COUNTER == 1)	
		//printk("[FGADC] FGADC_Reset_SW_Parameter \r\n");
		gFG_columb = 0;
		gFG_SW_CoulombCounter = 0;
		printk("[FGADC] Update DOD0(%d) by %d \r\n", gFG_DOD0, gFG_DOD1);
		gFG_DOD0 = gFG_DOD1;
#endif	
	}
	else
	{
		printk("[FGADC] FGADC_Reset_SW_Parameter : Start !! \r\n");
		//gFG_columb = 0;
		gFG_SW_CoulombCounter = 0;	
		while(val_car!=0x0)
		{			
			Temp_Reg = 0x18F0;
			OUTREG16(FGADC_CON0, Temp_Reg);
			mdelay(1);			
			gFG_columb = fgauge_read_columb_reset();
			val_car = gFG_columb;
			
			if (Enable_FGADC_LOG == 1){
				printk("[FGADC] CON0=0x%x,CON1=0x%x\n",FG_DRV_ReadReg16(FGADC_CON0),val_car);
			}		
		}
		gFG_columb = 0;
		printk("[FGADC] FGADC_Reset_SW_Parameter : Done \r\n");

		if(g_Calibration_FG==1)
		{						
			gFG_DOD0 = 0;
			printk("[FGADC] FG Calibration DOD0=%d and DOD1=%d \r\n", gFG_DOD0, gFG_DOD1);
		}
		else
		{
			printk("[FGADC] Update DOD0(%d) by %d \r\n", gFG_DOD0, gFG_DOD1);
			gFG_DOD0 = gFG_DOD1;		
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//// Internal API 
///////////////////////////////////////////////////////////////////////////////////////////
static int g_FGADC_thread_timeout = 0;

static DEFINE_MUTEX(FGADC_mutex);
static DECLARE_WAIT_QUEUE_HEAD(FGADC_thread_wq);

void FGADC_dump_register(void)
{
	unsigned short regVal=0;	

	regVal = INREG16(PLL_CON6_BASE_ADDR);
	printk("[FGADC] PLL_CON6_BASE_ADDR=%x\r\n", regVal);

	regVal = INREG16(FG_PLL_CON0_BASE_ADDR);
	printk("[FGADC] FG_PLL_CON0_BASE_ADDR=%x\r\n", regVal);

	regVal = INREG16(0xF702F900);
	printk("[FGADC] VCORE_CON_0=%x\r\n", regVal);
	
	regVal = INREG16(FGADC_CON0);
	printk("[FGADC] FGADC_CON0=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON1);
	printk("[FGADC] FGADC_CON1=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON2);
	printk("[FGADC] FGADC_CON2=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON3);
	printk("[FGADC] FGADC_CON3=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON4);
	printk("[FGADC] FGADC_CON4=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON5);
	printk("[FGADC] FGADC_CON5=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON6);
	printk("[FGADC] FGADC_CON6=%x\r\n", regVal);

	regVal = INREG16(FGADC_CON7);
	printk("[FGADC] FGADC_CON7=%x\r\n", regVal);
	
}

void FGADC_dump_register_csv(void)
{
	//unsigned short regVal=0;	

	printk("[FGADC_REG] %x,%x,%x,%x,%x,%x,%x,%x\r\n", INREG16(FGADC_CON0), INREG16(FGADC_CON1), INREG16(FGADC_CON2),
		INREG16(FGADC_CON3), INREG16(FGADC_CON4), INREG16(FGADC_CON5), INREG16(FGADC_CON6), INREG16(FGADC_CON7));
	
}

kal_int32 FGADC_E1_SW_Simulate_Coulomb_Counter(void)
{
	kal_int32 dvalue_CAR = 0;

	gFG_current = fgauge_read_current();

	if(gFG_Is_Charging == KAL_TRUE)
	{
		//gFG_SW_CoulombCounter = gFG_SW_CoulombCounter + (gFG_current * 1); // 1000ms
		//gFG_SW_CoulombCounter = gFG_SW_CoulombCounter + ((gFG_current*167)/1000); // 166ms
		gFG_SW_CoulombCounter = gFG_SW_CoulombCounter + (((gFG_current*167)+(1000/2))/1000); // 166ms		
	}
	else 
	{	
		/* HW SA : performance tuning */
		gFG_current = gFG_current - FG_CURRENT_OFFSET_DISCHARGING;
	
		//gFG_SW_CoulombCounter = gFG_SW_CoulombCounter - (gFG_current * 1); // 1000ms
		//gFG_SW_CoulombCounter = gFG_SW_CoulombCounter - ((gFG_current*167)/1000); // 166ms
		gFG_SW_CoulombCounter = gFG_SW_CoulombCounter - (((gFG_current*167)+(1000/2))/1000); // 166ms		
	}
	//dvalue_CAR = gFG_SW_CoulombCounter / 3600; //mAh
	dvalue_CAR = (gFG_SW_CoulombCounter / 3600) / 10;
	
	//printk("[FGADC] E1_SW_Simulate_Coulomb_Counter = %d mAs\n", gFG_SW_CoulombCounter);
	//printk("[FGADC] E1_SW_Simulate_Coulomb_Counter = %d mAh\n", dvalue_CAR);
	
	return dvalue_CAR;
}

void BatChargerOutDetect_SWworkaround(void)
{
	int charger_voltage=0;
	int R_CHARGER_1=270;
	int R_CHARGER_2=39;		
	
	if( g_charger_in_flag == 1)
	{
		upmu_adc_measure_vchr_enable(CHR, KAL_TRUE);
		charger_voltage = IMM_GetOneChannelValue(AUXADC_CHARGER_VOLTAGE_CHANNEL,5) * (((R_CHARGER_1+R_CHARGER_2)*100)/R_CHARGER_2);
		charger_voltage = charger_voltage / 100;
		//printk("%d\r\n",charger_voltage);
		
		if(charger_voltage < MAX_V_CHARGER)
		{
			pchr_turn_off_charging();
			printk("[BatChargerOutDetect_SWworkaround] %d < %d\r\n", charger_voltage, MAX_V_CHARGER);
		}
	}
	
}

int Get_I_Charging(void)
{
	int ADC_I_SENSE=0;
	int ADC_BAT_SENSE=0;
	int ICharging=0;
	int R_BAT_SENSE=2;
	int R_I_SENSE=2;
	int R_CURRENT_SENSE=2;
	
	upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
	upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
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

void BatChargerOutDetect_SWworkaround_v2(void)
{
	int ret_check_I_charging=0;
	int j=0;	
	int sw_chr_out_flag=0;
	int repeat_times = 10;
	//int repeat_times = 1;

	ret_check_I_charging = Get_I_Charging();	
	
	if( g_charger_in_flag == 1)
	{
		//printk("[BATTERY] check %d mA !\n", ret_check_I_charging);
	
		if( (ret_check_I_charging < CHR_OUT_CURRENT) && (g_HW_stop_charging==0) 		
			&& (g_SW_CHR_OUT_EN==1) && (g_HW_Charging_Done==0) )	
		{
			sw_chr_out_flag = 1;
			
			for(j=0 ; j<repeat_times ; j++)
			{
				ret_check_I_charging = Get_I_Charging();			
				//printk("[BATTERY] double check %d mA !\n", ret_check_I_charging);
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
					pchr_turn_off_charging();
					printk("[BatChargerOutDetect_SWworkaround_v2] %d < %d\r\n", ret_check_I_charging, CHR_OUT_CURRENT);
					printk("[BatChargerOutDetect_SWworkaround_v2] %d,%d,%d\r\n", g_HW_stop_charging, g_SW_CHR_OUT_EN, g_HW_Charging_Done);
				}
				else
				{
					printk("[BatChargerOutDetect_SWworkaround_v2] ignore\r\n");
					printk("[BatChargerOutDetect_SWworkaround_v2] %d,%d,%d\r\n", g_HW_stop_charging, g_SW_CHR_OUT_EN, g_HW_Charging_Done);
				}
			}
		}		
	}	
}

#if defined(CONFIG_POWER_EXT)
#else
static int FGADC_thread_kthread(void *x)
{
    /* Run on a process content */  
    while (1) {           
	
        mutex_lock(&FGADC_mutex);

#if (ENABLE_SW_COULOMB_COUNTER == 1)
		//printk("[FGADC]ENABLE_SW_COULOMB_COUNTER = 1\r\n");
		fgauge_Normal_Mode_Work();		
#else
		//printk("[FGADC]ENABLE_SW_COULOMB_COUNTER = 0\r\n");		
#endif

		//FGADC_dump_parameter();
		//FGADC_dump_register();	
		
		if(i_kthread_index == 0) 
		{
			if(get_chip_eco_ver()==CHIP_E1)
			{
				//Do nothing
			}
			else
			{
				fgauge_Normal_Mode_Work();
			}
		
			if (Enable_FGADC_LOG == 1) 
			{
				//printk("gFG_Is_Charging,gFG_current,gFG_SW_CoulombCounter,gFG_columb,gFG_voltage,
				//gFG_capacity_by_v,gFG_capacity_by_c,gFG_capacity_by_c_init,Qmax,Vcompensate\r\n");
				#if 1				
				printk("[FGADC] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",gFG_Is_Charging,gFG_current,
				    gFG_SW_CoulombCounter,gFG_columb,gFG_voltage,gFG_capacity_by_v,gFG_capacity_by_c,gFG_capacity_by_c_init, 
					gFG_BATT_CAPACITY,gFG_compensate_value,gFG_ori_voltage,OCV_BOARD_COMPESATE,R_FG_BOARD_SLOPE,
					ENABLE_SW_COULOMB_COUNTER,gFG_voltage_init,MinErrorOffset,gFG_DOD0,gFG_DOD1,current_get_ori);
				FGADC_dump_register_csv();
				#endif												
			}

			if(get_chip_eco_ver()==CHIP_E1)
			{
				i_kthread_index=6; //1s
			}
			else
			{
				i_kthread_index=30; //5s				
			}	
    	}
		i_kthread_index--;

		mutex_unlock(&FGADC_mutex);
			
		wait_event(FGADC_thread_wq, g_FGADC_thread_timeout);		
		
        g_FGADC_thread_timeout=0;

		if(gFG_booting_counter_I > MAX_BOOTING_TIME_FGCURRENT)
		{
			gFG_booting_counter_I = 0;
			gFG_booting_counter_I_FLAG = 1;
		}
		else 
		{
			if(gFG_booting_counter_I_FLAG == 0)
			{
				gFG_booting_counter_I++;

				if(get_chip_eco_ver()!=CHIP_E1)
				{
					gFG_current_auto_detect_R_fg_total+= fgauge_read_current();
					gFG_current_auto_detect_R_fg_count++;
				}
			}
		}

		if(get_chip_eco_ver()==CHIP_E1)
		{
			//charger plug-out detection workaround	
		
			if(vchr_kthread_index == 0)
			{				
				//BatChargerOutDetect_SWworkaround();
				BatChargerOutDetect_SWworkaround_v2();
				vchr_kthread_index=3;
			}
			vchr_kthread_index--;
			}
		else
		{
			//charger plug-out detection by VCDT detection when CHR on
		}

    }

    return 0;
}
#endif

void FGADC_thread_wakeup(UINT16 i)
{
    g_FGADC_thread_timeout=1;

	//printk("******** MT6573 FGADC : FGADC_thread_wakeup ********\n" );
	
    wake_up(&FGADC_thread_wq);
}

void FGadcThread_XGPTConfig(void)
{
    GPT_CONFIG config;
    GPT_NUM  gpt_num = GPT2;    
    GPT_CLK_DIV clkDiv = GPT_CLK_DIV_128;

	printk("******** MT6573 FGADC : FGadcThread_XGPTConfig : use GPT2 ********\n" );

    GPT_Init (gpt_num, FGADC_thread_wakeup);
    config.num = gpt_num;
    config.mode = GPT_REPEAT;
    config.clkDiv = clkDiv;
    //config.u4Timeout = 1*128;
    config.u4Timeout = 21;
    
    if (GPT_Config(config) == FALSE )
        return;                       
        
    GPT_Start(gpt_num);  

    return ;
}

///////////////////////////////////////////////////////////////////////////////////////////
//// Logging System
///////////////////////////////////////////////////////////////////////////////////////////
static struct proc_dir_entry *proc_entry_fgadc;
static char proc_fgadc_data[32];  

ssize_t fgadc_log_write( struct file *filp, const char __user *buff,
                        unsigned long len, void *data )
{
	if (copy_from_user( &proc_fgadc_data, buff, len )) {
		printk("fgadc_log_write error.\n");
		return -EFAULT;
	}

	if (proc_fgadc_data[0] == '1') {
		printk("enable FGADC driver log system\n");
		Enable_FGADC_LOG = 1;
	} else {
		printk("Disable FGADC driver log system\n");
		Enable_FGADC_LOG = 0;
	}
	
	return len;
}

int init_proc_log_fg(void)
{
	int ret=0;
	proc_entry_fgadc = create_proc_entry( "fgadc_log", 0644, NULL );
	
	if (proc_entry_fgadc == NULL) {
		ret = -ENOMEM;
	  	printk("init_proc_log_fg: Couldn't create proc entry\n");
	} else {
		proc_entry_fgadc->write_proc = fgadc_log_write;
		//proc_entry->owner = THIS_MODULE;
		printk("init_proc_log_fg loaded.\n");
	}
  
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
//// Create File For Power Consumption Profile : FG_Current
///////////////////////////////////////////////////////////////////////////////////////////
static ssize_t show_FG_Current(struct device *dev,struct device_attribute *attr, char *buf)
{
	// Power Consumption Profile---------------------
	gFG_current = fgauge_read_current();
	if(gFG_Is_Charging==KAL_TRUE)
	{
		gFG_current_inout_battery = 0 - gFG_current;
	}
	else
	{
		gFG_current_inout_battery = gFG_current;
	}
	printk("[FG] %d\r\n", gFG_current_inout_battery);
	//-----------------------------------------------
	printk("[FG] gFG_current_inout_battery : %d\n", gFG_current_inout_battery);
	return sprintf(buf, "%d\n", gFG_current_inout_battery);
}
static ssize_t store_FG_Current(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}
static DEVICE_ATTR(FG_Current, 0664, show_FG_Current, store_FG_Current);

///////////////////////////////////////////////////////////////////////////////////////////
//// platform_driver API 
///////////////////////////////////////////////////////////////////////////////////////////
static int mt6573_fgadc_probe(struct platform_device *dev)	
{
	int ret_device_file = 0;

#if defined(CONFIG_POWER_EXT)
//#if 0
	printk("[FGADC] INIT : EVB \n");
#else
	printk("[FGADC] MT6573 FGADC driver probe!! \n" );

	/* FG driver init */
	fgauge_initialization();

	/* Run FGADC Thread Use GPT timer */ 
    FGadcThread_XGPTConfig();

    /* FGADC kernel thread for 1s check gas gauge status */
    kthread_run(FGADC_thread_kthread, NULL, "FGADC_thread_kthread");

	/*LOG System Set*/
	init_proc_log_fg();
#endif

	ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Current);

	return 0;
}

static int mt6573_fgadc_remove(struct platform_device *dev)	
{
	printk("[FGADC] MT6573 FGADC driver remove!! \n" );

	return 0;
}

static void mt6573_fgadc_shutdown(struct platform_device *dev)	
{
	printk("[FGADC] MT6573 FGADC driver shutdown!! \n" );		
}

kal_uint32 gRTC_time_suspend=0;
kal_uint32 gRTC_time_resume=0;
kal_uint32 gFG_capacity_before=0;
kal_uint32 gFG_capacity_after=0;
kal_uint32 gFG_RTC_time_MAX=3600; //60mins
//kal_uint32 gFG_RTC_time_MAX=60; //1mins

static int mt6573_fgadc_suspend(struct platform_device *dev, pm_message_t state)	
{

#if defined(CONFIG_POWER_EXT)
//#if 0
	printk("[FGADC_suspend] EVB !!\n");

#else

	#if 1
	printk("[FGADC_suspend] Disable sleep mode !!\n");
	#else 
	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	struct rtc_time tm;
	unsigned long time;

	if(get_chip_eco_ver()!=CHIP_E1)
	{
		printk("[FGADC_suspend] MT6573 FGADC driver suspend!!\n");

		FGADC_Reset_SW_Parameter();
		//Turn Off FG
		MTKFG_PLL_Control(KAL_FALSE);		

		rtc_read_time(rtc, &tm);
		rtc_tm_to_time(&tm, &time);
		gRTC_time_suspend=time;		
		gFG_capacity_before=gFG_capacity_by_c;
		
		printk("[FGADC_suspend] gRTC_time_suspend=%d, gFG_capacity_before=%d\n", 
			gRTC_time_suspend, gFG_capacity_before);		
	}
	#endif
	
#endif

	return 0;
}

static int mt6573_fgadc_resume(struct platform_device *dev)
{
#if defined(CONFIG_POWER_EXT)
//#if 0
	printk("[FGADC_RESUME] EVB !!\n");

#else

	#if 1
	printk("[mt6573_fgadc_resume] Disable sleep mode !!\n");
	#else 
	kal_uint16 Temp_Reg = 0;
	int i=0;
	int index=1;
	kal_int32 FG_voltage_sum=0;

	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	struct rtc_time tm;
	unsigned long time;
	
	//unsigned long RTC_BatteryPercent=0;
	kal_int32 temp_RTC=0;
	kal_int32 temp_FG=0;
	//kal_int32 temp_offset=0;
	kal_uint32 gRTC_time_offset=0;	
	kal_int32 gFG_percent_resume=0;	

	kal_uint16 val_car=0;

	if(get_chip_eco_ver()!=CHIP_E1)
	{
		printk("[FGADC_RESUME] MT6573 FGADC driver resume, gFG_DOD0=%d!!\n", gFG_DOD0);
			
		//Turn On FG 
		MTKFG_PLL_Control(KAL_TRUE);

		rtc_read_time(rtc, &tm);
		rtc_tm_to_time(&tm, &time);
		gRTC_time_resume=time;
	
		printk("[FGADC_RESUME] FGADC_Reset some SW_Parameter \r\n");
		gFG_columb = 0;
		gFG_SW_CoulombCounter = 0;
		Temp_Reg = 0x18F0;
    	OUTREG16(FGADC_CON0, Temp_Reg);
		val_car = FG_DRV_ReadReg16(FGADC_CON1);
		#if 0
		val_car = FG_DRV_ReadReg16(FGADC_CON1);
		while(val_car!=0x0)
		{			
			OUTREG16(FGADC_CON0, Temp_Reg);
			printk(".");
			val_car = FG_DRV_ReadReg16(FGADC_CON1);
		}
		#endif
		
		gFGvbatBufferFirst = KAL_FALSE;

		gRTC_time_offset=(gRTC_time_resume-gRTC_time_suspend);

		if(gRTC_time_offset > gFG_RTC_time_MAX)
		{	
			// Use AUXADC to update DOD		
			//gFG_voltage = fgauge_read_voltage();
			for(i=0 ; i<index ; i++)
			{
				FG_voltage_sum += fgauge_read_voltage();
			}
			gFG_voltage = (FG_voltage_sum/index);		
			
			gFG_current = fgauge_read_current();		 	
			gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,5); //mV
			gFG_voltage = gFG_voltage + OCV_BOARD_COMPESATE;			
			gFG_capacity_by_v = fgauge_read_capacity_by_v();				
			gFG_capacity_by_c=gFG_capacity_by_v;
			gEstBatCapacity = gFG_capacity_by_c;
			gFG_DOD0 = 100 - gFG_capacity_by_v;		

			printk("[FGADC_RESUME] %d,%d,%d,%d\n", gFG_current, gFG_voltage, gFG_capacity_by_v, bat_volt_check_point);			

			gFG_percent_resume=100-gFG_DOD0;			
			if(gFG_percent_resume > bat_volt_check_point)
			{
				//restore
				gFG_capacity_by_v = bat_volt_check_point;
				gFG_capacity_by_c=gFG_capacity_by_v;
				gEstBatCapacity = gFG_capacity_by_c;
				gFG_DOD0 = 100 - gFG_capacity_by_v;
				gFG_DOD1 = gFG_DOD0;
			}

			printk("[FGADC_RESUME] Sleep Time(%d) > %d, gFG_capacity_by_v=%d, bat_volt_check_point=%d, gFG_DOD0=%d, gFG_DOD1=%d, gFG_percent_resume=%d\n", 
				gRTC_time_offset, gFG_RTC_time_MAX, gFG_capacity_by_v, bat_volt_check_point, gFG_DOD0, gFG_DOD1, gFG_percent_resume);
		}
		else
		{
			printk("[FGADC_RESUME] Need update DOD0(%d) by bat_volt_check_point(%d)\n", 
				gFG_DOD0, bat_volt_check_point);
			//restore
			gFG_capacity_by_v = bat_volt_check_point;			
			gFG_capacity_by_c=gFG_capacity_by_v;
			gEstBatCapacity = gFG_capacity_by_c;
			gFG_DOD0 = 100 - gFG_capacity_by_v;
			gFG_DOD1 = gFG_DOD0;

			printk("[FGADC_RESUME] Sleep Time(%d) <= %d, gFG_capacity_by_v=%d, bat_volt_check_point=%d, gFG_DOD0=%d, gFG_DOD1=%d\n", 
				gRTC_time_offset, gFG_RTC_time_MAX, gFG_capacity_by_v, bat_volt_check_point, gFG_DOD0, gFG_DOD1);
		}

		printk("[FGADC_RESUME] gRTC_time_suspend=%d, gRTC_time_resume=%d, gFG_capacity_before=%d, gFG_capacity_after=%d, temp_RTC=%d, temp_FG=%d\n", 
			gRTC_time_suspend, gRTC_time_resume, gFG_capacity_before, gFG_capacity_after, temp_RTC, temp_FG);
		
		printk("[FGADC_RESUME] %d,%d,%d,%d,%d,gFG_temp=%d,gFG_DOD1_return=%d,val_car=%d\n", 
			gFG_current, gFG_voltage, gFG_capacity_by_v, FG_voltage_sum, index, gFG_temp, gFG_DOD1_return,val_car);
	}
	#endif
	
#endif

	return 0;
}

struct platform_device MT6573_fgadc_device = {
		.name				= "mt6573-fgadc",
		.id					= -1,
};

static struct platform_driver mt6573_fgadc_driver = {
	.probe		= mt6573_fgadc_probe,
	.remove		= mt6573_fgadc_remove,
	.shutdown	= mt6573_fgadc_shutdown,
	//#ifdef CONFIG_PM
	.suspend	= mt6573_fgadc_suspend,
	.resume		= mt6573_fgadc_resume,
	//#endif
	.driver         = {
        .name = "mt6573-fgadc",
    },
};

static int __init mt6573_fgadc_init(void)
{
	int ret;
	
	ret = platform_device_register(&MT6573_fgadc_device);
	if (ret) {
		printk("****[mt6573_fgadc_driver] Unable to device register(%d)\n", ret);
		return ret;
	}
	
	ret = platform_driver_register(&mt6573_fgadc_driver);
	if (ret) {
		printk("****[mt6573_fgadc_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}

	printk("****[mt6573_fgadc_driver] Initialization : DONE \n");

	return 0;

}

static void __exit mt6573_fgadc_exit (void)
{
}

module_init(mt6573_fgadc_init);
module_exit(mt6573_fgadc_exit);

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("MT6573 FGADC Device Driver");
MODULE_LICENSE("GPL");

