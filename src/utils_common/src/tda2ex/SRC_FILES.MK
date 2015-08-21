
SRCDIR += utils_common/src/tda2ex

SRCS_COMMON += utils_clk.c \

SRCS_ipu1_0 += utils_l3_emif_bw.c \


SRCS_ipu1_1 += \

ifeq ($(A15_TARGET_OS),Linux)
SRCS_ipu1_0 += utils_prcm.c
endif

