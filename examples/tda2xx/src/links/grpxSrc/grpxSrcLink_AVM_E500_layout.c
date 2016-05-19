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

//#define AVME500_FRAME_THICKNESS  (10)

#define CAR_START_X		191
#define CAR_START_Y		158
#define CAR_WIDTH		170
#define CAR_HEIGHT		404
#define X_MINUS	8
#define TOP_VIEW_TEXT_START_X	(552-X_MINUS-3)
#define TOP_VIEW_TEXT_START_Y	524
#define FULL_VIEW_TEXT_START_X	16
#define FULL_VIEW_TEXT_START_Y	574//558//584
#define FRONT_ICON_START_X		(554-X_MINUS)
#define REAR_ICON_START_X		(675-X_MINUS-3)
#define LEFT_ICON_START_X		(796-X_MINUS-5)
#define RIGHT_ICON_START_X		(917-X_MINUS-5)
#define FULLVIEW_ICON_START_X	(1039-X_MINUS-5)
#define SETTING_ICON_START_X	(1160-X_MINUS-5)
#define ICON_START_Y			584

#define TOP_FULLVIEW_START_XY	16
#define SIDEVIEW_START_X		552
#define TOP_VIEW_W				520
#define TOP_VIEW_H				688
#define SIDE_VIEW_W				712
#define SIDE_VIEW_H				508
#define FULL_VIEW_W				1248
#define FULL_VIEW_H				558

//#define COLOR_GREEN2     (0x960000)
//#define COLOR_RED2       (0x4C34FF)

UInt32 gGrpxSrcLinkID;
#define VERSION_TXT "Ver 1.0"

void drawVersionInfo( Draw2D_Handle pCtx, UInt32 startX, UInt32 startY, UInt32 fontIdx )
{
	Draw2D_FontPrm fontPrm;
	Draw2D_FontProperty fontProp;
	char verString[10];

	fontPrm.fontIdx = fontIdx;

	Draw2D_getFontProperty(&fontPrm, &fontProp);

	strcpy(verString, VERSION_TXT);

	Draw2D_clearString(pCtx, startX, startY, strlen(verString), &fontPrm );
	Draw2D_drawString(pCtx, startX, startY, verString,  &fontPrm );
}

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

	/* TOP VIEW */
	if( pObj->createArgs.sViewmode.viewmode == TOP_VIEW){ //ryuhs74@20151103 - AVM Top View
		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_NONE;
		Draw2D_drawBmp(pObj->draw2DHndl, FULLVIEW_ICON_START_X, ICON_START_Y, &bmpPrm);

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
	Draw2D_RegionPrm regionSideB;

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
	regionR.startX = 1280-19;//1280-16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionR.startY = 0;
	regionR.height = 720;
	regionR.width  = 19;//16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제

	Draw2D_fillRegion(pObj->draw2DHndl,&regionR);

	regionB.color  = AVME500_BACKGROUND_COLOR;
	regionB.startX = 0;
	regionB.startY = 720-16;
	regionB.height = 16;
	regionB.width  = 1280;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionB);

	regionM.color  = AVME500_BACKGROUND_COLOR;
	regionM.startX = 520+13;//520+16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionM.startY = 16;
	regionM.height = 720;//688; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionM.width  = 19;//16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제

	Draw2D_fillRegion(pObj->draw2DHndl,&regionM);

	regionSideB.color  = AVME500_BACKGROUND_COLOR;
	regionSideB.startX = 552; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionSideB.startY = FULL_VIEW_TEXT_START_Y;
	regionSideB.height = 720-(FULL_VIEW_TEXT_START_Y + 16);
	regionSideB.width  = 712;

	Draw2D_fillRegion(pObj->draw2DHndl,&regionSideB);


