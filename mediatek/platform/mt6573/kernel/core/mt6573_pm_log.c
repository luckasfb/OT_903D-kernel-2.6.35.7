


#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/tcm.h>
#include <asm/uaccess.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/mt6573_eint.h"
#include "mach/dma.h"
#include "mach/sync_write.h"
#include "mach/mt6573_pll.h"
#include "mach/mt6573_typedefs.h"
#include "mach/mtkpm.h"
#include "mach/mt6573_ost_sm.h"
#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"
#include "auddrv_register.h"

extern int wakelock_debug_mask ;
extern int Userwakelock_debug_mask ;
extern int Earlysuspend_debug_mask ;
extern int early_suspend_count; 
extern int forbid_id;
extern int console_suspend_enabled;

/* Debug message event */
#define DBG_PMAPI_MASK       (CM_DBG_FLAG)

#if 1
#define MSG(evt, fmt, args...) \
do {    \
    if ((DBG_PMAPI_##evt) & DBG_PMAPI_MASK) { \
        printk(fmt, ##args); \
    } \
} while(0)

#define MSG_FUNC_ENTRY(f)    MSG(ENTER, "<PMAPI FUNC>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...) do{}while(0)
#define MSG_FUNC_ENTRY(f)       do{}while(0)
#endif



static unsigned int u4SECRET_KEY = 0;

char MT65XX_PMU_name[MT65XX_POWER_COUNT_END+1][MAX_MOD_NAME]=
{
    "LDO_VA28",
    "LDO_VA25",	
    "LDO_VA12",
    "LDO_VRTC",
    "LDO_VMIC",
    "LDO_VTV",
    "LDO_VAUDN",
    "LDO_VAUDP",
    "LDO_PMUA",
    "LDO_VRF",
    "LDO_VCAMA",
    "LDO_VCAMD",
    "LDO_VIO",
    "LDO_VUSB",
    "LDO_VSIM",
    "LDO_VSIM2",
    "LDO_VIBR",
    "LDO_VMC",
    "LDO_VCAMA2",
    "LDO_VCAMD2",
    "LDO_VM12",
    "LDO_VM12_INT"
};


static int hwPMRegDump1
(
	char *page, 
	char **start, 
	off_t off,
	int count, 
	int *eof, 
	void *data
)
{
	int len = 0;
	char *p = page;


    p += sprintf(p, "\n\rPLL Register\n\r" );                                                        
	p += sprintf(p, "=========================================\n\r" );       
	p += sprintf(p, "XOSC_CON       0x%x = 0x%x\n\r",XOSC_CON       ,DRV_Reg32(XOSC_CON       ));  
	p += sprintf(p, "CLKSQ_CON       0x%x = 0x%x\n\r",CLKSQ_CON       ,DRV_Reg32(CLKSQ_CON       ));  
	p += sprintf(p, "PLL_CON0_REG       0x%x = 0x%x\n\r",PLL_CON0_REG       ,DRV_Reg32(PLL_CON0_REG       ));  
	p += sprintf(p, "PLL_CON1_REG       0x%x = 0x%x\n\r",PLL_CON1_REG       ,DRV_Reg32(PLL_CON1_REG       ));  
	p += sprintf(p, "PLL_CON2_REG       0x%x = 0x%x\n\r",PLL_CON2_REG       ,DRV_Reg32(PLL_CON2_REG       ));  
	p += sprintf(p, "PLL_CON4_REG       0x%x = 0x%x\n\r",PLL_CON4_REG       ,DRV_Reg32(PLL_CON4_REG       ));  
	p += sprintf(p, "PLL_CON5_REG       0x%x = 0x%x\n\r",PLL_CON5_REG       ,DRV_Reg32(PLL_CON5_REG       ));  
	p += sprintf(p, "PLL_CON6_REG       0x%x = 0x%x\n\r",PLL_CON6_REG       ,DRV_Reg32(PLL_CON6_REG       ));  
	p += sprintf(p, "CLKSW_PLLDIV_CON0  0x%x = 0x%x\n\r",CLKSW_PLLDIV_CON0  ,DRV_Reg32(CLKSW_PLLDIV_CON0  ));  
	p += sprintf(p, "CLKSW_PLLDIV_CON1  0x%x = 0x%x\n\r",CLKSW_PLLDIV_CON1  ,DRV_Reg32(CLKSW_PLLDIV_CON1  ));  
	p += sprintf(p, "CLKSW_PLLDIV_CON3  0x%x = 0x%x\n\r",CLKSW_PLLDIV_CON3  ,DRV_Reg32(CLKSW_PLLDIV_CON3  ));  
	p += sprintf(p, "CLKSW_PLLCNTEN_CON 0x%x = 0x%x\n\r",CLKSW_PLLCNTEN_CON ,DRV_Reg32(CLKSW_PLLCNTEN_CON ));  
	p += sprintf(p, "MPLL_CON0_REG      0x%x = 0x%x\n\r",MPLL_CON0_REG      ,DRV_Reg32(MPLL_CON0_REG      ));  
	p += sprintf(p, "MPLL_CON1_REG      0x%x = 0x%x\n\r",MPLL_CON1_REG      ,DRV_Reg32(MPLL_CON1_REG      ));  
	p += sprintf(p, "MPLL_CON3_REG      0x%x = 0x%x\n\r",MPLL_CON3_REG      ,DRV_Reg32(MPLL_CON3_REG      ));  
	p += sprintf(p, "AMPLL_CON0_REG     0x%x = 0x%x\n\r",AMPLL_CON0_REG     ,DRV_Reg32(AMPLL_CON0_REG     ));  
	p += sprintf(p, "AMPLL_CON1_REG     0x%x = 0x%x\n\r",AMPLL_CON1_REG     ,DRV_Reg32(AMPLL_CON1_REG     ));  
	p += sprintf(p, "AMPLL_CON3_REG     0x%x = 0x%x\n\r",AMPLL_CON3_REG     ,DRV_Reg32(AMPLL_CON3_REG     ));  
	p += sprintf(p, "DPLL_CON0_REG      0x%x = 0x%x\n\r",DPLL_CON0_REG      ,DRV_Reg32(DPLL_CON0_REG      ));  
	p += sprintf(p, "DPLL_CON1_REG      0x%x = 0x%x\n\r",DPLL_CON1_REG      ,DRV_Reg32(DPLL_CON1_REG      ));  
	p += sprintf(p, "DPLL_CON3_REG      0x%x = 0x%x\n\r",DPLL_CON3_REG      ,DRV_Reg32(DPLL_CON3_REG      ));  
	p += sprintf(p, "EPLL_CON0_REG      0x%x = 0x%x\n\r",EPLL_CON0_REG      ,DRV_Reg32(EPLL_CON0_REG      ));  
	p += sprintf(p, "EPLL_CON1_REG      0x%x = 0x%x\n\r",EPLL_CON1_REG      ,DRV_Reg32(EPLL_CON1_REG      ));  
	p += sprintf(p, "EPLL_CON3_REG      0x%x = 0x%x\n\r",EPLL_CON3_REG      ,DRV_Reg32(EPLL_CON3_REG      ));  
	p += sprintf(p, "CPLL_CON0_REG      0x%x = 0x%x\n\r",CPLL_CON0_REG      ,DRV_Reg32(CPLL_CON0_REG      ));  
	p += sprintf(p, "WPLL_CON0_REG      0x%x = 0x%x\n\r",WPLL_CON0_REG      ,DRV_Reg32(WPLL_CON0_REG      ));  
	p += sprintf(p, "GPLL_CON0_REG      0x%x = 0x%x\n\r",GPLL_CON0_REG      ,DRV_Reg32(GPLL_CON0_REG      ));  
	p += sprintf(p, "THREEDPLL_CON0_REG 0x%x = 0x%x\n\r",THREEDPLL_CON0_REG ,DRV_Reg32(THREEDPLL_CON0_REG ));  
	p += sprintf(p, "TVPLL_CON0_REG     0x%x = 0x%x\n\r",TVPLL_CON0_REG     ,DRV_Reg32(TVPLL_CON0_REG     ));  
	p += sprintf(p, "FGPLL_CON0_REG     0x%x = 0x%x\n\r",FGPLL_CON0_REG     ,DRV_Reg32(FGPLL_CON0_REG     ));  
	p += sprintf(p, "AUXPLL_CON0_REG    0x%x = 0x%x\n\r",AUXPLL_CON0_REG    ,DRV_Reg32(AUXPLL_CON0_REG    ));  

	p += sprintf(p, "\n\rAPCONFIG Register\n\r" );                                                        
	p += sprintf(p, "=========================================\n\r" );       
	p += sprintf(p, "APHW_VER       0x%x = 0x%x\n\r",APHW_VER      ,DRV_Reg32(APHW_VER       ));  
	p += sprintf(p, "APSW_VER       0x%x = 0x%x\n\r",APSW_VER      ,DRV_Reg32(APSW_VER       ));  
	p += sprintf(p, "APHW_CODE      0x%x = 0x%x\n\r",APHW_CODE     ,DRV_Reg32(APHW_CODE      ));  
	p += sprintf(p, "CHIP_STA       0x%x = 0x%x\n\r",CHIP_STA      ,DRV_Reg32(CHIP_STA       ));  
	p += sprintf(p, "APSYS_RST      0x%x = 0x%x\n\r",APSYS_RST     ,DRV_Reg32(APSYS_RST      ));  
	p += sprintf(p, "APP_MEM_PD     0x%x = 0x%x\n\r",APP_MEM_PD    ,DRV_Reg32(APP_MEM_PD     ));  
	p += sprintf(p, "MM1_MEM_PD0    0x%x = 0x%x\n\r",MM1_MEM_PD0   ,DRV_Reg32(MM1_MEM_PD0    ));  
	p += sprintf(p, "MM1_MEM_PD1    0x%x = 0x%x\n\r",MM1_MEM_PD1   ,DRV_Reg32(MM1_MEM_PD1    ));  
	p += sprintf(p, "MM2_MEM_PD     0x%x = 0x%x\n\r",MM2_MEM_PD    ,DRV_Reg32(MM2_MEM_PD     ));  
	p += sprintf(p, "APP_MEM_DELSEL 0x%x = 0x%x\n\r",APP_MEM_DELSEL,DRV_Reg32(APP_MEM_DELSEL ));  
	p += sprintf(p, "AP_MEM_DELSEL  0x%x = 0x%x\n\r",AP_MEM_DELSEL ,DRV_Reg32(AP_MEM_DELSEL  ));  
	p += sprintf(p, "AP_MEM_PD      0x%x = 0x%x\n\r",AP_MEM_PD     ,DRV_Reg32(AP_MEM_PD      ));  
	p += sprintf(p, "APP_CON        0x%x = 0x%x\n\r",APP_CON       ,DRV_Reg32(APP_CON        ));  
	p += sprintf(p, "BUS_GATING_EN  0x%x = 0x%x\n\r",BUS_GATING_EN ,DRV_Reg32(BUS_GATING_EN  ));  
	p += sprintf(p, "TOPSM_DMY0 	0x%x = 0x%x\n\r",TOPSM_DMY0 	,DRV_Reg32(TOPSM_DMY0 		 ));  
	p += sprintf(p, "TOPSM_DMY1 	0x%x = 0x%x\n\r",TOPSM_DMY1 	,DRV_Reg32(TOPSM_DMY1 		 ));  
	p += sprintf(p, "AP_SAPD_WAYEN 	0x%x = 0x%x\n\r",AP_SAPD_WAYEN ,DRV_Reg32(AP_SAPD_WAYEN 	 ));  
	p += sprintf(p, "AP_SLPPRT_BUS  0x%x = 0x%x\n\r",AP_SLPPRT_BUS ,DRV_Reg32(AP_SLPPRT_BUS  ));  
	p += sprintf(p, "APPER_SLP_CON  0x%x = 0x%x\n\r",APPER_SLP_CON ,DRV_Reg32(APPER_SLP_CON  ));  
	p += sprintf(p, "APARM_FSEL     0x%x = 0x%x\n\r",APARM_FSEL    ,DRV_Reg32(APARM_FSEL     ));  
	p += sprintf(p, "DSP_FSEL       0x%x = 0x%x\n\r",DSP_FSEL      ,DRV_Reg32(DSP_FSEL       ));  
	p += sprintf(p, "EMI_FSEL       0x%x = 0x%x\n\r",EMI_FSEL      ,DRV_Reg32(EMI_FSEL       ));  
	p += sprintf(p, "FBUS_FSEL      0x%x = 0x%x\n\r",FBUS_FSEL     ,DRV_Reg32(FBUS_FSEL      ));  
	p += sprintf(p, "SBUS_FSEL      0x%x = 0x%x\n\r",SBUS_FSEL     ,DRV_Reg32(SBUS_FSEL      ));  
	p += sprintf(p, "DSP_IDLE_FSEL  0x%x = 0x%x\n\r",DSP_IDLE_FSEL ,DRV_Reg32(DSP_IDLE_FSEL  ));  
	p += sprintf(p, "EMI_IDLE_FSEL  0x%x = 0x%x\n\r",EMI_IDLE_FSEL ,DRV_Reg32(EMI_IDLE_FSEL  ));  
	p += sprintf(p, "FBUS_IDLE_FSEL 0x%x = 0x%x\n\r",FBUS_IDLE_FSEL,DRV_Reg32(FBUS_IDLE_FSEL ));  
	p += sprintf(p, "SBUS_IDLE_FSEL 0x%x = 0x%x\n\r",SBUS_IDLE_FSEL,DRV_Reg32(SBUS_IDLE_FSEL ));  
	p += sprintf(p, "RG_CK_ALW_ON   0x%x = 0x%x\n\r",RG_CK_ALW_ON  ,DRV_Reg32(RG_CK_ALW_ON   ));  
	p += sprintf(p, "RG_CK_DCM_EN   0x%x = 0x%x\n\r",RG_CK_DCM_EN  ,DRV_Reg32(RG_CK_DCM_EN   ));  
	p += sprintf(p, "RG_EMI_DBC     0x%x = 0x%x\n\r",RG_EMI_DBC    ,DRV_Reg32(RG_EMI_DBC     ));	                 
	p += sprintf(p, "RG_BUS_DBC     0x%x = 0x%x\n\r",RG_BUS_DBC    ,DRV_Reg32(RG_BUS_DBC     ));	                 
	p += sprintf(p, "MSDC_CCTL      0x%x = 0x%x\n\r",MSDC_CCTL     ,DRV_Reg32(MSDC_CCTL      ));                     
	p += sprintf(p, "MSDC2_CCTL     0x%x = 0x%x\n\r",MSDC2_CCTL    ,DRV_Reg32(MSDC2_CCTL     ));                     
	p += sprintf(p, "MSDC3_CCTL     0x%x = 0x%x\n\r",MSDC3_CCTL    ,DRV_Reg32(MSDC3_CCTL     ));                     
	p += sprintf(p, "MSDC4_CCTL     0x%x = 0x%x\n\r",MSDC4_CCTL    ,DRV_Reg32(MSDC4_CCTL     ));                     
	p += sprintf(p, "GC_CLK_MON0    0x%x = 0x%x\n\r",GC_CLK_MON0   ,DRV_Reg32(GC_CLK_MON0    ));                     
	p += sprintf(p, "GC_CLK_MON1    0x%x = 0x%x\n\r",GC_CLK_MON1   ,DRV_Reg32(GC_CLK_MON1    ));                     
	p += sprintf(p, "GC_CLK_MON2    0x%x = 0x%x\n\r",GC_CLK_MON2   ,DRV_Reg32(GC_CLK_MON2    ));                     
	p += sprintf(p, "APMCU_CG_CON0 	0x%x = 0x%x\n\r",APMCU_CG_CON0 ,DRV_Reg32(APMCU_CG_CON0 	 ));                     
	p += sprintf(p, "APMCU_CG_SET0 	0x%x = 0x%x\n\r",APMCU_CG_SET0 ,DRV_Reg32(APMCU_CG_SET0 	 ));                     
	p += sprintf(p, "APMCU_CG_CLR0 	0x%x = 0x%x\n\r",APMCU_CG_CLR0 ,DRV_Reg32(APMCU_CG_CLR0 	 ));                     
	p += sprintf(p, "APMCU_CG_CON1 	0x%x = 0x%x\n\r",APMCU_CG_CON1 ,DRV_Reg32(APMCU_CG_CON1 	 ));                     
	p += sprintf(p, "APMCU_CG_SET1 	0x%x = 0x%x\n\r",APMCU_CG_SET1 ,DRV_Reg32(APMCU_CG_SET1 	 ));                     
	p += sprintf(p, "APMCU_CG_CLR1 	0x%x = 0x%x\n\r",APMCU_CG_CLR1 ,DRV_Reg32(APMCU_CG_CLR1 	 ));

	p += sprintf(p, "MMSYS1_CG_CON0     0x%x = 0x%x\n\r",MMSYS1_CG_CON0    ,DRV_Reg32(MMSYS1_CG_CON0     ));                     
	p += sprintf(p, "MMSYS1_CG_CON1    0x%x = 0x%x\n\r",MMSYS1_CG_CON1   ,DRV_Reg32(MMSYS1_CG_CON1    ));                     
	p += sprintf(p, "MMSYS1_CG_SET0    0x%x = 0x%x\n\r",MMSYS1_CG_SET0   ,DRV_Reg32(MMSYS1_CG_SET0    ));                     
	p += sprintf(p, "MMSYS1_CG_SET1    0x%x = 0x%x\n\r",MMSYS1_CG_SET1   ,DRV_Reg32(MMSYS1_CG_SET1    ));                     
	p += sprintf(p, "MMSYS1_CG_CLR0 	0x%x = 0x%x\n\r",MMSYS1_CG_CLR0 ,DRV_Reg32(MMSYS1_CG_CLR0 	 ));                     
	p += sprintf(p, "MMSYS1_CG_CLR1 	0x%x = 0x%x\n\r",MMSYS1_CG_CLR1 ,DRV_Reg32(MMSYS1_CG_CLR1 	 ));                     
	p += sprintf(p, "MMSYS2_CG_CON0 	0x%x = 0x%x\n\r",MMSYS2_CG_CON0 ,DRV_Reg32(MMSYS2_CG_CON0 	 ));                     
	p += sprintf(p, "MMSYS2_CG_SET0 	0x%x = 0x%x\n\r",MMSYS2_CG_SET0 ,DRV_Reg32(MMSYS2_CG_SET0 	 ));                     
	p += sprintf(p, "MMSYS2_CG_CLR0 	0x%x = 0x%x\n\r",MMSYS2_CG_CLR0 ,DRV_Reg32(MMSYS2_CG_CLR0 	 ));                     
	p += sprintf(p, "AUDIO_TOP_CON0 	0x%x = 0x%x\n\r",AUDIO_TOP_CON0_REG ,DRV_Reg32(AUDIO_TOP_CON0_REG 	 ));


	*start = page + off;

	len = p - page;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}



static int hwPMRegDump2
(
	char *page, 
	char **start, 
	off_t off,
	int count, 
	int *eof, 
	void *data
)
{
	int len = 0;
	char *p = page;
#if 0
    hwLowPowerPLLSwitch();
#endif

	p += sprintf(p, "\n\rAPOST Register\n\r" );                                                        
	p += sprintf(p, "=========================================\n\r" );       
	p += sprintf(p, "OST_CON           0x%x = 0x%x\n\r",OST_CON            ,DRV_Reg32(OST_CON             ));  
	p += sprintf(p, "OST_CMD           0x%x = 0x%x\n\r",OST_CMD            ,DRV_Reg32(OST_CMD             ));  
	p += sprintf(p, "OST_STA           0x%x = 0x%x\n\r",OST_STA            ,DRV_Reg32(OST_STA            ));  
	p += sprintf(p, "OST_FRM           0x%x = 0x%x\n\r",OST_FRM            ,DRV_Reg32(OST_FRM             ));  
	p += sprintf(p, "OST_FRM_F32K      0x%x = 0x%x\n\r",OST_FRM_F32K       ,DRV_Reg32(OST_FRM_F32K            ));  
	p += sprintf(p, "OST_UFN           0x%x = 0x%x\n\r",OST_UFN            ,DRV_Reg32(OST_UFN           ));  
	p += sprintf(p, "OST_AFN           0x%x = 0x%x\n\r",OST_AFN            ,DRV_Reg32(OST_AFN          ));  
	p += sprintf(p, "OST_AFN_DLY       0x%x = 0x%x\n\r",OST_AFN_DLY        ,DRV_Reg32(OST_AFN_DLY          ));  
	p += sprintf(p, "OST_UFN_R         0x%x = 0x%x\n\r",OST_UFN_R          ,DRV_Reg32(OST_UFN_R           ));  
	p += sprintf(p, "OST_AFN_R         0x%x = 0x%x\n\r",OST_AFN_R          ,DRV_Reg32(OST_AFN_R       ));  
	p += sprintf(p, "OST_INT_MASK      0x%x = 0x%x\n\r",OST_INT_MASK       ,DRV_Reg32(OST_INT_MASK        ));  
	p += sprintf(p, "OST_ISR           0x%x = 0x%x\n\r",OST_ISR            ,DRV_Reg32(OST_ISR            ));  
	p += sprintf(p, "OST_EVENT_MASK    0x%x = 0x%x\n\r",OST_EVENT_MASK     ,DRV_Reg32(OST_EVENT_MASK              ));  
	p += sprintf(p, "OST_WAKEUP_STA    0x%x = 0x%x\n\r",OST_WAKEUP_STA     ,DRV_Reg32(OST_WAKEUP_STA        ));  
	p += sprintf(p, "OST_DBG_WAKEUP    0x%x = 0x%x\n\r",OST_DBG_WAKEUP     ,DRV_Reg32(OST_DBG_WAKEUP 		 ));  
	p += sprintf(p, "RM_CLK_SETTLE     0x%x = 0x%x\n\r",RM_CLK_SETTLE      ,DRV_Reg32(RM_CLK_SETTLE 	 ));  
	p += sprintf(p, "RM_TMRPWR_SETTLE  0x%x = 0x%x\n\r",RM_TMRPWR_SETTLE   ,DRV_Reg32(RM_TMRPWR_SETTLE        ));  
	p += sprintf(p, "RM_TMR_TRG0       0x%x = 0x%x\n\r",RM_TMR_TRG0        ,DRV_Reg32(RM_TMR_TRG0        ));  
	p += sprintf(p, "RM_TMR_PWR0       0x%x = 0x%x\n\r",RM_TMR_PWR0        ,DRV_Reg32(RM_TMR_PWR0           ));  
	p += sprintf(p, "RM_TMR_PWR1       0x%x = 0x%x\n\r",RM_TMR_PWR1        ,DRV_Reg32(RM_TMR_PWR1             ));  
	p += sprintf(p, "RM_PERI_CON       0x%x = 0x%x\n\r",RM_PERI_CON        ,DRV_Reg32(RM_PERI_CON            ));  
	p += sprintf(p, "RM_TMR_SSTA       0x%x = 0x%x\n\r",RM_TMR_SSTA        ,DRV_Reg32(RM_TMR_SSTA            ));  
	p += sprintf(p, "TOPSM_DBG         0x%x = 0x%x\n\r",TOPSM_DBG          ,DRV_Reg32(TOPSM_DBG        ));  
	p += sprintf(p, "FRC_CON           0x%x = 0x%x\n\r",FRC_CON            ,DRV_Reg32(FRC_CON        ));  
	p += sprintf(p, "FRC_F32K_FM       0x%x = 0x%x\n\r",FRC_F32K_FM        ,DRV_Reg32(FRC_F32K_FM       ));  
	p += sprintf(p, "FRC_VAL_R         0x%x = 0x%x\n\r",FRC_VAL_R          ,DRV_Reg32(FRC_VAL_R       ));  
	p += sprintf(p, "GPS_SYNC          0x%x = 0x%x\n\r",GPS_SYNC           ,DRV_Reg32(GPS_SYNC         ));  
	p += sprintf(p, "FRC_SYNC1         0x%x = 0x%x\n\r",FRC_SYNC1          ,DRV_Reg32(FRC_SYNC1         ));  
	p += sprintf(p, "FRC_SYNC2         0x%x = 0x%x\n\r",FRC_SYNC2          ,DRV_Reg32(FRC_SYNC2           ));	                 
	p += sprintf(p, "FM_CON            0x%x = 0x%x\n\r",FM_CON             ,DRV_Reg32(FM_CON           ));                     
	p += sprintf(p, "FM_CAL            0x%x = 0x%x\n\r",FM_CAL             ,DRV_Reg32(FM_CAL           ));                     
	p += sprintf(p, "FM_T0             0x%x = 0x%x\n\r",FM_T0              ,DRV_Reg32(FM_T0           ));                     
	p += sprintf(p, "FM_T1             0x%x = 0x%x\n\r",FM_T1              ,DRV_Reg32(FM_T1          ));                     
	p += sprintf(p, "F32K_CNT          0x%x = 0x%x\n\r",F32K_CNT           ,DRV_Reg32(F32K_CNT          ));                     
	p += sprintf(p, "CCF_CLK_CON       0x%x = 0x%x\n\r",CCF_CLK_CON        ,DRV_Reg32(CCF_CLK_CON          ));                     
	p += sprintf(p, "RM_PWR_CON0       0x%x = 0x%x\n\r",RM_PWR_CON0        ,DRV_Reg32(RM_PWR_CON0 	 ));                     
	p += sprintf(p, "RM_PWR_CON1       0x%x = 0x%x\n\r",RM_PWR_CON1        ,DRV_Reg32(RM_PWR_CON1 	 ));                     
	p += sprintf(p, "RM_PWR_CON2       0x%x = 0x%x\n\r",RM_PWR_CON2        ,DRV_Reg32(RM_PWR_CON2 	 ));                     
	p += sprintf(p, "RM_PWR_CON3       0x%x = 0x%x\n\r",RM_PWR_CON3        ,DRV_Reg32(RM_PWR_CON3 	 ));                     
	p += sprintf(p, "RM_PWR_CON4       0x%x = 0x%x\n\r",RM_PWR_CON4        ,DRV_Reg32(RM_PWR_CON4 	 ));                     
	p += sprintf(p, "RM_PWR_CON5       0x%x = 0x%x\n\r",RM_PWR_CON5        ,DRV_Reg32(RM_PWR_CON5 	 )); 
	p += sprintf(p, "RM_PWR_CON6       0x%x = 0x%x\n\r",RM_PWR_CON6        ,DRV_Reg32(RM_PWR_CON6 	 ));                     
	p += sprintf(p, "RM_PWR_CON7       0x%x = 0x%x\n\r",RM_PWR_CON7        ,DRV_Reg32(RM_PWR_CON7 	 ));                     
	p += sprintf(p, "RM_PWR_STA        0x%x = 0x%x\n\r",RM_PWR_STA         ,DRV_Reg32(RM_PWR_STA 	 ));                     
	p += sprintf(p, "RM_PLL_MASK0      0x%x = 0x%x\n\r",RM_PLL_MASK0       ,DRV_Reg32(RM_PLL_MASK0 	 ));                     
	p += sprintf(p, "RM_PLL_MASK1      0x%x = 0x%x\n\r",RM_PLL_MASK1       ,DRV_Reg32(RM_PLL_MASK1 	 ));    
	p += sprintf(p, "CCF_CLK_CON_R     0x%x = 0x%x\n\r",CCF_CLK_CON_R      ,DRV_Reg32(CCF_CLK_CON_R 	 ));    

	p += sprintf(p, "\n\rPMU Register\n\r" );                                                        
	p += sprintf(p, "=========================================\n\r" );       
	p += sprintf(p, "RETENTION_CON0 0x%x = 0x%x\n\r",RETENTION_CON0 ,DRV_Reg32(RETENTION_CON0 ));  
	p += sprintf(p, "LPOSC_CON0     0x%x = 0x%x\n\r",LPOSC_CON0     ,DRV_Reg32(LPOSC_CON0     ));  
	p += sprintf(p, "STRUP_CON0     0x%x = 0x%x\n\r",STRUP_CON0     ,DRV_Reg32(STRUP_CON0     ));  
	p += sprintf(p, "VA28_CON0      0x%x = 0x%x\n\r",VA28_CON0      ,DRV_Reg32(VA28_CON0      ));  
	p += sprintf(p, "VA25_CON0      0x%x = 0x%x\n\r",VA25_CON0      ,DRV_Reg32(VA25_CON0      ));  
	p += sprintf(p, "VA25_CON1      0x%x = 0x%x\n\r",(VA25_CON0+0x4)      ,DRV_Reg32(VA25_CON0+0x4      ));  
	p += sprintf(p, "VA12_CON0      0x%x = 0x%x\n\r",VA12_CON0      ,DRV_Reg32(VA12_CON0      ));  
	p += sprintf(p, "VRTC_CON0      0x%x = 0x%x\n\r",VRTC_CON0      ,DRV_Reg32(VRTC_CON0      ));  
	p += sprintf(p, "VMIC_CON0      0x%x = 0x%x\n\r",VMIC_CON0      ,DRV_Reg32(VMIC_CON0      ));  
	p += sprintf(p, "VTV_CON0       0x%x = 0x%x\n\r",VTV_CON0       ,DRV_Reg32(VTV_CON0       ));  
	p += sprintf(p, "VAUDN_CON0     0x%x = 0x%x\n\r",VAUDN_CON0     ,DRV_Reg32(VAUDN_CON0     ));  
	p += sprintf(p, "VAUDP_CON0     0x%x = 0x%x\n\r",VAUDP_CON0     ,DRV_Reg32(VAUDP_CON0     ));  
	p += sprintf(p, "PMUA_CON0      0x%x = 0x%x\n\r",PMUA_CON0      ,DRV_Reg32(PMUA_CON0      ));  
	p += sprintf(p, "VRF_CON0       0x%x = 0x%x\n\r",VRF_CON0       ,DRV_Reg32(VRF_CON0       ));  
	p += sprintf(p, "VCAMA_CON0     0x%x = 0x%x\n\r",VCAMA_CON0     ,DRV_Reg32(VCAMA_CON0     ));  
	p += sprintf(p, "VCAMD_CON0  	0x%x = 0x%x\n\r",VCAMD_CON0     ,DRV_Reg32(VCAMD_CON0  	));  
	p += sprintf(p, "VIO_CON0    	0x%x = 0x%x\n\r",VIO_CON0       ,DRV_Reg32(VIO_CON0    	));  
	p += sprintf(p, "VUSB_CON0      0x%x = 0x%x\n\r",VUSB_CON0      ,DRV_Reg32(VUSB_CON0   	));  
	p += sprintf(p, "VSIM_CON0      0x%x = 0x%x\n\r",VSIM_CON0      ,DRV_Reg32(VSIM_CON0      ));  
	p += sprintf(p, "VSIM2_CON0     0x%x = 0x%x\n\r",VSIM2_CON0     ,DRV_Reg32(VSIM2_CON0     ));  
	p += sprintf(p, "VIBR_CON0      0x%x = 0x%x\n\r",VIBR_CON0      ,DRV_Reg32(VIBR_CON0      ));  
	p += sprintf(p, "VMC_CON0       0x%x = 0x%x\n\r",VMC_CON0       ,DRV_Reg32(VMC_CON0       ));  
	p += sprintf(p, "VCAMA2_CON0    0x%x = 0x%x\n\r",VCAMA2_CON0    ,DRV_Reg32(VCAMA2_CON0    ));  
	p += sprintf(p, "VCAMD2_CON0    0x%x = 0x%x\n\r",VCAMD2_CON0    ,DRV_Reg32(VCAMD2_CON0    ));  
	p += sprintf(p, "VM12_CON0      0x%x = 0x%x\n\r",VM12_CON0      ,DRV_Reg32(VM12_CON0      ));  
	p += sprintf(p, "VM12_INT_CON0  0x%x = 0x%x\n\r",VM12_INT_CON0  ,DRV_Reg32(VM12_INT_CON0  ));  
	p += sprintf(p, "VCORE_CON0     0x%x = 0x%x\n\r",VCORE_CON0     ,DRV_Reg32(VCORE_CON0     ));  
    p += sprintf(p, "VCORE_CON1     0x%x = 0x%x\n\r",(VCORE_CON0+0x4)     ,DRV_Reg32(VCORE_CON0+0x4     ));  
	p += sprintf(p, "VIO1V8_CON0    0x%x = 0x%x\n\r",VIO1V8_CON0    ,DRV_Reg32(VIO1V8_CON0    ));  
	p += sprintf(p, "VAPROC_CON0    0x%x = 0x%x\n\r",VAPROC_CON0    ,DRV_Reg32(VAPROC_CON0    ));  
    p += sprintf(p, "VAPROC_CON1    0x%x = 0x%x\n\r",(VAPROC_CON0+0x4)    ,DRV_Reg32(VAPROC_CON0+0x4    ));  
	p += sprintf(p, "VRF1V8_CON0    0x%x = 0x%x\n\r",VRF1V8_CON0    ,DRV_Reg32(VRF1V8_CON0    ));  
	p += sprintf(p, "PMIC_BB_CON0   0x%x = 0x%x\n\r",PMIC_BB_CON0   ,DRV_Reg32(PMIC_BB_CON0   ));  
	p += sprintf(p, "KPLED_CON0     0x%x = 0x%x\n\r",KPLED_CON0     ,DRV_Reg32(KPLED_CON0     ));	                 
	p += sprintf(p, "PMIC_OC_CON0   0x%x = 0x%x\n\r",PMIC_OC_CON0   ,DRV_Reg32(PMIC_OC_CON0   ));	                 
	p += sprintf(p, "CHR_CON0       0x%x = 0x%x\n\r",CHR_CON0       ,DRV_Reg32(CHR_CON0       ));  

	*start = page + off;

	len = p - page;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}


static int hwPMFlagRead(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    p += sprintf(p, "Mediatek Power DEBUG flag\n\r" );
    p += sprintf(p, "=========================================\n\r" );
    p += sprintf(p, "CM_DBG_FLAG = 0x%x\n\r",CM_DBG_FLAG);
    p += sprintf(p, "wakelock_debug_mask = 0x%x\n\r",wakelock_debug_mask);
    p += sprintf(p, "Userwakelock_debug_mask = 0x%x\n\r",Userwakelock_debug_mask);
    p += sprintf(p, "Earlysuspend_debug_mask = 0x%x\n\r",Earlysuspend_debug_mask);
    p += sprintf(p, "MT6516 console suspend enable = 0x%x\n\r",console_suspend_enabled);	
    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

static int
hwPMFlagWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize = 0;
    UINT32 u4NewPMDbgLevel = 0;
    UINT32 u4NewPMWakelockLevel = 0;
    UINT32 u4NewUserWakelockLevel = 0;
    UINT32 u4NewEarlysuspendLevel = 0;
    UINT32 u4ConsoleSuspend = 0;


    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
		return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%x %x %x %x %x", &u4NewPMDbgLevel, &u4NewPMWakelockLevel, 
        &u4NewUserWakelockLevel, &u4NewEarlysuspendLevel, &u4ConsoleSuspend) == 5) {
        CM_DBG_FLAG = u4NewPMDbgLevel;
        wakelock_debug_mask = u4NewPMWakelockLevel;
        Userwakelock_debug_mask = u4NewUserWakelockLevel;
        Earlysuspend_debug_mask = u4NewEarlysuspendLevel;
	console_suspend_enabled = u4ConsoleSuspend;
    }
    return count;
}

