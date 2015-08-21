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
 * \file system_linkApi_local.c
 *
 * \brief  Utility function to call link callback functions on local core.
 *
 *         This file implements the utility function to call the link call
 *         back functions on local core only.
 *
 * \version 0.0 (Apr 2014) : [YM] First version taken from bios side vision sdk
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "system_priv_common.h"


/**
 *******************************************************************************
 *
 * \brief Function to call link control command. This is through mailboxes.
 *
 *
 * \param   linkId      [IN] linkID for which message is intended
 * \param   cmd         [IN] Control command
 * \param   pPrm        [IN] Command parameter
 * \param   prmSize     [IN] Size of parameter
 * \param   waitAck     [IN] Flag to wait for ack or not.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */

Int32 System_linkControl_local(UInt32 linkId, UInt32 cmd, Void *pPrm, UInt32 prmSize, Bool waitAck)
{
    Int32 status;
    OSA_MbxHndl *pToMbx;
    OSA_TskHndl *pTsk;
    UInt32 flags=0;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    OSA_assert(  linkId < SYSTEM_LINK_ID_MAX);

    if(waitAck)
        OSA_mutexLock(&gSystem_objCommon.linkControlMutex);

    pTsk = System_getLinkTskHndl(linkId);

    pToMbx = &pTsk->mbxHndl;

    if(waitAck)
        flags = OSA_MBX_WAIT_ACK;

    status = OSA_mbxSendMsg(pToMbx,&gSystem_objCommon.mbx,  cmd, pPrm, flags);

    if(waitAck)
        OSA_mutexUnlock(&gSystem_objCommon.linkControlMutex);

    return status;
}


/**
 *******************************************************************************
 *
 * \brief Function to send message to link. This is through mailboxes.
 *
 *
 * \param   linkId      [IN] linkID for which message is intended
 * \param   cmd         [IN] command
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */

Int32 System_sendLinkCmd_local(UInt32 linkId, UInt32 cmd, Void *payload)
{
    return System_linkControl_local(linkId, cmd, NULL, 0, FALSE);
}

/**
 *******************************************************************************
 *
 * \brief Function to get link information from remote link on local processor.
 *
 *
 * \param   linkId      [IN] Link Id for which message is intended.
 * \param   info        [OUT] Link information
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */

Int32 System_linkGetInfo_local(UInt32 linkId, System_LinkInfo * info)
{
    System_LinkObj *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    OSA_assert(  linkId < SYSTEM_LINK_ID_MAX);

    pTsk = &gSystem_objCommon.linkObj[linkId];

    if(pTsk->getLinkInfo!=NULL)
        return (pTsk->getLinkInfo(pTsk->pTsk, info));

    return SYSTEM_LINK_STATUS_EFAIL;
}

