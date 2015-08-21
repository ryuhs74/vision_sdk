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
 * \ingroup UTILS_MEM_API
 * \defgroup UTILS_MEM_DEBUG_API Memory allocator debug log API
 *
 * \brief  APIs to allocate system buffer memory from a predefined memory pool
 *
 * @{
 *
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \file utils_mem_debug.h
 *
 * \brief Memory allocator debug log API
 *
 * \version 0.0 First version
 * \version 0.1 Updates as per code review comments
 *
 *******************************************************************************
*/

#ifndef _UTILS_MEM_DEBUG_H_
#define _UTILS_MEM_DEBUG_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/utils_common/include/utils.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Control to enable memory log
 *******************************************************************************
*/
#define UTILS_MEM_ENABLE_MEMLOG                                           (TRUE)

/**
 *******************************************************************************
 * \brief Control to enable SRheap memory log
 *******************************************************************************
*/
#define UTILS_MEM_ENABLE_MEMLOG_SRHEAP                                    (TRUE)

#ifdef UTILS_MEM_ENABLE_MEMLOG
#ifdef UTILS_MEM_ENABLE_MEMLOG_SRHEAP
#include <ti/ipc/SharedRegion.h>
#endif

/**
 *******************************************************************************
 * \brief Maximum number of heaps
 *******************************************************************************
*/
#define  UTILS_MEM_MAXHEAPS                                                 (8)

/*******************************************************************************
 *  Functions
 *******************************************************************************
*/

/**
 *******************************************************************************
 *
 * \brief Function to get free memory stats from all the heaps in the system
 *
 * \param  freeSize    [IN] Pointer to freeSize array to be populated
 * \param  nMaxSize    [IN] Max size of free size array
 *
 * \return  Number of heaps in the system
 *
 *******************************************************************************
*/
static Int utilsmem_memstats_getfreestats(UInt32 freeSize[], UInt32 nMaxSize)
{

#ifdef UTILS_MEM_ENABLE_MEMLOG_SRHEAP
    SharedRegion_Entry srEntry;
#endif

    Int i;
    Int heapIndex = 0;
    Memory_Stats memstats;
    Int numStaticHeaps = 0;
    Int numDynamicHeaps = 0;
    HeapMem_Handle nextHeap, curHeap;

    numStaticHeaps = HeapMem_Object_count();
    for (i = 0; i < numStaticHeaps; i++)
    {
        IHeap_Handle heapHandle = HeapMem_Handle_upCast(
                                    HeapMem_Object_get(NULL,i));

        Memory_getStats(heapHandle, &(memstats));
        UTILS_assert((nMaxSize > heapIndex));
        freeSize[heapIndex++] = memstats.totalFreeSize;
    }
    nextHeap = HeapMem_Object_first();

    do {
        curHeap = nextHeap;
        if (NULL != curHeap)
        {
            IHeap_Handle heapHandle = HeapMem_Handle_upCast(curHeap);

            numDynamicHeaps++;
            Memory_getStats(heapHandle, &(memstats));
            UTILS_assert((nMaxSize > heapIndex));
            freeSize[heapIndex++] = memstats.totalFreeSize;
            nextHeap = HeapMem_Object_next(curHeap);
        }
    } while (nextHeap != NULL);

#ifdef UTILS_MEM_ENABLE_MEMLOG_SRHEAP
    for (i = 0; i < SharedRegion_getNumRegions(); i++)
    {
        SharedRegion_getEntry(i, &srEntry);
        if (TRUE == srEntry.isValid)
        {
            if (TRUE == srEntry.createHeap)
            {
                IHeap_Handle srHeap = SharedRegion_getHeap(i);

                UTILS_assert((nMaxSize > heapIndex));
                Memory_getStats(srHeap, &(memstats));
                freeSize[heapIndex++] = memstats.totalFreeSize;
            }
        }
    }
#endif
    return heapIndex;
}

/**
 *******************************************************************************
 *
 * \brief  Function to get used memory stats from all the heaps in the system
 *
 * \param  initialfreeSize        [IN] Pointer to initial free size array
 * \param  usedSize               [IN] Pointer to used size array to be populated
 * \param  numHeaps               [IN] Number of heaps in the system
 *
 * \return None
 *
 *******************************************************************************
*/
static Void utilsmem_memstats_getusedsize(UInt32 initialfreeSize[],
                                          UInt32 usedSize[],
                                          UInt32 numHeaps)
{
    Int i;
    UInt32 currentfreeSize[UTILS_MEM_MAXHEAPS];
    UInt32 numHeapsEnd;

    UTILS_assert((numHeaps <= UTILS_MEM_MAXHEAPS));
    numHeapsEnd = utilsmem_memstats_getfreestats(&(currentfreeSize[0]),
                                                 UTILS_MEM_MAXHEAPS);
    UTILS_assert((numHeapsEnd == numHeaps));

    for (i = 0; i < numHeaps; i++)
    {
        if((initialfreeSize[i] >= currentfreeSize[i]))
        {
            usedSize[i] = initialfreeSize[i] - currentfreeSize[i];
        }
        else
        {
            usedSize[i] = 0;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief  Function to get freed memory stats from all the heaps in the system
 *
 * \param  initialfreeSize      [IN] Pointer to initial free size array
 * \param  freedSize            [IN] Pointer to freed size array to be populated
 * \param  numHeaps             [IN] Number of heaps in the system
 *
 * \return None
 *
 *******************************************************************************
*/
static Void utilsmem_memstats_getfreedsize(UInt32 initialfreeSize[],
                                           UInt32 freedSize[],
                                           UInt32 numHeaps)
{
    Int i;
    UInt32 currentfreeSize[UTILS_MEM_MAXHEAPS];
    UInt32 numHeapsEnd;

    UTILS_assert((numHeaps <= UTILS_MEM_MAXHEAPS));
    numHeapsEnd = utilsmem_memstats_getfreestats(&(currentfreeSize[0]),
                                                 UTILS_MEM_MAXHEAPS);
    UTILS_assert((numHeapsEnd == numHeaps));

    for (i = 0; i < numHeaps; i++)
    {
        if((currentfreeSize[i] >= initialfreeSize[i]))
        {
            freedSize[i] = currentfreeSize[i] - initialfreeSize[i];
        }
        else
        {
            freedSize[i] = 0;
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief  Function to check if memory leak occured
 *
 * \param  allocSize         [IN] Pointer to alloc size array
 * \param  freedSize         [IN] Pointer to freed size array
 * \param  numHeaps          [IN] Number of heaps in the system
 * \param  id                [IN] ID used to identify stage when printing leaks.
 *
 * \return None
 *
 *******************************************************************************
*/
static Void utilsmem_memstats_checkleak(UInt32 allocSize[],
                                        UInt32 freedSize[],
                                        UInt32 numHeaps,
                                        UInt32 id)
{
    Int i;

    for (i = 0; i < numHeaps; i++)
    {
        if (allocSize[i] != freedSize[i])
        {
            Vps_printf(" UTILS: MEM: MemoryLeak: STAGE:%d HEAPNUM:%d "
                        "ALLOC=%d FREED=%d\n",
                       id, i, allocSize[i],
                       freedSize[i]);
        }
    }
}

#endif

#ifdef UTILS_MEM_ENABLE_MEMLOG
#define UTILS_MEMLOG_USED_START()  do {                                                     \
                                        UInt32 nNumHeaps =      0;                          \
                                        UInt32 freeMemStart[UTILS_MEM_MAXHEAPS];            \
                                                                                            \
                                        nNumHeaps =                                         \
                                        utilsmem_memstats_getfreestats(freeMemStart,        \
                                                                       UTILS_MEM_MAXHEAPS)
#else
#define UTILS_MEMLOG_USED_START()
#endif

#ifdef UTILS_MEM_ENABLE_MEMLOG
#define UTILS_MEMLOG_USED_END(pUsedSize)                                                   \
                                        utilsmem_memstats_getusedsize(freeMemStart,        \
                                                                      pUsedSize,           \
                                                                      nNumHeaps);          \
                                      } while (0)
#else
#define UTILS_MEMLOG_USED_END(pUsedSize)
#endif

#ifdef UTILS_MEM_ENABLE_MEMLOG
#define UTILS_MEMLOG_FREE_START()  do {                                                   \
                                        UInt32 nNumHeaps =      0;                        \
                                        UInt32 freeMemStart[UTILS_MEM_MAXHEAPS];          \
                                        UInt32 freedSize[UTILS_MEM_MAXHEAPS];             \
                                                                                          \
                                        nNumHeaps =                                       \
                                        utilsmem_memstats_getfreestats(freeMemStart,      \
                                                                       UTILS_MEM_MAXHEAPS)
#else
#define UTILS_MEMLOG_FREE_START()
#endif

#ifdef UTILS_MEM_ENABLE_MEMLOG
#define UTILS_MEMLOG_FREE_END(pAllocSize,id)                                              \
                                        utilsmem_memstats_getfreedsize(freeMemStart,      \
                                                                          freedSize,      \
                                                                          nNumHeaps);     \
                                        utilsmem_memstats_checkleak(pAllocSize,           \
                                                                    freedSize,            \
                                                                    nNumHeaps,            \
                                                                    id);                  \
                                   } while (0)
#else
#define UTILS_MEMLOG_FREE_END(pAllocSize,id)
#endif

#ifdef UTILS_MEM_ENABLE_MEMLOG
#define UTILS_MEMLOG_PRINT(str,pUsedSizeArray,sizeOfArray)                    \
                      do {                                                    \
                          Int i;                                              \
                                                                              \
                          for (i = 0; i < sizeOfArray; i++) {                 \
                              if (pUsedSizeArray[i] != 0) {                   \
                                  Vps_printf (" %s HEAPID: %d, MEM USED: %d\n",\
                                              str,i, pUsedSizeArray[i]);      \
                              }                                               \
                          }                                                   \
                      } while (0)
#else
#define UTILS_MEMLOG_PRINT(str,pUsedSizeArray,sizeOfArray)
#endif

#endif

/* @} */
