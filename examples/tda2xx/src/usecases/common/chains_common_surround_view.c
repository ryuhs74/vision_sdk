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
#include <examples/tda2xx/include/chains_common.h>


/*
 *******************************************************************************
 *
 * Surround view calibration task parameters
 *
 *******************************************************************************
 */

#define CHAIN_SURROUND_VIEW_PERS_MAT_TSK_STACK_SIZE              (32*1024)
#define CHAIN_SURROUND_VIEW_PERS_MAT_TSK_PRI                     (4)
#define CHAIN_SURROUND_VIEW_QSPI_FLASH_WRITE_TASK_SLEEP_DURATION (1000)

/**
 *******************************************************************************
 * \brief GA ouput calibration table and perspective matrix sizes and
 *        their respective offsets in the flash memory
 *        Use a 4 byte Magic sequence in the begining of the both these
 *        tables to check the validity of tables before use
 *******************************************************************************
 */


#define GA_OUTPUT_LUT_FLASHMEM_OFFSET         (16*1024*1024)
#define GA_OUTPUT_LUT_SIZE                    (10*1024*1024)
#define GA_OUTPUT_LUT_SIZE_MEM_ALLOC          (20*1024*1024)

#define GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET (GA_OUTPUT_LUT_FLASHMEM_OFFSET + GA_OUTPUT_LUT_SIZE)
#define GA_PERSPECTIVE_MATRIX_SIZE            (64*1024)

#define GA_MAGIC_PATTERN_SIZE_IN_BYTES        (4)
#define GA_OUTPUT_LUT_MAGIC_SEQUENCE          (0x1234ABCD)
#define GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE  (0xABCD1234)

/*
 *******************************************************************************
 *
 * Surround view use-case parameters
 *
 *******************************************************************************
 */
#define LVDS_CAPTURE_WIDTH              (1280)
#define LVDS_CAPTURE_HEIGHT             (720)

#define SV_OUTPUT_WIDTH_TDA3XX          (640)
#define SV_OUTPUT_HEIGHT_TDA3XX         (760)
#define SV_OUTPUT_POSX_TDA3XX           (1024 - 640)
#define SV_OUTPUT_POSY_TDA3XX           (4) /* (768-760)/2 */

#define SV_OUTPUT_WIDTH_720P_RES_TDA3XX     (1000U)
#define SV_OUTPUT_HEIGHT_720P_RES_TDA3XX    (720U)
#define SV_OUTPUT_POSX_720P_RES_TDA3XX      (140U)
#define SV_OUTPUT_POSY_720P_RES_TDA3XX      (0U)

#define SV_OUTPUT_WIDTH_TDA2XX          (880)
#define SV_OUTPUT_HEIGHT_TDA2XX         (1080)
#define SV_OUTPUT_POSX_TDA2XX           (25+320+10)
#define SV_OUTPUT_POSY_TDA2XX           (0)

#define SV_CARBOX_WIDTH                 (190)
#define SV_CARBOX_HEIGHT                (360)

#define SV_INPUT_WIDTH                  (LVDS_CAPTURE_WIDTH)
#define SV_INPUT_HEIGHT                 (LVDS_CAPTURE_HEIGHT)
#define SV_NUM_VIEWS                    (4)

#define SVORG_SCALED_WIDTH              (LVDS_CAPTURE_WIDTH/4)
#define SVORG_SCALED_HEIGHT             (LVDS_CAPTURE_HEIGHT/4)
#define SVORG_MOSAIC_SPACING_HOR        (0)
#define SVORG_MOSAIC_SPACING_VER        (35)
#define SVORG_MOSAIC_WIDTH              (SVORG_SCALED_WIDTH)
#define SVORG_MOSAIC_HEIGHT             ((SVORG_SCALED_HEIGHT)*SV_NUM_VIEWS + SVORG_MOSAIC_SPACING_VER*(SV_NUM_VIEWS-1))
#define SVORG_MOSAIC_POSX               (25)
#define SVORG_MOSAIC_POSY               (100+35)

#define FRONTCAM_SCALED_WIDTH           (LVDS_CAPTURE_WIDTH/2)
#define FRONTCAM_SCALED_HEIGHT          (LVDS_CAPTURE_HEIGHT/2)
#define FRONTCAM_MOSAIC_SPACING_HOR     (0)
#define FRONTCAM_MOSAIC_SPACING_VER     (50)
#define FRONTCAM_MOSAIC_WIDTH           (FRONTCAM_SCALED_WIDTH)
#define FRONTCAM_MOSAIC_HEIGHT          ((FRONTCAM_SCALED_HEIGHT*2)+FRONTCAM_MOSAIC_SPACING_VER)
#define FRONTCAM_MOSAIC_POSX            (1920-25-640)
#define FRONTCAM_MOSAIC_POSY            (100+50)

#define DOF_WIDTH_ALIGN                 (64)
#define DOF_HEIGHT_ALIGN                (32)

#define FEATUREPLANE_ALG_WIDTH          (640)
#define FEATUREPLANE_ALG_HEIGHT         (360)

#define FEATUREPLANE_NUM_OUT_BUF        (8)

/**
 *******************************************************************************
 * \brief Channels with timestamp difference <= SYNC_DELTA_IN_MSEC
 *        are synced together by sync link
 *******************************************************************************
 */
#define TIGHT_SYNC_DELTA_IN_MSEC              (16)
#define LOOSE_SYNC_DELTA_IN_MSEC              (0x7FFFFFFF)

/**
 *******************************************************************************
 * \brief Channels with timestamp older than SYNC_DROP_THRESHOLD_IN_MSEC
 *        are dropped by sync link
 *******************************************************************************
 */
#define TIGHT_SYNC_DROP_THRESHOLD_IN_MSEC     (33)
#define LOOSE_SYNC_DROP_THRESHOLD_IN_MSEC     (0x7FFFFFFF)


/**
 *******************************************************************************
 * \brief Enum for various GA calibration modes (Calibration ON/OFF)
 *
 *******************************************************************************
*/
typedef enum
{
    CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO = 0,
    CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE,
    CHAINS_SURROUND_VIEW_GA_CALIBRATION_MAXNUM,
    CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE32BITS = 0x7FFFFFFF
    /**< To make sure enum is 32 bits */
}Chains_SurroundView_GACalibrationType;


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
    Chains_SurroundView_GACalibrationType GACalibrationType;
    Chains_SurroundView_GACalibrationType GACalibrationTypePrev;
    /**< This is a placeholder to remember the previous state
     *   of the SV alg GA calibration type.
     */
    char * gaLUTDDRPtr;
    char * persMatDDRPtr;
    Bool   TblWriteTaskExit;
    BspOsal_TaskHandle TblWriteTask;
    /**< Handle to task handing PersMat TBL write to QSPI flash */
} ChainsCommon_SurroundView_GACalibrationInfo;

/**
 *******************************************************************************
 * \brief stack for PersMat table write task
 *******************************************************************************
 */
#pragma DATA_ALIGN(ChainsCommon_SurroundView_PersMatTblWriteTskStack, 32)
#pragma DATA_SECTION(ChainsCommon_SurroundView_PersMatTblWriteTskStack, ".bss:taskStackSection")
UInt8 ChainsCommon_SurroundView_PersMatTblWriteTskStack[CHAIN_SURROUND_VIEW_PERS_MAT_TSK_STACK_SIZE];


