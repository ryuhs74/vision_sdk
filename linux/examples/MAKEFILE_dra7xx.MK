STRIP_M4  = $(CODEGEN_PATH_M4)/bin/armstrip -p
STRIP_DSP = $(CODEGEN_PATH_DSP)/bin/strip6x -p
STRIP_EVE = $(CODEGEN_PATH_EVE)/bin/strip-arp32 -p

demo:
	$(MAKE) -fMAKEFILE_dra7xx.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE_dra7xx.MK libs TARGET=lib
	$(MAKE) -fMAKEFILE_dra7xx.MK package

vision_sdk_linux_lib:
ifneq ($(TARGET),depend)
# Copy IPU1-0 generated memory map file to library include folder.
# If header file is not present then use previously generated file
	-cp $(vision_sdk_PATH)/binaries/obj/vision_sdk/$(PLATFORM)/ipu1_0/$(PROFILE_ipu1_0)/vision_sdk_configuro/osa_mem_map.h $(vision_sdk_PATH)/linux/src/osa/include
endif
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/src/links/ipcOut MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/src/links/ipcIn  MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/src/links/null  MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/src/utils/multiproc MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE_dra7xx.MK -C$(vision_sdk_PATH)/linux/src/system MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/src/osa/src MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/examples/common MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/examples/dra7xx/src/ti-plugins MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/examples/dra7xx/src/common MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/examples/dra7xx/src/links/ep MODULE=vision_sdk_linux_lib $(TARGET)
	$(MAKE) -fMAKEFILE.MK -C$(vision_sdk_PATH)/linux/examples/dra7xx/src/chains/lvds_vip_2d_srv MODULE=vision_sdk_linux_lib $(TARGET)


libs: vision_sdk_linux_lib 

package:
	mkdir -p $(vision_sdk_PATH)/temp/visionsdk-fw/
	mkdir -p $(vision_sdk_PATH)/temp/visionsdk-fw/lib
	mkdir -p $(vision_sdk_PATH)/temp/visionsdk-fw/include
	mkdir -p $(vision_sdk_PATH)/temp/visionsdk-fw/fw

ifeq ($(PROC_IPU1_0_INCLUDE),yes)
		cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_ipu1_0_$(PROFILE_ipu1_0).xem4 $(vision_sdk_PATH)/temp/visionsdk-fw/fw/dra7-ipu1-fw.xem4
		$(STRIP_M4) $(vision_sdk_PATH)/temp/visionsdk-fw/fw/dra7-ipu1-fw.xem4
endif
ifeq ($(PROC_DSP1_INCLUDE),yes)
		cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_c66xdsp_1_$(PROFILE_c66xdsp_1).xe66 $(vision_sdk_PATH)/temp/visionsdk-fw/fw/dra7-dsp1-fw.xe66
		$(STRIP_DSP) $(vision_sdk_PATH)/temp/visionsdk-fw/fw/dra7-dsp1-fw.xe66
endif
ifeq ($(PROC_DSP2_INCLUDE),yes)
		cp $(vision_sdk_PATH)/binaries/vision_sdk/bin/$(PLATFORM)/vision_sdk_c66xdsp_2_$(PROFILE_c66xdsp_2).xe66 $(vision_sdk_PATH)/temp/visionsdk-fw/fw/dra7-dsp2-fw.xe66
		$(STRIP_DSP) $(vision_sdk_PATH)/temp/visionsdk-fw/fw/dra7-dsp2-fw.xe66
endif
	#copy libadas.a
	cp $(vision_sdk_PATH)/binaries/lib/a15/debug/vision_sdk_linux_lib.a $(vision_sdk_PATH)/temp/visionsdk-fw/lib/libadas.a
	cp $(vision_sdk_PATH)/linux/src/links/sgx3Dsrv/jeep_outside2_raw.bmp $(vision_sdk_PATH)/temp/visionsdk-fw/

	#copy header files
	cp $(vision_sdk_PATH)/include/link_api/system_const.h $(vision_sdk_PATH)/temp/visionsdk-fw/include/
	cp $(vision_sdk_PATH)/include/link_api/system_link_info.h $(vision_sdk_PATH)/temp/visionsdk-fw/include/
	cp $(vision_sdk_PATH)/include/link_api/system_buffer.h $(vision_sdk_PATH)/temp/visionsdk-fw/include/
	cp $(vision_sdk_PATH)/linux/src/osa/include/osa_types.h $(vision_sdk_PATH)/temp/visionsdk-fw/include/
	cp $(vision_sdk_PATH)/linux/src/osa/include/osa_debug.h $(vision_sdk_PATH)/temp/visionsdk-fw/include/

	@echo \# Creating tar for firmwares !!!
	tar -czf $(vision_sdk_PATH)/binaries/visionsdk-fw.tar.gz  -C $(vision_sdk_PATH)/temp/ .
	rm -Rf $(vision_sdk_PATH)/temp/visionsdk-fw

clean:
	$(MAKE) -fMAKEFILE_dra7xx.MK libs TARGET=clean

all: clean demo

.PHONY: libs clean demo all vision_sdk_linux_lib 

