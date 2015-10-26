/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <examples/tda2xx/src/draw2d/draw2d_priv.h>


Int32 Draw2D_getFontProperty(Draw2D_FontPrm *pPrm, Draw2D_FontProperty *pProp)
{
    if(pProp==NULL)
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

    /* default */
    Draw2D_getFontProperty00(pProp); /* default */

    if(pPrm!=NULL)
    {
        if(pPrm->fontIdx==0)
            Draw2D_getFontProperty00(pProp);
        else
        if(pPrm->fontIdx==1)
            Draw2D_getFontProperty01(pProp);
        else
        if(pPrm->fontIdx==2)
            Draw2D_getFontProperty02(pProp);
        else
        if(pPrm->fontIdx==3)
            Draw2D_getFontProperty03(pProp);
        else
        if(pPrm->fontIdx==4)
            Draw2D_getFontProperty04(pProp);
        else
        if(pPrm->fontIdx==5)
            Draw2D_getFontProperty05(pProp);

        else
        if(pPrm->fontIdx==6)
            Draw2D_getFontProperty06(pProp);

        else
        if(pPrm->fontIdx==7)
            Draw2D_getFontProperty07(pProp);

    }

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 Draw2D_getBmpProperty(Draw2D_BmpPrm *pPrm, Draw2D_BmpProperty *pProp)
{
    if(pProp==NULL)
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

    /* default */
    //Draw2D_getBmpProperty00(pProp); /* default */ //ryuhs74@20150909 - Del TI Logo

    if(pPrm!=NULL)
    {
    	/*
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_TI_LOGO)
            Draw2D_getBmpProperty00(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_TI_LOGO_1) //ryuhs74@20150909 - Del TI Logo
            Draw2D_getBmpProperty10(pProp);
        else
         */
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_SURROUND_VIEW)
            Draw2D_getBmpProperty01(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_FRONT_CAM_ANALYTICS)
            Draw2D_getBmpProperty02(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_OPTFLOW_LUT_0)
            Draw2D_getBmpProperty03(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_OPTFLOW_LUT_1)
            Draw2D_getBmpProperty04(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_JEEP_IMAGE)
            Draw2D_getBmpProperty06(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_STEREO_COLORBAR_20x720)
            Draw2D_getBmpProperty07(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_JEEP_220_X_330)
            Draw2D_getBmpProperty08(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_SURROUND_VIEW_SMALL)
            Draw2D_getBmpProperty09(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_TI_LOGO_SMALL)
            Draw2D_getBmpProperty12(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_STEREO_COLORBAR_35x450)
            Draw2D_getBmpProperty13(pProp);
        else
        if(pPrm->bmpIdx==DRAW2D_BMP_IDX_JEEP_IMAGE_TRUESCALE)
            Draw2D_getBmpProperty14(pProp);

    }

    return SYSTEM_LINK_STATUS_SOK;
}

UInt32 Draw2D_getFontCharAddr(Draw2D_FontProperty *font, char c)
{
    if(font==NULL)
        return NULL;

    if(c<' ' || c>'~')
        c = ' '; /* if out of bound draw 'space' char */

    c = c - ' ';

    return font->addr + c*font->width*font->bpp;
}

