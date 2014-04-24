
#ifndef __MT6573_TIMER_H__
#define __MT6573_TIMER_H__

#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>


typedef void (*clock_init_func)(void);

struct mt65xx_clock
{
    struct clock_event_device clockevent;
    struct clocksource clocksource;
    struct irqaction irq;
    clock_init_func init_func;
};

#endif  /* !__MT6573_TIMER_H__ */

