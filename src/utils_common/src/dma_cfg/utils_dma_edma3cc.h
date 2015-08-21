/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_dma_edma3cc.h
 *
 * \brief EDMA controller specific configuration
 *
 * \version 0.0 (Aug 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_DMA_EDMA3CC_H_
#define _UTILS_DMA_EDMA3CC_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_dma.h>
#include <ti/sysbios/family/shared/vayu/IntXbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Maximum number of EDMA instances that are supported
 *******************************************************************************
 */
/* At max there can be 2 instances of EDMA contorller
 * - System EDMA
 * - Local EDMA
 *
 * DSP  supports - System EDMA as well as local EDMA
 * EVE  supports - Only Local EDMA
 * IPU1 supports - Only System EDMA
 * A15  supports - Only System EDMA
 *
 */
#define UTILS_DMA_MAX_EDMA_INST     (2)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Information related to interrupt for this EDMA controller on this
 *         CPU
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 ccXferCompCpuInt;
    /**< DMA transfer complete interrupt number at the CPU */

    UInt32 ccXferCompXbarInt;
    /**< DMA transfer complete interrupt number at the XBAR */

    UInt32 ccXferCompCtrlModXbarIndex;
    /**< DMA transfer complete interrupt control module XBAR index number */

    UInt32 ccErrorCpuInt;
    /**< DMA transfer error interrupt number at the CPU */

    UInt32 ccErrorXbarInt;
    /**< DMA transfer error interrupt number at the XBAR */

    UInt32 ccErrorCtrlModXbarIndex;
    /**< DMA transfer error interrupt control module XBAR index number */

    UInt32 tc0ErrorCpuInt;
    /**< DMA transfer controller error interrupt number at the CPU */

    UInt32 tc0ErrorXbarInt;
    /**< DMA transfer controller error interrupt number at the XBAR */

    UInt32 tc0ErrorCtrlModXbarIndex;
    /**< DMA transfer controller error interrupt control
     *   module XBAR index number */

    UInt32 tc1ErrorCpuInt;
    /**< DMA transfer controller error interrupt number at the CPU */

    UInt32 tc1ErrorXbarInt;
    /**< DMA transfer controller error interrupt number at the XBAR */

    UInt32 tc1ErrorCtrlModXbarIndex;
    /**< DMA transfer controller error interrupt control module XBAR
     *   index number */

    UInt32 ccXferCompCpuHwiInt;
    /**< DMA transfer complete interrupt CPU interrupt vector number
     *   ONLY applicable for DSP
     */

    UInt32 ccErrorCpuHwiInt;
    /**< DMA transfer error interrupt CPU interrupt vector number
     *   ONLY applicable for DSP
     */

    UInt32 tc0ErrorCpuHwiInt;
    /**< DMA transfer contorller error interrupt CPU interrupt vector number
     *   ONLY applicable for DSP
     */

    UInt32 tc1ErrorCpuHwiInt;
    /**< DMA transfer contorller error interrupt CPU interrupt vector number
     *   ONLY applicable for DSP
     */

} Utils_DmaIntrCfg;

/**
 *******************************************************************************
 *
 *  \brief Information related to a specific EDMA controller
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 edma3InstanceId;
    /**< EDMA controller instance ID */

    EDMA3_DRV_Handle hEdma;
    /**< Handle to EDMA controller LLD driver */

    BspOsal_IntrHandle   hwiCCXferCompInt;
    /**< Handle to HWI for transfer complete */

    unsigned int regionId;
    /**< EDMA region ID associated with this CPU */

    EDMA3_OS_Sem_Handle semHandle;
    /**< EDMA controller semaphore used by driver internally */

    EDMA3_DRV_GblConfigParams *pGblCfgParams;
    /**< EDMA controller config */

    EDMA3_DRV_InstanceInitConfig *pInstInitConfig;
    /**< EDMA controller config specific to this CPU */

    Utils_DmaIntrCfg  *pIntrConfig;
    /**< DMA controller interrupt related config for this CPU */

} Utils_DmaObj;



/*******************************************************************************
 *  Functions
 *******************************************************************************
 */


EDMA3_DRV_Result Utils_edma3Init (UInt32 edma3Id);

