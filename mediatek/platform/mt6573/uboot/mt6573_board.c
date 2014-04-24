

#include <common.h>
#include <asm/arch/mt65xx_disp_drv.h>
#include <asm/arch/mt65xx_logo.h>
#include <asm/arch/mt65xx_uart.h>
#include <asm/arch/mt65xx_serial.h>
#include <asm/arch/mt6573_gpio.h>
#include <asm/arch/boot_mode.h>
#include <video.h>

#if 0
#include <asm/arch/mt6516.h>
#include <asm/arch/mt6516_utils.h>
#include <asm/arch/mt6516_typedefs.h>
#include <asm/arch/mt6516_pmu_hw.h>
#include <asm/arch/mt6516_pmu_sw.h>
#include <asm/arch/mt6516_serial.h>
#include <asm/arch/mt6516_gpio.h>
#include <asm/arch/mt6516_uart.h>

#include <asm/arch/mt6516_bat.h>
#include <asm/arch/mt6516_i2c.h>
#include <asm/arch/mt6516_wdt_hw.h>
#include <asm/arch/mt6516_auxadc_hw.h>
#include <asm/arch/mt6516_charger.h>
#include <asm/arch/mt6516_bat.h>
#endif
#include <asm/io.h>
#include <asm/mach-types.h>
#include "mt65xx_partition.h"
#include <asm/arch/mt65xx_leds.h>

#if 0
#include <asm/arch/mt6516_pmic6326_hw.h>
#include <asm/arch/mt6516_pmic6326_sw.h>
#endif

#ifdef CFG_META_MODE
#include <asm/arch/mt65xx_meta.h>
#endif

// ===========================================
// REGISTER DEFINITIONS
// ===========================================

#if 0
#define TVENC                   		(0x80089008)
#define MIPI_PD_B00             		(0x80060b00)
#define MIPI_PD_B04             		(0x80060b04)
#define MIPI_PD_B08             		(0x80060b08)
#define MIPI_PD_B0C             		(0x80060b0c)
#define MIPI_PD_B10             		(0x80060b10)
#define MIPI_PD_B14             		(0x80060b14)
#define MIPI_PD_B18             		(0x80060b18)
#define MIPI_PD_B1C             		(0x80060b1c)
#define MIPI_PD_B40             		(0x80060b40)
#define MIPI_PD_B44             		(0x80060b44)
#define MIPI_PD_B48             		(0x80060b48)
#define MIPI_PD_B4C             		(0x80060b4c)
#define MIPI_PD_04C             		(0x8006004c)

#define PDN_AFE_AAPDN           		(0x80060208)
#define PDN_AFE_AAC_NEW         		(0x8006020C)
#define PDN_AFE_AAC_CON1        		(0x80060210)

#define PDN_CON   	            		 (PLL_BASE+0x0010)
#define CLK_CON 	                     (PLL_BASE+0x0014)
#define MPLL 	                         (PLL_BASE+0x0020)
#define MPLL2  	                         (PLL_BASE+0x0024)
#define UPLL                             (PLL_BASE+0x0030)
#define UPLL2                            (PLL_BASE+0x0034)
#define CPLL                             (PLL_BASE+0x0038)
#define CPLL2                            (PLL_BASE+0x003C)
#define TPLL                             (PLL_BASE+0x0040)
#define TPLL2                            (PLL_BASE+0x0044)
#define CPLL3                            (PLL_BASE+0x0048)
#define PLL_RES_CON0                     (PLL_BASE+0x004c)
#define PLL_BIAS                         (PLL_BASE+0x0050)
#define MCPLL                            (PLL_BASE+0x0058)
#define MCPLL2                           (PLL_BASE+0x005C)
#define CEVAPLL                          (PLL_BASE+0x0060)
#define CEVAPLL2                         (PLL_BASE+0x0064)
#define PLL_IDN                          (PLL_BASE+0x0070)
#define XOSC32_AC_CON                    (PLL_BASE+0x007C)
#define MIPITX_CON                       (PLL_BASE+0x0b00)


