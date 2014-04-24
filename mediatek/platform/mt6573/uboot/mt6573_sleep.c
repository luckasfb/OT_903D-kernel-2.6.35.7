

#include <common.h>

#include <asm/io.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/mt6573_rtc.h>

#define MT6573_APXGPT_IRQ_LINE  0x4
#define MT6573_APOST_IRQ_LINE   0x6

#define DRV_Reg(addr)           (*(volatile UINT32 *)(addr))
#define DRV_WriteReg(addr,data) ((*(volatile UINT32 *)(addr)) = (UINT32)(data))

#define IRQ_MASK0           (CIRQ_BASE + 0x00000000)
#define IRQ_MASK1           (CIRQ_BASE + 0x00000004)
#define IRQ_MASK2           (CIRQ_BASE + 0x00000008)
#define IRQ_MASK3           (CIRQ_BASE + 0x0000000C)
#define IRQ_MASK4           (CIRQ_BASE + 0x00000010)
#define IRQ_MASK_SET0       (CIRQ_BASE + 0x00000020)
#define IRQ_MASK_SET1       (CIRQ_BASE + 0x00000024)
#define IRQ_MASK_SET2       (CIRQ_BASE + 0x00000028)
#define IRQ_MASK_SET3       (CIRQ_BASE + 0x0000002c)
#define IRQ_MASK_SET4       (CIRQ_BASE + 0x00000030)
#define IRQ_MASK_CLR0       (CIRQ_BASE + 0x00000040)
#define IRQ_MASK_CLR1       (CIRQ_BASE + 0x00000044)
#define IRQ_MASK_CLR2       (CIRQ_BASE + 0x00000048)
#define IRQ_MASK_CLR3       (CIRQ_BASE + 0x0000004C)
#define IRQ_MASK_CLR4       (CIRQ_BASE + 0x00000050)

#define OST_CON             (OST_BASE + 0x0000)	/* 16 bits */
#define OST_CMD             (OST_BASE + 0x0004)
#define OST_STA             (OST_BASE + 0x0008)	/* 16 bits */
#define OST_FRM             (OST_BASE + 0x000c)	/* 16 bits */
#define OST_FRM_F32K        (OST_BASE + 0x0010)	/* 16 bits */
#define OST_UFN             (OST_BASE + 0x0014)
#define OST_AFN             (OST_BASE + 0x0018)
#define OST_AFN_DLY         (OST_BASE + 0x001c)
#define OST_UFN_R           (OST_BASE + 0x0020)
#define OST_AFN_R           (OST_BASE + 0x0024)
#define OST_INT_MASK        (OST_BASE + 0x0030)
#define OST_ISR             (OST_BASE + 0x0040)	/* 16 bits */
#define OST_EVENT_MASK      (OST_BASE + 0x0050)
#define OST_WAKEUP_STA      (OST_BASE + 0x0054)
#define OST_DBG_WAKEUP      (OST_BASE + 0x0060)

#define FRC_CON_EN          (0x1 << 0)
#define FRC_CON_KEY         (0x6573 << 16)

#define OST_CON_EN          (0x1 << 0)
#define OST_CON_UFN_DOWN    (0x1 << 1)

#define OST_CMD_PAUSE_STR   (0x1 << 0)
#define OST_CMD_OST_WR      (0x1 << 2)
#define OST_CMD_UFN_WR      (0x1 << 13)
#define OST_CMD_AFN_WR      (0x1 << 14)
#define OST_CMD_CON_WR      (0x1 << 15)
#define OST_CMD_KEY         (0x6573 << 16)

#define OST_STA_CMD_CPL     (0x1 << 1)

#define RM_SYSCLK_SETTLE    160	/* 160T 32K */
#define RM_PLL1_SETTLE      3   /* 3T 32K */
#define RM_PLL2_SETTLE      3   /* 3T 32K */

