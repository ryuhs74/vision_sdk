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
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_REMAPMERGE Algorithm Plugin: Remap merge API
 *
 *
 * \brief  This module has the interface for using remap merge algorithm
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_remapMerge.h
 *
 * \brief Algorithm Plugin : Remap merge API
 *
 * \version 0.0 (Oct 2014) : [YM] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_REMAPMERGE_H_
#define _ALGORITHM_LINK_REMAPMERGE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Link CMD: To save/dump captured frame into extra frame
 *                    Dumps the frame allocated at CreateTime
 *                    If extra frame buffer is not allocated at create time
 *                      returns Error
 *
 *      Can be used to save frame only when capture is running
 *
 *******************************************************************************
 */
#define REMAP_LINK_CMD_SAVE_FRAME             (0x5000)

/**
 *******************************************************************************
 *
 *   \brief Link CMD: Return's pointer to saved frame
 *
 *******************************************************************************
 */
#define REMAP_LINK_CMD_GET_SAVE_FRAME_STATUS      (0x5001)


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

typedef enum
{
    remapMergeTileApproach      = 0,
    /**< More cycles, optimized on memory utilization */
    remapMergeBBApproach        = 1,
    /**< Optimized on cycles, increased memory utilization */
    remapMergeTileAndBBApproach = 2
    /**< TBD */
} RemapMerge_Approach;


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for feature plane
 *          computation algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 isValid;
    UInt8 *pCalibLUTBuf;
    Uint32 calibLUTBufSize;
}
AlgorithmLink_RemapMergeCalibBufParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for feature plane
 *          computation algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params. This structure should be first element */
    UInt32                   imageWidth;
    /**< Width of the input frame */
    UInt32                   imageHeight;
    /**< Height of the input frame */
    System_LinkOutQueParams  outQueParams;
    /**< Output queue information */
    System_LinkInQueParams   inQueParams;
    /**< Input queue information */
    UInt32                   numOutBuffers;
    /**< Number of output Buffers */
    RemapMerge_Approach      remapMergeApproach;
    /**< Remap merge approach - different approaches supported
         by remap merge algorithm */
    UInt32                   enableMerge;
    /**< Based on this 2nd stage (Merge stage) of the alg is used */
    UInt32                   dstFormat;
    /**< Output format */
    UInt32                   srcFormat;
    /**< Input format */
    UInt32                   coordMapList;
    /**< Input format */
    UInt32                   isSrcMapFloat;
    /**< Input format */
    UInt32                   srcStride;
    /**< Input format */
    UInt32                   srcHeight;
    /**< Input format */
    UInt32                   dstStride;
    /**< Input format */
    UInt32                   roiWidth;
    /**< Input format */
    UInt32                   roiHeight;
    /**< Input format */
    UInt32                   blockWidthBB;
    /**< Input format */
    UInt32                   blockHeightBB;
    /**< Input format */
    UInt32                   blockWidthTile;
    /**< Input format */
    UInt32                   blockHeightTile;
    /**< Input format */
    UInt32                   tileWidth;
    /**< Input format */
    UInt32                   tileHeight;
    /**< Input format */
    UInt32                   interpolationLuma;
    /**< Input format */
    UInt32                   interpolationChroma;
    /**< Input format */
    UInt32                   mapQshift;
    /**< Input format */
    UInt32                   rightShift;
    /**< Input format */
    UInt32                   sat_high;
    /**< Input format */
    UInt32                   sat_high_set;
    /**< Input format */
    UInt32                   sat_low;
    /**< Input format */
    UInt32                   sat_low_set;
    /**< Input format */

    UInt32                  allocBufferForRawDump;
    /**< [IN] Flag to allocate extra frame buffer for RAW dump
              1, extra frame buffer is allocated
              0, extra frame buffer is not allocated, so RAW frames
                 cannot be dumped */

    AlgorithmLink_RemapMergeCalibBufParams	calibLUTBufPrms;
} AlgorithmLink_RemapMergeCreateParams;

/**
 *******************************************************************************
 *  \brief Information of saved RAW data frame
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */

    UInt32 isSaveFrameComplete;
    /**< TRUE: Frame is saved at address mentioned in 'bufAddr'
     *   FALSE: Frame is not yet saved, try after some time
     */

    UInt32 bufAddr;
    /**< Address where frame is saved for both left and right channel*/

    UInt32 bufSize;
    /**< Size of buffer where frame is saved */

} AlgorithmLink_RemapMergeSaveFrameStatus;


/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Set defaults for plugin create parameters
 *
 * \param pPrm  [OUT] plugin create parameters
 *
 *******************************************************************************
 */
static inline void AlgorithmLink_RemapMerge_Init(
    AlgorithmLink_RemapMergeCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_REMAPMERGE;

    pPrm->imageWidth = 1280;
    pPrm->imageHeight  = 720;
    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
    pPrm->outQueParams.nextLink     = SYSTEM_LINK_ID_INVALID;

    pPrm->numOutBuffers             = 4;


    pPrm->isSrcMapFloat       = 0;
    pPrm->srcFormat           = 6; //YUV420
    pPrm->mapQshift           = 2;
    pPrm->interpolationLuma   = 1;
    pPrm->interpolationChroma = 0;
    pPrm->rightShift          = 0;
    pPrm->sat_high            = 255;
    pPrm->sat_high_set        = 255;
    pPrm->sat_low             = 0;
    pPrm->sat_low_set         = 0;
    pPrm->enableMerge                     = 0;
    pPrm->dstFormat                       = 6;

}


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of soft isp algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_RemapMerge_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
