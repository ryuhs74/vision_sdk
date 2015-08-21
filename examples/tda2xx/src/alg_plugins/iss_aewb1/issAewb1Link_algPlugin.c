/*
 *******************************************************************************
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file issAewb1Link_algPlugin.c
 *
 * \brief  This file contains plug in functions for algorithm plugin
 *         Link
 *
 * \version 0.0 (Feb 2014) : [NN] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "issAewb1Link_priv.h"
#include <TI_dcc.h>
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>

#include <TI_aewb.h>


/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of gAlign algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_issAewb1_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_issAewb1Create;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_issAewb1Process;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_issAewb1Control;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_issAewb1Stop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_issAewb1Delete;

    if(System_getSelfProcId()==SYSTEM_PROC_IPU1_0)
    {
        algId = ALGORITHM_LINK_IPU_ALG_ISS_AEWB1;
    }

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin alg link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_issAewb1Create(void * pObj,void * pCreateParams)
{
    Int32 status;
    UInt32 prevLinkQId;
    UInt32 bufId;
    UInt32 chId;
    System_LinkInfo                prevLinkInfo;
    AlgorithmLink_IssAewbObj       * pAlgObj;
    AlgorithmLink_OutputQueueInfo  * pOutputQInfo;
    AlgorithmLink_InputQueueInfo   * pInputQInfo;
    System_LinkChInfo              * pOutChInfo;
    System_Buffer                  * pSystemBuffer;
    System_MetaDataBuffer          * pMetaDataBuffer;
    AlgorithmLink_IssAewbCreateParams *pLinkCreateParams;
    AlgorithmLink_IssAewbH3aParams * pH3aParams;

    pLinkCreateParams = (AlgorithmLink_IssAewbCreateParams *)pCreateParams;

    System_resetLinkMemAllocInfo(&pLinkCreateParams->memAllocInfo);

    if(System_useLinkMemAllocInfo(&pLinkCreateParams->memAllocInfo)==FALSE)
    {
        pAlgObj = (AlgorithmLink_IssAewbObj *)
                    Utils_memAlloc(
                        UTILS_HEAPID_DDR_CACHED_LOCAL,
                        sizeof(AlgorithmLink_IssAewbObj),
                        128);
    }
    else
    {
        pAlgObj = (AlgorithmLink_IssAewbObj *)
                    System_allocLinkMemAllocInfo(
                        &pLinkCreateParams->memAllocInfo,
                        sizeof(AlgorithmLink_IssAewbObj),
                        128);
    }
    UTILS_assert(pAlgObj != NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pAlgObj);

    pInputQInfo       = &pAlgObj->inputQInfo;
    pOutputQInfo      = &pAlgObj->outputQInfo;

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */
    memcpy(&pAlgObj->algLinkCreateParams,
           pLinkCreateParams,
           sizeof(pAlgObj->algLinkCreateParams));

    pLinkCreateParams = &pAlgObj->algLinkCreateParams;

    /* Check for errors in parameters */
    if (pAlgObj->algLinkCreateParams.numH3aPlanes > ALGORITHM_AEWB1_MAX_PLANES)
    {
        /* Does not support more than 2 planes of H3A data */
        UTILS_assert(FALSE);
    }

    pInputQInfo->qMode          = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;
    pOutputQInfo->qMode         = ALGORITHM_LINK_QUEUEMODE_NOTINPLACE;

    status = System_linkGetInfo(
                    pAlgObj->algLinkCreateParams.inQueParams.prevLinkId,
                    &prevLinkInfo
                    );
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    UTILS_assert(prevLinkInfo.numQue >= 1);

    prevLinkQId = pAlgObj->algLinkCreateParams.inQueParams.prevLinkQueId;
    pAlgObj->inQueInfo = prevLinkInfo.queInfo[prevLinkQId];


    pOutputQInfo->queInfo.numCh = pAlgObj->inQueInfo.numCh;
    UTILS_assert(pOutputQInfo->queInfo.numCh<=ISS_AEWB1_LINK_MAX_CH);

    for(chId=0; chId<pOutputQInfo->queInfo.numCh; chId++)
    {
        pOutChInfo          = &pOutputQInfo->queInfo.chInfo[chId];
        pOutChInfo->flags   = 0;
        pOutChInfo->height  = 0;
        pOutChInfo->width   = 0;
    }

    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(pObj,
                                1,
                                pInputQInfo,
                                1,
                                pOutputQInfo);

    if(pAlgObj->algLinkCreateParams.numOutBuffers >
        ISS_AEWB1_LINK_MAX_NUM_OUTPUT)
    {
        pAlgObj->algLinkCreateParams.numOutBuffers =
            ISS_AEWB1_LINK_MAX_NUM_OUTPUT;
    }

    pAlgObj->lock = BspOsal_semCreate(1u, TRUE);
    UTILS_assert(NULL != pAlgObj->lock);

    /* Allocate input and output buffers for the DCC*/
    if (pAlgObj->algLinkCreateParams.enableDcc)
    {
        status = Dcc_Create(pAlgObj,
            &pAlgObj->algLinkCreateParams.memAllocInfo);
        UTILS_assert(0 == status);
    }

    /*
     * Allocate memory for the output buffers and link metadata buffer with
     * system Buffer
     */
    for (bufId = 0; bufId <
        pOutputQInfo->queInfo.numCh*pAlgObj->algLinkCreateParams.numOutBuffers;
        bufId++)
    {
        pSystemBuffer       =   &pAlgObj->buffers[bufId];
        pMetaDataBuffer     =   &pAlgObj->metaDataBuffers[bufId];

        /*
         * Properties of pSystemBuffer, which do not get altered during
         * run time (frame exchanges) are initialized here
         */
        pSystemBuffer->bufType      =   SYSTEM_BUFFER_TYPE_METADATA;
        pSystemBuffer->payload      =   pMetaDataBuffer;
        pSystemBuffer->payloadSize  =   sizeof(System_MetaDataBuffer);
        pSystemBuffer->chNum        =
            bufId / pAlgObj->algLinkCreateParams.numOutBuffers;

        pMetaDataBuffer->numMetaDataPlanes  =
            pAlgObj->algLinkCreateParams.numH3aPlanes;

        pMetaDataBuffer->metaBufSize[0U] = ISS_AEWB1_LINK_MAX_BUF_SIZE;

        if(System_useLinkMemAllocInfo(
            &pAlgObj->algLinkCreateParams.memAllocInfo)==FALSE)
        {
            pMetaDataBuffer->bufAddr[0U] =  Utils_memAlloc(
                UTILS_HEAPID_DDR_CACHED_SR,
                pMetaDataBuffer->metaBufSize[0U],
                128);
        }
        else
        {
            pMetaDataBuffer->bufAddr[0U]
                =  (Ptr)System_allocLinkMemAllocInfo(
                        &pAlgObj->algLinkCreateParams.memAllocInfo,
                        pMetaDataBuffer->metaBufSize[0U],
                        128);
        }

        pMetaDataBuffer->metaFillLength[0U] =
            pMetaDataBuffer->metaBufSize[0U];

        UTILS_assert(pMetaDataBuffer->bufAddr[0U] != NULL);

        ((IssAewbAlgOutParams*)pMetaDataBuffer->bufAddr[0U])->channelId =
                pAlgObj->algLinkCreateParams.channelId;
        ((IssAewbAlgOutParams*)pMetaDataBuffer->bufAddr[0U])->numParams =
                pAlgObj->algLinkCreateParams.numH3aPlanes;

        pMetaDataBuffer->flags = 0;

        status = AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSystemBuffer);

        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    /* Create AWB Algorithm,
        created only if mode is AE/AWB/AEWB
        For the DCC only mode, where mode is set to none, algorithm instance
        is not created */
    if ((ALGORITHMS_ISS_AEWB_MODE_AWB == pAlgObj->algLinkCreateParams.mode) ||
        (ALGORITHMS_ISS_AEWB_MODE_AE == pAlgObj->algLinkCreateParams.mode) ||
        (ALGORITHMS_ISS_AEWB_MODE_AEWB == pAlgObj->algLinkCreateParams.mode))
    {
        UInt32 pixCtWin;

        pH3aParams = &pAlgObj->algLinkCreateParams.h3aParams;

        /* Calculate the Pixel count in the a window */
        pixCtWin = (pH3aParams->winSizeV /
                    pH3aParams->winSkipV) *
                   (pH3aParams->winSizeH /
                    pH3aParams->winSkipH);

        pAlgObj->algHndl = ALG_aewbCreate(
            pAlgObj->algLinkCreateParams.mode,
            pH3aParams->winCountH, pH3aParams->winCountV, pixCtWin,
            pAlgObj->algLinkCreateParams.dataFormat,
            pAlgObj->algLinkCreateParams.numSteps,
            &pAlgObj->algLinkCreateParams.aeDynParams,
            pAlgObj->algLinkCreateParams.calbData,
            &pAlgObj->algLinkCreateParams.memAllocInfo
            );

        UTILS_assert(NULL != pAlgObj->algHndl);
    }
    else
    {
        pAlgObj->algHndl = NULL;
    }

    /* By Default, both the algorithms are in Auto mode,
       it can be later on changed to Manual mode by calling set_2a_params
       ioctl */
    pAlgObj->aewbOut.aeMode = 0;
    pAlgObj->aewbOut.awbMode = 0;

    pAlgObj->isFirstFrameRecv    = FALSE;

    pAlgObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_ISS_AEWB");
    UTILS_assert(NULL != pAlgObj->linkStatsInfo);

    System_assertLinkMemAllocOutOfMem(
        &pAlgObj->algLinkCreateParams.memAllocInfo,
        "ALG_ISS_AEWB"
        );

    return SYSTEM_LINK_STATUS_SOK;
}

