

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "mach/mt6573_pll.h"
#include "mach/mt6573_gpt.h"
#include "mach/mt6573_typedefs.h"
#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"



/* Debug message event */
#define DBG_DVFS_NONE            0x00000000    
#define DBG_DVFS_ALL            0xFFFFFFFF    
//#define DBG_DVFS_MASK       (0x00000000)
#define DBG_DVFS_MASK       (0xFFFFFFFF)
#define DVFS_DELAY          5000

#if 1
#define MSG(evt, fmt, args...) \
do {    \
    if ((DBG_DVFS_##evt) & DBG_DVFS_MASK) { \
        printk(fmt, ##args); \
    } \
} while(0)

#define MSG_FUNC_ENTRY(f)    MSG(ENTER, "<DVFS FUNC>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...) do{}while(0)
#define MSG_FUNC_ENTRY(f)       do{}while(0)
#endif


#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)

#define DVFS_F1_TM  806
#define DVFS_F1     676
#define DVFS_F2     572
#define DVFS_F3     520
#define DVFS_F4     416
#define DVFS_F5     351
#define DVFS_F6     286
#define DVFS_F7     208
#define DVFS_F8     169

unsigned int g_cur_freq = DVFS_F1*1000; /*Khz*/
unsigned int g_cur_vol = UPMU_VOLT_1_3_5_0_V; /*mV*/

static int turbo_mode = 0;

struct mt6573_cpu_freq_info {
    unsigned int cpufreq_mhz;
    int	vcc_core;   /* in mV */
};

#define OP(cpufreq, vcore)  \
{                           \
    .cpufreq_mhz = cpufreq, \
    .vcc_core = vcore,      \
}

static struct mt6573_cpu_freq_info mt6573_freqs[] = {
    OP(DVFS_F8, 1000), 
    OP(DVFS_F7, 1050), 
    OP(DVFS_F6, 1100), 
    OP(DVFS_F5, 1150), 
    OP(DVFS_F4, 1200), 
    OP(DVFS_F3, 1250), 
    OP(DVFS_F2, 1300), 
    OP(DVFS_F1, 1350), 
};

static struct mt6573_cpu_freq_info mt6573_freqs_tm[] = {
    OP(DVFS_F8,     1000), 
    OP(DVFS_F7,     1050), 
    OP(DVFS_F6,     1100), 
    OP(DVFS_F5,     1150), 
    OP(DVFS_F4,     1200), 
    OP(DVFS_F3,     1250), 
    OP(DVFS_F2,     1300), 
    OP(DVFS_F1_TM,  1350), 
};

static unsigned int mt6573_cpu_freqs_num;
static struct mt6573_cpu_freq_info *mt6573_cpu_freqs;
static struct cpufreq_frequency_table *mt6573_cpu_freqs_table;

void DVFS_XGPTConfig(UINT32 us)
{    
    XGPT_CONFIG config;
    XGPT_NUM  xgpt_num = XGPT2;    
    XGPT_CLK_DIV clkDiv = XGPT_CLK_DIV_1;

    XGPT_Init (xgpt_num, NULL);
    config.num = xgpt_num;
    config.mode = XGPT_ONE_SHOT;
    config.clkDiv = clkDiv;
    config.bIrqEnable = TRUE;
    config.u4Compare = us*26; /* 1us count 26 times*/
    
    if (XGPT_Config(config) == FALSE )
        return;                       
        
    XGPT_Start(xgpt_num);  

    return;
}

static int setup_freqs_table(struct cpufreq_policy *policy,
            struct mt6573_cpu_freq_info *freqs, int num)
{
    struct cpufreq_frequency_table *table;
    int i, ret;
    
    table = kzalloc((num + 1) * sizeof(*table), GFP_KERNEL);
    if (table == NULL)
        return -ENOMEM;
    
    for (i = 0; i < num; i++) {
        table[i].index = i;
        table[i].frequency = freqs[i].cpufreq_mhz * 1000;
    }
    table[num].frequency = i;
    table[num].frequency = CPUFREQ_TABLE_END;
    
    mt6573_cpu_freqs = freqs;
    mt6573_cpu_freqs_num = num;
    mt6573_cpu_freqs_table = table;
    
    ret = cpufreq_frequency_table_cpuinfo(policy, table);
    if (!ret)
        cpufreq_frequency_table_get_attr(mt6573_cpu_freqs_table, policy->cpu);

    return ret;
}

static int mt6573_cpufreq_verify(struct cpufreq_policy *policy)
{
    MSG(ALL,"Call mt6573_cpufreq_verify !\n");	
    return cpufreq_frequency_table_verify(policy, mt6573_cpu_freqs_table);
}

static unsigned int mt6573_cpufreq_get(unsigned int cpu)
{
    MSG(ALL,"Call mt6573_cpufreq_get : %d !\n", g_cur_freq);
    return g_cur_freq;
}

static void mt6573_cpufreq_set(unsigned int freq)
{
    switch(freq)
    {
        case DVFS_F1_TM:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F1_TM_MHZ);
            break;
        case DVFS_F1:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F1_MHZ);
            break;
        case DVFS_F2:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F2_MHZ);
            break;
        case DVFS_F3:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F3_MHZ);
            break;
        case DVFS_F4:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F4_MHZ);
            break;
        case DVFS_F5:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F5_MHZ);
            break;
        case DVFS_F6:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F6_MHZ);
            break;
        case DVFS_F7:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F7_MHZ);
            break;
        case DVFS_F8:
            PLL_Fsel(MT65XX_APMCU_PLL, APMCU_F8_MHZ);
            break;
        default:
            MSG(ALL,"mt6573_cpufreq_set: specified frequency (%d Mhz) not support!\n", freq);
            break;
    }
    return;
}

