#*******************************************************************************
#                                                                              *
# Copyright (c) 2013 Texas Instruments Incorporated - http://www.ti.com/       *
#                        ALL RIGHTS RESERVED                                   *
#                                                                              *
#*******************************************************************************

# Filename: rules_arp32f.mk
#
# Make rules for EVE - This file has all the common rules and defines required
#                     for EVE ISA
#
# This file needs to change when:
#     1. Code generation tool chain changes (currently it uses arp32cgt_1.0.0)
#     2. Internal switches (which are normally not touched) has to change
#     3. XDC specific switches change
#     4. a rule common for EVE ISA has to be added or modified

# Set compiler/archiver/linker commands and include paths
CODEGEN_INCLUDE = $(CODEGEN_PATH_EVE)/include
CC = $(CODEGEN_PATH_EVE)/bin/cl-arp32
AR = $(CODEGEN_PATH_EVE)/bin/ar-arp32
LNK = $(CODEGEN_PATH_EVE)/bin/lnk-arp32


# Derive a part of RTS Library name based on ENDIAN: little/big
ifeq ($(ENDIAN),little)
  RTSLIB_ENDIAN =
else
  RTSLIB_ENDIAN = e
endif

# Derive compiler switch and part of RTS Library name based on FORMAT: COFF/ELF
ifeq ($(FORMAT),COFF)
  #CSWITCH_FORMAT = ti_arm9_abi
  #RTSLIB_FORMAT = _tiarm9

  CSWITCH_FORMAT =
  #RTSLIB_FORMAT =
  XDCINTERNAL_DEFINES = -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__vers_1_0_7_0_0
endif
ifeq ($(FORMAT),ELF)
  #CSWITCH_FORMAT = eabi
  #RTSLIB_FORMAT = _elf

  CSWITCH_FORMAT =
  RTSLIB_FORMAT = _elf
  XDCINTERNAL_DEFINES = -Dxdc_target_types__=ti/targets/arp32/elf/std.h -Dxdc_bld__vers_1_0_7_2_0_10271
endif

#########

# XDC Specific defines
ifneq ($(XDC_CFG_FILE_$(CORE)),)
  ifeq ($(PROFILE_$(CORE)),debug)
    CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
	CFG_LNKFILENAMEPART_XDC=
  endif
  ifeq ($(PROFILE_$(CORE)),release)
    CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
  endif
  ifeq ($(PROFILE_$(CORE)),whole_program_debug)
    CFG_CFILENAMEPART_XDC =p$(FORMAT_EXT)$(ISA)$(ENDIAN_EXT)
    CFG_LNKFILENAMEPART_XDC=_x
  endif
  CFG_CFILE_XDC =$(patsubst %.cfg,%_$(CFG_CFILENAMEPART_XDC).c,$(notdir $(XDC_CFG_FILE_$(CORE))))
  CFG_C_XDC = $(addprefix $(CONFIGURO_DIR)/package/cfg/,$(CFG_CFILE_XDC))
  CFG_C_NEW_XDC = $(CONFIGURO_DIR)/package/cfg
  XDCLNKCMD_FILE =$(patsubst %.c, %$(CFG_LNKFILENAMEPART_XDC).xdl, $(CFG_C_XDC))
  CFG_COBJ_XDC = $(patsubst %.c,%.$(OBJEXT),$(CFG_CFILE_XDC))
  LNKCMD_FILE = $(CONFIGURO_DIR)/linker_mod.cmd
  SPACE :=
  SPACE +=
  XDC_GREP_STRING = $(CONFIGURO_DIRNAME)
endif


#########

MISC_LNKCMD_INCLUDE=$(vision_sdk_PATH)/src/main_app/tda2xx/cfg/link_eve.cmd

# Internal CFLAGS - normally doesn't change
CFLAGS_INTERNAL =  -c -qq -pdsw225 $(CSWITCH_FORMAT) -eo.$(OBJEXT) -ea.$(ASMEXT) -DBIOS_BUILD -DSYSBIOS
CFLAGS_DIROPTS = -fr=$(OBJDIR) -fs=$(OBJDIR)
CFLAGS_NEW_DIROPTS = -fr=$(CFG_C_NEW_XDC) -fs=$(CFG_C_NEW_XDC)


ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)

