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
 * \file featurePlaneCompLink_priv.h Feature Plane Computation Algorithm Link
 *       private API/Data structures
 *
 * \brief  This link private header file has defined
 *         - Algorithm link instance/handle object
 *         - All the local data structures
 *         - Algorithm plug in function interfaces
 *
 * \version 0.0 (Feb 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _FEATUREPLANECOMPUTATION_LINK_PRIV_H_
#define _FEATUREPLANECOMPUTATION_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/algorithmLink_featurePlaneComputation.h>
#include <include/link_api/algorithmLink_algPluginSupport.h>
#include <src/utils_common/include/utils_prf.h>
#include <examples/tda2xx/src/alg_plugins/common/include/alg_ivision.h>
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>
#include <iti_pd_feature_plane_computation_ti.h>
#include <iyuv_scalar_ti.h>
#include <ifilter_2d_ti.h>
#include <iyuv_padding_ti.h>
#include <src/utils_common/include/utils_link_stats_if.h>


/*******************************************************************************
 *  Enums
 *******************************************************************************
 */

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */
#define ALG_IMAGE_PYRAMID_MAX_SCALES            (32)
#define ALG_IMAGE_PYRAMID_MAX_SCALE_STEPS       (8)

#define FEATUREPLANECOMP_LINK_MAX_NUM_OUTPUT    (8)
#define FEATUREPLANECOMP_PAD_X                  (64)
#define FEATUREPLANECOMP_PAD_Y                  (64)


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

typedef struct
{
    Void * handle;

    FILTER_2D_CreateParams prms;
    FILTER_2D_InArgs inArgs;
    FILTER_2D_OutArgs outArgs;

    IVISION_InBufs    inBufs;
    IVISION_OutBufs   outBufs;
    IVISION_BufDesc   inBufDesc;
    IVISION_BufDesc   outBufDesc;
    IVISION_BufDesc   *inBufDescList[FILTER_2D_TI_BUFDESC_IN_TOTAL];
    IVISION_BufDesc   *outBufDescList[FILTER_2D_TI_BUFDESC_OUT_TOTAL];

    Void  *outBufAddr[2];
    UInt32 outBufSize[2];
    UInt32 outPitch[2];
    UInt32 outWidth;
    UInt32 outHeight;
    UInt16 padX;
    UInt16 padY;

} Alg_Filter2dObj;

typedef struct
{
    Void * handle;

    YUV_PADDING_TI_CreateParams prms;
    IVISION_InArgs inArgs;
    YUV_PADDING_TI_outArgs outArgs;

    IVISION_InBufs    inBufs;
    IVISION_OutBufs   outBufs;
    IVISION_BufDesc   inBufDesc;
    IVISION_BufDesc   outBufDesc;
    IVISION_BufDesc   *inBufDescList[1];
    IVISION_BufDesc   *outBufDescList[1];

    Void  *outBufAddr[2];
    UInt32 outPitch[2];
    UInt32 outWidth;
    UInt32 outHeight;

    Bool   enable;

} Alg_YuvPaddingObj;

typedef struct
{
    Void * handle[ALG_IMAGE_PYRAMID_MAX_SCALES];

    UInt16 numScales;
    UInt16 scaleSteps;
    UInt32 sreachStep;

    scalePrams_t    scalePrms[ALG_IMAGE_PYRAMID_MAX_SCALES];

    YUV_SCALAR_TI_CreateParams prms[ALG_IMAGE_PYRAMID_MAX_SCALES];
    YUV_SCALAR_TI_InArgs inArgs[ALG_IMAGE_PYRAMID_MAX_SCALES];
    YUV_SCALAR_TI_outArgs outArgs[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_InBufs    inBufs[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_OutBufs   outBufs[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_BufDesc   inBufDesc[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_BufDesc   outBufDesc[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_BufDesc   *inBufDescList[ALG_IMAGE_PYRAMID_MAX_SCALES][1];
    IVISION_BufDesc   *outBufDescList[ALG_IMAGE_PYRAMID_MAX_SCALES][1];

    Void *outBufAddr[ALG_IMAGE_PYRAMID_MAX_SCALES][2];
    UInt32 outBufSize[ALG_IMAGE_PYRAMID_MAX_SCALES][2];
    UInt32 outPitch[ALG_IMAGE_PYRAMID_MAX_SCALES][2];

} Alg_ImagePyramidObj;

typedef struct
{
    Void * handle;

    PD_FEATURE_PLANE_COMPUTATION_CreateParams prms;
    PD_FEATURE_PLANE_COMPUTATION_InArgs inArgs;
    PD_FEATURE_PLANE_COMPUTATION_OutArgs outArgs;

    IVISION_InBufs    inBufs;
    IVISION_OutBufs   outBufs;
    IVISION_BufDesc   inBufDesc[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_BufDesc   outBufDesc;
    IVISION_BufDesc   *inBufDescList[ALG_IMAGE_PYRAMID_MAX_SCALES];
    IVISION_BufDesc   *outBufDescList[PD_FEATURE_PLANE_COMPUTATION_BUFDESC_OUT_TOTAL];

} Alg_FeatureComputeObj;


/**
 *******************************************************************************
 *
 *   \brief Structure containing feature plane computation algorithm link
 *          parameters
 *
 *          This structure holds any algorithm parameters specific to this link.
 *
 *******************************************************************************
*/
typedef struct
{
    Alg_Filter2dObj       algFilter2dObj;
    Alg_FeatureComputeObj algFeatureComputeObj;
    Alg_ImagePyramidObj   algImagePyramidObj;
    Alg_YuvPaddingObj     algYuvPaddingObj;

    AlgorithmLink_FeaturePlaneComputationCreateParams algLinkCreateParams;
    /**< Create params of feature plane computation algorithm link */

    System_Buffer buffers[FEATUREPLANECOMP_LINK_MAX_NUM_OUTPUT];
    /**< System buffers to exchange data with next link */

    System_MetaDataBuffer featurePlanes[FEATUREPLANECOMP_LINK_MAX_NUM_OUTPUT];
    /**< Payload for the system buffers */

    UInt32  outBufferSize;
    /**< Size of each output buffer */

    AlgorithmLink_InputQueueInfo  inputQInfo;
    /**< All the information about input Queue*/

    AlgorithmLink_OutputQueueInfo outputQInfo;
    /**< All the information about output Queue*/

    System_LinkChInfo inChInfo;
    /**< Information about input channel */

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

} AlgorithmLink_FeaturePlaneComputeObj;

/*******************************************************************************
 *  Algorithm Link Private Functions
 *******************************************************************************
 */
Int32 AlgorithmLink_featurePlaneComputeCreate(void * pObj,
                                              void * pCreateParams);
Int32 AlgorithmLink_featurePlaneComputeProcess(void * pObj);
Int32 AlgorithmLink_featurePlaneComputeControl(void * pObj,
                                               void * pControlParams);
Int32 AlgorithmLink_featurePlaneComputeStop(void * pObj);
Int32 AlgorithmLink_featurePlaneComputeDelete(void * pObj);
Int32 AlgorithmLink_featurePlaneComputePrintStatistics(void *pObj,
                AlgorithmLink_FeaturePlaneComputeObj *pFeaturePlaneComputeObj);

Int32 Alg_filter2dCreate(
            Alg_Filter2dObj *pObj,
            UInt32 inWidth,
            UInt32 inHeight,
            UInt32 inPitch[],
            UInt32 padX,
            UInt32 padY
);

Int32 Alg_filter2dProcess(
            Alg_Filter2dObj *pObj,
            Void *inBufAddr[]
        );

Int32 Alg_filter2dDelete(Alg_Filter2dObj *pObj);

Int32 Alg_yuvPaddingCreate(
            Alg_YuvPaddingObj *pObj,
            Alg_Filter2dObj *pFilter2dObj,
            Bool enable
);

Int32 Alg_yuvPaddingProcess(
            Alg_YuvPaddingObj *pObj
        );

Int32 Alg_yuvPaddingDelete(Alg_YuvPaddingObj *pObj);

Int32 Alg_imagePyramidCreate(
            Alg_ImagePyramidObj *pObj,
            Alg_YuvPaddingObj *pYuvPaddingObj,
            UInt8 numScales,
            UInt8 scaleSteps,
            UInt16 roiCenterX,
            UInt16 roiCenterY,
            UInt16 roiWidth,
            UInt16 roiHeight,
            UInt16 padX,
            UInt16 padY
);

Int32 Alg_imagePyramidProcess(
            Alg_ImagePyramidObj *pObj
        );

Int32 Alg_imagePyramidDelete(Alg_ImagePyramidObj *pObj);

Int32 Alg_featureComputeCreate(
            Alg_FeatureComputeObj *pObj,
            Alg_ImagePyramidObj *pImgPyramidObj,
            UInt32 *outBufSize,
            UInt32 padX,
            UInt32 padY
      );

Int32 Alg_featureComputeProcess(
            Alg_FeatureComputeObj *pObj,
            Void *outBufAddr
      );

Int32 Alg_featureComputeDelete(
            Alg_FeatureComputeObj *pObj
      );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*@}*/

/* Nothing beyond this point */
