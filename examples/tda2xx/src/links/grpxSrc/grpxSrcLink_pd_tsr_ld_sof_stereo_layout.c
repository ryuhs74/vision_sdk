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
#define CPU_BAR_HEIGHT      (80)

Int32 GrpxSrcLink_drawPdTsrLdSofStereoLayout(GrpxSrcLink_Obj *pObj)
{
    Draw2D_RegionPrm region;
    Draw2D_BmpPrm bmpPrm;
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    char loadString[GRPX_SRC_LINK_STR_SZ];
    Int32 startX;

    /* fill full buffer with black color */
    region.color  = RGB888_TO_RGB565(8,8,8);
    region.colorFormat = SYSTEM_DF_BGR16_565;
    region.startX = 0;
    region.startY = 0;
    region.height = pObj->info.queInfo[0].chInfo[0].height;
    region.width  = pObj->info.queInfo[0].chInfo[0].width;
    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* fill windows in buffer with transperency color */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 160;
    region.startY = 0;
    region.height = 900;
    region.width  = 1600;
    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* fill windows in buffer with transperency color */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 160+800+60;
    region.startY = 900;
    region.height = 180;
    region.width  = 320;
    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = 160+800+60+320+40;
    region.startY = 900;
    region.height = 180;
    region.width  = 320;
    Draw2D_fillRegion(pObj->draw2DHndl,&region);


    /* draw bitmap's */

    /* TI logo */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TI_LOGO;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   TI_LOGO_START_X,
                   TI_LOGO_START_Y,
                   &bmpPrm
                   );

    fontPrm.fontIdx = 5;

    /* draw algorithm name String */

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," PEDESTRIAN DETECT + TRAFFIC SIGN RECOGNITION ");

    startX = 160 + (800 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      5,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," LANE DETECT ");

    startX = 160 + 800 + (800 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      5,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," SPARSE OPTICAL FLOW ( MOTION ) ");

    startX = 160 + (800 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      450+5,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," STEREO DISPARITY MAP ( DEPTH ) ");

    startX = 160 + 800 + (800 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      450+5,
                      loadString,
                      &fontPrm);

    fontPrm.fontIdx = 5;

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," LEFT CAMERA ");

    startX = 160 + 800 + 60 + (320 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      900+5,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ," RIGHT CAMERA ");

    startX = 160 + 800 + 60 + 320 + 40 + (320 - strlen(loadString)*fontProp.width)/2;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      900+5,
                      loadString,
                      &fontPrm);

    /* Color Bar */
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_STEREO_COLORBAR_35x450;
    Draw2D_drawBmp(pObj->draw2DHndl,
                   160 + 800*2 + 20,
                   450,
                   &bmpPrm
                   );

    fontPrm.fontIdx = 7;

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ,"FAR");

    startX = 160 + 1600 + 20 + 20 + 10;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      450,
                      loadString,
                      &fontPrm);

    snprintf(loadString, GRPX_SRC_LINK_STR_SZ,"NEAR");

    startX = 160 + 1600 + 20 + 20 + 10;
    if(startX<0)
        startX = 0;

    Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      880,
                      loadString,
                      &fontPrm);

    return 0;
}

Int32 GrpxSrcLink_displayPdTsrLdSofStereoStats(GrpxSrcLink_Obj *pObj)
{
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    UInt32 startX, startY;

    fontPrm.fontIdx = 5;

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    startX = 160;
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
