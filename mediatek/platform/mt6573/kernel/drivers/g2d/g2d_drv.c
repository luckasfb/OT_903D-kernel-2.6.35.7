
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>

#include <asm/io.h>

#include <mach/irqs.h>

#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_irq.h>

#include <asm/tcm.h>

#include "g2d_6573_func.h"
#include "g2d_drv.h"

//--------------------------------------------------------------------------
// Defination
//--------------------------------------------------------------------------
 
//#define g2d_print(fmt,...)    
#define g2d_print printk
#define G2D_DEVNAME "mt6573_g2d"

//--------------------------------------------------------------------------
// Gloable Variable
//--------------------------------------------------------------------------

// device and driver
static dev_t g2d_devno;
static struct cdev *g2d_cdev;
static struct class *g2d_class = NULL;

static wait_queue_head_t g2dWaitQueue;
static spinlock_t g2dSpinLock;

static int g2d_status = 0;
//--------------------------------------------------------------------------
// G2D IRQ Function
//--------------------------------------------------------------------------
static __tcmfunc irqreturn_t g2d_drv_isr(int irq, void *dev_id)
{
    g2d_print("G2D Interrupt\n");
    
    if(irq == MT6573_G2D_IRQ_LINE)
    {
        mt6573_irq_mask(MT6573_G2D_IRQ_LINE);      

        wake_up_interruptible(&g2dWaitQueue);
        
        mt6573_irq_unmask(MT6573_G2D_IRQ_LINE);        
    }
    
    return IRQ_HANDLED;
}


//--------------------------------------------------------------------------
// G2D Power on/off Function
//--------------------------------------------------------------------------
void g2d_drv_power_on(void)
{  
    BOOL ret;
   
    ret = hwEnableClock(MT65XX_PDN_MM_GMC2,"G2D");
    ret = hwEnableClock(MT65XX_PDN_MM_G2D,"G2D");

    NOT_REFERENCED(ret);
}

void g2d_drv_power_off(void)
{  
    BOOL ret;

    ret = hwDisableClock(MT65XX_PDN_MM_G2D,"G2D");
    ret = hwDisableClock(MT65XX_PDN_MM_GMC2,"G2D");

    NOT_REFERENCED(ret);
}

//--------------------------------------------------------------------------
// G2D File Operations Function
//--------------------------------------------------------------------------
static int g2d_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;
    int timeout;
    long timeout_jiff;
    g2d_context_t g2d_context;

    switch(cmd)
    {
        case G2D_IOCTL_INIT:
            g2d_print("G2D Driver Initial\n");

            ret = 0;
            spin_lock(&g2dSpinLock);
            if(g2d_status != 0)
            {
                ret = -EBUSY;
            }
            else
            {
                g2d_status = 1;
                g2d_drv_power_on();
            }
            spin_unlock(&g2dSpinLock);
            
            return ret;
            
        case G2D_IOCTL_BITBLT:
            g2d_print("G2D Driver Bitblt\n");
            if(copy_from_user(&g2d_context, (void *)arg, sizeof(g2d_context_t)))
            {
                printk("G2D : Copy from user error\n");
                return -EFAULT;
            }

            g2d_print("Src base address : 0x%x\n", (unsigned int)g2d_context.gSrcAddr);
            g2d_print("Src with/height  : (%u, %u)\n", g2d_context.gSrcWidth, g2d_context.gSrcHeight);
            g2d_print("Src pitch/format : (%u, %u)\n", g2d_context.gSrcPitch, g2d_context.gSrcColorFormat);
            g2d_print("Dst base address : 0x%x\n", (unsigned int)g2d_context.gDstAddr);
            g2d_print("Dst with/height  : (%u, %u)\n", g2d_context.gDstWidth, g2d_context.gDstHeight);
            g2d_print("Dst pitch/format : (%u, %u)\n", g2d_context.gDstPitch, g2d_context.gDstColorFormat);
            g2d_print("Rotation : %u\n", g2d_context.gRotate);
            g2d_print("Blending : %d, constant alpha : 0x%x\n", g2d_context.enAlphaBlending, g2d_context.gConstAlpha);
            
            g2d_drv_start(&g2d_context);
            
            return 0;
            
        case G2D_IOCTL_WAIT:
            g2d_print("G2D Driver Wait\n");
            if(copy_from_user(&timeout, (void *)arg, sizeof(int)))
            {
                printk("G2D : Copy from user error\n");
                return -EFAULT;
            }

            //set timeout
            if(g2d_drv_get_status() == 1)
            {
                timeout_jiff = timeout* HZ / 1000; 
                wait_event_interruptible_timeout(g2dWaitQueue, !g2d_drv_get_status(), timeout_jiff);
            }

            ret = 0;
            if(g2d_drv_get_status() == 1)
            {
                ret = -EFAULT;
            }

            g2d_drv_reset();
            
            return ret;
            
        case G2D_IOCTL_DEINIT:
            g2d_print("G2D Driver Deinitial\n");
            if(g2d_status == 0) 
            {
                printk("[G2D] Must Initial First!!!\n");
                return -EFAULT;
            }

            spin_lock(&g2dSpinLock);
            g2d_status = 0;
            spin_unlock(&g2dSpinLock);
            g2d_drv_power_off();
            
            return 0;

        default :
            break; 
    }
    
    return -EINVAL;
}