static int hwPMStat
(
	char *page, 
	char **start, 
	off_t off,
	int count, 
	int *eof, 
	void *data
)
{
    int len = 0;
    int index = 0, i = 0, j = 0, index_mod = 0, category = -1;	
    char *p = page;
        
    p += sprintf(p, "\n\rPower management Status\n\r" );                                                        
    p += sprintf(p, "=========================================\n\r" );       

    //print the state(enable/disable) of all sub system(MTCMOS)
    p += sprintf(p, "SUBSYSTEM:0x%x \n\r",g_MT6573_BusHW.dwSubSystem_status); 
    
    //print the enabled sub system
    for ( index=MD2G_SUBSYS; index < MT65XX_SUBSYS_COUNT_END; index++ )
        if( g_MT6573_BusHW.dwSubSystem_status & (1<<index))	
        {
            category = MT65XX__CATEGORY_OF_SUBSYS[index];
            if(category >= 0)
            {
                if(index == AP_SUBSYS || index == MM1_SUBSYS)
                {
                    p += sprintf(p, "   [%d][%s] clock_status:[0x%x, 0x%x]\n\r",index, MT65XX_SUBSYS_name[index], g_MT6573_BusHW.dwSubSystem_clock[category], g_MT6573_BusHW.dwSubSystem_clock[category +1]);
                }else 
                {
                    p += sprintf(p, "   [%d][%s] clock_status:[0x%x]\n\r",index, MT65XX_SUBSYS_name[index], g_MT6573_BusHW.dwSubSystem_clock[category]);
                }
            }else {
                p += sprintf(p, "   [%d][%s]\n\r", index, MT65XX_SUBSYS_name[index]);
            }
        }
    p += sprintf(p, "\n\r"); 
       
    //scan pll
    p += sprintf(p, "PLL:\n\r");
    for ( index=MT65XX_APMCU_PLL; index < MT65XX_PLL_COUNT_END; index++ )
    {
        if(g_MT6573_BusHW.Pll[index].dwPllCount > 0)
        { 
            p += sprintf(p, "   [%d][%s] count[%d] master[",index,
                                                       g_MT6573_BusHW.Pll[index].name,
                                                       g_MT6573_BusHW.Pll[index].dwPllCount);
            for (index_mod = 0; index_mod < MAX_DEVICE; index_mod++)
            {
                if(!strcmp(g_MT6573_BusHW.Pll[index].mod_name[index_mod], NON_OP))
                    p += sprintf(p, ",");
                else
                    p += sprintf(p, "%s,",g_MT6573_BusHW.Pll[index].mod_name[index_mod]);
            }	
            p += sprintf(p, "]\n\r"); 
        }
    }
    p += sprintf(p, "\n\r"); 

    //scan power
    p += sprintf(p, "Power:\n\r");
    for ( index=MT65XX_POWER_LDO_VA28; index < MT65XX_POWER_COUNT_END; index++ )
    {
        if(g_MT6573_BusHW.Power[index].dwPowerCount > 0)
        {
            p += sprintf(p, "   [%d][%s] count[%d] master[",index,
                                                       g_MT6573_BusHW.Power[index].name,
                                                       g_MT6573_BusHW.Power[index].dwPowerCount);
            for (index_mod = 0 ; index_mod < MAX_DEVICE; index_mod++)
            {
                if(!strcmp(g_MT6573_BusHW.Power[index].mod_name[index_mod], NON_OP))
                    p += sprintf(p, ",");
                else
                    p += sprintf(p, "%s,",g_MT6573_BusHW.Power[index].mod_name[index_mod]);
            }	
            p += sprintf(p, "]\n\r"); 
        }
    }
    p += sprintf(p, "\n\r"); 
    
    //scan XX_CLK_EN
    p+= sprintf(p, "Pll Control:\n\r");
    for(i = 0; i< MT65XX_CLK_EN_COUNT_END; i++)
    {
         if(g_clken[i].dwClkEnCount > 0)
         {
             p += sprintf(p, "  [%d][%s] count[%d] master[", i, 
                                                               g_clken[i].name,
                                                               g_clken[i].dwClkEnCount);
             for(index_mod = 0; index_mod < MAX_DEVICE; index_mod++)
             {
                 if(!strcmp(g_clken[i].mod_name[index_mod], NON_OP))
                     p += sprintf(p, ",");
                 else 
                     p += sprintf(p, "%s,", g_clken[i].mod_name[index_mod]);
             }
             p += sprintf(p, "]\n\r");
         }
    }    
    p+= sprintf(p, "\n\r");

    //scan clock
    p += sprintf(p, "Clock:\n\r");
    for ( i = 0; i < MT65XX_CLOCK_CATEGORY_COUNT; i++ )
    {
       for( j = 0; j < MT65XX_CLOCK_NUMBER[i]; j++)
       {
          if(g_MT6573_BusHW.Clock[i][j].dwClockCount > 0)
          {
              p += sprintf(p, "   [%d,%d][%s] count[%d] type[%s] ",i, j,
                                                       g_MT6573_BusHW.Clock[i][j].name,
                                                       g_MT6573_BusHW.Clock[i][j].dwClockCount,
                                                       MT65XX_CLOCK_TYPE_name[g_MT6573_BusHW.Clock[i][j].clock_type]);
              if(g_MT6573_BusHW.Clock[i][j].active_pll_source >= 0)
              {
                  p += sprintf(p, "pll[%s] master[", MT65XX_PLL_name[g_MT6573_BusHW.Clock[i][j].active_pll_source]);
              }
              for (index_mod = 0; index_mod < MAX_DEVICE; index_mod++)
              {
                  if(!strcmp(g_MT6573_BusHW.Clock[i][j].mod_name[index_mod], NON_OP))
                      p += sprintf(p, ",");
                  else
                      p += sprintf(p, "%s,",g_MT6573_BusHW.Clock[i][j].mod_name[index_mod]);
              }	
              p += sprintf(p, "]\n\r"); 
          }
       }
    }
    p += sprintf(p, "\n\r"); 

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}




