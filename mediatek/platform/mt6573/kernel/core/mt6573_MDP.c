
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/mm.h>

//Arch dependent files
#include <mach/sync_write.h>
#include <mach/mt6573_MDP.h>
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_pll.h>
#include <mach/irqs.h>
#include <asm/tcm.h>
#include <mach/mt6573_m4u.h>

//--------------------------------------------Define-----------------------------------------------//
#define MT6573MDP_DEBUG
#ifdef MT6573MDP_DEBUG
#define MDPDB printk
#else
#define MDPDB(x,...)
#endif

#define MT6573MDP_MSG
#ifdef MT6573MDP_MSG
#define MDPMSG printk
#else
#define MDPMSG(x,...)
#endif

typedef struct {
    pid_t pid;
    unsigned long u4OwnResource;
} stMDPOpLog;

typedef struct {
    unsigned long u4FreeTable;
    unsigned long u4OccupiedTable;
    unsigned long u4GMCPowerSatus;
    spinlock_t ResLock;
} stMT6573MDPResTbl;

typedef struct {
    unsigned long u4Table;
    spinlock_t ResLock;
} stMT6573MDPIrqTbl;

typedef struct {
    spinlock_t ResLock;
    unsigned long u4Dirty;
    unsigned long u4FrameCounter;
    unsigned long u4AppliedFrame;
    stZoomSetting stReg;
} stMT6573MDPZoomReg;
//-------------------------------------Global variables------------------------------------------------//

stMT6573MDPResTbl g_stMT6573ResTbl;
stMT6573MDPIrqTbl g_stMT6573IRQTbl;

static wait_queue_head_t g_MT6573MDPResWaitQueue;
static wait_queue_head_t g_MT6573MDPIRQWaitQueue;

static unsigned long u4MT6573RotExecCnt[4];

static stMT6573MDPZoomReg g_stMT6573ZoomReg;

//--------------------------------------Functions-----------------------------------------------------//
void MT6573MDP_DUMPREG(void)
{
#ifdef MT6573MDP_DEBUG
    u32 u4RegValue = 0;
    u32 u4Index = 0;

    //Check stalled part
    MDPDB("Dump register!!\n ********************\n");
    MDPDB("Stalled part:\n");
    u4RegValue = ioread32(MMSYS1_CONFIG_BASE+0x56C);
    if(0x1 & u4RegValue){MDPDB("JPGDEC to BRZ\n");}
    if(0x2 & u4RegValue){MDPDB("BRZ to MOUT3\n");}
    if(0x4 & u4RegValue){MDPDB("MOUT3 to ISP\n");}
    if(0x8 & u4RegValue){MDPDB("MOUT3 to PRZ0\n");}
    if(0x10 & u4RegValue){MDPDB("MOUT3 to VRZ\n");}
    if(0x20 & u4RegValue){MDPDB("RDMA0 to MOUT2\n");}
    if(0x40 & u4RegValue){MDPDB("MOUT2 to ISP\n");}
    if(0x80 & u4RegValue){MDPDB("MOUT2 to PRZ0\n");}
    if(0x100 & u4RegValue){MDPDB("MOUT2 to VRZ\n");}
    if(0x200 & u4RegValue){MDPDB("MOUT2 to JPGDMA\n");}
    if(0x400 & u4RegValue){MDPDB("ISP to CRZ\n");}
    if(0x800 & u4RegValue){MDPDB("CRZ to OVL\n");}
    if(0x1000 & u4RegValue){MDPDB("OVL to IPP\n");}
    if(0x2000 & u4RegValue){MDPDB("IPP to MOUT0\n");}
    if(0x4000 & u4RegValue){MDPDB("MOUT0 to ROTDMA0\n");}
    if(0x8000 & u4RegValue){MDPDB("MOUT0 to PRZ0\n");}
    if(0x10000 & u4RegValue){MDPDB("MOUT0 to VRZ\n");}
    if(0x20000 & u4RegValue){MDPDB("MOUT0 to JPGDMA\n");}
    if(0x40000 & u4RegValue){MDPDB("PRZ0 to MOUT1\n");}
    if(0x80000 & u4RegValue){MDPDB("MOUT1 to ROTDMA1\n");}
    if(0x100000 & u4RegValue){MDPDB("MOUT1 to VRZ\n");}
    if(0x200000 & u4RegValue){MDPDB("VRZ to ROTDMA2\n");}
    if(0x400000 & u4RegValue){MDPDB("JPGENC to JPGDMA\n");}
    if(0x800000 & u4RegValue){MDPDB("RDMA1 to PRZ1\n");}
    if(0x1000000 & u4RegValue){MDPDB("LCDMUX to ROTDMA3\n");}
    if(0x2000000 & u4RegValue){MDPDB("ISPMUX to ISP\n");}
    if(0x4000000 & u4RegValue){MDPDB("PRZ0MUX to PRZ\n");}
    if(0x8000000 & u4RegValue){MDPDB("VRZMUX to VRZ\n");}
    if(0x10000000 & u4RegValue){MDPDB("PRZ1 to LCDMUX\n");}
    if(0x20000000 & u4RegValue){MDPDB("LCDMUX to LCD\n");}
    if(0x40000000 & u4RegValue){MDPDB("JPGDMAMUX to JPGDMA\n");}
    if(0x80000000 & u4RegValue){MDPDB("ROTDMA0MUX to ROTDMA0\n");}

    MDPDB("Clock gate:\n ********************\n");
    u4RegValue = ioread32(MMSYS1_CONFIG_BASE+0x300);
    MDPDB("+0x%x 0x%x\n", 0x300,u4RegValue);
    u4RegValue = ioread32(MMSYS1_CONFIG_BASE+0x304);
    MDPDB("+0x%x 0x%x\n", 0x304,u4RegValue);

    MDPDB("MMSYS1_CONFIG_BASE:\n ********************\n");
    for(u4Index = 0x500 ; u4Index < 0x56C ; u4Index += 4){
        u4RegValue = ioread32(MMSYS1_CONFIG_BASE+u4Index);
        MDPDB("+0x%x 0x%x\n", u4Index,u4RegValue);
    } 
    MDPDB("CRZ REG:\n ********************\n");
    for(u4Index = 0 ; u4Index < 0x28 ; u4Index += 4){
        u4RegValue = ioread32(CRZ_BASE+u4Index);
        MDPDB("+0x%x 0x%x\n", u4Index,u4RegValue);
    }
    u4RegValue = ioread32(CRZ_BASE+0x40);
    MDPDB("+0x%x 0x%x\n", 0x40 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0xF0);
    MDPDB("+0x%x 0x%x\n", 0xF0 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0xF4);
    MDPDB("+0x%x 0x%x\n", 0xF4 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0xF8);
    MDPDB("+0x%x 0x%x\n", 0xF8 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0x80);
    MDPDB("+0x%x 0x%x\n", 0x80 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0x84);
    MDPDB("+0x%x 0x%x\n", 0x84 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0xB0);
    MDPDB("+0x%x 0x%x\n", 0xB0 , u4RegValue);
    u4RegValue = ioread32(CRZ_BASE+0xB4);
    MDPDB("+0x%x 0x%x\n", 0xB4 , u4RegValue);

    MDPDB("ROTDMA0 REG:\n ********************\n");
    for(u4Index = 0 ; u4Index < 0x8C ; u4Index += 4){
        u4RegValue = ioread32(ROT_DMA0_BASE+u4Index);
        MDPDB("+0x%x 0x%x\n", u4Index,u4RegValue);
    }
    for(u4Index = 0x300 ; u4Index < 0x37C ; u4Index += 4){
        u4RegValue = ioread32(ROT_DMA0_BASE+u4Index);
        MDPDB("+0x%x 0x%x\n", u4Index,u4RegValue);
    }
    u4RegValue = ioread32(ROT_DMA0_BASE+0x400);
    MDPDB("+0x%x 0x%x\n", 0x400 , u4RegValue);
    u4RegValue = ioread32(ROT_DMA0_BASE+0x418);
    MDPDB("+0x%x 0x%x\n", 0x418 , u4RegValue);
    u4RegValue = ioread32(ROT_DMA0_BASE+0x430);
    MDPDB("+0x%x 0x%x\n", 0x430 , u4RegValue);
    for(u4Index = 0x800 ; u4Index < 0x838 ; u4Index += 4){
        u4RegValue = ioread32(ROT_DMA0_BASE+u4Index);
        MDPDB("+0x%x 0x%x\n", u4Index,u4RegValue);
    }
#endif
}

//file operations
static int MT6573_MDP_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    stMDPOpLog * pstLog;

    //Allocate and initialize private data
    a_pstFile->private_data = kmalloc(sizeof(stMDPOpLog) , GFP_ATOMIC);

    if(NULL == a_pstFile->private_data)
    {
        MDPDB("Not enough entry for MDP open operation\n");
        return -ENOMEM;
    }

    pstLog = (stMDPOpLog *)a_pstFile->private_data;
    pstLog->pid = current->pid;
    pstLog->u4OwnResource = 0;

    return 0;
}