#define OST_FRM_NUM         2
#define OST_FRM_F32K_VAL    295	/* 295T 32K */

/* TOPSM registers */
#define RM_CLK_SETTLE       (TOPSM_BASE + 0x0000)
#define RM_TMRPWR_SETTLE    (TOPSM_BASE + 0x0004)
#define RM_TMR_TRG0         (TOPSM_BASE + 0x0010)
#define RM_TMR_PWR0         (TOPSM_BASE + 0x0014)
#define RM_TMR_PWR1         (TOPSM_BASE + 0x0018)	/* 16 bits */
#define RM_PERI_CON         (TOPSM_BASE + 0x0030)
#define RM_TMR_SSTA         (TOPSM_BASE + 0x0040)
#define TOPSM_DBG           (TOPSM_BASE + 0x0050)
#define FRC_CON             (TOPSM_BASE + 0x0080)
#define FRC_F32K_FM         (TOPSM_BASE + 0x0084)
#define FRC_VAL_R           (TOPSM_BASE + 0x0088)
#define FRC_SYNC0           (TOPSM_BASE + 0x008c)
#define FRC_SYNC1           (TOPSM_BASE + 0x0090)
#define FRC_SYNC2           (TOPSM_BASE + 0x0094)
#define FM_CON              (TOPSM_BASE + 0x00a0)	/* 16 bits */
#define FM_CAL              (TOPSM_BASE + 0x00a4)
#define FM_T0               (TOPSM_BASE + 0x00a8)
#define FM_T1               (TOPSM_BASE + 0x00ac)
#define F32K_CNT            (TOPSM_BASE + 0x0104)
#define CCF_CLK_CON         (TOPSM_BASE + 0x0200)	/* 16 bits */
#define RM_PWR_CON0         (TOPSM_BASE + 0x0800)	/* 16 bits */
#define RM_PWR_CON1         (TOPSM_BASE + 0x0804)	/* 16 bits */
#define RM_PWR_CON2         (TOPSM_BASE + 0x0808)	/* 16 bits */
#define RM_PWR_CON3         (TOPSM_BASE + 0x080c)	/* 16 bits */
#define RM_PWR_CON4         (TOPSM_BASE + 0x0810)	/* 16 bits */
#define RM_PWR_CON5         (TOPSM_BASE + 0x0814)	/* 16 bits */
#define RM_PWR_CON6         (TOPSM_BASE + 0x0818)	/* 16 bits */
#define RM_PWR_CON7         (TOPSM_BASE + 0x081c)	/* 16 bits */
#define RM_PWR_STA          (TOPSM_BASE + 0x0820)
#define RM_PLL_MASK0        (TOPSM_BASE + 0x0830)
#define RM_PLL_MASK1        (TOPSM_BASE + 0x0834)

/* APCONFIG registers */
#define RG_CK_ALW_ON        (CONFIG_BASE + 0x0124)
#define RG_CK_DCM_EN        (CONFIG_BASE + 0x0128)
#define APMCU_CG_CLR1       (CONFIG_BASE + 0x0318)

/* PMU registers */
#define VA28_CON1           (PMU_BASE + 0x0714)
#define VAPROC_CON1         (PMU_BASE + 0x0944)
#define VCORE_CON1          (PMU_BASE + 0x0904)
#define VA25_CON1           (PMU_BASE + 0x0724)

