

#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/tcm.h>
#include <asm/uaccess.h>

#include "mach/mt6573_reg_base.h"
#include "mach/irqs.h"
#include "mach/mt6573_eint.h"
#include "mach/dma.h"
#include "mach/sync_write.h"
#include "mach/mt6573_pll.h"
#include "mach/mt6573_typedefs.h"
#include "mach/mtkpm.h"
#include "mach/mt6573_ost_sm.h"
#include "mach/mt6573_gpio.h"
#include "pmu6573_hw.h"
#include "pmu6573_sw.h"
#include "upmu_common_sw.h"
#include "auddrv_register.h"
#include "cust_leds.h"

UINT32 gChipVer ; 
UINT32 CM_DBG_FLAG = 0x0; //DBG_PMAPI_CG | DBG_PMAPI_SUB | DBG_PMAPI_CG_LIS;

ROOTBUS_HW g_MT6573_BusHW ;

BOOL bCanEnDVFS = FALSE ; 
BOOL bBUCK_ADJUST = FALSE ; 
BOOL bEnDVFSLog = FALSE;


#if 0

void hwLowPowerPLLSwitch(void)
{
    UINT32 delay;
    printk("dump code to internal SRAM\n\r");
    MT6573_EMI_switch();

    /* Configuring the Mixedsys clock control and overrides for low power Audio mode (option 1) */
    /* Fire up the AUXPLL, running at 208MHz */
    //*AUX_PLL_CON0 = *AUX_PLL_CON0 | 0x0001;
    DRV_SetReg32(AUXPLL_CON0_REG,0x0001);

    /* Waiting for AUXPLL Lock */
    while (((DRV_Reg32(AUXPLL_CON0_REG) & 0x8000) >> 15) == 0);

    // ===========================================================
    // Below are OVERRIDE MUX in PLL BLOCK
    // ===========================================================
    /* TINFO="RG_OVRD_SYS_CLK  : Select AUXPLL (26MHz)  as Sysclk source" */
    /* TINFO="RG_OVRD_BUS_CLK  : Select AUXPLL (104MHz) as Bus clock source" */
    /* TINFO="RG_OVRD_AUDIO_CLK: Select AUXPLL (26MHz)  as Audio clock source" */
    //*PLL_CON2   = *PLL_CON2 | 0x3200; /* bit 13,12, bit 9 */
    DRV_SetReg32(PLL_CON2_REG,0x3200);

    // ===========================================================
    // BELOW ARE MUX in CLKSW
    // ===========================================================
    /* TINFO="EMI_PLLSEL: Select AUXPLL (208M) as EMI clock source" */
    //*PLL_CON4   = (*PLL_CON4 & 0xFF0F) | 0x0020;
    //DRV_WriteReg32(PLL_CON4_REG, ((DRV_Reg32(PLL_CON4_REG)& 0xFF0F)|0x0020) );
    
    /* TINFO="APMCU_PLLSEL: Select AUXPLL (208M) as AP MCU clock source" */
    //*PLL_CON5 = (*PLL_CON5 & 0x0FFF) | 0x2000;
    DRV_WriteReg32(PLL_CON5_REG, ((DRV_Reg32(PLL_CON5_REG)& 0x0FFF)|0x2000) );

    // -- Not needed since OVRD used ?
    //  /* TINFO="SYS_PLLSEL: Select AUXPLL (26M) as SYSCLK clock source" */
    //*PLL_CON6 = *PLL_CON6 | 0x0010;
    //DRV_SetReg32(PLL_CON6_REG,0x0010);


    /*Step 0?: Backup DQSV*/
    /*Step 1: Disable Dummy Read*/
    *EMI_DRCT &=    ~0x1;       
    /*Step 2: Block all emi transaction*/
    *EMI_CONM =     0x003F;    
    /*Step 3.0: */
    *EMI_DQSE &= ~(1<<28); //Clear
    /*Step 3: Enter SDRAM self refresh mode and polling status*/
    *EMI_CONN |=    (1<<5);     
    while( ( *EMI_CONN & (1<<7) ) == 0 )
        ;
    /*Step 4: Wait for EMI idle bit assert*/
    while( ( *EMI_CONN & (1<<10) ) == 0 )   
        ;
    /*Step 5: Disable DQS strobe auto tracking*/
    *EMI_DQSE &= 0x7FFF0000;    

    /*Step 6: Set DQSA~DQSD*/
    /*Step 7: Change EMI Setting*/
    //_set_emi_register_by_emi_clock_index( FALSE, Prof.emi_idx );
#if 0    
    
    /*Step 8: Switch Clock*/
    *ARM_FSEL = _divisor_to_regval( SAL_CLK_ARM_DIVISIOR[Prof.arm_idx] );
    *DSP_FSEL = _divisor_to_regval( SAL_CLK_DSP_DIVISIOR[Prof.dsp_idx] );
    *EMI_FSEL = _divisor_to_regval( SAL_CLK_EMI_DIVISIOR[Prof.emi_idx] );
    
    *FBUS_FSEL = _multiplier_to_regval( SAL_CLK_FBUS_MULTIPLIER[Prof.fbus_idx] );
    *SBUS_FSEL = _multiplier_to_regval( SAL_CLK_SBUS_MULTIPLIER[Prof.sbus_idx] );
#endif

    /*Step 8: Switch Clock*/
    /* TINFO="EMI_PLLSEL: Select AUXPLL (208M) as EMI clock source" */
    //*PLL_CON4   = (*PLL_CON4 & 0xFF0F) | 0x0020;
    DRV_WriteReg32(PLL_CON4_REG, ((DRV_Reg32(PLL_CON4_REG)& 0xFF0F)|0x0020) );

    for( delay = 0; delay <= 0xFFFF; delay++ )
        ;
    
    /*Step 9: Unmask EMI access for all master*/
    *EMI_CONM = 0x0000;
    
    /*Step 10.0: IF Set, BUS will hang when access EMI*/
    //*EMI_DQSE |= (1<<28); 
    
    /*Step 10: Exit SDRAM self refresh mode and polling status*/
    *EMI_CONN &=    ~(1<<5);     
    while( ( *EMI_CONN & (1<<7) ) == 1 )
        ;    
    
    /*Step 11: Enable dummy Read*/
    *EMI_DRCT |=    0x1; 
}
#endif