static int hwPMDefault
(
	char *page, 
	char **start, 
	off_t off,
	int count, 
	int *eof, 
	void *data
)
{
    int len = 0;
    int index = 0, i = 0, j = 0;	
    char *p = page;
	
    p += sprintf(p, "\n\rSUBSYS Default on\n\r" );                                                        
    p += sprintf(p, "=========================================\n\r" );       
    for ( index=MD2G_SUBSYS; index < MT65XX_SUBSYS_COUNT_END; index++ )
    {
        if(g_MT6573_BusHW.dwSubSystem_defaultOn & (1<<index))
            p += sprintf(p, "   %s\n\r",MT65XX_SUBSYS_name[index]);    
    }    
    p += sprintf(p, "\n\r"); 

    p += sprintf(p, "\n\rPLL Default on\n\r" );                                                        
    p += sprintf(p, "=========================================\n\r" );       
    for ( index=MT65XX_APMCU_PLL; index < MT65XX_PLL_COUNT_END; index++ )
    {
        if(g_MT6573_BusHW.Pll[index].bDefault_on)
            p += sprintf(p, "   %s\n\r",g_MT6573_BusHW.Pll[index].name);    
    }    
    p += sprintf(p, "\n\r"); 

    p += sprintf(p, "\n\rLDO Default on\n\r" );                                                        
    p += sprintf(p, "=========================================\n\r" );       
    for ( index=MT65XX_POWER_LDO_VA28; index < MT65XX_POWER_COUNT_END; index++ )
    {
        if(g_MT6573_BusHW.Power[index].bDefault_on)
            p += sprintf(p, "   %s\n\r",g_MT6573_BusHW.Power[index].name);    
    }
    p += sprintf(p, "\n\r"); 

    p += sprintf(p, "\n\rClock Default on\n\r" );                                                        
    p += sprintf(p, "=========================================\n\r" );       
    for ( i = 0; i < MT65XX_CLOCK_CATEGORY_COUNT; i++ )
    {
       for( j = 0; j < MT65XX_CLOCK_NUMBER[i]; j++)
       {
          if(g_MT6573_BusHW.Clock[i][j].bDefault_on)
             p += sprintf(p, "   %s\n\r",g_MT6573_BusHW.Clock[i][j].name);    
       }
    }
    p += sprintf(p, "\n\r" );       
    
    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
    len = 0;

    return len < count ? len  : count;
}


