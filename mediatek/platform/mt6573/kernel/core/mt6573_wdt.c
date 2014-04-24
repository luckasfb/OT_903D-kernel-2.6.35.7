
#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>

// Monkey.QHQ
#include <linux/list.h>
#include <linux/init.h>
#include <linux/sched.h>
// Monkey.QHQ

#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_wdt.h>
#include <mach/mt6573_pll.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <asm/tcm.h>

#define NO_DEBUG 1
#define WDT_WORKAROUD 0

#define WDT_DEVNAME "watchdog"

static struct class *wdt_class = NULL;
static int wdt_major = 0;
static dev_t wdt_devno;
static struct cdev *wdt_cdev;

static char expect_close; // Not use
static spinlock_t mt65xx_wdt_spinlock = SPIN_LOCK_UNLOCKED;
static unsigned short timeout;
volatile kal_bool  mt65xx_wdt_INTR;
static int g_lastTime_TimeOutValue = 0;
static int g_WDT_enable = 0;

enum {
	WDT_NORMAL_MODE,
	WDT_EXP_MODE
} g_wdt_mode = WDT_NORMAL_MODE;

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
//MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");


void mt6573_wdt_SetTimeOutValue(unsigned short value)
{
	/*
	 * TimeOut = BitField 15:5
	 * Key	   = BitField  4:0 = 0x08
	 */	
	spin_lock(&mt65xx_wdt_spinlock);
	
    // 1 tick means 512 * T32K -> 1s = T32/512 tick = 64
	// --> value * (1<<6)
	timeout = (unsigned short)(value * ( 1 << 6) );

	timeout = timeout << 5; 

	DRV_WriteReg16(MT6573_WDT_LENGTH, (timeout | MT6573_WDT_LENGTH_KEY) );
	
	spin_unlock(&mt65xx_wdt_spinlock);
}
EXPORT_SYMBOL(mt6573_wdt_SetTimeOutValue);

void mt6573_wdt_SetResetLength(unsigned short value)
{
	/* 
	 * Lenght = 0xFFF for BitField 11:0
	 */
	
	spin_lock(&mt65xx_wdt_spinlock);
	
	DRV_WriteReg16(MT6573_WDT_INTERNAL, (value & 0xFFF) );

	spin_unlock(&mt65xx_wdt_spinlock);
}
EXPORT_SYMBOL(mt6573_wdt_SetResetLength);


void mt6573_wdt_ModeSelection(kal_bool en, kal_bool auto_rstart, kal_bool IRQ )
{
	unsigned short tmp;

    spin_lock(&mt65xx_wdt_spinlock);
    
	tmp = DRV_Reg16(MT6573_WDT_MODE);
	tmp |= MT6573_WDT_MODE_KEY;
	
	// Bit 0 : Whether enable watchdog or not
	if(en == KAL_TRUE)
		tmp |= MT6573_WDT_MODE_ENABLE;
	else
		tmp &= ~MT6573_WDT_MODE_ENABLE;
	
	// Bit 4 : Whether enable auto-restart or not for counting value
	if(auto_rstart == KAL_TRUE)
		tmp |= MT6573_WDT_MODE_AUTORST;
	else
		tmp &= ~MT6573_WDT_MODE_AUTORST;

	// Bit 3 : TRUE for generating Interrupt (False for generating Reset) when WDT timer reaches zero
	if(IRQ == KAL_TRUE)
		tmp |= MT6573_WDT_RESET_IRQ;
	else
		tmp &= ~MT6573_WDT_RESET_IRQ;

	DRV_WriteReg16(MT6573_WDT_MODE,tmp);

	spin_unlock(&mt65xx_wdt_spinlock);
}
EXPORT_SYMBOL(mt6573_wdt_ModeSelection);

/*Kick the watchdog*/
void mt6573_wdt_Restart(void)
{	
	spin_lock(&mt65xx_wdt_spinlock);
	
	DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY);

	spin_unlock(&mt65xx_wdt_spinlock);
}
EXPORT_SYMBOL(mt6573_wdt_Restart);

