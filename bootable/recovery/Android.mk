# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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


ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH := $(call my-dir)
#wschen
WITH_BACKUP_RESTORE := true

ifeq ($(MTK_GOOGLEOTA_SUPPORT),yes)
ifeq ($(WITH_BACKUP_RESTORE),true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    check_otapackage.c
LOCAL_MODULE := check_ota
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := user
LOCAL_STATIC_LIBRARIES := libbkrt libz libminzip libunz libcutils libc
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_UNSTRIPPED_PATH := $(TARGET_OUT_EXECUTABLES_UNSTRIPPED)
LOCAL_MODULE_CLASS := EXECUTABLES
include $(BUILD_EXECUTABLE)
endif
endif

include $(CLEAR_VARS)
commands_recovery_local_path := $(LOCAL_PATH)

##########################################
# Feature option
##########################################
# SPECIAL_FACTORY_RESET will backup /data/app when do factory reset if SD is existed
ifeq ($(MTK_SPECIAL_FACTORY_RESET),yes)
    SPECIAL_FACTORY_RESET := true
else
    SPECIAL_FACTORY_RESET := false
endif

# Currently, not support secure boot update on EMMC
ifneq ($(MTK_EMMC_SUPPORT),yes)
    ifeq ($(MTK_SEC_BOOT),ATTR_SBOOT_DISABLE)
        WITH_SBOOT_UPDATE := false
    else
        WITH_SBOOT_UPDATE := true
    endif    
else
    WITH_SBOOT_UPDATE := false
endif

WITH_SD_HOTPLUG := false

ifdef MTK_FOTA_SUPPORT
    ifeq ($(MTK_FOTA_SUPPORT),yes)
        WITH_FOTA := true
    else
        WITH_FOTA := false
    endif
else
    WITH_FOTA := false
endif

##########################################
# Specify source files
##########################################

LOCAL_SRC_FILES := \
    recovery.c \
    bootloader.c \
    install.c \
    roots.c \
    ui.c \
    verifier.c \
    encryptedfs_provisioning.c

ifeq ($(WITH_FOTA),true)
LOCAL_SRC_FILES += \
    fota/fota.c \
    fota/fota_fs.c \
    fota/fota_common.c
endif

ifeq ($(WITH_SBOOT_UPDATE),true)
LOCAL_SRC_FILES += \
    sec/sec.c
endif

##########################################
# Module initialization
##########################################

LOCAL_MODULE := recovery

LOCAL_FORCE_STATIC_EXECUTABLE := true

RECOVERY_API_VERSION := 3
LOCAL_CFLAGS += -DRECOVERY_API_VERSION=$(RECOVERY_API_VERSION)

#wschen
ifeq ($(WITH_BACKUP_RESTORE),true)
LOCAL_CFLAGS += -DSUPPORT_DATA_BACKUP_RESTORE
endif

ifeq ($(SPECIAL_FACTORY_RESET),true)
LOCAL_CFLAGS += -DSPECIAL_FACTORY_RESET
endif

ifeq ($(WITH_FOTA),true)
LOCAL_CFLAGS += -DSUPPORT_FOTA
LOCAL_CFLAGS += -fno-short-enums
endif

ifeq ($(WITH_SBOOT_UPDATE),true)
LOCAL_CFLAGS += -DSUPPORT_SBOOT_UPDATE
endif

ifeq ($(WITH_SD_HOTPLUG), true)
LOCAL_CFLAGS += -DSUPPORT_SD_HOTPLUG=1
else
LOCAL_CFLAGS += -DSUPPORT_SD_HOTPLUG=0
endif

ifeq ($(WITH_FOTA),true)
LOCAL_CFLAGS += -DFOTA_FIRST
# LOCAL_CFLAGS += -DMOTA_FIRST
endif

# This binary is in the recovery ramdisk, which is otherwise a copy of root.
# It gets copied there in config/Makefile.  LOCAL_MODULE_TAGS suppresses
# a (redundant) copy of the binary in /system/bin for user builds.
# TODO: Build the ramdisk image in a more principled way.

LOCAL_MODULE_TAGS := eng

##########################################
# Static library
##########################################

LOCAL_STATIC_LIBRARIES :=
#wschen
ifeq ($(WITH_BACKUP_RESTORE),true)
LOCAL_STATIC_LIBRARIES += libbkrt
endif
ifeq ($(TARGET_RECOVERY_UI_LIB),)
  LOCAL_SRC_FILES += default_recovery_ui.c
else
  LOCAL_STATIC_LIBRARIES += $(TARGET_RECOVERY_UI_LIB)
endif
LOCAL_STATIC_LIBRARIES += libext4_utils libz
LOCAL_STATIC_LIBRARIES += libminzip libunz libmtdutils libmincrypt
LOCAL_STATIC_LIBRARIES += libminui libpixelflinger_static libpng libcutils
LOCAL_STATIC_LIBRARIES += libstdc++ libc
ifeq ($(WITH_FOTA),true)
LOCAL_STATIC_LIBRARIES += upi
endif

ifeq ($(WITH_SBOOT_UPDATE),true)
LOCAL_STATIC_LIBRARIES += libsbup
endif

LOCAL_C_INCLUDES += \
	system/extras/ext4_utils \
	mediatek/custom/$(TARGET_PRODUCT)/recovery/inc \
	$(LOCAL_CUST_INC_PATH) \
	$(LOCAL_PATH)/fota/include \
	$(MTK_ROOT_CUSTOM_OUT)/kernel/dct
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := verifier_test.c verifier.c
LOCAL_MODULE := verifier_test
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := tests
LOCAL_STATIC_LIBRARIES := libmincrypt libcutils libstdc++ libc
include $(BUILD_EXECUTABLE)

##########################################
# Static library - WITH_FOTA
##########################################

ifeq ($(WITH_FOTA),true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := fota/upi.a
include $(BUILD_MULTI_PREBUILT)
endif

##########################################
# Static library - WITH_SBOOT_UPDATE
##########################################

ifeq ($(WITH_SBOOT_UPDATE),true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS += sec/libsbup.a
include $(BUILD_MULTI_PREBUILT)
endif

##########################################
# Static library - WITH_BACKUP_RESTORE
##########################################

ifeq ($(WITH_BACKUP_RESTORE),true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS += libbkrt.a
include $(BUILD_MULTI_PREBUILT)
endif

#############################################################################
#include $(CLEAR_VARS)
#LOCAL_C_INCLUDES:= system/extras/ext4_utils $(LOCAL_CUST_INC_PATH) $(LOCAL_PATH)/include
#LOCAL_SRC_FILES := fota_main.c \
#                   fota.c \
#                   fota_common.c \
#                   fota_fs.c \
#                   roots.c
#LOCAL_CFLAGS += -fshort-enums -g
#LOCAL_CFLAGS += -DSUPPORT_FOTA
#LOCAL_MODULE := fota1
#LOCAL_MODULE_TAGS := eng
#LOCAL_STATIC_LIBRARIES :=  libext4_utils libmincrypt libcutils libstdc++ libc libmtdutils upi
#LOCAL_SHARED_LIBRARIES := libcutils libc
#include $(BUILD_EXECUTABLE)

#############################################################################

include $(commands_recovery_local_path)/minui/Android.mk
include $(commands_recovery_local_path)/minzip/Android.mk
include $(commands_recovery_local_path)/mtdutils/Android.mk
include $(commands_recovery_local_path)/tools/Android.mk
include $(commands_recovery_local_path)/edify/Android.mk
include $(commands_recovery_local_path)/updater/Android.mk
include $(commands_recovery_local_path)/applypatch/Android.mk
commands_recovery_local_path :=

endif   # TARGET_ARCH == arm
endif    # !TARGET_SIMULATOR

