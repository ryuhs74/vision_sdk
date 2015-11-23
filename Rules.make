#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# file name: Rules.make
# set up the build environment

ifeq ($(vision_sdk_PATH), )

# Board type can be one of the following
#   1. TDA2XX_EVM
#   2. TDA3XX_EVM
#   3. TDA2XX_MC
#   4. TDA2EX_EVM
#
#e.g.
#VSDK_BOARD_TYPE := TDA3XX_EVM
#VSDK_BOARD_TYPE := TDA2EX_EVM
#VSDK_BOARD_TYPE := TDA2XX_MC
# Default board typ is TDA2xx EVM
ifeq ($(VSDK_BOARD_TYPE), )
  VSDK_BOARD_TYPE := TDA2XX_EVM
endif

#
# Set BUILD_OS to Windows_NT to compile from BUILD_OS prompt
#
BUILD_OS := Windows_NT

# Default build environment, windows or linux
ifeq ($(BUILD_OS), )
  BUILD_OS := Linux
endif

#
# Set BUILD_MACHINE to 32BIT as required, this is needed only
# if A15_TARGET_OS is going to be Linux
#
# BUILD_MACHINE :=32BIT
#
ifeq ($(BUILD_MACHINE), )
  BUILD_MACHINE := 64BIT
endif

#
# Set A15_TARGET_OS, master core can be A15 or IPU1-C0
# A15 can run Linux or Bios
# IPU1 will always run Bios
#
#                 A15_TARGET_OS support
#   Platform        BIOS        Linux
#
#   TDA2XX_EVM      yes         yes
#   TDA3XX_EVM      yes         no
#   TDA2XX_MC       yes         no
#   TDA2EX_EVM      yes         yes
#

# Default run environment Bios or Linux
ifeq ($(A15_TARGET_OS), )
 A15_TARGET_OS := Bios
# A15_TARGET_OS := Linux
endif

# Default platform
# Supported values: tda2xx-evm, tda3xx-evm, tda2xx-mc
ifeq ($(PLATFORM), )
    ifeq ($(VSDK_BOARD_TYPE), TDA2XX_EVM)
        PLATFORM := tda2xx-evm
    endif
    ifeq ($(VSDK_BOARD_TYPE), TDA3XX_EVM)
        PLATFORM := tda3xx-evm
    endif
    ifeq ($(VSDK_BOARD_TYPE), TDA2XX_MC)
        PLATFORM := tda2xx-mc
    endif
    ifeq ($(VSDK_BOARD_TYPE), TDA2EX_EVM)
        PLATFORM := tda2ex-evm
    endif
endif

ifeq ($(A15_TARGET_OS), Linux)
  BUILD_OS := Linux
endif

# Default build tda2xx only
ifeq ($(BUILD_DRA7XX), )
  BUILD_DRA7XX := no
endif

vision_sdk_RELPATH = vision_sdk

ifeq ($(BUILD_OS),Windows_NT)
  TI_SW_ROOT      := $(abspath ..)/ti_components
endif

ifeq ($(BUILD_OS),Linux)
  TI_SW_ROOT      := $(abspath ..)/ti_components
  # or /opt/ti if you follow the package installers
endif

vision_sdk_PATH  := $(abspath ..)/$(vision_sdk_RELPATH)
infoadas_PATH  :=

#
# Code gen and config tools
#

ifeq ($(BUILD_OS),Windows_NT)
CODEGEN_PATH_DSP ?= $(TI_SW_ROOT)/cg_tools/windows/c6000_7_4_2
CODEGEN_PATH_EVE ?= $(TI_SW_ROOT)/cg_tools/windows/arp32_1_0_6
CODEGEN_PATH_A15 ?= $(TI_SW_ROOT)/cg_tools/windows/gcc-arm-none-eabi-4_7-2013q3
CODEGEN_PATH_M4  ?= $(TI_SW_ROOT)/cg_tools/windows/ti-cgt-arm_5.2.4
endif

ifeq ($(BUILD_OS),Linux)
CODEGEN_PATH_DSP ?= $(TI_SW_ROOT)/cg_tools/linux/c6000_7_4_2
CODEGEN_PATH_EVE ?= $(TI_SW_ROOT)/cg_tools/linux/arp32_1_0_6
CODEGEN_PATH_M4  ?= $(TI_SW_ROOT)/cg_tools/linux/ti-cgt-arm_5.2.4