static int
hwPMSpareWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize;
    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
        return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%x", &u4SECRET_KEY) == 1) 
    {    
        u4SECRET_KEY = 0;
    }
    return count;
}

static int hwPMSpareRead(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
    int len = 0;
    char *p = page;
    u16 spar0;

    spar0 = 0;

    p += sprintf(p, "Mediatek Power Power Setting\n\r" );
    p += sprintf(p, "=========================================\n\r" );
    p += sprintf(p, "spar0 = 0x%x\n\r",spar0);
    if (spar0 & SPARE_SECRET_KEY)
    {
        if(spar0 & SPARE_E1_PATCH)
            p += sprintf(p, "Low Power E1 Patch\n\r");
        if(spar0 & SPARE_DVFS_EN)
            p += sprintf(p, "DVFS Enable\n\r");
        if(spar0 & SPARE_VAPROC_ADJUST_EN)
            p += sprintf(p, "VAPROC Adjust Enable\n\r");
        if(spar0 & SPARE_DVFS_LOG)
            p += sprintf(p, "DVFS Log Enable\n\r");
        if(spar0 & SPARE_DCM_EN)
            p += sprintf(p, "DCM Enable\n\r");
    }
    else
    {
        p += sprintf(p, "Not Config\n\r");
    }
    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;

}    


