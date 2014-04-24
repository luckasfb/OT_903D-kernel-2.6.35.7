

#include <common.h>
#include <asm/io.h>

//#define CFG_POWER_CHARGING

#ifdef CFG_POWER_CHARGING
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mt65xx_leds.h> 
#include <asm/arch/mt6573_key.h>
#include <asm/arch/mt6573_rtc.h>
#include <asm/arch/mt65xx_disp_drv.h>
#include <asm/arch/mt65xx_logo.h>
#include <asm/arch/boot_mode.h>

#include <cust_battery.h>

#include "pmu6573_sw.h"
#include "upmu_sw.h"

#undef printf

#define GPT_TIMER

#define Enable_BATDRV_LOG   KAL_FALSE

typedef enum
{
    USB_SUSPEND = 0,
    USB_UNCONFIGURED,
    USB_CONFIGURED
} usb_state_enum;

extern kal_bool meta_mode_check(void);

extern void apost_hw_init(void);
extern void mt6573_sleep(UINT32 ms, kal_bool force);

extern CHARGER_TYPE mt_charger_type_detection(void);
extern CHARGER_TYPE hw_charger_type_detection(void);

extern int IMM_GetOneChannelValue(int dwChannel, int deCount);

#define BATTERY_LOWVOL_THRESOLD             3450
#define CHR_OUT_CURRENT                     70

//#define BATT_ID_CHECK_SUPPORT	1

#if BATT_ID_CHECK_SUPPORT
	#define AUXADC_BATT_ID_CHANNEL 4
	#define CHG_MIN_VALID_BATT_ID  600     /*0.6V*/
	#define CHG_MAX_VALID_BATT_ID  900    /*0.9V*/
#endif
//#define MAX_CHARGING_TIME                 8*60*60     // 8hr
#define MAX_CHARGING_TIME                   12*60*60    // 12hr
#define MAX_POSTFULL_SAFETY_TIME            1*30*60     // 30mins
#define MAX_PreCC_CHARGING_TIME             1*30*60     // 0.5hr
#define MAX_CV_CHARGING_TIME                3*60*60     // 3hr
#define BAT_TASK_PERIOD                     1           // 1sec
#define BL_SWITCH_TIMEOUT                   5*10         // 6s  
#define POWER_ON_TIME                       2*1         // 0.5s

#define charger_OVER_VOL                    1
#define ADC_SAMPLE_TIMES                    5

#define  CHR_PRE                            0x1000
#define  CHR_CC                             0x1001 
#define  CHR_TOP_OFF                        0x1002 
#define  CHR_POST_FULL                      0x1003
#define  CHR_BATFULL                        0x1004 
#define  CHR_ERROR                          0x1005

typedef struct 
{
    kal_bool   	bat_exist;
#if BATT_ID_CHECK_SUPPORT
    kal_bool    bat_id_valid;   /*compatible with IEEE1725*/
#endif
    kal_bool   	bat_full;  
    kal_bool   	bat_low;  
    UINT32      bat_charging_state;
    UINT32      bat_vol;            
    kal_bool    charger_exist;   
    UINT32      pre_charging_current;
    UINT32      charging_current;
    UINT32      charger_vol;        
    UINT32      charger_protect_status; 
    UINT32      ISENSE;                
    UINT32      ICharging;
    INT32       temperature;
    UINT32      total_charging_time;
    UINT32      PRE_charging_time;
    UINT32      CC_charging_time;
    UINT32      TOPOFF_charging_time;
    UINT32      POSTFULL_charging_time;
    UINT32      charger_type;
    UINT32      PWR_SRC;
    UINT32      SOC;
    UINT32      ADC_BAT_SENSE;
    UINT32      ADC_I_SENSE;
} PMU_ChargerStruct;

typedef enum 
{
    PMU_STATUS_OK = 0,
    PMU_STATUS_FAIL = 1,
} PMU_STATUS;

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

int g_charger_is_turn_on = 0;

int g_thread_count = 10;
int g_first_top_off = 0;

// HW CV algorithm
unsigned int CHR_CON_0 = 0x7002FA00;
unsigned int CHR_CON_1 = 0x7002FA04;
unsigned int CHR_CON_2 = 0x7002FA08;
unsigned int CHR_CON_4 = 0x7002FA10;
unsigned int CHR_CON_6 = 0x7002FA18;
unsigned int CHR_CON_9 = 0x7002FA24;
unsigned int CHR_CON_10 = 0x7002FA28;
unsigned int PMIC_RESERVE_CON1 = 0x7002FE84;
volatile unsigned int save_value = 0x0;
volatile unsigned int CSDAC_DAT_MAX = 255;
volatile unsigned int CSDAC_DAT = 0;
volatile unsigned int VBAT_CV_DET = 0x0;
volatile unsigned int CS_DET = 0x0;
int g_sw_cv_enable = 0;

#if defined(CONFIG_POWER_EXT)
int CHARGING_FULL_CURRENT = 300;    // mA on EVB
#else
int CHARGING_FULL_CURRENT = 120;    // mA on phone
#endif

int gADC_BAT_SENSE_temp = 0;
int gADC_I_SENSE_temp = 0;
int gADC_I_SENSE_offset = 0;

int g_BatteryAverageCurrent = 0;

int g_usb_state = USB_UNCONFIGURED;
int g_temp_CC_value = Cust_CC_0MA;

int g_chr_event = 0;
int g_bat_volt_cp_flag = 0;

int g_prog = 25;
int g_prog_temp = 0;
int g_prog_first = 1;
int g_bl_switch_timer = 0;
int g_bat_volt_check_point = 0;
int g_low_bat_boot_display = 0;

kal_bool g_bl_on = KAL_TRUE;
kal_bool g_bl_switch = KAL_FALSE;
kal_bool g_user_view_flag = KAL_FALSE;

typedef struct{
    INT32 BatteryTemp;
    INT32 TemperatureR;
} BATT_TEMPERATURE;

/* convert register to temperature  */
INT32 BattThermistorConverTemp(INT32 Res)
{
    int i = 0;
    INT32 RES1 = 0, RES2 = 0;
    INT32 TBatt_Value = -200, TMP1 = 0, TMP2 = 0;

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

    if (Enable_BATDRV_LOG == 1) {
        printf("###### %d <-> %d ######\r\n", Batt_Temperature_Table[9].BatteryTemp, 
            Batt_Temperature_Table[9].TemperatureR);
    }

    if(Res >= Batt_Temperature_Table[0].TemperatureR)
    {
        #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
        printf("Res >= %d\n", Batt_Temperature_Table[0].TemperatureR);
        #endif
        TBatt_Value = -20;
    }
    else if(Res <= Batt_Temperature_Table[16].TemperatureR)
    {
        #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
        printf("Res <= %d\n", Batt_Temperature_Table[16].TemperatureR);
        #endif
        TBatt_Value = 60;
    }
    else
    {
        RES1 = Batt_Temperature_Table[0].TemperatureR;
        TMP1 = Batt_Temperature_Table[0].BatteryTemp;
        
        for (i = 0; i <= 16; i++)
        {
            if(Res >= Batt_Temperature_Table[i].TemperatureR)
            {
                RES2 = Batt_Temperature_Table[i].TemperatureR;
                TMP2 = Batt_Temperature_Table[i].BatteryTemp;
                break;
            }
            else
            {
                RES1 = Batt_Temperature_Table[i].TemperatureR;
                TMP1 = Batt_Temperature_Table[i].BatteryTemp;
            }
        }
        
        TBatt_Value = (((Res - RES2) * TMP1) + ((RES1 - Res) * TMP2)) / (RES1-RES2);
    }
    
    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printf("BattThermistorConverTemp() : TBatt_Value = %d\n",TBatt_Value);
    printf("BattThermistorConverTemp() : Res = %d\n",Res);
    printf("BattThermistorConverTemp() : RES1 = %d\n",RES1);
    printf("BattThermistorConverTemp() : RES2 = %d\n",RES2);
    printf("BattThermistorConverTemp() : TMP1 = %d\n",TMP1);
    printf("BattThermistorConverTemp() : TMP2 = %d\n",TMP2);
    #endif
    
    return TBatt_Value;    
}