static int MT6573_MDP_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    if(NULL != a_pstFile->private_data)
    {
        kfree(a_pstFile->private_data);
        a_pstFile->private_data = NULL;
    }

    return 0;
}

static int MT6573_UnLockResource(unsigned long a_u4ResTbl , stMDPOpLog * a_pstLog);


#define MT6573_ROTDMA0_RESET (ROT_DMA0_BASE + 0x38)
#define MT6573_ROTDMA1_RESET (ROT_DMA1_BASE + 0x38)
#define MT6573_ROTDMA2_RESET (ROT_DMA2_BASE + 0x38)
#define MT6573_ROTDMA3_RESET (ROT_DMA3_BASE + 0x38)
#define MT6573_RDMA0_RESET (R_DMA0_BASE + 0x38)
#define MT6573_RDMA1_RESET (R_DMA1_BASE + 0x38)
static void ResetDMA(unsigned long a_u4No)
{
    unsigned long u4Addr;
    unsigned long u4TimeOut;
    switch(a_u4No)
    {
        case 0 :
            u4Addr = MT6573_ROTDMA0_RESET;
        break;
        case 1 :
            u4Addr = MT6573_ROTDMA1_RESET;
        break;
        case 2 :
            u4Addr = MT6573_ROTDMA2_RESET;
        break;
        case 3 :
            u4Addr = MT6573_ROTDMA3_RESET;
        break;
        case 4 :
            u4Addr = MT6573_RDMA0_RESET;
        break;
        case 5 :
            u4Addr = MT6573_RDMA1_RESET;
        break;
        default :
        return;
    }

    mt65xx_reg_sync_writel(0x2 , u4Addr);

    u4TimeOut = 0;
    while(0x2 == ioread32(u4Addr))
    {
        u4TimeOut += 1;
        if(u4TimeOut > 50000)
        {
            MDPDB("process is terminated abnormally, so reset DMA but timeout ... DMA%d is dead\n" , (unsigned int)a_u4No);
            return;
        }
    }
    u4TimeOut = 0;

}

#define MT6573_CRZ_CON (CRZ_BASE + 0x4)
#define MT6573_PRZ0_CON (PRZ0_BASE + 0x4)
#define MT6573_PRZ1_CON (PRZ1_BASE + 0x4)
#define MT6573_BRZ_CON (BRZ_BASE)
#define MT6573_VRZ_CON (VRZ_BASE + 0x4)
#define MT6573_IPP_EN (IMGPROC_BASE + 0x320)
#define MT6573_MOUT0_CLR (IMGPROC_BASE + 0x504)
#define MT6573_MOUT1_CLR (IMGPROC_BASE + 0x518)
#define MT6573_MOUT2_CLR (IMGPROC_BASE + 0x52C)
#define MT6573_MOUT3_CLR (IMGPROC_BASE + 0x53C)
#define MT6573_JPGDMA_RESET (JPG_DMA_BASE + 0x8)
static int MT6573_MDP_Flush(struct file * a_pstFile , fl_owner_t a_id)
{
    stMDPOpLog * pstLog;
//TODO : Reset mva pool here~
    pstLog = (stMDPOpLog *)a_pstFile->private_data;

    if(NULL == pstLog)
    {
        MDPDB("Private data is null in flush operation. HOW COULD THIS HAPPEN ??\n");
        return 0;
    }

    if(pstLog->u4OwnResource)
    {
        MDPDB("Resource %lu is not released before PID : %d close this FD \n" , pstLog->u4OwnResource , pstLog->pid);
        //Error handling
        // Step 1 : Turn off engines
        if(ROTDMA0_FLAG & pstLog->u4OwnResource)
        {
            ResetDMA(0);
//            m4u_reset_mva_release_tlb(M4U_CLNTMOD_ROT0);
        }
        if(ROTDMA1_FLAG & pstLog->u4OwnResource)
        {
            ResetDMA(1);
//            m4u_reset_mva_release_tlb(M4U_CLNTMOD_ROT1);
        }
        if(ROTDMA2_FLAG & pstLog->u4OwnResource)
        {
            ResetDMA(2);
//            m4u_reset_mva_release_tlb(M4U_CLNTMOD_ROT2);
        }
        if(RDMA1_PRZ1_ROTDMA3_FLAG & pstLog->u4OwnResource)
        {
            ResetDMA(5);
            mt65xx_reg_sync_writel((1 << 16) , MT6573_PRZ1_CON);
            mt65xx_reg_sync_writel(0 , MT6573_PRZ1_CON);
            ResetDMA(3);
//            m4u_reset_mva_release_tlb(M4U_CLNTMOD_RDMA1);
//            m4u_reset_mva_release_tlb(M4U_CLNTMOD_ROT3);
        }
        if(JPGDMA_FLAG & pstLog->u4OwnResource)
        {
            mt65xx_reg_sync_writel(2 , MT6573_JPGDMA_RESET);
            mt65xx_reg_sync_writel(0 , MT6573_JPGDMA_RESET);
        }
        if(PRZ0_FLAG & pstLog->u4OwnResource)
        {
            mt65xx_reg_sync_writel((1 << 16) , MT6573_PRZ0_CON);
            mt65xx_reg_sync_writel(0 , MT6573_PRZ0_CON);
            mt65xx_reg_sync_writel(1 , MT6573_MOUT1_CLR);
            mt65xx_reg_sync_writel(0 , MT6573_MOUT1_CLR);
        }
        if(VRZ_FLAG & pstLog->u4OwnResource)
        {
            mt65xx_reg_sync_writel((1 << 16) , MT6573_VRZ_CON);
            mt65xx_reg_sync_writel(0 , MT6573_VRZ_CON);
        }
        if(CRZ_IPP_OVL_FLAG & pstLog->u4OwnResource)
        {
            mt65xx_reg_sync_writel((1 << 16) , MT6573_CRZ_CON);
            mt65xx_reg_sync_writel(0 , MT6573_CRZ_CON);
            mt65xx_reg_sync_writel(0 , MT6573_IPP_EN);
            mt65xx_reg_sync_writel((1 << 16) , MT6573_IPP_EN);
            mt65xx_reg_sync_writel(1 , MT6573_MOUT0_CLR);
            mt65xx_reg_sync_writel(0 , MT6573_MOUT0_CLR);
        }
        if(BRZ_FLAG & pstLog->u4OwnResource)
        {
            mt65xx_reg_sync_writel((1 << 16) , MT6573_BRZ_CON);
            mt65xx_reg_sync_writel(0 , MT6573_BRZ_CON);
            mt65xx_reg_sync_writel(1 , MT6573_MOUT3_CLR);
            mt65xx_reg_sync_writel(0 , MT6573_MOUT3_CLR);
        }
        if(RDMA0_FLAG & pstLog->u4OwnResource)
        {
            ResetDMA(4);
//            m4u_reset_mva_release_tlb(M4U_CLNTMOD_RDMA0);
            mt65xx_reg_sync_writel(1 , MT6573_MOUT2_CLR);
            mt65xx_reg_sync_writel(0 , MT6573_MOUT2_CLR);
        }        
        // Step 2 : Release resource, and turn off clock gate
        MT6573_UnLockResource(pstLog->u4OwnResource , pstLog);
    }

    return 0;
}

//return 1 if locked, 0 if not locked.
static int MT6573_TryLockResource(unsigned long au4ResTbl)
{
    unsigned long flags;
    spin_lock_irqsave(&g_stMT6573ResTbl.ResLock , flags);
    if((~g_stMT6573ResTbl.u4FreeTable) & au4ResTbl)
    {
        spin_unlock_irqrestore(&g_stMT6573ResTbl.ResLock , flags);
        return 0;
    }
    else
    {
        g_stMT6573ResTbl.u4FreeTable &= (~au4ResTbl);
    }
    spin_unlock_irqrestore(&g_stMT6573ResTbl.ResLock , flags);

    return 1;
}

