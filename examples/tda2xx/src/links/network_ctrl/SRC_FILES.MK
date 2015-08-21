
SRCDIR += $(vision_sdk_PATH)/examples/tda2xx/src/links/network_ctrl

NETWORK_SRCS = \
		network_ctrl_tsk.c \
		network_ctrl_handle_echo.c \
		network_ctrl_handle_mem_rd.c \
		network_ctrl_handle_mem_save.c \
		network_ctrl_handle_stereo_calib_image_save.c \
		network_ctrl_handle_stereo_set_params.c \
		network_ctrl_handle_stereo_set_dynamic_params.c \
		network_ctrl_handle_iss_raw_save.c \
		network_ctrl_handle_qspi_wr.c \
		network_ctrl_handle_sys_reset.c

SRCS_ipu1_0 += $(NETWORK_SRCS)
SRCS_ipu1_1 += $(NETWORK_SRCS)
SRCS_a15_0 += $(NETWORK_SRCS)
		