static int hwPMTestHelp(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;
    p += sprintf(p, "Mediatek help\n\r" );
    p += sprintf(p, "=========================================\n\r" );
    p += sprintf(p, "[pm_clock_test]: echo [category] [offset] [pll_mode] [subSys] [enable] > pm_clock_test\n\r");
    p += sprintf(p, "[pm_pll_control]: echo [clk_en_type] [enable] > pm_pll_control\n\r");
    p += sprintf(p, "[pm_pll_test]: echo [enable] [pllId] [totalTime] > pm_pll_test\n\r");
    p += sprintf(p, "[pm_pll_fsel]: echo [pllId] [pll_fsel] > pm_pll_fsel\n\r");
    p += sprintf(p, "[pm_subsystem_test]: echo [enable] [subSysId] [totalTime] > pm_subsystem_test\n\r");
    p += sprintf(p, "[pm_set_clock_type]: echo [clockId] [clockType] > pm_set_clock_type\n\r");
    p += sprintf(p, "[pm_clock_listen]: echo [enable] > pm_clock_listen\n\r");
    p += sprintf(p, "[pm_log_control]: echo [type] [id] [enable] > pm_log_control\n\r");

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}
static int hwPMClockRead(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;
    int i = 0;
    p += sprintf(p, "Mediatek Clock gating\n\r" );
    p += sprintf(p, "=========================================\n\r" );
    for(i = 0; i < MT65XX_CLOCK_CATEGORY_COUNT; i++)
        p += sprintf(p, "category[%d]=0x%x\n\r",i, g_MT6573_BusHW.dwSubSystem_clock[i]);

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

static int hwPMESControl_Read(char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;
    int i = 0, j = 0;

    p += sprintf(p, "============PM early suspend control========\n\r" );

    p += sprintf(p, "     forbid_id=[0x%x]\r\n", forbid_id);
    p += sprintf(p, "     early_suspend_count=[%d]\r\n", early_suspend_count);


    p += sprintf(p, "===========================================\n\r" );

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

static int
hwPMESControl_Write (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32];
    UINT32 u4CopySize = 0;
    UINT32 id = 0;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
                return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%x",&id) == 1) {
		forbid_id = id;
		printk("============PM early suspend control========\n\r");
        printk("     forbid_id=[0x%x]\r\n", forbid_id);       
		printk("============================================\n\r");
    }
    return count;
}

