/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include "mt6573.h"
#include "mt6573_typedefs.h"
#include "uart.h"
#include "mt65xx_meta.h"
#include "cust.h"
#include "mt6573_rtc.h"
#include <stdarg.h>

#define UART_LOG_PORT_SWITCH
#define Delay_Count                 324675
#define CONFIG_BAUDRATE             921600

#define UART_BASE(uart)             (uart)

#define UART_RBR(uart)              (UART_BASE(uart)+0x0)       /* Read only */
#define UART_THR(uart)              (UART_BASE(uart)+0x0)       /* Write only */
#define UART_IER(uart)              (UART_BASE(uart)+0x4)
#define UART_IIR(uart)              (UART_BASE(uart)+0x8)       /* Read only */
#define UART_FCR(uart)              (UART_BASE(uart)+0x8)       /* Write only */
#define UART_LCR(uart)              (UART_BASE(uart)+0xc)
#define UART_MCR(uart)              (UART_BASE(uart)+0x10)
#define UART_LSR(uart)              (UART_BASE(uart)+0x14)
#define UART_MSR(uart)              (UART_BASE(uart)+0x18)
#define UART_SCR(uart)              (UART_BASE(uart)+0x1c)
#define UART_DLL(uart)              (UART_BASE(uart)+0x0)       
#define UART_DLH(uart)              (UART_BASE(uart)+0x4)       
#define UART_EFR(uart)              (UART_BASE(uart)+0x8)       
#define UART_XON1(uart)             (UART_BASE(uart)+0x10)      
#define UART_XON2(uart)             (UART_BASE(uart)+0x14)      
#define UART_XOFF1(uart)            (UART_BASE(uart)+0x18)      
#define UART_XOFF2(uart)            (UART_BASE(uart)+0x1c)      
#define UART_AUTOBAUD_EN(uart)      (UART_BASE(uart)+0x20)
#define UART_HIGHSPEED(uart)        (UART_BASE(uart)+0x24)
#define UART_SAMPLE_COUNT(uart)     (UART_BASE(uart)+0x28)
#define UART_SAMPLE_POINT(uart)     (UART_BASE(uart)+0x2c)
#define UART_AUTOBAUD_REG(uart)     (UART_BASE(uart)+0x30)
#define UART_RATE_FIX_AD(uart)      (UART_BASE(uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)  (UART_BASE(uart)+0x38)
#define UART_GUARD(uart)            (UART_BASE(uart)+0x3c)
#define UART_ESCAPE_DAT(uart)       (UART_BASE(uart)+0x40)
#define UART_ESCAPE_EN(uart)        (UART_BASE(uart)+0x44)
#define UART_SLEEP_EN(uart)         (UART_BASE(uart)+0x48)
#define UART_VFIFO_EN(uart)         (UART_BASE(uart)+0x4c)
#define UART_RXTRI_AD(uart)         (UART_BASE(uart)+0x50)


#define UART_SET_BITS(BS,REG)       ((*(volatile U32*)(REG)) |= (U32)(BS))
#define UART_CLR_BITS(BS,REG)       ((*(volatile U32*)(REG)) &= ~((U32)(BS)))
#define UART_WRITE16(VAL, REG)      DRV_WriteReg(REG,VAL)
#define UART_READ32(REG)            DRV_Reg32(REG)
#define UART_WRITE32(VAL, REG)      DRV_WriteReg32(REG,VAL)

volatile unsigned int g_uart = UART4;
unsigned int g_brg; // output uart baudrate

