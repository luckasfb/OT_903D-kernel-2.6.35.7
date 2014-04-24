

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#if defined(CONFIG_ARCH_MT6516)
    #include <mach/mt6516_boot.h>
#elif defined(CONFIG_ARCH_MT6573)
    #include <mach/mt6573_boot.h>
   	#include <mach/mt6573_reg_base.h>
#elif defined(CONFIG_ARCH_MT6575)
   	//#include <mach/mt6573_boot.h>
#else
    #error "unknown arch"
#endif

#include <linux/disp_assert_layer.h>

#include "lcd_drv.h"
#include "lcd_reg.h"
#include "dpi_drv.h"
#include "dpi_reg.h"
#include "dsi_drv.h"
#include "dsi_reg.h"

#include "lcm_drv.h"

extern PLCD_REGS const LCD_REG;
extern PDSI_PHY_REGS const DSI_PHY_REG;

static UINT32 dsiTmpBufBpp = 0;

// ---------------------------------------------------------------------------
//  Private Variables
// ---------------------------------------------------------------------------

extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;

typedef struct
{
    UINT32 pa;
    UINT32 pitchInBytes;
} TempBuffer;

static TempBuffer s_tmpBuffers[3];

// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------

static void init_lcd_te_control(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

    if (LCM_DBI_TE_MODE_DISABLED == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_Enable(FALSE));
        return;
    }

    if (LCM_DBI_TE_MODE_VSYNC_ONLY == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_SetMode(LCD_TE_MODE_VSYNC_ONLY));
    } else if (LCM_DBI_TE_MODE_VSYNC_OR_HSYNC == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_SetMode(LCD_TE_MODE_VSYNC_OR_HSYNC));
        LCD_CHECK_RET(LCD_TE_ConfigVHSyncMode(dbi->te_hs_delay_cnt,
                                              dbi->te_vs_width_cnt,
                     (LCD_TE_VS_WIDTH_CNT_DIV)dbi->te_vs_width_cnt_div));
    } else ASSERT(0);

    LCD_CHECK_RET(LCD_TE_SetEdgePolarity(dbi->te_edge_polarity));
    LCD_CHECK_RET(LCD_TE_Enable(TRUE));
}

__inline DPI_FB_FORMAT get_dsi_tmp_buffer_format(void)
{
    switch(lcm_params->dpi.format)
    {
    case LCM_DPI_FORMAT_RGB565 : return DPI_FB_FORMAT_RGB565;
    case LCM_DPI_FORMAT_RGB666 :
    case LCM_DPI_FORMAT_RGB888 : return DPI_FB_FORMAT_RGB888;
    default : ASSERT(0);
    }
    return DPI_FB_FORMAT_RGB888;
}


__inline UINT32 get_dsi_tmp_buffer_bpp(void)
{
    static const UINT32 TO_BPP[] = {2, 3};
    
    return TO_BPP[get_dsi_tmp_buffer_format()];
}