static int hwPMLogControl_Read(char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;
    int i = 0, j = 0;

    p += sprintf(p, "============PM log control========\n\r" );
    p += sprintf(p, "  Clock log show:\n\r");
    for(i = 0; i < MT65XX_CLOCK_CATEGORY_COUNT; i++)
    {
        for(j = 0; j < MT65XX_CLOCK_NUMBER[i]; j++)
        {
            if(g_MT6573_BusHW.Clock[i][j].showlog)
               p += sprintf(p, "     [%d, 0x%x, %s]\r\n", i, j, g_MT6573_BusHW.Clock[i][j].name); 
        }
    } 
    
    p += sprintf(p, "  ClkEn log show:\n\r");
    for(i = 0; i < MT65XX_CLK_EN_COUNT_END; i++)
    {
        if(g_clken[i].showlog)
           p += sprintf(p, "     [%d, %s]\r\n", i, g_clken[i].name);
    }
    
    p += sprintf(p, "  subsys log show:\n\r");
    if(g_MT6573_BusHW.dwSubSystem_showlog)
       p += sprintf(p, "     [0x%x]\r\n", g_MT6573_BusHW.dwSubSystem_showlog);

    p += sprintf(p, "=====================================\n\r" );

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

static int
hwPMLogControl_Write (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32];
    UINT32 u4CopySize = 0;
    UINT32 type = 0;
    UINT32 id = 0;
    UINT32 enable = 1;
    int i = 0;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
                return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d %d %d",&type, &id,  &enable) == 3) {
        printk("modify log control: type=%d, id=%d, enable=%d\r\n", type, id, enable);
        if(type >= 0) 
        {
           switch(type)
           {
              case 0: 
                 set_clock_showlog(id, enable);
                 break;
              case 1: 
                 set_clkEn_showlog(id, enable);
                 break;
              case 2: 
                 set_subsys_showlog(id, enable);
                 break;
              case 10:
                 for(i = 0; i <= MT65XX_PDN_AUDIO_I2S; i++)
                    set_clock_showlog(i, enable);
                 break;
           }

        }else {
           printk("bad type:%d\r\n", type);
        }
    }
    return count;
}

static int
hwPMClockListen_Write (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize = 0;
    UINT32 enable = 1;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
		return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d", &enable) == 1) {
        set_clock_listen(enable);
    }
    return count;
}

static int hwPMClockListen_Read(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    printk("============show clock listen========\n\r" );
    show_clock_listen();
    printk("=====================================\n\r" );

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

static int
hwPMPLLControl_Write (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize = 0;
    UINT32 clk_en_type = 0;
    UINT32 enable = 1;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
		return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d %d", &clk_en_type, &enable) == 2) {
        hwSetPllCon_master(clk_en_type, (BOOL)enable, "CM_test");
    }
    return count;
}

static int hwPMPLLFsel_Read(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;
    int i = 0; 
    INT32 freq_value=0;
    for(i=0; i < MT65XX_PLL_COUNT_END; i++)
    {
        freq_value = PLL_Fsel_Read(i);
        if(freq_value == -1)
            p += sprintf(p, "[%d][%s]=[-1]\n\r", i, MT65XX_PLL_name[i]);
        else if(freq_value != -2)
            p += sprintf(p, "[%d][%s]=[0x%x]\n\r", i, MT65XX_PLL_name[i],freq_value);
    }

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}


static int
hwPMPLLFsel_Write (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize = 0;
    UINT32 pllId = 0;
    UINT32 pll_fsel = 0;


    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
		return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d %x", &pllId, &pll_fsel) == 2) {
        PLL_Fsel(pllId, (UINT16)pll_fsel);
    }
    return count;
}

static int
hwPMClockTypeWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize = 0;
    UINT32 clockId = 0;
    UINT32 clockType = 0;


    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
		return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d %d", &clockId, &clockType) == 2) {
        hwSetClockType(clockId, clockType, "clock manager");
    }
    return count;
}