void serial_setbrg (void)
{
    unsigned int byte;
    unsigned int highspeed;
    unsigned int quot, divisor, remainder;
    unsigned int ratefix;
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

int serial_nonblock_getc(void)
{
    return (int)UART_READ32(UART_RBR(g_uart));
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

#define	UART1_FOR_TOOL		(0)
#define	UART1_FOR_AP_LOG	(1)
#define	UART1_FOR_MD_LOG	(2)
#define UART1_INVALIDE_VAL	(3)
#define UART1_DEFAULT_SETTING	(UART1_FOR_TOOL)

unsigned int uart1_as_log_port_accord_dynamic_setting()
{
	#ifdef UART_LOG_PORT_SWITCH
	unsigned short val = rtc_rdwr_uart_bits(0)&0x3;

	switch(val)
	{
	default:
	case UART1_INVALIDE_VAL:
		/* RTC is not ready or its data not initialized, using default setting */
		val = UART1_DEFAULT_SETTING;
		if(val == UART1_FOR_AP_LOG)
			return 1;
		else
			return 0;
		
	case UART1_FOR_TOOL:
		return 0; // Enter here means meta tool will use UART1, can not output log

	case UART1_FOR_AP_LOG:
		return 1; // Enter here, AP can use UART1 as log port

	case UART1_FOR_MD_LOG:
		return 0; // Enter here means UART1 is assigned to MD, so AP can not use UART1 as log port
	}

	#else
	return 0; // We always return 0, if switch feature is not enabled.
	#endif
}

unsigned int uart1_as_tool_port_accord_dynamic_setting()
{
	#ifdef UART_LOG_PORT_SWITCH
	unsigned short val = rtc_rdwr_uart_bits(0)&0x3; 

	switch(val)
	{
	default:
	case UART1_INVALIDE_VAL:
		/* RTC is not ready or its data not initialized, using default setting */
		val = UART1_DEFAULT_SETTING;
		if(val == UART1_FOR_TOOL)
			return 1;
		else
			return 0;
		
	case UART1_FOR_TOOL:
		return 1; // Enter here means meta tool will use UART1, can not output log

	case UART1_FOR_AP_LOG:
		return 0; // Enter here, AP can use UART1 as log port

	case UART1_FOR_MD_LOG:
		return 1; // Enter here means UART1 is assigned to MD,
		          // but MD will not use UART1 port at pre-loader,
		          // so AP can use it as tool port
	}

	#else
	return 0; // We always return 0, if switch feature is not enabled.
	#endif
}

bool pl_uart_init (void)
{
    unsigned int speed, rate, tmp1, tmp2, high_div, remainder;
    unsigned int sample_point, sample_count, divisor;
    int highspeed = 3;
    int baud = 921600;
    int clock = 61440000;

    #define APMCU_CG_CLR0 0xf7026308
    #define APMCU_CG_SET0 0xf7026304

    mt6573_serial_set_current_uart(UART1);
    //PDN_Power_CONA_DOWN(PDN_PERI_UART1, 0);
    UART_SET_BITS(1 << 9, APMCU_CG_CLR0);
    UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */
    UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
    if (CFG_UART_LOG_PORT == UART1)
    {    
        #ifdef UART_LOG_PORT_SWITCH
        /**
         * Enter here means Log will output from UART1 for config setting, 
         * but we need check whether some tools alse use this port, such as meta.
         * If the dynamic setting for this port is tool use, we can not set UART1 as a log port.
         * If the dynamic setting for this port is log use, we can set UART1 port as log port safely.
         */
        if(uart1_as_log_port_accord_dynamic_setting()){
            g_brg = CONFIG_BAUDRATE;
        }else{
            g_brg = CONFIG_META_BAUDRATE; // Dynamic setting is tool use, set meta baud rate
        }
        #else
    	g_brg = CONFIG_BAUDRATE;
    	#endif
    }
    else
    {	
         g_brg = CONFIG_META_BAUDRATE;
    }
    serial_setbrg();


    mt6573_serial_set_current_uart(UART4);
    //PDN_Power_CONA_DOWN(PDN_PERI_UART4, 0);
    UART_SET_BITS(1 << 12, APMCU_CG_CLR0);
    UART_SET_BITS(UART_FCR_FIFO_INIT, UART_FCR(g_uart)); /* clear fifo */
    UART_WRITE16(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
    if (CFG_UART_LOG_PORT == UART4)
    {    
       g_brg = CONFIG_BAUDRATE;
    }
    else
    {
        #ifdef UART_LOG_PORT_SWITCH
    	/**
         * Enter here means Log will output from UART1 for config setting, and we ALSO make
         * the dynamic setting for this port is log use, we have to set UART4 as meta port.
         * If the dynamic setting for UART1 is tool use, we will NOT do log switch operation,
         * and set UART4 as log port.
         */
        if (uart1_as_log_port_accord_dynamic_setting()){
    		g_brg = CONFIG_META_BAUDRATE;
	}else{
		g_brg = CONFIG_BAUDRATE;
	}
        #else
        g_brg = CONFIG_META_BAUDRATE;
        #endif	
    }
    serial_setbrg();

    // default use UART4 as message output port
    if(CFG_UART_LOG_PORT == UART1)
    {
        #ifdef UART_LOG_PORT_SWITCH
	if(uart1_as_log_port_accord_dynamic_setting()){
		g_uart = UART1;
	}else{
		g_uart = UART4;
	}
	#else
	g_uart = UART1;
	#endif
    }
    else
        g_uart = UART4;
    return true;
}

void PutUARTByte (const char c)
{
    while (!(UART_READ32 (UART_LSR(g_uart)) & UART_LSR_THRE))
    {
    }

    if (c == '\n')
        UART_WRITE32 ((unsigned int) '\r', UART_THR(g_uart));

    UART_WRITE32 ((unsigned int) c, UART_THR(g_uart));
}


static void outByte (unsigned char c)
{
    PutUARTByte (c);
}

static void outString (const unsigned char *s)
{
    while (*s)
    {
        if (*s == '\n')
        {
            PutUARTByte ('\r');
        }
        PutUARTByte (*s++);
    }
}

static void outNumDecimal (unsigned long n)
{
    if (n >= 10)
    {
        outNumDecimal (n / 10);
        n %= 10;
    }
    outByte ((unsigned char) (n + '0'));
}

static void outNumHex (unsigned long n, long depth)
{
    if (depth)
    {
        depth--;
    }
    
    if ((n & ~0xf) || depth)
    {
        outNumHex (n >> 4, depth);
        n &= 0xf;
    }
    
    if (n < 10)
    {
        outByte ((unsigned char) (n + '0'));
    }
    else
    {
        outByte ((unsigned char) (n - 10 + 'A'));
    }
}

unsigned int g_enable_debug_msg = TRUE;

void dbg_print (char *sz, ...)
{
    unsigned char c;
    va_list vl;
    
    if (g_enable_debug_msg)
    {
        va_start (vl, sz);
        
        while (*sz)
        {
            c = *sz++;
            switch (c)
            {
                case '%':
                    c = *sz++;
                    switch (c)
                    {
                        case 'x':
                            outNumHex (va_arg (vl, unsigned long), 0);
                            break;
                        case 'B':
                            outNumHex (va_arg (vl, unsigned long), 2);
                            break;
                        case 'H':
                            outNumHex (va_arg (vl, unsigned long), 4);
                            break;
                        case 'X':
                            outNumHex (va_arg (vl, unsigned long), 8);
                            break;
                        case 'd':
                            {
                                long l;
                                
                                l = va_arg (vl, long);
                                if (l < 0)
                                {
                                    outByte ('-');
                                    l = -l;
                                }
                                outNumDecimal ((unsigned long) l);
                            }
                            break;
                        case 'u':
                            outNumDecimal (va_arg (vl, unsigned long));
                            break;
                        case 's':
                            outString ((const unsigned char *)
                            va_arg (vl, char *));
                            break;
                        case '%':
                            outByte ('%');
                            break;
                        case 'c':
                            c = va_arg (vl, int);
                            outByte (c);
                            break;
                        default:
                            outByte (' ');
                            break;
                    }
                    break;
                case '\r':
                    if (*sz == '\n')
                        sz++;
                    c = '\n';
                    // fall through
                case '\n':
                    outByte ('\r');
                    // fall through
                default:
                    outByte (c);
            }
        }
        
        va_end (vl);
    }
}