ifeq ($(A15_TARGET_OS),Bios)
CODEGEN_PATH_A15 ?= $(TI_SW_ROOT)/cg_tools/linux/gcc-arm-none-eabi-4_7-2013q3
endif

ifeq ($(A15_TARGET_OS),Linux)
CODEGEN_PATH_A15     ?= $(TI_SW_ROOT)/os_tools/linux/linaro/gcc-linaro-arm-linux-gnueabihf-4.7-2013.03-20130313_linux
A15_TOOLCHAIN_PREFIX ?= $(CODEGEN_PATH_A15)/bin/arm-linux-gnueabihf-
#
# Path where linux uImage, uboot etc will be copied to after build
#
LINUX_BOOT_OUT_FILES =$(vision_sdk_PATH)/linux/boot
LINUX_TARGETFS ?=$(vision_sdk_PATH)/linux/targetfs
endif

endif

#
# BIOS, IPC and XDC, M4 Codegen
#

ifeq ($(PLATFORM),$(filter $(PLATFORM), tda2xx-evm tda2ex-evm tda2xx-mc tda3xx-evm))
    bios_PATH ?= $(TI_SW_ROOT)/os_tools/bios_6_41_04_54
    ipc_PATH  ?= $(TI_SW_ROOT)/os_tools/ipc_3_36_01_11
    ifeq ($(BUILD_OS),Windows_NT)
        xdc_PATH  ?= $(TI_SW_ROOT)/os_tools/windows/xdctools_3_30_06_67_core
    endif
    ifeq ($(BUILD_OS),Linux)
        xdc_PATH  ?= $(TI_SW_ROOT)/os_tools/linux/xdctools_3_30_06_67_core
    endif
endif


ifeq ($(A15_TARGET_OS),Linux)
kernel_PATH ?= $(vision_sdk_PATH)/../ti_components/os_tools/linux/kernel/omap
kernel_addon_PATH ?= $(vision_sdk_PATH)/../ti_components/os_tools/linux/kernel/linux-kernel-addon
memcache_PATH ?= $(kernel_addon_PATH)/memcache
uboot_PATH  ?= $(vision_sdk_PATH)/../ti_components/os_tools/linux/u-boot/u-boot
sgx_PATH ?= $(vision_sdk_PATH)/../ti_components/os_tools/linux/sgx
endif


#
# Low-level drivers
#
bsp_PATH         ?= $(TI_SW_ROOT)/drivers/bsp_01_03_00_07
edma_PATH        ?= $(TI_SW_ROOT)/drivers/edma3_lld_02_12_00_20
starterware_PATH ?= $(TI_SW_ROOT)/drivers/starterware_01_03_00_09

# Set the PACKAGE_SELECT option for BSP/STW to individually control VIP, VPE and DSS
# For Bios on A15  - Display will be controlled by M4 Bios, so set as "all" (including DSS)
# For Linux on A15 - Display will be controlled by A15 Linux, so set as "vps-vip-vpe" (excluding DSS)
ifeq ($(A15_TARGET_OS),Bios)
BSP_STW_PACKAGE_SELECT := all
endif
ifeq ($(A15_TARGET_OS), Linux)
BSP_STW_PACKAGE_SELECT := vps-vip-vpe
endif

#
# Networking related packages
#
ndk_PATH   ?= $(TI_SW_ROOT)/networking/ndk_2_24_02_31
nsp_PATH   ?= $(TI_SW_ROOT)/networking/nsp_gmacsw_4_13_00_00
avbtp_PATH ?= $(TI_SW_ROOT)/networking/avbtp_0_09_00_01

