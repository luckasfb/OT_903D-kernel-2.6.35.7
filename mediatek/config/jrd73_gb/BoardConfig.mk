TARGET_CPU_ABI := armeabi

USE_CAMERA_STUB := true

TARGET_NO_FACTORYIMAGE := true

HAVE_HTC_AUDIO_DRIVER := true

#BOARD_USES_GENERIC_AUDIO := true
BOARD_USES_YUSU_AUDIO := true

# MTK, Infinity, 20090720, Add WiFi {
ifeq ($(MTK_WLAN_SUPPORT), yes)
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
HAVE_CUSTOM_WIFI_DRIVER_2 := true
HAVE_INTERNAL_WPA_SUPPLICANT_CONF := true
HAVE_CUSTOM_WIFI_HAL := mediatek
WPA_SUPPLICANT_VERSION := VER_0_6_X
endif
# MTK, Infinity, 20090720, Add WiFi }

BOARD_EGL_CFG := $(BOARD_CONFIG_DIR)/egl.cfg

# MTK, Baochu Wang, 20101130, Add A-GPS {
ifeq ($(MTK_AGPS_APP), yes)
   BOARD_AGPS_SUPL_LIBRARIES := true
else
   BOARD_AGPS_SUPL_LIBRARIES := false
endif
# MTK, Baochu Wang, 20101130, Add A-GPS }

ifeq ($(MTK_GPS_SUPPORT), yes)
    BOARD_GPS_LIBRARIES := true
else
    BOARD_GPS_LIBRARIES := false
endif

TARGET_KMODULES := true

ifeq ($(strip $(MTK_NAND_PAGE_SIZE)), 4K)
  BOARD_NAND_PAGE_SIZE := 4096 -s 128
else
  BOARD_NAND_PAGE_SIZE := 2048 -s 64   # default 2K
endif

# include all config files
include $(BOARD_CONFIG_DIR)/configs/*.mk

