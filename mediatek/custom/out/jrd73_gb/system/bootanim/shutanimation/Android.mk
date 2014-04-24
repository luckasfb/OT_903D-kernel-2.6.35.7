LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifdef OPTR_SPEC_SEG_DEF
    ifneq ($(OPTR_SPEC_SEG_DEF),NONE)
        OPTR_LIST := $(subst _, ,$(OPTR_SPEC_SEG_DEF))
        OPTR := $(word 1,$(OPTR_LIST))

        CUSTOMLIST := OP01 OP02

        ifneq ($(strip $(OPTR)), )
            ifneq ($(filter $(strip $(OPTR)), $(CUSTOMLIST)), )
                LOCAL_PATH := $(LOCAL_PATH)/$(OPTR)

                ifeq ($(LCM_HEIGHT), 800)
                    LOCAL_PATH := $(LOCAL_PATH)/WVGA
                else
                    LOCAL_PATH := $(LOCAL_PATH)/HVGA
                endif

                LOCAL_MODULE := shutanimation.zip
                LOCAL_MODULE_TAGS := user
                LOCAL_MODULE_CLASS := media
                LOCAL_MODULE_PATH := $(TARGET_OUT)/media
                LOCAL_SRC_FILES := $(LOCAL_MODULE)
                include $(BUILD_PREBUILT)
            endif
        endif
    else
        LOCAL_PATH := $(LOCAL_PATH)/default

        ifeq ($(LCM_HEIGHT), 800)
            LOCAL_PATH := $(LOCAL_PATH)/WVGA
        else
            ifeq ($(LCM_HEIGHT), 480)
                LOCAL_PATH := $(LOCAL_PATH)/HVGA
            else
                LOCAL_PATH := $(LOCAL_PATH)/QVGA
            endif
        endif

        LOCAL_MODULE := shutanimation.zip
        LOCAL_MODULE_TAGS := user
        LOCAL_MODULE_CLASS := media
        LOCAL_MODULE_PATH := $(TARGET_OUT)/media
        LOCAL_SRC_FILES := $(LOCAL_MODULE)
        include $(BUILD_PREBUILT)
    endif
endif