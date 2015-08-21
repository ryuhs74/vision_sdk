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
 * \ingroup DISPLAYCTRL_LINK_API
 * \defgroup DISPLAYCTRL_LINK_IMPL Display Controller Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file displayCtrlLink_priv.h
 *
 * \brief Display Controller Private Header File
 *
 *        This file has the structures, enums, function prototypes
 *        for display controller link, which are not exposed to the application
 *
 * \version 0.0 (Jun 2013) : [PS] First version
 * \version 0.1 (Jul 2013) : [PS] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _DISPLAYCTRL_LINK_PRIV_H_
#define _DISPLAYCTRL_LINK_PRIV_H_

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <include/link_api/displayCtrlLink.h>
#include <include/link_api/system_const.h>
#include <platforms/bsp_platform.h>


/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Display Controller Link Object
 *
 *          This is the object for dislay controller link.
 *
 *******************************************************************************
*/
typedef struct {
    /* DisplayCtrl link task */
    Utils_TskHndl tsk;

    UInt32 state;
    /**< Link state, one of SYSTEM_LINK_STATE_xxx */

    /* Global displayCtrl driver handle */
    FVID2_Handle fvidHandleDisplayCtrl;

} DisplayCtrlLink_Obj;

extern DisplayCtrlLink_Obj gDisplayCtrlLink_obj;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

Int32 DisplayCtrlLink_drvCreate(DisplayCtrlLink_Obj * pObj);

Int32 DisplayCtrlLink_drvDelete(DisplayCtrlLink_Obj * pObj);

Int32 DisplayCtrlLink_drvConfigInit(DisplayCtrlLink_VencInfo *pVencInfo,
                                   Vps_DctrlConfig *pDctrlCfg,
                                   Int32 vencCtr,
                                   Int32 *edgeCntr,
                                   UInt32 *usedpipeFlag);

Int32 DisplayCtrlLink_drvSetConfig(DisplayCtrlLink_Obj          *pObj,
                                   DisplayCtrlLink_ConfigParams *pConfigParams);

Int32 DisplayCtrlLink_drvClrConfig(DisplayCtrlLink_Obj          *pObj,
                                   DisplayCtrlLink_ConfigParams *pConfigParams);

Int32 DisplayCtrlLink_drvSetOvlyParams(DisplayCtrlLink_Obj        * pObj,
                                       DisplayCtrlLink_OvlyParams *pOvlyParams);

Int32 DisplayCtrlLink_drvSetOvlyPipelineParams(
          DisplayCtrlLink_Obj            *pObj,
          DisplayCtrlLink_OvlyPipeParams *pOvlyPipeParams);

Int32 DisplayCtrlLink_drvPrintStatus(DisplayCtrlLink_Obj * pObj);

#endif

/* @} */