#define HW_VER              			(CONFIG_BASE+0x0000)
#define SW_VER              			(CONFIG_BASE+0x0004)
#define HW_CODE             			(CONFIG_BASE+0x0008) 
#define SW_MISC_L						(CONFIG_BASE+0x0010)
#define SW_MISC_H           			(CONFIG_BASE+0x0014)
#define HW_MISC             			(CONFIG_BASE+0x0020)
#define ARM9_FREQ_DIV       			(CONFIG_BASE+0x0100)
#define SLEEP_CON           			(CONFIG_BASE+0x0204)
#define MCUCLK_CON          			(CONFIG_BASE+0x0208)
#define EMICLK_CON          			(CONFIG_BASE+0x020c)
#define ISO_EN              			(CONFIG_BASE+0x0300)
#define PWR_OFF             			(CONFIG_BASE+0x0304)
#define MCU_MEM_PDN         			(CONFIG_BASE+0x0308)
#define G1_MEM_PDN          			(CONFIG_BASE+0x030c)
#define G2_MEM_PDN          			(CONFIG_BASE+0x0310)
#define CEVA_MEM_PDN        			(CONFIG_BASE+0x0314)
#define IN_ISO_EN           			(CONFIG_BASE+0x0318)
#define PWR_ACK             			(CONFIG_BASE+0x031c)
#define ACK_CLR             			(CONFIG_BASE+0x0320)
#define APB_CON             			(CONFIG_BASE+0x0404)
#define SECURITY_REG        			(CONFIG_BASE+0x0408)
#define IO_DRV0             			(CONFIG_BASE+0x0500)
#define IO_DRV1             			(CONFIG_BASE+0x0504)
#define IC_SIZE             			(CONFIG_BASE+0x0600)
#define DC_SIZE             			(CONFIG_BASE+0x0604)
#define MDVCXO_OFF          			(CONFIG_BASE+0x0608)



#define GRAPH1SYS_CG_CON        		(GRAPH1SYS_CONFG_BASE + 0x300)
#define GRAPH1SYS_CG_SET        		(GRAPH1SYS_CONFG_BASE + 0x320)
#define GRAPH1SYS_CG_CLR        		(GRAPH1SYS_CONFG_BASE + 0x340)

#define GRAPH2SYS_CG_CON        		(GRAPH2SYS_BASE + 0x0)
#define GRAPH2SYS_CG_SET        		(GRAPH2SYS_BASE + 0x4)
#define GRAPH2SYS_CG_CLR        		(GRAPH2SYS_BASE + 0x8)
#define GRAPH2SYS_DESEL0        		(GRAPH2SYS_BASE + 0x10)
#define GRAPH2SYS_DESEL1        		(GRAPH2SYS_BASE + 0x14)
#define GRAPH2SYS_DESEL2        		(GRAPH2SYS_BASE + 0x18)

#define APMCUSYS_PDN_CON0              	(AMCONFG_BASE+0x0300)
#define APMCUSYS_PDN_SET0              	(AMCONFG_BASE+0x0320)
#define APMCUSYS_PDN_CLR0              	(AMCONFG_BASE+0x0340)

#define APMCUSYS_PDN_CON1              	(AMCONFG_BASE+0x0360)
#define APMCUSYS_PDN_SET1              	(AMCONFG_BASE+0x0380)
#define APMCUSYS_PDN_CLR1              	(AMCONFG_BASE+0x03A0)

#define APMCUSYS_DELSEL0              	(AMCONFG_BASE+0x0600)
#define APMCUSYS_DELSEL1              	(AMCONFG_BASE+0x0604)
#define APMCUSYS_DELSEL2              	(AMCONFG_BASE+0x0608)
#define APMCUSYS_DELSEL3              	(AMCONFG_BASE+0x060C)
#endif

typedef enum {
    MANUAL_RESET = 0,
    WDT_RESET,
} SYS_RESET_REASON;

DECLARE_GLOBAL_DATA_PTR;

// ===========================================
// EXTERNAL DEFINITIONS
// ===========================================
extern void dbg_print(char *sz,...);
extern u32 memory_size(void);
extern void mt65xx_pmu_init(void);
extern  int cm3623_init_client();


static SYS_RESET_REASON mt6516_get_reset_reason(void)
{  
#if 0
    u16 val = __raw_readw(RGU_BASE);

    if ((val & MT6516_WDT_STATUS_HWWDT) || (val & MT6516_WDT_STATUS_SWWDT))
        return WDT_RESET;
#endif
    return MANUAL_RESET;
}

#if 0
extern void mt6516_timer_init(void);
extern int mmc_init(int verbose);
extern int mboot_common_load_logo(unsigned long logo_addr,char* filename);

extern void pmic_init(void);
extern void pmic_backlight_on(void);

extern void pmic_config_interface(kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT);
#endif

#ifdef CFG_HAS_USB
extern int	drv_usbtty_init (void);
#endif 