/* OST wake-up sources */
#define WAKE_SRC_GPT        (0x1 << 0)
#define WAKE_SRC_ADC        (0x1 << 1)
#define WAKE_SRC_KP         (0x1 << 2)
#define WAKE_SRC_MSDC0      (0x1 << 3)
#define WAKE_SRC_MSDC1      (0x1 << 4)
#define WAKE_SRC_MSDC2      (0x1 << 5)
#define WAKE_SRC_EINT       (0x1 << 6)
#define WAKE_SRC_RTC        (0x1 << 7)
#define WAKE_SRC_CCIF_MD    (0x1 << 8)
#define WAKE_SRC_XTRIG      (0x1 << 9)
#define WAKE_SRC_ACCDET     (0x1 << 10)
#define WAKE_SRC_CCIF_DSP   (0x1 << 11)
#define WAKE_SRC_MSDC3      (0x1 << 12)
#define WAKE_SRC_XGPT       (0x1 << 13)
#define WAKE_SRC_LOW_BAT    (0x1 << 14)
#define WAKE_SRC_AFE        (0x1 << 15)
#define WAKE_SRC_FIQ        (0x1 << 16)
#define WAKE_SRC_IRQ        (0x1 << 17)

#define NUM_WAKE_SRC        (16 + 2)

#define IRQ_EOIOH0          (CIRQ_BASE + 0x0160)
#define IRQ_EOIOH1          (CIRQ_BASE + 0x0164)
#define IRQ_EOIOH2          (CIRQ_BASE + 0x0168)
#define IRQ_EOIOH3          (CIRQ_BASE + 0x016C)
#define IRQ_EOIOH4          (CIRQ_BASE + 0x0170)

extern void XGPT_OneShot_IRQ(kal_uint32 ms);

static UINT32 apost_wake_src = (
    WAKE_SRC_XGPT 
);

static u16 apost_wake_irq[NUM_WAKE_SRC] = {
    [13] = MT6573_APXGPT_IRQ_LINE,
};

