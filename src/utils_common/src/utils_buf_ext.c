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
 * \file utils_buf_ext.c
 *
 * \brief This file has the implementation of the system buffer exchange queue
 *        API
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <src/utils_common/include/utils_buf_ext.h>

Int32 Utils_bufCreateExt(Utils_BufHndlExt * pHndl, Bool blockOnGet,
                         Bool blockOnPut, UInt32 numAllocPools)
{
    Int32 status;
    UInt32 flags;
    Int i;

    flags = UTILS_QUE_FLAG_NO_BLOCK_QUE;

    if (blockOnGet)
        flags |= UTILS_QUE_FLAG_BLOCK_QUE_GET;
    if (blockOnPut)
        flags |= UTILS_QUE_FLAG_BLOCK_QUE_PUT;

    for (i = 0; i < numAllocPools; i++)
    {
        status = Utils_queCreate(&pHndl->emptyQue[i],
                                 UTILS_BUF_MAX_QUE_SIZE,
                                 pHndl->emptyQueMem[i], flags);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
    pHndl->numAllocPools = numAllocPools;

    status = Utils_queCreate(&pHndl->fullQue,
                             UTILS_BUF_MAX_QUE_SIZE, pHndl->fullQueMem, flags);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

Int32 Utils_bufDeleteExt(Utils_BufHndlExt * pHndl)
{
    Int i;

    for (i = 0; i < pHndl->numAllocPools; i++)
    {
       Utils_queDelete(&pHndl->emptyQue[i]);
    }
    pHndl->numAllocPools = 0;
    Utils_queDelete(&pHndl->fullQue);

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_bufGetEmptyExt(Utils_BufHndlExt * pHndl,
                           System_BufferList * pBufList,
                           UInt32 allocPoolID, UInt32 timeout)
{
    UInt32 idx, maxBufs;
    Int32 status;
    Utils_EncDecLinkPvtInfo *linkPvtInfo;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);

    if (timeout == BSP_OSAL_NO_WAIT)
        maxBufs = SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST;
    else
        maxBufs = pBufList->numBuf;

    UTILS_assert(maxBufs <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);
    UTILS_assert(allocPoolID < pHndl->numAllocPools);

    for (idx = 0; idx < maxBufs; idx++)
    {
        status =
            Utils_queGet(&(pHndl->emptyQue[allocPoolID]),
                         (Ptr *) & pBufList->buffers[idx], 1, timeout);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;
        linkPvtInfo =
            (Utils_EncDecLinkPvtInfo*) pBufList->buffers[idx]->pEncDecLinkPrivate;
        UTILS_assert(linkPvtInfo != NULL);

        linkPvtInfo->allocPoolID = allocPoolID;
    }

    pBufList->numBuf = idx;

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_bufGetEmptyBufferExt(Utils_BufHndlExt * pHndl,
                                System_Buffer ** pBuf,
                                UInt32 allocPoolID, UInt32 timeout)
{
    Int32 status;
    Utils_EncDecLinkPvtInfo *linkPvtInfo;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf != NULL);

    *pBuf = NULL;
    UTILS_assert(allocPoolID < pHndl->numAllocPools);
    status = Utils_queGet(&(pHndl->emptyQue[allocPoolID]),
                         (Ptr *) pBuf, 1, timeout);

    linkPvtInfo = (Utils_EncDecLinkPvtInfo *) ((*pBuf)->pEncDecLinkPrivate);
    UTILS_assert(linkPvtInfo != NULL);
    if (status == SYSTEM_LINK_STATUS_SOK)
    {
        linkPvtInfo->allocPoolID = allocPoolID;
    }

    return status;
}

Int32 Utils_bufPutEmptyExt(Utils_BufHndlExt * pHndl,
                           System_BufferList * pBufList)
{
    UInt32 idx;
    Int32 status;
    UInt32 allocPoolID;
    Utils_EncDecLinkPvtInfo *linkPvtInfo;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBuf <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < pBufList->numBuf; idx++)
    {
        linkPvtInfo =
            (Utils_EncDecLinkPvtInfo *) pBufList->buffers[idx]->pEncDecLinkPrivate;
        UTILS_assert(linkPvtInfo != NULL);
        allocPoolID = linkPvtInfo->allocPoolID;

        UTILS_assert(allocPoolID < pHndl->numAllocPools);
        status =
            Utils_quePut(&(pHndl->emptyQue[allocPoolID]),
                         pBufList->buffers[idx], BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_bufPutEmptyBufferExt(Utils_BufHndlExt * pHndl, System_Buffer * pBuf)
{
    Int32 status;
    Utils_EncDecLinkPvtInfo *linkPvtInfo;

    linkPvtInfo = (Utils_EncDecLinkPvtInfo *) pBuf->pEncDecLinkPrivate;
    UTILS_assert(pHndl != NULL);
    UTILS_assert(linkPvtInfo != NULL);
    UTILS_assert(linkPvtInfo->allocPoolID < pHndl->numAllocPools);

    status = Utils_quePut(&(pHndl->emptyQue[linkPvtInfo->allocPoolID]),
                         pBuf, BSP_OSAL_NO_WAIT);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_bufGetFullExt(Utils_BufHndlExt * pHndl,
                          System_BufferList * pBufList, UInt32 timeout)
{
    UInt32 idx, maxBufs;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);

    if (timeout == BSP_OSAL_NO_WAIT)
        maxBufs = SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST;
    else
        maxBufs = pBufList->numBuf;

    UTILS_assert(maxBufs <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < maxBufs; idx++)
    {
        status =
            Utils_queGet(&pHndl->fullQue, (Ptr *) & pBufList->buffers[idx], 1,
                         timeout);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;
    }

    pBufList->numBuf = idx;

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_bufGetFullBufferExt(Utils_BufHndlExt * pHndl,
                                System_Buffer ** pBuf, UInt32 timeout)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBuf != NULL);

    *pBuf = NULL;

    status = Utils_queGet(&pHndl->fullQue, (Ptr *) pBuf, 1, timeout);

    return status;
}

Int32 Utils_bufPutFullExt(Utils_BufHndlExt * pHndl,
                          System_BufferList * pBufList)
{
    UInt32 idx;
    Int32 status;

    UTILS_assert(pHndl != NULL);
    UTILS_assert(pBufList != NULL);
    UTILS_assert(pBufList->numBuf <= SYSTEM_MAX_BUFFERS_IN_BUFFER_LIST);

    for (idx = 0; idx < pBufList->numBuf; idx++)
    {
        status =
            Utils_quePut(&pHndl->fullQue, pBufList->buffers[idx],
                         BSP_OSAL_NO_WAIT);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_bufPutFullBufferExt(Utils_BufHndlExt * pHndl, System_Buffer * pBuf)
{
    Int32 status;

    UTILS_assert(pHndl != NULL);

    status = Utils_quePut(&pHndl->fullQue, pBuf, BSP_OSAL_NO_WAIT);
    if (status != SYSTEM_LINK_STATUS_SOK)
    {
#if 0
        Vps_printf
            (" ERROR: In Utils_bufPutFullBufExt(), Utils_quePut() failed !!!\n");
#endif
    }

    return status;
}

Void Utils_bufExtPrintStatus(UInt8 *str, Utils_BufHndlExt * pHndl)
{
    Uint8 i;

    Vps_printf("%s BufExt Q Status\n", str);
    for (i=0; i<pHndl->numAllocPools; i++)
    {
        Vps_printf(" Empty Q %d -> count %d, wrPtr %d, rdPtr %d\n", i,
        pHndl->emptyQue[i].count, pHndl->emptyQue[i].curWr, pHndl->emptyQue[i].curRd);
    }
    Vps_printf(" Full Q -> count %d, wrPtr %d, rdPtr %d\n", pHndl->fullQue.count,
        pHndl->fullQue.curWr, pHndl->fullQue.curRd);
}

