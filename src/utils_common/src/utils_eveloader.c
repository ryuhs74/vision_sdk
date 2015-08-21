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
 *  \file utils_eveloader.c
 *
 *  \brief This file has implementation for eve loader. This is mainly used
 *         when linux is running on A15. It uses SBL from starterware to
 *         boot load eves
 *
 *
 *  \version 0.0 (Aug 2014) : [YM] First version
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
*/
#include <src/utils_common/include/utils.h>
#include <src/utils_common/include/utils_eveloader.h>

UInt32 entryPoint_EVE1 = 0;
UInt32 entryPoint_EVE2 = 0;
UInt32 entryPoint_EVE3 = 0;
UInt32 entryPoint_EVE4 = 0;


/*
 * \brief       ImageCopy function is a wrapper to Multicore Image parser
 *              function. Based on boot-mode jump into specific
 *              function
 *
 * \param   none
 *
 * \return   error status.If error has occured it returns a non zero value.
 *             If no error has occured then return status will be zero. .
 *
 */
Int32 Utils_eveImageCopy(void)
{
    Int32 retval = 0;

    if (DDR3BootRprc() != SYSTEM_LINK_STATUS_SOK)
    {
        retval = -1;
    }

    return retval;
}


/**
 *******************************************************************************
 *
 * \brief Boots Eves with AppImage
 *
 *
 * \return SYSTEM_LINK_STATUS_SOK on success, else failure
 *
 *******************************************************************************
 */
Int32 Utils_eveBoot()
{

    Int32 retVal = SYSTEM_LINK_STATUS_SOK;

    /*Initialized the DPLL*/
    retVal = configure_dpll();

    /*slavecore prcm init & system reset*/
    slavecore_prcm_enable();

    /*Copy the Multicore Image file & entry point*/
    if (Utils_eveImageCopy() != 0)
    {
        Vps_printf("\n Utils_eveImageCopy() failed !!\n");
    }

#ifdef PROC_EVE1_INCLUDE
    EVE1_BringUp(entryPoint_EVE1);
#endif
#ifdef PROC_EVE2_INCLUDE
    EVE2_BringUp(entryPoint_EVE2);
#endif
#ifdef PROC_EVE3_INCLUDE
    EVE3_BringUp(entryPoint_EVE3);
#endif
#ifdef PROC_EVE4_INCLUDE
    EVE4_BringUp(entryPoint_EVE4);
#endif

    return retVal;
}
