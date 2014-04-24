

#include <linux/types.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/tcm.h>
#include <linux/proc_fs.h>  //proc file use
//
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/atomic.h>
#include <linux/sched.h>
#include <linux/mm.h>
//
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_isp.h>
//

#define ISP_TAG                 "[ISP] "
#define ISP_LOG(fmt, arg...)    printk(ISP_TAG fmt, ##arg)
#define ISP_ERR(fmt, arg...)    printk(ISP_TAG "Err: %5d:, "fmt, __LINE__, ##arg)
//
#define ISP_WR32(addr, data)    iowrite32(data, addr)
#define ISP_RD32(addr)          ioread32(addr)
#define ISP_SET_BIT(reg, bit)   ((*(volatile u32*)(reg)) |= (u32)(1 << (bit)))
#define ISP_CLR_BIT(reg, bit)   ((*(volatile u32*)(reg)) &= ~((u32)(1 << (bit))))
//
#define ISP_BASE                CAM_BASE
#define ISP_DEV_NAME            "mt6573-isp"

// Register definition
#define ISP_PHSCNT              (ISP_BASE + 0x000)
#define ISP_VFCON               (ISP_BASE + 0x018)
#define ISP_INTEN               (ISP_BASE + 0x01c)
#define ISP_INTSTA              (ISP_BASE + 0x020)
#define ISP_PATH                (ISP_BASE + 0x024)
#define ISP_RAWGAIN0            (ISP_BASE + 0x16C)
#define ISP_RAWGAIN1            (ISP_BASE + 0x170)
#define ISP_RESET               (ISP_BASE + 0x1D8)
#define ISP_TGSTATUS            (ISP_BASE + 0x1DC)
#define ISP_SHADING1            (ISP_BASE + 0x214)
#define ISP_VERSION             (ISP_BASE + 0x274)
//
#define ISP_G1MEMPDN            (MMSYS1_CONFIG_BASE + 0x30c)
//

#define ISP_INT_OVERRUN_MASK    ((u32)0x6)
#define ISP_INT_EXP_DONE        ((u32)0x1)
#define ISP_INT_IDLE            ((u32)0x1 << 3)
#define ISP_INT_ISP_DONE        ((u32)0x1 << 4)
#define ISP_INT_VSYNC           ((u32)0x1 << 10)

#define ISP_DBG_INT             0x0001
#define ISP_DBG_HOLD_REG        0x0002
#define ISP_DBG_READ_REG        0x0004
#define ISP_DBG_WRITE_REG       0x0008
#define ISP_DBG_CLK             0x0010
#define ISP_DBG_TASKLET         0x0020
#define ISP_DBG_WORKQUEUE       0x0040
//
unsigned long g_jiffies_exp_done = 0;
unsigned long g_jiffies_tasklet = 0;
static spinlock_t isp_lock;
static spinlock_t tasklet_lock;
static u8 *pcmd_buf = NULL;
static u8 *pread_buf = NULL;
static u8 *pwrite_buf = NULL;
static u32 buf_size = 4096;
static atomic_t hold_reg;
static atomic_t writing_reg;
static u32 hold_count;
static wait_queue_head_t isp_wait_queue;
static u32 irq_status;
static u32 dbgMask = 0x00000000;
static u32 g_u4DbgRegOfs = 0x0600;
static struct work_struct isp_work_queue;

static unsigned long ms_to_jiffies(unsigned long ms)
{
    return ((ms * HZ + 512) >> 10);
}

static int mt_isp_clk_ctrl(int en)
{
    if (dbgMask & ISP_DBG_CLK) {
        ISP_LOG("[mt_isp_clk_ctrl] %d \n", en);
    }
    //
    if (en) {
        hwEnableClock(MT65XX_PDN_MM_ISP, "ISP");
        hwEnableClock(MT65XX_PDN_MM_RESZ_LB, "ISP");
        ISP_CLR_BIT(ISP_G1MEMPDN, 9);
    }
    else {
        hwDisableClock(MT65XX_PDN_MM_ISP, "ISP");
        hwDisableClock(MT65XX_PDN_MM_RESZ_LB, "ISP");
        ISP_SET_BIT(ISP_G1MEMPDN, 9);
    }
    
    return 0;
}

