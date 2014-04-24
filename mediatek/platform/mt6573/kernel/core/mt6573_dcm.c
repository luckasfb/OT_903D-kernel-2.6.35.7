
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/io.h>
#include <asm/mach/irq.h>
#include "mach/sync_write.h"

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/sync_write.h"
#include "mach/mt6573_typedefs.h"
#include "mach/mt6573_pll.h"
#include "mach/mt6573_boot.h"
#include "mach/mt6573_wdt.h"
#include "mach/mt6573_ost_sm.h"
#include "mach/mt6573_gpt.h"
#include "mach/mt6573_wdt.h"
#include "mach/mtk_rtc.h"
#include "pmu6573_hw.h"
#include "pmu6573_sw.h"

#define DEEP_IDLE_LOG 1
#define OST_NAME	"mt6573-ost"

#define dcm_attr(_name) \
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = 0644,			\
	},					\
	.show	= _name##_show,			\
	.store	= _name##_store,		\
}
/* Debug message event */
#define DBG_PMAPI_MASK       (CM_DBG_FLAG)

#define MSG(evt, fmt, args...) \
do {    \
    if ((DBG_PMAPI_##evt) & DBG_PMAPI_MASK) { \
        printk(fmt, ##args); \
    } \
} while(0)

//deep idle condition 
#define g_APMCU0_DEEP_IDLE_E1 0x0dff	
#define g_APMCU1_DEEP_IDLE_E1 0x0388
#define g_MM1_CON0_DEEP_IDLE_E1 0xbffff7fe
#define g_MM1_CON1_DEEP_IDLE_E1  0x7f
#define g_MM2_DEEP_IDLE_E1 0x3e
#define g_AUDIO_DEEP_IDLE_E1 0x0

#define g_APMCU0_DEEP_IDLE_E2 0xdff	
#define g_APMCU1_DEEP_IDLE_E2 0x388
#define g_MM1_CON0_DEEP_IDLE_E2 0xbffff7fe
#define g_MM1_CON1_DEEP_IDLE_E2  0x7f
#define g_MM2_DEEP_IDLE_E2 0x3e
#define g_AUDIO_DEEP_IDLE_E2 0x0


//dcm condition
#define g_APMCU0_SLOW_IDLE_E1 0x0c0f	
#define g_APMCU1_SLOW_IDLE_E1 0x0008
#define g_MM1_CON0_SLOW_IDLE_E1 0xfffffffe
#define g_MM1_CON1_SLOW_IDLE_E1 0xff
#define g_MM2_SLOW_IDLE_E1 0x26
#define g_AUDIO_SLOW_IDLE_E1 0x0

#define g_APMCU0_SLOW_IDLE_E2 0x0c0f	
#define g_APMCU1_SLOW_IDLE_E2 0x0008
#define g_MM1_CON0_SLOW_IDLE_E2 0x0
#define g_MM1_CON1_SLOW_IDLE_E2 0x0
#define g_MM2_SLOW_IDLE_E2 0x04
#define g_AUDIO_SLOW_IDLE_E2 0x0

#define VER_E1 0
#define VER_E2 1
UINT32 g_ver;
#define MT65XX_TARGET_COUNT_END MT65XX_CLOCK_CATEGORY_COUNT
#define TEST_ARCH_IDLE 0
#define TEST_STATE_MASK 1
#define TEST_DEVICE_ABILITY 2
#define TEST_OUT_REASON 3
#define TEST_TIME_CRITERIA 4
#define TEST_WAKEUP_DELAY 5
#define DEBUG_STATE_MASK 1

#define FRC_CON_EN		(0x1 << 0)
#define FRC_CON_KEY		(0x6573 << 16)

#define OST_CON_EN		(0x1 << 0)
#define OST_CON_UFN_DOWN	(0x1 << 1)

#define OST_CMD_PAUSE_STR	(0x1 << 0)
#define OST_CMD_OST_WR		(0x1 << 2)
#define OST_CMD_OST_RD		(0x1 << 1)
#define OST_CMD_UFN_WR		(0x1 << 13)
#define OST_CMD_AFN_WR		(0x1 << 14)
#define OST_CMD_CON_WR		(0x1 << 15)
#define OST_CMD_KEY		    (0x6573 << 16)

#define OST_STA_CMD_CPL		(0x1 << 1)
#define LOW_BAT_ALARM       3400
#define LOW_BAT_LB          2800
#define VBAT_CHANNEL 	1
#define VBAT_COUNT 		3

#define RET_WAKE_REQ			0x1
#define RET_WAKE_INTR			0x2
#define RET_WAKE_TIMEOUT		0x3
#define RET_WAKE_ABORT			0x4
#define RET_WAKE_OTHERS			0x5
#define R_BAT_SENSE             0x2

typedef enum MT6573_DIVISOR 
{
    FSEL_DIV_1,
    FSEL_DIV_2,
    FSEL_DIV_4, 
    FSEL_DIV_8,
    FSEL_DIV_16,
    FSEL_DIV_32,
} MT6573_DIVISOR_SEL;

typedef enum MT6573_CLOCK 
{
    CLOCK_x2,
    CLOCK_x4,
    CLOCK_x8,
    CLOCK_x16,
    CLOCK_x32,
} MT6573_CLOCK_SEL;

DWORD slow_idle_working_ability_mask_default[][MT65XX_CLOCK_CATEGORY_COUNT] = {//IRDA; 1 means can not work
    {g_APMCU0_SLOW_IDLE_E1, g_APMCU1_SLOW_IDLE_E1, g_MM1_CON0_SLOW_IDLE_E1, g_MM1_CON1_SLOW_IDLE_E1, g_MM2_SLOW_IDLE_E1, g_AUDIO_SLOW_IDLE_E1},
    {g_APMCU0_SLOW_IDLE_E2, g_APMCU1_SLOW_IDLE_E2, g_MM1_CON0_SLOW_IDLE_E2, g_MM1_CON1_SLOW_IDLE_E2, g_MM2_SLOW_IDLE_E2, g_AUDIO_SLOW_IDLE_E2}
};

DWORD deep_idle_working_ability_mask_default[][MT65XX_CLOCK_CATEGORY_COUNT] = //USB, I2C; 1 means can not work
{
    {g_APMCU0_DEEP_IDLE_E1, g_APMCU1_DEEP_IDLE_E1, g_MM1_CON0_DEEP_IDLE_E1, g_MM1_CON1_DEEP_IDLE_E1, g_MM2_DEEP_IDLE_E1, g_AUDIO_DEEP_IDLE_E1},
    {g_APMCU0_DEEP_IDLE_E2, g_APMCU1_DEEP_IDLE_E2, g_MM1_CON0_DEEP_IDLE_E2, g_MM1_CON1_DEEP_IDLE_E2, g_MM2_DEEP_IDLE_E2, g_AUDIO_DEEP_IDLE_E2}
};

DWORD slow_idle_working_ability_mask[][MT65XX_CLOCK_CATEGORY_COUNT] = {//IRDA; 1 means can not work
    {g_APMCU0_SLOW_IDLE_E1, g_APMCU1_SLOW_IDLE_E1, g_MM1_CON0_SLOW_IDLE_E1, g_MM1_CON1_SLOW_IDLE_E1, g_MM2_SLOW_IDLE_E1, g_AUDIO_SLOW_IDLE_E1},
    {g_APMCU0_SLOW_IDLE_E2, g_APMCU1_SLOW_IDLE_E2, g_MM1_CON0_SLOW_IDLE_E2, g_MM1_CON1_SLOW_IDLE_E2, g_MM2_SLOW_IDLE_E2, g_AUDIO_SLOW_IDLE_E2}
};

DWORD deep_idle_working_ability_mask[][MT65XX_CLOCK_CATEGORY_COUNT] = //USB, I2C; 1 means can not work
{
    {g_APMCU0_DEEP_IDLE_E1, g_APMCU1_DEEP_IDLE_E1, g_MM1_CON0_DEEP_IDLE_E1, g_MM1_CON1_DEEP_IDLE_E1, g_MM2_DEEP_IDLE_E1, g_AUDIO_DEEP_IDLE_E1},
    {g_APMCU0_DEEP_IDLE_E2, g_APMCU1_DEEP_IDLE_E2, g_MM1_CON0_DEEP_IDLE_E2, g_MM1_CON1_DEEP_IDLE_E2, g_MM2_DEEP_IDLE_E2, g_AUDIO_DEEP_IDLE_E2}
};

static spinlock_t device_working_state_lock = SPIN_LOCK_UNLOCKED;
static UINT32 wakeup_delay = 138;

char sys_state_name[][MAX_MOD_NAME] = {"NORMAL",  "IDLE", "LITTLE_IDLE", "SLOW_IDLE", "DEEP_IDLE", "ALL_STATE"};
char pause_status_name[][MAX_MOD_NAME] = {"Wait CP15", "Pause active", "Failed with wakesrc", "Faild with UFN<2"};

unsigned int aparm_normal_clock[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };
unsigned int mdarm_normal_clock[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };

unsigned int fbus_normal_clock[] = { 0x00,  0x01, 0x03, 0x07, 0x0F };
unsigned int fbus_idle_clock[] = { 0x00, 0x01, 0x03, 0x07, 0x0F };

unsigned int sbus_normal_clock[] = { 0x00, 0x01, 0x03, 0x07 };
unsigned int sbus_idle_clock[] = {  0x00, 0x01, 0x03, 0x07 };

unsigned int emi_normal_clock[] = { 0x00, 0x01 , 0x03, 0x07, 0x0F, 0x1F };
unsigned int emi_idle_clock[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };

unsigned int dsp_normal_clock[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };
unsigned int dsp_idle_clock[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };

unsigned int emi_dbc[] = { 0x00, 0x17F };
unsigned int bus_dbc[] = { 0x101, 0x17F };

BOOL enter = FALSE;
UINT32 time_criteria = 58;

static UINT32 sys_last_state = NORMAL_STATE;
static CM_DCM_CFG_T dcm_cfg;
DWORD bStateMask = 0x0;//normal, idle, little_idle, slow_idle, deep_idle

/*Extern variable */ 
extern BOOL isSubsysCanDisable(DWORD sysId);
extern char MT65XX_CATEGORY_name[MT65XX_CLOCK_CATEGORY_COUNT][MAX_MOD_NAME];
extern void mt6573_wdt_ModeSelection(kal_bool en, kal_bool auto_rstart, kal_bool IRQ);
extern void enable_low_power_settings(void);
extern void disable_low_power_settings(void);
extern BOOL valid_clockId(int clockId);
extern int XGPT_Set_Next_Compare(unsigned long cycles);
extern struct kobject *power_kobj;

int factory_idle_current_init(void);

void MT6573_DCM_Config(CM_DCM_CFG_T *dcm_cfg)
{
#if 0
    /* Disable IRQ */
    DisableIRQ();
    /* Disable L1/L2 cache */
    CM_Disable_L1L2_DCache_ICache_MMU();
    /* Block all EMI accesses */
    *EMI_CONM = 0x0000007F;
    /* Disable Dummy Read function */
    *EMI_DRCT &= ~(1);
    /* Poll assertion of EMI IDLE bit */
    while ((*EMI_CONN >> 10) & 0x1 == 0) 
    /* Wait at least 1 EMI cycle */
    for(delay=0; delay<100; delay++);
    *MDARM_CCTL &= 0xFFFFFFE0;
    *MDARM_CCTL |= dcm_cfg->mdarm_fsel;
#endif

    /* Program FSEL dividers */

    //*APARM_FSEL = dcm_cfg->aparm_fsel;
    DRV_WriteReg32(APARM_FSEL,dcm_cfg->aparm_fsel);
    //*EMI_FSEL = dcm_cfg->emi_fsel;
    DRV_WriteReg32(EMI_FSEL,dcm_cfg->emi_fsel);
    //*EMI_IDLE_FSEL = dcm_cfg->emi_idle_fsel;
    DRV_WriteReg32(EMI_IDLE_FSEL,dcm_cfg->emi_idle_fsel);
    //*FBUS_FSEL = dcm_cfg->fbus_fsel;
    DRV_WriteReg32(FBUS_FSEL,dcm_cfg->fbus_fsel);
    //*FBUS_IDLE_FSEL = dcm_cfg->fbus_idle_fsel;
    DRV_WriteReg32(FBUS_IDLE_FSEL,dcm_cfg->fbus_idle_fsel);
    //*SBUS_FSEL = dcm_cfg->sbus_fsel;
    DRV_WriteReg32(SBUS_FSEL,dcm_cfg->sbus_fsel);
    //*SBUS_IDLE_FSEL = dcm_cfg->sbus_idle_fsel;
    DRV_WriteReg32(SBUS_IDLE_FSEL,dcm_cfg->sbus_idle_fsel);
    /*Do not touch MD site*/
#if 0
    *DSP_FSEL = dcm_cfg->dsp_fsel;
    *DSP_IDLE_FSEL = dcm_cfg->dsp_idle_fsel;
#endif
    //DRV_WriteReg32(RG_EMI_DBC,dcm_cfg->rg_emi_dbc);
    DRV_WriteReg32(RG_BUS_DBC,dcm_cfg->rg_bus_dbc);
#if 0
    /* Unblock all EMI accesses */
    *EMI_CONM = 0x00000000;
    /* Enable Dummy Read function */
    *EMI_DRCT |= (1);
    /* Diable L1/L2 cache*/
    CM_Enable_L1L2_DCache_ICache_MMU();
    /* Enable IRQ */
    EnableIRQ();    
    /* Program the EMI_ABCT register (CLK_FREQ field) based on the EMI_IDLE_FSEL setting */
    MT6573_Program_EMI_ABCT();
#endif
}


void MT6573_ENABLE_DCM_EN_IRQ_CLR(void)
{
    DRV_SetReg16(RG_CK_DCM_EN, (1<<7));
}

void MT6573_DISABLE_DCM_EN_IRQ_CLR(void)
{
    DRV_ClrReg16(RG_CK_DCM_EN,(1<<7));
}

inline void MT6573_ENABLE_HW_DCM_AP(void)
{
    DRV_SetReg16(RG_CK_DCM_EN,0x3);
}
inline void MT6573_DISABLE_HW_DCM_AP(void)
{
    DRV_ClrReg16(RG_CK_DCM_EN,0x3);
}

void MT6573_FBUSCLK_DIS(bool disable)
{
    if(disable)
       DRV_SetReg32(AP_SLPPRT_BUS, (1<<15));
    else
       DRV_ClrReg32(AP_SLPPRT_BUS, (1<<15));
}

void MT6573_INIT_IDLE_THREAD(void)
{
    if(get_chip_eco_ver() == CHIP_VER_E1)
        g_ver = VER_E1;
	  else
        g_ver = VER_E2;
	
    /* ARM and MD normal mode, div 1*/
    dcm_cfg.aparm_fsel = aparm_normal_clock[FSEL_DIV_1];
    dcm_cfg.mdarm_fsel = mdarm_normal_clock[FSEL_DIV_1];
    /* EMI Idle mode, div1 -> div32 */
    dcm_cfg.emi_fsel = emi_normal_clock[FSEL_DIV_1];
    dcm_cfg.emi_idle_fsel = emi_idle_clock[FSEL_DIV_32];
    /* AHB fast bus Idle mode, 3.84Mhz*32 -> 3.84Mhz*2 */
    dcm_cfg.fbus_fsel = fbus_normal_clock[CLOCK_x32];
    dcm_cfg.fbus_idle_fsel = fbus_idle_clock[CLOCK_x4];
    /* AHB slow bus Idle mode, 3.84Mhz*16 -> 3.84Mhz*2 */
    dcm_cfg.sbus_fsel = sbus_normal_clock[CLOCK_x16];
    dcm_cfg.sbus_idle_fsel = sbus_idle_clock[CLOCK_x4];
    /* DSP Idle mode, div1 -> div1 */
    dcm_cfg.dsp_fsel = dsp_normal_clock[FSEL_DIV_1];
    dcm_cfg.dsp_idle_fsel = dsp_idle_clock[FSEL_DIV_1];
    /* Set Debouce Max to prevent fake idle status*/
    //dcm_cfg.rg_emi_dbc = emi_dbc[1];
    dcm_cfg.rg_bus_dbc = bus_dbc[0];
    /* Disable the DCM_EN_IRQ_CLR feature.*/
    if(g_ver == VER_E2)
    {
        time_criteria = 58;
        MT6573_FBUSCLK_DIS(true);//set this, so that clock in multimedia memory won't slow down when bus idle
    }
    else 
    {
        time_criteria = 210;
    }
    MT6573_DISABLE_DCM_EN_IRQ_CLR();
    /* Set to HW*/
    MT6573_DCM_Config(&dcm_cfg);
    // for factory mode test.
    factory_idle_current_init();
}

BOOL valid_dcm_state(MT6573_STATE state)
{
    if((state <= ALL_STATE) && (state >= NORMAL_STATE))
         return TRUE;
    else 
    {
         return FALSE;
    }
}

void dcm_enable_state(MT6573_STATE state)
{
     if(!valid_dcm_state(state))
         return;
     if(state == ALL_STATE)
         bStateMask = 0x0;
     else
         bStateMask &= ~(0x1 << state);
     printk("[%s]: state[%s] has been enabled, bStateMask=0x%x\r\n", __FUNCTION__, sys_state_name[state], bStateMask);
}

void dcm_disable_state(MT6573_STATE state)
{
     if(!valid_dcm_state(state))
         return;

     if(state == ALL_STATE)
         bStateMask = 0x1f;
     else
     {
         bStateMask |= (0x1 << state);
     }
     printk("[%s]: state[%s] has been disabled, bStateMask=0x%x\r\n", __FUNCTION__, sys_state_name[state], bStateMask);
}

static void reset_device_working_ability(MT6573_STATE state)
{     
    int i = 0; 

     if(!valid_dcm_state(state))
         return;

    spin_lock(&device_working_state_lock);
    if((state == DEEP_IDLE_STATE) || (state == ALL_STATE)) 
    {
       for(i=0; i<MT65XX_CLOCK_CATEGORY_COUNT; i++)
           deep_idle_working_ability_mask[g_ver][i] = deep_idle_working_ability_mask_default[g_ver][i];
    }

    if((state == SLOW_IDLE_STATE) || (state == ALL_STATE)) 
    {
       for(i=0; i<MT65XX_CLOCK_CATEGORY_COUNT; i++)
           slow_idle_working_ability_mask[g_ver][i] = slow_idle_working_ability_mask_default[g_ver][i];
    }
    spin_unlock(&device_working_state_lock);

    printk("[%s]: all device's working ability under state[%s] has been reset\r\n", __FUNCTION__, sys_state_name[state]);
}
//notify that, clockId can not work under state
void clr_device_working_ability(UINT32 clockId, MT6573_STATE state)
{     
    INT32 category = 0, offset;
	 
    if(!valid_dcm_state(state))
         return;

    if (!valid_clockId(clockId))
    {
        return;
    }	
	 
    category = clockId / 32;
    offset = clockId %32;
    spin_lock(&device_working_state_lock);
	 
    if(state == DEEP_IDLE_STATE)
        deep_idle_working_ability_mask[g_ver][category] |= (0x1 << offset);
    else if(state == SLOW_IDLE_STATE)
        slow_idle_working_ability_mask[g_ver][category] |= (0x1 << offset);
    else if(state == ALL_STATE)
    {
        deep_idle_working_ability_mask[g_ver][category] |= (0x1 << offset);
        slow_idle_working_ability_mask[g_ver][category] |= (0x1 << offset);
    }
	 
    spin_unlock(&device_working_state_lock);
}
EXPORT_SYMBOL(clr_device_working_ability);

//notify that, clockId can work under state
void set_device_working_ability(UINT32 clockId, MT6573_STATE state)
{
    INT32 category = 0, offset;
		 
    if(!valid_dcm_state(state))
         return;

    if (!valid_clockId(clockId))
    {
        return;
    }	
	 
    category = clockId / 32;
    offset = clockId %32;
    spin_lock(&device_working_state_lock);
	 
    if(state == DEEP_IDLE_STATE)
        deep_idle_working_ability_mask[g_ver][category] &= ~(0x1 << offset);
    else if(state == SLOW_IDLE_STATE)
        slow_idle_working_ability_mask[g_ver][category] &= ~(0x1 << offset);
    else if(state == ALL_STATE)
    {
        deep_idle_working_ability_mask[g_ver][category] &= ~(0x1 << offset);
        slow_idle_working_ability_mask[g_ver][category] &= ~(0x1 << offset);
    }
	 
    spin_unlock(&device_working_state_lock);
}
EXPORT_SYMBOL(set_device_working_ability);

BOOL get_device_working_ability(UINT32 clockId, MT6573_STATE state)
{
    INT32 category = 0, offset;
    BOOL available = FALSE;
	 	 
    if(!valid_dcm_state(state))
         return FALSE;

    if (!valid_clockId(clockId))
    {
        return FALSE;
    }	
	 
    category = clockId / 32;
    offset = clockId %32;
    spin_lock(&device_working_state_lock);
	 
    if(state == DEEP_IDLE_STATE)
    {
        available = !(deep_idle_working_ability_mask[g_ver][category] & (0x1 << offset));
    }
    else
    {
        available = !(slow_idle_working_ability_mask[g_ver][category] & (0x1 << offset));
    }
	 	
    spin_unlock(&device_working_state_lock);

    return available;
}
EXPORT_SYMBOL(get_device_working_ability);


void display_out_reason_of_little_idle(void)
{
#ifdef DEBUG_STATE_MASK
    if(bStateMask & (0x1 << LITTLE_IDLE_STATE))
    {
        printk("  %s is blocked by mask\r\n", sys_state_name[LITTLE_IDLE_STATE]);
        return;
    }
#endif
    printk("  %s is blocked by nothing\r\n", sys_state_name[LITTLE_IDLE_STATE]);
}

void display_out_reason_of_slow_idle(void)
{
    int i = 0;
    DWORD g_SLOW_IDLE_BLOCK = 0;
	  
#ifdef DEBUG_STATE_MASK
    if(bStateMask & (0x1 << SLOW_IDLE_STATE))
    {
        printk("  %s is blocked by mask\r\n", sys_state_name[SLOW_IDLE_STATE]);
        return;
    }
#endif
    for(i=0; i<MT65XX_TARGET_COUNT_END; i++)
    {
        if(isSubsysCanDisable(MT65XX_SUBSYS_OF_CATEGORY[i]))
        {
            if(DRV_Reg32(TOPSM_BASE+0x800 + MT65XX_SUBSYS_OF_CATEGORY[i] * 4) == 0x19a)
	         continue;
            if((g_ver == VER_E2) && (MT65XX_SUBSYS_OF_CATEGORY[i] == MM1_SUBSYS) && ((DRV_Reg32(RM_PWR_STA) & (1 << 20)) == 0))
                 continue;
        }
        g_SLOW_IDLE_BLOCK = (~(DRV_Reg32(MT65XX_CG_CON[i]))) & slow_idle_working_ability_mask[g_ver][i];
        if(g_SLOW_IDLE_BLOCK)//(working now, but no ability
        {
            printk("  %s is blocked by clock:0x%x in category:%d\r\n", sys_state_name[SLOW_IDLE_STATE], g_SLOW_IDLE_BLOCK, i);
            return;
        }
    }
	
    printk("  %s is blocked by nothing\r\n", sys_state_name[SLOW_IDLE_STATE]);
}
 
void display_out_reason_of_deep_idle(void)
{
    int i = 0;
    DWORD g_DEEP_IDLE_BLOCK = 0;
	  
#ifdef DEBUG_STATE_MASK
    if(bStateMask & (0x1 << DEEP_IDLE_STATE))
    {
        printk("  %s is blocked by mask\r\n", sys_state_name[DEEP_IDLE_STATE]);
        return;
    }
#endif
    for(i=0; i<MT65XX_TARGET_COUNT_END; i++)
    {
        if(isSubsysCanDisable(MT65XX_SUBSYS_OF_CATEGORY[i]))
        {
            if(DRV_Reg32(TOPSM_BASE+0x800 + MT65XX_SUBSYS_OF_CATEGORY[i] * 4) == 0x19a)
	         continue;
            if((g_ver == VER_E2) && (MT65XX_SUBSYS_OF_CATEGORY[i] == MM1_SUBSYS) && ((DRV_Reg32(RM_PWR_STA) & (1 << 20)) == 0))
                 continue;
        }
        g_DEEP_IDLE_BLOCK = (~(DRV_Reg32(MT65XX_CG_CON[i]))) & deep_idle_working_ability_mask[g_ver][i];
        if(g_DEEP_IDLE_BLOCK)//(working now, but no ability
        {
            printk("  %s is blocked by clock:0x%x in category:%d\r\n", sys_state_name[DEEP_IDLE_STATE], g_DEEP_IDLE_BLOCK, i);
            return;
        }
    }
	
    printk("  %s is blocked by nothing\r\n", sys_state_name[DEEP_IDLE_STATE]);
}
 
//if bus idle, then enter deep idle
BOOL can_enter_deep_idle(void)
{
    int i = 0;
    DWORD g_DEEP_IDLE_BLOCK = 0;
	  
#ifdef DEBUG_STATE_MASK
    if(bStateMask & (0x1 << DEEP_IDLE_STATE))
        return FALSE;
#endif
    for(i=0; i<MT65XX_TARGET_COUNT_END; i++)
    {
        if(isSubsysCanDisable(MT65XX_SUBSYS_OF_CATEGORY[i]))
        {
            if(DRV_Reg32(TOPSM_BASE+0x800 + MT65XX_SUBSYS_OF_CATEGORY[i] * 4) == 0x19a)
	               continue;
            if((g_ver == VER_E2) && (MT65XX_SUBSYS_OF_CATEGORY[i] == MM1_SUBSYS) && ((DRV_Reg32(RM_PWR_STA) & (1 << 20)) == 0))
                 continue;
        }
        g_DEEP_IDLE_BLOCK = (~(DRV_Reg32(MT65XX_CG_CON[i]))) & deep_idle_working_ability_mask[g_ver][i];
        if(g_DEEP_IDLE_BLOCK)//(working now, but no ability
        {
            return FALSE;
        }
    }
	
    return TRUE;
}

BOOL can_enter_slow_idle(void)
{
    int i = 0;
    DWORD g_SLOW_IDLE_BLOCK = 0;

#ifdef DEBUG_STATE_MASK
    if(bStateMask & (0x1 << SLOW_IDLE_STATE))
    	  return FALSE;
#endif    	  
    for(i=0; i<MT65XX_TARGET_COUNT_END; i++)
    {
        if(isSubsysCanDisable(MT65XX_SUBSYS_OF_CATEGORY[i]))
        {
            if(DRV_Reg32(TOPSM_BASE+0x800 + MT65XX_SUBSYS_OF_CATEGORY[i] * 4) == 0x19a)
                continue;
            if((g_ver == VER_E2) && (MT65XX_SUBSYS_OF_CATEGORY[i] == MM1_SUBSYS) && ((DRV_Reg32(RM_PWR_STA) & (1 << 20)) == 0))
                 continue;
        }
        g_SLOW_IDLE_BLOCK = (~(DRV_Reg32(MT65XX_CG_CON[i]))) & slow_idle_working_ability_mask[g_ver][i];
        if(g_SLOW_IDLE_BLOCK)//(working now, but no ability)
        {
            return FALSE;
        }

    }
	
    return TRUE;
}

BOOL can_enter_little_idle(void)
{
#ifdef DEBUG_STATE_MASK
    if(bStateMask & (0x1 << LITTLE_IDLE_STATE))
    	  return FALSE;
#endif    	  
    return TRUE;
}

UINT32 OST_status(void)
{
    UINT32 reg_val = 0;
    reg_val = DRV_Reg32(OST_STA);
    return (reg_val&0x1);
}
UINT32 pause_status(void)
{
    UINT32 reg_val = 0;
    reg_val = DRV_Reg32(OST_STA);
    reg_val &= 0x0018;
    reg_val >>= 3;
    return reg_val; 
    
}

void mt6573_enter_deep_idle(void)
{
    //DRV_SetReg32(AMPLL_CON1_REG,0x100);//enable SW control of AMPLL, this will force AMPLL on when deep idle
    //DRV_SetReg32(WPLL_CON0_REG, 0x100);//enable SW control of WPLL, this will force 3GPLL on when deep idle
    DRV_WriteReg32(VA12_CON0,0xf);//enale sleep control from pad, enable SW control of VA12 LDO, enable VA12 LDO, VA12 is the  power used to keep PLL on(E1)
    //DRV_SetReg32(0xf7026124, 0x2);
    // use API: hwPowerOn() ?????????? VA12 is on by default?????, no need to enable it, but enable SW control of it 

}

void mt6573_leave_deep_idle(void)
{
    //UINT32 regval = 0;
    //DRV_ClrReg32(AMPLL_CON1_REG,0x100);//enable HW control of APMCU PLL
    //DRV_ClrReg32(WPLL_CON0_REG, 0x100);	//enable HW control of 3G PLL,
    DRV_WriteReg32(VA12_CON0,0xD);//enable HW control of VA12 // use API: ?????????
    //DRV_ClrReg32(0xf7026124, 0x2);

}

#ifdef DEEP_IDLE_LOG
UINT32 start, end, WFI_time=0, WAKE_time=0, compare, counter,delay=0;
#endif

UINT32 compare_regval=0, counter_regval=0;

void ost_dpidle_before_wfi(void)
{
    sys_last_state = DEEP_IDLE_STATE; 

    compare_regval -= (wakeup_delay + counter_regval);//110
    XGPT_Set_Next_Compare(compare_regval);       //XGPT_SetCompare(XGPT4, compare_regval);
    //XGPT_Set_Next_Compare(327680);       //XGPT_SetCompare(XGPT4, compare_regval);
#ifdef DEEP_IDLE_LOG
    enter = TRUE;
    start = GPT_GetTimeout(GPT4);
    WAKE_time = start - end;
#endif
}

void ost_dpidle_after_wfi(void)
{
    end = GPT_GetTimeout(GPT4);
    WFI_time = end - start;
}

bool ost_dpidle_can_enter(void)
{
    compare_regval = XGPT_GetCompare(XGPT4);
    counter_regval = XGPT_GetCounter(XGPT4);
#ifdef DEEP_IDLE_LOG
    compare = compare_regval;
    counter = counter_regval;
#endif
    if (compare_regval > (counter_regval + wakeup_delay + 10 + time_criteria)) { //function call delay: 28, average resume delay:3.38ms(110); sleep 1ms at least(33); updtate compare delay 0.6ms (10)
        enter = TRUE;
        return true;
    }

    enter = FALSE;
    return false;
}


void arch_idle(void)
{
    UINT32 temp = 0;

    sys_last_state = NORMAL_STATE;
    enter = FALSE;
    if(can_enter_deep_idle())
    {
       mt6573_enter_deep_idle();
       ost_go_to_dpidle();
       mt6573_leave_deep_idle();
       MSG(DCM, "leave deep IDLE, compare=%d, counter=%d, enter=%d, can sleep:%d(32K),WFI_time=%d(26M), COMPARE=%d,COUNT=%d,WAKE_time=%d, OST_WAKESRC=0x%x,XGPT_IRQSTA=0x%x, RM_TMR_SSTA=0x%x, pause status=%s\r\n", compare, counter, enter, (compare-counter),WFI_time, DRV_Reg32(0xf702c04c), DRV_Reg32(0xf702c048), WAKE_time, DRV_Reg32(OST_WAKEUP_STA),DRV_Reg32(0xf702c004), DRV_Reg32(RM_TMR_SSTA), pause_status_name[pause_status()]);	
#ifdef DEEP_IDLE_LOG
#endif
    }
    if(enter)
    {
         return;
    }

    if(can_enter_slow_idle())
    {
       MT6573_ENABLE_HW_DCM_AP();//enable HW control of DCM
       sys_last_state = SLOW_IDLE_STATE;
       dsb(); 
       outer_sync(); 
       __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp)); //CPU WFI
       /* Disable HW DCM here*/
       MT6573_DISABLE_HW_DCM_AP();//disable HW control of DCM
    }
    else
    {
       sys_last_state = IDLE_STATE;
       dsb(); 
       outer_sync(); 
       __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
    }
}

