
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/trace_seq.h>
#include <linux/ftrace_event.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>

#include "mach/mt65xx_mon.h"
#include "asm/hardware/cache-l2x0.h"
#include "mach/mt6573_reg_base.h"
#include "mach/sync_write.h"
#include "mach/mt6573_emi_bm.h"
#include "mach/mt6573_pll.h"

#define MON_LOG_BUFF_LEN (64*1024)
#define DEF_CPU_CNT0_EVT (0x7)
#define DEF_CPU_CNT1_EVT (0xA)
#define DEF_L2C_CNT0_EVT (4)
#define DEF_L2C_CNT1_EVT (5)
#define DEF_BM_RW_TYPE  (BM_BOTH_READ_WRITE)

struct mt65xx_mon_log *mt6573_mon_log_buff;
unsigned int mt6573_mon_log_buff_index;
static unsigned int cpu_cnt0_evt = DEF_CPU_CNT0_EVT;
static unsigned int cpu_cnt1_evt = DEF_CPU_CNT1_EVT;
static unsigned int l2c_cnt0_evt = DEF_L2C_CNT0_EVT;
static unsigned int l2c_cnt1_evt = DEF_L2C_CNT1_EVT;
enum { L2C_EVT_CO = 1, L2C_EVT_DRHIT = 2, L2C_EVT_DRREQ = 3, L2C_EVT_DWHIT = 4,
       L2C_EVT_DWREQ = 5, L2C_EVT_DWTREQ = 6, L2C_EVT_IRHIT = 7, L2C_EVT_IRREQ = 8,
       L2C_EVT_WA = 9, L2C_EVT_IPFALLOC = 10, L2C_EVT_EPFHIT = 11, 
       L2C_EVT_EPFALLOC = 12, L2C_EVT_SRRCVD = 13, L2C_EVT_SRCONF = 14,
       L2C_EVT_EPFRCVD = 15 };
static unsigned int bm_master_evt = BM_MASTER_AP_MCU;
static unsigned int bm_rw_type_evt = DEF_BM_RW_TYPE;

extern void enable_arm11_perf_mon(void);
extern void disable_arm11_perf_mon(void);
extern void reset_arm11_perf_mon_cnt(void);
extern unsigned int get_arm11_perf_mon_cyc_cnt(void);
extern void set_arm11_perf_mon_cyc_cnt(unsigned int cnt);
extern unsigned int get_arm11_perf_mon_cnt0(void);
extern void set_arm11_perf_mon_cnt0(unsigned int cnt);
extern unsigned int get_arm11_perf_mon_cnt1(void);
extern void set_arm11_perf_mon_cnt1(unsigned int cnt);
extern unsigned int set_arm11_perf_mon_cnt0_evt(unsigned int evt);
extern unsigned int set_arm11_perf_mon_cnt1_evt(unsigned int evt);

int mt65xx_mon_init(void)
{
    if (!mt6573_mon_log_buff) {
        mt6573_mon_log_buff = vmalloc(sizeof(struct mt65xx_mon_log) * MON_LOG_BUFF_LEN);
    }
    if (!mt6573_mon_log_buff) {
        printk(KERN_WARNING "fail to allocate the buffer for the monitor log\n");
    }

    BM_Init();

    /* DCM and CPU frequency division may be OFF to force fixed CPU clock rate */
    //force_off_dcm();
    dcm_disable_state(ALL_STATE);

    return 0;
}

int mt65xx_mon_deinit(void)
{
    BM_DeInit();
    //cancel_force_off_dcm();
    dcm_enable_state(ALL_STATE);

    return 0;
}

int mt65xx_mon_enable(void)
{
    set_arm11_perf_mon_cyc_cnt(0);
    set_arm11_perf_mon_cnt0(0);
    set_arm11_perf_mon_cnt1(0);
    reset_arm11_perf_mon_cnt();
    enable_arm11_perf_mon();

    mt65xx_reg_sync_writel(7, PL310_BASE + L2X0_EVENT_CNT_CTRL);

    BM_Disable();
    BM_Enable();
    
    return 0;
}