static inline void mt_isp_reset(void)
{
    // ensure the view finder is disabe
    ISP_CLR_BIT(ISP_VFCON, 6);
    // do hw reset
    ISP_WR32(ISP_RESET, 1);
    // delay at least 100us for reset the ISp (HW suggest)
    udelay(120);
    ISP_WR32(ISP_RESET, 0);
    //
    irq_status = 0;
}

static int mt_isp_read_reg(mt_isp_reg_io_t *preg_io)
{
    int ret = 0;
    int size = preg_io->count * sizeof(mt_isp_reg_t);
    mt_isp_reg_t *preg = (mt_isp_reg_t *) pread_buf;
    int i;

    if (dbgMask & ISP_DBG_READ_REG) {
        ISP_LOG("[mt_isp_read_reg] data: 0x%x, count: %d \n", (u32) preg_io->data, (u32) preg_io->count);
    }
    //
    if (size > buf_size) {
        ISP_ERR("size too big \n");
    }
    //
    if (copy_from_user((u8 *)preg, (u8 *) preg_io->data, size) != 0) {
        ISP_ERR("copy_from_user failed \n");
        ret = -EFAULT;
        goto mt_isp_read_reg_exit;
    }
    //
    for (i = 0; i < preg_io->count; i++) {
        preg[i].val = ISP_RD32(ISP_BASE + preg[i].addr);
        if (dbgMask & ISP_DBG_READ_REG) {
            ISP_LOG("  addr/val: 0x%08x/0x%08x \n", (u32) (ISP_BASE + preg[i].addr), (u32) preg[i].val);
        }
    }
    //
    if (copy_to_user((u8 *) preg_io->data, (u8 *) preg, size) != 0) {
        ISP_ERR("copy_to_user failed \n");
        ret = -EFAULT;
        goto mt_isp_read_reg_exit;
    }

mt_isp_read_reg_exit:

    return ret;
}

static int mt_isp_write_reg_to_hw(mt_isp_reg_t *preg, u32 count)
{
    int ret = 0;
    int i;

    if (dbgMask & ISP_DBG_WRITE_REG) {
        ISP_LOG("[mt_isp_write_reg_to_hw] \n");
    }
    //
    for (i = 0; i < count; i++) {
        if (dbgMask & ISP_DBG_WRITE_REG) {
            ISP_LOG(" addr/val: 0x%08x/0x%08x \n", (u32) (ISP_BASE + preg[i].addr), (u32) preg[i].val);
        }
        ISP_WR32(ISP_BASE + preg[i].addr, preg[i].val);
    }

    return ret;
}

void mt_isp_tasklet_write_reg(unsigned long arg)
{
    mt_isp_reg_t *preg = (mt_isp_reg_t *) pwrite_buf;

    spin_lock(&tasklet_lock);

    if (0==atomic_read(&hold_reg)) {
        //  Record it for debug.
        g_jiffies_tasklet = jiffies;

        //  Not hold.
        if ((dbgMask & ISP_DBG_HOLD_REG) || (dbgMask & ISP_DBG_TASKLET)) {
            ISP_LOG("[mt_isp_tasklet_write_reg] [not hold] hold_count:%d \n", hold_count);
        }
        //
        mt_isp_write_reg_to_hw(preg, hold_count);
        hold_count = 0;
        //
        atomic_set(&writing_reg , 0);
        wake_up_interruptible(&isp_wait_queue);
    }
    else
    {   //  Do nothing during holding.
        if ((dbgMask & ISP_DBG_HOLD_REG) || (dbgMask & ISP_DBG_TASKLET)) {
            ISP_LOG("[mt_isp_tasklet_write_reg] [hold] hold_count:%d \n", hold_count);
        }
    }

    spin_unlock(&tasklet_lock);

    if (dbgMask & ISP_DBG_TASKLET) {
        ISP_LOG("-[mt_isp_tasklet_write_reg] \n");
    }
}

DECLARE_TASKLET(tasklet_write_reg, mt_isp_tasklet_write_reg, 0);

extern int kdSensorSyncFunctionPtr(UINT16 *pRAWGain);
extern int kdSetSensorSyncFlag(BOOL bSensorSync);
void mt_isp_work_queue(struct work_struct *work)
{
    u16 rawGain[4];

    // Synchronization the exposure time, sensor gain and raw gain.    
    kdSensorSyncFunctionPtr(&rawGain[0]);
    kdSetSensorSyncFlag(FALSE);
    //
    if ((rawGain[0] != 0) && (rawGain[1] != 0) && (rawGain[2] != 0) && (rawGain[3] != 0) ) {
        ISP_WR32(ISP_RAWGAIN0, ((u32) rawGain[0] << 0) | ((u32) rawGain[1] << 16));
        ISP_WR32(ISP_RAWGAIN1, ((u32) rawGain[2] << 0) | ((u32) rawGain[3] << 16));
    }
}

