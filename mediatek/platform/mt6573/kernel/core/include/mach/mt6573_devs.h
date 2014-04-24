
#ifndef __MT6573_DEVS_H__
#define __MT6573_DEVS_H__

#include <board-custom.h>
#include <mach/board.h>

#define CFG_DEV_UART1
#define CFG_DEV_UART2
#define CFG_DEV_UART3
#define CFG_DEV_UART4


#define MT6573_UART_SIZE 0x100


extern int mt6573_board_init(void);

/* FIXME */
extern struct mt6573_nand_host_hw mt6573_nand_hw;

#endif  /* !__MT6573_DEVS_H__ */