/* convert ADC_bat_temp_volt to register */
INT32 BattVoltToTemp(UINT32 dwVolt)
{
    INT32 TRes;
    INT32 dwVCriBat = (TBAT_OVER_CRITICAL_LOW * RBAT_PULL_UP_VOLT) / (TBAT_OVER_CRITICAL_LOW + RBAT_PULL_UP_R); //~2000mV
    INT32 sBaTTMP = -100;
    
    if(dwVolt > dwVCriBat)
        TRes = TBAT_OVER_CRITICAL_LOW;
    else
        TRes = (RBAT_PULL_UP_R*dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt);        
        
    /* convert register to temperature */
    sBaTTMP = BattThermistorConverTemp(TRes);
    
    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printf("BattVoltToTemp() : TBAT_OVER_CRITICAL_LOW = %d\n", TBAT_OVER_CRITICAL_LOW);
    printf("BattVoltToTemp() : RBAT_PULL_UP_VOLT = %d\n", RBAT_PULL_UP_VOLT);
    printf("BattVoltToTemp() : dwVolt = %d\n", dwVolt);
    printf("BattVoltToTemp() : TRes = %d\n", TRes);
    printf("BattVoltToTemp() : sBaTTMP = %d\n", sBaTTMP);
    #endif
    
    return sBaTTMP;
}

kal_bool pmic_chrdet_status(void)
{
    if (upmu_is_chr_det(CHR) == KAL_TRUE)
    {
        return KAL_TRUE;
    }
    else
    {
        printf("[pmic_chrdet_status] No charger\r\n");
        return KAL_FALSE;
    }
}

void select_charging_curret()
{
    if (BMT_status.charger_type == STANDARD_HOST)
    {
        g_temp_CC_value = USB_CHARGER_CURRENT;
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] STANDARD_HOST CC mode charging : %d\r\n", USB_CHARGER_CURRENT);
        }
    }
    else if (BMT_status.charger_type == NONSTANDARD_CHARGER)
    {
        g_temp_CC_value = USB_CHARGER_CURRENT;
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] NONSTANDARD_CHARGER CC mode charging : %d\r\n", USB_CHARGER_CURRENT); // USB HW limitation
        }
    }
    else if (BMT_status.charger_type == STANDARD_CHARGER)
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] STANDARD_CHARGER CC mode charging : %d\r\n", AC_CHARGER_CURRENT);
        }
    }
    else if (BMT_status.charger_type == CHARGING_HOST)
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] CHARGING_HOST CC mode charging : %d\r\n", AC_CHARGER_CURRENT);
        }
    }
    else
    {
        g_temp_CC_value = Cust_CC_70MA;
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Default CC mode charging : %d\r\n", Cust_CC_70MA);
        }
    }
}

void ChargerHwInit(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] ChargerHwInit\n" );
    }
    upmu_csdac_dly(CHR,0x3);
    upmu_csdac_stp(CHR,0x0);
    //printf("[BATTERY] REG[0x7002FA0C]=%x\r\n", INREG16(0x7002FA0C));
}

void pchr_turn_off_charging (void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] pchr_turn_off_charging !\r\n");
    }
    
    upmu_chrwdt_int_enable(CHR, 0);     // CHRWDT_INT_EN
    upmu_chrwdt_enable(CHR, 0);         // CHRWDT_EN
    upmu_chrwdt_flag(CHR, 0);           // CHRWDT_FLAG
    upmu_csdac_enable(CHR, KAL_FALSE);  // CSDAC_EN
    upmu_chr_enable(CHR, KAL_FALSE);    // CHR_EN		
    
    if (get_chip_eco_ver() != CHIP_E1)
    {
        //for BC1.1 circuit
        upmu_bc11_bb_crtl_enable(CHR, KAL_TRUE);
        upmu_bc11_reset_circuit(CHR, 0x0);
    }
    
    g_charger_is_turn_on = 0;
}

void pchr_turn_on_charging (void)
{
    if ( BMT_status.bat_charging_state == CHR_ERROR ) 
    {
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Charger Error, turn OFF charging !\r\n");
        }
        pchr_turn_off_charging();
    }
    else
    {
        ChargerHwInit();
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] pchr_turn_on_charging !\r\n");
        }
        
        select_charging_curret();
        
        if( g_temp_CC_value == Cust_CC_0MA)
        {
            pchr_turn_off_charging();
        }
        else
        {			
            upmu_chr_current(CHR, g_temp_CC_value);     // CS_VTH
            
            upmu_cs_enable(CHR, KAL_TRUE);              // CS_EN
            upmu_vbat_cc_vth(CHR, g_PMIC_CC_VTH);       // CC_VTH
            if (get_chip_eco_ver() == CHIP_E1)
            {
                upmu_vbat_cv_vth(CHR, g_PMIC_CV_VTH);		// CV_VTH
            }
            else
            {
                upmu_vbat_cv_vth(CHR, PMIC_ADPT_VOLT_04_200000_V);	// VBAT_CV_VTH = 0x10 = 4.2V
            }			
            upmu_vbat_cc_det_enable(CHR, KAL_TRUE);     // CC_EN
            upmu_vbat_cv_det_enable(CHR, KAL_TRUE);     // CV_EN
            
            upmu_chrwdt_td(CHR, 0x3);                   // CHRWDT_TD 32s
            upmu_chrwdt_int_enable(CHR, 1);             // CHRWDT_INT_EN
            upmu_chrwdt_enable(CHR, 1);                 // CHRWDT_EN
            upmu_chrwdt_flag(CHR, 1);                   // CHRWDT_FLAG
            
            upmu_csdac_enable(CHR, KAL_TRUE);           // CSDAC_EN
            upmu_chr_enable(CHR, KAL_TRUE);             // CHR_EN
            
            g_charger_is_turn_on = 1;
        }
    }
    
    if(get_chip_eco_ver()!=CHIP_E1)
    {
        //for BC1.1 circuit
        upmu_bc11_bb_crtl_enable(CHR, KAL_TRUE);
        upmu_bc11_reset_circuit(CHR, 0x1);
    }
}

void SW_CV_Algo_prepare(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] SW_CV_Algo_prepare:%d\n\r", g_temp_CC_value);
    }

    if (g_temp_CC_value == Cust_CC_0MA)
    {
        pchr_turn_off_charging();
    }
    else
    {
        upmu_pchr_csdac_test_enable(CHR, KAL_TRUE);         // CSDAC_TEST = 1
        upmu_pchr_test_csdac_dat(CHR, 0);                   // CSDAC_DAT = 0
        
        upmu_csdac_enable(CHR, KAL_TRUE);                   // CSDAC_EN = 1
        upmu_chr_enable(CHR, KAL_TRUE);                     // CHR_EN = 1
        
        upmu_chr_current(CHR, g_temp_CC_value);             // CS_VTH
        
        //upmu_pchr_ft_ctrl(CHR,0x5);                       // PCHR_FT_CTRL[6:4]=101
        SETREG16(PMIC_RESERVE_CON1,0x0001);                 // [0] CV_MODE=1
        //SETREG16(PMIC_RESERVE_CON1,0x0002);                 // [1] VCDT_MODE=1
        CLRREG16(PMIC_RESERVE_CON1,0x0002);                 // [1] VCDT_MODE=0
        
        upmu_vbat_cv_vth(CHR, PMIC_ADPT_VOLT_04_200000_V);  // VBAT_CV_VTH = 0x10 = 4.2V
        upmu_vbat_cv_det_enable(CHR, KAL_TRUE);             // VBAT_CV_EN = 1
    }	
}

