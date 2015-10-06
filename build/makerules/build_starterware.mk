#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

STARTERWARE_COMMON_OPTIONS=ROOTDIR=$(starterware_PATH) starterware_PATH=$(starterware_PATH) MAKERULEDIR=$(starterware_PATH)/build/makerules

# File name: build_starterware.mk
#            This file builds the starterware libs.

starterware:
	$(MAKE) -C $(starterware_PATH)/vpslib             $(TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0) INCLUDE_WDR_LDC=$(WDR_LDC_INCLUDE)
	$(MAKE) -C $(starterware_PATH)/drivers            $(TARGET) CORE=m4     PROFILE_m4=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/i2clib             $(TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/utils/common       $(TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/platform           $(TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/qspilib/qspi_flash $(TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/norflashlib        $(TARGET) CORE=m4vpss PROFILE_m4vpss=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/system_config      $(TARGET) CORE=m4 PROFILE_m4=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/pm/pmhal           $(TARGET) CORE=m4 PROFILE_m4=$(PROFILE_ipu1_0)
	$(MAKE) -C $(starterware_PATH)/pm/pmlib           $(TARGET) CORE=m4 PROFILE_m4=$(PROFILE_ipu1_0)
ifeq ($(PLATFORM), $(filter $(PLATFORM), tda3xx-evm))
	$(MAKE) -C $(starterware_PATH)                     sbl_lib  CORE=m4 PROFILE_m4=$(PROFILE_ipu1_0) 
endif

ifeq ($(PROC_A15_0_INCLUDE),yes)
ifeq ($(A15_TARGET_OS),Bios)
	$(MAKE) -C $(starterware_PATH)/system_config      $(TARGET) CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
	$(MAKE) -C $(starterware_PATH)/pm/pmhal           $(TARGET) CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
	$(MAKE) -C $(starterware_PATH)/pm/pmlib           $(TARGET) CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
	$(MAKE) -C $(starterware_PATH)/drivers            $(TARGET) CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
	$(MAKE) -C $(starterware_PATH)/platform           $(TARGET) CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
	$(MAKE) -C $(starterware_PATH)/qspilib/qspi_flash $(TARGET) CORE=a15_0 PROFILE_a15_0=$(PROFILE_a15_0)
endif
endif
ifeq ($(PROC_DSP_INCLUDE),yes)
	$(MAKE) -C $(starterware_PATH)/system_config      $(TARGET) CORE=c66x PROFILE_c66x=$(PROFILE_c66xdsp_1)
	$(MAKE) -C $(starterware_PATH)/pm/pmhal           $(TARGET) CORE=c66x PROFILE_c66x=$(PROFILE_c66xdsp_1)
	$(MAKE) -C $(starterware_PATH)/pm/pmlib           $(TARGET) CORE=c66x PROFILE_c66x=$(PROFILE_c66xdsp_1)
endif
ifeq ($(PROC_EVE_INCLUDE),yes)
	$(MAKE) -C $(starterware_PATH)/system_config      $(TARGET) CORE=arp32_1 PROFILE_arp32_1=$(PROFILE_arp32_1)
	$(MAKE) -C $(starterware_PATH)/pm/pmhal           $(TARGET) CORE=arp32_1 PROFILE_arp32_1=$(PROFILE_arp32_1)
	$(MAKE) -C $(starterware_PATH)/pm/pmlib           $(TARGET) CORE=arp32_1 PROFILE_arp32_1=$(PROFILE_arp32_1)
endif

# Uncomment below to build EDID programmer
#	$(MAKE) -fbuild_starterware.mk edid_programmer

starterware_clean:
	$(MAKE) -fbuild_starterware.mk starterware TARGET=clean
# Uncomment below to clean EDID programmer
#	$(MAKE) -fbuild_starterware.mk edid_programmer_clean

edid_programmer:
	$(MAKE) -C $(starterware_PATH)/ edid_programmer $(STARTERWARE_COMMON_OPTIONS)

edid_programmer_clean:
	$(MAKE) -C $(starterware_PATH)/ edid_programmer_clean $(STARTERWARE_COMMON_OPTIONS)