#endif
	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_TopView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;
	Draw2D_RegionPrm region;
	Draw2D_RegionPrm regionTopView;
	//Draw2D_LinePrm linePrm;

	//Top View, Side View Separation Bar
	region.color  = AVME500_BACKGROUND_COLOR;
	region.startX = 520+13;//520+16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	region.startY = 16;
	region.height = 688;
	region.width  = 19;//16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제

	Draw2D_fillRegion(pObj->draw2DHndl,&region);

	//Top View bottom of the transparent coloring
	regionTopView.color  = DRAW2D_TRANSPARENT_COLOR;
	regionTopView.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionTopView.startX = 16;
	regionTopView.startY = 524;
	regionTopView.width  = 520-3;//520; //ryuhs74 나중에 원복 - 초록색 한픽셀 삭제
	regionTopView.height = 720-( 524 +16 );
	Draw2D_fillRegion(pObj->draw2DHndl,&regionTopView);

	/*/ryuhs74@20151216 - Line Draw Test
	linePrm.lineSize = 5;
	linePrm.lineColor = COLOR_RED2;
	linePrm.lineColorFormat = SYSTEM_DF_BGR16_565;//SYSTEM_DF_BGR16_565
	Draw2D_drawLine(pObj->draw2DHndl, 16, 16, 182, 142, &linePrm );
	Draw2D_drawLine(pObj->draw2DHndl, 182+156, 36, 182+156+182, 36, &linePrm );
	Draw2D_drawLine(pObj->draw2DHndl, 16, 142+404+26, 182, 142+404+26, &linePrm );
	Draw2D_drawLine(pObj->draw2DHndl, 182+156, 142+404+26, 182+156+182, 142+404+26, &linePrm );
	*/
	if (pObj->createArgs.enableJeepOverlay == TRUE)
	{
		/* CMASK Image */
		/* TODO : Change Co-ordinates as per the requirement */
		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_CARMASK;
		Draw2D_drawBmp(pObj->draw2DHndl, CAR_START_X, CAR_START_Y, &bmpPrm );
	}

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TOP_VIEW_TXT;
	Draw2D_drawBmp(pObj->draw2DHndl, TOP_VIEW_TEXT_START_X, TOP_VIEW_TEXT_START_Y, &bmpPrm);
	//drawVersionInfo( pObj->draw2DHndl, TOP_VIEW_TEXT_START_X + 45, TOP_VIEW_TEXT_START_Y+40, 7 );

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_FullView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;
	Draw2D_RegionPrm regionMidleBar;
	Draw2D_RegionPrm regionBootmBar;

	/*/대각선 transparent coloring
	regionMidleBar.color  = DRAW2D_TRANSPARENT_COLOR;
	regionMidleBar.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionMidleBar.startX = 16;
	regionMidleBar.startY = 16;
	regionMidleBar.width  = 182;
	regionMidleBar.height = 142;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);

	regionMidleBar.startX = 182+156;
	regionMidleBar.startY = 16;
	regionMidleBar.width  = 156;
	regionMidleBar.height = 142;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);

	regionMidleBar.startX = 16;
	regionMidleBar.startY = 142+404;
	regionMidleBar.width  = 182;
	regionMidleBar.height = 720-16-142-404;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);

	regionMidleBar.startX = 182+156;
	regionMidleBar.startY = 142+404;
	regionMidleBar.width  = 156;
	regionMidleBar.height = 720-16-142-404;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);
	*/

	//Car Image transparent coloring
	regionMidleBar.color  = DRAW2D_TRANSPARENT_COLOR;
	regionMidleBar.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionMidleBar.startX = CAR_START_X;//182;
	regionMidleBar.startY = CAR_START_Y;//142;
	regionMidleBar.width  = CAR_WIDTH;//156;
	regionMidleBar.height = CAR_HEIGHT;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);

	//Top View, Side View Separation Bar of the transparent coloring
	regionMidleBar.color  = DRAW2D_TRANSPARENT_COLOR;
	regionMidleBar.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionMidleBar.startX = 552-19;//552-16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionMidleBar.startY = 16;
	regionMidleBar.width  = 19;//16; //ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionMidleBar.height = 558;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionMidleBar);

	//Side View bottom of the transparent coloring
	regionBootmBar.color  = DRAW2D_TRANSPARENT_COLOR;
	regionBootmBar.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	regionBootmBar.startX = 552;
	regionBootmBar.startY = 524;
	regionBootmBar.width  = 712-3;//712;//ryuhs74 나중에 원복 - - 초록색 한픽셀 삭제
	regionBootmBar.height = 50;
	Draw2D_fillRegion(pObj->draw2DHndl,&regionBootmBar);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_TXT;
	Draw2D_drawBmp(pObj->draw2DHndl, FULL_VIEW_TEXT_START_X, FULL_VIEW_TEXT_START_Y, &bmpPrm);
	drawVersionInfo( pObj->draw2DHndl, FULL_VIEW_TEXT_START_X+220, FULL_VIEW_TEXT_START_Y + 120, 7 );

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_ColorBarTestView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;
	int i=0;

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_COLORBARTEST;


	for(i=0; i<36;i++)
		Draw2D_drawBmp(pObj->draw2DHndl, 0, i*20, &bmpPrm);

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

void GrpxSrcLink_displayTxt(GrpxSrcLink_Obj *pObj, char* pStr, UInt32 isDisplay)
{
	Draw2D_FontPrm fontPrm;
	Draw2D_FontProperty fontProp;
	UInt32 startX = 32;
	UInt32 startY = 32;

	fontPrm.fontIdx = 5;
	Draw2D_getFontProperty(&fontPrm, &fontProp);

	if( isDisplay == 0){
		Draw2D_RegionPrm region;

	    Draw2D_clearString(pObj->draw2DHndl, startX, startY, strlen(pStr), &fontPrm );

	    region.color  = DRAW2D_TRANSPARENT_COLOR;
	    region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	    region.startX = startX;
	    region.startY = startY;
	    region.width  = fontProp.width * strlen(pStr);
	    region.height = fontProp.height;
		Draw2D_fillRegion(pObj->draw2DHndl,&region);
	} else {
		Draw2D_drawString(pObj->draw2DHndl, startX, startY, pStr, &fontPrm );
	}
}

void GrpxSrcLink_Clear_E500UI(GrpxSrcLink_Obj *pObj)
{
	Draw2D_RegionPrm region;
	region.color  = DRAW2D_TRANSPARENT_COLOR;
	region.colorFormat = DRAW2D_TRANSPARENT_COLOR_FORMAT;
	region.startX = 0;
	region.startY = 0;
	region.width  = 1280;
	region.height = 720;
	Draw2D_fillRegion(pObj->draw2DHndl,&region);
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