int mt65xx_mon_disable(void)
{
    disable_arm11_perf_mon();

    mt65xx_reg_sync_writel(0, PL310_BASE + L2X0_EVENT_CNT_CTRL);

    BM_Pause();
    
    return 0;
}

unsigned int mt65xx_mon_log(void)
{
    unsigned int cur;

    cur = mt6573_mon_log_buff_index;
    mt6573_mon_log_buff_index++;
    mt6573_mon_log_buff_index %= MON_LOG_BUFF_LEN;

    if (mt6573_mon_log_buff) {
        mt6573_mon_log_buff[cur].cpu_cyc = get_arm11_perf_mon_cyc_cnt();
        mt6573_mon_log_buff[cur].cpu_cnt0 = get_arm11_perf_mon_cnt0();
        mt6573_mon_log_buff[cur].cpu_cnt1 = get_arm11_perf_mon_cnt1();
        mt6573_mon_log_buff[cur].l2c_cnt0 = readl(PL310_BASE + L2X0_EVENT_CNT0_VAL);
        mt6573_mon_log_buff[cur].l2c_cnt1 = readl(PL310_BASE + L2X0_EVENT_CNT1_VAL);

        BM_SetMonitorType(0);
        mt6573_mon_log_buff[cur].BM_BCNT = BM_GetBusCycCount();
        mt6573_mon_log_buff[cur].BM_TACT = BM_GetTransAllCount();
        mt6573_mon_log_buff[cur].BM_TSCT = BM_GetTransCount(1);
        mt6573_mon_log_buff[cur].BM_WACT = BM_GetWordAllCount();
        mt6573_mon_log_buff[cur].BM_WSCT = BM_GetWordCount(1);
        mt6573_mon_log_buff[cur].BM_BACT = BM_GetBusyAllCount();
        mt6573_mon_log_buff[cur].BM_BSCT = BM_GetBusyCount();
        mt6573_mon_log_buff[cur].BM_TSCT2 = BM_GetTransCount(2);
        mt6573_mon_log_buff[cur].BM_WSCT2 = BM_GetWordCount(2);
        mt6573_mon_log_buff[cur].BM_TSCT3 = BM_GetTransCount(3);
        mt6573_mon_log_buff[cur].BM_WSCT3 = BM_GetWordCount(3);

        BM_SetMonitorType(BM_SEL_ALL);
        mt6573_mon_log_buff[cur].BM_SCNT = BM_GetSliceCount();
        mt6573_mon_log_buff[cur].BM_SACT = BM_GetSliceAllCount();
        mt6573_mon_log_buff[cur].BM_ECCT = BM_GetEmiClockCount();
        mt6573_mon_log_buff[cur].BM_RHCT = BM_GetRowHitCount();
        mt6573_mon_log_buff[cur].BM_RSCT = BM_GetRowStartCount();
        mt6573_mon_log_buff[cur].BM_RCCT = BM_GetRowConflictCount();
        mt6573_mon_log_buff[cur].BM_IBCT = BM_GetInterBankCount();    
    
        mt6573_mon_log_buff[cur].BM_TPCT1 = BM_GetTransTypeCount(1); 
    }
    
    return cur;
}

