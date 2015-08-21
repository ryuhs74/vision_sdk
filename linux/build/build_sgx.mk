#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_sgx.mk
#            This file builds the sgx.

SGX_DRA7XX_MAKEFILE := $(sgx_PATH)/omap5-sgx-ddk-linux/eurasia_km/eurasiacon/build/linux2/omap5430_linux

sgx_build:
	$(MAKE) -C $(SGX_DRA7XX_MAKEFILE) ARCH=arm CROSS_COMPILE=$(A15_TOOLCHAIN_PREFIX) KERNELDIR=$(kernel_PATH) DISCIMAGE=$(LINUX_TARGETFS)

sgx_all:
	$(MAKE) -fbuild_sgx.mk sgx_clean
	$(MAKE) -fbuild_sgx.mk sgx

sgx:
	$(MAKE) -fbuild_sgx.mk sgx_build SGX_TARGET=sgx_build

sgx_clean:
	$(MAKE) -C $(SGX_DRA7XX_MAKEFILE) ARCH=arm CROSS_COMPILE=$(A15_TOOLCHAIN_PREFIX)  KERNELDIR=$(kernel_PATH) DISCIMAGE=$(LINUX_TARGETFS) clean

sgx_install:
	$(MAKE) -C $(SGX_DRA7XX_MAKEFILE) ARCH=arm CROSS_COMPILE=$(A15_TOOLCHAIN_PREFIX) KERNELDIR=$(kernel_PATH) DISCIMAGE=$(LINUX_TARGETFS) kbuild_install

