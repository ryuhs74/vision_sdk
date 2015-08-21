/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include <include/link_api/system.h>
#include <src/utils_common/include/utils_qspi.h>
#include <src/utils_common/include/utils_mem.h>
#include <examples/tda2xx/include/chains_main_srv_calibration.h>
#include <ti/sysbios/knl/Task.h>

/*
 *******************************************************************************
 *
 * Surround view calibration task parameters
 *
 *******************************************************************************
 */

#define CHAIN_COMMON_SRV_PERS_MAT_TSK_STACK_SIZE              (32*1024)
#define CHAIN_COMMON_SRV_PERS_MAT_TSK_PRI                     (4)
#define CHAIN_COMMON_SRV_QSPI_FLASH_WRITE_TASK_SLEEP_DURATION (1000)


/**
 *******************************************************************************
 *
 *  \brief  Data structure for the GA calibration
 *          The GA output LUT and perspestive matrix are generated while
 *          calibration ON and these tables are stored in QSPI flash.
 *          Successive run read these tables and input the same to SV GA
 *          Algorithm.
 *
 *******************************************************************************
 */
typedef struct {
    Chain_Common_SRV_GACalibrationType GACalibrationType;
    Chain_Common_SRV_GACalibrationType GACalibrationTypePrev;
    /**< This is a placeholder to remember the previous state
     *   of the SV alg GA calibration type.
     */
    char * gaLUTDDRPtr;
    char * persMatDDRPtr;
    AlgorithmLink_GAlignCalibrationMode calMode;
    Bool   TblWriteTaskExit;
    BspOsal_TaskHandle TblWriteTask;
    /**< Handle to task handing PersMat TBL write to QSPI flash */
} Chain_Common_SRV_GACalibrationInfo;

/**
 *******************************************************************************
 * \brief stack for PersMat table write task
 *******************************************************************************
 */
#pragma DATA_ALIGN(Chain_Common_SRV_PersMatTblWriteTskStack, 32)
#pragma DATA_SECTION(Chain_Common_SRV_PersMatTblWriteTskStack, ".bss:taskStackSection")
UInt8 Chain_Common_SRV_PersMatTblWriteTskStack[CHAIN_COMMON_SRV_PERS_MAT_TSK_STACK_SIZE];


Chain_Common_SRV_GACalibrationInfo gChainCommon_SRVCalibInfo;

