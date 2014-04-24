
#include "mt6573_reg_base.h"
#include "irqs.h"

/* leave disable_fiq as an empty macro */
.macro disable_fiq
.endm

.macro  get_irqnr_preamble, base, tmp
.endm

.macro  arch_ret_to_user, tmp1, tmp2
.endm

.macro get_irqnr_and_base, irqnr, irqstat, base, tmp
    ldr \base, =IRQ_STA2
    ldr \irqnr, [\base]
    mov \irqstat, \irqnr

    @ test irqstat
8:
    bic \irqstat, \irqstat, #0xFF
    teq \irqstat, #0x100
    .endm