#if 1
void Uboot_power_saving(void)
{
#if 0
    u32 u4Val = 0;
    
#if 0   
	printf("GPIO Set\n");
    /* GPIO48~55, turn-off PWM 0~1 */
    DRV_WriteReg32(0x80002660, 0x0555);
    /* GPIO56~63, turn-off EINT 0~4 */
    DRV_WriteReg32(0x80002670, 0x0015);
    /* GPIO64~71, turn-off EINT 5~7, UART4 CTS/RTS */
    DRV_WriteReg32(0x80002680, 0x1400);
    /* GPIO112~119, turn-off KP Row2~4, CLK_OUT0~4 */
    DRV_WriteReg32(0x800026E0, 0x0000);
    /* GPIO128~135, Keep I2C SCL2 ,turn-off others */
    DRV_WriteReg32(0x80002700, 0x4000);    
    /* GPIO136~143, Keep I2C SDA2 ,turn-off TRACE signal */
    DRV_WriteReg32(0x80002710, 0x0001);    
#endif

    /* TV power down*/
    DRV_ClrReg32(TVENC, 0x13E0);

    /* AFE power down*/
    DRV_WriteReg32(PDN_AFE_AAPDN, 0); 
    DRV_WriteReg32(PDN_AFE_AAC_NEW, 0);
    DRV_WriteReg32(PDN_AFE_AAC_CON1, 0x0003);

    /* MIPI power down 
       Jett: Don't power down MIPI during uboot, or the DPI signal would
             be turned off and the uboot logo is disappeared
    */
#if 0
    PW_DEBUG("MIPI power down\n");
    DRV_WriteReg32(MIPI_PD_B00, 0);
    DRV_WriteReg32(MIPI_PD_B04, 0);
    DRV_WriteReg32(MIPI_PD_B08, 0);
    DRV_WriteReg32(MIPI_PD_B0C, 0);
    DRV_WriteReg32(MIPI_PD_B10, 0);
    DRV_WriteReg32(MIPI_PD_B14, 0);
    DRV_WriteReg32(MIPI_PD_B18, 0);
    DRV_WriteReg32(MIPI_PD_B1C, 0);
    DRV_WriteReg32(MIPI_PD_B40, 0);
    DRV_WriteReg32(MIPI_PD_B44, 0);
    DRV_WriteReg32(MIPI_PD_B48, 0);
    DRV_WriteReg32(MIPI_PD_B4C, 0);
    DRV_WriteReg32(MIPI_PD_04C, 1);
#endif

	/* MCU CG*/
    DRV_SetReg32 (APMCUSYS_PDN_SET0, (1<<8)|(1<<9)|(1<<21));

    /* MCU memory PDN*/
    u4Val = 0;
    u4Val = ((1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|    \
        (1<<11)|(1<<12)|(1<<13)|(1<<16) );
    DRV_SetReg32 (MCU_MEM_PDN, u4Val);


    /* GRAPH1SYS CG*/
    DRV_SetReg32 (GRAPH1SYS_CG_SET, (1<<0));

    /* GRAPH1SYS memory PDN*/
    u4Val = 0;
    u4Val = ((1<<2)|(1<<3)|(1<<4)|(1<<6)|(1<<9));
    DRV_SetReg32 (G1_MEM_PDN, u4Val);

	/* GRAPH2SYS CG */
    DRV_SetReg32 (GRAPH2SYS_CG_SET, 0x1FF);	

    /* GRAPH2SYS  memory PDN*/
    u4Val = 0;
    u4Val = ((1<<0));
    DRV_SetReg32 (G2_MEM_PDN, u4Val);

	/* GRAPH2SYS MTCMOS */
	DRV_SetReg16(ISO_EN, 1<<4);
	DRV_SetReg16(IN_ISO_EN, 1<<4 );    
	DRV_SetReg16(PWR_OFF, 1<<4 );
	DRV_SetReg16(ACK_CLR, 0x2); 
	
    /* CEVA memory PDN*/
    u4Val = 0;
    u4Val = ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5));
    DRV_SetReg32 (CEVA_MEM_PDN, u4Val);

	/* CEVA MTCMOS */	
	DRV_SetReg16(ISO_EN, 1<<5);
	DRV_SetReg16(IN_ISO_EN, 1<<5 );    
	DRV_SetReg16(PWR_OFF, 1<<5 );
	DRV_SetReg16(ACK_CLR, 0x1); 

    /* Stop CEVA PLL*/
    DRV_ClrReg32(CEVAPLL2,1<<0);

    /* Stop UPLL*/
    DRV_ClrReg32(PDN_CON,1<<4);


    /* Stop DPLL*/
