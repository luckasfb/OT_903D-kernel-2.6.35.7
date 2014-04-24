
#include <linux/pm.h>

#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/hardware/cache-l2x0.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"

extern struct sys_timer mt6573_timer;
extern void mt6573_fixup(struct machine_desc *desc, struct tag *tags, char **cmdline, struct meminfo *mi);
extern void mt6573_power_off(void);

void __init mt6573_init(void)
{
    pm_power_off = mt6573_power_off;

#if defined(CONFIG_CACHE_L2X0)
    l2x0_init((void __iomem *)PL310_BASE, 0x01000000, 0xFE7FFFFF);
#endif  /* CONFIG_CACHE_L2X0 */
}

static struct map_desc mt6573_io_desc[] __initdata = 
{
    {
        .virtual = EMI_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(EMI_BASE)),
        .length = SZ_1M,
        .type = MT_DEVICE
    },
    {
        .virtual = AFE_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(AFE_BASE)),
        .length = SZ_4K,
        .type = MT_DEVICE
    },
    {
        .virtual = MM_SYSRAM_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MM_SYSRAM_BASE)),
        .length = 272 * 1024,
        .type = MT_DEVICE_WC
    },
    {
        .virtual = SYSTEM_FLEXL2_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(SYSTEM_FLEXL2_BASE)),
        .length = SZ_128K,
        /* CC: modify for ARM PWR test */
#if 0
        .type = MT_DEVICE_CACHED
#endif
        .type = MT_HIGH_VECTORS
    },
    {
        .virtual = MD_DEVAPC_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MD_DEVAPC_BASE)),
        .length = SZ_1K,
        .type = MT_DEVICE
    },
};

void __init mt6573_map_io(void)
{
    iotable_init(mt6573_io_desc, ARRAY_SIZE(mt6573_io_desc));
}

MACHINE_START(MT6573, "MT6573")
    .phys_io        = IO_VIRT_TO_PHYS(UART1_BASE),
    .io_pg_offst    = (UART1_BASE >> 18) & 0xfffc,
    .boot_params    = PHYS_OFFSET + 0x00000100,
    .map_io         = mt6573_map_io,
    .init_irq       = mt6573_init_irq,
    .timer          = &mt6573_timer,
    .init_machine   = mt6573_init,
    .fixup          = mt6573_fixup
MACHINE_END

