config-type   := auto-merge
merge-command := python $(MTK_ROOT_BUILD)/tools/config/merge-kconfig.py
merge-order   := \
    common \
    platform \
    $(if $(call seq,yes,$(HAVE_AEE_FEATURE)),AEE,) \
    project \
    flavor \
    $(if $(call seq,user,$(TARGET_BUILD_VARIANT)),USER,)

