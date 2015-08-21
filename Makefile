# (c) Texas Instruments

include Rules.make

##########################################
#                                        #
# Vision-SDK Top Level Build Targets     #
#                                        #
##########################################

vision_sdk: 
ifeq ($(BUILD_DEPENDANCY_ALWAYS),yes)
	$(MAKE) depend
endif
	$(MAKE) vision_sdk_alg_plugins
	$(MAKE) vision_sdk_lib
	$(MAKE) vision_sdk_examples
	$(MAKE) vision_sdk_apps
ifeq ($(A15_TARGET_OS), Linux)
	$(MAKE) vision_sdk_linux
endif

clean: bsp_clean edma3lld_clean starterware_clean vision_sdk_alg_plugins_clean vision_sdk_lib_clean vision_sdk_examples_clean
ifeq ($(A15_TARGET_OS), Linux)
	$(MAKE) vision_sdk_linux_clean
endif

all: clean vision_sdk

##########################################
#                                        #
# WARNING: This deletes complete         #
# directory $(DEST_ROOT)                 #
#				                         #
##########################################

allclean:
	$(RM) -rf $(DEST_ROOT)

##########################################
#                                        #
# Vision-SDK Examples Build Targets      #
#                                        #
##########################################

vision_sdk_examples:
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/examples $(TARGET)

vision_sdk_examples_clean:
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/examples clean

vision_sdk_examples_all: vision_sdk_examples_clean vision_sdk_examples

##########################################
#                                        #
# Vision-SDK Application Build Targets   #
#                                        #
##########################################
vision_sdk_apps:
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/examples apps

#####################################################
#                                                   #
# Vision-SDK Alg-plugin Examples Build Targets      #
#                                                   #
#####################################################

vision_sdk_alg_plugins:
ifeq ($(BUILD_ALGORITHMS),yes)
	$(MAKE) algorithmslib
endif
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/examples/tda2xx/src/alg_plugins $(TARGET)

vision_sdk_alg_plugins_clean: 
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/examples/tda2xx/src/alg_plugins clean

vision_sdk_alg_plugins_all: vision_sdk_alg_plugins_clean vision_sdk_alg_plugins

######################################
#                                    #
# Vision-SDK Library Build Targets   #
#                                    #
######################################

vision_sdk_lib:
	$(MAKE) -fMAKEFILE.MK -C $(vision_sdk_PATH)/src libs

vision_sdk_lib_clean: 
	$(MAKE) -fMAKEFILE.MK -C $(vision_sdk_PATH)/src clean

vision_sdk_lib_all: vision_sdk_lib_clean vision_sdk_lib


#######################################
#                                     #
# Target to touch files required to   #
# recompile to change the number of   #
# proccesor's to build                #
#                                     #
#######################################
depend: 
	$(MAKE) edma3lld
	$(MAKE) bsp
	$(MAKE) starterware
	$(TOUCH) $(vision_sdk_PATH)/src/links_common/system/system_common.c
	$(TOUCH) $(vision_sdk_PATH)/src/links_common/algorithm/algorithmLink_cfg.c
	$(TOUCH) $(vision_sdk_PATH)/src/links_ipu/system/system_ipu1_1.c
	$(TOUCH) $(vision_sdk_PATH)/src/links_ipu/system/system_ipu1_0.c
	$(TOUCH) $(vision_sdk_PATH)/src/links_a15/system/system_a15.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/ndk/ndk_nsp_hooks.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2xx/cfg/IPC_common.cfg
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2xx/eve1/src/tlb_config_eve1.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2xx/eve2/src/tlb_config_eve2.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2xx/eve3/src/tlb_config_eve3.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2xx/eve4/src/tlb_config_eve4.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda3xx/cfg/IPC_common.cfg
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda3xx/eve1/src/tlb_config_eve1.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2ex/cfg/IPC_common.cfg
ifeq ($(HCF_INCLUDE),yes)
	$(TOUCH) $(vision_sdk_PATH)/examples/tda2xx/src/hcf/src/system/system_hcf_common.c
endif 
ifeq ($(A15_TARGET_OS),Linux)
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2xx/ipu1_0/src/main_ipu1_0.c
	$(TOUCH) $(vision_sdk_PATH)/src/main_app/tda2ex/ipu1_0/src/main_ipu1_0.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/utils_eveloader.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/eveloader/utils_rprc_parse.c
