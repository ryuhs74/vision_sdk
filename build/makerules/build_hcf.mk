#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_hcf.mk
#            This file builds the HCF libs.

hcf_build:
	$(MAKE) -C $(hcf_PATH)/ $(HCF_TARGET) CGT6X_ROOT=$(CODEGEN_PATH_DSP) TIARMCGT_ROOT=$(CODEGEN_PATH_M4) BIOS_ROOT=$(bios_PATH) IPC_ROOT=$(ipc_PATH) XDC_ROOT=$(xdc_PATH) TMS470_ROOT=$(CODEGEN_PATH_A15) ARP32CGT_ROOT=$(CODEGEN_PATH_EVE) CGT7X_ROOT=$(CODEGEN_PATH_DSP) GCC_ROOT=$(CODEGEN_PATH_A15) CROSS_COMPILE=arm-none-eabi-

hcf:
	$(MAKE) -fbuild_hcf.mk hcf_build HCF_TARGET=

hcf_clean:
	$(MAKE) -fbuild_hcf.mk hcf_build HCF_TARGET=clean


	