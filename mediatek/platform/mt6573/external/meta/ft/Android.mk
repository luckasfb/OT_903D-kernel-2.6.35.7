# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
PLATFORM_PATH := $(MTK_PATH_PLATFORM)/external/meta
LOCAL_SHARED_LIBRARIES := libc libnvram libcutils libnetutils libmedia libhardware_legacy libfile_op libhwm libacdk libaudioflinger libaudiocompensationfilter libdl libutils

ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libmatv_cust
endif

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
                    $(MTK_PATH_SOURCE)/external/meta/common/inc \
                    $(PLATFORM_PATH)/include \
                    $(PLATFORM_PATH)/ft \
                    $(PLATFORM_PATH)/Meta_APEditor \
                    $(PLATFORM_PATH)/sdcard \
                    $(PLATFORM_PATH)/gpio \
                    $(PLATFORM_PATH)/cpu \
                    $(PLATFORM_PATH)/keypadbk \
                    $(PLATFORM_PATH)/lcd \
                    $(PLATFORM_PATH)/LCDBK \
                    $(PLATFORM_PATH)/ADC \
                    $(PLATFORM_PATH)/BatteryIC \
                    $(PLATFORM_PATH)/pmic \
                    $(PLATFORM_PATH)/vibrator \
                    $(PLATFORM_PATH)/gsensor \
                    $(PLATFORM_PATH)/meta_lock \
                    $(PLATFORM_PATH)/cameratool/CCAP \
                    $(MTK_PATH_SOURCE)/external/mhal/src/core/scenario/6573/cameradebug/inc \
                    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
                    $(MTK_PATH_SOURCE)/external/mhal/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
                    $(MTK_PATH_CUSTOM)/hal/inc \
                    $(PLATFORM_PATH)/Audio \
                    $(PLATFORM_PATH)/msensor \
                    $(PLATFORM_PATH)/alsps \
                    $(PLATFORM_PATH)/gyroscope\
                    $(PLATFORM_PATH)/touch

ifeq ($(MTK_WLAN_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/wifi
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/gps
endif

ifeq ($(MTK_FM_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/fm
endif

ifeq ($(MTK_BT_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/bluetooth
endif

ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/matv \
                    $(MTK_PATH_SOURCE)/external/matvctrl
endif

LOCAL_SRC_FILES := ft_main.cpp \
                   ft_fnc.cpp

LOCAL_STATIC_LIBRARIES := libmeta_apeditor \
                          libmeta_gpio \
                          libmeta_cpu \
                          libmeta_keypadbk \
                          libmeta_lcd \
                          libmeta_lcdbk \
                          libmeta_sdcard \
                          libmeta_adc \
                          libmeta_battery \
                          libmeta_pmic \
                          libmeta_vibrator \
                          libmeta_gsensor \
                          libmeta_lock \
                          libccap \
                          libmeta_audio \
                          libmeta_msensor \
                          libmeta_alsps \
                          libmeta_gyroscope\
                          libmeta_touch
ifeq ($(MTK_WLAN_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES +=libmeta_wifi 
LOCAL_CFLAGS += \
    -DFT_WIFI_FEATURE
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libmeta_gps 
LOCAL_CFLAGS += \
    -DFT_GPS_FEATURE
endif

ifeq ($(MTK_FM_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libmeta_fm   
LOCAL_CFLAGS += \
    -DFT_FM_FEATURE
endif

ifeq ($(MTK_BT_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libmeta_bluetooth 
LOCAL_CFLAGS += \
    -DFT_BT_FEATURE
endif                         
                         
ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_STATIC_LIBRARIES += libmeta_matv
LOCAL_CFLAGS += \
    -DFT_MATV_FEATURE
endif                     

LOCAL_MODULE := libft
include $(BUILD_SHARED_LIBRARY)

