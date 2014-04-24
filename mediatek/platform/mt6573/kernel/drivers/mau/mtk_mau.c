

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()¡¢kthread_run()

#include <linux/aee.h>



#include <mach/irqs.h>
#include <asm/tcm.h>
#include <asm/io.h>


#include <mach/mtk_mau.h>
#include "mtk_mau_reg.h"
#include "mtk_mau_debug.h"


#if defined(CONFIG_ARCH_MT6516)
    #include <mach/mt6516_typedefs.h>
    #include <mach/mt6516_boot.h>
    #include <mach/mt6516_reg_base.h>
    #include <mach/mt6516_pll.h>

#elif defined(CONFIG_ARCH_MT6573)
    #include <mach/mt6573_typedefs.h>
    #include <mach/mt6573_boot.h>
    #include <mach/mt6573_reg_base.h>
    #include <mach/mt6573_pll.h>
    #include <mach/mt6573_m4u.h>
#else
    #error "unknown arch"
#endif



#define MAU_DEBUG
#ifdef MAU_DEBUG
#define MAUDBG printk
#else
#define MAUDBG(x,...)
#endif

#define MAU_MSG
#ifdef MAU_MSG
#define MAUMSG printk
#else
#define MAUMSG(x,...)
#endif

#define MAU_ASSERT(x) if(!(x)){printk("[MAU] assert fail, file:%s, line:%d", __FILE__, __LINE__);}



#ifndef SLEEP_MILLI_SEC
#define SLEEP_MILLI_SEC(nMilliSec)\
    do { \
        long timeout = (nMilliSec) * HZ / 1000; \
        while(timeout > 0) \
        { \
            timeout = schedule_timeout(timeout); \
        } \
    } while(0)
#endif


#define ENABLE_MAU2_INTERRUPT(enable) \
    do { \
        unsigned int gmc2_ctl = ioread32(MAU_REG_GMC2_CON_RD);  \
        gmc2_ctl = enable ? (gmc2_ctl | (0x1<<2)) : (gmc2_ctl & ~(0x1<<2)); \
        iowrite32(gmc2_ctl, MAU_REG_GMC2_CON_WT); \
    } while(0)





char const* const mauPortName[MAU_MASK_ALL] = {
    [MAU1_MASK_RSV          ] = "reserve",
    [MAU1_MASK_DEFECT       ] = "defect",
    [MAU1_MASK_OVL_MSK      ] = "ovl_msk",
    [MAU1_MASK_OVL_DCP      ] = "ovl_dcp",
    [MAU1_MASK_JPG_ENC      ] = "jpg_enc",
    [MAU1_MASK_DPI          ] = "dpi",
    [MAU1_MASK_ROTDMA0_OUT0 ] = "rot_dma0",
    [MAU1_MASK_ROTDMA1_OUT0 ] = "rot_dma1",
    [MAU1_MASK_ROTDMA2_OUT0 ] = "rot_dma2",
    [MAU1_MASK_ROTDMA3_OUT0 ] = "rot_dma3",
    [MAU1_MASK_TVROT_OUT0   ] = "tv_rot",
    [MAU1_MASK_TVC          ] = "tvc",
    [MAU1_MASK_CAM          ] = "cam",
    [MAU1_MASK_JPG_DEC_FDVT ] = "jpg_dec_fdvt",
    [MAU1_MASK_FDVT_OUT2    ] = "fdvt_out2",
    [MAU1_MASK_LCD_R        ] = "lcd_r",
    [MAU1_MASK_LCD_W        ] = "lcd_w",
    [MAU1_MASK_GCMQ         ] = "gcmq",
    [MAU1_MASK_G2D_WR       ] = "g2d_wr",
    [MAU1_MASK_G2D_RD       ] = "g2d_rd",
    [MAU1_MASK_RDMA0_YUV    ] = "r_dma0",
    [MAU1_MASK_RDMA1_YUV    ] = "r_dma1",
    [MAU1_MASK_FDVT_OUT1    ] = "fdvt_out1",
    [MAU1_MASK_DPI_HWC      ] = "dpi_hwc",
    [MAU1_MASK_GIF1         ] = "gif1",
    [MAU1_MASK_GIF2         ] = "gif2",
    [MAU1_MASK_GIF3         ] = "gif3",
    [MAU1_MASK_PNG1         ] = "png1",
    [MAU1_MASK_PNG2         ] = "png2",
    [MAU1_MASK_PNG3         ] = "png3",
    [MAU1_MASK_VRZ          ] = "vrz",
    [MAU1_MASK_PCA          ] = "pca",

    [MAU2_MASK_SPI          ] = "spi",
    [MAU2_MASK_RISC         ] = "risc",
    [MAU2_MASK_DMA          ] = "dma",
    [MAU2_MASK_BS           ] = "bs",
    [MAU2_MASK_POST         ] = "post",
    [MAU2_MASK_CDMA         ] = "cdma",
    [MAU2_MASK_PRED         ] = "pred",
    [MAU2_MASK_RESI         ] = "resi",
    [MAU2_MASK_VLCSAD       ] = "vlcsad",
};


