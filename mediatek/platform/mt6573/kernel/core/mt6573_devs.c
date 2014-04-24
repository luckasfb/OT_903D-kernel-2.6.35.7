
#include <linux/autoconf.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/android_pmem.h>
#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <mach/memory.h>
#include <mach/mt6573_boot.h> /*for BOOTMODE */
#include <mach/mt6573_typedefs.h>
#include <linux/sysfs.h>
#include <asm/io.h>
#include <linux/spi/spi.h>
#include <mach/mt6573_irq.h>

#include <linux/usb/android_composite.h>
#include <linux/etherdevice.h>
#include <mtk_usb_custom.h>

extern unsigned long max_pfn;
#define RESERVED_MEM_MODEM   		PHYS_OFFSET
#ifndef CONFIG_RESERVED_MEM_SIZE_FOR_PMEM
#define CONFIG_RESERVED_MEM_SIZE_FOR_PMEM 	0
#endif

#if defined(CONFIG_MTK_FB)
char temp_command_line[1024] = {0};
extern UINT32 DISP_GetVRamSizeBoot(char* cmdline);
extern void   mtkfb_set_lcm_inited(BOOL isLcmInited);
#define RESERVED_MEM_SIZE_FOR_FB (DISP_GetVRamSizeBoot((char*)&temp_command_line))
#else
#define RESERVED_MEM_SIZE_FOR_FB (0x400000)
#endif

unsigned int RESERVED_MEM_SIZE_FOR_PMEM = 0;

#define TOTAL_RESERVED_MEM_SIZE (RESERVED_MEM_SIZE_FOR_PMEM + \
                                 RESERVED_MEM_SIZE_FOR_FB)

#define MAX_PFN        ((max_pfn << PAGE_SHIFT) + PHYS_OFFSET)

#define PMEM_MM_START  (MAX_PFN)
#define PMEM_MM_SIZE   (RESERVED_MEM_SIZE_FOR_PMEM)

#define FB_START       (PMEM_MM_START + PMEM_MM_SIZE)
#define FB_SIZE        (RESERVED_MEM_SIZE_FOR_FB)

#include "mach/memory.h"
#include "mach/irqs.h"
#include "mach/mt6573_reg_base.h"
#include "mach/mt6573_devs.h"
extern BOOTMODE get_boot_mode(void);
extern void adjust_kernel_cmd_line_setting_for_console(char*, char*);
/*NOTICE: the compile option should be defined if EFUSE is programed*/
//#define CONFIG_MTK_USB_UNIQUE_SERIAL 
#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL 
char serial_number[64];
#endif 

#define NUM_USB_FUNCTIONS ARRAY_SIZE(usb_functions)
#define NUM_USB_PRODUCTS  ARRAY_SIZE(usb_products)

/*=======================================================================*/
/* MT6573 USB Support                                                    */
/*=======================================================================*/

/* MT6573 USB Functions and Products Definition */
static char *usb_functions[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
    "rndis", 
#endif
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
    "usb_mass_storage", 
#endif
#ifdef CONFIG_USB_ANDROID_ADB
    "adb", 
#endif
#ifdef CONFIG_USB_ANDROID_ACM 	
    "acm", 
#endif
};
static char *usb_ms[1] = {"usb_mass_storage"};
static char *usb_ms_adb[2] = {"usb_mass_storage", "adb"};
static char *usb_rndis[1] = {"rndis"};
static char *usb_rndis_adb[2] = {"rndis", "adb"};
static char *usb_acm[] = {"acm"};
static char *usb_ms_adb_acm[] = {"usb_mass_storage", "adb", "acm"};
static struct android_usb_product usb_products[] = {
    {
     .product_id = USB_MS_PRODUCT_ID,
     .num_functions = 1,
     .functions = (char **)usb_ms,
     },

    {
     .product_id = USB_MS_ADB_PRODUCT_ID,
     .num_functions = 2,
     .functions = (char **)usb_ms_adb,
     },

    {
     .product_id = USB_RNDIS_PRODUCT_ID,
     .num_functions = 1,
     .functions = (char **)usb_rndis,
     },

    {
     .product_id = USB_RNDIS_ADB_PRODUCT_ID,
     .num_functions = 2,
     .functions = (char **)usb_rndis_adb,
    },
#ifdef CONFIG_USB_ANDROID_ACM    
    {
     .product_id = USB_MS_ADB_ACM_PRODUCT_ID,
     .num_functions = ARRAY_SIZE(usb_ms_adb_acm),
     .functions = (char **)usb_ms_adb_acm,
    },
    {
     .product_id = USB_ACM_PRODUCT_ID,
     .num_functions = ARRAY_SIZE(usb_acm),
     .functions = (char **)usb_acm,
    },
#endif    
};

static struct android_usb_platform_data android_usb_pdata = {
    .num_products = NUM_USB_PRODUCTS,
    .products = usb_products,
    .num_functions = NUM_USB_FUNCTIONS,
    .functions = (char **)usb_functions,
};

static struct usb_ether_platform_data android_usb_eth_pdata = {
    .vendorID =    USB_ETH_VENDORID,
    .vendorDescr = USB_ETH_VENDORDESCR,
};

static struct usb_mass_storage_platform_data  android_usb_ms_pdata = {
    .vendor =  USB_MS_VENDOR,
    .product = USB_MS_PRODUCT,
    .release = USB_MS_RELEASE,
    .nluns = 1,
};

/* MT6573 USB Platform Devices Definition */
struct platform_device mt_device_usbgadget = {
	.name		  = "mt_udc",
	.id		  = -1,
};

struct platform_device android_usb_device = {
    .name = "android_usb",
    .id   =            -1,
    .dev = {
        .platform_data = &android_usb_pdata,
    },
};

struct platform_device android_usb_eth_device = {
    .name = "rndis",
    .id   =            -1,
    .dev = {
        .platform_data = &android_usb_eth_pdata,
    },
};

struct platform_device android_usb_ms_device = {
    .name = "usb_mass_storage",
    .id   =            -1,
    .dev = {
        .platform_data = &android_usb_ms_pdata,
    },
};
/*=======================================================================*/
/* Audio                                                 */
/*=======================================================================*/
static u64        AudDrv_dmamask      = 0xffffffffUL;
static struct platform_device AudDrv_device = {
	.name  = "AudDrv_driver_device",
	.id    = 0,
	.dev   = {
		        .dma_mask = &AudDrv_dmamask,
		        .coherent_dma_mask =  0xffffffffUL
	         }
};


