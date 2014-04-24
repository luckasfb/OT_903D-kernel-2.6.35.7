


#ifndef __FUEL_GAUGE_6573_SW_H__
#define __FUEL_GAUGE_6573_SW_H__

// =====================================================================================
// Enum

typedef enum
{
	NO_ACTION_ON_RESET = 0,
	CLEAR_RESET_DETECTION
}fg_reset_sw_control_enum;

typedef enum
{
	NO_ACTION_ON_BIT = 0,
	CLEAR_BIT
}fg_sw_clear_bit_enum;

typedef enum
{
	CMD_NO_ACTION = 0,
	CMD_READ_FG_DATA
}fg_sw_read_command_enum;

typedef enum
{
	BEHAVIOR_CONTINUE = 0,
	BEHAVIOR_RESET_TO_ZERO
}fg_sw_control_fg_behavior_enum;

typedef enum
{
	TIME_CONTINUE = 0,
	TIME_RESET_TO_ZERO
}fg_sw_reset_fg_time_enum;

typedef enum
{
	CHARGE_CONTINUE = 0,
	CHARGE_RESET_TO_ZERO
}fg_sw_reset_fg_charge_enum;

typedef enum
{
	T_X_2048 = 0,
	T_X_0512 = 1,
	T_X_1025 = 2,
	T_X_4096 = 3
}fg_calibration_rate_enum;

typedef enum
{
	DISABLE_AUTO_CALIBRATION = 0,
	FORCE_AUTO_CALIBRATION = 1,
	AUTO_CALIBRATION = 2
}fg_calibration_type_enum;

typedef enum
{
	FG_IDLE = 0,
	FG_ACTIVE = 1
}fg_active_enum;

typedef enum
{
	HW_ERROR = 0,
	HIGHER_THAN_THD = 1,
	LOWER_THAN_THD = 2
}fg_interrupt_check_enum;

// =====================================================================================
// HW Common API

// FGADC CON 0
void fg_reset_sw_control(fg_reset_sw_control_enum val);
void fg_sw_clear(fg_sw_clear_bit_enum val);
kal_uint16 fg_get_data_ready_status(void);
void fg_sw_read_command(fg_sw_read_command_enum val);
void fg_sw_control_fg_behavior(fg_sw_control_fg_behavior_enum val);
void fg_sw_reset_fg_time(fg_sw_reset_fg_time_enum val);
void fg_sw_reset_fg_charge(fg_sw_reset_fg_charge_enum val);
void fg_interrupt_enable(kal_bool enable);
void fg_set_auto_calibration_rate(fg_calibration_rate_enum val);
void fg_set_calibration_type(fg_calibration_type_enum val);
void fg_set_hw_active(fg_active_enum val);

// FGADC CON 1
kal_uint16 fg_get_car(void);

// FGADC CON 2
kal_uint16 fg_get_nter(void);

// FGADC CON 3
void fg_set_battery_low_threshold(kal_uint16 val);

// FGADC CON 4
void fg_set_battery_full_threshold(kal_uint16 val);

// FGADC CON 5
kal_uint16 fg_get_current(void);

// FGADC CON 6
void fg_set_adjust_offset_value(kal_uint16 val);

// FGADC CON 7
fg_interrupt_check_enum fg_get_higher_or_lower_thd(void);
kal_uint16 fg_get_reset_detect_debug(void);
void fg_adc_auto_reset(kal_uint16 val);
void fg_dig_test(kal_uint16 val);
void fg_fir_2_bypass(kal_uint16 val);
void fg_fir_1_bypass(kal_uint16 val);
void fg_set_over_sampling_rate(kal_uint16 val);

#endif // #ifndef __FUEL_GAUGE_6573_SW_H__

