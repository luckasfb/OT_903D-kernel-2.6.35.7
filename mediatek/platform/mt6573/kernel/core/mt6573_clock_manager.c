


#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/tcm.h>
#include <asm/uaccess.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/mt6573_eint.h"
#include "mach/dma.h"
#include "mach/sync_write.h"
#include "mach/mt6573_pll.h"
#include "mach/mt6573_boot.h"
#include "mach/mt6573_typedefs.h"
#include "mach/mtkpm.h"
#include "mach/mt6573_ost_sm.h"
#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"
#include "auddrv_register.h"

static spinlock_t MT65XX_csClock_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t MT65XX_csPLL_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t MT65XX_csSYS_lock = SPIN_LOCK_UNLOCKED;

BOOL finish_log_init = FALSE;

/* Debug message event */
#define DBG_PMAPI_MASK       (CM_DBG_FLAG)

#if 1
#define MSG(evt, fmt, args...) \
do {    \
    if ((DBG_PMAPI_##evt) & DBG_PMAPI_MASK) { \
        printk(fmt, ##args); \
    } \
} while(0)

#define MSG_FUNC_ENTRY(f)    MSG(ENTER, "<PMAPI FUNC>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...) do{}while(0)
#define MSG_FUNC_ENTRY(f)       do{}while(0)
#endif



char MT65XX_Peri0_name[MT65XX_PDN_PERI0_END-MT65XX_PDN_PERI0_BEGIN+1][MAX_MOD_NAME]=
{
    "PERI_MSDC",   
    "PERI_MSDC2" , 
    "PERI_MSDC3" , 
    "PERI_MSDC4" , 
    "PERI_ADC2G" , 
    "PERI_TP" ,    
    "PERI_2GTX" ,  
    "PERI_USB",    
    "PERI_USB11" , 
    "PERI_UART1" , 
    "PERI_UART2" , 
    "PERI_UART3" , 
    "PERI_UART4" , 
    "PERI_ADC3G"  
};         

char MT65XX_Peri1_name[MT65XX_PDN_PERI1_END-MT65XX_PDN_PERI1_BEGIN+1][MAX_MOD_NAME]=
{
    "PERI_OSTIMER_APARM" ,  
    "PERI_SIMIF0",   
    "PERI_SIMIF1" ,  
    "PERI_IRDA",   
    "PERI_APDMA" , 
    "PERI_PWM"  ,  
    "PERI_MIXEDSYS" ,
    "PERI_NFI",   
    "PERI_I2C" ,  
    "PERI_I2C2" , 
    "PERI_SEJ" ,  
    "PERI_ONEWIRE" , 
    "PERI_TRNG",    
};         




char MT65XX_MM1_p0_name[MT65XX_PDN_MM1_PART0_END-MT65XX_PDN_MM1_PART0_BEGIN+1][MAX_MOD_NAME]=
{

    "MM_GMC1" ,            
    "MM_G2D" ,               
    "MM_GCMQ" ,            
    "MM_JPGENC"  ,       
    "MM_IDP_MOUT0" ,       
    "MM_JPGDEC"   ,      
    "MM_JPEG_DMA" ,        
    "MM_IDP_MOUT1" ,       
    "MM_OVL_DMA"  ,                       
    "MM_ROT_DMA2" ,                        
    "MM_ISP"   ,                           
    "MM_IPP"  ,                            
    "MM_PRZ0"   ,                          
    "MM_CRZ"  ,                            
    "MM_VRZ" ,                             
    "MM_EIS" ,                             
    "MM_GIF"  ,                            
    "MM_R_DMA0" ,                      
    "MM_R_DMA1" ,                      
    "MM_PNG"   ,                           
    "MM_ROT_DMA0" ,                        
    "MM_ROT_DMA1" ,                        
    "MM_RESZ_LB" ,                         
    "MM_LCD" ,                             
    "MM_DPI" ,                             
    "MM_CSI2"  ,                           
    "MM_OVL_DMA_BPS"  ,                   
    "MM_FDVT"  ,                          
    "MM_DSI"  ,                            
    "MM_R_DMA1_BPS"    ,              
    "MM_MMLMU",                            
    "MM_BRZ"   ,         
};         
char MT65XX_MM1_p1_name[MT65XX_PDN_MM1_PART1_END-MT65XX_PDN_MM1_PART1_BEGIN+1][MAX_MOD_NAME]=
{
    "MM_PRZ1" ,            
    "MM_ROT_DMA3" ,        
    "MM_IDP_MOUT2"  ,      
    "MM_IDP_MOUT3",        
    "MM_TVC" ,             
    "MM_TVE" ,             
    "MM_TV_ROT"  ,     
    "MM_HIF",              
    "MM_GMC1E"  ,        
    "MM_GMC1SLV"  ,       
};         

char MT65XX_MM2_name[MT65XX_PDN_MM2_END-MT65XX_PDN_MM2_BEGIN+1][MAX_MOD_NAME]=
{
    "MM_GMC2",         
    "MM_MFV" ,         
    "MM_MFG_CORE"  ,   
    "MM_MFG_MEM" ,     
    "MM_MFG_SYS",      
    "MM_SPI" ,         
    "MM_GMC2_E" ,  
};         

char MT65XX_AUDIO_name[MT65XX_PDN_AUDIO_END-MT65XX_PDN_AUDIO_BEGIN+1][MAX_MOD_NAME]=
{
    "NOP",
    "NOP",
    "AUDIO_AFE",
    "NOP",
    "AUDIO_SIGMADSP" ,   
    "AUDIO_ADC",     
    "AUDIO_I2S" ,    
};         
DWORD MT65XX_PLL_LOCK_REG[MT65XX_PLL_COUNT_END] = 
{
    AMPLL_CON3_REG,
    MPLL_CON3_REG,
    DPLL_CON3_REG,
    EPLL_CON3_REG,
    WPLL_CON0_REG,
    GPLL_CON0_REG,
    CPLL_CON0_REG,
    THREEDPLL_CON0_REG,
    TVPLL_CON0_REG,
    FGPLL_CON0_REG,
    AUXPLL_CON0_REG,
    0xf7080800,
    0x00000000//no lock reg, HW control
};
DWORD MT65XX_PLL_LOCK[MT65XX_PLL_COUNT_END] = 
{
	AMPLL_LOCKED,
	MPLL_LOCKED,
	DPLL_LOCKED,
	EPLL_LOCKED,
	WPLL_LOCKED,
	GPLL_LOCKED,
	CPLL_LOCKED,
	THREEDPLL_LOCKED,
	TVPLL_LOCKED,
	FGPLL_LOCKED,
	AUXPLL_LOCKED,
	0x0,
	0x0//no lock, HW control
};

char MT65XX_PLL_name[MT65XX_PLL_COUNT_END+1][MAX_MOD_NAME]=
{
    "APMCU_PLL",                         
    "MDMCU_PLL",                         
    "DSP_PLL",                           
    "EMI_PLL",                           
    "3G_PLL",                            
    "2G_PLL",                            
    "CAMERA_PLL",                        
    "3D_PLL",                            
    "TV_PLL",                            
    "FG_PLL",                            
    "AUX_PLL",
    "MIPI_PLL",
    "26M_PLL"
};

char MT65XX_SUBSYS_name[MT65XX_SUBSYS_COUNT_END+1][MAX_MOD_NAME]=
{
    "MD2G_SUBSYS",
    "HSPA_SUBSYS",
    "FC4_SUBSYS", 
    "AUDIO_SUBSYS",
    "MM1_SUBSYS",
    "MM2_SUBSYS",
    "MD_SUBSYS",
    "AP_SUBSYS" 
};

char MT65XX_CATEGORY_name[MT65XX_CLOCK_CATEGORY_COUNT][MAX_MOD_NAME]=
{
    "APMCU0",
    "APMCU1",
    "MM1_CON0", 
    "MM1_CON1",
    "MM2",
    "AUDIO"
};

DWORD MT65XX_CG_CON[MT65XX_CLOCK_CATEGORY_COUNT]=
{
    APMCU_CG_CON0,
    APMCU_CG_CON1,
    MMSYS1_CG_CON0,
    MMSYS1_CG_CON1,
    MMSYS2_CG_CON0,
    AUDIO_TOP_CON0_REG
};
DWORD MT65XX_CLOCK_NUMBER[MT65XX_CLOCK_CATEGORY_COUNT]=
{
    14,
    13,
    32,
    10,
    7,
    7
};

DWORD MT65XX_CG_CLR[MT65XX_CLOCK_CATEGORY_COUNT]=
{
    APMCU_CG_CLR0,
    APMCU_CG_CLR1,
    MMSYS1_CG_CLR0,
    MMSYS1_CG_CLR1,
    MMSYS2_CG_CLR0,
    AUDIO_TOP_CON0_REG
};
DWORD MT65XX_CG_SET[MT65XX_CLOCK_CATEGORY_COUNT]=
{
    APMCU_CG_SET0,
    APMCU_CG_SET1,
    MMSYS1_CG_SET0,
    MMSYS1_CG_SET1,
    MMSYS2_CG_SET0,
    AUDIO_TOP_CON0_REG
};

DWORD MT65XX_PLL_REG[MT65XX_PLL_COUNT_END]=
{
    AMPLL_CON0_REG,
    MPLL_CON0_REG,
    DPLL_CON0_REG,
    EPLL_CON0_REG,
    WPLL_CON0_REG,
    GPLL_CON0_REG,
    CPLL_CON0_REG,
    THREEDPLL_CON0_REG,
    TVPLL_CON0_REG,
    FGPLL_CON0_REG,
    AUXPLL_CON0_REG,
    0xf7080800,
    0x00000000//no pll reg, HW control
};

DWORD MT65XX_POWER_REG[MT65XX_POWER_COUNT_END] =
{
    VA28_CON0,
    VA25_CON0,
    VA12_CON0,
    VRTC_CON0,
    VMIC_CON0,

    VTV_CON0,
    VAUDN_CON0,
    VAUDP_CON0, 
    PMUA_CON0,
    VRF_CON0,
    VCAMA_CON0, 

    VCAMD_CON0,
    VIO_CON0,
    VUSB_CON0,
    VSIM_CON0,
    VSIM2_CON0,

    VIBR_CON0,
    VMC_CON0,
    VCAMA2_CON0,
    VCAMD2_CON0,
    VM12_CON0,
	
    VM12_INT_CON0
};
MT65XX_PLL MT65XX_CLOCK_TYPE_VALUE[MT65XX_CLOCK_TYPE_COUNT][MAX_PLL_COUNT]=
{
	{MT65XX_26M_PLL, MT65XX_3G_PLL, MT65XX_2G_PLL, MT65XX_EMI_PLL},//FWMA_CK
	{MT65XX_26M_PLL, MT65XX_3G_PLL, MT65XX_2G_PLL, MT65XX_EMI_PLL},//FMSDC_CK
	{MT65XX_AUX_PLL, MT65XX_26M_PLL, -1, -1},//FSYS_CK
	{MT65XX_26M_PLL, MT65XX_2G_PLL, -1, -1},//FUSB_CK
	{MT65XX_26M_PLL, MT65XX_EMI_PLL, MT65XX_AUX_PLL, MT65XX_2G_PLL},//FEMI_CK
	{MT65XX_26M_PLL, MT65XX_MIPI_PLL, -1, -1},//FMIPI_CK
	{MT65XX_26M_PLL, MT65XX_CAMERA_PLL, MT65XX_2G_PLL, -1},//FCAM_CK
	{MT65XX_26M_PLL, MT65XX_TV_PLL, -1, -1},//FTV_CK
	{MT65XX_26M_PLL, MT65XX_3D_PLL, -1, -1},//F3D_CK
	{MT65XX_26M_PLL, MT65XX_APMCU_PLL, MT65XX_AUX_PLL, MT65XX_2G_PLL},//FAPMCU_CK
	{MT65XX_26M_PLL, MT65XX_MDMCU_PLL, MT65XX_AUX_PLL, -1},//FMDMCU_CK
	{MT65XX_26M_PLL, MT65XX_DSP_PLL, MT65XX_AUX_PLL, MT65XX_2G_PLL},//FDSP_CK
	{MT65XX_26M_PLL, MT65XX_2G_PLL, -1, -1},//FGSM_CK
	{MT65XX_26M_PLL, MT65XX_FG_PLL, -1, -1}//FFG_CK
};

DWORD MT65XX_PLLSEL_REG[MT65XX_CLOCK_TYPE_COUNT][2]=
{
      {PLL_CON5_REG, WMA_PLLSEL},//FWMA_CK
      {PLL_CON5_REG, MSDC_PLLSEL},//FMSDC_CK
      {PLL_CON6_REG, SYS_PLLSEL},//FSYS_CK
      {PLL_CON6_REG, USB_PLLSEL},//FUSB_CK
      {PLL_CON4_REG, EMI_PLLSEL},//FEMI_CK
      {0xf7080828, 0x0200},//FMIPI_CK  {MIPITX_CONA, MIPI_PLLSEL}, 0is MIPI, 1 is 26M, do not care about it
      {PLL_CON4_REG, CAM_PLLSEL},//FCAM_CK
      {PLL_CON6_REG, TV_PLLSEL},//FTV_CK
      {PLL_CON6_REG, THREED_PLLSEL},//F3D_CK
      {PLL_CON5_REG, APMCU_PLLSEL},//FAPMCU_CK
      {PLL_CON4_REG, MDMCU_PLLSEL},//FMDMCU_CK
      {PLL_CON4_REG, DSP_PLLSEL},//FDSP_CK
      {PLL_CON5_REG, GSM_PLLSEL},//FGSM_CK
      {PLL_CON6_REG, FG_PLLSEL}//FFG_CK
};

char MT65XX_CLOCK_TYPE_name[MT65XX_CLOCK_TYPE_COUNT][MAX_MOD_NAME]=
{
	"FWMA_CK",
	"FMSDC_CK",
	"FSYS_CK",
	"FUSB_CK",
	"FEMI_CK",
	"FMIPI_CK",
	"FCAM_CK",
	"FTV_CK",
	"F3D_CK",
	"FAPMCU_CK",
	"FMDMCU_CK",
	"FDSP_CK",
	"FGSM_CK",
	"FFG_CK"
};

DWORD MT65XX_CLOCK_TYPE_OF_CK[MT65XX_CLOCK_TYPE_COUNT][MT65XX_CLOCK_CATEGORY_COUNT]=
{
   {//FWMA_CK
    0x3e70,//APMCU_CG_CON0
    0x1fff,//APMCU_CG_CON1
    0xeefffbff, //MMSYS1_CG_CON0
    0x3df,//MMSYS1_CG_CON1
    0x37,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   }, 
   {//FMSDC_CK
    0xf,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FSYS_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x7f//AUDIO_SYS_CON
   },
   {//FUSB_CK
    0x180,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FEMI_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x48,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FMIPI_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x11000000, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FCMA_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x400, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FTV_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x20,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//F3D_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FAPMCU_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FMDMCU_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FDSP_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FGSM_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   },
   {//FFG_CK
    0x0,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0, //MMSYS1_CG_CON0
    0x0,//MMSYS1_CG_CON1
    0x0,//MMSYS2_CG_CON
    0x0//AUDIO_SYS_CON
   }
};
DWORD MT65XX_SUBSYS_OF_CATEGORY[MT65XX_CLOCK_CATEGORY_COUNT]=
{
	AP_SUBSYS,
	AP_SUBSYS,
	MM1_SUBSYS,
	MM1_SUBSYS,
	MM2_SUBSYS,
	AUDIO_SUBSYS
};
INT32 MT65XX__CATEGORY_OF_SUBSYS[MT65XX_SUBSYS_COUNT_END]=
{
	-1,//MD2G_SUBSYS
	-1,//HSPA_SUBSYS
	-1,//FC4_SUBSYS
	CLOCK_AUDIOSYS,//AUDIO_SUBSYS	
	CLOCK_MMSYS1_PART0,//MM1_SUBSYS
	CLOCK_MMSYS2_PART0,//MM2_SUBSYS
	CLOCK_PERI_PDN0,//AP_SUBSYS
	-1//MD_SUBSYS
};

UINT32 MT65XX_IS_CLK_EN[MT65XX_CLOCK_CATEGORY_COUNT] = 
{
    0x018f,//APMCU_CG_CON0
    0x0,//APMCU_CG_CON1
    0x0,//MM1_SUBSYS_CON0
    0x0,//MM1_SUBSYS_CON1
    0x0,//MM2_SUBSYS_CON
    0x74//AUDIO_TOP_CON0_REG
};
char MT65XX_CLK_EN_name[MT65XX_CLK_EN_COUNT_END][MAX_MOD_NAME] = 
{
    "MSDC_CLK_EN",
    "USB_CLK_EN",
    "AUD_CLK_EN"
};
DEVICE_CLK_EN g_clken[MT65XX_CLK_EN_COUNT_END];
static DEFINE_MUTEX(clock_listener_lock);
static LIST_HEAD(clock_listener_list);
static BOOL clock_listen_enable = true;
 
DWORD MT65XX_CLK_EN_REG[MT65XX_CLK_EN_COUNT_END][2] =
{
    {WPLL_CON0_REG, 0x80},
    {GPLL_CON0_REG, 0x80},
    {PLL_CON2_REG, 0x20}
};


BOOL set_clock_showlog(INT32 clockId, BOOL enable)
{
    INT32 category = 0, offset;
    category = clockId / 32;
    offset = clockId %32;

    if (category >= MT65XX_CLOCK_CATEGORY_COUNT|| category < 0 || offset >= MT65XX_CLOCK_NUMBER[category] || offset < 0 || clockId == 160 || clockId == 161 || clockId == 163)
    {
       MSG(CG,"[%s]: bad ClockId[%d, 0x%x], enable=%d\r\n", __FUNCTION__, category, offset, enable);
       return FALSE;
    }  
    g_MT6573_BusHW.Clock[category][offset].showlog = enable;
    MSG(CG, "[%s]: clock[%d, 0x%x, %s], enable:%d\r\n", __FUNCTION__, category, offset, g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.Clock[category][offset].showlog);
    return TRUE;
}

BOOL set_clkEn_showlog(INT32 clk_en_type, BOOL enable)
{
    if((clk_en_type >= MT65XX_MSDC_CLK_EN) && (clk_en_type < MT65XX_CLK_EN_COUNT_END)) 
    {
       g_clken[clk_en_type].showlog = enable;
       MSG(CG, "[%s]: clkEn[%d, %s], enable:%d\r\n", __FUNCTION__, clk_en_type, g_clken[clk_en_type].name, g_clken[clk_en_type].showlog);
    }else{
       MSG(CG, "[%s]: bad clkEn[%d], enable:%d\r\n", __FUNCTION__, clk_en_type, g_clken[clk_en_type].showlog);
       return FALSE;
    }
    return TRUE;
}

BOOL set_subsys_showlog(INT32 subsys, BOOL enable)
{
    if((subsys >= MD2G_SUBSYS) && (subsys < MT65XX_SUBSYS_COUNT_END))
    {
       if(enable)
          g_MT6573_BusHW.dwSubSystem_showlog |= (1<<subsys);
       else
          g_MT6573_BusHW.dwSubSystem_showlog &= ~(1<<subsys);
       MSG(SUB, "[%s]: subsys[%d, %s], enable:0x%x\r\n", __FUNCTION__, subsys, MT65XX_SUBSYS_name[subsys], g_MT6573_BusHW.dwSubSystem_showlog);
    }else {
       MSG(SUB, "[%s]: bad subsys[%d], enable:0x%x\r\n", __FUNCTION__, subsys, g_MT6573_BusHW.dwSubSystem_showlog);
       return FALSE;
    }
    return TRUE;
}

void set_clock_listen(BOOL enable)
{
     mutex_lock(&clock_listener_lock);
     clock_listen_enable = enable;
     mutex_unlock(&clock_listener_lock);
     MSG(CG_LIS, "[%s]: clock_listen_enable=%d\r\n", __FUNCTION__, clock_listen_enable);
}

void show_clock_listen(void)
{
     struct clock_listener *pos;

     mutex_lock(&clock_listener_lock);
     MSG(CG_LIS,"clock_listen_enable=%d\r\n", clock_listen_enable);
     list_for_each_entry(pos, &clock_listener_list, link) {
        MSG(CG_LIS,"  listener:%s   level:%d\r\n", pos->name, pos->level);
     }
     mutex_unlock(&clock_listener_lock);
}

void register_clock_listener(struct clock_listener *listener)
{
     struct list_head *pos;

     if(!clock_listen_enable) 
     {
        MSG(CG_LIS, "[%s]: failed to register clock_listener[%s], clock_listen_enable=%d\r\n", __FUNCTION__, listener->name, clock_listen_enable);
        return;
     }
     mutex_lock(&clock_listener_lock);
     list_for_each(pos, &clock_listener_list) {
        struct clock_listener *e;
        e = list_entry(pos, struct clock_listener, link);
        if (e->level > listener->level)
           break;
     }
     list_add_tail(&listener->link, pos);
     mutex_unlock(&clock_listener_lock);
     MSG(CG_LIS, "[%s]: clock_listener[%s] finish register\r\n", __FUNCTION__, listener->name);
}

void unregister_clock_listener(struct clock_listener *listener)
{
     if(!clock_listen_enable) 
     {
        MSG(CG_LIS, "[%s]: failed to unregister clock_listener[%s], clock_listen_enable=%d\r\n", __FUNCTION__, listener->name, clock_listen_enable);
        return;
     }
     mutex_lock(&clock_listener_lock);
     list_del(&listener->link);
     mutex_unlock(&clock_listener_lock);
     MSG(CG_LIS, "[%s]: clock_listener[%s] finish unregister\r\n", __FUNCTION__, listener->name);
}
 
BOOL POWER_Status(DWORD powerId)
{
    if(DRV_Reg32(MT65XX_POWER_REG[powerId]) & 0x1)
    {
        return TRUE;
    }else
    {
        return FALSE;
    }
}


BOOL isPllCanBeFsel(MT65XX_PLL pllId)
{
	if(0x4cf & (1<<pllId))
	     return TRUE;
	return FALSE;
}

BOOL isPllCanBeSwitched(DWORD clock_type, DWORD pllId)
{
    INT32 i = 0;
    if(g_MT6573_BusHW.dwClock_active_pll[clock_type] == pllId)
    {
        return TRUE;
    }
    if(g_MT6573_BusHW.dwClock_active_pll[clock_type] >= 0)
    {
        for(i=0; i<MT65XX_CLOCK_CATEGORY_COUNT; i++)
        {
           if(MT65XX_CLOCK_TYPE_OF_CK[clock_type][i] & g_MT6573_BusHW.dwSubSystem_clock[i])
           {			
               MSG(CG,"[%s]: clock_type[%s] can not be switched, active_pll_source[%s]!\r\n", __FUNCTION__, MT65XX_CLOCK_TYPE_name[clock_type], MT65XX_PLL_name[g_MT6573_BusHW.dwClock_active_pll[clock_type]]);
               return FALSE;
           }
        }
    }
    g_MT6573_BusHW.dwClock_active_pll[clock_type] = pllId;
    MSG(CG,"[%s]: clock_type[%s] can be switched, active_pll_source[%s]!\r\n", __FUNCTION__, MT65XX_CLOCK_TYPE_name[clock_type], MT65XX_PLL_name[g_MT6573_BusHW.dwClock_active_pll[clock_type]]);
    return TRUE;
}

INT16 PLL_Fsel_Read(MT65XX_PLL pllId)
{
    register UINT16 reg_value;
    if(!isPllCanBeFsel(pllId))
    {
        return -2;//pll's freq can not be changed
    }
    if(!PLL_Status(pllId))
    {
        return -1;//pll is disabled
    }

    reg_value = DRV_Reg16(MT65XX_PLL_REG[pllId]);
    reg_value &= 0x7f00;
    reg_value >>= 8;

    return reg_value;
}

void PLL_Fsel(MT65XX_PLL pllId, UINT16 pll_fsel_value)
{
   register UINT16 reg_value;

   if(!isPllCanBeFsel(pllId))
   {
       MSG(PLL, "[%s]: pll[%d, %s] fesl can not be configed\r\n", __FUNCTION__, pllId, MT65XX_PLL_name[pllId]);
   	   return;
   }
   if(!PLL_Status(pllId))
   {
       MSG(PLL, "[%s]: pll[%d, %s] is disabled, please enable it first\r\n", __FUNCTION__, pllId, MT65XX_PLL_name[pllId]);
       return;
   }

   /* Read current AMPPL CON0 register value into local variable*/
   reg_value = DRV_Reg16(MT65XX_PLL_REG[pllId]);
   MSG(PLL, "[%s]: read pll[%d, %s] reg[0x%x=0x%x]\r\n", __FUNCTION__, pllId, MT65XX_PLL_name[pllId],MT65XX_PLL_REG[pllId],DRV_Reg16(MT65XX_PLL_REG[pllId]));
  
   /* Clear APPS_MCU_PLL_FSEL field in local variable*/
   reg_value &= 0x80FF;

   /* Set APPS_MCU_PLL_FSEL field as required in local variable*/
   reg_value |= pll_fsel_value << 8;

   /* reconfigure digital dividers here if necessary*/
   DRV_WriteReg16(MT65XX_PLL_REG[pllId],reg_value);
   MSG(PLL, "[%s]: write pll[%d, %s] reg[0x%x=0x%x]\r\n", __FUNCTION__, pllId, MT65XX_PLL_name[pllId],MT65XX_PLL_REG[pllId],DRV_Reg16(MT65XX_PLL_REG[pllId]));
	 
   /* Wait for lock*/
   while (!(DRV_Reg16(MT65XX_PLL_LOCK_REG[pllId]) & MT65XX_PLL_LOCK[pllId]));
   MSG(PLL, "[%s]: config pll[%d, %s] fsel[0x%x] reg[0x%x=0x%x]\r\n", __FUNCTION__, pllId, MT65XX_PLL_name[pllId],pll_fsel_value, MT65XX_PLL_REG[pllId],DRV_Reg16(MT65XX_PLL_REG[pllId]));

   return;
}
void PLL_Set(DWORD pllId)
{
    DRV_SetReg16(MT65XX_PLL_REG[pllId], 0x1);
}
void PLL_Clear(DWORD pllId)
{
    DRV_ClrReg16(MT65XX_PLL_REG[pllId], 0x1);
}
BOOL PLL_Status(DWORD pllId)
{
    if(DRV_Reg16(MT65XX_PLL_REG[pllId]) & 0x1)
    {
        return TRUE;
    }else
    {
        return FALSE;
    }
}
void CG_Clear (DWORD category, UINT32 mode)
{
    if(category == CLOCK_AUDIOSYS)
        DRV_ClrReg32(MT65XX_CG_CLR[category], mode);
    else
        DRV_SetReg32(MT65XX_CG_CLR[category], mode);
}

void CG_Set (DWORD category, UINT32 mode)
{
    DRV_SetReg32(MT65XX_CG_SET[category], mode);
}

BOOL CG_Status(DWORD category, UINT32 mode)
{
    if(DRV_Reg32(MT65XX_CG_CON[category]) & mode)
    {      
        return TRUE;
    }
    else
    {     
        return FALSE;   
    }	
}

BOOL hwSetClockType(MT65XX_CLOCK clockId, MT65XX_CLOCK_TYPE clockType, char *mode_name)
{
    INT32 category = 0, offset;
    if(!finish_log_init)
    {
        MSG(CG, "[%s]: master[%s] please set clock type of clock[%d] after clock manager initiation",__FUNCTION__, mode_name, clockId);
        return FALSE;
    }
    category = clockId / 32;
    offset = clockId %32;
     
    if (category >= MT65XX_CLOCK_CATEGORY_COUNT|| category < 0 || offset >= MT65XX_CLOCK_NUMBER[category] || offset < 0 || clockId == 160 || clockId == 161 || clockId == 163)
    {
        MSG(CG, "[%s]: ClockId[%d] is wrong\r\n", __FUNCTION__, clockId);
        return FALSE;
    }
    
    if(clockType < FWMA_CK || clockType >= MT65XX_CLOCK_TYPE_COUNT)
    {
        MSG(CG, "[%s]: ClockType[%d] is wrong\r\n", __FUNCTION__, clockType);
        return FALSE;
    }   

    spin_lock(&MT65XX_csClock_lock);
    if(g_MT6573_BusHW.Clock[category][offset].clock_type != clockType)
    {        
        UINT32 oldType = g_MT6573_BusHW.Clock[category][offset].clock_type;
        g_MT6573_BusHW.Clock[category][offset].clock_type = clockType;
       
        if(g_MT6573_BusHW.Clock[category][offset].showlog)
           MSG(CG, "[%s]: clock[%s]'s clockType has been changed from [%d]%s to [%d]%s\r\n", __FUNCTION__, g_MT6573_BusHW.Clock[category][offset].name, oldType, MT65XX_CLOCK_TYPE_name[oldType], clockType, MT65XX_CLOCK_TYPE_name[clockType]);
    }
    spin_unlock(&MT65XX_csClock_lock);
    return TRUE;
}

INT32 isActivePllSource(DWORD pllId, INT32 category, INT32 offset)
{
    int i=0;

    if((pllId < 0) || (pllId >= MT65XX_PLL_COUNT_END))
        return -1;
	
    for(i = 0; i< MAX_PLL_COUNT; i++)
    {
        if(MT65XX_CLOCK_TYPE_VALUE[g_MT6573_BusHW.Clock[category][offset].clock_type][i] == pllId)
           return i;
    }
    return -1;
}
DWORD getPllSourceOfClock(DWORD category, UINT32 clock_mode)
{
    UINT32 i = 0;
    DWORD pllType = 0;

    for(i = 0; i< MT65XX_CLOCK_TYPE_COUNT; i++)
    {
        if(MT65XX_CLOCK_TYPE_OF_CK[i][category] & clock_mode)
        { 
            pllType = i;
            break;
        }
    }

    return pllType;

}



void getClock_Name(DWORD clock_category, DWORD clock_offset, char* clock_name)
{	
    switch(clock_category)
    {
        case CLOCK_PERI_PDN0:
            sprintf(clock_name , "%s", MT65XX_Peri0_name[clock_offset]);
            break;
        case CLOCK_PERI_PDN1:
            sprintf(clock_name , "%s", MT65XX_Peri1_name[clock_offset]);
            break;
        case CLOCK_MMSYS1_PART0:
            sprintf(clock_name , "%s", MT65XX_MM1_p0_name[clock_offset]);
            break;
        case CLOCK_MMSYS1_PART1:
            sprintf(clock_name , "%s", MT65XX_MM1_p1_name[clock_offset]);
            break;
        case CLOCK_MMSYS2_PART0:
            sprintf(clock_name , "%s", MT65XX_MM2_name[clock_offset]);
            break;
        case CLOCK_AUDIOSYS:
            sprintf(clock_name , "%s", MT65XX_AUDIO_name[clock_offset]);
            break;		
    }
	
}

void MTCMOS_En_reboot(void)
{
    volatile UINT32 reg_val = 0;

    if(get_chip_eco_ver() == CHIP_VER_E2)
    {
       //DRV_WriteReg32(RM_PWR_STA, 0xffffffff);
       DRV_SetReg32( (TOPSM_BASE+0x800+MM1_SUBSYS*4), (1<<PWR_REQ_EN));
       do{
           reg_val = DRV_Reg32(RM_PWR_STA);
       }while(!(reg_val & (1<<20)));//only AP or MD open MM1, MM1 will be open, so polling this status

       DRV_WriteReg32(TOPSM_DMY0,0x0);
       DRV_SetReg32(AP_SAPD_WAYEN, 1<< 8) ;

       MSG(SUB, "[%s] power on MMSYS1, TOPSM_BASE:[0x%x], AP_SAPD_WAYEN[0x%x]\r\n", __FUNCTION__,DRV_Reg32(TOPSM_BASE+0x800+MM1_SUBSYS*4), DRV_Reg32(AP_SAPD_WAYEN));	
    }
}

void MTCMOS_En(UINT32 subsys)
{
    volatile UINT32 reg_val = 0;
    struct clock_listener *pos;
    /* Subsys power software control */
    if((subsys == MM1_SUBSYS) && (get_chip_eco_ver() == CHIP_VER_E2))
    {
       //DRV_WriteReg32(RM_PWR_STA, 0xffffffff);
       DRV_SetReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_REQ_EN));
       do{
           reg_val = DRV_Reg32(RM_PWR_STA);
       }while(!(reg_val & (1<<20)));//only AP or MD open MM1, MM1 will be open, so polling this status
       DRV_WriteReg32(TOPSM_DMY0,0x0);
    }
    else
    {
       DRV_SetReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_CTRL));	
       /* Subsys Mem Power off and Subsys Power off */
       reg_val = DRV_Reg32(TOPSM_BASE+0x800+subsys*4);
       reg_val = ((reg_val | (1<<PWR_ON)) & ~(1<<PWR_MEM_OFF));
       DRV_WriteReg32((TOPSM_BASE+0x800+subsys*4), reg_val);
	
       /* MMSYS power */
       if (subsys == MM1_SUBSYS)
          DRV_WriteReg32(TOPSM_DMY0,0x0);
       if (subsys == MM2_SUBSYS)
          DRV_WriteReg32(TOPSM_DMY1,0x0);

       /* Subsys Clock on */
       DRV_ClrReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_CLK_DIS));

       /* Disable ISO*/
       DRV_ClrReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_ISO));

       /* Disassert Power on reset, Set bit 1*/
       DRV_SetReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_RST_B));

       /* Delay*/
       udelay(50);
    }
    if (subsys == AUDIO_SUBSYS) 
        DRV_SetReg32(AP_SAPD_WAYEN, 1<< 11) ;
    if (subsys == MM1_SUBSYS) 
        DRV_SetReg32(AP_SAPD_WAYEN, 1<< 8) ;
    if (subsys == MM2_SUBSYS) 
        DRV_SetReg32(AP_SAPD_WAYEN, 1<< 9) ;	

    if(subsys == MM1_SUBSYS)
    {
       DRV_WriteReg32(MMSYS1_CG_CLR0,0x1);
       DRV_WriteReg32(MMSYS1_CG_CLR1,0x300);

       /*notify clock listener to do restore after GMC1 is gated*/
       if(clock_listen_enable) 
       {
           mutex_lock(&clock_listener_lock);
           list_for_each_entry(pos, &clock_listener_list, link) {
               if (pos->ungate != NULL)
               {
                  MSG(CG_LIS, "[%s], clock_listener[%s] start ungate_callback\r\n", __FUNCTION__, pos->name);
                  pos->ungate(pos);
                  MSG(CG_LIS, "[%s], clock_listener[%s] finish ungate_callback\r\n", __FUNCTION__, pos->name);
               }
           }
           mutex_unlock(&clock_listener_lock);
       }
    }else if(subsys == MM2_SUBSYS)
    {
       DRV_WriteReg32(MMSYS2_CG_CLR0,0x41);
    }else if(subsys == AUDIO_SUBSYS)
    {
       DRV_ClrReg32(AUDIO_TOP_CON0_REG,0x4);
       DRV_SetReg16(PLL_CON2_REG, 0x20);
       udelay(50);
    }
    if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsys))
       MSG(SUB, "[%s] subSys[%s] TOPSM_BASE:[0x%x], AP_SAPD_WAYEN[0x%x]\r\n", __FUNCTION__,MT65XX_SUBSYS_name[subsys], DRV_Reg32(TOPSM_BASE+0x800+subsys*4), DRV_Reg32(AP_SAPD_WAYEN));	
}

