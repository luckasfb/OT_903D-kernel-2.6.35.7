

#ifndef __MT6573_PLL__
#define __MT6573_PLL__

#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/module.h>
#include <asm/tcm.h>

#include "mach/mt6573_reg_base.h"
#include "mach/sync_write.h"
#include "mach/mt6573_typedefs.h"

#include "pmu6573_sw.h"
#include "upmu_sw.h"

#define IDLE_LOW_POWER_TEST

#define PDN_ID_MAX  128

#define CHIP_VER_E1     0x8A00
#define CHIP_VER_E2     0xCA10
#define CLK_ID_MAX      9
#define CLK_ID_DSP      0
#define CLK_ID_AHB      1
#define CLK_ID_USB      2
#define CLK_ID_IRDA     3
#define CLK_ID_CAM      4
#define CLK_ID_TV       5
#define CLK_ID_CEVA     6
#define CLK_ID_MCARD    7
#define CLK_ID_MIPITX   8

#define MAX_DEVICE      5
#define MAX_MOD_NAME    32
#define MAX_PLL_COUNT	4

#define SPARE_SECRET_KEY        0x7300
#define SPARE_E1_PATCH          (0x1<<0)
#define SPARE_DVFS_EN           (0x1<<1)
#define SPARE_VAPROC_ADJUST_EN  (0x1<<2)
#define SPARE_DVFS_LOG          (0x1<<3)
#define SPARE_DCM_EN            (0x1<<4)

#define DBG_PMAPI_NONE            0x00000000    
#define DBG_PMAPI_CG            0x00000001    
#define DBG_PMAPI_PLL            0x00000002    
#define DBG_PMAPI_SUB            0x00000004    
#define DBG_PMAPI_PMIC           0x00000008  
#define DBG_PMAPI_DCM            0x00000010
#define DBG_PMAPI_CG_LIS            0x00000020    
#define DBG_PMAPI_ALL            0xFFFFFFFF    
#define ALL_STATE    MT6573_SYS_STATE_COUNT

#define NON_OP "NOP"

typedef enum MT6573_SYS_STATE
{
    NORMAL_STATE,//most consuming state
    IDLE_STATE,
    LITTLE_IDLE_STATE,
    SLOW_IDLE_STATE,
    DEEP_IDLE_STATE,//lowest consuming state
    MT6573_SYS_STATE_COUNT
}MT6573_STATE;
typedef enum {
        CLOCK_LISTEN_LOW = 1,
        CLOCK_LISTEN_MEDIUM= 2,
        CLOCK_LISTEN_HIGH = 3,
}CLOCK_LISTENER_LEVEL;

typedef enum MT65XX_CLK_EN_TYPE
{
    MT65XX_MSDC_CLK_EN = 0,
    MT65XX_USB_CLK_EN = 1,
    MT65XX_AUDIO_CLK_EN = 2,
    MT65XX_CLK_EN_COUNT_END = 3
}CLK_EN_TYPE;

typedef enum MT65XX_CLOCK_CATEGORY_TAG{
	CLOCK_PERI_PDN0,
	CLOCK_PERI_PDN1,
	CLOCK_MMSYS1_PART0 , 
	CLOCK_MMSYS1_PART1, 
	CLOCK_MMSYS2_PART0, 
	CLOCK_AUDIOSYS,
	MT65XX_CLOCK_CATEGORY_COUNT
}MT65XX_CLOCK_CATEGORY;

typedef enum MT65XX_CLOCK_TYPE_TAG{
	FWMA_CK,
	FMSDC_CK,
	FSYS_CK,
	FUSB_CK,
	FEMI_CK,
	FMIPI_CK,
	FCAM_CK,
	FTV_CK,
	F3D_CK,
	FAPMCU_CK,
	FMDMCU_CK,
	FDSP_CK,
	FGSM_CK,
	FFG_CK,
	MT65XX_CLOCK_TYPE_COUNT		
}MT65XX_CLOCK_TYPE;