__inline LCD_FB_FORMAT get_lcd_tmp_buffer_format(void)
{
    static const UINT32 TO_LCD_FORMAT[] = {
        LCD_FB_FORMAT_RGB565,
        LCD_FB_FORMAT_RGB888
    };
    
    return TO_LCD_FORMAT[get_dsi_tmp_buffer_format()];
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

static BOOL disp_drv_dsi_init_context(void)
{
    if (lcm_drv != NULL && lcm_params != NULL) 
		return TRUE;

    if (NULL == lcm_drv) {
        return FALSE;
    }

    lcm_drv->get_params(lcm_params);

	dsiTmpBufBpp=get_dsi_tmp_buffer_bpp();
    
    return TRUE;
}



static UINT32 get_fb_size(void)
{
    return DISP_GetScreenWidth() * 
           DISP_GetScreenHeight() * 
           ((DISP_GetScreenBpp() + 7) >> 3) * 
           DISP_GetPages();
}


static UINT32 get_intermediate_buffer_size(void)
{
    disp_drv_dsi_init_context();

    return 
#ifdef CONFIG_FB_MT6516_SIMULATE_MULTIPLE_RESOLUTIONS_ON_OPPO
    480 * 800 *
#else   
    DISP_GetScreenWidth() *
    DISP_GetScreenHeight() *
#endif              
    dsiTmpBufBpp *
    lcm_params->dpi.intermediat_buffer_num;
}


static UINT32 get_assert_layer_size(void)
{
    return DAL_GetLayerSize();
}


static void init_intermediate_buffers(UINT32 fbPhysAddr)
{
    UINT32 tmpFbStartPA = fbPhysAddr + get_fb_size();

#ifdef CONFIG_FB_MT6516_SIMULATE_MULTIPLE_RESOLUTIONS_ON_OPPO
    UINT32 tmpFbPitchInBytes = 480 * dsiTmpBufBpp;
    UINT32 tmpFbSizeInBytes  = tmpFbPitchInBytes * 800;
#else	
    UINT32 tmpFbPitchInBytes = DISP_GetScreenWidth() * dsiTmpBufBpp;
    UINT32 tmpFbSizeInBytes  = tmpFbPitchInBytes * DISP_GetScreenHeight();
#endif	

    UINT32 i;

	printk("[DISP] kernel - init_intermediate_buffers \n");
	printk("[DISP] kernel - tmpFbStartPA=%x, fbPhysAddr=%x \n", tmpFbStartPA, fbPhysAddr);
    
    for (i = 0; i < lcm_params->dpi.intermediat_buffer_num; ++ i)
    {
        TempBuffer *b = &s_tmpBuffers[i];

        // clean the intermediate buffers as black to prevent from noise display
        memset((void*)tmpFbStartPA, 0, tmpFbSizeInBytes);
        
        b->pitchInBytes = tmpFbPitchInBytes;
        b->pa = tmpFbStartPA;
        ASSERT((tmpFbStartPA & 0x7) == 0);  // check if 8-byte-aligned
        tmpFbStartPA += tmpFbSizeInBytes;
    }
}


static void init_assertion_layer(UINT32 fbVA, UINT32 fbPA)
{
	UINT32 offset;
    DAL_STATUS ret;

    disp_drv_dsi_init_context();

	if(lcm_params->dsi.mode == CMD_MODE)
		offset = get_fb_size();
	else
		offset = get_fb_size() + get_intermediate_buffer_size();

    ret = DAL_Init(fbVA + offset, fbPA + offset);
    ASSERT(DAL_STATUS_OK == ret);
}


static void init_mipi_pll(void)
{
    DSI_PHY_REG_ANACON0 con0 = DSI_PHY_REG->ANACON0;
    DSI_PHY_REG_ANACON1 con1 = DSI_PHY_REG->ANACON1;
    //DSI_PHY_REG_ANACON2 con2 = DSI_PHY_REG->ANACON2;


    con1.RG_PLL_DIV1 = lcm_params->dpi.mipi_pll_clk_div1;
    con1.RG_PLL_DIV2 = lcm_params->dpi.mipi_pll_clk_div2;

		con0.PLL_CLKR_EN = 1;
		con0.PLL_EN = 1;
		con0.RG_DPI_EN = 1;

    // Set to DSI_PHY_REG   
    OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&con0));
    OUTREG32(&DSI_PHY_REG->ANACON1, AS_UINT32(&con1));
    //OUTREG32(&DSI_PHY_REG->ANACON2, AS_UINT32(&con2));
}


static void init_io_pad(void)
{
#if 0
    GRAPH1SYS_LCD_IO_SEL_MODE sel_mode;

    if (lcm_params->dpi.is_serial_output) {
        sel_mode = GRAPH1SYS_LCD_IO_SEL_18CPU_8RGB;
    } else if (LCM_DPI_FORMAT_RGB565 == lcm_params->dpi.format) {
        sel_mode = GRAPH1SYS_LCD_IO_SEL_9CPU_16RGB;
    } else {
        sel_mode = GRAPH1SYS_LCD_IO_SEL_8CPU_18RGB;
    }

    MASKREG32(GRAPH1SYS_LCD_IO_SEL, GRAPH1SYS_LCD_IO_SEL_MASK, sel_mode);
#endif
}