static int g2d_open(struct inode *inode, struct file *file)
{
    g2d_print("G2D Driver open\n");
    return 0;
}

static ssize_t g2d_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    g2d_print("G2D Driver read\n");
    return 0;
}

static int g2d_release(struct inode *inode, struct file *file)
{
    g2d_print("G2D Driver release\n");
	return 0;
}

static int g2d_flush(struct file * a_pstFile , fl_owner_t a_id)
{
    // To Do : error handling here
    if(g2d_status != 0) 
    {
        printk("Error! Enable error handling for G2D\n");
        spin_lock(&g2dSpinLock);
        g2d_status = 0;
        spin_unlock(&g2dSpinLock);
        g2d_drv_power_off();
    }
    return 0;
}

/* Kernel interface */
static struct file_operations g2d_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= g2d_ioctl,
	.open		= g2d_open,
	.release	= g2d_release,
	.flush		= g2d_flush,
	.read       = g2d_read,
};

static int g2d_probe(struct platform_device *pdev)
{
    struct class_device;
    
	int ret;
    struct class_device *class_dev = NULL;
    
    printk("\n\n\n\n\n\n=====================g2d probe======================\n\n\n\n\n\n\n");
	ret = alloc_chrdev_region(&g2d_devno, 0, 1, G2D_DEVNAME);

	if(ret) {
	    printk("Error: Can't Get Major number for G2D Device\n");
	} else {
	    printk("Get G2D Device Major number (%d)\n", g2d_devno);
    }

	g2d_cdev = cdev_alloc();
    g2d_cdev->owner = THIS_MODULE;
	g2d_cdev->ops = &g2d_fops;

	ret = cdev_add(g2d_cdev, g2d_devno, 1);

    g2d_class = class_create(THIS_MODULE, G2D_DEVNAME);
    class_dev = (struct class_device *)device_create(g2d_class, NULL, g2d_devno, NULL, G2D_DEVNAME);

    
    // initial g2d spin lock and register ISR
    spin_lock_init(&g2dSpinLock);
    init_waitqueue_head(&g2dWaitQueue);
    
    mt6573_irq_set_sens(MT6573_G2D_IRQ_LINE, MT65xx_EDGE_SENSITIVE);
    mt6573_irq_unmask(MT6573_G2D_IRQ_LINE);

    if(request_irq(MT6573_G2D_IRQ_LINE, (irq_handler_t)g2d_drv_isr, 0, "g2d" , NULL))
    {
        printk("g2d request irq failed\n");
    }

    g2d_status = 0;
	printk("G2D Probe Done\n");

	NOT_REFERENCED(class_dev);
	return 0;
}

static int g2d_remove(struct platform_device *pdev)
{
	printk("G2D Device Remove\n");;
	free_irq(MT6573_G2D_IRQ_LINE, NULL);
	
	printk("Done\n");
	return 0;
}

static void g2d_shutdown(struct platform_device *pdev)
{
	printk("g2d device shutdown\n");
	/* Nothing yet */
}

/* PM suspend */
static int g2d_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

/* PM resume */
static int g2d_resume(struct platform_device *pdev)
{
    return 0;
}


static struct platform_driver g2d_driver = {
	.probe		= g2d_probe,
	.remove		= g2d_remove,
	.shutdown	= g2d_shutdown,
	.suspend	= g2d_suspend,
	.resume		= g2d_resume,
	.driver     = {
	              .name = G2D_DEVNAME,
	},
};

static void g2d_device_release(struct device *dev)
{
	// Nothing to release? 
}

static u64 g2d_dmamask = ~(u32)0;

static struct platform_device g2d_device = {
	.name	 = G2D_DEVNAME,
	.id      = 0,
	.dev     = {
		.release = g2d_device_release,
		.dma_mask = &g2d_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = 0,
};

static int __init g2d_init(void)
{
    int ret;

    printk("G2D Initialize\n");
    printk("Register G2D device\n");
	if(platform_device_register(&g2d_device))
	{
        printk("failed to register g2d device\n");
        ret = -ENODEV;
        return ret;
	}

    printk("Register the g2d driver\n");    
    if(platform_driver_register(&g2d_driver))
    {
        printk("failed to register g2d driver\n");
        platform_device_unregister(&g2d_device);
        ret = -ENODEV;
        return ret;
    }

    return 0;
}

static void __exit g2d_exit(void)
{
    cdev_del(g2d_cdev);
    unregister_chrdev_region(g2d_devno, 1);

    platform_driver_unregister(&g2d_driver);
	platform_device_unregister(&g2d_device);
	
	device_destroy(g2d_class, g2d_devno);
	class_destroy(g2d_class);
	
	printk("g2d device & driver exit\n");
}

module_init(g2d_init);
module_exit(g2d_exit);
MODULE_AUTHOR("Tzu-Meng, Chung <Tzu-Meng.Chung@mediatek.com>");
MODULE_DESCRIPTION("MT6573 G2D Driver");
MODULE_LICENSE("GPL");