typedef enum MT65XX_CLOCK_TAG {
    /*APMCU_CG_CON0 */
    MT65XX_PDN_PERI_MSDC    =    0,
    MT65XX_PDN_PERI_MSDC2   =    1,
    MT65XX_PDN_PERI_MSDC3    =    2,
    MT65XX_PDN_PERI_MSDC4    =    3,
    MT65XX_PDN_PERI_ADC2G    =    4,
    MT65XX_PDN_PERI_TP        =    5,
    MT65XX_PDN_PERI_2GTX    =    6,
    MT65XX_PDN_PERI_USB    =    7,
    MT65XX_PDN_PERI_USB11    =    8,
    MT65XX_PDN_PERI_UART1    =    9,
    MT65XX_PDN_PERI_UART2    =    10,
    MT65XX_PDN_PERI_UART3    =    11,
    MT65XX_PDN_PERI_UART4    =    12,
    MT65XX_PDN_PERI_ADC3G    =    13,

	/*APMCU_CG_CON1 */
    MT65XX_PDN_PERI_OSTIMER_APARM    =    32,
    MT65XX_PDN_PERI_SIMIF0    =    33,    
    MT65XX_PDN_PERI_SIMIF1    =    34,
    MT65XX_PDN_PERI_IRDA    =    35,    
    MT65XX_PDN_PERI_APDMA    =    36,
    MT65XX_PDN_PERI_PWM    =    37,    
    MT65XX_PDN_PERI_MIXEDSYS    =    38,
    MT65XX_PDN_PERI_NFI    =    39,    
    MT65XX_PDN_PERI_I2C    =    40,
    MT65XX_PDN_PERI_I2C2    =    41,    
    MT65XX_PDN_PERI_SEJ    =    42,
    MT65XX_PDN_PERI_ONEWIRE    =    43,    
    MT65XX_PDN_PERI_TRNG    =    44,

    /*MMSYS1 Clock Gating #1 */
    MT65XX_PDN_MM_GMC1			   =    64,  
    MT65XX_PDN_MM_G2D			  =    65,   	
    MT65XX_PDN_MM_GCMQ			   =    66,  
    MT65XX_PDN_MM_JPGENC		  =    67,   	
    MT65XX_PDN_MM_IDP_MOUT0		  =    68,   
    MT65XX_PDN_MM_JPGDEC		  =    69,   	
    MT65XX_PDN_MM_JPEG_DMA		  =    70,   
    MT65XX_PDN_MM_IDP_MOUT1		  =    71,   
    MT65XX_PDN_MM_OVL_DMA		  =    72,   	             
    MT65XX_PDN_MM_ROT_DMA2		  =    73,                   
    MT65XX_PDN_MM_ISP		      =    74,                   
    MT65XX_PDN_MM_IPP		      =    75,                   
    MT65XX_PDN_MM_PRZ0	          =    76,                   
    MT65XX_PDN_MM_CRZ		      =    77,                   
    MT65XX_PDN_MM_VRZ		      =    78,                   
    MT65XX_PDN_MM_EIS		      =    79,                   
    MT65XX_PDN_MM_GIF		     =    80,                    
    MT65XX_PDN_MM_R_DMA0	      =    81,                   
    MT65XX_PDN_MM_R_DMA1	      =    82,                   
    MT65XX_PDN_MM_PNG		      =    83,                   
    MT65XX_PDN_MM_ROT_DMA0		  =    84,                   
    MT65XX_PDN_MM_ROT_DMA1		  =    85,                   
    MT65XX_PDN_MM_RESZ_LB	     =    86,                    
    MT65XX_PDN_MM_LCD		      =    87,                   
    MT65XX_PDN_MM_DPI		      =    88,                   
    MT65XX_PDN_MM_CSI2	         =    89,                    
    MT65XX_PDN_MM_OVL_DMA_BPS	=   90,                     
    MT65XX_PDN_MM_FDVT		    =   91,                     
    MT65XX_PDN_MM_DSI		    =   92,                      
    MT65XX_PDN_MM_R_DMA1_BPS	=   93, 	                 
    MT65XX_PDN_MM_MMLMU	        =   94,                      
    MT65XX_PDN_MM_BRZ		    =   95,  
    
    /*MMSYS1 Clock Gating #2 */
    MT65XX_PDN_MM_PRZ1		    =    96,         
    MT65XX_PDN_MM_ROT_DMA3	   =    97,          
    MT65XX_PDN_MM_IDP_MOUT2	    =    98,         
    MT65XX_PDN_MM_IDP_MOUT3	   =    99,          
    MT65XX_PDN_MM_TVC		    =    100,        
    MT65XX_PDN_MM_TVE		   =    101,         
    MT65XX_PDN_MM_TV_ROT	    =    102,        
    MT65XX_PDN_MM_HIF		   =    103,         
    MT65XX_PDN_MM_GMC1E        =    104,        
    MT65XX_PDN_MM_GMC1SLV      =    105,         


	/*MMSYS2 Clock Gating */
    MT65XX_PDN_MM_GMC2		   =    128, 
    MT65XX_PDN_MM_MFV		   =    129, 	
    MT65XX_PDN_MM_MFG_CORE	   =    130, 
    MT65XX_PDN_MM_MFG_MEM	   =    131, 	
    MT65XX_PDN_MM_MFG_SYS	   =    132, 	
    MT65XX_PDN_MM_SPI	       =    133, 		
    MT65XX_PDN_MM_GMC2_E	   =    134,

	/*AUDIOSYS*/
	MT65XX_PDN_AUDIO_AFE		= 	162,
	MT65XX_PDN_AUDIO_SIGMADSP	= 	164,
	MT65XX_PDN_AUDIO_ADC		= 	165,
	MT65XX_PDN_AUDIO_I2S		= 	166,

	/*Alias*/
	MT65XX_CLOCK_COUNT_BEGIN =	MT65XX_PDN_PERI_MSDC,

	MT65XX_PDN_PERI0_BEGIN		= 	MT65XX_PDN_PERI_MSDC,
	MT65XX_PDN_PERI0_END		= 	MT65XX_PDN_PERI_ADC3G,

	MT65XX_PDN_PERI1_BEGIN		= 	MT65XX_PDN_PERI_OSTIMER_APARM,
	MT65XX_PDN_PERI1_END		= 	MT65XX_PDN_PERI_TRNG,

	MT65XX_PDN_MM1_PART0_BEGIN		= 	MT65XX_PDN_MM_GMC1,
	MT65XX_PDN_MM1_PART0_END		= 	MT65XX_PDN_MM_BRZ,

	MT65XX_PDN_MM1_PART1_BEGIN		= 	MT65XX_PDN_MM_PRZ1,
	MT65XX_PDN_MM1_PART1_END		= 	MT65XX_PDN_MM_GMC1SLV,

	MT65XX_PDN_MM2_BEGIN		= 	MT65XX_PDN_MM_GMC2,
	MT65XX_PDN_MM2_END		= 	MT65XX_PDN_MM_GMC2_E,

	MT65XX_PDN_AUDIO_BEGIN		= 	160,
	MT65XX_PDN_AUDIO_END		= 	MT65XX_PDN_AUDIO_I2S,

    MT65XX_CLOCK_COUNT_END,
           
} MT65XX_CLOCK;
                                                                                                                                                                                                                                                                                                                                                                                        

