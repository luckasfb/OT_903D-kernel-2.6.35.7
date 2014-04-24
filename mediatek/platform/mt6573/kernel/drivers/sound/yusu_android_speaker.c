


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <mach/mt6573_gpio.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <mach/mt6573_typedefs.h>
#include <linux/pmic6326_sw.h>
#include <linux/delay.h>
#include <mach/mt6573_pll.h>
#include "yusu_android_speaker.h"

//#define CONFIG_DEBUG_MSG
#ifdef CONFIG_DEBUG_MSG
#define PRINTK(format, args...) printk( KERN_EMERG format,##args )
#else
#define PRINTK(format, args...)
#endif


#define EXTERNAL_AMP_GPIO (104)



extern void Yusu_Sound_AMP_Switch(BOOL enable);

bool Speaker_Init(void)
{
    printk("Speaker Init Success");
    return true;
}

void Sound_SpeakerL_SetVolLevel(int level)
{
   PRINTK(" Sound_SpeakerL_SetVolLevel level=%d\n",level);
}

void Sound_SpeakerR_SetVolLevel(int level)
{
   PRINTK(" Sound_SpeakerR_SetVolLevel level=%d\n",level);
}

void Sound_Speaker_Turnon(int channel)
{
    printk("Sound_Speaker_Turnon channel = %d\n",channel);
    printk("Sound_Speaker_Turnon Speaker_Volume = %d\n",Speaker_Volume);

    mt_set_gpio_dir(EXTERNAL_AMP_GPIO,true); // set GPIO 104
    mt_set_gpio_out(EXTERNAL_AMP_GPIO,true);
}

void Sound_Speaker_Turnoff(int channel)
{
    printk("Sound_Speaker_Turnoff channel = %d\n",channel);

    mt_set_gpio_dir(EXTERNAL_AMP_GPIO,false); // set GPIO 104
    mt_set_gpio_out(EXTERNAL_AMP_GPIO,false);    
}

void Sound_Speaker_SetVolLevel(int level)
{
    Speaker_Volume =level;

}


void Sound_Headset_Turnon(void)
{

}
void Sound_Headset_Turnoff(void)
{

}