void mt6573_ost_register_dump(void)
{
    #if 0
    printf("\n");
    printf("OST_CON (%x)        = %x\n", OST_CON, DRV_Reg(OST_CON));
    printf("OST_CMD (%x)        = %x\n", OST_CMD, DRV_Reg(OST_CMD));
    printf("OST_STA (%x)        = %x\n", OST_STA, DRV_Reg(OST_STA));
    printf("OST_FRM (%x)        = %x\n", OST_FRM, DRV_Reg(OST_FRM));
    printf("OST_FRM_F32K (%x)   = %x\n", OST_FRM_F32K, DRV_Reg(OST_FRM_F32K));
    printf("OST_UFN (%x)        = %x\n", OST_UFN, DRV_Reg(OST_UFN));
    printf("OST_AFN (%x)        = %x\n", OST_AFN, DRV_Reg(OST_AFN));
    printf("OST_AFN_DLY (%x)    = %x\n", OST_AFN_DLY, DRV_Reg(OST_AFN_DLY));
    printf("OST_UFN_R (%x)      = %x\n", OST_UFN_R, DRV_Reg(OST_UFN_R));
    printf("OST_AFN_R (%x)      = %x\n", OST_AFN_R, DRV_Reg(OST_AFN_R));
    printf("OST_INT_MASK (%x)   = %x\n", OST_INT_MASK, DRV_Reg(OST_INT_MASK));
    printf("OST_ISR (%x)        = %x\n", OST_ISR, DRV_Reg(OST_ISR));
    printf("OST_EVENT_MASK (%x) = %x\n", OST_EVENT_MASK, DRV_Reg(OST_EVENT_MASK));
    printf("OST_WAKEUP_STA (%x) = %x\n", OST_WAKEUP_STA, DRV_Reg(OST_WAKEUP_STA));
    printf("OST_DBG_WAKEUP (%x) = %x\n", OST_DBG_WAKEUP, DRV_Reg(OST_DBG_WAKEUP));
    
    printf("\n");
    printf("RM_CLK_SETTLE (%x)  = %x\n", RM_CLK_SETTLE, DRV_Reg(RM_CLK_SETTLE));
    printf("RM_TMRPWR_SETTLE (%x) = %x\n", RM_TMRPWR_SETTLE, DRV_Reg(RM_TMRPWR_SETTLE));
    printf("RM_TMR_TRG0 (%x)    = %x\n", RM_TMR_TRG0, DRV_Reg(RM_TMR_TRG0));
    printf("RM_TMR_PWR0 (%x)    = %x\n", RM_TMR_PWR0, DRV_Reg(RM_TMR_PWR0));
    printf("RM_TMR_PWR1 (%x)    = %x\n", RM_TMR_PWR1, DRV_Reg(RM_TMR_PWR1));
    printf("RM_PERI_CON (%x)    = %x\n", RM_PERI_CON, DRV_Reg(RM_PERI_CON));
    printf("TOPSM_DBG (%x)      = %x\n", TOPSM_DBG, DRV_Reg(TOPSM_DBG));
    printf("FRC_CON (%x)        = %x\n", FRC_CON, DRV_Reg(FRC_CON));
    printf("CCF_CLK_CON (%x)    = %x\n", CCF_CLK_CON, DRV_Reg(CCF_CLK_CON));
    printf("RM_PWR_CON0 (%x)    = %x\n", RM_PWR_CON0, DRV_Reg(RM_PWR_CON0));
    printf("RM_PWR_CON1 (%x)    = %x\n", RM_PWR_CON1, DRV_Reg(RM_PWR_CON1));
    printf("RM_PWR_CON2 (%x)    = %x\n", RM_PWR_CON2, DRV_Reg(RM_PWR_CON2));
    printf("RM_PWR_CON3 (%x)    = %x\n", RM_PWR_CON3, DRV_Reg(RM_PWR_CON3));
    printf("RM_PWR_CON4 (%x)    = %x\n", RM_PWR_CON4, DRV_Reg(RM_PWR_CON4));
    printf("RM_PWR_CON5 (%x)    = %x\n", RM_PWR_CON5, DRV_Reg(RM_PWR_CON5));
    printf("RM_PWR_CON6 (%x)    = %x\n", RM_PWR_CON6, DRV_Reg(RM_PWR_CON6));
    printf("RM_PWR_CON7 (%x)    = %x\n", RM_PWR_CON7, DRV_Reg(RM_PWR_CON7));
    printf("RM_PWR_STA (%x)     = %x\n", RM_PWR_STA, DRV_Reg(RM_PWR_STA));
    printf("RM_PLL_MASK0 (%x)   = %x\n", RM_PLL_MASK0, DRV_Reg(RM_PLL_MASK0));
    printf("RM_PLL_MASK1 (%x)   = %x\n", RM_PLL_MASK1, DRV_Reg(RM_PLL_MASK1));

    printf("\n");
    printf("RG_CK_ALW_ON (%x)   = %x\n", RG_CK_ALW_ON, DRV_Reg(RG_CK_ALW_ON));
    
    printf("\n");
    printf("VA28_CON1 (%x)      = %x\n", VA28_CON1, DRV_Reg(VA28_CON1));
    printf("VAPROC_CON1 (%x)    = %x\n", VAPROC_CON1, DRV_Reg(VAPROC_CON1));
    printf("VCORE_CON1 (%x)     = %x\n", VCORE_CON1, DRV_Reg(VCORE_CON1));
    printf("VA25_CON1 (%x)      = %x\n", VA25_CON1, DRV_Reg(VA25_CON1));
    #endif    
}

static void apost_topsm_init(void)
{
    /* HW controls clock gating */
    DRV_WriteReg16(RG_CK_ALW_ON, 0);
 
    DRV_WriteReg32(RM_CLK_SETTLE, ((RM_PLL2_SETTLE << 24) | (RM_PLL1_SETTLE << 16) |
                   RM_SYSCLK_SETTLE));
    
    /* only AP-OST controls AP MCU/Peripheral power */
    DRV_WriteReg16(RM_TMR_PWR1, 0x0010);
    
    DRV_WriteReg32(TOPSM_DBG, 0);
    DRV_WriteReg32(FRC_CON, FRC_CON_KEY | FRC_CON_EN);
    
    /* allow individual PLL to be forced off */
    DRV_WriteReg16(CCF_CLK_CON, 0x4010);
	
    /* SW controls AP MCU */
    DRV_WriteReg16(RM_PWR_CON6, 0x0185);
    
    /* only AP-OST controls AP MCU clock */
    DRV_WriteReg32(RM_PLL_MASK1, 0x000f);
}

