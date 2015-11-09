
MODULE_NAME = vision_sdk_lib


INCDIR = .

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = bios xdc ipc starterware bsp vision_sdk fc xdais jpegvdec jpegvenc h264venc h264vdec edma vlib avbtp ndk nsp sbl

# List all the internal submodules whose interface header files
# need to be included for this module
INCLUDE_INTERNAL_INTERFACES = starterware_hal utils_platform qspi_flashlib starterware_vpslib sys_config starterware_pm_hal starterware_pm_lib sbl_lib norflash_lib

MODULE_SRC_BASE_PATH := $(vision_sdk_PATH)/src

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON =
CFLAGS_LOCAL_COMMON = $(vision_sdk_CFLAGS)


SRCDIR = utils_common links_ipu links_common links_dsp links_eve links_a15

include $(MODULE_SRC_BASE_PATH)/utils_common/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/links_common/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/links_ipu/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/links_dsp/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/links_eve/SRC_FILES.MK
include $(MODULE_SRC_BASE_PATH)/links_a15/SRC_FILES.MK


# Core/SoC/platform specific source files and CFLAGS
# Example:
#   SRCS_<core/SoC/platform-name> =
#   CFLAGS_LOCAL_<core/SoC/platform-name> =

# Include common make files
include $(ROOTDIR)/build/makerules/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
