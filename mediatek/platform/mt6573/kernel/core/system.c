
#include <linux/string.h>
#include <linux/module.h>
#include <linux/io.h>
#include <asm/mach/irq.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/sync_write.h"
#include "mach/mt6573_typedefs.h"
#include "mach/mt6573_pll.h"
#include "mach/mt6573_wdt.h"
#include <mach/mtk_rtc.h>



void arch_reset(char mode, const char *cmd)
{
    unsigned inter_mode = 0;
    printk("arch_reset: cmd = %s\n", cmd ? : "NULL");

    if (cmd && !strcmp(cmd, "charger")) {
        /* do nothing */
    } else if (cmd && !strcmp(cmd, "recovery")) {
        rtc_mark_recovery();
    } else {
       inter_mode = MT6573_WDT_MODE_DEBUG; 
       rtc_mark_swreset();
    }
		
    printk("mode:0x%x, length:0x%x, swrst:0x%x\n", DRV_Reg16(MT6573_WDT_MODE), DRV_Reg16(MT6573_WDT_LENGTH), DRV_Reg16(MT6573_WDT_SWRST)); 
    MTCMOS_En_reboot();
    DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY);
    DRV_WriteReg16(MT6573_WDT_MODE,  (MT6573_WDT_MODE_KEY|MT6573_WDT_MODE_EXTEN|MT6573_WDT_MODE_ENABLE|inter_mode));
    //DRV_WriteReg16(MT6573_WDT_LENGTH, MT6573_WDT_LENGTH_KEY);
    DRV_WriteReg16(MT6573_WDT_SWRST,  MT6573_WDT_SWRST_KEY);
    while(1)
    	printk("mode:0x%x, length:0x%x, swrst:0x%x\n", DRV_Reg16(MT6573_WDT_MODE), DRV_Reg16(MT6573_WDT_LENGTH), DRV_Reg16(MT6573_WDT_SWRST));    
}
