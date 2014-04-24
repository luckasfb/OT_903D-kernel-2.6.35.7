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



#ifndef _PMU_SW_H
#define _PMU_SW_H

#define  CHR_TICKLE                     0x1000
#define  CHR_PRE                        0x1001
#define  CHR_CC                         0x1002
#define  CHR_CV                         0x1003
#define  CHR_BATFULL                    0x1004
#define  CHR_ERROR                      0x1005

#define  PMU_CHARGEOFF                  0
#define  PMU_CHARGEON                   1

#define CHARGER_OVER_VOL                1
#define BATTERY_UNDER_VOL               2
#define BATTERY_OVER_TEMP               3

#define OV_431V                         0
#define OV_441V                         1


#define FULL_BATTERY_TICKLE_CURRENT     50
typedef struct
{
    BOOL bat_exist;
    BOOL bat_full;
    BOOL bat_low;
    UINT16 bat_charging_state;
    UINT16 bat_vol;             //BATSENSE
    BOOL charger_exist;
    UINT16 pre_charging_current;
    UINT16 charging_current;
    UINT16 charger_vol;         //CHARIN
    UINT8 charger_protect_status;
    UINT16 ISENSE;              //ISENSE
    UINT16 ICharging;
    INT16 temperature;
    UINT32 total_charging_time;
    UINT32 CV_charging_time;
    UINT32 PRE_charging_time;
    UINT32 charger_type;
    UINT32 PWR_SRC;
    //CEDEVICE_POWER_STATE Bat_pwr_state ;

} PMU_ChargerStruct;

typedef enum {
    CHARGER_UNKNOWN = 0,
    STANDARD_HOST,          // USB : 450mA
    CHARGING_HOST,
    NONSTANDARD_CHARGER,    
    STANDARD_CHARGER,       
} CHARGER_TYPE;

/*Function*/
extern void mt6516_pmu_init (void);
extern CHARGER_TYPE hw_charger_type_detection(void);

#endif  /*_PMU_SW_H*/
