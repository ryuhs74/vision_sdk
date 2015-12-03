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
 * \file grpxSrcLink_sved_layout.c
 *
 * \brief  This file has the implementation of GRPX layout for
 *         3D Surround view demo
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
#include <examples/tda2xx/src/links/uartCmd/uartCmd_priv.h> //ryuhs74@20151105

/**
 *******************************************************************************
 *
 * \brief Background Color of the Graphics Buffer
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define AVME500_BACKGROUND_COLOR ((UInt16)(RGB888_TO_RGB565(8,8,8)))

#define AVME500_FRAME_THICKNESS  (10)

#define CAR_START_X		200
#define CAR_START_Y		200
#define TOP_VIEW_TEXT_START_X	552
#define TOP_VIEW_TEXT_START_Y	524
#define FULL_VIEW_TEXT_START_X	16
#define FULL_VIEW_TEXT_START_Y	584
#define FRONT_ICON_START_X		554
#define REAR_ICON_START_X		675
#define LEFT_ICON_START_X		796
#define RIGHT_ICON_START_X		917
#define FULLVIEW_ICON_START_X	1039
#define SETTING_ICON_START_X	1160
#define ICON_START_Y			584

#define TOP_FULLVIEW_START_XY	16
#define SIDEVIEW_START_X		552
#define TOP_VIEW_W				520
#define TOP_VIEW_H				688
#define SIDE_VIEW_W				712
#define SIDE_VIEW_H				508
#define FULL_VIEW_W				1248
#define FULL_VIEW_H				558

UInt32 gGrpxSrcLinkID;

Int32 GrpxSrcLink_drawAVM_E500NorButton( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_NONE;
	Draw2D_drawBmp(pObj->draw2DHndl, FULLVIEW_ICON_START_X, ICON_START_Y, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, FRONT_ICON_START_X, ICON_START_Y, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_REAR_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, REAR_ICON_START_X, ICON_START_Y, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_RIGHT_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, RIGHT_ICON_START_X, ICON_START_Y, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_LEFT_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, LEFT_ICON_START_X, ICON_START_Y, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SETTING_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, SETTING_ICON_START_X, ICON_START_Y, &bmpPrm);

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 GrpxSrcLink_drawAVM_E500Button(GrpxSrcLink_Obj *pObj) //GrpxSrcLink_CreateParams* pObj;
{
	Draw2D_BmpPrm bmpPrm;


	/* TOP VIEW */
	if( pObj->createArgs.sViewmode.viewmode == TOP_VIEW){ //ryuhs74@20151103 - AVM Top View
		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_NONE;
		Draw2D_drawBmp(pObj->draw2DHndl, FULLVIEW_ICON_START_X, ICON_START_Y, &bmpPrm);

		if( pObj->createArgs.sViewmode.prvVient == FRONT_VIEW ){ //Prv Front Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, FRONT_ICON_START_X, ICON_START_Y, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.prvVient == REAR_VIEW ){ //Prv Rear Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_REAR_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, REAR_ICON_START_X, ICON_START_Y, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.prvVient == RIGHT_VIEW ){ //Prv Right Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_RIGHT_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, RIGHT_ICON_START_X, ICON_START_Y, &bmpPrm);
		} if( pObj->createArgs.sViewmode.prvVient == LEFT_VIEW ){ //Prv Left Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_LEFT_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, LEFT_ICON_START_X, ICON_START_Y, &bmpPrm);
		}

		if( pObj->createArgs.sViewmode.viewnt == FRONT_VIEW ){ //Draw Front Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, FRONT_ICON_START_X, ICON_START_Y, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.viewnt == REAR_VIEW ){ //Draw Rear Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_REAR_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, REAR_ICON_START_X, ICON_START_Y, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.viewnt == RIGHT_VIEW ){ //Draw Right Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_RIGHT_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, RIGHT_ICON_START_X, ICON_START_Y, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.viewnt == LEFT_VIEW ){ //Draw Left Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_LEFT_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, LEFT_ICON_START_X, ICON_START_Y, &bmpPrm);
		}

		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SETTING_VIEW_NOR;
		Draw2D_drawBmp(pObj->draw2DHndl, SETTING_ICON_START_X, 584, &bmpPrm);
	} else if( pObj->createArgs.sViewmode.viewmode == FULL_VIEW ){ //ryuhs74@20151103 - AVM Full View
		if( pObj->createArgs.sViewmode.viewnt == FRONT_VIEW){ //ryuhs74@20151103 - AVM FRONT Full View
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_FRONT;
			Draw2D_drawBmp(pObj->draw2DHndl, FULLVIEW_ICON_START_X, ICON_START_Y, &bmpPrm);
		} else { //ryuhs74@20151103 - AVM Rear Full View
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_REAR;
			Draw2D_drawBmp(pObj->draw2DHndl, FULLVIEW_ICON_START_X, ICON_START_Y, &bmpPrm);
		}

		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SETTING_VIEW_NOR;
		Draw2D_drawBmp(pObj->draw2DHndl, SETTING_ICON_START_X, ICON_START_Y, &bmpPrm);
	}

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_FillBacgroundColor( GrpxSrcLink_Obj *pObj )
{
#if 0
	Draw2D_RegionPrm region;

	region.color  = AVME500_BACKGROUND_COLOR;
	region.startX = 0;
	region.startY = 0;
	region.height = 720;
	region.width  = 1280;

	Draw2D_fillRegion(pObj->draw2DHndl,&region);
#else
	Draw2D_RegionPrm regionT;
	Draw2D_RegionPrm regionL;
	Draw2D_RegionPrm regionR;
	Draw2D_RegionPrm regionB;
	Draw2D_RegionPrm regionM;

	regionT.color  = AVME500_BACKGROUND_COLOR;
	regionT.startX = 0;
	regionT.startY = 0;
	regionT.height = 16;
	regionT.width  = 1280;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionT);

	regionL.color  = AVME500_BACKGROUND_COLOR;
	regionL.startX = 0;
	regionL.startY = 0;
	regionL.height = 720;
	regionL.width  = 16;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionL);

	regionR.color  = AVME500_BACKGROUND_COLOR;
	regionR.startX = 1280-16;
	regionR.startY = 0;
	regionR.height = 720;
	regionR.width  = 16;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionR);

	regionB.color  = AVME500_BACKGROUND_COLOR;
	regionB.startX = 0;
	regionB.startY = 720-16;
	regionB.height = 16;
	regionB.width  = 1280;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionB);

	regionM.color  = AVME500_BACKGROUND_COLOR;
	regionM.startX = 552-17;
	regionM.startY = 0;
	regionM.height = 574;
	regionM.width  = 16;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionM);


