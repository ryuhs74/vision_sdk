/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
*/
#include "utils_mem_priv.h"
#include <ti/ipc/SharedRegion.h>

#ifdef UTILS_MEM_HEAP_DDR_CACHED_SIZE
#pragma DATA_ALIGN(gUtils_memHeapDDR, UTILS_MEM_SR_HEAP_MIN_ALIGN)
#pragma DATA_SECTION(gUtils_memHeapDDR, ".bss:heapMemDDR")
UInt8 gUtils_memHeapDDR[UTILS_MEM_HEAP_DDR_CACHED_SIZE];
#endif

#ifdef UTILS_MEM_HEAP_OCMC_SIZE
#pragma DATA_ALIGN(gUtils_memHeapOCMC, UTILS_MEM_SR_HEAP_MIN_ALIGN)
#pragma DATA_SECTION(gUtils_memHeapOCMC, ".bss:heapMemOCMC")
UInt8 gUtils_memHeapOCMC[UTILS_MEM_HEAP_OCMC_SIZE];
#endif

Int32 Utils_memHeapSetup()
{
    #ifdef UTILS_MEM_HEAP_DDR_CACHED_SIZE
    {
        Utils_MemHeapObj *pObj;
        HeapMem_Params heapParams;

        pObj = &gUtils_memHeapObj[UTILS_HEAPID_DDR_CACHED_SR];

        HeapMem_Params_init(&heapParams);

        heapParams.buf = gUtils_memHeapDDR;
        heapParams.size = sizeof(gUtils_memHeapDDR);

        HeapMem_construct(&pObj->heapStruct, &heapParams);

        pObj->heapAddr   = (UInt32)heapParams.buf;
        pObj->heapSize   = heapParams.size;
        pObj->heapHandle = HeapMem_Handle_upCast(
                            HeapMem_handle(&pObj->heapStruct)
                            );
    }
    #endif

    #ifdef UTILS_MEM_HEAP_OCMC_SIZE
    {
        Utils_MemHeapObj *pObj;
        HeapMem_Params heapParams;

        pObj = &gUtils_memHeapObj[UTILS_HEAPID_OCMC_SR];

        HeapMem_Params_init(&heapParams);

        heapParams.buf = gUtils_memHeapOCMC;
        heapParams.size = sizeof(gUtils_memHeapOCMC);

        HeapMem_construct(&pObj->heapStruct, &heapParams);

        pObj->heapAddr   = (UInt32)heapParams.buf;
        pObj->heapSize   = heapParams.size;
        pObj->heapHandle = HeapMem_Handle_upCast(
                            HeapMem_handle(&pObj->heapStruct)
                            );
    }
    #endif

    #ifdef ENABLE_HEAP_SR0
    {
        Utils_MemHeapObj *pObj;
        SharedRegion_Entry srEntry;

        pObj = &gUtils_memHeapObj[UTILS_HEAPID_DDR_NON_CACHED_SR0];

        SharedRegion_entryInit(&srEntry);
        SharedRegion_getEntry(0, &srEntry);

        pObj->heapAddr   = (UInt32)srEntry.base;
        pObj->heapSize   = srEntry.len;
        pObj->heapHandle = SharedRegion_getHeap(0);
    }
    #endif

    return SYSTEM_LINK_STATUS_SOK;
}

IHeap_Handle Utils_memGetHeapHandleSR(UInt32 heapId)
{
    IHeap_Handle heapHandle = NULL;

    if(heapId == UTILS_HEAPID_DDR_CACHED_SR
        ||
       heapId == UTILS_HEAPID_OCMC_SR
        ||
       heapId == UTILS_HEAPID_DDR_NON_CACHED_SR0
        )
    {
        heapHandle = gUtils_memHeapObj[heapId].heapHandle;
    }

    return heapHandle;
}

Int32 Utils_memAllocSR(SystemCommon_AllocBuffer *pPrm)
{
    volatile Ptr addr = NULL;
    UInt32 align, size;
    Error_Block ebObj;
    Error_Block *eb = &ebObj;
    IHeap_Handle heapHandle = NULL;

    Error_init(eb);

    align = SystemUtils_align(pPrm->align, UTILS_MEM_SR_HEAP_MIN_ALIGN);
    size  = SystemUtils_align(pPrm->size, UTILS_MEM_SR_HEAP_MIN_ALIGN);

    heapHandle = Utils_memGetHeapHandleSR(pPrm->heapId);

    if(heapHandle)
    {
            /*
             * Heap is present in this CPU, allocate memory
             */
            addr = Memory_alloc(
                        heapHandle,
                        size,
                        align,
                        eb);
    }

    pPrm->bufferPtr = (UInt32)addr;

    /* read back value to ensure previous write, if it is to shared memory gets
     * written completely to memory
     */
    addr = (Ptr)pPrm->bufferPtr;

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Utils_memFreeSR(SystemCommon_FreeBuffer *pPrm)
{
    UInt32 size;
    IHeap_Handle heapHandle = NULL;

    size  = SystemUtils_align(pPrm->size, UTILS_MEM_SR_HEAP_MIN_ALIGN);

    heapHandle = Utils_memGetHeapHandleSR(pPrm->heapId);

    if(heapHandle)
    {
        /*
         * Heap is present in this CPU, free memory
         */
        Memory_free(
                    heapHandle,
                    (Ptr)pPrm->bufferPtr,
                    size);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
