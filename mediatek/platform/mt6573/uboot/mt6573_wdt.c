
#include <common.h>
#include <asm/io.h>

#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mt6573_wdt_hw.h>

static unsigned short timeout;

void hw_watchdog_disable(void)
{
    u16 tmp;
    
    tmp = DRV_Reg(MT6573_WDT_MODE);
    tmp &= ~MT6573_WDT_MODE_ENABLE;		  /* disable watchdog */
    tmp |= (MT6573_WDT_MODE_KEY);       /* need key then write is allowed */
    DRV_WriteReg(MT6573_WDT_MODE,tmp);
}

void MT6573_sw_watchdog_reset(void)
{
    /* Watchdog Rest */
    DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY); 
    DRV_WriteReg(MT6573_WDT_MODE, (MT6573_WDT_MODE_KEY|MT6573_WDT_MODE_EXTEN|MT6573_WDT_MODE_ENABLE));
    //DRV_WriteReg(MT6573_WDT_LENGTH, MT6573_WDT_LENGTH_KEY);
    DRV_WriteReg(MT6573_WDT_SWRST, MT6573_WDT_SWRST_KEY);
}


unsigned int MT6573_wdt_CheckStatus(void)
{
	unsigned int status;
	
	status = DRV_Reg16(MT6573_WDT_STATUS);
	
	return status;
}

/************************************/
/* Copy from pre-loader to do reset */
/************************************/
void MT6573_wdt_ModeSelection(kal_bool en, kal_bool auto_rstart, kal_bool IRQ )
{
	unsigned short tmp;
    
	tmp = DRV_Reg16(MT6573_WDT_MODE);
	tmp |= MT6573_WDT_MODE_KEY;
	
	// Bit 0 : Whether enable watchdog or not
	if(en == KAL_TRUE)
		tmp |= MT6573_WDT_MODE_ENABLE;
	else
		tmp &= ~MT6573_WDT_MODE_ENABLE;
	
	// Bit 4 : Whether enable auto-restart or not for counting value
	if(auto_rstart == KAL_TRUE)
		tmp |= MT6573_WDT_MODE_AUTORST;
	else
		tmp &= ~MT6573_WDT_MODE_AUTORST;

	// Bit 3 : TRUE for generating Interrupt (False for generating Reset) when WDT timer reaches zero
	if(IRQ == KAL_TRUE)
		tmp |= MT6573_WDT_RESET_IRQ;
	else
		tmp &= ~MT6573_WDT_RESET_IRQ;

	DRV_WriteReg16(MT6573_WDT_MODE,tmp);
}

void MT6573_wdt_SetTimeOutValue(unsigned short value)
{
	/*
	 * TimeOut = BitField 15:5
	 * Key	   = BitField  4:0 = 0x08
	 */
	
	// sec * 32768 / 512 = sec * 64 = sec * 1 << 6
	timeout = (unsigned short)(value * ( 1 << 6) );
	timeout = timeout << 5; 
	DRV_WriteReg16(MT6573_WDT_LENGTH, (timeout | MT6573_WDT_LENGTH_KEY) );	
}

void MT6573_wdt_Restart(void)
{
	// Reset WatchDogTimer's counting value to default value
	// ie., keepalive()

	DRV_WriteReg16(MT6573_WDT_RESTART, MT6573_WDT_RESTART_KEY);
}

void WDT_HW_Reset(void)
{
	//dbg_print("WDT_HW_Reset_test\n");
	
	// 1. set WDT timeout 1 secs, 1*64*512/32768 = 1sec
	MT6573_wdt_SetTimeOutValue(1);
	
	// 2. enable WDT reset, auto-restart disable, disable intr
	MT6573_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
	
	// 3. reset the watch dog timer to the value set in WDT_LENGTH register
	MT6573_wdt_Restart();
	
	// 4. system will reset
	while(1);
}

void hw_watchdog_reset(void)
{

}

void mt6573_arch_reset(char mode)
{
	printf("mt6573_arch_reset\n");

	MT6573_sw_watchdog_reset();

	while (1);
}

#define WDT_INGORE_POWER_KEY (0x0ffe)
bool WDT_boot_check(void)
{
	if (WDT_INGORE_POWER_KEY == DRV_Reg16(MT6573_WDT_INTERNAL)) 
  	{
  		return true;
  	}
  	return false; 
}
void WDT_arch_reset(char mode)
{
	WDT_sw_watchdog_reset();

	while (1);
}

