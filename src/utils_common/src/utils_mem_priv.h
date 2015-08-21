/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef _UTILS_MEM_PRIV_H_
#define _UTILS_MEM_PRIV_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils_mem.h>
#include <src/utils_common/include/utils_mem_cfg.h>
#include <include/link_api/system.h>
#include <include/link_api/system_common.h>

/* #define UTILS_MEM_DEBUG */

/*
 * SR heaps are defined on IPU1-0,
 * When DSP/EVE/A15/IPU1-1 asks for memory, this memory is allocated by IPU1-0
 * Since heap maintains a data structure in memory, we need to make sure this data
 * structure is not corrupted by unaligned cache operations on DSP/A15/IPU1-0
 *
 * Hence a minimum alignment which is >= max cache line lenght across all CPUs is
 * required.
 *
 * This define is used to specific this minimum alignment
 */
#define UTILS_MEM_SR_HEAP_MIN_ALIGN     (512)


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

typedef struct {

    HeapMem_Struct heapStruct;
    /**< Object to hold heap information */

    IHeap_Handle heapHandle;
    /**< Heap handle */

    Bool         isClearBufOnAlloc;
    /**< Flag to contorl clear the allocated memory with 0x80 pattern */

    char   heapName[32];
    /**< Name of heap */

    UInt32 heapAddr;
    /**< Physical base address of heap */

    UInt32 heapSize;
    /**< Total size of heap in bytes */

    UInt32 heapAllocOffset;
    /**< Heap alloc offset, only valid for L2 heap */

} Utils_MemHeapObj;


extern Utils_MemHeapObj gUtils_memHeapObj[UTILS_HEAPID_MAXNUMHEAPS];

/*******************************************************************************
 *  Functions
 *******************************************************************************
*/

Int32 Utils_memHeapSetup();

#endif

/* @} */