wait_queue_head_t mau1_monitor;
EXPORT_SYMBOL(mau1_monitor);


PMAU_REGS const MAU1_REG = (PMAU_REGS)(MAU1_BASE);
PMAU_REGS const MAU2_REG = (PMAU_REGS)(MAU2_BASE);
PMAU_REGS const MPU_REG = (PMAU_REGS)(MPU_BASE);





//static struct task_struct *monitor_task;

static bool is_mau_isr_log_on = true;
static bool is_mau_enabled =false;

typedef struct
{
    bool         	isEnabled;
    MTK_MAU_CONFIG  entryConf[3];
    unsigned long   violationCnt[3][MAU_MASK_ALL];
} _MAU_CONTEXT;
    static _MAU_CONTEXT mau_ctxt = {0};



/*****************************************************************************/


static MAU_REGS mau1RegBackup;
static MAU_REGS mau2RegBackup;
static bool mau2IntrEnableBackup = false;


#define MAU_REG_OFFSET(r)       offsetof(MAU_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))

const UINT32 BACKUP_MAU_REG_OFFSETS[] =
{
    MAU_REG_OFFSET(ENT0_RANGE_STR),
    MAU_REG_OFFSET(ENT0_RANGE_END),
    MAU_REG_OFFSET(ENT0_INVAL_LMST),
    MAU_REG_OFFSET(ENT0_INVAL_HMST),
    MAU_REG_OFFSET(ENT1_RANGE_STR),
    MAU_REG_OFFSET(ENT1_RANGE_END),
    MAU_REG_OFFSET(ENT1_INVAL_LMST),
    MAU_REG_OFFSET(ENT1_INVAL_HMST),
    MAU_REG_OFFSET(ENT2_RANGE_STR),
    MAU_REG_OFFSET(ENT2_RANGE_END),
    MAU_REG_OFFSET(ENT2_INVAL_LMST),
    MAU_REG_OFFSET(ENT2_INVAL_HMST),
    MAU_REG_OFFSET(INTERRUPT),
};

/*****************************************************************************/




void MAU_Mau1_Isr(void)
{
    if (MAU1_REG->ENT0_STATUS.ASSERT) {
    	mau_ctxt.violationCnt[0][MAU1_REG->ENT0_STATUS.ASSERT_ID]++;
    	if (is_mau_isr_log_on)
        	printk("[MAU] Violation! Port %s->[0x%x,0x%x]\n",
            	mauPortName[MAU1_REG->ENT0_STATUS.ASSERT_ID],
            	mau_ctxt.entryConf[0].StartAddr,
            	mau_ctxt.entryConf[0].EndAddr);
        OUTREG32(&MAU1_REG->ENT0_STATUS, 0x0);
    }

    if (MAU1_REG->ENT1_STATUS.ASSERT) {
    	mau_ctxt.violationCnt[1][MAU1_REG->ENT1_STATUS.ASSERT_ID]++;
    	if (is_mau_isr_log_on)
        	printk("[MAU] Violation! Port %s->[0x%x,0x%x]\n",
            	mauPortName[MAU1_REG->ENT1_STATUS.ASSERT_ID],
            	mau_ctxt.entryConf[1].StartAddr,
            	mau_ctxt.entryConf[1].EndAddr);

        OUTREG32(&MAU1_REG->ENT1_STATUS, 0x0);

    }

    if (MAU1_REG->ENT2_STATUS.ASSERT) {
    	mau_ctxt.violationCnt[2][MAU1_REG->ENT2_STATUS.ASSERT_ID]++;
    	if (is_mau_isr_log_on)
        	printk("[MAU] Violation! Port %s->[0x%x,0x%x]\n",
            	mauPortName[MAU1_REG->ENT2_STATUS.ASSERT_ID],
            	mau_ctxt.entryConf[2].StartAddr,
            	mau_ctxt.entryConf[2].EndAddr);
  
        OUTREG32(&MAU1_REG->ENT2_STATUS, 0x0);

    }
}
EXPORT_SYMBOL(MAU_Mau1_Isr);