void mt6573_load_spare_settings(void)
{
    u16 spar0;
    spar0 = 0;
    if (spar0 & SPARE_SECRET_KEY)
    {
        if(spar0 & SPARE_E1_PATCH)
            gChipVer = CHIP_VER_E2;

        if(spar0 & SPARE_DVFS_EN)
            bCanEnDVFS = TRUE;
        else
            bCanEnDVFS = FALSE;

        if(spar0 & SPARE_VAPROC_ADJUST_EN)
            bBUCK_ADJUST = TRUE;
        else
            bBUCK_ADJUST = FALSE;

        if(spar0 & SPARE_DVFS_LOG)
            bEnDVFSLog = TRUE;
        else
            bEnDVFSLog = FALSE;
    }
}

void mt6573_CG_init(void)
{
    UINT32 u4Val;
    struct cust_mt65xx_led *cust_led_list = get_cust_led_list();

    set_clock_listen(TRUE);
    DRV_SetReg32(APMCU_CG_CLR0, 0xffffffff);
    //DRV_SetReg32(APMCU_CG_SET0, 0x2fff);//ungate UART4
    DRV_SetReg32(APMCU_CG_SET0, 0x2dff);//ungate UART1 and UART4
    
    DRV_SetReg32(APMCU_CG_CLR1, 0xfffffff9);//unclear SIMIF0, SIMIF1
    if(cust_led_list[MT65XX_LED_TYPE_LCD].mode == MT65XX_LED_MODE_PWM)
    {
       DRV_SetReg32(APMCU_CG_SET1, 0x1f49);//ungate SIMIF0,SIMIF1,APDMA,NFI, PWM
    }
    else
    {
       DRV_SetReg32(APMCU_CG_SET1, 0x1f69);//ungate SIMIF0,SIMIF1,APDMA,NFI
    }

    DRV_SetReg32(MMSYS1_CG_CLR0, 0xffffffff);
    DRV_SetReg32(MMSYS1_CG_SET0, 0xfe7ffffe);//ungate LCD, DPI, GMC1
   
    DRV_SetReg32(MMSYS1_CG_CLR1, 0xffffffff);
    DRV_SetReg32(MMSYS1_CG_SET1, 0xff);//ungate GMC1E, GMC1SLV
   
    MTCMOS_En(MM2_SUBSYS);
    DRV_SetReg32(MMSYS2_CG_CLR0, 0xffffffff);
    DRV_SetReg32(MMSYS2_CG_SET0, 0x3e);
   
    MTCMOS_En(AUDIO_SUBSYS);
    u4Val = DRV_Reg32(AUDIO_TOP_CON0_REG);    
    u4Val |= 0x74;
    DRV_SetReg32(AUDIO_TOP_CON0_REG, u4Val); 

}