/**
 *******************************************************************************
 *
 * \brief   Task to write the GA persMat table to the QSPI flash
 *
 * \param   arg1    [IN]  Chain_Common_SRV_GACalibrationInfo
 * \param   arg2    [IN]  Chain_Common_SRV_GACalibrationInfo
 *
 *******************************************************************************
*/
static Void Chain_Common_SRV_PersMatTblWriteFun(UArg arg1, UArg arg2)
{

    Bool isPersMatTblUpdated;
    UInt32 persMatTblUpdateAfter1Min;
    UInt32 * ptr;
    Chain_Common_SRV_GACalibrationInfo *gaCalibInfo;

    gaCalibInfo = (Chain_Common_SRV_GACalibrationInfo *) arg1;
    isPersMatTblUpdated = FALSE;
    persMatTblUpdateAfter1Min = 0;
    gaCalibInfo->TblWriteTaskExit = FALSE;

    while (!isPersMatTblUpdated && !gaCalibInfo->TblWriteTaskExit)
    {
        Task_sleep(CHAIN_COMMON_SRV_QSPI_FLASH_WRITE_TASK_SLEEP_DURATION);

        /* Both persMat & GA LUT are written back to QSPI during demo
         * STOP.  But if the demo ended due to battery drained condition
         * both these tables will not get updated in QSPI flash.  To
         * avoid this scenario, only persMat tbl alone is updated
         * after 5 minutes from the start of the demo.
         */
        persMatTblUpdateAfter1Min++;
        if ((persMatTblUpdateAfter1Min >= 60) && (!isPersMatTblUpdated ) && FALSE)
        {
            isPersMatTblUpdated = TRUE;
            if (gaCalibInfo->GACalibrationType != CHAIN_COMMON_SRV_GA_CALIBRATION_NO)
            {
                ptr = (UInt32 *) gaCalibInfo->persMatDDRPtr;
                *ptr = GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE;
                System_qspiWriteSector(GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                      (UInt32) gaCalibInfo->persMatDDRPtr,
                                      GA_PERSPECTIVE_MATRIX_SIZE);
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * \brief   Function to allocate intermediate GA OUTPUT LUT and
 *          Perspective Matrix tables.  QSPI flash read/write
 *          is from/to this memory.
 *          Also create as task to update the peraMat table into
 *          QSPI flash after 5 min
 *
 * \param   gaCalibInfo    [IN]  Chain_Common_SRV_GACalibrationInfo
 *
 *******************************************************************************
*/
static Void Chain_Common_SRV_allocateGATbl(
            Chain_Common_SRV_GACalibrationInfo *gaCalibInfo)
{
    Int32 status;
    UInt32 * ptr;

    gaCalibInfo->gaLUTDDRPtr = Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_SR,
                                              GA_OUTPUT_LUT_SIZE_MEM_ALLOC, 32);
    UTILS_assert(gaCalibInfo->gaLUTDDRPtr != NULL);

    gaCalibInfo->persMatDDRPtr = Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_SR,
                                                GA_PERSPECTIVE_MATRIX_SIZE, 32);
    UTILS_assert(gaCalibInfo->persMatDDRPtr != NULL);

    gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_USERGALUT;

    /* Read the PersMat tbl always from QSPI so that it can be
     * write back unconditionally to QSPI Flash */
    System_qspiReadSector((UInt32) gaCalibInfo->persMatDDRPtr,
                          GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                          GA_PERSPECTIVE_MATRIX_SIZE);

    switch (gaCalibInfo->GACalibrationType)
    {
        case CHAIN_COMMON_SRV_GA_CALIBRATION_NO:
            System_qspiReadSector((UInt32) gaCalibInfo->gaLUTDDRPtr,
                                  GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                                  GA_MAGIC_PATTERN_SIZE_IN_BYTES);
            ptr = (UInt32 *) gaCalibInfo->gaLUTDDRPtr;
            if (*ptr == GA_OUTPUT_LUT_MAGIC_SEQUENCE)
            {
                gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_USERGALUT;
                System_qspiReadSector((UInt32) gaCalibInfo->gaLUTDDRPtr,
                                      GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                                      GA_OUTPUT_LUT_SIZE);
                //TBD - cache write back
            }
            else
            {
                gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_DEFAULT;
            }
            break;
        case CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE:
            System_qspiReadSector((UInt32) gaCalibInfo->persMatDDRPtr,
                                  GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                  GA_MAGIC_PATTERN_SIZE_IN_BYTES);
            ptr = (UInt32 *) gaCalibInfo->persMatDDRPtr;
            if (*ptr == GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE)
            {
                gaCalibInfo->calMode =
                                ALGLINK_GALIGN_CALMODE_FORCE_USERPERSMATRIX;
                System_qspiReadSector((UInt32) gaCalibInfo->persMatDDRPtr,
                                      GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                      GA_PERSPECTIVE_MATRIX_SIZE);
                //TBD - cache write back
            }
            else
            {
                gaCalibInfo->calMode =
                                ALGLINK_GALIGN_CALMODE_FORCE_DEFAULTPERSMATRIX;
            }
            break;
        default:
            UTILS_assert(0);
            break;
    }

#if 1
    gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_FORCE_DEFAULTPERSMATRIX;
#endif

    /*
     * Create a task to update the peraMat table into QSPI flash after 5 min
     */
    gaCalibInfo->TblWriteTask = BspOsal_taskCreate(
                                    (BspOsal_TaskFuncPtr)Chain_Common_SRV_PersMatTblWriteFun,
                                    CHAIN_COMMON_SRV_PERS_MAT_TSK_PRI,
                                    Chain_Common_SRV_PersMatTblWriteTskStack,
                                    sizeof(Chain_Common_SRV_PersMatTblWriteTskStack),
                                    gaCalibInfo);
    UTILS_assert(gaCalibInfo->TblWriteTask != NULL);

    status = Utils_prfLoadRegister(gaCalibInfo->TblWriteTask,
                                   "SRV_TBL_WRITE_TSK");
    UTILS_assert(status==SYSTEM_LINK_STATUS_SOK);
}

/**
 *******************************************************************************
 *
 * \brief   Function to de-allocate intermediate GA OUTPUT LUT and
 *          Perspective Matrix tables.
 *          - Also perform QSPI flash write after the succesful calibration
 *          - Delete the task to update the peraMat table into QSPI flash
 *
 * \param   gaCalibInfo    [IN]  Chain_Common_SRV_GACalibrationInfo
 *
 *******************************************************************************
*/
static Void Chain_Common_SRV_freeGATbl(
            Chain_Common_SRV_GACalibrationInfo *gaCalibInfo)
{
    UInt32 status;
    UInt32 * ptr;

    gaCalibInfo->TblWriteTaskExit = TRUE;

    if (gaCalibInfo->GACalibrationTypePrev == CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE)
    {
        Vps_printf("Started Storing Calibration tables into QSPI flash, please Wait \n");

        ptr = (UInt32 *) gaCalibInfo->persMatDDRPtr;
        *ptr = GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE;
        System_qspiWriteSector(GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                              (UInt32) gaCalibInfo->persMatDDRPtr,
                              GA_PERSPECTIVE_MATRIX_SIZE);

        ptr = (UInt32 *) gaCalibInfo->gaLUTDDRPtr;
        *ptr = GA_OUTPUT_LUT_MAGIC_SEQUENCE;
        System_qspiWriteSector(GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                              (UInt32) gaCalibInfo->gaLUTDDRPtr,
                              GA_OUTPUT_LUT_SIZE);
        gaCalibInfo->GACalibrationTypePrev = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;

        Vps_printf("Storing tables into flash completed. \n");
    }

    if (gaCalibInfo->GACalibrationType == CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE)
    {
        gaCalibInfo->GACalibrationTypePrev = CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE;
    }

    status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                           gaCalibInfo->gaLUTDDRPtr, GA_OUTPUT_LUT_SIZE_MEM_ALLOC);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                           gaCalibInfo->persMatDDRPtr, GA_PERSPECTIVE_MATRIX_SIZE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Vps_printf(" SRV: QSPI write Task Delete in progress !!!\n");

    Utils_prfLoadUnRegister(gaCalibInfo->TblWriteTask);

    BspOsal_taskDelete(&gaCalibInfo->TblWriteTask);

    Vps_printf(" SRV: QSPI write Task Delete DONE !!!\n");
}

/**
 *******************************************************************************
 *
 * \brief   Function implements different SRV calibration Types and Modes
 *
 * \param   gaCalibInfo    [IN]  Chain_Common_SRV_GACalibrationInfo
 *
 *******************************************************************************
*/
Void Chain_Common_SRV_Calibration(Chain_Common_SRV_CalibParams * calInfo)
{
    Chain_Common_SRV_GACalibrationInfo *gaCalibInfo;
    gaCalibInfo = &gChainCommon_SRVCalibInfo;

    switch (calInfo->calibState)
    {
      case CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_CREATETIME:
      {
        if(calInfo->startWithCalibration)
        {
            gChainCommon_SRVCalibInfo.GACalibrationType
                = CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE;
            gChainCommon_SRVCalibInfo.GACalibrationTypePrev
                = CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE;
        }
        else
        {
            gChainCommon_SRVCalibInfo.GACalibrationType
                = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;
            gChainCommon_SRVCalibInfo.GACalibrationTypePrev
                = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;
        }

        Chain_Common_SRV_allocateGATbl(
            &gChainCommon_SRVCalibInfo
            );
        calInfo->calMode = gChainCommon_SRVCalibInfo.calMode;
        calInfo->gaLUTAddr = (UInt32) gChainCommon_SRVCalibInfo.gaLUTDDRPtr;
        calInfo->persMatAddr = (UInt32) gChainCommon_SRVCalibInfo.persMatDDRPtr;
        break;
      }

      case CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_RUNTIME:
      {
        switch(calInfo->calType)
        {
            case CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE:
                gaCalibInfo->GACalibrationType = CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE;
                break;
            case CHAIN_COMMON_SRV_GA_ERASE_ENTIRE_TABLE:
                System_qspiEraseSector(GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                                       GA_OUTPUT_LUT_SIZE);
                System_qspiEraseSector(GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                       GA_PERSPECTIVE_MATRIX_SIZE);
                gaCalibInfo->GACalibrationType = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;
                gaCalibInfo->GACalibrationTypePrev = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;
                break;
            case CHAIN_COMMON_SRV_GA_CALIBRATION_NO:
                gaCalibInfo->GACalibrationType = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;
                break;
            default:
                Vps_printf("\nUnsupported option. Please try again\n");
                break;
        }
        break;
      }

      case CHAIN_COMMON_SRV_GA_CALIBRATION_STATE_DELETETIME:
      {
        Chain_Common_SRV_freeGATbl(
            &gChainCommon_SRVCalibInfo
            );
        break;
      }

      default:
          UTILS_assert(0);
          break;
    }
}