/*where to config watchdog kicker*/
#ifdef CONFIG_MTK_WD_KICKER

#include <wd_kicker.h>

static int mt6573_wk_wdt_config(enum wk_wdt_mode mode, int timeout)
{

	//disable WDT reset, auto-restart disable , disable intr
	mt6573_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	mt6573_wdt_Restart();

	if (mode == WK_WDT_EXP_MODE) {
		g_wdt_mode = WDT_EXP_MODE;
		mt6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);	        
	}
	else {
		g_wdt_mode = WDT_NORMAL_MODE;
#if WDT_WORKAROUD // for workaround HW issue: WDT can NOT reboot
        mt6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);
#else
        mt6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
#endif		
	}
	
	g_lastTime_TimeOutValue = timeout;
	mt6573_wdt_SetTimeOutValue(timeout);
	g_WDT_enable = 1;
	mt6573_wdt_Restart();

	return 0;
}

static struct wk_wdt mt6573_wk_wdt = {
	.config 	= mt6573_wk_wdt_config,
	.kick_wdt 	= mt6573_wdt_Restart
};
#endif

/*software reset*/
void mt6573_wdt_SWTrigger(void)
{
	// SW trigger WDT reset or IRQ
	
	spin_lock(&mt65xx_wdt_spinlock);	
	DRV_WriteReg16(MT6573_WDT_SWRST, MT6573_WDT_SWRST_KEY);
	spin_unlock(&mt65xx_wdt_spinlock);
}
EXPORT_SYMBOL(mt6573_wdt_SWTrigger);

unsigned char mt6573_wdt_CheckStatus(void)
{
	unsigned char status;

	spin_lock(&mt65xx_wdt_spinlock);	
	status = DRV_Reg16(MT6573_WDT_STATUS);
	spin_unlock(&mt65xx_wdt_spinlock);
	
	return status;
}
EXPORT_SYMBOL(mt6573_wdt_CheckStatus);

void WDT_arch_reset(char mode)
{
  spin_lock(&mt65xx_wdt_spinlock);
	/* Watchdog Rest */
  DRV_WriteReg(MT6573_WDT_MODE, (MT6573_WDT_MODE_KEY|MT6573_WDT_MODE_EXTEN|MT6573_WDT_MODE_ENABLE));
  DRV_WriteReg(MT6573_WDT_LENGTH, MT6573_WDT_LENGTH_KEY);
  DRV_WriteReg(MT6573_WDT_SWRST, MT6573_WDT_SWRST_KEY);
  spin_unlock(&mt65xx_wdt_spinlock);

  while (1);
}
EXPORT_SYMBOL(WDT_arch_reset);


void WDT_HW_Reset_test(void)
{
	printk("WDT_HW_Reset_test : System will reset after 5 secs\n");
	mt6573_wdt_SetTimeOutValue(5);   	
	//enable WDT reset, auto-restart enable ,disable interrupt.
	mt6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
	mt6573_wdt_Restart();
	
	while(1);
}
EXPORT_SYMBOL(WDT_HW_Reset_test);


void WDT_HW_Reset_kick_6times_test(void)
{
	int kick_times = 6;
	
	mt6573_wdt_SetTimeOutValue(5);	
	//enable WDT reset, auto-restart enable ,disable intr
	mt6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
	mt6573_wdt_Restart();

	// kick WDT test.
	while(kick_times >= 0)
	{
		mdelay(3000);
		printk("WDT_HW_Reset_test : reset after %d times !\n", kick_times);
		mt6573_wdt_Restart();
		kick_times--;
	}

	printk("WDT_HW_Reset_test : Kick stop,System will reset after 5 secs!!\n");
	while(1);
	
}
EXPORT_SYMBOL(WDT_HW_Reset_kick_6times_test);


void WDT_SW_Reset_test(void)
{
	printk("WDT_SW_Reset_test : System will reset Immediately\n");
		
	// disable WDT reset, auto-restart disable ,disable intr
	mt6573_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	
	mt6573_wdt_SetTimeOutValue(1);
	mt6573_wdt_Restart();
	
	mt6573_wdt_SWTrigger();
	
	while(1);
}
EXPORT_SYMBOL(WDT_SW_Reset_test);