static int mt_isp_write_reg(mt_isp_reg_io_t *preg_io)
{
    int ret = 0;
    int size = preg_io->count * sizeof(mt_isp_reg_t);
    int hold_size = 0;
    mt_isp_reg_t *preg = NULL;

    if (dbgMask & ISP_DBG_WRITE_REG) {
        ISP_LOG("[mt_isp_write_reg] data: 0x%x, count: %d \n", (u32) preg_io->data, (u32) preg_io->count);
    }
    //
    hold_size = hold_count * sizeof(mt_isp_reg_t);
    preg = (mt_isp_reg_t *) (pwrite_buf + hold_size);
    //
    if ((size + hold_size) > buf_size) {
        long const ms_tasklet = ((long)jiffies-(long)g_jiffies_tasklet) * 1000 / HZ;
        long const ms_exp_done = ((long)jiffies-(long)g_jiffies_exp_done) * 1000 / HZ;
        ISP_ERR("size too big (hold, hold_size, size)=(%d, %d, %d) \n", atomic_read(&hold_reg), hold_size, size);
        ISP_ERR("exp done before %ld ms, tasklet done before %ld ms \n", ms_tasklet, ms_exp_done);
        ret = -EFAULT;
        goto mt_isp_write_reg_exit;
    }
    //
    if (copy_from_user((u8 *)preg, (u8 *) preg_io->data, size) != 0) {
        ISP_ERR("copy_from_user failed \n");
        ret = -EFAULT;
        goto mt_isp_write_reg_exit;
    }
    //
    if (atomic_read(&hold_reg)) {
        // Write register to buffer
        hold_count += preg_io->count;
    }
    else {
        // Write register to hw
        ret = mt_isp_write_reg_to_hw(preg, preg_io->count);
    }
    //
mt_isp_write_reg_exit:

    return ret;
}


static int mt_isp_hold_reg(u32 is_hold)
{
    int ret = 0;

    if  ( ! spin_trylock_bh(&tasklet_lock) )
    {
        //  Should wait until tasklet done.
        int timeout;
        int is_lock = 0;

        if (dbgMask & ISP_DBG_TASKLET) {
            ISP_LOG("[mt_isp_hold_reg] Start wait ... \n");
        }
        timeout = wait_event_interruptible_timeout(
            isp_wait_queue, (is_lock = spin_trylock_bh(&tasklet_lock)), ms_to_jiffies(500)
        );
        if (dbgMask & ISP_DBG_TASKLET) {
            ISP_LOG("[mt_isp_hold_reg] End wait \n");
        }

        if (0==is_lock) {
            ISP_ERR("[mt_isp_hold_reg] Should not happen: timeout & is_lock==0 \n");
            ret = -EFAULT;
            goto lbExit;
        }
    }

    //  Here we get the lock.

    if (dbgMask & ISP_DBG_HOLD_REG) {
        ISP_LOG("[mt_isp_hold_reg] %d, %d \n", is_hold, atomic_read(&hold_reg));
    }

    if (is_hold == 0) {
        if (atomic_read(&hold_reg) && 0 < hold_count) {
            // Hold is switching from on to off,
            // Should write register to hw if count > 0
            atomic_set(&writing_reg, 1);
        }
    }

    atomic_set(&hold_reg, is_hold);

    spin_unlock_bh(&tasklet_lock);

lbExit:
    return ret;
}

static int mt_isp_reset_buf(void)
{
    ISP_LOG("+[mt_isp_reset_buf] (hold, hold_count)=(%d, %d) \n", atomic_read(&hold_reg), hold_count);
    spin_lock_bh(&tasklet_lock);

    hold_count = 0;
    atomic_set(&hold_reg, 0);

    spin_unlock_bh(&tasklet_lock);

    ISP_LOG("-[mt_isp_reset_buf] \n");
    return 0;
}