endif

depend_ndk: depend
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/network_api.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/dma_cfg/utils_dma_cfg_sys_edma.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/tda2xx/utils_prcm.c

depend_pm: 
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/utils_prf.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/utils_idle_m4.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/utils_idle_arp32.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/utils_idle_a15.c
	$(TOUCH) $(vision_sdk_PATH)/src/utils_common/src/utils_idle_c66x.c

#########################
#                       #
# BSP Build Targets     #
#                       #
#########################
bsp:
	$(MAKE) -C $(vision_sdk_PATH)/build/makerules -fbuild_bsp.mk bsp

bsp_clean:
	$(MAKE) -C $(vision_sdk_PATH)/build/makerules -fbuild_bsp.mk bsp_clean

bsp_all: bsp_clean bsp

#################################
#                               #
# Starterware Build Targets     #
#                               #
#################################

starterware:
	$(MAKE) -C ./build/makerules -fbuild_starterware.mk starterware

starterware_clean:
	$(MAKE) -C ./build/makerules -fbuild_starterware.mk starterware_clean

starterware_all: starterware_clean starterware

#################################
#                               #
# EDMA3LLD Build Targets        #
#                               #
#################################

edma3lld:
	$(MAKE) -C ./build/makerules -fbuild_edma3lld.mk edma3lld

edma3lld_clean:
	$(MAKE) -C ./build/makerules -fbuild_edma3lld.mk edma3lld_clean

edma3lld_all: edma3lld_clean edma3lld

#########################
#                       #
# IPC Build Targets     #
#                       #
#########################

ipc:
	$(MAKE) -C ./build/makerules -fbuild_ipc.mk ipc

ipc_clean:
	$(MAKE) -C ./build/makerules -fbuild_ipc.mk ipc_clean

ipc_all: ipc_clean ipc

#################################
#                               #
# SBL  build Targets            #
#                               #
#################################

sbl_sd:
	$(MAKE) -C ./build/makerules -fbuild_sbl.mk sbl_sd

sbl_qspi:
	$(MAKE) -C ./build/makerules -fbuild_sbl.mk sbl_qspi

sbl_qspi_sd:
	$(MAKE) -C ./build/makerules -fbuild_sbl.mk sbl_qspi_sd

sbl_nor:
	$(MAKE) -C ./build/makerules -fbuild_sbl.mk sbl_nor

sbl_clean:
	$(MAKE) -C ./build/makerules -fbuild_sbl.mk sbl_clean

sbl_all: sbl_clean sbl_sd sbl_qspi sbl_nor

#################################
#                               #
# Usecase auto-generate targets #
#                               #
#################################
vision_sdk_use_case_gen:
	$(MAKE) -C ./build/scripts -f use_case_gen.mk

##########################################
#                                        #
# Vision-SDK Print Build Config          #
#                                        #
##########################################
config:
	$(ECHO) \#
	$(ECHO) \# Platform config,
	$(ECHO) \# VSDK_BOARD_TYPE=$(VSDK_BOARD_TYPE) [options: TDA2XX_EVM TDA3XX_EVM TDA2XX_MC TDA2EX_EVM]
	$(ECHO) \# PLATFORM=$(PLATFORM)
ifeq ($(PLATFORM), $(filter $(PLATFORM), tda2xx-evm tda2xx-mc))
	$(ECHO) \# DDR_MEM=$(DDR_MEM) [options: DDR_MEM_256M DDR_MEM_1024M]
ifeq ($(A15_TARGET_OS),Bios)
	$(ECHO) \# DUAL_A15_SMP_BIOS=$(DUAL_A15_SMP_BIOS) [options: yes no]
endif
endif
ifeq ($(PLATFORM), tda3xx-evm)
	$(ECHO) \# DDR_MEM=$(DDR_MEM) [options: DDR_MEM_64M DDR_MEM_512M]
endif
ifeq ($(PLATFORM), tda2ex-evm)
	$(ECHO) \# DDR_MEM=$(DDR_MEM) [options: DDR_MEM_1024M]
