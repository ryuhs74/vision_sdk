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
 * \ingroup  ALGORITHM_LINK_PLUGIN
 * \defgroup ALGORITHM_LINK_ULTRASONICFUSION_API Algorithm Plugin: Ultrasonic Fusion API
 *
 * \brief  This module has the interface for using ultrasonic Fusion algorithm
 *
 *         Ultrasonic Fusion algorithm link -
 *           -# Will take in images from multiple views and generate
 *              LUTs (Fusion params) for Geometric alignment of views,
 *              which will get used by the synthesis stage.
 *           -# Will generate two types of LUTs:
 *               - Single LUT for Simple synthesis operation
 *               - Dual LUTs for blending type of synthesis
 *           -# Will always generate all the above 3 LUTs independent of
 *              the mode of operation of synthesis link
 *           -# Will call the algorithm process function once in K frames.
 *              For the remaining (K-1) frames, the link shall immediately
 *              release back the input video buffers.
 *
 * @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file algorithmLink_ultrasonicFusion.h
 *
 * \brief Algorithm Link API specific to ultrasonic Fusion algorithm
 *
 * \version 0.0 (Oct 2013) : [PS] First version
 *          0.1 (Dec 2013) : [PS] Added support for  Fusion support
 *
 *******************************************************************************
 */

#ifndef ALGORITHMLINK_ULTRASONICFUSION_H_
#define ALGORITHMLINK_ULTRASONICFUSION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <include/link_api/algorithmLink.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \ingroup LINK_API_CMD
 *   \addtogroup ALGORITHM_LINK_ULTRASONIC_FUSION_API_CMD Algorithm Plugin: Ultrasonic Fusion API Control Commands
 *
 *   @{
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \brief Returns Ultrasonic algorithm results
 *
 * \param AlgorithmLink_UltrasonicFusionGetResults * [OUT]
 *
 *******************************************************************************
 */
#define ALGLINK_ULTRASONICFUSION_CONFIG_CMD_GET_RESULTS        (0x9001)

/* @} */

/*
 *******************************************************************************
 * \brief Maximum Ultrasonic sensors that can be connected
 *******************************************************************************
 */
#define ALGLINK_ULTRASONICFUSION_MAX_DEVICES            (16)

/*
 *******************************************************************************
 * \brief Number of points for arc overlay
 *******************************************************************************
 */
#define ALGLINK_ULTRASONICFUSION_NUM_ARC_POINTS (9)


/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */
/**
 *******************************************************************************
 * \brief Enum for the input Q IDs
 *
 *******************************************************************************
*/
typedef enum
{
    ALGLINK_ULTRASONICFUSION_IPQID_MULTISENSOR = 0x0,
    /**< QueueId for multisensor input */

    ALGLINK_ULTRASONICFUSION_IPQID_UCLUT,
    /**< QueueId for GA LUTs */

    ALGLINK_ULTRASONICFUSION_IPQID_MAXIPQ,
    /**< Maximum number of input queues */

    ALGLINK_ULTRASONICFUSION_IPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}AlgorithmLink_UltrasonicFusionInputQueId;

/**
 *******************************************************************************
 * \brief Enum for the input Q IDs
 *
 *******************************************************************************
*/
typedef enum
{
    ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA = 0x0,
    /**< Overlay data (zones) */

    ALGLINK_ULTRASONICFUSION_OPQID_MAXOPQ,
    /**< Maximum number of output queues */

    ALGLINK_ULTRASONICFUSION_OPQID_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */

}AlgorithmLink_UltrasonicFusionOutputQueId;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure to represent a point
 *
 *******************************************************************************
*/
typedef struct {
	Int32 x;
    /**< Point position-X */
	Int32 y;
    /**< Point position-Y */
} AlgorithmLink_UltrasonicFusionPoint;

/**
 *******************************************************************************
 *
 *   \brief Ultrasonic sensor results for a given device
 *
 *******************************************************************************
*/
typedef struct {
	UInt8 drawDetectionArc;
    /**< draw this detection arc */
	UInt8 showSensorPosition;
    /**< show sensor position on SV output? */
	UInt16 numArcPoints;
    /**< Num of points in the Arc that is drawn */
	AlgorithmLink_UltrasonicFusionPoint detectionPoints[ALGLINK_ULTRASONICFUSION_NUM_ARC_POINTS];
    /**< points along detection arc */
	AlgorithmLink_UltrasonicFusionPoint sensorPosition;
    /**< Position of sensor */
	AlgorithmLink_UltrasonicFusionPoint strutPoints[2];
    /**< radial line from sensor position along cone (only shown if showSensorPosition>0) */
	UInt32 lineColor;
    /**< color of lines */
	UInt32 lineSize;
    /**< size of lines */
} AlgorithmLink_UltrasonicFusionDeviceResults;

/**
 *******************************************************************************
 *
 *   \brief Ultrasonic sensor results
 *
 *******************************************************************************
*/
typedef struct {
	UInt32 numDevice;
    /**< number of sensors */

	AlgorithmLink_UltrasonicFusionDeviceResults deviceStatus[ALGLINK_ULTRASONICFUSION_MAX_DEVICES];
    /**< Sensor results */

} AlgorithmLink_UltrasonicFusionResults;

/**
 *******************************************************************************
 *
 *   \brief Parameters passed into command
 *          ALGLINK_ULTRASONICFUSION_CONFIG_CMD_GET_RESULTS
 *
 *******************************************************************************
*/
typedef struct {

    AlgorithmLink_ControlParams baseClassControl;
    /**< Base control parameters */

    AlgorithmLink_UltrasonicFusionResults results;
    /**< Ultrasonic results */

} AlgorithmLink_UltrasonicFusionGetResults;

/**
 *******************************************************************************
 *
 *   \brief Structure containing create time parameters for Ultrasonic Fusion
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_CreateParams baseClassCreate;
    /**< Base class create params. This structure should be first element */
    UInt32                   numUltrasonic;
    /**< Max height of the output (stiched) frame */
    UInt32                   numViews;
    /**< number of input views from which output will be synthesized */
    UInt32                   numOutputTables;
    /**< Number of output GA LUT tables. All the 3 LUT tables are considered
     * as one unit. numOutputTables defines number of such units.
     */
    System_LinkInQueParams   inQueParams[ALGLINK_ULTRASONICFUSION_IPQID_MAXIPQ];
    /**< Input queue information */

    System_LinkOutQueParams   outQueParams[ALGLINK_ULTRASONICFUSION_OPQID_MAXOPQ];
    /**< Output queue information */

} AlgorithmLink_UltrasonicFusionCreateParams;

/**
 *******************************************************************************
 *
 *   \brief Structure containing control parameters for Ultrasonic Fusion
 *          algorithm
 *
 *******************************************************************************
*/
typedef struct
{
    AlgorithmLink_ControlParams baseClassControl;
    /**< Base class control params */
} AlgorithmLink_UltrasonicFusionControlParams;

/*******************************************************************************
 *  Functions
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of ultrasonic Fusion algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusion_initPlugin();

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* ALGORITHMLINK_ULTRASONICFUSION_H_ */

/* @} */
