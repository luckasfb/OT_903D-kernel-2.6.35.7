

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
    

extern void upmu_ldo_enable(upmu_ldo_list_enum ldo, kal_bool enable);
extern void upmu_ldo_vol_sel(upmu_ldo_list_enum ldo, upmu_ldo_vol_enum vol);


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


bool hwPowerOn(MT65XX_POWER powerId, MT65XX_POWER_VOLTAGE powervol, char *mode_name)
{
    UINT32 i = 0;    
    if(powerId >= MT65XX_POWER_COUNT_END)
    {
        MSG(PMIC,"[MT65XX PMU] Error!! powerId is wrong\r\n");
        return FALSE;
    }
    for (i = 0; i< MAX_DEVICE; i++)
    {
        if (!strcmp(g_MT6573_BusHW.Power[powerId].mod_name[i], NON_OP))
        {
            MSG(PMIC,"[%s] acquire powerId:%d index:%d mod_name: %s\r\n", 
                __FUNCTION__,powerId, i, mode_name);            
            sprintf(g_MT6573_BusHW.Power[powerId].mod_name[i] , "%s", mode_name);
            break ;
        }
        /* already it */
        #if 0
        else if (!strcmp(g_MT6573_BusHW.Power[powerId].mod_name[i], mode_name))
        {
            MSG(CG,"[%d] Power already register\r\n",powerId );        
        }
        #endif
    }    
    g_MT6573_BusHW.Power[powerId].dwPowerCount++ ;
    /* We've already enable this LDO before */
    if(g_MT6573_BusHW.Power[powerId].dwPowerCount > 1)
    {
        return TRUE;
    }    
    /* Turn on PMU LDO*/
    MSG(CG,"[%d] PMU LDO Enable\r\n",powerId );            

    if ((powerId == MT65XX_POWER_LDO_VIBR)
    	||(powerId == MT65XX_POWER_LDO_VCAMA)
    	||(powerId == MT65XX_POWER_LDO_VCAMD)
    	||(powerId == MT65XX_POWER_LDO_VCAMA2)
    	||(powerId == MT65XX_POWER_LDO_VCAMD2)
    	)
	    upmu_ldo_vol_sel(powerId, powervol);

    upmu_ldo_enable(powerId, KAL_TRUE);
    
    return TRUE; 
}

bool hwPowerDown(MT65XX_POWER powerId, char *mode_name)
{
    UINT32 i;
    BOOL bFind = FALSE;    
    if(powerId >= MT65XX_POWER_COUNT_END)
    {
        MSG(PMIC,"%s:%s:%d powerId:%d is wrong\r\n",__FILE__,__FUNCTION__, 
            __LINE__ , powerId);
        return FALSE;
    }    
    if(g_MT6573_BusHW.Power[powerId].dwPowerCount == 0)
    {
        MSG(PMIC,"%s:%s:%d powerId:%d (g_MT6573_BusHW.dwPowerCount[powerId] = 0)\r\n", 
            __FILE__,__FUNCTION__,__LINE__ ,powerId);
        return FALSE;
    }
    for (i = 0; i< MAX_DEVICE; i++)
    {
        if (!strcmp(g_MT6573_BusHW.Power[powerId].mod_name[i], mode_name))
        {
            MSG(PMIC,"[%s] powerId:%d index:%d mod_name: %s\r\n", 
                __FUNCTION__,powerId, i, mode_name);            
            sprintf(g_MT6573_BusHW.Power[powerId].mod_name[i] , "%s", NON_OP);
            bFind = TRUE;
            break ;
        }
    }   
    if(!bFind)
    {
        MSG(PMIC,"[%s] Cannot find [%d] master is [%s]\r\n",__FUNCTION__,powerId, mode_name);        
        return TRUE;
    }        
    g_MT6573_BusHW.Power[powerId].dwPowerCount--;
    if(g_MT6573_BusHW.Power[powerId].dwPowerCount > 0)
    {
        return TRUE;
    }
    /* Turn off PMU LDO*/
    MSG(CG,"[%d] PMU LDO Disable\r\n",powerId );            
    upmu_ldo_enable(powerId, KAL_FALSE);

    return TRUE;    
}

