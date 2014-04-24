
#include <asm/arch/custom/inc/cust_leds.h>
#include <asm/arch/mt6573_pwm.h>

 int Cust_SetBacklight_uboot(int level)
{
if(level)
{
		mt_set_gpio_mode(100,0);  // gpio mode
		mt_set_gpio_pull_enable(100,1);
		//---
		mt_set_gpio_dir(100,1); // output
		mt_set_gpio_out(100,1); // high
}
else
	{
		mt_set_gpio_mode(100,0);  // gpio mode
		mt_set_gpio_pull_enable(100,1);
		//---
		mt_set_gpio_dir(100,1); // output
		mt_set_gpio_out(100,0); // high
}

}

//extern int DISP_SetBacklight(int level);

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_PWM, PWM1},
	{"green",             MT65XX_LED_MODE_NONE, -1},
	{"blue",              MT65XX_LED_MODE_NONE, -1},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1},
	{"button-backlight",  MT65XX_LED_MODE_NONE, -1},
	{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)Cust_SetBacklight_uboot},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}