static void init_lcd(void)
{
		UINT32 i;		
		LCD_REG_DSI_DC tmp_reg;
		
		tmp_reg=LCD_REG->DS_DSI_CON;

    // Config LCD Controller
    LCD_CHECK_RET(LCD_LayerEnable(LCD_LAYER_ALL, FALSE));
    LCD_CHECK_RET(LCD_LayerSetTriggerMode(LCD_LAYER_ALL, LCD_SW_TRIGGER));
    LCD_CHECK_RET(LCD_EnableHwTrigger(FALSE));

    LCD_CHECK_RET(LCD_SetBackgroundColor(0));

		if(lcm_params->dsi.mode == CMD_MODE)
			LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, lcm_params->width, lcm_params->height));
		else
			LCD_CHECK_RET(LCD_SetRoiWindow(0, 0, DISP_GetScreenWidth(), DISP_GetScreenHeight()));

		if(lcm_params->dsi.mode == CMD_MODE) {
			// W2DSI = 1
			tmp_reg.DC_DSI=1;
			// color format
			// FIX ME !! Need to program for better migration.
			tmp_reg.CLR_FMT=(lcm_params->dbi.data_format.format-2);
			OUTREG32(&LCD_REG->DS_DSI_CON, AS_UINT32(&tmp_reg));
			LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_LCM));
			LCD_CHECK_RET(LCD_WaitDPIIndication(FALSE));
		    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0, FALSE));
		    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_1, FALSE));
		    LCD_CHECK_RET(LCD_FBEnable(LCD_FB_2, FALSE));
		    LCD_CHECK_RET(LCD_ConfigIfFormat(lcm_params->dbi.data_format.color_order,
	                                    lcm_params->dbi.data_format.trans_seq,
	                                    lcm_params->dbi.data_format.padding,
	                                    lcm_params->dbi.data_format.format,
	                                    to_lcd_if_width(lcm_params->dbi.data_format.width)));
		} else {
	
			LCD_CHECK_RET(LCD_FBSetFormat(get_lcd_tmp_buffer_format()));
			LCD_CHECK_RET(LCD_FBSetPitch(s_tmpBuffers[0].pitchInBytes));
			LCD_CHECK_RET(LCD_FBSetStartCoord(0, 0));
	
			for (i = 0; i < lcm_params->dpi.intermediat_buffer_num; ++ i)
			{
				LCD_CHECK_RET(LCD_FBSetAddress(LCD_FB_0 + i, s_tmpBuffers[i].pa));
				LCD_CHECK_RET(LCD_FBEnable(LCD_FB_0 + i, TRUE));
			}
	
			LCD_CHECK_RET(LCD_SetOutputMode(LCD_OUTPUT_TO_MEM));
			/**
			"LCD Delay Enable" function should be used when there is only
			single buffer between LCD and DPI.
			Double buffer even triple buffer need not enable it.
			*/
			LCD_CHECK_RET(LCD_WaitDPIIndication(FALSE));
		}
}
static void init_dpi(BOOL isDpiPoweredOn)
{
    const LCM_DPI_PARAMS *dpi = &(lcm_params->dpi);
    UINT32 i;
	DPI_REG_CNTL tmp_reg;

    DPI_CHECK_RET(DPI_Init(isDpiPoweredOn));

    DPI_CHECK_RET(DPI_EnableSeqOutput(FALSE));
#ifdef CONFIG_FB_MT6516_SIMULATE_MULTIPLE_RESOLUTIONS_ON_OPPO
    DPI_CHECK_RET(DPI_FBSetSize(480, 800));
#else	
    DPI_CHECK_RET(DPI_FBSetSize(DISP_GetScreenWidth(), DISP_GetScreenHeight()));
	// For test only
	//*((volatile unsigned int *)0x80130010 )= 0x013F00EF;
#endif	
    
    for (i = 0; i < dpi->intermediat_buffer_num; ++ i)
    {
        DPI_CHECK_RET(DPI_FBSetAddress(DPI_FB_0 + i, s_tmpBuffers[i].pa));
        DPI_CHECK_RET(DPI_FBSetPitch(DPI_FB_0 + i, s_tmpBuffers[i].pitchInBytes));
        DPI_CHECK_RET(DPI_FBEnable(DPI_FB_0 + i, TRUE));
    }
    DPI_CHECK_RET(DPI_FBSetFormat(get_dsi_tmp_buffer_format()));
    DPI_CHECK_RET(DPI_FBSyncFlipWithLCD(TRUE));

    if (LCM_COLOR_ORDER_BGR == dpi->rgb_order) {
        DPI_CHECK_RET(DPI_SetRGBOrder(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_BGR));
    } else {
        DPI_CHECK_RET(DPI_SetRGBOrder(DPI_RGB_ORDER_RGB, DPI_RGB_ORDER_RGB));
    }

	OUTREG32(&DPI_REG->FIFO_TH, 0x01000030);
	OUTREG32(&DPI_REG->FIFO_INC, 0x00000001);	

	tmp_reg=DPI_REG->CNTL;

#if defined(CONFIG_ARCH_MT6573)
	tmp_reg.DSI_MODE=1;
#endif	
	tmp_reg.PXL_FMT=1;
	tmp_reg.FB_CHK_EN=1;

	OUTREG32(&DPI_REG->CNTL, AS_UINT32(&tmp_reg));

    DPI_CHECK_RET(DPI_EnableClk());
}

