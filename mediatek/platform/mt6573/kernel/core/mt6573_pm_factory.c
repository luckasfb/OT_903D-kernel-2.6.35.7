



#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/leds.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/mt6573_pll.h"
#include "mach/mt6573_typedefs.h"
#include "mach/sync_write.h"
#include "mach/mt6573_ost_sm.h"

#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"
#include "cust_leds.h"





static struct mtk_irq_mask mask;

extern int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level);
extern int mt6573_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt6573_irq_mask_restore(struct mtk_irq_mask *mask);
extern int DISP_PanelEnable(BOOL bEnable);
extern int DISP_PowerEnable(BOOL bEnable);
 
#ifdef IDLE_LOW_POWER_TEST
void enable_low_power_settings(void)
{
    int ret;
    printk("mt6573_pm_factory: enable low power settings\n");
        
    //ARM 169MHz
    PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F8_MHZ);
    
    //BL
    mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, 0);
    
    //Turn Off LCD 
    DISP_PanelEnable(FALSE);
    DISP_PowerEnable(FALSE);	

    //Save interrupt status 
    ret = mt6573_irq_mask_all(&mask);
    if (!ret) {
        printk("success to mask all irq lines\n");
    } else {
        printk("fail to mask all irq lines\n");
    }
   
    mt6573_irq_unmask(MT6573_KEYPAD_IRQ_LINE);
}
#endif

#ifdef IDLE_LOW_POWER_TEST
void disable_low_power_settings(void)
{
    int turbo_mode, ret;
    printk("mt6573_pm_factory: disable low power settings\n");
    
    turbo_mode = (DRV_Reg32(0xF7024104) & 0x10000000) >> 28;
    
    //Restore interrupt mask
    ret = mt6573_irq_mask_restore(&mask);
    if (!ret) {
        printk("success to restore all irq lines\n");
    } else {
        printk("fail to restore all irq lines\n");
    }

    //Turn On LCD  
    DISP_PowerEnable(TRUE);
    DISP_PanelEnable(TRUE);	
    
    //BL
    mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_FULL);
    
    if (turbo_mode)
    {
        //ARM 806MHz
        PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F1_TM_MHZ);
    }
    else
    {
        //ARM 676MHz
        PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F1_MHZ);
    }
}
#endif

static int __init pm_factory_init(void)
{
    /* */
    return 0;
}

static void __exit pm_factory_exit(void)
{
    /* */
}

module_init(pm_factory_init);
module_exit(pm_factory_exit);

MODULE_DESCRIPTION("Power management factory mode driver for mt6573");
MODULE_LICENSE("GPL");

