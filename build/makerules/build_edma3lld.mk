#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_edma3lld.mk
#            This file builds the edma3lld libs.


EDMA3LLD_DRV_BUILD_OPTS=$(edma_PATH)/packages/ti/sdo/edma3/drv $(TARGET) MODULE_NAME=edma3lld_drv edma3lld_drv_PLATFORM_DEPENDENCY=yes
EDMA3LLD_RM_BUILD_OPTS=$(edma_PATH)/packages/ti/sdo/edma3/rm $(TARGET) MODULE_NAME=edma3lld_rm  edma3lld_rm_PLATFORM_DEPENDENCY=yes

edma3lld:
ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
	$(MAKE) -C $(EDMA3LLD_DRV_BUILD_OPTS) CORE=a15_0 ISA=a15
	$(MAKE) -C $(EDMA3LLD_RM_BUILD_OPTS) CORE=a15_0 ISA=a15 
endif
endif
	$(MAKE) -C $(EDMA3LLD_DRV_BUILD_OPTS) CORE=ipu1_0 ISA=m4
	$(MAKE) -C $(EDMA3LLD_RM_BUILD_OPTS) CORE=ipu1_0 ISA=m4 

	$(MAKE) -C $(EDMA3LLD_DRV_BUILD_OPTS) CORE=c66xdsp_1 ISA=66
	$(MAKE) -C $(EDMA3LLD_RM_BUILD_OPTS) CORE=c66xdsp_1 ISA=66 

ifeq ($(PLATFORM), $(filter $(PLATFORM), tda2xx-evm tda2xx-mc tda3xx-evm))
	$(MAKE) -C $(EDMA3LLD_DRV_BUILD_OPTS) CORE=arp32_1 ISA=arp32
	$(MAKE) -C $(EDMA3LLD_RM_BUILD_OPTS) CORE=arp32_1 ISA=arp32 
endif

edma3lld_clean:
	$(MAKE) -fbuild_edma3lld.mk edma3lld TARGET=clean



	