/*=======================================================================*/
/* MT6573 UART Ports                                                     */
/*=======================================================================*/
#if defined(CFG_DEV_UART1)
static struct resource mt6573_resource_uart1[] = {
	{
		.start		= UART1_BASE,
		.end		= UART1_BASE + MT6573_UART_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= MT6573_UART1_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif

#if defined(CFG_DEV_UART2)
static struct resource mt6573_resource_uart2[] = {
	{
		.start		= UART2_BASE,
		.end		= UART2_BASE + MT6573_UART_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= MT6573_UART2_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif

#if defined(CFG_DEV_UART3)
static struct resource mt6573_resource_uart3[] = {
	{
		.start		= UART3_BASE,
		.end		= UART3_BASE + MT6573_UART_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= MT6573_UART3_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif

#if defined(CFG_DEV_UART4)
static struct resource mt6573_resource_uart4[] = {
	{
		.start		= UART4_BASE,
		.end		= UART4_BASE + MT6573_UART_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= MT6573_UART4_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif

static struct platform_device mt6573_device_uart[] = {

    #if defined(CFG_DEV_UART1)
    {
    	.name			= "mt6573-uart",
    	.id				= 0,
    	.num_resources	= ARRAY_SIZE(mt6573_resource_uart1),
    	.resource		= mt6573_resource_uart1,
    },
    #endif
    #if defined(CFG_DEV_UART2)
    {
    	.name			= "mt6573-uart",
    	.id				= 1,
    	.num_resources	= ARRAY_SIZE(mt6573_resource_uart2),
    	.resource		= mt6573_resource_uart2,
    },
    #endif
    #if defined(CFG_DEV_UART3)
    {
    	.name			= "mt6573-uart",
    	.id				= 2,
    	.num_resources	= ARRAY_SIZE(mt6573_resource_uart3),
    	.resource		= mt6573_resource_uart3,
    },
    #endif