static void apost_pmu_init(void)
{
    UINT16 temp;
    
    if (get_chip_eco_ver() == CHIP_E1)
    {
        /* VA25 MODE_SEL, disable low power mode for VA25 */
        temp = DRV_Reg16(VA25_CON1);
        temp = temp | (0x1 << 4);
        temp = temp & 0xfffffffb;
        DRV_WriteReg16(VA25_CON1, temp);
    }
        
    /* clear CCI_SRCLKEN to enable HW sleep-mode control */
    temp = DRV_Reg16(VA28_CON1);
    temp &= ~(0x1 << 8);
    DRV_WriteReg16(VA28_CON1, temp);
    
    /* VAPROC = 0.9V in sleep mode */
    temp = DRV_Reg16(VAPROC_CON1);
    temp &= 0xfe0f;
    temp |= (20 << 4);
    DRV_WriteReg16(VAPROC_CON1, temp);
    
    /* VCORE = 0.9V in sleep mode */
    temp = DRV_Reg16(VCORE_CON1);
    temp &= 0xfe0f;
    temp |= (20 << 4);
    DRV_WriteReg16(VCORE_CON1, temp);
}

void apost_hw_init(void)
{
    apost_pmu_init();
    apost_topsm_init();    

    /* enable AP-OST clock */
    DRV_WriteReg32(APMCU_CG_CLR1, 0x0001);
    
    /*
     * NOTE:
     * 1. OST_FRM_NUM * OST_FRM_VAL - 30.5176 > OST_FRM_F32K_VAL * 30.5176
     * 2. OST_FRM_F32K_VAL > RM_SYSCLK_SETTLE + RM_PLL1_SETTLE + RM_PLL2_SETTLE
     * 3. typical values can cover RM_SYSCLK_SETTLE = 3 ~ 5 ms
     */
    DRV_WriteReg16(OST_FRM, 4615);	/* us */
    DRV_WriteReg16(OST_FRM_F32K, ((OST_FRM_NUM << 12) | OST_FRM_F32K_VAL));
    
    DRV_WriteReg32(OST_UFN, 5);
    DRV_WriteReg32(OST_AFN, 0);
    
    /* mask all wakeup sources */
    DRV_WriteReg32(OST_EVENT_MASK, 0xffffffff);
	
    DRV_WriteReg32(OST_INT_MASK, 0x001f);
    DRV_WriteReg16(OST_ISR, 0x001f);	/* write 1 clear */
    
    DRV_WriteReg16(OST_CON, OST_CON_EN);
    DRV_WriteReg32(OST_CMD, OST_CMD_KEY | OST_CMD_UFN_WR | OST_CMD_AFN_WR |
             OST_CMD_CON_WR | OST_CMD_OST_WR);
    while (!(DRV_Reg16(OST_STA) & OST_STA_CMD_CPL));

    printf("[BATTERY] apost_hw_init : Done\r\n");
}