ChainsCommon_SurroundView_GACalibrationInfo gChains_surroundViewCalibInfo;

/**
 *******************************************************************************
 * \brief Menu for Camera position Calibration settings.
 *******************************************************************************
 */
char gChainsCommon_SurroundView_MenuCalibration[] = {
    "\r\n "
    "\r\n ================================================"
    "\r\n Chains Run-time Camera position Calibration Menu"
    "\r\n ================================================"
    "\r\n "
    "\r\n 0: Run with GA LUT from flash (If not available, use default table)"
    "\r\n    Flash write will take place now, will take a couple of minutes"
    "\r\n "
    "\r\n 1: Force GA Calibration - Regenerate calibration tbl"
    "\r\n    Calibration & Flash write will take a couple of minutes"
    "\r\n "
    "\r\n 2: Erase entire calibration tbl from flash"
    "\r\n    Flash Erase will take a couple of minutes"
    "\r\n "
    "\r\n 3: Go to previous menu"
    "\r\n    Flash write will take place now, will take a couple of minutes"
    "\r\n "
    "\r\n Enter Choice: "
    "\r\n "
};

/**
 *******************************************************************************
 *
 * \brief   Task to write the GA persMat table to the QSPI flash
 *
 * \param   arg1    [IN]  ChainsCommon_SurroundView_GACalibrationInfo
 * \param   arg2    [IN]  ChainsCommon_SurroundView_GACalibrationInfo
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_PersMatTblWriteFun(UArg arg1, UArg arg2)
{

    Bool isPersMatTblUpdated;
    UInt32 persMatTblUpdateAfter1Min;
    UInt32 * ptr;
    ChainsCommon_SurroundView_GACalibrationInfo *gaCalibInfo;

    gaCalibInfo = (ChainsCommon_SurroundView_GACalibrationInfo *) arg1;
    isPersMatTblUpdated = FALSE;
    persMatTblUpdateAfter1Min = 0;
    gaCalibInfo->TblWriteTaskExit = FALSE;

    while (!isPersMatTblUpdated && !gaCalibInfo->TblWriteTaskExit)
    {
        Task_sleep(CHAIN_SURROUND_VIEW_QSPI_FLASH_WRITE_TASK_SLEEP_DURATION);

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
            if (gaCalibInfo->GACalibrationType != CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO)
            {
                Vps_printf("Storing Persp Matrix ChainsCommon_SurroundView_PersMatTblWriteFun() \n");
                ptr = (UInt32 *) gaCalibInfo->persMatDDRPtr;
                *ptr = GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE;
                System_qspiWriteSector(GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                      (UInt32) (gaCalibInfo->persMatDDRPtr),
                                      GA_PERSPECTIVE_MATRIX_SIZE);
                Vps_printf("Storing Persp Matrix ChainsCommon_SurroundView_PersMatTblWriteFun() done\n");
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
 * \param   gaCalibInfo    [IN]  ChainsCommon_SurroundView_GACalibrationInfo
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_allocateGATbl(
                     ChainsCommon_SurroundView_GACalibrationInfo *gaCalibInfo)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

    gaCalibInfo->gaLUTDDRPtr = Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_SR,
                                              GA_OUTPUT_LUT_SIZE_MEM_ALLOC, 32);
    UTILS_assert(gaCalibInfo->gaLUTDDRPtr != NULL);

    gaCalibInfo->persMatDDRPtr = Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_SR,
                                                GA_PERSPECTIVE_MATRIX_SIZE, 32);
    UTILS_assert(gaCalibInfo->persMatDDRPtr != NULL);

    /*
     * Create a task to update the peraMat table into QSPI flash after 5 min
     */
    gaCalibInfo->TblWriteTask = BspOsal_taskCreate(
                                    (BspOsal_TaskFuncPtr)ChainsCommon_SurroundView_PersMatTblWriteFun,
                                    CHAIN_SURROUND_VIEW_PERS_MAT_TSK_PRI,
                                    ChainsCommon_SurroundView_PersMatTblWriteTskStack,
                                    sizeof(ChainsCommon_SurroundView_PersMatTblWriteTskStack),
                                    gaCalibInfo
                                    );
    UTILS_assert(gaCalibInfo->TblWriteTask != NULL);

    /* Removing currently as the number of link stats entry for a
       single core is exceeding the current define, which is 64.
       Remove this when the above issue is fixed */

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
 * \param   gaCalibInfo    [IN]  ChainsCommon_SurroundView_GACalibrationInfo
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_freeGATbl(ChainsCommon_SurroundView_GACalibrationInfo *gaCalibInfo)
{
    UInt32 status;
    UInt32 * ptr;

    gaCalibInfo->TblWriteTaskExit = TRUE;

    if ((gaCalibInfo->GACalibrationTypePrev == CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE))
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
        gaCalibInfo->GACalibrationTypePrev = CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO;

        Vps_printf("Storing tables into flash completed. \n");
    }

    if (gaCalibInfo->GACalibrationType == CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE)
    {
        gaCalibInfo->GACalibrationTypePrev = CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE;
    }

    status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                           gaCalibInfo->gaLUTDDRPtr, GA_OUTPUT_LUT_SIZE_MEM_ALLOC);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                           gaCalibInfo->persMatDDRPtr, GA_PERSPECTIVE_MATRIX_SIZE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Vps_printf(" SRV: QSPI write Task Delete in progress !!!\n");

    /* Removing currently as the number of link stats entry for a
       single core is exceeding the current define, which is 64.
       Remove this when the above issue is fixed */

    Utils_prfLoadUnRegister(gaCalibInfo->TblWriteTask);

    BspOsal_taskDelete(&gaCalibInfo->TblWriteTask);

    Vps_printf(" SRV: QSPI write Task Delete DONE !!!\n");
}

Void ChainsCommon_SurroundView_SetSynthParams(
                                    AlgorithmLink_SynthesisCreateParams *pPrm,
                                    UInt16 svInWidth,
                                    UInt16 svInHeight,
                                    UInt16 svOutWidth,
                                    UInt16 svOutHeight,
                                    UInt16 svNumViews,
                                    Int16  svCarBoxWidth,
                                    Int16  svCarBoxHeight,
                                    AlgorithmLink_SrvOutputModes svOutputMode,
                                    Bool enableCarOverlayInAlg)
{
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_SYNTHESIS;
    pPrm->maxOutputWidth = svOutWidth;
    pPrm->maxOutputHeight = svOutHeight;
    pPrm->maxInputWidth = svInWidth;
    pPrm->maxInputHeight = svInHeight;
    pPrm->numViews = svNumViews;
    pPrm->carBoxWidth = svCarBoxWidth;
    pPrm->carBoxHeight = svCarBoxHeight;
    pPrm->numOutputFrames = 5;
    pPrm->numPhotometricStatisticsTables = 5;
    pPrm->numSgxBlendLUTables = 1;
    pPrm->synthesisMode = ALGORITHM_LINK_ALG_SIMPLESYNTHESIS;
    pPrm->svOutputMode = svOutputMode; //2D or 3D SRV
    pPrm->enableCarOverlayInAlg = enableCarOverlayInAlg; //2D or 3D SRV
}


