

#include <common.h>
#include <asm/io.h>
#define __ENABLE_UART_LOG_SWITCH_FEATURE__
//#define __LOG_SWITCH_TO_UART1__

#ifdef __ENABLE_UART_LOG_SWITCH_FEATURE__
/* Header file that need included */
#include <asm/arch/boot_mode.h>
#include <asm/arch/mt6573_rtc.h>
#define MD_CAN_USE_UART1	(0x1)
#define MD_CAN_NOT_USE_UART1	(0x0)
#define	UART1_FOR_TOOL		(0)
#define	UART1_FOR_AP_LOG	(1)
#define	UART1_FOR_MD_LOG	(2)
#define UART1_INVALIDE_VAL	(3)
#define UART1_DEFAULT_SETTING	(UART1_FOR_TOOL)
int is_uart1_can_be_used_by_MD(void);
int get_uart_log_port_id_in_uboot(void);
#endif // __ENABLE_UART_LOG_SWITCH_FEATURE__

#if 0
#include <asm/arch/mt6516.h>
#include <asm/arch/mt6516_typedefs.h>
#include <asm/arch/mt6516_pdn_hw.h>
#include <asm/arch/mt6516_pdn_sw.h>
#include <asm/arch/mt6516_uart.h>
#include <asm/arch/mt6516_serial.h>
#endif
#include <asm/arch/mt65xx_uart.h>
#include <asm/arch/mt65xx_serial.h>