void SW_CV_Algo_task(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] SW_CV_Algo_task---------------%x,%x\n\r", INREG16(CHR_CON_0), INREG16(CHR_CON_1));
    }
    
    upmu_chrwdt_td(CHR, 0x3);           // CHRWDT_TD 32s
    upmu_chrwdt_int_enable(CHR, 1);     // CHRWDT_INT_EN
    upmu_chrwdt_enable(CHR, 1);         // CHRWDT_EN
    upmu_chrwdt_flag(CHR, 1);           // CHRWDT_FLAG
    
    save_value = INREG16(CHR_CON_1);
    VBAT_CV_DET = (save_value & 0x4000) >> 14;
    
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] VBAT_CV_DET=%d\n\r", VBAT_CV_DET);
    }
    
    save_value = INREG16(CHR_CON_2);
    CS_DET = (save_value & 0x8000) >> 15;
    
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] CSDAC_DAT=%d, CS_DET=%d\n\r", CSDAC_DAT, CS_DET);
    }
    
    if (VBAT_CV_DET == 1)
    {
        if (CSDAC_DAT > 0)
        {
            CSDAC_DAT--;
        }							
    }
    else if (CS_DET == 1)
    {
        save_value = INREG16(CHR_CON_4);
        CSDAC_DAT_MAX = (save_value & 0xFF00) >> 8;
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[CS_DET==1] CSDAC_DAT_MAX=%d(%x)\n\r", CSDAC_DAT_MAX, save_value);
        }
        
        if (CSDAC_DAT > 0)
        {
            CSDAC_DAT--;
        }
    }
    else if ((VBAT_CV_DET == 0) && (CS_DET == 0))
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
        printf("CHR_CON_0 = %x\n", INREG16(CHR_CON_0));	
        printf("CHR_CON_1 = %x\n", INREG16(CHR_CON_1));
        printf("CHR_CON_2 = %x\n", INREG16(CHR_CON_2));
        printf("CHR_CON_4 = %x\n", INREG16(CHR_CON_4));
        printf("PMIC_RESERVE_CON1 = %x\n", INREG16(PMIC_RESERVE_CON1));
    }
}

int BAT_CheckPMUStatusReg(void)
{ 
    if( upmu_is_chr_det(CHR) == KAL_TRUE )
    {
        BMT_status.charger_exist = KAL_TRUE;
    }
    else
    {   
        BMT_status.charger_exist = KAL_FALSE;
        
        BMT_status.total_charging_time = 0;
        BMT_status.PRE_charging_time = 0;
        BMT_status.CC_charging_time = 0;
        BMT_status.TOPOFF_charging_time = 0;
        BMT_status.POSTFULL_charging_time = 0;
        
        BMT_status.bat_charging_state = CHR_PRE;        
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] BAT_CheckPMUStatusReg : charger loss \n");
        }
        
        return PMU_STATUS_FAIL;
    }  
    
    return PMU_STATUS_OK;
}

int Get_I_Charging_E1(void)
{
    int ADC_I_SENSE = 0;
    int ADC_BAT_SENSE = 0;
    int ICharging = 0;
    int pre_charger_state = g_charger_is_turn_on;
    
    if (!pre_charger_state)
        pchr_turn_on_charging();
        
    upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
    upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
    
    /* Get V_BAT_SENSE */
    ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL, 500) * R_BAT_SENSE;		
    
    /* Just charger in/out event, same as I_sense */
    ADC_I_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL, 500) * R_I_SENSE;
    
    if(ADC_I_SENSE > ADC_BAT_SENSE)
    {
        ICharging = (ADC_I_SENSE - ADC_BAT_SENSE) * 10 / R_CURRENT_SENSE;
    }
    else
    {
        ICharging = 0;
    }
        
    if (!pre_charger_state)
        pchr_turn_off_charging();
        
    return ICharging;
} 

#if 0
int Get_I_Charging(void)
{
    int ADC_I_SENSE = 0;
    int ADC_BAT_SENSE = 0;
    int ICharging = 0;
    
    //upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
    //upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
    
    ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL, 50) * R_BAT_SENSE;
    ADC_I_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL, 50) * R_I_SENSE;
    if(ADC_I_SENSE > ADC_BAT_SENSE)
    {
        ICharging = (ADC_I_SENSE - ADC_BAT_SENSE) * 10 / R_CURRENT_SENSE;
    }
    else
    {
        ICharging = 0;
    }
    return ICharging;
}
#endif

//new adc sampling algo.
int Get_I_Charging(void)
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
        printf("[Get_I_Charging:BAT_SENSE]\r\n");	
        for(i=0 ; i<repeat ; i++ )
        {
            printf("%d,", ADC_BAT_SENSE_tmp[i]);
        }
        printf("\r\n");
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
        printf("[Get_I_Charging:I_SENSE]\r\n");	
        for(i=0 ; i<repeat ; i++ )
        {
            printf("%d,", ADC_I_SENSE_tmp[i]);
        }
        printf("\r\n");
    }
    #endif
    
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[0];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[1];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[18];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[19];		
    ADC_BAT_SENSE = ADC_BAT_SENSE_sum / (repeat-4);
    
    #if 0
    if (Enable_BATDRV_LOG == 1) {
        printf("[Get_I_Charging] ADC_BAT_SENSE=%d\r\n", ADC_BAT_SENSE);
    }
    #endif
    
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[0];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[1];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[18];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[19];
    ADC_I_SENSE = ADC_I_SENSE_sum / (repeat-4);
    
    #if 0
    if (Enable_BATDRV_LOG == 1) {
        printf("[Get_I_Charging] ADC_I_SENSE(Before)=%d\r\n", ADC_I_SENSE);
    }
    #endif
    
    ADC_I_SENSE += gADC_I_SENSE_offset;
    
    #if 0
    if (Enable_BATDRV_LOG == 1) {
        printf("[Get_I_Charging] ADC_I_SENSE(After)=%d\r\n", ADC_I_SENSE);
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
    int bat_temperature_volt = 0;
#if BATT_ID_CHECK_SUPPORT
    int bat_id_volt = 0;    
#endif

    upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
    upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
    upmu_adc_measure_vchr_enable(CHR, KAL_TRUE);
    
    /* Get V_BAT_SENSE */
    if (g_chr_event == 0)
    {
        BMT_status.ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL, 500) * R_BAT_SENSE;
    }
    else
    {
        /* Just charger in/out event, same as I_sense */
        g_chr_event = 0;
        BMT_status.ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL, 500) * R_I_SENSE;
    }
    BMT_status.bat_vol = BMT_status.ADC_BAT_SENSE;
    
    #if 0
    /* Get V_I_SENSE */
    BMT_status.ADC_I_SENSE = IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL, 500) * R_I_SENSE;
    #endif
    
    /* Get V_Charger */
    BMT_status.charger_vol = IMM_GetOneChannelValue(AUXADC_CHARGER_VOLTAGE_CHANNEL, 50) * (((R_CHARGER_1+R_CHARGER_2) * 100) / R_CHARGER_2);
    BMT_status.charger_vol = BMT_status.charger_vol / 100;
    
    /* Get V_BAT_Temperature */
    bat_temperature_volt = IMM_GetOneChannelValue(AUXADC_TEMPERATURE_CHANNEL, 50);
    BMT_status.temperature = BattVoltToTemp(bat_temperature_volt);
   
#if BATT_ID_CHECK_SUPPORT
    /*Get battery id voltage*/
    bat_id_volt = IMM_GetOneChannelValue(AUXADC_BATT_ID_CHANNEL,5);         /**/
    if((bat_id_volt < CHG_MIN_VALID_BATT_ID) || (bat_id_volt > CHG_MAX_VALID_BATT_ID))      /*if battery id isn't within valid range*/
	{
	  BMT_status.bat_id_valid = 0;
	}
    else
          BMT_status.bat_id_valid = 1;

    if (Enable_BATDRV_LOG == 1) {
        printf("jrd_enter %s [BATTERY:ADC] batt_temp_volt:%ld BAT_id_volt:%ld\n",__func__, bat_temperature_volt, bat_id_volt);
    }