static void DSI_PHY_clk_setting(void)
{
		static unsigned int rg_pll_pr_div=0, rg_pll_fb_div2=0;    

		DSI_PHY_REG_ANACON0 mipitx_con0 = DSI_PHY_REG->ANACON0;
    DSI_PHY_REG_ANACON1 mipitx_con1 = DSI_PHY_REG->ANACON1;
    DSI_PHY_REG_ANACON3 mipitx_con3 = DSI_PHY_REG->ANACON3; 
          
    if (lcm_params->dsi.pll_div1>30) //vco high speed mode
    {
    	rg_pll_pr_div=1;
    	rg_pll_fb_div2=1;
    }
    else
    {
    	rg_pll_pr_div=0;
    	rg_pll_fb_div2=0;
    }  
                      
		mipitx_con0.PLL_EN=1;
		mipitx_con0.PLL_CLKR_EN=1;
		mipitx_con0.RG_LNT_LPTX_BIAS_EN=1;
		mipitx_con0.RG_LNT_HSTX_EDGE_SEL=0;
		mipitx_con0.RG_DSI_PHY_CK_PSEL=0;
		if(lcm_params->dsi.mode != CMD_MODE)
			mipitx_con0.RG_DPI_EN=1;
		else
			mipitx_con0.RG_DPI_EN=0;
		mipitx_con0.DSI_BIST_HS_BIAS_EN=0;
		mipitx_con0.DSI_EXT_CK_SEL=0;
		mipitx_con0.DSI_CK_INV=0;
    	
		mipitx_con1.RG_PLL_DIV1=lcm_params->dsi.pll_div1;
		mipitx_con1.RG_PLL_DIV2=lcm_params->dsi.pll_div2;
    mipitx_con1.RG_DSI_CK_SEL=(lcm_params->dsi.LANE_NUM-1);
		
		mipitx_con3.RG_PLL_KVSEL=2;
    mipitx_con3.RG_PLL_PRDIV=rg_pll_pr_div;
    //mipitx_con3.RG_PLL_VCOCAL_CKCTL=0;
    //mipitx_con3.RG_PLL_LKV_EN=0;
    mipitx_con3.RG_PLL_FB_DIV2=rg_pll_fb_div2;
    //mipitx_con3.RG_PLL_CDIV=0;
    mipitx_con3.RG_PLL_RT_ENB=1;
    //mipitx_con3.RG_PLL_FBCLK_MEN=0;
    //mipitx_con3.RG_PLL_REFCLK_MEN=0;
    mipitx_con3.RG_PLL_AUTOK_EN=1;
    mipitx_con3.RG_PLL_PODIV2_EN=0;
    
    // Set to DSI_PHY_REG
    OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&mipitx_con0));
    OUTREG32(&DSI_PHY_REG->ANACON1, AS_UINT32(&mipitx_con1));
    OUTREG32(&DSI_PHY_REG->ANACON3, AS_UINT32(&mipitx_con3));
    mdelay(10);
	
}

