/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "objectDrawLink_priv.h"


#define OBJECTDRAW_TRAFFIC_SIGN_WIDTH_1  (32)
#define OBJECTDRAW_TRAFFIC_SIGN_HEIGHT_1 (32)

#define OBJECTDRAW_TRAFFIC_SIGN_MAX    (43)

UInt8 gObjectDraw_trafficSigns_1[] = {
    #include "traffic_signs/00000_1.h"
    #include "traffic_signs/00001_1.h"
    #include "traffic_signs/00002_1.h"
    #include "traffic_signs/00003_1.h"
    #include "traffic_signs/00004_1.h"
    #include "traffic_signs/00005_1.h"
    #include "traffic_signs/00006_1.h"
    #include "traffic_signs/00007_1.h"
    #include "traffic_signs/00008_1.h"
    #include "traffic_signs/00009_1.h"
    #include "traffic_signs/00010_1.h"
    #include "traffic_signs/00011_1.h"
    #include "traffic_signs/00012_1.h"
    #include "traffic_signs/00013_1.h"
    #include "traffic_signs/00014_1.h"
    #include "traffic_signs/00015_1.h"
    #include "traffic_signs/00016_1.h"
    #include "traffic_signs/00017_1.h"
    #include "traffic_signs/00018_1.h"
    #include "traffic_signs/00019_1.h"
    #include "traffic_signs/00020_1.h"
    #include "traffic_signs/00021_1.h"
    #include "traffic_signs/00022_1.h"
    #include "traffic_signs/00023_1.h"
    #include "traffic_signs/00024_1.h"
    #include "traffic_signs/00025_1.h"
    #include "traffic_signs/00026_1.h"
    #include "traffic_signs/00027_1.h"
    #include "traffic_signs/00028_1.h"
    #include "traffic_signs/00029_1.h"
    #include "traffic_signs/00030_1.h"
    #include "traffic_signs/00031_1.h"
    #include "traffic_signs/00032_1.h"
    #include "traffic_signs/00033_1.h"
    #include "traffic_signs/00034_1.h"
    #include "traffic_signs/00035_1.h"
    #include "traffic_signs/00036_1.h"
    #include "traffic_signs/00037_1.h"
    #include "traffic_signs/00038_1.h"
    #include "traffic_signs/00039_1.h"
    #include "traffic_signs/00040_1.h"
    #include "traffic_signs/00041_1.h"
    #include "traffic_signs/00042_1.h"
};

Int32 AlgorithmLink_objectDrawCopyTrafficSign(
    UInt8 *bufAddrY,
    UInt8 *bufAddrC,
    UInt32 pitchY,
    UInt32 pitchC,
    UInt32 bufWidth,
    UInt32 bufHeight,
    UInt32 startX,
    UInt32 startY,
    UInt32 trafficSignId,
    UInt32 trafficSignType
    )
{
    UInt8 *pTrafficSign;
    UInt8 *pTrafficSignList;
    UInt32 copyWidth, copyHeight, i;
    UInt32 trafficSignSize;
    UInt32 trafficSignWidth, trafficSignHeight;

    if(trafficSignId>=OBJECTDRAW_TRAFFIC_SIGN_MAX)
        return -1;

    trafficSignWidth = OBJECTDRAW_TRAFFIC_SIGN_WIDTH_1;
    trafficSignHeight = OBJECTDRAW_TRAFFIC_SIGN_HEIGHT_1;
    pTrafficSignList = gObjectDraw_trafficSigns_1;

    trafficSignSize = trafficSignWidth*trafficSignHeight*3/2;

    /* align to multiple of 2, since data format is YUV420 */
    startX = SystemUtils_floor(startX, 2);
    startY = SystemUtils_floor(startY, 2);

    /* clip the copy area to limit within buffer area */
    copyWidth = trafficSignWidth;
    copyHeight = trafficSignHeight;

    if(startX > bufWidth
        ||
       startY > bufHeight
        )
    {
        /* Nothing to copy in this case */
        return 0;
    }

    if(startX+copyWidth > bufWidth)
    {
        copyWidth = bufWidth - startX;
    }

    if(startY+copyHeight > bufHeight)
    {
        copyHeight = bufHeight - startY;
    }

    /* adjust input buffer pointer to start location */
    bufAddrY = bufAddrY + pitchY*startY + startX;
    bufAddrC = bufAddrC + pitchC*startY/2 + startX;

    /* Copy Y */
    pTrafficSign = pTrafficSignList
                + trafficSignId*trafficSignSize;

    for(i=0; i<copyHeight; i++)
    {
        memcpy(bufAddrY, pTrafficSign, copyWidth);

        bufAddrY+=pitchY;
        pTrafficSign+=trafficSignWidth;
    }

    /* Copy C */
    pTrafficSign = pTrafficSignList
                + trafficSignId*trafficSignSize;

    pTrafficSign += trafficSignWidth*trafficSignHeight;

    for(i=0; i<copyHeight/2; i++)
    {
        memcpy(bufAddrC, pTrafficSign, copyWidth);

        bufAddrC+=pitchC;
        pTrafficSign+=trafficSignWidth;
    }

    return 0;
}












