static int
hwPMClockWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize;
    int i = 0, subSys = 0;
    DWORD category = 0, offset = 0,pll_mode; 
    UINT32 enableFirst = FALSE;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
      return 0;
    acBuf[u4CopySize] = '\0';

    //printk("scanf category clock staus\r\n");
    if (sscanf(acBuf, "%d %d %d %d %d", &category, &offset, &pll_mode,  &subSys, &enableFirst) == 5) 
    {
      printk("clock test: category[%d], offset[%d], pll_mode[%d], subSys[%d], enableFirst[%d]\r\n", category, offset, pll_mode, subSys, enableFirst);
       
      if(subSys >= MT65XX_SUBSYS_COUNT_END)//test all clock
      {
           printk("test all clock\r\n");
           for(i = 0; i < MT65XX_CLOCK_CATEGORY_COUNT; i++)
           {
               hwDoClockTest(i, (BOOL)enableFirst);
           }
      }
      else if(subSys < 0){//test one clock
          printk("test one clock\r\n");
          hwClockTestSlave(category, offset, (BOOL)enableFirst, pll_mode);

      }
      else{  //test clocks in one subsystem
          printk("test clocks in subSys[%s]\r\n", MT65XX_SUBSYS_name[subSys]);
          category = MT65XX__CATEGORY_OF_SUBSYS[subSys];
          hwDoClockTest(category, (BOOL)enableFirst);
      }  
      printk("finish clock gating test\r\n");
   }
   return count;
}
void hwDoClockTest(DWORD category, BOOL enableFirst){
  int j=0;
     for(j = 0; j < MT65XX_CLOCK_NUMBER[category]; j++)
     {
        hwClockTestSlave(category, j, enableFirst,1);
     }
     if(category== CLOCK_PERI_PDN0 || category==CLOCK_MMSYS1_PART0)
     {
         for(j=0; j<MT65XX_CLOCK_NUMBER[category+1]; j++)
         {
             hwClockTestSlave(category+1, j, enableFirst, 1);
         }
     }
}
void hwClockTestSlave(DWORD category, DWORD offset, BOOL enableFirst, DWORD pll_mode){
   if((category==CLOCK_AUDIOSYS) && (offset==0 || offset==1 || offset==3))//do not waste time on NOP
       return;
   if((category==CLOCK_PERI_PDN0) && (offset==12))//do not disable UART4 to stop logging
       return;
   if((category==CLOCK_PERI_PDN1) && (offset==1 || offset==2 || offset==4))//do not disable SIMIF0, SIMIF1, APDMA
       return;

   if(!enableFirst)
   {
       hwDisableClock_func(category, offset, "CG_TEST");
   }else{
       if(!isPllCanBeSwitched(g_MT6573_BusHW.Clock[category][offset].clock_type, MT65XX_CLOCK_TYPE_VALUE[g_MT6573_BusHW.Clock[category][offset].clock_type][pll_mode]))
       {
           return;
       }
       hwEnableClock_func(category, offset, "CG_TEST", pll_mode);
   }
   udelay(1000);

}
static int
hwPMPLLWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize;
    int i = 0,time=0, PLLId = 0;
    DWORD total_time;
    UINT32 enable = TRUE;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
        return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d %d %d", &enable, &PLLId, &total_time) == 3) {
       printk("PLLId[%d], enable[%d], total_time[%d]\r\n", PLLId, enable, total_time);
       if(PLLId < 0 || PLLId >= MT65XX_PLL_COUNT_END)
       {
           printk("test all PLL\r\n");
           while(time < total_time)
           {
               printk("the %d time test\r\n", time);
               for(i = 0; i < MT65XX_PLL_COUNT_END; i++)
               {
                   hwDoPLLTest(i, (BOOL)enable, time);
               }
               time++;
           }
        }else
        {
           printk("test one PLL[%s]\r\n", MT65XX_PLL_name[PLLId]);
           while(time < total_time)
           {
               printk("the %d time test\r\n", time);
               hwDoPLLTest(PLLId, (BOOL)enable, time);
               time++;
           }
        }
        printk("finish PLL test\r\n");
    }
    return count;
}
void hwDoPLLTest(DWORD pllId, BOOL enable, DWORD time)
{
    if(!enable) 
    {
        if(time%2)
           hwEnablePLL(pllId, "PLL_TEST");
        else
           hwDisablePLL(pllId, "PLL_TEST");
    }else{
        if(time%2)
           hwDisablePLL(pllId, "PLL_TEST");
        else
           hwEnablePLL(pllId, "PLL_TEST");
    }

}
static int
hwPMPowerWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize;
    UINT32 powerStatus; 
    int i = 0,time=0;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
        return 0;
    acBuf[u4CopySize] = '\0';

    printk("scanf power staus\r\n");
    if (sscanf(acBuf, "%x", &powerStatus) == 1) {
       while(time < 10)
       {
           printk("the %d time test\r\n", time);
           for(i = 0; i < MT65XX_POWER_COUNT_END; i++)
           {
               if((time%2) && (powerStatus & (1<<i)) )
                   hwPowerOn(i, 0,"Power_Test");
               else
                   hwPowerDown(i, "Power_Test");
           }
           time++;
       }
    }
    printk("finish power test\r\n");
    return 1;
}

static int
hwPMSubSystemWrite (
    struct file *file,
    const char *buffer,
    unsigned long count,
    void *data
    )
{
    char acBuf[32]; 
    UINT32 u4CopySize;
    int i = 0,time=0, subSys;
    DWORD total_time;
    UINT32 enable = TRUE;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
        return 0;
    acBuf[u4CopySize] = '\0';

    if (sscanf(acBuf, "%d %d %d", &enable, &subSys, &total_time) == 3) {
       printk("subSys[%d], enable[%d], total_time[%d]\r\n", subSys, enable, total_time);
       if(subSys < 0 || subSys >= MT65XX_SUBSYS_COUNT_END)
       {
           printk("test all subSys\r\n");
           while(time < total_time)
           {
               printk("the %d time test\r\n", time);
               for(i = 0; i < MT65XX_SUBSYS_COUNT_END; i++)
               {
                   hwDoSubSysTest(i, (BOOL)enable, time);
               }
               time++;
           }
        }else
        {
           printk("test one subSys[%s]\r\n", MT65XX_SUBSYS_name[subSys]);
           while(time < total_time)
           {
               printk("the %d time test\r\n", time);
               hwDoSubSysTest(subSys, (BOOL)enable, time);
               time++;
           }
        }
        printk("finish subSystem test\r\n");
    }
    return count;
}
void hwDoSubSysTest(DWORD subSys, BOOL enable, DWORD time)
{
    if(!enable) 
    {
        if(time%2)
           hwEnableSubsys(subSys);
        else
           hwDisableSubsys(subSys);
    }else{
        if(time%2)
           hwDisableSubsys(subSys);
        else
           hwEnableSubsys(subSys);
    }

}


