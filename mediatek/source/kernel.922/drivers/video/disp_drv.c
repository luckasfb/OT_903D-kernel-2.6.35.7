

#include <linux/delay.h>

#if defined(CONFIG_ARCH_MT6516)
	#include <mach/mt6516_typedefs.h>
	#include <mach/mt6516_gpio.h>
	#include <mach/mt6516_ap_config.h>
	#include <mach/mt6516_boot.h>
#elif defined(CONFIG_ARCH_MT6573)
	#include <mach/mt6573_typedefs.h>
	#include <mach/mt6573_gpio.h>
	#include <mach/mt6573_boot.h>
	#include <mach/mt6573_m4u.h>
#else
	#error "unknown arch"
#endif    

#include "disp_drv.h"
#include "lcd_drv.h"
#include "lcm_drv.h"
#include "dpi_drv.h"
#include "dsi_drv.h"

#include <linux/disp_assert_layer.h>

#if defined(MTK_TVOUT_SUPPORT)
#include "tv_out.h"
#include <linux/interrupt.h>
#endif

#include <linux/semaphore.h>
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static const DISP_DRIVER *disp_drv = NULL;
LCM_DRIVER  *lcm_drv = NULL;
static LCM_PARAMS s_lcm_params= {0};
LCM_PARAMS *lcm_params= &s_lcm_params;
static LCD_IF_ID ctrl_if = LCD_IF_PARALLEL_0;

static volatile int direct_link_layer = -1;

DECLARE_MUTEX(sem_update_screen);
static BOOL is_engine_in_suspend_mode = FALSE;
static BOOL is_lcm_in_suspend_mode    = FALSE;
static BOOL isLCMFound 					= FALSE;
extern unsigned int DAL_layerPA;

static size_t disp_log_on = false;
#define DISP_LOG(...) \
    do { \
        if (disp_log_on) printk("[disp log]"__VA_ARGS__); \
    }while (0)

#define DISP_FUNC()	\
	do { \
		if(disp_log_on) printk("[disp log] %s\n", __func__); \
	}while (0)

void disp_log_enable(bool enable)
{
	printk("disp log %s\n", enable?"enabled":"disabled");
	disp_log_on = enable;
}
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

static void lcm_set_reset_pin(UINT32 value)
{
	LCD_SetResetSignal(value);
}

static void lcm_udelay(UINT32 us)
{
	udelay(us);
}

static void lcm_mdelay(UINT32 ms)
{
	msleep(ms);
}

static void lcm_send_cmd(UINT32 cmd)
{
	if(lcm_params== NULL)
		return;

    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_LOW,
                              cmd, lcm_params->dbi.cpu_write_bits));
}

static void lcm_send_data(UINT32 data)
{
	if(lcm_params== NULL)
		return;

    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_HIGH,
                              data, lcm_params->dbi.cpu_write_bits));
}

static UINT32 lcm_read_data(void)
{
	UINT32 data = 0;

	if(lcm_params== NULL)
		return 0;

	ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
			LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

	LCD_CHECK_RET(LCD_ReadIF(ctrl_if, LCD_IF_A0_HIGH,
				&data, lcm_params->dbi.cpu_write_bits));

	return data;
}



static __inline LCD_IF_WIDTH to_lcd_if_width(LCM_DBI_DATA_WIDTH data_width)
{
    switch(data_width)
    {
 	   	case LCM_DBI_DATA_WIDTH_8BITS  : return LCD_IF_WIDTH_8_BITS;
    	case LCM_DBI_DATA_WIDTH_9BITS  : return LCD_IF_WIDTH_9_BITS;
    	case LCM_DBI_DATA_WIDTH_16BITS : return LCD_IF_WIDTH_16_BITS;
    	case LCM_DBI_DATA_WIDTH_18BITS : return LCD_IF_WIDTH_18_BITS;
    	case LCM_DBI_DATA_WIDTH_24BITS : return LCD_IF_WIDTH_24_BITS;
    	default : ASSERT(0);
    }

    return LCD_IF_WIDTH_18_BITS;
}

static void disp_drv_set_driving_current(LCM_PARAMS *lcm)
{
	LCD_Set_DrivingCurrent(lcm);
}

static void disp_drv_init_io_pad(LCM_PARAMS *lcm)
{
	LCD_Init_IO_pad(lcm);
}

