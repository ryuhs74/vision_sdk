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
 * \ingroup ULTRASONIC_CAPTURE_LINK_API
 * \defgroup ULTRASONIC_CAPTURE_LINK_IMPL UltrasonicCapture Link Implementation
 *
 * @{
 */

/**
 *******************************************************************************
 *
 * \file ultrasonicCaptureLink_priv.h UltrasonicCapture Link private API/Data structures
 *
 *******************************************************************************
 */

#ifndef _ULTRASONIC_CAPTURE_LINK_PRIV_H_
#define _ULTRASONIC_CAPTURE_LINK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */
#include <include/link_api/ultrasonicCaptureLink.h>
#include <src/links_ipu/system/system_priv_ipu1_0.h>
#include <devices/bsp_pga450.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Maximum frmaes an output queue can support
 *
 *******************************************************************************
 */
#define ULTRASONIC_CAPTURE_LINK_MAX_OUT_BUFFERS     (1)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */


/**
 *******************************************************************************
 *
 * \brief Structure to hold all UltrasonicCapture link related information
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 tskId;
    /**< Placeholder to store ultrasonicCapture link task id */

    Utils_TskHndl tsk;
    /**< Handle to capture link task */

    UltrasonicCaptureLink_CreateParams createArgs;
    /**< Create params for ultrasonicCapture link */

    System_LinkInfo info;
    /**< Output queue information of this link */

    Bool deviceIsDetected[BSP_PGA450_MAX_DEVICE+1];
    /**< TRUE: Device is detected during create,
     *   FALSE: Device is not detected during create
     */

    Utils_BufHndl outFrameQue;
    /**< Handles to each of the output queues */

    System_Buffer
                sysBufs[ULTRASONIC_CAPTURE_LINK_MAX_OUT_BUFFERS];
    /**< Placeholder to store the incoming buffers */

    System_MetaDataBuffer
                metaBufs[ULTRASONIC_CAPTURE_LINK_MAX_OUT_BUFFERS];

} UltrasonicCaptureLink_Obj;


void UltrasonicCaptureLink_hwSetup();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