static void _mau_mau2_isr(void)
{
    if (MAU2_REG->ENT0_STATUS.ASSERT) {
    	mau_ctxt.violationCnt[0][MAU2_REG->ENT0_STATUS.ASSERT_ID + MAU1_MASK_ALL +1]++;
    	if (is_mau_isr_log_on)
        	printk("[MAU] Violation! Port %s->[0x%x,0x%x]\n",
            	mauPortName[MAU2_REG->ENT0_STATUS.ASSERT_ID+ MAU1_MASK_ALL +1],
            	mau_ctxt.entryConf[0].StartAddr,
            	mau_ctxt.entryConf[0].EndAddr);

        OUTREG32(&MAU2_REG->ENT0_STATUS, 0x0);
    }

    if (MAU2_REG->ENT1_STATUS.ASSERT) {
    	mau_ctxt.violationCnt[1][MAU2_REG->ENT1_STATUS.ASSERT_ID+ MAU1_MASK_ALL +1]++;
    	if (is_mau_isr_log_on)
        	printk("[MAU] Violation! Port %s->[0x%x,0x%x]\n",
            	mauPortName[MAU2_REG->ENT1_STATUS.ASSERT_ID+ MAU1_MASK_ALL +1],
            	mau_ctxt.entryConf[1].StartAddr,
            	mau_ctxt.entryConf[1].EndAddr);


        OUTREG32(&MAU2_REG->ENT1_STATUS, 0x0);

    }

    if (MAU2_REG->ENT2_STATUS.ASSERT) {
    	mau_ctxt.violationCnt[2][MAU2_REG->ENT2_STATUS.ASSERT_ID+ MAU1_MASK_ALL +1]++;
    	if (is_mau_isr_log_on)
        	printk("[MAU] Violation! Port %s->[0x%x,0x%x]\n",
            	mauPortName[MAU2_REG->ENT2_STATUS.ASSERT_ID+ MAU1_MASK_ALL +1],
            	mau_ctxt.entryConf[2].StartAddr,
            	mau_ctxt.entryConf[2].EndAddr);

        OUTREG32(&MAU2_REG->ENT2_STATUS, 0x0);

    }

}


static __tcmfunc irqreturn_t mau_mau2_isr(int irq, void *dev_id)
{
    unsigned int  status = (MAU2_REG->INTERRUPT);

    mt6573_irq_mask(MT6573_GMC2_IRQ_LINE);

    //printk("_TVC_InterruptHandler, status: %x\n", status);

    if (1 == (status & 0x1)) {
        status &= ~0x1;
        printk("[MAU][MAU2] isr !!\n");
        _mau_mau2_isr();
    }

    OUTREG32(&MAU2_REG->INTERRUPT, AS_UINT32(&status));

    mt6573_irq_unmask(MT6573_GMC2_IRQ_LINE);

    return IRQ_HANDLED;

}


static void mau_reset_ctxt(void)
{
    printk("[MAU]%s", __func__);
    memset(&mau_ctxt, 0, sizeof(mau_ctxt));
    //memset(&mau2_ctxt, 0, sizeof(mau2_ctxt));
}






static void mau_mau1_stop(MTK_MAU_ENTRY eID)
{
    //MAU1
    MAU_REG_RANGE_START config = {0};
    config.RD = 0;
    config.WR = 0;

    switch (eID) {
        case MAU_ENTRY_0:
            OUTREG32(&MAU1_REG->ENT0_RANGE_STR, AS_UINT32(&config));
            break;
        case MAU_ENTRY_1:
            OUTREG32(&MAU1_REG->ENT1_RANGE_STR, AS_UINT32(&config));
            break;
        case MAU_ENTRY_2:
            OUTREG32(&MAU1_REG->ENT2_RANGE_STR, AS_UINT32(&config));
            break;
        case MAU_ENTRY_ALL:
            OUTREG32(&MAU1_REG->ENT0_RANGE_STR, AS_UINT32(&config));
            OUTREG32(&MAU1_REG->ENT1_RANGE_STR, AS_UINT32(&config));
            OUTREG32(&MAU1_REG->ENT2_RANGE_STR, AS_UINT32(&config));
            OUTREG32(&MAU1_REG->INTERRUPT, 0x0);
            mau_reset_ctxt();
            break;
        default:
            printk("[MAU]Error! %s pass wrong entry ID\n", __func__);
            break;


    }

}

