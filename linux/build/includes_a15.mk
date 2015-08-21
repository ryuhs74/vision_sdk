# (c) Texas Instruments

ifndef $(INCLUDES_MK)
INCLUDES_MK = 1


OSA_KERMOD_INC=-I$(memcache_PATH)/include -I$(memcache_PATH)/include/osa

LINUX_COMMON_INC = -I$(vision_sdk_PATH)/linux/include

LINK_API_INC = -I$(vision_sdk_PATH)/include/link_api

COMMON_INC= -I. -I$(vision_sdk_PATH)

OSA_INC = -I$(vision_sdk_PATH)/linux/src/osa/include

SGX_DRM_INC = -I$(LINUX_TARGETFS)/usr/include/libdrm -I$(LINUX_TARGETFS)/usr/include -I$(LINUX_TARGETFS)/usr/include/EGL

COMMON_LFLAGS = -L$(LINUX_TARGETFS)/usr/lib -Wl,--rpath-link,$(LINUX_TARGETFS)/usr/lib -L$(LINUX_TARGETFS)/lib -Wl,--rpath-link,$(LINUX_TARGETFS)/lib

PLAT_LINK =  $(COMMON_LFLAGS) -lEGL -lGLESv2 -lgbm -ldrm -ldrm_omap

DEFINE += -DA15_TARGET_OS_LINUX -DLINUX_BUILD

ifeq ($(PROC_IPU1_0_INCLUDE),yes)
  DEFINE += -DPROC_IPU1_0_INCLUDE
endif

ifeq ($(PROC_IPU1_1_INCLUDE),yes)
  DEFINE += -DPROC_IPU1_1_INCLUDE
endif

ifeq ($(PROC_DSP1_INCLUDE),yes)
  DEFINE += -DPROC_DSP1_INCLUDE
endif

ifeq ($(PROC_DSP2_INCLUDE),yes)
  DEFINE += -DPROC_DSP2_INCLUDE
endif

ifeq ($(PROC_EVE1_INCLUDE),yes)
  DEFINE += -DPROC_EVE1_INCLUDE
endif

ifeq ($(PROC_EVE2_INCLUDE),yes)
  DEFINE += -DPROC_EVE2_INCLUDE
endif

ifeq ($(PROC_EVE3_INCLUDE),yes)
  DEFINE += -DPROC_EVE3_INCLUDE
endif

ifeq ($(PROC_EVE4_INCLUDE),yes)
  DEFINE += -DPROC_EVE4_INCLUDE
endif

ifeq ($(PROC_A15_0_INCLUDE),yes)
  DEFINE += -DPROC_A15_0_INCLUDE
endif


endif # ifndef $(INCLUDES_MK)

