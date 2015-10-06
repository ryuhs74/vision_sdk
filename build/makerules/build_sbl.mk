#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_sbl.mk
#            This file builds the SBL from starterware package

#
# TDA2XX SBL options,
# -------------------
#
# PLATFORM = tda2xx (TDA2xx EVM) or tda2xx-mc (TDA2xx MonsterCam)
#
# OPPMODE  = opp_nom (default) or opp_od or opp_high
#
# EMIFMODE = DUAL_EMIF_1GB_512MB (default) or DUAL_EMIF_2X512MB  or SINGLE_EMIF_256MB
# NOTE:
# DUAL_EMIF_1GB_512MB : uses EMIF2 - 1GB and EMIF1 512MB, BUT in non-interleaved mode
#                       Vision SDK uses only 1GB hence effectivly this is single EMIF
#                       non-interleaved mode for Vision SDK
# DUAL_EMIF_2X512MB   : uses EMIF1 and EMIF 512MB each in interleaved mode
# SINGLE_EMIF_256MB   : uses EMIF1 256MB each in non-interleaved mode
# PM_OPTIMIZE     : SBL PM Optimization flag enabled/ disable
#
# BOOTMODE = sd or qspi or nor
# NOTE:
# qspi, nor boot mode is not supported in tda2xx-mc (MonsterCam)
#
#
# TDA3XX SBL options,
# -------------------
# BOOTMODE = qspi
#

SBL_COMMON_OPTIONS= ROOTDIR=$(starterware_PATH) starterware_PATH=$(starterware_PATH) MAKERULEDIR=$(starterware_PATH)/build/makerules UTILS_INSTALL_DIR=$(xdc_PATH)/bin


SBL_TDA2XX_OPTIONS= $(SBL_COMMON_OPTIONS) PLATFORM=tda2xx EMIFMODE=DUAL_EMIF_1GB_512MB FORCE_OPPMODE=TRUE OPPMODE=opp_nom PM_OPTIMIZE=TRUE
SBL_TDA2EX_OPTIONS= $(SBL_COMMON_OPTIONS) PLATFORM=tda2ex EMIFMODE=DUAL_EMIF_1GB_512MB FORCE_OPPMODE=TRUE OPPMODE=opp_nom PM_OPTIMIZE=TRUE
SBL_TDA2XX_MC_OPTIONS= $(SBL_COMMON_OPTIONS) PLATFORM=tda2xx-mc EMIFMODE=DUAL_EMIF_1GB_512MB FORCE_OPPMODE=TRUE OPPMODE=opp_nom PM_OPTIMIZE=TRUE
SBL_TDA3XX_OPTIONS= $(SBL_COMMON_OPTIONS)
ifeq ($(FAST_BOOT_INCLUDE), yes)
SBL_TDA3XX_OPTIONS+= SBL_CONFIG=disable_safety SBL_BUILD_MODE=prod SBL_OPT_MODE=high
endif


sbl_sd:
ifeq ($(VSDK_BOARD_TYPE),TDA2XX_EVM)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=sd  $(SBL_TDA2XX_OPTIONS) 
endif
ifeq ($(VSDK_BOARD_TYPE),TDA2XX_MC)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=sd  $(SBL_TDA2XX_MC_OPTIONS) 
endif
ifeq ($(VSDK_BOARD_TYPE),TDA2EX_EVM)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=sd  $(SBL_TDA2EX_OPTIONS) 
endif

sbl_qspi:
ifeq ($(VSDK_BOARD_TYPE),TDA2XX_EVM)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=qspi  $(SBL_TDA2XX_OPTIONS)
	$(MAKE) -C $(starterware_PATH) qspiFlashWriter  $(SBL_TDA2XX_OPTIONS)
endif
ifeq ($(PLATFORM),tda3xx-evm)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=qspi $(SBL_TDA3XX_OPTIONS) 
	$(MAKE) -C $(starterware_PATH) qspiFlashWriter  $(SBL_TDA3XX_OPTIONS)
endif
ifeq ($(VSDK_BOARD_TYPE),TDA2XX_MC)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=qspi  $(SBL_TDA2XX_MC_OPTIONS)
	$(MAKE) -C $(starterware_PATH) qspiFlashWriter  $(SBL_TDA2XX_MC_OPTIONS)
endif
ifeq ($(VSDK_BOARD_TYPE),TDA2EX_EVM)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=qspi $(SBL_TDA2EX_OPTIONS) 
	$(MAKE) -C $(starterware_PATH) qspiFlashWriter  $(SBL_TDA2EX_OPTIONS)
endif

sbl_qspi_sd:
ifeq ($(PLATFORM),tda3xx-evm)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=qspi_sd $(SBL_TDA3XX_OPTIONS) 
	$(MAKE) -C $(starterware_PATH) qspiFlashWriter  $(SBL_TDA3XX_OPTIONS)
endif

sbl_nor:
ifeq ($(VSDK_BOARD_TYPE),TDA2XX_EVM)
	$(MAKE) -C $(starterware_PATH) sbl BOOTMODE=nor  $(SBL_TDA2XX_OPTIONS)
	$(MAKE) -C $(starterware_PATH) nor_flash_writer  $(SBL_TDA2XX_OPTIONS)
endif

sbl_nor_updater:
ifeq ($(VSDK_BOARD_TYPE),TDA2XX_EVM)
	$(MAKE) -C $(starterware_PATH) nor_flash_updater  $(SBL_TDA2XX_OPTIONS)
endif

sbl_clean:
ifeq ($(PLATFORM),$(filter $(PLATFORM), tda2xx-evm tda2xx-mc))
	$(MAKE) -C $(starterware_PATH) sbl_all_clean  $(SBL_TDA2XX_OPTIONS)
	$(MAKE) -C $(starterware_PATH) sbl_all_clean  $(SBL_TDA2XX_MC_OPTIONS) 
	$(MAKE) -C $(starterware_PATH) qspiFlashWriter_clean  $(SBL_TDA2XX_OPTIONS) 
	$(MAKE) -C $(starterware_PATH) nor_flash_writer_clean  $(SBL_TDA2XX_OPTIONS) 
	$(MAKE) -C $(starterware_PATH) nor_flash_updater_clean  $(SBL_TDA2XX_OPTIONS) 
endif
ifeq ($(PLATFORM),tda3XX-evm)
	$(MAKE) -C $(starterware_PATH) sbl_all_clean $(SBL_TDA3XX_OPTIONS) 
endif