CFLAGS_INTERNAL += --emit_warnings_as_errors

endif

CFLAGS_INTERNAL += --silicon_version=v210

KFLAGS= -kh -kv --gen_func_subsections

ifeq ($(CPU_IDLE_ENABLED), yes)
CFLAGS_INTERNAL += -DCPU_IDLE_ENABLED
endif

#########XDC Config File for EVE##########

# CFLAGS based on profile selected
ifeq ($(PROFILE_$(CORE)), release)
 CFLAGS_INTERNAL += --symdebug:dwarf -O3
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=ARP32_far  -Dxdc_bld__profile_debug -D_DEBUG_=1
 ifndef MODULE_NAME
  CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_CFG_BASE_FILE_NAME)_pearp32F.h'
 endif
 LNKFLAGS_INTERNAL_PROFILE = -o2
endif


# CFLAGS based on profile selected
ifeq ($(PROFILE_$(CORE)), debug)
 CFLAGS_INTERNAL += --symdebug:dwarf
 CFLAGS_XDCINTERNAL = -Dxdc_target_name__=ARP32_far  -Dxdc_bld__profile_debug -D_DEBUG_=1
 ifndef MODULE_NAME
  CFLAGS_XDCINTERNAL += -Dxdc_cfg__header__='$(CONFIGURO_DIR)/package/cfg/$(XDC_CFG_BASE_FILE_NAME)_pearp32F.h'
 endif
 LNKFLAGS_INTERNAL_PROFILE =
endif


# For generic platform define GENERIC in CFLAGS
ifeq ($(PLATFORM),generic)
 CFLAGS_XDCINTERNAL += -DGENERIC
endif
CFLAGS_XDCINTERNAL += $(XDCINTERNAL_DEFINES)
CFLAGS_XDCINTERNAL += --silicon_version=v210
########################################

ifeq ($(A15_TARGET_OS),Linux)
CFLAGS_INTERNAL += -DA15_TARGET_OS_LINUX
endif

ifeq ($(A15_TARGET_OS),Bios)
CFLAGS_INTERNAL += -DA15_TARGET_OS_BIOS
endif

ifeq ($(IPU1_EVELOADER_INCLUDE), yes)
CFLAGS_INTERNAL += -DIPU1_LOAD_EVES
endif



# Following 'if...' block is for an application; to add a #define for each
#   component in the build. This is required to know - at compile time - which
#   components are on which core.
ifndef MODULE_NAME
  # Derive list of all packages from each of the components needed by the app
  PKG_LIST_ARP32_LOCAL = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))

  # Defines for the app and cfg source code to know which components/packages
  # are included in the build for the local CORE...
  CFLAGS_APP_DEFINES = $(foreach PKG,$(PKG_LIST_ARP32_LOCAL),-D_LOCAL_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_ARP32_LOCAL),-D_BUILD_$(PKG)_)

  ifeq ($(CORE),arp32_1)
    PKG_LIST_ARP32_REMOTE = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_$(CORE)_
  endif
  ifeq ($(CORE),arp32_2)
    PKG_LIST_ARP32_REMOTE = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_$(CORE)_
  endif
  ifeq ($(CORE),arp32_3)
    PKG_LIST_ARP32_REMOTE = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_$(CORE)_
  endif
  ifeq ($(CORE),arp32_4)
    PKG_LIST_ARP32_REMOTE = $(foreach COMP,$(COMP_LIST_$(CORE)),$($(COMP)_PKG_LIST))
    CFLAGS_APP_DEFINES += -D_LOCAL_CORE_$(CORE)_
  endif
  PKG_LIST_A8_REMOTE = $(foreach COMP,$(COMP_LIST_a8host),$($(COMP)_PKG_LIST))

  # Defines for the app and cfg source code to know which components/packages
  # are included in the build for the remote CORE...
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_ARP32_REMOTE),-D_REMOTE_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_ARP32_REMOTE),-D_BUILD_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_A8_REMOTE),-D_REMOTE_$(PKG)_)
  CFLAGS_APP_DEFINES += $(foreach PKG,$(PKG_LIST_A8_REMOTE),-D_BUILD_$(PKG)_)
