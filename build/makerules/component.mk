#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File: component.mk
#       This file is component include make file of vision_sdk.

# List of variables set in this file and their purpose:
# <mod>_RELPATH        - This is the relative path of the module, typically from
#                        top-level directory of the package
# <mod>_PATH           - This is the absolute path of the module. It derives from
#                        absolute path of the top-level directory (set in env.mk)
#                        and relative path set above
# <mod>_INCLUDE        - This is the path that has interface header files of the
#                        module. This can be multiple directories (space separated)
# <mod>_PKG_LIST       - Names of the modules (and sub-modules) that are a part
#                        part of this module, including itself.
# <mod>_PLATFORM_DEPENDENCY - "yes": means the code for this module depends on
#                             platform and the compiled obj/lib has to be kept
#                             under <platform> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no platform dependent code and hence
#                             the obj/libs are not kept under <platform> dir.
# <mod>_CORE_DEPENDENCY     - "yes": means the code for this module depends on
#                             core and the compiled obj/lib has to be kept
#                             under <core> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no core dependent code and hence
#                             the obj/libs are not kept under <core> dir.
# <mod>_APP_STAGE_FILES     - List of source files that belongs to the module
#                             <mod>, but that needs to be compiled at application
#                             build stage (in the context of the app). This is
#                             primarily for link time configurations or if the
#                             source file is dependent on options/defines that are
#                             application dependent. This can be left blank or
#                             not defined at all, in which case, it means there
#                             no source files in the module <mod> that are required
#                             to be compiled in the application build stage.
#

vision_sdk_PKG_LIST = vision_sdk_lib
vision_sdk_RELPATH = src

#
# vision_sdk Modules
#

# vision_sdk BIOS libraries
vision_sdk_lib_RELPATH = src
vision_sdk_lib_PATH = $(vision_sdk_PATH)/$(vision_sdk_lib_RELPATH)
vision_sdk_lib_INCLUDE = $(vision_sdk_PATH)
vision_sdk_lib_PKG_LIST = vision_sdk_lib
vision_sdk_lib_PLATFORM_DEPENDENCY = yes
vision_sdk_lib_CORE_DEPENDENCY = yes

# Component specific CFLAGS
CFLAGS_COMP_COMMON +=


# CFLAGS based on platform selected

ifeq ($(PLATFORM),$(filter $(PLATFORM), tda2xx-evm tda2xx-mc))
 vision_sdk_CFLAGS = -DTDA2XX_BUILD -DTDA2XX_FAMILY_BUILD -DPLATFORM_EVM_SI
 vision_sdk_CFLAGS += -D_DRA7xx -D_SC_VER_1_16 -D__sysbios__
ifeq ($(DDR_MEM),DDR_MEM_256M)
 vision_sdk_CFLAGS += -DTDA2XX_256MB_DDR
endif
ifeq ($(DDR_MEM),DDR_MEM_1024M)
 vision_sdk_CFLAGS += -DTDA2XX_1024MB_DDR
endif
ifeq ($(DUAL_A15_SMP_BIOS),yes)
 vision_sdk_CFLAGS += -DDUAL_A15_SMP_BIOS_INCLUDE
endif
endif

ifeq ($(PLATFORM),tda3xx-evm)
 vision_sdk_CFLAGS = -DTDA3XX_BUILD -DTDA3XX_FAMILY_BUILD -DPLATFORM_EVM_SI 
 vision_sdk_CFLAGS += -D_TDA3x -D_SC_VER_1_16 -D__sysbios__
ifeq ($(DDR_MEM),DDR_MEM_64M)
 vision_sdk_CFLAGS += -DTDA3XX_64MB_DDR
endif
endif

ifeq ($(PLATFORM),tda2ex-evm)
 vision_sdk_CFLAGS = -DTDA2EX_BUILD -DTDA2XX_FAMILY_BUILD -DPLATFORM_EVM_SI
 vision_sdk_CFLAGS += -D_DRA7xx -D_SC_VER_1_16 -D__sysbios__
ifeq ($(DDR_MEM),DDR_MEM_1024M)
 vision_sdk_CFLAGS += -DTDA2EX_1024MB_DDR
endif
endif

vision_sdk_CFLAGS += -DUTILS_ASSERT_ENABLE -DFVID2_BACKWARD_COMPATIBLE
vision_sdk_CFLAGS += -DUSE_STD_ASSERT -DTRACE_ENABLE -DASSERT_ENABLE

ifeq ($(A15_TARGET_OS),Bios)
vision_sdk_CFLAGS += -DENABLE_UART
endif

#
#  vision_sdk_BOARD_TYPE definition
#

ifeq ($(vision_sdk_BOARD_TYPE), TDA2XX_EVM)
  vision_sdk_CFLAGS += -D_GNU_SOURCE
endif

ifeq ($(VSDK_BOARD_TYPE), TDA2XX_EVM)
  vision_sdk_CFLAGS += -DBOARD_TYPE_TDA2XX_EVM
endif

ifeq ($(VSDK_BOARD_TYPE), TDA2EX_EVM)
  vision_sdk_CFLAGS += -DBOARD_TYPE_TDA2EX_EVM
endif

ifeq ($(VSDK_BOARD_TYPE), TDA2XX_MC)
  vision_sdk_CFLAGS += -DBOARD_TYPE_TDA2XX_MC
endif

# Enable standard asserts for klockwork build
ifeq ($(KW_BUILD),yes)
  vision_sdk_CFLAGS += -D__KLOCWORK__
endif

ifeq ($(PROFILE_$(CORE)),debug)
  vision_sdk_CFLAGS += -DDEBUG_BUILD
