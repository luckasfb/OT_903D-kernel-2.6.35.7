
#define REGS_AUDIOIN_BASE	(STMP3XXX_REGS_BASE + 0x4C000)

#define HW_AUDIOIN_CTRL		0x0
#define BM_AUDIOIN_CTRL_RUN	0x00000001
#define BP_AUDIOIN_CTRL_RUN	0
#define BM_AUDIOIN_CTRL_FIFO_ERROR_IRQ_EN	0x00000002
#define BM_AUDIOIN_CTRL_FIFO_OVERFLOW_IRQ	0x00000004
#define BM_AUDIOIN_CTRL_FIFO_UNDERFLOW_IRQ	0x00000008
#define BM_AUDIOIN_CTRL_WORD_LENGTH	0x00000020
#define BM_AUDIOIN_CTRL_CLKGATE	0x40000000
#define BM_AUDIOIN_CTRL_SFTRST	0x80000000

#define HW_AUDIOIN_STAT		0x10

#define HW_AUDIOIN_ADCSRR	0x20

#define HW_AUDIOIN_ADCVOLUME	0x30
#define BM_AUDIOIN_ADCVOLUME_VOLUME_RIGHT	0x000000FF
#define BP_AUDIOIN_ADCVOLUME_VOLUME_RIGHT	0
#define BM_AUDIOIN_ADCVOLUME_VOLUME_LEFT	0x00FF0000
#define BP_AUDIOIN_ADCVOLUME_VOLUME_LEFT	16

#define HW_AUDIOIN_ADCDEBUG	0x40

#define HW_AUDIOIN_ADCVOL	0x50
#define BM_AUDIOIN_ADCVOL_GAIN_RIGHT	0x0000000F
#define BP_AUDIOIN_ADCVOL_GAIN_RIGHT	0
#define BM_AUDIOIN_ADCVOL_SELECT_RIGHT	0x00000030
#define BP_AUDIOIN_ADCVOL_SELECT_RIGHT	4
#define BM_AUDIOIN_ADCVOL_GAIN_LEFT	0x00000F00
#define BP_AUDIOIN_ADCVOL_GAIN_LEFT	8
#define BM_AUDIOIN_ADCVOL_SELECT_LEFT	0x00003000
#define BP_AUDIOIN_ADCVOL_SELECT_LEFT	12
#define BM_AUDIOIN_ADCVOL_MUTE	0x01000000

#define HW_AUDIOIN_MICLINE	0x60

#define HW_AUDIOIN_ANACLKCTRL	0x70
#define BM_AUDIOIN_ANACLKCTRL_CLKGATE	0x80000000

#define HW_AUDIOIN_DATA		0x80