#if 0
	printf("DPLL Power Down\n");
    DRV_ClrReg32(PDN_CON,1<<3);
#endif

	/* Stop VUSB */
	//pmic_config_interface(0x3D, KAL_FALSE, VUSB_EN_MASK, VUSB_EN_SHIFT);	 //pmic_vsdio_enable(KAL_TRUE);

#endif
}    

void Uboot_power_saving2(void)
{
#if 0
    u32 u4Val = 0;

#if 0   
	printf("GPIO Set\n");
    /* GPIO48~55, turn-off PWM 0~1 */
    DRV_WriteReg32(0x80002660, 0x0555);
    /* GPIO56~63, turn-off EINT 0~4 */
    DRV_WriteReg32(0x80002670, 0x0015);
    /* GPIO64~71, turn-off EINT 5~7, UART4 CTS/RTS */
    DRV_WriteReg32(0x80002680, 0x1400);
    /* GPIO112~119, turn-off KP Row2~4, CLK_OUT0~4 */
    DRV_WriteReg32(0x800026E0, 0x0000);
    /* GPIO128~135, Keep I2C SCL2 ,turn-off others */
    DRV_WriteReg32(0x80002700, 0x4000);    
    /* GPIO136~143, Keep I2C SDA2 ,turn-off TRACE signal */
    DRV_WriteReg32(0x80002710, 0x0001);    
#endif

    /* TV power down*/
    DRV_ClrReg32(TVENC, 0x13E0);

    /* AFE power down*/
    DRV_WriteReg32(PDN_AFE_AAPDN, 0); 
    DRV_WriteReg32(PDN_AFE_AAC_NEW, 0);
    DRV_WriteReg32(PDN_AFE_AAC_CON1, 0x0003);

    /* MIPI power down*/
    DRV_WriteReg32(MIPI_PD_B00, 0);
    DRV_WriteReg32(MIPI_PD_B04, 0);
    DRV_WriteReg32(MIPI_PD_B08, 0);
    DRV_WriteReg32(MIPI_PD_B0C, 0);
    DRV_WriteReg32(MIPI_PD_B10, 0);
    DRV_WriteReg32(MIPI_PD_B14, 0);
    DRV_WriteReg32(MIPI_PD_B18, 0);
    DRV_WriteReg32(MIPI_PD_B1C, 0);
    DRV_WriteReg32(MIPI_PD_B40, 0);
    DRV_WriteReg32(MIPI_PD_B44, 0);
    DRV_WriteReg32(MIPI_PD_B48, 0);
    DRV_WriteReg32(MIPI_PD_B4C, 0);
    DRV_WriteReg32(MIPI_PD_04C, 1);
#if 1
	/* MCU CG*/
    DRV_SetReg32 (APMCUSYS_PDN_SET0, (1<<8)|(1<<9)|(1<<21));

    /* MCU memory PDN*/
    u4Val = 0;
    u4Val = ((1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|    \
        (1<<11)|(1<<12)|(1<<13)|(1<<16) );
    DRV_SetReg32 (MCU_MEM_PDN, u4Val);
#endif
#if 1
    /* GRAPH1SYS CG*/
    DRV_SetReg32 (GRAPH1SYS_CG_SET, (1<<0));

    /* GRAPH1SYS memory PDN*/
    u4Val = 0;
    u4Val = ((1<<2)|(1<<3)|(1<<4)|(1<<6)|(1<<9));
    DRV_SetReg32 (G1_MEM_PDN, u4Val);

	/* GRAPH2SYS CG */
    DRV_SetReg32 (GRAPH2SYS_CG_SET, 0x1FF);	

    /* GRAPH2SYS  memory PDN*/
    u4Val = 0;
    u4Val = ((1<<0));
    DRV_SetReg32 (G2_MEM_PDN, u4Val);

	/* GRAPH2SYS MTCMOS */
	DRV_SetReg16(ISO_EN, 1<<4);
	DRV_SetReg16(IN_ISO_EN, 1<<4 );    
	DRV_SetReg16(PWR_OFF, 1<<4 );
	DRV_SetReg16(ACK_CLR, 0x2); 
#endif	
    /* CEVA memory PDN*/
    u4Val = 0;
    u4Val = ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5));
    DRV_SetReg32 (CEVA_MEM_PDN, u4Val);

	/* CEVA MTCMOS */	
    //printf("CEVA MTCMOS\n");
	DRV_SetReg16(ISO_EN, 1<<5);
	DRV_SetReg16(IN_ISO_EN, 1<<5 );    
	DRV_SetReg16(PWR_OFF, 1<<5 );
	DRV_SetReg16(ACK_CLR, 0x1); 

