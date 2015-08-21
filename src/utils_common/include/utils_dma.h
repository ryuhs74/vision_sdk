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
 * \ingroup UTILS_API
 * \defgroup UTILS_DMA_API System DMA API
 *
 * \brief This module define APIs to trigger EDMAs
 *
 *        The DMA APIs are enabled on the below processor's
 *        - IPU1-0
 *        - IPU1-1
 *        - A15-0
 *        - DSP1
 *        - DSP2
 *        - EVE1
 *        - EVE2
 *        - EVE3
 *        - EVE4
 *
 *        The APIs in this module used the EDMA3LLD library to program the
 *        EDMA controller
 *
 *        Once Utils_dmaInit() is done, users can call the API
 *        Utils_dmaGetEdma3Hndl() to get the EDMA3LLD driver handle and then
 *        user can use the EDMA3LLD APIs directly
 *
 *        Few utility APIs to copy/fill frames are implemented in this library
 *        using the EDMA3LLD for convinience of application code.
 *
 *        For alogirithms it is recommended to use the EDMA3LLD APIs directly
 *        to avoid additional layer of software overhead.
 *
 *        NOTE: EVE software manages its own EDMA and is not controlled through
 *              these APIs
 *
 *        There are multiple EDMA controller's in the system as shown below
 *        - System wide EDMA controller
 *          - This controller can be used by all CPUs
 *        - CPU local EDMA controller
 *          - This is a EDMA controller dedicated for a given CPU
 *          - Such a EDMA controller is supported only on DSP's and EVE's
 *          - Normally algorithms running on a CPU use this EDMA controller
 *
 *        These APIs configure and use the system EDMA controller for A15,
 *        M4, DSP
 *        These APIs to the support for local EDMA controller for DSP, EVE.
 *
 *        All CPU Local EDMA resources are allocated to the given CPU.
 *
 *        System EDMA controller can be used in interrupt mode or polling mode
 *        Based on parameter passed during channel create.
 *
 *        DSP Local EDMA controller can only be used in polling mode.
 *
 *        This is because normally algorithms using this EDMA would use this
 *        EDMA to transfer small amounts of data between DDR and L2 RAM.
 *        In these cases polling method is better to transfer data
 *
 *        EVE Local EDMA controller supports interrupt mode or polling mode
 *        Based on parameter passed during channel create.
 *
 *        The system EDMA controller supports
 *        - 64 EDMA channels
 *        - 8  QDMA channels
 *        - 512 PaRAMs entries
 *
 *        The System EDMA CHs are split across processor's as shown below
 *        - IPU1-0
 *          - EDMA CHs - 16
 *          - QDMA CHs - 0
 *          - PaRAM    - 80
 *        - IPU1-1
 *          - EDMA CHs - 8
 *          - QDMA CHs - 0
 *          - PaRAM    - 24
 *        - DSP1
 *          - EDMA CHs - 4
 *          - QDMA CHs - 0
 *          - PaRAM    - 20
 *        - DSP2
 *          - EDMA CHs - 4
 *          - QDMA CHs - 0
 *          - PaRAM    - 20
 *        - A15-0
 *          - EDMA CHs - 32
 *          - QDMA CHs - 8
 *          - PaRAM    - 384
 *
 *        The rational for channel allocation is given below
 *        - A15 is given more EDMA/QDMA channels assuming algorithms running on
 *          A15 may need it.
 *        - DSP, EVE algorithm will use the dedicated local EDMA for algorithm
 *          purposes.
 *        - Few system EDMA channels are given to DSP for any system level DMA
 *          purposes.
 *        - IPU1-0, IPU1-1 are given to EDMA channels for any system level DMA
 *          purposes.
 *
 *        This static configuration can be changed by editing the config files
 *        present in "base dir"/src/utils_common/src/dma_cfg
 *
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils_dma.h
 *
 * \brief System DMA API
 *
 * \version 0.0 (Aug 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_DMA_H_
#define _UTILS_DMA_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>
#include <ti/sdo/edma3/drv/edma3_drv.h>
#include <ti/sdo/edma3/rm/edma3_rm.h>
#include <ti/sdo/edma3/rm/src/edma3resmgr.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Default event Q to for EDMA's
 *******************************************************************************
 */
#define UTILS_DMA_DEFAULT_EVENT_Q   (1)

/**
 *******************************************************************************
 * \brief Max pitch that is possible with EDMA
 *******************************************************************************
 */
#define UTILS_DMA_MAX_PITCH         (32767)

/**
 *******************************************************************************
 * \brief Max planes that can be used DMA's via these DMA APIs
 *******************************************************************************
 */
#define UTILS_DMA_MAX_PLANES        (2)

/**
 *******************************************************************************
 * \brief Macro to generate YUV422I pattern for a 32-bit value
 *******************************************************************************
 */
#define UTILS_DMA_GENERATE_YUV422I_FILL_PATTERN(y,u,v) ((((y) & 0xFF) << 0)  | \
                                                        (((u) & 0xFF) << 8)  | \
                                                        (((y) & 0xFF) << 16) | \
                                                        (((v) & 0xFF) << 24))