void MTCMOS_Dis(UINT32 subsys)
{
    volatile UINT32 reg_val = 0;
    struct clock_listener *pos;
    /*notify clock listener to do backup before GMC1 is gated*/
    if(subsys == MM1_SUBSYS)
    {
       if(clock_listen_enable) 
       {
          mutex_lock(&clock_listener_lock);
          list_for_each_entry_reverse(pos, &clock_listener_list, link) {
             if (pos->gate != NULL)
             {
                MSG(CG_LIS, "[%s], clock_listener[%s] start gate_callback\r\n", __FUNCTION__, pos->name);
                pos->gate(pos);
                MSG(CG_LIS, "[%s], clock_listener[%s] finish gate_callback\r\n", __FUNCTION__, pos->name);
             }
          }
          mutex_unlock(&clock_listener_lock);
       }
    }

    /* AP slave i/f way disable setting */
    if (subsys == AUDIO_SUBSYS)	
        DRV_ClrReg32(AP_SAPD_WAYEN, 1<< 11)	;
    if (subsys == MM1_SUBSYS) 
        DRV_ClrReg32(AP_SAPD_WAYEN, 1<< 8) ;
    if (subsys == MM2_SUBSYS) 
        DRV_ClrReg32(AP_SAPD_WAYEN, 1<< 9) ;

    if ((subsys == MM1_SUBSYS) && (get_chip_eco_ver() == CHIP_VER_E2))
    {
        DRV_WriteReg32(RM_PWR_STA, 0xffffffff);
        DRV_ClrReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_REQ_EN));

        /* delay*/
        udelay(50);//just close MM1 when both AP and MD close it. so do not polling this status
        DRV_WriteReg32(TOPSM_DMY0,0xFFFFFFFF);
    }    
    else
    {
        /* Assert Power on reset */
        DRV_ClrReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_RST_B));	

        /* Subsys Clock off and ISO en*/
        DRV_SetReg32( (TOPSM_BASE+0x800+subsys*4), (1<<PWR_CLK_DIS)|(1<<PWR_ISO) );	

        /* MMSYS power */
        if (subsys == MM1_SUBSYS)
            DRV_WriteReg32(TOPSM_DMY0,0xFFFFFFFF);
        if (subsys == MM2_SUBSYS)
            DRV_WriteReg32(TOPSM_DMY1,0xFFFFFFFF);
	
        /* Subsys Mem Power off and Subsys Power off	*/
        reg_val = DRV_Reg32(TOPSM_BASE+0x800+subsys*4);
        reg_val = ((reg_val | (1<<PWR_MEM_OFF)) & ~(1<<PWR_ON));
        DRV_WriteReg32((TOPSM_BASE+0x800+subsys*4), reg_val);
	
        /* delay*/
        udelay(50);

	if(subsys == AUDIO_SUBSYS)			
        {
           if(g_clken[MT65XX_AUDIO_CLK_EN].dwClkEnCount == 0) 
           {
              DRV_ClrReg16(PLL_CON2_REG, 0x20);
              udelay(50);
           }
        }
            
            //DRV_ClrReg32(PLL_CON2_REG, 0x20);
    }

    if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsys))
       MSG(SUB, "[%s] subSys[%s] TOPSM_BASE:[0x%x], AP_SAPD_WAYEN[0x%x]\r\n", __FUNCTION__,MT65XX_SUBSYS_name[subsys], DRV_Reg32(TOPSM_BASE+0x800+subsys*4), DRV_Reg32(AP_SAPD_WAYEN));	 
}

