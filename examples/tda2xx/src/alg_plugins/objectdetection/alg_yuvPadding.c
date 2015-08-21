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


Int32 Alg_yuvPaddingCreate(
            Alg_YuvPaddingObj *pObj,
            Alg_Filter2dObj *pFilter2dObj,
            Bool enable
    )
{
    memset(pObj, 0, sizeof(*pObj));

    pObj->prms.visionParams.algParams.size =
        sizeof(pObj->prms);

    pObj->prms.visionParams.cacheWriteBack = NULL;
    pObj->prms.maxImageWidth = pFilter2dObj->outPitch[0];
    pObj->prms.topPadding    = pFilter2dObj->padY;
    pObj->prms.leftPadding   = pFilter2dObj->padX;
    pObj->prms.rightPadding  = pFilter2dObj->padX;
    pObj->prms.BottomPadding = pFilter2dObj->padY;

    pObj->enable = enable;

    if(enable)
    {
        pObj->handle = AlgIvision_create(
                    &YUV_PADDING_TI_VISION_FXNS,
                    (IALG_Params*)&pObj->prms
                    );
        UTILS_assert(pObj->handle!=NULL);
    }

    pObj->inArgs.size =
        sizeof(pObj->inArgs);

    pObj->outArgs.iVisionOutArgs.size =
        sizeof(pObj->outArgs);

    pObj->inBufs.size = sizeof(pObj->inBufs);
    pObj->inBufs.numBufs = 1;
    pObj->inBufs.bufDesc  = pObj->inBufDescList;

    pObj->outBufs.size = sizeof(pObj->outBufs);
    pObj->outBufs.numBufs = 1;
    pObj->outBufs.bufDesc = pObj->outBufDescList;

    pObj->inBufDescList[0] = &pObj->inBufDesc;
    pObj->outBufDescList[0] = &pObj->outBufDesc;

    pObj->outPitch[0] = pFilter2dObj->outPitch[0];
    pObj->outPitch[1] = pFilter2dObj->outPitch[1];
    pObj->outWidth    = pFilter2dObj->outWidth + pFilter2dObj->padX*2;
    pObj->outHeight   = pFilter2dObj->outHeight + pFilter2dObj->padY*2;
    pObj->outBufAddr[0] = pFilter2dObj->outBufAddr[0];
    pObj->outBufAddr[1] = pFilter2dObj->outBufAddr[1];

    pObj->inBufDesc.numPlanes                          = 2;
    pObj->inBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    pObj->inBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    pObj->inBufDesc.bufPlanes[0].width                 = pFilter2dObj->outPitch[0];
    pObj->inBufDesc.bufPlanes[0].height                = pFilter2dObj->outHeight;
    pObj->inBufDesc.bufPlanes[0].frameROI.width        = pFilter2dObj->outWidth;
    pObj->inBufDesc.bufPlanes[0].frameROI.height       = pFilter2dObj->outHeight;
    pObj->inBufDesc.bufPlanes[0].planeType             = 0; //Luma Y
    pObj->inBufDesc.bufPlanes[0].buf
            = (UInt8*)pObj->outBufAddr[0]
                +
              pFilter2dObj->padY*pObj->outPitch[0]
                +
              pFilter2dObj->padX
            ;

    pObj->inBufDesc.bufPlanes[1].frameROI.topLeft.x    = 0;
    pObj->inBufDesc.bufPlanes[1].frameROI.topLeft.y    = 0;
    pObj->inBufDesc.bufPlanes[1].width                 = pFilter2dObj->outPitch[1];
    pObj->inBufDesc.bufPlanes[1].height                = pFilter2dObj->outHeight/2;
    pObj->inBufDesc.bufPlanes[1].frameROI.width        = pFilter2dObj->outWidth;
    pObj->inBufDesc.bufPlanes[1].frameROI.height       = pFilter2dObj->outHeight/2;
    pObj->inBufDesc.bufPlanes[1].planeType             = 1; //C
    pObj->inBufDesc.bufPlanes[1].buf
            = (UInt8*)pObj->outBufAddr[1]
                +
              pFilter2dObj->padY*pObj->outPitch[1]/2
                +
              pFilter2dObj->padX
            ;

    pObj->outBufDesc.numPlanes                          = 2;
    pObj->outBufDesc.bufPlanes[0].frameROI.topLeft.x    = 0;
    pObj->outBufDesc.bufPlanes[0].frameROI.topLeft.y    = 0;
    pObj->outBufDesc.bufPlanes[0].width                 = pObj->outPitch[0];
    pObj->outBufDesc.bufPlanes[0].height                = pObj->outHeight;
    pObj->outBufDesc.bufPlanes[0].frameROI.width        = pObj->outWidth;
    pObj->outBufDesc.bufPlanes[0].frameROI.height       = pObj->outHeight;
    pObj->outBufDesc.bufPlanes[0].planeType             = 0; //Luma Y
    pObj->outBufDesc.bufPlanes[0].buf                   = pObj->outBufAddr[0];

    pObj->outBufDesc.bufPlanes[1].frameROI.topLeft.x    = 0;
    pObj->outBufDesc.bufPlanes[1].frameROI.topLeft.y    = 0;
    pObj->outBufDesc.bufPlanes[1].width                 = pObj->outPitch[1];
    pObj->outBufDesc.bufPlanes[1].height                = pObj->outHeight/2;
    pObj->outBufDesc.bufPlanes[1].frameROI.width        = pObj->outWidth;
    pObj->outBufDesc.bufPlanes[1].frameROI.height       = pObj->outHeight/2;
    pObj->outBufDesc.bufPlanes[1].planeType             = 1; //Chroma UV
    pObj->outBufDesc.bufPlanes[1].buf                   = pObj->outBufAddr[1];

    return 0;
}

Int32 Alg_yuvPaddingProcess(Alg_YuvPaddingObj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if(pObj->enable)
    {
        status = AlgIvision_process(
                    pObj->handle,
                    &pObj->inBufs,
                    &pObj->outBufs,
                    (IVISION_InArgs*)&pObj->inArgs,
                    (IVISION_OutArgs*)&pObj->outArgs
                );
        UTILS_assert(status==0);
    }
    return status;
}

Int32 Alg_yuvPaddingDelete(Alg_YuvPaddingObj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    if(pObj->enable)
    {
        status = AlgIvision_delete(pObj->handle);
        UTILS_assert(status==0);
    }

    return status;
}
