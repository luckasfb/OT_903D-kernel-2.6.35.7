
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/rtpm_prio.h>
#include "wd_kicker.h"

#define PFX "wdk: "
#define DEBUG_WDK	0
#if DEBUG_WDK
#define dbgmsg(msg...) printk(PFX msg)
#else
#define dbgmsg(...)
#endif

#define msg(msg...) printk(KERN_INFO PFX msg);
#define warnmsg(msg...) printk(KERN_WARNING PFX msg);
#define errmsg(msg...) printk(KERN_WARNING PFX msg);

#define MIN_KICK_INTERVAL	 1
#define MAX_KICK_INTERVAL	30
#define PROC_WK "wdk"


static int kwdt_thread(void *arg);
static int start_kicker(void);

static int debug_sleep = 0;
static int data;
static spinlock_t lock = SPIN_LOCK_UNLOCKED;

//Monkey.QHQ
//static struct task_struct *wk_tsk;
struct task_struct *wk_tsk;
//Monkey.QHQ

enum wk_wdt_mode g_wk_wdt_mode = WK_WDT_NORMAL_MODE;
static struct wk_wdt *g_wk_wdt = NULL;
static int g_kinterval = -1;
static int g_timeout = -1;
static int g_need_config = 0;

static char cmd_buf[256];

static int wk_proc_cmd_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int len;
	len = snprintf(buf, count, "mode interval timeout \n%-4d  %-9d %-7d\n", g_wk_wdt_mode, g_kinterval, g_timeout);
	return len;
}

static int wk_proc_cmd_write(struct file *file, const char *buf, unsigned long count, void *data)
{
	int ret;
	static int wdt_start = 0;
	int timeout;
	int mode;
	int kinterval;

	if (count == 0)
		return -1;

	if(count > 255) 
		count = 255;

	ret = copy_from_user(cmd_buf, buf, count);
	if (ret < 0)
		return -1;
	
	cmd_buf[count] = '\0';

	dbgmsg("Write %s\n", cmd_buf);

	sscanf(cmd_buf, "%d %d %d %d", &mode, &kinterval, &timeout, &debug_sleep);

	dbgmsg("mode=%d interval=%d timeout=%d\n", mode, kinterval, timeout);

	if (timeout < kinterval) {
		errmsg("The interval(%d) value should be smaller than timeout value(%d)\n", kinterval, timeout);
		return -1;
	}

	if ((timeout <  MIN_KICK_INTERVAL) || (timeout > MAX_KICK_INTERVAL)) {
		errmsg("The timeout(%d) is invalid (%d - %d)\n", kinterval, MIN_KICK_INTERVAL, MAX_KICK_INTERVAL);
		return -1;
	}

	if ((kinterval <  MIN_KICK_INTERVAL) || (kinterval > MAX_KICK_INTERVAL)) {
		errmsg("The interval(%d) is invalid (%d - %d)\n",kinterval,  MIN_KICK_INTERVAL, MAX_KICK_INTERVAL);
		return -1;
	}

	if (!((mode == WK_WDT_NORMAL_MODE) || 
	      (mode == WK_WDT_EXP_MODE))) {
		errmsg("Tha watchdog kicker wdt mode is not correct\n");
		errmsg("WK_WDT_NORMAL_MODE = %d\n", WK_WDT_NORMAL_MODE);
		errmsg("WK_WDT_EXP_MODE = %d\n", WK_WDT_EXP_MODE);
		return -1;
	}
		
		
    	spin_lock(&lock);

	g_kinterval = kinterval;
#if 0 //ifdef CONFIG_MTK_AEE_FEATURE AEE FEATURE auto enable/disable
	g_wk_wdt_mode = WK_WDT_EXP_MODE;
	msg("Enable wdt with interrupt mode only %d\n", g_wk_wdt_mode);
#else
	g_wk_wdt_mode = mode;
#endif
	g_timeout = timeout;
        g_need_config = 1;

	/* Start once only */
	if (wdt_start == 0) {
		start_kicker();
	}

	wdt_start = 1;

    	spin_unlock(&lock);
	return count;
}

int wk_proc_init(void) {

	struct proc_dir_entry *de = create_proc_entry(PROC_WK, 0667, 0);

	msg("Initialize watchdog kicker\n");

	de->read_proc = wk_proc_cmd_read;
	de->write_proc = wk_proc_cmd_write;

	return 0 ;
}


void wk_proc_exit(void) 
{

	remove_proc_entry(PROC_WK, NULL);

}

int wk_register_wdt(struct wk_wdt *wk_wdt)
{
	if (!wk_wdt) {
		BUG();
	}

	g_wk_wdt = wk_wdt;

	return 0;
}
EXPORT_SYMBOL(wk_register_wdt);

static int kwdt_thread(void *arg)
{
	
	struct sched_param param = { .sched_priority = RTPM_PRIO_WDT};

        sched_setscheduler(current, SCHED_FIFO, &param);

        set_current_state(TASK_INTERRUPTIBLE);

	dbgmsg("WDT kicker thread start\n");

	for(;;)
	{
		if (kthread_should_stop()) break;

		msleep(g_kinterval * 1000);

		if (g_wk_wdt && g_wk_wdt->kick_wdt && g_wk_wdt->config) {
			
			if (g_need_config) {
				spin_lock(&lock);
				g_wk_wdt->config(g_wk_wdt_mode, g_timeout);
				g_need_config = 0;
				spin_unlock(&lock);
			}
			g_wk_wdt->kick_wdt();
		}
		else {
			errmsg("No watch dog driver is hooked\n");
			BUG();
		}

		#if (DEBUG_WDK==1)
		msleep(debug_sleep * 1000);
		dbgmsg("WD kicker woke up %d\n", debug_sleep);
		#endif

	}

	return 0;
}	

static int start_kicker(void)
{
	spin_unlock(&lock);
	wk_tsk = kthread_create(kwdt_thread, &data, "wdtk");
	spin_lock(&lock);
	if (IS_ERR(wk_tsk)) {
		int ret = PTR_ERR(wk_tsk);
		wk_tsk = NULL;
		return ret;
	}
	wake_up_process(wk_tsk);

	return 0;
}

static int __init init_wk(void)
{
	
	wk_proc_init();

	return 0;
}

static void __exit exit_wk(void)
{
	wk_proc_exit();
	kthread_stop(wk_tsk);
}

late_initcall(init_wk);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mediatek inc.");
MODULE_DESCRIPTION("The watchdog kicker");