static void disp_drv_init_ctrl_if(void)
{
	const LCM_DBI_PARAMS *dbi = NULL;

	if(lcm_params== NULL)
		return;

	dbi = &(lcm_params->dbi);
	switch(lcm_params->ctrl)
	{
		case LCM_CTRL_NONE :
		case LCM_CTRL_GPIO : return;

		case LCM_CTRL_SERIAL_DBI :
							 ASSERT(dbi->port <= 1);
							 ctrl_if = LCD_IF_SERIAL_0 + dbi->port;
							 LCD_ConfigSerialIF(ctrl_if,
									 (LCD_IF_SERIAL_BITS)dbi->data_width,
									 dbi->serial.clk_polarity,
									 dbi->serial.clk_phase,
									 dbi->serial.cs_polarity,
									 dbi->serial.clock_base,
									 dbi->serial.clock_div,
									 dbi->serial.is_non_dbi_mode);
							 break;

		case LCM_CTRL_PARALLEL_DBI :
							 ASSERT(dbi->port <= 2);
							 ctrl_if = LCD_IF_PARALLEL_0 + dbi->port;
							 LCD_ConfigParallelIF(ctrl_if,
									 (LCD_IF_PARALLEL_BITS)dbi->data_width,
									 (LCD_IF_PARALLEL_CLK_DIV)dbi->clock_freq,
									 dbi->parallel.write_setup,
									 dbi->parallel.write_hold,
									 dbi->parallel.write_wait,
									 dbi->parallel.read_setup,
									 dbi->parallel.read_latency,
									 dbi->parallel.wait_period);
							 break;

		default : ASSERT(0);
	}

	LCD_CHECK_RET(LCD_SelectWriteIF(ctrl_if));

	LCD_CHECK_RET(LCD_ConfigIfFormat(dbi->data_format.color_order,
				dbi->data_format.trans_seq,
				dbi->data_format.padding,
				dbi->data_format.format,
				to_lcd_if_width(dbi->data_format.width)));
}

static const LCM_UTIL_FUNCS lcm_utils =
{
	.set_reset_pin      = lcm_set_reset_pin,
	.set_gpio_out       = mt_set_gpio_out,
	.udelay             = lcm_udelay,
	.mdelay             = lcm_mdelay,
	.send_cmd           = lcm_send_cmd,
	.send_data          = lcm_send_data,
	.read_data          = lcm_read_data,
	.dsi_set_cmdq		= DSI_set_cmdq,
	.dsi_write_cmd		= DSI_write_lcm_cmd,
	.dsi_write_regs 	= DSI_write_lcm_regs,
	.dsi_read_reg		= DSI_read_lcm_reg,
	
	/** FIXME: GPIO mode should not be configured in lcm driver
	  REMOVE ME after GPIO customization is done    
	 */
	.set_gpio_mode        = mt_set_gpio_mode,
	.set_gpio_dir         = mt_set_gpio_dir,
	.set_gpio_pull_enable = mt_set_gpio_pull_enable
};



extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
LCM_DRIVER *disp_drv_get_lcm_driver(const char *lcm_name)
{
	LCM_DRIVER *lcm = NULL;
	printk("[LCM Auto Detect], we have %d lcm drivers built in\n", lcm_count);
	printk("[LCM Auto Detect], try to find driver for [%s]\n", 
			(lcm_name==NULL)?"unknown":lcm_name);

	if(lcm_count ==1)
	{
		// we need to verify whether the lcm is connected
		// even there is only one lcm type defined
		lcm = lcm_driver_list[0];
		lcm->set_util_funcs(&lcm_utils);
		lcm->get_params(&s_lcm_params);

		lcm_params = &s_lcm_params;
		lcm_drv = lcm;
		disp_drv_init_ctrl_if();
		disp_drv_set_driving_current(lcm_params);
		disp_drv_init_io_pad(lcm_params);

		if(lcm_drv->compare_id)
		{
			if(lcm_drv->compare_id() == TRUE)
			{
				printk("[LCM Specified] compare id success\n");
				isLCMFound = TRUE;
			}
			else
			{
				printk("[LCM Specified] compare id fail\n");
				printk("%s, lcm is not connected\n", __func__);
			}
		}
		else
		{
			printk("[LCM Specified] don't support read deivce id\n");
		}

        printk("[LCM Specified]\t[%s]\n", (lcm->name==NULL)?"unknown":lcm->name);

		goto done;
	}
	else
	{
		int i;

		for(i = 0;i < lcm_count;i++)
		{
			lcm_params = &s_lcm_params;
			lcm = lcm_driver_list[i];

			printk("[LCM Auto Detect] [%d] - [%s]\t", i, (lcm->name==NULL)?"unknown":lcm->name);

			lcm->set_util_funcs(&lcm_utils);
			memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
			lcm->get_params(lcm_params);

			disp_drv_init_ctrl_if();
			disp_drv_set_driving_current(lcm_params);
			disp_drv_init_io_pad(lcm_params);

			if(lcm_name != NULL)
			{
				if(!strcmp(lcm_name,lcm->name))
				{
					printk("\t\t[success]\n");
					isLCMFound = TRUE;
					lcm_drv = lcm;

					goto done;
				}
				else
				{
					printk("\t\t[fail]\n");
				}
			}
			else 
			{
				if(lcm->compare_id != NULL && lcm->compare_id())
				{
					printk("\t\t[success]\n");
					isLCMFound = TRUE;
					lcm_drv = lcm;

					goto done;
				}
				else
				{
					printk("\t\t[fail]\n");
				}
			}
		}
	}
done:
	return lcm_drv;
}


