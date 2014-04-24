
#ifndef __MT6573_MEMORY_H__
#define __MT6573_MEMORY_H__


#if defined (MODEM_2G)
#define PHYS_OFFSET 0x00A00000
#elif defined (MODEM_3G)
#define PHYS_OFFSET 0x01600000
#else
#define PHYS_OFFSET 0x01600000
#endif

#ifdef CONFIG_HAVE_TCM 
 /*
 * TCM memory whereabouts
 */
#define ITCM_SIZE	0x00004000
#define ITCM_OFFSET	(PAGE_OFFSET - ITCM_SIZE)
#define ITCM_END	(PAGE_OFFSET - 1)
#define DTCM_SIZE	0x00004000
#define DTCM_OFFSET	(ITCM_OFFSET - DTCM_SIZE)
#define DTCM_END	(ITCM_OFFSET - 1)
#endif


/* IO_VIRT = 0xF0000000 | (IO_PHYS[31:28] << 24) | IO_PHYS[27:0] */
#define IO_VIRT_TO_PHYS(v) ((((v) & 0x0f000000) << 4) | ((v) & 0x00ffffff))

#endif  /* !__MT6573_MEMORY_H__ */

