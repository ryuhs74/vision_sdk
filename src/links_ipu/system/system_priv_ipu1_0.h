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
 * \ingroup SYSTEM_IMPL
 *
 * \defgroup SYSTEM_IPU1_0_IMPL System implementation for IPU1 Core 0
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file system_priv_ipu1_0.h IPU1 private file containing all the header files
 *                            util files required by ipu
 *
 *
 * \version 0.0 (Jun 2013) : [CM] First version
 * \version 0.1 (Jul 2013) : [CM] Updates as per code review comments
 *
 *******************************************************************************
 */

#ifndef _SYSTEM_PRIV_IPU1_0_H_
#define _SYSTEM_PRIV_IPU1_0_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/captureLink.h>
#include <include/link_api/displayLink.h>
#include <include/link_api/displayCtrlLink.h>
#include <include/link_api/vpeLink.h>
#include <include/link_api/dupLink.h>
#include <include/link_api/gateLink.h>
#include <include/link_api/nullLink.h>
#include <include/link_api/selectLink.h>
#include <include/link_api/syncLink.h>
#include <include/link_api/mergeLink.h>
#include <include/link_api/ipcLink.h>
#include <include/link_api/algorithmLink.h>
#include <include/link_api/nullSrcLink.h>
#include <include/link_api/decLink.h>
#include <include/link_api/encLink.h>
#ifdef ISS_INCLUDE
#include <include/link_api/issCaptureLink.h>
#include <include/link_api/issM2mIspLink.h>
#include <include/link_api/issM2mSimcopLink.h>
#include <src/utils_common/include/utils_iss.h>
#endif
#include <include/link_api/systemLink_ipu1_0_params.h>
#include <src/links_common/system/system_priv_common.h>
#include <common/bsp_types.h>
#include <common/trace.h>
#include <common/bsp_config.h>
#include <common/bsp_utils.h>
#include <common/bsp_common.h>
#include <fvid2/fvid2.h>
#include <vps/vps_capture.h>
#include <devices/bsp_device.h>
#include <devices/bsp_videoDecoder.h>
#include <devices/bsp_videoSensor.h>
#include <boards/bsp_board.h>
#include <vps/vps.h>
#include <vps/vps_display.h>
#include <vps/vps_displayCtrl.h>
#include <vps/vps_cfgDss.h>
#include <vps/vps_displayDss.h>
#include <vps/vps_control.h>
#include <platforms/bsp_platform.h>
#include <i2c/bsp_i2c.h>
#include <src/utils_common/include/utils_stat_collector.h>
#include <include/link_api/avbRxLink.h>
#include <vps/vps_captureDssWb.h>


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Structure to hold the IPU1_0 specific data structures
 *
 *******************************************************************************
*/
typedef struct {

    UInt32 reserved;

} System_Ipu1_0_Obj;

extern System_Ipu1_0_Obj gSystem_objIpu1_0;

UInt32 System_getVpdmaPhysAddr();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_SYSTEM_PRIV_IPU1_0_H_*/

/* @} */