Int32 AlgorithmLink_issAewb1RunAlg(
    AlgorithmLink_IssAewbObj *pAlgObj,
    System_MetaDataBuffer *pInMetaBuf,
    System_MetaDataBuffer *pOutMetaBuf,
    UInt32 planeId,
    UInt32 chNum)
{
    IssAewbAlgOutParams  *pAlgOut = NULL;
    ALG_Output            output = {0};

    Cache_inv(
        pInMetaBuf->bufAddr[planeId],
        pInMetaBuf->metaBufSize[planeId],
        Cache_Type_ALLD,
        TRUE);

    /* Update Dynamic Parameters,
       as of now, there is no interface to do this, so using
       default parameters only */

    /* Run AWB Algorithm */
    ALG_aewbRun(pAlgObj->algHndl, pInMetaBuf->bufAddr[planeId], &output);

    /* Convert to format suitable for next link (ISS ISP processing)
     */
    pAlgOut
        = (IssAewbAlgOutParams*)
            pOutMetaBuf->bufAddr[0U];

    pAlgOut->channelId = chNum;

    BspOsal_semWait(pAlgObj->lock, BSP_OSAL_WAIT_FOREVER);
    /* Copy Parameters for Auto Mode */
    if (!pAlgObj->aewbOut.awbMode)
    {
        pAlgOut->outPrms[planeId].useWbCfg = output.useWbCfg;
        pAlgOut->outPrms[planeId].gain[0U] = output.rGain;
        pAlgOut->outPrms[planeId].gain[1U] = output.grGain;
        pAlgOut->outPrms[planeId].gain[2U] = output.gbGain;
        pAlgOut->outPrms[planeId].gain[3U] = output.bGain;

        pAlgOut->outPrms[planeId].offset[0U] = output.rOffset;
        pAlgOut->outPrms[planeId].offset[1U] = output.grOffset;
        pAlgOut->outPrms[planeId].offset[2U] = output.gbOffset;
        pAlgOut->outPrms[planeId].offset[3U] = output.bOffset;

        pAlgOut->outPrms[planeId].useColorTemp = output.useColorTemp;
        pAlgOut->outPrms[planeId].colorTemparature =
            output.colorTemparature;

        if (TRUE == output.useColorTemp)
        {
            pAlgObj->aewbOut.colorTemp = output.colorTemparature;
        }
        if (TRUE == output.useWbCfg)
        {
            pAlgObj->aewbOut.rGain = output.rGain;
            pAlgObj->aewbOut.gGain = output.grGain;
            pAlgObj->aewbOut.bGain = output.bGain;
        }
    }
    else
    {
        pAlgOut->outPrms[planeId].useWbCfg = TRUE;
        pAlgOut->outPrms[planeId].gain[0U] = pAlgObj->aewbOut.rGain;
        pAlgOut->outPrms[planeId].gain[1U] = pAlgObj->aewbOut.gGain;
        pAlgOut->outPrms[planeId].gain[2U] = pAlgObj->aewbOut.gGain;
        pAlgOut->outPrms[planeId].gain[3U] = pAlgObj->aewbOut.bGain;

        pAlgOut->outPrms[planeId].offset[0U] = 0;
        pAlgOut->outPrms[planeId].offset[1U] = 0;
        pAlgOut->outPrms[planeId].offset[2U] = 0;
        pAlgOut->outPrms[planeId].offset[3U] = 0;

        pAlgOut->outPrms[planeId].useColorTemp = TRUE;
        pAlgOut->outPrms[planeId].colorTemparature =
            pAlgObj->aewbOut.colorTemp;
    }

    /* Copy Parameters for Auto Mode */
    if (!pAlgObj->aewbOut.aeMode)
    {
        pAlgOut->outPrms[planeId].useAeCfg = output.useAeCfg;
        pAlgOut->outPrms[planeId].exposureTime = output.exposureTime;
        pAlgOut->outPrms[planeId].analogGain = output.analogGain;
        pAlgOut->outPrms[planeId].digitalGain = output.digitalGain;

        if (TRUE == output.useAeCfg)
        {
            /* Keeping copy of AE parameters for DCC */
            pAlgObj->aewbOut.expTime = output.exposureTime;
            pAlgObj->aewbOut.analogGain = output.analogGain;;
            pAlgObj->aewbOut.digitalGain = output.digitalGain;;
        }
    }
    else
    {
        pAlgOut->outPrms[planeId].useAeCfg = TRUE;
        pAlgOut->outPrms[planeId].exposureTime = pAlgObj->aewbOut.expTime;
        pAlgOut->outPrms[planeId].analogGain = pAlgObj->aewbOut.analogGain;
        pAlgOut->outPrms[planeId].digitalGain =
            pAlgObj->aewbOut.digitalGain;
    }
    BspOsal_semPost(pAlgObj->lock);

    return 0;
}