Void ChainsCommon_SurroundView_SetGAlignParams(
                            AlgorithmLink_GAlignCreateParams *pPrm,
                            UInt16 svInWidth,
                            UInt16 svInHeight,
                            UInt16 svOutWidth,
                            UInt16 svOutHeight,
                            UInt16 svNumViews,
                            Int16  svCarBoxWidth,
                            Int16  svCarBoxHeight,
                            AlgorithmLink_SrvOutputModes svOutputMode)
{
    UInt32 * ptr;
    ChainsCommon_SurroundView_GACalibrationInfo *gaCalibInfo;

    gaCalibInfo = &gChains_surroundViewCalibInfo;

    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_GALIGNMENT;
    pPrm->maxOutputWidth = svOutWidth;
    pPrm->maxOutputHeight = svOutHeight;
    pPrm->maxInputWidth = svInWidth;
    pPrm->maxInputHeight = svInHeight;
    pPrm->numViews = svNumViews;
    pPrm->carBoxWidth = svCarBoxWidth;
    pPrm->carBoxHeight = svCarBoxHeight;

    pPrm->numOutputTables = 3;
    pPrm->enablePixelsPerCm = 0;
    pPrm->svOutputMode = svOutputMode; //2D or 3D SRV
    /* Algo to ignore first 2 frames, before alig is attempted */
    pPrm->ignoreFirstNFrames = 2U;
    pPrm->defaultFocalLength = 455U;
    UTILS_assert(gaCalibInfo->gaLUTDDRPtr != NULL);
    UTILS_assert(gaCalibInfo->persMatDDRPtr != NULL);
    pPrm->calParams.calMode = ALGLINK_GALIGN_CALMODE_DEFAULT;
    pPrm->calParams.gaLUTDDRPtr =
          (gaCalibInfo->gaLUTDDRPtr + GA_MAGIC_PATTERN_SIZE_IN_BYTES);
    pPrm->calParams.persMatDDRPtr =
          (gaCalibInfo->persMatDDRPtr + GA_MAGIC_PATTERN_SIZE_IN_BYTES);

    /* Read the PersMat tbl always from QSPI so that it can be
     * write back unconditionally to QSPI Flash */
    System_qspiReadSector((UInt32) gaCalibInfo->persMatDDRPtr,
                          GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                          GA_PERSPECTIVE_MATRIX_SIZE);

    switch (gaCalibInfo->GACalibrationType)
    {
        case CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO:
            System_qspiReadSector((UInt32) gaCalibInfo->gaLUTDDRPtr,
                                  GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                                  GA_MAGIC_PATTERN_SIZE_IN_BYTES);
            ptr = (UInt32 *) gaCalibInfo->gaLUTDDRPtr;
            if (*ptr == GA_OUTPUT_LUT_MAGIC_SEQUENCE)
            {
                pPrm->calParams.calMode = ALGLINK_GALIGN_CALMODE_USERGALUT;
                System_qspiReadSector((UInt32) gaCalibInfo->gaLUTDDRPtr,
                                      GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                                      GA_OUTPUT_LUT_SIZE);
                //TBD - cache write back
            }
            else
            {
                pPrm->calParams.calMode = ALGLINK_GALIGN_CALMODE_DEFAULT;
            }
            break;
        case CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE:
            System_qspiReadSector((UInt32) gaCalibInfo->persMatDDRPtr,
                                  GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                  GA_MAGIC_PATTERN_SIZE_IN_BYTES);
            ptr = (UInt32 *) gaCalibInfo->persMatDDRPtr;
            if (*ptr == GA_PERSPECTIVE_MATRIX_MAGIC_SEQUENCE)
            {
                pPrm->calParams.calMode =
                                ALGLINK_GALIGN_CALMODE_FORCE_USERPERSMATRIX;
                System_qspiReadSector((UInt32) gaCalibInfo->persMatDDRPtr,
                                      GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                      GA_PERSPECTIVE_MATRIX_SIZE);
                //TBD - cache write back
            }
            else
            {
                pPrm->calParams.calMode =
                                ALGLINK_GALIGN_CALMODE_FORCE_DEFAULTPERSMATRIX;
            }
            break;
        default:
            UTILS_assert(0);
            break;
    }
}

Void ChainsCommon_SurroundView_SetPAlignParams(
                                    AlgorithmLink_PAlignCreateParams *pPrm,
                                    UInt16 svInWidth,
                                    UInt16 svInHeight,
                                    UInt16 svOutWidth,
                                    UInt16 svOutHeight,
                                    UInt16 svNumViews,
                                    Int16  svCarBoxWidth,
                                    Int16  svCarBoxHeight,
                                    AlgorithmLink_SrvOutputModes svOutputMode
                                    )
{
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_DSP_ALG_PALIGNMENT;
    pPrm->maxOutputWidth = svOutWidth;
    pPrm->maxOutputHeight = svOutHeight;
    pPrm->maxInputWidth = svInWidth;
    pPrm->maxInputHeight = svInHeight;
    pPrm->numViews = svNumViews;
    pPrm->carBoxWidth = svCarBoxWidth;
    pPrm->carBoxHeight = svCarBoxHeight;
    pPrm->numOutputTables = 5;
    pPrm->dataFormat = SYSTEM_DF_YUV420SP_UV;
    pPrm->svOutputMode = svOutputMode; //2D or 3D SRV
}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 * \param   syncMode [IN]    1 - Tight Sync, 0 - Loose Sync
 *          pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetSyncPrm(
                    SyncLink_CreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 syncMode
                    )
{
    UInt16 chId;

    pPrm->chParams.numCh = numLvdsCh;
    pPrm->chParams.numActiveCh = pPrm->chParams.numCh;
    for(chId = 0; chId < pPrm->chParams.numCh; chId++)
    {
        pPrm->chParams.channelSyncList[chId] = TRUE;
    }

    if(syncMode == 1)
    {
        pPrm->chParams.syncDelta = TIGHT_SYNC_DELTA_IN_MSEC;
        pPrm->chParams.syncThreshold = TIGHT_SYNC_DROP_THRESHOLD_IN_MSEC;
    }
    else
    {
        pPrm->chParams.syncDelta = LOOSE_SYNC_DELTA_IN_MSEC;
        pPrm->chParams.syncThreshold = LOOSE_SYNC_DROP_THRESHOLD_IN_MSEC;
    }

}

