

#define ENABLE_DPI_INTERRUPT        0
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#if ENABLE_DPI_REFRESH_RATE_LOG && !ENABLE_DPI_INTERRUPT
#error "ENABLE_DPI_REFRESH_RATE_LOG should be also ENABLE_DPI_INTERRUPT"
#endif

#include <linux/kernel.h>
#include <linux/string.h>

#if defined(CONFIG_ARCH_MT6573)
    #include <mach/mt6573_typedefs.h>
    #include <mach/mt6573_reg_base.h>
    #include <mach/mt6573_irq.h>
    #include <mach/mt6573_pll.h>
#else
    #error "unknown arch"
#endif

#include "dpi_reg.h"
#include "dpi_drv.h"
#include "dsi_reg.h"
#if ENABLE_DPI_INTERRUPT
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <asm/tcm.h>
#include <mach/irqs.h>
#include "mtkfb.h"
#endif


PDPI_REGS const DPI_REG = (PDPI_REGS)(DPI_BASE);
PDSI_PHY_REGS const DSI_PHY_REG = (PDSI_PHY_REGS)(DSI_PHY_BASE);
unsigned long  DPI_CRTL_DRIVING_REG = APCONFIG_BASE + 0x8C;
unsigned long  DPI_DATA_DRIVING_REG = APCONFIG_BASE + 0x88;
static int wst_step_DPI = -1;//for LCD&FM de-sense
static unsigned int default_pll_div1 = 0;
static bool is_get_default_clk = FALSE;
static BOOL s_isDpiPowerOn = FALSE;
static DPI_REGS regBackup;

#define DPI_REG_OFFSET(r)       offsetof(DPI_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))

const UINT32 BACKUP_DPI_REG_OFFSETS[] =
{
    DPI_REG_OFFSET(INT_ENABLE),
    DPI_REG_OFFSET(SIZE),
    DPI_REG_OFFSET(CLK_CNTL),
    DPI_REG_OFFSET(DITHER),

    DPI_REG_OFFSET(FB[0].ADDR),
    DPI_REG_OFFSET(FB[0].STEP),
    DPI_REG_OFFSET(FB[1].ADDR),
    DPI_REG_OFFSET(FB[1].STEP),
    DPI_REG_OFFSET(FB[2].ADDR),
    DPI_REG_OFFSET(FB[2].STEP),

    DPI_REG_OFFSET(OVL_CON),
//    DPI_REG_OFFSET(FBCD_LINE_W),
    DPI_REG_OFFSET(FIFO_TH),
    DPI_REG_OFFSET(FIFO_INC),
//    DPI_REG_OFFSET(FIFO_TH_MAX),
    DPI_REG_OFFSET(FIFO_MAX),

    DPI_REG_OFFSET(TGEN_HCNTL),
    DPI_REG_OFFSET(TGEN_VCNTL),

    DPI_REG_OFFSET(CUSR_CNTL),
    DPI_REG_OFFSET(CUSR_COORD),
    DPI_REG_OFFSET(CUSR_ADDR),

    DPI_REG_OFFSET(TMODE),

    DPI_REG_OFFSET(CNTL),
};

static void _BackupDPIRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;
    
    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i])));
    }
}

static void _RestoreDPIRegisters(void)
{
    DPI_REGS *reg = &regBackup;
    UINT32 i;

    for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
    {
        OUTREG32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i]),
                 AS_UINT32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i])));
    }
}

static void _ResetBackupedDPIRegisterValues(void)
{
    DPI_REGS *regs = &regBackup;
    memset((void*)regs, 0, sizeof(DPI_REGS));

    OUTREG32(&regs->CLK_CNTL, 0x00000101);
    OUTREG32(&regs->FIFO_MAX, 0x00000200);
}