typedef enum MT65XX_POWER_VOL_TAG 
{
    VOL_DEFAULT,
    VOL_0800 = UPMU_VOLT_0_8_0_0_V,     
    VOL_0850 = UPMU_VOLT_0_8_5_0_V,     
    VOL_0900 = UPMU_VOLT_0_9_0_0_V,     
    VOL_0950 = UPMU_VOLT_0_9_5_0_V,     
    VOL_1000 = UPMU_VOLT_1_0_0_0_V,     
    VOL_1050 = UPMU_VOLT_1_0_5_0_V,     
    VOL_1100 = UPMU_VOLT_1_1_0_0_V,     
    VOL_1150 = UPMU_VOLT_1_1_5_0_V,     
    VOL_1200 = UPMU_VOLT_1_2_0_0_V,     
    VOL_1250 = UPMU_VOLT_1_2_5_0_V,             
    VOL_1300 = UPMU_VOLT_1_3_0_0_V,    
    VOL_1350 = UPMU_VOLT_1_3_5_0_V,     
    VOL_1400 = UPMU_VOLT_1_4_0_0_V,     
    VOL_1450 = UPMU_VOLT_1_4_5_0_V,     
    VOL_1500 = UPMU_VOLT_1_5_0_0_V,    
    VOL_1550 = UPMU_VOLT_1_5_5_0_V,     
    VOL_1800 = UPMU_VOLT_1_8_0_0_V,    
    VOL_2500 = UPMU_VOLT_2_5_0_0_V,    
    VOL_2800 = UPMU_VOLT_2_8_0_0_V, 
    VOL_3000 = UPMU_VOLT_3_0_0_0_V,
    VOL_3300 = UPMU_VOLT_3_3_0_0_V,        
} MT65XX_POWER_VOLTAGE;

typedef enum MT65XX_POWER_TAG {
	/* LDO 1*/
	MT65XX_POWER_LDO_VA28 = 0,
	MT65XX_POWER_LDO_VA25,	
	MT65XX_POWER_LDO_VA12,
	MT65XX_POWER_LDO_VRTC,
	MT65XX_POWER_LDO_VMIC,
	MT65XX_POWER_LDO_VTV,
	MT65XX_POWER_LDO_VAUDN,
	MT65XX_POWER_LDO_VAUDP,
	MT65XX_POWER_LDO_PMUA,
	MT65XX_POWER_LDO_VRF,
	MT65XX_POWER_LDO_VCAMA,
	MT65XX_POWER_LDO_VCAMD,
	MT65XX_POWER_LDO_VIO,
	MT65XX_POWER_LDO_VUSB,
	MT65XX_POWER_LDO_VSIM,
	MT65XX_POWER_LDO_VSIM2,
	MT65XX_POWER_LDO_VIBR,
	MT65XX_POWER_LDO_VMC,
	MT65XX_POWER_LDO_VCAMA2,
	MT65XX_POWER_LDO_VCAMD2,
	MT65XX_POWER_LDO_VM12,
	MT65XX_POWER_LDO_VM12_INT,
    MT65XX_POWER_COUNT_END,
    MT65XX_POWER_NONE = -1
} MT65XX_POWER;

