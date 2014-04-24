
#ifndef __MT6573_EINT_H__
#define __MT6573_EINT_H__


#define EINT_STA (EINT_BASE + 0x0300)
#define EINT_INTACK (EINT_BASE + 0x0308)
#define EINT_EEVT (EINT_BASE + 0x0310)
#define EINT_MASK (EINT_BASE + 0x0318)
#define EINT_MASK_SET (EINT_BASE + 0x0320)
#define EINT_MASK_CLR (EINT_BASE + 0x0328)
#define EINT_SENS (EINT_BASE + 0x0330)
#define EINT_SENS_SET (EINT_BASE + 0x0338)
#define EINT_SENS_CLR (EINT_BASE + 0x0340)
#define EINT_D0EN (EINT_BASE + 0x0360)
#define EINT_D1EN (EINT_BASE + 0x0368)
#define EINT_D2EN (EINT_BASE + 0x0370)
#define EINT_CON(n) (EINT_BASE + 0x0380 + 4 * (n))


#define EINT_MAX_CHANNEL 32
#define MT65XX_EINT_POL_NEG 0
#define MT65XX_EINT_POL_POS 1


extern void mt65xx_eint_mask(unsigned int eint_num);
extern void mt65xx_eint_unmask(unsigned int eint_num);
extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern int mt65xx_eint_init(void);

#endif  /*!__MT6573_EINT_H__ */