endif

ifeq ($(PROFILE_$(CORE)),release)
  vision_sdk_CFLAGS += -DRELEASE_BUILD
endif

ifeq ($(PROFILE_$(CORE)),prod_release)
  vision_sdk_CFLAGS += -DPROD_RELEASE_BUILD
endif

ifeq ($(CORE),m3vpss)
  vision_sdk_CFLAGS += -DBUILD_M3
endif

ifeq ($(CORE),m3video)
  vision_sdk_CFLAGS += -DBUILD_M3
endif

ifeq ($(CORE),m4vpss)
  vision_sdk_CFLAGS += -DBUILD_M4
endif

ifeq ($(CORE),m4video)
  vision_sdk_CFLAGS += -DBUILD_M4
endif

ifeq ($(CORE),ipu1_0)
  vision_sdk_CFLAGS += -DBUILD_M4
  vision_sdk_CFLAGS += -DBUILD_M4_0
endif

ifeq ($(CORE),ipu1_1)
  vision_sdk_CFLAGS += -DBUILD_M4
  vision_sdk_CFLAGS += -DBUILD_M4_1
endif

ifeq ($(CORE),a8host)
  vision_sdk_CFLAGS += -DBUILD_A8
endif

ifeq ($(CORE),a15_0)
  vision_sdk_CFLAGS += -DBUILD_A15
endif

ifeq ($(CORE),c6xdsp)
  vision_sdk_CFLAGS += -DBUILD_DSP
endif

ifeq ($(CORE),c66x)
  vision_sdk_CFLAGS += -DBUILD_DSP
endif

ifeq ($(CORE),c66xdsp)
  vision_sdk_CFLAGS += -DBUILD_DSP
endif

ifeq ($(CORE),c66xdsp_1)
  vision_sdk_CFLAGS += -DBUILD_DSP
  vision_sdk_CFLAGS += -DBUILD_DSP_1
endif

ifeq ($(CORE),c66xdsp_2)
  vision_sdk_CFLAGS += -DBUILD_DSP
  vision_sdk_CFLAGS += -DBUILD_DSP_2
endif

ifeq ($(CORE),arp32_1)
  vision_sdk_CFLAGS += -DBUILD_ARP32
  vision_sdk_CFLAGS += -DBUILD_ARP32_1
endif

ifeq ($(CORE),arp32_2)
  vision_sdk_CFLAGS += -DBUILD_ARP32
  vision_sdk_CFLAGS += -DBUILD_ARP32_2
endif

ifeq ($(CORE),arp32_3)
  vision_sdk_CFLAGS += -DBUILD_ARP32
  vision_sdk_CFLAGS += -DBUILD_ARP32_3
endif

ifeq ($(CORE),arp32_4)
  vision_sdk_CFLAGS += -DBUILD_ARP32
  vision_sdk_CFLAGS += -DBUILD_ARP32_4
endif

ifeq ($(PROC_IPU1_0_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_IPU1_0_INCLUDE
endif

ifeq ($(PROC_IPU1_1_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_IPU1_1_INCLUDE
endif

ifeq ($(PROC_DSP1_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_DSP1_INCLUDE
endif

ifeq ($(PROC_DSP2_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_DSP2_INCLUDE
endif

ifeq ($(PROC_EVE1_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_EVE1_INCLUDE
endif

ifeq ($(PROC_EVE2_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_EVE2_INCLUDE
endif

ifeq ($(PROC_EVE3_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_EVE3_INCLUDE
endif

ifeq ($(PROC_EVE4_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPROC_EVE4_INCLUDE
endif

ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
  vision_sdk_CFLAGS += -DPROC_A15_0_INCLUDE
endif
endif

ifeq ($(NDK_PROC_TO_USE),ipu1_1)
  vision_sdk_CFLAGS += -DNDK_PROC_TO_USE_IPU1_1
endif

ifeq ($(NDK_PROC_TO_USE),ipu1_0)
  vision_sdk_CFLAGS += -DNDK_PROC_TO_USE_IPU1_0
endif

ifeq ($(NDK_PROC_TO_USE),a15_0)
  vision_sdk_CFLAGS += -DNDK_PROC_TO_USE_A15_0
endif

ifeq ($(AVBRX_INCLUDE),yes)
  vision_sdk_CFLAGS += -DAVBRX_INCLUDE
endif

ifeq ($(IVAHD_INCLUDE),yes)
  vision_sdk_CFLAGS += -DIVAHD_INCLUDE
endif

ifeq ($(DSS_INCLUDE),yes)
  vision_sdk_CFLAGS += -DDSS_INCLUDE
endif

ifeq ($(ISS_INCLUDE),yes)
  vision_sdk_CFLAGS += -DISS_INCLUDE
endif

ifeq ($(VPE_INCLUDE),yes)
  vision_sdk_CFLAGS += -DVPE_INCLUDE
endif

ifeq ($(HCF_INCLUDE),yes)
  vision_sdk_CFLAGS += -DHCF_INCLUDE
endif

ifeq ($(PM_FUNC_INCLUDE),yes)
  vision_sdk_CFLAGS += -DPM_FUNC_INCLUDE
endif

ifeq ($(DCAN_INCLUDE),yes)
  vision_sdk_CFLAGS += -DDCAN_INCLUDE
endif

ifeq ($(CRC_INCLUDE),yes)
  vision_sdk_CFLAGS += -DCRC_INCLUDE
endif

ifeq ($(FAST_BOOT_INCLUDE), yes)
  vision_sdk_CFLAGS += -DFAST_BOOT_INCLUDE
endif


export vision_sdk_CFLAGS

# Nothing beyond this point