static void disp_dump_lcm_parameters(LCM_PARAMS *lcm_params)
{
	unsigned char *LCM_TYPE_NAME[] = {"DBI", "DPI", "DSI"};
	unsigned char *LCM_CTRL_NAME[] = {"NONE", "SERIAL", "PARALLEL", "GPIO"};

	if(lcm_params == NULL)
		return;

	printk("[mtkfb] LCM TYPE: %s\n", LCM_TYPE_NAME[lcm_params->type]);
	printk("[mtkfb] LCM INTERFACE: %s\n", LCM_CTRL_NAME[lcm_params->ctrl]);
	printk("[mtkfb] LCM resolution: %d x %d\n", lcm_params->width, lcm_params->height);

	return;
}

char disp_lcm_name[256] = {0};
BOOL disp_get_lcm_name_boot(char *cmdline)
{
	BOOL ret = FALSE;
	char *p, *q;

	p = strstr(cmdline, "lcm=");
	if(p == NULL)
	{
		// we can't find lcm string in the command line, 
		// the uboot should be old version, or the kernel is loaded by ICE debugger
		return DISP_SelectDeviceBoot(NULL);
	}

	p += 4;
	if((p - cmdline) > strlen(cmdline+1))
	{
		ret = FALSE;
		goto done;
	}

	isLCMFound = strcmp(p, "0");
	printk("[mtkfb] LCM is %sconnected\n", ((isLCMFound)?"":"not "));
	p += 2;
	q = p;
	while(*q != ' ' && *q != '\0')
		q++;

	memset((void*)disp_lcm_name, 0, sizeof(disp_lcm_name));
	strncpy((char*)disp_lcm_name, (const char*)p, (int)(q-p));

	if(DISP_SelectDeviceBoot(disp_lcm_name))
		ret = TRUE;

done:
	return ret;
}

static BOOL disp_drv_init_context(void)
{
	if (disp_drv != NULL && lcm_drv != NULL){
		return TRUE;
	}

	disp_drv_init_ctrl_if();

	switch(lcm_params->type)
	{
		case LCM_TYPE_DBI : disp_drv = DISP_GetDriverDBI(); break;
		case LCM_TYPE_DPI : disp_drv = DISP_GetDriverDPI(); break;
		case LCM_TYPE_DSI : disp_drv = DISP_GetDriverDSI(); break;
		default : ASSERT(0);
	}

	if (!disp_drv) return FALSE;

	return TRUE;
}

BOOL DISP_IsLCDBusy(void)
{
	return LCD_IsBusy();
}

BOOL DISP_IsLcmFound(void)
{
	return isLCMFound;
}

BOOL DISP_IsContextInited(void)
{
	if(lcm_params && disp_drv && lcm_drv)
		return TRUE;
	else
		return FALSE;
}