// 1 : turn on, 0 : turn off
// This function is protected by g_stMT6573ResTbl.ResLock.
static unsigned long PowerManagement(unsigned long u4TurnOn, unsigned long u4ResTbl , unsigned long u4GMC)
{
    //TODO : Add check busy bit before disabling clock gate
    if(u4TurnOn)
    {
        if((0 == g_stMT6573ResTbl.u4GMCPowerSatus) && (1 == u4GMC))
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC1 , "MDP")){MDPDB("Enable GMC1 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC1E , "MDP")){MDPDB("Enable GMC1E clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_GMC1SLV , "MDP")){MDPDB("Enable GMC1SLV clock failed!\n");}
            g_stMT6573ResTbl.u4GMCPowerSatus = 1;
            //TODO : REMOVE ME
            //MDPDB("Enable GMC clock!!!!!!!!!!\n");
        }

        if(JPGDMA_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_JPEG_DMA , "MDP")){MDPDB("Enable JPGDMA clock failed!\n");}
        }

        if(ROTDMA0_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_ROT_DMA0 , "MDP")){MDPDB("Enable ROTDMA0 clock failed!\n");}
        }

        if(ROTDMA1_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_ROT_DMA1 , "MDP")){MDPDB("Enable ROTDMA1 clock failed!\n");}
        }

        if(ROTDMA2_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_ROT_DMA2 , "MDP")){MDPDB("Enable ROTDMA2 clock failed!\n");}
        }

        if(RDMA1_PRZ1_ROTDMA3_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_ROT_DMA3 , "MDP")){MDPDB("Enable ROTDMA3 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_PRZ1 , "MDP")){MDPDB("Enable PRZ1 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_R_DMA1 , "MDP")){MDPDB("Enable RDMA1 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_R_DMA1_BPS , "MDP")){MDPDB("Enable RDMA1 BPS clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_RESZ_LB , "MDP")){MDPDB("Enable resize line buffer clock failed!\n");}
        }

        if(PRZ0_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_IDP_MOUT1 , "MDP")){MDPDB("Enable MOUT1 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_PRZ0 , "MDP")){MDPDB("Enable PRZ0 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_RESZ_LB , "MDP")){MDPDB("Enable resize line buffer clock failed!\n");}
        }

        if(VRZ_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_VRZ , "MDP")){MDPDB("Enable VRZ clock failed!\n");}
        }

        if(CRZ_IPP_OVL_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_IDP_MOUT0 , "MDP")){MDPDB("Enable MOUT0 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_OVL_DMA, "MDP")){MDPDB("Enable OVL clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_OVL_DMA_BPS, "MDP")){MDPDB("Enable OVLBPS clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_IPP , "MDP")){MDPDB("Enable IPP clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_CRZ , "MDP")){MDPDB("Enable CRZ clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_RESZ_LB , "MDP")){MDPDB("Enable resize line buffer clock failed!\n");}
        }

        if(BRZ_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_IDP_MOUT3 , "MDP")){MDPDB("Enable MOUT3 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_BRZ , "MDP")){MDPDB("Enable BRZ clock failed!\n");}
        }

        if(RDMA0_FLAG & u4ResTbl)
        {
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_IDP_MOUT2 , "MDP")){MDPDB("Enable MOUT2 clock failed!\n");}
            if(TRUE != hwEnableClock(MT65XX_PDN_MM_R_DMA0 , "MDP")){MDPDB("Enable RDMA0 clock failed!\n");}
        }
    }
    else
    {
#if 1
        if(RDMA0_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_R_DMA0 , "MDP")){MDPDB("Disable RDMA0 clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_IDP_MOUT2 , "MDP")){MDPDB("Disable MOUT2 clock failed!\n");}
        }

        if(BRZ_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_BRZ , "MDP")){MDPDB("Disable BRZ clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_IDP_MOUT3 , "MDP")){MDPDB("Disable MOUT3 clock failed!\n");}
        }

        if(CRZ_IPP_OVL_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_RESZ_LB , "MDP")){MDPDB("Disable resize line buffer clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_CRZ , "MDP")){MDPDB("Disable CRZ clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_IPP , "MDP")){MDPDB("Disable IPP clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_OVL_DMA_BPS, "MDP")){MDPDB("Disable OVLBPS clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_OVL_DMA, "MDP")){MDPDB("Disable OVL clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_IDP_MOUT0 , "MDP")){MDPDB("Disable MOUT0 clock failed!\n");}
        }

        if(VRZ_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_VRZ , "MDP")){MDPDB("Disable VRZ clock failed!\n");}
        }

        if(PRZ0_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_RESZ_LB , "MDP")){MDPDB("Disable resize line buffer clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_PRZ0 , "MDP")){MDPDB("Disable PRZ0 clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_IDP_MOUT1 , "MDP")){MDPDB("Disable MOUT1 clock failed!\n");}
        }

        if(JPGDMA_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_JPEG_DMA , "MDP")){MDPDB("Disable JPGDMA clock failed!\n");}
        }

        if(ROTDMA0_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_ROT_DMA0 , "MDP")){MDPDB("Disable ROTDMA0 clock failed!\n");}
        }

        if(ROTDMA1_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_ROT_DMA1 , "MDP")){MDPDB("Disable ROTDMA1 clock failed!\n");}
        }

        if(ROTDMA2_FLAG & u4ResTbl)
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_ROT_DMA2 , "MDP")){MDPDB("Disable ROTDMA2 clock failed!\n");}
        }

        if(RDMA1_PRZ1_ROTDMA3_FLAG & u4ResTbl)
        {

            if(TRUE != hwDisableClock(MT65XX_PDN_MM_RESZ_LB , "MDP")){MDPDB("Disable resize line buffer clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_R_DMA1 , "MDP")){MDPDB("Disable RDMA1 clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_R_DMA1_BPS , "MDP")){MDPDB("Disable RDMA1 BPS clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_PRZ1 , "MDP")){MDPDB("Disable PRZ1 clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_ROT_DMA3 , "MDP")){MDPDB("Disable ROTDMA3 clock failed!\n");}
        }

        if(1 == g_stMT6573ResTbl.u4GMCPowerSatus && (1 == u4GMC))
        {
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC1SLV , "MDP")){MDPDB("Disable GMC1SLV clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC1E , "MDP")){MDPDB("Disable GMC1E clock failed!\n");}
            if(TRUE != hwDisableClock(MT65XX_PDN_MM_GMC1 , "MDP")){MDPDB("Disable GMC1 clock failed!\n");}
            g_stMT6573ResTbl.u4GMCPowerSatus = 0;
            //TODO : REMOVE ME
            //MDPDB("Disable GMC clock!!!!!!!!!!\n");
        }
#endif
    }

    return 0;
}

static unsigned long MsToJiffies(unsigned long u4ms)
{
    return ((u4ms*HZ + 512) >> 10);
}

#define MT6573_ALLFLAG (BRZ_FLAG | RDMA0_FLAG | CRZ_IPP_OVL_FLAG | PRZ0_FLAG | VRZ_FLAG | JPGDMA_FLAG | RDMA1_PRZ1_ROTDMA3_FLAG | ROTDMA0_FLAG | ROTDMA1_FLAG | ROTDMA2_FLAG)
static int MT6573_LockResource(unsigned long a_pParam , stMDPOpLog * a_pstLog)
{
    stLockResParam stParam;
    long iTimeOut;
    unsigned long u4EnableGMC;
    unsigned long flags;

    if(copy_from_user(&stParam, (struct stLockResParam *)a_pParam , sizeof(stLockResParam)))
    {
        MDPDB("[MT6573_MDP] Lock resource, Copy from user failed !!\n");
        return -EFAULT;
    }

    if(0 == stParam.u4LockResTable)
    {
        return 0;
    }

    spin_lock_irqsave(&g_stMT6573ResTbl.ResLock , flags);

//u4EnableGMC is 1 if all resource is free, 0 if any flag is there
    u4EnableGMC = (MT6573_ALLFLAG & (~g_stMT6573ResTbl.u4FreeTable));
    u4EnableGMC = u4EnableGMC > 0 ? 0 : 1;

//REMOVE IT
//MDPMSG("[mt6573_MDP] Lock resource!! FreeTbl:0x%x , OccupiedTbl:0x%x , Timeshare:0x%x , Flag:0x%x , Timeout:%lu\n" , 
//    g_stMT6573ResTbl.u4FreeTable , g_stMT6573ResTbl.u4OccupiedTable , stParam.u4IsTimeShared , stParam.u4LockResTable , stParam.u4TimeOutInms);

    if((~g_stMT6573ResTbl.u4FreeTable) & stParam.u4LockResTable)
    {
        //Cannot get resource rightaway
        //if((g_stMT6573ResTbl.u4OccupiedTable & stParam.u4LockResTable) || (0 == stParam.u4TimeOutInms))
        if(0 == stParam.u4TimeOutInms)
        {
//REMOVE IT
MDPMSG("[mt6573_MDP] No resource !! FreeTbl:0x%x , OccupiedTbl:0x%x , Timeshare:0x%x , Flag:0x%x , Timeout:%lu\n" , 
    (unsigned int)g_stMT6573ResTbl.u4FreeTable , (unsigned int)g_stMT6573ResTbl.u4OccupiedTable , (unsigned int)stParam.u4IsTimeShared , (unsigned int)stParam.u4LockResTable , stParam.u4TimeOutInms);

            //The required resource is in occupied table, not worth to wait.
            spin_unlock_irqrestore(&g_stMT6573ResTbl.ResLock , flags);
            return -EBUSY;
        }

        spin_unlock_irqrestore(&g_stMT6573ResTbl.ResLock , flags);
        iTimeOut = wait_event_interruptible_timeout(g_MT6573MDPResWaitQueue, MT6573_TryLockResource(stParam.u4LockResTable) , MsToJiffies(stParam.u4TimeOutInms));
        if(0 == iTimeOut)
        {
            MDPMSG("[mt6573_MDP] Lock resource time out , FreeTbl:0x%x , Timeshare:0x%x , Flag:0x%x , Timeout:%lu\n",
            (unsigned int)g_stMT6573ResTbl.u4FreeTable , (unsigned int)stParam.u4IsTimeShared , (unsigned int)stParam.u4LockResTable , stParam.u4TimeOutInms);
            return -EAGAIN;
        }

        //Lock sucess
        a_pstLog->u4OwnResource |= stParam.u4LockResTable;

        PowerManagement(1 , stParam.u4LockResTable , u4EnableGMC);

        return 0;
    }
    else
    {
        g_stMT6573ResTbl.u4FreeTable &= (~stParam.u4LockResTable);

        if(!stParam.u4IsTimeShared)
        {
            g_stMT6573ResTbl.u4OccupiedTable |= stParam.u4LockResTable;
        }

        //Lock sucess
        a_pstLog->u4OwnResource |= stParam.u4LockResTable;

        PowerManagement(1 , stParam.u4LockResTable , u4EnableGMC);
    }

//REMOVE IT
//MDPMSG("[mt6573_MDP] Lock success!! FreeTbl:0x%x , OccupiedTbl:0x%x , Flag:0x%x\n" , 
//    g_stMT6573ResTbl.u4FreeTable , g_stMT6573ResTbl.u4OccupiedTable , stParam.u4LockResTable);

    spin_unlock_irqrestore(&g_stMT6573ResTbl.ResLock , flags);

    return 0;
}

