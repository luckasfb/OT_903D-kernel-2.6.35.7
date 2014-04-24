


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

#define EINT_THERMAL_110_NUM    26
#define EINT_THERMAL_130_NUM    25
#define EINT_THERMAL_SEN        MT65xx_LEVEL_SENSITIVE
#define EINT_THERMAL_POL        MT65XX_EINT_POL_POS
#define EINT_THERMAL_DEB        1
#define VRTC_CON1               (VRTC_CON0 + 4)

void hwThermalOver110C(void)
{
    printk("Chip over 110C, power off Camera Sensor\n\r");
    /*Let Camera sensor power down*/
}

void hwThermalProtectInit(void)
{
    /*enable HW thermal shutdown interrupt */
    DRV_SetReg32(VRTC_CON1,1<<4);
    /*KPLED and VIBR Thermal shut down enable.*/
    DRV_SetReg32(KPLED_CON0,1<<11);
    
    /*check thermal controller, over 110C*/
	mt65xx_eint_set_sens(EINT_THERMAL_110_NUM,EINT_THERMAL_SEN);
	mt65xx_eint_set_polarity(EINT_THERMAL_110_NUM,EINT_THERMAL_POL);
	mt65xx_eint_set_hw_debounce(EINT_THERMAL_110_NUM,EINT_THERMAL_DEB);
	mt65xx_eint_registration(EINT_THERMAL_110_NUM,
							1,
							EINT_THERMAL_POL,
							hwThermalOver110C,
							1);
	mt65xx_eint_unmask(EINT_THERMAL_110_NUM);

}