static int mt_isp_wait_irq(u32 wait_irq_status)
{
    int ret = 0;
    int timeout;

    if (dbgMask & ISP_DBG_INT) {
        ISP_LOG("[mt_isp_wait_irq] \n" );
    }

    //always sync to next VD interrupt
    if (ISP_INT_VSYNC==wait_irq_status) {
       irq_status &= ~(ISP_INT_VSYNC);
    }

    timeout = wait_event_interruptible_timeout(
        isp_wait_queue, (wait_irq_status & irq_status), 3 * HZ);
    if (timeout == 0) {
        ISP_ERR("wait_event_interruptible_timeout timeout, %d, %d \n", wait_irq_status, irq_status);
        return -EAGAIN;
    }
    //
    spin_lock_irq(&isp_lock);
    //
    if (dbgMask & ISP_DBG_INT) {
        ISP_LOG("irq_status: 0x%x \n", irq_status);
    }
    //
    irq_status &= ~(wait_irq_status);
    //
    spin_unlock_irq(&isp_lock);

    return ret;
}

static int mt_isp_dump_reg(void)
{
    int ret = 0;
    int i;

    ISP_LOG("[mt_isp_dump_reg] E \n" );
    //
    spin_lock_irq(&isp_lock);
    //
    for (i = 0; i < 0x818; i += 4) {
        ISP_LOG("  addr, val: 0x%08x, 0x%08x \n", ISP_BASE + i, ISP_RD32(ISP_BASE + i));
    }
    //  CSI2 debug message
    for (i = 0x2000; i < 0x2030; i += 4) {
        ISP_LOG("  addr, val: 0x%08x, 0x%08x \n", ISP_BASE + i, ISP_RD32(ISP_BASE + i));
    }
    
    //  CSI2 debug message dump again to check the flag update to latest.
    for (i = 0x2000; i < 0x2030; i += 4) {
        ISP_LOG("  addr, val: 0x%08x, 0x%08x \n", ISP_BASE + i, ISP_RD32(ISP_BASE + i));
    }
    //
    spin_unlock_irq(&isp_lock);
    //
    ISP_LOG("[mt_isp_dump_reg] X \n" );

    return ret;
}

static __tcmfunc irqreturn_t mt_isp_irq(int irq, void *dev_id)
{
u32 irq_status_reg;

    // Read irq status
    //irq_status |= ISP_RD32(ISP_INTSTA);
    irq_status_reg = ISP_RD32(ISP_INTSTA);
    irq_status |= irq_status_reg;
    
    if (dbgMask & ISP_DBG_INT) {
        ISP_LOG("[mt_isp_irq] 0x%x \n", irq_status);
    }
    //
    if (irq_status & ISP_INT_OVERRUN_MASK) {
        ISP_ERR("ISP_INT_OVERRUN_MASK: 0x%x \n", irq_status);
        irq_status &= ~(ISP_INT_OVERRUN_MASK);
        //Incomplete frame error handling
        if(ioread32((ROT_DMA0_BASE + 0x18)) >> 31)
        {
            ISP_LOG("Speed up ROTDMA0!!\n");
            mt65xx_reg_sync_writel(0 , (ROT_DMA0_BASE + 0x300));
        }
        else if(ioread32((ROT_DMA1_BASE + 0x18)) >> 31)
        {
            ISP_LOG("Speed up ROTDMA1!!\n");
            mt65xx_reg_sync_writel(0 , (ROT_DMA1_BASE + 0x300));
        }
        else if(ioread32((ROT_DMA2_BASE + 0x18)) >> 31)
        {
            ISP_LOG("Speed up ROTDMA2!!\n");
            mt65xx_reg_sync_writel(0 , (ROT_DMA2_BASE + 0x300));
        }
        ISP_LOG("speed up ROTDMA done \n");
    
        // ensure the view finder is disabe
        ISP_CLR_BIT(ISP_VFCON, 6);
        // do hw reset
        //ISP_WR32(ISP_RESET, 1);
        mt65xx_reg_sync_writel(0x10000,ISP_RESET);
        //ISP_SET_BIT(ISP_RESET, 16); //SW reset
        //ISP_SET_BIT(ISP_RESET, 0); //HW reset
        // delay at least 100us for reset the ISp (HW suggest)
        udelay(120);
        //ISP_WR32(ISP_RESET, 0);
        mt65xx_reg_sync_writel(0,ISP_RESET);
        //ISP_CLR_BIT(ISP_RESET, 16); //SW reset
        //ISP_CLR_BIT(ISP_RESET, 0); //HW reset
        ISP_SET_BIT(ISP_VFCON, 6);
        irq_status = 0;
        ISP_LOG("ISP reset done\n");
    }
    //
    wake_up_interruptible(&isp_wait_queue);
    //
    if (irq_status_reg & ISP_INT_VSYNC) {
        if (dbgMask & ISP_DBG_WORKQUEUE) {
            ISP_LOG("[mt_isp_irq] schedule_work \n");
        }
        schedule_work(&isp_work_queue);
        //irq_status &= ~(ISP_INT_VSYNC);
    }
    // If ISP exp int is done, wake up tasklet to write register if it is buffered
    if  (irq_status & ISP_INT_EXP_DONE) {
        // Record it for debug
        g_jiffies_exp_done = jiffies;

        if  (atomic_read(&writing_reg)) {
            if (dbgMask & ISP_DBG_TASKLET) {
                ISP_LOG("[mt_isp_irq] tasklet_schedule \n");
            }
            tasklet_schedule(&tasklet_write_reg);
            irq_status &= ~(ISP_INT_EXP_DONE);
        }
    }

    return IRQ_HANDLED;
}

