

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
//#include <devices.h>
#include <lcd.h>
#include <video_fb.h>

#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
//#include <asm/arch/mt6573_pmu_hw.h>
//#include <asm/arch/mt6573_pdn_sw.h>
#include <asm/arch/mt6573_gpio.h>
#include <asm/arch/mt65xx_disp_drv.h>
#include <asm/arch/mt65xx_lcd_drv.h>
#include <asm/arch/mt65xx_dpi_drv.h>
#include <asm/arch/mt65xx_dsi_drv.h>

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Export Functions - Display
// ---------------------------------------------------------------------------

static void  *fb_addr      = NULL;
static void  *logo_db_addr = NULL;
static UINT32 fb_size      = 0;


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static const DISP_DRIVER *disp_drv = NULL;

static LCD_IF_ID ctrl_if = LCD_IF_PARALLEL_0;

LCM_DRIVER  *lcm_drv  = NULL;
static LCM_PARAMS s_lcm_params = {0};
LCM_PARAMS *lcm_params = NULL;

extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
static BOOL isLCMFound = FALSE;

UINT32 mt65xx_disp_get_vram_size(void)
{
    return DISP_GetVRamSize();
}

void mt65xx_disp_init(void *lcdbase)
{

    fb_size = CFG_DISPLAY_WIDTH * CFG_DISPLAY_HEIGHT * CFG_DISPLAY_BPP / 8;

    logo_db_addr = (void *)((UINT32)lcdbase - 4 * 1024 * 1024); // Use CEVA 4MB region
    fb_addr      = (void *)((UINT32)lcdbase + fb_size);

    DISP_CHECK_RET(DISP_Init((UINT32)lcdbase, (UINT32)lcdbase, FALSE));

	memset((void*)lcdbase, 0, fb_size);
    /* transparent front buffer for fb_console display */
    LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER, TRUE));
    LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER, (UINT32)lcdbase));
    LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER, LCD_LAYER_FORMAT_RGB565));
	LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER, CFG_DISPLAY_WIDTH*2));
    LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER, 0, 0));
    LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT));
    LCD_CHECK_RET(LCD_LayerSetSourceColorKey(FB_LAYER, TRUE, 0x0));
#ifdef MTK_QVGA_LANDSCAPE_SUPPORT
	if(MTK_LCM_PHYSICAL_ROTATION == 90)
	{
		LCD_CHECK_RET(LCD_LayerSetRotation(FB_LAYER, LCD_LAYER_ROTATE_270));
	}
	else if(MTK_LCM_PHYSICAL_ROTATION == 270)
	{
		LCD_CHECK_RET(LCD_LayerSetRotation(FB_LAYER, LCD_LAYER_ROTATE_90));
	}
	else
	{
		// do nothing here.
	}

#endif

    /* background buffer for uboot logo display */
    LCD_CHECK_RET(LCD_LayerEnable(FB_LAYER - 1, TRUE));
    LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER - 1, (UINT32)fb_addr));
    LCD_CHECK_RET(LCD_LayerSetFormat(FB_LAYER - 1, LCD_LAYER_FORMAT_RGB565));
    LCD_CHECK_RET(LCD_LayerSetOffset(FB_LAYER - 1, 0, 0));
    LCD_CHECK_RET(LCD_LayerSetSize(FB_LAYER - 1, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT));
    LCD_CHECK_RET(LCD_LayerSetPitch(FB_LAYER - 1, CFG_DISPLAY_WIDTH*2));
    // xuecheng, for debug
    #if 0
	mt65xx_disp_show_boot_logo();
	mt65xx_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    #endif
}


void mt65xx_disp_power(BOOL on)
{
    if (on) {
        DISP_PowerEnable(TRUE);
        DISP_PanelEnable(TRUE);
    } else {
        DISP_PanelEnable(FALSE);
        DISP_PowerEnable(FALSE);
    }
}


void* mt65xx_get_logo_db_addr(void)
{
    return logo_db_addr;
}


void* mt65xx_get_fb_addr(void)
{
    return fb_addr;
}


UINT32 mt65xx_get_fb_size(void)
{
    return fb_size;
}


void mt65xx_disp_update(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    DISP_CHECK_RET(DISP_UpdateScreen(x, y, width, height));
}


void mt65xx_disp_wait_idle(void)
{
    LCD_CHECK_RET(LCD_WaitForNotBusy());
}