static int MT6573_UnLockResource(unsigned long a_u4ResTbl , stMDPOpLog * a_pstLog)
{
    unsigned long u4EnableGMC;
    unsigned long flags;

    if(0 == a_u4ResTbl)
    {
        return 0;
    }

    spin_lock_irqsave(&g_stMT6573ResTbl.ResLock , flags);

//REMOVE IT
//MDPMSG("[mt6573_MDP] Unlock resource!! FreeTbl:0x%x , OccupiedTbl:0x%x , Flag:0x%x\n" , g_stMT6573ResTbl.u4FreeTable , g_stMT6573ResTbl.u4OccupiedTable , a_u4ResTbl);
//u4EnableGMC is 1 if there is no more resources, 0 if any flag is still there.
    u4EnableGMC = (MT6573_ALLFLAG & (~(g_stMT6573ResTbl.u4FreeTable | a_u4ResTbl)));
    u4EnableGMC = u4EnableGMC > 0 ? 0 : 1;

    PowerManagement(0 , (a_u4ResTbl & (~g_stMT6573ResTbl.u4FreeTable)) , u4EnableGMC);

    g_stMT6573ResTbl.u4FreeTable |= a_u4ResTbl;

    g_stMT6573ResTbl.u4OccupiedTable &= (~a_u4ResTbl);

    a_pstLog->u4OwnResource &= (~a_u4ResTbl);

    wake_up_interruptible(&g_MT6573MDPResWaitQueue);

//MDPMSG("[mt6573_MDP] UnLock success!! FreeTbl:0x%x , OccupiedTbl:0x%x , Flag:0x%x\n" , 
//    g_stMT6573ResTbl.u4FreeTable , g_stMT6573ResTbl.u4OccupiedTable , a_u4ResTbl);

    spin_unlock_irqrestore(&g_stMT6573ResTbl.ResLock , flags);

    return 0;
}

static int MT6573_WAIT_IRQ(unsigned long a_pParam)
{
    stWaitIrqParam stParam;
    long iTimeOut;
    unsigned long flags;

    if(copy_from_user(&stParam, (struct stWaitIrqParam *)a_pParam , sizeof(stWaitIrqParam))){
        MDPDB("[MT6573_MDP] Wait IRQ, Copy from user failed !!\n");
        return -EFAULT;
    }

//REMOVE IT
//MDPMSG("[mt6573_MDP] wait interrupt!! Tbl:0x%x , Flag:0x%x , TimeoutMS:%lu, TimeoutJF:%lu\n" , g_stMT6573IRQTbl.u4Table , stParam.u4IrqNo , stParam.u4TimeOutInms , MsToJiffies(stParam.u4TimeOutInms));

    iTimeOut = wait_event_interruptible_timeout(g_MT6573MDPIRQWaitQueue, (g_stMT6573IRQTbl.u4Table & stParam.u4IrqNo) , MsToJiffies(stParam.u4TimeOutInms));

    spin_lock_irqsave(&g_stMT6573IRQTbl.ResLock , flags);

    if(0 == iTimeOut)
    {
        MDPDB("[MT6573_MDP] PID : %d Wait IRQ time out !!Tbl : 0x%x , Flag : 0x%x , timeout : %lu\n" , current->pid , (unsigned int)g_stMT6573IRQTbl.u4Table , (unsigned int)stParam.u4IrqNo , stParam.u4TimeOutInms);
        MT6573MDP_DUMPREG();
        spin_unlock_irqrestore(&g_stMT6573IRQTbl.ResLock , flags);
        return -EAGAIN;
    }

    g_stMT6573IRQTbl.u4Table &= (~stParam.u4IrqNo);
//REMOVE IT
//MDPMSG("[mt6573_MDP] wait interrupt done!! Tbl:0x%x\n" , g_stMT6573IRQTbl.u4Table);

    spin_unlock_irqrestore(&g_stMT6573IRQTbl.ResLock , flags);

    return 0;
}

//ROT_DMA0_BASE
#define MT6573_ROTDMA0_EXEC_CNT (ROT_DMA0_BASE + 0x80)
#define MT6573_ROTDMA0_DROP_CNT (ROT_DMA0_BASE + 0x88)
#define MT6573_ROTDMA1_EXEC_CNT (ROT_DMA1_BASE + 0x80)
#define MT6573_ROTDMA2_EXEC_CNT (ROT_DMA2_BASE + 0x80)
#define MT6573_ROTDMA3_EXEC_CNT (ROT_DMA3_BASE + 0x80)
static int MT6573_CLR_IRQ(unsigned long a_u4IRQFlag)
{
    unsigned long flag1 , flag2;
    spin_lock_irqsave(&g_stMT6573IRQTbl.ResLock , flag1);

    switch(a_u4IRQFlag)
    {
        case ROTDMA0_FLAG :
            u4MT6573RotExecCnt[0] = ioread32(MT6573_ROTDMA0_EXEC_CNT);
            spin_lock_irqsave(&g_stMT6573ZoomReg.ResLock , flag2);
            g_stMT6573ZoomReg.u4Dirty = 0;
            spin_unlock_irqrestore(&g_stMT6573ZoomReg.ResLock , flag2);
    	 break;
        case ROTDMA1_FLAG :
            u4MT6573RotExecCnt[1] = ioread32(MT6573_ROTDMA1_EXEC_CNT);
    	 break;
        case ROTDMA2_FLAG :
            u4MT6573RotExecCnt[2] = ioread32(MT6573_ROTDMA2_EXEC_CNT);
    	 break;
        case RDMA1_PRZ1_ROTDMA3_FLAG :
            u4MT6573RotExecCnt[3] = ioread32(MT6573_ROTDMA3_EXEC_CNT);
    	 break;
    	 default :
    	 break;
    }

//REMOVE IT
//MDPMSG("[mt6573_MDP] clear interrupt!! Tbl:0x%x , Flag:0x%x\n" , g_stMT6573IRQTbl.u4Table , a_u4IRQFlag);

    g_stMT6573IRQTbl.u4Table &= (~a_u4IRQFlag);

    spin_unlock_irqrestore(&g_stMT6573IRQTbl.ResLock , flag1);

    return 0;
}

static inline int MT6573ROTDMA0_TimeStamp(unsigned long a_u4Cmd , stTimeStamp * a_u4Param);

