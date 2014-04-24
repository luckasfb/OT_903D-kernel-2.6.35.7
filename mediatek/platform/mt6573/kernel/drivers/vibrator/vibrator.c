

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/workqueue.h>

#include "timed_output.h"
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>

#include <linux/jiffies.h>
#include <linux/timer.h>

#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_gpt.h>

#define VERSION					        "v 0.1"
#define VIB_DEVICE	        			"mt6573_vibrator"

#define COUNT_DOWN_TIME					50

#define VIBR_HRTIMER

#ifndef VIBR_HRTIMER
XGPT_NUM Vibrator_XGPT = XGPT7;
#endif

#define RSUCCESS        0


/* Debug message event */
#define DBG_EVT_NONE		0x00000000	/* No event */
#define DBG_EVT_INT			0x00000001	/* Interrupt related event */
#define DBG_EVT_TASKLET		0x00000002	/* Tasklet related event */

#define DBG_EVT_ALL			0xffffffff
 
#define DBG_EVT_MASK      	(DBG_EVT_TASKLET)

#if 1
#define MSG(evt, fmt, args...) \
do {	\
	if ((DBG_EVT_##evt) & DBG_EVT_MASK) { \
		printk(fmt, ##args); \
	} \
} while(0)

#define MSG_FUNC_ENTRY(f)	MSG(FUC, "<FUN_ENT>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...) do{}while(0)
#define MSG_FUNC_ENTRY(f)	   do{}while(0)
#endif

#define VIBR_CON0 ((volatile unsigned long*)(0xF702F7B0))

static int vibr_Enable(void)
{
    printk("[vibrator]vibr_Enable \n");
	hwPowerOn(MT65XX_POWER_LDO_VIBR, VOL_2800 , "VIBR");
	return 0;
}

static int vibr_Disable(void)
{
    while((INREG32(VIBR_CON0)&1))
    {
        printk("[vibrator]vibr_Disable \n");
	hwPowerDown(MT65XX_POWER_LDO_VIBR , "VIBR");
        //printk("[vibrator]vibr_Disable:VIBR_CON0=0x%x \r\n", INREG32(VIBR_CON0));
    }
	
	return 0;
}


//static struct work_struct vibrator_work;
static struct hrtimer vibe_timer;
static spinlock_t vibe_lock;


static int vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) 
	{
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} 
	else
		return 0;
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
        unsigned long   flags;

	spin_lock_irqsave(&vibe_lock, flags);

        #ifdef VIBR_HRTIMER
	while(hrtimer_cancel(&vibe_timer))
        {
            printk("[vibrator]vibrator_enable: try to cancel hrtimer \n");
        }
	#else
        XGPT_Reset(Vibrator_XGPT);
        #endif

	if (value == 0)
        {      
            printk("[vibrator]vibrator_enable: disable \n");
            vibr_Disable();
          
        }
	else 
	{
			value = ((value > 15000) ? 15000 : value);
            printk("[vibrator]vibrator_enable: vibrator start: %d \n", value); 
         
            #ifdef VIBR_HRTIMER
	    hrtimer_start(&vibe_timer, 
			ktime_set(value / 1000, (value % 1000) * 1000000),
			HRTIMER_MODE_REL);

            #else
            XGPT_CONFIG config;
			config.num = Vibrator_XGPT;
            config.clkDiv = 0;
            config.mode = XGPT_ONE_SHOT;
            config.bIrqEnable = TRUE;
            config.u4Compare = value*32768/1000;
            
            if(!XGPT_Config(config))
            {
                printk("[vibrator]vibrator_enable: config XGPT: %d fail!\n", value); 
            }
            
            XGPT_Start(Vibrator_XGPT);
            #endif
           
            vibr_Enable();
           
	}
	spin_unlock_irqrestore(&vibe_lock, flags);

}


#ifdef VIBR_HRTIMER
static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
     printk("[vibrator]vibrator_timer_func: vibrator will disable \n");

    vibr_Disable();
            
 	return HRTIMER_NORESTART;
}

#else
void vibrator_timer_func(UINT16 temp)
{
    printk("[vibrator]vibrator_timer_func: vibrator will disable \n");

    vibr_Disable();
}
#endif


static struct timed_output_dev mt6573_vibrator = 
{
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

static int vib_probe(struct platform_device *pdev)
{
	return 0;
}

static int vib_remove(struct platform_device *pdev)
{
	return 0;
}

static void vib_shutdown(struct platform_device *pdev)
{
		vibr_Disable();

}
static struct platform_driver vibrator_driver = 
{
    .probe		= vib_probe,
	.remove	    = vib_remove,
    .shutdown = vib_shutdown,
    .driver     = {
    .name = VIB_DEVICE,
    },
};

static ssize_t store_vibr_on(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	if(buf != NULL && size != 0)
	{
		printk("[vibrator]buf is %s and size is %d \n",buf,size);
		if(buf[0]== '0')
		{
			vibr_Disable();
		}else
		{
			vibr_Enable();
		}
	}
	return size;
}

static DEVICE_ATTR(vibr_on, 0664, NULL, store_vibr_on);


static s32 __devinit vib_mod_init(void)
{	
	s32 ret;

	printk("MediaTek MT6573 vibrator driver register, version %s\n", VERSION);

	spin_lock_init(&vibe_lock);
    
        #ifdef VIBR_HRTIMER
	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

        #else
        XGPT_Init(Vibrator_XGPT, vibrator_timer_func);
        #endif
    
	timed_output_dev_register(&mt6573_vibrator);

        ret = platform_driver_register(&vibrator_driver);

        if(ret) 
        {
		printk("[vibrator]Unable to register vibrator driver (%d)\n", ret);
		return ret;
        }	

	ret = device_create_file(mt6573_vibrator.dev,&dev_attr_vibr_on);
    if(ret)
    {
        printk("[vibrator]device_create_file vibr_on fail! \n");
    }
    
	printk("[vibrator]vib_mod_init Done \n");
 
    return RSUCCESS;
}

 
static void __exit vib_mod_exit(void)
{
	printk("MediaTek MT6573 vibrator driver unregister, version %s \n", VERSION);
	printk("[vibrator]vib_mod_exit Done \n");
}

module_init(vib_mod_init);
module_exit(vib_mod_exit);
MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("MT6573 Vibrator Driver (VIB)");
MODULE_LICENSE("GPL");

