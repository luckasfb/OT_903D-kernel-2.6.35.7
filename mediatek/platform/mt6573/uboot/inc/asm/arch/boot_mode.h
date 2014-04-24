
#ifndef _MT6573_BOOT_MODE_H_
#define _MT6573_BOOT_MODE_H_

/* MT6573 boot type definitions */
typedef enum 
{
    NORMAL_BOOT = 0,
    META_BOOT = 1,
    RECOVERY_BOOT = 2,    
    SW_REBOOT = 3,
    FACTORY_BOOT = 4,
    ADVMETA_BOOT = 5,
    ATE_FACTORY_BOOT = 6,
    ALARM_BOOT = 7,
    UNKNOWN_BOOT
} BOOTMODE;

typedef struct {
  unsigned int maggic_number;
  BOOTMODE boot_mode;
  unsigned int e_flag;
} BOOT_ARGUMENT;

/* MT6573 chip definitions */
typedef enum 
{
    CHIP_E1 = 0x8a00,    
    CHIP_E2 = 0xca10,
    CHIP_E3 = 0x8a02,    
} CHIP_VER;


#define BOOT_ARGUMENT_MAGIC 0x504c504c

//Mt6573 boot argument Location
#define BOOT_ARGUMENT_LOCATION 0xA0000

extern void boot_mode_select(void);
extern BOOTMODE g_boot_mode;
extern CHIP_VER get_chip_eco_ver(void);

#endif