/**
 *******************************************************************************
 *
 * \brief   Set Sync Create Parameters
 *
 * \param   syncMode [IN]    1 - Tight Sync, 0 - Loose Sync
 *          pPrm    [OUT]    SyncLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetSyncFcPrm(
                    SyncLink_CreateParams *pPrm
                    )
{
    pPrm->chParams.numCh = 2;
    pPrm->chParams.syncDelta = 16;
    pPrm->chParams.syncThreshold = 0xFFFF;
}

/**
 *******************************************************************************
 *
 * \brief   Set Display Create Parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetDisplayPrm(
                            DisplayLink_CreateParams *pSvDisplay,
                            DisplayLink_CreateParams *pSvOrgDisplay,
                            DisplayLink_CreateParams *pFrontCamDisplay,
                            DisplayLink_CreateParams *pGrpxDisplay,
                            UInt32 displayWidth,
                            UInt32 displayHeight
                            )
{
    float displayWidthScale = 1;
    float displayHeightScale = 1;
    UInt32 svOutWidth, svOutHeight;
    UInt32 svPosX, svPosY;

    if(Bsp_platformIsTda3xxFamilyBuild())
    {
        displayWidthScale = (1024.0)/displayWidth;
        displayHeightScale = (768.0)/displayHeight;
        svOutWidth = SV_OUTPUT_WIDTH_TDA3XX;
        svOutHeight = SV_OUTPUT_HEIGHT_TDA3XX;
        svPosX = SV_OUTPUT_POSX_TDA3XX;
        svPosY = SV_OUTPUT_POSY_TDA3XX;
        if ((1280U == displayWidth) && (720U == displayHeight))
        {
            displayWidthScale = 1U;
            displayHeightScale = 1U;
            svOutWidth = SV_OUTPUT_WIDTH_720P_RES_TDA3XX;
            svOutHeight = SV_OUTPUT_HEIGHT_720P_RES_TDA3XX;
            svPosX = SV_OUTPUT_POSX_720P_RES_TDA3XX;
            svPosY = SV_OUTPUT_POSY_720P_RES_TDA3XX;
        }
    }
    else
    {
        displayWidthScale = (1920.0)/displayWidth;
        displayHeightScale = (1080.0)/displayHeight;
        svOutWidth = SV_OUTPUT_WIDTH_TDA2XX;
        svOutHeight = SV_OUTPUT_HEIGHT_TDA2XX;
        svPosX = SV_OUTPUT_POSX_TDA2XX;
        svPosY = SV_OUTPUT_POSY_TDA2XX;
    }

    if(pSvDisplay)
    {
        pSvDisplay->rtParams.tarWidth         = (float)svOutWidth / displayWidthScale;
        pSvDisplay->rtParams.tarHeight        = (float)svOutHeight / displayHeightScale;
        pSvDisplay->rtParams.posX             = (float)svPosX / displayWidthScale;
        pSvDisplay->rtParams.posY             = (float)svPosY / displayHeightScale;
        pSvDisplay->displayId                 = DISPLAY_LINK_INST_DSS_VID1;
    }
    if(pSvOrgDisplay)
    {
        pSvOrgDisplay->rtParams.tarWidth      = (float)SVORG_MOSAIC_WIDTH / displayWidthScale;
        pSvOrgDisplay->rtParams.tarHeight     = (float)SVORG_MOSAIC_HEIGHT / displayHeightScale;
        pSvOrgDisplay->rtParams.posX          = (float)SVORG_MOSAIC_POSX / displayWidthScale;
        pSvOrgDisplay->rtParams.posY          = (float)SVORG_MOSAIC_POSY / displayHeightScale;
        pSvOrgDisplay->displayId              = DISPLAY_LINK_INST_DSS_VID2;
    }
    if(pFrontCamDisplay)
    {
        pFrontCamDisplay->rtParams.tarWidth   = (float)FRONTCAM_MOSAIC_WIDTH / displayWidthScale;
        pFrontCamDisplay->rtParams.tarHeight  = (float)FRONTCAM_MOSAIC_HEIGHT / displayHeightScale;
        pFrontCamDisplay->rtParams.posX       = (float)FRONTCAM_MOSAIC_POSX / displayWidthScale;
        pFrontCamDisplay->rtParams.posY       = (float)FRONTCAM_MOSAIC_POSY / displayHeightScale;
        pFrontCamDisplay->displayId           = DISPLAY_LINK_INST_DSS_VID3;
    }
    if(pGrpxDisplay)
    {
        pGrpxDisplay->rtParams.tarWidth       = displayWidth;
        pGrpxDisplay->rtParams.tarHeight      = displayHeight;
        pGrpxDisplay->rtParams.posX           = 0;
        pGrpxDisplay->rtParams.posY           = 0;
        pGrpxDisplay->displayId               = DISPLAY_LINK_INST_DSS_GFX1;
    }
}

/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 *          This function is used to set the VPE params.
 *          It is called in Create function. It is advisable to have
 *          Chains_lvdsMultiVipCaptureDisplay_ResetLinkPrm prior to set params
 *          so all the default params get set.
 *
 * \param   pPrm         [OUT]    DisplayLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetVpePrm(
                        VpeLink_CreateParams *pSvOrgVpe,
                        VpeLink_CreateParams *pFrontCamVpe,
                        UInt32 frontCamAlgOutWidth,
                        UInt32 frontCamAlgOutHeight
                    )
{
    VpeLink_CreateParams *pPrm;
    UInt32 chId;
    VpeLink_ChannelParams *chPrms;
    UInt32 outId = 0;

    if(pSvOrgVpe)
    {
        pPrm = pSvOrgVpe;
        pPrm->enableOut[0] = TRUE;
        for (chId = 0; chId < 4; chId++)
        {
            chPrms = &pPrm->chParams[chId];
            chPrms->outParams[outId].numBufsPerCh =
                                     VPE_LINK_NUM_BUFS_PER_CH_DEFAULT;

            chPrms->outParams[outId].width = SVORG_SCALED_WIDTH;
            chPrms->outParams[outId].height = SVORG_SCALED_HEIGHT;
            chPrms->outParams[outId].dataFormat = SYSTEM_DF_YUV420SP_UV;

            chPrms->scCfg.bypass       = FALSE;
            chPrms->scCfg.nonLinear    = FALSE;
            chPrms->scCfg.stripSize    = 0;

            chPrms->scCropCfg.cropStartX = 0;
            chPrms->scCropCfg.cropStartY = 0;
            chPrms->scCropCfg.cropWidth = LVDS_CAPTURE_WIDTH;
            chPrms->scCropCfg.cropHeight = LVDS_CAPTURE_HEIGHT;
        }
    }
    if(pFrontCamVpe)
    {
        pPrm = pFrontCamVpe;
        pPrm->enableOut[0] = TRUE;
        for (chId = 0; chId < 2; chId++)
        {
            chPrms = &pPrm->chParams[chId];
            chPrms->outParams[outId].numBufsPerCh =
                                     VPE_LINK_NUM_BUFS_PER_CH_DEFAULT;

            chPrms->outParams[outId].width = FRONTCAM_SCALED_WIDTH;
            chPrms->outParams[outId].height = FRONTCAM_SCALED_HEIGHT;
            chPrms->outParams[outId].dataFormat = SYSTEM_DF_YUV420SP_UV;

            chPrms->scCfg.bypass       = FALSE;
            chPrms->scCfg.nonLinear    = FALSE;
            chPrms->scCfg.stripSize    = 0;


            chPrms->scCropCfg.cropStartX = 0;
            chPrms->scCropCfg.cropStartY = 0;
            chPrms->scCropCfg.cropWidth = LVDS_CAPTURE_WIDTH;
            chPrms->scCropCfg.cropHeight = LVDS_CAPTURE_HEIGHT;

            if(chId==1)
            {
                /* CH1 is front cam algorithm output,
                 * its size depends on the algo that is used
                 *
                 * For Edge detect, output size is sensor W/2 x H/2
                 * For Dense Optical flow, output size is sensor W x H
                 *   aligned to algo specific values
                 */
                chPrms->scCropCfg.cropWidth = frontCamAlgOutWidth;
                chPrms->scCropCfg.cropHeight = frontCamAlgOutHeight;
            }
        }
    }
}

