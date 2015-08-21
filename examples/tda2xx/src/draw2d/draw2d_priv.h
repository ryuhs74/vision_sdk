/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \defgroup DRAW_2D_API Font and 2D Drawing API
 *
 * \brief  This module has the interface for drawing fonts and 2D primitives
 *         like lines
 *
 *         NOTE: This is limited demo API and not a comprehensive 2D drawing
 *               library
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file draw2d.h
 *
 * \brief Font and 2D Drawing API
 *
 * \version 0.0 (Oct 2013) : [KC] First version
 *
 *******************************************************************************
 */

#ifndef _DRAW_2D_PRIV_H_
#define _DRAW_2D_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <examples/tda2xx/include/draw2d.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

typedef struct {

    Draw2D_BufInfo bufInfo;

} Draw2D_Obj;


Int32 Draw2D_getFontProperty00(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty01(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty02(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty03(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty04(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty05(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty06(Draw2D_FontProperty *pProp);
Int32 Draw2D_getFontProperty07(Draw2D_FontProperty *pProp);
Int32 Draw2D_getBmpProperty00(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty01(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty02(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty03(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty04(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty06(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty07(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty08(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty09(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty10(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty11(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty12(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty13(Draw2D_BmpProperty *pProp);
Int32 Draw2D_getBmpProperty14(Draw2D_BmpProperty *pProp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* _DRAW_2D_H_ */

/* @} */

/* Nothing beyond this point */

