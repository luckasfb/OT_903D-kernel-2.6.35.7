
#ifndef __MT6573_IRQS_H__
#define __MT6573_IRQS_H__

#include "mt6573_irq.h"


#define NR_IRQS NR_MT6573_IRQ_LINE
#define MT65xx_EDGE_SENSITIVE 0
#define MT65xx_LEVEL_SENSITIVE 1

#if !defined(__ASSEMBLY__)


struct mtk_irq_mask
{
    unsigned int header;   /* for error checking */
    __u32 mask0;
    __u32 mask1;
    __u32 mask2;
    __u32 mask3;
    __u32 mask4;
    unsigned int footer;   /* for error checking */
};


extern void mt6573_irq_mask(unsigned int irq_line);
extern void mt6573_irq_unmask(unsigned int irq_line);
extern void mt6573_irq_ack(unsigned int irq_line);
extern void mt6573_irq_set_sens(unsigned int irq_line, unsigned int sens);
extern void mt6573_init_irq(void);


#define mt65xx_irq_mask(l) mt6573_irq_mask((l))
#define mt65xx_irq_unmask(l) mt6573_irq_unmask((l))
#define mt65xx_irq_ack(l) mt6573_irq_ack((l))
#define mt65xx_irq_set_sens(l, s) mt6573_irq_set_sens((l), (s))

#endif  /* !__ASSEMBLY__ */

#endif  /* !__MT6573_IRQS_H__ */