#endif
	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_TopView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;
	Draw2D_RegionPrm region;
	Draw2D_RegionPrm regionTopView;

	//Top View, Side View Separation Bar
	region.color  = AVME500_BACKGROUND_COLOR;
	region.startX = 552-17;
	region.startY = 0;
	region.height = 574;
	region.width  = 16;

	Draw2D_fillRegion(pObj->draw2DHndl,&region);

	//Top View bottom of the transparent coloring
	regionTopView.color  = DRAW2D_TRANSPARENT_COLOR;
	regionTopView.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionTopView.startX = 16;
	regionTopView.startY = 524;
	regionTopView.width  = 520-10;
	regionTopView.height = 720-524;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionTopView);

	if (pObj->createArgs.enableJeepOverlay == TRUE)
	{
		/* CMASK Image */
		/* TODO : Change Co-ordinates as per the requirement */
		//bmpPrm.bmpIdx = DRAW2D_BMP_IDX_JEEP_IMAGE_TRUESCALE;
		//Draw2D_drawBmp(pObj->draw2DHndl,
		//			(326+520), //520 comes from layout
		//			(324+5), //5 comes from layout
		//			&bmpPrm
		//			);
	}

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TOP_VIEW_TXT;
	Draw2D_drawBmp(pObj->draw2DHndl, TOP_VIEW_TEXT_START_X, TOP_VIEW_TEXT_START_Y, &bmpPrm);

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_FullView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;
	Draw2D_RegionPrm regionMidleBar;
	Draw2D_RegionPrm regionBootmBar;

	//Top View, Side View Separation Bar of the transparent coloring
	regionMidleBar.color  = DRAW2D_TRANSPARENT_COLOR;
	regionMidleBar.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionMidleBar.startX = 552-16;
	regionMidleBar.startY = 0;
	regionMidleBar.width  = 16;
	regionMidleBar.height = 574;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);

	//Side View bottom of the transparent coloring
	regionBootmBar.color  = DRAW2D_TRANSPARENT_COLOR;
	regionBootmBar.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionBootmBar.startX = 552;
	regionBootmBar.startY = 524;
	regionBootmBar.width  = 712;
	regionBootmBar.height = 60;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionBootmBar);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_TXT;
	Draw2D_drawBmp(pObj->draw2DHndl, FULL_VIEW_TEXT_START_X, FULL_VIEW_TEXT_START_Y, &bmpPrm);

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 GrpxSrcLink_drawAVM_E500Layout(GrpxSrcLink_Obj *pObj) // 이 함수를 TOP / FULL 그리는 함수를 다시 나눈다.
{
    /* fill full buffer with background color */
    Draw2D_FillBacgroundColor( pObj );

    /*
	 * fill transprenecy color in portions where video should be visible
	 * Section
	 */
    if( pObj->createArgs.sViewmode.viewmode == TOP_VIEW){ //ryuhs74@20151103 - AVM Top View
    	Draw2D_AVME500_TopView( pObj );
    } else if( pObj->createArgs.sViewmode.viewmode == FULL_VIEW ){ //ryuhs74@20151103 - AVM Full View
    	Draw2D_AVME500_FullView( pObj );
    }

    GrpxSrcLink_drawAVM_E500NorButton(pObj);
    GrpxSrcLink_drawAVM_E500Button(pObj);

    return SYSTEM_LINK_STATUS_SOK;
}

