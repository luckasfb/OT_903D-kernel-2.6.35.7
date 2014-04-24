

#define ENABLE_DSI_INTERRUPT 0 

#include <linux/delay.h>

#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/mt6573_irq.h>
#include <mach/mt6573_pll.h>
#include <mach/mt6573_gpio.h>

#include "dsi_reg.h"
#include "dsi_drv.h"

#if ENABLE_DSI_INTERRUPT
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <asm/tcm.h>
#include <mach/irqs.h>
#include "mtkfb.h"
static wait_queue_head_t _dsi_wait_queue;
#endif


//#define PWR_OFF                 (APCONFIG_BASE + 0x0304)
//#define GRAPH1_PDN              (1 << 3)
#define G1_MEM_PDN              (APCONFIG_BASE + 0x0060)
#define G1_MEM_DSI              (1)
#define GRAPH1SYS_CG_SET        (MMSYS1_CONFIG_BASE + 0x320)
#define GRAPH1SYS_CG_CLR        (MMSYS1_CONFIG_BASE + 0x340)
#define GRAPH1SYS_CG_DSI        (1 << 28)

PDSI_REGS const DSI_REG = (PDSI_REGS)(DSI_BASE);
PDSI_CMDQ_REGS const DSI_CMDQ_REG = (PDSI_CMDQ_REGS)(DSI_BASE+0x180);

typedef struct
{
	DSI_REGS regBackup;
} DSI_CONTEXT;

static bool s_isDsiPowerOn = FALSE;
static DSI_CONTEXT _dsiContext;

static void lcm_mdelay(UINT32 ms)
{
    udelay(1000 * ms);
}

#if ENABLE_DSI_INTERRUPT
static __tcmfunc irqreturn_t _DSI_InterruptHandler(int irq, void *dev_id)
{   
    DSI_INT_STATUS_REG status = DSI_REG->DSI_INTSTA;

    if (status.RD_RDY)
    {
        wake_up_interruptible(&_dsi_wait_queue);
    }

    if (status.CMD_DONE)
    {
        wake_up_interruptible(&_dsi_wait_queue);
    }

    return IRQ_HANDLED;
}
#endif


static BOOL _IsEngineBusy(void)
{
	DSI_STATUS_REG status;

	status = DSI_REG->DSI_STA;
	
	if (status.BUSY || status.ERR_MSG)
	{
		printk("[DISP] DSI status BUSY: %d, ERR_MSG: %d !! \n", status.BUSY, status.ERR_MSG); 		
	
		return TRUE;
	}

	//printk("[DISP] DSI status Done !! \n"); 		

	return FALSE;
}


static void _WaitForEngineNotBusy(void)
{
#if ENABLE_DSI_INTERRUPT
    static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec

    if (in_interrupt())
    {
        // perform busy waiting if in interrupt context
        while(_IsEngineBusy()) {}
    }
    else
    {
        while (_IsEngineBusy())
        {
            long ret = wait_event_interruptible_timeout(_dsi_wait_queue, 
                                                        !_IsEngineBusy(),
                                                        WAIT_TIMEOUT);
            if (0 == ret) {
                printk("[WARNING] Wait for LCD engine not busy timeout!!!\n");
            }
        }
    }
#else

    //printk("[DISP] _WaitForEngineNotBusy !! \n"); 
    while(_IsEngineBusy()) { }

#endif    
}