Int32 Draw2D_drawString_rot(Draw2D_Handle pCtx,
                        UInt32 startX,
                        UInt32 startY,
                        char *str,
                        Draw2D_FontPrm *pPrm,
                        UInt32 rotate)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Draw2D_Obj *pObj = (Draw2D_Obj *)pCtx;
    UInt32 len, width, height, fontAddr, h, i, w, px, py;
    UInt16 *fontAddr16, color;
    Draw2D_FontProperty font;

    if(pObj==NULL || str==NULL)
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

    Draw2D_getFontProperty(pPrm, &font);

    len = strlen(str);

    width = font.width*len;
    height = font.height;

    if(startX >= pObj->bufInfo.bufWidth)
        return 0;

    if(startY >= pObj->bufInfo.bufHeight)
        return 0;

    if(0 == rotate)
    {
        if((startX + width)> pObj->bufInfo.bufWidth)
        {
            width = pObj->bufInfo.bufWidth - startX;
        }

        if((startY + height)> pObj->bufInfo.bufHeight)
        {
            height = pObj->bufInfo.bufHeight - startY;
        }
    }
    else if(1 == rotate)
    {
        if((startX + height)> pObj->bufInfo.bufWidth)
        {
            height = pObj->bufInfo.bufWidth - startX;
        }

        if(startY < width/2)
        {
            width = startY*2;
        }
    }
    else if(2 == rotate)
    {
        if(startX < height)
        {
            height = startX;
        }

        if((startY + width/2) > pObj->bufInfo.bufHeight)
        {
            width = 2*(pObj->bufInfo.bufHeight - startY);
        }
    }


    len = width/font.width;

    /* draw 'len' char's from string 'str' */
    if(0 == rotate)
    {
        for(i=0; i<len; i++)
        {
            fontAddr = Draw2D_getFontCharAddr(&font, str[i]);
            px  = startX + i*font.width;
            py  = startY;

            /* draw font char */
            for(h=0; h<height; h++)
            {
                fontAddr16 = (UInt16*)fontAddr;
                for(w=0; w<font.width; w++)
                {
                    /* Assume color format is 2 bytes per pixel */
                    color = *fontAddr16;
                    Draw2D_drawPixel(
                        pCtx,
                        px+w,
                        py+h,
                        color,
                        font.colorFormat
                        );
                    fontAddr16++;
                }
                fontAddr += font.lineOffset;
            }
        }
    }
    else if(1 == rotate)
    {
        for(i=0; i<len; i++)
        {
            fontAddr = Draw2D_getFontCharAddr(&font, str[i]);
            px  = startX;
            py  = startY - i*font.width;

            /* draw font char */
            for(h=0; h<height; h++)
            {
                fontAddr16 = (UInt16*)fontAddr;
                for(w=0; w<font.width; w++)
                {
                    /* Assume color format is 2 bytes per pixel */
                    color = *fontAddr16;
                    Draw2D_drawPixel(
                        pCtx,
                        px+h,
                        py-w,
                        color,
                        font.colorFormat
                        );
                    fontAddr16++;
                }
                fontAddr += font.lineOffset;
            }
        }
    }
    else if(2 == rotate)
    {
        for(i=0; i<len; i++)
        {
            fontAddr = Draw2D_getFontCharAddr(&font, str[i]);
            px  = startX;
            py  = startY + i*font.width;

            /* draw font char */
            for(h=0; h<height; h++)
            {
                fontAddr16 = (UInt16*)fontAddr;
                for(w=0; w<font.width; w++)
                {
                    /* Assume color format is 2 bytes per pixel */
                    color = *fontAddr16;
                    Draw2D_drawPixel(
                        pCtx,
                        px-h,
                        py+w,
                        color,
                        font.colorFormat
                        );
                    fontAddr16++;
                }
                fontAddr += font.lineOffset;
            }
        }
    }

    return status;
}

Int32 Draw2D_drawString(Draw2D_Handle pCtx,
                        UInt32 startX,
                        UInt32 startY,
                        char *str,
                        Draw2D_FontPrm *pPrm)
{
    return Draw2D_drawString_rot(pCtx,
                          startX,
                          startY,
                          str,
                          pPrm,
                          0);
}

Int32 Draw2D_clearString(Draw2D_Handle pCtx,
                            UInt32 startX,
                            UInt32 startY,
                            UInt32 stringLength,
                            Draw2D_FontPrm *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    char tmpString[80];
    UInt32 len, clearLen;
    Draw2D_FontProperty font;

    if(pCtx==NULL)
        return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

    Draw2D_getFontProperty(pPrm, &font);

    len = sizeof(tmpString)-1;
    memset(tmpString, ' ', len);

    while(stringLength)
    {
        if(stringLength<len)
            clearLen = stringLength;
        else
            clearLen = len;

        tmpString[clearLen] = 0;
        Draw2D_drawString(pCtx, startX, startY, tmpString, pPrm);

        startX += clearLen*font.width;

        stringLength -= clearLen;
    }
    return status;
}

