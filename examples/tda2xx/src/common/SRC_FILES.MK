
SRCDIR += $(vision_sdk_PATH)/examples/tda2xx/src/common

ifeq ($(A15_TARGET_OS), Bios)
SRCS_ipu1_0 += \
		chains_main_bios.c 
endif

ifeq ($(A15_TARGET_OS), Linux)
SRCS_ipu1_0 += \
		chains_main_linux.c \
		chains_main_srv_calibration.c
endif
