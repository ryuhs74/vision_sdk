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
#include "featurePlaneCompLink_priv.h"

#define FILTER_COEF_WIDTH   3
#define FILTER_COEF_HEIGHT  3

static uint8_t filterCoeff[FILTER_COEF_WIDTH * FILTER_COEF_HEIGHT]=
{
  1, 2, 1,
  1, 2, 1,
  0, 0, 0,
};


Int32 Alg_filter2dCreate(
            Alg_Filter2dObj *pObj,
            UInt32 inWidth,
            UInt32 inHeight,
            UInt32 inPitch[],
            UInt32 padX,
            UInt32 padY
)
{
    memset(pObj, 0, sizeof(*pObj));

    pObj->padX = padX;
    pObj->padY = padY;

    pObj->prms.visionParams.algParams.size =
        sizeof(pObj->prms);

    pObj->prms.visionParams.cacheWriteBack = NULL;
    pObj->prms.filterCoefWidth = FILTER_COEF_WIDTH;
    pObj->prms.filterCoefHeight = FILTER_COEF_HEIGHT;
    pObj->prms.filterCoef = filterCoeff;
    pObj->prms.imageFormat = FILTER_2D_TI_IMAGE_FORMAT_YUV420;
    pObj->prms.separableFilter = 1;
    pObj->prms.enableContrastStretching = TRUE;
    pObj->prms.enableFilter = TRUE;
    pObj->prms.minVal = 0;
    pObj->prms.maxVal = 255;
    pObj->prms.minPercentileThreshold = 1;
    pObj->prms.maxPercentileThreshold = 99;

    pObj->handle = AlgIvision_create(
                &FILTER_2D_TI_VISION_FXNS,
                (IALG_Params*)&pObj->prms
                );
    UTILS_assert(pObj->handle!=NULL);

    pObj->inArgs.iVisionInArgs.size =
        sizeof(pObj->inArgs);

    pObj->inArgs.minVal = 0;
    pObj->inArgs.maxVal = 255;

    pObj->outArgs.iVisionOutArgs.size =
        sizeof(pObj->outArgs);

    pObj->inBufs.size = sizeof(pObj->inBufs);
    pObj->inBufs.numBufs = FILTER_2D_TI_BUFDESC_IN_TOTAL;
    pObj->inBufs.bufDesc  = pObj->inBufDescList;

    pObj->outBufs.size = sizeof(pObj->outBufs);
    pObj->outBufs.numBufs = FILTER_2D_TI_BUFDESC_OUT_TOTAL;
    pObj->outBufs.bufDesc = pObj->outBufDescList;

    pObj->inBufDescList[FILTER_2D_TI_BUFDESC_IN_IMAGEBUFFER] = &pObj->inBufDesc;
    pObj->outBufDescList[FILTER_2D_TI_BUFDESC_OUT_IMAGE_BUFFER] = &pObj->outBufDesc;

    pObj->inBufDesc.numPlanes                          = 2;
    pObj->inBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    pObj->inBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    pObj->inBufDesc.bufPlanes[0].width                 = inPitch[0];
    pObj->inBufDesc.bufPlanes[0].height                = inHeight;
    pObj->inBufDesc.bufPlanes[0].frameROI.width        = inWidth;
    pObj->inBufDesc.bufPlanes[0].frameROI.height       = inHeight;
    pObj->inBufDesc.bufPlanes[0].planeType             = 0; //Luma Y

    pObj->inBufDesc.bufPlanes[1].frameROI.topLeft.x    = 0;
    pObj->inBufDesc.bufPlanes[1].frameROI.topLeft.y    = 0;
    pObj->inBufDesc.bufPlanes[1].width                 = inPitch[1];
    pObj->inBufDesc.bufPlanes[1].height                = inHeight/2;
    pObj->inBufDesc.bufPlanes[1].frameROI.width        = inWidth;
    pObj->inBufDesc.bufPlanes[1].frameROI.height       = inHeight/2;
    pObj->inBufDesc.bufPlanes[1].planeType             = 1; //C

    pObj->outWidth  = inWidth;
    pObj->outHeight = inHeight;

    pObj->outPitch[0] = SystemUtils_align(inWidth+padX*2, 128);
    pObj->outPitch[1] = pObj->outPitch[0];
    pObj->outBufSize[0] = pObj->outPitch[0]*(inHeight+padY*2);
    pObj->outBufSize[1] = pObj->outPitch[1]*(inHeight+padY*2)/2;

    pObj->outBufAddr[0] =
        Utils_memAlloc(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->outBufSize[0],
            32
        );
    UTILS_assert(pObj->outBufAddr[0]!=NULL);
    pObj->outBufAddr[1] =
        Utils_memAlloc(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->outBufSize[1],
            32
        );
    UTILS_assert(pObj->outBufAddr[1]!=NULL);

    pObj->outBufDesc.numPlanes                          = 2;
    pObj->outBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    pObj->outBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    pObj->outBufDesc.bufPlanes[0].width                 = pObj->outPitch[0];
    pObj->outBufDesc.bufPlanes[0].height                = inHeight;
    pObj->outBufDesc.bufPlanes[0].frameROI.width        = inWidth;
    pObj->outBufDesc.bufPlanes[0].frameROI.height       = inHeight;
    pObj->outBufDesc.bufPlanes[0].planeType             = 0; //Luma Y
    pObj->outBufDesc.bufPlanes[0].buf                   =
        (UInt8*)pObj->outBufAddr[0] + pObj->outPitch[0]*padY + padX;

    pObj->outBufDesc.bufPlanes[1].frameROI.topLeft.x    = 0;
    pObj->outBufDesc.bufPlanes[1].frameROI.topLeft.y    = 0;
    pObj->outBufDesc.bufPlanes[1].width                 = pObj->outPitch[1];
    pObj->outBufDesc.bufPlanes[1].height                = inHeight/2;
    pObj->outBufDesc.bufPlanes[1].frameROI.width        = inWidth;
    pObj->outBufDesc.bufPlanes[1].frameROI.height       = inHeight/2;
    pObj->outBufDesc.bufPlanes[1].planeType             = 1; //Chroma UV
    pObj->outBufDesc.bufPlanes[1].buf                   =
        (UInt8*)pObj->outBufAddr[1] + pObj->outPitch[1]*padY/2 + padX;

    return 0;
}

Int32 Alg_filter2dProcess(
            Alg_Filter2dObj *pObj,
            Void *inBufAddr[]
        )
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    pObj->inBufDesc.bufPlanes[0].buf = inBufAddr[0];
    pObj->inBufDesc.bufPlanes[1].buf = inBufAddr[1];

    status = AlgIvision_process(
                pObj->handle,
                &pObj->inBufs,
                &pObj->outBufs,
                (IVISION_InArgs*)&pObj->inArgs,
                (IVISION_OutArgs*)&pObj->outArgs
            );
    UTILS_assert(status==0);

    pObj->inArgs.minVal = pObj->outArgs.minVal;
    pObj->inArgs.maxVal = pObj->outArgs.maxVal;

    return status;
}

Int32 Alg_filter2dDelete(Alg_Filter2dObj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    status = AlgIvision_delete(pObj->handle);
    UTILS_assert(status==0);

    status = Utils_memFree(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->outBufAddr[0],
            pObj->outBufSize[0]
        );
    UTILS_assert(status==0);

    status = Utils_memFree(
            UTILS_HEAPID_DDR_CACHED_SR,
            pObj->outBufAddr[1],
            pObj->outBufSize[1]
        );
    UTILS_assert(status==0);

    return status;
}