static void ChainsCommon_SurroundView_SetSelectPrm(SelectLink_CreateParams *pPrm)
{
    pPrm->numOutQue = 2;

    pPrm->outQueChInfo[0].outQueId   = 0;
    pPrm->outQueChInfo[0].numOutCh   = 4;
    pPrm->outQueChInfo[0].inChNum[0] = 0;
    pPrm->outQueChInfo[0].inChNum[1] = 1;
    pPrm->outQueChInfo[0].inChNum[2] = 2;
    pPrm->outQueChInfo[0].inChNum[3] = 3;

    pPrm->outQueChInfo[1].outQueId   = 1;
    pPrm->outQueChInfo[1].numOutCh   = 1;
    pPrm->outQueChInfo[1].inChNum[0] = 4;
}

/**
 *******************************************************************************
 *
 * \brief   Set DMA SW Mosaic Create Parameters
 *
 *          It is called in Create function.
 *          In this function SwMs alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetAlgDmaSwMsPrm(
                    AlgorithmLink_DmaSwMsCreateParams *pPrm,
                    UInt32 numLvdsCh,
                    UInt32 channelWidth,
                    UInt32 channelHeight,
                    UInt32 layoutType,
                    UInt32 channelSpacingHor,
                    UInt32 channelSpacingVer
                   )
{
    UInt32 algId, winId;
    UInt32 useLocalEdma;
    AlgorithmLink_DmaSwMsLayoutWinInfo *pWinInfo;
    UInt32 secondRowFlag, numLvdsChBy2;

    useLocalEdma = FALSE;
    algId = ALGORITHM_LINK_IPU_ALG_DMA_SWMS;

    pPrm->baseClassCreate.algId   = algId;
    pPrm->numOutBuf               = 4;
    pPrm->useLocalEdma            = useLocalEdma;
    pPrm->initLayoutParams.numWin = numLvdsCh;

    switch(layoutType)
    {
        default:
        case 0:
             /*
              * vertical strip
              */
            pPrm->maxOutBufWidth     = channelWidth;
            pPrm->maxOutBufHeight    = (channelHeight*(numLvdsCh)) +
                                       (channelSpacingVer*(numLvdsCh-1));

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                pWinInfo->outStartX = 0;
                pWinInfo->outStartY = winId*(channelHeight+channelSpacingVer);
             }

            break;
        case 1:
             /*
              * Horizontal strip
              */
            pPrm->maxOutBufWidth     = (channelWidth*(numLvdsCh)) +
                                       (channelSpacingHor*(numLvdsCh-1));
            pPrm->maxOutBufHeight    = channelHeight;

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                pWinInfo->outStartX = winId*(channelWidth+channelSpacingHor);
                pWinInfo->outStartY = 0;
             }

            break;

        case 2:
             /*
              * Two Horizontal strips
              */
            numLvdsChBy2 = (numLvdsCh+1) / 2;
            pPrm->maxOutBufWidth     = (channelWidth*(numLvdsChBy2)) +
                                       (channelSpacingHor*(numLvdsChBy2-1));
            pPrm->maxOutBufHeight    = (channelHeight*2) + channelSpacingVer;

            for(winId=0; winId<pPrm->initLayoutParams.numWin; winId++)
            {
                pWinInfo = &pPrm->initLayoutParams.winInfo[winId];
                pWinInfo->chId = winId;
                pWinInfo->inStartX = 0;
                pWinInfo->inStartY = 0;
                pWinInfo->width    = channelWidth;
                pWinInfo->height   = channelHeight;
                secondRowFlag = ( winId>= numLvdsChBy2 ? 1 : 0);
                pWinInfo->outStartX = (winId % numLvdsChBy2) *(channelWidth+channelSpacingHor);
                pWinInfo->outStartY = secondRowFlag * (channelHeight+channelSpacingVer);
             }

            break;

    }

    pPrm->initLayoutParams.outBufWidth  = pPrm->maxOutBufWidth;
    pPrm->initLayoutParams.outBufHeight = pPrm->maxOutBufHeight;

}

