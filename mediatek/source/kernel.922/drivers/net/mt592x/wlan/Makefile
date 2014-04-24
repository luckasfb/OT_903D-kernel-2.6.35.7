obj-$(CONFIG_MT592X) += wlan.o

always := $(shell cp $(src)/gl_sec.o.new $(obj)/gl_sec.o)
always := $(touch $(obj)/gl_sec.o)

# COMMON_OBJS
wlan-objs                   := wlan_lib.o wlan_oid.o timer.o dump.o util.o wlan_oid_rftest.o

# MGMT_OBJS
wlan-objs                   += arb_fsm.o assoc.o auth.o bss.o ibss.o join_fsm.o mgt_buf.o mib.o scan.o scan_fsm.o
wlan-objs                   += sec_fsm.o sta_record.o parse.o privacy.o pwr_mgt_fsm.o qos_enhance.o rate.o
wlan-objs                   += roaming_fsm.o rsn.o rx.o tkip_mic.o tx.o rftest.o statistics.o domain.o

# NIC_OBJS
wlan-objs                   += nic.o nic_tx.o nic_rx.o nic_eeprom.o nic_privacy.o nic_rate.o nic_hw_scan.o nic_pwr_mgt.o 
wlan-objs                   += nic_hw_timer.o nic_eeprom.o

# OS_OBJS
wlan-objs                   += gl_init.o gl_kal.o gl_wext.o gl_wext_priv.o
wlan-$(CONFIG_MT592X_PROC)  += gl_proc.o
wlan-$(CONFIG_MT592X_PTA)   += nic_pta.o pta_fsm.o hal_pta.o

# CHIP_OBJS
wlan-objs                   += hal.o hal_wlantable.o hal_pwr_mgt.o hal_hw_scan.o mt592x.o
wlan-$(CONFIG_MT5921)       += mt5921.o bb_mt5921.o rf_mt5921.o
wlan-$(CONFIG_MT5922)       += mt5922.o bb_mt5911.o rf_al2236.o

#WAPI_OBJS
ifeq ($(MTK_WAPI_SUPPORT), yes)
wlan-objs                   += sms4c.o wpi_pcrypt.o wapi.o wpi.o gl_sec.o
endif



# HOST_IF
#wlan-$(CONFIG_MT592X_SDIO)  += sdio.o
wlan-y  += sdio.o


# compile option
ccflags-y += -DLINUX -Wimplicit -Wunused
ccflags-y += -DBUILD_META=0 -DMT5911_BB=0 -DMT5921_BB=0 -DBUILD_QA_DBG=0 -DSUPPORT_NEW_PTA_BOARD=1

ccflags-$(CONFIG_MT5921) += -DMT5921 -DMODULE_PREFIX="MT5921"
ccflags-$(CONFIG_MT5922) += -DMT5922 -DMODULE_PREFIX="MT5922"

ccflags-$(CONFIG_MT592X_SDIO) += -D_HIF_SDIO
ccflags-$(CONFIG_MT592X_EHPI) += -D_HIF_EHPI

ifeq ($(CONFIG_MT592X_DEBUG), y)
ccflags-y += -DDBG=1
else
ccflags-y += -DDBG=0
endif

ifeq ($(CONFIG_MT592X_PROC), y)
ccflags-y += -DWLAN_INCLUDE_PROC
endif

ifeq ($(CONFIG_MT592X_IBSS), y)
ccflags-y += -DPSEUDO_IBSS=1
endif

ifeq ($(CONFIG_MT592X_SW_ENC), y)
ccflags-y += -DBUILD_SW_ENCRYPT=1
endif

ifeq ($(CONFIG_MT592X_WPS), y)
ccflags-y += -DSUPPORT_WPS=1
endif

ifeq ($(CONFIG_MT592X_PTA), y)
ccflags-y += -DBUILD_PTA=1
endif

ifeq ($(CONFIG_MT592X_EEPROM),y)
ccflags-y += -DBUILD_USE_EEPROM=1
else
ccflags-y += -DBUILD_USE_EEPROM=0
endif



