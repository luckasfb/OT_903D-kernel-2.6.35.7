include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).a

ifeq ($(TARGET_BUILD_MMITEST),true)
CFLAGS += -DBATT_ID_CHECK_SUPPORT=0
CFLAGS += -DMMITEST
#$(warning "mmitest = 1 ----------------------------------")
else
CFLAGS += -DBATT_ID_CHECK_SUPPORT=1
#$(warning "mmitest = 0 ----------------------------------")
endif

COBJS := mt6573.o mt6573_board.o mt6573_serial.o mt6573_wdt.o boot_mode.o load_image.o mt6573_nand.o pmu6573.o upmu_common.o mt6573_auxadc.o mt6573_bat.o mt6573_sdc.o \
         mmc_core.o \
         mmc_test.o \
         mt65xx_leds.o \
         mt6573_timer.o mt6573_key.o mt6573_rtc.o mt6573_gpio.o mt6573_gpio_init.o mt6573_i2c.o\
    mt6573_factory.o mt6573_recovery.o mt6573_meta.o	\
    mt6573_disp_drv.o \
	partition_mt.o \
    mt6573_disp_assert_layer.o \
    mt6573_disp_drv_dbi.o \
    mt6573_disp_drv_dpi.o \
    mt6573_disp_drv_dsi.o \
    mt6573_dsi_drv.o \
    mt6573_dpi_drv.o \
    mt6573_lcd_drv.o \
    mt6573_logo.o \
    mt6573_pwm.o \
    mt6573_gpt.o \
    mt6573_sleep.o \
    MetaLock_Check.o \
	bmt.o \
	mt6573_alsps.o\
    
			
			
SOBJS	:= lowlevel_init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
