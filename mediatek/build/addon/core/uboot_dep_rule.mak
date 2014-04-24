ifeq (emulator,$(strip $(MTK_PROJECT)))
  ifeq (yes,$(strip $(BUILD_UBOOT)))
    $(call dep-err-common, PLEASE set BUILD_UBOOT as "no" when building emulator)
  endif
endif