typedef enum MT65XX_PLL_TAG {
    MT65XX_APMCU_PLL,
    MT65XX_MDMCU_PLL,
    MT65XX_DSP_PLL,
    MT65XX_EMI_PLL,
    MT65XX_3G_PLL,
    MT65XX_2G_PLL,
    MT65XX_CAMERA_PLL,
    MT65XX_3D_PLL,
    MT65XX_TV_PLL,
    MT65XX_FG_PLL,
    MT65XX_AUX_PLL,	
    MT65XX_MIPI_PLL,
    MT65XX_26M_PLL,
    MT65XX_PLL_COUNT_END,
} MT65XX_PLL;

typedef enum MT65XX_SUBSYS_TAG {
    MD2G_SUBSYS,
    HSPA_SUBSYS,
    FC4_SUBSYS, 
    AUDIO_SUBSYS,
    MM1_SUBSYS,
    MM2_SUBSYS,
    MD_SUBSYS, 
    AP_SUBSYS,
    MT65XX_SUBSYS_COUNT_END,
} MT65XX_SUBSYS;

struct clock_listener {
        struct list_head link;
        CLOCK_LISTENER_LEVEL level;
        char name[MAX_MOD_NAME];
        void (*gate)(struct clock_listener *h);
        void (*ungate)(struct clock_listener *h);
};

typedef struct {
    DWORD dwClkEnCount;
    DWORD reg;
    DWORD mask;
    char mod_name[MAX_DEVICE][MAX_MOD_NAME];
    char name[MAX_MOD_NAME];
    BOOL showlog;
}DEVICE_CLK_EN;

typedef struct { 
    DWORD dwClockCount; 
    DWORD clock_type;
    INT32 active_pll_source;
    BOOL bDefault_on;
    char name[MAX_MOD_NAME];        
    char mod_name[MAX_DEVICE][MAX_MOD_NAME];        
    BOOL showlog;
} DEVICE_POWERID;

typedef struct { 
    DWORD dwPllCount; 
    BOOL bDefault_on;
    char name[MAX_MOD_NAME];        
    char mod_name[MAX_DEVICE][MAX_MOD_NAME];        
} DEVICE_PLLID;

typedef struct { 
    DWORD dwPowerCount; 
    BOOL bDefault_on;
    char name[MAX_MOD_NAME];        
    char mod_name[MAX_DEVICE][MAX_MOD_NAME];    
} DEVICE_POWER;


typedef struct
{    
    DEVICE_POWERID Clock[MT65XX_CLOCK_CATEGORY_COUNT][32];//clock source managed by clock manager
    DEVICE_POWER Power[MT65XX_POWER_COUNT_END];
    DEVICE_PLLID Pll[MT65XX_PLL_COUNT_END];
    DWORD dwSubSystem_status;
    DWORD dwSubSystem_defaultOn;    
    DWORD dwSubSystem_clock[MT65XX_CLOCK_CATEGORY_COUNT];//the current clock gating status of sub system, if no clock is enabled then 0, add by Cui Zhang
    DWORD dwClock_active_pll[MT65XX_CLOCK_TYPE_COUNT];//the current pll source of every cock type
    DWORD dwSubSystem_showlog; 
} ROOTBUS_HW;

typedef struct CM_DCM_CFG_STRUCT
{
    DWORD aparm_fsel;
    DWORD mdarm_fsel;
    DWORD emi_fsel;
    DWORD emi_idle_fsel;
    DWORD fbus_fsel;
    DWORD fbus_idle_fsel;
    DWORD sbus_fsel;
    DWORD sbus_idle_fsel;
    DWORD dsp_fsel;
    DWORD dsp_idle_fsel;
    DWORD rg_emi_dbc;
    DWORD rg_bus_dbc;
} CM_DCM_CFG_T;


#define MPLL_LOCKED                    0x20
#define DPLL_LOCKED                    0x20
#define WPLL_LOCKED                    0x400
#define GPLL_LOCKED                    0x8000
#define EPLL_LOCKED                    0x20
#define AMPLL_LOCKED                   0x20
#define CPLL_LOCKED                    0x8000
#define THREEDPLL_LOCKED               0x8000
#define TVPLL_LOCKED                   0x8000
#define FGPLL_LOCKED                   0x8000
#define AUXPLL_LOCKED                  0x8000
	
#define RG_OVRD_SYS_CLK                0x2000

#define APMCU_F1_TM_MHZ               (0x7F)    // AMPLL = 806 MHz
#define APMCU_F1_MHZ                  (0x75)    // AMPLL = 676 MHz
#define APMCU_F2_MHZ                  (0x6D)    // AMPLL = 572 MHz
#define APMCU_F3_MHZ                  (0x69)    // AMPLL = 520 MHz
#define APMCU_F4_MHZ                  (0x61)    // AMPLL = 416 MHz
#define APMCU_F5_MHZ                  (0x5C)    // AMPLL = 351 MHz
#define APMCU_F6_MHZ                  (0x52)    // AMPLL = 286 MHz
#define APMCU_F7_MHZ                  (0x44)    // AMPLL = 208 MHz
#define APMCU_F8_MHZ                  (0x3B)    // AMPLL = 169 MHz

