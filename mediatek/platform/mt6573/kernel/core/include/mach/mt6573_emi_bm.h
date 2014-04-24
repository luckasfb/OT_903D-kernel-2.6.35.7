
#ifndef __MT6573_EMI_BM_H__
#define __MT6573_EMI_BW_H__

#ifndef EMI_DRCT
#define EMI_DRCT (EMI_BASE + 0x0078)
#endif

#ifndef EMI_ARBA
#define EMI_ARBA (EMI_BASE + 0x0100)
#endif

#ifndef EMI_ARBB
#define EMI_ARBB (EMI_BASE + 0x0108)
#endif

#ifndef EMI_ARBC
#define EMI_ARBC (EMI_BASE + 0x0110)
#endif

#ifndef EMI_ARBD
#define EMI_ARBD (EMI_BASE + 0x0118)
#endif

#ifndef EMI_ARBE
#define EMI_ARBE (EMI_BASE + 0x0120)
#endif

#ifndef EMI_ARBF
#define EMI_ARBF (EMI_BASE + 0x0128)
#endif

#ifndef EMI_ARBG
#define EMI_ARBG (EMI_BASE + 0x0130)
#endif

#define EMI_BMEN (EMI_BASE + 0x0400)
#define EMI_BCNT (EMI_BASE + 0x0408)
#define EMI_TACT (EMI_BASE + 0x0410)
#define EMI_TSCT (EMI_BASE + 0x0418)
#define EMI_WACT (EMI_BASE + 0x0420)
#define EMI_WSCT (EMI_BASE + 0x0428)
#define EMI_BACT (EMI_BASE + 0x0430)
#define EMI_BSCT (EMI_BASE + 0x0438)
#define EMI_MSEL (EMI_BASE + 0x0440)
#define EMI_TSCT2 (EMI_BASE + 0x0448)
#define EMI_TSCT3 (EMI_BASE + 0x0450)
#define EMI_WSCT2 (EMI_BASE + 0x0458)  
#define EMI_WSCT3 (EMI_BASE + 0x0460)  
#define EMI_MSEL2 (EMI_BASE + 0x0468)
#define EMI_MSEL3 (EMI_BASE + 0x0470)
#define EMI_MSEL4 (EMI_BASE + 0x0478)
#define EMI_MSEL5 (EMI_BASE + 0x0480)
#define EMI_MSEL6 (EMI_BASE + 0x0488)
#define EMI_MSEL7 (EMI_BASE + 0x0490)
#define EMI_MSEL8 (EMI_BASE + 0x0498)
#define EMI_MSEL9 (EMI_BASE + 0x04A0)
#define EMI_MSEL10 (EMI_BASE + 0x04A8)
#define EMI_BMID0 (EMI_BASE + 0x04B0)
#define EMI_BMID1 (EMI_BASE + 0x04B8)
#define EMI_BMID2 (EMI_BASE + 0x04C0)
#define EMI_BMID3 (EMI_BASE + 0x04C8)
#define EMI_BMID4 (EMI_BASE + 0x04D0)
#define EMI_BMID5 (EMI_BASE + 0x04D8)
#define EMI_TTYPE1 (EMI_BASE + 0x0500)
#define EMI_TTYPE2 (EMI_BASE + 0x0508)
#define EMI_TTYPE3 (EMI_BASE + 0x0510)
#define EMI_TTYPE4 (EMI_BASE + 0x0518)
#define EMI_TTYPE5 (EMI_BASE + 0x0520)
#define EMI_TTYPE6 (EMI_BASE + 0x0528)
#define EMI_TTYPE7 (EMI_BASE + 0x0530)
#define EMI_TTYPE8 (EMI_BASE + 0x0538)
#define EMI_TTYPE9 (EMI_BASE + 0x0540)
#define EMI_TTYPE10 (EMI_BASE + 0x0548)
#define EMI_TTYPE11 (EMI_BASE + 0x0550)
#define EMI_TTYPE12 (EMI_BASE + 0x0558)
#define EMI_TTYPE13 (EMI_BASE + 0x0560)
#define EMI_TTYPE14 (EMI_BASE + 0x0568)
#define EMI_TTYPE15 (EMI_BASE + 0x0570)
#define EMI_TTYPE16 (EMI_BASE + 0x0578)
#define EMI_TTYPE17 (EMI_BASE + 0x0580)
#define EMI_TTYPE18 (EMI_BASE + 0x0588)
#define EMI_TTYPE19 (EMI_BASE + 0x0590)
#define EMI_TTYPE20 (EMI_BASE + 0x0598)    
#define EMI_TTYPE21 (EMI_BASE + 0x05A0)               

