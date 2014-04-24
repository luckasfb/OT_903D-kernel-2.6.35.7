
#include <common.h>
#include <asm/io.h>

#include "mt6573_auxadc_sw.h"
#include "mt6573_auxadc_hw.h"

///////////////////////////////////////////////////////////////////////////////////////////
//// Define
typedef unsigned short  kal_uint16;
typedef signed int      kal_int32;
typedef enum 
{
  KAL_FALSE,
  KAL_TRUE
} kal_bool;

kal_bool g_EFUSE_Cali = KAL_FALSE;

#define DRV_Reg(addr)               (*(volatile kal_uint16 *)(addr))
#define DRV_WriteReg(addr,data)     ((*(volatile kal_uint16 *)(addr)) = (kal_uint16)(data))

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

#define AUXADC_DRV_ClearBits16(addr, data)           DRV_ClearBits(addr,data)
#define AUXADC_DRV_SetBits16(addr, data)             DRV_SetBits(addr,data)
#define AUXADC_DRV_WriteReg16(addr, data)            DRV_WriteReg(addr, data)
#define AUXADC_DRV_ReadReg16(addr)                   DRV_Reg(addr)
#define AUXADC_DRV_SetData16(addr, bitmask, value)   DRV_SetData(addr, bitmask, value)
#define AUXADC_DRV_WriteReg32(addr, data)			 writel(addr, data)
#define AUXADC_DRV_ReadReg32(addr)					 readl(addr)	

#define AUXADC_DVT_DELAYMACRO(u4Num)                                     \
{                                                                        \
    unsigned int u4Count = 0 ;                                                 \
    for (u4Count = 0; u4Count < u4Num; u4Count++ );                      \
}

#define AUXADC_SET_BITS(BS,REG)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define AUXADC_CLR_BITS(BS,REG)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

#define VOLTAGE_FULL_RANGE 	2500 // VA voltage
#define ADC_PRECISE 		4096 // 12 bits

#define AUXADC_CHARGER_VOLTAGE_CHANNEL     2

int auxadc_cali_slop_def[14]   = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
int auxadc_cali_offset_def[14] = {58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267};
int auxadc_cali_cal_def[1]     = {0};
int auxadc_cali_slop_efuse[14]   = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
int auxadc_cali_offset_efuse[14] = {58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267,58267};
int auxadc_cali_cal_efuse[1]     = {0};

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//// Internal API
inline static void mt6573_ADC_2G_power_up(void)
{
    //2010/07/27: mt6573, the ADC 2G power on is controlled by APMCU_CG_CLR0
    //#define PDN_CLR0 (0xF7026308)  
    #define PDN_CLR0 (0x70026308)  
    unsigned int poweron = 1 << 4;
    AUXADC_SET_BITS(poweron, PDN_CLR0);
}

inline static void mt6573_ADC_2G_power_down(void)
{
    //2010/07/27: mt6573, the ADC 2G power on is controlled by APMCU_CG_SET0
    //#define PDN_SET0 (0xF7026304)  
    #define PDN_SET0 (0x70026304)  
    unsigned int poweroff = 1 << 4;
    AUXADC_SET_BITS(poweroff, PDN_SET0);
}

inline static void mt6573_ADC_3G_power_up(void)
{
    //2010/07/27: mt6573, the ADC 3G power on is controlled by APMCU_CG_CLR0
    //#define PDN_CLR0 (0xF7026308)  
    #define PDN_CLR0 (0x70026308)  
    unsigned int poweron = 1 << 13;
    AUXADC_SET_BITS(poweron, PDN_CLR0);
}

inline static void mt6573_ADC_3G_power_down(void)
{
    //2010/07/27: mt6573, the ADC 3G power on is controlled by APMCU_CG_SET0
    //#define PDN_SET0 (0xF7026304)  
    #define PDN_SET0 (0x70026304)  
    unsigned int poweroff = 1 << 13;
    AUXADC_SET_BITS(poweroff, PDN_SET0);
}
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

