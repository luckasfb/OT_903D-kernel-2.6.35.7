

#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/fb.h>

#include <linux/debugfs.h>

#if defined(CONFIG_ARCH_MT6516)
    #include <mach/mt6516_typedefs.h>
#elif defined(CONFIG_ARCH_MT6573)
    #include <mach/mt6573_typedefs.h>
#else
    #error "unknown arch"
#endif

#include "lcd_drv.h"
#include "lcd_reg.h"
#include "dpi_drv.h"
#include "dpi_reg.h"
#include "dsi_drv.h"
#include "dsi_reg.h"

#if defined(MTK_TVOUT_SUPPORT)
#include "tv_out.h"
#include "tvc_drv.h"
#include "tve_drv.h"
#include "tvrot_drv.h"
#endif

#include "debug.h"

// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------

extern long tpd_last_down_time;
extern int  tpd_start_profiling;
extern void mtkfb_log_enable(bool enable);
extern void disp_log_enable(bool enable);
extern void dbi_log_enable(bool enable);
#if defined (MTK_TVOUT_SUPPORT)
bool capture_tv_buffer = false;
#endif

extern LCM_DRIVER *lcm_drv;
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------

static const long int DEFAULT_LOG_FPS_WND_SIZE = 30;

typedef struct {
    unsigned int en_fps_log;
    unsigned int en_touch_latency_log;
    unsigned int log_fps_wnd_size;
    unsigned int force_dis_layers;
} DBG_OPTIONS;

static DBG_OPTIONS dbg_opt = {0};

static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > mtkfb\n"
    "\n"
    "ACTION\n"
    "        fps:[on|off]\n"
    "             enable fps and lcd update time log\n"
    "\n"
    "        tl:[on|off]\n"
    "             enable touch latency log\n"
    "\n"
    "        layer\n"
    "             dump lcd layer information\n"
    "\n"
    "        suspend\n"
    "             enter suspend mode\n"
    "\n"
    "        resume\n"
    "             leave suspend mode\n"
    "\n"
    "        lcm:[on|off]\n"
    "             power on/off lcm\n"
    "\n"
    "        lcd:[on|off]\n"
    "             power on/off display engine\n"
    "\n"
    "        te:[on|off]\n"
    "             turn on/off tearing-free control\n"
    "\n"
    "        tv:[on|off]\n"
    "             turn on/off tv-out\n"
    "\n"
    "        tvsys:[ntsc|pal]\n"
    "             switch tv system\n"
    "\n"
    "        reg:[lcd|dpi|dsi|tvc|tve]\n"
    "             dump hw register values\n"
    "\n"
    "        regw:addr=val\n"
    "             write hw register\n"
    "\n"
    "        regr:addr\n"
    "             read hw register\n";


// ---------------------------------------------------------------------------
//  Information Dump Routines
// ---------------------------------------------------------------------------

static __inline int is_layer_enable(unsigned int roi_ctl, unsigned int layer)
{
    return (roi_ctl >> (31 - layer)) & 0x1;
}


static __inline const char *narrate_lcd_layer_format(unsigned int format)
{
    #if defined(CONFIG_ARCH_MT6516)
        switch(format)
        {
            case LCD_LAYER_FORMAT_8BPP     : return "8bpp index";
            case LCD_LAYER_FORMAT_RGB565   : return "RGB565";
            case LCD_LAYER_FORMAT_ARGB8888 : return "ARGB8888";
            case LCD_LAYER_FORMAT_RGB888   : return "RGB888";
            default : ASSERT(0);
        }
    #elif defined(CONFIG_ARCH_MT6573)
        switch(format)
        {
   
            case LCD_LAYER_FORMAT_RGB565    : return "RGB565";
            case LCD_LAYER_FORMAT_YUYV422   : return "YUYV422";
            case LCD_LAYER_FORMAT_RGB888    : return "RGB888";
            case LCD_LAYER_FORMAT_ARGB8888  : return "ARGB8888";
            case LCD_LAYER_FORMAT_PARGB8888 : return "PARGB8888";
            case LCD_LAYER_FORMAT_xRGB8888  : return "xRGB8888";
            default : ASSERT(0);
        } 
	#else
    	#error "unknown arch"
    #endif   
}


