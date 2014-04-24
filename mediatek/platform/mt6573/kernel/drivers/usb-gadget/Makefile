obj-$(CONFIG_MTK_USB_GADGET) := udc.o udc_log.o

ifeq ($(CONFIG_MTK_AEE_KDUMP),y)
obj-$(CONFIG_MTK_USB_GADGET) += mt6573_usb_phy.o mt6573_usb.o
endif
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power/
