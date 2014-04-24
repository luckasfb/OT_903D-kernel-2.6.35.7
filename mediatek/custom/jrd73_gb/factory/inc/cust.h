
#ifndef FTM_CUST_H
#define FTM_CUST_H

#define FEATURE_FTM_AUDIO
//#define FEATURE_DUMMY_AUDIO

//Customer can enable test item if they need
//#define FEATURE_FTM_PHONE_MIC_HEADSET_LOOPBACK
//#define FEATURE_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
//#define FEATURE_FTM_PHONE_MIC_SPEAKER_LOOPBACK
#define FEATURE_FTM_WAVE_PLAYBACK

#define FEATURE_FTM_BATTERY
#define FEATURE_FTM_BT
#define FEATURE_FTM_FM
//#define FEATURE_FTM_FMTX
#define FEATURE_FTM_GPS
#if defined(MTK_WLAN_SUPPORT)
#define FEATURE_FTM_WIFI
#endif
#define FEATURE_FTM_MAIN_CAMERA
#define FEATURE_FTM_SUB_CAMERA
// #define FEATURE_FTM_FLASH
#define FEATURE_FTM_KEYS
#define FEATURE_FTM_LCD
#define FEATURE_FTM_LED
#define FEATURE_FTM_MEMCARD
//#define FEATURE_FTM_RTC
#define FEATURE_FTM_TOUCH
#define FEATURE_FTM_VIBRATOR
#define FEATURE_FTM_IDLE
#define FEATURE_FTM_CLEARFLASH
#ifdef HAVE_MATV_FEATURE
#define FEATURE_FTM_MATV
#endif
#define FEATURE_FTM_FONT_10x18
#if defined MTK_TVOUT_SUPPORT
#define FEATURE_FTM_TVOUT
#endif
#define FEATURE_FTM_HEADSET
#define HEADSET_BUTTON_DETECTION
//#define FEATURE_FTM_OFN
#define FEATURE_FTM_SIM
#define FEATURE_FTM_PRODUCTINFO

/// #define BATTERY_TYPE_Z3  /// del for jrd factory mode battery resister
#define BATTERY_TYPE_B61UN   /// add this for use adc channel 5 for temperature

#define FEATURE_FTM_USB
#include "cust_font.h"		/* common part */
#include "cust_keys.h"		/* custom part */
#include "cust_lcd.h"		/* custom part */
#include "cust_led.h"		/* custom part */
#include "cust_touch.h"         /* custom part */

#endif /* FTM_CUST_H */
