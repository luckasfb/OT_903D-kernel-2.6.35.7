
#include <mt6573.h>
#include <mt6573_typedefs.h>
#include <mt6573_gpio.h>
#include <mt6573_key.h>
#include <mt6573_timer.h>

void mt6573_gpio_init(void)
{
    /* KCOL0 ~ KCOL2: input + pull enable + pull up */
    mt_set_gpio_mode(GPIO42, GPIO_MODE_01);
    mt_set_gpio_mode(GPIO41, GPIO_MODE_01);
    mt_set_gpio_mode(GPIO40, GPIO_MODE_01);

    mt_set_gpio_dir(GPIO42, GPIO_DIR_IN);
    mt_set_gpio_dir(GPIO41, GPIO_DIR_IN);
    mt_set_gpio_dir(GPIO40, GPIO_DIR_IN);

    mt_set_gpio_pull_enable(GPIO42, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO41, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO40, GPIO_PULL_ENABLE);

    mt_set_gpio_pull_select(GPIO42, GPIO_PULL_UP);
    mt_set_gpio_pull_select(GPIO41, GPIO_PULL_UP);
    mt_set_gpio_pull_select(GPIO40, GPIO_PULL_UP);

    /* KROW0 ~ KROW1: output + pull disable + pull down */
    mt_set_gpio_mode(GPIO50, GPIO_MODE_01);
    mt_set_gpio_mode(GPIO49, GPIO_MODE_01);
    mt_set_gpio_mode(GPIO48, GPIO_MODE_01);

    mt_set_gpio_dir(GPIO50, GPIO_DIR_OUT);
    mt_set_gpio_dir(GPIO49, GPIO_DIR_OUT);
    mt_set_gpio_dir(GPIO48, GPIO_DIR_OUT);

    mt_set_gpio_pull_enable(GPIO50, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_enable(GPIO49, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_enable(GPIO48, GPIO_PULL_DISABLE);

    mt_set_gpio_pull_select(GPIO50, GPIO_PULL_DOWN);
    mt_set_gpio_pull_select(GPIO49, GPIO_PULL_DOWN);
    mt_set_gpio_pull_select(GPIO48, GPIO_PULL_DOWN);

    /* set other KCOL and KROW to right mode */
    mt_set_gpio_mode(GPIO39, GPIO_MODE_04);
    mt_set_gpio_mode(GPIO38, GPIO_MODE_00);
    mt_set_gpio_mode(GPIO37, GPIO_MODE_04);
    mt_set_gpio_mode(GPIO36, GPIO_MODE_04);
    mt_set_gpio_mode(GPIO35, GPIO_MODE_04);

    mt_set_gpio_mode(GPIO47, GPIO_MODE_00);
    mt_set_gpio_mode(GPIO46, GPIO_MODE_00);
    mt_set_gpio_mode(GPIO45, GPIO_MODE_05);
    mt_set_gpio_mode(GPIO44, GPIO_MODE_00);
    mt_set_gpio_mode(GPIO43, GPIO_MODE_00);

    /* set keypad debounce time and wait for stable state */
    DRV_WriteReg16(KP_DEBOUNCE, 0x400);
    mdelay(33);     /* delay 33 ms */
}