    #if defined(CFG_DEV_UART4)
    {
    	.name			= "mt6573-uart",
    	.id				= 3,
    	.num_resources	= ARRAY_SIZE(mt6573_resource_uart4),
    	.resource		= mt6573_resource_uart4,
    },
    #endif
};
/*=======================================================================*/
/* MT6573 GPIO                                                           */
/*=======================================================================*/
struct platform_device gpio_dev =
{
    .name = "mt6573-gpio",
    .id   = -1,
};
/*=======================================================================*/
/* MT6573 Keypad                                                         */
/*=======================================================================*/
#ifdef CONFIG_MTK_KEYPAD
static struct platform_device kpd_pdev = {
	.name	= "mt6573-kpd",
	.id	= -1,
};
#endif

/*=======================================================================*/
/* MT6573 jogball                                                        */
/*=======================================================================*/
#ifdef CONFIG_MOUSE_PANASONIC_EVQWJN
static struct platform_device jbd_pdev = {
	.name = "mt6573-jb",
	.id = -1,
};
#endif

#ifdef CONFIG_RFKILL
/*=======================================================================*/
/* MT6573 RFKill module (BT and WLAN)                                             */
/*=======================================================================*/
/* MT66xx RFKill BT */
struct platform_device mt6573_device_rfkill = {
	.name	       = "mt-rfkill",
	.id            = -1,
};
#endif

#if 1
/*=======================================================================*/
/* MT6573 FM module                                                    */
/*=======================================================================*/
/* MT3326 FM */
struct platform_device mt6573_fm = 
{
    .name = "fm",
    .id   = -1,    
};
#endif

/*=======================================================================*/
/* MT6573 GPS module                                                    */
/*=======================================================================*/
/* MT3326 GPS */
#ifdef CONFIG_MTK_GPS
struct platform_device mt3326_device_gps = {
	.name	       = "mt3326-gps",
	.id            = -1,
	.dev = {
        .platform_data = &mt3326_gps_hw,
    },	
};
#endif

/*=======================================================================*/
/* MT6573 SD Hosts                                                       */
/*=======================================================================*/
#if defined(CFG_DEV_MSDC0)
static struct resource mt6573_resource_sd0[] = {
    {
        .start  = MSDC_BASE,
        .end    = MSDC_BASE + 0x100,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = AP_DMA_BASE + 0x180,
        .end    = AP_DMA_BASE + 0x180 + 0x60,
        .flags  = IORESOURCE_DMA,
    },
    {
        .start  = MT6573_MSDC0_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = MT6573_MSDC0_CD_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
};
#endif

#if defined(CFG_DEV_MSDC1)
static struct resource mt6573_resource_sd1[] = {
    {
        .start  = MSDC2_BASE,
        .end    = MSDC2_BASE + 0x100,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = AP_DMA_BASE + 0x200,
        .end    = AP_DMA_BASE + 0x200 + 0x60,
        .flags  = IORESOURCE_DMA,
    },
    {
        .start  = MT6573_MSDC1_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = MT6573_MSDC1_CD_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
};
#endif

#if defined(CFG_DEV_MSDC2)
static struct resource mt6573_resource_sd2[] = {
    {
        .start  = MSDC3_BASE,
        .end    = MSDC3_BASE + 0x100,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = AP_DMA_BASE + 0x280,
        .end    = AP_DMA_BASE + 0x280 + 0x60,
        .flags  = IORESOURCE_DMA,
    },
    {
        .start  = MT6573_MSDC2_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = MT6573_MSDC2_CD_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
};
#endif

#if defined(CFG_DEV_MSDC3)
static struct resource mt6573_resource_sd3[] = {
    {
        .start  = MSDC4_BASE,
        .end    = MSDC4_BASE + 0x100,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = AP_DMA_BASE + 0x300,
        .end    = AP_DMA_BASE + 0x300 + 0x60,
        .flags  = IORESOURCE_DMA,
    },
    {
        .start  = MT6573_MSDC3_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = MT6573_MSDC3_CD_IRQ_LINE,
        .flags  = IORESOURCE_IRQ,
    },
};
#endif

static struct platform_device mt6573_device_sd[] =
{
#if defined(CFG_DEV_MSDC0)
    {
        .name           = "mt6573-sd",
        .id             = 0,
        .num_resources  = ARRAY_SIZE(mt6573_resource_sd0),
        .resource       = mt6573_resource_sd0,
        .dev = {
            .platform_data = &mt6573_sd0_hw,
        },
    },
#endif
#if defined(CFG_DEV_MSDC1)
    {
        .name           = "mt6573-sd",
        .id             = 1,
        .num_resources  = ARRAY_SIZE(mt6573_resource_sd1),
        .resource       = mt6573_resource_sd1,
        .dev = {
            .platform_data = &mt6573_sd1_hw,
        },
    },
#endif
#if defined(CFG_DEV_MSDC2)
    {
        .name           = "mt6573-sd",
        .id             = 2,
        .num_resources  = ARRAY_SIZE(mt6573_resource_sd2),
        .resource       = mt6573_resource_sd2,
        .dev = {
            .platform_data = &mt6573_sd2_hw,
        },
    },
#endif
#if defined(CFG_DEV_MSDC3)
    {
        .name           = "mt6573-sd",
        .id             = 3,
        .num_resources  = ARRAY_SIZE(mt6573_resource_sd3),
        .resource       = mt6573_resource_sd3,
        .dev = {
            .platform_data = &mt6573_sd3_hw,
        },
    },
#endif
};


/*=======================================================================*/
/* MT6573 NAND                                                           */
/*=======================================================================*/
#if defined(CONFIG_MTK_MTD_NAND)
#define NFI_base    NFI_BASE//0x80032000
#define NFIECC_base NFIECC_BASE//0x80038000
static struct resource mt6573_resource_nand[] = {
	{
		.start		= NFI_base,
		.end		= NFI_base + 0x1A0,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= NFIECC_base,
		.end		= NFIECC_base + 0x150,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= MT6573_NFI_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
	{
		.start		= MT6573_NFIECC_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device mt6573_nand_dev = {
    .name = "mt6573-nand",
    .id   = 0,
   	.num_resources	= ARRAY_SIZE(mt6573_resource_nand),
   	.resource		= mt6573_resource_nand,
    .dev            = {
        .platform_data = &mt6573_nand_hw,
    },
};
#endif

/*=======================================================================*/
/*=======================================================================*/
/* MT6573 I2C                                                            */
/*=======================================================================*/
static struct resource mt6573_resource_i2c1[] = {
	{
		.start		= I2C1_BASE,
		.end		= I2C1_BASE + 0x70,
		.flags		= IORESOURCE_MEM,
	},
    {
        .start		= MT6573_I2C_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct resource mt6573_resource_i2c2[] = {
	{
		.start		= I2C2_BASE,
		.end		= I2C2_BASE + 0x70,
		.flags		= IORESOURCE_MEM,
	},
    {
        .start		= MT6573_I2C_DUAL_IRQ_LINE,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device mt6516_device_i2c[] = {
    {
    	.name	       = "mt6573-i2c",
        .id            = 0,
    	.num_resources = ARRAY_SIZE(mt6573_resource_i2c1),
    	.resource      = mt6573_resource_i2c1,
    },
    {
    	.name	       = "mt6573-i2c",
        .id            = 1,
    	.num_resources = ARRAY_SIZE(mt6573_resource_i2c2),
    	.resource      = mt6573_resource_i2c2,
    },
};

/*=======================================================================*/
/* MT6573 PMEM                                                           */
/*=======================================================================*/
#if defined(CONFIG_ANDROID_PMEM)
static struct android_pmem_platform_data  pdata_multimedia = {
        .name = "pmem_multimedia",
        .no_allocator = 0,
        .cached = 1,
        .buffered = 1
};

static struct platform_device pmem_multimedia_device = {
        .name = "android_pmem",
        .id = 1,
        .dev = { .platform_data = &pdata_multimedia }
};
#endif
/*=======================================================================*/
/* MT6573 ofn                                                           */
/*=======================================================================*/
#if defined(CONFIG_MOUSE_AVAGOTECH_A320)
static struct platform_device ofn_driver =
{
    .name = "mtofn",
    .id   = -1,
};
#endif
/*=======================================================================*/
/* HID Keyboard  add by zhangsg                                                 */
/*=======================================================================*/

static struct platform_device mt_hid_dev = {
    .name = "hid-keyboard",
    .id   = -1,
};
#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL)
/* ========================================================================= */
/* implementation of serial number attribute                                 */
/* ========================================================================= */
#define to_sysinfo_attribute(x) container_of(x, struct sysinfo_attribute, attr)

struct sysinfo_attribute{
    struct attribute attr;
    ssize_t (*show)(char *buf);
    ssize_t (*store)(const char *buf, size_t count);
};

static struct kobject sn_kobj;

static ssize_t sn_show(char *buf){
    return snprintf(buf, 4096, "%s\n", serial_number);
}

static ssize_t sn_store(const char *buf, size_t count){
    return 0;
}

struct sysinfo_attribute sn_attr = {
    .attr = {"SerialNumber", THIS_MODULE, 0644},
    .show = sn_show,
    .store = sn_store
};

static ssize_t sysinfo_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct sysinfo_attribute *sysinfo_attr = to_sysinfo_attribute(attr);
    ssize_t ret = -EIO;

    if(sysinfo_attr->show)
        ret = sysinfo_attr->show(buf);

    return ret;
}

static ssize_t sysinfo_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    struct sysinfo_attribute *sysinfo_attr = to_sysinfo_attribute(attr);
    ssize_t ret = -EIO;

    if(sysinfo_attr->store)
        ret = sysinfo_attr->store(buf, count);
    
    return ret;
}

static struct sysfs_ops sn_sysfs_ops = {
    .show = sysinfo_show,
    .store = sysinfo_store
};

static struct attribute *sn_attrs[] = {
    &sn_attr.attr,
    NULL
};

static struct kobj_type sn_ktype = {
    .sysfs_ops = &sn_sysfs_ops,
    .default_attrs = sn_attrs
};

static char udc_chr[32] = {"ABCDEFGHIJKLMNOPQRSTUVWSYZ456789"};
int get_serial(uint64_t hwkey, uint32_t chipid, char ser[64])
{
    uint16_t hashkey[4];
    int idx, ser_idx;
    uint32_t digit, id;
    uint64_t tmp = hwkey;

    memset(ser, 0x00, 64);
    
    /* split to 4 key with 16-bit width each */
    tmp = hwkey;
    for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
        hashkey[idx] = (uint16_t)(tmp & 0xffff);
        tmp >>= 16;
    }
    
    /* hash the key with chip id */
    id = chipid;
    for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {        
        digit = (id % 10);
        hashkey[idx] = (hashkey[idx] >> digit) | (hashkey[idx] << (16-digit));
        id = (id / 10);
    }    

    /* generate serail using hashkey */    
    ser[0] = 'M';
    ser[1] = 'T';
    ser[2] = '-';
    ser_idx = 3;
    for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
        ser[ser_idx++] = (hashkey[idx] & 0x001f); 
        ser[ser_idx++] = (hashkey[idx] & 0x00f8) >> 3;
        ser[ser_idx++] = (hashkey[idx] & 0x1f00) >> 8; 
        ser[ser_idx++] = (hashkey[idx] & 0xf800) >> 11;
    }
    for (idx = 3; idx < ser_idx; idx++)
        ser[idx] = udc_chr[(int)ser[idx]];
    ser[ser_idx] = 0x00;
    return 0;
}

#endif

/*=======================================================================*/
/*=======================================================================*/
/* Commandline fitlter to choose the supported commands                  */
/*=======================================================================*/
static void cmdline_filter(struct tag *cmdline_tag, char *default_cmdline)
{
	const char *desired_cmds[] = {
	                                 "rdinit=",
                                     "nand_manf_id=",
                                     "nand_dev_id=",
                                     "uboot_ver=",
                                     "uboot_build_ver=",
                                     "jrd_pl_ver=",   // Chu Hongyu adds version on 2011 09 13
        				"jrd_uboot_ver=",// Chu Hongyu adds version on 2011 09 13
        				 "lcm="
			             };

	int i;
	char *cs,*ce;

	cs = cmdline_tag->u.cmdline.cmdline;
	ce = cs;
	while((__u32)ce < (__u32)tag_next(cmdline_tag)) {

	    while(*cs == ' ' || *cs == '\0') {
	    	cs++;
	    	ce = cs;
	    }

	    if (*ce == ' ' || *ce == '\0') {
	    	for (i = 0; i < sizeof(desired_cmds)/sizeof(char *); i++){
	    	    if (memcmp(cs, desired_cmds[i], strlen(desired_cmds[i])) == 0) {
	    	        *ce = '\0';
         	        //Append to the default command line
        	        strcat(default_cmdline, " ");
	    	        strcat(default_cmdline, cs);
	    	    }
	    	}
	    	cs = ce + 1;
	    }
	    ce++;
	}
	if (strlen(default_cmdline) >= COMMAND_LINE_SIZE)
	{
		panic("Command line length is too long.\n\r");
	}
}
/*=======================================================================*/
/* MT6573 fixup function and related codes				 */
/*=======================================================================*/

struct {
	u32 base;
	u32 size;
} bl_fb = {0, 0};

static int use_bl_fb = 0;

static int __init parse_tag_videofb_fixup(const struct tag *tags)
{
	bl_fb.base = tags->u.videolfb.lfb_base;
	bl_fb.size = tags->u.videolfb.lfb_size;
        use_bl_fb++;
	return 0;
}

void mt6573_fixup(struct machine_desc *desc, struct tag *tags, char **cmdline, struct meminfo *mi)
{
    struct tag *cmdline_tag = NULL;
    struct tag *reserved_mem_bank_tag = NULL;
    struct tag *none_tag = NULL;

    int32_t max_limit_size = CONFIG_MAX_DRAM_SIZE_SUPPORT -
                             RESERVED_MEM_MODEM;
    int32_t avail_dram = 0;
    int32_t bl_mem_sz = 0;

	// xuecheng, set cmdline to temp_command_line, for display driver calculate reserved memory size
	struct tag *temp_tags = tags;
	for (; temp_tags->hdr.size; temp_tags = tag_next(temp_tags)) 
	{
		if(temp_tags->hdr.tag == ATAG_CMDLINE)
			cmdline_filter(temp_tags, (char*)&temp_command_line);
	}

    for (; tags->hdr.size; tags = tag_next(tags)) {
        if (tags->hdr.tag == ATAG_MEM) {
	    bl_mem_sz += tags->u.mem.size;

	    /*
             * Modify the memory tag to limit available memory to
             * CONFIG_MAX_DRAM_SIZE_SUPPORT
             */
            if (max_limit_size > 0) {
                if (max_limit_size >= tags->u.mem.size) {
                    max_limit_size -= tags->u.mem.size;
		    avail_dram += tags->u.mem.size;
                } else {
                    tags->u.mem.size = max_limit_size;
		    avail_dram += max_limit_size;
                    max_limit_size = 0;
                }

		// By Keene: 
		// remove this check to avoid calcuate pmem size before we know all dram size
		// Assuming the minimum size of memory bank is 256MB
                //if (tags->u.mem.size >= (TOTAL_RESERVED_MEM_SIZE)) {
                    reserved_mem_bank_tag = tags;
                //}
            } else {
                tags->u.mem.size = 0;
            }
        } else if (tags->hdr.tag == ATAG_CMDLINE) {
            cmdline_tag = tags;
        } else if (tags->hdr.tag == ATAG_BOOT) {
            g_boot_mode = tags->u.boot.bootmode;
        } else if (tags->hdr.tag == ATAG_VIDEOLFB) {
            parse_tag_videofb_fixup(tags);
        }
    }
    /*
    * If the maximum memory size configured in kernel
    * is smaller than the actual size (passed from BL)
    * Still limit the maximum memory size but use the FB
    * initialized by BL
    */
    if (bl_mem_sz >= (CONFIG_MAX_DRAM_SIZE_SUPPORT - RESERVED_MEM_MODEM)) {
	use_bl_fb++;
    }

    /*
     * Setup PMEM size
     */
    if (avail_dram < 0x10000000)
        RESERVED_MEM_SIZE_FOR_PMEM = 0x1700000;
    else 
        RESERVED_MEM_SIZE_FOR_PMEM = 0x1E00000;

    /* Reserve memory in the last bank */
    if (reserved_mem_bank_tag)
        reserved_mem_bank_tag->u.mem.size -= ((__u32)TOTAL_RESERVED_MEM_SIZE);
    else // we should always have reserved memory
    	BUG();

    if(tags->hdr.tag == ATAG_NONE)
	none_tag = tags;

    if (cmdline_tag != NULL) {

        // This function may modify ttyMT3 to ttyMT0 if needed
        adjust_kernel_cmd_line_setting_for_console(cmdline_tag->u.cmdline.cmdline, *cmdline);
        cmdline_filter(cmdline_tag, *cmdline);
        /* Use the default cmdline */
        memcpy((void*)cmdline_tag,
               (void*)tag_next(cmdline_tag),
               /* ATAG_NONE actual size */
               (uint32_t)(none_tag) - (uint32_t)(tag_next(cmdline_tag)) + 8);
    }
}
/*=======================================================================*/
/* MT6573 sensor module                                                  */
/*=======================================================================*/
struct platform_device sensor_gsensor = {
	.name	       = "gsensor",
	.id            = -1,
};

struct platform_device sensor_msensor = {
	.name	       = "msensor",
	.id            = -1,
};

struct platform_device sensor_orientation = {
	.name	       = "orientation",
	.id            = -1,
};

struct platform_device sensor_alsps = {
	.name	       = "als_ps",
	.id            = -1,
};

struct platform_device sensor_gyroscope = {
	.name	       = "gyroscope",
	.id            = -1,
};

struct platform_device sensor_barometer = {
	.name	       = "barometer",
	.id            = -1,
};
/* hwmon sensor */
struct platform_device hwmon_sensor = {
	.name	       = "hwmsensor",
	.id            = -1,
};
/*=======================================================================*/
/* MT6573 FB                                                             */
/*=======================================================================*/

#if defined(CONFIG_MTK_FB)



static u64 mtkfb_dmamask = ~(u32)0;

static struct resource resource_fb[] = {
	{
		.start		= 0, /* Will be redefined later */
		.end		= 0,
		.flags		= IORESOURCE_MEM
	}
};

static struct platform_device mt6573_device_fb = {
    .name = "mtkfb",
    .id   = 0,
    .num_resources = ARRAY_SIZE(resource_fb),
    .resource      = resource_fb,
    .dev = {
        .dma_mask = &mtkfb_dmamask,
        .coherent_dma_mask = 0xffffffff,
    },
};
#endif

#if defined (CONFIG_MT6573_SPI)
static struct resource mt6573_spi_resources[] =
{
	[0]={
		.start = 0x700B2000,
		.end = 0x700B2000+0x0028,
		.flags = IORESOURCE_MEM,
	},
	[1]={
		.start = 133,
		.flags=IORESOURCE_IRQ,
	},
};

static struct platform_device mt6573_spi_device = {
	.name = "mt6573_spi",
	.num_resources = ARRAY_SIZE(mt6573_spi_resources),
	.resource=mt6573_spi_resources
};

static struct spi_board_info spi_board_devs[] __initdata = {
	[0] = {
        	.modalias="spidev",
		.bus_num = 0,
		.chip_select=0,
		.mode = SPI_MODE_3,
	},
};
#endif

#if defined(MTK_TVOUT_SUPPORT) 

static struct resource mt6573_TVOUT_resource[] = {
    [0] = {//TVC
        .start = TVC_BASE,
        .end   = TVC_BASE + 0x78,
        .flags = IORESOURCE_MEM,
    },
    [1] = {//TVR
        .start = TV_ROT_BASE,
        .end   = TV_ROT_BASE + 0x378,
        .flags = IORESOURCE_MEM,
    },
    [2] = {//TVE
        .start = TVE_BASE,
        .end   = TVE_BASE + 0x84,
        .flags = IORESOURCE_MEM,
    },
};

static u64 mt6573_TVOUT_dmamask = ~(u32)0;

static struct platform_device mt6573_TVOUT_dev = {
	.name		  = "TV-out",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(mt6573_TVOUT_resource),
	.resource	  = mt6573_TVOUT_resource,
	.dev              = {
		.dma_mask = &mt6573_TVOUT_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};
#endif

static struct resource mtk_mau_resource[] = {
    [0] = {//MPU
        .start = MPU_BASE,
        .end   = MPU_BASE + 0x44,
        .flags = IORESOURCE_MEM,
    },
    [1] = {//MAU1
        .start = MAU1_BASE,
        .end   = MAU1_BASE + 0x44,
        .flags = IORESOURCE_MEM,
    },
    [2] = {//TMAU2
        .start = MAU2_BASE,
        .end   = MAU2_BASE + 0x44,
        .flags = IORESOURCE_MEM,
    },
};

static u64 mtk_mau_dmamask = ~(u32)0;

static struct platform_device mtk_mau_dev = {
	.name		  = "MTK_MAU",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(mtk_mau_resource),
	.resource	  = mtk_mau_resource,
	.dev              = {
		.dma_mask = &mtk_mau_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};




//#if defined(CONFIG_MDP_MT6573)
static struct resource mt6573_MDP_resource[] = {
    [0] = {//BRZ
        .start = BRZ_BASE,
        .end   = BRZ_BASE + 0x34,
        .flags = IORESOURCE_MEM,
    },
    [1] = {//CRZ
        .start = CRZ_BASE,
        .end   = CRZ_BASE + 0xB8,
        .flags = IORESOURCE_MEM,
    },
    [2] = {//PRZ0
        .start = PRZ0_BASE,
        .end   = PRZ0_BASE + 0xB8,
        .flags = IORESOURCE_MEM,
    },
    [3] = {//PRZ1
        .start = PRZ1_BASE,
        .end   = PRZ1_BASE + 0xB8,
        .flags = IORESOURCE_MEM,
    },
    [4] = {//IPP
        .start = IMGPROC_BASE,
        .end   = IMGPROC_BASE + 0x32C,
        .flags = IORESOURCE_MEM,
    },
    [5] = {//MOUT
        .start = MMSYS1_CONFIG_BASE,
        .end   = MMSYS1_CONFIG_BASE + 0x54C,
        .flags = IORESOURCE_MEM,
    },
    [6] = {//ROTDMA0
        .start = ROT_DMA0_BASE,
        .end   = ROT_DMA0_BASE + 0x37C,
        .flags = IORESOURCE_MEM,
    },
    [7] = {//ROTDMA1
        .start = ROT_DMA1_BASE,
        .end   = ROT_DMA1_BASE + 0x37C,
        .flags = IORESOURCE_MEM,
    },
    [8] = {//ROTDMA2
        .start = ROT_DMA2_BASE,
        .end   = ROT_DMA2_BASE + 0x37C,
        .flags = IORESOURCE_MEM,
    },
    [9] = {//ROTDMA3
        .start = ROT_DMA3_BASE,
        .end   = ROT_DMA3_BASE + 0x37C,
        .flags = IORESOURCE_MEM,
    },
    [10] = {//RDMA0
        .start = R_DMA0_BASE,
        .end   = R_DMA0_BASE + 0x38C,
        .flags = IORESOURCE_MEM,
    },
    [11] = {//RDMA1
        .start = R_DMA1_BASE,
        .end   = R_DMA1_BASE + 0x38C,
        .flags = IORESOURCE_MEM,
    },
    [12] = {//JPGDMA
        .start = JPG_DMA_BASE,
        .end   = JPG_DMA_BASE + 0x24,
        .flags = IORESOURCE_MEM,
    },
    [13] = {//VRZ
        .start = VRZ_BASE,
        .end   = VRZ_BASE + 0x60,
        .flags = IORESOURCE_MEM,
    },
    [14] = {//OVL
        .start = OVL_BASE,
        .end   = OVL_BASE + 0x14C,
        .flags = IORESOURCE_MEM,
    },
    [15] = {//ROTDMA IRQ line
        .start = MT6573_ROT_DMA_IRQ_LINE,
        .end   = MT6573_ROT_DMA_IRQ_LINE,
        .flags = IORESOURCE_IRQ,
    },
    [16] = {//RDMA IRQ line
        .start = MT6573_R_DMA_IRQ_LINE,
        .end   = MT6573_R_DMA_IRQ_LINE,
        .flags = IORESOURCE_IRQ,
    },
    [17] = {//RESZ IRQ line
        .start = MT6573_RESZ_IRQ_LINE,
        .end   = MT6573_RESZ_IRQ_LINE,
        .flags = IORESOURCE_IRQ,
    },
    [18] = {//CRZ IRQ line
        .start = MT6573_CRZ_IRQ_LINE,
        .end   = MT6573_CRZ_IRQ_LINE,
        .flags = IORESOURCE_IRQ,
    }
};

static u64 mt6573_MDP_dmamask = ~(u32)0;

static struct platform_device mt6573_MDP_dev = {
	.name		  = "mt6573-MDP",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(mt6573_MDP_resource),
	.resource	  = mt6573_MDP_resource,
	.dev              = {
		.dma_mask = &mt6573_MDP_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};
//#endif
static struct platform_device mt6573_SYSRAM_dev = {
	.name		  = "mt6573-SYSRAM",
	.id		  = 0,
};

/*=======================================================================*/
/* MT6573 ISP                                                            */
/*=======================================================================*/
static struct resource mt_resource_isp[] = {
    [0] = { // ISP configuration
        .start = CAM_BASE,
        .end   = CAM_BASE + 0x818,
        .flags = IORESOURCE_MEM,
    },
    [1] = { // statistic result
        .start = ISPMEM_BASE,
        .end   = ISPMEM_BASE + 0x39C,
        .flags = IORESOURCE_MEM,
    },
    [2] = { // ISP IRQ
        .start = MT6573_CAM_IRQ_LINE,
        .flags = IORESOURCE_IRQ,
    }
};
static u64 mt_isp_dmamask = ~(u32) 0;
//
static struct platform_device mt_isp_dev = {
	.name		   = "mt-isp",
	.id		       = 0,
	.num_resources = ARRAY_SIZE(mt_resource_isp),
	.resource	   = mt_resource_isp,
	.dev           = {
		.dma_mask  = &mt_isp_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

/*=======================================================================*/
/* MT6573 EIS                                                            */
/*=======================================================================*/
static struct resource mt_resource_eis[] = {
    [0] = { // EIS configuration
        .start = EIS_BASE,
        .end   = EIS_BASE + 0x2C,
        .flags = IORESOURCE_MEM,
    }
};
static u64 mt_eis_dmamask = ~(u32) 0;
//
static struct platform_device mt_eis_dev = {
	.name		   = "mt-eis",
	.id		       = 0,
	.num_resources = ARRAY_SIZE(mt_resource_eis),
	.resource	   = mt_resource_eis,
	.dev           = {
		.dma_mask  = &mt_eis_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

/*=======================================================================*/
/* Image sensor                                                        */
/*=======================================================================*/
static struct platform_device sensor_dev = {
	.name		  = "image_sensor",
	.id		  = -1,
};

/*=======================================================================*/
/* Lens actuator                                                        */
/*=======================================================================*/
static struct platform_device actuator_dev = {
	.name		  = "lens_actuator",
	.id		  = -1,
};

/*=======================================================================*/
/* MT6573 Touch Panel                                                    */
/*=======================================================================*/
static struct platform_device mtk_tpd_dev = {
    .name   = "mtk-tpd",
    .id     = -1,
};

/*=======================================================================*/
/* MT6573 Battery                                                        */
/*=======================================================================*/
struct platform_device MT6573_battery_device = {
    .name   = "mt6573-battery",
    .id	    = -1,
};

/*=======================================================================*/
/* MT6573 Battery Notify                                                 */
/*=======================================================================*/
struct platform_device MT_batteryNotify_device = {
    .name   = "mt-battery",
    .id	    = -1,
};

/*=======================================================================*/
static struct platform_device nfc_pn544_dev = {
    .name   = "pn544",
    .id     = -1,
};


/* MT6573 Board Device Initialization                                    */
/*=======================================================================*/
__init int mt6573_board_init(void)
{
    /*
     * NoteXXX: mt65xx_board_init() is used for chip-dependent code.
     *          It is suggested to put driver code in this function to do:
     *          1). Capability structure of platform devices.
     *          2). Define platform devices with their resources.
     *          3). Register MT65XX platform devices.
     *
     *          board specific code is suggested to put in custom_board_init().
     */

    int i = 0, retval = 0;

#if defined(CONFIG_ANDROID_PMEM)
    pdata_multimedia.start = PMEM_MM_START;;
    pdata_multimedia.size = PMEM_MM_SIZE;
    printk("PMEM start: 0x%lx size: 0x%lx\n", pdata_multimedia.start, pdata_multimedia.size);

    retval = platform_device_register(&pmem_multimedia_device);
    if (retval != 0){
       return retval;
    }
#endif

#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL)   
    {
        uint64_t key;
        key = (uint32_t)__raw_readl(0xF7024144);
        key = (key << 32) | ((uint32_t)__raw_readl(0xF7024140));
				if(key != 0)
	        get_serial(key, 6573, serial_number);
  			else
  				memcpy(serial_number,"0123456789ABCDEF",16);  
        retval = kobject_init_and_add(&sn_kobj, &sn_ktype, NULL, "SystemInfo");
        if (retval < 0) {
            printk("[%s] fail to add kobject\n", "SystemInfo");
            return retval;
        } 
    }
#endif     

#if defined(CONFIG_MTK_SERIAL)
    for (i = 0; i < ARRAY_SIZE(mt6573_device_uart); i++){
        retval = platform_device_register(&mt6573_device_uart[i]);
        if (retval != 0){
            return retval;
        }
    }
#endif

#if 1         
	retval = platform_device_register(&AudDrv_device);
	printk("AudDrv_driver_device \n!");
   if (retval != 0)
      return retval;            
#endif

#if defined(CONFIG_MTK_MTD_NAND)
    retval = platform_device_register(&mt6573_nand_dev);
    if (retval != 0) {
        printk(KERN_ERR "register nand device fail\n");
        return retval;
    }
#endif

	retval = platform_device_register(&gpio_dev);
	if (retval != 0){
		return retval;
	}
	
#ifdef CONFIG_MTK_KEYPAD
	retval = platform_device_register(&kpd_pdev);
	if (retval != 0) {
		return retval;
	}
#endif

#ifdef CONFIG_MOUSE_PANASONIC_EVQWJN
	retval = platform_device_register(&jbd_pdev);
	if (retval != 0) {
		return retval;
	}
#endif

#ifdef CONFIG_MTK_SMART_BATTERY
  retval = platform_device_register(&MT6573_battery_device);
  if (retval != 0) {
	  printk("[mt6573_devs] Unable to device register(%d)\n", retval);
	  return retval;
  }
  else
  {
  	printk("[mt6573_devs] MT6573_battery_device OK\n");
  }

  retval = platform_device_register(&MT_batteryNotify_device);
  if (retval != 0) {
	  printk("[mt6573_devs] Unable to device register(%d)\n", retval);
	  return retval;
  }
  else
  {
  	printk("[mt6573_devs] MT_batteryNotify_device OK\n");
  }
#endif
 
#if defined(CONFIG_MTK_I2C)
		for (i = 0; i < ARRAY_SIZE(mt6516_device_i2c); i++){
			retval = platform_device_register(&mt6516_device_i2c[i]);
			if (retval != 0){
				return retval;
			}
		}
#endif

#if defined(CONFIG_MTK_MMC)
	for (i = 0; i < ARRAY_SIZE(mt6573_device_sd); i++){
		retval = platform_device_register(&mt6573_device_sd[i]);
		if (retval != 0){
			 return retval;
		}
	}
#endif

#if defined(CONFIG_KEYBOARD_HID)
	retval = platform_device_register(&mt_hid_dev);
	if (retval != 0){
		return retval;
	}	
#endif

#if defined(CONFIG_MTK_TOUCHPANEL)
	retval = platform_device_register(&mtk_tpd_dev);
	if (retval != 0){
		return retval;
	}
#endif
#if defined(CONFIG_MOUSE_AVAGOTECH_A320)
    retval = platform_device_register(&ofn_driver);
    if (retval != 0){
        return retval;
    }
#endif
#if defined(CONFIG_MTK_FB)
    /* 
     * Bypass matching the frame buffer info. between boot loader and kernel 
     * if the limited memory size of the kernel is smaller than the 
     * memory size from bootloader
     */
    if (((bl_fb.base == FB_START) && (bl_fb.size == FB_SIZE)) || 
         (use_bl_fb == 2)) {
        printk("FB is initialized by BL(%d)\n", use_bl_fb);
        mtkfb_set_lcm_inited(TRUE);
    } else if ((bl_fb.base == 0) && (bl_fb.size == 0)) {
        printk("FB is not initialized(%d)\n", use_bl_fb);
        mtkfb_set_lcm_inited(FALSE);
    } else {
        printk(
"******************************************************************************\n"
"   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n"
"******************************************************************************\n"
"\n"
"  The default FB base & size values are not matched between BL and kernel\n"
"    - BOOTLD: start 0x%08x, size %d\n"
"    - KERNEL: start 0x%08lx, size %d\n"
"\n"
"  If you see this warning message, please update your uboot.\n"
"\n"
"******************************************************************************\n"
"   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n"
"******************************************************************************\n"
"\n",   bl_fb.base, bl_fb.size, FB_START, FB_SIZE);

        {
            int delay_sec = 5;
            
            while (delay_sec >= 0) {
                printk("\rcontinue after %d seconds ...", delay_sec--);
                mdelay(1000);
            }
            printk("\n");
        }
#if 0
        panic("The default base & size values are not matched "
              "between BL and kernel\n");
#endif
    }

    resource_fb[0].start = FB_START;
    resource_fb[0].end   = FB_START + FB_SIZE - 1;

    printk("FB start: 0x%x end: 0x%x\n", resource_fb[0].start,
                                         resource_fb[0].end);

    retval = platform_device_register(&mt6573_device_fb);
    if (retval != 0) {
         return retval;
    }
#endif

#if defined(MTK_TVOUT_SUPPORT)
    retval = platform_device_register(&mt6573_TVOUT_dev);
	printk("register TV-out device\n");
    if (retval != 0) {
         return retval;
    }
#endif

    retval = platform_device_register(&mtk_mau_dev);
    printk("register MTK_MAU device\n");
    if (retval != 0) {
        return retval;
    }


#if defined(CONFIG_MTK_USB_GADGET)
	retval = platform_device_register(&mt_device_usbgadget);
	if (retval != 0){
        return retval;
	}

#endif

#if defined(CONFIG_USB_ANDROID)
#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL)
    {   /*bootmode*/
        BOOTMODE bootmode = get_boot_mode();
        if ((bootmode != META_BOOT) && (bootmode != FACTORY_BOOT))
            android_usb_pdata.serial_number = serial_number;
        else
            printk("check bootmode(%d)\n", bootmode);
    }
#endif
    android_usb_pdata.functions = (char**)usb_functions;
    retval = platform_device_register(&android_usb_device);
    if(retval != 0){
        return retval;
    }

#endif

#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
    retval = platform_device_register(&android_usb_ms_device);
    if(retval != 0){
        return retval;
    }
#endif

#if defined(CONFIG_USB_ANDROID_RNDIS)
    random_ether_addr((u8 *)android_usb_eth_pdata.ethaddr);

    retval = platform_device_register(&android_usb_eth_device);
    if(retval != 0){
        return retval;
    }
#endif



//#if defined(CONFIG_MDP_MT6573)
	retval = platform_device_register(&mt6573_MDP_dev);
	if (retval != 0){
		return retval;
	}
//#endif
	retval = platform_device_register(&mt6573_SYSRAM_dev);
	if (retval != 0){
		return retval;
	}


#if defined(MTK_SENSOR_SUPPORT)

	retval = platform_device_register(&hwmon_sensor);
	printk("hwmon_sensor device!");
	if (retval != 0)
		return retval;

#if defined(CUSTOM_KERNEL_ACCELEROMETER)
	retval = platform_device_register(&sensor_gsensor);
		printk("sensor_gsensor device!");
	if (retval != 0)
		return retval;
#endif

#if defined(CUSTOM_KERNEL_MAGNETOMETER)
	retval = platform_device_register(&sensor_msensor);
		printk("sensor_msensor device!");
	if (retval != 0)
		return retval;
		
	retval = platform_device_register(&sensor_orientation);
		printk("sensor_osensor device!");
	if (retval != 0)
		return retval;
		
#endif

#if defined(CUSTOM_KERNEL_GYROSCOPE)
	retval = platform_device_register(&sensor_gyroscope);
		printk("sensor_gyroscope device!");
	if (retval != 0)
		return retval;
#endif

#if defined(CUSTOM_KERNEL_BAROMETER)
	retval = platform_device_register(&sensor_barometer);
		printk("sensor_barometer device!");
	if (retval != 0)
		return retval;
#endif

#if defined(CUSTOM_KERNEL_ALSPS)
	retval = platform_device_register(&sensor_alsps);
		printk("sensor_alsps device!");
	if (retval != 0)
		return retval;
#endif
#endif

#if 1 ///defined(CONFIG_VIDEO_CAPTURE_DRIVERS)
    retval = platform_device_register(&sensor_dev);
    if (retval != 0){
    	return retval;
    }
#endif

#if 1  //defined(CONFIG_ACTUATOR)
    retval = platform_device_register(&actuator_dev);
    if (retval != 0){
        return retval;
    }
#endif


#if 1 //defined(CONFIG_ISP_MT6573)
    retval = platform_device_register(&mt_isp_dev);
    if (retval != 0){
        return retval;
    }
#endif

#if 1
    retval = platform_device_register(&mt_eis_dev);
    if (retval != 0){
        return retval;
    }
#endif

#ifdef CONFIG_RFKILL
    retval = platform_device_register(&mt6573_device_rfkill);
    if (retval != 0){
		return retval;
	}
#endif

#if defined(CONFIG_MTK_GPS)
	retval = platform_device_register(&mt3326_device_gps);
	if (retval != 0){
		return retval;
	}	
#endif

#if defined(CONFIG_MT6573_SPI)
    spi_register_board_info(spi_board_devs, ARRAY_SIZE(spi_board_devs));
    platform_device_register(&mt6573_spi_device);
#endif

#if 1 //defined(CONFIG_FM_RADIO_MT6573)
    retval = platform_device_register(&mt6573_fm);
	printk("mt6573_fm device!");
    if (retval != 0)
        return retval;
#endif

//#if defined(CONFIG_NFC_PN544) //NFC
#if (MTK_NFC_SUPPORT == yes)
	retval = platform_device_register(&nfc_pn544_dev);
	printk("nfc_pn544_dev register ret %d", retval);
	if (retval != 0){
		return retval;
	}
#endif

    return 0;
}

int is_pmem_range(unsigned long *base, unsigned long size)
{
        unsigned long start = (unsigned long)base;
        unsigned long end = start + size;

        //printk("[PMEM] start=0x%p,end=0x%p,size=%d\n", start, end, size);
        //printk("[PMEM] PMEM_MM_START=0x%p,PMEM_MM_SIZE=%d\n", PMEM_MM_START, PMEM_MM_SIZE);

        if (start < PMEM_MM_START)
                return 0;
        if (end >= PMEM_MM_START + PMEM_MM_SIZE)
                return 0;

        return 1;
}
EXPORT_SYMBOL(is_pmem_range);

void get_pmem_range(unsigned long * pu4StartAddr, unsigned long * pu4Size)
{
    *pu4StartAddr = PMEM_MM_START;
    *pu4Size = PMEM_MM_SIZE;
}
EXPORT_SYMBOL(get_pmem_range);

unsigned int get_memory_size (void)
{
    printk ("total size :0x%x-0x%x-0x%x\n", CONFIG_MAX_DRAM_SIZE_SUPPORT, TOTAL_RESERVED_MEM_SIZE, RESERVED_MEM_MODEM) ;
    return (CONFIG_MAX_DRAM_SIZE_SUPPORT) - (TOTAL_RESERVED_MEM_SIZE) - (RESERVED_MEM_MODEM) ;
}
EXPORT_SYMBOL(get_memory_size) ;

#include <asm/sections.h>
void get_text_region (unsigned int *s, unsigned int *e)
{
    *s = (unsigned int)_text, *e=(unsigned int)_etext ;
}
EXPORT_SYMBOL(get_text_region) ;
