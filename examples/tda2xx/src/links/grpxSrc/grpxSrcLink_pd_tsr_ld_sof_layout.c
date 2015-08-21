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

#define TI_LOGO_START_X     (10)
#define TI_LOGO_START_Y     (10)

#define CPU_BAR_WIDTH       (30)
#define CPU_BAR_HEIGHT      (100)

Int32 GrpxSrcLink_drawPdTsrLdSofLayout(GrpxSrcLink_Obj *pObj)
{
    Draw2D_RegionPrm region;
    Draw2D_BmpPrm bmpPrm;
    Draw2D_BmpProperty bmpProp;
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    char loadString[GRPX_SRC_LINK_STR_SZ];
    Int32 startX;

    /* fill full buffer with background color */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 0;
    region.startY = 0;
    region.height = pObj->info.queInfo[0].chInfo[0].height;
    region.width  = pObj->info.queInfo[0].chInfo[0].width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* draw bitmap's */

    /* TI logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TI_LOGO;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   TI_LOGO_START_X,
                   TI_LOGO_START_Y,
                   &bmpPrm
                   );

    fontPrm.fontIdx = 1;

    /* draw algorithm name String */

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ,"PEDESTRIAN DETECT + TSR");

    startX = (pObj->info.queInfo[0].chInfo[0].width/3 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      pObj->info.queInfo[0].chInfo[0].height/4 - fontProp.height,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ,"LANE DETECT");

    startX = (pObj->info.queInfo[0].chInfo[0].width/3 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX + 640,
                      pObj->info.queInfo[0].chInfo[0].height/4 - fontProp.height,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," SPARSE OPTICAL FLOW ( MOTION ) ");

    startX = (pObj->info.queInfo[0].chInfo[0].width/3 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX + 1280,
                      pObj->info.queInfo[0].chInfo[0].height/4 - fontProp.height,
                      loadString,
                      &fontPrm);



    /* Front cam analytics logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_CAM_ANALYTICS;

    Draw2D_getBmpProperty(&bmpPrm, &bmpProp);

    startX = (pObj->info.queInfo[0].chInfo[0].width - bmpProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawBmp(pObj->draw2DHndl,
                   startX,
                   0,
                   &bmpPrm
                   );

    return 0;
}

Int32 GrpxSrcLink_displayPdTsrLdSofStats(GrpxSrcLink_Obj *pObj)
{
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    UInt32 startX, startY;

    fontPrm.fontIdx = 1;

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    startX = TI_LOGO_START_X;
    startY = pObj->info.queInfo[0].chInfo[0].height - (TI_LOGO_START_Y + CPU_BAR_HEIGHT + fontProp.height*2 + 8);

    GrpxSrcLink_drawCpuLoad(pObj,
            startX,
            startY,
            CPU_BAR_WIDTH,
            CPU_BAR_HEIGHT,
            4,
            4,
            fontPrm.fontIdx
            );

    return 0;
}
