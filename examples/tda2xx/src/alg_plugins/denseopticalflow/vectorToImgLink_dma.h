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
 * \file vectorToImageLink_dma.h
 *
 * \brief  This file contains DMA APIs for use with vector to image conversion
 *
 * \version 0.0 (Nov 2013) : [KC] First version
 *
 *******************************************************************************
*/

#ifndef _VECTORTOIMAGE_LINK_DMA_H_
#define _VECTORTOIMAGE_LINK_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "vectorToImgLink_priv.h"
#include "src/utils_common/include/utils_dma.h"

/**
 * \brief Number of DMA channels used
 *
 *        0 - for input[0]
 *        1 - for input[1]
 *        2 - for output
 */
#define NUM_DMA_CH  (3)

/**
 * \brief Number of buffer used for ping/pong
 */
#define NUM_PING_PONG_BUF   (2)

/**
 * \brief number of lines in ping or pong buffer
 */
#define NUM_LINES_IN_BUF    (3)

/**
 * \brief Structure to hold information related to DMA processing for this
 *        algorithm
 */
typedef struct
{
    unsigned int edmaChId[NUM_DMA_CH];
    /**< EDMA channel ID's */

    unsigned int tccId[NUM_DMA_CH];
    /**< TCC ID of the EDMA channels */

    EDMA3_DRV_Handle hEdma;
    /**< Handle to EDMA controller */

    EDMA3_DRV_PaRAMRegs *pParamSet[NUM_DMA_CH];
    /**< Pointer to physical area of PaRAM for this channel */

    UInt8       *pLineBufVectorX[NUM_PING_PONG_BUF];
    /**< Ping/Pong line buffer address for input Vector X */

    UInt8       *pLineBufVectorY[NUM_PING_PONG_BUF];
    /**< Ping/Pong line buffer address for input Vector Y */

    UInt8       *pLineBufOutput[NUM_PING_PONG_BUF];
    /**< Ping/Pong line buffer address for output */

    UInt8       *pColorMapLut;
    /**< Pointer to LUT in internal memory */

    UInt8       *pAllocAddrL2;
    /**< Address of memory allocated from L2 */

    UInt32       allocSizeL2;
    /**< Size of memory to alloc from L2 */

    UInt32      inPitch;
    /**< Pitch of input in bytes */

    UInt32      outPitch;
    /**< Pitch of output in bytes */

    UInt32      inBytesPerLine;
    /**< Valid data size in bytes in a line */

    UInt32      outBytesPerLine;
    /**< Valid data size in bytes in a line */

    UInt32      numLines;
    /**< Number of lines in line buffer */

    UInt32      edmaOpt[NUM_DMA_CH];
    /**< Value used in EDMA CH OPT field */

    UInt32      channelEnableMaskH;
    /**< Mask to enable DMA channels */

    UInt32      channelEnableMaskL;
    /**< Mask to enable DMA channels */


} AlgorithmLink_VectorToImageDmaObj;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