BOOL DRV_SetRegWithMask(UINT32 pDest, DWORD mask, UINT32 value)
{
    UINT8 bit_pos = 0;
    while(!((mask >> bit_pos++) & 1));
    value = value<<(bit_pos-1);
    if((DRV_Reg16(pDest) & mask)!= value)
    {
        MASKREG16(pDest, mask, value);
        udelay(50);
        return TRUE;
    }
    return FALSE;
}

void hwSwitchPLL(DWORD clock_type, DWORD pll_mode)
{
    g_MT6573_BusHW.dwClock_active_pll[clock_type] = MT65XX_CLOCK_TYPE_VALUE[clock_type][pll_mode];
    if(clock_type == FMIPI_CK || clock_type == FSYS_CK)
    {
       pll_mode = (pll_mode+1)%2; //0 for MIPI, 1 for 26MHz
    }
    if(pll_mode == 3)
       pll_mode = 4;

    if(DRV_SetRegWithMask(MT65XX_PLLSEL_REG[clock_type][0], MT65XX_PLLSEL_REG[clock_type][1], pll_mode))
    {    		
        MSG(PLL,"[%s] after set, clock_type[%s] pll_mode:[0x%x], PLL_CON[0x%x=0x%x]\r\n", __FUNCTION__, MT65XX_CLOCK_TYPE_name[clock_type], pll_mode, MT65XX_PLLSEL_REG[clock_type][0], DRV_Reg16(MT65XX_PLLSEL_REG[clock_type][0]));	 
    }
}