#
# Algorithm related packages
#
fc_PATH        ?= $(TI_SW_ROOT)/algorithms_codecs/framework_components_3_31_00_02
xdais_PATH     ?= $(TI_SW_ROOT)/algorithms_codecs/xdais_7_24_00_04
hdvicplib_PATH ?= $(TI_SW_ROOT)/algorithms_codecs/ivahd_hdvicp20api_01_00_00_23_production
jpegvenc_PATH  ?= $(TI_SW_ROOT)/algorithms_codecs/ivahd_jpegvenc_01_00_16_01_production
jpegvdec_PATH  ?= $(TI_SW_ROOT)/algorithms_codecs/ivahd_jpegvdec_01_00_13_01_production
h264venc_PATH  ?= $(TI_SW_ROOT)/algorithms_codecs/ivahd_h264enc_02_00_09_01_production
h264vdec_PATH  ?= $(TI_SW_ROOT)/algorithms_codecs/ivahd_h264vdec_02_00_17_01_production
evealg_PATH    ?= $(TI_SW_ROOT)/algorithms_codecs/eve_sw_01_09_00_00
vlib_PATH      ?= $(TI_SW_ROOT)/algorithms_codecs/vlib_c66x_3_2_1_0
lane_detect_PATH ?= $(TI_SW_ROOT)/algorithms_codecs/REL.200.V.LD.C66X.00.02.01.00/200.V.LD.C66X.00.02
object_detect_PATH ?= $(TI_SW_ROOT)/algorithms_codecs/REL.200.V.OD.C66X.00.04.00.00/200.V.OD.C66X.00.04
hcf_PATH       ?= $(TI_SW_ROOT)/algorithms_codecs/hcf/repos/
stereo_postprocess_PATH ?= $(TI_SW_ROOT)/algorithms_codecs/REL.200.V.ST.C66X.00.02.02.00/200.V.ST.C66X.02.02

ROOTDIR := $(vision_sdk_PATH)

XDCPATH = $(bios_PATH)/packages;$(ipc_PATH)/packages;$(xdc_PATH)/packages;$(edma_PATH)/packages;$(bsp_PATH);$(starterware_PATH);$(fc_PATH)/packages;$(vision_sdk_PATH);$(xdais_PATH)/packages;$(hdvicplib_PATH)/packages;$(jpegvdec_PATH)/packages;$(jpegvenc_PATH)/packages;$(ndk_PATH)/packages;$(nsp_PATH)/packages;$(avbtp_PATH)/packages;
edma_DRV_PATH = $(edma_PATH)/packages/ti/sdo/edma3/drv
edma_RM_PATH  = $(edma_PATH)/packages/ti/sdo/edma3/rm

# postfixes we require
_CORES     := M4 DSP EVE A15
# Check for the existence of each compiler!
$(foreach path,$(_CORES),$(if $(realpath $(CODEGEN_PATH_$(path))),,$(error CODEGEN_PATH_$(path) does not exist! ($(CODEGEN_PATH_$(path))))))
# prefixes we require...
_REQ_PATHS := xdc bios ipc bsp edma starterware fc xdais evealg
_OPT_PATHS := ndk nsp avbtp hdvicplib jpegvdec jpegvenc vlib
# Check for the existence of each xxxxx_PATH variable! Error if it does not exist.
$(foreach path,$(_REQ_PATHS),$(if $(realpath $($(path)_PATH)),,$(error $(path)_PATH does not exist! ($($(path)_PATH)))))
$(foreach path,$(_OPT_PATHS),$(if $(realpath $($(path)_PATH)),,$(warning $(path)_PATH does not exist! ($($(path)_PATH)))))

ifeq ($(A15_TARGET_OS), Linux)
  DEFAULT_UBOOT_CONFIG  := dra7xx_evm_config
  DEFAULT_KERNEL_CONFIG := omap2plus_defconfig
  ifeq ($(VSDK_BOARD_TYPE), TDA2EX_EVM)
    DEFAULT_DTB := dra72-evm-infoadas.dtb
  else
    DEFAULT_DTB := dra7-evm-infoadas.dtb
  endif
endif

###############################
# Set DDR_MEM config.         #
# Available/Supported configs #
#   For TDS2XX: 256M, 1G;     #
###############################

ifeq ($(PLATFORM), tda2xx-evm)
ifeq ($(DDR_MEM), )
#  DDR_MEM := DDR_MEM_256M
  DDR_MEM := DDR_MEM_1024M
endif
endif

ifeq ($(PLATFORM), tda2xx-mc)
ifeq ($(DDR_MEM), )
#  DDR_MEM := DDR_MEM_256M
  DDR_MEM := DDR_MEM_1024M
endif
endif

###############################
# Set DDR_MEM config.         #
# Available/Supported configs #
#   For TDA3XX: 512M, 64M;    #
###############################

ifeq ($(PLATFORM), tda3xx-evm)
ifeq ($(DDR_MEM), )
#  DDR_MEM := DDR_MEM_64M
  DDR_MEM := DDR_MEM_512M
endif
endif

###############################
# Set DDR_MEM config.         #
# Available/Supported configs #
#   For TDS2EX: 1G;           #
###############################

ifeq ($(PLATFORM), tda2ex-evm)
ifeq ($(DDR_MEM), )
  DDR_MEM := DDR_MEM_1024M
endif
endif