#if 0
Void GrpxSrcLink_displaySurroundViewStandaloneDrawCpuLoadBar(
                    GrpxSrcLink_Obj *pObj,
                    UInt32 cpuLoadInt,
                    UInt32 cpuLoadFract,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height
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

    /* fill in in active load color */
    region.color  = color[0];
    region.startX = startX;
    region.startY = startY;
    region.height = barHeight[0];
    region.width  = width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

    /* fill active load color */
    region.color  = color[1];
    region.startX = startX;
    region.startY = startY + barHeight[0];
    region.height = barHeight[1];
    region.width  = width;

    Draw2D_fillRegion(pObj->draw2DHndl,&region);

}

Int32 GrpxSrcLink_displaySurroundViewStandaloneStats(GrpxSrcLink_Obj *pObj)
{
    Draw2D_FontPrm fontPrm;
    UInt32 procId;
    Utils_SystemLoadStats *loadStats;
    char loadString[GRPX_SRC_LINK_STR_SZ];
    Draw2D_FontProperty fontProp;
    UInt32 startX, startY, startX1, startY1;
    UInt32 statsHeight;

    fontPrm.fontIdx = 3;

    Draw2D_getFontProperty(&fontPrm, &fontProp);

    statsHeight = 180;
    startY =  180 - fontProp.height;
    startX =  pObj->info.queInfo[0].chInfo[0].width - 520;
    startX1 = pObj->info.queInfo[0].chInfo[0].width - 520;
    startY1 = 0 + SVED_FRAME_THICKNESS;


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

            if(BSP_PLATFORM_SOC_ID_TDA2EX == Bsp_platformGetSocId()
                &&
                (procId == SYSTEM_PROC_DSP2
                    ||
                 procId == SYSTEM_PROC_EVE1
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
                          "%-4s\n",
                          System_getProcName(procId)
                          );

            if (SYSTEM_PROC_IPU1_0 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          "M4-0"
                          );
            }
            else if (SYSTEM_PROC_IPU1_1 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          "M4-1"
                          );
            }
            else if (SYSTEM_PROC_A15_0 == procId)
            {
               snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%-4s\n",
                          "A15"
                          );
               /* HACK for Electronica, needs to fix properly */
#ifdef A15_TARGET_OS_LINUX
               loadStats->totalLoadParams.integerValue = 3;
               loadStats->totalLoadParams.fractionalValue = 0;
#endif
            }
            else
            {
                /*
                 * Avoid MISRA C Warnings
                 */
            }

            /* draw CPU name */
            Draw2D_clearString(pObj->draw2DHndl,
                      startX,
                      startY,
                      strlen(loadString),
                      &fontPrm
                      );

            Draw2D_drawString(pObj->draw2DHndl,
                      startX,
                      startY,
                      loadString,
                      &fontPrm
                      );

            GrpxSrcLink_displaySurroundViewStandaloneDrawCpuLoadBar
                (
                    pObj,
                    loadStats->totalLoadParams.integerValue, /* CPU load integer value */
                    loadStats->totalLoadParams.fractionalValue, /* CPU load integer value */
                    startX,
                    fontProp.height + SVED_FRAME_THICKNESS*2,
                    30,
                    (statsHeight - SVED_FRAME_THICKNESS) - (fontProp.height + SVED_FRAME_THICKNESS)*2
                );

            /* draw CPU load as text */
            snprintf(loadString, GRPX_SRC_LINK_STR_SZ,
                          "%02d.%d%%\n",
                          loadStats->totalLoadParams.integerValue,
                          loadStats->totalLoadParams.fractionalValue
                          );
            Draw2D_clearString(pObj->draw2DHndl,
                      startX1,
                      startY1,
                      strlen(loadString),
                      &fontPrm
                      );

            Draw2D_drawString(pObj->draw2DHndl,
                      startX1,
                      startY1,
                      loadString,
                      &fontPrm
                      );
           startX1 = startX1 + fontProp.width*6 + 0;
           startX = startX+fontProp.width*6 + 0;
    }
    return 0;
}
#endif