extern void get_pmem_range(unsigned long * pu4StartAddr, unsigned long * pu4Size);
static inline int MT6573_CheckPmemRange(stPMEMRange * a_u4Param)
{
    unsigned long u4Addr, u4Size;
    stPMEMRange input;
    unsigned int pageOffset;
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;

    if(copy_from_user(&input , (struct stPMEMRange *)a_u4Param , sizeof(stPMEMRange)))
    {
        MDPDB("MDP check pmem range copy from user failed!!\n");
        return -EFAULT;
    }    

    get_pmem_range(&u4Addr , &u4Size);

    //virtual to physical
//printk("Virtual addr : 0x%x , size : %lu\n" , input.u4StartAddr , input.u4Size);
    pageOffset = (input.u4StartAddr & (PAGE_SIZE - 1));
    pgd = pgd_offset(current->mm, input.u4StartAddr);
    if(pgd_none(*pgd)||pgd_bad(*pgd))
    {
        MDPDB("warning: upper layer pass address va=0x%x, pgd invalid! \n", (unsigned int)input.u4StartAddr);
        input.u4Result = 2;
        goto out;
    }

    pmd = pmd_offset(pgd, input.u4StartAddr);
    if(pmd_none(*pmd)||pmd_bad(*pmd))
    {
        MDPDB("warning: upper layer pass address va=0x%x, pmd invalid! \n", (unsigned int)input.u4StartAddr);
        input.u4Result = 2;
        goto out;
    }
        
    pte = pte_offset_map(pmd, input.u4StartAddr);
    if(pte_present(*pte)) 
    { 
        input.u4StartAddr = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;
    }     

    if((input.u4StartAddr >= u4Addr) && (input.u4StartAddr < (u4Addr + u4Size)))
    {
        //inside of pmem
        if((input.u4StartAddr + input.u4Size) <= (u4Addr + u4Size))
        {
            input.u4Result = 1;
        }
        else
        {
            input.u4Result = 2;
            MDPDB("physical address : 0x%x, size : %lu , pmem addr : 0x%x , pmem size : %lu\n" , (unsigned int)input.u4StartAddr , input.u4Size , (unsigned int)u4Addr , u4Size);
        }
    }
    else
    {
        //outside of pmem
        input.u4Result = 0;
    }
//printk("Physical : 0x%x , result : %d\n" , input.u4StartAddr , input.u4Result);
out:
    if(copy_to_user((void __user *) a_u4Param , &input , sizeof(stPMEMRange)))
    {
        MDPDB("MDP get pmem range copy to user failed!!\n");
        return -EFAULT;
    }

    return 0;
}

static int MT6573_SetZoom(stZoomSetting * a_u4Param)
{
    unsigned long u4Flags;

    spin_lock_irqsave(&g_stMT6573ZoomReg.ResLock , u4Flags);
    g_stMT6573ZoomReg.u4Dirty = 0;
    spin_unlock_irqrestore(&g_stMT6573ZoomReg.ResLock , u4Flags);

    if(copy_from_user(&g_stMT6573ZoomReg.stReg , (struct stZoomSetting *)a_u4Param , sizeof(stZoomSetting)))
    {
        MDPDB("MDP set zoom copy from user failed!!\n");
        return -EFAULT;
    }    

    spin_lock_irqsave(&g_stMT6573ZoomReg.ResLock , u4Flags);
    g_stMT6573ZoomReg.u4Dirty = 1;
    spin_unlock_irqrestore(&g_stMT6573ZoomReg.ResLock , u4Flags);

    return 0;
}

#define MT6573_RMMU0_RESET (ROT_DMA0_BASE + 0x820)
#define MT6573_RMMU1_RESET (ROT_DMA1_BASE + 0x820)
#define MT6573_RMMU2_RESET (ROT_DMA2_BASE + 0x820)
#define MT6573_RMMU3_RESET (ROT_DMA3_BASE + 0x820)
//Reset RMMU->reset ROTDMA->deassert ROTDMA-> deassert RMMU
//Notice , this function is ued to recover rotdma.
static int MT6573_HWReset(unsigned long a_u4RotDMANo)
{
    unsigned long u4ROTDMAAddr;
    unsigned long u4RMMUAddr;
    unsigned long u4TimeOut;
    switch(a_u4RotDMANo)
    {
        case 0 :
            u4ROTDMAAddr = MT6573_ROTDMA0_RESET;
            u4RMMUAddr = MT6573_RMMU0_RESET;
        break;
        case 1 :
            u4ROTDMAAddr = MT6573_ROTDMA1_RESET;
            u4RMMUAddr = MT6573_RMMU1_RESET;
        break;
        case 2 :
            u4ROTDMAAddr = MT6573_ROTDMA2_RESET;
            u4RMMUAddr = MT6573_RMMU2_RESET;
        break;
        case 3 :
            u4ROTDMAAddr = MT6573_ROTDMA3_RESET;
            u4RMMUAddr = MT6573_RMMU3_RESET;
        break;
        default :
        return 0;
    }

    mt65xx_reg_sync_writel(0x1 , u4RMMUAddr);
    mt65xx_reg_sync_writel(0x1 , u4ROTDMAAddr);
    //50 cycles
    for(u4TimeOut = 0 ; u4TimeOut < 50 ; u4TimeOut += 1){;}
    
    mt65xx_reg_sync_writel(0 , u4ROTDMAAddr);
    mt65xx_reg_sync_writel(0 , u4RMMUAddr);

    return 0;
}

static int MT6573_MDP_Ioctl(struct inode * a_pstInode,
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    int i4RetVal = 0;

    stMDPOpLog * pstLog;

    pstLog = (stMDPOpLog *)a_pstFile->private_data;

    if(NULL == pstLog)
    {
        MDPDB("Private data is null in ioctl operation. HOW COULD THIS HAPPEN ??\n");
        return -1;
    }

    switch(a_u4Command)
    {
        case MT6573MDP_T_LOCKRESOURCE :
            i4RetVal = MT6573_LockResource(a_u4Param , pstLog);

        break;

        case MT6573MDP_T_UNLOCKRESOURCE :
            i4RetVal = MT6573_UnLockResource(a_u4Param , pstLog);
        break;

        case MT6573MDP_X_WAITIRQ :
            i4RetVal = MT6573_WAIT_IRQ(a_u4Param);
        break;

        case MT6573MDP_T_CLRIRQ :
            MT6573_CLR_IRQ(a_u4Param);
        break;

        case MT6573MDP_T_DUMPREG :
            MT6573MDP_DUMPREG();
        break;

        case MT6573MDP_G_TIMESTAMP :
            i4RetVal = MT6573ROTDMA0_TimeStamp(1 , (stTimeStamp *)a_u4Param);
        break;

        case MT6573MDP_G_PMEMRANGE :
            i4RetVal = MT6573_CheckPmemRange((stPMEMRange *)a_u4Param);
        break;

        case MT6573MDP_T_SETZOOM :
            i4RetVal = MT6573_SetZoom((stZoomSetting *)a_u4Param);
        break;

        case MT6573MDP_T_HWRESET :
            i4RetVal = MT6573_HWReset(a_u4Param);
        break;

        default :
            MDPDB("MT6573 MDP ioctl : No such command!!\n");
            i4RetVal = -EINVAL;
        break;
    }

    return i4RetVal;
}

static int MT6573_MDP_unlocked_ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    return MT6573_MDP_Ioctl(a_pstFile->f_dentry->d_inode , a_pstFile , a_u4Command , a_u4Param);
}

static int MT6573_MDP_mmap(struct file * a_pstFile, struct vm_area_struct * a_pstVMArea)
{
    stMDPOpLog * pstLog;

    pstLog = (stMDPOpLog *)a_pstFile->private_data;

    if(NULL == pstLog)
    {
        MDPDB("Private data is null in mmap operation. HOW COULD THIS HAPPEN ??\n");
        return -1;
    }

    //TODO : check registers to be mapped is locked by this fd.
    //TODO : check if physical address inside of registers.
//REMOVE IT
//MDPMSG("[mt6573_MDP] mmap!! \n");

    a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);

    if(remap_pfn_range(a_pstVMArea , a_pstVMArea->vm_start , a_pstVMArea->vm_pgoff , 
	(a_pstVMArea->vm_end - a_pstVMArea->vm_start) , a_pstVMArea->vm_page_prot))
    {
        MDPDB("MMAP failed!!\n");
        return -1;
    }

    return 0;
}

static const struct file_operations g_stMT6573_MDP_fops = 
{
	.owner = THIS_MODULE,
	.open = MT6573_MDP_Open,
	.release = MT6573_MDP_Release,
	.flush = MT6573_MDP_Flush,
//	.ioctl = MT6573_MDP_Ioctl,
	.unlocked_ioctl = MT6573_MDP_unlocked_ioctl,
	.mmap = MT6573_MDP_mmap
};

