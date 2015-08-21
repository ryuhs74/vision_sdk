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
 * \file utils_mem.c
 *
 * \brief  This file has the implementataion of Display Control Driver Calls
 *
 *         This file implements the calls to display controller driver.
 *         Calls to driver create, control commands, deletion is done in this
 *         file. Conversion / population of FVID2 parameters based on
 *         parameters of link API structures happen here.|
 *
 * \version 0.0 (Jun 2013) : [PS] First version
 * \version 0.1 (Jul 2013) : [PS] Updates as per code review comments
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
*/
#include "utils_mem_priv.h"


/**
 *******************************************************************************
 * \brief Macro to enable the memory/Heap debug
 *******************************************************************************
*/




#ifdef ENABLE_HEAP_L2

/**
 *******************************************************************************
 * \brief L2 heap memory
 *******************************************************************************
*/
#pragma DATA_ALIGN(gUtils_memHeapL2, 4)
#pragma DATA_SECTION(gUtils_memHeapL2, ".bss:heapMemL2")
UInt8 gUtils_memHeapL2[UTILS_MEM_HEAP_L2_SIZE];

#endif

Utils_MemHeapObj gUtils_memHeapObj[UTILS_HEAPID_MAXNUMHEAPS];

/**
 *******************************************************************************
 *
 * \brief One time system init of memory allocator
 *
 *        Should be called by application before using allocate APIs
 *
 * \return SYSTEM_LINK_STATUS_SOK on sucess, else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memInit()
{
    extern const IHeap_Handle Memory_defaultHeapInstance;
    Utils_MemHeapStats memStats;
    Utils_HeapId       heapId;

    memset(gUtils_memHeapObj, 0, sizeof(gUtils_memHeapObj));

    sprintf(gUtils_memHeapObj[UTILS_HEAPID_DDR_NON_CACHED_SR0].heapName,
                "SR_DDR_NON_CACHED");

    sprintf(gUtils_memHeapObj[UTILS_HEAPID_DDR_CACHED_SR].heapName,
                "SR_DDR_CACHED");

    sprintf(gUtils_memHeapObj[UTILS_HEAPID_OCMC_SR].heapName,
                "SR_OCMC");

    #ifdef BUILD_M4_0
    Utils_memHeapSetup();
    #endif

    heapId = UTILS_HEAPID_DDR_CACHED_LOCAL;

    sprintf(gUtils_memHeapObj[heapId].heapName, "LOCAL_DDR");
    gUtils_memHeapObj[heapId].heapHandle = Memory_defaultHeapInstance;
    gUtils_memHeapObj[heapId].heapAddr   = 0; /* Local heap address cannot be retrived */
    gUtils_memHeapObj[heapId].isClearBufOnAlloc = FALSE;

    Utils_memGetHeapStats(heapId, &memStats);

    gUtils_memHeapObj[heapId].heapSize = memStats.heapSize;

#ifdef ENABLE_HEAP_L2
    heapId = UTILS_HEAPID_L2_LOCAL;

    sprintf(gUtils_memHeapObj[heapId].heapName, "LOCAL_L2");
    gUtils_memHeapObj[heapId].heapHandle = NULL;
    gUtils_memHeapObj[heapId].heapAddr = (UInt32)gUtils_memHeapL2;
    gUtils_memHeapObj[heapId].heapAllocOffset = 0;
    gUtils_memHeapObj[heapId].isClearBufOnAlloc = FALSE;
    gUtils_memHeapObj[heapId].heapSize   = sizeof(gUtils_memHeapL2);
#endif

    Utils_memClearOnAlloc(TRUE);

    if(System_isFastBootEnabled())
        Utils_memClearOnAlloc(FALSE);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief One time system de-init of memory allocator
 *
 *        Should be called by application at system de-init
 *
 * \return SYSTEM_LINK_STATUS_SOK on sucess, else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memDeInit()
{
    memset(gUtils_memHeapObj, 0, sizeof(gUtils_memHeapObj));

    return SYSTEM_LINK_STATUS_SOK;
}