static int mt_isp_ioctl(struct inode *inode, struct file *file,
                        unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    //
    if (_IOC_DIR(cmd) != _IOC_NONE) {
        // IO write
        if (_IOC_DIR(cmd) & _IOC_WRITE) {
            if (copy_from_user(pcmd_buf, (void *) arg, _IOC_SIZE(cmd)) != 0) {
                ISP_ERR("copy_from_user failed \n");
                return -EFAULT;
            }
        }
    }
    //
    switch (cmd) {
    case MT_ISP_IOC_T_RESET:
        ISP_LOG("[MT_ISP_IOC_T_RESET] \n");
        mt_isp_reset();
        break;
    case MT_ISP_IOC_G_READ_REG:
        ret = mt_isp_read_reg((mt_isp_reg_io_t *) pcmd_buf);
        break;
    case MT_ISP_IOC_S_WRITE_REG:
        ret = mt_isp_write_reg((mt_isp_reg_io_t *) pcmd_buf);
        break;
    case MT_ISP_IOC_T_HOLD_REG:
        ret = mt_isp_hold_reg(*(u32 *) pcmd_buf);
        break;
    //case MT_ISP_IOC_T_RUN:
    //   break;
    case MT_ISP_IOC_T_WAIT_IRQ:
        ret = mt_isp_wait_irq(*(u32 *) pcmd_buf);
        break;
    case MT_ISP_IOC_T_DUMP_REG:
        ret = mt_isp_dump_reg();
        break;
    case MT_ISP_IOC_T_DBG_FLAG:
        ISP_LOG("[MT_ISP_IOC_T_DBG_FLAG]: 0x%x \n", *(u32 *) pcmd_buf);
        dbgMask = *(u32 *) pcmd_buf;
        break;
    case MT_ISP_IOC_T_RESET_BUF:
        ret = mt_isp_reset_buf();
        break;
    default:
        ISP_ERR("[mt_isp_ioctl] unknown cmd \n");
        ret = -EPERM;
        break;
    }
    //
    if (_IOC_READ & _IOC_DIR(cmd)) {
        if (copy_to_user((void __user *) arg, pcmd_buf , _IOC_SIZE(cmd)) != 0) {
            ISP_ERR("copy_to_user failed \n");
            return -EFAULT;
        }
    }

    return ret;
}

static int mt_isp_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    ISP_LOG("[mt_isp_open] \n");
    //
    spin_lock_irq(&isp_lock);
    if (pcmd_buf) {
        // Buffer has been allocated
        ISP_LOG("  open more than once \n");
        goto mt_isp_open_exit;
    }
    // Allocate buffer for cmd/read/write buffer
    if (pcmd_buf != NULL) {
        ISP_ERR("pcmd_buf is not null \n");
    }
    if (pread_buf != NULL) {
        ISP_ERR("pread_buf is not null \n");
    }
    if (pwrite_buf != NULL) {
        ISP_ERR("pread_buf is not null \n");
    }
    pcmd_buf = (u8 *) kmalloc(buf_size, GFP_KERNEL);
    if (pcmd_buf == NULL) {
        ISP_ERR("kmalloc failed \n ");
        ret = -ENOMEM;
        goto mt_isp_open_exit;
    }
    pread_buf = (u8 *) kmalloc(buf_size, GFP_KERNEL);
    if (pread_buf == NULL) {
        ISP_ERR("kmalloc failed \n ");
        ret = -ENOMEM;
        goto mt_isp_open_exit;
    }
    pwrite_buf = (u8 *) kmalloc(buf_size, GFP_KERNEL);
    if (pwrite_buf == NULL) {
        ISP_ERR("kmalloc failed \n ");
        ret = -ENOMEM;
        goto mt_isp_open_exit;
    }
    //
    atomic_set(&hold_reg, 0);
    atomic_set(&writing_reg, 0);
    hold_count = 0;
    irq_status = 0;
    //
    init_waitqueue_head(&isp_wait_queue);
    //
    INIT_WORK(&isp_work_queue, mt_isp_work_queue);
    // Enable clock
    mt_isp_clk_ctrl(1);
        
mt_isp_open_exit:
    if (ret < 0) {
        if (pcmd_buf) {
            kfree(pcmd_buf);
            pcmd_buf = NULL;
        }
        if (pread_buf) {
            kfree(pread_buf);
            pread_buf = NULL;
        }
        if (pwrite_buf) {
            kfree(pwrite_buf);
            pwrite_buf = NULL;
        }
    }

    spin_unlock_irq(&isp_lock);

    return ret;
}

static int mt_isp_release(struct inode *inode, struct file *file)
{
    ISP_LOG("[mt_isp_release] \n");

    //
    mt_isp_clk_ctrl(0);
    //                
    if (pcmd_buf) {
        kfree(pcmd_buf);
        pcmd_buf = NULL;
    }
    if (pread_buf) {
        kfree(pread_buf);
        pread_buf = NULL;
    }
    if (pwrite_buf) {
        kfree(pwrite_buf);
        pwrite_buf = NULL;
    }

    return 0;
}

static int mt_isp_mmap(struct file *file, struct vm_area_struct *vma)
{
    ISP_LOG("[mt_isp_mmap] \n");

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);            
    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, 
        vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
        return -EAGAIN;
    }

    return 0;
}

static dev_t dev_isp;
static struct cdev *pcdev_isp = NULL;
static struct class *pclass_isp = NULL;
static const struct file_operations mt_isp_fops = {
    .owner   = THIS_MODULE,
    .open    = mt_isp_open,
    .release = mt_isp_release,
    //.flush   = mt_isp_flush,
    .mmap    = mt_isp_mmap,
    .ioctl   = mt_isp_ioctl
};
static int isp_irq_num;

inline static void mt_isp_unregister_char_driver(void)
{
    ISP_LOG("[mt_isp_unregister_char_driver] \n");

    //Release char driver
    if (pcdev_isp != NULL) {
        cdev_del(pcdev_isp);
        pcdev_isp = NULL;
    }
    //
    unregister_chrdev_region(dev_isp, 1);
}

inline static int mt_isp_register_char_driver(void)
{
    int ret = 0;

    ISP_LOG("[mt_isp_register_char_driver] \n");
    //
    if ( (ret = alloc_chrdev_region(&dev_isp, 0, 1, ISP_DEV_NAME)) < 0 ) {
        ISP_ERR("alloc_chrdev_region failed, %d \n", ret);
        return ret;
    }
    //Allocate driver
    pcdev_isp = cdev_alloc();
    if (pcdev_isp == NULL) {
        ISP_ERR("cdev_alloc failed\n");
        ret = -ENOMEM;
        goto mt_isp_register_char_driver_exit;
    }
    //Attatch file operation.
    cdev_init(pcdev_isp, &mt_isp_fops);
    //
    pcdev_isp->owner = THIS_MODULE;
    //Add to system
    if ( (ret = cdev_add(pcdev_isp, dev_isp, 1)) < 0) {
        ISP_ERR("Attatch file operation failed, %d \n", ret);
        goto mt_isp_register_char_driver_exit;
    }

mt_isp_register_char_driver_exit:
    //
    if (ret < 0) {
        mt_isp_unregister_char_driver();
    }

    return ret;
}

