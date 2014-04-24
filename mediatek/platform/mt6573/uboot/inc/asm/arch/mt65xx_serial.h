
#ifndef __MT65XX_SERIAL_H__
#define __MT65XX_SERIAL_H__

#include <asm/arch/mt65xx.h>

typedef enum
{
	UART1 = UART1_BASE,
	UART2 = UART2_BASE,
	UART3 = UART3_BASE,
	UART4 = UART4_BASE
} MT65XX_UART;

extern void mt6573_serial_init(void);
extern int serial_nonblock_getc(void);

#endif /* __MT65XX_SERIAL_H__ */