static void _BackupDSIRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);
	
    //memcpy((void*)&(_dsiContext.regBackup), (void*)DSI_BASE, sizeof(DSI_REGS));

    OUTREG32(&regs->DSI_INTEN, AS_UINT32(&DSI_REG->DSI_INTEN));
    OUTREG32(&regs->DSI_MODE_CTRL, AS_UINT32(&DSI_REG->DSI_MODE_CTRL));
    OUTREG32(&regs->DSI_TXRX_CTRL, AS_UINT32(&DSI_REG->DSI_TXRX_CTRL));
    OUTREG32(&regs->DSI_PSCTRL, AS_UINT32(&DSI_REG->DSI_PSCTRL));

    OUTREG32(&regs->DSI_VSA_NL, AS_UINT32(&DSI_REG->DSI_VSA_NL));		
    OUTREG32(&regs->DSI_VBP_NL, AS_UINT32(&DSI_REG->DSI_VBP_NL));		
    OUTREG32(&regs->DSI_VFP_NL, AS_UINT32(&DSI_REG->DSI_VFP_NL));		
    OUTREG32(&regs->DSI_VACT_NL, AS_UINT32(&DSI_REG->DSI_VACT_NL));		

    OUTREG32(&regs->DSI_LINE_NB, AS_UINT32(&DSI_REG->DSI_LINE_NB));		
    OUTREG32(&regs->DSI_HSA_NB, AS_UINT32(&DSI_REG->DSI_HSA_NB));		
    OUTREG32(&regs->DSI_HBP_NB, AS_UINT32(&DSI_REG->DSI_HBP_NB));		
    OUTREG32(&regs->DSI_HFP_NB, AS_UINT32(&DSI_REG->DSI_HFP_NB));		

    OUTREG32(&regs->DSI_RGB_NB, AS_UINT32(&DSI_REG->DSI_RGB_NB));		
    OUTREG32(&regs->DSI_HSA_WC, AS_UINT32(&DSI_REG->DSI_HSA_WC));		
    OUTREG32(&regs->DSI_HBP_WC, AS_UINT32(&DSI_REG->DSI_HBP_WC));		
    OUTREG32(&regs->DSI_HFP_WC, AS_UINT32(&DSI_REG->DSI_HFP_WC));		
	
    OUTREG32(&regs->DSI_MEM_CONTI, AS_UINT32(&DSI_REG->DSI_MEM_CONTI));

    OUTREG32(&regs->DSI_PHY_TIMECON0, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON0));
    OUTREG32(&regs->DSI_PHY_TIMECON1, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON1));
    OUTREG32(&regs->DSI_PHY_TIMECON2, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON2));
    OUTREG32(&regs->DSI_PHY_TIMECON3, AS_UINT32(&DSI_REG->DSI_PHY_TIMECON3));
	
}

static void _RestoreDSIRegisters(void)
{
    DSI_REGS *regs = &(_dsiContext.regBackup);

    OUTREG32(&DSI_REG->DSI_INTEN, AS_UINT32(&regs->DSI_INTEN));	
    OUTREG32(&DSI_REG->DSI_MODE_CTRL, AS_UINT32(&regs->DSI_MODE_CTRL));	
    OUTREG32(&DSI_REG->DSI_TXRX_CTRL, AS_UINT32(&regs->DSI_TXRX_CTRL));	
    OUTREG32(&DSI_REG->DSI_PSCTRL, AS_UINT32(&regs->DSI_PSCTRL));	

    OUTREG32(&DSI_REG->DSI_VSA_NL, AS_UINT32(&regs->DSI_VSA_NL));		
    OUTREG32(&DSI_REG->DSI_VBP_NL, AS_UINT32(&regs->DSI_VBP_NL));		
    OUTREG32(&DSI_REG->DSI_VFP_NL, AS_UINT32(&regs->DSI_VFP_NL));		
    OUTREG32(&DSI_REG->DSI_VACT_NL, AS_UINT32(&regs->DSI_VACT_NL));		

    OUTREG32(&DSI_REG->DSI_LINE_NB, AS_UINT32(&regs->DSI_LINE_NB));		
    OUTREG32(&DSI_REG->DSI_HSA_NB, AS_UINT32(&regs->DSI_HSA_NB));		
    OUTREG32(&DSI_REG->DSI_HBP_NB, AS_UINT32(&regs->DSI_HBP_NB));		
    OUTREG32(&DSI_REG->DSI_HFP_NB, AS_UINT32(&regs->DSI_HFP_NB));		

    OUTREG32(&DSI_REG->DSI_RGB_NB, AS_UINT32(&regs->DSI_RGB_NB));		
    OUTREG32(&DSI_REG->DSI_HSA_WC, AS_UINT32(&regs->DSI_HSA_WC));		
    OUTREG32(&DSI_REG->DSI_HBP_WC, AS_UINT32(&regs->DSI_HBP_WC));		
    OUTREG32(&DSI_REG->DSI_HFP_WC, AS_UINT32(&regs->DSI_HFP_WC));		

    OUTREG32(&DSI_REG->DSI_MEM_CONTI, AS_UINT32(&regs->DSI_MEM_CONTI));		

    OUTREG32(&DSI_REG->DSI_PHY_TIMECON0, AS_UINT32(&regs->DSI_PHY_TIMECON0));		
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON1, AS_UINT32(&regs->DSI_PHY_TIMECON1));
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON2, AS_UINT32(&regs->DSI_PHY_TIMECON2));		
    OUTREG32(&DSI_REG->DSI_PHY_TIMECON3, AS_UINT32(&regs->DSI_PHY_TIMECON3));		

}