//test for FM desense
#define AMPLL_FSEL_663M  0x74
#define AMPLL_FSEL_676M  0x75 //default value
#define AMPLL_FSEL_689M  0x76
#define AMPLL_FSEL_DEFAULT AMPLL_FSEL_676M
#define AMPLL_FSEL_DEFAULT_LESS AMPLL_FSEL_663M
#define AMPLL_FSEL_DEFAULT_MORE AMPLL_FSEL_689M

#define MPLL_FSEL_507M  0x68
#define MPLL_FSEL_520M  0x69 //default value
#define MPLL_FSEL_533M  0x6a
#define MPLL_FSEL_DEFAULT MPLL_FSEL_520M
#define MPLL_FSEL_DEFAULT_LESS MPLL_FSEL_507M
#define MPLL_FSEL_DEFAULT_MORE MPLL_FSEL_533M

#define DPLL_FSEL_273  0x33
#define DPLL_FSEL_279_5  0x34 //default value
#define DPLL_FSEL_286  0x35
#define DPLL_FSEL_DEFAULT DPLL_FSEL_279_5
#define DPLL_FSEL_DEFAULT_LESS DPLL_FSEL_273
#define DPLL_FSEL_DEFAULT_MORE DPLL_FSEL_286

#define EPLL_FSEL_377  0x6e
#define EPLL_FSEL_390  0x6f //default value
#define EPLL_FSEL_403  0x70
#define EPLL_FSEL_DEFAULT EPLL_FSEL_390
#define EPLL_FSEL_DEFAULT_LESS EPLL_FSEL_377
#define EPLL_FSEL_DEFAULT_MORE EPLL_FSEL_403

#define CPLL_FSEL_139_8 0x25
#define CPLL_FSEL_143  0x26 //default value
#define CPLL_FSEL_146_3  0x27
#define CPLL_FSEL_DEFAULT CPLL_FSEL_143
#define CPLL_FSEL_DEFAULT_LESS CPLL_FSEL_139_8
#define CPLL_FSEL_DEFAULT_MORE CPLL_FSEL_146_3

#define TRHEED_FSEL_191_8  0x35
#define THREED_FSEL_195  0x36 //default value
#define THREED_FSEL_198_3  0x37
#define THREED_FSEL_DEFAULT THREED_FSEL_195
#define THREED_FSEL_DEFAULT_LESS TRHEED_FSEL_191_8
#define THREED_FSEL_DEFAULT_MORE THREED_FSEL_198_3

#define AUX_FSEL_104  0x03 //default value
#define AUX_FSEL_124_8  0x02
#define AUX_FSEL_DEFAULT AUX_FSEL_104	
#define AUX_FSEL_DEFAULT_MORE AUX_FSEL_124_8


#define PWR_RST_B		0
#define PWR_ISO			1
#define PWR_ON			2
#define PWR_MEM_OFF		3
#define PWR_CLK_DIS		4
#define PWR_REQ_EN		6
#define PWR_CTRL		7
#define PWRON_SETTLE_SHIFT	8

/* MDSYS */
#define XOSC_CON                       (MIXEDSYS0_BASE+0x0)
#define CLKSQ_CON                      (MIXEDSYS0_BASE+0x80)
#define PLL_CON0_REG                   (MIXEDSYS0_BASE+0x100)
#define PLL_CON1_REG                   (MIXEDSYS0_BASE+0x104)
#define PLL_CON2_REG                   (MIXEDSYS0_BASE+0x108)
#define PLL_CON4_REG                   (MIXEDSYS0_BASE+0x110)
#define PLL_CON5_REG                   (MIXEDSYS0_BASE+0x114)
#define PLL_CON6_REG                   (MIXEDSYS0_BASE+0x118)
#define CLKSW_PLLDIV_CON0              (MIXEDSYS0_BASE+0x11C)
#define CLKSW_PLLDIV_CON1              (MIXEDSYS0_BASE+0x120)
#define CLKSW_PLLDIV_CON3              (MIXEDSYS0_BASE+0x128)
#define CLKSW_PLLCNTEN_CON             (MIXEDSYS0_BASE+0x12C)
#define MPLL_CON0_REG                  (MIXEDSYS0_BASE+0x140)   
#define MPLL_CON1_REG                  (MIXEDSYS0_BASE+0x144)   
#define MPLL_CON3_REG                  (MIXEDSYS0_BASE+0x14C)   
#define AMPLL_CON0_REG                 (MIXEDSYS0_BASE+0x160)   
#define AMPLL_CON1_REG                 (MIXEDSYS0_BASE+0x164)   
#define AMPLL_CON3_REG                 (MIXEDSYS0_BASE+0x16C)   
#define DPLL_CON0_REG                  (MIXEDSYS0_BASE+0x180)   
#define DPLL_CON1_REG                  (MIXEDSYS0_BASE+0x184)   
#define DPLL_CON3_REG                  (MIXEDSYS0_BASE+0x18C)   
#define EPLL_CON0_REG                  (MIXEDSYS0_BASE+0x1C0)   
#define EPLL_CON1_REG                  (MIXEDSYS0_BASE+0x1C4)   
#define EPLL_CON3_REG                  (MIXEDSYS0_BASE+0x1CC)   
#define CPLL_CON0_REG                  (MIXEDSYS0_BASE+0x200)   
#define WPLL_CON0_REG                  (MIXEDSYS0_BASE+0x240)   
#define GPLL_CON0_REG                  (MIXEDSYS0_BASE+0x280)   
#define THREEDPLL_CON0_REG             (MIXEDSYS0_BASE+0x2C0)   
#define TVPLL_CON0_REG                 (MIXEDSYS0_BASE+0x300)   
#define FGPLL_CON0_REG                 (MIXEDSYS0_BASE+0x340)   
#define AUXPLL_CON0_REG                (MIXEDSYS0_BASE+0x380)   

