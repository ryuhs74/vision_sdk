/**
 *                                                                            
 * Copyright (c) 2015 CAMMSYS - http://www.cammsys.net/                       
 *                                                                            
 * All rights reserved.                                                       
 *                                                                            
 * @file	uartCmd_priv.h
 * @author  Raven
 * @date	Sep 21, 2015
 * @brief	                                                                     
 */
#ifndef EXAMPLES_TDA2XX_SRC_LINKS_UARTCMD_UARTCMD_PRIV_H_
#define EXAMPLES_TDA2XX_SRC_LINKS_UARTCMD_UARTCMD_PRIV_H_

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */


/*******************************************************************************
 *  Defines
 *******************************************************************************
 */

#define UARTCMD_LINK_TSK_STACK_SIZE        (SYSTEM_DEFAULT_TSK_STACK_SIZE)
#define UARTCMD_LINK_TSK_PRI               (12)

 /**
 *******************************************************************************
 *
 * \brief Maximum number of UARTCMD link objects
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define UARTCMD_LINK_OBJ_MAX    (1)

 /**
 *******************************************************************************
 *
 * \brief Maximum value of UARTCMD factor
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define UARTCMD_LINK_MAX_NUM_UARTCMDS    (3)

/**
 *******************************************************************************
 *
 * \brief Maximum frmaes an output queue can support
 *
 * SUPPORTED in ALL platforms
 *
 *******************************************************************************
 */
#define UARTCMD_LINK_MAX_FRAMES_PER_OUT_QUE    \
                           (SYSTEM_LINK_FRAMES_PER_CH*SYSTEM_MAX_CH_PER_OUT_QUE)

/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */



/**
 *******************************************************************************
 *
 * \brief Structure to hold all UARTCMD link related information
 *
 *******************************************************************************
 */
typedef struct {
    UInt32 tskId;
    /**< Placeholder to store UARTCMD link task id */
} UARTCMDLink_Obj;

extern UARTCMDLink_Obj gUARTCMDLink_obj[];

Void UARTCMDLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg);

Int32 UARTCMDLink_tskCreate(UInt32 instId);


#endif /* EXAMPLES_TDA2XX_SRC_LINKS_UARTCMD_UARTCMD_PRIV_H_ */
