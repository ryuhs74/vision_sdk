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
 * \file uFusionLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for ultrasonic Fusion Link
 *
 * \version 0.0 (July 2014) : [PS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "svAlgLink_priv.h"
#include "uFusionLink_priv.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>

#define CAPTURE_DELAY 2

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of uFusion algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusion_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_UltrasonicFusionCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_UltrasonicFusionProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_UltrasonicFusionControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_UltrasonicFusionStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_UltrasonicFusionDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_ULTRASONICFUSION;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for geometric alignment alg link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusionCreate(void * pObj, void * pCreateParams)
{
    void                       * algHandle;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    Int32                        numInputQUsed;
    Int32                        numOutputQUsed;
    AlgLink_MemRequests          memRequests;

    AlgorithmLink_UltrasonicFusionObj              * pUltrasonicFusionObj;
    AlgorithmLink_UltrasonicFusionCreateParams     * pUltrasonicFusionLinkCreateParams;
    SV_UFusion_CreationParamsStruct       * pAlgCreateParams;
    AlgorithmLink_InputQueueInfo         * pInputQInfo;
    UInt32                                 memTabId;

    pUltrasonicFusionLinkCreateParams =
        (AlgorithmLink_UltrasonicFusionCreateParams *)pCreateParams;

    /*
     * Space for Algorithm specific object gets allocated here.
     * Pointer gets recorded in algorithmParams
     */
    if(sizeof(AlgorithmLink_UltrasonicFusionObj) > SV_ALGLINK_SRMEM_THRESHOLD)
    {
        pUltrasonicFusionObj = (AlgorithmLink_UltrasonicFusionObj *)
                        Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_SR,
                                       sizeof(AlgorithmLink_UltrasonicFusionObj), 32);
    }
    else
    {
        pUltrasonicFusionObj = (AlgorithmLink_UltrasonicFusionObj *)
                        malloc(sizeof(AlgorithmLink_UltrasonicFusionObj));
    }

    UTILS_assert(pUltrasonicFusionObj!=NULL);
    memset(pUltrasonicFusionObj, 0, sizeof(*pUltrasonicFusionObj));


    pAlgCreateParams = &pUltrasonicFusionObj->algCreateParams;

    pInputQInfo  = &pUltrasonicFusionObj->inputQInfo[0];

    AlgorithmLink_setAlgorithmParamsObj(pObj, pUltrasonicFusionObj);

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy((void*)(&pUltrasonicFusionObj->algLinkCreateParams),
           (void*)(pUltrasonicFusionLinkCreateParams),
           sizeof(AlgorithmLink_UltrasonicFusionCreateParams)
          );

    /*
     * Populating parameters corresponding to Q usage of ultrasonic Fusion
     * algorithm link
     */
    numInputQUsed     = ALGLINK_ULTRASONICFUSION_IPQID_MAXIPQ;

    pInputQInfo[ALGLINK_ULTRASONICFUSION_IPQID_MULTISENSOR].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pInputQInfo[ALGLINK_ULTRASONICFUSION_IPQID_UCLUT].qMode =
        ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    //Set output Queue Info
    //======================================
    AlgorithmLink_UltrasonicFusionOutputQueId      outputQId;
    AlgorithmLink_OutputQueueInfo                * pOutputQInfo;
    pOutputQInfo = &pUltrasonicFusionObj->outputQInfo[0];
    numOutputQUsed    = ALGLINK_ULTRASONICFUSION_OPQID_MAXOPQ;

    //ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA
    outputQId = ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA;
    pOutputQInfo[outputQId].qMode = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo[outputQId].queInfo.numCh = 1; //# of channels used
    pOutputQInfo[outputQId].queInfo.chInfo[0].flags = 0;
    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(pObj,
                                numInputQUsed,
                                pInputQInfo,
                                numOutputQUsed,
                                pOutputQInfo
                                );
    /*
     * Algorithm creation happens here
     * - Population of create time parameters
     * - Create call for algorithm
     * - Algorithm handle gets recorded inside link object
     */


       //Creation parameters for Ultrasonic Fusion
    pAlgCreateParams->numUSensors =
        pUltrasonicFusionLinkCreateParams->numUltrasonic;
    pAlgCreateParams->numViews =
        pUltrasonicFusionLinkCreateParams->numViews;
    pAlgCreateParams->SVOutDisplayHeight = SV_ALGLINK_OUTPUT_FRAME_HEIGHT;
    pAlgCreateParams->SVOutDisplayWidth = SV_ALGLINK_OUTPUT_FRAME_WIDTH;

    pAlgCreateParams->numArcPoints = ALGLINK_ULTRASONICFUSION_NUM_ARC_POINTS; //number of arc points to draw
    pAlgCreateParams->showSensorPosition = 0; //show sensor position in SV?
    pAlgCreateParams->lineColor = 0; //color of lines for arc overlay, 0 -red;
    pAlgCreateParams->lineSize = 200; //size of lines for arc overlay
    pAlgCreateParams->sensorPositionLineLength = 30;//line distance for sensor position overlay


    /*
     * First time call is just to get size for algorithm handle.
     *
     * TBD - Currently since memquery function is dependent on alg handle
     * space, there are two calls - first for alg handle and then for other
     * requests. In future, once this dependency is removed, there will be
     * only call of MemQuery
     */
    Alg_UltrasonicFusionMemQuery(pAlgCreateParams, &memRequests, 1);
    memTabId = 0;
    memRequests.memTab[memTabId].basePtr = malloc(
                                            memRequests.memTab[memTabId].size);
    UTILS_assert(memRequests.memTab[memTabId].basePtr != NULL);

    /*
     * Memory allocations for the requests done by algorithm
     * For now treating all requests as persistent and allocating in DDR
     */
    Alg_UltrasonicFusionMemQuery(pAlgCreateParams, &memRequests, 0);
    for(memTabId = 1 ; memTabId < memRequests.numMemTabs ; memTabId++)
    {
        if(memRequests.memTab[memTabId].size > 0)
        {
            if(memRequests.memTab[memTabId].size > SV_ALGLINK_SRMEM_THRESHOLD)
            {
                memRequests.memTab[memTabId].basePtr = Utils_memAlloc(
                                            UTILS_HEAPID_DDR_CACHED_SR,
                                            memRequests.memTab[memTabId].size,
                                            memRequests.memTab[memTabId].alignment);
            }
            else
            {
                memRequests.memTab[memTabId].basePtr =
                    malloc(memRequests.memTab[memTabId].size);
            }

            UTILS_assert(memRequests.memTab[memTabId].basePtr != NULL);
        }
    }

    algHandle = Alg_UltrasonicFusionCreate(pAlgCreateParams, &memRequests);
    UTILS_assert(algHandle != NULL);

    pUltrasonicFusionObj->algHandle = algHandle;

    //Initialize Output Buffers
    //============================================
    //- Connecting metadata buffer to system buffer payload
    //- Memory allocation for buffers
    //- Put the buffer into empty queue
    System_Buffer              * outputBuffer;
    System_MetaDataBuffer      * outputBuffer_payload;
    UInt32 metaBufSize;
    Int32 bufferId;

    for (bufferId=0; bufferId < ULTRASONICFUSION_LINK_MAX_NUM_OUTPUT; bufferId++)
    {
        //ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA
        outputQId = ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA;
        outputBuffer = &(pUltrasonicFusionObj->buffers[outputQId][bufferId]);
        outputBuffer_payload = &(pUltrasonicFusionObj->metaBuffers[outputQId][bufferId]);

        outputBuffer->payload     = outputBuffer_payload;
        outputBuffer->payloadSize = sizeof(System_MetaDataBuffer);
        outputBuffer->bufType     = SYSTEM_BUFFER_TYPE_METADATA;
        outputBuffer->chNum       = 0;

        outputBuffer_payload->numMetaDataPlanes = 1;
        outputBuffer_payload->flags             = 0;

        //YUV
        metaBufSize = SV_ALGLINK_UF_OVERLAYDATA_SIZE;
        outputBuffer_payload->bufAddr[0] =  Utils_memAlloc(
                                            UTILS_HEAPID_DDR_CACHED_SR,
                                            metaBufSize,
                                            ALGORITHMLINK_FRAME_ALIGN);
        outputBuffer_payload->metaBufSize[0]    = metaBufSize;
        outputBuffer_payload->metaFillLength[0] = metaBufSize;
        outputBuffer_payload->flags             = 0;
        UTILS_assert(outputBuffer_payload->bufAddr[0] != NULL);

        AlgorithmLink_putEmptyOutputBuffer(pObj, outputQId, outputBuffer);
    }

    //Other Initializations
    //========================================
    pUltrasonicFusionObj->frameDropCounter          = 0;

    pUltrasonicFusionObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj),"ALG_ULTRASONIC_FUSION");
    UTILS_assert(NULL != pUltrasonicFusionObj->linkStatsInfo);

    pUltrasonicFusionObj->numInputChannels = 1;
    pUltrasonicFusionObj->frmCnt = 0;
    pUltrasonicFusionObj->isFirstFrameRecv   = FALSE;
    pUltrasonicFusionObj->receivedUCLUTFlag  = FALSE;
    pUltrasonicFusionObj->isFirstOPGenerated = FALSE;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process Plugin of ultrasonic Fusion algorithm link
 *
 *        This function and the algorithm process function execute
 *        on same processor core. Hence processor gets
 *        locked with execution of the function, until completion. Only a
 *        link with higher priority can pre-empt this function execution.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusionProcess(void * pObj)
{
    AlgorithmLink_UltrasonicFusionObj * pUltrasonicFusionObj;
    void                       * algHandle;
    AlgorithmLink_UltrasonicFusionInputQueId    inputQId;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    Int32                        inputStatus;
    UInt32                       bufId;
    System_BufferList            inputBufList;
    System_BufferList            inputBufListReturn;
    System_Buffer              * pSystemBufferMultisensor;
    Bool                         bufDropFlag;
    System_MetaDataBuffer      * pMultisensorBuffer;
    System_MetaDataBuffer      * pinUCLUTBuffer;
    UInt32                       channelId;
    System_Buffer              * outputBuffer;
    System_MetaDataBuffer      * outputBuffer_payload;
    System_BufferList outputBufListReturn;
    System_LinkStatistics      * linkStatsInfo;
    AlgorithmLink_UltrasonicFusionOutputQueId outputQId;

    AlgorithmLink_UltrasonicFusionCreateParams  * pUltrasonicFusionLinkCreateParams;
    //Vps_printf("\n ultrasonic Fusion link test");

    //Vps_printf("~~~~~~~~~~ Entering UFUSION Process ~~~~~~~~~~~~~"); //MM

    pUltrasonicFusionObj = (AlgorithmLink_UltrasonicFusionObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    linkStatsInfo = pUltrasonicFusionObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    algHandle               = pUltrasonicFusionObj->algHandle;
    pUltrasonicFusionLinkCreateParams = (AlgorithmLink_UltrasonicFusionCreateParams *)
                                    &pUltrasonicFusionObj->algLinkCreateParams;

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    if(pUltrasonicFusionObj->isFirstFrameRecv==FALSE)
    {
        pUltrasonicFusionObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(
            &linkStatsInfo->linkStats,
            pUltrasonicFusionObj->numInputChannels,
            1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }

    linkStatsInfo->linkStats.newDataCmdCount++;

  /*
     * Get Input buffers from previous link for
     * Qid = ALGLINK_ULTRASONICFUSION_IPQID_UCLUT and store latest copy locally.
     */
    inputQId = ALGLINK_ULTRASONICFUSION_IPQID_UCLUT;

    System_getLinksFullBuffers(
        pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkId,
        pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    //Vps_printf("PixelsPerCm # input buffers: %d \n",inputBufList.numBuf); //MM

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            /*
             * At any point in time, Ultrasonic fusion link will hold only one UC LUT.
             * So whenever UC LUT is received, the previously received one
             * will be released and the newly received one will be archived.
             */
            if(pUltrasonicFusionObj->receivedUCLUTFlag == TRUE)
            {
                inputBufListReturn.numBuf     = 1;
                inputBufListReturn.buffers[0] = pUltrasonicFusionObj->sysBufferUCLUT;
                bufDropFlag = FALSE;

                AlgorithmLink_releaseInputBuffer(
                    pObj,
                    inputQId,
                    pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkId,
                    pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
                    &inputBufListReturn,
                    &bufDropFlag);

                pUltrasonicFusionObj->receivedUCLUTFlag = FALSE;
            }

            pUltrasonicFusionObj->sysBufferUCLUT = inputBufList.buffers[bufId];
            /*TBD: Check for parameter correctness. If in error, return input*/
            pUltrasonicFusionObj->receivedUCLUTFlag = TRUE;
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = ALGLINK_ULTRASONICFUSION_IPQID_MULTISENSOR and process them if output and UCLUT
     * are available.
     */
    inputQId = ALGLINK_ULTRASONICFUSION_IPQID_MULTISENSOR;

    System_getLinksFullBuffers(
        pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkId,
        pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    //Vps_printf("Ultrasonic Capture # input buffers: %d \n",inputBufList.numBuf); //MM

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSystemBufferMultisensor = inputBufList.buffers[bufId];
            UTILS_assert(pSystemBufferMultisensor != NULL);
            pMultisensorBuffer = (System_MetaDataBuffer *)
                                (pSystemBufferMultisensor->payload);
            bufDropFlag = TRUE;
            /*TBD: Check for parameter correctness. If in error, return input*/
            inputStatus = SYSTEM_LINK_STATUS_SOK;

            if(pUltrasonicFusionObj->receivedUCLUTFlag == TRUE &&
                inputStatus == SYSTEM_LINK_STATUS_SOK)
            {
                //============================================
                //ALGLINK_ULTRASONICFUSION_OPQID_OVERLAY_YUV
                //=============================================
                outputQId = ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA;
                channelId = 0;
                //request empty buffer from framework
                status = AlgorithmLink_getEmptyOutputBuffer(
                                                        pObj,
                                                        outputQId,
                                                        channelId,
                                                        &outputBuffer);

                if(status != SYSTEM_LINK_STATUS_SOK)
                     linkStatsInfo->linkStats.outBufErrorCount++;
                else
                {
                      //set local pointer to output payload
                      outputBuffer->srcTimestamp = pSystemBufferMultisensor->srcTimestamp;
                      outputBuffer->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();
                      outputBuffer_payload = (System_MetaDataBuffer *)outputBuffer->payload;

                      if(pUltrasonicFusionObj->receivedUCLUTFlag == TRUE)
                          pinUCLUTBuffer = (System_MetaDataBuffer *)pUltrasonicFusionObj->
                            sysBufferUCLUT->payload;
                      else
                          pinUCLUTBuffer->bufAddr[0]=NULL;

                      Cache_inv(pMultisensorBuffer->bufAddr[0],
						 pMultisensorBuffer->metaBufSize[0],
                         Cache_Type_ALLD, TRUE);

                      if(pUltrasonicFusionObj->receivedUCLUTFlag == TRUE)
                          Cache_inv(pinUCLUTBuffer->bufAddr[0],
                                    SV_ALGLINK_GA_PIXELSPERCM_SIZE,
                                    Cache_Type_ALLD, TRUE);

                      status = Alg_UltrasonicFusionProcess(
                                           algHandle,
                                           pMultisensorBuffer->bufAddr[0],
                                           pinUCLUTBuffer->bufAddr[0],
                                           &pUltrasonicFusionObj->ultrasonicResults,
                                           outputBuffer_payload->bufAddr[0]
                                           );

                      UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                      Cache_wb(outputBuffer_payload->bufAddr[0],
                               outputBuffer_payload->metaBufSize[0],
                               Cache_Type_ALLD,
                               TRUE);

                      Utils_updateLatency(&linkStatsInfo->linkLatency,
                                    outputBuffer->linkLocalTimestamp);
                      Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                    outputBuffer->srcTimestamp);

                      if (pSystemBufferMultisensor != NULL)
                      {
                        linkStatsInfo->linkStats.chStats
                                    [pSystemBufferMultisensor->chNum].inBufProcessCount++;
                        linkStatsInfo->linkStats.chStats
                                    [pSystemBufferMultisensor->chNum].outBufCount[0]++;
                      }

                      /*
                       * Informing next link that a new data has peen put for its
                       * processing
                       */
                      // ALGLINK_ULTRASONICFUSION_OPQID_OVERLAY_YUV
                      //-----------------------------------------------
                      outputQId = ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA;
                      //tell framework that output buffer is filled
                      status    = AlgorithmLink_putFullOutputBuffer(pObj,
                                                                    outputQId,
                                                                    outputBuffer);
                      UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                      //tell next link that data is ready
                      System_sendLinkCmd(
                            pUltrasonicFusionLinkCreateParams->outQueParams[outputQId].nextLink,
                            SYSTEM_CMD_NEW_DATA,
                            NULL);

                      //release output buffer
                      // (note: buffer is freed only when next also frees the buffer)
                      outputBufListReturn.numBuf     = 1;
                      outputBufListReturn.buffers[0] = outputBuffer;
                      AlgorithmLink_releaseOutputBuffer(pObj,
                                                        outputQId,
                                                        &outputBufListReturn);

                      bufDropFlag = FALSE;

                }
            }

            inputQId                      = ALGLINK_ULTRASONICFUSION_IPQID_MULTISENSOR;
            inputBufListReturn.numBuf     = 1;
            inputBufListReturn.buffers[0] = pSystemBufferMultisensor;
            AlgorithmLink_releaseInputBuffer(
                pObj,
                inputQId,
                pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkId,
                pUltrasonicFusionLinkCreateParams->inQueParams[inputQId].prevLinkQueId,
                &inputBufListReturn,
                &bufDropFlag);
        }
    }
    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Control Plugin for ultrasonic Fusion algorithm link
 *
 *
 * \param  pObj               [IN] Algorithm link object handle
 * \param  pControlParams     [IN] Pointer to control parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

Int32 AlgorithmLink_UltrasonicFusionControl(void * pObj, void * pControlParams)
{
    AlgorithmLink_UltrasonicFusionObj        * pUltrasonicFusionObj;
    AlgorithmLink_ControlParams    * pAlgLinkControlPrm;
    void                           * algHandle;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pUltrasonicFusionObj = (AlgorithmLink_UltrasonicFusionObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);
    algHandle  = pUltrasonicFusionObj->algHandle;

    pAlgLinkControlPrm = (AlgorithmLink_ControlParams *)pControlParams;

    /*
     * There can be other commands to alter the properties of the alg link
     * or properties of the core algorithm.
     * In this simple example, there is just a control command to print
     * statistics and a default call to algorithm control.
     */
    switch(pAlgLinkControlPrm->controlCmd)
    {

        case SYSTEM_CMD_PRINT_STATISTICS:
            AlgorithmLink_UltrasonicFusionPrintStatistics(pObj, pUltrasonicFusionObj);
            break;

        case ALGLINK_ULTRASONICFUSION_CONFIG_CMD_GET_RESULTS:
        {
            AlgorithmLink_UltrasonicFusionGetResults *pResults;

            UTILS_assert(pAlgLinkControlPrm->size == sizeof(*pResults));

            pResults = (AlgorithmLink_UltrasonicFusionGetResults*)pAlgLinkControlPrm;

            memcpy(&pResults->results,
                &pUltrasonicFusionObj->ultrasonicResults,
                sizeof(pUltrasonicFusionObj->ultrasonicResults)
                );
        }
            break;

        default:
            status = Alg_UltrasonicFusionControl(algHandle,
                                                  &(pUltrasonicFusionObj->controlParams)
                                                  );
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop Plugin for ultrasonic Fusion algorithm link
 *
 *        For this algorithm there is no locking of frames and hence no
 *        flushing of frames. Also there are no any other functionality to be
 *        done at the end of execution of this algorithm. Hence this function
 *        is an empty function for this algorithm.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusionStop(void * pObj)
{
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete Plugin for UltrasonicFusion algorithm link
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusionDelete(void * pObj)
{
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    AlgLink_MemRequests          memRequests;
    AlgorithmLink_UltrasonicFusionObj           * pUltrasonicFusionObj;
    UInt32                              memTabId;

    pUltrasonicFusionObj = (AlgorithmLink_UltrasonicFusionObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    Alg_UltrasonicFusionDelete(pUltrasonicFusionObj->algHandle, &memRequests);

    status = Utils_linkStatsCollectorDeAllocInst(pUltrasonicFusionObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);


    /*
     * Memory allocations for the requests done by algorithm
     */
    for(memTabId = 0 ; memTabId < memRequests.numMemTabs ; memTabId++)
    {
        if(memTabId==0)
        {
            /* memTabId 0 is alloacted via 'malloc' */
            free(memRequests.memTab[memTabId].basePtr);
        }
        else
        if(memRequests.memTab[memTabId].size > 0)
        {
            if(memRequests.memTab[memTabId].size > SV_ALGLINK_SRMEM_THRESHOLD)
            {
                status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                                       memRequests.memTab[memTabId].basePtr,
                                       memRequests.memTab[memTabId].size);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }
            else
            {
                free(memRequests.memTab[memTabId].basePtr);
            }
        }
    }

    /*======================================================================
    * Deletion of output buffers
     ======================================================================*/
    //UInt32                                  metaBufSize;
    System_MetaDataBuffer                   * outputBuffer_payload;
    AlgorithmLink_UltrasonicFusionOutputQueId   outputQId;
    Int32 bufferId;

    //ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA
    //------------------------------------------------
    outputQId = ALGLINK_ULTRASONICFUSION_OPQID_OVERLAYDATA;

    for (bufferId = 0; bufferId < ULTRASONICFUSION_LINK_MAX_NUM_OUTPUT; bufferId++)
    {
        outputBuffer_payload = &(pUltrasonicFusionObj->metaBuffers[outputQId][bufferId]);

        status =  Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                                    outputBuffer_payload->bufAddr[0],
                                    SV_ALGLINK_UF_OVERLAYDATA_SIZE);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }


    /*
     * Space for Algorithm specific object gets freed here.
     */
    if(sizeof(AlgorithmLink_UltrasonicFusionObj) > SV_ALGLINK_SRMEM_THRESHOLD)
    {
        status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                               pUltrasonicFusionObj,
                               sizeof(AlgorithmLink_UltrasonicFusionObj));
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }
    else
    {
        free(pUltrasonicFusionObj);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Print link statistics
 *
 * \param  pObj                [IN] Algorithm link object handle
 * \param  pUltrasonicFusionObj       [IN] Frame copy link Object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_UltrasonicFusionPrintStatistics(void *pObj,
                       AlgorithmLink_UltrasonicFusionObj *pUltrasonicFusionObj)
{
    UTILS_assert(NULL != pUltrasonicFusionObj->linkStatsInfo);

    Utils_printLinkStatistics(&pUltrasonicFusionObj->linkStatsInfo->linkStats, "ALG_ULTRASONICFUSION", TRUE);

    Utils_printLatency("ALG_ULTRASONICFUSION",
                       &pUltrasonicFusionObj->linkStatsInfo->linkLatency,
                       &pUltrasonicFusionObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