bool hwEnablePLL(MT65XX_PLL PllId, char *mode_name)
{
    UINT32 i;
    if ((PllId >= MT65XX_PLL_COUNT_END) || (PllId < MT65XX_APMCU_PLL))
    {
        MSG(PLL,"[%s]: bad PllId[%d].\r\n", __FUNCTION__, PllId);
        return FALSE;
    }
    if(!isPllCanDisable(PllId))
    {
        //MSG(PLL, "[%s]: pll[%d,%s] need not be enabled\r\n", __FUNCTION__, PllId, MT65XX_PLL_name[PllId]);
        return TRUE;
    }
    spin_lock(&MT65XX_csPLL_lock);    
    for (i = 0; i< MAX_DEVICE; i++)
    {
        if (!strcmp(g_MT6573_BusHW.Pll[PllId].mod_name[i], NON_OP))
        {
            //MSG(PLL,"[%s] acquire PLLId:%d index:%d mod_name: %s\r\n", __FUNCTION__,PllId, i, mode_name);            
            sprintf(g_MT6573_BusHW.Pll[PllId].mod_name[i] , "%s", mode_name);
            break ;
        }
        /* Has already register it */
        #if 0
        else if (!strcmp(g_MT6573_BusHW.Pll[PllId].mod_name[i], mode_name))
        {
            MSG(PLL,"[%s]: PLL[%d] already register just return\r\n",__FUNCTION__, PllId );
        }
        #endif
    }
    /* Only new request, we add counter */
    g_MT6573_BusHW.Pll[PllId].dwPllCount++;
    /* Other guy has enable it before, just leave */
    if(g_MT6573_BusHW.Pll[PllId].dwPllCount > 1)
    {
        spin_unlock(&MT65XX_csPLL_lock);
        //MSG(PLL, "[%s]: pll[%d, %s] has been enabled\r\n", __FUNCTION__, PllId, MT65XX_PLL_name[PllId]);
        return TRUE;
    }
    if(!PLL_Status(PllId))
    {    
        PLL_Set(PllId);
        if(PllId == MT65XX_MIPI_PLL)
            udelay(50);
        else
            while (!(DRV_Reg32(MT65XX_PLL_LOCK_REG[PllId]) & MT65XX_PLL_LOCK[PllId]));
        MSG(PLL, "[%s]: master[%s] enable pll[%d, %s] CON[regx%x=0x%x]\r\n", __FUNCTION__, mode_name, PllId, MT65XX_PLL_name[PllId],MT65XX_PLL_REG[PllId],DRV_Reg16(MT65XX_PLL_REG[PllId]));
    }    
    spin_unlock(&MT65XX_csPLL_lock);
    return TRUE;
} 

