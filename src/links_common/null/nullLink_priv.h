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
 * \ingroup NULL_LINK_API
 * \defgroup NULL_LINK_IMPL Null Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file nullLink_priv.h Null Link private API/Data structures
 *
 * \brief  This file is a private header file for null link implementation
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of null link.
 *
 * \version 0.0 (Jul 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _NULL_LINK_PRIV_H_
#define _NULL_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/nullLink.h>
#include <src/utils_common/include/network_api.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Maximum number of null link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define NULL_LINK_OBJ_MAX    (2)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 ******************************************************************************
 *
 * \brief Structure to hold all Null link related to network receive
 *
 ******************************************************************************
 */
typedef struct {

    Network_SockObj sockObj;
    /**< Information about network socket */

    UInt32 state;
    /**< State of server socket */

} NullLink_NetworkTxObj;

/**
 *******************************************************************************
 *
 * \brief Information about frames dump memory and its current state
 *
 *******************************************************************************
 */
typedef struct {

    UInt16 chId;
    /**< Channel ID */

    UInt16 inQueId;
    /**< Input Que ID to which this channel belongs */

    UInt32 memAddr;
    /**< Base address to where frames should be copied */

    UInt32 memSize;
    /**< Size of memory segment */

    UInt32 curMemOffset;
    /**< Current offset in memory segment where frames should be copied */

    UInt32 numFrames;
    /**< Number of frames dumped to memory */

} NullLink_DumpFramesObj;

/**
 *******************************************************************************
 *
 * \brief Structure to hold all Dup link related information
 *
 *******************************************************************************
 */

typedef struct {
    UInt32 tskId;
    /**< Placeholder to store null link task id */

    Utils_TskHndl tsk;
    /**< Handle to null link task */

    NullLink_CreateParams createArgs;
    /**< Create params for null link */

    System_LinkQueInfo  inQueInfo[NULL_LINK_MAX_IN_QUE];
    /**< Input Queue info */

    NullLink_DumpFramesObj dumpFramesObj[NULL_LINK_MAX_IN_QUE][SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Information about frames dump memory and its current state */

    Utils_DmaChObj  dumpFramesDmaObj;
    /**< DMA object to use when dumping frames to memory */

    UInt32 recvCount;
    /**< Count of buffers received */

    char *dataDumpPtr;
    /* Memory Ptr to dump bitstream data */

    UInt32 dataDumpSize;
    /* Size of the Memory dump for bitstream data */

    NullLink_NetworkTxObj netTxObj;
    /**< Information related to sending data over network */

} NullLink_Obj;

Int32 NullLink_networkTxCreate(NullLink_Obj *pObj);
Int32 NullLink_networkTxDelete(NullLink_Obj *pObj);
Int32 NullLink_networkTxSendData(NullLink_Obj * pObj, UInt32 queId, UInt32 channelId,
                            System_Buffer *pBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */
