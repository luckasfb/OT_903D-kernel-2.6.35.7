
#ifndef __ARCH_ARM_MACH_BOARD_H
#define __ARCH_ARM_MACH_BOARD_H

#include <linux/autoconf.h>
#include <linux/pm.h>
//#include <mach/mt6573.h>
#include <board-custom.h>

typedef void (*sdio_irq_handler_t)(void*);  /* external irq handler */
typedef void (*pm_callback_t)(pm_message_t state, void *data);

#define MSDC_CD_PIN_EN      (1 << 0)  /* card detection pin is wired   */
#define MSDC_WP_PIN_EN      (1 << 1)  /* write protection pin is wired */
#define MSDC_RST_PIN_EN     (1 << 2)  /* emmc reset pin is wired       */
#define MSDC_SDIO_IRQ       (1 << 3)  /* use internal sdio irq (bus)   */
#define MSDC_EXT_SDIO_IRQ   (1 << 4)  /* use external sdio irq         */
#define MSDC_REMOVABLE      (1 << 5)  /* removable slot                */
#define MSDC_SYS_SUSPEND    (1 << 6)  /* suspended by system           */
#define MSDC_HIGHSPEED      (1 << 7)  /* high-speed mode support       */

/* configure the output driving capacity and slew rate */
#define MSDC_ODC_4MA        (0x0)
#define MSDC_ODC_8MA        (0x4)
#define MSDC_ODC_12MA       (0x2)
#define MSDC_ODC_16MA       (0x6)
#define MSDC_ODC_SLEW_FAST  (0)
#define MSDC_ODC_SLEW_SLOW  (1)

#define MSDC_CMD_PIN        (0)
#define MSDC_DAT_PIN        (1)
#define MSDC_CD_PIN         (2)
#define MSDC_WP_PIN         (3)

enum {
    MSDC_CLKSRC_98MHZ = 0,  /* 98.3MHz */
    MSDC_CLKSRC_81MHZ = 1,  /* 81.9MHz */
    MSDC_CLKSRC_70MHZ = 2,  /* 70.2MHz */
    MSDC_CLKSRC_61MHZ = 3   /* 61.4MHz */
};

enum {
    MSDC_PULL_RES_NONE = 0, /* No pull up/down resister */
    MSDC_PULL_RES_47K  = 2, /* 47k pull up/down resister */
    MSDC_PULL_RES_23K  = 3, /* 23.5k pull up/down resister */
};

enum {
    EDGE_RISING  = 0,
    EDGE_FALLING = 1
};

struct mt6573_sd_host_hw {
    unsigned char  clk_src;          /* host clock source */
    unsigned char  cmd_edge;         /* command latch edge */
    unsigned char  data_edge;        /* data latch edge */
    unsigned char  cmd_odc;          /* command driving capability */
    unsigned char  data_odc;         /* data driving capability */
    unsigned char  cmd_slew_rate;    /* command slew rate */
    unsigned char  data_slew_rate;   /* data slew rate */
    unsigned char  cmd_pull_res;     /* command pin pull resistor */
    unsigned char  dat_pull_res;     /* data pin pull resistor */
    unsigned char  clk_pull_res;     /* clk pin pull resistor */
    unsigned char  rst_wp_pull_res;  /* reset/wp pin pull resistor */
    unsigned char  padding;          /* unused padding */
    unsigned long  flags;            /* hardware capability flags */
    unsigned long  data_pins;        /* data pins */
    unsigned long  data_offset;      /* data address offset */

    /* config gpio pull mode */
    void (*config_gpio_pin)(int type, int pull);

    /* external power control for card */
    void (*ext_power_on)(void);
    void (*ext_power_off)(void);

    /* external sdio irq operations */
    void (*request_sdio_eirq)(sdio_irq_handler_t sdio_irq_handler, void *data);
    void (*enable_sdio_eirq)(void);
    void (*disable_sdio_eirq)(void);

    /* external cd irq operations */
    void (*request_cd_eirq)(sdio_irq_handler_t cd_irq_handler, void *data);
    void (*enable_cd_eirq)(void);
    void (*disable_cd_eirq)(void);
    int  (*get_cd_status)(void);
    
    /* power management callback for external module */
    void (*register_pm)(pm_callback_t pm_cb, void *data);
};

extern struct mt6573_sd_host_hw mt6573_sd0_hw;
extern struct mt6573_sd_host_hw mt6573_sd1_hw;
extern struct mt6573_sd_host_hw mt6573_sd2_hw;
extern struct mt6573_sd_host_hw mt6573_sd3_hw;


/*GPS driver*/
#define GPS_FLAG_FORCE_OFF  0x0001
struct mt3326_gps_hardware {
    int (*ext_power_on)(int);
    int (*ext_power_off)(int);
};
extern struct mt3326_gps_hardware mt3326_gps_hw;

/* NAND driver */
struct mt6573_nand_host_hw {
    unsigned int nfi_bus_width;		    /* NFI_BUS_WIDTH */ 
	unsigned int nfi_access_timing;		/* NFI_ACCESS_TIMING */  
	unsigned int nfi_cs_num;			/* NFI_CS_NUM */
	unsigned int nand_sec_size;			/* NAND_SECTOR_SIZE */
	unsigned int nand_sec_shift;		/* NAND_SECTOR_SHIFT */
	unsigned int nand_ecc_size;
	unsigned int nand_ecc_bytes;
	unsigned int nand_ecc_mode;
};
extern struct mt6573_nand_host_hw mt6573_nand_hw;

#if 0

/*gsensor driver*/
#define GSENSOR_DATA_NUM (3)
struct gsensor_hardware {
    int i2c_num;
    int direction;
    s16 offset[GSENSOR_DATA_NUM+1]; /*+1: for alignment*/
    // TODO: add external interrupt
};


/*msensor driver*/
struct msensor_hardware {
    int i2c_num;
    int direction;
};
#endif

#endif /* __ARCH_ARM_MACH_BOARD_H */