bool hwDisablePLL(MT65XX_PLL PllId, char *mode_name)
{
    UINT32 i = 0;    

    if(!finish_log_init)
    {
        MSG(CG, "[%s]: master[%s] please disable pll[%s] after clock manager initiation\r\n",__FUNCTION__, mode_name, MT65XX_PLL_name[PllId] );
        return FALSE;
    }
    if ((PllId >= MT65XX_PLL_COUNT_END) || (PllId < MT65XX_APMCU_PLL))
    {
        MSG(PLL,"[%s] bad PLLId[%d]\r\n", __FUNCTION__, PllId);
        return FALSE;
    }     
    if(!isPllCanDisable(PllId))
    {
        //MSG(PLL, "[%s]: pll[%d,%s] can not be disabled\r\n", __FUNCTION__, PllId, MT65XX_PLL_name[PllId]);
        return TRUE;
    }
    spin_lock(&MT65XX_csPLL_lock);
    if(g_MT6573_BusHW.Pll[PllId].dwPllCount == 0)
    {        
        //MSG(PLL,"[%s]: Pll[%d, %s] has been disabled\r\n",__FUNCTION__, PllId, MT65XX_PLL_name[PllId]);            
        spin_unlock(&MT65XX_csPLL_lock);
        return TRUE;
    }
    for (i = 0; i< MAX_DEVICE; i++)
    {
        /*Got it, we release it*/
        if (!strcmp(g_MT6573_BusHW.Pll[PllId].mod_name[i], mode_name))
        {
            //MSG(PLL,"[%s] pll[%d, %s] index:%d mod_name: %s\r\n", __FUNCTION__,PllId,MT65XX_PLL_name[PllId], i, mode_name);            
            sprintf(g_MT6573_BusHW.Pll[PllId].mod_name[i] , "%s", NON_OP);
            break ;
        }
    }
    g_MT6573_BusHW.Pll[PllId].dwPllCount--;
    if(g_MT6573_BusHW.Pll[PllId].dwPllCount > 0)
    {        
        //MSG(PLL,"[%s]: Someone still use Pll[%d, %s]\r\n",__FUNCTION__, PllId, MT65XX_PLL_name[PllId]);            
        spin_unlock(&MT65XX_csPLL_lock);
        return TRUE;
    }
    /* FIXME : For MT6573 E1, PLL cannot be disable */
    if(PLL_Status(PllId))
    {
       PLL_Clear(PllId);
       //if(PllId == MT65XX_MIPI_PLL)
            udelay(50);
      // else
            //while (!(DRV_Reg32(MT65XX_PLL_LOCK_REG[PllId]) & MT65XX_PLL_LOCK[PllId]));
       MSG(PLL, "[%s]: master[%s] disable pll[%d, %s] CON[regx%x=0x%x]\r\n", __FUNCTION__, mode_name, PllId, MT65XX_PLL_name[PllId],MT65XX_PLL_REG[PllId],DRV_Reg16(MT65XX_PLL_REG[PllId]));
    }
    spin_unlock(&MT65XX_csPLL_lock);
    return TRUE;
}
BOOL isPllCanDisable(DWORD pllId)
{
   if(0x103f&(1<<pllId))
       return FALSE;
   return TRUE;
}
BOOL isSubsysCanDisable(DWORD sysId)
{
   if(0xc7&(1<<sysId))
       return FALSE;
       
   return TRUE;
}