void WDT_count_test(void)
{
	/*
	 * Try DVT testsuite : WDT_count_test (Non-reset test)
	 */
	printk("WDT_count_test Start..........\n");

	mt6573_wdt_SetTimeOutValue(10);
	// enable WDT reset, auto-restart disable, enable intr
	mt6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);
	mt6573_wdt_Restart();

	printk("1/2 : Waiting 10 sec. for WDT with WDT_status.\n");
	
	// wait and check WDT status
	while(DRV_Reg16(MT6573_WDT_STATUS) != MT6573_WDT_STATUS_HWWDT);	
	printk("WDT_count_test done by WDT_STATUS!!\n");

    /*status is checked*/

    /*check interrupt.*/ 
	mt65xx_wdt_INTR = 0; // set to zero before WDT counting down

	/*can continue.*/
	mt6573_wdt_Restart();
	
	printk("2/2 : Waiting 10 sec. for WDT with IRQ.\n");

	//need a ISR, when interrupt, set mt65xx_wdt_INTR to 1.
	while(mt65xx_wdt_INTR == 0);
	printk("WDT_count_test done by IRQ!!\n");
	
	printk("WDT_count_test Finish !!\n");	
}
EXPORT_SYMBOL(WDT_count_test);

//void Mt6516_traceCallStack(void);
//void aee_bug(const char *source, const char *msg);

//Monkey.QHQ
static void wdt_report_info (void)
{
    //extern struct task_struct *wk_tsk;
    struct task_struct *task ;
    task = &init_task ;
    
    printk ("Qwdt: -- watchdog time out\n") ;
    for_each_process (task)
    {
        if (task->state == 0)
        {
            printk ("PID: %d, name: %s\n backtrace:\n", task->pid, task->comm) ;
            show_stack (task, NULL) ;
            printk ("\n") ;
        }
    }
    
    
    printk ("backtrace of current task:\n") ;
    show_stack (NULL, NULL) ;
    
    
    printk ("Qwdt: -- watchdog time out\n") ;    
}
//Monkey.QHQ

static __tcmfunc irqreturn_t mt6573_wdt_ISR(int irq, void *dev_id)
{
	mt65xx_irq_mask(MT6573_APWDT_IRQ_LINE);
	mt65xx_wdt_INTR = 1;
#if WDT_WORKAROUD // for workaround HW issue: WDT can NOT reboot
#ifdef USER_BUILD_KERNEL //USR_BUILD
    if (g_wdt_mode == WDT_NORMAL_MODE) {
		mt65xx_irq_ack(MT6573_APWDT_IRQ_LINE);
		mt65xx_irq_unmask(MT6573_APWDT_IRQ_LINE);
		MTCMOS_En(MM1_SUBSYS);
	    DRV_WriteReg16(MT6573_WDT_MODE,MT6573_WDT_MODE_KEY|MT6573_WDT_MODE_EXTEN);// disable
	    DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY);//restart
	    /*enable, auto_restart,*/
	    DRV_WriteReg16(MT6573_WDT_MODE,MT6573_WDT_MODE_KEY|MT6573_WDT_MODE_ENABLE|MT6573_WDT_MODE_EXTEN|MT6573_WDT_MODE_AUTORST);
	    DRV_WriteReg16(MT6573_WDT_LENGTH, (1<<6 | MT6573_WDT_LENGTH_KEY) );
		DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY);//restart
		while(1)
		   printk("WDT shutdown machine\n");
	}		  