endif

# Assemble CFLAGS from all other CFLAGS definitions
_CFLAGS = $(CFLAGS_INTERNAL) $(CFLAGS_GLOBAL_$(CORE)) $(CFLAGS_XDCINTERNAL) $(CFLAGS_LOCAL_COMMON) $(CFLAGS_LOCAL_$(CORE)) $(CFLAGS_LOCAL_$(PLATFORM)) $(CFLAGS_LOCAL_$(SOCFAMILY)) $(CFLAGS_LOCAL_$(SOC)) $(CFLAGS_APP_DEFINES) $(CFLAGS_COMP_COMMON) $(CFLAGS_GLOBAL_$(PLATFORM))

#Add some additional include paths
INCLUDES +=

# Object file creation
# The first $(CC) generates the dependency make files for each of the objects
# The second $(CC) compiles the source to generate object
$(OBJ_PATHS): $(OBJDIR)/%.$(OBJEXT): %.c
	$(ECHO) \# Compiling $(PLATFORM):$(CORE):$(PROFILE_$(CORE)):$(APP_NAME)$(MODULE_NAME): $<
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fc $<
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) -fc $<

$(OBJ_PATHS_CPP): $(OBJDIR)/%.$(OBJEXT): %.cpp
	$(ECHO) \# Compiling $(PLATFORM):$(CORE):$(PROFILE_$(CORE)):$(APP_NAME)$(MODULE_NAME): $<
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) $<
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_DIROPTS) $<

$(OBJ_PATHS_K): $(OBJDIR)/%.$(OBJEXT): %.k
	$(ECHO) \# Compiling $(PLATFORM):$(CORE):$(PROFILE_$(CORE)):$(APP_NAME)$(MODULE_NAME): $<
	$(CC) $(_CFLAGS) $(INCLUDES) $(KFLAGS) $(CFLAGS_DIROPTS) -ft=$(dir $< ) $<


# Archive flags - normally doesn't change
ARFLAGS = rq

# Archive/library file creation
$(LIBDIR)/$(MODULE_NAME).$(LIBEXT) : $(OBJ_PATHS_K) $(OBJ_PATHS) $(OBJ_PATHS_CPP)
	$(ECHO) \#
	$(ECHO) \# Archiving $(PLATFORM):$(CORE):$(PROFILE_$(CORE)):$(MODULE_NAME)
	$(ECHO) \#
	$(AR) $(ARFLAGS) $@ $(OBJ_PATHS) $(OBJ_PATHS_CPP) $(OBJ_PATHS_K)

# Linker options and rules
LNKFLAGS_INTERNAL_COMMON = --warn_sections -q -e=_c_int00 --silicon_version=arp32 -c -x --zero_init=off

# Assemble Linker flags from all other LNKFLAGS definitions
_LNKFLAGS = $(LNKFLAGS_INTERNAL_COMMON) $(LNKFLAGS_INTERNAL_PROFILE) $(LNKFLAGS_GLOBAL_$(CORE)) $(LNKFLAGS_LOCAL_COMMON) $(LNKFLAGS_LOCAL_$(CORE))

# Path of the RTS library - normally doesn't change for a given tool-chain
RTSLIB_PATH = $(CODEGEN_PATH_EVE)/lib/rtsarp32_v203.lib