bool hwEnableSubsys(MT65XX_SUBSYS subsysId)
{
    if(!finish_log_init)
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s]: please enable subsystem[%s] after clock manager initiation",__FUNCTION__, MT65XX_SUBSYS_name[subsysId] );
        return FALSE;
    }
    if ((subsysId >= MT65XX_SUBSYS_COUNT_END) || (subsysId < MD2G_SUBSYS))
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s]: bad subSysId[%d]\r\n", __FUNCTION__, subsysId);
        return FALSE;
    }
    if(!isSubsysCanDisable(subsysId))
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s]: subSystem[%d,%s] need not be enabled\r\n", __FUNCTION__, subsysId, MT65XX_SUBSYS_name[subsysId]);
        return TRUE;
    }
    /* We've already enable it */
    if (g_MT6573_BusHW.dwSubSystem_status & (1<<subsysId))
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s]:subSystem[%s] Already on\r\n",__FUNCTION__, MT65XX_SUBSYS_name[subsysId]);
        return TRUE; 
    }
    spin_lock(&MT65XX_csSYS_lock);
    /* Start HW setting here*/
    MTCMOS_En(subsysId);
    /* End HW setting */
    g_MT6573_BusHW.dwSubSystem_status |= (1<<subsysId);	
    //MSG(SUB,"[%s]:subSystem[%s]is enabled dwSubSystem_status[0x%x]\r\n",__FUNCTION__, MT65XX_SUBSYS_name[subsysId], g_MT6573_BusHW.dwSubSystem_status); 
    spin_unlock(&MT65XX_csSYS_lock);
    return TRUE;
} 

bool hwDisableSubsys(MT65XX_SUBSYS subsysId)
{
    INT32 category = -1;
    if(!finish_log_init)
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s]: please disable subsystem[%s] after clock manager initiation\r\n",__FUNCTION__, MT65XX_SUBSYS_name[subsysId] );
        return FALSE;
    }
    if ((subsysId >= MT65XX_SUBSYS_COUNT_END) || (subsysId < MD2G_SUBSYS))
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
   	       MSG(SUB, "[%s]:subSysId[%d] is wrong\r\n", __FUNCTION__, subsysId);
        return FALSE;
    }
    if(!isSubsysCanDisable(subsysId))
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s]: subSystem[%d,%s] can not be disabled\r\n", __FUNCTION__, subsysId, MT65XX_SUBSYS_name[subsysId]);
        return TRUE;
    }
    /* We've already disable it */
    if (!(g_MT6573_BusHW.dwSubSystem_status & (1<<subsysId)))
    {
        if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
    	   MSG(SUB, "[%s] subSystem[%s] Already off\r\n",__FUNCTION__,MT65XX_SUBSYS_name[subsysId]);
        return TRUE;
    }
    category = MT65XX__CATEGORY_OF_SUBSYS[subsysId];
    if(category >= 0)
    {
        if(subsysId == MM1_SUBSYS)
        {
           return TRUE; //for WDT HW reset CR:66254
           if((g_MT6573_BusHW.dwSubSystem_clock[category]!=0) || (g_MT6573_BusHW.dwSubSystem_clock[category+1]!=0))
           {
               if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
                  MSG(SUB,"[%s]: subSystem[%s] is still used! clock_status[0x%x, 0x%x]\r\n",__FUNCTION__,MT65XX_SUBSYS_name[subsysId], g_MT6573_BusHW.dwSubSystem_clock[category], g_MT6573_BusHW.dwSubSystem_clock[category+1]);
               return FALSE;
          }
        }
        else
        {
            if(g_MT6573_BusHW.dwSubSystem_clock[category] != 0)
            {
                if(g_MT6573_BusHW.dwSubSystem_showlog & (1<<subsysId))
                   MSG(SUB, "[%s]: subSystem[%s] is still used! clock_status[0x%x]\r\n", __FUNCTION__,MT65XX_SUBSYS_name[subsysId], g_MT6573_BusHW.dwSubSystem_clock[category]);
                return FALSE;
            }
        }
    }
    spin_lock(&MT65XX_csSYS_lock);
    /* Start HW setting here*/
    MTCMOS_Dis(subsysId);
    /* End HW setting */
    g_MT6573_BusHW.dwSubSystem_status &= ~(1<<subsysId);    
    spin_unlock(&MT65XX_csSYS_lock);
    return TRUE;    
}

void hwSetClkEn(DWORD clk_en_type, BOOL enable, char* master)
{
    INT32 i = 0;
    if(enable)
    {
         //record master name
         for (i = 0; i< MAX_DEVICE; i++)
         {
             if (!strcmp(g_clken[clk_en_type].mod_name[i], NON_OP))
             {
                sprintf(g_clken[clk_en_type].mod_name[i] , "%s", master);
                break ;
             }
         }
         //do enable XX_CLK_EN when reference count is 0
         if(g_clken[clk_en_type].dwClkEnCount == 0) 
         {
             DRV_SetReg16(g_clken[clk_en_type].reg, g_clken[clk_en_type].mask);
             udelay(50);
         }
         //increase reference count
         g_clken[clk_en_type].dwClkEnCount++;
    }  
    else
    {
          if(g_clken[clk_en_type].dwClkEnCount == 0)
          {
              if(g_clken[clk_en_type].showlog)
                 MSG(CG, "[%s]: master[%s] need not to disable %s, it has been disabled\r\n", __FUNCTION__, master, g_clken[clk_en_type].name);
              return;
          }
          //remove master name
          for (i = 0; i< MAX_DEVICE; i++)
          {
             if (!strcmp(g_clken[clk_en_type].mod_name[i], master))
             {
                  sprintf(g_clken[clk_en_type].mod_name[i] , "%s", NON_OP);
                  break ;
             }
         }
         //decreas reference count
         g_clken[clk_en_type].dwClkEnCount--;
         //do disable XX_CLK_EN when reference count is 0
         if(g_clken[clk_en_type].dwClkEnCount == 0) 
         {
             DRV_ClrReg16(g_clken[clk_en_type].reg, g_clken[clk_en_type].mask);
             udelay(50);
         }
    }  
    if(g_clken[clk_en_type].showlog)
       MSG(CG, "[%s]: master:%s, enable:%d, type:%s, reg[0x%x=0x%x], g_clken_count[%d]\r\n", __FUNCTION__, master, enable, g_clken[clk_en_type].name, g_clken[clk_en_type].reg, DRV_Reg16(g_clken[clk_en_type].reg), g_clken[clk_en_type].dwClkEnCount);
}
//interface for driver without clock enabled/disabled
BOOL hwSetPllCon_master(DWORD clk_en_type, BOOL enable, char* master)
{
    if(clk_en_type < 0 || clk_en_type >= MT65XX_CLK_EN_COUNT_END)
    {
         MSG(PLL, "[%s]: bad clk_en_type:%d \r\n", __FUNCTION__, clk_en_type);
         return FALSE;
    }
    hwSetClkEn(clk_en_type, enable, master);
    return TRUE;
}
//interface for clock manager when driver enable/disable a clock
void hwSetPllCon_clock(INT32 category, INT32 offset, BOOL enable)
{
    INT32 clk_en_type = -1;
    DWORD mode = 1 << offset;
    char p[MAX_MOD_NAME];

    if(!(MT65XX_IS_CLK_EN[category] & mode))
    {
	return;
    }

    if(category == CLOCK_PERI_PDN0)
    {
        if(mode & 0xf)
            clk_en_type = MT65XX_MSDC_CLK_EN;
        else if(mode & 0x0180)
            clk_en_type = MT65XX_USB_CLK_EN;
    }else if(category == CLOCK_AUDIOSYS)
    {
        if(mode & 0x74)
            clk_en_type = MT65XX_AUDIO_CLK_EN;
    }
    sprintf(p, "%s_CK", g_MT6573_BusHW.Clock[category][offset].name);
    hwSetClkEn(clk_en_type, enable, p);
}

