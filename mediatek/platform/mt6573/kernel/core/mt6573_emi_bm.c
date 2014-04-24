

#include <linux/kernel.h>
#include <asm/io.h>

#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_emi_bm.h>
#include <mach/sync_write.h>
#include <mach/mt6573_typedefs.h>

static __u32 emi_dr, emi_gntcnt_a, emi_gntcnt_b, emi_gntcnt_b, emi_gntcnt_c, emi_gntcnt_d, emi_gntcnt_e, emi_gntcnt_f, emi_gntcnt_g;

void BM_Init(void)
{
    /* EMI dummy read should be disabled first */
    if (readl(EMI_DRCT) & 1) {
        emi_dr = 1;
        writel(readl(EMI_DRCT) & ~1, EMI_DRCT);
    }

    /* SM_GNTCNT_DIS in EMI_ARBx should be 0 */
    if (readl(EMI_ARBA) & 0x00008000) {
        emi_gntcnt_a = 1;
        writel(readl(EMI_ARBA) & ~0x00008000, EMI_ARBA);
    }
    if (readl(EMI_ARBB) & 0x00008000) {
        emi_gntcnt_b = 1;
        writel(readl(EMI_ARBB) & ~0x00008000, EMI_ARBB);
    }
    if (readl(EMI_ARBC) & 0x00008000) {
        emi_gntcnt_c = 1;
        writel(readl(EMI_ARBC) & ~0x00008000, EMI_ARBC);
    }
    if (readl(EMI_ARBD) & 0x00008000) {
        emi_gntcnt_d = 1;
        writel(readl(EMI_ARBD) & ~0x00008000, EMI_ARBD);
    }
    if (readl(EMI_ARBE) & 0x00008000) {
        emi_gntcnt_e = 1;
        writel(readl(EMI_ARBE) & ~0x00008000, EMI_ARBE);
    }
    if (readl(EMI_ARBF) & 0x00008000) {
        emi_gntcnt_f = 1;
        writel(readl(EMI_ARBF) & ~0x00008000, EMI_ARBF);
    }
    if (readl(EMI_ARBG) & 0x00008000) {
        emi_gntcnt_g = 1;
        mt65xx_reg_sync_writel(readl(EMI_ARBG) & ~0x00008000, EMI_ARBG);
    }
}

void BM_DeInit(void)
{
    /* restore SM_GNTCNT_DIS in EMI_ARBx */
    if (emi_gntcnt_a) {
        emi_gntcnt_a = 0;
        writel(readl(EMI_ARBA) | 0x00008000, EMI_ARBA);
    }
    if (emi_gntcnt_b) {
        emi_gntcnt_b = 0;
        writel(readl(EMI_ARBB) | 0x00008000, EMI_ARBB);
    }
    if (emi_gntcnt_c) {
        emi_gntcnt_c = 0;
        writel(readl(EMI_ARBC) | 0x00008000, EMI_ARBC);
    }
    if (emi_gntcnt_d) {
        emi_gntcnt_d = 0;
        writel(readl(EMI_ARBD) | 0x00008000, EMI_ARBD);
    }
    if (emi_gntcnt_e) {
        emi_gntcnt_e = 0;
        writel(readl(EMI_ARBE) | 0x00008000, EMI_ARBE);
    }
    if (emi_gntcnt_f) {
        emi_gntcnt_f = 0;
        writel(readl(EMI_ARBF) | 0x00008000, EMI_ARBF);
    }
    if (emi_gntcnt_g) {
        emi_gntcnt_g = 0;
        writel(readl(EMI_ARBG) | 0x00008000, EMI_ARBG);
    }

    /* restore EMI dummy read setting */
    if (emi_dr) {
        emi_dr = 0;
        mt65xx_reg_sync_writel(readl(EMI_DRCT) | 1, EMI_DRCT);
    }
}

void BM_Enable(void)
{
    unsigned int value;
    
    value = readl(EMI_BMEN);
    mt65xx_reg_sync_writel((value & (~BUS_MON_PAUSE)) | BUS_MON_EN, EMI_BMEN);
}

void BM_Disable(void)
{
    unsigned int value;
    
    value = readl(EMI_BMEN);
    mt65xx_reg_sync_writel(value & (~BUS_MON_EN), EMI_BMEN);
}

