
#ifndef _MT6573_OST_SM_
#define _MT6573_OST_SM_

#include <linux/kernel.h>
#include <mach/mt6573_reg_base.h>

/* OST registers */
#define OST_CON			(APMCU_OSTIMER_BASE + 0x0000)	/* 16 bits */
#define OST_CMD			(APMCU_OSTIMER_BASE + 0x0004)
#define OST_STA			(APMCU_OSTIMER_BASE + 0x0008)	/* 16 bits */
#define OST_FRM			(APMCU_OSTIMER_BASE + 0x000c)	/* 16 bits */
#define OST_FRM_F32K		(APMCU_OSTIMER_BASE + 0x0010)	/* 16 bits */
#define OST_UFN			(APMCU_OSTIMER_BASE + 0x0014)
#define OST_AFN			(APMCU_OSTIMER_BASE + 0x0018)
#define OST_AFN_DLY		(APMCU_OSTIMER_BASE + 0x001c)
#define OST_UFN_R		(APMCU_OSTIMER_BASE + 0x0020)
#define OST_AFN_R		(APMCU_OSTIMER_BASE + 0x0024)
#define OST_INT_MASK		(APMCU_OSTIMER_BASE + 0x0030)
#define OST_ISR			(APMCU_OSTIMER_BASE + 0x0040)	/* 16 bits */
#define OST_EVENT_MASK		(APMCU_OSTIMER_BASE + 0x0050)
#define OST_WAKEUP_STA		(APMCU_OSTIMER_BASE + 0x0054)
#define OST_DBG_WAKEUP		(APMCU_OSTIMER_BASE + 0x0060)
#define OST_EN_IRQ_MASK		(APMCU_OSTIMER_BASE + 0x0400)	/* 16 bits */

/* TOPSM registers */
#define RM_CLK_SETTLE		(TOPSM_BASE + 0x0000)
#define RM_TMRPWR_SETTLE	(TOPSM_BASE + 0x0004)
#define RM_TMR_TRG0		(TOPSM_BASE + 0x0010)
#define RM_TMR_PWR0		(TOPSM_BASE + 0x0014)
#define RM_TMR_PWR1		(TOPSM_BASE + 0x0018)	/* 16 bits */
#define RM_PERI_CON		(TOPSM_BASE + 0x0030)
#define RM_TMR_SSTA		(TOPSM_BASE + 0x0040)
#define TOPSM_DBG		(TOPSM_BASE + 0x0050)
#define FRC_CON			(TOPSM_BASE + 0x0080)
#define FRC_F32K_FM		(TOPSM_BASE + 0x0084)
#define FRC_VAL_R		(TOPSM_BASE + 0x0088)
#define GPS_SYNC		(TOPSM_BASE + 0x008c)
#define FRC_SYNC1		(TOPSM_BASE + 0x0090)
#define FRC_SYNC2		(TOPSM_BASE + 0x0094)
#define FM_CON			(TOPSM_BASE + 0x00a0)	/* 16 bits */
#define FM_CAL			(TOPSM_BASE + 0x00a4)
#define FM_T0			(TOPSM_BASE + 0x00a8)
#define FM_T1			(TOPSM_BASE + 0x00ac)
#define F32K_CNT		(TOPSM_BASE + 0x0104)
#define CCF_CLK_CON		(TOPSM_BASE + 0x0200)	/* 16 bits */
#define TDD_SYNC		(TOPSM_BASE + 0x0400)
#define RM_PWR_CON0		(TOPSM_BASE + 0x0800)	/* 16 bits */
#define RM_PWR_CON1		(TOPSM_BASE + 0x0804)	/* 16 bits */
#define RM_PWR_CON2		(TOPSM_BASE + 0x0808)	/* 16 bits */
#define RM_PWR_CON3		(TOPSM_BASE + 0x080c)	/* 16 bits */
#define RM_PWR_CON4		(TOPSM_BASE + 0x0810)	/* 16 bits */
#define RM_PWR_CON5		(TOPSM_BASE + 0x0814)	/* 16 bits */
#define RM_PWR_CON6		(TOPSM_BASE + 0x0818)	/* 16 bits */
#define RM_PWR_CON7		(TOPSM_BASE + 0x081c)	/* 16 bits */
#define RM_PWR_STA		(TOPSM_BASE + 0x0820)
#define RM_PLL_MASK0		(TOPSM_BASE + 0x0830)
#define RM_PLL_MASK1		(TOPSM_BASE + 0x0834)
#define CCF_CLK_CON_R		(TOPSM_BASE + 0x0c00)	/* 16 bits */

/* OST wakeup sources */
#define WAKE_SRC_GPT		(1U << 0)
#define WAKE_SRC_TS		(1U << 1)
#define WAKE_SRC_KP		(1U << 2)
#define WAKE_SRC_MSDC0		(1U << 3)
#define WAKE_SRC_MSDC1		(1U << 4)
#define WAKE_SRC_MSDC2		(1U << 5)
#define WAKE_SRC_EINT		(1U << 6)
#define WAKE_SRC_RTC		(1U << 7)
#define WAKE_SRC_CCIF_MD	(1U << 8)
#define WAKE_SRC_ACCDET		(1U << 10)
#define WAKE_SRC_CCIF_DSP	(1U << 11)
#define WAKE_SRC_MSDC3		(1U << 12)
#define WAKE_SRC_XGPT		(1U << 13)
#define WAKE_SRC_LOW_BAT	(1U << 14)
#define WAKE_SRC_AFE		(1U << 21)

#define NUM_WAKE_SRC		22

typedef enum {
	WR_NONE = 0,
	WR_WAKE_SRC = 1,
	WR_LOW_BAT = 2,
} wake_reason_t;

extern wake_reason_t ost_go_to_sleep(void);

extern void ost_dpidle_before_wfi(void);
extern void ost_dpidle_after_wfi(void);
extern bool ost_dpidle_can_enter(void);
extern void ost_go_to_dpidle(void);

extern void ost_mod_init(void);


typedef void (*ccci_callback_t)(void);

extern void slp_set_ccci_callback(ccci_callback_t cc);
extern void slp_mod_init(void);

#endif
