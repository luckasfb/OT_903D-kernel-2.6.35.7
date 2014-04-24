obj-$(CONFIG_MT592X_SDIO_CLNT) += mt_sd_clnt.o
mt_sd_clnt-y += platform.o hif.o 

obj-$(CONFIG_MT592X) += wlan/

ifeq ($(CONFIG_MT592X_EEPROM),y)
ccflags-y += -DBUILD_USE_EEPROM=0
else
ccflags-y += -DBUILD_USE_EEPROM=0
endif