#define MT6573_CRZ_INT (CRZ_BASE + 0xC)
#define MT6573_CRZ_LOCK (CRZ_BASE + 0x20)
#define MT6573_CRZ_CFG (CRZ_BASE)
#define MT6573_CRZ_SRCSZ (CRZ_BASE + 0x10)
#define MT6573_CRZ_CROPLR (CRZ_BASE + 0xF4)
#define MT6573_CRZ_CROPTB (CRZ_BASE + 0xF8)
#define MT6573_CRZ_HRATIO (CRZ_BASE + 0x18)
#define MT6573_CRZ_VRATIO (CRZ_BASE + 0x1C)
#define ABS(a , b) (a > b ? (a-b) : (b-a))
static __tcmfunc void MT6573CRZ_ISR(void)
{
    unsigned long u4RegVal = 0;
    u4RegVal = ioread32(MT6573_CRZ_INT);

    if(0x1 & u4RegVal)
    {
        g_stMT6573IRQTbl.u4Table |= CRZ_IPP_OVL_FLAG;//Frame end interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] CRZ int : frameend \n");
    }

    if(0x2 & u4RegVal)
    {
        g_stMT6573ZoomReg.u4FrameCounter += 1;
        if(1 == g_stMT6573ZoomReg.u4Dirty && (1 < ABS(g_stMT6573ZoomReg.u4FrameCounter , g_stMT6573ZoomReg.u4AppliedFrame)))
        {
            //Frame start
            mt65xx_reg_sync_writel(1 , MT6573_CRZ_LOCK);
            mt65xx_reg_sync_writel(g_stMT6573ZoomReg.stReg.u4CFG , MT6573_CRZ_CFG);
            mt65xx_reg_sync_writel(g_stMT6573ZoomReg.stReg.u4SRCSZ , MT6573_CRZ_SRCSZ);
            mt65xx_reg_sync_writel(g_stMT6573ZoomReg.stReg.u4CROPLR , MT6573_CRZ_CROPLR);
            mt65xx_reg_sync_writel(g_stMT6573ZoomReg.stReg.u4CROPTB , MT6573_CRZ_CROPTB);
            mt65xx_reg_sync_writel(g_stMT6573ZoomReg.stReg.u4HRATIO , MT6573_CRZ_HRATIO);
            mt65xx_reg_sync_writel(g_stMT6573ZoomReg.stReg.u4VRATIO , MT6573_CRZ_VRATIO);
            mt65xx_reg_sync_writel(0 , MT6573_CRZ_LOCK);
            g_stMT6573ZoomReg.u4AppliedFrame = g_stMT6573ZoomReg.u4FrameCounter;
            g_stMT6573ZoomReg.u4Dirty = 0;
//            MDPDB("CRZ Set zoom!!\n");        
        }
    }

    if(0x10 & u4RegVal)
    {
        MDPDB("CRZ input pixel is not enough!!\n");
        //Slow down ROTDMA to make isp overrun happen.
        if(ioread32((ROT_DMA0_BASE + 0x18)) >> 31)
        {
            MDPDB("Slow down ROTDMA0!!\n");
            mt65xx_reg_sync_writel(((0xFFFF << 16) | 0x1) , (ROT_DMA0_BASE + 0x300));
        }
        else if(ioread32((ROT_DMA1_BASE + 0x18)) >> 31)
        {
            MDPDB("Slow down ROTDMA1!!\n");
            mt65xx_reg_sync_writel(((0xFFFF << 16) | 0x1) , (ROT_DMA1_BASE + 0x300));
        }
        else if(ioread32((ROT_DMA2_BASE + 0x18)) >> 31)
        {
            MDPDB("Slow down ROTDMA2!!\n");
            mt65xx_reg_sync_writel(((0xFFFF << 16) | 0x1) , (ROT_DMA2_BASE + 0x300));
        }
        MDPDB("slowdown ROTDMA done\n");
    }

    if(0x20 & u4RegVal)
    {
        MDPDB("CRZ Output too slow!!\n");
    }

    //Double buffer
}

#define MT6573_RDMA0_IRQ_FLAG (R_DMA0_BASE)
#define MT6573_RDMA0_IRQ_FLAG_CLR (R_DMA0_BASE + 0x8)
static __tcmfunc void MT6573RDMA_ISR(void)
{
    unsigned long u4RegVal = 0;
    u4RegVal = ioread32(MT6573_RDMA0_IRQ_FLAG);
    u4RegVal &= 0x23;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            g_stMT6573IRQTbl.u4Table |= RDMA0_FLAG;//complete interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] RDMA0 int : done \n");
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("RDMA0 Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to RDMA0 descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_RDMA0_IRQ_FLAG_CLR);//clear
    }

}

#define MT6573_ROTDMA0_IRQ_FLAG (ROT_DMA0_BASE)
#define MT6573_ROTDMA0_IRQ_FLAG_CLR (ROT_DMA0_BASE + 0x8)
#define MT6573_ROTDMA0_IRQ_FLAG_CFG (ROT_DMA0_BASE + 0x18)
#define MT6573_ROTDMA0_QUEUE_RSTA (ROT_DMA0_BASE + 0x40)
#define MT6573_ROTDMA0_RMMU_CON (ROT_DMA0_BASE + 0x800)
#define MT6573_ROTDMA0_RMMU_IRQ_FLAG (ROT_DMA0_BASE + 0x830)
#define MT6573_ROTDMA0_RMMU_IRQ_CLR (ROT_DMA0_BASE + 0x834)
#define MT6573_ROTDMA1_IRQ_FLAG (ROT_DMA1_BASE)
#define MT6573_ROTDMA1_IRQ_FLAG_CLR (ROT_DMA1_BASE + 0x8)
#define MT6573_ROTDMA1_RMMU_CON (ROT_DMA1_BASE + 0x800)
#define MT6573_ROTDMA1_RMMU_IRQ_FLAG (ROT_DMA1_BASE + 0x830)
#define MT6573_ROTDMA1_RMMU_IRQ_CLR (ROT_DMA1_BASE + 0x834)
#define MT6573_ROTDMA2_IRQ_FLAG (ROT_DMA2_BASE)
#define MT6573_ROTDMA2_IRQ_FLAG_CLR (ROT_DMA2_BASE + 0x8)
#define MT6573_ROTDMA2_RMMU_CON (ROT_DMA2_BASE + 0x800)
#define MT6573_ROTDMA2_RMMU_IRQ_FLAG (ROT_DMA2_BASE + 0x830)
#define MT6573_ROTDMA2_RMMU_IRQ_CLR (ROT_DMA2_BASE + 0x834)
//a_u4Cmd : 0 print time stamp, 1 : get time stamp
static inline int MT6573ROTDMA0_TimeStamp(unsigned long a_u4Cmd , stTimeStamp * a_u4Param)
{
    struct timeval tv;
    unsigned long u4Index = 0;
    unsigned long u4RegVal;
    unsigned long u4BuffCnt = ioread32(MT6573_ROTDMA0_IRQ_FLAG_CFG);
    static stTimeStamp TimeStamp;

    u4BuffCnt = ((u4BuffCnt >> 8 ) & 0xF);

    if(0 == a_u4Cmd)
    {
        do_gettimeofday(&tv);
        u4RegVal = ioread32(MT6573_ROTDMA0_QUEUE_RSTA);
        u4Index = ((0xF00 & u4RegVal) >> 8);
        u4Index = ((u4Index > 0) ? (u4Index - 1) : u4BuffCnt);
        TimeStamp.u4Sec[u4Index] = tv.tv_sec;
        TimeStamp.u4MicroSec[u4Index] = tv.tv_usec;
    }
    else
    {
        if(copy_to_user((void __user *) a_u4Param , &TimeStamp , sizeof(stTimeStamp)))
        {
            MDPDB("MDP time stamp copy to user failed!!\n");
            return -1;
        }
    }
    return 0;
}


static __tcmfunc void MT6573ROTDMA_ISR(void)
{
    unsigned long u4RegVal = 0;
    unsigned int u4DebugCounter = 0;
    //ROTDMA0
    u4RegVal = ioread32(MT6573_ROTDMA0_IRQ_FLAG);
    u4RegVal &= 0x33;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            if(u4MT6573RotExecCnt[0] != ioread32(MT6573_ROTDMA0_EXEC_CNT))
            {
                u4MT6573RotExecCnt[0] = ioread32(MT6573_ROTDMA0_EXEC_CNT);

                g_stMT6573IRQTbl.u4Table |= ROTDMA0_FLAG;//complete interrupt

                MT6573ROTDMA0_TimeStamp(0 , NULL);
                //MDPDB("ROT0 flag exec\n");
            }
            else
            {
                u4DebugCounter = ioread32(MT6573_ROTDMA0_DROP_CNT);
                //OSS3 request not to print per frame even this condition is incorrect!!
                if((50 > u4DebugCounter) || ((1000 < u4DebugCounter) && (0 == (u4DebugCounter & 0x3FF))))
                MDPDB("ROTDMA0 totally drop %d frame\n" , u4DebugCounter);
            }
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA0 Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to ROTDMA0 descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA0_IRQ_FLAG_CLR);//clear
    }

    //ROTDMA0 RMMU
    u4RegVal = ioread32(MT6573_ROTDMA0_RMMU_IRQ_FLAG);
    u4RegVal &= 0x3;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            MDPDB("ROTDMA0 RMMU translate fault!!\n");
            MT6573MDP_DUMPREG();
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA0 RMMU physcial address fault!!\n");
            MT6573MDP_DUMPREG();
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA0_RMMU_IRQ_CLR);//clear rmmu int
    }

    //ROTDMA1
    u4RegVal = ioread32(MT6573_ROTDMA1_IRQ_FLAG);
    u4RegVal &= 0x33;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            if(u4MT6573RotExecCnt[1] != ioread32(MT6573_ROTDMA1_EXEC_CNT))
            {
                u4MT6573RotExecCnt[1] = ioread32(MT6573_ROTDMA1_EXEC_CNT);

                g_stMT6573IRQTbl.u4Table |= ROTDMA1_FLAG;//complete interrupt
            }
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA1 Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to ROTDMA1 descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA1_IRQ_FLAG_CLR);//clear
    }

    //ROTDMA1 RMMU
    u4RegVal = ioread32(MT6573_ROTDMA1_RMMU_IRQ_FLAG);
    u4RegVal &= 0x3;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            MDPDB("ROTDMA1 RMMU translate fault!!\n");
            MT6573MDP_DUMPREG();
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA1 RMMU physcial address fault!!\n");
            MT6573MDP_DUMPREG();
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA1_RMMU_IRQ_CLR);//clear rmmu int
    }

    //ROTDMA2
    u4RegVal = ioread32(MT6573_ROTDMA2_IRQ_FLAG);
    u4RegVal &= 0x33;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            if(u4MT6573RotExecCnt[2] != ioread32(MT6573_ROTDMA2_EXEC_CNT))
            {
                u4MT6573RotExecCnt[2] = ioread32(MT6573_ROTDMA2_EXEC_CNT);

                g_stMT6573IRQTbl.u4Table |= ROTDMA2_FLAG;//complete interrupt
            }
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA2 Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to ROTDMA2 descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA2_IRQ_FLAG_CLR);//clear
    }

    //ROTDMA2 RMMU
    u4RegVal = ioread32(MT6573_ROTDMA2_RMMU_IRQ_FLAG);
    u4RegVal &= 0x3;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            MDPDB("ROTDMA2 RMMU translate fault!!\n");
            MT6573MDP_DUMPREG();
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA2 RMMU physcial address fault!!\n");
            MT6573MDP_DUMPREG();
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA2_RMMU_IRQ_CLR);//clear rmmu int
    }

}