BOOL DISP_SelectDeviceBoot(const char* lcm_name)
{
	LCM_DRIVER *lcm = NULL;
	int i;

	printk("%s\n", __func__);
	if(lcm_name == NULL)
	{
		// we can't do anything in boot stage if lcm_name is NULL
		return false;
	}
	for(i = 0;i < lcm_count;i++)
	{
		lcm_params = &s_lcm_params;
		lcm = lcm_driver_list[i];

		printk("[LCM Auto Detect] [%d] - [%s]\t", 
			i, 
			(lcm->name==NULL)?"unknown":lcm->name);

		lcm->set_util_funcs(&lcm_utils);
		memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
		lcm->get_params(lcm_params);

		// if lcm type is speficied, we don't need to compare the lcm name
		// in case the uboot is old version, which don't have lcm name in command line
		if(lcm_count == 1)
		{
			lcm_drv = lcm;
			isLCMFound = TRUE;
			break;
		}

		if(!strcmp(lcm_name,lcm->name))
		{
			printk("\t\t[success]\n");
			lcm_drv = lcm;
			isLCMFound = TRUE;

			break;
		}
		else
		{
			printk("\t\t[fail]\n");
		}
	}

	if (NULL == lcm_drv)
	{
		printk("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	switch(lcm_params->type)
	{
		case LCM_TYPE_DBI : disp_drv = DISP_GetDriverDBI(); break;
		case LCM_TYPE_DPI : disp_drv = DISP_GetDriverDPI(); break;
		case LCM_TYPE_DSI : disp_drv = DISP_GetDriverDSI(); break;
		default : ASSERT(0);
	}

	disp_dump_lcm_parameters(lcm_params);
	return TRUE;
}

BOOL DISP_SelectDevice(const char* lcm_name)
{
	LCD_STATUS ret;

	ret = LCD_Init();
	printk("ret of LCD_Init() = %d\n", ret);

	lcm_drv = disp_drv_get_lcm_driver(lcm_name);
	if (NULL == lcm_drv)
	{
		printk("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	disp_dump_lcm_parameters(lcm_params);
	return disp_drv_init_context();
}

BOOL DISP_DetectDevice(void)
{
	LCD_STATUS ret;

	ret = LCD_Init();
	printk("ret of LCD_Init() = %d\n", ret);

	lcm_drv = disp_drv_get_lcm_driver(NULL);
	if (NULL == lcm_drv)
	{
		printk("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	disp_dump_lcm_parameters(lcm_params);
	return disp_drv_init_context();
}

// ---------------------------------------------------------------------------
//  DISP Driver Implementations
// ---------------------------------------------------------------------------

DISP_STATUS DISP_Init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
	DISP_STATUS r = DISP_STATUS_OK;


	// LCD_Init() was moved into context init
	/* power on LCD before config its registers*/
	//LCD_CHECK_RET(LCD_Init());

	if (!disp_drv_init_context()) {
		return DISP_STATUS_NOT_IMPLEMENTED;
	}

	disp_drv_init_ctrl_if();

	r = (disp_drv->init) ?
		(disp_drv->init(fbVA, fbPA, isLcmInited)) :
		DISP_STATUS_NOT_IMPLEMENTED;

	return r;
}


DISP_STATUS DISP_Deinit(void)
{
	DISP_CHECK_RET(DISP_PanelEnable(FALSE));
	DISP_CHECK_RET(DISP_PowerEnable(FALSE));

	return DISP_STATUS_OK;
}

// -----

DISP_STATUS DISP_PowerEnable(BOOL enable)
{
    DISP_STATUS ret = DISP_STATUS_OK;


	if (down_interruptible(&sem_update_screen)) {
		printk("[DISP] ERROR: Can't get sem_update_screen in DISP_PowerEnable()\n");
		return DISP_STATUS_ERROR;
	}

	disp_drv_init_context();

	is_engine_in_suspend_mode = enable ? FALSE : TRUE;

	ret = (disp_drv->enable_power) ?
		(disp_drv->enable_power(enable)) :
		DISP_STATUS_NOT_IMPLEMENTED;

    if (enable) {
        DAL_OnDispPowerOn();
    }
	
	// If in direct link mode, re-start LCD after system resume
	//
	if (enable && -1 != direct_link_layer) {
		LCD_CHECK_RET(LCD_StartTransfer(FALSE));
	}

	up(&sem_update_screen);


	return ret;
}


DISP_STATUS DISP_PanelEnable(BOOL enable)
{
	static BOOL s_enabled = TRUE;
	DISP_STATUS ret = DISP_STATUS_OK;
	DISP_LOG("panel is %s\n", enable?"enabled":"disabled");

	if (down_interruptible(&sem_update_screen)) {
		printk("[DISP] ERROR: Can't get sem_update_screen in DISP_PanelEnable()\n");
		return DISP_STATUS_ERROR;
	}

	disp_drv_init_context();

	is_lcm_in_suspend_mode = enable ? FALSE : TRUE;

	if (!lcm_drv->suspend || !lcm_drv->resume) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	if (enable && !s_enabled) {
	
		if(NULL != lcm_drv->check_status)
		{
			if(lcm_drv->check_status() == 0)
			{
				printk("================================\n");
				printk("%s, LCM is not powered on!!\n", __func__);
				printk("%s, we will power on LCM again here\n", __func__);
				printk("================================\n");
				lcm_drv->init();
			}
			else
			{
				lcm_drv->resume();
			}
		}
		else
		{
			lcm_drv->resume();
		}

		s_enabled = TRUE;
	}
	else if (!enable && s_enabled)
	{
		LCD_CHECK_RET(LCD_WaitForNotBusy());
		if(lcm_params->type==LCM_TYPE_DSI)
			DSI_CHECK_RET(DSI_WaitForNotBusy());
		s_enabled = FALSE;
		lcm_drv->suspend();
	}

End:
	up(&sem_update_screen);

	return ret;
}

DISP_STATUS DISP_SetBacklight(UINT32 level)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	disp_drv_init_context();

	LCD_WaitForNotBusy();
	if(lcm_params->type==LCM_TYPE_DSI)
		DSI_CHECK_RET(DSI_WaitForNotBusy());

	if (!lcm_drv->set_backlight) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	lcm_drv->set_backlight(level);

End:
	return ret;
}

DISP_STATUS DISP_SetPWM(UINT32 divider)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	disp_drv_init_context();

	LCD_WaitForNotBusy();
	if(lcm_params->type==LCM_TYPE_DSI)
		DSI_CHECK_RET(DSI_WaitForNotBusy());

	if (!lcm_drv->set_pwm) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	lcm_drv->set_pwm(divider);
End:
	return ret;
}

DISP_STATUS DISP_GetPWM(UINT32 divider, unsigned int *freq)
{
	DISP_STATUS ret = DISP_STATUS_OK;
	
	disp_drv_init_context();

	if (!lcm_drv->get_pwm) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	*freq = lcm_drv->get_pwm(divider);

End:
	return ret;
}
// -----

DISP_STATUS DISP_SetFrameBufferAddr(UINT32 fbPhysAddr)
{
	disp_drv_init_context();

	return (disp_drv->set_fb_addr) ?
		(disp_drv->set_fb_addr(fbPhysAddr)) :
		DISP_STATUS_NOT_IMPLEMENTED;
}

// -----

static BOOL is_overlaying = FALSE;

DISP_STATUS DISP_EnterOverlayMode(void)
{
	DISP_FUNC();
	if (is_overlaying) {
		return DISP_STATUS_ALREADY_SET;
	} else {
		is_overlaying = TRUE;
	}

	return DISP_STATUS_OK;
}


DISP_STATUS DISP_LeaveOverlayMode(void)
{
	DISP_FUNC();
	if (!is_overlaying) {
		return DISP_STATUS_ALREADY_SET;
	} else {
		is_overlaying = FALSE;
	}

	return DISP_STATUS_OK;
}


// -----

DISP_STATUS DISP_EnableDirectLinkMode(UINT32 layer)
{
#if defined(CONFIG_ARCH_MT6516)
	if (layer != direct_link_layer) {
		LCD_CHECK_RET(LCD_LayerSetTriggerMode(layer, LCD_HW_TRIGGER_DIRECT_COUPLE));
		LCD_CHECK_RET(LCD_LayerSetHwTriggerSrc(layer, LCD_HW_TRIGGER_SRC_IBW2));
		LCD_CHECK_RET(LCD_EnableHwTrigger(TRUE));
		LCD_CHECK_RET(LCD_StartTransfer(FALSE));
		direct_link_layer = layer;
	}
#elif defined(CONFIG_ARCH_MT6573)
#else
#endif   
	return DISP_STATUS_OK;
}


DISP_STATUS DISP_DisableDirectLinkMode(UINT32 layer)
{
#if defined(CONFIG_ARCH_MT6516)
	if (layer == direct_link_layer) {
		LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));
		direct_link_layer = -1;
	}
	LCD_CHECK_RET(LCD_LayerSetTriggerMode(layer, LCD_SW_TRIGGER));
#elif defined(CONFIG_ARCH_MT6573)
#else
#endif   
	return DISP_STATUS_OK;
}

// -----

extern int MT6516IDP_EnableDirectLink(void);

DISP_STATUS DISP_UpdateScreen(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
	DISP_LOG("update screen, (%d,%d),(%d,%d)\n", x, y, width, height);
	if (down_interruptible(&sem_update_screen)) {
		printk("[DISP] ERROR: Can't get sem_update_screen in DISP_UpdateScreen()\n");
		return DISP_STATUS_ERROR;
	}

	// if LCM is powered down, LCD would never recieve the TE signal
	//
	if (is_lcm_in_suspend_mode || is_engine_in_suspend_mode) goto End;

	LCD_CHECK_RET(LCD_WaitForNotBusy());
	if(lcm_params->type==LCM_TYPE_DSI)
		DSI_CHECK_RET(DSI_WaitForNotBusy());

	if (lcm_drv->update) {
#ifdef MTK_QVGA_LANDSCAPE_SUPPORT 
	if((MTK_LCM_PHYSICAL_ROTATION == 90 || MTK_LCM_PHYSICAL_ROTATION == 270) && 
		(get_boot_mode() == FACTORY_BOOT || get_boot_mode() == RECOVERY_BOOT))
		{
       	 	lcm_drv->update(x, y, height, width);
		}
		else
		{
			lcm_drv->update(x, y, width, height);
		}
#else
		lcm_drv->update(x, y, width, height);
#endif
    }

    LCD_CHECK_RET(LCD_SetRoiWindow(x, y, width, height));

#ifdef MTK_QVGA_LANDSCAPE_SUPPORT 
	if((MTK_LCM_PHYSICAL_ROTATION == 90 || MTK_LCM_PHYSICAL_ROTATION == 270) && 
		(get_boot_mode() == FACTORY_BOOT || get_boot_mode() == RECOVERY_BOOT))
	{
		LCD_CHECK_RET(LCD_SetRoiWindow(x, y, height, width));
	}
#endif

	LCD_CHECK_RET(LCD_FBSetStartCoord(x, y));

	if (-1 != direct_link_layer) {
		//MT6516IDP_EnableDirectLink();     // FIXME
	} else {
#if defined(MTK_TVOUT_SUPPORT)
	if (!in_interrupt())
	{
		TVOUT_ScreenUpdateLock();
	}
#endif
		disp_drv->update_screen();
	}

End:
	up(&sem_update_screen);

	return DISP_STATUS_OK;
}


// ---------------------------------------------------------------------------
//  Retrieve Information
// ---------------------------------------------------------------------------

UINT32 DISP_GetScreenWidth(void)
{
	if(lcm_params)
	    return lcm_params->width;
	else
	{
		printk("WARNING!! get screen width before display driver inited!\n");
		return 0;
	}
}


UINT32 DISP_GetScreenHeight(void)
{
	if(lcm_params)
    	return lcm_params->height;
	else
	{
		printk("WARNING!! get screen height before display driver inited!\n");
		return 0;
	}
}


UINT32 DISP_GetScreenBpp(void)
{
	return 32;  // ARGB8888
}


UINT32 DISP_GetPages(void)
{
//#if defined(CONFIG_ARCH_MT6573)
//	return 4;
//#else
    return 2;   // Double Buffers
//#endif
}


BOOL DISP_IsDirectLinkMode(void)
{
	return (-1 != direct_link_layer) ? TRUE : FALSE;
}


BOOL DISP_IsInOverlayMode(void)
{
	return is_overlaying;
}


#define ALIGN_TO_POW_OF_2(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

UINT32 DISP_GetVRamSize(void)
{
	// Use a local static variable to cache the calculated vram size
	//    
	static UINT32 vramSize = 0;
	
    if (0 == vramSize)
    {
        disp_drv_init_context();

        vramSize = disp_drv->get_vram_size();
        
        // Align vramSize to 1MB
        //
        vramSize = ALIGN_TO_POW_OF_2(vramSize, 0x100000);

        printk("DISP_GetVRamSize: %u bytes\n", vramSize);
    }

    return vramSize;
}

UINT32 DISP_GetVRamSizeBoot(char *cmdline)
{
	static UINT32 vramSize = 0;

	if(vramSize)
	{
		return vramSize;
	}

	disp_get_lcm_name_boot(cmdline);

	// if can't get the lcm type from uboot, we will return 0x800000 for a safe value
	if(disp_drv)
	    vramSize = disp_drv->get_vram_size();
	else
	{
		printk("%s, can't get lcm type, reserved memory size will be set as 0x800000\n", __func__);
		return 0x800000;
    }   
    // Align vramSize to 1MB
    //
    vramSize = ALIGN_TO_POW_OF_2(vramSize, 0x100000);

    printk("DISP_GetVRamSizeBoot: %u bytes[%dMB]\n", vramSize, (vramSize>>20));

    return vramSize;
}



PANEL_COLOR_FORMAT DISP_GetPanelColorFormat(void)
{
	disp_drv_init_context();

	return (disp_drv->get_panel_color_format) ?
		(disp_drv->get_panel_color_format()) :
		DISP_STATUS_NOT_IMPLEMENTED;
}

UINT32 DISP_GetPanelBPP(void)
{
	PANEL_COLOR_FORMAT fmt;
	disp_drv_init_context();

	if(disp_drv->get_panel_color_format == NULL) 
	{
		return DISP_STATUS_NOT_IMPLEMENTED;
	}

	fmt = disp_drv->get_panel_color_format();
	switch(fmt)
	{
		case PANEL_COLOR_FORMAT_RGB332:
			return 8;
		case PANEL_COLOR_FORMAT_RGB444:
			return 12;
		case PANEL_COLOR_FORMAT_RGB565:
			return 16;
		case PANEL_COLOR_FORMAT_RGB666:
			return 18;
		case PANEL_COLOR_FORMAT_RGB888:
			return 24;
		default:
			return 0;
	}
}

UINT32 DISP_GetOutputBPPforDithering(void)
{
	disp_drv_init_context();

	return (disp_drv->get_dithering_bpp) ?
		(disp_drv->get_dithering_bpp()) :
		DISP_STATUS_NOT_IMPLEMENTED;
}

DISP_STATUS DISP_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
	DISP_FUNC();
	disp_drv_init_context();

	return (disp_drv->capture_framebuffer) ?
		(disp_drv->capture_framebuffer(pvbuf, bpp)) :
		DISP_STATUS_NOT_IMPLEMENTED;
}