/* APCONFIG*/
#define APHW_VER                        (APCONFIG_BASE+0x0000)
#define APSW_VER                        (APCONFIG_BASE+0x0004)
#define APHW_CODE                       (APCONFIG_BASE+0x0008)
#define CHIP_STA                        (APCONFIG_BASE+0x000C)
#define APSYS_RST                       (APCONFIG_BASE+0x0010)
#define APP_MEM_PD                      (APCONFIG_BASE+0x0020)
#define MM1_MEM_PD0                     (APCONFIG_BASE+0x0024)
#define MM1_MEM_PD1                     (APCONFIG_BASE+0x0028)
#define MM2_MEM_PD                      (APCONFIG_BASE+0x002C)
#define APP_MEM_DELSEL                  (APCONFIG_BASE+0x0034)
#define AP_MEM_DELSEL                   (APCONFIG_BASE+0x0038)
#define AP_MEM_PD                       (APCONFIG_BASE+0x003C)
#define APP_CON                         (APCONFIG_BASE+0x0040)
#define BUS_GATING_EN                   (APCONFIG_BASE+0x0050)
#define TOPSM_DMY0 						(APCONFIG_BASE+0x0060)
#define TOPSM_DMY1 						(APCONFIG_BASE+0x0064)
#define AP_SAPD_WAYEN 					(APCONFIG_BASE+0x00B0)
#define AP_SLPPRT_BUS                   (APCONFIG_BASE+0x00C0)
#define APPER_SLP_CON                   (APCONFIG_BASE+0x00C8)
#define APARM_FSEL                      (APCONFIG_BASE+0x0100)
#define DSP_FSEL                        (APCONFIG_BASE+0x0104)
#define EMI_FSEL                        (APCONFIG_BASE+0x0108)
#define FBUS_FSEL                       (APCONFIG_BASE+0x010C)
#define SBUS_FSEL                       (APCONFIG_BASE+0x0110)
#define DSP_IDLE_FSEL                   (APCONFIG_BASE+0x0114)
#define EMI_IDLE_FSEL                   (APCONFIG_BASE+0x0118)
#define FBUS_IDLE_FSEL                  (APCONFIG_BASE+0x011C)
#define SBUS_IDLE_FSEL                  (APCONFIG_BASE+0x0120)
#define RG_CK_ALW_ON                    (APCONFIG_BASE+0x0124)
#define RG_CK_DCM_EN                    (APCONFIG_BASE+0x0128)
#define RG_EMI_DBC                      (APCONFIG_BASE+0x012C)
#define RG_BUS_DBC                      (APCONFIG_BASE+0x0130)
#define MSDC_CCTL                       (APCONFIG_BASE+0x0140)
#define MSDC2_CCTL                      (APCONFIG_BASE+0x0144)
#define MSDC3_CCTL                      (APCONFIG_BASE+0x0148)
#define MSDC4_CCTL                      (APCONFIG_BASE+0x014C)
#define GC_CLK_MON0                     (APCONFIG_BASE+0x0160)
#define GC_CLK_MON1                     (APCONFIG_BASE+0x0164)
#define GC_CLK_MON2                     (APCONFIG_BASE+0x0168)
#define APMCU_CG_CON0 					(APCONFIG_BASE+0x0300)
#define APMCU_CG_SET0 					(APCONFIG_BASE+0x0304)
#define APMCU_CG_CLR0 					(APCONFIG_BASE+0x0308)
#define APMCU_CG_CON1 					(APCONFIG_BASE+0x0310)
#define APMCU_CG_SET1 					(APCONFIG_BASE+0x0314)
#define APMCU_CG_CLR1 					(APCONFIG_BASE+0x0318)

/* MTCMOS */
#define MMSYS1_CG_CON0					(MMSYS1_CONFIG_BASE+0x300)
#define MMSYS1_CG_CON1					(MMSYS1_CONFIG_BASE+0x304)
#define MMSYS1_CG_SET0					(MMSYS1_CONFIG_BASE+0x320)
#define MMSYS1_CG_SET1					(MMSYS1_CONFIG_BASE+0x324)
#define MMSYS1_CG_CLR0					(MMSYS1_CONFIG_BASE+0x340)
#define MMSYS1_CG_CLR1					(MMSYS1_CONFIG_BASE+0x344)