#endif
 
    #if 0
    /* Calculate the charging current */
    if(BMT_status.ADC_I_SENSE > BMT_status.ADC_BAT_SENSE)
        BMT_status.ICharging = (BMT_status.ADC_I_SENSE - BMT_status.ADC_BAT_SENSE) * 10 / R_CURRENT_SENSE;
    else
        BMT_status.ICharging = 0;
    #endif
    
    BMT_status.ICharging = Get_I_Charging();
    
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY:ADC] VCHR:%d BAT_SENSE:%d I_SENSE:%d Current:%d\n", BMT_status.charger_vol,
        BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE, BMT_status.ICharging);
    }
    
    g_BatteryAverageCurrent = BMT_status.ICharging;
}

UINT32 BattVoltToPercent(UINT16 dwVoltage)
{
    UINT32 m = 0;
    UINT32 VBAT1 = 0, VBAT2 = 0;
    UINT32 bPercntResult = 0, bPercnt1 = 0, bPercnt2 = 0;
    
    if (Enable_BATDRV_LOG == 1) {
        printf("###### 100 <-> voltage : %d ######\r\n", Batt_VoltToPercent_Table[10].BattVolt);
    }
    
    if(dwVoltage <= Batt_VoltToPercent_Table[0].BattVolt)
    {
        bPercntResult = Batt_VoltToPercent_Table[0].BattPercent;
        return bPercntResult;
    }
    else if (dwVoltage >= Batt_VoltToPercent_Table[10].BattVolt)
    {
        bPercntResult = Batt_VoltToPercent_Table[10].BattPercent;		
        return bPercntResult;
    }
    else
    {        
        VBAT1 = Batt_VoltToPercent_Table[0].BattVolt;
        bPercnt1 = Batt_VoltToPercent_Table[0].BattPercent;
        for(m = 1; m <= 10; m++)
        {
            if(dwVoltage <= Batt_VoltToPercent_Table[m].BattVolt)
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
    
    bPercntResult = ( ((dwVoltage - VBAT1) * bPercnt2) + ((VBAT2 - dwVoltage) * bPercnt1) ) / (VBAT2 - VBAT1);    
    
    return bPercntResult;
}

int BatChargerOutDetect_SWworkaround_v3(void)
{
    int ret_check_I_charging = 0;
    int j = 0;
    int sw_chr_out_flag = 0;
    int repeat_times = 20;
    //int repeat_times = 1;
    int BAT_status = 0;
	
    /* Get Battery Information */
    BAT_GetVoltage();
    /* If charger does not exist or batt id error*/
#if BATT_ID_CHECK_SUPPORT
    if ((upmu_is_chr_det(CHR) == KAL_FALSE) || (BMT_status.bat_id_valid == 0))
#else
    if (upmu_is_chr_det(CHR) == KAL_FALSE)
#endif
    {
        BMT_status.charger_type = CHARGER_UNKNOWN;
        BMT_status.bat_full = KAL_FALSE;
        g_bat_full_user_view = KAL_FALSE;
        g_usb_state = USB_UNCONFIGURED;

        g_HW_Charging_Done = 0;
        g_Charging_Over_Time = 0;

        CSDAC_DAT_MAX = 255;
     #if BATT_ID_CHECK_SUPPORT
        printf("[BATTERY- workaround] No Charger,or bat_id:%d, Power OFF !?\n",BMT_status.bat_id_valid);
     #else
        printf("[BATTERY] No Charger, Power OFF !?\n");
     #endif
        pchr_turn_off_charging();
        printf("[BATTERY-workaround] mt6573_power_off !!\n");
        #ifndef NO_POWER_OFF
        mt6573_power_off();
        #endif
        while(1);
    }
    
    ret_check_I_charging = Get_I_Charging();	
    
    //printf("[BATTERY] check %d mA !\n", ret_check_I_charging);
    
    if ((ret_check_I_charging < CHR_OUT_CURRENT) && (g_SW_CHR_OUT_EN == 1) && (g_HW_Charging_Done == 0))	
    {
        sw_chr_out_flag = 1;
    
        for (j = 0; j < repeat_times ; j++)
        {
            ret_check_I_charging = Get_I_Charging();
            //printf("[BATTERY] double check %d mA !\n", ret_check_I_charging);
            
            if(ret_check_I_charging > CHR_OUT_CURRENT)
            {
                sw_chr_out_flag = 0;
                break;
            }			
        }
        
        if (sw_chr_out_flag == 1)
        {
            if ((ret_check_I_charging < CHR_OUT_CURRENT) && (g_SW_CHR_OUT_EN == 1) && (g_HW_Charging_Done == 0))
            {
                pchr_turn_off_charging();
                if (Enable_BATDRV_LOG == 1) 
                {
                    printf("[BatChargerOutDetect_SWworkaround_v3] %d < %d\r\n", ret_check_I_charging, CHR_OUT_CURRENT);
                    printf("[BatChargerOutDetect_SWworkaround_v3] %d,%d\r\n", g_SW_CHR_OUT_EN, g_HW_Charging_Done);
                }
            }
            else
            {
                if (Enable_BATDRV_LOG == 1) 
                {
                    printf("[BatChargerOutDetect_SWworkaround_v3] ignore\r\n");
                    printf("[BatChargerOutDetect_SWworkaround_v3] %d,%d\r\n", g_SW_CHR_OUT_EN, g_HW_Charging_Done);
                }
            }
        }
    }
    
    return sw_chr_out_flag;
}

int BAT_CheckBatteryStatus(void)
{
    int BAT_status = PMU_STATUS_OK;
    int i = 0;
    int ret_check_I_charging = 0;
    int j = 0;
    int sw_chr_out_flag = 0;
    int repeat_times = 10;
    
    /* Get Battery Information */
    BAT_GetVoltage();
    
    if (get_chip_eco_ver() == CHIP_E1)
    {
        // E1 SW workaround : charger out detection
        if( (g_BatteryAverageCurrent < CHR_OUT_CURRENT) && (g_SW_CHR_OUT_EN == 1) && (g_HW_Charging_Done == 0) )
        {
            sw_chr_out_flag = 1;
            
            for(j = 0 ; j < repeat_times ; j++)
            {
                ret_check_I_charging = Get_I_Charging_E1();
                printf("[BATTERY] check %d mA !\n", ret_check_I_charging);
                
                if(ret_check_I_charging > CHR_OUT_CURRENT)
                {
                    sw_chr_out_flag = 0;
                    break;
                }			
            }
            
            if (sw_chr_out_flag == 1)
            {
                printf("[BATTERY] BAT_CheckBatteryStatus : No Charger, Power OFF by I < %d mA !\n", CHR_OUT_CURRENT);
                pchr_turn_off_charging();
                #ifndef NO_POWER_OFF
                mt6573_power_off();
                #endif
                while(1);
            }
        }
    }
    else
    {
        // charger plug-out detection by VCDT detection when CHR on
    }
    
    upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
    upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
    BMT_status.ADC_BAT_SENSE = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,500) * R_BAT_SENSE;
    BMT_status.bat_vol = BMT_status.ADC_BAT_SENSE;	
	
    //new adc sampling algo.
    /*
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
            printf("[BATTERY] gADC_BAT_SENSE_temp=%d, gADC_I_SENSE_temp=%d\n\r", gADC_BAT_SENSE_temp, gADC_I_SENSE_temp);
        }
        
        gADC_I_SENSE_offset = gADC_BAT_SENSE_temp - gADC_I_SENSE_temp;
    }
    */
	
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
    
    if (g_bat_volt_cp_flag == 0) 
    {
        g_bat_volt_cp_flag = 1;
        g_bat_volt_check_point = BMT_status.SOC;
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
    
    /**************** For UBOOT : Start ****************/
    if (g_low_bat_boot_display == 0)
    {
    
        /* SOC only UP when charging */
        if ( BMT_status.SOC > g_bat_volt_check_point ) {
            g_bat_volt_check_point = BMT_status.SOC;		
        }
        
        /* UBOOT charging LED */
        if ( (g_bat_volt_check_point >= 90)  || (g_user_view_flag == KAL_TRUE) ) {
            leds_battery_full_charging();
        } else if(g_bat_volt_check_point <= 10) {
            leds_battery_low_charging();
        } else {
            leds_battery_medium_charging();
        }
        
        #if 0
        /* UBOOT charging animation */
        if ( (BMT_status.bat_full) || (g_user_view_flag == KAL_TRUE) ) 
        {		
            if(g_bl_on == KAL_TRUE)
            {	
                mt65xx_disp_show_battery_full();	
            }
            g_user_view_flag = KAL_TRUE;
        } 
        else 
        {	
            if ( (g_bat_volt_check_point>=0) && (g_bat_volt_check_point<25) )
            {
                g_prog_temp = 0;
            }
            else if ( (g_bat_volt_check_point>=25) && (g_bat_volt_check_point<50) )
            {
                g_prog_temp = 25;
            }
            else if ( (g_bat_volt_check_point>=50) && (g_bat_volt_check_point<75) )
            {
                g_prog_temp = 50;
            }
            else if ( (g_bat_volt_check_point>=75) && (g_bat_volt_check_point<100) )
            {
                g_prog_temp = 75;
            }
            else
            {
                g_prog_temp = 100;
            }
            
            if (g_prog_first == 1)
            {
                g_prog = g_prog_temp;
                g_prog_first = 0;
            }
            if(g_bl_on == KAL_TRUE)
            {	
                mt65xx_disp_show_battery_capacity(g_prog);
            }
            g_prog += 25;
            if (g_prog > 100) g_prog = g_prog_temp;
        }
        
        /* UBOOT charging idle mode */
        if (!g_bl_switch) {
            mt65xx_disp_power(KAL_TRUE);
            g_bl_switch_timer++;
            mt65xx_backlight_on();
            g_bl_on = KAL_TRUE;				
        }	
        if (mt6573_detect_key(BACKLIGHT_KEY)) { 
            g_bl_switch = KAL_FALSE;
            g_bl_switch_timer = 0;
            g_bl_on = KAL_TRUE;
            printf("[BATTERY] mt65xx_backlight_on\r\n");
        }	
        if (g_bl_switch_timer > BL_SWITCH_TIMEOUT) {
            g_bl_switch = KAL_TRUE;
            g_bl_switch_timer = 0;
            mt65xx_backlight_off();
            mt65xx_disp_power(KAL_FALSE);
            g_bl_on = KAL_FALSE;
            printf("[BATTERY] mt65xx_backlight_off\r\n");
        }
        #endif
    }

    /**************** For UBOOT : End ****************/	

    //if (Enable_BATDRV_LOG == 1) {
    printf("[BATTERY:AVG] BatTemp:%d Vbat:%d VBatSen:%d SOC:%d ChrDet:%d Vchrin:%d Icharging:%d(%d) ChrType:%d\r\n", 
    BMT_status.temperature ,BMT_status.bat_vol, BMT_status.ADC_BAT_SENSE, BMT_status.SOC, 
    upmu_is_chr_det(CHR), BMT_status.charger_vol, BMT_status.ICharging, g_BatteryAverageCurrent, CHR_Type_num );  
    //}  
    
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] CON_9:%x, CON10:%x\n\r", INREG16(CHR_CON_9), INREG16(CHR_CON_10));
    }
    
    /* Protection Check : start*/
    //BAT_status = BAT_CheckPMUStatusReg();
    //if(BAT_status != PMU_STATUS_OK)
    //    return PMU_STATUS_FAIL;                  
    
    #if (BAT_TEMP_PROTECT_ENABLE == 1)
    if ((BMT_status.temperature <= MIN_CHARGE_TEMPERATURE) || 
        (BMT_status.temperature == ERR_CHARGE_TEMPERATURE))
    {
        printf("[BATTERY] Battery Under Temperature or NTC fail !!\n\r");
        BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL;       
    }
    #endif		    
    
    if (BMT_status.temperature >= MAX_CHARGE_TEMPERATURE)
    {
        printf("[BATTERY] Battery Over Temperature !!\n\r");
        BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL;       
    }
    
    if (upmu_is_chr_det(CHR) == KAL_TRUE)
    {
        #if (V_CHARGER_ENABLE == 1)
        if (BMT_status.charger_vol <= V_CHARGER_MIN )
        {
            printf("[BATTERY]Charger under voltage!!\r\n");                    
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;        
        }
        #endif
        
        if ( BMT_status.charger_vol >= V_CHARGER_MAX )
        {
            printf("[BATTERY]Charger over voltage !!\r\n");                    
            BMT_status.charger_protect_status = charger_OVER_VOL;
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;        
        }
    }
    /* Protection Check : end*/
    
    if (upmu_is_chr_det(CHR) == KAL_TRUE)
    {
        if ((BMT_status.bat_vol < RECHARGING_VOLTAGE) && (BMT_status.bat_full) && (g_HW_Charging_Done == 1) && (!g_Battery_Fail))	
        {
            //if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Battery Re-charging !!\n\r");                
            //}
            BMT_status.bat_full = KAL_FALSE;    
            g_bat_full_user_view = KAL_TRUE;
            //BMT_status.bat_charging_state = CHR_CC;
            BMT_status.bat_charging_state = CHR_PRE;
                        
            g_HW_Charging_Done = 0;
            
            CSDAC_DAT_MAX = 255;
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
        printf(  "[BATTERY] BAD Battery status... Charging Stop !!\n\r");            
    }
    
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
    
    /*  Disable charger */
    pchr_turn_off_charging();
    
    g_sw_cv_enable = 0;
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_ChargingOTAction(void)
{    
    printf(  "[BATTERY] Charging over 12 hr stop !!\n\r");            
    
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
    
    g_HW_Charging_Done = 1;	
    g_Charging_Over_Time = 1;
    
    /*  Disable charger*/
    pchr_turn_off_charging();
    
    g_sw_cv_enable = 0;
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryFullAction(void)
{
    //if (Enable_BATDRV_LOG == 1) {    
    printf(  "[BATTERY] Battery full !!\n\r");            
    //}
    
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
    
    g_HW_Charging_Done = 1;
    
    /*  Disable charger */
    pchr_turn_off_charging();
    
    g_sw_cv_enable = 0;
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_PreChargeModeAction(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf(  "[BATTERY] Pre-CC mode charge, timer=%d on %d !!\n\r",
        BMT_status.PRE_charging_time, BMT_status.total_charging_time);    
    }
    
    BMT_status.PRE_charging_time += BAT_TASK_PERIOD;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    
    /*  Enable charger */
    pchr_turn_on_charging();	        
    
    if (BMT_status.bat_vol > V_PRE2CC_THRES)
    {
        BMT_status.bat_charging_state = CHR_CC;
    }
    
    save_value = 0x0;
    CSDAC_DAT_MAX = 255;
    CSDAC_DAT = 0;
    VBAT_CV_DET = 0x0;
    CS_DET = 0x0;
    g_sw_cv_enable = 0;
        
    return PMU_STATUS_OK;        
} 

PMU_STATUS BAT_ConstantCurrentModeAction(void)
{
    int i = 0;
    
    if (Enable_BATDRV_LOG == 1) {
        printf(  "[BATTERY] CC mode charge, timer=%d on %d !!\n\r",
        BMT_status.CC_charging_time, BMT_status.total_charging_time);    
    }
    
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time += BAT_TASK_PERIOD;
    BMT_status.TOPOFF_charging_time = 0;
    
    upmu_pchr_test_csdac_dat(CHR, 0); // CSDAC_DAT = 0
    CLRREG16(0x7002FA10, 0x0002); // CSDAC_TEST = 0
    
    /*  Enable charger */
    pchr_turn_on_charging();	        
    
    if (BMT_status.bat_vol > V_CC2TOPOFF_THRES)
    {
        if (get_chip_eco_ver() != CHIP_E1)
        {
            pchr_turn_off_charging();
        }
        
        BMT_status.bat_charging_state = CHR_TOP_OFF;
        g_first_top_off = 1;
        
        if (get_chip_eco_ver() != CHIP_E1)
        {            
            SW_CV_Algo_prepare();
            
            if (g_temp_CC_value == AC_CHARGER_CURRENT)
            {
                for (i = 0; i < BATTERY_AVERAGE_SIZE; i++) {	            
                    batteryCurrentBuffer[i] = 650;
                }
                batteryCurrentSum = 650 * BATTERY_AVERAGE_SIZE;
            }
            else
            {
                for (i = 0; i < BATTERY_AVERAGE_SIZE; i++) {	            
                    batteryCurrentBuffer[i] = 450;
                }
                batteryCurrentSum = 450 * BATTERY_AVERAGE_SIZE;
            }
        }
        
        g_sw_cv_enable = 1;
    }
    else
    {
        g_sw_cv_enable = 0; 
    }
    
    save_value = 0x0;
    CSDAC_DAT_MAX = 255;
    CSDAC_DAT = 0;
    VBAT_CV_DET = 0x0;
    CS_DET = 0x0;
    
    return PMU_STATUS_OK;        
}    

PMU_STATUS BAT_TopOffModeAction(void)
{
    //int i = 0;
    //int CV_counter = 0;
    int ret_check_I_charging = 0;
    
    if (Enable_BATDRV_LOG == 1) {
        printf(  "[BATTERY] Top Off mode charge, timer=%d on %d !!\n\r",
        BMT_status.TOPOFF_charging_time, BMT_status.total_charging_time);    
    }
    
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time += BAT_TASK_PERIOD;
    
    if (BMT_status.TOPOFF_charging_time >= 20)
    {
        ret_check_I_charging = Get_I_Charging();
        if (ret_check_I_charging > CHR_OUT_CURRENT)
        {
            /* enable charger */
            pchr_turn_on_charging();	  
        }   
        if (Enable_BATDRV_LOG == 1) {
            printk("[BAT_TopOffModeAction] ret_check_I_charging=%dmA\n\r", ret_check_I_charging);
        }
    }
    else
    {
        /* enable charger */
        pchr_turn_on_charging();	  
    }
    
    g_sw_cv_enable = 1;   
    
    return PMU_STATUS_OK;        
} 

PMU_STATUS BAT_PostFullModeAction(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] In BAT_PostFullModeAction() \n");
    }
    
    g_sw_cv_enable = 0;
    
    return PMU_STATUS_OK;        
} 

void BAT_thread(void)
{
    int BAT_status = 0;
    
    if (Enable_BATDRV_LOG == 1) {
        printk("[BATTERY_TOP] LOG. %d,%d,%d----------------------------\n", BATTERY_AVERAGE_SIZE, CHARGING_FULL_CURRENT, RECHARGING_VOLTAGE);
    }
    
    /* If charger does not exist or batt id error*/
#if BATT_ID_CHECK_SUPPORT
    if ((upmu_is_chr_det(CHR) == KAL_FALSE) || (BMT_status.bat_id_valid == 0))
#else
    if (upmu_is_chr_det(CHR) == KAL_FALSE)
#endif
    {
        BMT_status.charger_type = CHARGER_UNKNOWN;
        BMT_status.bat_full = KAL_FALSE;
        g_bat_full_user_view = KAL_FALSE;
        g_usb_state = USB_UNCONFIGURED;
        
        g_HW_Charging_Done = 0;
        g_Charging_Over_Time = 0;
        
        CSDAC_DAT_MAX = 255;
     #if BATT_ID_CHECK_SUPPORT
	printf("[BATTERY] No Charger,or bat_id:%d, Power OFF !?\n",BMT_status.bat_id_valid);
     #else	   
        printf("[BATTERY] No Charger, Power OFF !?\n");
     #endif	
        pchr_turn_off_charging();
        printf("[BATTERY] mt6573_power_off !!\n");
        #ifndef NO_POWER_OFF
        mt6573_power_off();
        #endif
        while(1);
    }
    
    BMT_status.total_charging_time++;
    
    if (get_chip_eco_ver() == CHIP_E1)
    {
        if ((BMT_status.total_charging_time % 9) == 0)
        {
            pchr_turn_off_charging();
            return;
        }
    }
    else
    {
        if ((upmu_is_chr_det(CHR) == KAL_TRUE) && (g_HW_Charging_Done == 0) &&
            (BMT_status.bat_charging_state != CHR_ERROR) &&
            (BMT_status.bat_charging_state != CHR_TOP_OFF))
        {
            if ((BMT_status.total_charging_time % 9) == 0)
            {		
                pchr_turn_off_charging();
                
                upmu_adc_measure_vbat_enable(CHR, KAL_TRUE);
                upmu_adc_measure_vsen_enable(CHR, KAL_TRUE);
                
                //new adc sampling algo.		
                if (BMT_status.bat_charging_state != CHR_TOP_OFF)
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
                        printf("[BATTERY] gADC_BAT_SENSE_temp=%d, gADC_I_SENSE_temp=%d\n\r", gADC_BAT_SENSE_temp, gADC_I_SENSE_temp);
                    }
                    
                    gADC_I_SENSE_offset = gADC_BAT_SENSE_temp - gADC_I_SENSE_temp;
                }
            
                return;   
            }
            else
            {
                pchr_turn_on_charging();
            }
        }
        else
        {
            //printf("SW CV mode do not dis-charging 1s\n");
        }
        
        if (BatChargerOutDetect_SWworkaround_v3()) // for charger out detection
            return;
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
        
    /* No Charger */
    if(BAT_status == PMU_STATUS_FAIL || g_Battery_Fail)    
    {
        BAT_BatteryStatusFailAction();
    }
    
    /* Battery Full */
    /* HW charging done, real stop charging */
    else if (g_HW_Charging_Done == 1)
    {   
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Battery real full. \n");
        }
        BAT_BatteryFullAction();		
    }
    
    /* Charging Overtime, can not charging */
    else if (g_Charging_Over_Time == 1)
    {
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Charging Over Time. \n");
        }
        pchr_turn_off_charging();
    }
    
    /* Battery Not Full and Charger exist : Do Charging */
    else
    {
        /* Charging 8hr */
        if(BMT_status.total_charging_time >= MAX_CHARGING_TIME)
        {
            BMT_status.bat_charging_state == CHR_BATFULL;
            BAT_ChargingOTAction();
            return;
        }
        
        if ( BMT_status.TOPOFF_charging_time >= MAX_CV_CHARGING_TIME )
        {
            if (Enable_BATDRV_LOG == 1) {
                printf("BMT_status.TOPOFF_charging_time >= %d \r\n", MAX_CV_CHARGING_TIME);
            }
            BMT_status.bat_charging_state == CHR_BATFULL;
            BAT_BatteryFullAction();										
            return;
        }
        
        if(get_chip_eco_ver() == CHIP_E1)
        {
            if ( (BMT_status.bat_charging_state == CHR_TOP_OFF) &&
                (BMT_status.SOC == 100) && 
                (BMT_status.bat_vol >= Batt_VoltToPercent_Table[10].BattVolt) )
            {
                if (Enable_BATDRV_LOG == 1) {
                    printf("[BATTERY] Battery real full(%d,%d) and disable charging !\n", 
                    BMT_status.SOC, Batt_VoltToPercent_Table[10].BattVolt); 
                }
                BMT_status.bat_charging_state == CHR_BATFULL;
                BAT_BatteryFullAction();
                return;
            }	
        }
        else
        {
            /* charging full condition when charging current < CHARGING_FULL_CURRENT mA on CHR_TOP_OFF mode*/
            if ( (BMT_status.bat_charging_state == CHR_TOP_OFF ) 
                && (BMT_status.TOPOFF_charging_time > 60)
                && (BMT_status.ICharging <= CHARGING_FULL_CURRENT))
            {
                BMT_status.bat_charging_state = CHR_BATFULL;
                BAT_BatteryFullAction();				
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
            
            default :
                if (Enable_BATDRV_LOG == 1) {
                    printf("BMT_status.bat_charging_state ??\n");
                }
                break;
        }    
    }
    
    g_SW_CHR_OUT_EN = 1;
    
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] BAT_thread : Done\n");
        printf("[BATTERY] CON_9:%x, CON10:%x\n\r", INREG16(CHR_CON_9), INREG16(CHR_CON_10));
    }
}

