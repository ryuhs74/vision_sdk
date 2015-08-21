#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# File name: build_ipc.mk
#            This file builds the BSP libs.

ipc_build:
ifeq ($(PLATFORM), $(filter $(PLATFORM), tda2xx-evm tda2xx-mc))
ifeq ($(A15_TARGET_OS), Bios)
	$(MAKE) -fipc-bios.mak -C $(ipc_PATH)/ $(IPC_TARGET) PLATFORM=DRA7XX IPCSRC=$(ipc_PATH) XDC_INSTALL_DIR=$(xdc_PATH) BIOS_INSTALL_DIR=$(bios_PATH) ti.targets.arm.elf.M4=$(CODEGEN_PATH_M4) ti.targets.elf.C66=$(CODEGEN_PATH_DSP) ti.targets.arp32.elf.ARP32=$(CODEGEN_PATH_EVE) gnu.targets.arm.A15F=$(CODEGEN_PATH_A15)
endif
ifeq ($(A15_TARGET_OS), Linux)
	$(MAKE) -fipc-bios.mak -C $(ipc_PATH)/ $(IPC_TARGET) PLATFORM=DRA7XX IPCSRC=$(ipc_PATH) XDC_INSTALL_DIR=$(xdc_PATH) BIOS_INSTALL_DIR=$(bios_PATH) ti.targets.arm.elf.M4=$(CODEGEN_PATH_M4) ti.targets.elf.C66=$(CODEGEN_PATH_DSP) ti.targets.arp32.elf.ARP32=$(CODEGEN_PATH_EVE)
endif
endif
ifeq ($(PLATFORM), tda3xx-evm)
	$(MAKE) -fipc-bios.mak -C $(ipc_PATH)/ $(IPC_TARGET) PLATFORM=TDA3XX IPCSRC=$(ipc_PATH) XDC_INSTALL_DIR=$(xdc_PATH) BIOS_INSTALL_DIR=$(bios_PATH) ti.targets.arm.elf.M4=$(CODEGEN_PATH_M4) ti.targets.elf.C66=$(CODEGEN_PATH_DSP) ti.targets.arp32.elf.ARP32=$(CODEGEN_PATH_EVE) 
endif
ifeq ($(PLATFORM), tda2ex-evm)
ifeq ($(A15_TARGET_OS), Bios)
	$(MAKE) -fipc-bios.mak -C $(ipc_PATH)/ $(IPC_TARGET) PLATFORM=DRA7XX IPCSRC=$(ipc_PATH) XDC_INSTALL_DIR=$(xdc_PATH) BIOS_INSTALL_DIR=$(bios_PATH) ti.targets.arm.elf.M4=$(CODEGEN_PATH_M4) ti.targets.elf.C66=$(CODEGEN_PATH_DSP) gnu.targets.arm.A15F=$(CODEGEN_PATH_A15)
endif
ifeq ($(A15_TARGET_OS), Linux)
	$(MAKE) -fipc-bios.mak -C $(ipc_PATH)/ $(IPC_TARGET) PLATFORM=DRA7XX IPCSRC=$(ipc_PATH) XDC_INSTALL_DIR=$(xdc_PATH) BIOS_INSTALL_DIR=$(bios_PATH) ti.targets.arm.elf.M4=$(CODEGEN_PATH_M4) ti.targets.elf.C66=$(CODEGEN_PATH_DSP) 
endif
endif

ipc:
	$(MAKE) -fbuild_ipc.mk ipc_build IPC_TARGET=release

ipc_clean:
	$(MAKE) -fbuild_ipc.mk ipc_build IPC_TARGET=clean