Int32 AlgorithmLink_issAewb1Process(void * pObj)
{
    UInt32 bufId, plnCnt;
    Int32 status;
    Bool   bufDropFlag;
    AlgorithmLink_IssAewbObj            * pAlgObj;
    System_BufferList                   inputBufList;
    AlgorithmLink_IssAewbCreateParams   * pLinkCreatePrms;
    System_Buffer                       * pSysOutBuffer;
    System_Buffer                       * pSysInBuffer;
    System_MetaDataBuffer               * pInMetaBuf;
    System_MetaDataBuffer               * pOutMetaBuf;
    System_BufferList                   bufListReturn;
    System_LinkStatistics               * linkStatsInfo;

    pAlgObj = (AlgorithmLink_IssAewbObj *)
                AlgorithmLink_getAlgorithmParamsObj(pObj);

    linkStatsInfo = pAlgObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    pLinkCreatePrms = &pAlgObj->algLinkCreateParams;

    if (pAlgObj->isFirstFrameRecv == FALSE)
    {
        pAlgObj->isFirstFrameRecv = TRUE;

        Utils_resetLinkStatistics(&linkStatsInfo->linkStats, 1, 1);

        Utils_resetLatency(&linkStatsInfo->linkLatency);
        Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
    }


    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    System_getLinksFullBuffers(pLinkCreatePrms->inQueParams.prevLinkId,
                               pLinkCreatePrms->inQueParams.prevLinkQueId,
                               &inputBufList);

    if (inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysInBuffer = inputBufList.buffers[bufId];
            if(pSysInBuffer == NULL)
            {
                linkStatsInfo->linkStats.inBufErrorCount++;
                continue;
            }

            UTILS_assert(pSysInBuffer->chNum <
                         pAlgObj->outputQInfo.queInfo.numCh);

            linkStatsInfo->linkStats.chStats
                [pSysInBuffer->chNum].inBufRecvCount++;

            status = AlgorithmLink_getEmptyOutputBuffer(
                                                    pObj,
                                                    0,
                                                    pSysInBuffer->chNum,
                                                    &pSysOutBuffer);
            if(status != SYSTEM_LINK_STATUS_SOK)
            {
                linkStatsInfo->linkStats.chStats
                            [pSysInBuffer->chNum].inBufDropCount++;
                linkStatsInfo->linkStats.chStats
                            [pSysInBuffer->chNum].outBufDropCount[0]++;
            }
            else
            {
                pSysOutBuffer->srcTimestamp = pSysInBuffer->srcTimestamp;
                pSysOutBuffer->linkLocalTimestamp =
                    Utils_getCurGlobalTimeInUsec();
                pOutMetaBuf  = (System_MetaDataBuffer *)pSysOutBuffer->payload;
                pInMetaBuf  = (System_MetaDataBuffer *) pSysInBuffer->payload;

                pOutMetaBuf->numMetaDataPlanes = pInMetaBuf->numMetaDataPlanes;

                /* Run AEWB Algorithm only if it is enabled at create time */
                if ((ALGORITHMS_ISS_AEWB_MODE_AWB == pLinkCreatePrms->mode) ||
                    (ALGORITHMS_ISS_AEWB_MODE_AE == pLinkCreatePrms->mode) ||
                    (ALGORITHMS_ISS_AEWB_MODE_AEWB == pLinkCreatePrms->mode))
                {
                    for (plnCnt = 0u; plnCnt < pOutMetaBuf->numMetaDataPlanes;
                         plnCnt ++)
                    {
                        AlgorithmLink_issAewb1RunAlg(
                            pAlgObj,
                            pInMetaBuf,
                            pOutMetaBuf,
                            plnCnt,
                            pSysInBuffer->chNum);
                    }
                }

                if (TRUE == pAlgObj->algLinkCreateParams.enableDcc)
                {
                    Dcc_update_params(
                        pAlgObj,
                        (IssAewbAlgOutParams*)pOutMetaBuf->bufAddr[0U]);
                }

                /* This function should not be null */
                UTILS_assert(NULL != pLinkCreatePrms->cfgCbFxn);

                pLinkCreatePrms->cfgCbFxn(
                    (IssAewbAlgOutParams*)pOutMetaBuf->bufAddr[0U],
                    pLinkCreatePrms->appData);

                Utils_updateLatency(&linkStatsInfo->linkLatency,
                                    pSysOutBuffer->linkLocalTimestamp);
                Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                                    pSysOutBuffer->srcTimestamp);

                linkStatsInfo->linkStats.chStats
                            [pSysInBuffer->chNum].inBufProcessCount++;
                linkStatsInfo->linkStats.chStats
                            [pSysInBuffer->chNum].outBufCount[0]++;

                status = AlgorithmLink_putEmptyOutputBuffer(pObj, 0, pSysOutBuffer);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);


                if ((TRUE == pLinkCreatePrms->isWdrEnable) &&
                    (NULL != pLinkCreatePrms->mergeCbFxn))
                {
                    pLinkCreatePrms->mergeCbFxn(
                        (IssAewbAlgOutParams*)pOutMetaBuf->bufAddr[0U],
                        pLinkCreatePrms->appData);
                }
            }

            bufListReturn.numBuf = 1;
            bufListReturn.buffers[0] = pSysInBuffer;
            bufDropFlag = FALSE;
            AlgorithmLink_releaseInputBuffer(
                          pObj,
                          0,
                          pLinkCreatePrms->inQueParams.prevLinkId,
                          pLinkCreatePrms->inQueParams.prevLinkQueId,
                          &bufListReturn,
                          &bufDropFlag);
        }
    }
    return status;
}

