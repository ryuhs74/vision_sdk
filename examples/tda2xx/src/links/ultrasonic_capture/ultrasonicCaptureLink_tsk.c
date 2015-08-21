/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file ultrasonicCaptureLink_tsk.c
 *
 *******************************************************************************
 */

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "ultrasonicCaptureLink_priv.h"
#include <boards/bsp_board.h>

#define ULTRASONIC_CAPTURE_LINK_TSK_STACK_SIZE  (16*KB)
#define ULTRASONIC_CAPTURE_LINK_TSK_PRI         (10)

/* polling interval in units of msecs */
#define ULTRASONIC_CAPTURE_POLLING_INTERVAL     (100)

/**
 *******************************************************************************
 * \brief Link Stack
 *******************************************************************************
 */
#pragma DATA_ALIGN(gUltrasonicCaptureLink_tskStack, 32)
#pragma DATA_SECTION(gUltrasonicCaptureLink_tskStack, ".bss:taskStackSection")
UInt8 gUltrasonicCaptureLink_tskStack[ULTRASONIC_CAPTURE_LINK_TSK_STACK_SIZE];

/**
 *******************************************************************************
 * \brief Link object, stores all link related information
 *******************************************************************************
 */
UltrasonicCaptureLink_Obj gUltrasonicCaptureLink_obj;