#define MMSYS2_CG_CON0					(MMSYS2_CONFIG_BASE+0x300)
#define MMSYS2_CG_SET0					(MMSYS2_CONFIG_BASE+0x320)
#define MMSYS2_CG_CLR0					(MMSYS2_CONFIG_BASE+0x340)

#define AUDIO_TOP_CON0_REG      (AFE_BASE + AUDIO_TOP_CON0)

//pllsel bits
#define APMCU_PLLSEL 0x7000
#define MDMCU_PLLSEL 0x3000
#define DSP_PLLSEL 0x0700
#define EMI_PLLSEL 0x0070
#define WMA_PLLSEL 0x0f00
#define MSDC_PLLSEL 0x000f
#define GSM_PLLSEL 0x0030
#define CAM_PLLSEL 0x000f
#define THREED_PLLSEL 0x0300
#define TV_PLLSEL 0x000c
#define FG_PLLSEL 0x00c0
#define SYS_PLLSEL 0x0030
#define USB_PLLSEL 0x0003

//pll
void PLL_Set(DWORD pllId);
void PLL_Clear(DWORD pllId);
BOOL PLL_Status(DWORD pllId);
void hwSetClkEn(DWORD clk_en_type, BOOL enable, char* master);
bool hwEnablePLL(MT65XX_PLL PllId, char *mode_name);
bool hwDisablePLL(MT65XX_PLL PllId, char *mode_name);
INT32 isActivePllSource(DWORD pllId, INT32 category, INT32 offset);
DWORD getPllSourceOfClock(DWORD catagory, UINT32 clock_mode);
BOOL isPllCanDisable(DWORD pllId);
BOOL isPllCanBeFsel(MT65XX_PLL pllId);
void PLL_Fsel(MT65XX_PLL pllId, UINT16 pll_fsel_value);
INT16 PLL_Fsel_Read(MT65XX_PLL pllId);
void hwDoPLLTest(DWORD pllId, BOOL enable, DWORD time);
void hwSwitchPLL(DWORD clock_type, DWORD pll_mode);
BOOL DRV_SetRegWithMask(UINT32 pDest, DWORD mask, UINT32 value);
//sub system
bool hwEnableSubsys(MT65XX_SUBSYS subsysId);
bool hwDisableSubsys(MT65XX_SUBSYS subsysId);
void hwDoSubSysTest(DWORD subSys, BOOL enable, DWORD time);
BOOL isSubsysCanDisable(DWORD sysId);
BOOL set_subsys_showlog(INT32 subsys, BOOL enable);
//clock
BOOL isPllCanBeSwitched(DWORD clock_type, DWORD pllId);
void CG_Clear (DWORD catagory, UINT32 mode);
void CG_Set (DWORD catagory, UINT32 mode);
BOOL CG_Status (DWORD catagory, UINT32 mode);
void hwSetPllCon_clock(INT32 category, INT32 offset, BOOL enable);
BOOL hwSetPllCon_master(DWORD clk_en_type, BOOL enable, char* master);
bool hwEnableClock(MT65XX_CLOCK clockId, char *mode_name);
bool hwEnableClock_withPll(MT65XX_CLOCK clockId, char *mode_name, DWORD pllId);
bool hwEnableClock_func(INT32 category, INT32 offset, char *mode_name, DWORD pll_mode);
bool hwDisableClock(MT65XX_CLOCK clockId, char *mode_name);
bool hwDisableClock_func(INT32 category, INT32 offset, char *mode_name);
void hwDoClockTest(DWORD category, BOOL enableFirst);
void hwClockTestSlave(DWORD category, DWORD offset, BOOL enableFirst, DWORD pll_mode);
void getClock_Name(DWORD clock_catagory,DWORD clock_mode,char * clock_name);
BOOL hwSetClockType(MT65XX_CLOCK clockId, MT65XX_CLOCK_TYPE clockType, char *mode_name);
void register_clock_listener(struct clock_listener *listener);
void unregister_clock_listener(struct clock_listener *listener);
void set_clock_listen(BOOL enable);
void show_clock_listen(void);
BOOL set_clock_showlog(INT32 clockId, BOOL enable);
BOOL set_clkEn_showlog(INT32 clk_en_type, BOOL enable);
BOOL get_device_working_ability(UINT32 clockId, MT6573_STATE state);
void set_device_working_ability(UINT32 clockId, MT6573_STATE state);
void clr_device_working_ability(UINT32 clockId, MT6573_STATE state);
//power
BOOL POWER_Status(DWORD powerId);
bool hwPowerOn(MT65XX_POWER powerId, MT65XX_POWER_VOLTAGE powervol, char *mode_name);
bool hwPowerDown(MT65XX_POWER powerId, char *mode_name);
//Thermal
void hwThermalProtectInit(void);
//DCM
void MT6573_INIT_IDLE_THREAD(void);
void MT6573_Enable_HW_DCM_AP(void);
void MT6573_Disable_HW_DCM_AP(void);
void dcm_enable_state(MT6573_STATE state);
void dcm_disable_state(MT6573_STATE state);
//MTCMOS
void MTCMOS_En(UINT32 subsys);
void MTCMOS_En_reboot(void);
// PM log
void mt6573_log_init(void);
//DVFS
void mt6573_pll_Configure_All(void);
void mt6573_pll_Configure_APMCU_F1(void);
void mt6573_pll_Configure_APMCU_F2(void);
void mt6573_pll_Configure_APMCU_F3(void);
void mt6573_pll_Configure_APMCU_F4(void);
void mt6573_pll_Configure_APMCU_F5(void);
void mt6573_pll_Configure_APMCU_F6(void);
void mt6573_pll_Configure_APMCU_F7(void);
void mt6573_pll_Configure_APMCU_F8(void);
//Init
void mt6573_power_management_init(void);


