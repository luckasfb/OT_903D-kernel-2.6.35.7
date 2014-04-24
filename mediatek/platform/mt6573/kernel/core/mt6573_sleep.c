
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <mach/mt6573_boot.h>
#include <mach/mt6573_ost_sm.h>
#include <mach/mt6573_pll.h>
#include <mach/mtk_rtc.h>
#include <mach/mt6573_gpio.h>

#include <board-custom.h>
#include <pmu6573_hw.h>
#include <mt6573_battery.h>

#define VA28_CON1	(VA28_CON0 + 4)
#define VCORE_CON1	(VCORE_CON0 + 4)
#define VAPROC_CON1	(VAPROC_CON0 + 4)

#define slp_read16(addr)	(*(volatile u16 *)(addr))
#define slp_write16(addr, val)	(*(volatile u16 *)(addr) = (u16)(val))

#define slp_read32(addr)	(*(volatile u32 *)(addr))
#define slp_write32(addr, val)	(*(volatile u32 *)(addr) = (u32)(val))

#define slp_write_sync()	\
do {				\
	dsb();			\
	outer_sync();		\
} while (0)

extern void MT6573_ENABLE_HW_DCM_AP(void);
extern void MT6573_DISABLE_HW_DCM_AP(void);

extern void wake_up_bat(void);

static wake_reason_t slp_wake_reason;

static int slp_dump_gpio = 0;
static int slp_dump_regs = 1;

static DEFINE_SPINLOCK(slp_cc_lock);
static ccci_callback_t slp_ccci_callback = NULL;

void slp_set_ccci_callback(ccci_callback_t cc)
{
	unsigned long flags;

	spin_lock_irqsave(&slp_cc_lock, flags);
	slp_ccci_callback = cc;
	spin_unlock_irqrestore(&slp_cc_lock, flags);
}
EXPORT_SYMBOL(slp_set_ccci_callback);

static int slp_md_sta_show(struct seq_file *m, void *v)
{
	u32 ssta, mdssta = 0;
	int i;

	ssta = slp_read32(RM_TMR_SSTA);
	for (i = 0; i < 4; i++) {
		/* check if timer is in Pause or Pre-Pause state */
		if ((ssta & 0xf) == 0x2 || (ssta & 0xf) == 0x4)
			mdssta |= (1U << (i * 4));
		ssta >>= 4;
	}

	seq_printf(m, "0x%x\n", mdssta);

	return 0;
}

static int slp_md_sta_open(struct inode *inode, struct file *file)
{
	return single_open(file, slp_md_sta_show, NULL);
}

