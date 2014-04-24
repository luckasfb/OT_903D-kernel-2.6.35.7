

#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>

void XGPT_OneShot_IRQ(kal_uint32 ms)
{
    UINT32 cnt1, cnt2, div;
    
    DRV_WriteReg32(MT6573_XGPT6_CON, 0x0002);   /* stop and clear */
    
    /* SW workaround in E1 */
    cnt1 = DRV_Reg32(MT6573_XGPT6_COUNT);
    while (1) {
        cnt2 = DRV_Reg32(MT6573_XGPT6_COUNT);
        if (cnt2 == cnt1 && cnt2 == 0)
            break;
        cnt1 = cnt2;
    }    
    
    DRV_WriteReg32(MT6573_XGPT_IRQACK, 0x0020); /* ack old irq */

    DRV_SetReg32(MT6573_XGPT_IRQEN, 0x0020);
    DRV_WriteReg32(MT6573_XGPT6_PRESCALE, 0x0007);  /* 256 */
    DRV_WriteReg32(MT6573_XGPT6_COMPARE, 256 * ms / 1000);
    //DRV_WriteReg32(MT6573_XGPT6_COMPARE, 256);    /* 1s */

    DRV_WriteReg32(MT6573_XGPT6_CON, 0x0003);   /* start one-shot mode */
}