BOOL valid_clockId(int clockId)
{
    INT32 category = 0, offset;
    category = clockId / 32;
    offset = clockId %32;

    if (category >= MT65XX_CLOCK_CATEGORY_COUNT|| category < 0 || offset >= MT65XX_CLOCK_NUMBER[category] || offset < 0 || clockId == 160 || clockId == 161 || clockId == 163)
   {
	   MSG(DCM,"[%s]: ClockId[%d] is wrong\r\n", __FUNCTION__, clockId);
	   return FALSE;
   }  
	
   return TRUE;
}

bool hwEnableClock(MT65XX_CLOCK clockId, char *mode_name)
{
    INT32 category = 0, offset;

    if(!finish_log_init)
    {
    	MSG(CG, "[%s]: master[%s] please enable clock[%d] after clock manager initiation",__FUNCTION__, mode_name, clockId);
        return FALSE;
    }
    category = clockId / 32;
    offset = clockId %32;
     
    if (category >= MT65XX_CLOCK_CATEGORY_COUNT|| category < 0 || offset >= MT65XX_CLOCK_NUMBER[category] || offset < 0 || clockId == 160 || clockId == 161 || clockId == 163)
    {
        MSG(CG, "[%s]: ClockId[%d] is wrong\r\n", __FUNCTION__, clockId);
        return FALSE;
    }
    if((clockId == MT65XX_PDN_PERI_SIMIF0) || (clockId == MT65XX_PDN_PERI_SIMIF1))	
    {
    	MSG(CG, "[%s]: clock[%d, %s] can not be enabled by clock manager\r\n",__FUNCTION__, clockId,g_MT6573_BusHW.Clock[category][offset].name);
        return TRUE;
    }
    if(!isPllCanBeSwitched(g_MT6573_BusHW.Clock[category][offset].clock_type, MT65XX_CLOCK_TYPE_VALUE[g_MT6573_BusHW.Clock[category][offset].clock_type][1]))
    {
       return FALSE;
    }	
    return hwEnableClock_func(category, offset, mode_name, 1);
}

bool hwEnableClock_withPll(MT65XX_CLOCK clockId, char *mode_name, DWORD pllId)
{
    INT32 category = 0, offset, pll_mode;

    if(!finish_log_init)
    {
    	MSG(CG, "[%s]: master[%s] please enable clock[%d] after clock manager initiation",__FUNCTION__, mode_name, clockId);
        return FALSE;
    }
    category = clockId / 32;
    offset = clockId %32;
     
    if (category >= MT65XX_CLOCK_CATEGORY_COUNT|| category < 0 || offset >= MT65XX_CLOCK_NUMBER[category] || offset < 0 || clockId == 160 || clockId == 161 || clockId == 163)
    {
        MSG(CG, "[%s]: ClockId[%d] is wrong\r\n", __FUNCTION__, clockId);
        return FALSE;
    }
    if((clockId == MT65XX_PDN_PERI_SIMIF0) || (clockId == MT65XX_PDN_PERI_SIMIF1))	
    {
        MSG(CG, "[%s]: clock[%d, %s] can not be enabled by clock manager\r\n",__FUNCTION__, clockId,g_MT6573_BusHW.Clock[category][offset].name);
        return TRUE;
    }
    pll_mode = isActivePllSource(pllId, category, offset);
    if(pll_mode < 0)
    {
    	MSG(CG, "[%s]: pllId[%d] is wrong\r\n", __FUNCTION__, pllId);
        return FALSE;
    } 
    if(!isPllCanBeSwitched(g_MT6573_BusHW.Clock[category][offset].clock_type, pllId))
    {
        return FALSE;
    }
    return hwEnableClock_func(category, offset, mode_name, pll_mode);
}
bool hwEnableClock_func(INT32 category, INT32 offset, char *mode_name, DWORD pll_mode)
{
    INT32 i=0, subSysId=-1;//add by Cui Zhang 
    DWORD mode = 1 << offset;		

    spin_lock(&MT65XX_csClock_lock);
    g_MT6573_BusHW.Clock[category][offset].dwClockCount++;
    for (i = 0; i< MAX_DEVICE; i++)
    {
        if (!strcmp(g_MT6573_BusHW.Clock[category][offset].mod_name[i], NON_OP))
        {
            sprintf(g_MT6573_BusHW.Clock[category][offset].mod_name[i] , "%s", mode_name);
            break ;
        }
    }
    if(i == MAX_DEVICE)
    {
	 if(g_MT6573_BusHW.Clock[category][offset].showlog)
            MSG(CG, "[%s]: master[%s] enable clock[%d, 0x%x, %s],with no more space of enable record, count[%d]\r\n", __FUNCTION__, mode_name, category, mode,  g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.Clock[category][offset].dwClockCount);
    }
    /*Begin HW setting*/
     if(g_MT6573_BusHW.Clock[category][offset].dwClockCount==1)
     {	        
         //update dwSubSystem_clock
         g_MT6573_BusHW.dwSubSystem_clock[category]|= mode; 

         hwEnablePLL(MT65XX_CLOCK_TYPE_VALUE[g_MT6573_BusHW.Clock[category][offset].clock_type][pll_mode], mode_name);		 
         hwSwitchPLL(g_MT6573_BusHW.Clock[category][offset].clock_type, pll_mode);

         //cancel this since 3G PLL can not be disabled
         /*if((g_MT6573_BusHW.Clock[category][offset].clock_type != FWMA_CK) && (g_MT6573_BusHW.Clock[category][offset].clock_type != FEMI_CK))
         {
            hwEnablePLL(MT65XX_CLOCK_TYPE_VALUE[FWMA_CK][0], mode_name);
         }*/
         g_MT6573_BusHW.Clock[category][offset].active_pll_source = MT65XX_CLOCK_TYPE_VALUE[g_MT6573_BusHW.Clock[category][offset].clock_type][pll_mode];

         //enable subsystem        
         subSysId = MT65XX_SUBSYS_OF_CATEGORY[category]; 
         if (!(g_MT6573_BusHW.dwSubSystem_status & (1<<subSysId)))
         {
            if(subSysId ==MM1_SUBSYS ||subSysId ==MM2_SUBSYS || subSysId ==AUDIO_SUBSYS)
            {
                hwEnableSubsys(subSysId);
            }
         }
     }else if(g_MT6573_BusHW.Clock[category][offset].dwClockCount > 1)
     {
         if(g_MT6573_BusHW.Clock[category][offset].showlog)
             MSG(CG,"[%s]: master[%s],clock[%s] has been enabled! pll_source[%s], count[%d], clock_status[0x%x]\r\n",__FUNCTION__, mode_name, g_MT6573_BusHW.Clock[category][offset].name, MT65XX_PLL_name[g_MT6573_BusHW.Clock[category][offset].active_pll_source], g_MT6573_BusHW.Clock[category][offset].dwClockCount, g_MT6573_BusHW.dwSubSystem_clock[category] ); 
         spin_unlock(&MT65XX_csClock_lock);    
         return TRUE;
     }
     //clear clock gating
     hwSetPllCon_clock(category, offset, TRUE);
     if(CG_Status(category,mode))
        CG_Clear(category, mode);
     if(g_MT6573_BusHW.Clock[category][offset].showlog)
        MSG(CG, "[%s]: master[%s] enable clock[%d,0x%x,%s],count[%d], clock_status[0x%x], CON[reg:0x%x=0x%x]!!\r\n",__FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.Clock[category][offset].dwClockCount,g_MT6573_BusHW.dwSubSystem_clock[category], MT65XX_CG_CON[category],DRV_Reg32(MT65XX_CG_CON[category]));
     spin_unlock(&MT65XX_csClock_lock);    
     return TRUE;
}

