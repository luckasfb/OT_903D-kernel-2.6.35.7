
#ifndef __MT65XX_MON_H__
#define __MT65XX_MON_H__

struct mt65xx_mon_log
{
    __u32 cpu_cyc;
    __u32 cpu_cnt0;
    __u32 cpu_cnt1;
    __u32 l2c_cnt0;
    __u32 l2c_cnt1;
    __u32 BM_BCNT;
    __u32 BM_TACT;
    __u32 BM_TSCT;
    __u32 BM_WACT;
    __u32 BM_WSCT;
    __u32 BM_BACT;
    __u32 BM_BSCT;
    __u32 BM_TSCT2;
    __u32 BM_WSCT2;
    __u32 BM_TSCT3;
    __u32 BM_WSCT3;
    __u32 BM_SCNT;
    __u32 BM_SACT;
    __u32 BM_ECCT;
    __u32 BM_RHCT;
    __u32 BM_RSCT;
    __u32 BM_RCCT;
    __u32 BM_IBCT;          
    __u32 BM_TPCT1;
};

extern int mt65xx_mon_init(void);
extern int mt65xx_mon_deinit(void);
extern int mt65xx_mon_enable(void);
extern int mt65xx_mon_disable(void);
extern unsigned int mt65xx_mon_log(void);
extern int mt65xx_mon_print_log(unsigned int log, struct trace_iterator *iter);

#endif  /* !__MT65XX_MON_H__ */