#######################################
# To Eanble Dual A15 in SMP BIOS mode #
# Set DUAL_A15_SMP_BIOS := yes        #
# This is supported only on TDA2xx    #
#######################################

# By default SMP BIOS is disabled. It is can be enabled for TDA2xx platform below 
DUAL_A15_SMP_BIOS := no

ifeq ($(PLATFORM),$(filter $(PLATFORM), tda2xx-evm tda2xx-mc))
ifeq ($(A15_TARGET_OS),Bios)
  DUAL_A15_SMP_BIOS := no
endif
endif

# Default profile: release
# Supported profiles: release & debug
ifeq ($(PROFILE), )
# Enable debug profile for all cores
 PROFILE = debug
# Enable release profile for all cores
# PROFILE = release
endif

ifeq ($(PROFILE_ipu1_0), )
  PROFILE_ipu1_0 := $(PROFILE)
endif
ifeq ($(PROFILE_ipu1_1), )
  PROFILE_ipu1_1 := $(PROFILE)
endif

ifeq ($(PROFILE_c66xdsp_1), )
  PROFILE_c66xdsp_1 := $(PROFILE)
endif
ifeq ($(PROFILE_c66xdsp_2), )
  PROFILE_c66xdsp_2 := $(PROFILE)
endif

ifeq ($(PROFILE_arp32_1), )
  PROFILE_arp32_1 := $(PROFILE)
endif
ifeq ($(PROFILE_arp32_2), )
  PROFILE_arp32_2 := $(PROFILE)
endif
ifeq ($(PROFILE_arp32_3), )
  PROFILE_arp32_3 := $(PROFILE)
endif
ifeq ($(PROFILE_arp32_4), )
  PROFILE_arp32_4 := $(PROFILE)
endif

# A15 use gcc tools (compiler, linker etc.), which support only debug mode
ifeq ($(PROFILE_a15_0), )
  PROFILE_a15_0 := debug
endif

# Default klockwork build flag, yes or no
ifeq ($(KW_BUILD), )
  KW_BUILD := no
endif

# Default C++ build flag, yes or no
ifeq ($(CPLUSPLUS_BUILD), )
  CPLUSPLUS_BUILD := no
endif

#
# Change below to include or exclude certain core's
#
PROC_DSP1_INCLUDE=no
PROC_DSP2_INCLUDE=no
PROC_EVE1_INCLUDE=no
PROC_EVE2_INCLUDE=no
PROC_EVE3_INCLUDE=no
PROC_EVE4_INCLUDE=no
PROC_A15_0_INCLUDE=yes
PROC_IPU1_0_INCLUDE=yes
PROC_IPU1_1_INCLUDE=no

#
# Change below to include or exclude certain HW module's
#
# Below are default's based on TDA2xx BIOS build, these are overridden
# later to TDA3xx or TDA2ex specfic one's or Linux on A15 specific one's
#


AVBRX_INCLUDE=yes
IVAHD_INCLUDE=yes
VPE_INCLUDE=yes
DSS_INCLUDE=yes
ISS_INCLUDE=no
WDR_LDC_INCLUDE=no
CRC_INCLUDE=no

#
#
# Enable below macro to enable DCAN integration into Vision SDK.
#
DCAN_INCLUDE=no


#
# CPU to use to run Networking stack. Valid values are
# ipu1_0 ipu1_1 a15_0 none
#
ifeq ($(NDK_PROC_TO_USE), )
NDK_PROC_TO_USE=a15_0
endif

# Fast boot usecase is currently supported only for tda3x
FAST_BOOT_INCLUDE=no

#
# Some CPU's and module's force disable if build is for TDA3xx
#
# Some modules and core's are disabled since they are not
# present in TDA3xx, like VPE, IVAHD, A15_0, EVE2/3/4
#
# Enable some module's if they present only in TDA3xx, like ISS
#
ifeq ($(PLATFORM), tda3xx-evm)

# Disabling or enabling below CPUs/modules for now until they are tested
PROC_DSP1_INCLUDE=yes
PROC_DSP2_INCLUDE=yes
# Currently, for 64MB DDR build for TDA3xx, DSP2 is not included
ifeq ($(DDR_MEM), DDR_MEM_64M)
PROC_DSP2_INCLUDE=no
endif
PROC_EVE1_INCLUDE=yes
PROC_IPU1_1_INCLUDE=yes
ISS_INCLUDE=yes
WDR_LDC_INCLUDE=no
AVBRX_INCLUDE=no
NDK_PROC_TO_USE=ipu1_1
FAST_BOOT_INCLUDE=no
# Below CPUs/modules are not present in TDA3xx
PROC_A15_0_INCLUDE=no
PROC_EVE2_INCLUDE=no
PROC_EVE3_INCLUDE=no
PROC_EVE4_INCLUDE=no
IVAHD_INCLUDE=no
VPE_INCLUDE=no
CRC_INCLUDE=yes
endif

