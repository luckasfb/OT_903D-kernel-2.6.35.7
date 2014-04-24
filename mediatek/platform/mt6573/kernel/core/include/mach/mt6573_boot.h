
 /* linux/include/asm-arm/arch-mt6573/mt6573_boot.h
  *
  *
  * Copyright (C) 2008,2009 MediaTek <www.mediatek.com>
  * Authors: Infinity Chen <infinity.chen@mediatek.com>  
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  */

#ifndef _MT6573_BOOT_H_
#define _MT6573_BOOT_H_

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

#define BOOT_DEV_NAME 	   	    "BOOT"
#define BOOT_SYSFS 	   		    "boot"
#define BOOT_SYSFS_ATTR         "boot_mode"
#define MD_SYSFS_ATTR           "md"

/* MT6573 chip definitions */
typedef enum 
{
    CHIP_E1 = 0x8a00,    
    CHIP_E2 = 0xca10,
    CHIP_E3 = 0x8a02,    
} CHIP_VER;

/*modem image version definitions*/
typedef enum{
	AP_IMG_INVALID = 0,
	AP_IMG_2G_GPRS,
	AP_IMG_3G_FDD,
	AP_IMG_3G_TDD
}AP_IMG_TYPE;

/*define modem&dsp type, offset and size*/
typedef struct{
	int ap_type;	/*2G_GPRS, 3G_FDD, 3G_FDD*/
	unsigned long md_offset;
	unsigned long md_size;
	unsigned long dsp_offset;
	unsigned long dsp_size;
}MD_LOAD_INFO;


/* this vairable will be set by mt6573_fixup */
extern BOOTMODE g_boot_mode;

extern BOOTMODE get_boot_mode(void);
extern bool is_meta_mode(void);
extern bool is_advanced_meta_mode(void);

extern void boot_register_md_func(ssize_t (*show)(char*), ssize_t (*store)(const char*,size_t));

extern unsigned int get_chip_code(void);
extern CHIP_VER get_chip_eco_ver(void);

//extern AP_IMG_TYPE get_ap_img_ver(void);
extern void get_md_load_info(MD_LOAD_INFO * info);

#endif 

