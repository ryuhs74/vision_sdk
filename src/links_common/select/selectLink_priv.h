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
 * \ingroup SELECT_LINK_API
 * \defgroup SELECT_LINK_IMPL Select Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file selectLink_priv.h Select Link private API/Data structures
 *
 * \brief  This file is a private header file for Select link implementation
 *         This file lists the data structures, function prototypes which are
 *         implemented and used as a part of Select link.
 *         Select Link is used in cases where the input buffers are selectively
 *         sent to next links.
 *
 * \version 0.0 (Nov 2013) : [CM] First version
 *
 *******************************************************************************
 */

#ifndef _SELECT_LINK_PRIV_H_
#define _SELECT_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/selectLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Maximum number of SELECT link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SELECT_LINK_OBJ_MAX    (2)

/**
 *******************************************************************************
 *
 * \brief Select Link channels not mapped
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SELECT_LINK_CH_NOT_MAPPED   (0xFFFF)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief
 *
 *  Channel link info to hold the information of the perticular channel
 *
 *
 *
 *******************************************************************************
 */
typedef struct {

    UInt32 queId;
    /**< Holds the queue Id of the channel */

    UInt32 outChNum;
    /**< Channel number of the output channel */

    Bool   rtChInfoUpdate;
    /**< Flag to indicate if the channel info has updated */

    System_LinkChInfo rtChInfo;
    /**< Link Channel Info */
} SelectLink_ChInfo;

/**
 *******************************************************************************
 *
 * \brief
 *
 * Structure to hold all Select link related information
 *
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 tskId;
    /**< Placeholder to store select link task id */

    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */

    Utils_TskHndl tsk;
    /**< Handle to select link task */

    SelectLink_CreateParams createArgs;
    /**< Create params for select link */

    System_LinkInfo inTskInfo;
    /**< Output queue information of previous link */

    System_LinkInfo info;
    /**< Output queue information of this link */

    Utils_BufHndl outFrameQue[SELECT_LINK_MAX_OUT_QUE];
    /**< Handles to the output queue */

    SelectLink_ChInfo   inChInfo[SYSTEM_MAX_CH_PER_OUT_QUE];
    /**< Input Channel info */

    SelectLink_OutQueChInfo   prevOutQueChInfo[SELECT_LINK_MAX_OUT_QUE];
    /**< Previous Channel Queue Info*/

    Utils_LinkStatistics linkStats;
    /**< Statistics related to this link */

    UInt32 getFrameCount;
    /**< Count of incoming frames */

    UInt32 putFrameCount;
    /**< Count of outgoing frames */

} SelectLink_Obj;


extern SelectLink_Obj gSelectLink_obj[];

Void SelectLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

Int32 SelectLink_tskCreate(UInt32 instId);

Int32 SelectLink_drvSetOutQueChInfo(SelectLink_Obj * pObj, SelectLink_OutQueChInfo *pPrm);
Int32 SelectLink_drvGetOutQueChInfo(SelectLink_Obj * pObj, SelectLink_OutQueChInfo *pPrm);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