void mt6573_sleep(UINT32 ms, kal_bool ap_force)
{
    UINT32 i;
    UINT32 mask0, mask1, mask2, mask3, mask4;

    rtc_writeif_lock();

    disable_interrupts();
    
    for (i = 0; i < NUM_WAKE_SRC; i++) {
        if (apost_wake_src & (0x1 << i))
            DRV_WriteReg((IRQ_MASK_CLR0 + (apost_wake_irq[i] >> 5) * 4), (1 << (apost_wake_irq[i] & 0x0000001F)));
    }
    
    DRV_WriteReg32(IRQ_EOIOH0, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH1, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH2, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH3, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH4, 0xFFFFFFFF);
    
    mask0 = DRV_Reg32(IRQ_MASK0);
    mask1 = DRV_Reg32(IRQ_MASK1);
    mask2 = DRV_Reg32(IRQ_MASK2);
    mask3 = DRV_Reg32(IRQ_MASK3);
    mask4 = DRV_Reg32(IRQ_MASK4);
    
    DRV_WriteReg32(IRQ_MASK_SET0, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_MASK_SET1, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_MASK_SET2, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_MASK_SET3, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_MASK_SET4, 0xFFFFFFFF);
    
    DRV_WriteReg((IRQ_MASK_CLR0 + (MT6573_APOST_IRQ_LINE >> 5) * 4), (1 << (MT6573_APOST_IRQ_LINE & 0x0000001F)));
    
    XGPT_OneShot_IRQ(ms);
    
    /* AP FORCE 26Mhz OFF */
    if (ap_force)
    {
        DRV_WriteReg32(RM_TMR_PWR0, 0);
        DRV_WriteReg32(RM_PERI_CON, 0x0f01);
        DRV_WriteReg32(TOPSM_DBG, 0x000f);
        DRV_WriteReg32(RM_PLL_MASK0, 0x0f0f0f0f);
    }
    
    /* AP FORCE 26MHz ON */
    #if 0
    DRV_WriteReg16(CCF_CLK_CON, 0x0012);
    #endif 
    
    DRV_WriteReg32(OST_UFN, 5);
    DRV_WriteReg32(OST_AFN, 0);
	
    /* unmask wake-up sources */
    DRV_WriteReg32(OST_EVENT_MASK, ~apost_wake_src);
    
    DRV_WriteReg32(OST_INT_MASK, 0x0007);
    
    DRV_WriteReg32(OST_CMD, OST_CMD_KEY | OST_CMD_UFN_WR | OST_CMD_AFN_WR | OST_CMD_OST_WR);
    while (!(DRV_Reg16(OST_STA) & OST_STA_CMD_CPL));
    
    DRV_WriteReg32(OST_CMD, OST_CMD_KEY | OST_CMD_PAUSE_STR);
    while (!(DRV_Reg16(OST_STA) & OST_STA_CMD_CPL));
    
    mt6573_ost_register_dump();
    
    __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : : "r" (0));
    
    DRV_WriteReg32(OST_INT_MASK, 0x001f);
    DRV_WriteReg16(OST_ISR, 0x001f);	/* write 1 clear */
    
    DRV_WriteReg32(MT6573_XGPT_IRQEN, 0x0000);
    DRV_WriteReg32(MT6573_XGPT_IRQACK, 0x0020); /* ack old irq */
    
    mt6573_ost_register_dump();
    
    /* AP FORCE 26Mhz OFF */
    if (ap_force)
    {
        DRV_WriteReg32(RM_TMR_PWR0, 0x08040102);
        DRV_WriteReg32(RM_PERI_CON, 0x0301);
        DRV_WriteReg32(TOPSM_DBG, 0);
        DRV_WriteReg32(RM_PLL_MASK0, 0);
    }
    
    /* AP FORCE 26MHz ON */
    #if 0
    DRV_WriteReg16(CCF_CLK_CON, 0x0010);
    #endif
    
    DRV_WriteReg32(IRQ_EOIOH0, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH1, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH2, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH3, 0xFFFFFFFF);
    DRV_WriteReg32(IRQ_EOIOH4, 0xFFFFFFFF);
    
    DRV_WriteReg32(IRQ_MASK_CLR0, ~mask0);
    DRV_WriteReg32(IRQ_MASK_CLR1, ~mask1);
    DRV_WriteReg32(IRQ_MASK_CLR2, ~mask2);
    DRV_WriteReg32(IRQ_MASK_CLR3, ~mask3);
    DRV_WriteReg32(IRQ_MASK_CLR4, ~mask4);
    
    enable_interrupts();    

    rtc_writeif_unlock();
}
