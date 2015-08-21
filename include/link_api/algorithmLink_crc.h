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
 * \defgroup ALGORITHM_LINK_CRC_API Algorithm Plugin: CRC API
 *
 * \brief  This module has the interface for using CRC algorithm
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_crc.h
 *
 * \brief Algorithm Link API specific to CRC algorithm
 *
 * \version 0.0 (May 2015) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHMLINK_CRC_H_
#define _ALGORITHMLINK_CRC_H_

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

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure to hold the CRC Signature object
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32   crcVal_L;
    /**< CRC signature - lower 32 bit value */
    UInt32   crcVal_H;
    /**< CRC signature - upper 32 bit value */
} AlgorithmLink_CrcSig_Obj;

/*******************************************************************************
 *  \brief CRC Algorithm Callback function to get the CRC signature
 *
 *         Use this function to add the logic for Frame freeze decision making.
 *         CRC Alg only return the CRC signature for each frame.
 *         The implementation in SDK example detect "frame freeze" scenario
 *         if the CRC signature of 5 consecutive frames are the same.
 *         Also sends the frame freeze detect notification to GRPX link Once
 *         it detect the frame Freeze
 *
 *******************************************************************************
 */
typedef Void (*AlgorithmLink_CrcSigCallback) (
                             AlgorithmLink_CrcSig_Obj *crcSig,
                             Void *appData);

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for CRC algorithm
 *
 *          This structure contains the create time parameters of CRC
 *          algorithm.
 *          Please note: roiWidth should be a multiple of 8, this is required
 *          because CRC can hold only 8 bytes at a time.
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params */
    UInt32                    startX;
    /**< X-Coordinate position of the frame */
    UInt32                    startY;
    /**< Y-Coordinate position of the frame  */
    UInt32                    roiHeight;
    /**< Height of the frame on which CRC to be performed */
    UInt32                    roiWidth;
    /**< Width of the frame on which CRC to be performed,
         Should be a multiple of 8 */
    Void                      *appData;
    /**< Private Application data */
    AlgorithmLink_CrcSigCallback cfgCbFxn;
    /**< CRC Algorithm Callback function to get the CRC signature */
    System_LinkInQueParams    inQueParams;
    /**< Input queue information */
    System_LinkMemAllocInfo   memAllocInfo;
    /**< Memory alloc region info, used to pass user alloc memory address */
} AlgorithmLink_CrcCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for CRC algorithm
 *
 *          This structure is a replica of control parameters of CRC
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_crcControlParams;

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
static inline void AlgorithmLink_Crc_Init (AlgorithmLink_CrcCreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->baseClassCreate.size  = sizeof(*pPrm);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_IPU_ALG_HW_CRC;

    pPrm->startX    = 0;
    pPrm->startY    = 0;
    pPrm->roiHeight = 800;
    pPrm->roiWidth  = 480;
    pPrm->appData   = NULL;
    pPrm->cfgCbFxn  = NULL;

    pPrm->inQueParams.prevLinkId    = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of CRC algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_Crc_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */

/* Nothing beyond this point */
