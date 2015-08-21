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
 *   \ingroup FRAMEWORK_MODULE_API
 *   \defgroup NULL_LINK_API Null Link API
 *
 *   Null Link can be used to take input from a link and then without doing
 *   anything return it back to the same link. This useful when a link output
 *   cannot be given to any other link for testing purpose we just want to run
 *   a given link but not really use the output. In such cases the output queue
 *   of link can be connected to a Null link. The null link will operate like
 *   any other link from interface point of view. But it wont do anything with
 *   the frames it gets. It will simply return it back to the sending link.
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file nullLink.h
 *
 * \brief Null link API public header file.
 *
 * \version 0.0 (Jul 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _NULL_LINK_H_
#define _NULL_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <include/link_api/networkCtrl_if.h>
#include <include/link_api/system.h>

/* @{ */

/**

*******************************************************************************
 *
 * \brief Max output queues supported by Null Link
 *
 * SUPPORTED in ALL platforms
 *

*******************************************************************************
*/
#define NULL_LINK_MAX_IN_QUE        (4)

/* @} */

/**
 ******************************************************************************
 * \brief Null link data Copy types.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef enum {
    NULL_LINK_COPY_TYPE_NONE = 0,
    /**< No dumping enabled */
    NULL_LINK_COPY_TYPE_2D_MEMORY,
    /**< For dumping video frames to memory. This uses the DMA driver */
    NULL_LINK_COPY_TYPE_BITSTREAM_MEMORY,
    /**< For dumping bitstream to memory. This uses CPU based memcpy */
    NULL_LINK_COPY_TYPE_BITSTREAM_FILE,
    /**< For dumping bitstream to file. This uses CPU based memcpy */
    NULL_LINK_COPY_TYPE_NETWORK,
    /**< For dumping buffer data, frames/bitstream/meta data over network */
     NULL_LINK_COPY_TYPE_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */
}NullLink_CopyType;

/******************************************************************************
 *
 *  Data structures
 *
*******************************************************************************
*/

/**
 ******************************************************************************
 * \brief Null link configuration parameters.
 *
 * SUPPORTED in ALL platforms
 *
*******************************************************************************
*/
typedef struct
{
    UInt32  numInQue;
    /**< Number of input queues */

    System_LinkInQueParams   inQueParams[NULL_LINK_MAX_IN_QUE];
    /**< Input queue information */

    NullLink_CopyType  dumpDataType;
    /**< Copies frames captured by NULL link to pre-defined location in DDR
     *   memory.
     *
     *   User can then via CCS save those frames to Host PC.
     *
     *   Frames will be copied one after other in a cricular manner in
     *   the memory segment specified
     *   via dumpFramesMemoryAddr, dumpFramesMemorySize.
     *
     *   Only Video frames will dumped to memory.
     *   Only CH0 from input que 0 will be dumped to memory.
     */

    UInt32  dumpFramesMemoryAddr;
    /**< Valid only when dumpDataType is
     *   COPY_TYPE_2D_MEMORY or COPY_TYPE_BITSTREAM_MEMORY
     *   Address in memory where the frames should be copied to
     */
    UInt32  dumpFramesMemorySize;
    /**< Valid only when dumpDataType is
     *   COPY_TYPE_2D_MEMORY or COPY_TYPE_BITSTREAM_MEMORY
     *   Size of memory into which the frames will be copied
     */

    char  nameDataFile[260];
    /**< Valid only when dumpDataType is COPY_TYPE_BITSTREAM_FILE
     *   File containing the stream data. This is a binary file.
     *   260 is filename size limit set by WIndows 7
     *   This file resides on local machine and used only for the purpose of
     *   debugging with CCS
     */

    UInt32 networkServerPort;
    /**< Server port ot use when dumpDataType is
     *   NULL_LINK_COPY_TYPE_NETWORK
     */

} NullLink_CreateParams;

/******************************************************************************
*
*  Functions
*
*******************************************************************************
*/

/**
*******************************************************************************
 *
 * \brief Null link register and init
 *
 *    - Creates link task
 *    - Registers as a link with the system API
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 NullLink_init();


/**
*******************************************************************************
 *
 * \brief Null link de-register and de-init
 *
 *    - Deletes link task
 *    - De-registers as a link with the system API
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 NullLink_deInit();

/**
 *******************************************************************************
 *
 * \brief Null link set default parameters for create time params
 *   This function does the following
 *      - memset create params object
 * \param  pPrm  [OUT]  NullLink Create time Params
 *
 *******************************************************************************
 */
static inline void NullLink_CreateParams_Init(NullLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->numInQue = 1;
    pPrm->networkServerPort = NETWORK_TX_SERVER_PORT;
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/
