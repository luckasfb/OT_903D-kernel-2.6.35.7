LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

TARGET_PROVIDES_INIT_RC := true

# add your file here (as target) and place the file in this directory
files := $(TARGET_OUT_KEYLAYOUT)/mt6573-kpd.kl \
         $(TARGET_ROOT_OUT)/init.rc \
         $(TARGET_OUT_ETC)/vold.fstab \
         $(TARGET_OUT_DATA)/http-proxy-cfg \
         $(TARGET_OUT_ETC)/player.cfg \
         $(TARGET_ROOT_OUT_SBIN)/advanced_meta_init \
         $(TARGET_ROOT_OUT_SBIN)/meta_init \
         $(TARGET_ROOT_OUT_SBIN)/meta_tst \
         $(TARGET_ROOT_OUT)/advanced_meta_init.rc \
         $(TARGET_ROOT_OUT)/meta_init.rc \

files += $(strip \
             $(foreach file,$(call wildcard2, $(LOCAL_PATH)/*.xml), \
                  $(addprefix $(PRODUCT_OUT)/system/etc/permissions/,$(notdir $(file))) \
              ) \
          )


ifeq ($(HAVE_AEE_FEATURE),yes)
ifeq ($(PARTIAL_BUILD),true)
files += $(TARGET_ROOT_OUT)/init.aee.customer.rc
else
files += $(TARGET_ROOT_OUT)/init.aee.mtk.rc
endif
endif

# 1. for all files, 
# 2. make them as targets,
# 3. make their filename as prerequisite.
$(foreach file,$(files), $(eval $(call copy-one-file, \
    $(LOCAL_PATH)/$(lastword $(subst /, ,$(file))),$(file))))
ALL_PREBUILT += $(files)

modem_debug_db :=

ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
  modem_debug_db += $(TARGET_OUT_DATA)/mdl/catcher_filter.bin
endif

$(foreach file,$(modem_debug_db), $(eval $(call copy-one-file, \
    $(LOCAL_PATH)/modem/$(lastword $(subst /, ,$(file))),$(file))))
ALL_PREBUILT += $(modem_debug_db)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := mt6573-kpd.kcm
include $(BUILD_KEY_CHAR_MAP)

$(call config-custom-folder,modem:modem)
##### INSTALL MODEM FIRMWARE #####

#include $(CLEAR_VARS)
#LOCAL_MODULE := modem.img
#LOCAL_MODULE_TAGS := user
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
#LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
#include $(BUILD_PREBUILT)

##################################

##### INSTALL DSP FIRMWARE #####

include $(CLEAR_VARS)
LOCAL_MODULE := DSP_ROM
LOCAL_MODULE_TAGS := user
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES := modem/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

$(eval $(call copy-one-file,$(LOCAL_PATH)/modem/DSP_BL,$(PRODUCT_OUT)/DSP_BL))
ALL_PREBUILT += $(PRODUCT_OUT)/DSP_BL
##################################