Int32 AlgorithmLink_issAewb1Control(void *pObj, void *pControlParams)
{
    Int32 status    = SYSTEM_LINK_STATUS_SOK;
    AlgorithmLink_IssAewbObj *pAlgObj = NULL;
    AlgorithmLink_ControlParams *pAlgLinkControlPrm = NULL;
    AlgorithmLink_IssAewbAeControlParams *aeCtrlPrms = NULL;
    AlgorithmLink_IssAewbAwbControlParams *awbCtrlPrms = NULL;
    AlgorithmLink_IssAewbDccControlParams *dccCtrlPrms = NULL;
    AlgorithmLink_IssAewbCreateParams     *createPrms;

    pAlgObj = (AlgorithmLink_IssAewbObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    pAlgLinkControlPrm = (AlgorithmLink_ControlParams *)pControlParams;

    createPrms = &pAlgObj->algLinkCreateParams;

    /* Check for errors */
    /* AE/AWB parameters can be changed only if they are created/enabled
       at the algorithm create time */
    if ((ALGORITHMS_ISS_AEWB_MODE_AWB != createPrms->mode) &&
        (ALGORITHMS_ISS_AEWB_MODE_AEWB != createPrms->mode))
    {
        /* AWB parameters cannot be set since create mode does not
           include AWB */
        if (ALGORITHM_AEWB_LINK_CMD_SET_AWB_CALB_DATA ==
            pAlgLinkControlPrm->controlCmd)
        {
            status = SYSTEM_LINK_STATUS_EFAIL;
        }
    }
    if ((ALGORITHMS_ISS_AEWB_MODE_AE != createPrms->mode) &&
        (ALGORITHMS_ISS_AEWB_MODE_AEWB != createPrms->mode))
    {
        /* AE parameters cannot be set since create mode does not
           include AE */
        if (ALGORITHM_AEWB_LINK_CMD_SET_AE_DYNAMIC_PARAMS ==
            pAlgLinkControlPrm->controlCmd)
        {
            status = SYSTEM_LINK_STATUS_EFAIL;
        }
    }
    /* None of the DCC commands are valid if DCC is not enabled
       at create time */
    if (!createPrms->enableDcc)
    {
        if ((ALGORITHM_AEWB_LINK_CMD_GET_DCC_BUF_PARAMS ==
                pAlgLinkControlPrm->controlCmd) ||
            (ALGORITHM_AEWB_LINK_CMD_PARSE_AND_SET_DCC_PARAMS ==
                pAlgLinkControlPrm->controlCmd))
        {
            status = SYSTEM_LINK_STATUS_EFAIL;
        }
    }

    if (SYSTEM_LINK_STATUS_SOK != status)
    {
        /* Returning from here,
           This voilates MICRA-C */
        return (status);
    }

    /*
     * There can be other commands to alter the properties of the alg link
     * or properties of the core algorithm.
     */
    switch(pAlgLinkControlPrm->controlCmd)
    {
        /* Control command is to set AE dynamic parameters */
        case ALGORITHM_AEWB_LINK_CMD_SET_AE_DYNAMIC_PARAMS:
        {
            aeCtrlPrms = (AlgorithmLink_IssAewbAeControlParams *)
                                    pControlParams;

            if (NULL != aeCtrlPrms)
            {
                status = ALG_aewbSetAeDynParams(
                            pAlgObj->algHndl,
                            aeCtrlPrms->aeDynParams);
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            break;
        }

        /* Control command is to set AWB Calibration data */
        case ALGORITHM_AEWB_LINK_CMD_SET_AWB_CALB_DATA:
        {
            awbCtrlPrms = (AlgorithmLink_IssAewbAwbControlParams *)
                                    pControlParams;

            if (NULL != awbCtrlPrms)
            {
                status = ALG_aewbSetAwbCalbData(
                            pAlgObj->algHndl,
                            awbCtrlPrms->calbData);
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            break;
        }

        case SYSTEM_CMD_PRINT_STATISTICS:
            AlgorithmLink_issAewb1PrintStatistics(pObj, pAlgObj);
            break;

        /* Parse DCC parameters and set them in ISP */
        case ALGORITHM_AEWB_LINK_CMD_GET_DCC_BUF_PARAMS:
        {
            dccCtrlPrms = (AlgorithmLink_IssAewbDccControlParams *)
                                    pControlParams;

            if (NULL != dccCtrlPrms)
            {
                if (NULL != pAlgObj->dccObj.dccInBuf)
                {
                    dccCtrlPrms->dccBuf = pAlgObj->dccObj.dccInBuf;
                }
                else
                {
                    dccCtrlPrms->dccBuf = NULL;
                }
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            break;
        }

        /* Parse DCC parameters and set them in ISP */
        case ALGORITHM_AEWB_LINK_CMD_PARSE_AND_SET_DCC_PARAMS:
        {
            dccCtrlPrms = (AlgorithmLink_IssAewbDccControlParams *)
                                    pControlParams;

            if (NULL != dccCtrlPrms)
            {
                if ((NULL != dccCtrlPrms->pIspCfg) &&
                    (NULL != dccCtrlPrms->pSimcopCfg))
                {
                    Dcc_parse_and_save_params(
                                pAlgObj,
                                dccCtrlPrms->pIspCfg,
                                dccCtrlPrms->pSimcopCfg,
                                dccCtrlPrms->dccBufSize);
                }
                else
                {
                    /* ISP Config is local object, so ok to reset it. */
                    memset(&pAlgObj->dccObj.ispCfgPrms,
                           0,
                           sizeof(IssIspConfigurationParameters));
                    memset(&pAlgObj->dccObj.simcopCfgPrms,
                           0,
                           sizeof(IssM2mSimcopLink_ConfigParams));
                    Dcc_parse_and_save_params(
                                pAlgObj,
                                &pAlgObj->dccObj.ispCfgPrms,
                                &pAlgObj->dccObj.simcopCfgPrms,
                                dccCtrlPrms->dccBufSize);
                }

                if (0 == status)
                {
                    /* Set the AWB Calb Parameters */
                    if (TRUE == pAlgObj->dccObj.dccOutPrms.useAwbCalbCfg)
                    {
                        /* Setting AWB Calibration data immediately
                            structures AlgorithmLink_IssAewbAwbCalbData and
                            awb_calc_data_t are exactly same, so type
                            casting here, awb_calc_data_t is used internally
                            by algorithm, whereas
                            AlgorithmLink_IssAewbAwbCalbData is used by the
                            AEWB link plugin */
                        status = ALG_aewbSetAwbCalbData(
                                    pAlgObj->algHndl,
                                    (AlgorithmLink_IssAewbAwbCalbData *)
                                        &pAlgObj->dccObj.dccOutPrms.awbCalbData);
                    }

                    if ((NULL == dccCtrlPrms->pIspCfg) &&
                        (NULL == dccCtrlPrms->pSimcopCfg) &&
                        (NULL != createPrms->dccIspCfgFxn))
                    {
                        /* Call the callback function to update
                           parameters in ISP */
                        pAlgObj->dccObj.ispCfgPrms.channelId =
                            pAlgObj->algLinkCreateParams.channelId;
                        createPrms->dccIspCfgFxn(
                            &pAlgObj->dccObj.ispCfgPrms,
                            &pAlgObj->dccObj.simcopCfgPrms,
                            createPrms->appData);
                    }
                }
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            break;
        }

        case ALGORITHM_AEWB_LINK_CMD_SET_CAMERA_INFO:
        {
            AlgorithmLink_IssAewbDccCameraInfo *pCamInfo;

            pCamInfo = (AlgorithmLink_IssAewbDccCameraInfo *)
                                    pControlParams;

            if (NULL != pCamInfo)
            {
                pAlgObj->algLinkCreateParams.dccCameraId = pCamInfo->cameraId;
                pAlgObj->dccObj.width = pCamInfo->width;
                pAlgObj->dccObj.height = pCamInfo->height;
                status = SYSTEM_LINK_STATUS_SOK;
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }

            break;
        }

        case ALGORITHM_AEWB_LINK_CMD_GET_2A_PARAMS:
        {
            AlgorithmLink_IssAewb2AControlParams *pAewb2APrms;

            pAewb2APrms = (AlgorithmLink_IssAewb2AControlParams *)
                                    pControlParams;

            if (NULL != pAewb2APrms)
            {
                /* Copy Parameters only if Auto mode is selected */
                memcpy(&pAewb2APrms->aewb2APrms, &pAlgObj->aewbOut,
                    sizeof(AlgorithmLink_IssAewb2AParams));
                status = SYSTEM_LINK_STATUS_SOK;
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            break;
        }

        case ALGORITHM_AEWB_LINK_CMD_SET_2A_PARAMS:
        {
            AlgorithmLink_IssAewb2AControlParams *pAewb2APrms;

            pAewb2APrms = (AlgorithmLink_IssAewb2AControlParams *)
                                    pControlParams;

            if (NULL != pAewb2APrms)
            {
                BspOsal_semWait(pAlgObj->lock, BSP_OSAL_WAIT_FOREVER);
                if (pAewb2APrms->aewb2APrms.aeMode)
                {
                    /* Copy Fixed output parameters for AE */
                    pAlgObj->aewbOut.digitalGain = pAewb2APrms->aewb2APrms.digitalGain;
                    pAlgObj->aewbOut.analogGain = pAewb2APrms->aewb2APrms.analogGain;
                    pAlgObj->aewbOut.expTime = pAewb2APrms->aewb2APrms.expTime;
                    pAlgObj->aewbOut.aeMode = TRUE;
                }
                else
                {
                    /* AUTO Mode is selected */
                    pAlgObj->aewbOut.aeMode = FALSE;
                }

                if (pAewb2APrms->aewb2APrms.awbMode)
                {
                    /* Copy Fixed output parameters for AWB */
                    pAlgObj->aewbOut.rGain = pAewb2APrms->aewb2APrms.rGain;
                    pAlgObj->aewbOut.gGain = pAewb2APrms->aewb2APrms.gGain;
                    pAlgObj->aewbOut.bGain = pAewb2APrms->aewb2APrms.bGain;
                    pAlgObj->aewbOut.colorTemp = pAewb2APrms->aewb2APrms.colorTemp;
                    pAlgObj->aewbOut.awbMode = TRUE;
                }
                else
                {
                    /* AUTO Mode is selected */
                    pAlgObj->aewbOut.awbMode = FALSE;
                }
                BspOsal_semPost(pAlgObj->lock);
                status = SYSTEM_LINK_STATUS_SOK;
            }
            else
            {
                status = SYSTEM_LINK_STATUS_EINVALID_PARAMS;
            }
            break;
        }
        default:
            break;
    }

    return status;
}

Int32 AlgorithmLink_issAewb1Stop(void * pObj)
{
    return 0;
}

Int32 AlgorithmLink_issAewb1Delete(void * pObj)
{

    UInt32 status;
    UInt32 bufId;
    AlgorithmLink_IssAewbObj *pAlgObj;
    System_MetaDataBuffer *pMetaDataBuffer;

    pAlgObj = (AlgorithmLink_IssAewbObj *)
                                AlgorithmLink_getAlgorithmParamsObj(pObj);

    status = Utils_linkStatsCollectorDeAllocInst(pAlgObj->linkStatsInfo);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    if (NULL != pAlgObj->lock)
    {
        BspOsal_semDelete(&pAlgObj->lock);
    }
    /*
     * Free link buffers
     */
    for (bufId = 0;
        bufId < pAlgObj->outputQInfo.queInfo.numCh*
                pAlgObj->algLinkCreateParams.numOutBuffers;
        bufId++)
    {
        pMetaDataBuffer     =   &pAlgObj->metaDataBuffers[bufId];

        UTILS_assert(NULL != pMetaDataBuffer->bufAddr[0]);
        if(System_useLinkMemAllocInfo(
            &pAlgObj->algLinkCreateParams.memAllocInfo)==FALSE)
        {
            status = Utils_memFree(
                                UTILS_HEAPID_DDR_CACHED_SR,
                                pMetaDataBuffer->bufAddr[0],
                                pMetaDataBuffer->metaBufSize[0]
                               );
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
        }
    }

    /* Delete only if the algorithm instance is created */
    if (NULL != pAlgObj->algHndl)
    {
        ALG_aewbDelete(pAlgObj->algHndl,
            &pAlgObj->algLinkCreateParams.memAllocInfo);
        pAlgObj->algHndl = NULL;
    }

    if (TRUE == pAlgObj->algLinkCreateParams.enableDcc)
    {
        Dcc_delete(pAlgObj,
            &pAlgObj->algLinkCreateParams.memAllocInfo);
    }

    if(System_useLinkMemAllocInfo(
        &pAlgObj->algLinkCreateParams.memAllocInfo)==FALSE)
    {
        Utils_memFree(
            UTILS_HEAPID_DDR_CACHED_LOCAL,
            pAlgObj,
            sizeof(AlgorithmLink_IssAewbObj)
            );
    }

    pAlgObj = NULL;

    return status;
}

Int32 AlgorithmLink_issAewb1PrintStatistics(void *pObj,
                AlgorithmLink_IssAewbObj *pAlgObj)
{
    UTILS_assert(NULL != pAlgObj->linkStatsInfo);

    Utils_printLinkStatistics(&pAlgObj->linkStatsInfo->linkStats,
                            "ALG_ISS_AEWB",
                            TRUE);

    Utils_printLatency("ALG_ISS_AEWB",
                       &pAlgObj->linkStatsInfo->linkLatency,
                       &pAlgObj->linkStatsInfo->srcToLinkLatency,
                        TRUE);

    return SYSTEM_LINK_STATUS_SOK;
}