DISP_STATUS DISP_Capture_Videobuffer(unsigned int pvbuf, unsigned int bpp)
{
	DISP_FUNC();
	if (down_interruptible(&sem_update_screen)) {
		printk("[DISP] ERROR: Can't get sem_update_screen in DISP_Capture_Videobuffer()\n");
		return DISP_STATUS_ERROR;
	}
	disp_drv_init_context();
	LCD_Capture_Videobuffer(pvbuf, bpp);
	up(&sem_update_screen);
	return DISP_STATUS_OK;
}
// xuecheng, 2010-09-19
// this api is for mATV signal interfere workaround.
// immediate update == (TE disabled + delay update in overlay mode disabled)
static BOOL is_immediateupdate = false;
DISP_STATUS DISP_ConfigImmediateUpdate(BOOL enable)
{
	disp_drv_init_context();

	if(enable == TRUE)
	{
		LCD_TE_Enable(FALSE);
	}
	else
	{
		if(disp_drv->init_te_control)
			disp_drv->init_te_control();
		else
			return DISP_STATUS_NOT_IMPLEMENTED;
	}

	is_immediateupdate = enable;

	return DISP_STATUS_OK;
}

BOOL DISP_IsImmediateUpdate(void)
{
	return is_immediateupdate;
}

DISP_STATUS DISP_FMDesense_Query()
{
	if(LCM_TYPE_DBI == lcm_params->type){//DBI
    	return (DISP_STATUS)LCD_FMDesense_Query();
	}
	else if(LCM_TYPE_DPI == lcm_params->type){//DPI
		return (DISP_STATUS)DPI_FMDesense_Query();
	}
	else{// DSI
	    printk("DISP_FMDesense_Query():DSI driver not ready\n");
		return DISP_STATUS_NOT_IMPLEMENTED;
	}
}