bool hwDisableClock(MT65XX_CLOCK clockId, char *mode_name)
{
    INT32 category = 0, offset;

    if(!finish_log_init)
    {
      MSG(CG, "[%s]: master[%s] please disable clock[%d] after clock manager initiation\r\n",__FUNCTION__, mode_name, clockId);
      return FALSE;
    }
    category = clockId / 32;
    offset = clockId %32;
    if (category >= MT65XX_CLOCK_CATEGORY_COUNT|| category < 0 || offset >= MT65XX_CLOCK_NUMBER[category] || offset < 0 || clockId == 160 || clockId == 161 || clockId == 163)
    {
        MSG(CG, "[%s]: bad clockId[%d]\r\n",__FUNCTION__, clockId);
        return FALSE;
    }
    if((clockId == MT65XX_PDN_PERI_SIMIF0) || (clockId == MT65XX_PDN_PERI_SIMIF1)|| (clockId == MT65XX_PDN_PERI_APDMA))	
    {
        MSG(CG, "[%s]: clock[%d, %s] can not be disabled by clock manager\r\n",__FUNCTION__, clockId,g_MT6573_BusHW.Clock[category][offset].name);
        return TRUE;
    }
    return hwDisableClock_func(category, offset, mode_name);
}

bool hwDisableClock_func(INT32 category, INT32 offset, char *mode_name)
{
    UINT32 i = 0, subSysId=0;//add by Cui Zhang
    DWORD mode = 0;
    BOOL allow_gate = TRUE;

    mode = 1 << offset;
     
    spin_lock(&MT65XX_csClock_lock);    
    if(g_MT6573_BusHW.Clock[category][offset].dwClockCount == 0)
    {
        spin_unlock(&MT65XX_csClock_lock);
        if(g_MT6573_BusHW.Clock[category][offset].showlog)
           MSG(CG, "[%s]: master[%s], clock[%d,0x%x,%s] has been disabled! clock_status[0x%x]\r\n",__FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.dwSubSystem_clock[category]);
        return TRUE;
    }
    for (i = 0; i< MAX_DEVICE; i++)
    {
        if (!strcmp(g_MT6573_BusHW.Clock[category][offset].mod_name[i], mode_name))
        {
            sprintf(g_MT6573_BusHW.Clock[category][offset].mod_name[i] , "%s", NON_OP);
            break ;
        }
    }
    if(i == MAX_DEVICE)
    {
        if(g_MT6573_BusHW.Clock[category][offset].showlog)
            MSG(CG,"[%s]: master[%s] disable clock[%d, 0x%x, %s], but there is no enable record, count[%d]\r\n", __FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.Clock[category][offset].dwClockCount);
    }

    g_MT6573_BusHW.Clock[category][offset].dwClockCount --;
    if(g_MT6573_BusHW.Clock[category][offset].dwClockCount > 0)
    {
        if(g_MT6573_BusHW.Clock[category][offset].showlog)
           MSG(CG, "[%s]: master[%s], clock[%d,0x%x,%s] is still used, count[%d], clock_status[0x%x]\r\n", __FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.Clock[category][offset].dwClockCount,g_MT6573_BusHW.dwSubSystem_clock[category] );
        spin_unlock(&MT65XX_csClock_lock);        
        return TRUE;
    }
    //update dwSubSystem_clock
    g_MT6573_BusHW.dwSubSystem_clock[category] &= ~mode; 
	
    subSysId = MT65XX_SUBSYS_OF_CATEGORY[category];
    
    //do not actually disable GMC1, GMC1E, GMC1SLV if the other clocks in MM1 are still active
    if((category==CLOCK_MMSYS1_PART0 && offset==0) || (category==CLOCK_MMSYS1_PART1 && (offset==8 || offset==9)))
    {
        allow_gate = FALSE;
        if((g_MT6573_BusHW.dwSubSystem_clock[CLOCK_MMSYS1_PART0]&0xfffffffe) || (g_MT6573_BusHW.dwSubSystem_clock[CLOCK_MMSYS1_PART1]&0xff))
        {    
            MSG(CG,"[%s]:master[%s] disable  clock[%d,0x%x,%s], but cannot gate it here,count[%d], clock_status[0x%x], CON[reg:0x%x=0x%x] \r\n",__FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name,g_MT6573_BusHW.Clock[category][offset].dwClockCount, g_MT6573_BusHW.dwSubSystem_clock[category], MT65XX_CG_CON[category],DRV_Reg32(MT65XX_CG_CON[category]));       
            spin_unlock(&MT65XX_csClock_lock);
            return TRUE;
        }
    }
    //do not actually disable GMC2, GMC2_E if the other clocks in MM2 are still active
    else if(category==CLOCK_MMSYS2_PART0 && (offset==0 || offset==6))
    {
        allow_gate = FALSE;
        if(g_MT6573_BusHW.dwSubSystem_clock[category]&0x3e)
        {
           MSG(CG,"[%s]:master[%s] disable  clock[%d,0x%x,%s], but cannot gate it here,count[%d], clock_status[0x%x], CON[reg:0x%x=0x%x] \r\n",__FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name,g_MT6573_BusHW.Clock[category][offset].dwClockCount, g_MT6573_BusHW.dwSubSystem_clock[category], MT65XX_CG_CON[category],DRV_Reg32(MT65XX_CG_CON[category]));  
            spin_unlock(&MT65XX_csClock_lock);
            return TRUE;
        }
    }

    //do not actually disable AFE if the other clocks in AUDIO are still active
    else if(category==CLOCK_AUDIOSYS && offset==2)
    {
        allow_gate = FALSE;
        if(g_MT6573_BusHW.dwSubSystem_clock[category]&0x70)
        {
           MSG(CG,"[%s]:master[%s] disable  clock[%d,0x%x,%s], but cannot gate it here,count[%d], clock_status[0x%x], CON[reg:0x%x=0x%x] \r\n",__FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name,g_MT6573_BusHW.Clock[category][offset].dwClockCount, g_MT6573_BusHW.dwSubSystem_clock[category], MT65XX_CG_CON[category],DRV_Reg32(MT65XX_CG_CON[category]));  
           hwSetPllCon_clock(category, offset, FALSE);
           spin_unlock(&MT65XX_csClock_lock);
           return TRUE;
        }
    }
   
    if(allow_gate)
    {
        if(get_chip_eco_ver() == CHIP_VER_E2)
        {
           if((category==3) && (offset==7))//MT65XX_PDN_MM_HIF
           {
           }
           else 
           {
              if(!CG_Status(category, mode))
                 CG_Set(category, mode);
           }

       }else
       {
           if(!CG_Status(category, mode))
               CG_Set(category, mode);
       }
    }
    if(g_MT6573_BusHW.Clock[category][offset].showlog)
        MSG(CG, "[%s]:master[%s] disable  clock[%d,0x%x,%s] clock_status[0x%x], CON[reg:0x%x=0x%x], count[%d]\r\n",__FUNCTION__, mode_name, category, mode, g_MT6573_BusHW.Clock[category][offset].name, g_MT6573_BusHW.dwSubSystem_clock[category], MT65XX_CG_CON[category],DRV_Reg32(MT65XX_CG_CON[category]), g_MT6573_BusHW.Clock[category][offset].dwClockCount);
    hwSetPllCon_clock(category, offset, FALSE);
    //disable subsystem
    if(g_MT6573_BusHW.dwSubSystem_clock[category] == 0)
    {
       hwDisableSubsys(subSysId);
    }
    //disable pll		
    hwDisablePLL(g_MT6573_BusHW.Clock[category][offset].active_pll_source, mode_name);	

    if((g_MT6573_BusHW.Clock[category][offset].active_pll_source != FWMA_CK) && (g_MT6573_BusHW.Clock[category][offset].active_pll_source != FEMI_CK))
    {
        hwDisablePLL(MT65XX_CLOCK_TYPE_VALUE[FWMA_CK][0], mode_name);
    }
    g_MT6573_BusHW.Clock[category][offset].active_pll_source = -1;
	
    spin_unlock(&MT65XX_csClock_lock);
    return TRUE;
}


EXPORT_SYMBOL(hwEnableSubsys);
EXPORT_SYMBOL(hwDisableSubsys);
EXPORT_SYMBOL(hwEnableClock);
EXPORT_SYMBOL(hwDisableClock);
EXPORT_SYMBOL(hwPowerDown);
EXPORT_SYMBOL(hwPowerOn);
EXPORT_SYMBOL(hwEnablePLL);
EXPORT_SYMBOL(hwDisablePLL);
EXPORT_SYMBOL(hwSetClockType);
EXPORT_SYMBOL(PLL_Fsel);
EXPORT_SYMBOL(hwSetPllCon_master);
EXPORT_SYMBOL(register_clock_listener);
EXPORT_SYMBOL(unregister_clock_listener);
EXPORT_SYMBOL(MTCMOS_En_reboot);