#if 1
    /* Stop CEVA PLL*/
    DRV_ClrReg32(CEVAPLL2,1<<0);
#endif

    /* Stop UPLL*/
    DRV_ClrReg32(PDN_CON,1<<4);
    /* Stop DPLL*/
#if 0
	printf("DPLL Power Down\n");
    DRV_ClrReg32(PDN_CON,1<<3);
#endif

#endif
}    


#endif

static void mt6573_pinmux_init(void)
{
	mt_gpio_set_default();
}

int board_init (void)
{

    gd->bd->bi_arch_number = MACH_TYPE_MT6573;	/* board id for linux */
    gd->bd->bi_boot_params = CFG_BOOTARGS_ADDR; /* address of boot parameters */



#if 0
    UBOOT_TRACER;
#endif
    
    mt6573_serial_init();
    mt6573_pinmux_init();

    mt65xx_pmu_init();

#if 0
    UBOOT_TRACER;
#endif

#ifdef CM3623
    // als_ps init and must befor lcm
    // fix hw issue
    //the chip cm3623 will pull vdd2.8 to 1.8 if cm3623 uninit when battary low
	printf("cm3623 init\r\n");
	cm3623_init_client();
#endif

    leds_init();

    gd->fb_base = memory_size() - mt65xx_disp_get_vram_size();
    #ifdef CFG_LCD
    mt65xx_disp_init((void*)gd->fb_base);
    UBOOT_TRACER;
    #endif 

#ifndef CONFIG_SYS_NO_DCACHE
    /* Hong-Rong: To enable MMU & d-cache */
    dcache_enable();
#endif

    return 0;
}

extern const char version_string;
extern kal_bool pmic_chrdet_status(void);

int misc_init_r (void)
{
    BOOT_ARGUMENT *boot_arg;
    int ret = 0;

    mt6573_part_init(BLK_NUM(1 * GB));
    mt6573_part_dump();

    //*****************
    //* check mode (1) factory mode (2) meta mode ...
    boot_mode_select();

    printf("MT6573 CHIP ECO Version = %x\n", get_chip_eco_ver());

#ifdef CONFIG_MMC
    mmc_legacy_init(1);
#endif

       // BEGIN
    ret = mboot_common_load_logo((unsigned long)mt65xx_get_logo_db_addr(),
                    CFG_LOGO_NAME);
    if (ret <= 0)
    {
        printf("[ERROR] load logo partition failed, ret: %d\n", ret);
    }


    // END
   

#ifdef CFG_LCD

    /* clean console screen */
    video_clean_screen();

    /* load logo image to the background buffer */

    ret = mboot_common_load_logo((unsigned long)mt65xx_get_logo_db_addr(),
                    CFG_LOGO_NAME);
    if (ret <= 0)
    {
        printf("[ERROR] load logo partition failed, ret: %d\n", ret);
    }

    #ifdef CFG_POWER_CHARGING
    /* Charger exist */
    if( pmic_chrdet_status() )
    {
	    printf("Show CHARGING_PICTURE\n");
    	mt65xx_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
        mt65xx_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
		//mt65xx_backlight_on();	/*don't open backlight now*/
    } 
    else
    #endif
    {        
    }

#endif

  /*Avoid 3G load run onto 2G-platform*/
#if defined (MODEM_3G)
  boot_arg = (volatile BOOT_ARGUMENT *)(BOOT_ARGUMENT_LOCATION);
  if(boot_arg->e_flag == 0x1)
  {
    mt65xx_disp_show_boot_logo();
//    mt65xx_backlight_on();	/*don't open backlight now*/
    printf("[Uboot]ERROR!!! 3G load could not run at 2G-platform!");
    video_printf("[ERROR]3G load can't run at 2G-phone! \n");
    while(1);
  }
#endif

    // Battery Animation Unit Test
#if 0
    {
        int prog = 0;
        
        mt65xx_disp_enter_charging_state();

        while(1)
        {
            udelay(500 * 1000); // delay 0.5 sec
            mt65xx_disp_show_battery_capacity(prog);
            prog += 25;
            if (prog > 100) prog = 0;
        }
    }
#endif

    return 0;
}

u32 get_board_rev(void)
{
        u32 btype = 0x6573;
        u32 hw_ver = 0x8A00;
        u32 fw_ver = 0x8A00;

        return (btype << 16 | hw_ver << 8 | fw_ver);
}

