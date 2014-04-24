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

#include <mt6573.h>
#include <mt6573_typedefs.h>
#include <mt6573_rtc.h>
#include <mt6573_wdt_hw.h>

#include <cust_rtc.h>

#define RTC_RELPWR_WHEN_XRST    1   /* BBPU = 0 when xreset_rstb goes low */

#define RTC_PWRON_SEC           1   /* must be > 0 */

#define RTC_GPIO_USER_MASK      (((1U << 12) - 1) & 0xff00)

#define rtc_busy_wait()                             \
do {                                                \
    while (DRV_Reg16(RTC_BBPU) & RTC_BBPU_CBUSY);   \
} while (0)

static void rtc_write_trigger(void)
{
    DRV_WriteReg16(RTC_WRTGR, 1);
    rtc_busy_wait();
}

static void rtc_writeif_unlock(void)
{
    DRV_WriteReg16(RTC_PROT, 0x586a);
    rtc_write_trigger();
    DRV_WriteReg16(RTC_PROT, 0x9136);
    rtc_write_trigger();
}

static void rtc_xosc_write(U16 val, bool reload)
{
    U16 bbpu;

    DRV_WriteReg16(RTC_OSC32CON, 0x1a57);
    rtc_busy_wait();
    DRV_WriteReg16(RTC_OSC32CON, 0x2b68);
    rtc_busy_wait();

    DRV_WriteReg16(RTC_OSC32CON, val);
    rtc_busy_wait();

    if (reload) {
        bbpu = DRV_Reg16(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
        DRV_WriteReg16(RTC_BBPU, bbpu);
        rtc_write_trigger();
    }
}

static void rtc_android_init(void)
{
    U16 irqsta;

    DRV_WriteReg16(RTC_IRQ_EN, 0);
    DRV_WriteReg16(RTC_CII_EN, 0);
    DRV_WriteReg16(RTC_AL_MASK, 0);

    DRV_WriteReg16(RTC_AL_YEA, 0);
    DRV_WriteReg16(RTC_AL_MTH, 1);
    DRV_WriteReg16(RTC_AL_DOM, 1);
    DRV_WriteReg16(RTC_AL_DOW, 1);
    DRV_WriteReg16(RTC_AL_HOU, 0);
    DRV_WriteReg16(RTC_AL_MIN, 0);
    DRV_WriteReg16(RTC_AL_SEC, 0);

    DRV_WriteReg16(RTC_PDN1, 0);
    DRV_WriteReg16(RTC_PDN2, 0x0080);   /* set USB Download bit */
    DRV_WriteReg16(RTC_SPAR0, 0xc000);  /* switch to GPI + HP mode when LPD */
    DRV_WriteReg16(RTC_SPAR1, 0);

    DRV_WriteReg16(RTC_DIFF, 0);
    DRV_WriteReg16(RTC_CALI, 0);
    rtc_write_trigger();

    irqsta = DRV_Reg16(RTC_IRQ_STA);    /* read clear */

    /* init time counters after resetting RTC_DIFF and RTC_CALI */
    DRV_WriteReg16(RTC_TC_YEA, RTC_DEFAULT_YEA - RTC_MIN_YEAR);
    DRV_WriteReg16(RTC_TC_MTH, RTC_DEFAULT_MTH);
    DRV_WriteReg16(RTC_TC_DOM, RTC_DEFAULT_DOM);
    DRV_WriteReg16(RTC_TC_DOW, 1);
    DRV_WriteReg16(RTC_TC_HOU, 0);
    DRV_WriteReg16(RTC_TC_MIN, 0);
    DRV_WriteReg16(RTC_TC_SEC, 0);
    rtc_write_trigger();
}

static void rtc_gpio_init(void)
{
    U16 con;

    /* GPI mode and pull enable + pull down */
    con = DRV_Reg16(RTC_CON) & 0x800c;
    con &= ~(RTC_CON_GOE | RTC_CON_GPU);
    con |= RTC_CON_GPEN | RTC_CON_F32KOB;
    DRV_WriteReg16(RTC_CON, con);
    rtc_write_trigger();
}

static void rtc_xosc_init(void)
{
    U16 con;

    con = DRV_Reg16(RTC_OSC32CON);

    /* disable 32K export if there are no RTC_GPIO users */
    if (!(DRV_Reg16(RTC_PDN1) & RTC_GPIO_USER_MASK)) {
        rtc_gpio_init();
        con &= ~RTC_OSC32CON_GPIOCKEN;
    }

    if ((con & 0x001f) != 0xf) {    /* check XOSCCALI */
        con &= ~RTC_OSC32CON_AMPEN;
        rtc_xosc_write(con, false);     /* HP mode + XOSCCALI = old */
        con = (con & ~0x001f) | 0xf;
        rtc_xosc_write(con, false);     /* HP mode + XOSCCALI = 0xf */
    }

    con |= RTC_OSC32CON_AMPEN;
    rtc_xosc_write(con, true);          /* LP mode + XOSCCALI = 0xf */

    DRV_WriteReg16(RTC_SPAR0, 0xc000);  /* switch to GPI + HP mode when LPD */
    rtc_write_trigger();
}

static void rtc_lpd_init(void)
{
    U16 con;

    con = DRV_Reg16(RTC_CON) | RTC_CON_LPEN;
    con &= ~RTC_CON_LPRST;
    DRV_WriteReg16(RTC_CON, con);
    rtc_write_trigger();

    con |= RTC_CON_LPRST;
    DRV_WriteReg16(RTC_CON, con);
    rtc_write_trigger();

    con &= ~RTC_CON_LPRST;
    DRV_WriteReg16(RTC_CON, con);
    rtc_write_trigger();
}

static void rtc_power_inconsistent_init(void)
{
    rtc_gpio_init();
    rtc_xosc_write(0x0007, false);  /* HP mode + XOSCCALI = 0x7 */
    rtc_android_init();

    /* write powerkeys */
    DRV_WriteReg16(RTC_POWERKEY1, RTC_POWERKEY1_KEY);
    DRV_WriteReg16(RTC_POWERKEY2, RTC_POWERKEY2_KEY);
    rtc_write_trigger();

    rtc_lpd_init();

    rtc_xosc_write(0x000f, false);  /* HP mode + XOSCCALI = 0xf */
    rtc_xosc_write(0x010f, true);   /* LP mode + XOSCCALI = 0xf */
}

static void rtc_bbpu_power_down(void)
{
    U16 bbpu;

    /* pull PWRBB low */
    bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_PWREN;
    rtc_writeif_unlock();
    DRV_WriteReg16(RTC_BBPU, bbpu);
    rtc_write_trigger();
}

void rtc_bbpu_power_on(void)
{
    U16 bbpu;

    /* pull PWRBB high */
#if RTC_RELPWR_WHEN_XRST
    bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_BBPU | RTC_BBPU_PWREN;
#else
    bbpu = RTC_BBPU_KEY | RTC_BBPU_BBPU | RTC_BBPU_PWREN;
#endif
    DRV_WriteReg16(RTC_BBPU, bbpu);
    rtc_write_trigger();
}

void rtc_mark_recovery_bits(U16 val)
{
    U16 pdn1;

    pdn1 = DRV_Reg16(RTC_PDN1) & ~0x0030;
    pdn1 |= (val & 0x0030);
    DRV_WriteReg16(RTC_PDN1, pdn1);
    rtc_write_trigger();
}

void rtc_mark_bypass_pwrkey(void)
{
    U16 pdn1;

    pdn1 = DRV_Reg16(RTC_PDN1) | 0x0040;
    DRV_WriteReg16(RTC_PDN1, pdn1);
    rtc_write_trigger();
}

static void rtc_clean_mark(void)
{
    U16 pdn1, pdn2;

    pdn1 = DRV_Reg16(RTC_PDN1) & ~0x0040;
    pdn2 = DRV_Reg16(RTC_PDN2) & ~0x0010;
    DRV_WriteReg16(RTC_PDN1, pdn1);
    DRV_WriteReg16(RTC_PDN2, pdn2);
    rtc_write_trigger();
}

bool rtc_dl_bit_check(bool clear)
{
    U16 pdn2;

    pdn2 = DRV_Reg16(RTC_PDN2);
    if (clear) {
        DRV_WriteReg16(RTC_PDN2, pdn2 & ~0x0080);
        rtc_write_trigger();
    }
    return !!(pdn2 & 0x0080);
}

U16 rtc_rdwr_uart_bits(U16 *val)
{
    U16 pdn2;

    if (DRV_Reg16(RTC_CON) & RTC_CON_LPSTA_RAW)
        return 3;   /* UART bits are invalid due to RTC uninit */

    if (val) {
        pdn2 = DRV_Reg16(RTC_PDN2) & ~0x0060;
        pdn2 |= (*val & 0x0003) << 5;
        DRV_WriteReg16(RTC_PDN2, pdn2);
        rtc_write_trigger();
    }

    return (DRV_Reg16(RTC_PDN2) & 0x0060) >> 5;
}

bool rtc_boot_check(void)
{
    U16 irqsta, pdn1, pdn2, spar1;

    dbg_print("bbpu = 0x%x, con = 0x%x\n",
               DRV_Reg16(RTC_BBPU), DRV_Reg16(RTC_CON));

    /* normally HW reload is done in BROM but check again here */
    while (DRV_Reg16(RTC_BBPU) & RTC_BBPU_CBUSY);

    rtc_writeif_unlock();

    if (DRV_Reg16(RTC_POWERKEY1) != RTC_POWERKEY1_KEY ||
        DRV_Reg16(RTC_POWERKEY2) != RTC_POWERKEY2_KEY ||
        (DRV_Reg16(RTC_CON) & RTC_CON_LPSTA_RAW)) {
        dbg_print("powerkey1 = 0x%x, powerkey2 = 0x%x\n",
                  DRV_Reg16(RTC_POWERKEY1), DRV_Reg16(RTC_POWERKEY2));
        rtc_power_inconsistent_init();
    } else {
        dbg_print("xosc = 0x%x, spar0 = 0x%x\n",
                  DRV_Reg16(RTC_OSC32CON), DRV_Reg16(RTC_SPAR0));
        rtc_xosc_init();
        rtc_clean_mark();
    }

    irqsta = DRV_Reg16(RTC_IRQ_STA);    /* read clear */
    pdn1 = DRV_Reg16(RTC_PDN1);
    pdn2 = DRV_Reg16(RTC_PDN2);
    spar1 = DRV_Reg16(RTC_SPAR1);
    dbg_print("irqsta = 0x%x, pdn1 = 0x%x, pdn2 = 0x%x, spar1 = 0x%x\n",
              irqsta, pdn1, pdn2, spar1);

    if (irqsta & RTC_IRQ_STA_AL) {
#if RTC_RELPWR_WHEN_XRST
        /* set AUTO bit because AUTO = 0 when PWREN = 1 and alarm occurs */
        U16 bbpu = DRV_Reg16(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_AUTO;
        DRV_WriteReg16(RTC_BBPU, bbpu);
        rtc_write_trigger();
#endif

        if (pdn1 & 0x0080) {    /* power-on time is available */
            U16 now_sec, now_min, now_hou, now_dom, now_mth;
            U16 irqen, min, hou, dom, mth;

            now_sec = DRV_Reg16(RTC_TC_SEC);
            now_min = DRV_Reg16(RTC_TC_MIN);
            now_hou = DRV_Reg16(RTC_TC_HOU);
            now_dom = DRV_Reg16(RTC_TC_DOM);
            now_mth = DRV_Reg16(RTC_TC_MTH);
            if (DRV_Reg16(RTC_TC_SEC) < now_sec) {  /* SEC has carried */
                now_sec = DRV_Reg16(RTC_TC_SEC);
                now_min = DRV_Reg16(RTC_TC_MIN);
                now_hou = DRV_Reg16(RTC_TC_HOU);
                now_dom = DRV_Reg16(RTC_TC_DOM);
                now_mth = DRV_Reg16(RTC_TC_MTH);
            }
            dbg_print("now = %d/%d %d:%d:%d\n",
                      now_mth, now_dom, now_hou, now_min, now_sec);

            min = spar1 & 0x003f;
            hou = (spar1 & 0x07c0) >> 6;
            dom = (spar1 & 0xf800) >> 11;
            mth = pdn2 & 0x000f;
            dbg_print("power-on = %d/%d %d:%d:%d\n",
                      mth, dom, hou, min, RTC_PWRON_SEC);

            if (now_mth == mth && now_dom == dom &&
                now_hou == hou && now_min == min &&
                now_sec >= (RTC_PWRON_SEC - 1) && now_sec <= (RTC_PWRON_SEC + 4)) {
                pdn1 &= ~(0x0080 | 0x0020);
                DRV_WriteReg16(RTC_PDN1, pdn1 | 0x0040);
                DRV_WriteReg16(RTC_PDN2, pdn2 | 0x0010);
                rtc_write_trigger();
                return true;
            }

            /* set power-on alarm when power-on time is available */
            DRV_WriteReg16(RTC_AL_MTH, mth);
            DRV_WriteReg16(RTC_AL_DOM, dom);
            DRV_WriteReg16(RTC_AL_HOU, hou);
            DRV_WriteReg16(RTC_AL_MIN, min);
            DRV_WriteReg16(RTC_AL_SEC, RTC_PWRON_SEC);
            DRV_WriteReg16(RTC_AL_MASK, 0x0050);    /* mask YEA and DOW */
            rtc_write_trigger();
            irqen = DRV_Reg16(RTC_IRQ_EN) | RTC_IRQ_EN_ONESHOT_AL;
            DRV_WriteReg16(RTC_IRQ_EN, irqen);
            rtc_write_trigger();
        }
    }

    if ((pdn1 & 0x0030) == 0x0010) {    /* factory data reset */
        /* keep bit 4 set until rtc_boot_check() in U-Boot */
        return true;
    }

    if ((pdn1 & 0x0030) == 0x0020) {    /* SW reset */
        pdn1 = (pdn1 & ~0x0020) | 0x0040;
        DRV_WriteReg16(RTC_PDN1, pdn1);
        rtc_write_trigger();
        return true;
    }

    if (WDT_boot_check()) {     /* WDT reset */
        DRV_WriteReg16(RTC_PDN1, pdn1 | 0x0040);
        rtc_write_trigger();
        return true;
    }

    return false;
}

void pl_power_off(void)
{
    dbg_print("pl_power_off\n");

    rtc_bbpu_power_down();

    while (1);
}

//MODULE_AUTHOR("Terry Chang <terry.chang@mediatek.com>");
//MODULE_DESCRIPTION("MT6573 RTC Preloader Driver v1.3");
