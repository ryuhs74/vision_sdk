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
 * \file issAewbIpncLink_priv.h ISS AEWB Link private API/Data
 *       structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Apr 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ISS_AEWBIPNCIPNC_LINK_PRIV_H_
#define _ISS_AEWBIPNCIPNC_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/issM2mIspLink.h>
#include <include/link_api/algorithmLink_issAewb.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>


#include <idcc.h>
#include <src/utils_common/include/utils_link_stats_if.h>

/*******************************************************************************
 *  Enums
 *******************************************************************************
 */


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */



/**
 *******************************************************************************
 *   \brief Max number of buffers
 *******************************************************************************
 */
#define ISS_AEWB1_LINK_MAX_NUM_OUTPUT    (3)


/**
 *******************************************************************************
 *   \brief Max number of input channels
 *******************************************************************************
 */
#define ISS_AEWB1_LINK_MAX_CH            (4)


/**
 *******************************************************************************
 *   \brief Max size of AEWB results
 *******************************************************************************
 */
#define ISS_AEWB1_LINK_MAX_BUF_SIZE      (SystemUtils_align(sizeof(IssAewbAlgOutParams), 128))



/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

typedef struct {
    UInt16 window_data[8][8];
    UInt16 unsat_block_ct[8];
} AlgorithmLink_IssAwebAlgData;

typedef struct
{
    dcc_parser_input_params_t   dccInPrms;
    dcc_parser_output_params_t  dccOutPrms;
    IssIspConfigurationParameters ispCfgPrms;
    IssM2mSimcopLink_ConfigParams simcopCfgPrms;

    Int32 prevRgb2Rgb1Idx;
    Int32 prevRgb2Rgb2Idx;
    Int32 prevCnfIdx;
    Int32 prevNsf3vIdx;
    Int32 prev3dLutIdx;

    Ptr dccInBuf;
    Ptr dccOutBuf;
    UInt32 dccOutBufSize;

    UInt32 width, height;
} Dcc_Object;



/**
 *******************************************************************************
 *
 *   \brief Structure containing algorithm link
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_IssAewbCreateParams algLinkCreateParams;

    System_LinkQueInfo inQueInfo;

    System_Buffer buffers[ISS_AEWB1_LINK_MAX_CH*ISS_AEWB1_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */

    System_MetaDataBuffer metaDataBuffers[ISS_AEWB1_LINK_MAX_CH*ISS_AEWB1_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */

    AlgorithmLink_InputQueueInfo  inputQInfo;
    /**< All the information about input Queue*/

    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Queue */

    System_LinkStatistics   *linkStatsInfo;
    /**< Pointer to the Link statistics information,
         used to store below information
            1, min, max and average latency of the link
            2, min, max and average latency from source to this link
            3, links statistics like frames captured, dropped etc
        Pointer is assigned at the link create time from shared
        memory maintained by utils_link_stats layer */


    Bool isFirstFrameRecv;
    /**< Flag to indicate if first frame is received, this is used as trigger
     *   to start stats counting
     */
    Void *algHndl;
    /**< Handle to the Algorithm */

    Dcc_Object dccObj;

    /* Keep track of last AE/AWB outputs so that they can be used in DCC */
    AlgorithmLink_IssAewb2AParams aewbOut;
    BspOsal_SemHandle             lock;
    /**< Semaphore to protect #aewbOut parameters */

} AlgorithmLink_IssAewbObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_issAewb1Create(void * pObj,
                                              void * pCreateParams);
Int32 AlgorithmLink_issAewb1Process(void * pObj);
Int32 AlgorithmLink_issAewb1Control(void * pObj, void * pControlParams);
Int32 AlgorithmLink_issAewb1Stop(void * pObj);
Int32 AlgorithmLink_issAewb1Delete(void * pObj);
Int32 AlgorithmLink_issAewb1PrintStatistics(void *pObj,
                AlgorithmLink_IssAewbObj *pIssAewbObj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