void factory_idle(void)
{
    UINT32 temp = 0;
    
    if(can_enter_slow_idle())
    {
       MT6573_ENABLE_HW_DCM_AP();//enable HW control of DCM
       dsb(); 
       outer_sync(); 
       __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp)); //CPU WFI
       MT6573_DISABLE_HW_DCM_AP();//disable HW control of DCM
    }
    else
    {
       dsb(); 
       outer_sync(); 
       __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
    }
}

void factory_idle_test(void)
{
    mt6573_wdt_ModeSelection(false, false, false);
    
    //this should be set by low power requirement.
    #ifdef IDLE_LOW_POWER_TEST
        enable_low_power_settings();
    #endif
        factory_idle();
    #ifdef IDLE_LOW_POWER_TEST
        disable_low_power_settings();
    #endif

    mt6573_wdt_ModeSelection(true, false, true);
}

static ssize_t idle_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "=============== DCM state ===============\r\n\n");
    p += sprintf(p, "  [Available state]:\r\n");
    p += sprintf(p, "      DEEP IDLE:   %d\r\n", can_enter_deep_idle());
    p += sprintf(p, "      SLOW IDLE:   %d\r\n", can_enter_slow_idle());
    p += sprintf(p, "      LITTLE IDLE: %d\r\n", can_enter_little_idle());
    p += sprintf(p, "\n  [slow_idle_working_ability_mask]:\r\n");
    p += sprintf(p, "      AMPMCU0:  0x%x\r\n", slow_idle_working_ability_mask[g_ver][0]);
    p += sprintf(p, "      AMPMCU1:  0x%x\r\n", slow_idle_working_ability_mask[g_ver][1]);
    p += sprintf(p, "      MM1_CON0: 0x%x\r\n", slow_idle_working_ability_mask[g_ver][2]);
    p += sprintf(p, "      MM1_CON1: 0x%x\r\n", slow_idle_working_ability_mask[g_ver][3]);
    p += sprintf(p, "      MM2:      0x%x\r\n", slow_idle_working_ability_mask[g_ver][4]);
    p += sprintf(p, "      AUDIO:    0x%x\r\n", slow_idle_working_ability_mask[g_ver][5]);
    p += sprintf(p, "\n  [deep_idle_working_ability_mask]:\r\n");
    p += sprintf(p, "      AMPMCU0:  0x%x\r\n", deep_idle_working_ability_mask[g_ver][0]);
    p += sprintf(p, "      AMPMCU1:  0x%x\r\n", deep_idle_working_ability_mask[g_ver][1]);
    p += sprintf(p, "      MM1_CON0: 0x%x\r\n", deep_idle_working_ability_mask[g_ver][2]);
    p += sprintf(p, "      MM1_CON1: 0x%x\r\n", deep_idle_working_ability_mask[g_ver][3]);
    p += sprintf(p, "      MM2:      0x%x\r\n", deep_idle_working_ability_mask[g_ver][4]);
    p += sprintf(p, "      AUDIO:    0x%x\r\n", deep_idle_working_ability_mask[g_ver][5]);
    p += sprintf(p, "\n  [StateMask]: 0x%x\r\n", bStateMask);
   // p += sprintf(p, "      NORMAL:     0x0001\r\n");
   // p += sprintf(p, "      IDLE:       0x0002\r\n");
   // p += sprintf(p, "      LITTLE IDLE:0x0004\r\n");	  
   // p += sprintf(p, "      SLOW IDLE:  0x0008\r\n");	  
   // p += sprintf(p, "      DEEP IDLE:  0x0010\r\n");
    p += sprintf(p, "\n  [last state]: %s\r\n", sys_state_name[sys_last_state]);
    p += sprintf(p, "\n  [wakeup_delay]: %d\r\n", wakeup_delay);
    p += sprintf(p, "\n  [time_criteria]: %d\r\n", time_criteria);
    p += sprintf(p, "\n=========================================\r\n");
    p += sprintf(p, "\n=============== DCM help ================\r\n\n");
    p += sprintf(p, "  [arch_idle()]:    echo 0 > idle_state\r\n");
    p += sprintf(p, "  [state mask]:     echo 1 <enable> <state> > idle_state\r\n");
    p += sprintf(p, "  [device ability]: echo 2 <enable> <clockId> <state> > idle_state\r\n");
    p += sprintf(p, "                    (clockId=255, reset the working ability of all devices)\r\n");
    p += sprintf(p, "  [out reason]:     echo 3 <state> > idle_state\r\n");
    p += sprintf(p, "  [time criteria]:  echo 4 <time> > idle_state\r\n");
    p += sprintf(p, "  [wakeup_delay]:   echo 5 <time> > idle_state\r\n");
    p += sprintf(p, "\n  enable: 0=disable; 1=enable\r\n");
    p += sprintf(p, "  state: [0]=normal.     [1]idle.       [2]little idle.\r\n");
    p += sprintf(p, "         [3]=slow idle.  [4]deep idle.  [5]all states.\r\n");
    p += sprintf(p, "\n=========================================\r\n");

    len = p - buf;
    return len;
}