#
# Some CPU's and module's force disable if build is for TDA2ex
#
# Some modules and core's are disabled since they are not
# present in TDA2ex, like DSP2, EVE1/2/3/4
#
ifeq ($(PLATFORM), tda2ex-evm)

# Disabling or enabling below CPUs/modules for now until they are tested
PROC_DSP1_INCLUDE=yes
PROC_IPU1_1_INCLUDE=yes
PROC_A15_0_INCLUDE=yes
IVAHD_INCLUDE=yes
VPE_INCLUDE=yes
# Below CPUs/modules are not present in TDA2ex
PROC_DSP2_INCLUDE=no
PROC_EVE1_INCLUDE=no
PROC_EVE2_INCLUDE=no
PROC_EVE3_INCLUDE=no
PROC_EVE4_INCLUDE=no
ISS_INCLUDE=no
WDR_LDC_INCLUDE=no
CRC_INCLUDE=no
endif


#
# Some CPU's and module's force disable when linux run's on A15
#
ifeq ($(A15_TARGET_OS), Linux)
PROC_IPU1_1_INCLUDE=no
DSS_INCLUDE=no
NDK_PROC_TO_USE=none
endif

#
# To get a faster incremental build do below
# BUILD_DEPENDANCY_ALWAYS=no
#
# IMPORTANT: In this case make sure to do
# (g)make -s depend
# in case of any change in dependancies like
# - changes in BSP/Starterware/EDMA3LLD .c or .h file
# - changes in number of cores used for build (value of PROC_<CPU>_INCLUDE)
# - change in value of DDR_MEM
# - change in value of NDK_PROC_TO_USE
#
BUILD_DEPENDANCY_ALWAYS=yes

#
# Used to control building of algorithm source.
# By default algorithm source not included in Vision SDK
#
BUILD_ALGORITHMS=no

TREAT_WARNINGS_AS_ERROR=yes

#
# HCF is an experimental module, NOT to used by users of Vision SDK
# HCF wont be included in Vision SDK release
#
# Keep below as 'no' always
#
HCF_INCLUDE=no


ifeq ($(NDK_PROC_TO_USE),ipu1_1)
    ifeq ($(PROC_IPU1_1_INCLUDE),no)
        NDK_PROC_TO_USE=none
    endif
endif

ifeq ($(NDK_PROC_TO_USE),ipu1_0)
    ifeq ($(PROC_IPU1_0_INCLUDE),no)
        NDK_PROC_TO_USE=none
    endif
endif

ifeq ($(NDK_PROC_TO_USE),a15_0)
    ifeq ($(PROC_A15_0_INCLUDE),no)
        NDK_PROC_TO_USE=none
    endif
endif


#
# NDK/NSP on A15 with SMP BIOS has issues and hence disable networking
# support on A15, when A15 is running SMP Bios
#
ifeq ($(DUAL_A15_SMP_BIOS),yes)
    ifeq ($(NDK_PROC_TO_USE),a15_0)
        NDK_PROC_TO_USE=none
    endif
endif


#
# IPU1_EVELOADER_INCLUDE is used by IPU firmware to decide on
# EVE loading from IPU1. This is used only when A15_TARGET_OS
# is Linux.
#

IPU1_EVELOADER_INCLUDE=no

ifeq ($(A15_TARGET_OS), Linux)
    ifeq ($(PROC_EVE1_INCLUDE), yes)
        IPU1_EVELOADER_INCLUDE=yes
    endif
    ifeq ($(PROC_EVE2_INCLUDE), yes)
        IPU1_EVELOADER_INCLUDE=yes
    endif
    ifeq ($(PROC_EVE3_INCLUDE), yes)
        IPU1_EVELOADER_INCLUDE=yes
    endif
    ifeq ($(PROC_EVE4_INCLUDE), yes)
        IPU1_EVELOADER_INCLUDE=yes
    endif

endif

#
# Used to enable or disable CPU idle functionality in SDK
# By Default CPU idle is enabled
#
CPU_IDLE_ENABLED=yes