//BRZ,VRZ,PRZ0
#define MT6573_BRZ_INT (BRZ_BASE + 0x8)
#define MT6573_VRZ_INT (VRZ_BASE + 0xC)
#define MT6573_PRZ0_INT (PRZ0_BASE + 0xC)
#define MT6573_PRZ1_INT (PRZ1_BASE + 0xC)
static __tcmfunc void MT6573RZ_ISR(void)
{
    unsigned long u4RegVal = 0;
    u4RegVal = ioread32(MT6573_BRZ_INT);
    u4RegVal &= 0xC;
    if(u4RegVal)
    {
        g_stMT6573IRQTbl.u4Table |= BRZ_FLAG;//complete interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] BRZ int : done \n");
        mt65xx_reg_sync_writel((u4RegVal | 0x3),MT6573_BRZ_INT);
    }

    u4RegVal = ioread32(MT6573_VRZ_INT);
    if(0x1 & u4RegVal)
    {
        g_stMT6573IRQTbl.u4Table |= VRZ_FLAG;//complete interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] VRZ int : done \n");
    }

    //PRZ0
    u4RegVal = ioread32(MT6573_PRZ0_INT);
    if(0x1 & u4RegVal)
    {
        g_stMT6573IRQTbl.u4Table |= PRZ0_FLAG;//frame end interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] PRZ0 int : frameend \n");
    }

    //PRZ0 double buffer
}

