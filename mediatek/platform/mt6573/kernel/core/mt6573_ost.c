
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/string.h>

#include <asm/tcm.h>
#include <mach/mt6573_boot.h>
#include <mach/irqs.h>
#include <mach/mt6573_ost_sm.h>
#include <mach/mt6573_pll.h>

#include <board-custom.h>
#include <mt6573_battery.h>

#define AP_FORCE_26M_OFF	0

#define OST_NAME		"mt6573-ost"

#define FRC_CON_EN		(1U << 0)
#define FRC_CON_KEY		(0x6573 << 16)

#define OST_CON_EN		(1U << 0)
#define OST_CON_UFN_DOWN	(1U << 1)

#define OST_CMD_PAUSE_STR	(1U << 0)
#define OST_CMD_OST_WR		(1U << 2)
#define OST_CMD_UFN_WR		(1U << 13)
#define OST_CMD_AFN_WR		(1U << 14)
#define OST_CMD_CON_WR		(1U << 15)
#define OST_CMD_KEY		(0x6573 << 16)

#define OST_STA_CMD_CPL		(1U << 1)

/* typical values */
#define RM_SYSCLK_SETTLE	160	/* T 32K */
#define RM_PLL1_SETTLE		3	/* T 32K */
#define RM_PLL2_SETTLE		3	/* T 32K */

/* typical values */
#define OST_FRM_VAL		4615	/* us */
#define OST_FRM_NUM		2
#define OST_FRM_F32K_VAL	295	/* T 32K */

#define ost_read16(addr)	(*(volatile u16 *)(addr))
#define ost_write16(addr, val)	(*(volatile u16 *)(addr) = (u16)(val))

#define ost_read32(addr)	(*(volatile u32 *)(addr))
#define ost_write32(addr, val)	(*(volatile u32 *)(addr) = (u32)(val))

#define ost_write_sync()	\
do {				\
	dsb();			\
	outer_sync();		\
} while (0)

extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);

extern int mt6573_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt6573_irq_mask_restore(struct mtk_irq_mask *mask);

static DEFINE_SPINLOCK(ost_lock);

static u32 ost_wake_src = (
#ifdef PLATFORM_EVB
	WAKE_SRC_KP | WAKE_SRC_MSDC0 | WAKE_SRC_EINT | WAKE_SRC_RTC | WAKE_SRC_CCIF_MD
#else
	WAKE_SRC_KP | WAKE_SRC_MSDC0 | WAKE_SRC_EINT | WAKE_SRC_RTC | WAKE_SRC_CCIF_MD
#endif
);

static u16 ost_wake_irq[NUM_WAKE_SRC] = {
	[2] = MT6573_KEYPAD_IRQ_LINE,
	[3] = MT6573_MSDC0_CD_IRQ_LINE,
	[6] = MT6573_APARM_EINT_IRQ_LINE,
	[7] = MT6573_RTC_IRQ_LINE,
	[8] = MT6573_CCIF1_AP_IRQ_LINE,
};

static void ost_output_wake_reason(u32 wakesta, u16 isr)
{
	char str[128] = { 0 };

	if (wakesta & WAKE_SRC_KP)
		strcat(str, "KP ");

	if (wakesta & WAKE_SRC_MSDC0)
		strcat(str, "MSDC0 ");

	if (wakesta & WAKE_SRC_EINT)
		strcat(str, "EINT ");

	if (wakesta & WAKE_SRC_RTC)
		strcat(str, "RTC ");

	if (wakesta & WAKE_SRC_CCIF_MD)
		strcat(str, "CCIF_MD ");

	printk("wake up by %s(0x%x)(0x%x)\n", str, wakesta, isr);
}

static u32 ost_get_wake_period(unsigned long wake_cnt)
{
	u32 sec = 300;
	printk("sec = %u\n", sec);
	return sec;
}