static void mau_mau2_stop(MTK_MAU_ENTRY eID)
{
    //MAU2
    MAU_REG_RANGE_START config = {0};
    config.RD = 0;
    config.WR = 0;

    switch (eID) {
        case MAU_ENTRY_0:
            OUTREG32(&MAU2_REG->ENT0_RANGE_STR, AS_UINT32(&config));
            break;
        case MAU_ENTRY_1:
            OUTREG32(&MAU2_REG->ENT1_RANGE_STR, AS_UINT32(&config));
            break;
        case MAU_ENTRY_2:
            OUTREG32(&MAU2_REG->ENT2_RANGE_STR, AS_UINT32(&config));
            break;
        case MAU_ENTRY_ALL:
            OUTREG32(&MAU2_REG->ENT0_RANGE_STR, AS_UINT32(&config));
            OUTREG32(&MAU2_REG->ENT1_RANGE_STR, AS_UINT32(&config));
            OUTREG32(&MAU2_REG->ENT2_RANGE_STR, AS_UINT32(&config));
            OUTREG32(&MAU2_REG->INTERRUPT, 0x0);

            //OUTREG32(&MAU_REG_GMC2_CON_CLR, 0x0);
            //iowrite32(0x0, MAU_REG_GMC2_CON_CLR);
            ENABLE_MAU2_INTERRUPT(false);

            mau_reset_ctxt();
            break;
        default:
            printk("[MAU]Error! %s pass wrong entry ID\n", __func__);
            break;


    }

}


static int mau_mau1_config(MTK_MAU_CONFIG* pMauConf)
{
    MAUDBG("[MAU][MAU1_CONFIG] eID=0x%x, enable=%d, GMC1=0x%x, GMC2=0x%x, ReadEn=0x%x, WriteEn=0x%x, StartAddr=0x%x, EndAddr=0x%x \n",
        pMauConf->EntryID,
        pMauConf->Enable,
        pMauConf->InvalidMasterGMC1,
        pMauConf->InvalidMasterGMC2,
        pMauConf->ReadEn,
        pMauConf->WriteEn,
        pMauConf->StartAddr,
        pMauConf->EndAddr);

    memcpy(&mau_ctxt.entryConf[pMauConf->EntryID], pMauConf, sizeof(MTK_MAU_CONFIG));

    if (pMauConf->Enable == false) {
        mau_mau1_stop(pMauConf->EntryID);
    }
    else {

        MAU_REG_RANGE_START config = {0};
        UINT32 endAddr;
        UINT32 portMask;

	    MAU_ASSERT(pMauConf->EndAddr >= pMauConf->StartAddr);

        config.RD = pMauConf->ReadEn;
        config.WR = pMauConf->WriteEn;
        config.STR = (pMauConf->StartAddr)>>2;

        endAddr = (pMauConf->EndAddr)>>2;
        portMask = pMauConf->InvalidMasterGMC1;

        switch (pMauConf->EntryID) {
            case MAU_ENTRY_0:
                OUTREG32(&MAU1_REG->ENT0_RANGE_STR, AS_UINT32(&config));
                OUTREG32(&MAU1_REG->ENT0_RANGE_END, AS_UINT32(&endAddr));
                OUTREG32(&MAU1_REG->ENT0_INVAL_LMST, AS_UINT32(&portMask));
                break;
            case MAU_ENTRY_1:
                OUTREG32(&MAU1_REG->ENT1_RANGE_STR, AS_UINT32(&config));
                OUTREG32(&MAU1_REG->ENT1_RANGE_END, AS_UINT32(&endAddr));
                OUTREG32(&MAU1_REG->ENT1_INVAL_LMST, AS_UINT32(&portMask));
                break;
            case MAU_ENTRY_2:
                OUTREG32(&MAU1_REG->ENT2_RANGE_STR, AS_UINT32(&config));
                OUTREG32(&MAU1_REG->ENT2_RANGE_END, AS_UINT32(&endAddr));
                OUTREG32(&MAU1_REG->ENT2_INVAL_LMST, AS_UINT32(&portMask));
                break;
            default:
                printk("[MAU]Error! %s pass wrong entry ID\n", __func__);
                break;
        }

        OUTREG32(&MAU1_REG->INTERRUPT, 0x1);

        MAU_Mau1DumpReg();

    }
    return 0;
}




