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
#include <examples/tda2xx/include/chains_main_srv_calibration.h>
#include <osa_file.h>
#include <osa_mem.h>
#include <osa_thr.h>
#include <linux/src/system/system_priv_ipc.h>

/*
 *******************************************************************************
 *
 * Surround view calibration task parameters
 *
 *******************************************************************************
 */

#define CHAIN_COMMON_SRV_CALIBDATA_FILE_IO_TASK_SLEEP_DURATION (1000000)


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
    UInt8 *gaLUTDDRPtr;
    UInt8 *persMatDDRPtr;
    AlgorithmLink_GAlignCalibrationMode calMode;
    Bool   TblWriteTaskExit;
    OSA_ThrHndl TblWriteTask;
    /**< Handle to task handing PersMat TBL write to QSPI flash */
} Chain_Common_SRV_GACalibrationInfo;

/**
 *******************************************************************************
 * \brief stack for PersMat table write task
 *******************************************************************************
 */

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
Void *Chain_Common_SRV_PersMatTblWriteFun(Void *arg)
{

    Bool isPersMatTblUpdated;
    UInt32 persMatTblUpdateAfter1Min;
    UInt32 * ptr;
    Chain_Common_SRV_GACalibrationInfo *gaCalibInfo;

    gaCalibInfo = (Chain_Common_SRV_GACalibrationInfo *) arg;
    isPersMatTblUpdated = FALSE;
    persMatTblUpdateAfter1Min = 0;
    gaCalibInfo->TblWriteTaskExit = FALSE;

    while (!isPersMatTblUpdated && !gaCalibInfo->TblWriteTaskExit)
    {
        Task_sleep(CHAIN_COMMON_SRV_CALIBDATA_FILE_IO_TASK_SLEEP_DURATION);

        /* Both persMat & GA LUT are written back to QSPI during demo
         * STOP.  But if the demo ended due to battery drained condition
         * both these tables will not get updated in QSPI flash.  To
         * avoid this scenario, only persMat tbl alone is updated
         * after 5 minutes from the start of the demo.
         */
        persMatTblUpdateAfter1Min++;
        if ((persMatTblUpdateAfter1Min >= 60) && (!isPersMatTblUpdated ))
        {
            isPersMatTblUpdated = TRUE;
            if (gaCalibInfo->GACalibrationType != CHAIN_COMMON_SRV_GA_CALIBRATION_NO)
            {
               Vps_printf("Storing Persp Matrix ChainsCommon_SurroundView_PersMatTblWriteFun() \n");
               ptr = (UInt32 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO);
               *ptr = GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE;
               OSA_fileWriteFileOffset(
                  CALIBDATA_FILENAME,
                  (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
                  GA_PERSPECTIVE_MATRIX_SIZE,
                  GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET
                  );
               Vps_printf("Storing Persp Matrix ChainsCommon_SurroundView_PersMatTblWriteFun() done\n");
            }
        }
    }
    return NULL;
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
    UInt32 * ptr;
    UInt32 actualReadSize;

    gaCalibInfo->gaLUTDDRPtr = (UInt8 *)System_ipcMemAlloc(OSA_HEAPID_DDR_CACHED_SR1, GA_OUTPUT_LUT_SIZE_MEM_ALLOC, 32);
    UTILS_assert(gaCalibInfo->gaLUTDDRPtr != NULL);

    gaCalibInfo->persMatDDRPtr = (UInt8 *)System_ipcMemAlloc(OSA_HEAPID_DDR_CACHED_SR1, GA_PERSPECTIVE_MATRIX_SIZE, 32);
    UTILS_assert(gaCalibInfo->persMatDDRPtr != NULL);

    gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_USERGALUT;

    /* Read the PersMat tbl always from QSPI so that it can be
     * write back unconditionally to QSPI Flash */
    OSA_fileReadFileOffset(
       CALIBDATA_FILENAME,
       (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
       GA_PERSPECTIVE_MATRIX_SIZE,
       &actualReadSize,
       GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET
       );

    switch (gaCalibInfo->GACalibrationType)
    {
        case CHAIN_COMMON_SRV_GA_CALIBRATION_NO:
            OSA_fileReadFileOffset(
               CALIBDATA_FILENAME,
               (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->gaLUTDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
               GA_MAGIC_PATTERN_SIZE_IN_BYTES,
               &actualReadSize,
               GA_OUTPUT_LUT_FLASHMEM_OFFSET
               );
            ptr = (UInt32 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->gaLUTDDRPtr, OSA_MEM_REGION_TYPE_AUTO);
            if (*ptr == GA_OUTPUT_LUT_MAGIC_SEQUENCE)
            {
                gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_USERGALUT;
                OSA_fileReadFileOffset(
                  CALIBDATA_FILENAME,
                  (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->gaLUTDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
                  GA_OUTPUT_LUT_SIZE,
                  &actualReadSize,
                  GA_OUTPUT_LUT_FLASHMEM_OFFSET
                  );
                //TBD - cache write back
            }
            else
            {
                gaCalibInfo->calMode = ALGLINK_GALIGN_CALMODE_DEFAULT;
            }
            break;
        case CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE:
           OSA_fileReadFileOffset(
             CALIBDATA_FILENAME,
             (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
             GA_MAGIC_PATTERN_SIZE_IN_BYTES,
             &actualReadSize,
             GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET
             );
             ptr = (UInt32 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO);
             if (*ptr == GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE)
             {
                gaCalibInfo->calMode =
                                ALGLINK_GALIGN_CALMODE_FORCE_USERPERSMATRIX;
                OSA_fileReadFileOffset(
                  CALIBDATA_FILENAME,
                  (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
                  GA_PERSPECTIVE_MATRIX_SIZE,
                  &actualReadSize,
                  GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET
                  );

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
    OSA_thrCreate(&gaCalibInfo->TblWriteTask, Chain_Common_SRV_PersMatTblWriteFun, OSA_THR_PRI_MIN, OSA_THR_STACK_SIZE_DEFAULT, gaCalibInfo);
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
    UInt32 * ptr;

    gaCalibInfo->TblWriteTaskExit = TRUE;

    if (gaCalibInfo->GACalibrationTypePrev == CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE)
    {
        Vps_printf("Started Storing Calibration tables into file, please Wait \n");

        ptr = (UInt32 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO);
        *ptr = GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE;
        OSA_fileWriteFileOffset(
          CALIBDATA_FILENAME,
          (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->persMatDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
          GA_PERSPECTIVE_MATRIX_SIZE,
          GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET
          );

        ptr = (UInt32 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->gaLUTDDRPtr, OSA_MEM_REGION_TYPE_AUTO);
        *ptr = GA_OUTPUT_LUT_MAGIC_SEQUENCE;
        OSA_fileWriteFileOffset(
          CALIBDATA_FILENAME,
          (UInt8 *)OSA_memPhys2Virt((unsigned int)gaCalibInfo->gaLUTDDRPtr, OSA_MEM_REGION_TYPE_AUTO),
          GA_OUTPUT_LUT_SIZE,
          GA_OUTPUT_LUT_FLASHMEM_OFFSET
          );
        gaCalibInfo->GACalibrationTypePrev = CHAIN_COMMON_SRV_GA_CALIBRATION_NO;

        Vps_printf("Storing tables into file completed. \n");
    }

    if (gaCalibInfo->GACalibrationType == CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE)
    {
        gaCalibInfo->GACalibrationTypePrev = CHAIN_COMMON_SRV_GA_CALIBRATION_FORCE;
    }
    System_ipcMemFree(OSA_HEAPID_DDR_CACHED_SR1, (UInt32)gaCalibInfo->gaLUTDDRPtr, GA_OUTPUT_LUT_SIZE_MEM_ALLOC);
    System_ipcMemFree(OSA_HEAPID_DDR_CACHED_SR1, (UInt32)gaCalibInfo->persMatDDRPtr, GA_PERSPECTIVE_MATRIX_SIZE);

    Vps_printf(" SRV: file write Task Delete in progress !!!\n");
    OSA_thrDelete(&gaCalibInfo->TblWriteTask);

    Vps_printf(" SRV: file write Task Delete DONE !!!\n");
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
                OSA_fileCreateFile(CALIBDATA_FILENAME,
                  GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET + GA_PERSPECTIVE_MATRIX_SIZE,
                  TRUE);
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