void BM_Pause(void)
{
    unsigned int value;
    
    value = readl(EMI_BMEN);
    mt65xx_reg_sync_writel(value | BUS_MON_PAUSE, EMI_BMEN);
}

void BM_Continue(void)
{
    unsigned int value;
    
    value = readl(EMI_BMEN);
    mt65xx_reg_sync_writel(value & (~BUS_MON_PAUSE), EMI_BMEN);
}

bool BM_IsOverrun(void)
{
    unsigned int value;
    
    value = readl(EMI_BMEN);
    if(value & BC_OVERRUN)
        return true;
    else
        return false; 
}

void BM_SetMonitorType(int MonitorType)
{
    unsigned int value;
    value = readl(EMI_BMEN);
    mt65xx_reg_sync_writel((value & ~(BM_SEL_ALL)) | MonitorType, EMI_BMEN);
}

void BM_SetReadWriteType(int ReadWriteType)
{
    unsigned int value;

    value = readl(EMI_BMEN);
    mt65xx_reg_sync_writel((value & 0xFFFFFFCF) | (ReadWriteType << 4), EMI_BMEN);
}
///////////////////////////////////////////////////////////////////////////////
int BM_GetBusCycCount(void)
{
    unsigned int value;
    value = readl(EMI_BMEN);
    
    if((value & BM_SEL_SLICE) != 0)
            return BM_ERR_WRONG_REQ;
    else if(value & BC_OVERRUN)
            return BM_ERR_OVERRUN;
    else
            return(readl(EMI_BCNT));
}

int BM_GetSliceCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_SLICE) == 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_BCNT));
}
///////////////////////////////////////////////////////////////////////////////
int BM_GetTransAllCount(void)
{    
    if((readl(EMI_BMEN) & BM_SEL_SLICE_ALL) != 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_TACT));
}

int BM_GetSliceAllCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_SLICE_ALL) == 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_TACT));
}

///////////////////////////////////////////////////////////////////////////////
#if 0
int BM_GetTansCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_EMI_CLOCK) != 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_TSCT));
}
#else
int BM_GetTransCount(int counter_num)
{
    switch(counter_num)
    {
        case 1:
                if((readl(EMI_BMEN) & BM_SEL_EMI_CLOCK) != 0)
                    return BM_ERR_WRONG_REQ;
                else
                    return(readl(EMI_TSCT));        
        case 2:
                return(readl(EMI_TSCT2));
        case 3:
                return(readl(EMI_TSCT3));
        default:
                return(BM_ERR_WRONG_REQ);
    }    
}
#endif

int BM_GetEmiClockCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_EMI_CLOCK) == 0)
            return BM_ERR_WRONG_REQ;        
    else
            return(readl(EMI_TSCT));
}
///////////////////////////////////////////////////////////////////////////////
int BM_GetWordAllCount(void)
{
    unsigned int value;
    value = readl(EMI_BMEN);
        
    if((value & BM_SEL_ROW_HIT) != 0)
            return BM_ERR_WRONG_REQ;    
    else if(value & BC_OVERRUN)
            return BM_ERR_OVERRUN;                        
    else
            return(readl(EMI_WACT));
}

int BM_GetRowHitCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_ROW_HIT) == 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_WACT));
}
///////////////////////////////////////////////////////////////////////////////
#if 0
int BM_GetWordCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_ROW_START) != 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_WSCT));
}
#else
int BM_GetWordCount(int counter_num)
{
    switch(counter_num)
    {
        case 1:
                if((readl(EMI_BMEN) & BM_SEL_ROW_START) != 0)
                    return BM_ERR_WRONG_REQ;
                else
                    return(readl(EMI_WSCT));                
        case 2:
                return(readl(EMI_WSCT2));
        case 3:
                return(readl(EMI_WSCT3));
        default:
                return(BM_ERR_WRONG_REQ);
    }        
}
#endif