static int mau_mau2_config(MTK_MAU_CONFIG* pMauConf)
{
    MAUDBG("[MAU][MAU2_CONFIG] eID=0x%x,enable=%d, GMC1=0x%x, GMC2=0x%x, ReadEn=0x%x, WriteEn=0x%x, StartAddr=0x%x, EndAddr=0x%x \n",
        pMauConf->EntryID,
        pMauConf->Enable,
        pMauConf->InvalidMasterGMC1,
        pMauConf->InvalidMasterGMC2,
        pMauConf->ReadEn,
        pMauConf->WriteEn,
        pMauConf->StartAddr,
        pMauConf->EndAddr);

    memcpy(&mau_ctxt.entryConf[pMauConf->EntryID], pMauConf, sizeof(MTK_MAU_CONFIG));

    if (pMauConf->Enable == false) {
        mau_mau2_stop(pMauConf->EntryID);
    }
    else {

        MAU_REG_RANGE_START config = {0};
        UINT32 endAddr;
        UINT32 portMask;

	    MAU_ASSERT(pMauConf->EndAddr >= pMauConf->StartAddr);

        config.RD = pMauConf->ReadEn;
        config.WR = pMauConf->WriteEn;
        config.STR = (pMauConf->StartAddr)>>2;

        endAddr = (pMauConf->EndAddr)>>2;
        portMask = pMauConf->InvalidMasterGMC2;

        switch (pMauConf->EntryID) {
            case MAU_ENTRY_0:
                OUTREG32(&MAU2_REG->ENT0_RANGE_STR, AS_UINT32(&config));
                OUTREG32(&MAU2_REG->ENT0_RANGE_END, AS_UINT32(&endAddr));
                OUTREG32(&MAU2_REG->ENT0_INVAL_LMST, AS_UINT32(&portMask));
                break;
            case MAU_ENTRY_1:
                OUTREG32(&MAU2_REG->ENT1_RANGE_STR, AS_UINT32(&config));
                OUTREG32(&MAU2_REG->ENT1_RANGE_END, AS_UINT32(&endAddr));
                OUTREG32(&MAU2_REG->ENT1_INVAL_LMST, AS_UINT32(&portMask));
                break;
            case MAU_ENTRY_2:
                OUTREG32(&MAU2_REG->ENT2_RANGE_STR, AS_UINT32(&config));
                OUTREG32(&MAU2_REG->ENT2_RANGE_END, AS_UINT32(&endAddr));
                OUTREG32(&MAU2_REG->ENT2_INVAL_LMST, AS_UINT32(&portMask));
                break;
            default:
                printk("[MAU]Error! %s pass wrong entry ID\n", __func__);
                break;
        }

        //OUTREG32(&MAU1_REG->INTERRUPT, 0x1);
        //OUTREG32(&MAU_REG_GMC2_CON_WT, 0x1<<2);
        ENABLE_MAU2_INTERRUPT(true);
        MAU_Mau2DumpReg();

    }
    return 0;
}






#if 0
static bool mau_check_assert(void)
{
    if (MAU1_REG->ENT0_STATUS.ASSERT ||
        MAU1_REG->ENT1_STATUS.ASSERT ||
        MAU1_REG->ENT2_STATUS.ASSERT)
        return true;
    else
        return false;
}
#endif

int MAU_Mau1PowerOn(void)
{
	MAUDBG("[MAU] %s\n", __func__);

    if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC1 , "MAU")){MAUDBG("Enable GMC1 clock failed!\n");}
	if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC1E , "MAU")){MAUDBG("Enable GMC1E clock failed!\n");}
    if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC1SLV , "MAU")){MAUDBG("Enable GMC1SLV clock failed!\n");}


  return 0;
}

int MAU_Mau1PowerOff(void)
{
	MAUDBG("[MAU] %s\n", __func__);

	if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC1 , "MAU")){MAUDBG("Disable GMC1 clock failed!\n");}
	if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC1E , "MAU")){MAUDBG("Disable GMC1E clock failed!\n");}
	if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC1SLV , "MAU")){MAUDBG("Disable GMC1SLV clock failed!\n");}

  return 0;
}

int MAU_Mau2PowerOn(void)
{
	MAUDBG("[MAU] %s\n", __func__);

    if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC2 , "MAU")){MAUDBG("Enable GMC2 clock failed!\n");}
	if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC2_E , "MAU")){MAUDBG("Enable GMC2E clock failed!\n");}

    //MAU_Mau2RestoreReg();

  return 0;
}

int MAU_Mau2PowerOff(void)
{
	MAUDBG("[MAU] %s\n", __func__);
    //MAU_Mau2BackupReg();

    if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC2 , "MAU")){MAUDBG("Disable GMC2 clock failed!\n");}
    if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC2_E , "MAU")){MAUDBG("Disable GMC2E clock failed!\n");}

  return 0;
}


void MAU_Mau1BackupReg(void)
{
    MAU_REGS *reg = &mau1RegBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_MAU_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_MAU_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(MAU1_REG, BACKUP_MAU_REG_OFFSETS[i])));
    }
}

void MAU_Mau1RestoreReg(void)
{
    MAU_REGS *reg = &mau1RegBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_MAU_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(MAU1_REG, BACKUP_MAU_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_MAU_REG_OFFSETS[i])));
    }
}