#if ENABLE_DPI_REFRESH_RATE_LOG
static void _DPI_LogRefreshRate(DPI_REG_INTERRUPT status)
{
    static unsigned long prevUs = 0xFFFFFFFF;

    if (status.VSYNC)
    {
        struct timeval curr;
        do_gettimeofday(&curr);

        if (prevUs < curr.tv_usec)
        {
            printk("[DPI][Debug] Receive 1 vsync in %lu us\n", 
                   curr.tv_usec - prevUs);
        }
        prevUs = curr.tv_usec;
    }
}
#else
#define _DPI_LogRefreshRate(x)  do {} while(0)
#endif

#if ENABLE_DPI_INTERRUPT
static __tcmfunc irqreturn_t _DPI_InterruptHandler(int irq, void *dev_id)
{   
    static int counter = 0;
    DPI_REG_INTERRUPT status = DPI_REG->INT_STATUS;
    if (status.FIFO_EMPTY) ++ counter;

    if (status.VSYNC && counter) {
        printk("[Error] DPI FIFO is empty, "
               "received %d times interrupt !!!\n", counter);
        counter = 0;
    }

    _DPI_LogRefreshRate(status);

    return IRQ_HANDLED;
}
#endif


DPI_STATUS DPI_Init(BOOL isDpiPoweredOn)
{
    DPI_REG_CNTL cntl;

    if (isDpiPoweredOn) {
        _BackupDPIRegisters();
//        s_isDpiPowerOn = TRUE;
    } else {
        _ResetBackupedDPIRegisterValues();
//        DPI_PowerOn();
    }
    DPI_PowerOn();

    // Always enable frame shift protection and recovery
    cntl = DPI_REG->CNTL;
    cntl.FS_PROT_EN = 1;
    cntl.FS_RC_EN = 1;

    // Enable adaptive FIFO high/low threshold control
    cntl.ADP_EN = 1;
     
    OUTREG32(&DPI_REG->CNTL, AS_UINT32(&cntl));

    // Config ultra high threshold water mark
    {
        DPI_REG_FIFO_TH th = DPI_REG->FIFO_TH;

        th.LOW = 64;
        th.HIGH = 128;

        OUTREG32(&DPI_REG->FIFO_TH, AS_UINT32(&th));
        DPI_REG->FIFO_INC = 8;
    }

#if ENABLE_DPI_INTERRUPT
    if (request_irq(MT6573_DPI_IRQ_LINE,
        _DPI_InterruptHandler, 0, MTKFB_DRIVER, NULL) < 0)
    {
        printk("[DPI][ERROR] fail to request DPI irq\n"); 
        return DPI_STATUS_ERROR;
    }

    {
        DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
        enInt.FIFO_EMPTY = 1;
        enInt.VSYNC = 1;
        OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
    }
#endif

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_Deinit(void)
{
    DPI_DisableClk();
    DPI_PowerOff();

    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Init_PLL(unsigned int mipi_pll_clk_ref,unsigned int mipi_pll_clk_div1,unsigned int mipi_pll_clk_div2)
{
    DSI_PHY_REG_ANACON0 con0 = DSI_PHY_REG->ANACON0;
    DSI_PHY_REG_ANACON1 con1 = DSI_PHY_REG->ANACON1;

    con1.RG_PLL_DIV1 = mipi_pll_clk_div1;
    con1.RG_PLL_DIV2 = mipi_pll_clk_div2;

	con0.PLL_CLKR_EN = 1;
	con0.PLL_EN = 1;
	con0.RG_DPI_EN = 1;

    // Set to DSI_PHY_REG
    
    OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&con0));
    OUTREG32(&DSI_PHY_REG->ANACON1, AS_UINT32(&con1));
	return DPI_STATUS_OK;
}

