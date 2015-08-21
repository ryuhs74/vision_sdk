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
 * \file dbc.h
 *
 * \brief  Design By Contract Definitions - Provides basic
 *         Design By Contract support, including asserts, preconditions,
 *         and postconditions. *
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _DBC_
#define _DBC_


/** @ingroup DSPDBC */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


/* DBC_ASSERTS is intended to be set to 1 in debug builds */
#if DBC_ASSERTS
#include <xdc/runtime/System.h>

/**< Assert a logical condition to be TRUE. */
#define DBC_assert(c) if (!(c)) { \
        System_printf("Assertion Failed: file=%s, line=%d.\n", __FILE__, \
        __LINE__); \
        System_abort("Aborting...");}

#define DBC_require DBC_assert  /**< Assert a precondition to be TRUE. */
#define DBC_ensure  DBC_assert  /**< Assert a postcondition to be TRUE. */

#else

#define DBC_assert(c)   /**< Assert a logical condition to be TRUE. */
#define DBC_require(c)  /**< Assert a precondition to be TRUE. */
#define DBC_ensure(c)   /**< Assert a postcondition to be TRUE. */

#endif


#ifdef __cplusplus
}
#endif /* extern "C" */


/*@}*/ /* ingroup DSPDBC */


#endif /* _DBC_ */


/* Nothing beyond this point */