#define MT6573_RDMA1_IRQ_FLAG (R_DMA1_BASE)
#define MT6573_RDMA1_IRQ_FLAG_CLR (R_DMA1_BASE + 0x8)
#define MT6573_PRZ1_INT (PRZ1_BASE + 0xC)
#define MT6573_ROTDMA3_IRQ_FLAG (ROT_DMA3_BASE)
#define MT6573_ROTDMA3_IRQ_FLAG_CLR (ROT_DMA3_BASE + 0x8)
#define MT6573_ROTDMA3_RMMU_CON (ROT_DMA3_BASE + 0x800)
#define MT6573_ROTDMA3_RMMU_IRQ_FLAG (ROT_DMA3_BASE + 0x830)
#define MT6573_ROTDMA3_RMMU_IRQ_CLR (ROT_DMA3_BASE + 0x834)
static __tcmfunc void MT6573RDMA1PRZ1ROTDMA3_ISR(void)
{
    unsigned long u4RegVal = 0;

    u4RegVal = ioread32(MT6573_ROTDMA3_IRQ_FLAG);
    u4RegVal &= 0x33;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            if(u4MT6573RotExecCnt[3] != ioread32(MT6573_ROTDMA3_EXEC_CNT))
            {
                u4MT6573RotExecCnt[3] = ioread32(MT6573_ROTDMA3_EXEC_CNT);

                g_stMT6573IRQTbl.u4Table |= RDMA1_PRZ1_ROTDMA3_FLAG;//complete interrupt
            }
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA3 Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to ROTDMA3 descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA3_IRQ_FLAG_CLR);//clear
    }


    //ROTDMA3 RMMU
    u4RegVal = ioread32(MT6573_ROTDMA3_RMMU_IRQ_FLAG);
    u4RegVal &= 0x3;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
            MDPDB("ROTDMA3 RMMU translate fault!!\n");
            MT6573MDP_DUMPREG();
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("ROTDMA3 RMMU physcial address fault!!\n");
            MT6573MDP_DUMPREG();
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_ROTDMA3_RMMU_IRQ_CLR);//clear rmmu int
    }

    //RDMA1
    u4RegVal = ioread32(MT6573_RDMA1_IRQ_FLAG);
    u4RegVal &= 0x23;
    if(u4RegVal)
    {
        if(0x2 & u4RegVal)
        {
            MDPDB("RDMA1 Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to RDMA1 descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_RDMA1_IRQ_FLAG_CLR);//clear
    }

    //PRZ1
    u4RegVal = ioread32(MT6573_PRZ1_INT);
}

#define MT6573_OVLJPGDMA_IRQ_LINE 139
#define MT6573_OVL_IRQ_FLAG (OVL_BASE)
#define MT6573_OVL_IRQ_FLAG_CLR (OVL_BASE + 0x8)
#define MT6573_JPGDMA_INTERRUPT (JPG_DMA_BASE + 0x20)
static __tcmfunc void MT6573OVLJPGDMA_ISR(void)
{
    unsigned long u4RegVal = 0;

    u4RegVal = ioread32(MT6573_OVL_IRQ_FLAG);
    u4RegVal &= 0x33;
    if(u4RegVal)
    {
        if(0x1 & u4RegVal)
        {
//        g_stMT6573IRQTbl.u4Table |= OVL_FLAG;//complete interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] OVL int : done \n");
        }
        if(0x2 & u4RegVal)
        {
            MDPDB("OVL Invalid descriptor mode\n");
        }
        if(0x20 & u4RegVal)
        {
            MDPDB("SW write to OVL descriptor queue when writer is busy\n");
        }
        mt65xx_reg_sync_writel(u4RegVal , MT6573_OVL_IRQ_FLAG_CLR);//clear
    }

    u4RegVal = ioread32(MT6573_JPGDMA_INTERRUPT);
    u4RegVal &= 0x1;
    if(u4RegVal)
    {
        g_stMT6573IRQTbl.u4Table |= JPGDMA_FLAG;//complete interrupt
//REMOVE IT
//MDPMSG("[mt6573_MDP] JPGDMA int : done \n");
        mt65xx_reg_sync_writel(0 , MT6573_JPGDMA_INTERRUPT);//clear
    }

}

#define MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE 138
static __tcmfunc irqreturn_t MT6573MDP_isr(int irq, void *dev_id)
{
    mt6573_irq_mask(MT6573_OVLJPGDMA_IRQ_LINE);
    mt6573_irq_mask(MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE);
    mt6573_irq_mask(MT6573_CRZ_IRQ_LINE);
    mt6573_irq_mask(MT6573_ROT_DMA_IRQ_LINE);
    mt6573_irq_mask(MT6573_R_DMA_IRQ_LINE);
    mt6573_irq_mask(MT6573_RESZ_IRQ_LINE);
//REMOVE IT
//MDPDB("Comes an interrupt %d!!\n" , irq);
    switch(irq)
    {
        case MT6573_OVLJPGDMA_IRQ_LINE :
            MT6573OVLJPGDMA_ISR();
        break;
        case MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE :
            MT6573RDMA1PRZ1ROTDMA3_ISR();
        break;
        case MT6573_CRZ_IRQ_LINE :
            MT6573CRZ_ISR();
        break;
        case MT6573_ROT_DMA_IRQ_LINE:
            MT6573ROTDMA_ISR();
        break;
        case MT6573_R_DMA_IRQ_LINE :
            MT6573RDMA_ISR();
        break;
        case MT6573_RESZ_IRQ_LINE :
            MT6573RZ_ISR();
        break;
        default :
            MDPDB("Whose ISR : %d ?? Something wrong in registering ISR!!\n" , irq);
        break;
    }

    wake_up_interruptible(&g_MT6573MDPIRQWaitQueue);

    mt6573_irq_unmask(MT6573_OVLJPGDMA_IRQ_LINE);
    mt6573_irq_unmask(MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE);
    mt6573_irq_unmask(MT6573_CRZ_IRQ_LINE);
    mt6573_irq_unmask(MT6573_ROT_DMA_IRQ_LINE);
    mt6573_irq_unmask(MT6573_R_DMA_IRQ_LINE);
    mt6573_irq_unmask(MT6573_RESZ_IRQ_LINE);    

    return IRQ_HANDLED;
}

static struct cdev * g_pMT6573MDP_CharDrv = NULL;
static dev_t g_MT6573MDPdevno = MKDEV(MT6573_MDP_DEV_MAJOR_NUMBER,0);
static inline int RegisterMT6573MDPCharDrv(void)
{
    if( alloc_chrdev_region(&g_MT6573MDPdevno, 0, 1,"mt6573-MDP") )
    {
        MDPDB("[mt6573_MDP] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pMT6573MDP_CharDrv = cdev_alloc();

    if(NULL == g_pMT6573MDP_CharDrv)
    {
        unregister_chrdev_region(g_MT6573MDPdevno, 1);

        MDPDB("[mt6573_MDP] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pMT6573MDP_CharDrv, &g_stMT6573_MDP_fops);

    g_pMT6573MDP_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pMT6573MDP_CharDrv, g_MT6573MDPdevno, 1))
    {
        printk("[mt6516_IDP] Attatch file operation failed\n");

        unregister_chrdev_region(g_MT6573MDPdevno, 1);

        return -EAGAIN;
    }

    return 0;

}

static inline void UnregisterMT6573MDPCharDrv(void)
{
    //Release char driver
    cdev_del(g_pMT6573MDP_CharDrv);

    unregister_chrdev_region(g_MT6573MDPdevno, 1);
}

static struct class *pMT6573MDP_CLASS = NULL;
// Called to probe if the device really exists. and create the semaphores
static int MT6573MDP_probe(struct platform_device *pdev)
{
//    int i4IRQ = 0;
//    u32 u4Index = 0;
//    struct resource * pstRes = NULL;
    struct device* mdp_device = NULL;

    MDPMSG("[mt6573_MDP] probing MT6573 MDP\n");

    //Check platform_device parameters
    if(NULL == pdev)
    {
        MDPDB("[MT6573MDP] platform data missed\n");

        return -ENXIO;
    }

    //register char driver
    //Allocate major no
    if(RegisterMT6573MDPCharDrv())
    {
        dev_err(&pdev->dev,"register char failed\n");

        return -EAGAIN;
    }

    //TODO : Mapping to MDP registers
    //For MT6573, there is no diff by calling request_region() or request_mem_region(),
    /*
    for(u4Index = 0; u4Index < 6; u4Index += 1){
        pstRes = platform_get_resource(pdev, IORESOURCE_MEM, u4Index);

        if(NULL == pstRes)
        {
            dev_err(&pdev->dev,"get resource failed\n");

            return -ENOMEM;
        }

        pstRes = request_mem_region(pstRes->start, pstRes->end - pstRes->start + 1,pdev->name);

        if(NULL == pstRes)
        {
            dev_err(&pdev->dev,"request I/O mem failed\n");
        }
        //ioremap_noncache()
    }
    */

    //Set IRQ
    if(request_irq(MT6573_OVLJPGDMA_IRQ_LINE , (irq_handler_t)MT6573MDP_isr, 0, pdev->name , NULL))
    {
        dev_err(&pdev->dev,"request OVL & JPGDMA IRQ line failed\n");
        return -ENODEV;
    }

    if(request_irq(MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE , (irq_handler_t)MT6573MDP_isr, 0, pdev->name , NULL))
    {
        dev_err(&pdev->dev,"request RDMA1 & PRZ1 & ROTDMA3 IRQ line failed\n");
        return -ENODEV;
    }

    if(request_irq(MT6573_CRZ_IRQ_LINE , (irq_handler_t)MT6573MDP_isr, 0, pdev->name , NULL))
    {
        dev_err(&pdev->dev,"request CRZ IRQ line failed\n");
        return -ENODEV;
    }

    if(request_irq(MT6573_ROT_DMA_IRQ_LINE , (irq_handler_t)MT6573MDP_isr, 0, pdev->name , NULL))
    {
        dev_err(&pdev->dev,"request rotdma IRQ line failed\n");
        return -ENODEV;
    }

    if(request_irq(MT6573_R_DMA_IRQ_LINE , (irq_handler_t)MT6573MDP_isr, 0, pdev->name , NULL))
    {
        dev_err(&pdev->dev,"request rdma IRQ line failed\n");
        return -ENODEV;
    }

    if(request_irq(MT6573_RESZ_IRQ_LINE , (irq_handler_t)MT6573MDP_isr, 0, pdev->name , NULL))
    {
        dev_err(&pdev->dev,"request rdma IRQ line failed\n");
        return -ENODEV;
    }

    mt6573_irq_set_sens(MT6573_OVLJPGDMA_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_set_sens(MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_set_sens(MT6573_CRZ_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_set_sens(MT6573_ROT_DMA_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_set_sens(MT6573_R_DMA_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_set_sens(MT6573_RESZ_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    mt6573_irq_unmask(MT6573_OVLJPGDMA_IRQ_LINE);
    mt6573_irq_unmask(MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE);
    mt6573_irq_unmask(MT6573_CRZ_IRQ_LINE);
    mt6573_irq_unmask(MT6573_ROT_DMA_IRQ_LINE);
    mt6573_irq_unmask(MT6573_R_DMA_IRQ_LINE);
    mt6573_irq_unmask(MT6573_RESZ_IRQ_LINE);

    pMT6573MDP_CLASS = class_create(THIS_MODULE, "idpdrv");
    if (IS_ERR(pMT6573MDP_CLASS)) {
        int ret = PTR_ERR(pMT6573MDP_CLASS);
        printk("Unable to create class, err = %d\n", ret);
        return ret;            
    }    
    mdp_device = device_create(pMT6573MDP_CLASS, NULL, g_MT6573MDPdevno, NULL, "mt6573-MDP");

    //Initialize variables
    g_stMT6573ResTbl.u4FreeTable = MT6573_ALLFLAG;
    g_stMT6573ResTbl.u4OccupiedTable = 0;
    g_stMT6573ResTbl.u4GMCPowerSatus = 0;
    spin_lock_init(&g_stMT6573ResTbl.ResLock);

    g_stMT6573IRQTbl.u4Table = 0;
    spin_lock_init(&g_stMT6573IRQTbl.ResLock);

    g_stMT6573ZoomReg.u4Dirty = 0;
    spin_lock_init(&g_stMT6573ZoomReg.ResLock);

    init_waitqueue_head(&g_MT6573MDPResWaitQueue);
    init_waitqueue_head(&g_MT6573MDPIRQWaitQueue);

    MDPMSG("[mt6573_MDP] probe MT6573 MDP success\n");

    return 0;
}

// Called when the device is being detached from the driver
static int MT6573MDP_remove(struct platform_device *pdev)
{
//    struct resource * pstRes = NULL;
//    u32 u4Index = 0;
//    int i4IRQ = 0;

    //unregister char driver.
    UnregisterMT6573MDPCharDrv();

    //TODO : unmaping IDP registers
    /*
    for(u4Index = 0; u4Index < 6; u4Index += 1)
    {
        pstRes = platform_get_resource(pdev, IORESOURCE_MEM, 0);

        release_mem_region(pstRes->start, (pstRes->end - pstRes->start + 1));
    }
    */

    //Release IRQ
    free_irq(MT6573_OVLJPGDMA_IRQ_LINE , NULL);

    free_irq(MT6573_RDMA1PRZ1ROTDMA3_IRQ_LINE , NULL);

    free_irq(MT6573_CRZ_IRQ_LINE , NULL);

    free_irq(MT6573_ROT_DMA_IRQ_LINE , NULL);

    free_irq(MT6573_R_DMA_IRQ_LINE , NULL);

    free_irq(MT6573_RESZ_IRQ_LINE , NULL);

    //
    device_destroy(pMT6573MDP_CLASS, g_MT6573MDPdevno);

    class_destroy(pMT6573MDP_CLASS);

    MDPDB("MT6573 MDP is removed\n");

    return 0;
}

static int MT6573MDP_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int MT6573MDP_resume(struct platform_device *pdev)
{
    return 0;
}

static struct platform_driver g_stMT6573MDP_Platform_Driver = {
    .probe		= MT6573MDP_probe,
    .remove	= MT6573MDP_remove,
    .suspend	= MT6573MDP_suspend,
    .resume	= MT6573MDP_resume,
    .driver		= {
        .name	= "mt6573-MDP",
        .owner	= THIS_MODULE,
    }
};

static int __init MT6573_MDP_Init(void)
{
    if(platform_driver_register(&g_stMT6573MDP_Platform_Driver)){
        printk("[MT6573MDP]failed to register MT6573 MDP driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit MT6573_MDP_Exit(void)
{
    platform_driver_unregister(&g_stMT6573MDP_Platform_Driver);
}

module_init(MT6573_MDP_Init);
module_exit(MT6573_MDP_Exit);
MODULE_DESCRIPTION("MT6573 MDP driver");
MODULE_AUTHOR("Gipi <Gipi.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");