static void DSI_Config_VDO_Timing(void)
{
	OUTREG32(&DSI_REG->DSI_VSA_NL, lcm_params->dsi.vertical_sync_active);
	OUTREG32(&DSI_REG->DSI_VBP_NL, lcm_params->dsi.vertical_backporch);
	OUTREG32(&DSI_REG->DSI_VFP_NL, lcm_params->dsi.vertical_frontporch);
	OUTREG32(&DSI_REG->DSI_VACT_NL, lcm_params->dsi.vertical_active_line);

	OUTREG32(&DSI_REG->DSI_LINE_NB, lcm_params->dsi.line_byte);
	OUTREG32(&DSI_REG->DSI_HSA_NB, lcm_params->dsi.horizontal_sync_active_byte);
	OUTREG32(&DSI_REG->DSI_HBP_NB, lcm_params->dsi.horizontal_backporch_byte);
	OUTREG32(&DSI_REG->DSI_HFP_NB, lcm_params->dsi.horizontal_frontporch_byte);
	OUTREG32(&DSI_REG->DSI_RGB_NB, lcm_params->dsi.rgb_byte);

	OUTREG32(&DSI_REG->DSI_HSA_WC, lcm_params->dsi.horizontal_sync_active_word_count);
	OUTREG32(&DSI_REG->DSI_HBP_WC, lcm_params->dsi.horizontal_backporch_word_count);
	OUTREG32(&DSI_REG->DSI_HFP_WC, lcm_params->dsi.horizontal_frontporch_word_count);	
}

static void init_dsi(void)
{
	unsigned int setting[4];
	DSI_TXRX_CTRL_REG tmp_reg1;
	DSI_PSCTRL_REG tmp_reg2;

	DSI_CHECK_RET(DSI_Init());
	//DSI_CHECK_RET(DSI_PowerOn());

	tmp_reg1=DSI_REG->DSI_TXRX_CTRL;
	tmp_reg1.CKSM_EN=1;
	tmp_reg1.ECC_EN=1;
	tmp_reg1.LANE_NUM=lcm_params->dsi.LANE_NUM;
	tmp_reg1.VC_NUM=lcm_params->dsi.VC_NUM;	
	OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&tmp_reg1));	
	
	OUTREG32(&DSI_REG->DSI_MEM_CONTI, (lcm_params->dsi.DSI_WMEM_CONTI<<16)|(lcm_params->dsi.DSI_RMEM_CONTI));	

	//initialize DSI_PHY
	setting[0]=(lcm_params->dsi.HS_TRAIL<<24)|(lcm_params->dsi.HS_ZERO<<16)|(lcm_params->dsi.HS_PRPR<<8)|(lcm_params->dsi.LPX);
	setting[1]=(lcm_params->dsi.TA_SACK<<24)|(lcm_params->dsi.TA_GET<<16)|(lcm_params->dsi.TA_SURE<<8)|(lcm_params->dsi.TA_GO);
	setting[2]=(lcm_params->dsi.CLK_TRAIL<<24)|(lcm_params->dsi.CLK_ZERO<<16)|(lcm_params->dsi.LPX_WAIT<<8)|(lcm_params->dsi.CONT_DET);
	setting[3]=(lcm_params->dsi.CLK_HS_PRPR);
	DSI_PHY_TIMCONFIG(&setting[0]);

	DSI_PHY_clk_setting();

	DSI_lane0_ULP_mode(0);	

	if(lcm_params->dsi.mode != CMD_MODE)
	{
		DSI_Config_VDO_Timing();

		tmp_reg2.DSI_PS_SEL=lcm_params->dsi.PS;
		tmp_reg2.DSI_PS_WC=lcm_params->dsi.word_count;

		OUTREG32(&DSI_REG->DSI_PSCTRL, AS_UINT32(&tmp_reg2));	
	}
	
	// FIX ME
#if defined(CONFIG_ARCH_MT6573)
	*(volatile unsigned int *) (APCONFIG_BASE+0x0044) |= 0x00000100;	// enable MIPI TX IO
#endif
	
}