static void dump_lcd_layer_info(void)
{
    unsigned int roi_ctl = AS_UINT32(&LCD_REG->WROI_CONTROL);

    unsigned int i = 0;

    printk("------------------------------------------\n"
           "[mtkfb] dump lcd layer information\n"
           "------------------------------------------\n");

    for (i = 0; i < 6; ++ i)
    {
        unsigned int layer_en = is_layer_enable(roi_ctl, i);
        
        printk("Layer[%d] is %s\n", i, layer_en ? "enabled" : "disabled");
        if (!layer_en) continue;

        printk("   size   : %d x %d\n", LCD_REG->LAYER[i].SIZE.WIDTH,
                                        LCD_REG->LAYER[i].SIZE.HEIGHT);
        printk("   offset : (%d, %d)\n", LCD_REG->LAYER[i].OFFSET.X,
                                         LCD_REG->LAYER[i].OFFSET.Y);
        printk("   format : %s\n", 
               narrate_lcd_layer_format(LCD_REG->LAYER[i].CONTROL.CLRDPT));
        printk("   dlink  : %d\n", 
               (AS_UINT32(&LCD_REG->WROI_DC) >> (31 - i)) & 0x1);
    }

    printk("\n");
}


// ---------------------------------------------------------------------------
//  FPS Log
// ---------------------------------------------------------------------------

typedef struct {
    long int current_lcd_time_us;
    long int current_te_delay_time_us;
    long int total_lcd_time_us;
    long int total_te_delay_time_us;
    long int start_time_us;
    long int trigger_lcd_time_us;
    unsigned int trigger_lcd_count;
} FPS_LOGGER;

static FPS_LOGGER fps = {0};


static long int get_current_time_us(void)
{
    struct timeval t;
    do_gettimeofday(&t);
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}


static void __inline reset_fps_logger(void)
{
    memset(&fps, 0, sizeof(fps));
}


void DBG_OnTriggerLcd(void)
{
    if (!dbg_opt.en_fps_log && !dbg_opt.en_touch_latency_log) return;
    
    fps.trigger_lcd_time_us = get_current_time_us();
    if (fps.trigger_lcd_count == 0) {
        fps.start_time_us = fps.trigger_lcd_time_us;
    }
}


void DBG_OnTeDelayDone(void)
{
    long int time;
    
    if (!dbg_opt.en_fps_log && !dbg_opt.en_touch_latency_log) return;

    time = get_current_time_us();
    fps.current_te_delay_time_us = (time - fps.trigger_lcd_time_us);
    fps.total_te_delay_time_us += fps.current_te_delay_time_us;
}


void DBG_OnLcdDone(void)
{
    long int time;

    if (!dbg_opt.en_fps_log && !dbg_opt.en_touch_latency_log) return;

    // deal with touch latency log

    time = get_current_time_us();
    fps.current_lcd_time_us = (time - fps.trigger_lcd_time_us);

#if 0   // FIXME
    if (dbg_opt.en_touch_latency_log && tpd_start_profiling) {

        printk("[mtkfb] Touch Latency: %ld ms\n", 
               (time - tpd_last_down_time) / 1000);

        printk("[mtkfb] LCD update time %ld ms (TE delay %ld ms + LCD %ld ms)\n",
               fps.current_lcd_time_us / 1000,
               fps.current_te_delay_time_us / 1000,
               (fps.current_lcd_time_us - fps.current_te_delay_time_us) / 1000);
        
        tpd_start_profiling = 0;
    }
#endif

    if (!dbg_opt.en_fps_log) return;

    // deal with fps log
        
    fps.total_lcd_time_us += fps.current_lcd_time_us;
    ++ fps.trigger_lcd_count;

    if (fps.trigger_lcd_count >= dbg_opt.log_fps_wnd_size) {

        long int f = fps.trigger_lcd_count * 100 * 1000 * 1000 
                     / (time - fps.start_time_us);

        long int update = fps.total_lcd_time_us * 100 
                          / (1000 * fps.trigger_lcd_count);

        long int te = fps.total_te_delay_time_us * 100 
                      / (1000 * fps.trigger_lcd_count);

        long int lcd = (fps.total_lcd_time_us - fps.total_te_delay_time_us) * 100
                       / (1000 * fps.trigger_lcd_count);

        printk("[mtkfb] FPS: %ld.%02ld, Avg. update time: %ld.%02ld ms "
               "(TE delay %ld.%02ld ms, LCD %ld.%02ld ms)\n",
               f / 100, f % 100,
               update / 100, update % 100,
               te / 100, te % 100,
               lcd / 100, lcd % 100);
        
        reset_fps_logger();
    }
}

// ---------------------------------------------------------------------------
//  Command Processor
// ---------------------------------------------------------------------------