endif
	$(ECHO) \#
	$(ECHO) \# Build config,
	$(ECHO) \# BUILD_OS=$(BUILD_OS) [options: Windows_NT Linux]
	$(ECHO) \# A15_TARGET_OS=$(A15_TARGET_OS) [options: Bios Linux]
	$(ECHO) \# PROFILE=$(PROFILE) [options: debug release]
	$(ECHO) \# BUILD_DEPENDANCY_ALWAYS=$(BUILD_DEPENDANCY_ALWAYS)
	$(ECHO) \# BUILD_ALGORITHMS=$(BUILD_ALGORITHMS)
ifeq ($(A15_TARGET_OS), Linux)
	$(ECHO) \# DEFAULT_UBOOT_CONFIG=$(DEFAULT_UBOOT_CONFIG)
	$(ECHO) \# DEFAULT_KERNEL_CONFIG=$(DEFAULT_KERNEL_CONFIG)
	$(ECHO) \# DEFAULT_DTB=$(DEFAULT_DTB)
endif
	$(ECHO) \#
	$(ECHO) \# CPU config,
	$(ECHO) \# PROC_IPU1_0_INCLUDE=$(PROC_IPU1_0_INCLUDE)
	$(ECHO) \# PROC_IPU1_1_INCLUDE=$(PROC_IPU1_1_INCLUDE)
	$(ECHO) \# PROC_DSP1_INCLUDE=$(PROC_DSP1_INCLUDE)
	$(ECHO) \# PROC_DSP2_INCLUDE=$(PROC_DSP2_INCLUDE)
	$(ECHO) \# PROC_EVE1_INCLUDE=$(PROC_EVE1_INCLUDE)
	$(ECHO) \# PROC_EVE2_INCLUDE=$(PROC_EVE2_INCLUDE)
	$(ECHO) \# PROC_EVE3_INCLUDE=$(PROC_EVE3_INCLUDE)
	$(ECHO) \# PROC_EVE4_INCLUDE=$(PROC_EVE4_INCLUDE)
	$(ECHO) \# PROC_A15_0_INCLUDE=$(PROC_A15_0_INCLUDE)
	$(ECHO) \#
	$(ECHO) \# Module config,
	$(ECHO) \# NDK_PROC_TO_USE=$(NDK_PROC_TO_USE)
	$(ECHO) \# AVBRX_INCLUDE=$(AVBRX_INCLUDE)
	$(ECHO) \# DCAN_INCLUDE=$(DCAN_INCLUDE)
	$(ECHO) \# IVAHD_INCLUDE=$(IVAHD_INCLUDE)
	$(ECHO) \# VPE_INCLUDE=$(VPE_INCLUDE)
	$(ECHO) \# ISS_INCLUDE=$(ISS_INCLUDE)
	$(ECHO) \# DSS_INCLUDE=$(DSS_INCLUDE)
	$(ECHO) \# HCF_INCLUDE=$(HCF_INCLUDE)
	$(ECHO) \# CRC_INCLUDE=$(CRC_INCLUDE)
	$(ECHO) \# CPU_IDLE_ENABLED=$(CPU_IDLE_ENABLED)
	$(ECHO) \# FAST_BOOT_INCLUDE=$(FAST_BOOT_INCLUDE)
	$(ECHO) \#




#######################################
#                                     #
# Linux Kernel and U-boot build       #
# Vision SDK linux side app build     #
#                                     #
# NOTE: Should be used only when      #
#       A15_TARGET_OS is Linux        #
#                                     #
#######################################

uboot:
	$(MAKE) -C ./linux/build -f build_uboot.mk uboot

uboot_all:
	$(MAKE) -C ./linux/build -f build_uboot.mk uboot_all

uboot_clean:
	$(MAKE) -C ./linux/build -f build_uboot.mk uboot_clean


kernel:
	$(MAKE) -C ./linux/build -f build_kernel.mk kernel

kernel_all:
	$(MAKE) -C ./linux/build -f build_kernel.mk kernel_all

kernel_menuconfig:
	$(MAKE) -C ./linux/build -f build_kernel.mk kernel_menuconfig

kernel_clean:
	$(MAKE) -C ./linux/build -f build_kernel.mk kernel_clean

sgx:
	$(MAKE) -C ./linux/build -f build_sgx.mk sgx