/**
 *******************************************************************************
 *
 * \brief Default EDMA TC to use for EDMA
 *
 *******************************************************************************
 */
#define UTILS_DMA_DEFAULT_EVENT_Q           (1)

/**
 *******************************************************************************
 *
 * \brief Local EDMA instance ID
 *
 *******************************************************************************
 */
#define UTILS_DMA_SYSTEM_EDMA_INST_ID           (0)

/**
 *******************************************************************************
 *
 * \brief Local EDMA instance ID
 *
 *******************************************************************************
 */
#define UTILS_DMA_LOCAL_EDMA_INST_ID           (1)


/**
 *******************************************************************************
 *
 * \brief Maximum PaRAMs that will be linked during transfers
 *
 *******************************************************************************
 */#define UTILS_DMA_MAX_TX_OBJ                   (8)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

typedef struct
{
    Ptr    destAddr;
    /**< Physical address of destination buffer
     *   - For max efficiency recommended to be 32-byte aligned
     */

    Ptr    srcAddr;
    /**   Physical address of a source buffer
     *   - For max efficiency recommended to be 32-byte aligned
     */

    UInt32 length;
    /**   Number of bytes to be transferred
     */

}Utils_DmaCopy1D;


/**
 *******************************************************************************
 * \brief Copy/Fill 2D API parameters
 *******************************************************************************
 */
typedef struct
{
    UInt32 dataFormat;
    /**< Following data formats are supported,
     *   - SYSTEM_DF_RAW08 - can be used any 8-bit data format DMA copy
     *   - SYSTEM_DF_RAW16 - can be used any 16-bit data format DMA copy
     *                       like YUV422I or RGB565
     *   - SYSTEM_DF_RAW24 - can be used for RGB24-bit data format DMA copy
     *                       like RGB888
     *   - SYSTEM_DF_YUV420SP_UV
     */

    Ptr    destAddr[UTILS_DMA_MAX_PLANES];
    /**< Physical address of destination buffer
     *   - For max efficiency recommended to be 32-byte aligned
     */

    UInt32 destPitch[UTILS_DMA_MAX_PLANES];
    /**< Pitch in bytes of the destination buffer
     *   - For max efficiency recommended to be 32-byte aligned
     */

    UInt32 destStartX;
    /**< Start position, X-direction in destination buffer
     *   - Specfied in unit of pixels
     *   - MUST be multiple of 2 for YUV420SP
     *   - For max efficiency recommended to be 32-byte aligned
     */

    UInt32 destStartY;
    /**< Start position, Y-direction in destination buffer
     *   - Specfied in unit of lines
     *   - MUST be multiple of 2 of YUV420SP data format
     */

    UInt32 width;
    /**< Width of data to fill in destination buffer
     *   - Specfied in unit of pixels
     *   - MUST be multiple of 2 for YUV420SP
     *   - For max efficiency recommended to be 32-byte aligned
     */

    UInt32 height;
    /**< height of data to fill in destination buffer
     *   - Specfied in unit of lines
     *   - MUST be multiple of 2 for YUV420SP
     */

    Ptr    srcAddr[UTILS_DMA_MAX_PLANES];
    /**<
     *   For Utils_dmaCopy2D(),
     *   Physical address of a source buffer
     *   - For max efficiency recommended to be 32-byte aligned
     *
     *   For Utils_dmaFill2D(),
     *   Physical address of a source line buffer
     *   - User should make sure the buffer pointed to by srcAddr[]
     *     can hold one line of size 'width' pixels
     *   - User should pre-fill this buffer with the value he wants to fill
     *     across the buffer
     *   - For max efficiency recommended to be 32-byte aligned
     */

    UInt32 srcPitch[UTILS_DMA_MAX_PLANES];
    /**<
     *   For Utils_dmaCopy2D(),
     *   Pitch in bytes of the source buffer
     *   - For max efficiency recommended to be 32-byte aligned
     *
     *   For Utils_dmaFill2D(),
     *   - NOT USED
     */

    UInt32 srcStartX;
    /**<
     *   For Utils_dmaCopy2D(),
     *   Start position, X-direction in source buffer
     *   - Specfied in unit of pixels
     *   - MUST be multiple of 2 for YUV420SP
     *   - For max efficiency recommended to be 32-byte aligned
     *
     *   For Utils_dmaFill2D(),
     *   - NOT USED
     */

    UInt32 srcStartY;
    /**<
     *   For Utils_dmaCopy2D(),
     *   Start position, Y-direction in source buffer
     *   - Specfied in unit of lines
     *   - MUST be multiple of 2 of YUV420SP data format
     *
     *   For Utils_dmaFill2D(),
     *   - NOT USED
     */

} Utils_DmaCopyFill2D;