/**
 *******************************************************************************
 *
 * \brief   Set Graphics Create Parameters
 *
 *
 *          This function is used to set the Grtaphics Link params.
 *          It is called in Create function.
 *
 * \param   pPrm         [IN]    GrpxSrcLink_CreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetGrpxSrcPrms(
                                           GrpxSrcLink_CreateParams *pPrm,
                                           UInt32 displayWidth,
                                           UInt32 displayHeight)
{

    pPrm->grpxBufInfo.dataFormat  = SYSTEM_DF_BGR16_565;
    pPrm->grpxBufInfo.height   = displayHeight;
    pPrm->grpxBufInfo.width    = displayWidth;

    if(Bsp_platformIsTda3xxFamilyBuild())
    {
        pPrm->tda3xxSvFsRotLayoutEnable = TRUE;
        pPrm->statsDisplayEnable = TRUE;
        pPrm->enableJeepOverlay = TRUE;
    }
    else
    {
        pPrm->surroundViewEdgeDetectLayoutEnable = TRUE;
        pPrm->statsDisplayEnable = TRUE;
        pPrm->enableJeepOverlay = TRUE;
    }
}


/**
 *******************************************************************************
 *
 * \brief   Set Edge Detection Alg parameters
 *
 *          It is called in Create function.
 *          In this function alg link params are set
 *          The algorithm which is to run on core is set to
 *          baseClassCreate.algId. The input whdth and height to alg are set.
 *          Number of input buffers required by alg are also set here.
 *
 * \param   pPrm    [IN]    AlgorithmLink_EdgeDetectionCreateParams
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetEdgeDetectionAlgPrms(
                                     AlgorithmLink_EdgeDetectionCreateParams *pPrm,
                                     UInt32 maxWidth,
                                     UInt32 maxHeight)
{
    pPrm->baseClassCreate.size  = sizeof(AlgorithmLink_EdgeDetectionCreateParams);
    pPrm->baseClassCreate.algId = ALGORITHM_LINK_EVE_ALG_EDGEDETECTION;
    pPrm->maxWidth    = maxWidth;
    pPrm->maxHeight   = maxHeight;
    pPrm->numOutputFrames = 4;
}

/**
 *******************************************************************************
 *
 * \brief   Set Dense Optical Flow parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetDenseOptFlowAlgPrms(
                       AlgorithmLink_DenseOptFlowCreateParams *pPrm,
                       UInt32 width,
                       UInt32 height,
                       UInt32 eveId,
                       UInt32 numOfEve,
                       AlgorithmLink_DenseOptFlowLKnumPyr numPyramids)
{
    AlgorithmLink_DenseOptFlowCreateParams_Init(pPrm);

    pPrm->processPeriodicity = numOfEve;
    pPrm->processStartFrame  = eveId;
    pPrm->roiParams.width    = width;
    pPrm->roiParams.height   = height;
    pPrm->algEnable          = TRUE;
    pPrm->numOutBuf          = 4;

    pPrm->numPyramids     = numPyramids;
    pPrm->enableSmoothing = TRUE;
    pPrm->smoothingSize   = ALGLINK_DENSEOPTFLOW_LKSMOOTHSIZE_5x5;
    pPrm->maxVectorSizeX  = 16;
    pPrm->maxVectorSizeY  = 16;
}

/**
 *******************************************************************************
 *
 * \brief   Set Vector to image algorithm parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
static Void ChainsCommon_SurroundView_SetVectorToImageAlgPrms(
                       AlgorithmLink_VectorToImageCreateParams *pPrm,
                       UInt32 width,
                       UInt32 height,
                       UInt32 lutId,
                       Bool isLutSize_129x129,
                       UInt32 numOfEve)
{
    AlgorithmLink_VectorToImageCreateParams_Init(pPrm);

    pPrm->maxWidth  = width;
    pPrm->maxHeight = height;
    pPrm->numOutputFrames = 5;
    pPrm->lutId   = lutId;
    pPrm->isLutSize_129x129   = isLutSize_129x129;
    pPrm->dataFormat = SYSTEM_DF_YUV422I_YUYV;
}

/**
 *******************************************************************************
 *
 * \brief   Set PD draw parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetObjectDrawPrms(
                   AlgorithmLink_ObjectDrawCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    UTILS_assert(pPrm != NULL);
    pPrm->imgFrameWidth    = width;
    pPrm->imgFrameHeight   = height;
    pPrm->numOutBuffers = FEATUREPLANE_NUM_OUT_BUF;
    pPrm->pdRectThickness = 3;
}

/**
 *******************************************************************************
 *
 * \brief   Set Feature Plane Compute Alg parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetFeaturePlaneComputeAlgPrms(
                   AlgorithmLink_FeaturePlaneComputationCreateParams *pPrm,
                   UInt32 width,
                   UInt32 height)
{
    pPrm->imgFrameHeight = height;
    pPrm->imgFrameWidth  = width;
    pPrm->numOutBuffers  = FEATUREPLANE_NUM_OUT_BUF;
}

/**
 *******************************************************************************
 *
 * \brief   Set Feature Plane Classify Alg parameters
 *
 * \param   pPrm    [IN]    algorithm parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetObjectDetectPrm(
                   AlgorithmLink_ObjectDetectionCreateParams *pPrm)
{
    pPrm->numOutBuffers  = FEATUREPLANE_NUM_OUT_BUF;
    pPrm->enablePD       = TRUE;
    pPrm->enableTSR      = TRUE;
}

/**
 *******************************************************************************
 *
 * \brief   Set VPE Create Parameters
 *
 * \param   pPrm    [OUT]    VpeLink_CreateParams
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetFCVpePrm(
                    VpeLink_CreateParams *pPrm,
                    UInt32 outWidth,
                    UInt32 outHeight,
                    UInt32 srcWidth,
                    UInt32 srcHeight,
                    System_VideoDataFormat dataFormat
                    )
{
    pPrm->enableOut[0] = TRUE;

    pPrm->chParams[0].outParams[0].width = SystemUtils_floor(outWidth, 4);
    pPrm->chParams[0].outParams[0].height = SystemUtils_floor(outHeight, 2);
    pPrm->chParams[0].outParams[0].numBufsPerCh = 3;

    pPrm->chParams[0].scCropCfg.cropStartX = 0;
    pPrm->chParams[0].scCropCfg.cropStartY = 0;
    pPrm->chParams[0].scCropCfg.cropWidth  = srcWidth;
    pPrm->chParams[0].scCropCfg.cropHeight = srcHeight;

    pPrm->chParams[0].outParams[0].dataFormat = dataFormat;
    pPrm->chParams[0].outParams[0].numBufsPerCh = 8;
}

/**
 *******************************************************************************
 *
 * \brief   Set Algorithm related parameters
 *
 *******************************************************************************
*/
Void ChainsCommon_SurroundView_SetLaneDetectPrm(
                    AlgorithmLink_LaneDetectCreateParams *pAlgPrm,
                    AlgorithmLink_LaneDetectDrawCreateParams *pDrawPrm,
                    UInt32 startX,
                    UInt32 startY,
                    UInt32 width,
                    UInt32 height
                    )
{
    pAlgPrm->imgFrameStartX = startX;
    pAlgPrm->imgFrameStartY = startY;
    pAlgPrm->imgFrameWidth  = width;
    pAlgPrm->imgFrameHeight = height;

    pAlgPrm->roiStartX      = 32 - LD_FILTER_TAP_X;
    pAlgPrm->roiStartY      = 120;
    pAlgPrm->roiWidth       = 576 + 2*LD_FILTER_TAP_X;
    pAlgPrm->roiHeight      = 240;

    pDrawPrm->imgFrameStartX = startX;
    pDrawPrm->imgFrameStartY = startY;
    pDrawPrm->imgFrameWidth  = width;
    pDrawPrm->imgFrameHeight  = height;
    pDrawPrm->enableDrawLines = TRUE;

    pAlgPrm->cannyHighThresh        = 30;
    pAlgPrm->cannyLowThresh         = 20;
    pAlgPrm->houghNmsThresh         = 20;
    pAlgPrm->startThetaLeft         = 100;
    pAlgPrm->endThetaLeft           = 150;
    pAlgPrm->startThetaRight        = 10;
    pAlgPrm->endThetaRight          = 60;
    pAlgPrm->thetaStepSize          = 1;
}

Void ChainsCommon_SurroundView_CalibInit(Bool startWithCalibration)
{
    if(startWithCalibration)
    {
        gChains_surroundViewCalibInfo.GACalibrationType
            = CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE;
        gChains_surroundViewCalibInfo.GACalibrationTypePrev
            = CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE;
    }
    else
    {
        gChains_surroundViewCalibInfo.GACalibrationType
            = CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO;
        gChains_surroundViewCalibInfo.GACalibrationTypePrev
            = CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO;
    }

    ChainsCommon_SurroundView_allocateGATbl(
        &gChains_surroundViewCalibInfo
        );
}

/**
 *******************************************************************************
 *
 * \brief   Run time Menu selection for the Calibration options
 *
 *          This functions set/implement follwing
 *          - Set calibartion option enable
 *          - Set calibartion option disable
 *          - Set QSPI Flash erase option
 *
 * \param   gaCalibInfo    [IN]  ChainsCommon_SurroundView_GACalibrationInfo
 *
 *******************************************************************************
*/
char ChainsCommon_SurroundView_MenuCalibration()
{
    char ch;
    UInt32 done = FALSE;
    ChainsCommon_SurroundView_GACalibrationInfo *gaCalibInfo;

    gaCalibInfo = &gChains_surroundViewCalibInfo;

    while(!done)
    {
        Task_sleep(500);
        Vps_printf(gChainsCommon_SurroundView_MenuCalibration);
        Task_sleep(500);

        ch = Chains_readChar();

        switch(ch)
        {
            case '1':
                gaCalibInfo->GACalibrationType = CHAINS_SURROUND_VIEW_GA_CALIBRATION_FORCE;
                done = TRUE;
                break;
            case '2':
                System_qspiEraseSector(GA_OUTPUT_LUT_FLASHMEM_OFFSET,
                                       GA_OUTPUT_LUT_SIZE);
                System_qspiEraseSector(GA_PERSPECTIVE_MATRIX_FLASHMEM_OFFSET,
                                       GA_PERSPECTIVE_MATRIX_SIZE);
                gaCalibInfo->GACalibrationType = CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO;
                gaCalibInfo->GACalibrationTypePrev = CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO;
                break;
            case '0':
            case '3':
                gaCalibInfo->GACalibrationType = CHAINS_SURROUND_VIEW_GA_CALIBRATION_NO;
                done = TRUE;
                break;
        }
    }

    return ch;
}