#endif
#endif
	//Monkey.QHQ
  wdt_report_info () ;
  //Monkey.QHQ
	BUG();

    //need to modift. 
	//Mt6516_traceCallStack();
	//aee_bug("WATCHDOG", "Watch Dog Timeout");
	mt65xx_irq_ack(MT6573_APWDT_IRQ_LINE);
	mt65xx_irq_unmask(MT6573_APWDT_IRQ_LINE);
	
	return IRQ_HANDLED;
}
static int mt65xx_wdt_open(struct inode *inode, struct file *file)
{
	printk( "\n******** WDT driver open!! ********\n" );
	
	#if NO_DEBUG
	
	//mt6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);

	/*
	 * default : user can not stop WDT
	 *
	 * If the userspace daemon closes the file without sending
	 * this special character "V", the driver will assume that the daemon (and
	 * userspace in general) died, and will stop pinging the watchdog without
	 * disabling it first.  This will then cause a reboot.
	 */
	expect_close = 0;
	#endif	

	return nonseekable_open(inode, file);
}

static int mt65xx_wdt_release(struct inode *inode, struct file *file)
{
	printk( "\n******** mt6573 WDT driver release!! ********\n");

	//if( expect_close == 42 )
	if( expect_close == 0 )
	{
		#if NO_DEBUG		
		mt6573_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
		#endif
	}
	else
	{
		#if NO_DEBUG
		mt6573_wdt_Restart();
		#endif
	}

	expect_close = 0;
	
	g_WDT_enable = 0;
		
	return 0;
}

static ssize_t mt65xx_wdt_write(struct file *file, const char __user *data,
								size_t len, loff_t *ppos)
{
	printk( "\n******** mt6573 WDT driver : write<%d> ********\n",len);	

	if(len) 
	{
		if(!nowayout) 
		{			
			size_t i;
			expect_close = 0;
			for( i = 0 ; i != len ; i++ ) 
			{
				char c;
				if( get_user(c, data + i) )
					return -EFAULT;
				
				if( c == 'V' )
				{
					expect_close = 42;
					printk( "\nnowayout=N and write=V, you can disable HW WDT\n");
				}
			}
		}		
        		
        mt6573_wdt_Restart();
	}
	return len;
}

#define OPTIONS WDIOF_KEEPALIVEPING | WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE

static struct watchdog_info mt65xx_wdt_ident = {
	.options          = OPTIONS,
	.firmware_version =	0,
	.identity         =	"MT6573 Watchdog",
};

static int mt65xx_wdt_ioctl(struct inode *inode, struct file *file,
							unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_TimeOutValue;

	printk("******** mt6573 WDT driver ioctl Cmd<%d>!! ********\n",cmd);
	switch (cmd) {
		default:
			return -ENOIOCTLCMD;

		case WDIOC_GETSUPPORT:
			return copy_to_user(argp, &mt65xx_wdt_ident, sizeof(mt65xx_wdt_ident)) ? -EFAULT : 0;

		case WDIOC_GETSTATUS:
		case WDIOC_GETBOOTSTATUS:
			return put_user(0, p);

		case WDIOC_KEEPALIVE:
			mt6573_wdt_Restart();
			return 0;

		case WDIOC_SETTIMEOUT:
			if( get_user(new_TimeOutValue, p) )
				return -EFAULT;
			
			mt6573_wdt_SetTimeOutValue(new_TimeOutValue);

			g_lastTime_TimeOutValue = new_TimeOutValue;
			g_WDT_enable = 1;

			if (g_wdt_mode == WDT_EXP_MODE)
				mt6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);	        
			else
				mt6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);

			mt6573_wdt_Restart();
			
			//why not just retrun new_TimeOutValue or g_lastTime_TimeOutValue.
			return put_user(timeout >> 11, p);

		case WDIOC_GETTIMEOUT:
			return put_user(timeout >> 11, p);
	}

	return 0;
}


static struct file_operations mt65xx_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.open		= mt65xx_wdt_open,
	.release	= mt65xx_wdt_release,
	.write		= mt65xx_wdt_write,
	.ioctl		= mt65xx_wdt_ioctl,
};