DPI_STATUS DPI_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
	MASKREG32(DPI_CRTL_DRIVING_REG, (0xF << 24), (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (24));
	MASKREG32(DPI_DATA_DRIVING_REG, (0xF << 8),  (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (8));
	MASKREG32(DPI_DATA_DRIVING_REG, (0xF << 12), (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (12));
	MASKREG32(DPI_DATA_DRIVING_REG, (0xF << 16), (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (16));
	MASKREG32(DPI_DATA_DRIVING_REG, (0xF << 20), (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (20));
	MASKREG32(DPI_DATA_DRIVING_REG, (0xF << 24), (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (24));
	MASKREG32(DPI_DATA_DRIVING_REG, (0xF << 28), (((lcm_params->dpi.io_driving_current) | LCM_DRIVING_CURRENT_SLEW_CNTL) & 0xF) << (28));
	return DPI_STATUS_OK;
}

DPI_STATUS DPI_PowerOn()
{
    if (!s_isDpiPowerOn)
    {
#if 1   // FIXME
        BOOL ret = hwEnableClock(MT65XX_PDN_MM_DPI, "DPI");
        ASSERT(ret);
#else
#endif        
        _RestoreDPIRegisters();
        s_isDpiPowerOn = TRUE;
    }

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_PowerOff()
{
    if (s_isDpiPowerOn)
    {
        BOOL ret = TRUE;
        _BackupDPIRegisters();
#if 1   // FIXME
        ret = hwDisableClock(MT65XX_PDN_MM_DPI, "DPI");
#else
#endif        
        ASSERT(ret);
        s_isDpiPowerOn = FALSE;
    }

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_EnableClk()
{
    DPI_REG_EN en = DPI_REG->DPI_EN;
    en.EN = 1;
    OUTREG32(&DPI_REG->DPI_EN, AS_UINT32(&en));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_DisableClk()
{
    DPI_REG_EN en = DPI_REG->DPI_EN;
    en.EN = 0;
    OUTREG32(&DPI_REG->DPI_EN, AS_UINT32(&en));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_EnableSeqOutput(BOOL enable)
{
    DPI_REG_CNTL cntl = DPI_REG->CNTL;
    cntl.INTF_68_EN = enable ? 1 : 0;
    OUTREG32(&DPI_REG->CNTL, AS_UINT32(&cntl));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_SetRGBOrder(DPI_RGB_ORDER input, DPI_RGB_ORDER output)
{
    DPI_REG_CNTL cntl = DPI_REG->CNTL;
    cntl.SRGB_ORDER = input;    // Source RGB Order
    cntl.IRGB_ORDER = output;   // Interface RGB Order
    OUTREG32(&DPI_REG->CNTL, AS_UINT32(&cntl));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigPixelClk(DPI_POLARITY polarity, UINT32 divisor, UINT32 duty)
{
    DPI_REG_CLKCNTL ctrl;

    ASSERT(divisor >= 2);
    ASSERT(duty > 0 && duty < divisor);
    
    ctrl.POLARITY = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    ctrl.DIVISOR = divisor - 1;
    ctrl.DUTY = duty;

    OUTREG32(&DPI_REG->CLK_CNTL, AS_UINT32(&ctrl));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigDataEnable(DPI_POLARITY polarity)
{
    DPI_REG_TGEN_HCNTL hctrl = DPI_REG->TGEN_HCNTL;
    hctrl.DE_POL = (DPI_POLARITY_FALLING == polarity) ? 1 : 0;
    OUTREG32(&DPI_REG->TGEN_HCNTL, AS_UINT32(&hctrl));
    
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigVsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
    DPI_REG_TGEN_VCNTL ctrl;

    ctrl.VSYNC_POL = (DPI_POLARITY_RISING == polarity) ? 1 : 0;
    ctrl.VPW = pulseWidth - 1;
    ctrl.VBP = backPorch - 1;
    ctrl.VFP = frontPorch - 1;

    OUTREG32(&DPI_REG->TGEN_VCNTL, AS_UINT32(&ctrl));
    
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigHsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
    DPI_REG_TGEN_HCNTL ctrl = DPI_REG->TGEN_HCNTL;

    ctrl.HSYNC_POL = (DPI_POLARITY_RISING == polarity) ? 1 : 0;
    ctrl.HPW = pulseWidth - 1;
    ctrl.HBP = backPorch - 1;
    ctrl.HFP = frontPorch - 1;

    OUTREG32(&DPI_REG->TGEN_HCNTL, AS_UINT32(&ctrl));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FBEnable(DPI_FB_ID id, BOOL enable)
{
    DPI_REG_CNTL cntl = DPI_REG->CNTL;

    switch (id)
    {
        case DPI_FB_0:
            // do nothing
            break;

        case DPI_FB_1:
            cntl.FB1_EN = enable ? 1 : 0;
            break;

        case DPI_FB_2:
            cntl.FB2_EN = enable ? 1 : 0;
            break;

        default:
            ASSERT(0);
    }

    OUTREG32(&DPI_REG->CNTL, AS_UINT32(&cntl));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FBSyncFlipWithLCD(BOOL enable)
{
    DPI_REG_CNTL cntl = DPI_REG->CNTL;
    cntl.FB_CHK_EN = enable ? 1 : 0;
    OUTREG32(&DPI_REG->CNTL, AS_UINT32(&cntl));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FBSetFormat(DPI_FB_FORMAT format)
{
    DPI_REG_CNTL cntl = DPI_REG->CNTL;
    cntl.PXL_FMT = format;
    OUTREG32(&DPI_REG->CNTL, AS_UINT32(&cntl));

    return DPI_STATUS_OK;
}

DPI_FB_FORMAT DPI_FBGetFormat(void)
{
    DPI_REG_CNTL cntl = DPI_REG->CNTL;
    return cntl.PXL_FMT;
}



DPI_STATUS DPI_FBSetSize(UINT32 width, UINT32 height)
{
    DPI_REG_SIZE size;
    size.WIDTH = width;
    size.HEIGHT = height;
    
    OUTREG32(&DPI_REG->SIZE, AS_UINT32(&size));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_FBSetAddress(DPI_FB_ID id, UINT32 address)
{
    ASSERT(id < DPI_FB_NUM);
    DPI_REG->FB[id].ADDR = address;

    return DPI_STATUS_OK;
}    


DPI_STATUS DPI_FBSetPitch(DPI_FB_ID id, UINT32 pitchInByte)
{
    ASSERT(id < DPI_FB_NUM);
    DPI_REG->FB[id].STEP = pitchInByte;

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_SetFifoThreshold(UINT32 low, UINT32 high)
{
    DPI_REG_FIFO_TH th = DPI_REG->FIFO_TH;

    if (high > DPI_FIFO_TH_MAX) {
        high = DPI_FIFO_TH_MAX;
    }
    if (low > high) {
        low = high;
    }

    th.LOW = low;
    th.HIGH = high;
    OUTREG32(&DPI_REG->FIFO_TH, AS_UINT32(&th));

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_DumpRegisters(void)
{
    UINT32 i;

    printk("---------- Start dump DPI registers ----------\n");
    
    for (i = 0; i < sizeof(DPI_REGS); i += 4)
    {
        printk("DPI+%04x : 0x%08x\n", i, INREG32(DPI_BASE + i));
    }

    return DPI_STATUS_OK;
}

UINT32 DPI_GetCurrentFB(void)
{
    DPI_REG_STATUS status;

    status = DPI_REG->STATUS;

    switch(status.FB_INUSE)
    {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        default:
            ASSERT(0);
    }
}


DPI_STATUS DPI_Capture_Framebuffer(unsigned int pvbuf, unsigned int bpp)
{
    unsigned int i = 0;
    unsigned char *fbv;
    unsigned int fbsize = 0;
    unsigned int dpi_fb_bpp = 0;
    unsigned int w,h;

	if(s_isDpiPowerOn == false)
	{
		DPI_PowerOn();
	}

    if(pvbuf == 0 || bpp == 0)
    {
        printk("DPI_Capture_Framebuffer, ERROR, parameters wrong: pvbuf=0x%08x, bpp=%d\n", pvbuf, bpp);
        return DPI_STATUS_OK;
    }

    if(DPI_FBGetFormat() == DPI_FB_FORMAT_RGB565)
    {
        dpi_fb_bpp = 16;
    }
    else if(DPI_FBGetFormat() == DPI_FB_FORMAT_RGB888)
    {
        dpi_fb_bpp = 24;
    }
    else
    {
        printk("DPI_Capture_Framebuffer, ERROR, dpi_fb_bpp is wrong: %d\n", dpi_fb_bpp);
        return DPI_STATUS_OK;
    }

    w = DISP_GetScreenWidth();
    h = DISP_GetScreenHeight();
    fbsize = w*h*dpi_fb_bpp/8;

    fbv = (unsigned char*)ioremap_cached((unsigned int)DPI_REG->FB[DPI_GetCurrentFB()].ADDR, fbsize);
 
    printk("current fb count is %d\n", DPI_GetCurrentFB());

    if(bpp == 32 && dpi_fb_bpp == 24)
    {
    	for(i = 0;i < w*h; i++)
    	{
            *(unsigned int*)(pvbuf+i*4) = 0xff000000|fbv[i*3]|(fbv[i*3+1]<<8)|(fbv[i*3+2]<<16);
    	}
    }
    else if(bpp == 32 && dpi_fb_bpp == 16)
    {
        unsigned int t;
	unsigned short* fbvt = (unsigned short*)fbv;
        for(i = 0;i < w*h; i++)
    	{
	    t = fbvt[i];
            *(unsigned int*)(pvbuf+i*4) = 0xff000000|((t&0x001F)<<3)|((t&0x07E0)<<5)|((t&0xF800)<<8);
    	}
    }
    else if(bpp == 16 && dpi_fb_bpp == 16)
    {
    	memcpy((void*)pvbuf, (void*)fbv, fbsize);
    }
    else if(bpp == 16 && dpi_fb_bpp == 24)
    {
    	for(i = 0;i < w*h; i++)
    	{
            *(unsigned short*)(pvbuf+i*2) = ((fbv[i*3+0]&0x1F)>>3)|
	                                    ((fbv[i*3+1]&0xFC)<<3)|
					    ((fbv[i*3+2]&0x1F)<<8);
    	}
    }
    else
    {
    	printk("DPI_Capture_Framebuffer, bpp:%d & dpi_fb_bpp:%d is not supported now\n", bpp, dpi_fb_bpp);
    }

    iounmap(fbv);
    
	if(s_isDpiPowerOn == false)
	{
		DPI_PowerOff();
	}

    return DPI_STATUS_OK;    
}

DPI_STATUS DPI_FMDesense_Query()
{
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_FM_Desense(unsigned long freq)
{
    UINT32 a,b,c,d,post_divider;
    UINT32 dpi_clk_div,rg_pll_div1,rg_pll_div2;
	DPI_REG_CLKCNTL ctrl;
    DSI_PHY_REG_ANACON1 con1;
	printk("[enter DPI_FM_Desense] pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));
//get dpi pll and clk	
	OUTREG32(&ctrl, AS_UINT32(&DPI_REG->CLK_CNTL));
	dpi_clk_div = ctrl.DIVISOR;

    OUTREG32(&con1, AS_UINT32(&DSI_PHY_REG->ANACON1));
	rg_pll_div1 = con1.RG_PLL_DIV1;
    rg_pll_div2 = con1.RG_PLL_DIV2;

//	calculate whether need to de-sense
    post_divider = (rg_pll_div2 == 0)?1:(rg_pll_div2 * 2);
    a = freq - 2;
    b = freq + 2;

	c = (a * post_divider * (dpi_clk_div + 1))%(260 * (rg_pll_div1 + 1));
	d = (b * post_divider * (dpi_clk_div + 1))%(260 * (rg_pll_div1 + 1));
	a = (a * post_divider * (dpi_clk_div + 1))/(260 * (rg_pll_div1 + 1));
	b = (b  * post_divider * (dpi_clk_div + 1))/(260 * (rg_pll_div1 + 1));

	if((b > a)||(c == 0)||(d == 0)){
	    printk("[DPI_FM_Desense] need to modify DPI PLL setting, freq = %ld\n",freq);
        rg_pll_div1 -= wst_step_DPI;
		wst_step_DPI = 0 - wst_step_DPI;

		con1.RG_PLL_DIV1 = rg_pll_div1;
		
		OUTREG32(&DSI_PHY_REG->ANACON1, AS_UINT32(&con1));
	}
	else{
		printk("[DPI_FM_Desense] not need to modify DPI PLL setting,freq = %ld\n",freq);
	}	
	printk("[leave DPI_FM_Desense]:pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));   
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Reset_CLK()
{
    UINT32 rg_pll_div1;
	DSI_PHY_REG_ANACON1 con1;
	printk("[enter DPI_Reset_CLK] pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));
	if(wst_step_DPI > 0){
	    printk("[DPI_Reset_CLK] need to restore PLL setting\n");
	    OUTREG32(&con1, AS_UINT32(&DSI_PHY_REG->ANACON1));
		rg_pll_div1 = con1.RG_PLL_DIV1;
		rg_pll_div1 -= wst_step_DPI;
		wst_step_DPI = 0 - wst_step_DPI;

		con1.RG_PLL_DIV1 = rg_pll_div1;
		OUTREG32(&DSI_PHY_REG->ANACON1, AS_UINT32(&con1));
	}
	else{
		printk("[DPI_Reset_CLK] pll is default setting, not need to restore it\n");
	}
	printk("[leave DPI_Reset_CLK] pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));
	return DPI_STATUS_OK;
}

DPI_STATUS DPI_Get_Default_CLK(unsigned int *clk)
{
    DSI_PHY_REG_ANACON1 con1;
	
	if(is_get_default_clk){
	    *clk = default_pll_div1;
	    return DPI_STATUS_OK;
	}

	printk("[enter DPI_Get_Default_CLK] pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));
    OUTREG32(&con1, AS_UINT32(&DSI_PHY_REG->ANACON1));
	default_pll_div1 = con1.RG_PLL_DIV1;
	*clk = default_pll_div1;
	is_get_default_clk = TRUE;
	printk("[leave DPI_FM_Desense]:pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));   
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Get_Current_CLK(unsigned int *clk)
{
    DSI_PHY_REG_ANACON1 con1;

	printk("[enter DPI_Get_Current_CLK] pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));
    OUTREG32(&con1, AS_UINT32(&DSI_PHY_REG->ANACON1));
	*clk = con1.RG_PLL_DIV1;
	printk("[leave DPI_Get_Current_CLK]:rg_pll_div1 = %d\n", *clk);   
    return DPI_STATUS_OK;
}

DPI_STATUS DPI_Change_CLK(unsigned int clk)
{
    UINT32 rg_pll_div1;
    DSI_PHY_REG_ANACON1 con1;
	printk("[enter DPI_Change_CLK] pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));

    OUTREG32(&con1, AS_UINT32(&DSI_PHY_REG->ANACON1));

    rg_pll_div1 = clk;

	con1.RG_PLL_DIV1 = rg_pll_div1;
		
	OUTREG32(&DSI_PHY_REG->ANACON1, AS_UINT32(&con1));
	printk("[leave DPI_Change_CLK]:pll_setting = 0x%x\n", INREG32(&DSI_PHY_REG->ANACON1));   
    return DPI_STATUS_OK;
}
