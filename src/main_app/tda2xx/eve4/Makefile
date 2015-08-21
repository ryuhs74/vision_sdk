
APP_NAME = vision_sdk

SRCDIR = src
INCDIR = .

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = bios xdc starterware bsp ipc vision_sdk fc xdais edma

# List all the components required by the application
COMP_LIST_arp32_4  = bios xdc ipc fc vision_sdk

# XDC CFG File
XDC_CFG_FILE_arp32_4  = MAIN_APP_eve4.cfg

# Common source files and CFLAGS across all platforms and cores
SRCS_arp32_4  = main_eve4.c tlb_config_eve4.c

CFLAGS_LOCAL_arp32_4 = $(vision_sdk_CFLAGS)

# Core/SoC/platform specific source files and CFLAGS
# Example:
#   SRCS_<core/SoC/platform-name> =
#   CFLAGS_LOCAL_<core/SoC/platform-name> =

# Include common make files
include $(ROOTDIR)/build/makerules/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