DISP_STATUS DISP_FM_Desense(unsigned long freq)
{
    DISP_STATUS ret = DISP_STATUS_OK;
	if(LCM_TYPE_DBI == lcm_params->type){//DBI
		printk("DISP_FM_Desense():DBI interface\n");        
		LCD_CHECK_RET(LCD_FM_Desense(ctrl_if, freq));
	}
	else if(LCM_TYPE_DPI == lcm_params->type){//DPI
	    printk("DISP_FM_Desense():DPI interface\n");
		DPI_CHECK_RET(DPI_FM_Desense(freq));
	}
	else{// DSI
	    printk("DISP_FM_Desense():DSI driver not ready\n");
	}
	return ret;
}

DISP_STATUS DISP_Reset_Update()
{
    DISP_STATUS ret = DISP_STATUS_OK;
	if(LCM_TYPE_DBI == lcm_params->type){//DBI
		printk("DISP_Reset_Update():DBI interface\n");        
		LCD_CHECK_RET(LCD_Reset_WriteCycle(ctrl_if));
	}
	else if(LCM_TYPE_DPI == lcm_params->type){//DPI
	    printk("DISP_Reset_Update():DPI interface\n");
		DPI_CHECK_RET(DPI_Reset_CLK());
	}
	else{// DSI
	    printk("DISP_Reset_Update():DSI driver not ready\n");
	}
	return ret;
}