static ssize_t idle_state_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t n)
{
    DWORD test_mode, enable, state, clockId;

    printk("\n=============== DCM_TEST ================\r\n\n");
    if(sscanf(buf, "%d %d %d %d", &test_mode, &enable, &clockId, &state) == 4)
    {
        if(test_mode == TEST_DEVICE_ABILITY)
        {
            if(clockId == 255)
            {
                reset_device_working_ability(state);
            }
            else if(enable)
            {
                set_device_working_ability(clockId, state);
            }else
            {
                clr_device_working_ability(clockId, state);
            }
            printk("\n=========================================\r\n");
            return n;
        }
        else
        {
            printk("  TEST_DEVICE_ABILITY, Bad argument!\r\n");
        }
    }
    else if(sscanf(buf, "%d %d %d", &test_mode, &enable, &state) == 3)
    {
        if(test_mode == TEST_STATE_MASK)
        {
            if(!valid_dcm_state(state))
            {
                printk("  Bad state!\r\n");
                printk("  0=normal; 1=idle; 2=little idle; 3=slow idle; 4=deep idle, 5=all states\r\n");
            }
            if(enable)
            {
                dcm_enable_state(state);
            }
            else
            {
                dcm_disable_state(state);
            }

            printk("\n=========================================\r\n");
            return n;
        }
        else
        {
            printk("  TEST_STATE_MASK, Bad argument!\r\n");
        }

    }
    else if(sscanf(buf, "%d %d", &test_mode, &state) == 2)
    {
        if(test_mode == TEST_OUT_REASON)
        {
            switch(state)
            {
                case DEEP_IDLE_STATE:
                     display_out_reason_of_deep_idle();
                     break;
                case SLOW_IDLE_STATE:
                     display_out_reason_of_slow_idle();
                     break;
                case LITTLE_IDLE_STATE:
                     display_out_reason_of_slow_idle();
                     break;
                case NORMAL_STATE:
                case IDLE_STATE:
                     printk("  %s is blocked by nothing\r\n", sys_state_name[state]);
                     break;
                default:
                     printk("  Bad state\r\n");
                     break;
            }
 
            printk("\n=========================================\r\n");
            return n;
        }
        else if(test_mode == TEST_TIME_CRITERIA)
        {
            time_criteria = state;
            printk("  TEST_TIME_CRITERIA:time_criteria = %d\r\n", state);
        }
        else if(test_mode == TEST_WAKEUP_DELAY)
        {
            wakeup_delay = state;
            printk("  TEST_WAKEUP_DELAY:wakeup_delay=%d\r\n", wakeup_delay);
        }
        else
        {
            printk("  TEST_OUT_REASON / TEST_TIME_CRITERIA, Bad argument!\r\n");
        }
    }
    else if(sscanf(buf, "%d", &test_mode) == 1)
    {
        if(test_mode == TEST_ARCH_IDLE)
        {
            factory_idle_test();
            printk("\n=========================================\r\n");
            return n;
        }
        else
        {
            printk("  TEST_ARCH_IDLE, Bad argument! test_mode=%d\r\n", test_mode);
        }
    }
    else
    {
        printk("  Bad argument! test_mode=%d\r\n", test_mode);
    }

    printk("\n=========================================\r\n");

    return -EINVAL;
}

dcm_attr(idle_state);

int factory_idle_current_init(void)
{
    return sysfs_create_file(power_kobj, &idle_state_attr.attr);	//add it to power
}
  