static int mt_isp_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct resource *pres = NULL;
    int i;

    ISP_LOG("[mt_isp_probe] enter \n");
    // Check platform_device parameters
    if (pdev == NULL) {
        dev_err(&pdev->dev, "pdev is NULL \n");
        return -ENXIO;
    }
    // Register char driver
    if ( (ret = mt_isp_register_char_driver()) ) {
        dev_err(&pdev->dev, "register char failed \n");
        return ret;
    }
    // Mapping CAM_REGISTERS
    for (i = 0; i < 2; i++) {
        pres = platform_get_resource(pdev, IORESOURCE_MEM, i);
        if (pres == NULL) {
            dev_err(&pdev->dev, "platform_get_resource failed \n");
            ret = -ENOMEM;
            goto mt_isp_probe_exit;
        }
        pres = request_mem_region(pres->start, pres->end - pres->start + 1, pdev->name);
        if (pres == NULL) {
            dev_err(&pdev->dev, "request_mem_region failed \n");
            ret = -ENOMEM;
            goto mt_isp_probe_exit;
        }
    }
    // Request CAM_ISP IRQ
    isp_irq_num = platform_get_irq(pdev, 0);
    ISP_LOG("IRQ Num = %d \n", isp_irq_num);
    if ( (ret = request_irq(isp_irq_num, (irq_handler_t)mt_isp_irq, 0, pdev->name, NULL)) ) {
        dev_err(&pdev->dev, "request IRQ failed\n");
        goto mt_isp_probe_exit;
    }
    mt6573_irq_set_sens(MT6573_CAM_IRQ_LINE,MT65xx_LEVEL_SENSITIVE);
    // Someone has enable isp irq ??
    // enable_irq(isp_irq_num);
    //
    // Create class register
    pclass_isp = class_create(THIS_MODULE, "ispdrv");
    if (IS_ERR(pclass_isp)) {
        ret = PTR_ERR(pclass_isp);
        ISP_ERR("Unable to create class, err = %d\n", ret);
        return ret;
    }
    // FIXME: error handling
    device_create(pclass_isp, NULL, dev_isp, NULL, ISP_DEV_NAME);
    // Initialize critical section
    spin_lock_init(&isp_lock);
    spin_lock_init(&tasklet_lock);

mt_isp_probe_exit:
    //
    if (ret < 0) {
        mt_isp_unregister_char_driver();
    }
    //
    ISP_LOG("[mt_isp_probe] exit \n");
    //
    return ret;
}

static int mt_isp_remove(struct platform_device *pdev)
{
    struct resource *pres;
    int i;
    int irq_num;

    ISP_LOG("[mt_isp_remove] \n");
    // unregister char driver.
    mt_isp_unregister_char_driver();
    // unmaping ISP CAM_REGISTER registers
    for (i = 0; i < 2; i++) {
        pres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        release_mem_region(pres->start, (pres->end - pres->start + 1));
    }
    // Release IRQ
    disable_irq(isp_irq_num);
    irq_num = platform_get_irq(pdev, 0);
    free_irq(irq_num , NULL);
    //
    device_destroy(pclass_isp, dev_isp);
    //
    class_destroy(pclass_isp);
    pclass_isp = NULL;

    return 0;
}

static int mt_isp_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int mt_isp_resume(struct platform_device *pdev)
{
    return 0;
}

static struct platform_driver isp_driver = {
    .probe   = mt_isp_probe,
    .remove  = mt_isp_remove,
    .suspend = mt_isp_suspend,
    .resume  = mt_isp_resume,
    .driver  = {
        .name  =  "mt-isp",
        .owner = THIS_MODULE,
    }
};

static int
mt_isp_dump_reg_to_proc(
    char *page, char **start, off_t off, int count, int *eof, void *data
)
{
    char *p = page;
    int len = 0;
    u32 u4Index = 0 ;

    ISP_LOG("[mt_isp_dump_reg_to_proc] \n");
    //
    p += sprintf(p, "\n\r MT6573 ISP Register \n\r");
    p += sprintf(p,"======General Setting ====\n\r");
    for (; u4Index < 0x3C; u4Index += 4) {
        p += sprintf(p,"+0x%0x 0x%0x\n\r", u4Index, ISP_RD32(ISP_BASE + u4Index));
    }
    p += sprintf(p,"======Result Win Setting ====\n\r");
    for (u4Index = 0x174; u4Index < 0x190; u4Index += 4) {
        p += sprintf(p,"+0x%0x 0x%0x\n\r", u4Index, ISP_RD32(ISP_BASE + u4Index));
    }
    //
    for (u4Index = 0x1D4; u4Index <= 0x1D8; u4Index +=4) {
        p += sprintf(p, "+0x%0x 0x%0x\n\r", u4Index, ISP_RD32(ISP_BASE + u4Index));
    }

    *start = page + off;

    len = p - page;
    if (len > off) {
        len -= off;
    }
    else {
        len = 0;
    }

    return len < count ? len : count;
}

