ifeq ($(TARGET_BUILD_MMITEST), true)
EXTRA_CFLAGS += -DBATT_ID_CHECK_SUPPORT=0
else
EXTRA_CFLAGS += -DBATT_ID_CHECK_SUPPORT=1
endif
obj-$(CONFIG_MTK_PMU)           += upmu_common.o
obj-$(CONFIG_MTK_PMU)           += pmu6573.o
obj-$(CONFIG_MTK_FGADC)         += mt6573_fuel_gauge.o
obj-$(CONFIG_MTK_SMART_BATTERY) += mt6573_battery.o

ccflags-y += -I$(MTK_PATH_SOURCE)/drivers/power \
             -I$(MTK_PATH_PLATFORM)/drivers/usb-gadget \
             -I$(MTK_PATH_PLATFORM)/drivers/power
