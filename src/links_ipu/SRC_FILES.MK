
SRCDIR += links_ipu/vip_capture \
        links_ipu/display \
        links_ipu/display_ctrl \
        links_ipu/system \
        links_ipu/vpe \
        links_ipu/avb_rx \
        links_ipu/iva/codec_utils/src \
        links_ipu/iva/iva_dec \
        links_ipu/iva/iva_enc \
        links_ipu/iss_capture \
        links_ipu/iss_m2misp \
        links_ipu/iss_m2msimcop

ifeq ($(ISS_INCLUDE),yes)
SRCS_ipu1_0 += \
        issCaptureLink_tsk.c issCaptureLink_drv.c \
		issM2mIspLink_tsk.c issM2mIspLink_drv_apply_config.c \
		issM2mIspLink_drv_create_delete.c \
		issM2mIspLink_drv_process_frame.c \
		issM2mSimcopLink_tsk.c issM2mSimcopLink_drv.c
endif

# Files common for all SOC Families
SRCS_ipu1_0 += \
        captureLink_tsk.c captureLink_drv.c captureLink_subframe_drv.c \
        captureLink_displaywb_drv.c \
        system_bsp_init.c \
        system_ipu1_0.c \
        systemLink_tsk_ipu1.c \

ifeq ($(DSS_INCLUDE),yes)
SRCS_ipu1_0 += displayLink_tsk.c displayLink_drv.c \
				displayCtrlLink_tsk.c displayCtrlLink_drv.c
endif

ifeq ($(VPE_INCLUDE),yes)
SRCS_ipu1_0 += vpeLink_tsk.c vpeLink_drvCommon.c
endif

ifneq ($(VPE_INCLUDE),yes)
ifeq ($(ISS_INCLUDE),yes)
SRCS_ipu1_0 += vpeLink_tsk.c vpeLink_drvCommon.c
endif
endif


ifeq ($(VPE_INCLUDE),yes)
SRCS_ipu1_0 += vpeLink_drv.c
endif

ifeq ($(ISS_INCLUDE),yes)
SRCS_ipu1_0 += vpeLink_drvIss.c
endif

ifeq ($(IVAHD_INCLUDE),yes)
SRCS_ipu1_0 += iresman_hdvicp2_fwif.c \
        utils_encdec.c iresman_tiledmemory.c hdvicp2_config.c \
        alg_control.c alg_create.c alg_malloc.c \
        encLink_tsk.c encLink_common.c \
        decLink_tsk.c decLink_common.c \
        encLink_jpeg.c encLink_h264.c \
        decLink_jpeg.c decLink_h264.c
endif

SRCS_ipu1_1 += \
        system_ipu1_1.c \
        systemLink_tsk_ipu1.c