/*****************************************************************************/
int MAU_Config(MTK_MAU_CONFIG* pMauConf)
{
    
    mau_mau1_config(pMauConf);
    
    
    MAU_Mau2PowerOn();
    mau_mau2_config(pMauConf);
    MAU_Mau2BackupReg();
    MAU_Mau2PowerOff();

    return 0;
    
}


void MAU_LogSwitch(bool enable)
{
    printk("[MAU]LOG %s\n", (enable?"ON":"OFF"));
    if (is_mau_isr_log_on == enable) return;
    else is_mau_isr_log_on = enable;

    return;

}

void MAU_PrintStatus(char* buf, unsigned int buf_len, unsigned int* num)
{
    unsigned int n = *num;
    int id = 0;
    int i = 0;
    is_mau_enabled = mau_ctxt.entryConf[0].Enable ||
                      mau_ctxt.entryConf[1].Enable ||
                      mau_ctxt.entryConf[2].Enable;

    n += scnprintf(buf + n, buf_len - n, "[MM MAU] ----------debug info status start ----------\n");
    n += scnprintf(buf + n, buf_len - n, "--mm mau is %s\n", (is_mau_enabled?"ON":"OFF"));

    if (is_mau_enabled) {
        n += scnprintf(buf + n, buf_len - n, "--Module ID List:");
        for (i = 0; i<MAU_MASK_ALL; i++) {
            if (i%8 == 0) n += scnprintf(buf + n, buf_len - n, "\n");
	        n += scnprintf(buf + n, buf_len - n, "  %02d:%s", i, mauPortName[i]);
        }
        n += scnprintf(buf + n, buf_len - n, "\n");
        n += scnprintf(buf + n, buf_len - n, "---------------------------------------------\n");

        for (id = 0; id < MAU_ENTRY_ALL; id++) {
            n += scnprintf(buf + n, buf_len - n, "--entry [%d] \n", id);
            n += scnprintf(buf + n, buf_len - n, "--protected mem: [0x%x, 0x%x]\n",
                           mau_ctxt.entryConf[id].StartAddr,
                           mau_ctxt.entryConf[id].EndAddr);
            n += scnprintf(buf + n, buf_len - n, "--read(%s) write(%s)\n",
                           (mau_ctxt.entryConf[id].ReadEn?"Y":"N"),
                           (mau_ctxt.entryConf[id].WriteEn?"Y":"N"));
            n += scnprintf(buf + n, buf_len - n, "--guarded modules: ");
            for (i=0; i < MAU_MASK_ALL; i++ ) {
                //MAU_ASSERT(i<32);
                if (i < MAU1_MASK_ALL) {
                    if ((0x1<<i) & mau_ctxt.entryConf[id].InvalidMasterGMC1) {
                        n += scnprintf(buf + n, buf_len - n, "%02d ", i+1);
                    }
                }
                else if (i > MAU1_MASK_ALL)
                {
                    if ((0x1<<(i-MAU1_MASK_ALL-1)) & mau_ctxt.entryConf[id].InvalidMasterGMC2) {
                        n += scnprintf(buf + n, buf_len - n, "%02d ", i);
                    }

                }
            }
            n += scnprintf(buf + n, buf_len - n, "\n");
            n += scnprintf(buf + n, buf_len - n, "--violation times: ");
	        for ( i = 0; i < MAU_MASK_ALL; i++ ) {
                if (i%8 == 0) n += scnprintf(buf + n, buf_len - n, "\n");
		        n += scnprintf(buf + n, buf_len - n, "  %02d:%d", i, (int)mau_ctxt.violationCnt[id][i]);

		    }
            n += scnprintf(buf + n, buf_len - n, "\n");
            n += scnprintf(buf + n, buf_len - n, "---------------------------------------------");
            n += scnprintf(buf + n, buf_len - n, "\n");
	    }
    }

    n += scnprintf(buf + n, buf_len - n, "[MM MAU] ----------debug info status end   ----------\n");
    buf[n++] = 0;

    *num = n;
}


void MAU_Mau1DumpReg(void)
{

    UINT32 i;


    printk("---------- Start dump MAU1 registers ----------\n"
           "MAU1_BASE: 0x%08x\n", MAU1_BASE);

    for (i = 0; i < sizeof(MAU_REGS); i += 4)
    {
        printk("MAU1+%04x : 0x%08x\n", i, INREG32(MAU1_BASE + i));
    }

    return;

}