int BM_GetRowStartCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_ROW_START) == 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_WSCT));
}
///////////////////////////////////////////////////////////////////////////////
int BM_GetBusyAllCount(void)
{
    unsigned int value;
    value = readl(EMI_BMEN);
        
    if((value & BM_SEL_ROW_CONFLICT) != 0)
            return BM_ERR_WRONG_REQ;
    else if(value & BC_OVERRUN)
            return BM_ERR_OVERRUN;                    
    else
            return(readl(EMI_BACT));
}

int BM_GetRowConflictCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_ROW_CONFLICT) == 0)
            return BM_ERR_WRONG_REQ;        
    else
            return(readl(EMI_BACT));
}
///////////////////////////////////////////////////////////////////////////////
int BM_GetBusyCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_INTER_BANK) != 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_BSCT));
}

int BM_GetInterBankCount(void)
{
    if((readl(EMI_BMEN) & BM_SEL_INTER_BANK) == 0)
            return BM_ERR_WRONG_REQ;
    else
            return(readl(EMI_BSCT));
}
///////////////////////////////////////////////////////////////////////////////

int BM_GetTransTypeCount(int counter_num)
{
    if(counter_num < 1 || counter_num > 21)
        return(BM_ERR_WRONG_REQ);
    else
        return(readl((unsigned int)EMI_TTYPE1+(counter_num-1)*8));
}

void BM_SetMonitorCounter(int counter_num, int master, int trans_type)
{
    unsigned int value;
    __u32 MSEL_Array[BM_COUNTER_MAX/2+1] = {EMI_BMEN,  EMI_MSEL,  EMI_MSEL2, EMI_MSEL3, 
                                            EMI_MSEL4, EMI_MSEL5, EMI_MSEL6, EMI_MSEL7, 
                                            EMI_MSEL8, EMI_MSEL9, EMI_MSEL10 };
                                                   
    if(counter_num < 1 || counter_num > 21)
            return;
                                                            
    if(counter_num == 1)
    {
            value = readl(MSEL_Array[counter_num / 2]);    
            value &= 0x0000FFFF;
            value |= ((master << 16) | (trans_type << 24));
    }
    else  
    {
            value = readl(MSEL_Array[counter_num / 2]);        
            if(counter_num % 2 == 0)
            {
                    value &= 0xFFFF0000;
                    value |= (master | (trans_type << 8));
            }
            else
            {
                    value &= 0x0000FFFF;
                    value |= ((master << 16) | (trans_type << 24));                
            }
    }
    mt65xx_reg_sync_writel(value, MSEL_Array[counter_num / 2]);
}

void BM_SetMaster(int counter_num, int master)
{
    unsigned int value;
    __u32 MSEL_Array[BM_COUNTER_MAX/2+1] = {EMI_BMEN,  EMI_MSEL,  EMI_MSEL2, EMI_MSEL3, 
                                            EMI_MSEL4, EMI_MSEL5, EMI_MSEL6, EMI_MSEL7, 
                                            EMI_MSEL8, EMI_MSEL9, EMI_MSEL10 };
                                                   
    if(counter_num < 1 || counter_num > 21)
            return;
                                                            
    if(counter_num == 1)
    {
            value = readl(MSEL_Array[counter_num / 2]);    
            value &= 0xFF00FFFF;
            value |= (master << 16);
    }
    else  
    {
            value = readl(MSEL_Array[counter_num / 2]);        
            if(counter_num % 2 == 0)
            {
                    value &= 0xFFFFFF00;
                    value |= (master);
            }
            else
            {
                    value &= 0xFF00FFFF;
                    value |= (master << 16);                
            }
    }
    mt65xx_reg_sync_writel(value, MSEL_Array[counter_num / 2]);
}

void  BM_SetIDSelect(int counter_num, int id, int enable)
{
    unsigned int value;
    unsigned int target_register;
    int shift_num;
    
       if(counter_num < 1 || counter_num > 21)
        return;
            
    if(id > 0x1F)
        return;
    
    if((enable != 0) && (enable != 1))
        return;
            
    target_register = ((unsigned int)EMI_BMID0 + (((counter_num-1) >> 2) << 3));
    value = readl(target_register);
    shift_num = ((counter_num - 1) % 4) << 3;
    value = (value & (~(0xFF << shift_num))) | (id << shift_num) | (enable << (shift_num+7));
    mt65xx_reg_sync_writel(value, target_register);
}