static void process_dbg_opt(const char *opt)
{
    if (0 == strncmp(opt, "fps:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2)) {
            dbg_opt.en_fps_log = 1;
        } else if (0 == strncmp(opt + 4, "off", 3)) {
            dbg_opt.en_fps_log = 0;
        } else {
            goto Error;
        }
        reset_fps_logger();
    }
    else if (0 == strncmp(opt, "tl:", 3))
    {
        if (0 == strncmp(opt + 3, "on", 2)) {
            dbg_opt.en_touch_latency_log = 1;
        } else if (0 == strncmp(opt + 3, "off", 3)) {
            dbg_opt.en_touch_latency_log = 0;
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "suspend", 4))
    {
        DISP_PanelEnable(FALSE);
        DISP_PowerEnable(FALSE);
    }
    else if (0 == strncmp(opt, "resume", 4))
    {
        DISP_PowerEnable(TRUE);
        DISP_PanelEnable(TRUE);
    }
    else if (0 == strncmp(opt, "lcm:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2)) {
            DISP_PanelEnable(TRUE);
        } else if (0 == strncmp(opt + 4, "off", 3)) {
            DISP_PanelEnable(FALSE);
        }
		else if (0 == strncmp(opt + 4, "init", 4)) {
			if (NULL != lcm_drv && NULL != lcm_drv->init) {
        		lcm_drv->init();
    		}
        }else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "lcd:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2)) {
            DISP_PowerEnable(TRUE);
        } else if (0 == strncmp(opt + 4, "off", 3)) {
            DISP_PowerEnable(FALSE);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "te:", 3))
    {
        if (0 == strncmp(opt + 3, "on", 2)) {
            LCD_TE_Enable(TRUE);
        } else if (0 == strncmp(opt + 3, "off", 3)) {
            LCD_TE_Enable(FALSE);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "layer", 5))
    {
        dump_lcd_layer_info();
    }
    else if (0 == strncmp(opt, "reg:", 4))
    {
        if (0 == strncmp(opt + 4, "lcd", 3)) {
            LCD_DumpRegisters();
        } else if (0 == strncmp(opt + 4, "dpi", 3)) {
            DPI_DumpRegisters();
        } else if (0 == strncmp(opt + 4, "dsi", 3)) {
            DSI_DumpRegisters();
#if defined(MTK_TVOUT_SUPPORT)
        } else if (0 == strncmp(opt + 4, "tvc", 3)) {
            TVC_DumpRegisters();
        } else if (0 == strncmp(opt + 4, "tve", 3)) {
            TVE_DumpRegisters();
        } else if (0 == strncmp(opt + 4, "tvr", 3)) {
            TVR_DumpRegisters();
#endif		
        } else {
            goto Error;
        }
    }

#if defined(MTK_TVOUT_SUPPORT)
    else if (0 == strncmp(opt, "tv2:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2)) {
            TVOUT_TvCablePlugIn();
        } else if (0 == strncmp(opt +4, "off", 3)) {
            TVOUT_TvCablePlugOut();
        } else {
            goto Error;
        }
    }
	else if (0 == strncmp(opt, "tv2sys:", 7))
    {
        if (0 == strncmp(opt + 7, "ntsc", 4)) {
            TVOUT_SetTvSystem(TVOUT_SYSTEM_NTSC);
        } else if (0 == strncmp(opt + 7, "pal", 3)) {
            TVOUT_SetTvSystem(TVOUT_SYSTEM_PAL);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "tv2rot:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            TVOUT_SetOrientation(TVOUT_ROT_270);
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            TVOUT_SetOrientation(TVOUT_ROT_0);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "tv2cb:", 6))
    {
        if (0 == strncmp(opt + 6, "on", 2)) {
            TVOUT_EnableColorBar(TRUE);
        } else if (0 == strncmp(opt + 6, "off", 3)) {
            TVOUT_EnableColorBar(FALSE);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "tv2vm:", 6))
    {
        if (0 == strncmp(opt + 6, "on", 2)) {
            TVOUT_DisableVideoMode(false);
        } else if (0 == strncmp(opt + 6, "off", 3)) {
            TVOUT_DisableVideoMode(true);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "tv2log:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            TVOUT_EnableLog(true);
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            TVOUT_EnableLog(false);
        } else {
            goto Error;
        }
    }

    else if (0 == strncmp(opt, "tv2cap:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            capture_tv_buffer = true;
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            capture_tv_buffer = false;
        } else {
            goto Error;
        }
    }      
    else if (0 == strncmp(opt, "tv2cam:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            TVOUT_ForceClose();
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            TVOUT_RestoreOpen();
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "tvuser:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            TVOUT_TurnOn(true);
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            TVOUT_TurnOn(false);
        } else {
            goto Error;
        }
    }