sgx_all:
	$(MAKE) -C ./linux/build -f build_sgx.mk sgx_all

sgx_clean:
	$(MAKE) -C ./linux/build -f build_sgx.mk sgx_clean

linux: uboot kernel sgx

linux_clean: uboot_clean sgx_clean kernel_clean

linux_all: uboot_all sgx_all kernel_all

linux_install:
	$(MAKE) -C ./linux/build -f build_kernel.mk kernel_install
	$(MAKE) -C ./linux/build -f build_sgx.mk sgx_install
	$(MAKE) -C ./linux/build -f build_uboot.mk uboot_install

vision_sdk_linux_install:
	$(MAKE) -C ./linux/build -f build_kernel.mk linux_app_install

vision_sdk_linux:
ifeq ($(BUILD_DRA7XX), yes)
	$(MAKE) -fMAKEFILE_dra7xx.MK -C$(vision_sdk_PATH)/linux/examples
else 
	$(MAKE) -fMAKEFILE_tda2xx.MK -C$(vision_sdk_PATH)/linux/examples
	$(MAKE) vision_sdk_linux_install
endif

vision_sdk_linux_clean:
	$(MAKE) -fMAKEFILE_tda2xx.MK -C$(vision_sdk_PATH)/linux/examples clean
ifeq ($(BUILD_DRA7XX), yes)
	$(MAKE) -fMAKEFILE_dra7xx.MK -C$(vision_sdk_PATH)/linux/examples clean
endif

vision_sdk_linux_all: vision_sdk_linux_clean vision_sdk_linux

## TI internal make targets. NOT to be used by customers ##

#######################################
#                                     #
# Algorithms                          #
#                                     #
#######################################

algorithmslib:
	$(MAKE) -C ./algorithms -fMAKEFILE.MK algorithmslib

algorithmslib_clean:
	$(MAKE) -C ./algorithms -fMAKEFILE.MK algorithmslib_clean

algorithmslib_all: algorithmslib_clean algorithmslib

##########################################
#                                        #
# Vision-SDK test suite Build Targets    #
#                                        #
# NOTE: Testsuite software NOT included  #
#       in release package		 #
##########################################

vision_sdk_test: 
ifeq ($(BUILD_DEPENDANCY_ALWAYS),yes)
	$(MAKE) depend
endif
	$(MAKE) vision_sdk_alg_plugins
	$(MAKE) vision_sdk_lib
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/testsuite $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/testsuite apps 

vision_sdk_test_clean: bsp_clean edma3lld_clean starterware_clean vision_sdk_alg_plugins_clean vision_sdk_lib_clean
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/testsuite clean

vision_sdk_test_all: vision_sdk_test vision_sdk_test_clean

##############################################
#                                            #
# Vision-SDK Linux test suite Build Targets  #
#                                            #
# NOTE: Testsuite software NOT included      #
#       in release package		     #
##############################################

vision_sdk_linux_test: 
	$(MAKE) vision_sdk_alg_plugins
	$(MAKE) vision_sdk_lib
	$(MAKE) vision_sdk_examples
	$(MAKE) vision_sdk_apps
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/testsuite
	$(MAKE) vision_sdk_linux_install

vision_sdk_linux_test_clean: bsp_clean edma3lld_clean starterware_clean vision_sdk_alg_plugins_clean vision_sdk_lib_clean vision_sdk_examples_clean
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/testsuite clean

vision_sdk_linux_test_all: vision_sdk_linux_test vision_sdk_linux_test_clean

##############################################
#                                            #
# HCF Build Targets                          #
#                                            #
# NOTE: Experimental module NOT included     #
#       in release package                   #
##############################################

hcf:
	$(MAKE) -C ./build/makerules -fbuild_hcf.mk hcf

hcf_clean:
	$(MAKE) -C ./build/makerules -fbuild_hcf.mk hcf_clean

hcf_all: hcf_clean hcf


.PHONY : allclean vision_sdk clean all vision_sdk_examples vision_sdk_alg_plugins vision_sdk_lib depend bsp starterware edma3lld sbl_sd sbl_nor sbl_qspi vision_sdk_test algorithmslib uboot uboot_all uboot_clean kernel kernel_all kernel_menuconfig kernel_clean sgx sgx_clean sgx_all $(bsp_PKG_LIST)

