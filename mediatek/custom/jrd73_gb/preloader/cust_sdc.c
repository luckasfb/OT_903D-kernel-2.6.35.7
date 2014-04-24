
#include <cust_sdc.h>

#define CFG_DEV_MSDC0
//#define CFG_DEV_MSDC1
//#define CFG_DEV_MSDC2

#if defined(CFG_DEV_MSDC0)
static struct msdc_hw msdc_hw0 = {
    .clk_src        = MSDC_CLKSRC_98MHZ,
    .cmd_edge       = EDGE_RISING,
    .data_edge      = EDGE_RISING,
    .cmd_odc        = MSDC_ODC_16MA,
    .data_odc       = MSDC_ODC_16MA,
    .cmd_slew_rate  = MSDC_ODC_SLEW_FAST,
    .data_slew_rate = MSDC_ODC_SLEW_FAST,
    .cmd_pull_res   = MSDC_PULL_RES_23K,
    .dat_pull_res   = MSDC_PULL_RES_23K,
    .clk_pull_res   = MSDC_PULL_RES_23K,
    .rst_wp_pull_res= MSDC_PULL_RES_23K,
    .boot_type      = RAW_BOOT,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_RST_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,
};
#endif
#if defined(CFG_DEV_MSDC1)
static struct msdc_hw msdc_hw1 = {
    .clk_src        = MSDC_CLKSRC_98MHZ,
    .cmd_edge       = EDGE_RISING,
    .data_edge      = EDGE_RISING,
    .cmd_odc        = MSDC_ODC_16MA,
    .data_odc       = MSDC_ODC_16MA,
    .cmd_slew_rate  = MSDC_ODC_SLEW_FAST,
    .data_slew_rate = MSDC_ODC_SLEW_FAST,
    .cmd_pull_res   = MSDC_PULL_RES_23K,
    .dat_pull_res   = MSDC_PULL_RES_23K,
    .clk_pull_res   = MSDC_PULL_RES_23K,
    .rst_wp_pull_res= MSDC_PULL_RES_23K,
    .boot_type      = NON_BOOTABLE,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,
};
#endif
#if defined(CFG_DEV_MSDC2)
static struct msdc_hw msdc_hw2 = {
    .clk_src        = MSDC_CLKSRC_98MHZ,
    .cmd_edge       = EDGE_RISING,
    .data_edge      = EDGE_RISING,
    .cmd_odc        = MSDC_ODC_16MA,
    .data_odc       = MSDC_ODC_16MA,
    .cmd_slew_rate  = MSDC_ODC_SLEW_FAST,
    .data_slew_rate = MSDC_ODC_SLEW_FAST,
    .cmd_pull_res   = MSDC_PULL_RES_23K,
    .dat_pull_res   = MSDC_PULL_RES_23K,
    .clk_pull_res   = MSDC_PULL_RES_23K,
    .rst_wp_pull_res= MSDC_PULL_RES_23K,
    .boot_type      = NON_BOOTABLE,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND,
};
#endif
#if defined(CFG_DEV_MSDC3)
static struct msdc_hw msdc_hw3 = {
    .clk_src        = MSDC_CLKSRC_98MHZ,
    .cmd_edge       = EDGE_RISING,
    .data_edge      = EDGE_RISING,
    .cmd_odc        = MSDC_ODC_16MA,
    .data_odc       = MSDC_ODC_16MA,
    .cmd_slew_rate  = MSDC_ODC_SLEW_FAST,
    .data_slew_rate = MSDC_ODC_SLEW_FAST,
    .cmd_pull_res   = MSDC_PULL_RES_23K,
    .dat_pull_res   = MSDC_PULL_RES_23K,
    .clk_pull_res   = MSDC_PULL_RES_23K,
    .rst_wp_pull_res= MSDC_PULL_RES_23K,
    .boot_type      = NON_BOOTABLE,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,
};
#endif

struct msdc_hw *cust_msdc_hw[] = 
{
#if defined(CFG_DEV_MSDC0)
    &msdc_hw0,
#else
    NULL,
#endif
#if defined(CFG_DEV_MSDC1)
    &msdc_hw1,
#else
    NULL,
#endif
#if defined(CFG_DEV_MSDC2)
    &msdc_hw2,
#else
    NULL,
#endif
#if defined(CFG_DEV_MSDC3)
    &msdc_hw3,
#else
    NULL,
#endif
};

