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
 *   \ingroup SAMPLE_MODULE_API
 *   \defgroup ULTRASONIC_CAPTURE_LINK_API UltrasonicCapture Link API
 *
 *             Capture data from ultrasonic sensors connected via UART
 *             Sends measurement info to next link
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file ultrasonicCaptureLink.h
 *
 * \brief UltrasonicCapture link API public header file.
 *
 * \version 0.0 (Jul 2013) : [NN] First version
 *
 *******************************************************************************
 */

#ifndef _ULTRASONIC_CAPTURE_LINK_H_
#define _ULTRASONIC_CAPTURE_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

 /* @{ */

/**
 *******************************************************************************
 * \brief Maximum number of possible devices that can be connected
 *******************************************************************************
 */
#define ULTRASONIC_CAPTURE_MAX_DEVICES                  (16)

/**
 *******************************************************************************
 * \brief Measurement mode is short distance
 *******************************************************************************
 */
#define ULTRASONIC_CAPTURE_MEASUREMENT_MODE_SHORT       (1)

/**
 *******************************************************************************
 * \brief Measurement mode is long distance
 *******************************************************************************
 */
#define ULTRASONIC_CAPTURE_MEASUREMENT_MODE_LONG        (2)

/* @} */

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
*/

/**
 *******************************************************************************
 * \brief Device measurement info
 *******************************************************************************
 */
typedef struct {

    UInt32 deviceId;
    /**< ID of device for which this measurement is done */

    UInt32 distanceLong;
    /**< Detected object distance in cm */

    UInt32 distanceShort;
    /**< Detected object distance in cm */

} UltrasonicCapture_DeviceMeasurementInfo;

/**
 *******************************************************************************
 * \brief Structure of data included in the buffer that is passed to next link
 *******************************************************************************
 */
typedef struct
{
    UInt32 numSensors;
    /**< Number of sensors for which data is measured */

    UltrasonicCapture_DeviceMeasurementInfo deviceInfo
        [ULTRASONIC_CAPTURE_MAX_DEVICES];
    /**< Measurement information of individual device's */

} UltrasonicCapture_MeasurementInfo;



/**
 *******************************************************************************
 * \brief Ultrasonic capture link configuration parameters.
 *******************************************************************************
*/
typedef struct
{
    UInt32  uartInstId;
    /**< 0..9, UART instance to use for ultrasonic sensor   */

    System_LinkOutQueParams   outQueParams;
    /**< output queue information */

} UltrasonicCaptureLink_CreateParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

 /**
 *******************************************************************************
 *
 * \brief Init function for Ultrasonic capture link.
 *
 *        This function does the following
 *          - Creates a task for the link
 *          - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 UltrasonicCaptureLink_init();

/**
 *******************************************************************************
 *
 * \brief De-init function for Ultrasonic capture link.
 *
 *        This function de-registers this link from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 UltrasonicCaptureLink_deInit();

/**
 *******************************************************************************
 *
 * \brief UltrasonicCapture link set default parameters for create time params
 *
 * \param  pPrm  [OUT]  UltrasonicCaptureLink Create time Params
 *
 *******************************************************************************
 */
static inline void UltrasonicCaptureLink_CreateParams_Init(UltrasonicCaptureLink_CreateParams *pPrm)
{
    memset(pPrm, 0, sizeof(*pPrm));

    /* value of 9 corresponds to UART instance 10 as specified in TRM */
    pPrm->uartInstId = 9;

    return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


/*@}*/