void MAU_Mau2DumpReg(void)
{

    UINT32 i;

    printk("---------- Start dump MAU2 registers ----------\n"
           "MAU2_BASE: 0x%08x\n", MAU2_BASE);

    printk("GMC2+%08x : 0x%08x\n", MAU_REG_GMC2_CON_RD, INREG32(MAU_REG_GMC2_CON_RD));

    for (i = 0; i < sizeof(MAU_REGS); i += 4)
    {
        printk("MAU2+%04x : 0x%08x\n", i, INREG32(MAU2_BASE + i));
    }

    return;

}

void MAU_MpuDumpReg(void)
{

    UINT32 i;

    printk("---------- Start dump MPU registers ----------\n"
           "MPU_BASE: 0x%08x\n", MPU_BASE);

    for (i = 0; i < sizeof(MAU_REGS); i += 4)
    {
        printk("MPU+%04x : 0x%08x\n", i, INREG32(MPU_BASE + i));
    }

    return;

}




void MAU_Mau2BackupReg(void)
{
    MAU_REGS *reg = &mau2RegBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_MAU_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_MAU_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(MAU2_REG, BACKUP_MAU_REG_OFFSETS[i])));
    }

    if (ioread32(MAU_REG_GMC2_CON_RD) & (0x1<<2)) mau2IntrEnableBackup = true;
    else mau2IntrEnableBackup = false;

}
EXPORT_SYMBOL(MAU_Mau2BackupReg);

void MAU_Mau2RestoreReg(void)
{
    MAU_REGS *reg = &mau2RegBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_MAU_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(MAU2_REG, BACKUP_MAU_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_MAU_REG_OFFSETS[i])));
    }

    ENABLE_MAU2_INTERRUPT(mau2IntrEnableBackup);
}
EXPORT_SYMBOL(MAU_Mau2RestoreReg);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mau_early_suspend(struct early_suspend *h)
{
    MAUDBG("[MAU]%s \n", __func__);

    //MAU_Mau1BackupReg();
    MAU_Mau1PowerOff();

}

static void mau_late_resume(struct early_suspend *h)
{
    MAUDBG("[MAU]%s \n", __func__);

    MAU_Mau1PowerOn();
    //MAU_Mau1RestoreReg();


}

static struct early_suspend mau_early_suspend_handler =
{
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = mau_early_suspend,
	.resume = mau_late_resume,
};
#endif
static void mau_high_gate(struct clock_listener *cl)
{
   printk("[%s]:clock_listener:gate high\r\n", __FUNCTION__);
   if (!is_mau_isr_log_on) MAU_Mau1DumpReg();
   MAU_Mau1BackupReg();
}

static void mau_high_ungate(struct clock_listener *cl)
{
   printk("[%s]:clock_listener:ungate high\r\n", __FUNCTION__);
   MAU_Mau1RestoreReg();
   if (!is_mau_isr_log_on) MAU_Mau1DumpReg();
}

struct clock_listener mau_clock_listener = {
   .name = "mau_high",
   .level = CLOCK_LISTEN_HIGH,
   .gate = mau_high_gate,
   .ungate = mau_high_ungate,
};


#if 0
void mau_monitor(void)
{
    static int i =0;
    MAUDBG("[MAU] %s enter\n",__func__);
    while(!kthread_should_stop())
    {
        long ret = wait_event_interruptible(mau1_monitor,
                           mau_check_assert());
		printk("[MAU]monitor wake up, check assert %d\n", i++);
        //SLEEP_MILLI_SEC(1000);
        if (MAU1_REG->ENT0_STATUS.ASSERT) {
            printk("[MAU] Enty 0: %s-->\n",
                mau1PortName[MAU1_REG->ENT0_STATUS.ASSERT_ID]);
            OUTREG32(&MAU1_REG->ENT0_STATUS, 0x0);
        }

        if (MAU1_REG->ENT1_STATUS.ASSERT) {
            printk("[MAU] Enty 1: %s-->\n",
                mau1PortName[MAU1_REG->ENT1_STATUS.ASSERT_ID]);
            OUTREG32(&MAU1_REG->ENT1_STATUS, 0x0);
        }

        if (MAU1_REG->ENT2_STATUS.ASSERT) {
            printk("[MAU] Enty 2: %s-->\n",
                mau1PortName[MAU1_REG->ENT2_STATUS.ASSERT_ID]);
            OUTREG32(&MAU1_REG->ENT2_STATUS, 0x0);
        }
		mau_mau1_dump_reg();


    }
    MAUDBG("[MAU] %s exit\n",__func__);
    return;
}

void mau_start_monitor(void)
{
    MAUDBG("[MAU] %s\n",__func__);
    monitor_task = kthread_run(mau_monitor,NULL,"mau_monitor");
}

void mau_stop_monitor(void)
{
    MAUDBG("[MAU] %s\n",__func__);

    if (monitor_task)
        kthread_stop(monitor_task);
    wake_up_interruptible(&mau1_monitor);

}