static void _ResetBackupedDSIRegisterValues(void)
{
    DSI_REGS *regs = &_dsiContext.regBackup;
    memset((void*)regs, 0, sizeof(DSI_REGS));
}

DSI_STATUS DSI_Init(void)
{
    DSI_STATUS ret = DSI_STATUS_OK;

    memset(&_dsiContext, 0, sizeof(_dsiContext));

    // LCD controller would NOT reset register as default values
    // Do it by SW here
    //
    _ResetBackupedDSIRegisterValues();

    ret = DSI_PowerOn();

    //DSI_REG->DSI_INTEN.CMD_DONE=1;

    ASSERT(ret == DSI_STATUS_OK);

#if ENABLE_DSI_INTERRUPT
    if (request_irq(MT6573_DSI_IRQ_LINE,
        _DSI_InterruptHandler, 0, MTKFB_DRIVER, NULL) < 0)
    {
        printk("[DSI][ERROR] fail to request DSI irq\n"); 
        return DSI_STATUS_ERROR;
    }

    init_waitqueue_head(&_dsi_wait_queue);
    DSI_REG->DSI_INTEN.CMD_DONE=1;
    DSI_REG->DSI_INTEN.RD_RDY=1;
#endif
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_Deinit(void)
{
    DSI_STATUS ret = DSI_PowerOff();

    ASSERT(ret == DSI_STATUS_OK);

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOn(void)
{
    if (!s_isDsiPowerOn)
    {
#if 1   // FIXME
        BOOL ret = hwEnableClock(MT65XX_PDN_MM_DSI, "DSI");
        ASSERT(ret);
#else
        //UINT32 temp = *(unsigned int*)(APCONFIG_BASE+0xb0);
        //*(unsigned int*)(APCONFIG_BASE+0xb0) = (temp|0x100);

        *(unsigned int*)(MMSYS1_CONFIG_BASE+0x340) |= 0x10000000;
        //#warning "TODO: power on LCD manually before Kelvin's PM API is ready"
#endif        
        _RestoreDSIRegisters();
        s_isDsiPowerOn = TRUE;
    }

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_PowerOff(void)
{
    if (s_isDsiPowerOn)
    {
        BOOL ret = TRUE;
        _WaitForEngineNotBusy();
        _BackupDSIRegisters();
#if 1   // FIXME
        ret = hwDisableClock(MT65XX_PDN_MM_DSI, "DSI");
        ASSERT(ret);
#else
        //UINT32 temp = *(unsigned int*)(APCONFIG_BASE+0xb0);
        //*(unsigned int*)(APCONFIG_BASE+0xb0) = (temp|0x100);

        *(unsigned int*)(MMSYS1_CONFIG_BASE+0x320) |= 0x10000000;
        //#warning "TODO: power off LCD manually before Kelvin's PM API is ready"
#endif        
        s_isDsiPowerOn = FALSE;
    }
    
    return DSI_STATUS_OK;
}


DSI_STATUS DSI_WaitForNotBusy(void)
{
    _WaitForEngineNotBusy();

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_EnableClk(void)
{
	DSI_REG->DSI_START.DSI_START=0;
	DSI_REG->DSI_START.DSI_START=1;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_DisableClk(void)
{
	DSI_REG->DSI_START.DSI_START=0;

    return DSI_STATUS_OK;
}


DSI_STATUS DSI_handle_TE(void)
{

	unsigned int data_array;

	//data_array=0x00351504;
	//DSI_set_cmdq(&data_array, 1, 1);

	//lcm_mdelay(10);

	// RACT	
	//data_array=1;
	//OUTREG32(&DSI_REG->DSI_RACK, data_array);

	// TE + BTA
	data_array=0x24;
	printk("[DISP] DSI_handle_TE TE + BTA !! \n");
	OUTREG32(&DSI_CMDQ_REG->data0, data_array);

	//DSI_CMDQ_REG->data0.byte0=0x24;
	//DSI_CMDQ_REG->data0.byte1=0;
	//DSI_CMDQ_REG->data0.byte2=0;
	//DSI_CMDQ_REG->data0.byte3=0;

	DSI_REG->DSI_CMDQ_SIZE.CMDQ_SIZE=1;

	DSI_REG->DSI_START.DSI_START=0;
	DSI_REG->DSI_START.DSI_START=1;

	// wait TE Trigger status
//	printk("[DISP] wait TE Trigger status !! \n");
//	do
//	{
		lcm_mdelay(10);

		data_array=INREG32(&DSI_REG->DSI_INTSTA);
		printk("[DISP] DSI INT state : %x !! \n", data_array);
	
		data_array=INREG32(&DSI_REG->DSI_TRIG_STA);
		printk("[DISP] DSI TRIG TE status check : %x !! \n", data_array);
//	} while(!(data_array&0x4));

//	printk("[DISP] DSI TRIG TE check Done !! \n");

	// RACT	
	printk("[DISP] DSI Set RACT !! \n");
	data_array=1;
	OUTREG32(&DSI_REG->DSI_RACK, data_array);

	return DSI_STATUS_OK;

}


void DSI_PHY_TIMCONFIG(unsigned int *setting)
{
	OUTREG32(&DSI_REG->DSI_PHY_TIMECON0, AS_UINT32((setting+0)));
	OUTREG32(&DSI_REG->DSI_PHY_TIMECON1, AS_UINT32((setting+1)));
	OUTREG32(&DSI_REG->DSI_PHY_TIMECON2, AS_UINT32((setting+2)));	
	OUTREG32(&DSI_REG->DSI_PHY_TIMECON3, AS_UINT32((setting+3)));
}


void DSI_clk_ULP_mode(bool enter)
{
	DSI_PHY_LCCON_REG tmp_reg1;
	DSI_PHY_REG_ANACON0	tmp_reg2;

	tmp_reg1=DSI_REG->DSI_PHY_LCCON;
	tmp_reg2=DSI_PHY_REG->ANACON0;

	if(enter) {

		tmp_reg1.LC_HS_TX_EN=0;
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.LC_ULPM_EN=1;
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg2.PLL_EN=0;
		OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&tmp_reg2));

	}
	else {

		tmp_reg2.PLL_EN=1;
		OUTREG32(&DSI_PHY_REG->ANACON0, AS_UINT32(&tmp_reg2));
		lcm_mdelay(1);
		tmp_reg1.LC_ULPM_EN=0;
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.LC_WAKEUP_EN=1;
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.LC_WAKEUP_EN=0;
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.LC_HS_TX_EN=1;
		OUTREG32(&DSI_REG->DSI_PHY_LCCON, AS_UINT32(&tmp_reg1));

	}
}

void DSI_lane0_ULP_mode(bool enter)
{
	DSI_PHY_LD0CON_REG tmp_reg1;

	tmp_reg1=DSI_REG->DSI_PHY_LD0CON;

	if(enter) {

		tmp_reg1.L0_HS_TX_EN=0;
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.L0_ULPM_EN=1;
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));

	}
	else {

		tmp_reg1.L0_ULPM_EN=0;
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.L0_WAKEUP_EN=1;
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));
		lcm_mdelay(1);
		tmp_reg1.L0_WAKEUP_EN=0;
		OUTREG32(&DSI_REG->DSI_PHY_LD0CON, AS_UINT32(&tmp_reg1));

	}
}


