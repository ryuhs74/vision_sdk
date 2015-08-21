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
 * \file _alg.h
 *
 * \brief  Alg alloc memory and dealloc memory functions are defined
 *
 * \version 0.0 (Jan 2014) : [SS] First version
 *
 *******************************************************************************
 */

#ifndef _ALG_
#define _ALG_

/*
 *  ======== _ALG_allocMemory ========
 */
extern Bool _ALG_allocMemory(IALG_MemRec *memTab, Int n);

/*
 *  ======== _ALG_freeMemory ========
 */
extern Void _ALG_freeMemory(IALG_MemRec *memTab, Int n);

#endif

/* Nothing beyond this point */