int mt6573IDP_EnableDirectLink(void)
{
    return 0;   // dummy function
}

const char* mt65xx_disp_get_lcm_id(void)
{
	if(lcm_drv)
		return lcm_drv->name;
	else
		return NULL;
}

// ---------------------------------------------------------------------------
//  Export Functions - Console
// ---------------------------------------------------------------------------

#ifdef CONFIG_CFB_CONSOLE

//  video_hw_init -- called by drv_video_init() for framebuffer console

extern UINT32 memory_size(void);

void *video_hw_init (void)
{
    static GraphicDevice s_mt65xx_gd;

	memset(&s_mt65xx_gd, 0, sizeof(GraphicDevice));

    s_mt65xx_gd.frameAdrs  = memory_size() - mt65xx_disp_get_vram_size();
    s_mt65xx_gd.winSizeX   = CFG_DISPLAY_WIDTH;
    s_mt65xx_gd.winSizeY   = CFG_DISPLAY_HEIGHT;
    s_mt65xx_gd.gdfIndex   = GDF_16BIT_565RGB;
    s_mt65xx_gd.gdfBytesPP = CFG_DISPLAY_BPP / 8;
    s_mt65xx_gd.memSize    = s_mt65xx_gd.winSizeX * s_mt65xx_gd.winSizeY * s_mt65xx_gd.gdfBytesPP;

    return &s_mt65xx_gd;
}


void video_set_lut(unsigned int index,  /* color number */
                   unsigned char r,     /* red */
                   unsigned char g,     /* green */
                   unsigned char b)     /* blue */
{
}

#endif  // CONFIG_CFB_CONSOLE
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
    udelay(1000 * ms);
}

static void lcm_send_cmd(UINT32 cmd)
{
	if(lcm_params == NULL)
		return;

    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_LOW,
                              cmd, lcm_params->dbi.cpu_write_bits));
}

static void lcm_send_data(UINT32 data)
{
	if(lcm_params == NULL)
		return;

    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_WriteIF(ctrl_if, LCD_IF_A0_HIGH,
                              data, lcm_params->dbi.cpu_write_bits));
}

static UINT32 lcm_read_data(void)
{
    UINT32 data = 0;

 	if(lcm_params == NULL)
		return;
   
    ASSERT(LCM_CTRL_SERIAL_DBI   == lcm_params->ctrl ||
           LCM_CTRL_PARALLEL_DBI == lcm_params->ctrl);

    LCD_CHECK_RET(LCD_ReadIF(ctrl_if, LCD_IF_A0_HIGH,
                             &data, lcm_params->dbi.cpu_write_bits));

    return data;
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


static void disp_drv_init_ctrl_if(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

	if(lcm_params == NULL)
		return;

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


#define IO_DRV0 (CONFIG_BASE + 0x0500)

#define SET_DRV_CURRENT(offset, value) \
    MASKREG32(IO_DRV0, (0xF << offset), ((value) & 0xF) << (offset))

static void disp_drv_set_driving_current(LCM_PARAMS *lcm)
{
	LCD_Set_DrivingCurrent(lcm);
}

static void disp_drv_init_io_pad(LCM_PARAMS *lcm)
{
	LCD_Init_IO_pad(lcm);
}

extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
LCM_DRIVER *disp_drv_get_lcm_driver(const char* lcm_name)
{
	LCM_DRIVER *lcm = NULL;
	printf("[LCM Auto Detect], we have %d lcm drivers built in\n", lcm_count);

	if(lcm_count ==1)
	{
		// we need to verify whether the lcm is connected
		// even there is only one lcm type defined
		lcm = lcm_driver_list[0];
		lcm->set_util_funcs(&lcm_utils);
		lcm->get_params(&s_lcm_params);

		lcm_params = &s_lcm_params;
		lcm_drv = lcm;
		isLCMFound = TRUE;
        
        printf("[LCM Specified]\t[%s]\n", (lcm->name==NULL)?"unknown":lcm->name);

		goto done;
	}
	else
	{
		int i;

		for(i = 0;i < lcm_count;i++)
		{
			lcm_params = &s_lcm_params;
			lcm = lcm_driver_list[i];

			printf("[LCM Auto Detect] [%d] - [%s]\t", 
				i, 
				(lcm->name==NULL)?"unknown":lcm->name);

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
					printf("\t\t[success]\n");
					isLCMFound = TRUE;
					lcm_drv = lcm;

					goto done;
				}
				else
				{
					printf("\t\t[fail]\n");
				}
			}
			else 
			{
				if(lcm->compare_id != NULL && lcm->compare_id())
				{
					printf("\t\t[success]\n");
					isLCMFound = TRUE;
					lcm_drv = lcm;

					goto done;
				}
				else
				{
					printf("\t\t[fail]\n");
				}
			}
		}
		
		printf("[LCM Auto Detect] no lcm device found\n");
		printf("[LCM Auto Detect] we will using the lcm driver with minimum resolution\n");

		// if we are here, it means:
		// 1.there is no correct driver for the lcm used;
		// 2.there is no lcm device connected.
		// we will find the lcm with minimum resolution to use, 
		// in case physical memory is not enough
		{
			LCM_PARAMS s_lcm_params_new = {0};
			LCM_PARAMS *lcm_params_new = &s_lcm_params_new;

			lcm = lcm_driver_list[0];
			memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
			lcm->get_params(lcm_params);

			for(i = 1;i < lcm_count;i++)
			{
				memset((void*)lcm_params_new, 0, sizeof(LCM_PARAMS));
				lcm_driver_list[i]->get_params(lcm_params_new);

				if((lcm_params->width * lcm_params->height) > 
					(lcm_params_new->width * lcm_params_new->height))
				{
					lcm = lcm_driver_list[i];
					memset((void*)lcm_params, 0, sizeof(LCM_PARAMS));
					lcm->get_params(lcm_params);
				}
			}
		}
	}

done:
	return lcm;
}