Int32 Draw2D_drawBmp(Draw2D_Handle pCtx,
                        UInt32 startX,
                        UInt32 startY,
                        Draw2D_BmpPrm *pPrm)
{
    return Draw2D_drawBmp_rot(pCtx,
                        startX,
                        startY,
                        pPrm,
                        0);
}

Int32 Draw2D_drawBmp_rot(Draw2D_Handle pCtx,
                        UInt32 startX,
                        UInt32 startY,
                        Draw2D_BmpPrm *pPrm,
                        UInt32 rotate)
{

    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Draw2D_Obj *pObj = (Draw2D_Obj *)pCtx;
    UInt32 width, height, bmpAddr, h, w;
    UInt16 color, *bmpAddr16;
    Draw2D_BmpProperty bmp;

#if 1 ///@todo Surrund View problem craven@151026
    if(pObj!=NULL)
        return SYSTEM_LINK_STATUS_SOK;
#else
    if(pObj==NULL)
    	return SYSTEM_LINK_STATUS_EINVALID_PARAMS;

#endif
    Draw2D_getBmpProperty(pPrm, &bmp);

    width = bmp.width;
    height = bmp.height;

    if (startX >= pObj->bufInfo.bufWidth)
        return 0;

    if (startY >= pObj->bufInfo.bufHeight)
        return 0;

    if (pObj->bufInfo.dataFormat != SYSTEM_DF_BGR16_565
         &&
        pObj->bufInfo.dataFormat != SYSTEM_DF_BGRA16_4444
        )
    {
        return 0;
    }

    if(0 == rotate)
    {
        if((startX + width)> pObj->bufInfo.bufWidth)
        {
            width = pObj->bufInfo.bufWidth - startX;
        }

        if((startY + height)> pObj->bufInfo.bufHeight)
        {
            height = pObj->bufInfo.bufHeight - startY;
        }
    }
    else if(1 == rotate)
    {
        if((startX + height)> pObj->bufInfo.bufWidth)
        {
            height = pObj->bufInfo.bufWidth - startX;
        }

        if(startY < width/2)
        {
            width = startY*2;
        }
    }
    else if(2 == rotate)
    {
        if(startX < height)
        {
            height = startX;
        }

        if((startY + width/2) > pObj->bufInfo.bufHeight)
        {
            width = 2*(pObj->bufInfo.bufHeight - startY);
        }
    }

    /* draw bitmap */
    bmpAddr = bmp.addr;

    if(0 == rotate)
    {
        /* draw bmp */
        for(h=0; h<height; h++)
        {
            bmpAddr16 = (UInt16*)bmpAddr;
            for(w=0; w<bmp.width; w++)
            {
                /* Assume color format is 2 bytes per pixel */
                color = *bmpAddr16;
                Draw2D_drawPixel(
                    pCtx,
                    startX+w,
                    startY+h,
                    color,
                    bmp.colorFormat
                    );
                bmpAddr16++;
            }
            bmpAddr += bmp.lineOffset;
        }
    }
    else if(1 == rotate)
    {
        /* draw bmp */
        for(h=0; h<height; h++)
        {
            bmpAddr16 = (UInt16*)bmpAddr;
            for(w=0; w<bmp.width; w++)
            {
                /* Assume color format is 2 bytes per pixel */
                color = *bmpAddr16;
                Draw2D_drawPixel(
                    pCtx,
                    startX+h,
                    startY-w,
                    color,
                    bmp.colorFormat
                    );
                bmpAddr16++;
            }
            bmpAddr += bmp.lineOffset;
        }
    }
    else if(2 == rotate)
    {
        /* draw bmp */
        for(h=0; h<height; h++)
        {
            bmpAddr16 = (UInt16*)bmpAddr;
            for(w=0; w<bmp.width; w++)
            {
                /* Assume color format is 2 bytes per pixel */
                color = *bmpAddr16;
                Draw2D_drawPixel(
                    pCtx,
                    startX-h,
                    startY+w,
                    color,
                    bmp.colorFormat
                    );
                bmpAddr16++;
            }
            bmpAddr += bmp.lineOffset;
        }
    }

    return status;
}
