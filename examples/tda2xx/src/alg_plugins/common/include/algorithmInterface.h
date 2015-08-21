/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 * TI CONFIDENTIAL - NDA RESTRICTIONS
 *
 *******************************************************************************
 */
/*
 *=======================================================================
 *
 * \file algorithmInterface.h
 *
 * \brief Interface file for memory requests and other interactions between 
 *        algorithm and framework
 *
 *        This interface file is a clean and quick way only for short term
 *        It is NOT product quality.
 *
 *        Usage Flow:
 *        Step1: Framework shall call function Algorithm_memQuery
 *        with create time params and pointer to struct AlgLink_MemRequests
 *        Step2: Algorithm shall implement function Algorithm_memQuery. 
 *        In this function, it shall populate all the necessary 
 *        memory requests based on the create time parameters. 
 *          NOTE: Memory needed for algorithm internal object also needs to be 
 *          requested. So it will be one of the element in memTab array. 
 *          Keep it as the first element memtab[0]
 *        Step3: Framework shall allocate the requested memory and populate
 *        pointer in memTab structure
 *        Step4: Framework shall call function Algorithm_create
 *        with create time params and pointer to struct AlgLink_MemRequests
 *        Step5: Now algorithm will have all the pointer values for all buffers 
 *
 * \version 0.0 (Oct 8 2013) : PS
 * \version 0.1 (Oct 9 2013) : BZ
 *=======================================================================*/

#ifndef _IMEMEREQUESTALGO_H_
#define _IMEMEREQUESTALGO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>

/*******************************************************************************
 *  Defines
 ******************************************************************************/
#define MAX_NUM_MEMORY_REQUESTS (16)
/**< Maximum number of memory requests */

#define ALGORITHM_PROCESS_OK (SYSTEM_LINK_STATUS_SOK)
/**< Return value for algorithm process being successful */

#define ALGORITHM_PROCESS_FAIL (SYSTEM_LINK_STATUS_EFAIL)
/**< Return value for algorithm process being failure */

/*******************************************************************************
 *  Enum's
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *  \brief  Enumerations for the various memory locations
 *
 *******************************************************************************
*/
typedef enum
{
    ALGORITHM_LINK_MEM_DSPL1D = 0,
    /**< DSP L1D */

    ALGORITHM_LINK_MEM_DSPL2,
    /**< DSP L2 */
    
    ALGORITHM_LINK_MEM_OCMC,
    /**< OCMC */

    ALGORITHM_LINK_MEM_DDR,
    /**< DDR */

    ALGORITHM_LINK_MEM_FORCE32BITS = 0xFFFFFFFF
    /**< This should be the last value after the max enumeration value.
     *   This is to make sure enum size defaults to 32 bits always regardless
     *   of compiler.
     */
} AlgorithmLink_MemoryLocation;

/*******************************************************************************
 *  Data structures
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 *   \brief Structure for one memory request
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 size;
    /**< Size of the memory buffer needed */
    UInt32 alignment;
    /**< Alignment (in bytes) needed for base address of the buffer */
    UInt32 persistentFlag;
    /**< 1 - Persitant memory request, 0 - Scratch memory request */
    AlgorithmLink_MemoryLocation memLocation;
    /**< To indication the location where memory is needed */
    void * basePtr;
    /**< Alignment needed for base address of the pointer */
} AlgLink_MemTab;

/**
 *******************************************************************************
 *
 *   \brief Structure of all memory requests to be exchanged between alg
 *          and framework
 *
 *******************************************************************************
*/
typedef struct
{
    UInt32 numMemTabs;
    /**< Number of memory requests done by the algorithm */
    AlgLink_MemTab memTab[MAX_NUM_MEMORY_REQUESTS];
    /**< Place holder for all the memory requests */
} AlgLink_MemRequests;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* Nothing beyond this point */