EDMA3_DRV_Result Utils_edma3DeInit (UInt32 edma3Id);

Bool Utils_dmaIsIntrSupported(UInt32 edmaInstId);

EDMA3_DRV_Result Utils_edma3OsSemCreate(int initVal,
							EDMA3_OS_Sem_Handle *hSem);

EDMA3_DRV_Result Utils_edma3OsSemDelete(EDMA3_OS_Sem_Handle hSem);

/*
 *******************************************************************************
 * Below function's are specific to a CPU, every CPU implementing EDMA APIs
 * needs to implement the below functions specific to their configuration
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief Register the ISRs with the underlying OS
 *
 *        This API is implemented by CPU specific file
 *        dma_cfg\utils_dma_edma3cc_<cpu>_intr.c
 *
 *  \param pObj     [IN] EDMA controller information
 *
 *******************************************************************************
 */
void Utils_edma3RegisterInterrupts (Utils_DmaObj *pObj);

/**
 *******************************************************************************
 *
 *  \brief Un-Register the ISRs with the underlying OS
 *
 *        This API is implemented by CPU specific file
 *        dma_cfg\utils_dma_edma3cc_<cpu>_intr.c
 *
 *  \param pObj     [IN] EDMA controller information
 *
 *******************************************************************************
 */
void Utils_edma3UnRegisterInterrupts (Utils_DmaObj *pObj);

/**
 *******************************************************************************
 *
 *  \brief Find the region ID associated with EDMA controller and CPU ID
 *
 *        This API is implemented by CPU specific config file
 *        dma_cfg\utils_dma_cfg_<cpu>.c
 *
 *  \param edmaInstId   [IN] EDMA controller ID
 *
 *  \return region ID to use
 *
 *******************************************************************************
 */
UInt32 Utils_edma3GetRegionId(unsigned int edmaInstId);

/**
 *******************************************************************************
 *
 *  \brief To check whether the global EDMA3 configuration is required or not.
 *
 *        It should be done ONCE by any of the masters present in the system.
 *        This function checks whether the global configuration is required
 *        by the current master or not.
 *        In case of many masters, it should be done only by one of the masters.
 *        Hence this function will return TRUE only once and FALSE for all
 *        other masters.
 *
 *        This API is implemented by CPU specific config file
 *        dma_cfg\utils_dma_cfg_<cpu>.c
 *
 *  \param edmaInstId   [IN] EDMA controller ID
 *
 *  \return TRUE, perform global init
 *
 *******************************************************************************
 */
Bool Utils_edma3IsGblConfigRequired(unsigned int edmaInstId);

/**
 *******************************************************************************
 *
 *  \brief Return EDMA controller global config for this CPU
 *
 *        This API is implemented by CPU specific config file
 *        dma_cfg\utils_dma_cfg_<cpu>.c
 *
 *  \param edmaInstId   [IN] EDMA controller ID
 *
 *  \return Pointer to config
 *
 *******************************************************************************
 */
EDMA3_DRV_GblConfigParams *Utils_edma3GetGblCfg  (unsigned int edmaInstId);

/**
 *******************************************************************************
 *
 *  \brief Return EDMA controller instance specific config for this CPU
 *
 *        This API is implemented by CPU specific config file
 *        dma_cfg\utils_dma_cfg_<cpu>.c
 *
 *  \param edmaInstId   [IN] EDMA controller ID
 *
 *  \return Pointer to config
 *
 *******************************************************************************
 */
EDMA3_DRV_InstanceInitConfig *Utils_edma3GetInstCfg (unsigned int edmaInstId);

/**
 *******************************************************************************
 *
 *  \brief Return EDMA controller interrupt related config for this CPU
 *
 *        This API is implemented by CPU specific config file
 *        dma_cfg\utils_dma_cfg_<cpu>.c
 *
 *  \param edmaInstId   [IN] EDMA controller ID
 *
 *  \return Pointer to config
 *
 *******************************************************************************
 */
Utils_DmaIntrCfg *Utils_edma3GetIntrCfg (unsigned int edmaInstId);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  /* _UTILS_DMA_EDMA3CC_H_ */

