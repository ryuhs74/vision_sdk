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
 * \file fullViewLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for frame copy Link
 *
 * \version 0.0 (Aug 2013) : [PS] First version
 * \version 0.1 (Aug 2013) : [PS] Changes to make plug in functions not rely
 *                                on the elements of Algorithm link object.
 *                                Necessary functionality is accomplished via
 *                                API call backs to link skeletal implementation
 * \version 0.2 (Sept 2013) : [PS] Added code for multi channel support
 * \version 0.3 (Sept 2013) : [PS] Added code for stats collector
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "fullViewLink_priv.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of frame copy algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_FullView_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_fullViewCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_fullViewProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_fullViewControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_fullViewStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_fullViewDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_FULLVIEW;
#endif

#ifdef BUILD_ARP32
    algId = ALGORITHM_LINK_EVE_ALG_FULLVIEW;
#endif

#ifdef BUILD_A15
    algId = ALGORITHM_LINK_A15_ALG_FULLVIEW;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for frame copy algorithm link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_fullViewCreate(void * pObj, void * pCreateParams)
{
    Alg_FullView_Obj          * algHandle;
    Int32                        frameIdx;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       maxHeight;
    UInt32                       maxWidth;
    System_Buffer              * pSystemBuffer;
    System_VideoFrameBuffer    * pSystemVideoFrameBuffer;
    System_LinkInfo              prevLinkInfo;
    Int32                        outputQId;
    Int32                        channelId;
    Int32                        numChannelsUsed;
    Int32                        numInputQUsed;
    Int32                        numOutputQUsed;
    UInt32                       prevLinkQueId;
    UInt32                       dataFormat;
    System_LinkChInfo          * pOutChInfo;
    System_LinkChInfo          * pPrevChInfo;
    UInt32                       prevChInfoFlags;

    AlgorithmLink_FullViewObj          * pFullViewObj;
    AlgorithmLink_FullViewCreateParams * pFullViewCreateParams;
    AlgorithmLink_OutputQueueInfo       * pOutputQInfo;
    AlgorithmLink_InputQueueInfo        * pInputQInfo;


    pFullViewCreateParams =
        (AlgorithmLink_FullViewCreateParams *)pCreateParams;

    /*
     * Space for Algorithm specific object gets allocated here.
     * Pointer gets recorded in algorithmParams
     */
    pFullViewObj = (AlgorithmLink_FullViewObj *)
                        Utils_memAlloc(UTILS_HEAPID_DDR_CACHED_LOCAL,
                                       sizeof(AlgorithmLink_FullViewObj), 32);

    UTILS_assert(pFullViewObj!=NULL);

    pOutputQInfo = &pFullViewObj->outputQInfo;
    pInputQInfo  = &pFullViewObj->inputQInfo;

    AlgorithmLink_setAlgorithmParamsObj(pObj, pFullViewObj);

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    pFullViewObj->algLinkCreateParams.maxHeight =
        pFullViewCreateParams->maxHeight;
    pFullViewObj->algLinkCreateParams.maxWidth  =
        pFullViewCreateParams->maxWidth;
    pFullViewObj->algLinkCreateParams.numOutputFrames =
        pFullViewCreateParams->numOutputFrames;
    pFullViewObj->pLUT = pFullViewCreateParams->pLut;


    Vps_printf( "LUT[%X] [%X] [%X]\n", pFullViewObj->pLUT[0],pFullViewObj->pLUT[1],pFullViewObj->pLUT[2]);

    memcpy((void*)(&pFullViewObj->outQueParams),
           (void*)(&pFullViewCreateParams->outQueParams),
           sizeof(System_LinkOutQueParams));
    memcpy((void*)(&pFullViewObj->inQueParams),
           (void*)(&pFullViewCreateParams->inQueParams),
           sizeof(System_LinkInQueParams));

    /*
     * Populating parameters corresponding to Q usage of frame copy
     * algorithm link
     */
    numInputQUsed     = 1;
    numOutputQUsed    = 1;
    numChannelsUsed   = 1;
    pInputQInfo->qMode  = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo->qMode = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    outputQId                 = 0;
    pOutputQInfo->queInfo.numCh = numChannelsUsed;

    /*
     * Channel info of current link will be obtained from previous link.
     * If any of the properties get changed in the current link, then those
     * values need to be updated accordingly in
     * pOutputQInfo->queInfo.chInfo[channelId]
     * In frame copy example, only pitch changes. Hence only it is
     * updated. Other parameters are copied from prev link.
     */
    status = System_linkGetInfo(pFullViewCreateParams->inQueParams.prevLinkId,
                                &prevLinkInfo);

    prevLinkQueId = pFullViewCreateParams->inQueParams.prevLinkQueId;

    pFullViewObj->numInputChannels = prevLinkInfo.queInfo[prevLinkQueId].numCh;

    maxHeight = pFullViewObj->algLinkCreateParams.maxHeight;
    maxWidth  = pFullViewObj->algLinkCreateParams.maxWidth;

    /*
     * Make pitch a multiple of ALGORITHMLINK_FRAME_ALIGN, so that if the frame
     * origin is aligned, then individual lines are also aligned
     * Also note that the pitch is kept same independent of width of
     * individual channels
     */
    pFullViewObj->pitch = maxWidth;
    if(maxWidth % ALGORITHMLINK_FRAME_ALIGN)
    {
        pFullViewObj->pitch += (ALGORITHMLINK_FRAME_ALIGN -
                                (maxWidth % ALGORITHMLINK_FRAME_ALIGN));
    }

    /*
     * Channel Info Population
     */
    for(channelId =0 ; channelId < pFullViewObj->numInputChannels; channelId++)
    {

      pOutChInfo      = &(pOutputQInfo->queInfo.chInfo[channelId]);
      pPrevChInfo     = &(prevLinkInfo.queInfo[prevLinkQueId].chInfo[channelId]);
      prevChInfoFlags = pPrevChInfo->flags;

      /*
       * Certain channel info parameters simply get defined by previous link
       * channel info. Hence copying them to output channel info
       */
      pOutChInfo->startX = pPrevChInfo->startX;
      pOutChInfo->startY = pPrevChInfo->startY;
      pOutChInfo->width  = pPrevChInfo->width;
      pOutChInfo->height = pPrevChInfo->height;
      pOutChInfo->flags  = prevChInfoFlags;

      dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(prevChInfoFlags);

      if((dataFormat != SYSTEM_DF_YUV422I_YUYV)
         &&
         (dataFormat != SYSTEM_DF_YUV420SP_UV)
        )
      {
        return SYSTEM_LINK_STATUS_EFAIL;
      }

      if(pPrevChInfo->width > maxWidth || pPrevChInfo->height > maxHeight)
      {
        return SYSTEM_LINK_STATUS_EFAIL;
      }

      /*
       * Certain channel info parameters are properties of the current link,
       * They are set here.
       */
      pOutChInfo->pitch[0] = pFullViewObj->pitch;
      pOutChInfo->pitch[1] = pFullViewObj->pitch;
      pOutChInfo->pitch[2] = pFullViewObj->pitch;

      if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
      {
        pOutChInfo->pitch[0] = pFullViewObj->pitch * 2;
      }

      /*
       * Taking a copy of input channel info in the link object for any future
       * use
       */
      memcpy((void *)&(pFullViewObj->inputChInfo[channelId]),
             (void *)&(prevLinkInfo.queInfo[prevLinkQueId].chInfo[channelId]),
             sizeof(System_LinkChInfo)
            );
    }

    /*
     * If any output buffer Q gets used in INPLACE manner, then
     * pOutputQInfo->inQueParams and
     * pOutputQInfo->inputQId need to be populated appropriately.
     */

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

    pFullViewObj->createParams.maxHeight    = maxHeight;
    pFullViewObj->createParams.maxWidth     = maxWidth;
    pFullViewObj->frameDropCounter          = 0;

    algHandle = Alg_FullViewCreate(&pFullViewObj->createParams);
    UTILS_assert(algHandle != NULL);

    pFullViewObj->algHandle = algHandle;

    /*
     * Creation of output buffers for output buffer Q = 0 (Used)
     *  - Connecting video frame buffer to system buffer payload
     *  - Memory allocation for Luma and Chroma buffers (Assume 420 format)
     *  - Put the buffer into empty queue
     */
    outputQId = 0;

    for(channelId =0 ; channelId < pFullViewObj->numInputChannels; channelId++)
    {
      for(frameIdx = 0;
          frameIdx < pFullViewObj->algLinkCreateParams.numOutputFrames;
          frameIdx++)
      {
        pSystemBuffer           =
                             &(pFullViewObj->buffers[channelId][frameIdx]);
        pSystemVideoFrameBuffer =
                             &(pFullViewObj->videoFrames[channelId][frameIdx]);

        /*
         * Properties of pSystemBuffer, which do not get altered during
         * run time (frame exchanges) are initialized here
         */
        pSystemBuffer->payload     = pSystemVideoFrameBuffer;
        pSystemBuffer->payloadSize = sizeof(System_VideoFrameBuffer);
        pSystemBuffer->bufType     = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
        pSystemBuffer->chNum       = channelId;

        memcpy((void *)&pSystemVideoFrameBuffer->chInfo,
               (void *)&pOutputQInfo->queInfo.chInfo[channelId],
               sizeof(System_LinkChInfo));

        /*
         * Buffer allocation done for maxHeight, maxWidth and also assuming
         * worst case num planes = 2, for data Format SYSTEM_DF_YUV422I_YUYV
         * run time (frame exchanges) are initialized here
         */
        pSystemVideoFrameBuffer->bufAddr[0] = Utils_memAlloc(
                                           UTILS_HEAPID_DDR_CACHED_SR,
                                           (maxHeight*(pFullViewObj->pitch)*2),
                                           ALGORITHMLINK_FRAME_ALIGN);

        /*
         * Carving out memory pointer for chroma which will get used in case of
         * SYSTEM_DF_YUV422SP_UV
         */
        pSystemVideoFrameBuffer->bufAddr[1] = (void*)(
            (UInt32) pSystemVideoFrameBuffer->bufAddr[0] +
            (UInt32)(maxHeight*(pFullViewObj->pitch))
            );

        UTILS_assert(pSystemVideoFrameBuffer->bufAddr[0] != NULL);

        AlgorithmLink_putEmptyOutputBuffer(pObj, outputQId, pSystemBuffer);
      }
    }

    pFullViewObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_FRAME_COPY");
    UTILS_assert(NULL != pFullViewObj->linkStatsInfo);

    pFullViewObj->isFirstFrameRecv = FALSE;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process Plugin for frame copy algorithm link
 *
 *        This function executes on the DSP or EVE or IPU or A15 processor.
 *        Hence processor gets
 *        locked with execution of the function, until completion. Only a
 *        link with higher priority can pre-empt this function execution.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_fullViewProcess(void * pObj)
{
    AlgorithmLink_FullViewObj * pFullViewObj;
    Alg_FullView_Obj          * algHandle;
    Int32                        inputQId;
    Int32                        outputQId;
    UInt32                       channelId;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       bufId;
    System_BufferList            inputBufList;
    System_BufferList            inputBufListReturn;
    System_BufferList            outputBufListReturn;
    System_Buffer              * pSysBufferInput;
    System_VideoFrameBuffer    * pSysVideoFrameBufferInput;
    System_Buffer              * pSysBufferOutput;
    System_VideoFrameBuffer    * pSysVideoFrameBufferOutput;
    UInt32                       dataFormat;
    UInt32                       outPitch[SYSTEM_MAX_PLANES];
    UInt32                       bufSize[SYSTEM_MAX_PLANES];
    UInt32                       bufCntr;
    UInt32                       numBuffs;
    System_LinkChInfo          * pInputChInfo;
    System_LinkChInfo          * pOutputChInfo;
    Bool                         bufDropFlag;
    AlgorithmLink_ViewMode       viewMode;
    System_LinkStatistics      * linkStatsInfo;


    pFullViewObj = (AlgorithmLink_FullViewObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    linkStatsInfo = pFullViewObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    algHandle     = pFullViewObj->algHandle;

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    /*
     * Getting input buffers from previous link
     */
    System_getLinksFullBuffers(pFullViewObj->inQueParams.prevLinkId,
                               pFullViewObj->inQueParams.prevLinkQueId,
                               &inputBufList);

    if(inputBufList.numBuf)
    {
        if(pFullViewObj->isFirstFrameRecv==FALSE)
        {
            pFullViewObj->isFirstFrameRecv = TRUE;

            Utils_resetLinkStatistics(
                    &linkStatsInfo->linkStats,
                    pFullViewObj->numInputChannels,
                    1);

            Utils_resetLatency(&linkStatsInfo->linkLatency);
            Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
        }

        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {

          pSysBufferInput           = inputBufList.buffers[bufId];
          pSysVideoFrameBufferInput = pSysBufferInput->payload;

          channelId = pSysBufferInput->chNum;

          if(channelId < pFullViewObj->numInputChannels)
          {
            linkStatsInfo->linkStats.chStats[channelId].inBufRecvCount++;
          }

          /*
           * Error checks can be done on the input buffer and only later,
           * it can be picked for processing
           */
          if((pSysBufferInput->bufType != SYSTEM_BUFFER_TYPE_VIDEO_FRAME)
             ||
             (channelId >= pFullViewObj->numInputChannels)
            )
          {
            bufDropFlag = TRUE;
            linkStatsInfo->linkStats.inBufErrorCount++;
          }
          else
          {

          /*
           * Getting free (empty) buffers from pool of output buffers
           */
          outputQId = 0;


          status = AlgorithmLink_getEmptyOutputBuffer(pObj,
                                                      outputQId,
                                                      channelId,
                                                      &pSysBufferOutput);

          /*
           * Get into algorithm processing only if an output frame is available.
           * Else input buffer will be returned back to sender and its a case
           * of frame drop.
           */
          if(status == SYSTEM_LINK_STATUS_SOK)
          {

          pSysVideoFrameBufferOutput = pSysBufferOutput->payload;
          pOutputChInfo = &(pFullViewObj->outputQInfo.queInfo.chInfo[channelId]);
          pInputChInfo  = &(pFullViewObj->inputChInfo[channelId]);

          /*
           * If there is any parameter change on the input channel,
           * then, channel info needs to be read from pSysVideoFrameBufferInput.
           * And then,
           *  - Update the local copies present in OutputQInfo and inputChInfo
           *  - Also update channel info in pSysVideoFrameBufferOutput to
           *    pass on new parameters to next link
           */

          if(SYSTEM_LINK_CH_INFO_GET_FLAG_IS_RT_PRM_UPDATE(
                        pSysVideoFrameBufferInput->chInfo.flags))
          {
            pInputChInfo = &(pSysVideoFrameBufferInput->chInfo);

            memcpy(&(pFullViewObj->inputChInfo[channelId]),
                   pInputChInfo,
                   sizeof(System_LinkChInfo));

            memcpy(pOutputChInfo,
                   pInputChInfo,
                   sizeof(System_LinkChInfo));

            dataFormat =
                  SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pInputChInfo->flags);

            /*
             * Upon dataFormat change pitch for plane 0 needs to be updated
             * Plane 1 is used only for 420 SP case and it need not be altered
             */
            pOutputChInfo->pitch[0] = pFullViewObj->pitch;
            if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
            {
                pOutputChInfo->pitch[0] = pFullViewObj->pitch * 2;
            }

            /*
             * Also update the Channel info in Output System Buffer to pass it
             * on to next link
             */
            memcpy(&(pSysVideoFrameBufferOutput->chInfo),
                  pOutputChInfo,
                  sizeof(System_LinkChInfo));
          }
          else
          {
            /*
             * Indicating to next link that there has been no parameter update
             */
            SYSTEM_LINK_CH_INFO_SET_FLAG_IS_RT_PRM_UPDATE(
                        pSysVideoFrameBufferOutput->chInfo.flags,
                        0);
          }

          /*
           * Call to the algorithm
           */
          outPitch[0] = pOutputChInfo->pitch[0];
          outPitch[1] = pOutputChInfo->pitch[1];

          dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(
                        pOutputChInfo->flags);

          /*
           * Default
           */
          viewMode = ALGORITHM_LINK_ALG_CPUFULLVIEW;

#if 0
          switch(System_getSelfProcId())
          {
            case SYSTEM_PROC_A15_0:
                viewMode = ALGORITHM_LINK_ALG_CPUFULLVIEW;
                break;
            case SYSTEM_PROC_DSP1:
            case SYSTEM_PROC_DSP2:
            case SYSTEM_PROC_EVE1:
            case SYSTEM_PROC_EVE2:
            case SYSTEM_PROC_EVE3:
            case SYSTEM_PROC_EVE4:
                viewMode = ALGORITHM_LINK_ALG_DMAFULLVIEW;
                break;
          }
#endif
          switch (dataFormat)
          {
              case SYSTEM_DF_YUV422I_YUYV:
                  numBuffs = 1;
                  break;
              case SYSTEM_DF_YUV420SP_UV:
                  numBuffs = 2;
                  break;
              default:
                  numBuffs = 1;
                  UTILS_assert (0);
                  break;
          }

          pSysBufferOutput->srcTimestamp = pSysBufferInput->srcTimestamp;
          pSysBufferOutput->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

          /*
           * Cache Invalidate of input buffer
           */
          if(viewMode == ALGORITHM_LINK_ALG_CPUFULLVIEW)
          {
            bufSize[0]  = ((pInputChInfo->height)*(pInputChInfo->pitch[0]));
            bufSize[1]  = ((pInputChInfo->height)*(pInputChInfo->pitch[1]));

            for(bufCntr = 0; bufCntr < numBuffs; bufCntr++)
            {
                Cache_inv(pSysVideoFrameBufferInput->bufAddr[bufCntr],
                          bufSize[bufCntr],
                          Cache_Type_ALLD,
                          TRUE
                         );
            }
          }

          Alg_FullViewProcess(algHandle,
                               (UInt32 **)pSysVideoFrameBufferInput->bufAddr,
                               (UInt32 **)pSysVideoFrameBufferOutput->bufAddr,
                               1248,
                               558,
                               pInputChInfo->pitch,
                               outPitch,
                               0,
                               0,
							   pFullViewObj->pLUT
                              );

          /*
           * Cache Write back of output buffer to DDR
           */
          if(viewMode == ALGORITHM_LINK_ALG_CPUFULLVIEW)
          {
            bufSize[0]  = ((pOutputChInfo->height)*(outPitch[0]));
            bufSize[1]  = ((pOutputChInfo->height)*(outPitch[1]));
            for(bufCntr = 0; bufCntr < numBuffs; bufCntr++)
            {
              Cache_wb(pSysVideoFrameBufferOutput->bufAddr[bufCntr],
                       bufSize[bufCntr],
                       Cache_Type_ALLD,
                       TRUE
                      );
            }
          }

          Utils_updateLatency(&linkStatsInfo->linkLatency,
                              pSysBufferOutput->linkLocalTimestamp);
          Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                              pSysBufferOutput->srcTimestamp);

          linkStatsInfo->linkStats.chStats
                    [pSysBufferInput->chNum].inBufProcessCount++;
          linkStatsInfo->linkStats.chStats
                    [pSysBufferInput->chNum].outBufCount[0]++;

          /*
           * Putting filled buffer into output full buffer Q
           * Note that this does not mean algorithm has freed the output buffer
           */
          status = AlgorithmLink_putFullOutputBuffer(pObj,
                                                     outputQId,
                                                     pSysBufferOutput);

          UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

          /*
           * Informing next link that a new data has peen put for its
           * processing
           */
          System_sendLinkCmd(pFullViewObj->outQueParams.nextLink,
                             SYSTEM_CMD_NEW_DATA,
                             NULL);

          /*
           * Releasing (Free'ing) output buffer, since algorithm does not need
           * it for any future usage.
           * In case of INPLACE computation, there is no need to free output
           * buffer, since it will be freed as input buffer.
           */
          outputQId                      = 0;
          outputBufListReturn.numBuf     = 1;
          outputBufListReturn.buffers[0] = pSysBufferOutput;

          AlgorithmLink_releaseOutputBuffer(pObj,
                                            outputQId,
                                            &outputBufListReturn);

          bufDropFlag = FALSE;

          }
          else
          {
            bufDropFlag = TRUE;

            linkStatsInfo->linkStats.outBufErrorCount++;
            linkStatsInfo->linkStats.chStats
                            [pSysBufferInput->chNum].inBufDropCount++;
            linkStatsInfo->linkStats.chStats
                            [pSysBufferInput->chNum].outBufDropCount[0]++;

          } /* Output Buffer availability */

          } /* Input Buffer validity */

          /*
           * Releasing (Free'ing) input buffer, since algorithm does not need
           * it for any future usage.
           */
          inputQId                      = 0;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSysBufferInput;
          AlgorithmLink_releaseInputBuffer(
                                      pObj,
                                      inputQId,
                                      pFullViewObj->inQueParams.prevLinkId,
                                      pFullViewObj->inQueParams.prevLinkQueId,
                                      &inputBufListReturn,
                                      &bufDropFlag);

        }

    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Control Plugin for frame copy algorithm link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to control parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

Int32 AlgorithmLink_fullViewControl(void * pObj, void * pControlParams)
{
    AlgorithmLink_FullViewObj     * pFullViewObj;
    AlgorithmLink_ControlParams    * pAlgLinkControlPrm;
    Alg_FullView_Obj              * algHandle;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pFullViewObj = (AlgorithmLink_FullViewObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);
    algHandle     = pFullViewObj->algHandle;

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
            AlgorithmLink_fullViewPrintStatistics(pObj, pFullViewObj);
            break;

        default:
            status = Alg_FullViewControl(algHandle,
                                          &(pFullViewObj->controlParams)
                                          );
            break;
    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop Plugin for frame copy algorithm link
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
Int32 AlgorithmLink_fullViewStop(void * pObj)
{
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete Plugin for frame copy algorithm link
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_fullViewDelete(void * pObj)
{
    Alg_FullView_Obj          * algHandle;
    Int32                        frameIdx;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       maxHeight;
    UInt32                       channelId;

    System_VideoFrameBuffer    * pSystemVideoFrameBuffer;
    AlgorithmLink_FullViewObj * pFullViewObj;

    pFullViewObj = (AlgorithmLink_FullViewObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);
    algHandle     = pFullViewObj->algHandle;

    status = Utils_linkStatsCollectorDeAllocInst(pFullViewObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = Alg_FullViewDelete(algHandle);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    maxHeight = pFullViewObj->algLinkCreateParams.maxHeight;

    for(channelId =0 ; channelId < pFullViewObj->numInputChannels; channelId++)
    {
      for(frameIdx = 0;
          frameIdx < (pFullViewObj->algLinkCreateParams.numOutputFrames);
          frameIdx++)
      {
        pSystemVideoFrameBuffer =
                             &(pFullViewObj->videoFrames[channelId][frameIdx]);

        /*
         * Free'ing up of allocated buffers
         */
        status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
                               pSystemVideoFrameBuffer->bufAddr[0],
                               (maxHeight*pFullViewObj->pitch*2)
                               );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
      }
    }

    status = Utils_memFree(UTILS_HEAPID_DDR_CACHED_LOCAL,
                           pFullViewObj, sizeof(AlgorithmLink_FullViewObj));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Print link statistics
 *
 * \param  pObj                [IN] Algorithm link object handle
 * \param  pFullViewObj       [IN] Frame copy link Object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_fullViewPrintStatistics(void *pObj,
                       AlgorithmLink_FullViewObj *pFullViewObj)
{

    UTILS_assert(NULL != pFullViewObj->linkStatsInfo);

    Utils_printLinkStatistics(&pFullViewObj->linkStatsInfo->linkStats, "ALG_FULLVIEW", TRUE);

    Utils_printLatency("ALG_FULLVIEW",
                       &pFullViewObj->linkStatsInfo->linkLatency,
                       &pFullViewObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}

/* Nothing beyond this point */