LIB_PATHS += $(DEST_ROOT)/lib/$(PLATFORM)/$(CORE)/$(PROFILE_$(CORE))/vision_sdk_lib.$(LIBEXT)
LIB_PATHS += $(evealg_PATH)/algframework/lib/$(PROFILE_$(CORE))/libevealgframework.eve.lib
LIB_PATHS += $(evealg_PATH)/kernels/lib/$(PROFILE_$(CORE))/libevekernels.eve.lib
LIB_PATHS += $(evealg_PATH)/kernels/lib/$(PROFILE_$(CORE))/libevenatckernels.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/apps_nonbam/lib/$(PROFILE_$(CORE))/libeveapps.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/ti_pd_feature_plane_computation/algo/lib/$(PROFILE_$(CORE))/libeveFeaturePlaneComputation.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/soft_isp/algo/lib/$(PROFILE_$(CORE))/libeveSoftISP.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/census/algo/lib/$(PROFILE_$(CORE))/libeveCensus.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/disparity/algo/lib/$(PROFILE_$(CORE))/libeveDisparity.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/remap_merge/algo/lib/$(PROFILE_$(CORE))/libeveRemapMerge.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/imagePyramid_u8/algo/lib/$(PROFILE_$(CORE))/libeveImagePyramid_u8.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/harrisCornerDetection32/algo/lib/$(PROFILE_$(CORE))/libeveHarrisCornerDetection32.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/pyramid_lk_tracker/algo/lib/$(PROFILE_$(CORE))/libevePyramidLKTracker.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/fast9_best_feature_to_front/algo/lib/$(PROFILE_$(CORE))/libeveFAST9BestFeatureToFront.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/fast9_corner_detect/algo/lib/$(PROFILE_$(CORE))/libeveFast9CornerDetect.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/yuv_padding/algo/lib/$(PROFILE_$(CORE))/libeveYUVPAdding.eve.lib
LIB_PATHS += $(evealg_PATH)/starterware/libs/vayu/eve/$(PROFILE_$(CORE))/libevestarterware_eve.lib
LIB_PATHS += $(evealg_PATH)/algorithms/pyramid_lk_sof/algo/lib/$(PROFILE_$(CORE))/libevesof.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/filter_2d/algo/lib/$(PROFILE_$(CORE))/libeveFilter2D.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/yuv_scalar/algo/lib/$(PROFILE_$(CORE))/libeveYUVScalar.eve.lib
LIB_PATHS += $(evealg_PATH)/apps/bin_image_to_list/algo/lib/$(PROFILE_$(CORE))/libeveBinImageToList.eve.lib

ifeq ($(HCF_INCLUDE),yes)
LIB_PATHS += $(hcf_PATH)/out/VAYU_BIOS/EVE/SYSBIOS/$(PROFILE_$(CORE))/hcf.lib
LIB_PATHS += $(hcf_PATH)/out/VAYU_BIOS/EVE/SYSBIOS/$(PROFILE_$(CORE))/sosal.lib
endif

LIB_PATHS += $(APP_LIBS_$(CORE))
LIB_PATHS += $(DEST_ROOT)/lib/$(PLATFORM)/$(ISA)/$(PROFILE_$(CORE))/edma3lld_rm.$(LIBEXT)
LIB_PATHS += $(DEST_ROOT)/lib/$(PLATFORM)/$(ISA)/$(PROFILE_$(CORE))/edma3lld_drv.$(LIBEXT)
LIB_PATHS += $(DEST_ROOT)/lib/$(PLATFORM)/$(ISA)/$(PROFILE_$(CORE))/bsp_osal.$(LIBEXT)
LIB_PATHS += $(RTSLIB_PATH)

PM_FUNC_INCLUDE_PATH +=$(DEST_ROOT)/lib/$(PLATFORM)/$(ISA)/$(PROFILE_$(CORE))/starterware_pm_hal.aearp32F
PM_FUNC_INCLUDE_PATH +=$(DEST_ROOT)/lib/$(PLATFORM)/$(ISA)/$(PROFILE_$(CORE))/sys_config.aearp32F
PM_FUNC_INCLUDE_PATH +=$(DEST_ROOT)/lib/$(PLATFORM)/$(ISA)/$(PROFILE_$(CORE))/starterware_pm_lib.aearp32F

LNK_LIBS = $(addprefix -l,$(LIB_PATHS))

ifdef APP_NAME

	ifeq ($(LNK_Starterware_LIB), yes)
		LNK_LIBS += $(StarterwareHalLIB_PATH)
	endif

	ifeq ($(LNK_Starterware_Common_LIB), yes)
		LNK_LIBS += $(Starterware_Common_PATH)
	endif

	ifeq ($(LNK_SYSCONFIG_LIB), yes)
		LNK_LIBS += $(SysConfig_PATH)
	endif

	LNK_LIBS += $(PM_FUNC_INCLUDE_PATH)
	
endif

# Linker - to create executable file

EXE_NAME = $(BINDIR)/$(APP_NAME)_$(CORE)_$(PROFILE_$(CORE)).$(EXEEXT)

