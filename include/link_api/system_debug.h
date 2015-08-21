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
 *  \ingroup SYSTEM_LINK_API
 *  \defgroup SYSTEM_DEBUG_API System Debug Const's
 *
 *  Modify this file to enable/disable prints from different links.
 *
 *  When SYSTEM_DEBUG_xxx_RT  is enabled a print is done for every frame.
 *  In real-time systems such prints may slow down the system and hence
 *  are intended to used only for debug purposes.
 *
 *  @{
*/

/**
 *******************************************************************************
 *
 *  \file system_debug.h
 *  \brief System Debug Const's
 *
 *******************************************************************************
*/

#ifndef _SYSTEM_DEBUG_H_
#define _SYSTEM_DEBUG_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

/* @{ */

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of the system
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of the AVB link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_AVBTP

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of the capture link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_CAPTURE

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of VPE link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_VPE

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of ISS M2M link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_ISSM2M

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of display link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_DISPLAY

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of DUP link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_DUP

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of SW Mosaic link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_SWMS
/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of IPC link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_IPC

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of merge link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_MERGE

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug encoder link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_ENC

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of decoder link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_DEC

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of select link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_SELECT

/**
 *******************************************************************************
 *
 *   \brief Define to enable debug of Algorithm link
 *
 *******************************************************************************
*/
#define SYSTEM_DEBUG_ALGORITHM

/**
 *******************************************************************************
 *
 * \brief Enable real time debug of links
 *
 *******************************************************************************
 */
#define SYSTEM_RT_STATS_LOG_INTERVAL        (10)


/**
 *******************************************************************************
 *
 *  \brief Enable real time logs of links at various processing time instances
 *   like input dequeing, procesing, output queueing etc
 *
 *******************************************************************************
*/
#ifdef SYSTEM_DEBUG_RT
    #define SYSTEM_DEBUG_CAPTURE_RT
    #define SYSTEM_DEBUG_VPE_RT
    #define SYSTEM_DEBUG_DISPLAY_RT
    #define SYSTEM_DEBUG_SWMS_RT
    #define SYSTEM_DEBUG_IPC_RT
    #define SYSTEM_DEBUG_ENC_RT
    #define SYSTEM_DEBUG_DEC_RT
#endif

/*@}*/

#ifdef  __cplusplus
}
#endif

#endif /* _SYSTEM_DEBUG_H_ */

/* @} */

