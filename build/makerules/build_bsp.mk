#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_bsp.mk
#            This file builds the BSP libs.

bsp_build:
	$(MAKE) -C $(bsp_PATH)/ $(BSP_TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0)

bsp:
	$(MAKE) -fbuild_bsp.mk bsp_build BSP_TARGET=bsp
	$(MAKE) -C $(bsp_PATH)/src/osal $(BSP_TARGET) CORE=c66x PROFILE_c66x=$(PROFILE_c66xdsp_1)
ifeq ($(PLATFORM), $(filter $(PLATFORM), tda2xx-evm tda2xx-mc tda3xx-evm))
	$(MAKE) -C $(bsp_PATH)/src/osal CORE=arp32_1 PROFILE_arp32_1=$(PROFILE_arp32_1)
endif
ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
	$(MAKE) -C $(bsp_PATH)/src/osal CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
endif
endif

bsp_clean:
	$(MAKE) -fbuild_bsp.mk bsp_build BSP_TARGET=clean

