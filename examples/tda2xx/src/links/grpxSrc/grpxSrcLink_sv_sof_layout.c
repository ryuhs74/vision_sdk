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
 * \file grpxSrcLink_sv_sof_layout.c
 *
 * \brief  This file has the implementation of GRPX layout for
 *         2D Surround view + SOF demo on TDA3xx
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


/**
 *******************************************************************************
 *
 * \brief Background Color of the Graphics Buffer
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define SV_SOF_BACKGROUND_COLOR ((UInt16)(RGB888_TO_RGB565(16,16,16)))

/* TDA3xx defaults for 1024 X 768 resolution */
UInt32  gSvDispWidth = 1024U;
UInt32  gSvDispHeight = 768U;
UInt32  gSvOutWidth = 640U;
UInt32  gSvOutHeight = 760U;
UInt32  gSvStartX = 0U;
UInt32  gSvSofBorder = 5U;
UInt32  gSvSofJeepWidth = 220U;
UInt32  gSvSofJeepHeight = 330U;
UInt32  gSvLogoWidth = 480U;
UInt32  gSvMode2OutWidth = 1000U;
UInt32  gSvMode2OutHeight = 760U;
UInt32  gSvSofStatsHeight = 0U;

Int32 GrpxSrcLink_drawSurroundViewSOFLayout(GrpxSrcLink_Obj *pObj,
                                            System_LinkChInfo *pChInfo)
{
    Draw2D_RegionPrm region;
    Draw2D_BmpPrm bmpPrm;
    Draw2D_FontPrm fontPrm;
    Draw2D_FontProperty fontProp;
    char loadString[GRPX_SRC_LINK_STR_SZ];
    UInt32 curX, curY, rot;

    /* Get the position based on the resolution */
    gSvDispWidth = pChInfo->width;
    gSvDispHeight = pChInfo->height;
    gSvStartX = gSvDispWidth - gSvOutWidth;
    gSvSofStatsHeight = ((gSvOutHeight / 8U) - (gSvSofBorder*2));
    if ((1280U == gSvDispWidth) && (720U == gSvDispHeight))
    {
        gSvOutWidth = 1000U;
        gSvOutHeight = 760U;
        gSvStartX = gSvDispWidth - gSvOutWidth;
        gSvMode2OutHeight = gSvDispHeight;
        gSvSofStatsHeight = gSvOutHeight / 6U;
    }

    /* fill full buffer with background color */
    region.color  = SV_SOF_BACKGROUND_COLOR;
    region.colorFormat = SYSTEM_DF_BGR16_565;
    region.startX = 0;
    region.startY = 0;
    region.height = pObj->info.queInfo[0].chInfo[0].height;
    region.width  = pObj->info.queInfo[0].chInfo[0].width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* fill transparency color in portions where video should be visible */

    /* SV */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = gSvStartX + gSvSofBorder;
    region.startY = gSvSofBorder;
    region.width  = gSvOutWidth - 2*gSvSofBorder;
    region.height = gSvDispHeight - 2*gSvSofBorder;
    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* SRV data */
    region.color  = DRAW2D_TRANSPARENT_COLOR;
    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
    region.startX = ((gSvDispWidth - gSvMode2OutWidth)/2);
    region.startY = ((gSvDispHeight - gSvMode2OutHeight)/2);
    region.width  = gSvMode2OutWidth;
    region.height = gSvMode2OutHeight;
    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* draw bitmap's */
    /* TI logo */
    curX = gSvDispWidth - ((gSvDispWidth - gSvMode2OutWidth)/2);
    curY = ((gSvDispHeight - gSvMode2OutHeight)/2);
    rot = 2;
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TI_LOGO_SMALL;
    Draw2D_drawBmp_rot(pObj->draw2DHndl,
                   curX,
                   curY,
                   &bmpPrm,
                   rot);

    /* Surround view logo */
    curX = gSvDispWidth - gSvSofBorder*4;
    curY = (gSvDispHeight/2) - (gSvLogoWidth/2);
    rot = 2;
    bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SURROUND_VIEW_SMALL;
    Draw2D_drawBmp_rot(pObj->draw2DHndl,
                   curX,
                   curY,
                   &bmpPrm,
                   rot
                   );

    if (pObj->createArgs.enableJeepOverlay == TRUE)
    {
        /* Jeep Image */
        bmpPrm.bmpIdx = DRAW2D_BMP_IDX_JEEP_220_X_330;
        curX = (gSvDispWidth/2 + gSvSofJeepHeight/2);
        curY = (gSvDispHeight/2 - gSvSofJeepWidth/2);
        rot = 2;
        Draw2D_drawBmp_rot(pObj->draw2DHndl,
                       curX,
                       curY,
                       &bmpPrm,
                       rot);
    }

    /* String for input resolution */
    fontPrm.fontIdx = 7;
    Draw2D_getFontProperty(&fontPrm, &fontProp);

    /* String for output resolution */
    snprintf(loadString, GRPX_SRC_LINK_STR_SZ, "RESOLUTION: %dx%d",
                gSvOutHeight, gSvMode2OutWidth);
    if ((1280U == gSvDispWidth) && (720U == gSvDispHeight))
    {
        curX = ((gSvDispWidth - gSvMode2OutWidth)/2) - fontProp.height - 1U;
    }
    else
    {
        curX = ((gSvDispWidth - gSvMode2OutWidth)/2) + fontProp.height - 1U;
    }
    curY = gSvDispHeight - ((gSvDispHeight - gSvMode2OutHeight)/2) - fontProp.width*strlen(loadString);
    rot = 2;
    Draw2D_drawString_rot(pObj->draw2DHndl,
          curX,
          curY,
          loadString,
          &fontPrm,
          rot
          );

    return SYSTEM_LINK_STATUS_SOK;
}