/**
 *******************************************************************************
 *
 * \brief Allocate memory from Frame buffer memory pool
 *
 * \param heapId   [IN] Heap ID
 * \param size     [IN] size in bytes
 * \param align    [IN] alignment in bytes
 *
 * \return NULL or error, else memory pointer
 *
 *******************************************************************************
*/
Ptr Utils_memAlloc(Utils_HeapId heapId, UInt32 size, UInt32 align)
{
    Ptr addr;
    Error_Block ebObj;
    Error_Block *eb = &ebObj;
    IHeap_Handle heapHandle;

    if(heapId>=UTILS_HEAPID_MAXNUMHEAPS)
        return NULL;

    Error_init(eb);

    #ifdef UTILS_MEM_DEBUG
    {
        Utils_MemHeapStats stats;

        Utils_memGetHeapStats(heapId, &stats);

        Vps_printf(" UTILS: MEM: Alloc'ing from heap %s. (required size = %d B,"
               "free space = %d B)\n",
               gUtils_memHeapObj[heapId].heapName,
               size,
               stats.freeSize
              );
    }
    #endif

    if(heapId==UTILS_HEAPID_L2_LOCAL)
    {
        UInt32 offset;
        UInt32 oldIntState;

        oldIntState = Hwi_disable();

        offset = SystemUtils_align(
                    gUtils_memHeapObj[heapId].heapAllocOffset,
                    align);

        if( (offset + size) > gUtils_memHeapObj[heapId].heapSize)
        {
            addr = NULL;
        }
        else
        {
            addr = (Ptr)(gUtils_memHeapObj[heapId].heapAddr + offset);

            gUtils_memHeapObj[heapId].heapAllocOffset
                = offset + size;
        }

        Hwi_restore(oldIntState);
    }
    else
    if(heapId==UTILS_HEAPID_DDR_CACHED_LOCAL)
    {
        heapHandle = gUtils_memHeapObj[heapId].heapHandle;
        /*
         * Heap is present in this CPU, allocate memory
         */
        addr = Memory_alloc(heapHandle,
                           size,
                           align,
                           eb);
    }
    else
    if(heapId==UTILS_HEAPID_DDR_CACHED_SR
        ||
       heapId==UTILS_HEAPID_OCMC_SR
        ||
       heapId==UTILS_HEAPID_DDR_NON_CACHED_SR0
        )
    {
        /*
         * Allocate from heap created on IPU1-0
         */
        SystemCommon_AllocBuffer bufAlloc;
        Int32 status = SYSTEM_LINK_STATUS_SOK;

        bufAlloc.bufferPtr = (UInt32)NULL;
        bufAlloc.heapId = heapId;
        bufAlloc.size = size;
        bufAlloc.align = align;

        #ifdef BUILD_M4_0
        /*
         * Alloc locally
         */
        status = Utils_memAllocSR(&bufAlloc);
        #else
        /*
         * Alloc by sending command to IPU1-0 core
         */
        status = System_linkControl(
                    SYSTEM_LINK_ID_IPU1_0,
                    SYSTEM_COMMON_CMD_ALLOC_BUFFER,
                    &bufAlloc,
                    sizeof(bufAlloc),
                    TRUE
                    );
        #endif

        if(status!=SYSTEM_LINK_STATUS_SOK)
        {
            addr = NULL;
        }
        else
        {
            addr = (Ptr)bufAlloc.bufferPtr;
        }
    }
    else
    {
        UTILS_assert(0);
    }

    #ifdef UTILS_MEM_DEBUG
    Vps_printf(" UTILS: MEM: ALLOC, addr = 0x%08x, size = %d bytes, heap = %s \n",
            addr,
            size,
            gUtils_memHeapObj[heapId].heapName
           );
    #endif

    if (!Error_check(eb)
        &&
        (addr != NULL)
        &&
        gUtils_memHeapObj[heapId].isClearBufOnAlloc
        )
    {
        memset(addr, 0x0, size);
    }

    return addr;
}