#define BM_COUNTER_MAX 21

#define BM_MASTER_MULTIMEDIA1             0x001   
#define BM_MASTER_MULTIMEDIA2              0x002
#define BM_MASTER_AP_MCU                 0x004
#define BM_MASTER_AUDIO_APDMA_DEBUG        0x008
#define BM_MASTER_MD_DSP                 0x010
#define BM_MASTER_MD_MCU                0x020
#define BM_MASTER_2G_3G_MDDMA           0x040
#define BM_MASTER_DUMMY_READ            0x080

#define BUS_MON_EN                        0x00000001
#define BUS_MON_PAUSE                    0x00000002
#define BC_OVERRUN                        0x00000100

#define BM_SEL_ALL                      0x0000FE00
#define BM_SEL_ROW_HIT                    0x00008000
#define BM_SEL_ROW_START                0x00004000
#define BM_SEL_ROW_CONFLICT                0x00002000
#define BM_SEL_INTER_BANK                0x00001000
#define BM_SEL_EMI_CLOCK                0x00000800
#define BM_SEL_SLICE_ALL                0x00000400
#define BM_SEL_SLICE                    0x00000200

#define BM_SEL_WORD_ALL                    0x0
#define BM_SEL_WORD                        0x0
#define BM_SEL_BUSY_ALL                    0x0
#define BM_SEL_BUSY                        0x0
#define BM_SEL_BUSCYC                    0x0
#define BM_SEL_TRANS_ALL                0x0
#define BM_SEL_TRANS                    0x0


#define BM_ERR_WRONG_REQ                    -1
#define BM_ERR_OVERRUN                        -2

typedef enum
{
    BM_BOTH_READ_WRITE,
    BM_READ_ONLY,
    BM_WRITE_ONLY
}BM_RW_Type;

enum 
{
        BM_TRANS_TYPE_1BEAT = 0x0,
        BM_TRANS_TYPE_2BEAT,                        
        BM_TRANS_TYPE_3BEAT,
        BM_TRANS_TYPE_4BEAT,
        BM_TRANS_TYPE_5BEAT,                    
        BM_TRANS_TYPE_6BEAT,                        
        BM_TRANS_TYPE_7BEAT,
        BM_TRANS_TYPE_8BEAT,
        BM_TRANS_TYPE_9BEAT,                        
        BM_TRANS_TYPE_10BEAT,                    
        BM_TRANS_TYPE_11BEAT,
        BM_TRANS_TYPE_12BEAT,
        BM_TRANS_TYPE_13BEAT,                    
        BM_TRANS_TYPE_14BEAT,                    
        BM_TRANS_TYPE_15BEAT,
        BM_TRANS_TYPE_16BEAT
};

#define    BM_TRANS_TYPE_1Byte            0x00
#define    BM_TRANS_TYPE_2Byte            0x10
#define    BM_TRANS_TYPE_4Byte            0x20
#define    BM_TRANS_TYPE_8Byte            0x30
//#define    BM_TRANS_TYPE_16Byte        0x40

#define BM_TRANS_TYPE_BURST_WRAP    0x00
#define BM_TRANS_TYPE_BURST_INCR    0x40
                        
extern void BM_Init(void);
extern void BM_DeInit(void);
extern void BM_Enable(void);
extern void BM_Disable(void);
extern void BM_Pause(void);
extern void BM_Continue(void);
extern bool BM_IsOverrun(void);
extern void BM_SetMaster(int counter_num, int master);
extern void BM_SetMonitorCounter(int counter_num, int master, int trans_type);
extern void BM_SetReadWriteType(int ReadWriteType);
extern void BM_SetMonitorType(int MonitorType);
extern int BM_GetBusCycCount(void);
extern int BM_GetSliceCount(void);
extern int BM_GetTransAllCount(void);
extern int BM_GetSliceAllCount(void);
extern int BM_GetTransCount(int counter_num);
extern int BM_GetEmiClockCount(void);
extern int BM_GetWordAllCount(void);
extern int BM_GetRowHitCount(void);
extern int BM_GetWordCount(int counter_num);
extern int BM_GetRowStartCount(void);
extern int BM_GetBusyAllCount(void);
extern int BM_GetRowConflictCount(void);
extern int BM_GetBusyCount(void);
extern int BM_GetInterBankCount(void);
extern int BM_GetTransTypeCount(int counter_num);
extern void BM_SetIDSelect(int counter_num, int id, int enable);

#endif  /* !__MT6573_EMI_BM_H__ */

