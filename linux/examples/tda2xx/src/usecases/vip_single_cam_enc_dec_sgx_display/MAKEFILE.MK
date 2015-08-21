
include $(vision_sdk_PATH)/linux/build/common_header_a15.mk

INCLUDE+= -I$(ipc_PATH)/hlos_common/include -I$(ipc_PATH)/packages -I$(kernel_PATH)/include/generated/uapi $(OSA_INC) -I$(ipc_PATH)/linux/include -I$(vision_sdk_PATH)/linux/include

LIBS += $(LIB_BASE_DIR)/vision_sdk_linux_demo.a $(LIB_BASE_DIR)/vision_sdk_linux_lib.a $(LIB_BASE_DIR)/vision_sdk_linux_demo.a $(LIB_BASE_DIR)/vision_sdk_linux_lib.a 

include $(vision_sdk_PATH)/linux/build/common_footer_a15.mk
