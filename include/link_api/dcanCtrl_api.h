 /*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \ingroup SAMPLE_MODULE_API
 *   \defgroup DCAN_CTRL_API DCAN Control API
 *
 *   This API is used to enable DCAN control in Vision SDK.
 *
 *   @{
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file dcanCtrl_api.h
 *
 * \brief DCAN Control API
 *
 *******************************************************************************
 */


#ifndef _DCAN_CTRL_API_H_
#define _DCAN_CTRL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Includes
 *******************************************************************************
 */
#include <include/link_api/system.h>

#define SYSTEM_DCAN_CTRL_DISPLAY_DURATION_MS       (20000)
#define SYSTEM_DCAN_CTRL_DISPLAY_STARTX            (20)
#define SYSTEM_DCAN_CTRL_DISPLAY_STARTY            (140)
#define SYSTEM_DCAN_CTRL_DISPLAY_FONTID            (5)

/*******************************************************************************
 *  Typedef's
 *******************************************************************************
 */


/*******************************************************************************
 *  Functions's
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief Create a thread to handle command recevied over network
 *
 *******************************************************************************
 */
Void System_dcanInit();

/*******************************************************************************
 *  \brief Delete the thread used for handling network commands
 *
 *******************************************************************************
 */
Void System_dcanDeInit();



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DCAN_CTRL_API_H_ */

/* @} */