//This function is to get adc calibration data from EFUSE
void adc_read_efuse_data(kal_int32 *slope, kal_int32 *offset)
{
	kal_int32 efuseA, efuseB;
	kal_int32 a,b;
	kal_int32 dat = 0;	

	dat = AUXADC_DRV_ReadReg32(AUXADC_EFUSE_DAT); 
	efuseA = (dat & EFUSE_ADC_A_MASK) >> EFUSE_ADC_A_SHIFT; 
	efuseB = (dat & EFUSE_ADC_B_MASK) >> EFUSE_ADC_B_SHIFT;	

	a=1000000+(((efuseA-256)*1000000)/4096);
	b=((efuseB-128)*1000000)/4096;

	*slope  = ((25000000/a)*100000)/4096;
	*offset = (-2500000/a)*b;
}
//// Common API
int IMM_GetOneChannelValue(int dwChannel, int deCount)
{
    unsigned int u4Sample_times = 0;
    unsigned int dat = 0;
	unsigned int u4channel[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned int adc_result = 0;
    unsigned int adc_temp_result = 0;
   kal_int32 slope,offset;	
   kal_int32 slop_def, offset_def;
   int i;	

	if(g_EFUSE_Cali == KAL_FALSE) {
		//read calibration data from EFUSE
		if(AUXADC_DRV_ReadReg32(AUXADC_EFUSE_DAT) & EFUSE_ADC_CALI_EN_MASK) {
			adc_read_efuse_data(&slope, &offset);
			for(i=0;i<14;i++) {
				auxadc_cali_slop_efuse[i] = (int)slope;
				auxadc_cali_offset_efuse[i] = (int)offset;
			}
			g_EFUSE_Cali = KAL_TRUE;
		}
	}

    /* Enable ADC power bit */    
    mt6573_ADC_2G_power_up();
	mt6573_ADC_3G_power_up();

    /* Initialize ADC control register */
    AUXADC_DRV_WriteReg16(AUXADC_CON0, 0);
    AUXADC_DRV_WriteReg16(AUXADC_CON1, 0);    
    AUXADC_DRV_WriteReg16(AUXADC_CON2, 0);    
    AUXADC_DRV_WriteReg16(AUXADC_CON3, 0);   

    do
    {
        //pmic_adc_vbat_enable(KAL_TRUE);		// move to the whom driver
        //pmic_adc_isense_enable(KAL_TRUE); 	// move to the whom driver

        AUXADC_DRV_WriteReg16(AUXADC_CON1, 0);        
        AUXADC_DRV_WriteReg16(AUXADC_CON1, 0x1FFF);
         
        AUXADC_DVT_DELAYMACRO(1000);

        /* Polling until bit STA = 0 */
        while (0 != (0x01 & AUXADC_DRV_ReadReg16(AUXADC_CON3)));          

        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT0);        
        u4channel[0]  += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT1);        
        u4channel[1]  += (dat & 0x0FFF);   
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT2);        
        u4channel[2]  += (dat & 0x0FFF);   
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT3);        
        u4channel[3]  += (dat & 0x0FFF);   
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT4);
        u4channel[4]  += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT5);
        u4channel[5]  += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT6);
        u4channel[6]  += (dat & 0x0FFF);  
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT7);
        u4channel[7]  += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT8);
        u4channel[8]  += (dat & 0x0FFF);    
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT9);
        u4channel[9]  += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT10);
        u4channel[10] += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT11);
        u4channel[11] += (dat & 0x0FFF);
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT12);
        u4channel[12] += (dat & 0x0FFF);		
        dat = AUXADC_DRV_ReadReg16(AUXADC_DAT13);
        u4channel[13] += (dat & 0x0FFF);
        
        u4Sample_times++;
    }
    while (u4Sample_times < deCount);

    /* Disable ADC power bit */    
    mt6573_ADC_2G_power_down();
	mt6573_ADC_3G_power_down();

    #if 0
    printf("BAT_GetVoltage : channel_0  = %d / %d \n", u4channel[0], u4Sample_times );
    printf("BAT_GetVoltage : channel_1  = %d / %d \n", u4channel[1], u4Sample_times );
    printf("BAT_GetVoltage : channel_2  = %d / %d \n", u4channel[2], u4Sample_times );
    printf("BAT_GetVoltage : channel_3  = %d / %d \n", u4channel[3], u4Sample_times );
    printf("BAT_GetVoltage : channel_4  = %d / %d \n", u4channel[4], u4Sample_times );
    printf("BAT_GetVoltage : channel_5  = %d / %d \n", u4channel[5], u4Sample_times );
    printf("BAT_GetVoltage : channel_6  = %d / %d \n", u4channel[6], u4Sample_times );
    printf("BAT_GetVoltage : channel_7  = %d / %d \n", u4channel[7], u4Sample_times );
    printf("BAT_GetVoltage : channel_8  = %d / %d \n", u4channel[8], u4Sample_times );
    printf("BAT_GetVoltage : channel_9  = %d / %d \n", u4channel[9], u4Sample_times );
    printf("BAT_GetVoltage : channel_10 = %d / %d \n", u4channel[10], u4Sample_times );
    printf("BAT_GetVoltage : channel_11 = %d / %d \n", u4channel[11], u4Sample_times );
    printf("BAT_GetVoltage : channel_12 = %d / %d \n", u4channel[12], u4Sample_times );
    printf("BAT_GetVoltage : channel_13 = %d / %d \n", u4channel[13], u4Sample_times );	
    #endif

	/* Value averaging  */ 
    u4channel[0]  = u4channel[0]  / deCount;
    u4channel[1]  = u4channel[1]  / deCount;
    u4channel[2]  = u4channel[2]  / deCount;
    u4channel[3]  = u4channel[3]  / deCount;
    u4channel[4]  = u4channel[4]  / deCount;
    u4channel[5]  = u4channel[5]  / deCount;
    u4channel[6]  = u4channel[6]  / deCount;
    u4channel[7]  = u4channel[7]  / deCount;
    u4channel[8]  = u4channel[8]  / deCount;
    u4channel[9]  = u4channel[9]  / deCount;
    u4channel[10] = u4channel[10] / deCount;
    u4channel[11] = u4channel[11] / deCount;
    u4channel[12] = u4channel[12] / deCount;
    u4channel[13] = u4channel[13] / deCount;

//	adc_temp_result = ((u4channel[dwChannel]*VOLTAGE_FULL_RANGE)/ADC_PRECISE);
	if(g_EFUSE_Cali == KAL_FALSE) {
	adc_temp_result = ((u4channel[dwChannel]*VOLTAGE_FULL_RANGE)/ADC_PRECISE);
	adc_temp_result = adc_temp_result * 2;
	if(dwChannel!=AUXADC_CHARGER_VOLTAGE_CHANNEL) {
		adc_result = ((adc_temp_result * (*(auxadc_cali_slop_def+ dwChannel))) + (*(auxadc_cali_offset_def+ dwChannel))) / 1000;
	} else {
		adc_result = adc_temp_result;
		}
	adc_result = adc_result/2;
} else {
	adc_temp_result = u4channel[dwChannel];
		adc_result = ((adc_temp_result * (*(auxadc_cali_slop_efuse+ dwChannel))) + (*(auxadc_cali_offset_efuse+ dwChannel))) / 1000;
	}

	return adc_result;
	
}

///////////////////////////////////////////////////////////////////////////////////////////