Void ChainsCommon_SurroundView_SetParams(
                            CaptureLink_CreateParams *pVipCapture,
                            AvbRxLink_CreateParams *pAvbRxPrm,
                            DecLink_CreateParams *pDecPrm,
                            SelectLink_CreateParams *pCaptureSelect,
                            VpeLink_CreateParams *pSvOrgVpe,
                            VpeLink_CreateParams *pFrontCamVpe,
                            SyncLink_CreateParams *pSvSync,
                            SyncLink_CreateParams *pSvOrgSync,
                            SyncLink_CreateParams *pFrontCamSync,
                            AlgorithmLink_SynthesisCreateParams *pSynthPrm,
                            AlgorithmLink_GAlignCreateParams *pGAlignPrm,
                            AlgorithmLink_PAlignCreateParams *pPAlignPrm,
                            AlgorithmLink_EdgeDetectionCreateParams *pEdgeDetect,
                            AlgorithmLink_DenseOptFlowCreateParams *pDof,
                            AlgorithmLink_VectorToImageCreateParams *pDofVectorToImage,
                            AlgorithmLink_DmaSwMsCreateParams *pSvOrgDmaSwMs,
                            AlgorithmLink_DmaSwMsCreateParams *pFrontCamDmaSwMs,
                            GrpxSrcLink_CreateParams *pGrpxSrc,
                            DisplayLink_CreateParams *pSvDisplay,
                            DisplayLink_CreateParams *pSvOrgDisplay,
                            DisplayLink_CreateParams *pFrontCamDisplay,
                            DisplayLink_CreateParams *pGrpxDisplay,
                            Chains_DisplayType displayType,
                            UInt32 numLvdsCh,
                            AlgorithmLink_SrvOutputModes svOutputMode,
                            VpeLink_CreateParams *VPE_algPdPrm,
                            AlgorithmLink_FeaturePlaneComputationCreateParams *Alg_FeaturePlaneComputationPrm,
                            AlgorithmLink_ObjectDetectionCreateParams *Alg_ObjectDetectionPrm,
                            SyncLink_CreateParams *Sync_algPdPrm,
                            AlgorithmLink_ObjectDrawCreateParams *Alg_ObjectDrawPrm,
                            SyncLink_CreateParams *Sync_algLdPrm,
                            AlgorithmLink_LaneDetectCreateParams *Alg_LaneDetectPrm,
                            AlgorithmLink_LaneDetectDrawCreateParams *Alg_LaneDetectDrawPrm,
                            Bool enableCarOverlayInAlg
)
{
    UInt32 portId[VIDEO_SENSOR_MAX_LVDS_CAMERAS];
    UInt32 displayWidth, displayHeight;
    UInt32 frontCamAlgOutWidth, frontCamAlgOutHeight;
    UInt32 svOutWidth, svOutHeight;

    if(Bsp_platformIsTda3xxFamilyBuild())
    {
        svOutWidth = SV_OUTPUT_WIDTH_TDA3XX;
        svOutHeight = SV_OUTPUT_HEIGHT_TDA3XX;
    }
    else
    {
        svOutWidth = SV_OUTPUT_WIDTH_TDA2XX;
        svOutHeight = SV_OUTPUT_HEIGHT_TDA2XX;
    }

    /* by default assume front cam WxH == sensor WxH
     * But this will be overridden later in the logic below
     */
    frontCamAlgOutWidth  = LVDS_CAPTURE_WIDTH;
    frontCamAlgOutHeight = LVDS_CAPTURE_HEIGHT;

    UTILS_assert(pSynthPrm!=NULL);
    UTILS_assert(pGAlignPrm!=NULL);
    UTILS_assert(pPAlignPrm!=NULL);
    UTILS_assert(pSvSync!=NULL);

    ChainsCommon_GetDisplayWidthHeight(
        displayType,
        &displayWidth,
        &displayHeight
        );

    if(pVipCapture)
    {
        ChainsCommon_MultiCam_StartCaptureDevice(
            CHAINS_CAPTURE_SRC_OV10635,
            portId,
            numLvdsCh
            );

        ChainsCommon_MultiCam_SetCapturePrms(pVipCapture,
                LVDS_CAPTURE_WIDTH,
                LVDS_CAPTURE_HEIGHT,
                portId,
                numLvdsCh
                );
        {
            UInt32 i;
            CaptureLink_VipInstParams *pInstPrm;
            for (i=0; i<SYSTEM_CAPTURE_VIP_INST_MAX; i++)
            {
                pInstPrm = &pVipCapture->vipInst[i];
                pInstPrm->numBufs = 6;
            }
            /* skip alternate frame to make it 15fps output for Front camera */
            //pVipCapture->vipInst[4].outParams[0].frameSkipMask
            //    = 0x2AAAAAAA;
        }
    }

    if(pAvbRxPrm && pDecPrm)
    {
        ChainsCommon_SetAvbRxDecodePrm(
            pAvbRxPrm,
            pDecPrm,
            LVDS_CAPTURE_WIDTH,
            LVDS_CAPTURE_HEIGHT,
            numLvdsCh
            );
    }

    ChainsCommon_SurroundView_SetSynthParams(pSynthPrm,
                                            SV_INPUT_WIDTH,
                                            SV_INPUT_HEIGHT,
                                            svOutWidth,
                                            svOutHeight,
                                            SV_NUM_VIEWS,
                                            SV_CARBOX_WIDTH,
                                            SV_CARBOX_HEIGHT,
                                            svOutputMode,
                                            enableCarOverlayInAlg);

    ChainsCommon_SurroundView_SetGAlignParams(pGAlignPrm,
                                            SV_INPUT_WIDTH,
                                            SV_INPUT_HEIGHT,
                                            svOutWidth,
                                            svOutHeight,
                                            SV_NUM_VIEWS,
                                            SV_CARBOX_WIDTH,
                                            SV_CARBOX_HEIGHT,
                                            svOutputMode);

    ChainsCommon_SurroundView_SetPAlignParams(pPAlignPrm,
                                            SV_INPUT_WIDTH,
                                            SV_INPUT_HEIGHT,
                                            svOutWidth,
                                            svOutHeight,
                                            SV_NUM_VIEWS,
                                            SV_CARBOX_WIDTH,
                                            SV_CARBOX_HEIGHT,
                                            svOutputMode);

    if(pEdgeDetect)
    {
        frontCamAlgOutWidth  = LVDS_CAPTURE_WIDTH/2;
        frontCamAlgOutHeight = LVDS_CAPTURE_HEIGHT/2;
        ChainsCommon_SurroundView_SetEdgeDetectionAlgPrms(
                                    pEdgeDetect,
                                    LVDS_CAPTURE_WIDTH,
                                    LVDS_CAPTURE_HEIGHT);
    }

    if(pDof && pDofVectorToImage)
    {
        /* align to algorithm required W x H */
        frontCamAlgOutWidth
            = SystemUtils_floor(LVDS_CAPTURE_WIDTH,
                                    DOF_WIDTH_ALIGN);
        frontCamAlgOutHeight
            = SystemUtils_floor(LVDS_CAPTURE_HEIGHT,
                                    DOF_HEIGHT_ALIGN*2);

        ChainsCommon_SurroundView_SetDenseOptFlowAlgPrms(
                pDof,
                frontCamAlgOutWidth,
                frontCamAlgOutHeight,
                0,
                1,
                ALGLINK_DENSEOPTFLOW_LKNUMPYR_1);


        ChainsCommon_SurroundView_SetVectorToImageAlgPrms(
                                pDofVectorToImage,
                                frontCamAlgOutWidth,
                                frontCamAlgOutHeight,
                                1,
                                TRUE,
                                2 );

        /* crop a bit before giving Vector to image output to scalar */
        frontCamAlgOutWidth -= 64;
        frontCamAlgOutHeight -= 64;
    }

    if(pCaptureSelect)
    {
        ChainsCommon_SurroundView_SetSelectPrm(pCaptureSelect);
    }

    if(pAvbRxPrm)
    {
        ChainsCommon_SurroundView_SetSyncPrm(pSvSync, 4, 0);
    }
    else
    {
        ChainsCommon_SurroundView_SetSyncPrm(pSvSync, 4, 1);
    }

    if(pSvOrgSync)
    {
        if(pAvbRxPrm)
        {
            ChainsCommon_SurroundView_SetSyncPrm(pSvOrgSync, 4, 0);
        }
        else
        {
            ChainsCommon_SurroundView_SetSyncPrm(pSvOrgSync, 4, 1);
        }

    }
    if(pFrontCamSync)
    {
        ChainsCommon_SurroundView_SetSyncPrm(pFrontCamSync, 2, 0);
    }

    ChainsCommon_SurroundView_SetVpePrm(pSvOrgVpe, pFrontCamVpe,
                    frontCamAlgOutWidth, frontCamAlgOutHeight);

    if(pSvOrgDmaSwMs)
    {
        ChainsCommon_SurroundView_SetAlgDmaSwMsPrm(
                    pSvOrgDmaSwMs,
                    SV_NUM_VIEWS,
                    SVORG_SCALED_WIDTH,
                    SVORG_SCALED_HEIGHT,
                    0,
                    SVORG_MOSAIC_SPACING_HOR,
                    SVORG_MOSAIC_SPACING_VER
                   );
    }
    if(pFrontCamDmaSwMs)
    {
        ChainsCommon_SurroundView_SetAlgDmaSwMsPrm(
                    pFrontCamDmaSwMs,
                    2,
                    FRONTCAM_SCALED_WIDTH,
                    FRONTCAM_SCALED_HEIGHT,
                    0,
                    FRONTCAM_MOSAIC_SPACING_HOR,
                    FRONTCAM_MOSAIC_SPACING_VER
                   );
    }
    if(pGrpxSrc)
    {
        ChainsCommon_SurroundView_SetGrpxSrcPrms(
                            pGrpxSrc,
                            displayWidth,
                            displayHeight
                        );

        if(pEdgeDetect)
        {
            pGrpxSrc->surroundViewEdgeDetectLayoutEnable = TRUE;
            pGrpxSrc->surroundViewDOFLayoutEnable = FALSE;
            pGrpxSrc->surroundViewPdTsrLayoutEnable = FALSE;
            pGrpxSrc->surroundViewLdLayoutEnable = FALSE;
        }
        else
        if(pDof && pDofVectorToImage)
        {
            pGrpxSrc->surroundViewEdgeDetectLayoutEnable = FALSE;
            pGrpxSrc->surroundViewDOFLayoutEnable = TRUE;
            pGrpxSrc->surroundViewPdTsrLayoutEnable = FALSE;
            pGrpxSrc->surroundViewLdLayoutEnable = FALSE;
        }
        else
        if(Alg_ObjectDetectionPrm && Alg_LaneDetectPrm)
        {
            pGrpxSrc->surroundViewEdgeDetectLayoutEnable = FALSE;
            pGrpxSrc->surroundViewDOFLayoutEnable = FALSE;
            pGrpxSrc->surroundViewPdTsrLayoutEnable = TRUE;
            pGrpxSrc->surroundViewLdLayoutEnable = TRUE;
        }
    }

    if(pSvDisplay||pSvOrgDisplay||pFrontCamDisplay||pGrpxDisplay)
    {
        ChainsCommon_SurroundView_SetDisplayPrm(
                            pSvDisplay,
                            pSvOrgDisplay,
                            pFrontCamDisplay,
                            pGrpxDisplay,
                            displayWidth,
                            displayHeight
                        );

        ChainsCommon_StartDisplayCtrl(
            displayType,
            displayWidth,
            displayHeight
            );
    }

    if (VPE_algPdPrm)
    {
        ChainsCommon_SurroundView_SetFCVpePrm(
                        VPE_algPdPrm,
                        FEATUREPLANE_ALG_WIDTH,
                        FEATUREPLANE_ALG_HEIGHT,
                        LVDS_CAPTURE_WIDTH,
                        LVDS_CAPTURE_HEIGHT,
                        SYSTEM_DF_YUV420SP_UV
                    );
    }

    if (Alg_FeaturePlaneComputationPrm && Alg_ObjectDetectionPrm )
    {
        ChainsCommon_SurroundView_SetFeaturePlaneComputeAlgPrms(
                        Alg_FeaturePlaneComputationPrm,
                        FEATUREPLANE_ALG_WIDTH,
                        FEATUREPLANE_ALG_HEIGHT
                    );

        ChainsCommon_SurroundView_SetObjectDetectPrm(
                        Alg_ObjectDetectionPrm
                    );

        ChainsCommon_SurroundView_SetSyncFcPrm(Sync_algPdPrm);

        if (Alg_ObjectDrawPrm)
        {
            ChainsCommon_SurroundView_SetObjectDrawPrms(
                            Alg_ObjectDrawPrm,
                            FEATUREPLANE_ALG_WIDTH,
                            FEATUREPLANE_ALG_HEIGHT
                        );
        }
    }

    if (Alg_LaneDetectPrm && Alg_LaneDetectDrawPrm)
    {
        ChainsCommon_SurroundView_SetSyncFcPrm(Sync_algLdPrm);

        AlgorithmLink_LaneDetect_Init(Alg_LaneDetectPrm);
        AlgorithmLink_LaneDetectDraw_Init(Alg_LaneDetectDrawPrm);

        ChainsCommon_SurroundView_SetLaneDetectPrm(
            Alg_LaneDetectPrm,
            Alg_LaneDetectDrawPrm,
            0,
            0,
            FEATUREPLANE_ALG_WIDTH,
            FEATUREPLANE_ALG_HEIGHT
            );
    }
}

Void ChainsCommon_SurroundView_CalibDeInit()
{
    ChainsCommon_SurroundView_freeGATbl(
        &gChains_surroundViewCalibInfo
        );
}