void uboot_charging_display()
{
    #define BATTERY_BAR  10
    
    if ((BMT_status.bat_full) || (g_user_view_flag == KAL_TRUE) ) 
    {		
        if (g_bl_on == KAL_TRUE)
        {	
            mt65xx_disp_show_battery_full();	
        }
        g_user_view_flag = KAL_TRUE;
    } 
    else 
    {
        g_prog_temp = (g_bat_volt_check_point/BATTERY_BAR) * BATTERY_BAR;
        
        if (g_prog_first == 1)
        {
            g_prog = g_prog_temp;
            g_prog_first = 0;
        }
        if(g_bl_on == KAL_TRUE)
        {	
            mt65xx_disp_show_battery_capacity(g_prog);
        }
        g_prog += BATTERY_BAR;
        if (g_prog > 90) g_prog = g_prog_temp;
    }
    
    /* UBOOT charging idle mode */
    if (!g_bl_switch) 
    {
        mt65xx_disp_power(KAL_TRUE);
        g_bl_switch_timer++;
        mt65xx_backlight_on();
        g_bl_on = KAL_TRUE;				
    }	
    
    if (g_bl_switch_timer > BL_SWITCH_TIMEOUT) 
    {
        g_bl_switch = KAL_TRUE;
        g_bl_switch_timer = 0;
        mt65xx_backlight_off();
        mt65xx_disp_power(KAL_FALSE);
        g_bl_on = KAL_FALSE;
        printf("[BATTERY] mt65xx_backlight_off\r\n");
    }
}