/**
 *******************************************************************************
 * \brief Information related one EDMA transfer
 *******************************************************************************
 */
typedef struct {

    UInt32               paramId;
    /**< PaRAM instance that is used for this EDMA */

    EDMA3_DRV_PaRAMRegs *pParamSet;
    /**< Pointer to physical area of this PaRAM */

} Utils_DmaTxObj;

/**
 *******************************************************************************
 *
 * \brief Information related one logical EDMA CH
 *
 *        - User should not set or modify any of the fields in this structure
 *        - This fields are initialized when using the DMA APIs and are used
 *          by the API internally
 *
 *******************************************************************************
 */
typedef struct {

    unsigned int edmaChId;
    /**< EDMA CH ID that is used for this EDMA */

    unsigned int tccId;
    /**< EDMA TCC ID that is used for this EDMA */

    unsigned int eventQ;
    /**< EDMA TC that is used for this EDMA */

    BspOsal_SemHandle semComplete;
    /**< EDMA completion semaphore */

    BspOsal_SemHandle semLock;
    /**< Multual exclusion semaphore for this logical channel */

    UInt32 maxTransfers;
    /**< Max possible linked transfers for this logical channel */

    Bool   enableIntCb;
    /**< Enable completion interrupt for this channel */

    Utils_DmaTxObj  txObj[UTILS_DMA_MAX_TX_OBJ];
    /**< Pointer to information for each linked transfer */

    EDMA3_DRV_Handle hEdma;
    /**< Handle to EDMA controller associated with this logical DMA channel */

} Utils_DmaChObj;

/**
 *******************************************************************************
 *
 * \brief Parameter to use when creating a logical DMA channel
 *
 *        To avoid compatibility issues, user should called
 *        Utils_DmaChCreateParams_Init() to set default values for all params
 *        in this structure
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 eventQ;
    /**< Event queue to use for this transfer */

    UInt32 maxTransfers;
    /**< Max possible EDMA transfer in a single API invocation */

    UInt32 enableIntCb;
    /**< Enable interrupt based EDMA completion waiting */

    UInt32 edmaInstId;
    /**< EDMA instace ID to use for this logical EDMA channel
     *   Valid values,
     *   - UTILS_DMA_SYSTEM_EDMA_INST_ID for M4, A15, DSP
     *   - UTILS_DMA_LOCAL_EDMA_INST_ID for DSP
     */

} Utils_DmaChCreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Set defaults for Utils_dmaCreateCh()
 *
 * \param  pPrm [OUT] Parameters to init to default value
 *
 *******************************************************************************
 */
static inline void Utils_DmaChCreateParams_Init(Utils_DmaChCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->eventQ = UTILS_DMA_DEFAULT_EVENT_Q;
    pPrm->maxTransfers = 1;
    pPrm->enableIntCb = TRUE;
    pPrm->edmaInstId = UTILS_DMA_SYSTEM_EDMA_INST_ID;

    if(System_getSelfProcId()==SYSTEM_PROC_EVE1
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE2
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE3
        ||
       System_getSelfProcId()==SYSTEM_PROC_EVE4
      )
    {
        pPrm->edmaInstId = UTILS_DMA_LOCAL_EDMA_INST_ID;
    }
}

Int32 Utils_dmaInit();
Int32 Utils_dmaDeInit();

Int32 Utils_dmaCreateCh(Utils_DmaChObj *pObj, Utils_DmaChCreateParams *pPrm);

Int32 Utils_dmaFill2D(Utils_DmaChObj *pObj, Utils_DmaCopyFill2D *pPrm,
                            UInt32 numTransfers);

Int32 Utils_dmaCopy2D(Utils_DmaChObj *pObj, Utils_DmaCopyFill2D *pPrm,
                            UInt32 numTransfers);

Int32 Utils_dmaCopy1D(Utils_DmaChObj *pObj, Utils_DmaCopy1D *pPrm);

Int32 Utils_dmaDeleteCh(Utils_DmaChObj *pObj);

EDMA3_DRV_Handle Utils_dmaGetEdma3Hndl(UInt32 edmaInstId);

Int32 Utils_dmaTestCopyFill(Int32 doInitDeInit);

#endif

/* @} */
