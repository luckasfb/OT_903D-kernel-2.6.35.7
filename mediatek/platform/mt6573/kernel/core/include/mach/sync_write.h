
#ifndef __MT6573_SYNC_WRITE_H__
#define __MT6573_SYNC_WRITE_H__

#if defined(__KERNEL__)

#include <linux/io.h>
#include <asm/cacheflush.h>
#include <asm/system.h>


#define mt65xx_reg_sync_writel(v, a) \
        do {    \
            writel((v), (a));   \
            dsb();  \
            outer_sync();   \
        } while (0)

#define mt65xx_reg_sync_writew(v, a) \
        do {    \
            writew((v), (a));   \
            dsb();  \
            outer_sync();   \
        } while (0)

#define mt65xx_reg_sync_writeb(v, a) \
        do {    \
            writeb((v), (a));   \
            dsb();  \
            outer_sync();   \
        } while (0)

#else   /* __KERNEL__ */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define dsb()   \
        do {    \
            __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                  : : "r" (0) : "memory"); \
        } while (0)

#define outer_sync()    \
        do {    \
            int fd; \
            char buf[] = "1";   \
            fd = open("/sys/bus/platform/drivers/outercache/outer_sync", O_WRONLY); \
            if (fd != -1) {  \
                write(fd, buf, strlen(buf)); \
            }   \
        } while (0)

#define mt65xx_reg_sync_writel(v, a) \
        do {    \
            *(volatile unsigned int *)(a) = (v);    \
            __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                  : : "r" (0) : "memory"); \
            outer_sync();   \
        } while (0)

#define mt65xx_reg_sync_writew(v, a) \
        do {    \
            *(volatile unsigned short *)(a) = (v);    \
            __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                  : : "r" (0) : "memory"); \
            outer_sync();   \
        } while (0)

#define mt65xx_reg_sync_writeb(v, a) \
        do {    \
            *(volatile unsigned char *)(a) = (v);    \
            __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                  : : "r" (0) : "memory"); \
            outer_sync();   \
        } while (0)

#endif  /* __KERNEL__ */

#endif  /* !__MT6573_SYNC_WRITE_H__ */

