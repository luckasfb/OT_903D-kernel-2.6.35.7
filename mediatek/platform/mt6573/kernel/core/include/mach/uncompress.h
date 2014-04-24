
#ifndef __MT6573_UNCOMPRESS_H__
#define __MT6573_UNCOMPRESS_H__

#define MT6573_UART0_LSR (*(volatile unsigned char *)0x70003014)
#define MT6573_UART0_THR (*(volatile unsigned char *)0x70003000)
#define MT6573_UART0_LCR (*(volatile unsigned char *)0x7000300c)
#define MT6573_UART0_DLL (*(volatile unsigned char *)0x70003000)
#define MT6573_UART0_DLH (*(volatile unsigned char *)0x70003004)
#define MT6573_UART0_FCR (*(volatile unsigned char *)0x70003008)
#define MT6573_UART0_MCR (*(volatile unsigned char *)0x70003010)
#define MT6573_UART0_SPEED (*(volatile unsigned char *)0x70003024)
#define MT6573_UART0_PWR (*(volatile unsigned char *)0x70026308)

static void decomp_setup(void)
{
	u32 tmp = 1 << 9;
	
	MT6573_UART0_PWR = tmp; /* power on */

#if defined(CONFIG_MT6573_FPGA)
	MT6573_UART0_LCR = 0x3;
	tmp = MT6573_UART0_LCR;
	MT6573_UART0_LCR = (tmp | 0x80);
        MT6573_UART0_SPEED = 0x2;
	MT6573_UART0_DLL = 0x2b;
	MT6573_UART0_DLH = 0;
	MT6573_UART0_LCR = tmp;
	MT6573_UART0_FCR = 0x0047;
	MT6573_UART0_MCR = (0x1 | 0x2);
	MT6573_UART0_FCR = (0x10 | 0x80 | 0x07);
#elif defined(CONFIG_MT6573_EVB_BOARD)
	MT6573_UART0_LCR = 0x3;
	tmp = MT6573_UART0_LCR;
	MT6573_UART0_LCR = (tmp | 0x80);
        MT6573_UART0_SPEED = 0x0;
	MT6573_UART0_DLL = 0x21;
	MT6573_UART0_DLH = 0;
	MT6573_UART0_LCR = tmp;
	MT6573_UART0_FCR = 0x0047;
	MT6573_UART0_MCR = (0x1 | 0x2);
#endif
}

static inline void putc(char c)
{
    while (!(MT6573_UART0_LSR & 0x20));    
    MT6573_UART0_THR = c;        
}

static inline void flush(void)
{
}


#define arch_decomp_setup() decomp_setup()

#define arch_decomp_wdog()

#endif /* !__MT6573_UNCOMPRESS_H__ */