static int mt65xx_wdt_probe(struct platform_device *dev)
{
	int ret;
	
	printk("******** mt6573 WDT driver probe!! ********\n" );
		
	mt65xx_irq_set_sens(MT6573_APWDT_IRQ_LINE, MT65xx_EDGE_SENSITIVE);		
	ret = request_irq(MT6573_APWDT_IRQ_LINE, (irq_handler_t)mt6573_wdt_ISR, 0, "mt6573_watchdog", NULL);
	if(ret != 0)
	{
		printk( "mt65xx_wdt_probe : Failed to request irq (%d)\n", ret);
		return ret;
	}
	printk("mt65xx_wdt_probe : Success to request irq\n");

#ifdef CONFIG_MTK_WD_KICKER
	wk_register_wdt(&mt6573_wk_wdt);
#endif

	return 0;
}

static int mt65xx_wdt_remove(struct platform_device *dev)
{
	printk("******** mt6573 WDT driver remove!! ********\n" );
	free_irq(MT6573_APWDT_IRQ_LINE, NULL);
		
	return 0;
}

static void mt65xx_wdt_shutdown(struct platform_device *dev)
{
	printk("******** mt6573 WDT driver shutdown!! ********\n" );

	mt6573_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	mt6573_wdt_Restart();
}

static int mt65xx_wdt_suspend(struct platform_device *dev, pm_message_t state)
{
	printk("******** mt6573 WDT driver suspend!! ********\n" );
	
	mt6573_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	mt6573_wdt_Restart();
	
	return 0;
}

static int mt65xx_wdt_resume(struct platform_device *dev)
{
	printk("******** mt6573 WDT driver resume!! ********\n" );
	
	if ( g_WDT_enable == 1 ) 
	{
		mt6573_wdt_SetTimeOutValue(g_lastTime_TimeOutValue);
		
		if (g_wdt_mode == WDT_EXP_MODE)
			mt6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);
		else
		{
#if WDT_WORKAROUD // for workaround HW issue: WDT can NOT reboot
        	mt6573_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);
#else
        	mt6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
#endif	
		}

		mt6573_wdt_Restart();
	}
	
	return 0;
}

static struct platform_driver mt65xx_wdt_driver =
{
    .driver     = {
      .name		= "mt6573-wdt",
    },
    .probe		= mt65xx_wdt_probe,
    .remove		= mt65xx_wdt_remove,
    .shutdown	= mt65xx_wdt_shutdown,
    .suspend	= mt65xx_wdt_suspend,
    .resume		= mt65xx_wdt_resume,
};

struct platform_device mt6573_device_wdt = {
		.name				= "mt6573-wdt",
		.id					= 0,
		.dev				= {
		}
};

static int __init mt65xx_wdt_init(void)
{
	struct class_device *class_dev = NULL;
	int ret;
	
	ret = platform_device_register(&mt6573_device_wdt);
	if (ret) {
		printk("****[mt65xx_wdt_driver] Unable to device register(%d)\n", ret);
		return ret;
	}

	ret = platform_driver_register(&mt65xx_wdt_driver);
	if (ret) {
		printk("****[mt65xx_wdt_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}

	ret = alloc_chrdev_region(&wdt_devno, 0, 1, WDT_DEVNAME);
	if (ret) 
		printk("Error: Can't Get Major number for mt6573 WDT \n");
	
	wdt_cdev = cdev_alloc();
	wdt_cdev->owner = THIS_MODULE;
	wdt_cdev->ops = &mt65xx_wdt_fops;
	
	ret = cdev_add(wdt_cdev, wdt_devno, 1);
	if(ret)
	    printk("mt6573 wdt Error: cdev_add\n");
	
	wdt_major = MAJOR(wdt_devno);
	wdt_class = class_create(THIS_MODULE, WDT_DEVNAME);
	class_dev = (struct class_device *)device_create(wdt_class, 
													NULL, 
													wdt_devno, 
													NULL, 
													WDT_DEVNAME);

	return 0;	
}

static void __exit mt65xx_wdt_exit (void)
{
}

module_init(mt65xx_wdt_init);
module_exit(mt65xx_wdt_exit);

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("MT6573 Watchdog Device Driver");
MODULE_LICENSE("GPL");