Void GrpxSrcLink_displaySurroundViewSOFDrawCpuLoadBar_rot(
                    GrpxSrcLink_Obj *pObj,
                    UInt32 cpuLoadInt,
                    UInt32 cpuLoadFract,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height,
                    UInt32 rotate
                )
{
    Draw2D_RegionPrm region;
    UInt32 color[2];
    UInt32 barHeight[2];

    color[0] = RGB888_TO_RGB565(40, 40, 40);
    color[1] = RGB888_TO_RGB565(0, 160, 0);

    if(cpuLoadFract>=5)
        cpuLoadInt++;

    barHeight[0] = (height * (100 - cpuLoadInt))/100;

    if(barHeight[0] > height)
        barHeight[0] = height;

    barHeight[1] = height - barHeight[0];

    if(0 == rotate)
    {
        /* fill in in active load color */
        region.color  = color[0];
        region.colorFormat = SYSTEM_DF_BGR16_565;
        region.startX = startX;
        region.startY = startY;
        region.height = barHeight[0];
        region.width  = width;

        Draw2D_fillRegion(pObj->draw2DHndl,&region);

        /* fill active load color */
        region.color  = color[1];
        region.colorFormat = SYSTEM_DF_BGR16_565;
        region.startX = startX;
        region.startY = startY + barHeight[0];
        region.height = barHeight[1];
        region.width  = width;

        Draw2D_fillRegion(pObj->draw2DHndl,&region);
    }
    else if(1 == rotate)
    {
        /* fill in in active load color */
        region.color  = color[0];
        region.colorFormat = SYSTEM_DF_BGR16_565;
        region.startX = startX;
        region.startY = startY - width;
        region.height = width;
        region.width  = barHeight[0];

        Draw2D_fillRegion(pObj->draw2DHndl,&region);

        /* fill active load color */
        region.color  = color[1];
        region.colorFormat = SYSTEM_DF_BGR16_565;
        region.startX = startX + barHeight[0];
        region.startY = startY - width;
        region.height = width;
        region.width  = barHeight[1];

        Draw2D_fillRegion(pObj->draw2DHndl,&region);
    }
    else if(2 == rotate)
    {
        /* fill in in active load color */
        region.color  = color[0];
        region.colorFormat = SYSTEM_DF_BGR16_565;
        region.startX = startX - barHeight[0];
        region.startY = startY;
        region.height = width;
        region.width  = barHeight[0];

        Draw2D_fillRegion(pObj->draw2DHndl,&region);

        /* fill active load color */
        region.color  = color[1];
        region.colorFormat = SYSTEM_DF_BGR16_565;
        region.startX = startX - barHeight[0] - barHeight[1];
        region.startY = startY;
        region.height = width;
        region.width  = barHeight[1];

        Draw2D_fillRegion(pObj->draw2DHndl,&region);
    }
}