/**
 *******************************************************************************
 * \brief Creates link related information, including buffer allocation
 *
 * \param  pObj     [IN]  link instance handle
 * \param  pPrm     [IN]  Create params for link
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_drvCreate(UltrasonicCaptureLink_Obj * pObj, UltrasonicCaptureLink_CreateParams * pPrm)
{
    UInt32 bufId;
    Int32 status;
    System_Buffer *pSysBuf;
    System_MetaDataBuffer *pMetaBuf;
    UInt32 deviceId;
    Bool   isDetected;

    UInt8 ultrasonicSensorDetected; //to  record number of sensors detected
    Vps_printf(" ULTRASONIC: Create in progress !!!");

    memcpy(&pObj->createArgs, pPrm, sizeof(pObj->createArgs));

    /* set output info */
    memset(&pObj->info, 0, sizeof(pObj->info));
    pObj->info.numQue = 1;
    pObj->info.queInfo[0].numCh = 1;

    SYSTEM_LINK_CH_INFO_SET_FLAG_BUF_TYPE(
            pObj->info.queInfo[0].chInfo[0].flags,
            SYSTEM_BUFFER_TYPE_METADATA
            );

    status = Utils_bufCreate(&pObj->outFrameQue, FALSE, FALSE);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
	ultrasonicSensorDetected = 0;

    for (bufId = 0; bufId < ULTRASONIC_CAPTURE_LINK_MAX_OUT_BUFFERS; bufId++)
    {
        pSysBuf = &pObj->sysBufs[bufId];
        pMetaBuf = &pObj->metaBufs[bufId];

        memset(pSysBuf, 0, sizeof(*pSysBuf));
        memset(pMetaBuf, 0, sizeof(*pMetaBuf));

        pSysBuf->bufType = SYSTEM_BUFFER_TYPE_METADATA;
        pSysBuf->payloadSize = sizeof(*pMetaBuf);
        pSysBuf->payload = pMetaBuf;

        pMetaBuf->numMetaDataPlanes = 1;
        pMetaBuf->metaBufSize[0] = sizeof(UltrasonicCapture_MeasurementInfo);
        pMetaBuf->bufAddr[0] =
                    Utils_memAlloc( UTILS_HEAPID_DDR_CACHED_SR,
                                    pMetaBuf->metaBufSize[0],
                                    32
                                    );

        pMetaBuf->metaFillLength[0] = pMetaBuf->metaBufSize[0];

        status = Utils_bufPutEmptyBuffer(&pObj->outFrameQue,
                                         pSysBuf);
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    if(pObj->createArgs.uartInstId!=BSP_DEVICE_UART_INST_ID_9)
    {
        Vps_printf(
            " ULTRASONIC: WARNING: UART instance used for communication MUST be UART%d !!!\n",
                        BSP_DEVICE_UART_INST_ID_9);
        pObj->createArgs.uartInstId = BSP_DEVICE_UART_INST_ID_9;
    }

    for (deviceId = 0U; deviceId < BSP_PGA450_MAX_DEVICE; deviceId++)
    {
        pObj->deviceIsDetected[deviceId] = FALSE;

        isDetected = Bsp_pga450ProbeDevice(pObj->createArgs.uartInstId, deviceId);
        if (TRUE == isDetected)
        {
            pObj->deviceIsDetected[deviceId] = TRUE;
            Vps_printf(" ULTRASONIC: UART%d: DEVICE%d detected !\n",
                pObj->createArgs.uartInstId, deviceId);
            ultrasonicSensorDetected++;
        }
        /*else
        {
            Vps_printf(
                " ULTRASONIC: UART%d: DEVICE%d NOT FOUND !!!\n",
                pObj->createArgs.uartInstId, deviceId);
        }*/
    }

    Vps_printf(" ULTRASONIC: %d ultrasonic sensors have been detected\n", ultrasonicSensorDetected);
    Vps_printf(" ULTRASONIC: Create DONE !!!");

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Stop link
 *
 * \param  pObj     [IN]  link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_drvStart(UltrasonicCaptureLink_Obj * pObj)
{

    Vps_printf(" ULTRASONIC: Start DONE !!!");

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Start link
 *
 * \param  pObj     [IN]  link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_drvStop(UltrasonicCaptureLink_Obj * pObj)
{

    Vps_printf(" ULTRASONIC: Stop DONE !!!");

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Get measurement data from ultrasonic sensor
 *
 * \param  pObj     [IN]  link instance handle
 *
 * \return status   SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_tskRun(UltrasonicCaptureLink_Obj * pObj,
                         Utils_TskHndl * pTsk,
                         Utils_MsgHndl ** pMsg, Bool * done, Bool * ackMsg)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool runDone, runAckMsg;
    Utils_MsgHndl *pRunMsg;
    UInt32 cmd, deviceId;
    UltrasonicCaptureLink_CreateParams *pCreateArgs;
    System_Buffer *pBuf;
    System_MetaDataBuffer *pMetaBuf;
    UltrasonicCapture_MeasurementInfo *pMesurementInfo;
    UltrasonicCapture_DeviceMeasurementInfo *pDeviceInfo;
    Int32 distMeasuredShort; //distMeasuredLong;
    UInt32 curTime;

    /* READY loop done and ackMsg status */
    *done = FALSE;
    *ackMsg = FALSE;
    *pMsg = NULL;

    pCreateArgs = &pObj->createArgs;

    /* RUN loop done and ackMsg status */
    runDone = FALSE;
    runAckMsg = FALSE;

    /* RUN state loop */
    while (!runDone)
    {
        curTime = Utils_getCurTimeInMsec();

        pBuf = NULL;
        Utils_bufGetEmptyBuffer(&pObj->outFrameQue,
                                &pBuf, BSP_OSAL_NO_WAIT);

        if(pBuf)
        {
            pBuf->srcTimestamp  = Utils_getCurGlobalTimeInUsec();

            pMetaBuf = (System_MetaDataBuffer*)pBuf->payload;

            pMesurementInfo = (UltrasonicCapture_MeasurementInfo*)
                                    pMetaBuf->bufAddr[0];

            pMesurementInfo->numSensors = 0;

			//ultrasonic sensors are polled one by one in the follow for loop
            for (deviceId = 0U; deviceId < BSP_PGA450_MAX_DEVICE; deviceId++)
            {
                if(pObj->deviceIsDetected[deviceId])
                {
                    distMeasuredShort = Bsp_pga450GetMeasurement(
                               pCreateArgs->uartInstId,
                               deviceId,
                               BSP_PGA450_DISTANCE_MODE_SHORT);
/*
                    if(distMeasuredShort >= 0)
                    {
                        Vps_printf(" ULTRASONIC: DEVICE%d: SHORT DISTANCE = %d cm\n",
                                        deviceId, distMeasuredShort);
                    }
                    else
                    {
                        Vps_printf(
                            " ULTRASONIC: DEVICE%d: SHORT DISTANCE = FAILED !!!\n",
                            deviceId);
                    }

                    distMeasuredLong = Bsp_pga450GetMeasurement(
                               pCreateArgs->uartInstId,
                               deviceId,
                               BSP_PGA450_DISTANCE_MODE_LONG);

                    if(distMeasuredLong >= 0)
                    {
                        Vps_printf(" ULTRASONIC: DEVICE%d: LONG  DISTANCE = %d cm\n",
                               deviceId, distMeasuredLong);
                    }
                    else
                    {
                        Vps_printf(
                            " ULTRASONIC: DEVICE%d: LONG  DISTANCE = FAILED !!!\n",
                            deviceId);
                    }
*/
                    pDeviceInfo =
                        &pMesurementInfo->deviceInfo
                                    [pMesurementInfo->numSensors];

                    pDeviceInfo->deviceId = deviceId;
                    pDeviceInfo->distanceShort = distMeasuredShort;
                   // pDeviceInfo->distanceLong  = distMeasuredLong;

                    pMesurementInfo->numSensors++;
                }
            }

            Cache_wb(
                pMetaBuf->bufAddr[0],
                pMetaBuf->metaBufSize[0],
                Cache_Type_ALLD,
                TRUE
              );

            if(pMesurementInfo->numSensors > 0)
            {
                /* sensor data available, send data to next link */
                status = Utils_bufPutFullBuffer(&pObj->outFrameQue,
                                                pBuf);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

                System_sendLinkCmd(pCreateArgs->outQueParams.nextLink,
                                  SYSTEM_CMD_NEW_DATA, NULL);
            }
            else
            {
                /* No sensor detected, dont send buffer to next link */
                status = Utils_bufPutEmptyBuffer(&pObj->outFrameQue,
                                                pBuf);
                UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            }
        }

        curTime = Utils_getCurTimeInMsec() - curTime;

        /* sleep such that task gets invoked once every 33 msec */
        if(curTime<ULTRASONIC_CAPTURE_POLLING_INTERVAL)
            Task_sleep(ULTRASONIC_CAPTURE_POLLING_INTERVAL-curTime);

        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pRunMsg, BSP_OSAL_NO_WAIT);
        if (status == SYSTEM_LINK_STATUS_SOK)
        {
            /* extract message command from message */
            cmd = Utils_msgGetCmd(pRunMsg);

            switch (cmd)
            {
                case SYSTEM_CMD_STOP:
                    /* stop RUN loop and goto READY state */
                    runDone = TRUE;

                    /* ACK message after actually stopping the driver outside the
                     * RUN loop */
                    runAckMsg = TRUE;
                    break;

                case SYSTEM_CMD_DELETE:

                    /* stop RUN loop and goto IDLE state */

                    /* exit RUN loop */
                    runDone = TRUE;

                    /* exit READY loop */
                    *done = TRUE;

                    /* ACK message after exiting READY loop */
                    *ackMsg = TRUE;

                    /* Pass the received message to the READY loop */
                    *pMsg = pRunMsg;

                    break;
                default:

                    /* invalid command for this state ACK it and continue RUN
                     * loop */
                    Utils_tskAckOrFreeMsg(pRunMsg, status);
                    break;
            }
        }
    }

    /* RUN loop exited, stop driver */
    UltrasonicCaptureLink_drvStop(pObj);

    /* ACK message if not ACKed earlier */
    if (runAckMsg)
        Utils_tskAckOrFreeMsg(pRunMsg, status);

    return status;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to ULTRASONIC_CAPTURE link to get data from
 *    the output queue of ULTRASONIC_CAPTURE link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [OUT] A List of buffers needed for the next link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_getFullBuffers(Void * ptr, UInt16 queId,
                            System_BufferList * pBufList)
{
    Int32 status;
    UltrasonicCaptureLink_Obj *pObj;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    pObj = (UltrasonicCaptureLink_Obj *) pTsk->appData;

    status = Utils_bufGetFull(&pObj->outFrameQue, pBufList,
                              BSP_OSAL_NO_WAIT);

    return status;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to ULTRASONIC_CAPTURE link to get output queue
 *    Information of ULTRASONIC_CAPTURE link
 *
 * \param  ptr      [IN]  Handle to task
 * \param  info     [OUT] output queues information of ULTRASONIC_CAPTURE link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_getLinkInfo(Void * ptr, System_LinkInfo * info)
{
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;

    UltrasonicCaptureLink_Obj *pObj = (UltrasonicCaptureLink_Obj *) pTsk->appData;

    memcpy(info, &pObj->info, sizeof(*info));

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 * \brief Function called by links connected to ULTRASONIC_CAPTURE link to return back
 *    buffers
 *
 * \param  ptr      [IN]  Handle to task
 * \param  queId    [IN]  output queue Id
 * \param  pBufList [IN]  A List of buffers returned back to ULTRASONIC_CAPTURE link
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_putEmptyBuffers(Void * ptr, UInt16 queId,
                              System_BufferList * pBufList)
{
    Int32 status;
    Utils_TskHndl *pTsk = (Utils_TskHndl *) ptr;
    UltrasonicCaptureLink_Obj *pObj = (UltrasonicCaptureLink_Obj *) pTsk->appData;

    status = Utils_bufPutEmpty(&pObj->outFrameQue, pBufList);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 * \brief Function to delete ULTRASONIC_CAPTURE link. This will simply delete all output
 *    queues and the semaphore
 *
 * \param  pObj     [IN]  ULTRASONIC_CAPTURE link instance handle
 *
 * \return SYSTEM_LINK_STATUS_SOK on success
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_drvDelete(UltrasonicCaptureLink_Obj * pObj)
{
    Int32 status;
    UInt32 bufId;
    System_MetaDataBuffer *pMetaBuf;

    Vps_printf(" ULTRASONIC: Delete in progress !!!");

    for (bufId = 0; bufId < ULTRASONIC_CAPTURE_LINK_MAX_OUT_BUFFERS; bufId++)
    {
        pMetaBuf = &pObj->metaBufs[bufId];

        status = Utils_memFree( UTILS_HEAPID_DDR_CACHED_SR,
                        pMetaBuf->bufAddr[0],
                        pMetaBuf->metaBufSize[0]
                    );
        UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
    }

    status = Utils_bufDelete(&pObj->outFrameQue);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    Vps_printf(" ULTRASONIC: Delete DONE !!!");

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function implements the following.
 *    Accepts commands for
 *     - Creating ULTRASONIC_CAPTURE link
 *     - Arrival of new data
 *     - Deleting ULTRASONIC_CAPTURE link
 * \param  pTsk [IN] Task Handle
 * \param  pMsg [IN] Message Handle
 *
 *******************************************************************************
 */
Void UltrasonicCaptureLink_tskMain(struct Utils_TskHndl * pTsk, Utils_MsgHndl * pMsg)
{
    UInt32 cmd = Utils_msgGetCmd(pMsg);
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Bool ackMsg, done;

    UltrasonicCaptureLink_Obj *pObj = (UltrasonicCaptureLink_Obj*) pTsk->appData;

    if(cmd!=SYSTEM_CMD_CREATE)
    {
        /* invalid command recived in IDLE status, be in IDLE state and ACK
         * with error status */
        Utils_tskAckOrFreeMsg(pMsg, SYSTEM_LINK_STATUS_EFAIL);
        return;
    }

    /* Create command received, create the driver */
    status = UltrasonicCaptureLink_drvCreate(pObj, Utils_msgGetPrm(pMsg));

    /* ACK based on create status */
    Utils_tskAckOrFreeMsg(pMsg, status);

    /* if create status is error then remain in IDLE state */
    if (status != SYSTEM_LINK_STATUS_SOK)
        return;

    done = FALSE;
    ackMsg = FALSE;

    /* READY state loop */
    while (!done)
    {
        /* wait for message */
        status = Utils_tskRecvMsg(pTsk, &pMsg, BSP_OSAL_WAIT_FOREVER);
        if (status != SYSTEM_LINK_STATUS_SOK)
            break;

        /* extract message command from message */
        cmd = Utils_msgGetCmd(pMsg);

        switch (cmd)
        {
            case SYSTEM_CMD_START:
                /* Start capture driver */
                status = UltrasonicCaptureLink_drvStart(pObj);

                /* ACK based on create status */
                Utils_tskAckOrFreeMsg(pMsg, status);

                /* if start status is error then remain in READY state */
                if (status == SYSTEM_LINK_STATUS_SOK)
                {
                    /* start success, entering RUN state */
                    status =
                        UltrasonicCaptureLink_tskRun(pObj, pTsk, &pMsg, &done, &ackMsg);

                    /** done = FALSE, exit RUN state
                      done = TRUE, exit RUN and READY state
                     */
                }

                break;
            case SYSTEM_CMD_DELETE:

                /* exit READY state */
                done = TRUE;
                ackMsg = TRUE;
                break;

            default:
                /* invalid command for this state ACK it and continue READY
                 * loop */
                Utils_tskAckOrFreeMsg(pMsg, status);
                break;
        }
    }

    /* exiting READY state, delete driver */
    UltrasonicCaptureLink_drvDelete(pObj);

    /* ACK message if not previously ACK'ed */
    if (ackMsg && pMsg != NULL)
        Utils_tskAckOrFreeMsg(pMsg, status);

    /* entering IDLE state */
    return;
}

/**
 *******************************************************************************
 *
 * \brief Create task for this link
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
*/
Int32 UltrasonicCaptureLink_tskCreate()
{
    Int32                status;
    UltrasonicCaptureLink_Obj        *pObj;
    char                 tskName[32];

    pObj = &gUltrasonicCaptureLink_obj;

    sprintf(tskName, "ULTRASONIC_CAPTURE");

    /*
     * Create link task, task remains in IDLE state.
     * UltrasonicCaptureLink_tskMain is called when a message command is received.
     */
    status = Utils_tskCreate(&pObj->tsk,
                             UltrasonicCaptureLink_tskMain,
                             ULTRASONIC_CAPTURE_LINK_TSK_PRI,
                             gUltrasonicCaptureLink_tskStack,
                             ULTRASONIC_CAPTURE_LINK_TSK_STACK_SIZE,
                             pObj,
                             tskName);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Init function for ULTRASONIC_CAPTURE link. This function does the following for each
 *   ULTRASONIC_CAPTURE link,
 *  - Creates a task for the link
 *  - Registers this link with the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 UltrasonicCaptureLink_init()
{
    Int32 status;
    System_LinkObj linkObj;
    UltrasonicCaptureLink_Obj *pObj;
    Bsp_Pga450InitParams    pga450InitParams;

    UInt32 procId = System_getSelfProcId();

    if(!Bsp_platformIsTda2xxFamilyBuild())
        return 0;

    if(Bsp_boardGetId() == BSP_BOARD_MONSTERCAM)
        return 0;

    pObj = &gUltrasonicCaptureLink_obj;

    memset(pObj, 0, sizeof(*pObj));

    pObj->tskId = SYSTEM_MAKE_LINK_ID(procId,
                                      SYSTEM_LINK_ID_ULTRASONIC_CAPTURE);

    linkObj.pTsk = &pObj->tsk;
    linkObj.linkGetFullBuffers = UltrasonicCaptureLink_getFullBuffers;
    linkObj.linkPutEmptyBuffers = UltrasonicCaptureLink_putEmptyBuffers;
    linkObj.getLinkInfo = UltrasonicCaptureLink_getLinkInfo;

    System_registerLink(pObj->tskId, &linkObj);

    /* Setup pinmux and power on UART */
    UltrasonicCaptureLink_hwSetup();

    BspPga450InitParams_init(&pga450InitParams);
    status = Bsp_pga450Init(&pga450InitParams);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = UltrasonicCaptureLink_tskCreate();
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    return status;
}

/**
 *******************************************************************************
 *
 * \brief De-init function for ULTRASONIC_CAPTURE link. This function de-registers this link
 *  from the system
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 UltrasonicCaptureLink_deInit()
{
    if(!Bsp_platformIsTda2xxFamilyBuild())
        return 0;

    if(Bsp_boardGetId() == BSP_BOARD_MONSTERCAM)
        return 0;

    Utils_tskDelete(&gUltrasonicCaptureLink_obj.tsk);

    Bsp_pga450DeInit();

    return SYSTEM_LINK_STATUS_SOK;
}