DISP_STATUS DISP_Get_Default_UpdateSpeed(unsigned int *speed)
{
    DISP_STATUS ret = DISP_STATUS_OK;
    if(LCM_TYPE_DBI == lcm_params->type){//DBI
		printk("DISP_Get_Default_UpdateSpeed():DBI interface\n");        
		LCD_CHECK_RET(LCD_Get_Default_WriteCycle(ctrl_if, speed));
	}
	else if(LCM_TYPE_DPI == lcm_params->type){//DPI
	    printk("DISP_Get_Default_UpdateSpeed():DPI interface\n");
		DPI_CHECK_RET(DPI_Get_Default_CLK(speed));
	}
	else{// DSI
	    printk("DISP_Get_Default_UpdateSpeed():DSI driver not ready\n");
	}
    return ret;
}

DISP_STATUS DISP_Get_Current_UpdateSpeed(unsigned int *speed)
{
    DISP_STATUS ret = DISP_STATUS_OK;
    if(LCM_TYPE_DBI == lcm_params->type){//DBI
		printk("DISP_Get_Current_UpdateSpeed():DBI interface\n");        
		LCD_CHECK_RET(LCD_Get_Current_WriteCycle(ctrl_if, speed));
	}
	else if(LCM_TYPE_DPI == lcm_params->type){//DPI
	    printk("DISP_Get_Current_UpdateSpeed():DPI interface\n");
		DPI_CHECK_RET(DPI_Get_Current_CLK(speed));
	}
	else{// DSI
	    printk("DISP_Get_Current_UpdateSpeed():DSI driver not ready\n");
	}
    return ret;
}

