

#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
//#include <cust_key.h>
#include <cust_kpd.h>
#include <asm/arch/mt6573_key.h>

#define GPIO_DIN_BASE	(GPIO_BASE + 0x0a00)

bool mt6573_detect_key(unsigned short key)	/* key: HW keycode */
{
	unsigned short idx, bit, din;

	if (key >= KPD_NUM_KEYS)
		return false;

	if (key % 9 == 8)
		key = 8;

#if 0 /* KPD_PWRKEY_USE_EINT */
	if (key == 8) {		/* Power key */
		idx = KPD_PWRKEY_EINT_GPIO / 16;
		bit = KPD_PWRKEY_EINT_GPIO % 16;

		din = DRV_Reg16(GPIO_DIN_BASE + (idx << 4)) & (1U << bit);
		din >>= bit;
		if (din == KPD_PWRKEY_GPIO_DIN) {
			printf("power key is pressed\n");
			return true;
		}
		return false;
	}
#endif

	idx = key / 16;
	bit = key % 16;

	din = DRV_Reg16(KP_MEM1 + (idx << 2)) & (1U << bit);
	if (!din) {
		//printf("key %d is pressed\n", key);
		return true;
	}
	return false;
}