/**
 *******************************************************************************
 *
 * \brief Free previously allocate Frame buffer memory pointer
 *
 * \param heapId   [IN] Heap ID;
 * \param addr     [IN] memory pointer to free
 * \param size     [IN] size of memory pointed to by the memory pointer
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memFree(Utils_HeapId heapId, Ptr addr, UInt32 size)
{
    IHeap_Handle heapHandle;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if(heapId>=UTILS_HEAPID_MAXNUMHEAPS)
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

#ifdef UTILS_MEM_DEBUG
    Vps_printf(" UTILS: MEM: FRAME FREE, addr = 0x%08x, size = %d bytes,"
           " heap = %s\n",
           addr,
           size,
           gUtils_memHeapObj[heapId].heapName);
#endif

    if(heapId==UTILS_HEAPID_L2_LOCAL)
    {
        UInt32 oldIntState;

        oldIntState = Hwi_disable();

        gUtils_memHeapObj[heapId].heapAllocOffset = 0;

        Hwi_restore(oldIntState);
    }
    else
    if(heapId==UTILS_HEAPID_DDR_CACHED_LOCAL)
    {
        heapHandle = gUtils_memHeapObj[heapId].heapHandle;
        /*
         * free previously allocated memory
         */
        Memory_free(heapHandle,
                    addr,
                    size);
    }
    else
    if(heapId==UTILS_HEAPID_DDR_CACHED_SR
        ||
       heapId==UTILS_HEAPID_OCMC_SR
        ||
       heapId==UTILS_HEAPID_DDR_NON_CACHED_SR0
        )
    {
        /*
         * Free from heap created on IPU1-0
         */
        SystemCommon_FreeBuffer bufFree;

        bufFree.bufferPtr = (UInt32)addr;
        bufFree.heapId = heapId;
        bufFree.size = size;

        #ifdef BUILD_M4_0
        Utils_memFreeSR(&bufFree);
        #else
        status = System_linkControl(
                    SYSTEM_LINK_ID_IPU1_0,
                    SYSTEM_COMMON_CMD_FREE_BUFFER,
                    &bufFree,
                    sizeof(bufFree),
                    TRUE
                    );
        #endif
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Control if allocated buffer needs to be cleared to 0
 *
 *        By default allocated buffer will not be cleared to 0
 *
 * \param   enable [IN] TRUE: clear allocated buffer, FALSE: do not clear
 *                      allocated buffer
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memClearOnAlloc(Bool enable)
{
    UInt32 i;

    for (i=0; i<UTILS_HEAPID_MAXNUMHEAPS; i++)
    {
        gUtils_memHeapObj[i].isClearBufOnAlloc = enable;
    }
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   Returns the heap information to the user
 *
 * \param   heapId  [IN] Heap ID;
 * \param   pStats  [OUT] Heap information
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 Utils_memGetHeapStats(Utils_HeapId heapId, Utils_MemHeapStats *pStats)
{
    IHeap_Handle heapHandle;
    Memory_Stats stats;
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    memset(pStats, 0, sizeof(*pStats));

    if(heapId>=UTILS_HEAPID_MAXNUMHEAPS)
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

    strcpy(pStats->heapName, gUtils_memHeapObj[heapId].heapName);
    pStats->heapAddr = gUtils_memHeapObj[heapId].heapAddr;

    if(heapId==UTILS_HEAPID_L2_LOCAL)
    {
        UInt32 oldIntState;

        oldIntState = Hwi_disable();

        pStats->heapSize = gUtils_memHeapObj[heapId].heapSize;
        pStats->freeSize =
            gUtils_memHeapObj[heapId].heapSize
            -
            gUtils_memHeapObj[heapId].heapAllocOffset
            ;

        Hwi_restore(oldIntState);
    }
    else
    {
        heapHandle = gUtils_memHeapObj[heapId].heapHandle;

        if(heapHandle==NULL)
        {
            status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
        }
        else
        {
            Memory_getStats(heapHandle,
                            &stats);

            pStats->heapSize = stats.totalSize;
            pStats->freeSize = stats.totalFreeSize;
        }
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Get buffer size based on data format
 *
 * CbCr buffer height is required for semiplanar data formats.
 * For semiplanar data formats CbCr buffer height will be
 * either equal to Y buffer height or half of Y buffer height
 * based on YUV422 or YUv420 data format. This function
 * calculates that internally. But in case cbCrBufferHeight needs to be
 * different than cbCrBufferHeight parameter can be used. For all normal
 * cases where function needs to internally calculate this, caller should
 * pass cbCrBufferHeight = 0. Currently cbCrBufferHeight takes effect for
 * only semiplanar data
 *
 * \param     pFormat          [IN] data format information
 * \param     *size            [IN] buffer size
 * \param     *cOffset         [IN] C plane offset for YUV420SP data
 * \param     cbCrBufferHeight [IN] Height of CbCr plane for YUV420SP data
 *                                  incase
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memFrameGetSize(FVID2_Format * pFormat,
                            UInt32 * size,
                            UInt32 * cOffset,
                            UInt32 cbCrBufferHeight)
{
    UInt32 bufferHeight;
    Int32 status = 0;

    bufferHeight = pFormat->height;

    switch (pFormat->dataFormat)
    {
        case FVID2_DF_YUV422I_YUYV:
        case FVID2_DF_YUV422I_YVYU:
        case FVID2_DF_YUV422I_UYVY:
        case FVID2_DF_YUV422I_VYUY:
        case FVID2_DF_YUV444I:
        case FVID2_DF_RGB24_888:
        case FVID2_DF_BGR24_888:
        case FVID2_DF_BGR16_565:
        case FVID2_DF_RAW_VBI:
        case FVID2_DF_ARGB32_8888:
        case FVID2_DF_RGBA32_8888:
        case FVID2_DF_ABGR32_8888:
        case FVID2_DF_BGRA32_8888:
        case FVID2_DF_BGRA16_4444:
        case FVID2_DF_BAYER_RAW:
        case FVID2_DF_BAYER_GRBG:
        case FVID2_DF_BAYER_RGGB:
        case FVID2_DF_BAYER_BGGR:
        case FVID2_DF_BAYER_GBRG:
        case FVID2_DF_RAW16:
            /*
             * for single plane data format's
             */
            *size = pFormat->pitch[0] * bufferHeight;
            break;

        case FVID2_DF_YUV422SP_UV:
        case FVID2_DF_YUV420SP_UV:

            /*
             * for Y plane
             */
            *size = pFormat->pitch[0] * bufferHeight;

            /*
             * cOffset is at end of Y plane
             */
            *cOffset = *size;

            if (pFormat->dataFormat == FVID2_DF_YUV420SP_UV)
            {
                if (0 == cbCrBufferHeight)
                {
                    bufferHeight = bufferHeight / 2;
                }
                else
                {
                    bufferHeight = cbCrBufferHeight;
                }
            }

            *size += pFormat->pitch[1] * bufferHeight;
            break;

        default:
            status = SYSTEM_LINK_STATUS_EFAIL;
            break;
    }

    /*
     * align size to minimum required frame buffer alignment
     */
    *size = VpsUtils_align(*size, VPS_BUFFER_ALIGNMENT);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Allocate a frame
 *
 * Use FVID2_Format to allocate a frame.
 * Fill FVID2_Frame fields like channelNum based on FVID2_Format
 *
 * CbCr buffer height is required for semiplanar data formats.
 * For semiplanar data formats CbCr buffer height will be
 * either equal to Y buffer height or half of Y buffer height
 * based on YUV422 or YUv420 data format. This function
 * calculates that internally. But in case cbCrBufferHeight needs to be
 * different than cbCrBufferHeight parameter can be used. For all normal
 * cases where function needs to internally calculate this, caller should
 * pass cbCrBufferHeight = 0. Currently cbCrBufferHeight takes effect for
 * only semiplanar data
 *
 * \param      pFormat          [IN]  Data format information
 * \param      pFrame           [OUT] Initialzed FVID2_Frame structure
 * \param      numFrames        [IN]  Number of frames to allocate
 * \param      cbCrBufferHeight [IN]  Height of CbCr plane for YUV420SP data
 *                                     incase
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memFrameAlloc(FVID2_Format * pFormat,
                          FVID2_Frame * pFrame,
                          UInt16 numFrames,
                          UInt32 cbCrBufferHeight)
{
    UInt32 size, cOffset, frameId;
    Int32 status;
    UInt8 *pBaseAddr;

    memset(pFrame, 0, sizeof(*pFrame)*numFrames);

    pFormat->height = VpsUtils_align(pFormat->height, 2);

    status = Utils_memFrameGetSize(pFormat, &size, &cOffset, cbCrBufferHeight);

    if (status == SYSTEM_LINK_STATUS_SOK)
    {
        /*
         * allocate the memory for 'numFrames'
         */

        /*
         * for all 'numFrames' memory is contigously allocated
         */
        pBaseAddr = Utils_memAlloc(
                            UTILS_HEAPID_DDR_CACHED_SR,
                            size * numFrames,
                            VPS_BUFFER_ALIGNMENT
                        );
        if (pBaseAddr == NULL)
        {
            status = SYSTEM_LINK_STATUS_EFAIL;
        }
    }

    if (status == SYSTEM_LINK_STATUS_SOK)
    {
        /*
         * init memory pointer for 'numFrames'
         */
        for (frameId = 0; frameId < numFrames; frameId++)
        {

            /*
             * copy channelNum to FVID2_Frame from FVID2_Format
             */
            pFrame->chNum = pFormat->chNum;
            pFrame->addr[0][0] = pBaseAddr;

            switch (pFormat->dataFormat)
            {
                case FVID2_DF_RAW_VBI:
                case FVID2_DF_YUV422I_UYVY:
                case FVID2_DF_YUV422I_VYUY:
                case FVID2_DF_YUV422I_YUYV:
                case FVID2_DF_YUV422I_YVYU:
                case FVID2_DF_YUV444I:
                case FVID2_DF_RGB24_888:
                case FVID2_DF_BGR24_888:
                case FVID2_DF_RGB16_565:
                case FVID2_DF_BGR16_565:
                case FVID2_DF_ARGB32_8888:
                case FVID2_DF_RGBA32_8888:
                case FVID2_DF_ABGR32_8888:
                case FVID2_DF_BGRA32_8888:
                case FVID2_DF_BGRA16_4444:
                case FVID2_DF_BAYER_RAW:
                case FVID2_DF_BAYER_GRBG:
                case FVID2_DF_BAYER_RGGB:
                case FVID2_DF_BAYER_BGGR:
                case FVID2_DF_BAYER_GBRG:
                case FVID2_DF_RAW16:
                    break;
                case FVID2_DF_YUV422SP_UV:
                case FVID2_DF_YUV420SP_UV:
                    /* assign pointer for C plane */
                    pFrame->addr[0][1] = (UInt8 *) pFrame->addr[0][0] + cOffset;
                    break;
                default:
                    /* illegal data format */
                    status = SYSTEM_LINK_STATUS_EFAIL;
                    break;
            }
            /*
             * go to next frame
             */
            pFrame++;

            /*
             * increment base address
             */
            pBaseAddr += size;
        }
    }

    if (status != 0)
    {
        void System_memPrintHeapStatus();

        Vps_printf(" UTILS: MEM: Memory allocation failed due to "
                   " insufficient free memory, "
                   "requested - %d \n",
                   size * numFrames);
        System_memPrintHeapStatus();

    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Free's previously allocate FVID2_Frame's
 *
 * CbCr buffer height is required for semiplanar data formats.
 * For semiplanar data formats CbCr buffer height will be
 * either equal to Y buffer height or half of Y buffer height
 * based on YUV422 or YUv420 data format. This function
 * calculates that internally. But in case cbCrBufferHeight needs to be
 * different than cbCrBufferHeight parameter can be used. For all normal
 * cases where function needs to internally calculate this, caller should
 * pass cbCrBufferHeight = 0. Currently cbCrBufferHeight takes effect for
 * only semiplanar data
 *
 * \param      pFormat          [IN]  Data format information
 * \param      pFrame           [OUT] Initialzed FVID2_Frame structure
 * \param      numFrames        [IN]  Number of frames to free
 * \param      cbCrBufferHeight [IN]  Height of CbCr plane for YUV420SP data
 *                                     incase
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success else SYSTEM_LINK_STATUS_EFAIL
 *
 *******************************************************************************
*/
Int32 Utils_memFrameFree(FVID2_Format * pFormat,
                         FVID2_Frame * pFrame,
                         UInt16 numFrames,
                         UInt32 cbCrBufferHeight)
{
    UInt32 size, cOffset;
    Int32 status;

    /*
     * get frame size for given 'pFormat'
     */
    status = Utils_memFrameGetSize(pFormat, &size, &cOffset, cbCrBufferHeight);

    if (status == SYSTEM_LINK_STATUS_SOK)
    {
        /*
         * free the frame buffer memory
         */

        /*
         * for all 'numFrames' memory is allocated contigously during alloc,
         * so first frame memory pointer points to the complete memory block
         * for all frames
         */
        Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                      pFrame->addr[0][0], size * numFrames);
    }

    return SYSTEM_LINK_STATUS_SOK;
}