void batdrv_init(void)
{
    int i = 0;
    
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
    BMT_status.charger_exist = 0;   /* for default, no charger */
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
    
    if (get_chip_eco_ver() != CHIP_E1)
    {
        printf("[BATTERY] Apply SW CV mode\r\n");
        //SETREG16(PMIC_RESERVE_CON1,0x0002); // [1] VCDT_MODE = 1
        CLRREG16(PMIC_RESERVE_CON1,0x0002); // [1] VCDT_MODE = 0
    }
	
    printf("[BATTERY] batdrv_init : Done\n");
}

void mt65xx_bat_init(void)
{
    long tmo = 0, tmo2 = 0;
    int i = 0, press_pwrkey_count = 0, loop_count = 0;
    
    kal_bool print_msg = KAL_FALSE;
    kal_bool pwrkey_ready = KAL_FALSE;
	/// mark_20110815_for_jrd_not_show_low_battery_after_release_power_key beg
	kal_bool back_to_power_off_charging = KAL_FALSE;

    #if (CHARGING_PICTURE == 1)
    mt65xx_disp_enter_charging_state();
    #else
    mt65xx_disp_show_boot_logo();
    #endif	
    
    batdrv_init();
    apost_hw_init();
    
    BMT_status.bat_full = KAL_FALSE;
    BAT_GetVoltage();
    
    if (BMT_status.bat_vol > RECHARGING_VOLTAGE) {
        g_user_view_flag = KAL_TRUE;
    } else {
        g_user_view_flag = KAL_FALSE;
    }
    
    if(mt6573_detect_key(8))
    {
        pwrkey_ready = KAL_TRUE;
    }
    else
    {
        pwrkey_ready = KAL_FALSE;
    }

        #if BATT_ID_CHECK_SUPPORT
        if(BMT_status.bat_id_valid == 1)
        {
        #endif
	    if(upmu_is_chr_det(CHR) == KAL_TRUE)
		{
                mt65xx_disp_show_battery_capacity(0);   /*display empty battery now, start to enter charging state*/
                mt65xx_disp_wait_idle();
                tmo = get_timer(0);
                while(get_timer(tmo) <= 50 /* ms */);
                mt65xx_backlight_on();
		}
        #if BATT_ID_CHECK_SUPPORT
        }
        else    /*batt id error power off now*/
        {
                BMT_status.charger_type = CHARGER_UNKNOWN;
                BMT_status.bat_full = KAL_FALSE;
                g_bat_full_user_view = KAL_FALSE;
                g_usb_state = USB_UNCONFIGURED;

                g_HW_Charging_Done = 0;
                g_Charging_Over_Time = 0;

                CSDAC_DAT_MAX = 255;
                printf("[BATTERY] bat_id:%d!error Power OFF !?\n",BMT_status.bat_id_valid);
                pchr_turn_off_charging();

                printf("[BATTERY] mt6573_power_off !!\n");
                #ifndef NO_POWER_OFF
                mt6573_power_off();
                #endif
                while(1);
        }
        #endif
    
    /* Boot with Charger */
    if ((upmu_is_chr_det(CHR) == KAL_TRUE))	
    {
        while (1) 
        {
            /* If charger does not exist */
	#if BATT_ID_CHECK_SUPPORT
	    if ((upmu_is_chr_det(CHR) == KAL_FALSE) || (BMT_status.bat_id_valid == 0))	
	#else
            if (upmu_is_chr_det(CHR) == KAL_FALSE)
	#endif
            {
                BMT_status.charger_type = CHARGER_UNKNOWN;
                BMT_status.bat_full = KAL_FALSE;
                g_bat_full_user_view = KAL_FALSE;
                g_usb_state = USB_UNCONFIGURED;
                
                g_HW_Charging_Done = 0;
                g_Charging_Over_Time = 0;
                
                CSDAC_DAT_MAX = 255;
	#if BATT_ID_CHECK_SUPPORT	                
                printf("[BATTERY] No Charger or bat_id:%d! Power OFF !?\n",BMT_status.bat_id_valid);
	#else
		printf("[BATTERY] No Charger! Power OFF !?\n");
	#endif
                pchr_turn_off_charging();
                
                printf("[BATTERY] mt6573_power_off !!\n");
                #ifndef NO_POWER_OFF
                mt6573_power_off();
                #endif
                while(1);
            }
           
#if 1

            while(1)
            {
                /* Get V_Charger */
                BMT_status.charger_vol = IMM_GetOneChannelValue(AUXADC_CHARGER_VOLTAGE_CHANNEL, 50) * (((R_CHARGER_1+R_CHARGER_2) * 100) / R_CHARGER_2);
                BMT_status.charger_vol = BMT_status.charger_vol / 100;
                if(BMT_status.charger_vol < V_CHARGER_MAX)
                {
                      break;
                }
                else
                {
                        BMT_status.charger_type = CHARGER_UNKNOWN;
                        BMT_status.bat_full = KAL_FALSE;
                        g_bat_full_user_view = KAL_FALSE;
                        g_usb_state = USB_UNCONFIGURED;

                        g_HW_Charging_Done = 0;
                        g_Charging_Over_Time = 0;

                        CSDAC_DAT_MAX = 255;
                        mt65xx_backlight_off();
                        pchr_turn_off_charging();
                }
            }

#endif
 
            if (rtc_boot_check() || meta_mode_check() || (pwrkey_ready == KAL_TRUE))
            {
                // Low Battery Safety Booting
                pchr_turn_off_charging();
                BMT_status.bat_vol = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,500) * R_BAT_SENSE;
                printf("check VBAT=%d < %d\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);
	    #if BATT_ID_CHECK_SUPPORT
		if(BMT_status.bat_id_valid == 1)
			pchr_turn_on_charging();
	    #else
		pchr_turn_on_charging();	
	    #endif
                
                while ( BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD )
                {	
                    if (g_low_bat_boot_display == 0)
                    {
                        mt65xx_disp_power(KAL_TRUE);
                        mt65xx_backlight_off();
                        printf("Before mt65xx_disp_show_low_battery\r\n");
                        mt65xx_disp_show_low_battery();
                        printf("After mt65xx_disp_show_low_battery\r\n");
                        mt65xx_disp_wait_idle();
                        printf("After mt65xx_disp_wait_idle\r\n");
                        
                        g_low_bat_boot_display = 1;												
                        
                        printf("Set low brightness\r\n");
                        mt65xx_leds_brightness_set(6, 10);
                    }
                    
                    rtc_boot_check();
                    BAT_thread();
                    printf("-");
                    
                    #ifdef GPT_TIMER		
                    if (g_bl_on == KAL_TRUE)
                        mt6573_sleep(1000, KAL_FALSE);
                    else
                        mt6573_sleep(1000, KAL_TRUE);                        
                    #else
                    tmo2 = get_timer(0);            
                    while(get_timer(tmo2) <= 1000 /* ms */);                    
                    #endif			
                    
					/// mark_20110815_for_jrd_not_show_low_battery_after_release_power_key beg
					if (pwrkey_ready && !mt6573_detect_key(8))
					{
					    back_to_power_off_charging = TRUE;
						break;
					}
                    
                    pchr_turn_off_charging();
                    BMT_status.bat_vol = IMM_GetOneChannelValue(AUXADC_BATTERY_VOLTAGE_CHANNEL,500) * R_BAT_SENSE;
                    printf("VBAT=%d < %d\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);
                #if BATT_ID_CHECK_SUPPORT
                    if(BMT_status.bat_id_valid == 1)
			pchr_turn_on_charging();
		#else
		    pchr_turn_on_charging();
		#endif

                }
		/// mark_20110815_for_jrd_not_show_low_battery_after_release_power_key beg
		if (!back_to_power_off_charging)
		{
                mt65xx_disp_power(KAL_TRUE);
                mt65xx_disp_show_boot_logo();
                
                // update twice here to ensure the LCM is ready to show the
                // boot logo before turn on backlight, OR user may glimpse
                // at the previous battery charging screen
                mt65xx_disp_show_boot_logo();
                mt65xx_disp_wait_idle();
                
                printf("Restore brightness\r\n");
                mt65xx_leds_brightness_set(6, 255);
                mt65xx_backlight_on();
                
                pchr_turn_off_charging();
                return;
            }

		back_to_power_off_charging = FALSE;
		pwrkey_ready = FALSE;
		g_low_bat_boot_display = 0;
				
                g_bl_switch = KAL_FALSE;
                g_bl_switch_timer = 0;
                g_bl_on = KAL_TRUE;
            }
            else
            {
                //printf("[BATTERY] U-BOOT Charging !!\n\r");  
            }
            
            if (print_msg == KAL_FALSE)
            {	
                uboot_charging_display();
                
                printf("[BATTERY] Charging !! Press Power Key to Booting !!!\n\r");				
                print_msg = KAL_TRUE;
                
                CHR_Type_num = mt_charger_type_detection();
                BMT_status.charger_type = CHR_Type_num;
                
                SETREG16(0x7002FE84,0x0004); // enable CSDAC_MODE
	    #if BATT_ID_CHECK_SUPPORT
		if(BMT_status.bat_id_valid == 1)
			pchr_turn_on_charging();
	    #else	
                pchr_turn_on_charging();
	    #endif
            }
            
            if (get_chip_eco_ver() != CHIP_E1 && g_sw_cv_enable == 1 && 
                BMT_status.bat_charging_state == CHR_TOP_OFF)
            {
                if (g_first_top_off)
                {
                    for (i = 0; i < 100; i++)
                    {
                        SW_CV_Algo_task();
                        
                        tmo = get_timer(0);			
                        while(get_timer(tmo) <= 20 /* ms */);
                        
                        if (((i % 5) == 0) && g_bl_switch == KAL_FALSE)
                        {
                            if ((BMT_status.total_charging_time % 9) == 0)
                            {
                                tmo = get_timer(0);			
                                while(get_timer(tmo) <= 50 /* ms */); 
                            }
                            uboot_charging_display();
                        }
                    }
                    g_first_top_off--;
                    BAT_thread();
                }
                else
                {
                    if (BatChargerOutDetect_SWworkaround_v3()) // for charger out detection
                        continue;
                    SW_CV_Algo_task();
                }
                
                if (g_thread_count >= 10)
                {
                    g_thread_count = 1;
                    BAT_thread();
                    printf(".");
                }
                else
                {
                    g_thread_count++;
                }
                
                #ifdef GPT_TIMER		
                if (g_bl_on == KAL_TRUE)
                    mt6573_sleep(100, KAL_FALSE);
                else
                    mt6573_sleep(100, KAL_TRUE);
                #else
                tmo = get_timer(0);			
                while(get_timer(tmo) <= 100 /* ms */);
                #endif		
            }
            else
            {
                if (g_thread_count >= 10)
                {
                    g_thread_count = 1;
                    BAT_thread();
                    printf(".");
                }
                else
                {
                    g_thread_count++;
                }
                
                #ifdef GPT_TIMER
                if (g_bl_on == KAL_TRUE)
                    mt6573_sleep(100, KAL_FALSE);
                else
                    mt6573_sleep(100, KAL_TRUE);
                #else
                tmo = get_timer(0);			
                while(get_timer(tmo) <= 100 /* ms */);
                #endif
            }
            
            if (++loop_count == 60) loop_count = 0;
            
            if (mt6573_detect_key(8))
                press_pwrkey_count++;
            else
                press_pwrkey_count = 0;
            
            if (press_pwrkey_count > POWER_ON_TIME)	
                pwrkey_ready = KAL_TRUE;
            else
                pwrkey_ready = KAL_FALSE;
            
            if (mt6573_detect_key(BACKLIGHT_KEY)) 
            {
                g_bl_switch = KAL_FALSE;
                g_bl_switch_timer = 0;
                g_bl_on = KAL_TRUE;
                printf("[BATTERY] mt65xx_backlight_on\r\n");
            }	
            
            if (((loop_count % 5) == 0) && g_bl_switch == KAL_FALSE) /// modify from 1 % to 5 %
            {
                if ((BMT_status.total_charging_time % 9) == 0)
                {
                    tmo = get_timer(0);			
                    while(get_timer(tmo) <= 50 /* ms */); 
                }
                uboot_charging_display();
            }
        }
    }
    else
    {
        if (rtc_boot_check() && BMT_status.bat_vol >= BATTERY_LOWVOL_THRESOLD)
        {
            printf("[BATTERY] battery voltage >= CLV ! Boot Linux Kernel !! \n\r");
            return;
        }
        else
        {
            printf("[BATTERY] battery voltage(%d) <= CLV ! Can not Boot Linux Kernel !! \n\r", BMT_status.bat_vol);
            pchr_turn_off_charging();
            
            #ifndef NO_POWER_OFF
            mt6573_power_off();
            #endif
        }
    }
    
    return;
}

#else

#include <asm/arch/mt65xx_typedefs.h>

void mt65xx_bat_init(void)
{
    printf("[BATTERY] Skip mt65xx_bat_init !!\n\r");
    printf("[BATTERY] If you want to enable power off charging, \n\r");
    printf("[BATTERY] Please #define CFG_POWER_CHARGING!!\n\r");
}

#endif