void mt6573_log_init(void)
{
    UINT32 k, i,j,u4Status;
    struct proc_dir_entry *prEntry;

    /* Clear all parameters*/
    //printk("[%s]: init clock...\r\n",__FUNCTION__);
    for(i = 0; i < MT65XX_CLOCK_CATEGORY_COUNT; i++)
    {
        for(j = 0; j < MT65XX_CLOCK_NUMBER[i]; j++)
        {
            /* Set reference counter */
            g_MT6573_BusHW.Clock[i][j].dwClockCount = 0;		
            sprintf(g_MT6573_BusHW.Clock[i][j].name , "%s", NON_OP);
            /* Set Master name*/
            for (k = 0; k< MAX_DEVICE; k++)
            {
                sprintf(g_MT6573_BusHW.Clock[i][j].mod_name[k] , "%s", NON_OP);
            }
            /* Set category */
            //g_MT6573_BusHW.Clock[i][j].category = i;
            /* Set mode */
            //g_MT6573_BusHW.Clock[i][j].mode = 1<<j;
            /* Set name */
            getClock_Name(i, j, g_MT6573_BusHW.Clock[i][j].name);   
            /* Set default on */
            if(!CG_Status(i, 1<<j))
            {
                g_MT6573_BusHW.Clock[i][j].bDefault_on = TRUE;
                //g_MT6573_BusHW.Clock[i][j].dwClockCount = 1;
                //sprintf(g_MT6573_BusHW.Clock[i][j].mod_name[0] , "DEFAULT");
            }
            else 
                g_MT6573_BusHW.Clock[i][j].bDefault_on = FALSE;
            /* Set pll source */
            g_MT6573_BusHW.Clock[i][j].clock_type = getPllSourceOfClock(i, (1<<j));
            /* Set active pll source */
            g_MT6573_BusHW.Clock[i][j].active_pll_source = -1;
            g_MT6573_BusHW.Clock[i][j].showlog = FALSE;
        }
    }
   
    /* init g_clken*/
    for(i = 0; i < MT65XX_CLK_EN_COUNT_END; i++)
    {
        g_clken[i].dwClkEnCount = 0;
        g_clken[i].showlog= FALSE;
        g_clken[i].reg = MT65XX_CLK_EN_REG[i][0];
        g_clken[i].mask = MT65XX_CLK_EN_REG[i][1];
        
        sprintf(g_clken[i].name, "%s", MT65XX_CLK_EN_name[i]);
        for(k=0; k < MAX_DEVICE; k++)
        { 
            sprintf(g_clken[i].mod_name[k], "%s", NON_OP);
        }
    }
      
    /*init  power */
    //printk("[%s]: init power...\r\n",__FUNCTION__);
    
    for ( i=MT65XX_POWER_LDO_VA28; i < MT65XX_POWER_COUNT_END; i++ )
    {   
        g_MT6573_BusHW.Power[i].dwPowerCount = 0;
        g_MT6573_BusHW.Power[i].bDefault_on = FALSE;
		for (j = 0; j< MAX_DEVICE; j++)
		{
        sprintf(g_MT6573_BusHW.Power[i].mod_name[j] , "%s", NON_OP);
		}
        sprintf(g_MT6573_BusHW.Power[i].name , "%s", MT65XX_PMU_name[i]);
    }
    /*init pll*/
    //printk("[%s]: init pll...\r\n",__FUNCTION__);
    for ( i=MT65XX_APMCU_PLL; i < MT65XX_PLL_COUNT_END; i++ )
    {
        g_MT6573_BusHW.Pll[i].dwPllCount = 0;
		
        for (j = 0; j< MAX_DEVICE; j++)
        {
            sprintf(g_MT6573_BusHW.Pll[i].mod_name[j] , "%s", NON_OP);
        }
        sprintf(g_MT6573_BusHW.Pll[i].name , "%s", MT65XX_PLL_name[i]);
		
        if(i == MT65XX_26M_PLL)
            g_MT6573_BusHW.Pll[i].bDefault_on = TRUE;
        else if(PLL_Status(i))
        {
            g_MT6573_BusHW.Pll[i].bDefault_on = TRUE;
            //g_MT6573_BusHW.Pll[i].dwPllCount = 1;
            //sprintf(g_MT6573_BusHW.Pll[i].mod_name[0] , "DEFAULT")
        }
        else
            g_MT6573_BusHW.Pll[i].bDefault_on = FALSE;
    }

    /*init dwSubSystem*/
    //printk("[%s]: init subSystem...\r\n",__FUNCTION__);
    g_MT6573_BusHW.dwSubSystem_status = 0;
    g_MT6573_BusHW.dwSubSystem_defaultOn = 0;	
    for ( i = MD2G_SUBSYS; i < MT65XX_SUBSYS_COUNT_END; i++ )
    {
        g_MT6573_BusHW.dwSubSystem_clock[i] = 0;//add by Cui Zhang
        u4Status = DRV_Reg32(RM_PWR_CON0+i*4);
        if(u4Status & (1<<2))
        g_MT6573_BusHW.dwSubSystem_defaultOn |= (1<<i) ;
    }	
	
    for(i=0; i< MT65XX_CLOCK_TYPE_COUNT; i++)
    {
        g_MT6573_BusHW.dwClock_active_pll[i] = MT65XX_CLOCK_TYPE_VALUE[i][1];//default pll is MT65XX_CLOCK_TYPE_VALUE[clock_type][1]
    }

    /* VA28 (VTCXO) */
    u4Status = DRV_Reg32(VA28_CON0);
    if(u4Status & (1<<1)) /*Enable by SW*/
    {
        if(u4Status & (1<<0))
            g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VA28].bDefault_on = TRUE;
    }
    else /*Enable by SCLKEN*/
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VA28].bDefault_on = TRUE;
    
    /* VA25  */
    u4Status = DRV_Reg32(VA25_CON0);
    if(u4Status & (1<<1)) /*Enable by SW*/
    {
        if(u4Status & (1<<0))
            g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VA25].bDefault_on = TRUE;
    }
    else /*Enable by SCLKEN*/
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VA25].bDefault_on = TRUE;

    /* VA12  */
    u4Status = DRV_Reg32(VA12_CON0);
    if(u4Status & (1<<1)) /*Enable by SW*/
    {
        if(u4Status & (1<<0))
            g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VA12].bDefault_on = TRUE;
    }
    else /*Enable by SCLKEN*/
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VA12].bDefault_on = TRUE;

    /* VRTC  */
    u4Status = DRV_Reg32(VRTC_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VRTC].bDefault_on = TRUE;

    /* VMIC  */
    u4Status = DRV_Reg32(VMIC_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VMIC].bDefault_on = TRUE;

    /* VTV */
    u4Status = DRV_Reg32(VTV_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VTV].bDefault_on = TRUE;

    /* VAUDN */
    u4Status = DRV_Reg32(VAUDN_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VAUDN].bDefault_on = TRUE;

    /* VAUDP */
    u4Status = DRV_Reg32(VAUDP_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VAUDP].bDefault_on = TRUE;

    /* PMUA */
    g_MT6573_BusHW.Power[MT65XX_POWER_LDO_PMUA].bDefault_on = TRUE;
    
    /* VRF */
    u4Status = DRV_Reg32(VRF_CON0);
    if(u4Status & (1<<1)) /*Enable by SW*/
    {
        if(u4Status & (1<<0))
            g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VRF].bDefault_on = TRUE;
    }
    else /*Enable by SCLKEN*/
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VRF].bDefault_on = TRUE;

    /* VCAMA */
    u4Status = DRV_Reg32(VCAMA_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VCAMA].bDefault_on = TRUE;

    /* VCAMD */
    u4Status = DRV_Reg32(VCAMD_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VCAMD].bDefault_on = TRUE;

    /* VIO */
    u4Status = DRV_Reg32(VIO_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VIO].bDefault_on = TRUE;

    /* VUSB */
    u4Status = DRV_Reg32(VUSB_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VUSB].bDefault_on = TRUE;

    /* VSIM */
    u4Status = DRV_Reg32(VSIM_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VSIM].bDefault_on = TRUE;

    /* VSIM2 */
    u4Status = DRV_Reg32(VSIM2_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VSIM2].bDefault_on = TRUE;

    /* VIBR */
    u4Status = DRV_Reg32(VIBR_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VIBR].bDefault_on = TRUE;

    /* VMC */
    u4Status = DRV_Reg32(VMC_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VMC].bDefault_on = TRUE;

    /* VCAMA2 */
    u4Status = DRV_Reg32(VCAMA2_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VCAMA2].bDefault_on = TRUE;

    /* VCAMD2 */
    u4Status = DRV_Reg32(VCAMD2_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VCAMD2].bDefault_on = TRUE;

    /* VM12 */
    u4Status = DRV_Reg32(VM12_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VM12].bDefault_on = TRUE;

    /* VM12_INT */
    u4Status = DRV_Reg32(VM12_INT_CON0);
    if(u4Status & (1<<0))
        g_MT6573_BusHW.Power[MT65XX_POWER_LDO_VM12_INT].bDefault_on = TRUE;
    finish_log_init = TRUE;

    /* Register dump1*/
    prEntry = create_proc_read_entry("pm_reg1", S_IRUGO, NULL, hwPMRegDump1, NULL);	
    /* Register dump2*/
    prEntry = create_proc_read_entry("pm_reg2", S_IRUGO, NULL, hwPMRegDump2, NULL);	
    /* Device model*/
    prEntry = create_proc_read_entry("pm_stat", S_IRUGO, NULL, hwPMStat, NULL);	
    /* Default*/
    prEntry = create_proc_read_entry("pm_default", S_IRUGO, NULL, hwPMDefault, NULL);	
    /* Log Setting */

    prEntry = create_proc_entry("pm_flag", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMFlagRead;
        prEntry->write_proc = hwPMFlagWrite;
    }

    prEntry = create_proc_entry("pm_clock_test", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMClockRead;
        prEntry->write_proc = hwPMClockWrite;
    }
    /* power test */
    prEntry = create_proc_entry("pm_power_test", 0, NULL);
    if (prEntry) {
        prEntry->write_proc = hwPMPowerWrite;
    }
    /* pll test */
    prEntry = create_proc_entry("pm_pll_test", (S_IRUGO | S_IWUGO), NULL);
    if (prEntry) {
        prEntry->write_proc = hwPMPLLWrite;
    }
    /* subSystem test */
    prEntry = create_proc_entry("pm_subsystem_test", 0, NULL);
    if (prEntry) {
        prEntry->write_proc = hwPMSubSystemWrite;
    }
    /* pll_fsel test */
    prEntry = create_proc_entry("pm_pll_fsel", (S_IRUGO | S_IWUGO), NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMPLLFsel_Read;
        prEntry->write_proc = hwPMPLLFsel_Write;
    }
    /* set_clock_type test */
    prEntry = create_proc_entry("pm_set_clock_type", 0, NULL);
    if (prEntry) {
        prEntry->write_proc = hwPMClockTypeWrite;
    }
    /* set_pll_control test */
    prEntry = create_proc_entry("pm_pll_control", 0, NULL);
    if (prEntry) {
        prEntry->write_proc = hwPMPLLControl_Write;
    }
    /* clock_listen control */
    prEntry = create_proc_entry("pm_clock_listen", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMClockListen_Read;
        prEntry->write_proc = hwPMClockListen_Write;
    }
    /* log_show_control */
    prEntry = create_proc_entry("pm_log_control", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMLogControl_Read;
        prEntry->write_proc = hwPMLogControl_Write;
    }
	/* early_suspend_control */
    prEntry = create_proc_entry("pm_early_suspend_control", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMESControl_Read;
        prEntry->write_proc = hwPMESControl_Write;
    }
    /* help */
    prEntry = create_proc_read_entry("pm_help", S_IRUGO, NULL, hwPMTestHelp, NULL);	
    /* Test proc*/
    prEntry = create_proc_entry("pm_spare", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = hwPMSpareRead;
        prEntry->write_proc = hwPMSpareWrite;
    }

}    