void mt6573_chip_dep_init(void)
{
     dcm_disable_state(DEEP_IDLE_STATE);
        
    /* ALL MMSYS clock is couple with Bus clock, we cannot enable DCM at E1 */
    if (gChipVer == CHIP_VER_E1 )
    {
        /*VA25 MODE_SEL, disable low power mode for VA25*/
        DRV_SetReg32((VA25_CON0+0x4),(0x1<<4));    
        DRV_ClrReg32((VA25_CON0+0x4),(0x1<<3));    
        /*VA12 MODE_SEL*/
        DRV_SetReg32((VA12_CON0),(0x1<<1));    
        /*VA12 Force On*/
        DRV_SetReg32((VA12_CON0),(0x1<<0));    
        /*MPLL force on */        
        DRV_SetReg32(MPLL_CON1_REG, (0x1<<8));   
        /*APLL force on */        
        DRV_SetReg32(AMPLL_CON1_REG, (0x1<<8));   
        /*DPLL force on */        
        DRV_SetReg32(DPLL_CON1_REG, (0x1<<8));   
        /*EPLL force on */        
        DRV_SetReg32(EPLL_CON1_REG, (0x1<<8));   
        /*3GPLL force on */        
        DRV_SetReg32(WPLL_CON0_REG, (0x1<<8));   
        /*2GPLL force on */        
        DRV_SetReg32(GPLL_CON0_REG, (0x1<<8));   

        dcm_enable_state(SLOW_IDLE_STATE);
        dcm_enable_state(LITTLE_IDLE_STATE);
        /* The bus gating register for subsystem. This option will enables clock gating automatically 
                when the bus fabric is detected in idle mode. Together, HW will mask the handshake signals from AXI masters*/
        DRV_SetReg32(BUS_GATING_EN,0xDF);

    }
    else /*Let Topsm turn off it */
    {
        /*VA25 MODE_SEL, disable low power mode for VA25*/
        //DRV_SetReg32((VA25_CON0+0x4),(0x1<<4));    
        //DRV_ClrReg32((VA25_CON0+0x4),(0x1<<3));    

        DRV_ClrReg32(MPLL_CON1_REG, (0x1<<8));   
        DRV_ClrReg32(AMPLL_CON1_REG, (0x1<<8));   
        DRV_ClrReg32(DPLL_CON1_REG, (0x1<<8));   
        DRV_ClrReg32(EPLL_CON1_REG, (0x1<<8));   
        DRV_ClrReg32(WPLL_CON0_REG, (0x1<<8));   
        DRV_ClrReg32(GPLL_CON0_REG, (0x1<<8));   

        dcm_enable_state(SLOW_IDLE_STATE);
        dcm_enable_state(DEEP_IDLE_STATE);
        dcm_enable_state(LITTLE_IDLE_STATE);
        /* The bus gating register for subsystem. This option will enables clock gating automatically 
                when the bus fabric is detected in idle mode. Together, HW will mask the handshake signals from AXI masters*/
        DRV_SetReg32(BUS_GATING_EN,0xFF);
        
    }
}

void mt6573_dcm_init(void)
{
    /* HW DCM init*/
    MT6573_INIT_IDLE_THREAD();              
    /* The Clock CG enable of AXI bus. This option will enables clock gating automatically 
        when there is no traffic in the bus fabric. Together, HW will mask the handshake signals from AXI masters.*/
    DRV_SetReg32(APPER_SLP_CON,0x3);
    /* The Clock CG enable of AXI bus. This option will enables clock gating automatically 
        when there is no traffic in the bus fabric (bus_f122). Together, HW will mask the handshake signals from AXI masters.*/
    DRV_SetReg32(AP_SLPPRT_BUS,(1<<8) );

}

void mt6573_power_management_init(void)
{
    /* Check Chip Version */
    gChipVer = DRV_Reg32(APHW_VER);
    printk("[%s]: gChipVer = 0x%x\r\n",__FUNCTION__, gChipVer);
    /* Load DVFS, DCM Setting from Spare*/
    mt6573_load_spare_settings();
    /* Clock Gating init, gated un-necessary power*/
    mt6573_CG_init();
    /* Set specific chip setting*/
    mt6573_chip_dep_init();         
    /* Power mamagement log init*/
    mt6573_log_init();
    /* DCM init*/
    mt6573_dcm_init();
    /* Thermal protect Init*/
    hwThermalProtectInit();
    /* Sleep Controller init*/
    slp_mod_init();
}