int mt65xx_mon_print_log(unsigned int log, struct trace_iterator *iter)
{
    struct trace_seq *s = &iter->seq;

    if (log >= MON_LOG_BUFF_LEN) {
        return -EINVAL;
    }

    if (!iter) {
        return -EINVAL;
    }

    if (mt6573_mon_log_buff && (log == 0)) {
        trace_seq_printf(s, "MON_LOG_BUFF_LEN = %d, ", MON_LOG_BUFF_LEN);
    }
    if (mt6573_mon_log_buff) {
        trace_seq_printf(s, "cpu_cyc = %d, cpu_cnt0 = %d, cpu_cnt1 = %d, ", mt6573_mon_log_buff[log].cpu_cyc, mt6573_mon_log_buff[log].cpu_cnt0, mt6573_mon_log_buff[log].cpu_cnt1);
        trace_seq_printf(s, "l2c_cnt0 = %d, l2c_cnt1 = %d, ", mt6573_mon_log_buff[log].l2c_cnt0, mt6573_mon_log_buff[log].l2c_cnt1);
        trace_seq_printf(s, "BM_BCNT = %d, BM_TACT = %d, BM_TSCT = %d, ", mt6573_mon_log_buff[log].BM_BCNT, mt6573_mon_log_buff[log].BM_TACT, mt6573_mon_log_buff[log].BM_TSCT);
        trace_seq_printf(s, "BM_WACT = %d, BM_WSCT = %d, BM_BACT = %d, ", mt6573_mon_log_buff[log].BM_WACT, mt6573_mon_log_buff[log].BM_WSCT, mt6573_mon_log_buff[log].BM_BACT);
        trace_seq_printf(s, "BM_BSCT = %d, ", mt6573_mon_log_buff[log].BM_BSCT);
        trace_seq_printf(s, "BM_TSCT2 = %d, BM_WSCT2 = %d, ", mt6573_mon_log_buff[log].BM_TSCT2, mt6573_mon_log_buff[log].BM_WSCT2);
        trace_seq_printf(s, "BM_TSCT3 = %d, BM_WSCT3 = %d, ", mt6573_mon_log_buff[log].BM_TSCT3, mt6573_mon_log_buff[log].BM_WSCT3);
        trace_seq_printf(s, "BM_SCNT = %d, BM_SACT = %d, BM_ECCT = %d, ", mt6573_mon_log_buff[log].BM_SCNT, mt6573_mon_log_buff[log].BM_SACT, mt6573_mon_log_buff[log].BM_ECCT);
        trace_seq_printf(s, "BM_RHCT = %d, BM_RSCT = %d, BM_RCCT = %d, ", mt6573_mon_log_buff[log].BM_RHCT, mt6573_mon_log_buff[log].BM_RSCT, mt6573_mon_log_buff[log].BM_RCCT);
        trace_seq_printf(s, "BM_IBCT = %d, BM_TPCT1 = %d\n", mt6573_mon_log_buff[log].BM_IBCT, mt6573_mon_log_buff[log].BM_TPCT1);        
    }

    return 0;
}

#define DEFINE_CPU_CNT_EVT_SHOW(_N)    \
static ssize_t cpu_cnt##_N##_evt_show(struct device_driver *driver, char *buf)   \
{   \
    return snprintf(buf, PAGE_SIZE, "counter event = %d\n", cpu_cnt##_N##_evt); \
}

#define DEFINE_CPU_CNT_EVT_STORE(_N)   \
ssize_t cpu_cnt##_N##_evt_store(struct device_driver *driver, const char *buf, size_t count) \
{   \
    char *p = (char *)buf;  \
    cpu_cnt##_N##_evt = simple_strtoul(p, &p, 16);   \
    set_arm11_perf_mon_cnt##_N##_evt(cpu_cnt##_N##_evt);  \
    return count;   \
}

DEFINE_CPU_CNT_EVT_SHOW(0)
DEFINE_CPU_CNT_EVT_SHOW(1)
DEFINE_CPU_CNT_EVT_STORE(0)
DEFINE_CPU_CNT_EVT_STORE(1)

#define DEFINE_L2C_CNT_EVT_SHOW(_N) \
static ssize_t l2c_cnt##_N##_evt_show(struct device_driver *driver, char *buf)   \
{   \
    return snprintf(buf, PAGE_SIZE, "counter event = %d\n", l2c_cnt##_N##_evt); \
}

#define DEFINE_L2C_CNT_EVT_STORE(_N)    \
ssize_t l2c_cnt##_N##_evt_store(struct device_driver *driver, const char *buf, size_t count) \
{   \
    if (!strncmp(buf, "CO", strlen("CO"))) {   \
        l2c_cnt##_N##_evt = L2C_EVT_CO; \
    } else if (!strncmp(buf, "DRHIT", strlen("DRHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DRHIT; \
    } else if (!strncmp(buf, "DRREQ", strlen("DRREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DRREQ; \
    } else if (!strncmp(buf, "DWHIT", strlen("DWHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DWHIT; \
    } else if (!strncmp(buf, "HWREQ", strlen("HWREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DWREQ; \
    } else if (!strncmp(buf, "HWTREQ", strlen("HWTREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DWTREQ; \
    } else if (!strncmp(buf, "IRHIT", strlen("IRHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_IRHIT; \
    } else if (!strncmp(buf, "IRREQ", strlen("IRREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_IRREQ; \
    } else if (!strncmp(buf, "WA", strlen("WA"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_WA; \
    } else if (!strncmp(buf, "IPFALLOC", strlen("IPFALLOC"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_IPFALLOC; \
    } else if (!strncmp(buf, "EPFHIT", strlen("EPFHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_EPFHIT; \
    } else if (!strncmp(buf, "EPFALLOC", strlen("EPFALLOC"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_EPFALLOC; \
    } else if (!strncmp(buf, "SRRCVD", strlen("SRRCVD"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_SRRCVD; \
    } else if (!strncmp(buf, "SRCONF", strlen("SRCONF"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_SRCONF; \
    } else if (!strncmp(buf, "EPFRCVD", strlen("EPFRCVD"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_EPFRCVD; \
    } else {    \
        printk("invalid event\n");  \
        return count;   \
    }   \
    mt65xx_reg_sync_writel(l2c_cnt##_N##_evt << 2, PL310_BASE + L2X0_EVENT_CNT##_N##_CFG);    \
    return count;   \
}

DEFINE_L2C_CNT_EVT_SHOW(0)
DEFINE_L2C_CNT_EVT_SHOW(1)
DEFINE_L2C_CNT_EVT_STORE(0)
DEFINE_L2C_CNT_EVT_STORE(1)

static ssize_t bm_master_evt_show(struct device_driver *driver, char *buf)   
{   
    return snprintf(buf, PAGE_SIZE, "EMI bus monitor master = %d\n", bm_master_evt); 
}

ssize_t bm_master_evt_store(struct device_driver *driver, const char *buf, size_t count) 
{   
    if (!strncmp(buf, "MM1", strlen("MM1"))) {   
        bm_master_evt = BM_MASTER_MULTIMEDIA1; 
    } else if (!strncmp(buf, "MM2", strlen("MM2"))) { 
        bm_master_evt = BM_MASTER_MULTIMEDIA2; 
    } else if (!strncmp(buf, "APMCU", strlen("APMCU"))) { 
        bm_master_evt = BM_MASTER_AP_MCU; 
    } else if (!strncmp(buf, "AUDIO_APDMA_DEBUG", strlen("AUDIO_APDMA_DEBUG"))) { 
        bm_master_evt = BM_MASTER_AUDIO_APDMA_DEBUG; 
    } else if (!strncmp(buf, "MDDSP", strlen("MDDSP"))) { 
        bm_master_evt = BM_MASTER_MD_DSP; 
    } else if (!strncmp(buf, "MDMCU", strlen("MDMCU"))) { 
        bm_master_evt = BM_MASTER_MD_MCU; 
    } else if (!strncmp(buf, "2G_3G_MDDMA", strlen("2G_3G_MDDMA"))) { 
        bm_master_evt = BM_MASTER_2G_3G_MDDMA; 
    } else if (!strncmp(buf, "DUMMY_READ", strlen("DUMMY_READ"))) { 
        bm_master_evt = BM_MASTER_DUMMY_READ; 
    } else {    
        printk("invalid event\n");  
        return count;   
    }
        
    BM_SetMaster(1, bm_master_evt);  
    return count;   
}

static ssize_t bm_rw_type_evt_show(struct device_driver *driver, char *buf)   
{   
    return snprintf(buf, PAGE_SIZE, "EMI bus read write type = %d\n", bm_rw_type_evt); 
}

ssize_t bm_rw_type_evt_store(struct device_driver *driver, const char *buf, size_t count) 
{   
    if (!strncmp(buf, "RW", strlen("RW"))) {   
        bm_rw_type_evt = BM_BOTH_READ_WRITE; 
    } else if (!strncmp(buf, "RO", strlen("RO"))) { 
        bm_rw_type_evt = BM_READ_ONLY; 
    } else if (!strncmp(buf, "WO", strlen("WO"))) { 
        bm_rw_type_evt = BM_WRITE_ONLY; 
    } else {    
        printk("invalid event\n");  
        return count;   
    }       
    
    BM_SetReadWriteType(bm_rw_type_evt);  
    return count;   
}

DRIVER_ATTR(cpu_cnt0_evt, 0644, cpu_cnt0_evt_show, cpu_cnt0_evt_store);
DRIVER_ATTR(cpu_cnt1_evt, 0644, cpu_cnt1_evt_show, cpu_cnt1_evt_store);
DRIVER_ATTR(l2c_cnt0_evt, 0644, l2c_cnt0_evt_show, l2c_cnt0_evt_store);
DRIVER_ATTR(l2c_cnt1_evt, 0644, l2c_cnt1_evt_show, l2c_cnt1_evt_store);

DRIVER_ATTR(bm_master_evt, 0644, bm_master_evt_show, bm_master_evt_store);
DRIVER_ATTR(bm_rw_type_evt, 0644, bm_rw_type_evt_show, bm_rw_type_evt_store);

static struct device_driver mt6573_mon_drv = 
{
    .name = "mt6573_monitor",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static int __init mt6573_mon_mod_init(void)
{
    int ret;

    /* register driver and create sysfs files */
    ret = driver_register(&mt6573_mon_drv);
    if (ret) {
        printk("fail to register mt6573_mon_drv\n");
        return ret;
    }
    ret = driver_create_file(&mt6573_mon_drv, &driver_attr_cpu_cnt0_evt);
    ret |= driver_create_file(&mt6573_mon_drv, &driver_attr_cpu_cnt1_evt);
    ret |= driver_create_file(&mt6573_mon_drv, &driver_attr_l2c_cnt0_evt);
    ret |= driver_create_file(&mt6573_mon_drv, &driver_attr_l2c_cnt1_evt);
    ret |= driver_create_file(&mt6573_mon_drv, &driver_attr_bm_master_evt);
    ret |= driver_create_file(&mt6573_mon_drv, &driver_attr_bm_rw_type_evt);
        
    if (ret) {
        printk("fail to create mt6573_mon sysfs files\n");
        return ret;
    }

    /* SPNIDEN[12] must be 1 for using ARM11 performance monitor unit */
    *(volatile unsigned int *)0xF702A000 |= 0x1000;

    /* set default ARM11 perf monitor events */
    set_arm11_perf_mon_cnt0_evt(DEF_CPU_CNT0_EVT);
    set_arm11_perf_mon_cnt1_evt(DEF_CPU_CNT1_EVT);

    /* set default L2C counter's events */
    mt65xx_reg_sync_writel(DEF_L2C_CNT0_EVT << 2, PL310_BASE + L2X0_EVENT_CNT0_CFG);
    mt65xx_reg_sync_writel(DEF_L2C_CNT1_EVT << 2, PL310_BASE + L2X0_EVENT_CNT1_CFG);

    /* init EMI bus monitor */
    BM_SetReadWriteType(DEF_BM_RW_TYPE);
    BM_SetMonitorCounter(1, BM_MASTER_AP_MCU, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);
    BM_SetMonitorCounter(2, BM_MASTER_MD_DSP | BM_MASTER_MD_MCU | BM_MASTER_2G_3G_MDDMA, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);
    BM_SetMonitorCounter(3, BM_MASTER_MULTIMEDIA1 | BM_MASTER_MULTIMEDIA2 | BM_MASTER_AUDIO_APDMA_DEBUG, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);

    return 0;
}

static void __exit mt6573_mon_mod_exit(void)
{
}

module_init(mt6573_mon_mod_init);
module_exit(mt6573_mon_mod_exit);