static struct file_operations slp_md_sta_fops = {
	.open		= slp_md_sta_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void slp_dump_pm_regs(void)
{
	/* PLL registers */
	printk(KERN_DEBUG "CLKSQ_CON        0x%x = 0x%x\n", CLKSQ_CON         , slp_read16(CLKSQ_CON));
	printk(KERN_DEBUG "PLL_CON6         0x%x = 0x%x\n", PLL_CON6_REG      , slp_read16(PLL_CON6_REG));
	printk(KERN_DEBUG "MPLL_CON1        0x%x = 0x%x\n", MPLL_CON1_REG     , slp_read16(MPLL_CON1_REG));
	printk(KERN_DEBUG "AMPLL_CON1       0x%x = 0x%x\n", AMPLL_CON1_REG    , slp_read16(AMPLL_CON1_REG));
	printk(KERN_DEBUG "DPLL_CON1        0x%x = 0x%x\n", DPLL_CON1_REG     , slp_read16(DPLL_CON1_REG));
	printk(KERN_DEBUG "EPLL_CON1        0x%x = 0x%x\n", EPLL_CON1_REG     , slp_read16(EPLL_CON1_REG));
	printk(KERN_DEBUG "CPLL_CON0        0x%x = 0x%x\n", CPLL_CON0_REG     , slp_read16(CPLL_CON0_REG));
	printk(KERN_DEBUG "WPLL_CON0        0x%x = 0x%x\n", WPLL_CON0_REG     , slp_read16(WPLL_CON0_REG));
	printk(KERN_DEBUG "GPLL_CON0        0x%x = 0x%x\n", GPLL_CON0_REG     , slp_read16(GPLL_CON0_REG));
	printk(KERN_DEBUG "THREEDPLL_CON0   0x%x = 0x%x\n", THREEDPLL_CON0_REG, slp_read16(THREEDPLL_CON0_REG));
	printk(KERN_DEBUG "TVPLL_CON0       0x%x = 0x%x\n", TVPLL_CON0_REG    , slp_read16(TVPLL_CON0_REG));
	printk(KERN_DEBUG "FGPLL_CON0       0x%x = 0x%x\n", FGPLL_CON0_REG    , slp_read16(FGPLL_CON0_REG));
	printk(KERN_DEBUG "AUXPLL_CON0      0x%x = 0x%x\n", AUXPLL_CON0_REG   , slp_read16(AUXPLL_CON0_REG));

	/* APCONFIG/MMSYS registers */
	printk(KERN_DEBUG "AP_MEM_PD        0x%x = 0x%x\n", AP_MEM_PD     , slp_read32(AP_MEM_PD));
	printk(KERN_DEBUG "MM1_MEM_PD       0x%x = 0x%x\n", TOPSM_DMY0    , slp_read32(TOPSM_DMY0));
	printk(KERN_DEBUG "MM2_MEM_PD       0x%x = 0x%x\n", TOPSM_DMY1    , slp_read32(TOPSM_DMY1));
	printk(KERN_DEBUG "RG_CK_ALW_ON     0x%x = 0x%x\n", RG_CK_ALW_ON  , slp_read16(RG_CK_ALW_ON));
	printk(KERN_DEBUG "RG_CK_DCM_EN     0x%x = 0x%x\n", RG_CK_DCM_EN  , slp_read16(RG_CK_DCM_EN));
	printk(KERN_DEBUG "APMCU_CG_CON0    0x%x = 0x%x\n", APMCU_CG_CON0 , slp_read32(APMCU_CG_CON0));
	printk(KERN_DEBUG "APMCU_CG_CON1    0x%x = 0x%x\n", APMCU_CG_CON1 , slp_read32(APMCU_CG_CON1));
	printk(KERN_DEBUG "MMSYS1_CG_CON0   0x%x = 0x%x\n", MMSYS1_CG_CON0, slp_read32(MMSYS1_CG_CON0));
	printk(KERN_DEBUG "MMSYS1_CG_CON1   0x%x = 0x%x\n", MMSYS1_CG_CON1, slp_read32(MMSYS1_CG_CON1));
	printk(KERN_DEBUG "MMSYS2_CG_CON0   0x%x = 0x%x\n", MMSYS2_CG_CON0, slp_read32(MMSYS2_CG_CON0));

	/* TOPSM registers */
	printk(KERN_DEBUG "RM_CLK_SETTLE    0x%x = 0x%x\n", RM_CLK_SETTLE   , slp_read32(RM_CLK_SETTLE));
	printk(KERN_DEBUG "RM_TMRPWR_SETTLE 0x%x = 0x%x\n", RM_TMRPWR_SETTLE, slp_read32(RM_TMRPWR_SETTLE));
	printk(KERN_DEBUG "RM_TMR_TRG0      0x%x = 0x%x\n", RM_TMR_TRG0     , slp_read32(RM_TMR_TRG0));
	printk(KERN_DEBUG "RM_TMR_PWR0      0x%x = 0x%x\n", RM_TMR_PWR0     , slp_read32(RM_TMR_PWR0));
	printk(KERN_DEBUG "RM_TMR_PWR1      0x%x = 0x%x\n", RM_TMR_PWR1     , slp_read16(RM_TMR_PWR1));
	printk(KERN_DEBUG "RM_PERI_CON      0x%x = 0x%x\n", RM_PERI_CON     , slp_read32(RM_PERI_CON));
	printk(KERN_DEBUG "RM_TMR_SSTA      0x%x = 0x%x\n", RM_TMR_SSTA     , slp_read32(RM_TMR_SSTA));
	printk(KERN_DEBUG "TOPSM_DBG        0x%x = 0x%x\n", TOPSM_DBG       , slp_read32(TOPSM_DBG));
	printk(KERN_DEBUG "FRC_F32K_FM      0x%x = 0x%x\n", FRC_F32K_FM     , slp_read32(FRC_F32K_FM));
	printk(KERN_DEBUG "CCF_CLK_CON      0x%x = 0x%x\n", CCF_CLK_CON     , slp_read16(CCF_CLK_CON));
	printk(KERN_DEBUG "RM_PWR_CON0      0x%x = 0x%x\n", RM_PWR_CON0     , slp_read16(RM_PWR_CON0));
	printk(KERN_DEBUG "RM_PWR_CON1      0x%x = 0x%x\n", RM_PWR_CON1     , slp_read16(RM_PWR_CON1));
	printk(KERN_DEBUG "RM_PWR_CON2      0x%x = 0x%x\n", RM_PWR_CON2     , slp_read16(RM_PWR_CON2));
	printk(KERN_DEBUG "RM_PWR_CON3      0x%x = 0x%x\n", RM_PWR_CON3     , slp_read16(RM_PWR_CON3));
	printk(KERN_DEBUG "RM_PWR_CON4      0x%x = 0x%x\n", RM_PWR_CON4     , slp_read16(RM_PWR_CON4));
	printk(KERN_DEBUG "RM_PWR_CON5      0x%x = 0x%x\n", RM_PWR_CON5     , slp_read16(RM_PWR_CON5));
	printk(KERN_DEBUG "RM_PWR_CON6      0x%x = 0x%x\n", RM_PWR_CON6     , slp_read16(RM_PWR_CON6));
	printk(KERN_DEBUG "RM_PWR_CON7      0x%x = 0x%x\n", RM_PWR_CON7     , slp_read16(RM_PWR_CON7));
	printk(KERN_DEBUG "RM_PLL_MASK0     0x%x = 0x%x\n", RM_PLL_MASK0    , slp_read32(RM_PLL_MASK0));
	printk(KERN_DEBUG "RM_PLL_MASK1     0x%x = 0x%x\n", RM_PLL_MASK1    , slp_read32(RM_PLL_MASK1));
	printk(KERN_DEBUG "CCF_CLK_CON_R    0x%x = 0x%x\n", CCF_CLK_CON_R   , slp_read16(CCF_CLK_CON_R));

	/* PMU registers */
	printk(KERN_DEBUG "VA28_CON0        0x%x = 0x%x\n", VA28_CON0  , slp_read16(VA28_CON0));
	printk(KERN_DEBUG "VA25_CON0        0x%x = 0x%x\n", VA25_CON0  , slp_read16(VA25_CON0));
	printk(KERN_DEBUG "VA12_CON0        0x%x = 0x%x\n", VA12_CON0  , slp_read16(VA12_CON0));
	printk(KERN_DEBUG "VMIC_CON0        0x%x = 0x%x\n", VMIC_CON0  , slp_read16(VMIC_CON0));
	printk(KERN_DEBUG "VTV_CON0         0x%x = 0x%x\n", VTV_CON0   , slp_read16(VTV_CON0));
	printk(KERN_DEBUG "VAUDN_CON0       0x%x = 0x%x\n", VAUDN_CON0 , slp_read16(VAUDN_CON0));
	printk(KERN_DEBUG "VAUDP_CON0       0x%x = 0x%x\n", VAUDP_CON0 , slp_read16(VAUDP_CON0));
	printk(KERN_DEBUG "VCAMA_CON0       0x%x = 0x%x\n", VCAMA_CON0 , slp_read16(VCAMA_CON0));
	printk(KERN_DEBUG "VCAMD_CON0       0x%x = 0x%x\n", VCAMD_CON0 , slp_read16(VCAMD_CON0));
	printk(KERN_DEBUG "VUSB_CON0        0x%x = 0x%x\n", VUSB_CON0  , slp_read16(VUSB_CON0));
	printk(KERN_DEBUG "VIBR_CON0        0x%x = 0x%x\n", VIBR_CON0  , slp_read16(VIBR_CON0));
	printk(KERN_DEBUG "VMC_CON0         0x%x = 0x%x\n", VMC_CON0   , slp_read16(VMC_CON0));
	printk(KERN_DEBUG "VCAMA2_CON0      0x%x = 0x%x\n", VCAMA2_CON0, slp_read16(VCAMA2_CON0));
	printk(KERN_DEBUG "VCAMD2_CON0      0x%x = 0x%x\n", VCAMD2_CON0, slp_read16(VCAMD2_CON0));
	printk(KERN_DEBUG "VCORE_CON1       0x%x = 0x%x\n", VCORE_CON1 , slp_read16(VCORE_CON1));
	printk(KERN_DEBUG "VAPROC_CON1      0x%x = 0x%x\n", VAPROC_CON1, slp_read16(VAPROC_CON1));
	printk(KERN_DEBUG "KPLED_CON0       0x%x = 0x%x\n", KPLED_CON0 , slp_read16(KPLED_CON0));
}

static int slp_suspend_ops_valid(suspend_state_t pm_state)
{
	return pm_state == PM_SUSPEND_MEM;
}

static int slp_suspend_ops_begin(suspend_state_t state)
{
	/* legacy log */
	printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk("_Chip_pm_begin @@@@@@@@@@@@@@@@@@@@@@\n");
	printk(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	slp_wake_reason = WR_NONE;

	return 0;
}

static int slp_suspend_ops_prepare(void)
{
	unsigned long flags;

	/* legacy log */
	printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk("_Chip_pm_prepare @@@@@@@@@@@@@@@@@@@@\n");
	printk(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	/* let CCCI notify MD that AP will go to sleep */
	spin_lock_irqsave(&slp_cc_lock, flags);
	if (slp_ccci_callback)
		slp_ccci_callback();
	spin_unlock_irqrestore(&slp_cc_lock, flags);

	return 0;
}

static int slp_suspend_ops_enter(suspend_state_t state)
{
	/* legacy log */
	printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk("_Chip_pm_enter @@@@@@@@@@@@@@@@@@@@@@\n");
	printk(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	if (slp_dump_gpio)
		gpio_dump_regs();

	if (get_chip_eco_ver() == CHIP_E1) {
		/* disable DCM to workaround EMI auto-refresh issue */
		MT6573_DISABLE_HW_DCM_AP();
	} else {
		MT6573_ENABLE_HW_DCM_AP();
	}

	if (slp_dump_regs)
		slp_dump_pm_regs();

	rtc_disable_writeif();

	slp_wake_reason = ost_go_to_sleep();

	rtc_enable_writeif();

	MT6573_DISABLE_HW_DCM_AP();

	return 0;
}

static void slp_suspend_ops_finish(void)
{
	/* legacy log */
	printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk("_Chip_pm_finish @@@@@@@@@@@@@@@@@@@@@\n");
	printk(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	/* debug help */
	printk("Battery_Voltage = %lu\n", BAT_Get_Battery_Voltage());
}

static void slp_suspend_ops_end(void)
{
	/* legacy log */
	printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk("_Chip_pm_end @@@@@@@@@@@@@@@@@@@@@@@@\n");
	printk(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	if (slp_wake_reason == WR_LOW_BAT)
		wake_up_bat();
}

static struct platform_suspend_ops slp_suspend_ops = {
	.valid		= slp_suspend_ops_valid,
	.begin		= slp_suspend_ops_begin,
	.prepare	= slp_suspend_ops_prepare,
	.enter		= slp_suspend_ops_enter,
	.finish		= slp_suspend_ops_finish,
	.end		= slp_suspend_ops_end,
};

static void slp_pmu_init(void)
{
	u16 con1;

#ifdef VCORE_1_1_V_IN_SLEEP
	/* Vcore = 1.1V in sleep mode */
	con1 = (slp_read16(VCORE_CON1) & 0xfe0f) | (28 << 4);
	slp_write16(VCORE_CON1, con1);
#else
	/* Vcore = 0.9V in sleep mode */
	con1 = (slp_read16(VCORE_CON1) & 0xfe0f) | (20 << 4);
	slp_write16(VCORE_CON1, con1);
#endif

	/* Vaproc = 0.9V in sleep mode */
	con1 = (slp_read16(VAPROC_CON1) & 0xfe0f) | (20 << 4);
	slp_write16(VAPROC_CON1, con1);

	/* clear CCI_SRCLKEN to enable HW sleep-mode control */
	con1 = slp_read16(VA28_CON1) & ~(1U << 8);
	slp_write16(VA28_CON1, con1);

	slp_write_sync();
}

void slp_mod_init(void)
{
	slp_pmu_init();

	ost_mod_init();

	suspend_set_ops(&slp_suspend_ops);

	proc_create_data("slp_md_sta", 0444, NULL, &slp_md_sta_fops, NULL);
}

module_param(slp_dump_gpio, bool, 0644);
module_param(slp_dump_regs, bool, 0644);

MODULE_AUTHOR("Terry Chang <terry.chang@mediatek.com>");
MODULE_DESCRIPTION("MT6573 Sleep Driver v1.7");
