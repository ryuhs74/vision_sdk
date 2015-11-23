
MODULE_NAME = vision_sdk_alg_plugins

vision_sdk_alg_plugins_PLATFORM_DEPENDENCY = yes
vision_sdk_alg_plugins_CORE_DEPENDENCY = no

INCDIR = .

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = bios xdc ipc starterware bsp vision_sdk fc xdais jpegvdec jpegvenc edma vlib lane_detect object_detect evealg stereo_postprocess bsp_examples_utility

# List all the internal submodules whose interface header files
# need to be included for this module
INCLUDE_INTERNAL_INTERFACES = starterware_hal starterware_pm_hal

ifeq ($(HCF_INCLUDE),yes)
INCLUDE_EXTERNAL_INTERFACES += hcf
endif

ifeq ($(DCAN_INCLUDE),yes)
INCLUDE_EXTERNAL_INTERFACES += starterware starterware_hal sys_config uartConsole utils_platform
endif

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON =
SRCS_tda2xx =
CFLAGS_LOCAL_COMMON = $(vision_sdk_CFLAGS)

MODULE_SRC_BASE_PATH := $(vision_sdk_PATH)/examples/tda2xx/src/alg_plugins

SRCDIR  =

include $(MODULE_SRC_BASE_PATH)/common/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/softisp/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/remapmerge/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/disparityhamdist/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/census/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/stereo_postprocessing/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/colortogray/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/denseopticalflow/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/edgedetection/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/framecopy/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/fullView/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/surroundview/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/objectdetection/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/sparseopticalflow/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/lanedetection/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/subframecopy/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/dmaSwMs/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/surroundViewCammsys/SRC_FILES.MK
ifeq ($(CRC_INCLUDE),yes)
include $(MODULE_SRC_BASE_PATH)/crc/SRC_FILES.MK
endif
include $(vision_sdk_PATH)/examples/tda2xx/src/draw2d/SRC_FILES.MK
include $(vision_sdk_PATH)/examples/tda2xx/src/links/network_ctrl/SRC_FILES.MK

ifeq ($(DCAN_INCLUDE),yes)
include $(vision_sdk_PATH)/examples/tda2xx/src/links/dcan/SRC_FILES.MK
endif

ifeq ($(HCF_INCLUDE),yes)
include $(vision_sdk_PATH)/examples/tda2xx/src/hcf/SRC_FILES.MK
endif

ifeq ($(ISS_INCLUDE),yes)
include $(MODULE_SRC_BASE_PATH)/iss_aewb1/SRC_FILES.MK
endif

# Core/SoC/platform specific source files and CFLAGS
# Example:
#   SRCS_<core/SoC/platform-name> =
#   CFLAGS_LOCAL_<core/SoC/platform-name> =

# Include common make files
include $(ROOTDIR)/build/makerules/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
