# Disable XDC package build by default
ENABLE_XDC_PKG_BUILD ?= false

XDC_BUILD_PKG_LIST_COMMON += 
XDC_BUILD_PKG_LIST_ipu1_0 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_ipu1_0 += $(fc_PATH)/packages/ti.sdo.fc.ires.hdvicp
XDC_BUILD_PKG_LIST_ipu1_0 += $(fc_PATH)/packages/ti.sdo.fc.dskt2
XDC_BUILD_PKG_LIST_ipu1_1 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_c66xdsp_1 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_c66xdsp_2 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_arp32_1 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_arp32_2 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_arp32_3 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_arp32_4 += $(XDC_BUILD_PKG_LIST_COMMON)
XDC_BUILD_PKG_LIST_a15_0 += $(XDC_BUILD_PKG_LIST_COMMON)


include $(ROOTDIR)/build/makerules/platform.mk
include $(ROOTDIR)/build/makerules/env.mk

apps:
# IPU1_0 built last

ifeq ($(PROC_EVE1_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld XDCARGS="profile=$(PROFILE_arp32_1)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_1))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve1 xdc_configuro CORE=arp32_1
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve1 CORE=arp32_1
endif

ifeq ($(PROC_EVE2_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld XDCARGS="profile=$(PROFILE_arp32_2)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_2))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve2 xdc_configuro CORE=arp32_2
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve2 CORE=arp32_2
endif

ifeq ($(PROC_EVE3_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld XDCARGS="profile=$(PROFILE_arp32_3)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_3))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve3 xdc_configuro CORE=arp32_3
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve3 CORE=arp32_3
endif

ifeq ($(PROC_EVE4_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld XDCARGS="profile=$(PROFILE_arp32_4)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_4))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve4 xdc_configuro CORE=arp32_4
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve4 CORE=arp32_4
endif

ifeq ($(IPU1_EVELOADER_INCLUDE), yes)
	$(MAKE) -f $(vision_sdk_PATH)/build/makerules/build_appImageHeader.mk build_AppImageHeader
endif

ifeq ($(PROC_IPU1_1_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_m4.bld XDCARGS="profile=$(PROFILE_ipu1_1)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_ipu1_1))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/ipu1_1 xdc_configuro CORE=ipu1_1 
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/ipu1_1 CORE=ipu1_1
endif

ifeq ($(PROC_DSP1_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_c66.bld XDCARGS="profile=$(PROFILE_c66xdsp_1)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_c66xdsp_1))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/dsp1 xdc_configuro CORE=c66xdsp_1
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/dsp1 CORE=c66xdsp_1
endif

ifeq ($(PROC_DSP2_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_c66.bld XDCARGS="profile=$(PROFILE_c66xdsp_2)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_c66xdsp_2))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/dsp2 xdc_configuro CORE=c66xdsp_2
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/dsp2 CORE=c66xdsp_2
endif

ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_a15.bld XDCARGS="profile=$(PROFILE_a15_0)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_a15_0))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/a15_0 xdc_configuro CORE=a15_0
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/a15_0 CORE=a15_0
endif
endif

ifeq ($(PROC_IPU1_0_INCLUDE),yes)
ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_m4.bld XDCARGS="profile=$(PROFILE_ipu1_0)" -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_ipu1_0))
endif
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/ipu1_0 xdc_configuro CORE=ipu1_0 
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/ipu1_0 CORE=ipu1_0
endif

libs:
ifeq ($(PROC_EVE1_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=arp32_1
endif
ifeq ($(PROC_EVE2_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=arp32_2
endif
ifeq ($(PROC_EVE3_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=arp32_3
endif
ifeq ($(PROC_EVE4_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=arp32_4
endif
ifeq ($(PROC_IPU1_0_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=ipu1_0
endif
ifeq ($(PROC_IPU1_1_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=ipu1_1
endif
ifeq ($(PROC_DSP1_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=c66xdsp_1
endif
ifeq ($(PROC_DSP2_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=c66xdsp_2
endif
ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
	$(MAKE) -C $(vision_sdk_PATH)/src CORE=a15_0
endif
endif

clean:
ifeq ($(PROC_EVE1_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=arp32_1
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve1 clean CORE=arp32_1
endif
ifeq ($(PROC_EVE2_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=arp32_2
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve2 clean CORE=arp32_2
endif
ifeq ($(PROC_EVE3_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=arp32_3
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve3 clean CORE=arp32_3
endif
ifeq ($(PROC_EVE4_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=arp32_4
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/eve4 clean CORE=arp32_4
endif
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=ipu1_0
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/ipu1_0 clean CORE=ipu1_0
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=ipu1_1
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/ipu1_1 clean CORE=ipu1_1
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=c66xdsp_1
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/dsp1 clean CORE=c66xdsp_1
ifeq ($(PROC_DSP2_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=c66xdsp_2
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/dsp2 clean CORE=c66xdsp_2
endif
ifeq ($(PROC_A15_0_INCLUDE),yes)
	$(MAKE) -C $(vision_sdk_PATH)/src clean CORE=a15_0
	$(MAKE) -C $(vision_sdk_PATH)/src/main_app/$(SOC)/a15_0 clean CORE=a15_0
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_m4.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_ipu1_0))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_m4.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_ipu1_1))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_c66.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_c66xdsp_1))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_c66.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_c66xdsp_2))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_1))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_2))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_3))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_arp32.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_arp32_4))
endif

ifeq ($(ENABLE_XDC_PKG_BUILD),true)
	$(xdc_PATH)/xdc clean XDCBUILDCFG=$(vision_sdk_PATH)/build/$(SOC)/config_a15.bld -PD $(subst .,/,$(XDC_BUILD_PKG_LIST_a15_0))
endif

all: clean apps