void DSI_set_cmdq(unsigned int *pdata, unsigned int queue_size, unsigned char force_update)
{
	UINT32 i;

	ASSERT(queue_size<=16);

	for(i=0; i<queue_size; i++)
		OUTREG32(&DSI_CMDQ_REG->data0[i], AS_UINT32((pdata+i)));

	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, queue_size);

	if(force_update) {
		OUTREG32(&DSI_REG->DSI_START, 0);
		OUTREG32(&DSI_REG->DSI_START, 1);
		//while(_IsCMDQBusy()) {}
		_WaitForEngineNotBusy();
	}
}

DSI_STATUS DSI_Write_T0_INS(DSI_T0_INS *t0)
{
    OUTREG32(&DSI_CMDQ_REG->data0[0], AS_UINT32(t0));	

	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;	
}


DSI_STATUS DSI_Write_T1_INS(DSI_T1_INS *t1)
{
    OUTREG32(&DSI_CMDQ_REG->data0[0], AS_UINT32(t1));	

	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T2_INS(DSI_T2_INS *t2)
{
	unsigned int i;
	
	OUTREG32(&DSI_CMDQ_REG->data0[0], AS_UINT32(t2));

	for(i=0;i<((t2->WC16-1)>>2)+1;i++)
	    OUTREG32(&DSI_CMDQ_REG->data0[1+i], AS_UINT32((t2->pdata+i)));

	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, (((t2->WC16-1)>>2)+2));
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}