#ifdef CFG_META_MODE
#include <asm/arch/mt65xx_meta.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define ASSERT( expr ) if( !( expr ) ) { \
		printf("<ASSERT> %s:line %d %s\n",__FILE__,__LINE__,(int)(#expr)); }

#define UART_READ8(REG)             __raw_readb(REG)
#define UART_READ16(REG)            __raw_readw(REG)
#define UART_READ32(REG)            __raw_readl(REG)
#define UART_WRITE8(VAL, REG)       __raw_writeb(VAL, REG)
#define UART_WRITE16(VAL, REG)      __raw_writew(VAL, REG)
#define UART_WRITE32(VAL, REG)      __raw_writel(VAL, REG)

#define UART_SET_BITS(BS,REG)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define UART_CLR_BITS(BS,REG)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

#if 0
#define UART_BASE(uart)					  (uart + IO_OFFSET)
#endif
#define UART_BASE(uart)					  (uart)

#define UART_RBR(uart)                    (UART_BASE(uart)+0x0)  /* Read only */
#define UART_THR(uart)                    (UART_BASE(uart)+0x0)  /* Write only */
#define UART_IER(uart)                    (UART_BASE(uart)+0x4)
#define UART_IIR(uart)                    (UART_BASE(uart)+0x8)  /* Read only */
#define UART_FCR(uart)                    (UART_BASE(uart)+0x8)  /* Write only */
#define UART_LCR(uart)                    (UART_BASE(uart)+0xc)
#define UART_MCR(uart)                    (UART_BASE(uart)+0x10)
#define UART_LSR(uart)                    (UART_BASE(uart)+0x14)
#define UART_MSR(uart)                    (UART_BASE(uart)+0x18)
#define UART_SCR(uart)                    (UART_BASE(uart)+0x1c)
#define UART_DLL(uart)                    (UART_BASE(uart)+0x0)  /* Only when LCR.DLAB = 1 */
#define UART_DLH(uart)                    (UART_BASE(uart)+0x4)  /* Only when LCR.DLAB = 1 */
#define UART_EFR(uart)                    (UART_BASE(uart)+0x8)  /* Only when LCR = 0xbf */
#define UART_XON1(uart)                   (UART_BASE(uart)+0x10) /* Only when LCR = 0xbf */
#define UART_XON2(uart)                   (UART_BASE(uart)+0x14) /* Only when LCR = 0xbf */
#define UART_XOFF1(uart)                  (UART_BASE(uart)+0x18) /* Only when LCR = 0xbf */
#define UART_XOFF2(uart)                  (UART_BASE(uart)+0x1c) /* Only when LCR = 0xbf */
#define UART_AUTOBAUD_EN(uart)            (UART_BASE(uart)+0x20)
#define UART_HIGHSPEED(uart)              (UART_BASE(uart)+0x24)
#define UART_SAMPLE_COUNT(uart)           (UART_BASE(uart)+0x28) 
#define UART_SAMPLE_POINT(uart)           (UART_BASE(uart)+0x2c) 
#define UART_AUTOBAUD_REG(uart)           (UART_BASE(uart)+0x30)
#define UART_RATE_FIX_AD(uart)            (UART_BASE(uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)        (UART_BASE(uart)+0x38)
#define UART_GUARD(uart)                  (UART_BASE(uart)+0x3c)
#define UART_ESCAPE_DAT(uart)             (UART_BASE(uart)+0x40)
#define UART_ESCAPE_EN(uart)              (UART_BASE(uart)+0x44)
#define UART_SLEEP_EN(uart)               (UART_BASE(uart)+0x48)
#define UART_VFIFO_EN(uart)               (UART_BASE(uart)+0x4c)
#define UART_RXTRI_AD(uart)               (UART_BASE(uart)+0x50)

// output uart port
volatile unsigned int g_uart;
// output uart baudrate
unsigned int g_brg;

void serial_setbrg(void)
{
    unsigned int byte;
    unsigned int highspeed;
    unsigned int quot, divisor, remainder;
    //unsigned int ratefix; 
    unsigned int uartclk;

    //uartclk = CFG_EMI_CLK >> 1;
    uartclk = 61440000;
    if (g_brg <= 115200 ) {
        highspeed = 0;
        quot = 16;
    } else {
        highspeed = 2;
        quot = 4;
    }

    /* Set divisor DLL and DLH  */             
    divisor   =  uartclk / (quot * g_brg);
    remainder =  uartclk % (quot * g_brg);
          
    if (remainder >= (quot / 2) * g_brg)
        divisor += 1;

    UART_WRITE16(highspeed, UART_HIGHSPEED(g_uart));
    byte = UART_READ32(UART_LCR(g_uart));     /* DLAB start */
    UART_WRITE32((byte | UART_LCR_DLAB), UART_LCR(g_uart));
    UART_WRITE32((divisor & 0x00ff), UART_DLL(g_uart));
    UART_WRITE32(((divisor >> 8)&0x00ff), UART_DLH(g_uart));
    UART_WRITE32(byte, UART_LCR(g_uart));     /* DLAB end */
}

// Shu-Hsin : add this non-blocking getc fucntion
int serial_nonblock_getc(void)
{
 	return (int)UART_READ32(UART_RBR(g_uart));
}

int serial_getc(void)
{
	while (!(UART_READ32(UART_LSR(g_uart)) & UART_LSR_DR)); 	
 	return (int)UART_READ32(UART_RBR(g_uart));
}

void serial_putc(const char c)
{
	while (!(UART_READ32(UART_LSR(g_uart)) & UART_LSR_THRE));
	
	if (c == '\n')
		UART_WRITE32((unsigned int)'\r', UART_THR(g_uart));

	UART_WRITE32((unsigned int)c, UART_THR(g_uart));
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

int serial_tstc(void)
{
	return UART_READ32(UART_LSR(g_uart)) & UART_LSR_DR;
}

int serial_init(void)
{
	return 0;
}

void mt6573_serial_set_current_uart(MT65XX_UART uart_base)
{
	switch(uart_base)
	{	
        case UART1 :
			g_uart = uart_base;
			break;
		case UART4 :
			g_uart = uart_base;
			break;
		default:
			ASSERT(0);
			break;
	}
}

void mt6573_serial_init(void)
{
    #define APMCU_CG_CLR0 0xf7026308
    #define APMCU_CG_SET0 0xf7026304
	gd->bd->bi_baudrate = CONFIG_BAUDRATE;

	#ifdef CFG_META_MODE
	mt6573_serial_set_current_uart(UART1);	
    //PDN_Power_CONA_DOWN(PDN_PERI_UART1, 0);
    UART_SET_BITS(1 << 9, APMCU_CG_CLR0);
	UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */	
	UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
	g_brg = CONFIG_META_BAUDRATE;
    serial_setbrg();         
    #endif

	#ifndef __ENABLE_UART_LOG_SWITCH_FEATURE__
	UART_SET_BITS(1 << 9, APMCU_CG_CLR0);
	mt6573_serial_set_current_uart(UART1);
	UART_WRITE16(0xFE, UART_SCR(g_uart)); // Always Notify MD can use UART1

	//PDN_Power_CONA_DOWN(PDN_PERI_UART4, 0);
	UART_SET_BITS(1 << 12, APMCU_CG_CLR0);
	mt6573_serial_set_current_uart(UART4);
	UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */	
	UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
	g_brg = CONFIG_BAUDRATE;
	serial_setbrg();
	#else
	// 1. Set special pattern to UART1 SCR for MD
	UART_SET_BITS(1 << 9, APMCU_CG_CLR0);
	mt6573_serial_set_current_uart(UART1);
	if(MD_CAN_USE_UART1 == is_uart1_can_be_used_by_MD()){ // Use UART4
		/**
		 * AP use UART4, so MD use UART1, Write SCR register to Notify MD. 
		 * Not equal to 0x55 means MD can use UART1
		 * Specially, for MD side origin design, MD will NOT use uart1 as log port when enter meta mode,
		 * even if uart1's setting is MD can use. 
		 */
		UART_WRITE16(0xFE, UART_SCR(g_uart)); // Notify MD can use UART1 only
	}else{
		/**
		 * AP use UART1 as log port, or, as tool connection port, Write SCR register 0x55
		 * which means MD should not use UART1
		*/ 
		UART_WRITE16(0x55, UART_SCR(g_uart));
	}
	// 2. Set UART log port id
	if(get_uart_log_port_id_in_uboot() == 3){
		// Use UART4 as log port
		UART_SET_BITS(1 << 12, APMCU_CG_CLR0);
		mt6573_serial_set_current_uart(UART4);
	}else{
		// Use UART1 as log port
		UART_SET_BITS(1 << 9, APMCU_CG_CLR0);
		mt6573_serial_set_current_uart(UART1);
	}
	// 3. Configure log port
	UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */	
	UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
	g_brg = CONFIG_BAUDRATE;
	serial_setbrg();
	#endif
}

void mt6516_dbg_init(void)
{
    return;
}

#ifdef __ENABLE_UART_LOG_SWITCH_FEATURE__
#ifdef __LOG_SWITCH_TO_UART1__
const unsigned int using_uart1_as_log_port = 1;
#else
const unsigned int using_uart1_as_log_port = 0;
#endif
static void change_uart_port(char * cmd_line, char new_val)
{
	int i;
	int len;
	char *ptr;
	if(NULL == cmd_line)
		return;

	len = strlen(cmd_line);
	ptr = cmd_line;

	i = strlen("ttyMT");
	if(len < i)
		return;
	len = len-i;

	for(i=0; i<=len; i++)
	{
		if(strncmp(ptr, "ttyMT", 5)==0)
		{
			ptr[5] = new_val; // Find and modify
			break;
		}
		ptr++;
	}
}
void custom_port_in_kernel(BOOTMODE boot_mode, char *command)
{
	unsigned short val;
	/**
	 * There are 3 usage for UART1: AP log, MD log, tool connection.
	 * Specially, if UART1 is assigned to AP, we need consider two case.
	 * Case I, UART1 is used for AP log; case II, UART1 is used for tool connection.
	 * For case II, we should not switch log from UART4 to UART1
	 */
	if(!using_uart1_as_log_port)
		return;

	val = rtc_rdwr_uart_bits(0)&0x3;

	switch(val)
	{
	default:
	case UART1_INVALIDE_VAL:
		/* RTC is not ready or its data not initialized, using default setting */
		val = UART1_DEFAULT_SETTING;
		if(val == UART1_FOR_AP_LOG)
			break; /* Enter here, AP use UART1 output log. Switch it */
		else
			return;
		
	case UART1_FOR_TOOL:   /* Enter here, tool connection, do NOT switch to UART1 */
	case UART1_FOR_MD_LOG: /* Assign UART1 for MD use */
		return;

	case UART1_FOR_AP_LOG:
		break;         /* Enter here, AP use UART1 output log. Switch it */
	}

	/* After all above check, we actually need switch UART4 to UART1, modify command line now */
	change_uart_port(command, '0');
	#if 0
	switch(boot_mode)
	{
	/* These switch only affect the kernel behavior */
	case NORMAL_BOOT:
		change_uart_port(command, '0');
		break;
	case META_BOOT:
		/*No change. Log to uart4. If need to change, call "changePort" here*/
		break;  
	case ADVMETA_BOOT:
		/*No change. Log to uart4. If need to change, call "changePort" here*/
		break; 
	case RECOVERY_BOOT:
		change_uart_port(command, '0');
		break;   
	case FACTORY_BOOT:
		/*No change. Log to uart4. If need to change, call "changePort" here*/
		break;     
	case ATE_FACTORY_BOOT:
		/*No change. Log to uart4. If need to change, call "changePort" here*/
		break;   
	default:
		break;
	}
	#endif
}

int is_uart1_can_be_used_by_MD()
{
	unsigned short val = rtc_rdwr_uart_bits(0)&0x3;

	if(val == UART1_INVALIDE_VAL)
		val = UART1_DEFAULT_SETTING;

	// UART1 log Feature enable && (UART1 assign to AP or AP tool use)
	if (using_uart1_as_log_port) {
		if(val == UART1_FOR_MD_LOG)
			return MD_CAN_USE_UART1; //AP UART4
		else
			return MD_CAN_NOT_USE_UART1; //AP UART1
	}else
		return MD_CAN_USE_UART1; // Default setting, MD can use uart 1
}

int get_uart_log_port_id_in_uboot()
{
	unsigned short val = rtc_rdwr_uart_bits(0)&0x3;

	if(val == UART1_INVALIDE_VAL)
		val = UART1_DEFAULT_SETTING;

	// UART1 log Feature enable && UART1 assign to AP && not tool use
	if(using_uart1_as_log_port&&(val==UART1_FOR_AP_LOG))
		return 0; // UART1
	return 3; // UART4
}
#else
void custom_port_in_kernel(BOOTMODE boot_mode, char *command)
{
	// Dummy function case
}

int is_uart1_can_be_used_by_MD()
{
	return MD_CAN_USE_UART1; // UART4 (0~3)
}

int get_uart_log_port_id_in_uboot()
{
	return 3; // UART4 (0~3)
}
#endif