ifeq ($(PROFILE),debug)
CPU_IDLE_ENABLED=no
endif

#
# When HCF is enabled, disable PM features
# since EVE load calc will go wrong with PM and HCF enabled together
#
ifeq ($(HCF_INCLUDE),yes)
    CPU_IDLE_ENABLED=no
endif

PROC_EVE_INCLUDE = no
ifeq ($(PROC_EVE2_INCLUDE),yes)
	PROC_EVE_INCLUDE=yes
endif
ifeq ($(PROC_EVE3_INCLUDE),yes)
	PROC_EVE_INCLUDE=yes
endif
ifeq ($(PROC_EVE4_INCLUDE),yes)
	PROC_EVE_INCLUDE=yes
endif
ifeq ($(PROC_EVE1_INCLUDE),yes)
	PROC_EVE_INCLUDE=yes
endif

PROC_DSP_INCLUDE = no
ifeq ($(PROC_DSP1_INCLUDE),yes)
	PROC_DSP_INCLUDE=yes
endif
ifeq ($(PROC_DSP2_INCLUDE),yes)
	PROC_DSP_INCLUDE=yes
endif

export TREAT_WARNINGS_AS_ERROR
export BUILD_OS
export BUILD_MACHINE
export PLATFORM
export CORE
export PROFILE_ipu1_0
export PROFILE_ipu1_1
export PROFILE_c66xdsp_1
export PROFILE_c66xdsp_2
export PROFILE_arp32_1
export PROFILE_arp32_2
export PROFILE_arp32_3
export PROFILE_arp32_4
export PROFILE_a15_0
export CODEGEN_PATH_M4
export CODEGEN_PATH_DSP
export CODEGEN_PATH_EVE
export CODEGEN_PATH_A15
export bios_PATH
export ipc_PATH
export kernel_PATH
export memcache_PATH
export kernel_addon_PATH
export uboot_PATH
export sgx_PATH
export xdc_PATH
export starterware_PATH
export edma_PATH
export edma_DRV_PATH
export edma_RM_PATH
export bsp_PATH
export ndk_PATH
export nsp_PATH
export avbtp_PATH
export evealg_PATH
export fc_PATH
export xdais_PATH
export hdvicplib_PATH
export jpegvdec_PATH
export jpegvenc_PATH
export h264venc_PATH
export h264vdec_PATH
export vlib_PATH
export vision_sdk_RELPATH
export vision_sdk_PATH
export ROOTDIR
export XDCPATH
export KW_BUILD
export DDR_MEM
export DEST_ROOT
export PROC_DSP1_INCLUDE
export PROC_DSP2_INCLUDE
export PROC_DSP_INCLUDE
export PROC_EVE1_INCLUDE
export PROC_EVE2_INCLUDE
export PROC_EVE3_INCLUDE
export PROC_EVE4_INCLUDE
export PROC_EVE_INCLUDE
export PROC_A15_0_INCLUDE
export PROC_IPU1_0_INCLUDE
export PROC_IPU1_1_INCLUDE
export NDK_PROC_TO_USE
export A15_TARGET_OS
export A15_TOOLCHAIN_PREFIX
export DEFAULT_UBOOT_CONFIG
export DEFAULT_KERNEL_CONFIG
export DEFAULT_DTB
export LINUX_BOOT_OUT_FILES
export LINUX_TARGETFS
export BSP_STW_PACKAGE_SELECT
export AVBRX_INCLUDE
export IVAHD_INCLUDE
export VPE_INCLUDE
export DSS_INCLUDE
export ISS_INCLUDE
export WDR_LDC_INCLUDE
export IPU1_EVELOADER_INCLUDE
export lane_detect_PATH
export object_detect_PATH
export hcf_PATH
export HCF_INCLUDE
export VSDK_BOARD_TYPE
export CPU_IDLE_ENABLED
export stereo_postprocess_PATH
export infoadas_PATH
export BUILD_DRA7XX
export DCAN_INCLUDE
export DUAL_A15_SMP_BIOS
export FAST_BOOT_INCLUDE
export CRC_INCLUDE

endif

ifeq ($(MAKERULEDIR), )
  MAKERULEDIR := $(ROOTDIR)/build/makerules
  export MAKERULEDIR
endif

include $(MAKERULEDIR)/build_config.mk
include $(MAKERULEDIR)/platform.mk
include $(MAKERULEDIR)/env.mk
include $(MAKERULEDIR)/component.mk