static int mt6573_cpufreq_target(struct cpufreq_policy *policy,
            unsigned int target_freq,
            unsigned int relation)
{
    struct mt6573_cpu_freq_info *next;
    struct cpufreq_freqs freqs;
    int idx;
    
    if (policy->cpu != 0)
        return -EINVAL;
    
    /* Lookup the next frequency */
    if (cpufreq_frequency_table_target(policy, mt6573_cpu_freqs_table, target_freq, relation, &idx))
        return -EINVAL;
    
    if (turbo_mode)
    {
        next = &mt6573_freqs_tm[idx];
    }
    else
    {
        next = &mt6573_freqs[idx];
    }
    
    freqs.old = policy->cur;
    freqs.new = next->cpufreq_mhz * 1000;
    freqs.cpu = policy->cpu;
    
    if(bEnDVFSLog) 
    {
        printk("CPU frequency from %d MHz to %d MHz%s\n",
            freqs.old / 1000, freqs.new / 1000,
            (freqs.old == freqs.new) ? " (skipped)" : "");
    }
    
    if (freqs.old == target_freq)
        return 0;
    
    /* Update cpu freq information */
    g_cur_freq = next->cpufreq_mhz;
    
    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
    
    /* Each BUCK voltage step is 25mV, should delay 360us*/
    if (g_cur_freq == DVFS_F1_TM) 
    {       
        if(bBUCK_ADJUST) 
        {
            if(bEnDVFSLog) printk(".");
            upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_5_0_V);
            udelay(((UPMU_VOLT_1_3_5_0_V-g_cur_vol)/25)*360);
            //DVFS_XGPTConfig(DVFS_DELAY);
            //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
        }
        mt6573_cpufreq_set(DVFS_F1_TM);
        g_cur_vol = UPMU_VOLT_1_3_5_0_V;
    }
    else if (g_cur_freq == DVFS_F1) 
    {       
        if(bBUCK_ADJUST) 
        {
            if(bEnDVFSLog) printk(".");
            upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_5_0_V);
            udelay(((UPMU_VOLT_1_3_5_0_V-g_cur_vol)/25)*360);
            //DVFS_XGPTConfig(DVFS_DELAY);
            //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
        }
        mt6573_cpufreq_set(DVFS_F1);
        g_cur_vol = UPMU_VOLT_1_3_5_0_V;
    }
    else if (g_cur_freq == DVFS_F2) 
    {
        if (freqs.new > freqs.old) 
        {
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_0_0_V);
                udelay(((UPMU_VOLT_1_3_0_0_V-g_cur_vol)/25)*360);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
            }
            mt6573_cpufreq_set(DVFS_F2);
        }
        else 
        {
            mt6573_cpufreq_set(DVFS_F2);
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_0_0_V);            
                udelay(((g_cur_vol-UPMU_VOLT_1_3_0_0_V)/25)*360);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
            }
        }        
        g_cur_vol = UPMU_VOLT_1_3_0_0_V;
    }
    else if (g_cur_freq == DVFS_F3) 
    {
        if (freqs.new > freqs.old) 
        {
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_2_5_0_V);
                udelay(((UPMU_VOLT_1_2_5_0_V-g_cur_vol)/25)*360);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
            }    
            mt6573_cpufreq_set(DVFS_F3);
        }
        else 
        {
            mt6573_cpufreq_set(DVFS_F3);
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_2_5_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((g_cur_vol-UPMU_VOLT_1_2_5_0_V)/25)*360);
            }
        }
        g_cur_vol = UPMU_VOLT_1_2_5_0_V;
    }
    else if (g_cur_freq == DVFS_F4) 
    {
        if (freqs.new > freqs.old) 
        {
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_2_0_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((UPMU_VOLT_1_2_0_0_V-g_cur_vol)/25)*360);
            }
            mt6573_cpufreq_set(DVFS_F4);
        }
        else 
        {
            mt6573_cpufreq_set(DVFS_F4);
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_2_0_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((g_cur_vol-UPMU_VOLT_1_2_0_0_V)/25)*360);
            }
        }
        g_cur_vol = UPMU_VOLT_1_2_0_0_V;
    }
    else if (g_cur_freq == DVFS_F5) 
    {
        if (freqs.new > freqs.old) 
        {
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_1_5_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((UPMU_VOLT_1_1_5_0_V-g_cur_vol)/25)*360);
            }
            mt6573_cpufreq_set(DVFS_F5);
        }
        else
        {
            mt6573_cpufreq_set(DVFS_F5);
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_1_5_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((g_cur_vol-UPMU_VOLT_1_1_5_0_V)/25)*360);
            }
        }
        g_cur_vol = UPMU_VOLT_1_1_5_0_V;
    }
    else if (g_cur_freq == DVFS_F6) 
    {
        if (freqs.new > freqs.old) 
        {
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_1_0_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((UPMU_VOLT_1_1_0_0_V-g_cur_vol)/25)*360);
            }    
            mt6573_cpufreq_set(DVFS_F6);
        }
        else
        {
            mt6573_cpufreq_set(DVFS_F6);
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_1_0_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((g_cur_vol-UPMU_VOLT_1_1_0_0_V)/25)*360);
            }
        }
        g_cur_vol = UPMU_VOLT_1_1_0_0_V;
    }
    else if (g_cur_freq == DVFS_F7) 
    {
        if (freqs.new > freqs.old) 
        {
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_0_5_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((UPMU_VOLT_1_0_5_0_V-g_cur_vol)/25)*360);
            }
            mt6573_cpufreq_set(DVFS_F7);
        }
        else
        {
            mt6573_cpufreq_set(DVFS_F7);
            if(bBUCK_ADJUST) 
            {
                if(bEnDVFSLog) printk(".");
                upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_0_5_0_V);
                //DVFS_XGPTConfig(DVFS_DELAY);
                //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
                udelay(((g_cur_vol-UPMU_VOLT_1_0_5_0_V)/25)*360);
            }
        }
        g_cur_vol = UPMU_VOLT_1_0_5_0_V;
    }
    else if (g_cur_freq == DVFS_F8) 
    {
        mt6573_cpufreq_set(DVFS_F8);
        if(bBUCK_ADJUST) 
        {
            if(bEnDVFSLog) printk(".");
            upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_0_0_0_V);
            //DVFS_XGPTConfig(DVFS_DELAY);
            //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
            udelay(((g_cur_vol-UPMU_VOLT_1_0_0_0_V)/25)*360);
        }
        g_cur_vol = UPMU_VOLT_1_0_0_0_V;
    }
    else 
    {
        if(bBUCK_ADJUST) 
        {
            if(bEnDVFSLog) printk(".");
            upmu_buck_normal_voltage_adjust(BUCK_VAPROC, UPMU_VOLT_1_3_5_0_V);
            //DVFS_XGPTConfig(DVFS_DELAY);
            //__asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (temp));
            udelay(((UPMU_VOLT_1_3_5_0_V-g_cur_vol)/25)*360);
        }
        
        if (turbo_mode)
        {
            mt6573_cpufreq_set(DVFS_F1_TM);
        }
        else
        {
            mt6573_cpufreq_set(DVFS_F1);
        }
        g_cur_vol = UPMU_VOLT_1_3_5_0_V;
    }	
    cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
    return 0;
}