#if 0
void DSI_Init_VideoMode(void)
{
    static volatile unsigned int done = 0;
//;------------------------------------------------------------------------
//; set DPI to show picture (DPI setting)
//;------------------------------------------------------------------------
     *(volatile unsigned int *) DPI_SIZE = 0x032001E0		; //DPI_SIZE, 480x800, [25:16]=height,[9:0]=width
     //*(volatile unsigned int *) DPI_FB0_ADDR = ((unsigned int)&DPI_src_buffer1[0]); //DPI_FB0_ADDR (Memory offset to avoid BMP file header part)           
    //    SET_DPI_FB0_ADDR((unsigned int)&DPI_src_buffer1[0]);	       
    	
     *(volatile unsigned int *) DPI_FB0_STEP = 0x000005A0  	; //DPI_FB0_STEP = widthxbpp = 480x3 (width x bpp)
     *(volatile unsigned int *) DPI_FIFO_TH = 0x01000030        ; //DPI_FIFO_TH, threshold high/low
     *(volatile unsigned int *) DPI_CON = 0x00000014        	; //DPI_CON, DSI_EN @[2], FB1_EN @[8], FB2_EN @[9]                                           	                                      //clr_fmt @[5:4] 0:rgb565, 1: rgb888, 2: xrgb8888, 3:rgbx8888
     *(volatile unsigned int *) DPI_FIFO_MAX = 0x00000200       ; //DPI_FIFO_MAX
    
     *(volatile unsigned int *) DSI_VSA_NL = 0x00000003   	; //DSI_VSA_NL (Set Vertical Sync pulse=2)
     if (g_VideoModeType==1)
     {
     *(volatile unsigned int *) DSI_VBP_NL = 0x0000000c   	; //DSI_VBP_NL (Set Vertical Bachporch=2) (Himax suggest 2, but it may suffer line shift unless we set 12=0x0C)
     }
     else
     {
     *(volatile unsigned int *) DSI_VBP_NL = 0x0000000f   	; //DSI_VBP_NL (Set Vertical Bachporch=2) (Himax suggest 2, but it may suffer line shift unless we set 12=0x0E)
     }
	
     *(volatile unsigned int *) DSI_VFP_NL = 0x00000002   	; //DSI_VFP_NL (Set Vertical Frontachporch=2)
     *(volatile unsigned int *) DSI_VACT_NL = 0x00000320   ; //DSI_VACT_NL(Set Vertical Active line=800)    
     *(volatile unsigned int *) DSI_LINE_NB = 0x00000800   	; //DSI_LINE_NB 
     *(volatile unsigned int *) DSI_HSA_NB = 0x0000001A   	; //DSI_HSA_NB (Only use for Sync_Pulse mode)
     *(volatile unsigned int *) DSI_HBP_NB = 0x00000092   	; //DSI_HBP_NB (Himax suggest HBP>2us, Set to 50 based on 494Mbps)
     *(volatile unsigned int *) DSI_HFP_NB = 0x00000092   	; //DSI_HFP_NB (Himax suggest HBP>2us, Set to 50 based on 494Mbps)
     *(volatile unsigned int *) DSI_HSA_WC = 0x00000014   	; //DSI_HSA_WC = HSA_NB-6
     *(volatile unsigned int *) DSI_HBP_WC = 0x0000008C   ; //DSI_HBP_WC = HBP_NB-6
     *(volatile unsigned int *) DSI_HFP_WC = 0x0000008C   	;//DSI_HFP_WC = HFP_NB-6
//------------------------------------------------------------------------
// DPI color format related (DPI setting)
//------------------------------------------------------------------------
//*(volatile unsigned int *) DSI_PSCON = 0x000025A0   	; //DSI_PSCON, [13:12] 0:packed rgb565  [11:0]=widthxbpp
                                       	                //                   1:lossely rgb666
                                       	                //                   2:packed rgb888
                                       	                //                   3:packed rgb666
                                       	                //only valid when video mode


    switch(g_ColorFormat)
    {
    case PACKED_RGB565:
        *(volatile unsigned int *) DSI_RGB_NB = 0x000003C6   	; //DSI_RGB_NB (RGB*BPP+6 ex. 480x800 -> 480x3+6=1446 -> 0x5A6)
        *(volatile unsigned int *) DSI_PSCON  = ((0x000003C0)|(g_ColorFormat<<0xC))         ; //config for video mode color fmt 
	break;
    case LOOSED_RGB666:
        *(volatile unsigned int *) DSI_RGB_NB = 0x000005A6   	; //DSI_RGB_NB (RGB*BPP+6 ex. 480x800 -> 480x3+6=1446 -> 0x5A6)
        *(volatile unsigned int *) DSI_PSCON  = ((0x000005A0)|(g_ColorFormat<<0xC))         ; //config for video mode color fmt 
	break; 
    case PACKED_RGB888:
        *(volatile unsigned int *) DSI_RGB_NB = 0x000005A6   	; //DSI_RGB_NB (RGB*BPP+6 ex. 480x800 -> 480x3+6=1446 -> 0x5A6)
        *(volatile unsigned int *) DSI_PSCON  = ((0x000005A0)|(g_ColorFormat<<0xC))         ; //config for video mode color fmt 
        break;
    case PACKED_RGB666:
        *(volatile unsigned int *) DSI_RGB_NB = 0x0000043E   	; //DSI_RGB_NB (RGB*BPP+6 ex. 480x800 -> 480x3+6=1446 -> 0x5A6)
        *(volatile unsigned int *) DSI_PSCON  = ((0x00000438)|(g_ColorFormat<<0xC))         ; //config for video mode color fmt 
        break;   		
    default:
        //dbg_print("Format setting error \n\r");
        while(1);
        break;
    }

                                            
     *(volatile unsigned int *) DSI_MODE_CON = g_VideoModeType; //[1:0]:DSI_MODE_CON, 0:command mode, 1:sync_pulse, 2:sync_event, 3:burst
     
     *(volatile unsigned int *) DPI_EN = 0x000             ; //start dpi
     *(volatile unsigned int *) DPI_EN = 0x001             ; //start dpi
     //delay_a_while_loop(A_CONSTANT_SHORT_DELAY);
     REG_DSI_START = 0x0;
     //delay_a_while_loop(A_CONSTANT_DELAY);
     REG_DSI_START = 0x1;
}
#endif