static void disp_dump_lcm_parameters(LCM_PARAMS *lcm_params)
{
	unsigned char *LCM_TYPE_NAME[] = {"DBI", "DPI", "DSI"};
	unsigned char *LCM_CTRL_NAME[] = {"NONE", "SERIAL", "PARALLEL", "GPIO"};
	printf("[LCM Auto Detect] LCM TYPE: %s\n", LCM_TYPE_NAME[lcm_params->type]);
	printf("[LCM Auto Detect] LCM INTERFACE: %s\n", LCM_CTRL_NAME[lcm_params->ctrl]);
	printf("[LCM Auto Detect] LCM resolution: %d x %d\n", lcm_params->width, lcm_params->height);

	return;
}
static BOOL disp_drv_init_context(void)
{
	LCD_STATUS ret;
	if (disp_drv != NULL && lcm_drv != NULL){
		return TRUE;
	}

	DISP_DetectDevice();

	disp_drv_init_ctrl_if();
	disp_drv_set_driving_current(NULL);

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

BOOL DISP_IsLcmFound(void)
{
	return isLCMFound;
}

BOOL DISP_SelectDevice(const char* lcm_name)
{
	LCD_STATUS ret;

	ret = LCD_Init();
	printf("ret of LCD_Init() = %d\n", ret);

	lcm_drv = disp_drv_get_lcm_driver(lcm_name);
	if (NULL == lcm_drv)
	{
		printf("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	disp_dump_lcm_parameters(lcm_params);
	return disp_drv_init_context();
}

BOOL DISP_DetectDevice(void)
{
	LCD_STATUS ret;

	ret = LCD_Init();

	lcm_drv = disp_drv_get_lcm_driver(NULL);
	if (NULL == lcm_drv)
	{
		printf("%s, disp_drv_get_lcm_driver() returns NULL\n", __func__);
		return FALSE;
	}

	disp_dump_lcm_parameters(lcm_params);
	return true;
}

// ---------------------------------------------------------------------------
//  DISP Driver Implementations
// ---------------------------------------------------------------------------

DISP_STATUS DISP_Init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    if (!disp_drv_init_context()) {
        return DISP_STATUS_NOT_IMPLEMENTED;
    }

    /* power on LCD before config its registers*/
    LCD_CHECK_RET(LCD_Init());

    disp_drv_init_ctrl_if();
    
    return (disp_drv->init) ?
           (disp_drv->init(fbVA, fbPA, isLcmInited)) :
           DISP_STATUS_NOT_IMPLEMENTED;
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
    disp_drv_init_context();
        
    return (disp_drv->enable_power) ?
           (disp_drv->enable_power(enable)) :
           DISP_STATUS_NOT_IMPLEMENTED;
}


DISP_STATUS DISP_PanelEnable(BOOL enable)
{
    static BOOL s_enabled = TRUE;

    disp_drv_init_context();

    if (!lcm_drv->suspend || !lcm_drv->resume) {
        return DISP_STATUS_NOT_IMPLEMENTED;
    }

    if (enable && !s_enabled) {
        s_enabled = TRUE;
        lcm_drv->resume();
    }
    else if (!enable && s_enabled)
    {
        s_enabled = FALSE;
        LCD_CHECK_RET(LCD_WaitForNotBusy());
        lcm_drv->suspend();
    }

    return DISP_STATUS_OK;
}

DISP_STATUS DISP_SetBacklight(UINT32 level)
{
	DISP_STATUS ret = DISP_STATUS_OK;

	disp_drv_init_context();

	LCD_WaitForNotBusy();

	if (!lcm_drv->set_backlight) {
		ret = DISP_STATUS_NOT_IMPLEMENTED;
		goto End;
	}

	lcm_drv->set_backlight(level);

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
    if (is_overlaying) {
        return DISP_STATUS_ALREADY_SET;
    } else {
        is_overlaying = TRUE;
    }

    return DISP_STATUS_OK;
}


DISP_STATUS DISP_LeaveOverlayMode(void)
{
    if (!is_overlaying) {
        return DISP_STATUS_ALREADY_SET;
    } else {
        is_overlaying = FALSE;
    }

    return DISP_STATUS_OK;
}

// -----
#if 0
static volatile int direct_link_layer = -1;

DISP_STATUS DISP_EnableDirectLinkMode(UINT32 layer)
{
    if (layer != direct_link_layer) {
        LCD_CHECK_RET(LCD_LayerSetTriggerMode(layer, LCD_HW_TRIGGER_DIRECT_COUPLE));
        LCD_CHECK_RET(LCD_LayerSetHwTriggerSrc(layer, LCD_HW_TRIGGER_SRC_IBW2));
        LCD_CHECK_RET(LCD_EnableHwTrigger(TRUE));
        LCD_CHECK_RET(LCD_StartTransfer(FALSE));
        direct_link_layer = layer;
    }

    return DISP_STATUS_OK;
}



DISP_STATUS DISP_DisableDirectLinkMode(UINT32 layer)
{
    if (layer == direct_link_layer) {
        LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));
        direct_link_layer = -1;
    }
    LCD_CHECK_RET(LCD_LayerSetTriggerMode(layer, LCD_SW_TRIGGER));

    return DISP_STATUS_OK;
}
#endif