#endif


static int mau_ioctl(struct inode * a_pstInode,
						 struct file * a_pstFile,
						 unsigned int a_u4Command,
						 unsigned long a_u4Param)
{
    int ret = 0;

	return ret;
}


static const struct file_operations mauFops =
{
	.owner = THIS_MODULE,
	.ioctl = mau_ioctl,
};

static struct cdev * pMauDev = NULL;
static dev_t mauDevNo = MKDEV(MTK_MAU_MAJOR_NUMBER,0);
static inline int mau_register(void)
{
    MAUDBG("[MAU] %s\n",__func__);
    if (alloc_chrdev_region(&mauDevNo, 0, 1,"MTK_MAU")){
        MAUDBG("[MAU] Allocate device no failed\n");
        return -EAGAIN;
    }

    //Allocate driver
    pMauDev = cdev_alloc();

    if (NULL == pMauDev) {
        unregister_chrdev_region(mauDevNo, 1);
        MAUDBG("[MAU] Allocate mem for kobject failed\n");
        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(pMauDev, &mauFops);
    pMauDev->owner = THIS_MODULE;


    //Add to system
    if (cdev_add(pMauDev, mauDevNo, 1)) {
        printk("[MAU] Attatch file operation failed\n");
        unregister_chrdev_region(mauDevNo, 1);
        return -EAGAIN;
    }

    return 0;
}



static struct class *pMauClass = NULL;
// Called to probe if the device really exists. and create the semaphores
static int mau_probe(struct platform_device *pdev)
{
    struct device* mauDevice = NULL;
    printk("[MAU] %s\n",__func__);

    //Check platform_device parameters
    if (NULL == pdev) {
        MAUMSG("[MAU] platform data missed\n");
        return -ENXIO;
    }

    //register char driver
    //Allocate major no
    if (mau_register()) {
        dev_err(&pdev->dev,"register char failed\n");
        return -EAGAIN;
    }

    pMauClass = class_create(THIS_MODULE, "MTK_MAU");
    if (IS_ERR(pMauClass)) {
        int ret = PTR_ERR(pMauClass);
        printk("Unable to create class, err = %d\n", ret);
        return ret;
    }
    mauDevice = device_create(pMauClass, NULL, mauDevNo, NULL, "MTK_MAU");

    init_waitqueue_head(&mau1_monitor);


    MAU_DBG_Init();
	mau_reset_ctxt();


    if (request_irq(MT6573_GMC2_IRQ_LINE,
                    (irq_handler_t)mau_mau2_isr,
                    0, "GMC2", NULL) < 0)
    {
        printk("[MAU][ERROR] fail to request MAU2 irq\n");
        return -EINTR;
    }

    MAU_Mau1PowerOn();

#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&mau_early_suspend_handler);
#endif
    register_clock_listener(&mau_clock_listener);

    return 0;
}



// Called when the device is being detached from the driver
static int mau_remove(struct platform_device *pdev)
{
    //Release char driver
    cdev_del(pMauDev);

    unregister_chrdev_region(mauDevNo, 1);

    device_destroy(pMauClass, mauDevNo);

    class_destroy(pMauClass);

    MAUDBG("MTK_MAU driver is removed\n");

    return 0;
}


static int mau_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    printk("[MAU] suspend\n");
    return 0;
}

static int mau_resume(struct platform_device *pdev)
{
    printk("[MAU] resume\n");
    return 0;
}

static struct platform_driver mauDrv = {
    .probe	= mau_probe,
    .remove	= mau_remove,
    .suspend= mau_suspend,
    .resume	= mau_resume,
    .driver	= {
    .name	= "MTK_MAU",
    .owner	= THIS_MODULE,
    }
};


static int __init mau_init(void)
{
    if(platform_driver_register(&mauDrv)){
        printk("[MAU]failed to register MAU driver\n");
        return -ENODEV;
    }
    printk("[MAU] %s\n",__func__);

    //memset(&mau1RegBackup, 0, sizeof(MAU_REGS));
    //memset(&mau2RegBackup, 0, sizeof(MAU_REGS));

    return 0;
}

static void __exit mau_exit(void)
{
    printk("[MAU] %s\n",__func__);
    platform_driver_unregister(&mauDrv);

    
    MAU_Mau1PowerOff();

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&mau_early_suspend_handler);
#endif
}



module_init(mau_init);
module_exit(mau_exit);

MODULE_DESCRIPTION("MTK MAU driver");
MODULE_AUTHOR("Mingchen <Mingchen.gao@Mediatek.com>");
MODULE_LICENSE("GPL");