// ---------------------------------------------------------------------------
//  DBI Display Driver Public Functions
// ---------------------------------------------------------------------------

bool DDMS_capturing=0;

static DISP_STATUS dsi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
	if (!disp_drv_dsi_init_context()) 
		return DISP_STATUS_NOT_IMPLEMENTED;

	if(lcm_params->dsi.mode == CMD_MODE) {
	
		init_lcd();
		init_dsi();

		init_assertion_layer(fbVA, fbPA);		

		if (NULL != lcm_drv->init && !isLcmInited) {
			lcm_drv->init();
		}

		DSI_clk_ULP_mode(0);

		OUTREG32(&DSI_REG->DSI_MODE_CTRL, lcm_params->dsi.mode);			
		
		
		init_lcd_te_control();
		DPI_PowerOn();
		DPI_PowerOff();
	}
	else {

		init_intermediate_buffers(fbPA);
		
		init_mipi_pll();
		init_io_pad();
		
	    init_lcd();
		init_dpi(isLcmInited);
		init_dsi();

		init_assertion_layer(fbVA, fbPA);

		if (NULL != lcm_drv->init && !isLcmInited) {
			lcm_drv->init();
		}

		//DSI_Init_VideoMode();

		DSI_clk_ULP_mode(0);
		
		OUTREG32(&DSI_REG->DSI_MODE_CTRL, lcm_params->dsi.mode);

		DSI_CHECK_RET(DSI_EnableClk());

	}
	
	return DISP_STATUS_OK;
}

static DISP_STATUS dsi_enable_power(BOOL enable)
{
	DSI_PHY_REG_ANACON0 mipitx_con0 = DSI_PHY_REG->ANACON0;

	disp_drv_dsi_init_context();
	
	if(lcm_params->dsi.mode == CMD_MODE) {

		if (enable) {
			DSI_PHY_clk_setting();
			DSI_CHECK_RET(DSI_PowerOn());
			DSI_lane0_ULP_mode(0);	
			DSI_clk_ULP_mode(0);			
			LCD_CHECK_RET(LCD_PowerOn());
		} else {
			LCD_CHECK_RET(LCD_PowerOff());
			DSI_CHECK_RET(DSI_PowerOff());
	        mipitx_con0.PLL_EN=0;
	        OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&mipitx_con0));
		}
	} else {

	    if (enable) {
	        DSI_CHECK_RET(DSI_PowerOn());
	        DPI_CHECK_RET(DPI_PowerOn());
	        LCD_CHECK_RET(LCD_PowerOn());
	        DPI_CHECK_RET(DPI_EnableClk());
	        DSI_CHECK_RET(DSI_EnableClk());
	    } else {
	        DSI_CHECK_RET(DSI_DisableClk());
	        DPI_CHECK_RET(DPI_DisableClk());
	        LCD_CHECK_RET(LCD_PowerOff());
	        DPI_CHECK_RET(DPI_PowerOff());
	        DSI_CHECK_RET(DSI_PowerOff());
	    }
	}

    return DISP_STATUS_OK;
}