static int  mt_isp_reg_debug( 
    struct file *file, const char *buffer, unsigned long count, void *data
)
{
    char regBuf[64] = {'\0'}; 
    u32 u4CopyBufSize = (count < (sizeof(regBuf) - 1)) ? (count) : (sizeof(regBuf) - 1); 
    u32 u4Addr = 0; 
    u32 u4Data = 0; 

    if (copy_from_user(regBuf, buffer, u4CopyBufSize))
        return -EFAULT;

    if (sscanf(regBuf, "%x %x",  &u4Addr, &u4Data) == 2) {
            iowrite32(u4Data, CAM_BASE + u4Addr); 
            ISP_LOG("write addr = 0x%08x, data = 0x%08x\n", u4Addr, ioread32(CAM_BASE + u4Addr));             
    }
    else if (sscanf(regBuf, "%x", &u4Addr) == 1) {    
            ISP_LOG("read addr = 0x%08x, data = 0x%08x\n", u4Addr, ioread32(CAM_BASE + u4Addr)); 
    }

    return count; 
}

static int
mt_isp_read_flag_by_proc(
    char *page, char **start, off_t off, int count, int *eof, void *data
)
{
    char *p = page;
    int len = 0;
    //
    p += sprintf(p, "[isp debug]\r\n");
    p += sprintf(p, "dbgMask = 0x%08X\r\n", dbgMask);
    p += sprintf(p, "read addr[0x%08X] = 0x%08X\r\n", g_u4DbgRegOfs, ISP_RD32(ISP_BASE + g_u4DbgRegOfs));

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

static int
mt_isp_write_flag_by_proc( 
    struct file *file, const char *buffer, unsigned long count, void *data
)
{
    char acBuf[32]; 
    unsigned long u4CopySize = 0;
    u32 u4DbgMask = 0;
    u32 u4DbgRegOfs = 0;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if  ( copy_from_user(acBuf, buffer, u4CopySize) )
		return 0;
    acBuf[u4CopySize] = '\0';

    if ( 2 == sscanf(acBuf, "%x%x", &u4DbgMask, &u4DbgRegOfs) )
    {
        dbgMask = u4DbgMask;
        g_u4DbgRegOfs = u4DbgRegOfs;
    }
    else if ( 1 == sscanf(acBuf, "%x", &u4DbgMask) )
    {
        dbgMask = u4DbgMask;
    }

    return count;
}

static int __init mt_isp_init(void)
{
    int ret = 0;
    struct proc_dir_entry *prEntry;

    ISP_LOG("[mt_isp_init] \n");
    //
    if ((ret = platform_driver_register(&isp_driver)) < 0) {
        ISP_ERR("platform_driver_register fail \n");
        return ret;
    }
    //
    prEntry = create_proc_entry("driver/isp_reg", 0, NULL); 
    if (prEntry) {
        prEntry->read_proc = mt_isp_dump_reg_to_proc; 
        prEntry->write_proc = mt_isp_reg_debug; 
    }
    else {
        ISP_ERR("add /proc/driver/isp_reg entry fail \n");  
    }
    //
    prEntry = create_proc_entry("driver/isp_flag", 0, NULL); 
    if (prEntry) {
        prEntry->read_proc = mt_isp_read_flag_by_proc;
        prEntry->write_proc = mt_isp_write_flag_by_proc;
    }
    else {
        ISP_ERR("add /proc/driver/isp_flag entry fail \n");
    }

    return ret;
}

static void __exit mt_isp_exit(void)
{
    ISP_LOG("[mt_isp_exit] \n");

    platform_driver_unregister(&isp_driver);
}

void mt_isp_mclk_ctrl(int en)
{
    if (en)
    {
        ISP_SET_BIT(ISP_PHSCNT, 29);
    }
    else
    {
        ISP_CLR_BIT(ISP_PHSCNT, 29);
    }
}
EXPORT_SYMBOL(mt_isp_mclk_ctrl);
module_init(mt_isp_init);
module_exit(mt_isp_exit);
MODULE_DESCRIPTION("MT6573 ISP driver");
MODULE_AUTHOR("ME3");
MODULE_LICENSE("GPL");

