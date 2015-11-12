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
#include <examples/tda2xx/src/links/uartCmd/uartCmd_priv.h> //ryuhs74@20151105 - Test UI CMD

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

UInt32 gGrpxSrcLinkID;

Int32 GrpxSrcLink_drawAVM_E500NorButton( GrpxSrcLink_Obj *pObj )
{
	Draw2D_BmpPrm bmpPrm;

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_NONE;
	Draw2D_drawBmp(pObj->draw2DHndl, 1054, 584, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, 554, 584, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_REAR_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, 679, 584, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_RIGHT_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, 929, 584, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_LEFT_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, 804, 584, &bmpPrm);

	bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SETTING_VIEW_NOR;
	Draw2D_drawBmp(pObj->draw2DHndl, 1179, 584, &bmpPrm);

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 GrpxSrcLink_drawAVM_E500Button(GrpxSrcLink_Obj *pObj) //GrpxSrcLink_CreateParams* pObj;
{
	Draw2D_BmpPrm bmpPrm;


	/* TOP VIEW */
	if( pObj->createArgs.sViewmode.viewmode == TOP_VIEW){ //ryuhs74@20151103 - AVM Top View
		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_NONE;
		Draw2D_drawBmp(pObj->draw2DHndl, 1054, 584, &bmpPrm);

		if( pObj->createArgs.sViewmode.prvVient == FRONT_VIEW ){ //Prv Front Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, 554, 584, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.prvVient == REAR_VIEW ){ //Prv Rear Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_REAR_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, 679, 584, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.prvVient == RIGHT_VIEW ){ //Prv Right Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_RIGHT_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, 929, 584, &bmpPrm);
		} if( pObj->createArgs.sViewmode.prvVient == LEFT_VIEW ){ //Prv Left Sel Image, Draw Nor Image
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_LEFT_VIEW_NOR;
			Draw2D_drawBmp(pObj->draw2DHndl, 804, 584, &bmpPrm);
		}

		if( pObj->createArgs.sViewmode.viewnt == FRONT_VIEW ){ //Draw Front Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FRONT_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, 554, 584, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.viewnt == REAR_VIEW ){ //Draw Rear Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_REAR_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, 679, 584, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.viewnt == RIGHT_VIEW ){ //Draw Right Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_RIGHT_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, 929, 584, &bmpPrm);
		} else if( pObj->createArgs.sViewmode.viewnt == LEFT_VIEW ){ //Draw Left Sel
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_LEFT_VIEW_SEL;
			Draw2D_drawBmp(pObj->draw2DHndl, 804, 584, &bmpPrm);
		}

		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SETTING_VIEW_NOR;
		Draw2D_drawBmp(pObj->draw2DHndl, 1179, 584, &bmpPrm);
	} else if( pObj->createArgs.sViewmode.viewmode == FULL_VIEW ){ //ryuhs74@20151103 - AVM Full View
		if( pObj->createArgs.sViewmode.viewnt == FRONT_VIEW){ //ryuhs74@20151103 - AVM FRONT Full View
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_FRONT;
			Draw2D_drawBmp(pObj->draw2DHndl, 1054, 584, &bmpPrm);
		} else { //ryuhs74@20151103 - AVM Rear Full View
			bmpPrm.bmpIdx = DRAW2D_BMP_IDX_FULL_VIEW_REAR;
			Draw2D_drawBmp(pObj->draw2DHndl, 1054, 584, &bmpPrm);
		}

		bmpPrm.bmpIdx = DRAW2D_BMP_IDX_SETTING_VIEW_NOR;
		Draw2D_drawBmp(pObj->draw2DHndl, 1179, 584, &bmpPrm);
	}

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_FillBacgroundColor( GrpxSrcLink_Obj *pObj )
{
	Draw2D_RegionPrm region;

	region.color  = AVME500_BACKGROUND_COLOR;
	region.startX = 0;
	region.startY = 0;
	region.height = pObj->info.queInfo[0].chInfo[0].height;
	region.width  = pObj->info.queInfo[0].chInfo[0].width;

	Draw2D_fillRegion(pObj->draw2DHndl,&region);

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_TopView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_RegionPrm region;
	Draw2D_FontPrm fontPrm;
	//Draw2D_BmpPrm bmpPrm;

	/* TOP( Surround ) View video Region */
	region.color  = DRAW2D_TRANSPARENT_COLOR;
	region.startX = 16;
	region.startY = 16;
	region.height = 520;
	region.width  = 688;

	Draw2D_fillRegion(pObj->draw2DHndl,&region);

	/* Side View Region */
	region.color  = DRAW2D_TRANSPARENT_COLOR;
	region.startX = 552;
	region.startY = 16;
	region.height = 712;
	region.width  = 508;

	Draw2D_fillRegion(pObj->draw2DHndl,&region);

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

	fontPrm.fontIdx = 5;
	Draw2D_drawString(pObj->draw2DHndl, 554, 524,  "Warning! Please check around the vehicle directly", &fontPrm );

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_AVME500_FullView( GrpxSrcLink_Obj *pObj )
{
	Draw2D_RegionPrm region;
	Draw2D_FontPrm fontPrm;

	/* Full View Region */
	region.color  = DRAW2D_TRANSPARENT_COLOR;
	region.startX = 16;
	region.startY = 16;
	region.height = 1248;
	region.width  = 558;

	Draw2D_fillRegion(pObj->draw2DHndl,&region);

	fontPrm.fontIdx = 5;
	Draw2D_drawString(pObj->draw2DHndl, 16, 568,  "Warning! Please check around", &fontPrm );
	Draw2D_drawString(pObj->draw2DHndl, 50, 592,  "the vehicle directly", &fontPrm );

	return SYSTEM_LINK_STATUS_SOK;
}

Int32 GrpxSrcLink_drawAVM_E500Layout(GrpxSrcLink_Obj *pObj) // 이 함수를 TOP / FULL 그리는 함수를 다시 나눈다.
{
    //Draw2D_RegionPrm region;
    //Draw2D_FontPrm fontPrm;
    //Draw2D_BmpPrm bmpPrm;

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

void GrpxSrcLink_putCmd( uint8_t _cmd )
{
	/*
	 * case 0x0F : //Front - IRDA_KEY_UP = (0x0F)
		case 0x0E : //Rear - IRDA_KEY_DOWN = (0x0E)
		case 0x0B : //LFET - IRDA_KEY_LEFT = (0x0B)
		case 0x0A : //RIGHT - IRDA_KEY_RIGHT = (0x0A)
		case 0x05 : //Full - IRDA_KEY_FULL = (0x05),
	 */
#if 1
	GrpxSrcLink_CreateParams viewmodeprm;
	Int32 status;

	if( _cmd == CMD_REQ_FRONT_VIEW ){
		Vps_printf("In GrpxSrcLink_putCmd, CMD_REQ_FRONT_VIEW");

		viewmodeprm.sViewmode.viewmode =  TOP_VIEW;
		viewmodeprm.sViewmode.viewnt = FRONT_VIEW;
		_cmd = SYSTEM_CMD_FRONT_SIDE_VIEW;
	} else if( _cmd == CMD_REQ_REAR_VIEW ){
		Vps_printf("In GrpxSrcLink_putCmd, CMD_REQ_REAR_VIEW");

		viewmodeprm.sViewmode.viewmode =  TOP_VIEW;
		viewmodeprm.sViewmode.viewnt = REAR_VIEW;
		_cmd = SYSTEM_CMD_REAR_SIDE_VIEW;
	} else if( _cmd == CMD_REQ_RIGHT_VIEW ){
		Vps_printf("In GrpxSrcLink_putCmd, CMD_REQ_RIGHT_VIEW");

		viewmodeprm.sViewmode.viewmode =  TOP_VIEW;
		viewmodeprm.sViewmode.viewnt = RIGHT_VIEW;
		_cmd = SYSTEM_CMD_RIGH_SIDE_VIEW;
	} else if( _cmd == CMD_REQ_LEFT_VIEW ){
		Vps_printf("In GrpxSrcLink_putCmd, CMD_REQ_LEFT_VIEW");

		viewmodeprm.sViewmode.viewmode =  TOP_VIEW;
		viewmodeprm.sViewmode.viewnt = LEFT_VIEW;
		_cmd = SYSTEM_CMD_LEFT_SIDE_VIEW;
	}else if( _cmd == CMD_REQ_FULL_FRONT_VIEW ){
		Vps_printf("In GrpxSrcLink_putCmd, CMD_REQ_FULL_FRONT_VIEW");

		viewmodeprm.sViewmode.viewmode =  FULL_VIEW;
		viewmodeprm.sViewmode.viewnt = FRONT_VIEW;
		_cmd = SYSTEM_CMD_FULL_FRONT_VIEW;
	} else if( _cmd == CMD_REQ_FULL_REAR_VIEW ){
		Vps_printf("In GrpxSrcLink_putCmd, CMD_REQ_FULL_REAR_VIEW");

		viewmodeprm.sViewmode.viewmode =  FULL_VIEW;
		viewmodeprm.sViewmode.viewnt = REAR_VIEW;
		_cmd = SYSTEM_CMD_FULL_REAR_VIEW;
	}

	status = System_linkControl(gGrpxSrcLinkID, _cmd, &viewmodeprm, sizeof(viewmodeprm), TRUE); //gGrpxSrcLinkID 객체가 두개.

	Vps_printf("   CMD Send %s gGrpxSrcLinkID\n", ( status == 0x0)?"Success":"Fail");


#endif
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
