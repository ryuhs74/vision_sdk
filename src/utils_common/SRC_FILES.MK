
SRCDIR += utils_common/src utils_common/src/dma_cfg \
          utils_common/src/ndk utils_common/src/eveloader \
          utils_common/src/stat_collector_utils

INCDIR = .\
         utils_common/src/$(SOC)/

ifeq ($(SOC),tda2xx)
include $(vision_sdk_PATH)/src/utils_common/src/tda2xx/SRC_FILES.MK
endif

ifeq ($(SOC),tda3xx)
include $(vision_sdk_PATH)/src/utils_common/src/tda3xx/SRC_FILES.MK
endif

ifeq ($(SOC),tda2ex)
include $(vision_sdk_PATH)/src/utils_common/src/tda2ex/SRC_FILES.MK
endif

SRCS_COMMON += utils_remote_log_server.c utils.c utils_buf.c utils_mbx.c \
               utils_mem.c  utils_prf.c utils_que.c utils_tsk.c utils_tsk_multi_mbx.c \
               utils_ipc_que.c \
               utils_remote_log_client.c \
               utils_global_time.c \
               utils_buf_ext.c utils_timer_reconfig.c \
               utils_link_stats_collector.c

SRC_DMA_COMMON = utils_dma.c \
                 utils_dma_edma3cc.c

SRCS_ipu1_0 += utils_execp_trace_local_m4.c utils_uart.c \
               $(SRC_DMA_COMMON) \
               utils_dma_edma3cc_ipu_a15_intr.c \
               utils_dma_cfg_ipu1_0.c \
               utils_cbuf_ocmc.c utils_qspi.c \
               utils_idle_m4.c \
               utils_temperature.c \
               utils_stat_collector.c \
               utils_prcm_stats.c \
               sci.c \
               cToolsHelper.c \
			   utils_mem_ipu1_0.c \



ifeq ($(ISS_INCLUDE), yes)
SRCS_ipu1_0 += utils_iss.c
endif

ifeq ($(IPU1_EVELOADER_INCLUDE), yes)
SRCS_ipu1_0 += utils_eveloader.c utils_eve_boot.c utils_rprc_parse.c utils_soc_init.c utils_eve_prcmlib_dpll.c
endif


SRCS_ipu1_1 += utils_execp_trace_remote_m4.c \
               $(SRC_DMA_COMMON) \
               utils_dma_edma3cc_ipu_a15_intr.c \
               utils_dma_cfg_ipu1_1.c \
               utils_idle_m4.c \
               utils_stat_collector.c \
               sci.c \
               cToolsHelper.c \
               utils_qspi.c



SRCS_c66xdsp_1 += utils_execp_trace_dsp.c $(SRC_DMA_COMMON) \
                  utils_dma_cfg_dsp1.c \
                  utils_dma_edma3cc_dsp_intr.c \
                  utils_idle_c66x.c \
				  network_api.c \
				  utils_cache_c66x.c

SRCS_c66xdsp_2 += utils_execp_trace_dsp.c $(SRC_DMA_COMMON) \
                  utils_dma_cfg_dsp2.c \
                  utils_dma_edma3cc_dsp_intr.c \
                  utils_idle_c66x.c \
				  network_api.c \
				  utils_cache_c66x.c

SRCS_arp32_1 += $(SRC_DMA_COMMON) utils_dma_cfg_eve.c utils_dma_edma3cc_eve_intr.c utils_vip_interrupt.c utils_execp_trace_eve.c network_api.c utils_idle_arp32.c
SRCS_arp32_2 += $(SRC_DMA_COMMON) utils_dma_cfg_eve.c utils_dma_edma3cc_eve_intr.c utils_vip_interrupt.c utils_execp_trace_eve.c network_api.c utils_idle_arp32.c
SRCS_arp32_3 += $(SRC_DMA_COMMON) utils_dma_cfg_eve.c utils_dma_edma3cc_eve_intr.c utils_vip_interrupt.c utils_execp_trace_eve.c network_api.c utils_idle_arp32.c
SRCS_arp32_4 += $(SRC_DMA_COMMON) utils_dma_cfg_eve.c utils_dma_edma3cc_eve_intr.c utils_vip_interrupt.c utils_execp_trace_eve.c network_api.c utils_idle_arp32.c

SRCS_a15_0 +=  $(SRC_DMA_COMMON) \
               utils_dma_edma3cc_ipu_a15_intr.c \
               utils_dma_cfg_a15_0.c \
               utils_execp_trace_a15.c \
               utils_idle_a15.c \
               utils_qspi.c

SRCS_ipu1_0 += ndk_nsp_hooks.c network_api.c
SRCS_ipu1_1 += ndk_nsp_hooks.c network_api.c
SRCS_a15_0  += ndk_nsp_hooks.c network_api.c



