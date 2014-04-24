obj-y := core.o irq.o system.o timer.o mt6573_devs.o mt6573_board.o dma.o mt6573_boot.o mt6573_gpt.o mt6573_eint.o mt6573_auxadc.o mt6573_gpt.o
obj-y += mt6573_gpio.o mt6573_pwm.o
#obj-y += mt6573_auxadc.o mt6573_gpt.o mt6573_pwm.o
obj-y += mt6573_pm_api.o mt6573_ost.o mtkpm.o cpufreq-mt6573.o mt6573_pm_factory.o mt6573_sleep.o mt6573_pm_log.o mt6573_avs_thermal.o mt6573_clock_manager.o mt6573_pm_ldo.o mt6573_dcm.o
obj-$(CONFIG_MT65XX_TRACER) += mt6573_mon.o arm11_perf_mon.o mt6573_emi_bm.o 
obj-y += mt6573_MDP.o mt6573_sysram.o
obj-y += mt6573_isp.o
obj-y += mt6573_eis.o
obj-y += mt6573_fdvt.o
obj-y += mt6573_wdt.o
obj-y += mt_rfkill.o
ifneq ($(MTK_EMMC_SUPPORT),yes)
obj-y += sbchk_base.o
endif
obj-y += outercache.o mt6573_emi_bwl.o mt6573_emi_mpu.o
obj-y += flush_dcache_all.o inner_flush_dcache_all.o
# MTK COMBO chip or not
ifneq ($(CONFIG_MTK_COMBO),)
ifeq ($(CONFIG_MTK_COMBO),n)
#mt_bt.c should not be built on all MT6573 project[mt_bt.c must be compiled after btwlanem.c is built ]
#obj-y += mt_bt.o
else
obj-y += mt_combo.o
endif
else
#mt_bt.c should not be built on all MT6573 project[mt_bt.c must be compiled after btwlanem.c is built ]
#obj-y += mt_bt.o
endif
obj-y += arm_pwr_test.o arm_pwr_test_asm.o

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power \
             -I$(MTK_PATH_SOURCE)/drivers/power \
             -I$(MTK_PATH_SOURCE)/drivers/sound \
             -I$(MTK_PATH_PLATFORM)/drivers/sound \
             -I$(MTK_PATH_SOURCE)/drivers/wdk
