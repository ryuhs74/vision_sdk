

SRCDIR += links_common/system \
          links_common/ipcOut \
          links_common/dup \
          links_common/null \
          links_common/ipcIn \
          links_common/sync \
          links_common/merge \
          links_common/select \
          links_common/algorithm \
          links_common/nullSrc \
          links_common/avb_rx \
          links_common/gate \

SRCS_COMMON += nullLink_tsk.c dupLink_tsk.c system_linkApi.c \
                system_linkApi_local.c system_common.c \
                system_ipc.c system_ipc_msgq.c system_ipc_notify.c \
                ipcOutLink_tsk.c ipcOutLink_drv.c \
                ipcInLink_tsk.c ipcInLink_drv.c \
                syncLink_tsk.c mergeLink_tsk.c \
                algorithmLink_algPluginSupport.c \
                algorithmLink_cfg.c algorithmLink_tsk.c \
                selectLink_tsk.c \
                nullSrcLink_tsk.c \
                nullSrcLink_networkRx.c \
                nullLink_networkTx.c \
                system_tsk_multi_mbx.c \
                gateLink_tsk.c \

SRCS_MULTI_MBX = ipcInLink_tsk_create_multi_mbx.c \
                 ipcOutLink_tsk_create_multi_mbx.c \
                 algorithmLink_tsk_create_multi_mbx.c \
                 dupLink_tsk_create_multi_mbx.c \
                 mergeLink_tsk_create_multi_mbx.c \
                 selectLink_tsk_create_multi_mbx.c \
                 syncLink_tsk_create_multi_mbx.c \
                 gateLink_tsk_create_multi_mbx.c \

SRCS_SINGLE_MBX = ipcInLink_tsk_create_single_mbx.c \
                  ipcOutLink_tsk_create_single_mbx.c \
                  algorithmLink_tsk_create_single_mbx.c \
                  dupLink_tsk_create_single_mbx.c \
                  mergeLink_tsk_create_single_mbx.c \
                  selectLink_tsk_create_single_mbx.c \
                  syncLink_tsk_create_single_mbx.c \
                  gateLink_tsk_create_single_mbx.c \

SRCS_arp32_1   += $(SRCS_MULTI_MBX)
SRCS_arp32_2   += $(SRCS_MULTI_MBX)
SRCS_arp32_3   += $(SRCS_MULTI_MBX)
SRCS_arp32_4   += $(SRCS_MULTI_MBX)

SRCS_c66xdsp_1 += $(SRCS_MULTI_MBX)
SRCS_c66xdsp_2 += $(SRCS_MULTI_MBX)

SRCS_a15_0     += $(SRCS_SINGLE_MBX)

SRCS_ipu1_1    += $(SRCS_SINGLE_MBX)

SRCS_ipu1_0    += $(SRCS_SINGLE_MBX)

ifeq ($(AVBRX_INCLUDE),yes)
SRCS_ipu1_0 += avbRxLink_drv.c avbRxLink_tsk.c
SRCS_ipu1_1 += avbRxLink_drv.c avbRxLink_tsk.c
SRCS_a15_0  += avbRxLink_drv.c avbRxLink_tsk.c
endif

ifeq ($(A15_TARGET_OS), Linux)
SRCS_ipu1_0 += system_rpmsg.c 
SRCS_ipu1_1 += system_rpmsg.c 
SRCS_c66xdsp_1 += system_rpmsg.c 
SRCS_c66xdsp_2 += system_rpmsg.c 
endif





