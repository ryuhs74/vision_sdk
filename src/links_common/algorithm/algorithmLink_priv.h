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
 * \ingroup ALGORITHM_LINK_API
 * \defgroup ALGORITHM_LINK_IMPL Algorithm Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_priv.h Algorithm Link private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 *
 *******************************************************************************
 */

#ifndef _ALGORITHM_LINK_PRIV_H_
#define _ALGORITHM_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <src/links_common/system/system_priv_common.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include "algorithmLink_cfg.h"

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Max Number of algorithm link queues (I/P or O/P) per link
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_MAX_NUMQUEUES                     (3)

/**
 *******************************************************************************
 *
 *   \brief Max Number of channels per queue of algorithm link
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_MAX_NUMCHPERQUEUE                     (5)

/**
 *******************************************************************************
 *
 *   \brief Max Number of system buffers per link queue (I/P or O/P)
 *
 *   SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define ALGORITHM_LINK_MAX_QUEUELENGTH     (16*ALGORITHM_LINK_MAX_NUMCHPERQUEUE)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the algorithm buffer release status
 *
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_RELSTATUS_NOTREADY = 0,
    /**< Not yet ready to be released */
    ALGORITHM_LINK_RELSTATUS_READY,
    /**< Ready to be released */
    ALGORITHM_LINK_RELSTATUS_ERROR,
    /**< Error occured in status update functionN */
    ALGORITHM_LINK_RELSTATUS_FORCE32BITS = 0x7FFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_ReleaseStatus;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Data Structure for the Algorithm link Buffer free status
 *
 *          In algorithm link a buffer can be freed up only after both the
 *          current link and the next link which uses this buffer, free it up.
 *          Below structure is defined to do this book keeping.
 *
 *******************************************************************************
*/
typedef struct
{
    System_Buffer *pBuff;
    /**<  Pointer of the system buffer */
} AlgorithmLink_BufferFreeStatus;

/**
 *******************************************************************************
 *
 *   \brief Data Structure for the Que of system buffers
 *
 *   Que handle and the associated memory for queue elements are grouped.
 *
 *******************************************************************************
*/
typedef struct {
    Utils_QueHandle    queHandle;
    /**< Handle to the queue for this channel */
    System_Buffer      *queMem[ALGORITHM_LINK_MAX_QUEUELENGTH];
    /**< Queue memory */
} AlgorithmLink_SysBufferQue;

/**
 *******************************************************************************
 *
 *   \brief Algorithm link instance object
 *
 *          This structure is defined as a common object for all algorithm
 *          links.
 *          There is a pointer "algorithmParams" which shall point to
 *          structure containing algorithm specific parameters. This specific
 *          structure will be defined as per algorithm in the corresponding link
 *          file.
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 linkId;
    /**< placeholder to store the Algorithm link task Id */
    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */
    Utils_TskHndl tsk;
    /**< placeholder to store the Algorithm link task handler */
    UInt32 algId;
    /**< Id of the algorithm used in this link */
    AlgorithmLink_CreateParams createArgs;
    /**< placeholder to store the Algorithm link create parameters */
    void * algorithmParams;
    /**< pointer to structure which holds all algorithm specific parameters */
    System_LinkInfo linkInfo;
    /**< Specifies a place holder that describe the LINK information */
    Int32 numInputQUsed;
    /**< Number of input queues used by this algorithm link */
    Int32 numOutputQUsed;
    /**< Number of output queues used by this algorithm link */
    AlgorithmLink_InputQueueInfo inputQInfo[ALGORITHM_LINK_MAX_NUMQUEUES];
    /**< Information about the output queues */
    AlgorithmLink_OutputQueueInfo outputQInfo[ALGORITHM_LINK_MAX_NUMQUEUES];
    /**< Information about the output queues */
    AlgorithmLink_SysBufferQue emptyOutputQ[ALGORITHM_LINK_MAX_NUMQUEUES]
                                           [ALGORITHM_LINK_MAX_NUMCHPERQUEUE];
    /**< Queues to hold empty output buffers. Seperate Q for each channel */
    AlgorithmLink_SysBufferQue fullOutputQ[ALGORITHM_LINK_MAX_NUMQUEUES];
    /**< Queue to hold full output buffers. Single Q for all chanls of givenQ */
    AlgorithmLink_BufferFreeStatus inputQFreeStatus
                                         [ALGORITHM_LINK_MAX_NUMQUEUES]
                                         [ALGORITHM_LINK_MAX_QUEUELENGTH];
    /**< Needed for book keeping related to release (freeing up) of input
     * buffers
     */
    AlgorithmLink_BufferFreeStatus outputQFreeStatus
                                          [ALGORITHM_LINK_MAX_NUMQUEUES]
                                          [ALGORITHM_LINK_MAX_QUEUELENGTH];
    /**< Needed for book keeping related to release (freeing up) of output
     * buffers
     */
    BspOsal_SemHandle lock;
    /**< Link level lock, used while updates the link params */
    UInt32 totalTime;
    /**< Total time duration for link, used to calculate the FPS */
    UInt32 startTime;
    /**< Start time of link, used to calculate the link stats */
    UInt32 prevTime;
    /**< Time info used to calculate some of the link stats */
    UInt32 minCbTime;
    /**< Minimum call back time noticed over the run */
    UInt32 maxCbTime;
    /**< Maximum call back time identified over the run */
    UInt32 lastCbTime;
    /**< The last/latest call back time */

    UInt32 memUsed[UTILS_MEM_MAXHEAPS];
    /**< Memory used by this link */

    AlgorithmLink_AlgPluginPutEmptyBuffers callbackPutEmptyBuffers;
    /**< User specified callback to call before releasing buffers */

} AlgorithmLink_Obj;

/**
 *******************************************************************************
 *
 * \brief Extern gAlgorithmLinkFuncTable
 *
 *******************************************************************************
 */
extern AlgorithmLink_FuncTable
        gAlgorithmLinkFuncTable[ALGORITHM_LINK_ALG_MAXNUM];

/**
 *******************************************************************************
 *
 * \brief Extern gAlgorithmLink_obj
 *
 *******************************************************************************
 */
extern AlgorithmLink_Obj gAlgorithmLink_obj[ALGORITHM_LINK_OBJ_MAX];

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_tskRun(AlgorithmLink_Obj * pObj, Utils_TskHndl * pTsk);
Void AlgorithmLink_tskMain(struct Utils_TskHndl *pTsk, Utils_MsgHndl *pMsg);
Int32 AlgorithmLink_init();
Int32 AlgorithmLink_deInit();
Int32 AlgorithmLink_getInfo(Void * ptr, System_LinkInfo * info);
Int32 AlgorithmLink_getFullBuffers(Void              * ptr,
                                   UInt16              queId,
                                   System_BufferList * pBufList);
Int32 AlgorithmLink_putEmptyBuffers(Void              *ptr,
                                    UInt16             queId,
                                    System_BufferList *pBufList);
AlgorithmLink_ReleaseStatus AlgorithmLink_sysBufRelStatusUpdate(
                        System_Buffer                   *pSysBuffer,
                        AlgorithmLink_BufferFreeStatus  *pBufferFreeStatusBase);

Int32 AlgorithmLink_tskCreate(UInt32 instId);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