static __init int mt6573_cpufreq_init(struct cpufreq_policy *policy)
{
    int ret = -EINVAL;
    
    turbo_mode = (DRV_Reg32(0xF7024104) & 0x10000000) >> 28;
    
    if (turbo_mode)
    {
        /* set default policy and cpuinfo, unit : Khz */
        policy->cpuinfo.min_freq = DVFS_F8 * 1000;
        policy->cpuinfo.max_freq = DVFS_F1_TM * 1000;
        policy->cpuinfo.transition_latency = 1000; /* FIXME: 1 ms, assumed */
        policy->cur = policy->min = policy->max = DVFS_F1_TM * 1000;
        
        g_cur_freq = DVFS_F1_TM * 1000; /*Khz*/
        
        printk("mt6573_cpufreq_init: cpu is running at turbo mode\n");
        ret = setup_freqs_table(policy, ARRAY_AND_SIZE(mt6573_freqs_tm));
    }
    else
    {
        /* set default policy and cpuinfo, unit : Khz */
        policy->cpuinfo.min_freq = DVFS_F8 * 1000;
        policy->cpuinfo.max_freq = DVFS_F1 * 1000;
        policy->cpuinfo.transition_latency = 1000; /* FIXME: 1 ms, assumed */
        policy->cur = policy->min = policy->max = DVFS_F1 * 1000;
        
        g_cur_freq = DVFS_F1 * 1000; /*Khz*/
        
        printk("mt6573_cpufreq_init: cpu is running at normal mode\n");
        ret = setup_freqs_table(policy, ARRAY_AND_SIZE(mt6573_freqs));
    }
    
    if (ret) {
        pr_err("failed to setup frequency table\n");
        return ret;
    }
    
    return 0;
}

static struct cpufreq_driver mt6573_cpufreq_driver = {
    .verify		= mt6573_cpufreq_verify,
    .target		= mt6573_cpufreq_target,
    .init		= mt6573_cpufreq_init,
    .get		= mt6573_cpufreq_get,
    .name		= "mt6573-cpufreq",
};

static int __init cpufreq_init(void)
{
    if(bCanEnDVFS)
        return cpufreq_register_driver(&mt6573_cpufreq_driver);
    else
        return 0;
}
module_init(cpufreq_init);

static void __exit cpufreq_exit(void)
{
    cpufreq_unregister_driver(&mt6573_cpufreq_driver);
}
module_exit(cpufreq_exit);

MODULE_DESCRIPTION("CPU frequency scaling driver for mt6573");
MODULE_LICENSE("GPL");