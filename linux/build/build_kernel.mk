#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_kernel.mk
#            This file builds the kernel & corresponding modules.
#            Also copies images to required folders (filesystem or vision_sdk/linux/boot)
#

STRIP_M4  = $(CODEGEN_PATH_M4)/bin/armstrip -p
STRIP_DSP = $(CODEGEN_PATH_DSP)/bin/strip6x -p
STRIP_EVE = $(CODEGEN_PATH_EVE)/bin/strip-arp32 -p

kernel_build:
	$(MAKE) -C $(kernel_PATH) ARCH=arm CROSS_COMPILE=$(A15_TOOLCHAIN_PREFIX) $(KERNEL_TARGET)

kernel_all:
	$(MAKE) -fbuild_kernel.mk kernel_clean
	$(MAKE) -fbuild_kernel.mk kernel

kernel:
	$(kernel_PATH)/scripts/kconfig/merge_config.sh -O $(kernel_PATH) -m $(kernel_PATH)/arch/arm/configs/omap2plus_defconfig $(kernel_PATH)/ti_config_fragments/ipc.cfg $(kernel_PATH)/ti_config_fragments/power.cfg $(kernel_PATH)/ti_config_fragments/audio_display.cfg $(kernel_PATH)/ti_config_fragments/system_test.cfg $(kernel_PATH)/ti_config_fragments/baseport.cfg $(kernel_PATH)/ti_config_fragments/wlan.cfg $(kernel_PATH)/ti_config_fragments/connectivity.cfg $(kernel_PATH)/ti_config_fragments/auto.cfg
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=olddefconfig
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=zImage
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=modules
	$(MAKE) -C $(memcache_PATH)/build/ modules
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=$(DEFAULT_DTB)

kernel_menuconfig:
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=menuconfig
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=zImage
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=modules
	$(MAKE) -C $(memcache_PATH)/build/ modules
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=$(DEFAULT_DTB)

kernel_clean:
	$(MAKE) -C $(kernel_PATH) ARCH=arm CROSS_COMPILE=$(A15_TOOLCHAIN_PREFIX) distclean
	$(MAKE) -C $(memcache_PATH)/build ARCH=arm CROSS_COMPILE=$(A15_TOOLCHAIN_PREFIX) clean

kernel_install:
	$(MAKE) -fbuild_kernel.mk kernel_build KERNEL_TARGET=modules_install INSTALL_MOD_PATH=$(LINUX_TARGETFS)
	install -d $(LINUX_BOOT_OUT_FILES)
	install  $(kernel_PATH)/arch/arm/boot/zImage $(LINUX_TARGETFS)/boot
	install  $(kernel_PATH)/arch/arm/boot/dts/$(DEFAULT_DTB) $(LINUX_TARGETFS)/boot
	$(MAKE)  -C $(memcache_PATH)/build TARGET=install INSTALL_MOD_PATH=$(LINUX_TARGETFS)

linux_app_install:
	mkdir -p $(LINUX_TARGETFS)/lib/firmware
	-rm -rf $(LINUX_TARGETFS)/lib/firmware/dra7-ipu1-fw.xem4
	-rm -rf $(LINUX_TARGETFS)/lib/firmware/dra7-dsp1-fw.xe66
	-rm -rf $(LINUX_TARGETFS)/lib/firmware/dra7-dsp2-fw.xe66


ifeq ($(PROC_IPU1_0_INCLUDE),yes)
	cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_ipu1_0_$(PROFILE_ipu1_0).xem4 $(LINUX_TARGETFS)/lib/firmware/dra7-ipu1-fw.xem4
	$(STRIP_M4) $(LINUX_TARGETFS)/lib/firmware/dra7-ipu1-fw.xem4
endif
ifeq ($(PROC_DSP1_INCLUDE),yes)
	cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_c66xdsp_1_$(PROFILE_c66xdsp_1).xe66 $(LINUX_TARGETFS)/lib/firmware/dra7-dsp1-fw.xe66
	$(STRIP_DSP) $(LINUX_TARGETFS)/lib/firmware/dra7-dsp1-fw.xe66
endif
ifeq ($(PROC_DSP2_INCLUDE),yes)
	cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_c66xdsp_2_$(PROFILE_c66xdsp_2).xe66 $(LINUX_TARGETFS)/lib/firmware/dra7-dsp2-fw.xe66
	$(STRIP_DSP) $(LINUX_TARGETFS)/lib/firmware/dra7-dsp2-fw.xe66
endif
	mkdir -p $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(vision_sdk_PATH)/linux/scripts/vision_sdk_load.sh $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(vision_sdk_PATH)/linux/scripts/vision_sdk_unload.sh $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(vision_sdk_PATH)/linux/scripts/vision_sdk_ov490_pinmux.sh $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_linux_demo.out $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(vision_sdk_PATH)/linux/src/links/sgx3Dsrv/jeep_outside.bmp $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(vision_sdk_PATH)/linux/src/links/sgx3Dsrv/jeep_outside2_raw.bmp $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(kernel_addon_PATH)/scripts/camnodes.sh $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(kernel_addon_PATH)/scripts/memcache_load.sh $(LINUX_TARGETFS)/opt/vision_sdk
	cp $(kernel_addon_PATH)/scripts/memcache_unload.sh $(LINUX_TARGETFS)/opt/vision_sdk
ifeq ($(VSDK_BOARD_TYPE),TDA2EX_EVM)
	cp $(kernel_addon_PATH)/scripts/set_vip_mux.sh $(LINUX_TARGETFS)/opt/vision_sdk/
endif
	cp $(kernel_addon_PATH)/scripts/powervr.ini $(LINUX_TARGETFS)/etc/
	mkdir -p $(LINUX_TARGETFS)/opt/vision_sdk/bin
	cp $(memcache_PATH)/build/memcache.ko $(LINUX_TARGETFS)/opt/vision_sdk/bin
