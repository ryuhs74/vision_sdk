

include $(ROOTDIR)/build/makerules/platform.mk
include $(ROOTDIR)/build/makerules/env.mk

libs:
ifeq ($(PROC_IPU1_0_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=ipu1_0 
endif
ifeq ($(PROC_IPU1_1_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=ipu1_1
endif
ifeq ($(PROC_DSP1_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=c66xdsp_1 
endif
ifeq ($(PROC_DSP2_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=c66xdsp_2 
endif
ifeq ($(PROC_EVE1_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=arp32_1
endif
ifeq ($(PROC_EVE2_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=arp32_2
endif
ifeq ($(PROC_EVE3_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=arp32_3
endif
ifeq ($(PROC_EVE4_INCLUDE),yes)
	$(MAKE) $(TARGET) CORE=arp32_4
endif
ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
	$(MAKE) $(TARGET) CORE=a15_0
endif
endif

clean:
	$(MAKE) -fMAKEFILE.MK TARGET=clean

all: clean libs


