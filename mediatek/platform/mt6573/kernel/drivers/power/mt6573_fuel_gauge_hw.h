


#ifndef __FUEL_GAUGE_6573_HW_H__
#define __FUEL_GAUGE_6573_HW_H__

#define FGADC_CON0    (0xF702EB80)
#define FGADC_CON1    (0xF702EB84)
#define FGADC_CON2    (0xF702EB88)
#define FGADC_CON3    (0xF702EB8C)
#define FGADC_CON4    (0xF702EB90)
#define FGADC_CON5    (0xF702EB94)
#define FGADC_CON6    (0xF702EB98)
#define FGADC_CON7    (0xF702EB9C)

#define PLL_CON6_BASE_ADDR		(0xF702E118)
#define FG_PLL_CON0_BASE_ADDR	(0xF702E340)

//#define UNIT_FGCURRENT 	(146718) 	// 146.718 uA
//#define UNIT_FGTIME 	(16) 		// 0.16s
//#define UNIT_FGCHARGE 	(6574581) 	// 0.006574581 uAh

#define UNIT_FGCURRENT 	(158122) 	// 158.122 uA
//#define UNIT_FGCURRENT 	(158935) 	// 158.935 uA
#define UNIT_FGTIME 	(16) 		// 0.16s
//#define UNIT_FGCHARGE 	(21961412) 	// 0.021961412 uAh //6329
#define UNIT_FGCHARGE 	(7085603) 	// 0.007085603 uAh //6573

//FGADC CON 0
#define FG_SW_RSTCLR_MASK        0x0001
#define FG_SW_RSTCLR_SHIFT       0

#define FG_SW_CLEAR_MASK        0x0002
#define FG_SW_CLEAR_SHIFT       1

#define FG_LATCHDATA_ST_MASK        0x0004
#define FG_LATCHDATA_ST_SHIFT       2

#define FG_SW_READ_PRE_MASK        0x0008
#define FG_SW_READ_PRE_SHIFT       3

#define FG_SW_CR_MASK        0x0010
#define FG_SW_CR_SHIFT       4

#define FG_TIME_RST_MASK        0x0020
#define FG_TIME_RST_SHIFT       5

#define FG_CHARGE_RST_MASK        0x0040
#define FG_CHARGE_RST_SHIFT       6

#define FG_INT_EN_MASK        0x0080
#define FG_INT_EN_SHIFT       7

#define FG_AUTOCALRATE_MASK        0x0300
#define FG_AUTOCALRATE_SHIFT       8

#define FG_CAL_MASK        0x0C00
#define FG_CAL_SHIFT       10

#define FG_ON_MASK        0x1000
#define FG_ON_SHIFT       12

//FGADC CON 1
#define FG_CAR_MASK        0xFFFF
#define FG_CAR_SHIFT       0

//FGADC CON 2
#define FG_NTER_MASK        0xFFFF
#define FG_NTER_SHIFT       0

//FGADC CON 3
#define FG_BLTR_MASK        0xFFFF
#define FG_BLTR_SHIFT       0

//FGADC CON 4
#define FG_BFTR_MASK        0xFFFF
#define FG_BFTR_SHIFT       0

//FGADC CON 5
#define FG_CURRENT_OUT_MASK        0xFFFF
#define FG_CURRENT_OUT_SHIFT       0

//FGADC CON 6
#define FG_ADJUST_OFFSET_VALUE_MASK        0xFFFF
#define FG_ADJUST_OFFSET_VALUE_SHIFT       0

//FGADC CON 7
#define FG_ISR_MASK        0x0003
#define FG_ISR_SHIFT       0

#define FG_ADC_RSTDETECT_MASK        0x0004
#define FG_ADC_RSTDETECT_SHIFT       2

#define FG_ADC_AUTORST_MASK        0x0008
#define FG_ADC_AUTORST_SHIFT       3

#define FG_DIG_TEST_MASK        0x0010
#define FG_DIG_TEST_SHIFT       4

#define FG_FIR2BYPASS_MASK        0x0020
#define FG_FIR2BYPASS_SHIFT       5

#define FG_FIR1BYPASS_MASK        0x0040
#define FG_FIR1BYPASS_SHIFT       6

#define FG_OSR_MASK        0x0380
#define FG_OSR_SHIFT       7

#endif // #ifndef __FUEL_GAUGE_6573_HW_H__

