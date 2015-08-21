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
 * \defgroup UTILS_API Utility functions
 *
 * \brief This module define APIs for commonly used utility functions
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file utils.h
 *
 * \brief Utility functions
 *
 * \version 0.0 (July 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _UTILS_H_
#define _UTILS_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include <include/link_api/system.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Text.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/utils/Load.h>
#include <ti/sysbios/hal/Cache.h>
#include <ti/sysbios/knl/Clock.h>
#include <vps/vps.h>
#include <osal/bsp_osal.h>

#include <src/utils_common/include/utils_prf.h>
#include <src/utils_common/include/utils_remote_log_if.h>

#include <include/link_api/system_inter_link_api.h>
#include <include/link_api/systemLink_common.h>



/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Get number of elements in a array
 *
 * \param array [IN] array  data type
 *
 * \return number of elements in the array
 *
 *******************************************************************************
 */
#define UTILS_ARRAYSIZE(array)             ((sizeof(array)/sizeof((array)[0])))

/**
 *******************************************************************************
 *
 * \brief Get index of a element in a array
 *
 * \param elem     [IN] pointer to a element in the array
 * \param array    [IN] array data type
 *
 * \return index of the element in the array
 *
 *******************************************************************************
 */
#define UTILS_ARRAYINDEX(elem,array)                   ((elem) - &((array)[0]))

/**
 *******************************************************************************
 *
 * \brief Check if element is a valid entry of the array
 *
 * \param elem     [IN] pointer to a element in the array
 * \param array    [IN] array data type
 *
 * \return TRUE: element lies inside the array \n
 *         FALSE: element lies outside the array
 *
 *******************************************************************************
 */
#define UTILS_ARRAYISVALIDENTRY(elem,array) ((UTILS_ARRAYINDEX(elem,array) <   \
                                             UTILS_ARRAYSIZE(array))           \
                                             ? TRUE                            \
                                             : FALSE)

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */


/**
 *******************************************************************************
 *  \brief Clock ID to be used with Utils_getClkHz()
 *******************************************************************************
 */
typedef enum {

    UTILS_CLK_ID_EVE,
    /**< EVE CPU ID */

    UTILS_CLK_ID_DSP,
    /**< DSP CPU ID */

    UTILS_CLK_ID_IPU,
    /**< IPU CPU ID */

    UTILS_CLK_ID_A15,
    /**< A15 CPU ID */

    UTILS_CLK_ID_MAX
    /**< Max clock ID */

} Utils_ClkId;

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Context information related to skipping of buffer's
 *
 *******************************************************************************
 */
typedef struct {
    Int32 inputFrameRate;
    /**< Incoming buffer frame-rate */

    Int32 outputFrameRate;
    /**< Outgoing buffer frame-rate */

    Int32 firstTime;
    /**< Flag to indicate if buffer skiplogic is being called for first frame */

    Int32 inCnt;
    /**< Current rate of incoming buffers */

    Int32 outCnt;
    /**< Current rate of outgoing buffers */

    Int32 multipleCnt;
    /**< inputFrameRate x outputFrameRate */

} Utils_BufSkipContext;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */


int    xstrtoi(char *hex);

Int32  Utils_setCpuFrequency (UInt32 freq);

UInt32 Utils_getCurTimeInMsec();

UInt64 Utils_getCurTimeInUsec();

Void   Utils_resetSkipBufContext(Utils_BufSkipContext *skipCtx,
                    UInt32 inputFrameRate, UInt32 outputFrameRate);

Bool   Utils_doSkipBuf(Utils_BufSkipContext *skipCtx );

Int32 Utils_getAppInitState(int coreId, unsigned int *pState);

Int32 Utils_setAppInitState(int coreId, unsigned int state);

Int32 Utils_globalTimerInit();

UInt32 Utils_getClkHz(Utils_ClkId clkId);

Int32 Utils_netGetIpAddrStr(char *ipAddrStr);

Bool Utils_netIsAvbEnabled();

void  Utils_ndkGetIpAddrStr(char *ipAddrStr);

UInt32 Utils_netGetProcId();

/**
 *******************************************************************************
 *
 * \brief Get number of planes corresponding to a given data format
 *
 * \param dataFormat [IN] data format
 *
 * \return Number of planes, 0 is returned in case dataFormat is unknown
 *
 *******************************************************************************
 */
static inline UInt32 Utils_getNumPlanes(FVID2_DataFormat dataFormat)
{
    UInt32 numPlanes = 0;

    switch(dataFormat)
    {
        case FVID2_DF_YUV422I_UYVY:
        case FVID2_DF_YUV422I_YUYV:
        case FVID2_DF_YUV422I_YVYU:
        case FVID2_DF_YUV422I_VYUY:
        case FVID2_DF_YUV444I:
        case FVID2_DF_RGB24_888:
        case FVID2_DF_BGR24_888:
        case FVID2_DF_BGR16_565:
        case FVID2_DF_ARGB32_8888:
        case FVID2_DF_RGBA32_8888:
        case FVID2_DF_ABGR32_8888:
        case FVID2_DF_BGRA32_8888:
        case FVID2_DF_BGRA16_4444:
            numPlanes = 1;
            break;
        case FVID2_DF_YUV422SP_UV:
        case FVID2_DF_YUV422SP_VU:
        case FVID2_DF_YUV420SP_UV:
        case FVID2_DF_YUV420SP_VU:
            numPlanes = 2;
            break;
        case FVID2_DF_YUV422P:
        case FVID2_DF_YUV420P:
        case FVID2_DF_YUV444P:
            numPlanes = 3;
            break;
        default:
            Vps_printf(" UTILS: WARNING: Unknown data format [%d]."
                       " Setting numPlanes to zero!!!\n",
                       dataFormat);
            numPlanes = 0;
            break;
    }
    return numPlanes;
}

#endif /* ifndef _UTILS_H_ */

/* @} */