static wake_reason_t __tcmfunc ost_enter_pwake_pause_mode(void)
{
	u16 isr;
	u32 ufn, wakesta;
	unsigned long vbat, cnt = 0;

	while (1) {
		ufn = ost_get_wake_period(cnt) * 1000000 / OST_FRM_VAL;
		ost_write32(OST_UFN, ufn);
		ost_write32(OST_AFN, 0);

		/* unmask wakeup sources */
		ost_write32(OST_EVENT_MASK, ~ost_wake_src);

		/* unmask Pause Interrupt, Pause Abort and UFN Timeout */
		ost_write32(OST_INT_MASK, 0x0003);

		ost_write16(OST_CON, OST_CON_UFN_DOWN | OST_CON_EN);
		ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_CON_WR | OST_CMD_AFN_WR |
		                     OST_CMD_UFN_WR | OST_CMD_OST_WR);
		while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));

		ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_PAUSE_STR);
		while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));

		/* flush L1 and L2 store buffers */
		ost_write_sync();

		/* enter WFI mode */
		__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : : "r" (0));

		wakesta = ost_read32(OST_WAKEUP_STA);
		isr = ost_read16(OST_ISR);

		ost_write32(OST_INT_MASK, 0x001f);
		ost_write16(OST_ISR, 0x001f);	/* write 1 clear */
		ost_write_sync();

		if (isr == 0x0004) {	/* UFN Timeout */
			vbat = BAT_Get_Battery_Voltage();
			printk("vbat-%lu = %lu\n", ++cnt, vbat);
			if (vbat <= SYSTEM_OFF_VOLTAGE) {
				printk("low battery => wake up\n");
				return WR_LOW_BAT;
			}
		} else {
			ost_output_wake_reason(wakesta, isr);
			return WR_WAKE_SRC;
		}
	}

	return WR_NONE;
}

#if 0
static wake_reason_t __tcmfunc ost_enter_pause_mode(void)
{
	u16 isr;
	u32 wakesta;

	ost_write32(OST_UFN, 5);
	ost_write32(OST_AFN, 0);

	/* unmask wakeup sources */
	ost_write32(OST_EVENT_MASK, ~ost_wake_src);

	/* unmask Pause Interrupt and Pause Abort */
	ost_write32(OST_INT_MASK, 0x0007);

	ost_write16(OST_CON, OST_CON_EN);
	ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_CON_WR | OST_CMD_AFN_WR |
	                     OST_CMD_UFN_WR | OST_CMD_OST_WR);
	while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));

	ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_PAUSE_STR);
	while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));

	/* flush L1 and L2 store buffers */
	ost_write_sync();

	/* enter WFI mode */
	__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : : "r" (0));

	wakesta = ost_read32(OST_WAKEUP_STA);
	isr = ost_read16(OST_ISR);

	ost_write32(OST_INT_MASK, 0x001f);
	ost_write16(OST_ISR, 0x001f);	/* write 1 clear */
	ost_write_sync();

	ost_output_wake_reason(wakesta, isr);

	return WR_WAKE_SRC;
}
#endif

static void ost_enable_wake_irq(unsigned int irq, bool force)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	if (desc->status & IRQ_SUSPENDED)
		__enable_irq(desc, irq, true);
	else if (force)
		mt65xx_irq_unmask(irq);
}

wake_reason_t ost_go_to_sleep(void)
{
	int i;
	unsigned long flags;
	struct mtk_irq_mask mask;
	wake_reason_t wr;

	spin_lock_irqsave(&ost_lock, flags);
	for (i = 0; i < NUM_WAKE_SRC; i++) {
		if (ost_wake_src & (1U << i))
			ost_enable_wake_irq(ost_wake_irq[i], false);
	}

	mt6573_irq_mask_all(&mask);

	ost_enable_wake_irq(MT6573_APOST_IRQ_LINE, true);

	/* OST will periodically wake up */
	wr = ost_enter_pwake_pause_mode();

	mt6573_irq_mask_restore(&mask);
	spin_unlock_irqrestore(&ost_lock, flags);

	return wr;
}

void __attribute__((weak)) ost_dpidle_before_wfi(void)
{
}

void __attribute__((weak)) ost_dpidle_after_wfi(void)
{
}

bool __attribute__((weak)) ost_dpidle_can_enter(void)
{
	return true;
}

void ost_go_to_dpidle(void)
{
	u32 wakesrc;
	unsigned long flags;
	struct mtk_irq_mask mask;

	wakesrc = WAKE_SRC_KP | WAKE_SRC_MSDC0 | WAKE_SRC_EINT | WAKE_SRC_RTC |
	          WAKE_SRC_CCIF_MD | WAKE_SRC_ACCDET | WAKE_SRC_XGPT |
	          WAKE_SRC_AFE;

	spin_lock_irqsave(&ost_lock, flags);
	mt6573_irq_mask_all(&mask);

	mt65xx_irq_unmask(MT6573_APOST_IRQ_LINE);

	/* keep SRCLKENA high (26M on) when in sleep mode */
	ost_write16(CCF_CLK_CON, 0x4012);

	ost_write32(OST_UFN, 5);
	ost_write32(OST_AFN, 0);

	/* unmask wakeup sources */
	ost_write32(OST_EVENT_MASK, ~wakesrc);

	/* unmask Pause Interrupt and Pause Abort */
	ost_write32(OST_INT_MASK, 0x0007);

	ost_write16(OST_CON, OST_CON_EN);
	ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_CON_WR | OST_CMD_AFN_WR |
	                     OST_CMD_UFN_WR | OST_CMD_OST_WR);
	while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));

	if (ost_dpidle_can_enter()) {
		ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_PAUSE_STR);
		while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));

		ost_dpidle_before_wfi();

		/* flush L1 and L2 store buffers */
		ost_write_sync();

		/* enter WFI mode */
		__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : : "r" (0));

		ost_dpidle_after_wfi();
	}

	ost_write32(OST_INT_MASK, 0x001f);
	ost_write16(OST_ISR, 0x001f);	/* write 1 clear */

	/* restore CCF_CLK_CON */
	ost_write16(CCF_CLK_CON, 0x4010);
	ost_write_sync();

	mt6573_irq_mask_restore(&mask);
	spin_unlock_irqrestore(&ost_lock, flags);
}