Int32 GrpxSrcLink_displaySurroundViewSOFStats(GrpxSrcLink_Obj *pObj)
{
    Draw2D_FontPrm fontPrm;
    UInt32 procId;
    Utils_SystemLoadStats *loadStats;
    char loadString[GRPX_SRC_LINK_STR_SZ];
    Draw2D_FontProperty fontProp;
    UInt32 startX, startY, startX1, startY1, rot = 0;
    UInt32 statsHeight;
    UInt32 barWidth, barHeight, barOffset;

    fontPrm.fontIdx = 7;
    Draw2D_getFontProperty(&fontPrm, &fontProp);

    if ((1280U == gSvDispWidth) && (720U == gSvDispHeight))
    {
        startX =  ((gSvDispWidth - gSvMode2OutWidth)/2) +
                    fontProp.height - gSvSofStatsHeight;
    }
    else
    {
        startX =  ((gSvDispWidth - gSvMode2OutWidth)/2) + fontProp.height;
    }
    startY =  ((gSvDispHeight - gSvMode2OutHeight)/2);
    statsHeight = gSvSofStatsHeight;
    barHeight = (statsHeight - fontProp.height*2 - gSvSofBorder*4);
    barWidth = fontProp.width*2;
    barOffset = fontProp.width; /* Offset from startX */
    startY1 = startY;
    startX1 = startX + gSvSofBorder*2 + barHeight + fontProp.height;
    rot = 2;

    for (procId = 0; procId < SYSTEM_PROC_MAX; procId++)
    {
            loadStats = &pObj->statsDisplayObj.systemLoadStats[procId];

            if(Bsp_platformIsTda3xxFamilyBuild()
                &&
                (procId == SYSTEM_PROC_A15_0
                    ||
                 procId == SYSTEM_PROC_EVE2
                    ||
                 procId == SYSTEM_PROC_EVE3
                    ||
                 procId == SYSTEM_PROC_EVE4
                )
              )
            {
                /* These CPUs dont exist in TDA3xx */
                continue;
            }

            snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s",
                          System_getProcName(procId)
                          );

            if (SYSTEM_PROC_IPU1_0 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s",
                          "M4-0"
                          );
            }
            else if (SYSTEM_PROC_IPU1_1 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s",
                          "M4-1"
                          );
            }
            else if (SYSTEM_PROC_A15_0 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s",
                          "A15"
                          );
               /* HACK for Electronica, needs to fix properly */
               loadStats->totalLoadParams.integerValue = 14;
               loadStats->totalLoadParams.fractionalValue = 7;
            }
            else
            {
                /*
                 * Avoid MISRA C Warnings
                 */
            }

            /* draw CPU name */
            Draw2D_drawString_rot(pObj->draw2DHndl,
                      startX,
                      startY,
                      loadString,
                      &fontPrm,
                      rot
                      );

            if (0 == rot)
            {
                GrpxSrcLink_displaySurroundViewSOFDrawCpuLoadBar_rot
                    (
                        pObj,
                        loadStats->totalLoadParams.integerValue, /* CPU load integer value */
                        loadStats->totalLoadParams.fractionalValue, /* CPU load integer value */
                        startX + barOffset,
                        startY1 + gSvSofBorder + fontProp.height,
                        barWidth,
                        barHeight,
                        rot
                    );
            }
            else if (1 == rot)
            {
                GrpxSrcLink_displaySurroundViewSOFDrawCpuLoadBar_rot
                    (
                        pObj,
                        loadStats->totalLoadParams.integerValue, /* CPU load integer value */
                        loadStats->totalLoadParams.fractionalValue, /* CPU load integer value */
                        startX1 + fontProp.height + gSvSofBorder,
                        startY - barOffset,
                        barWidth,
                        barHeight,
                        rot
                    );
            }
            else if (2 == rot)
            {
                GrpxSrcLink_displaySurroundViewSOFDrawCpuLoadBar_rot
                    (
                        pObj,
                        loadStats->totalLoadParams.integerValue, /* CPU load integer value */
                        loadStats->totalLoadParams.fractionalValue, /* CPU load integer value */
                        startX1 - fontProp.height - gSvSofBorder,
                        startY + barOffset,
                        barWidth,
                        barHeight,
                        rot
                    );
            }

            snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%02d.%d%%",
                          loadStats->totalLoadParams.integerValue,
                          loadStats->totalLoadParams.fractionalValue
                          );

            /* draw CPU load as text */
            Draw2D_drawString_rot(pObj->draw2DHndl,
                      startX1,
                      startY1,
                      loadString,
                      &fontPrm,
                      rot
                      );

            if(0 == rot)
            {
                startX1 = startX1 + fontProp.width*6 + 0;
                startX  = startX  + fontProp.width*6 + 0;
            }
            else if(1 == rot)
            {
                startY1 = startY1 - fontProp.width*6 + 0;
                startY  = startY  - fontProp.width*6 + 0;
            }
            else if(2 == rot)
            {
                startY1 = startY1 + fontProp.width*6 + 0;
                startY  = startY  + fontProp.width*6 + 0;
            }
    }
    return 0;
}

