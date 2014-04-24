ifeq (MT5192_FM, $(strip $(MTK_FM_CHIP)))
  ifeq (no, $(strip $(MTK_MT519X_FM_SUPPORT)))
    $(call dep-err-seta-or-setb,MTK_MT519X_FM_SUPPORT,yes,MTK_FM_CHIP,non MT5192_FM)
  endif
endif

ifeq (MT6620_FM, $(strip $(MTK_FM_CHIP)))
  ifeq (yes, $(strip $(MTK_MT519X_FM_SUPPORT)))
    $(call dep-err-seta-or-setb,MTK_MT519X_FM_SUPPORT,no,MTK_FM_CHIP,non MT6620_FM)
  endif
endif

ifeq (MT6620_FM, $(strip $(MTK_FM_CHIP)))
  ifneq (mt6620, $(strip $(CUSTOM_KERNEL_FM)))
    $(call dep-err-seta-or-setb,CUSTOM_KERNEL_FM,mt6620,MTK_FM_CHIP,MT6620_FM)
  endif
endif

ifeq (MT5192_FM, $(strip $(MTK_FM_CHIP)))
  ifneq (, $(strip $(CUSTOM_KERNEL_FM)))
    $(call dep-err-seta-or-setb,MTK_FM_CHIP,non MT5192_FM,CUSTOM_KERNEL_FM,)
  endif
endif

#################################################################
# for camera feature

ifneq ($(strip $(CUSTOM_HAL_CAMERA)), $(strip $(CUSTOM_KERNEL_CAMERA)))
    $(call dep-err-seta-or-setb,CUSTOM_HAL_CAMERA,$(CUSTOM_KERNEL_CAMERA),CUSTOM_KERNEL_CAMERA,$(CUSTOM_HAL_CAMERA))
endif

##################################################################
# for combo feature
ifdef MTK_COMBO_CHIP
  ifneq (,$(filter MT6620E1 MT6620E2,$(MTK_COMBO_CHIP)))
     ifneq (combo, $(CUSTOM_HAL_COMBO))
       $(call dep-err-seta-or-setb,CUSTOM_HAL_COMBO,combo,MTK_COMBO_CHIP,MT6620E3 or above)
     endif
  else
     ifneq (mt6620, $(CUSTOM_HAL_COMBO))
       $(call dep-err-seta-or-setb,CUSTOM_HAL_COMBO,mt6620,MTK_COMBO_CHIP,MT6620E2 or MT6620E1)
     endif
  endif
endif

##############################################################
# for share modem

ifeq (2,$(strip $(MTK_SHARE_MODEM_SUPPORT)))
  ifeq ($(call gt,$(MTK_SHARE_MODEM_CURRENT),2),T)
    $(call dep-err-common, please set MTK_SHARE_MODEM_CURRENT as 2 or 1 or 0 when MTK_SHARE_MODEM_SUPPORT=2)
  endif
endif

ifeq (1,$(strip $(MTK_SHARE_MODEM_SUPPORT)))
  ifeq ($(call gt,$(MTK_SHARE_MODEM_CURRENT),1),T)
    $(call dep-err-common, please set MTK_SHARE_MODEM_CURRENT as 1 or 0 when MTK_SHARE_MODEM_SUPPORT=1)
  endif
endif

ifeq (yes,$(strip $(GEMINI)))
  ifneq (2,$(strip $(MTK_SHARE_MODEM_CURRENT)))
    $(call dep-err-common, please set MTK_SHARE_MODEM_CURRENT=2 when GEMINI=yes)
  endif
endif

ifeq (no,$(strip $(GEMINI)))
  ifneq (1,$(strip $(MTK_SHARE_MODEM_CURRENT)))
    $(call dep-err-common, please set MTK_SHARE_MODEM_CURRENT=1 when GEMINI=no)
  endif
endif

##############################################################
# for mtk sec modem

ifeq (yes,$(strip $(MTK_SEC_MODEM_AUTH)))
  ifeq (no,$(strip $(MTK_SEC_MODEM_ENCODE)))
    $(call dep-err-ona-or-offb, MTK_SEC_MODEM_ENCODE, MTK_SEC_MODEM_AUTH)
  endif
endif
