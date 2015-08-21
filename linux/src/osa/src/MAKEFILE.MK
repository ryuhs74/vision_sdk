
include $(vision_sdk_PATH)/linux/build/common_header_a15.mk

INCLUDE+= -I$(vision_sdk_PATH)/linux/src/osa/include $(LINUX_COMMON_INC) $(OSA_KERMOD_INC)

include $(vision_sdk_PATH)/linux/build/common_footer_a15.mk


