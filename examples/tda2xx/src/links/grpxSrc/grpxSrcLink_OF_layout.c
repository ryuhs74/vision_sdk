/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file grpxSrcLink_OF_layout.c
 *
 * \brief  This file has the implementation of GRPX layout for Optical Flow
 *         demo
 *
 * \version 0.0 (Oct 2013) : [NN] First version
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "grpxSrcLink_priv.h"

#define RGB888_TO_RGB565(r,g,b)     ((((UInt32)(r>>3) & 0x1F) << 11) | (((UInt32)(g>>2) & 0x3F) << 5) | (((UInt32)(b>>3) & 0x1F)))

/**
 *******************************************************************************
 *
 * \brief Background Color of the Graphics Buffer
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define OF_BACKGROUND_COLOR (DRAW2D_TRANSPARENT_COLOR)

#define OF_FRAME_THICKNESS  (25)
#define OF_LUT_HEIGHT       (129)

Int32 GrpxSrcLink_drawOpticalFlowLayout(GrpxSrcLink_Obj *pObj)
{
    Draw2D_RegionPrm region;
    Draw2D_BmpPrm bmpPrm;
    GrpxSrcLink_CreateParams *createPrms = &pObj->createArgs;
    GrpxSrcLink_OpticalFlowParameters *ofPrms;
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    System_LinkInfo captLinkInfo;
    UInt32 width = 0,height = 0;
    char loadString[GRPX_SRC_LINK_STR_SZ];

    /* fill full buffer with background color */
    region.color  = OF_BACKGROUND_COLOR;
    region.colorFormat = SYSTEM_DF_BGR16_565;
    region.startX = 0;
    region.startY = 0;
    region.height = pObj->info.queInfo[0].chInfo[0].height;
    region.width  = pObj->info.queInfo[0].chInfo[0].width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    ofPrms = &createPrms->opticalFlowParams;

    switch(ofPrms->lutId)
    {
        default:
        case 0:
            bmpPrm.bmpIdx = DRAW2D_BMP_IDX_OPTFLOW_LUT_0;
            break;
        case 1:
            bmpPrm.bmpIdx = DRAW2D_BMP_IDX_OPTFLOW_LUT_1;
            break;
        case 2:
            bmpPrm.bmpIdx = DRAW2D_BMP_IDX_OPTFLOW_LUT_2;
            break;
    }
    Draw2D_drawBmp(pObj->draw2DHndl,
                   OF_FRAME_THICKNESS,
                   pObj->info.queInfo[0].chInfo[0].height - (OF_LUT_HEIGHT + OF_FRAME_THICKNESS),
                   &bmpPrm
                   );

    /* draw bitmap's */

    /* TI logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TI_LOGO;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   OF_FRAME_THICKNESS,
                   OF_FRAME_THICKNESS,
                   &bmpPrm
                   );

    fontPrm.fontIdx = 2;

    /* draw resolution String */

    Draw2D_getFontProperty(&fontPrm, &fontProp);


    System_linkGetInfo(SYSTEM_LINK_ID_CAPTURE,&captLinkInfo);

    if(captLinkInfo.queInfo[0].chInfo[0].width == 1280)
    {
        width = 1280;
        height = 720;
    }
    else if(captLinkInfo.queInfo[0].chInfo[0].width == 1920)
    {
        width = 1920;
        height = 1080;
    }
    snprintf(loadString, GRPX_SRC_LINK_STR_SZ, "RESOLUTION : %d x %d", width, height);

    /* TODO : Get Resolution and FPS from OF Alg Plugin, instead of Hardcoding*/
    Draw2D_drawString(pObj->draw2DHndl,
                      (OF_FRAME_THICKNESS * 2 + OF_LUT_HEIGHT),
                      pObj->info.queInfo[0].chInfo[0].height - (OF_LUT_HEIGHT + OF_FRAME_THICKNESS),
                      loadString,
                      &fontPrm);
    return 0;
}

Int32 GrpxSrcLink_displayOpticalFlowDetectStats(GrpxSrcLink_Obj *pObj)
{
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    UInt32 startX, startY;

    fontPrm.fontIdx = 7;

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    startX = OF_FRAME_THICKNESS*2 + OF_LUT_HEIGHT ;
    startY = pObj->info.queInfo[0].chInfo[0].height - (OF_FRAME_THICKNESS + OF_LUT_HEIGHT / 2 + fontProp.height*2 + 8);

    GrpxSrcLink_drawCpuLoad(pObj,
            startX,
            startY,
            30,
            OF_LUT_HEIGHT/2,
            4,
            4,
            7
            );

    return 0;
}
