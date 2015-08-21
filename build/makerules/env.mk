#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File: env.mk. This file contains all the paths and other ENV variables

#
# Module paths
#

# Destination root directory.
#   - specify the directory where you want to place the object, archive/library,
#     binary and other generated files in a different location than source tree
#   - or leave it blank to place then in the same tree as the source
DEST_ROOT = $(vision_sdk_PATH)/binaries

# Utilities directory. This is required only if the build machine is Windows.
#   - specify the installation directory of utility which supports POSIX commands
#     (eg: Cygwin installation or MSYS installation).
UTILS_INSTALL_DIR := $(xdc_PATH)

# Set path separator, etc based on the OS
ifeq ($(OS),Windows_NT)
  PATH_SEPARATOR = ;
  UTILSPATH = $(UTILS_INSTALL_DIR)/bin/
else
  # else, assume it is linux
  PATH_SEPARATOR = :
  UTILSPATH = /bin/
endif

# BIOS
bios_INCLUDE = $(bios_PATH)/packages
export bios_INCLUDE

# IPC
ipc_INCLUDE = $(ipc_PATH)/packages
export ipc_INCLUDE

# XDC
xdc_INCLUDE = $(xdc_PATH)/packages
export xdc_INCLUDE

# BSP drivers
bsp_INCLUDE = $(bsp_PATH)/include
PACKAGE_SELECT=$(BSP_STW_PACKAGE_SELECT)
export PACKAGE_SELECT
include $(bsp_PATH)/build/makerules/component.mk
export bsp_INCLUDE

vision_sdk_INCLUDE = $(vision_sdk_PATH)
include $(vision_sdk_PATH)/build/makerules/component.mk
export vision_sdk_INCLUDE

#FC include
fc_INCLUDE = $(fc_PATH)/packages
export fc_INCLUDE

#XDAIS include
xdais_INCLUDE = $(xdais_PATH)/packages
export xdais_INCLUDE

#JPEGVDEC include
jpegvdec_INCLUDE = $(jpegvdec_PATH)/packages
export jpegvdec_INCLUDE

#JPEGVENC include
jpegvenc_INCLUDE = $(jpegvenc_PATH)/packages
export jpegvenc_INCLUDE

#H264VENC include
h264venc_INCLUDE = $(h264venc_PATH)/packages
export h264venc_INCLUDE

#H264VDEC include
h264vdec_INCLUDE = $(h264vdec_PATH)/packages $(h264vdec_PATH)/packages/ti/sdo/codecs/h264vdec/
export h264vdec_INCLUDE

#HDVICP API
hdvicpapi_INCLUDE = $(hdvicplib_PATH)/packages
export hdvicpapi_INCLUDE

# DSP Algos
lane_detect_INCLUDE = $(lane_detect_PATH)/modules/ti_lane_detection/inc
export lane_detect_INCLUDE

object_detect_INCLUDE = $(object_detect_PATH)/modules/ti_object_detection/inc
export object_detect_INCLUDE

stereo_postprocess_INCLUDE = $(stereo_postprocess_PATH)/modules/ti_stereovision/inc
export stereo_postprocess_INCLUDE

# EVE SW
evealg_INCLUDE = $(evealg_PATH)/ $(evealg_PATH)/common \
			$(evealg_PATH)/apps/apps_nonbam/inc \
			$(evealg_PATH)/apps/fast9_best_feature_to_front/algo/inc \
			$(evealg_PATH)/apps/pyramid_lk_tracker/algo/inc \
			$(evealg_PATH)/apps/remap_merge/algo/inc \
			$(evealg_PATH)/kernels/vlib/vcop_remap/inc \
			$(evealg_PATH)/apps/ti_pd_feature_plane_computation/algo/inc \
			$(evealg_PATH)/apps/yuv_scalar/algo/inc \
			$(evealg_PATH)/apps/filter_2d/algo/inc \
			$(evealg_PATH)/apps/yuv_padding/algo/inc \
			$(evealg_PATH)/apps/bin_image_to_list/algo/inc/ \
			$(evealg_PATH)/apps/harrisCornerDetection32/algo/inc/

export evealg_INCLUDE

# HCF
hcf_INCLUDE = $(hcf_PATH)/hcf/include $(hcf_PATH)/hcf/source $(hcf_PATH)/hcf/source/include $(hcf_PATH)/sosal/include $(vision_sdk_PATH)/examples/tda2xx/src/hcf/
export hcf_INCLUDE

# EDMA
edma_INCLUDE = $(edma_PATH)/packages
export edma_INCLUDE

vlib_INCLUDE = $(vlib_PATH)/packages
export vlib_INCLUDE

ndk_INCLUDE = $(ndk_PATH)/packages
export ndk_INCLUDE

nsp_INCLUDE = $(nsp_PATH)/packages
export nsp_INCLUDE

# AVBTP drivers
avbtp_INCLUDE = $(avbtp_PATH)/packages
export avbtp_INCLUDE

# Starterware
starterware_INCLUDE = $(starterware_PATH)/include
PACKAGE_SELECT=$(BSP_STW_PACKAGE_SELECT)
export PACKAGE_SELECT
include $(starterware_PATH)/build/makerules/component.mk
export starterware_INCLUDE

