
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>


extern void flush_dcache_all(void);

/*This can be used ONLY by the M4U driver!*/
void inner_dcache_flush_all(void)
{
  flush_dcache_all();
}

EXPORT_SYMBOL(inner_dcache_flush_all);
