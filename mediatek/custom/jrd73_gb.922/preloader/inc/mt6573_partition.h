
#ifndef __MT6573_PARTITION_H__
#define __MT6573_PARTITION_H__

//#include <part.h>
#include "mt6573.h"

#define PART_PRELOADER           "PRELOADER"
#define PART_DSP_DL                 "DSP_DL"
#define PART_MBOOT                  "MBOOT"
#define PART_MBL                    "MBL"
#define PART_UBOOT                  "UBOOT"
#define PART_SECURE                 "SECCNFG"
#define PART_LOGO                   "LOGO"
#define PART_CUST                   "CUSTOME"
#define PART_BACKUP                 "BACKUP"
#define PART_IMAGE                  "IMAGE"
#define PART_BOOTIMG                "BOOTIMG"
#define PART_USER                   "USER"
#define PART_ANDSYSIMG              "ANDSYSIMG"
#define PART_CUSTPACK               "CUSTPACK"
#define PART_CACHE                  "CACHE"
#define PART_MISC                   "MISC"
#define PART_NVRAM                  "NVRAM"
#define PART_RECOVERY               "RECOVERY"
#define PART_EXPDB                  "EXPDB"
#define PART_AUTHEN_FILE            "AUTHEN_FILE"
#define PART_SECSTATIC              "SEC_RO"
#define PART_KERNEL                 "KERNEL"
#define PART_ROOTFS                 "ROOTFS"
#define PART_PRO_INFO			"PRO_INFO"

#define KB                          (1024)
#define MB                          (1024 * KB)
#define GB                          (1024 * MB)

#define PART_MAX_COUNT              20

#define PART_FLAG_NONE              0
#define PART_FLAG_LEFT              0x1
#define PART_FLAG_END               0x2


#define PART_MAGIC                  0x58881688

/*=======================================================================*/
/* Partitions                                                            */
/*=======================================================================*/
#define CFG_NAND_BOOT
#if defined(CFG_NAND_BOOT)
#define CFG_USE_MBL_PARTITION
#define CFG_USE_UBOOT_PARTITION
#define CFG_USE_SECURE_PARTITION
#define CFG_USE_LOGO_PARTITION
//----------------------------------------
// use Android Boot Image instead of kernel and rootfs
#define CFG_USE_BOOTIMG_PARTITION
//----------------------------------------
#define CFG_USE_RECOVERY_PARTITION
#define CFG_USE_SECSTATIC_PARTITION
#define CFG_USE_MISC_PARTITION
#define CFG_USE_NVRAM_PARTITION
#define CFG_USE_CACHE_PARTITION
#define CFG_USE_ANDROID_SYSIMG_PARTITION
#define CFG_USE_CUSTPACK_PARTITION
#define CFG_USE_EXPDB_PARTITION
#define CFG_USE_USER_PARTITION
#elif defined (CFG_MMC_BOOT)
#define CFG_USE_MBL_PARTITION
#define CFG_USE_UBOOT_PARTITION
#define CFG_USE_CUSTOME_PARTITION
#define CFG_USE_KERNEL_PARTITION
#define CFG_USE_ROOTFS_GZ_PARTITION
#define CFG_USE_LOGO_PARTITION
#define CFG_USE_USER_PARTITION
#endif

typedef union
{
    struct
    {
        unsigned int magic;     /* partition magic */
        unsigned int dsize;     /* partition data size */
        char name[32];          /* partition name */
    } info;
} part_hdr_t;

typedef struct
{
    unsigned char *name;        /* partition name */
    unsigned long pgnum;        /* partition pages */
    unsigned long flags;        /* partition flags */
    unsigned long startblk;     /* partition start blk */
    char secHeader;
} part_t;


extern unsigned int PAGE_SIZE;
extern unsigned int BLOCK_SIZE;

extern part_t *mt6573_part_get_partition (char *name);
extern void mt6573_part_init (void);
extern void mt6573_part_dump (void);

#endif /* __MT6573_PARTITION_H__ */