#sbl
sbl_INCLUDE =  $(starterware_PATH)/include/armv7m/tda2xx $(starterware_PATH)/bootloader/sbl/include $(starterware_PATH)/system_config/tda2xx_common $(starterware_PATH)/system_config/armv7m/tda2xx $(starterware_PATH)/utils/uart_console $(starterware_PATH)/include/armv7m
export sbl_INCLUDE

# Package Name
PACKAGE_NAME = $(vision_sdk_RELPATH)
export PACKAGE_NAME

#
# Tools paths
#

# Cortex-A8
CODEGEN_PATH_A8 =

# DSP
CODEGEN_PATH_C674 = $(CODEGEN_PATH_DSP)


# Commands commonly used within the make files
RM = $(UTILSPATH)rm
RMDIR = $(UTILSPATH)rm -rf
MKDIR = $(UTILSPATH)mkdir
ECHO = @$(UTILSPATH)echo

#emake provides detailed info for build analysis.
EMAKE = emake --emake-emulation=gmake --emake-gen-subbuild-db=1 --emake-annodetail=waiting

ifeq ($(OS),Windows_NT)
  MAKE = gmake
#  MAKE =  $(EMAKE)
else
  MAKE = make
endif
EGREP = $(UTILSPATH)egrep
CP = $(UTILSPATH)cp
ifeq ($(OS),Windows_NT)
  CHMOD = $(UTILSPATH)echo
else
  CHMOD = $(UTILSPATH)chmod
endif

ifeq ($(OS),Windows_NT)
  TOUCH=$(xdc_PATH)/bin/touch
else
  TOUCH=touch
endif

#
# XDC specific ENV variables
#
# XDC Config.bld file (required for configuro); Derives from top-level vision_sdk_PATH
ifeq ($(SOCFAMILY),tda2xx)
 ifeq ($(SOC),tda2xx)
  ifeq ($(CONFIG_BLD_XDC_m4),)
    CONFIG_BLD_XDC_m4     = $(vision_sdk_PATH)/build/tda2xx/config_m4.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_66),)
    CONFIG_BLD_XDC_66     = $(vision_sdk_PATH)/build/tda2xx/config_c66.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_arp32),)
    CONFIG_BLD_XDC_arp32 = $(vision_sdk_PATH)/build/tda2xx/config_arp32.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_a15),)
    CONFIG_BLD_XDC_a15 = $(vision_sdk_PATH)/build/tda2xx/config_a15.bld
  endif
 endif
 ifeq ($(SOC),tda2ex)
  ifeq ($(CONFIG_BLD_XDC_m4),)
    CONFIG_BLD_XDC_m4     = $(vision_sdk_PATH)/build/tda2ex/config_m4.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_66),)
    CONFIG_BLD_XDC_66     = $(vision_sdk_PATH)/build/tda2ex/config_c66.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_a15),)
    CONFIG_BLD_XDC_a15 = $(vision_sdk_PATH)/build/tda2ex/config_a15.bld
  endif
 endif
endif

ifeq ($(SOCFAMILY),tda3xx)
  ifeq ($(CONFIG_BLD_XDC_m4),)
    CONFIG_BLD_XDC_m4     = $(vision_sdk_PATH)/build/tda3xx/config_m4.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_66),)
    CONFIG_BLD_XDC_66     = $(vision_sdk_PATH)/build/tda3xx/config_c66.bld
  endif
  ifeq ($(CONFIG_BLD_XDC_arp32),)
    CONFIG_BLD_XDC_arp32 = $(vision_sdk_PATH)/build/tda3xx/config_arp32.bld
  endif
endif

XDCROOT = $(xdc_PATH)
XDCTOOLS = $(xdc_PATH)
BIOSROOT = $(bios_PATH)
export BIOSROOT
export XDCROOT
export XDCTOOLS

CGTOOLS = $(CODEGEN_PATH_M4)
export CGTOOLS

CGTOOLS_DSP = $(CODEGEN_PATH_C674)
export CGTOOLS_DSP

CODESOURCERYCGTOOLS = $(CODEGEN_PATH_A8)
export CODESOURCERYCGTOOLS

CGTOOLS_EVE = $(CODEGEN_PATH_EVE)
export CGTOOLS_EVE

CGTOOLS_A15 = $(CODEGEN_PATH_A15)
export CGTOOLS_A15

STRIP470 = $(CODEGEN_PATH_M4)/bin/strip470 -p
STRIP6x = $(CODEGEN_PATH_C674)/bin/strip6x -p
STRIP_ALL_ARM = $(CODEGEN_PREFIX)strip -s
STRIP_DEBUG_ARM = $(CODEGEN_PREFIX)strip --strip-debug

PATH += $(PATH_SEPARATOR)$(xdc_PATH)$(PATH_SEPARATOR)$(CODEGEN_PATH_M4)/bin$(PATH_SEPARATOR)$(CODEGEN_PATH_C674)/bin$(PATH_SEPARATOR)$(CODEGEN_PATH_EVE)/bin$(PATH_SEPARATOR)$(CODEGEN_PATH_A15)/bin$(PATH_SEPARATOR)
export PATH

# Nothing beyond this point