DSI_STATUS DSI_Write_T3_INS(DSI_T3_INS *t3)
{
    OUTREG32(&DSI_CMDQ_REG->data0[0], AS_UINT32(t3));	

	OUTREG32(&DSI_REG->DSI_CMDQ_SIZE, 1);
	OUTREG32(&DSI_REG->DSI_START, 0);
	OUTREG32(&DSI_REG->DSI_START, 1);

	return DSI_STATUS_OK;
}


void DSI_write_lcm_cmd(unsigned int cmd)
{
	DSI_T0_INS *t0_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=SHORT_PACKET_RW;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=LOW_POWER;
	CONFG_tmp.CL=CL_8BITS;
	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;

	t0_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t0_tmp->Data_ID= (cmd&0xFF);
	t0_tmp->Data0 = 0x0;
	t0_tmp->Data1 = 0x0;	

	DSI_Write_T0_INS(t0_tmp);
}


void DSI_write_lcm_regs(unsigned int addr, unsigned int *para, unsigned int nums)
{
	DSI_T2_INS *t2_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=LONG_PACKET_W;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=LOW_POWER;
	CONFG_tmp.CL=CL_8BITS;
	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;

	t2_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t2_tmp->Data_ID = (addr&0xFF);
	t2_tmp->WC16 = nums;	
	t2_tmp->pdata = para;	

	DSI_Write_T2_INS(t2_tmp);

}

UINT32 DSI_read_lcm_reg(void)
{
	// TBD
	return 0;
}

DSI_STATUS DSI_write_lcm_fb(unsigned int addr, bool long_length)
{
	DSI_T1_INS *t1_tmp=0;
	DSI_CMDQ_CONFG CONFG_tmp;

	CONFG_tmp.type=FB_WRITE;
	CONFG_tmp.BTA=DISABLE_BTA;
	CONFG_tmp.HS=HIGH_SPEED;

	if(long_length)
		CONFG_tmp.CL=CL_16BITS;
	else
		CONFG_tmp.CL=CL_8BITS;		

	CONFG_tmp.TE=DISABLE_TE;
	CONFG_tmp.RPT=DISABLE_RPT;


	t1_tmp->CONFG = *((unsigned char *)(&CONFG_tmp));
	t1_tmp->Data_ID= 0x39;
	t1_tmp->mem_start0 = (addr&0xFF);	

	if(long_length)
		t1_tmp->mem_start1 = ((addr>>8)&0xFF);

	return DSI_Write_T1_INS(t1_tmp);	

	
}


DSI_STATUS DSI_read_lcm_fb(void)
{
	// TBD
	return DSI_STATUS_OK;
}


// -------------------- Retrieve Information --------------------

DSI_STATUS DSI_DumpRegisters(void)
{
    UINT32 i;

    printk("---------- Start dump DSI registers ----------\n");
    
    for (i = 0; i < sizeof(DSI_REGS); i += 4)
    {
        printk("DSI+%04x : 0x%08x\n", i, INREG32(DSI_BASE + i));
    }
    for (i = 0; i < sizeof(DSI_PHY_REGS); i += 4)
    {
        printk("DSI_PHY+%04x : 0x%08x\n", i, INREG32(DSI_PHY_BASE + i));
    }

    return DSI_STATUS_OK;
}