static irqreturn_t __tcmfunc ost_irq_handler(int irq, void *dev_id)
{
	printk("!!! OST ISR SHOULD NOT BE EXECUTED !!!\n");

	spin_lock(&ost_lock);
	ost_write32(OST_INT_MASK, 0x001f);
	ost_write16(OST_ISR, 0x001f);	/* write 1 clear */
	ost_write_sync();
	spin_unlock(&ost_lock);

	return IRQ_HANDLED;
}

static void ost_topsm_init(void)
{
	/* HW controls clock gating */
	ost_write16(RG_CK_ALW_ON, 0);

	/* NOTE: MD may reset RM_SYSCLK_SETTLE later */
	ost_write32(RM_CLK_SETTLE, (RM_PLL2_SETTLE << 24) |
	                           (RM_PLL1_SETTLE << 16) |
	                           RM_SYSCLK_SETTLE);

	/* only AP-OST controls AP MCU/Peripheral power */
	ost_write16(RM_TMR_PWR1, 0x0010);

	ost_write32(TOPSM_DBG, 0);
	ost_write32(FRC_CON, FRC_CON_KEY | FRC_CON_EN);

	/* bit 14: ignore CSYSPWRUPREQ to workaround SIB wakeup issue */
	/* bit 4: allow individual PLL to be forced off */
	ost_write16(CCF_CLK_CON, 0x4010);

	/* SW controls AP MCU */
	ost_write16(RM_PWR_CON6, 0x0185);

	/* only AP-OST controls AP MCU clock */
	ost_write32(RM_PLL_MASK1, 0x000f);

#if AP_FORCE_26M_OFF
	ost_write32(RM_TMR_PWR0, 0);
	ost_write32(RM_PERI_CON, 0x0f01);
	ost_write32(TOPSM_DBG, 0x000f);
	ost_write32(RM_PLL_MASK0, 0x0f0f0f0f);
#endif

	ost_write_sync();
}

void ost_mod_init(void)
{
	int r;
	unsigned long flags;

	hwEnableClock(MT65XX_PDN_PERI_OSTIMER_APARM, "OST");

	spin_lock_irqsave(&ost_lock, flags);
	ost_topsm_init();

	/*
	 * NOTE:
	 * 1. OST_FRM_NUM * OST_FRM_VAL - 30.5176 > OST_FRM_F32K_VAL * 30.5176
	 * 2. OST_FRM_F32K_VAL > RM_SYSCLK_SETTLE + RM_PLL1_SETTLE + RM_PLL2_SETTLE
	 * 3. typical values can cover RM_SYSCLK_SETTLE = 3 ~ 5 ms
	 */
	ost_write16(OST_FRM, OST_FRM_VAL);
	ost_write16(OST_FRM_F32K, (OST_FRM_NUM << 12) | OST_FRM_F32K_VAL);

	ost_write32(OST_UFN, 0);
	ost_write32(OST_AFN, 0);

	/* mask all wakeup sources */
	ost_write32(OST_EVENT_MASK, 0xffffffff);

	ost_write32(OST_INT_MASK, 0x001f);
	ost_write16(OST_ISR, 0x001f);	/* write 1 clear */

	ost_write16(OST_CON, OST_CON_EN);
	ost_write32(OST_CMD, OST_CMD_KEY | OST_CMD_CON_WR | OST_CMD_AFN_WR |
	                     OST_CMD_UFN_WR | OST_CMD_OST_WR);
	while (!(ost_read16(OST_STA) & OST_STA_CMD_CPL));
	spin_unlock_irqrestore(&ost_lock, flags);

	r = request_irq(MT6573_APOST_IRQ_LINE, (irq_handler_t)ost_irq_handler,
	                IRQF_TRIGGER_LOW, OST_NAME, NULL);
	if (r) {
		printk("register OST IRQ failed (%d)\n", r);
		WARN_ON(1);
	}
}

MODULE_AUTHOR("Terry Chang <terry.chang@mediatek.com>");
MODULE_DESCRIPTION("MT6573 OST Driver v2.8.1");