#endif
    else if (0 == strncmp(opt, "regw:", 5))
    {
        char *p = (char *)opt + 5;
        unsigned long addr = simple_strtoul(p, &p, 16);
        unsigned long val  = simple_strtoul(p + 1, &p, 16);

        if (addr) {
            OUTREG32(addr, val);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "regr:", 5))
    {
        char *p = (char *)opt + 5;
        unsigned int addr = (unsigned int) simple_strtoul(p, &p, 16);

        if (addr) {
            printk("Read register 0x%08x: 0x%08x\n", addr, INREG32(addr));
        } else {
            goto Error;
        }
    }
	else if(0 == strncmp(opt, "bkl:", 4))
	{
		char *p = (char *)opt + 4;
		unsigned int level = (unsigned int) simple_strtoul(p, &p, 10);
		printk("debug.c, process_dbg_opt(), set backlight level = %d\n", level);
		DISP_SetBacklight(level);
	}
	else if(0 == strncmp(opt, "dither:", 7))
	{
		unsigned int lrs, lgs, lbs, dbr, dbg, dbb;
		char *p = (char *)opt + 7;
		lrs = (unsigned int) simple_strtoul(p, &p, 16);
		p++;
		lgs = (unsigned int) simple_strtoul(p, &p, 16);
		p++;
		lbs = (unsigned int) simple_strtoul(p, &p, 16);
		p++;
		dbr = (unsigned int) simple_strtoul(p, &p, 16);
		p++;
		dbg = (unsigned int) simple_strtoul(p, &p, 16);
		p++;
		dbb = (unsigned int) simple_strtoul(p, &p, 16);

		printk("process_dbg_opt(), %d %d %d %d %d %d\n", lrs, lgs, lbs, dbr, dbg, dbb);
#if defined(CONFIG_ARCH_MT6573)
		LCD_WaitForNotBusy();
		LCD_ConfigDither(lrs, lgs, lbs, dbr, dbg, dbb);
		LCD_StartTransfer(true);
#endif
	}
    else if (0 == strncmp(opt, "mtkfblog:", 9))
    {
        if (0 == strncmp(opt + 9, "on", 2)) {
            mtkfb_log_enable(true);
        } else if (0 == strncmp(opt + 9, "off", 3)) {
            mtkfb_log_enable(false);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "displog:", 8))
    {
        if (0 == strncmp(opt + 8, "on", 2)) {
            disp_log_enable(true);
        } else if (0 == strncmp(opt + 8, "off", 3)) {
            disp_log_enable(false);
        } else {
            goto Error;
        }
    }
#if defined(CONFIG_ARCH_MT6573)
    else if (0 == strncmp(opt, "lcdlog:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            dbi_log_enable(true);
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            dbi_log_enable(false);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "dpilog:", 7))
    {
        if (0 == strncmp(opt + 7, "on", 2)) {
            //dpi_log_enable(true);
        } else if (0 == strncmp(opt + 7, "off", 3)) {
            //dpi_log_enable(false);
        } else {
            goto Error;
        }
    }
#endif
    else if (0 == strncmp(opt, "log:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2)) {
			mtkfb_log_enable(true);
			disp_log_enable(true);
#if defined(CONFIG_ARCH_MT6573)
			dbi_log_enable(true);
#endif
            //dpi_log_enable(true);
        } else if (0 == strncmp(opt + 4, "off", 3)) {
            mtkfb_log_enable(false);
			disp_log_enable(false);
#if defined(CONFIG_ARCH_MT6573)
			dbi_log_enable(false);
#endif
			//dpi_log_enable(false);
        } else {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "update", 6))
    {
		DISP_UpdateScreen(0, 0, DISP_GetScreenWidth(), DISP_GetScreenHeight());
    }
    else
	{
		goto Error;
	}

    return;

Error:
    printk("[mtkfb] parse command error!\n\n%s", STR_HELP);
}


static void process_dbg_cmd(char *cmd)
{
    char *tok;
    
    printk("[mtkfb] %s\n", cmd);
    
    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}


// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *mtkfb_dbgfs = NULL;


static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char debug_buffer[2048];

static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
    int n = 0;

    n += scnprintf(debug_buffer + n, debug_bufmax - n, STR_HELP);
    debug_buffer[n++] = 0;

    return simple_read_from_buffer(ubuf, count, ppos, debug_buffer, n);
}


static ssize_t debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(debug_buffer) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax) 
        count = debug_bufmax;

	if (copy_from_user(&debug_buffer, ubuf, count))
		return -EFAULT;

	debug_buffer[count] = 0;

    process_dbg_cmd(debug_buffer);

    return ret;
}


static struct file_operations debug_fops = {
	.read  = debug_read,
    .write = debug_write,
	.open  = debug_open,
};


void DBG_Init(void)
{
    mtkfb_dbgfs = debugfs_create_file("mtkfb",
        S_IFREG|S_IRUGO, NULL, (void *)0, &debug_fops);

    memset(&dbg_opt, sizeof(dbg_opt), 0);
    dbg_opt.log_fps_wnd_size = DEFAULT_LOG_FPS_WND_SIZE;
}


void DBG_Deinit(void)
{
    debugfs_remove(mtkfb_dbgfs);
}

