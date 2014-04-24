
#include "tpd.h"
#define AUX_TP_DEBT           (AUXADC_BASE + 0x0050)
#define AUX_TP_CMD            (AUXADC_BASE + 0x0054)
#define AUX_TP_CON            (AUXADC_BASE + 0x0058)
#define AUX_TP_DATA0          (AUXADC_BASE + 0x005C)
#define AUXADC_CON3           (AUXADC_BASE + 0x000C)
#define AUXADC_CON3_STA_MASK  0x0001
#define TP_DEBT_MASK          0x3fff
#define TP_CMD_PD_MASK        0x0003
#define TP_CMD_PD_YDRV_SH     0x0000
#define TP_CMD_PD_IRQ_SH      0x0001
#define TP_CMD_PD_IRQ         0x0003
#define TP_CMD_SE_DF_MASK     0x0004
#define TP_CMD_DIFFERENTIAL   0x0000
#define TP_CMD_SINGLE_END     0x0004
#define TP_CMD_MODE_MASK      0x0008
#define TP_CMD_MODE_10BIT     0x0000
#define TP_CMD_MODE_8BIT      0x0008
#define TP_CMD_ADDR_MASK      0x0070
#define TP_CMD_ADDR_Y         0x0010
#define TP_CMD_ADDR_Z1        0x0030
#define TP_CMD_ADDR_Z2        0x0040
#define TP_CMD_ADDR_X         0x0050
#define TP_CON_SPL_MASK       0x0001
#define TP_CON_SPL_TRIGGER    0x0001
#define TP_CON_STATUS_MASK    0x0002
#define TP_DAT0_DAT_MASK      0x03ff
#define TP_DEBOUNCE_TIME      (4*32) /* 20ms */
#define TP_AUXADC_POWER_UP    0x0c000c00

#define PMIC_RESERVE_CON0		0xF702FE80
/* DIFFERENTIAL | MODE_10BIT | PD_YDRV_SH */
#define TP_SAMPLE_SETTING     0x0000 

void tpd_adc_init(void);
u16 tpd_read(int position);
u16 tpd_read_adc(u16 pos);

void tpd_adc_init(void) {
    u16 spl_num = 8<<8;
    __raw_writew(TP_DEBOUNCE_TIME, AUX_TP_DEBT);
    __raw_writew(spl_num, PMIC_RESERVE_CON0);
}

void tpd_set_debounce_time(int debounce_time) {
    __raw_writew(debounce_time, AUX_TP_DEBT);
}

void tpd_set_spl_number(int spl_num) {
    __raw_writew(spl_num<<8, PMIC_RESERVE_CON0);
}

u16 tpd_read(int position) {
    switch(position) {
        default:
        case TPD_X:  return tpd_read_adc(TP_CMD_ADDR_X);
        case TPD_Y:  return tpd_read_adc(TP_CMD_ADDR_Y);
        case TPD_Z1: return tpd_read_adc(TP_CMD_ADDR_Z1);
        case TPD_Z2: return tpd_read_adc(TP_CMD_ADDR_Z2);
    } return 0;
}

/* pass command, return sampled data */
u16 tpd_read_adc(u16 pos) {
   __raw_writew(pos | TP_SAMPLE_SETTING, AUX_TP_CMD);
   __raw_writew(TP_CON_SPL_TRIGGER, AUX_TP_CON);
   while(TP_CON_SPL_MASK & __raw_readw(AUX_TP_CON)) { ; }
   return __raw_readw(AUX_TP_DATA0); 
}

u16 tpd_read_status() {
    return __raw_readw(AUX_TP_CON) & 2;
}