DISP_STATUS DISP_Change_Update(unsigned int speed)
{
    DISP_STATUS ret = DISP_STATUS_OK;
	if(LCM_TYPE_DBI == lcm_params->type){//DBI
		printk("DISP_Change_Update():DBI interface\n");        
		LCD_CHECK_RET(LCD_Change_WriteCycle(ctrl_if, speed));
	}
	else if(LCM_TYPE_DPI == lcm_params->type){//DPI
	    printk("DISP_Change_Update():DPI interface\n");
		DPI_CHECK_RET(DPI_Change_CLK(speed));
	}
	else{// DSI
	    printk("DISP_Change_Update():DSI driver not ready\n");
	}
	return ret;
}

#if defined(MTK_M4U_SUPPORT)
DISP_STATUS DISP_InitM4U()
{
    LCD_InitM4U();
	if(LCM_TYPE_DPI == lcm_params->type){
//	    DPI_InitM4U();     //DPI not use m4u currently
	}
	return DISP_STATUS_OK;
}

DISP_STATUS DISP_ConfigAssertLayerMva()
{
    unsigned int mva;
	ASSERT(DAL_layerPA);
    LCD_CHECK_RET(LCD_AllocUIMva(DAL_layerPA, &mva, DAL_GetLayerSize())); 
	printk("DAL Layer PA = DAL_layerPA = 0x%x, MVA = 0x%x\n", DAL_layerPA, mva);
	LCD_CHECK_RET(LCD_LayerSetAddress(ASSERT_LAYER, mva));
	return DISP_STATUS_OK;
}

DISP_STATUS DISP_AllocUILayerMva(unsigned int pa, unsigned int *mva, unsigned int size)
{
    LCD_CHECK_RET(LCD_AllocUIMva(pa, mva, size)); 
	printk("UI Layer PA = 0x%x, MVA = 0x%x\n", pa, *mva);
	return DISP_STATUS_OK;
}

DISP_STATUS DISP_AllocOverlayMva(unsigned int va, unsigned int *mva, unsigned int size)
{
    LCD_CHECK_RET(LCD_AllocOverlayMva(va, mva, size));
	return DISP_STATUS_OK;
}

DISP_STATUS DISP_DeallocMva(unsigned int va, unsigned int mva, unsigned int size)
{
    LCD_DeallocMva(va, mva, size);
    return DISP_STATUS_OK;
}
#endif