extern UINT32 gChipVer ; 
extern UINT32 CM_DBG_FLAG ;

extern ROOTBUS_HW g_MT6573_BusHW ;
extern DWORD bStateMask ;
extern BOOL bCanEnDVFS ; 
extern BOOL bBUCK_ADJUST ; 
extern BOOL bEnDVFSLog ;
extern BOOL finish_log_init ;

extern char MT65XX_Peri0_name[MT65XX_PDN_PERI0_END-MT65XX_PDN_PERI0_BEGIN+1][MAX_MOD_NAME];
extern char MT65XX_Peri1_name[MT65XX_PDN_PERI1_END-MT65XX_PDN_PERI1_BEGIN+1][MAX_MOD_NAME];
extern char MT65XX_MM1_p0_name[MT65XX_PDN_MM1_PART0_END-MT65XX_PDN_MM1_PART0_BEGIN+1][MAX_MOD_NAME];
extern char MT65XX_MM1_p1_name[MT65XX_PDN_MM1_PART1_END-MT65XX_PDN_MM1_PART1_BEGIN+1][MAX_MOD_NAME];
extern char MT65XX_MM2_name[MT65XX_PDN_MM2_END-MT65XX_PDN_MM2_BEGIN+1][MAX_MOD_NAME];
extern char MT65XX_AUDIO_name[MT65XX_PDN_AUDIO_END-MT65XX_PDN_AUDIO_BEGIN+1][MAX_MOD_NAME];
extern DWORD MT65XX_PLL_LOCK_REG[MT65XX_PLL_COUNT_END] ; 
extern DWORD MT65XX_PLL_LOCK[MT65XX_PLL_COUNT_END] ; 
extern char MT65XX_PLL_name[MT65XX_PLL_COUNT_END+1][MAX_MOD_NAME];
extern char MT65XX_SUBSYS_name[MT65XX_SUBSYS_COUNT_END+1][MAX_MOD_NAME];
extern DWORD MT65XX_CG_CON[MT65XX_CLOCK_CATEGORY_COUNT];
extern DWORD MT65XX_CLOCK_NUMBER[MT65XX_CLOCK_CATEGORY_COUNT];
extern DWORD MT65XX_CG_CLR[MT65XX_CLOCK_CATEGORY_COUNT];
extern DWORD MT65XX_CG_SET[MT65XX_CLOCK_CATEGORY_COUNT];
extern DWORD MT65XX_PLL_REG[MT65XX_PLL_COUNT_END];
extern DWORD MT65XX_POWER_REG[MT65XX_POWER_COUNT_END] ;
extern MT65XX_PLL MT65XX_CLOCK_TYPE_VALUE[MT65XX_CLOCK_TYPE_COUNT][MAX_PLL_COUNT];
extern DWORD MT65XX_PLLSEL_REG[MT65XX_CLOCK_TYPE_COUNT][2];
extern char MT65XX_CLOCK_TYPE_name[MT65XX_CLOCK_TYPE_COUNT][MAX_MOD_NAME];
extern DWORD MT65XX_CLOCK_TYPE_OF_CK[MT65XX_CLOCK_TYPE_COUNT][MT65XX_CLOCK_CATEGORY_COUNT];
extern DWORD MT65XX_SUBSYS_OF_CATEGORY[MT65XX_CLOCK_CATEGORY_COUNT];
extern INT32 MT65XX__CATEGORY_OF_SUBSYS[MT65XX_SUBSYS_COUNT_END];
extern DEVICE_CLK_EN g_clken[MT65XX_CLK_EN_COUNT_END];
extern char MT65XX_CLK_EN_name[MT65XX_CLK_EN_COUNT_END][MAX_MOD_NAME];
extern DWORD MT65XX_CLK_EN_REG[MT65XX_CLK_EN_COUNT_END][2];
extern BOOL valid_clockId(int clockId);

#endif  





