
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include "mach/mt6573_reg_base.h"
#include "mach/mt6573_emi_bwl.h"
#include "mach/sync_write.h"

DECLARE_MUTEX(emi_bwl_sem);

static struct device_driver mem_bw_ctrl =
{
    .name = "mem_bw_ctrl",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

/* define EMI bandwiwth limiter control table */
static struct emi_bwl_ctrl ctrl_tbl[NR_CON_SCE];

/* current concurrency scenario */
static int cur_con_sce = 0x0FFFFFFF;

/* define concurrency scenario strings */
static const char *con_sce_str[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) #con_sce,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};

/* define EMI bandwidth allocation tables */
static const unsigned int emi_arba_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arba,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};
static const unsigned int emi_arbb_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arbb,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};
static const unsigned int emi_arbc_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arbc,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};
static const unsigned int emi_arbd_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arbd,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};
static const unsigned int emi_arbe_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arbe,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};
static const unsigned int emi_arbf_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arbf,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};
static const unsigned int emi_arbg_val[] =
{
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe, arbf, arbg) arbg,
#include "mach/mt6573_con_sce.h"
#undef X_CON_SCE
};

int mtk_mem_bw_ctrl(int sce, int op)
{
    int i, highest;

    if (sce >= NR_CON_SCE) {
        return -1;
    }
    if (op != ENABLE_CON_SCE && op != DISABLE_CON_SCE) {
        return -1;
    }
    if (in_interrupt()) {
        return -1;
    }

    down(&emi_bwl_sem);

    if (op == ENABLE_CON_SCE) {
        ctrl_tbl[sce].ref_cnt++;
    } else if (op == DISABLE_CON_SCE) {
        if (ctrl_tbl[sce].ref_cnt != 0) {
            ctrl_tbl[sce].ref_cnt--;
        } 
    }

    /* find the scenario with the highest priority */
    highest = -1;
    for (i = 0; i < NR_CON_SCE; i++) {
        if (ctrl_tbl[i].ref_cnt != 0) {
            highest = i;
            break;
        }
    }
    if (highest == -1) {
        highest = CON_SCE_NORMAL;
    }

    /* set new EMI bandwidth limiter value */
    if (highest != cur_con_sce) {
        writel(emi_arba_val[highest], EMI_ARBA);
        writel(emi_arbb_val[highest], EMI_ARBB);
        writel(emi_arbc_val[highest], EMI_ARBC);
        writel(emi_arbd_val[highest], EMI_ARBD);
        writel(emi_arbe_val[highest], EMI_ARBE);
        writel(emi_arbf_val[highest], EMI_ARBF);
        mt65xx_reg_sync_writel(emi_arbg_val[highest], EMI_ARBG);
#if 0   /* EMI_ARBCT will be set once at init */
        mt65xx_reg_sync_writel(0x00070020, EMI_ARBCT);
#endif
        cur_con_sce = highest;
    }

    up(&emi_bwl_sem);

    return 0;
}

#if 0
EXPORT_SYMBOL(mtk_mem_bw_ctrl);
#endif

static ssize_t con_sce_show(struct device_driver *driver, char *buf)
{
    if (cur_con_sce >= NR_CON_SCE) {
        sprintf(buf, "none\n");
    } else {
        sprintf(buf, "%s\n", con_sce_str[cur_con_sce]);
    }

    return strlen(buf);
}

static ssize_t con_sce_store(struct device_driver *driver, const char *buf, size_t count)
{
    int i;

    for (i = 0; i < NR_CON_SCE; i++) {
        if (!strncmp(buf, con_sce_str[i], strlen(con_sce_str[i]))) {
            if (!strncmp(buf + strlen(con_sce_str[i]) + 1, EN_CON_SCE_STR, strlen(EN_CON_SCE_STR))) {
                mtk_mem_bw_ctrl(i, ENABLE_CON_SCE);
                printk("concurrency scenario %s ON\n", con_sce_str[i]);
                break;
            } else if (!strncmp(buf + strlen(con_sce_str[i]) + 1, DIS_CON_SCE_STR, strlen(DIS_CON_SCE_STR))) {
                mtk_mem_bw_ctrl(i, DISABLE_CON_SCE);
                printk("concurrency scenario %s OFF\n", con_sce_str[i]);
                break;
            }
        }
    }

    return count;
}

DRIVER_ATTR(concurrency_scenario, 0644, con_sce_show, con_sce_store);

static int __init emi_bwl_mod_init(void)
{
    int ret;

    mt65xx_reg_sync_writel(0x00070020, EMI_ARBCT);

    ret = mtk_mem_bw_ctrl(CON_SCE_NORMAL, ENABLE_CON_SCE);
    if (ret) {
        printk("fail to set EMI bandwidth limiter\n");
    }

    ret = driver_register(&mem_bw_ctrl);
    if (ret) {
        printk("fail to register EMI_BW_LIMITER driver\n");
    }
    ret = driver_create_file(&mem_bw_ctrl, &driver_attr_concurrency_scenario);
    if (ret) {
        printk("fail to create EMI_BW_LIMITER sysfs file\n");
    }

    return 0;
}

static void __exit emi_bwl_mod_exit(void)
{
}

module_init(emi_bwl_mod_init);
module_exit(emi_bwl_mod_exit);