// -----

extern int mt65xxIDP_EnableDirectLink(void);

DISP_STATUS DISP_UpdateScreen(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    LCD_CHECK_RET(LCD_WaitForNotBusy());

    if ((lcm_drv->update) &&
	   ((lcm_params->type==LCM_TYPE_DBI) || ((lcm_params->type==LCM_TYPE_DSI) && (lcm_params->dsi.mode==CMD_MODE))))
		{
        lcm_drv->update(x, y, width, height);
    }	

    LCD_CHECK_RET(LCD_SetRoiWindow(x, y, width, height));
    LCD_CHECK_RET(LCD_FBSetStartCoord(x, y));

    LCD_CHECK_RET(LCD_StartTransfer(FALSE));

		if (lcm_params->type==LCM_TYPE_DSI) {
			DSI_CHECK_RET(DSI_EnableClk());
		}

    return DISP_STATUS_OK;
}


// ---------------------------------------------------------------------------
//  Retrieve Information
// ---------------------------------------------------------------------------

UINT32 DISP_GetScreenWidth(void)
{
    disp_drv_init_context();
    return lcm_params->width;
}


UINT32 DISP_GetScreenHeight(void)
{
    disp_drv_init_context();
    return lcm_params->height;
}


UINT32 DISP_GetScreenBpp(void)
{
    return 32;  // ARGB8888
}


UINT32 DISP_GetPages(void)
{
    //return 4;
    return 2;   // Double Buffers
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

        //printf("DISP_GetVRamSize: %u bytes\n", vramSize);
    }

    return vramSize;
}


PANEL_COLOR_FORMAT DISP_GetPanelColorFormat(void)
{
    disp_drv_init_context();
        
    return (disp_drv->get_panel_color_format) ?
           (disp_drv->get_panel_color_format()) :
           DISP_STATUS_NOT_IMPLEMENTED;
}