static DISP_STATUS dsi_set_fb_addr(UINT32 fbPhysAddr)
{
    LCD_CHECK_RET(LCD_LayerSetAddress(FB_LAYER, fbPhysAddr));

    return DISP_STATUS_OK;
}


static DISP_STATUS dsi_update_screen(void)
{
	disp_drv_dsi_init_context();

#if defined(CONFIG_ARCH_MT6573)
	*(volatile unsigned int *) (APCONFIG_BASE+0x0044) |= 0x00000100;	// enable MIPI TX IO
#endif

	//printk("[DISP] kernel - DSI_handle_TE \n"); 	
	//DSI_CHECK_RET(DSI_handle_TE());

	if(lcm_params->dsi.mode == CMD_MODE && !DDMS_capturing) {
		LCD_CHECK_RET(LCD_StartTransfer(FALSE));
		DSI_CHECK_RET(DSI_EnableClk());
	}

	if (DDMS_capturing)
		printk("[DISP] kernel - dsi_update_screen. DDMS is capturing. Skip one frame. \n");		

	return DISP_STATUS_OK;
}


static UINT32 dsi_get_vram_size(void)
{
    UINT32 vramSize;

    disp_drv_dsi_init_context();

	if(lcm_params->dsi.mode == CMD_MODE) {
		vramSize = get_fb_size() + get_assert_layer_size();
	} else {
		vramSize = get_fb_size() + get_assert_layer_size() + get_intermediate_buffer_size();
	}
    
    return vramSize;
}

static PANEL_COLOR_FORMAT dsi_get_panel_color_format(void)
{
    disp_drv_dsi_init_context();

	if(lcm_params->dsi.mode == CMD_MODE) {

	    switch(lcm_params->dbi.data_format.format)
	    {
		    case LCM_DBI_FORMAT_RGB332 : return PANEL_COLOR_FORMAT_RGB332;
		    case LCM_DBI_FORMAT_RGB444 : return PANEL_COLOR_FORMAT_RGB444;
		    case LCM_DBI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
		    case LCM_DBI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
		    case LCM_DBI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
		    default : ASSERT(0);
	    }
		
	} else {

		switch(lcm_params->dpi.format)
		{
			case LCM_DPI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
			case LCM_DPI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
			case LCM_DPI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
			default : ASSERT(0);
		}
	}
}

static UINT32 dsi_get_dithering_bpp(void)
{
	return PANEL_COLOR_FORMAT_TO_BPP(dsi_get_panel_color_format());
}

DISP_STATUS dsi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
	LCD_REG_DSI_DC tmp_reg;

	tmp_reg=LCD_REG->DS_DSI_CON;

	DSI_CHECK_RET(DSI_WaitForNotBusy());

	DDMS_capturing=1;

	// W2DSI = 0
	tmp_reg.DC_DSI=0;
	OUTREG32(&LCD_REG->DS_DSI_CON, AS_UINT32(&tmp_reg));

    LCD_CHECK_RET(LCD_Capture_Framebuffer(pvbuf, bpp));

	// W2DSI = 1
	tmp_reg.DC_DSI=1;
	OUTREG32(&LCD_REG->DS_DSI_CON, AS_UINT32(&tmp_reg));

	DDMS_capturing=0;

	return DISP_STATUS_OK;	
}

const DISP_DRIVER *DISP_GetDriverDSI()
{
    static const DISP_DRIVER DSI_DISP_DRV =
    {
        .init                   = dsi_init,
        .enable_power           = dsi_enable_power,
        .set_fb_addr            = dsi_set_fb_addr,
        .update_screen          = dsi_update_screen,       
        .get_vram_size          = dsi_get_vram_size,

        .get_panel_color_format = dsi_get_panel_color_format,
        .init_te_control        = init_lcd_te_control,
        .get_dithering_bpp		= dsi_get_dithering_bpp,
				.capture_framebuffer		= dsi_capture_framebuffer,
    };

    return &DSI_DISP_DRV;
}

