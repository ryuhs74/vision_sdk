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
 * \version 0.0 (Jun 2013) : [KC] First version taken from DVR RDK and
 *                                cleaned up for Vision_sdk
 * \version 0.1 (Jul 2013) : [HS] Commenting style update as per defined
 *                                format.
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
Int32 System_linkControl_local(UInt32 linkId, UInt32 cmd, Void * pPrm,
                               UInt32 prmSize, Bool waitAck)
{
    Int32 status;
    Utils_MbxHndl *pToMbx;
    Utils_TskHndl *pTsk;
    UInt32 flags = 0;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk   = (Utils_TskHndl*) gSystem_objCommon.linkObj[linkId].pTsk;
    if(pTsk==NULL)
    {
        Vps_printf(" SYSTEM: ERROR: No link registered at link ID [%d] !!!\n", linkId);
        return SYSTEM_LINK_STATUS_EFAIL;
    }
    pToMbx = &pTsk->mbx;

    if (waitAck)
        flags = UTILS_MBX_FLAG_WAIT_ACK;

    status = Utils_mbxSendMsg(&gSystem_objCommon.mbx, pToMbx, cmd, pPrm, flags);

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
 * \param   payload     [IN] Payload of command
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success.
 *
 *******************************************************************************
 */
Int32 System_sendLinkCmd_local(UInt32 linkId, UInt32 cmd, Void *payload)
{
    Utils_MbxHndl *pToMbx;
    Utils_TskHndl *pTsk;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pTsk   = (Utils_TskHndl*) gSystem_objCommon.linkObj[linkId].pTsk;
    pToMbx = &pTsk->mbx;

    return Utils_mbxSendCmd(pToMbx, cmd, payload);
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
    System_LinkObj *pLinkObj;

    linkId = SYSTEM_GET_LINK_ID(linkId);

    UTILS_assert(linkId < SYSTEM_LINK_ID_MAX);

    pLinkObj = &gSystem_objCommon.linkObj[linkId];

    if (pLinkObj->getLinkInfo != NULL)
        return pLinkObj->getLinkInfo(pLinkObj->pTsk, info);

    return FVID2_EFAIL;
}