$(EXE_NAME) : $(OBJ_PATHS_K) $(OBJ_PATHS_ASM) $(OBJ_PATHS) $(OBJ_PATHS_CPP) $(LIB_PATHS) $(LNKCMD_FILE) $(CFG_C_NEW_XDC)/$(CFG_COBJ_XDC) $(MISC_LNKCMD_INCLUDE)
	$(ECHO) \# Linking into $(EXE_NAME)...
	$(ECHO) \#
	$(LNK) $(_LNKFLAGS) $(OBJ_PATHS_ASM) $(OBJ_PATHS) $(OBJ_PATHS_CPP) $(OBJ_PATHS_K) -l$(LNKCMD_FILE) -o $@ -m $@.map $(LNK_LIBS) $(MISC_LNKCMD_INCLUDE)
	$(ECHO) \#
	$(ECHO) \# $@ created.
	$(ECHO) \#
ifeq ($(IPU1_EVELOADER_INCLUDE),yes)
	mono $(vision_sdk_PATH)/src/utils_common/src/eveloader/tools/out2rprc.exe $(EXE_NAME) $(EXE_NAME).rprc
endif

# XDC specific - assemble XDC-Configuro command
ifeq ($(PROFILE_$(CORE)),prod_release)
  CONFIGURO_PROFILE = release
else
  CONFIGURO_PROFILE = $(PROFILE_$(CORE))
endif
CONFIGURO_CMD = $(xdc_PATH)/xs xdc.tools.configuro --generationOnly -o $(CONFIGURO_DIR) -t $(TARGET_XDC) -p $(PLATFORM_XDC) \
               -r $(CONFIGURO_PROFILE) -c $(CODEGEN_PATH_EVE) -b $(CONFIG_BLD_XDC_$(ISA)) --cfgArgs $(CFGARGS_XDC) $(XDC_CFG_FILE_NAME)
_XDC_GREP_STRING = \"$(XDC_GREP_STRING)\"
EGREP_CMD = $(EGREP) -ivw $(XDC_GREP_STRING) $(XDCLNKCMD_FILE)

ifeq ($(OS),Windows_NT)
EVERYONE := $(word 1,$(shell whoami -groups | findstr "S-1-1-0"))
endif

# Invoke configuro for the rest of the components
#  NOTE: 1. String handling is having issues with various make versions when the
#           cammand is directly tried to be given below. Hence, as a work-around,
#           the command is re-directed to a file (shell or batch file) and then
#           executed
#        2. The linker.cmd file generated, includes the libraries generated by
#           XDC. An egrep to search for these and omit in the .cmd file is added
#           after configuro is done
xdc_configuro : $(XDC_CFG_FILE)
	$(ECHO) \# Invoking configuro...
	$(MKDIR) -p $(DEST_ROOT)
	$(ECHO) -e $(CONFIGURO_CMD) > $(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat
ifeq ($(OS),Windows_NT)
	CACLS $(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat /E /P $(EVERYONE):F
else
	$(CHMOD) a+x $(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat
endif
	$(DEST_ROOT)/maketemp_configuro_cmd_$(CORE).bat
	$(CP) $(XDCLNKCMD_FILE) $(LNKCMD_FILE)
	$(ECHO) \# Configuro done!



ifndef MODULE_NAME
$(CFG_C_NEW_XDC)/$(CFG_COBJ_XDC) : $(CFG_C_XDC)
	$(ECHO) \# Compiling generated $(CFG_COBJ_XDC)
	$(CC) -ppd=$(DEPFILE).P $(_CFLAGS) $(INCLUDES) $(CFLAGS_NEW_DIROPTS) -fc $(CFG_C_XDC)
	$(CC) $(_CFLAGS) $(INCLUDES) $(CFLAGS_NEW_DIROPTS) -fc $(CFG_C_XDC)
endif


# Include dependency make files that were generated by $(CC)
-include $(SRCS:%.c=$(DEPDIR)/%.P)
-include $(SRCS_CPP:%.cpp=$(DEPDIR)/%.P)
-include $(SRCS_K:%.k=$(DEPDIR)/%.P)

# Nothing beyond this point
