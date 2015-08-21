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
 * \file sgx3DsrvLink_drv.c
 *
 * \brief  This file has the implementation of Sgx3Dsrv Link
 *
 *         Sgx3Dsrv Link is used to feed video frames to SGX for
 *         rendering.
 *         The rendered output will be pushed to display via DRM.
 *
 * \version 0.0 (Jun 2014) : [SS] First version
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "srv3dinfoadaslink_priv.h"
#include <linux/src/osa/include/osa_mem.h>


extern srv3dinfoadaslink_Obj gsrv3dInfoAdasLink_obj[];

/**
 *******************************************************************************
 *
 * \brief Sgx3Dsrv link create function
 *
 *        This Set the Link and driver create time parameters.
 *        - Get the channel info from previous link
 *        - Set the internal data structures
 *        - Call the create and control functions
 *
 * \param   pObj     [IN] Sgx3Dsrv Link Instance handle
 * \param   pPrm     [IN] Sgx3Dsrv link create parameters
 *                        This need to be configured by the application
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_drvCreate(srv3dinfoadaslink_Obj *pObj,
                             srv3dinfoadaslink_CreateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    UInt32                  inQue, channelId;
    Sgx3DsrvLink_InputQueId inputQId;
    UInt32                  prevChInfoFlags;
    System_LinkChInfo     * pPrevChInfo;
    UInt32 surfaceId, i;
    struct link_cb cb;


#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Create in progress !!!\n");
#endif

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));
    status = OSA_mutexCreate(&(pObj->lock));
    OSA_assert(status == OSA_SOK);

    OSA_assert(pPrm->numInQue <= SRV3DINFOADAS_LINK_IPQID_MAXIPQ);

    for (inQue = 0; inQue < pPrm->numInQue; inQue++)
    {
        status = System_linkGetInfo(pPrm->inQueParams[inQue].prevLinkId,
                                    &pObj->inTskInfo[inQue]);
        OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
        OSA_assert(pPrm->inQueParams[inQue].prevLinkQueId <
                   pObj->inTskInfo[inQue].numQue);
        memcpy(&pObj->inQueInfo[inQue],
               &pObj->inTskInfo[inQue].queInfo[pPrm->inQueParams[inQue].prevLinkQueId],
               sizeof(pObj->inQueInfo));
    }

    OSA_assert(pObj->createArgs.inBufType[SRV3DINFOADAS_LINK_IPQID_MULTIVIEW] ==
                     SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER);
    OSA_assert(pObj->createArgs.inBufType[SRV3DINFOADAS_LINK_IPQID_PALUT] ==
                     SYSTEM_BUFFER_TYPE_METADATA);
    OSA_assert(pObj->createArgs.inBufType[SRV3DINFOADAS_LINK_IPQID_GALUT] ==
                     SYSTEM_BUFFER_TYPE_METADATA);
    OSA_assert(pObj->createArgs.inBufType[SRV3DINFOADAS_LINK_IPQID_BLENDLUT] ==
                     SYSTEM_BUFFER_TYPE_METADATA);

    inputQId = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;
    channelId = 0;
    pPrevChInfo   =
        &(pObj->inQueInfo[inputQId].chInfo[channelId]);

    OSA_assert(pObj->createArgs.maxOutputWidth  <= SRV3DINFOADAS_LINK_OUTPUT_FRAME_WIDTH);
    OSA_assert(pObj->createArgs.maxOutputHeight <= SRV3DINFOADAS_LINK_OUTPUT_FRAME_HEIGHT);

    prevChInfoFlags    = pPrevChInfo->flags;
    pObj->inDataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(prevChInfoFlags);
    pObj->inPitch[0] = pPrevChInfo->pitch[0];
    pObj->inPitch[1] = pPrevChInfo->pitch[1];

    if((pPrevChInfo->width > pObj->createArgs.maxInputWidth)
       ||
       (pPrevChInfo->height > pObj->createArgs.maxInputHeight)
      )
    {
      OSA_assert(NULL);
    }
    surfaceId = 0;
    for(i = 0; i < 4; i++) {
            pObj->multiView.buffers[i].surface_id                    = pObj->surface_Id[surfaceId];
            pObj->multiView.buffers[i].buf_type                      = SCENE_BUF_TYPE_PADDR;
            pObj->multiView.buffers[i].width      = pPrevChInfo->width;
        pObj->multiView.buffers[i].height     = pPrevChInfo->height;
        pObj->multiView.buffers[i].pitches[0] = pPrevChInfo->pitch[0];
        pObj->multiView.buffers[i].pitches[1] = pPrevChInfo->pitch[1];
        pObj->multiView.buffers[i].offsets[0] = 0;
        pObj->multiView.buffers[i].offsets[1] = pPrevChInfo->pitch[0] * pPrevChInfo->height;
        pObj->multiView.buffers[i].fourcc     = FOURCC_STR("NV12");
            surfaceId++;
    }

    /*
     * Creation of local input Qs for SGX3DSRV_LINK_IPQID_MULTIVIEW and
     * SGX3DSRV_LINK_IPQID_PALUT.
     * For ALGLINK_SYNTHESIS_IPQID_GALUT, always just one entry is kept.
     */
    inputQId = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;
    status  = OSA_queCreate(&(pObj->localInputQ[inputQId].queHandle),
                               SRV3DINFOADAS_LINK_MAX_LOCALQUEUELENGTH);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SRV3DINFOADAS_LINK_IPQID_PALUT;
    status  = OSA_queCreate(&(pObj->localInputQ[inputQId].queHandle),
                               SRV3DINFOADAS_LINK_MAX_LOCALQUEUELENGTH);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);



    OSA_resetLatency(&pObj->linkLatency);
    OSA_resetLatency(&pObj->srcToLinkLatency);

    pObj->numInputChannels = 1;
    OSA_resetLinkStatistics(&pObj->linkStats, pObj->numInputChannels, 1);

    pObj->isFirstFrameRecv     = FALSE;
    pObj->receivedGALUTFlag    = FALSE;
    pObj->receivedBlendLUTFlag = FALSE;
    pObj->receivedFirstPALUTFlag = FALSE;

  cb.recipient        = RCVR_3DLINK;
  cb.surface_info     = srv3dInfoAdasLink_getSurfaceIds;
  cb.return_frame_buf = srv3dInfoAdasLink_putEmptyBuffers;

  LINK_register_cb(&cb);
  pObj->glInitDone = 0;
#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Create Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

Void srv3dInfoAdasLink_getSurfaceIds(UInt32 num_surfaces, UInt32 *surfaceId)
{
    Int32 i;
    srv3dinfoadaslink_Obj *pObj = &gsrv3dInfoAdasLink_obj[0];
    struct scene_surface_3d_info info;

    if(pObj == NULL)
       return ;

    if(num_surfaces > SRV3DINFOADAS_LINK_MAX_SURFACES)
        return;

    pObj->num_surfaces = num_surfaces;

    for(i = 0 ; i < num_surfaces; i++)
       pObj->surface_Id[i] = surfaceId[i];

    for(i = 0; i < 4; i++)
        pObj->multiView.buffers[i].surface_id = surfaceId[i];

    pObj->stitched.buffers[0].surface_id = surfaceId[i];
    info.surface_id = pObj->stitched.buffers[0].surface_id;
    SCENE_get_3d_info (&info);
    pObj->surface3d = info.surface_3d_surf;
    pObj->display3d = info.surface_3d_disp;
    pObj->width3d = info.width;
    pObj->height3d = info.height;


    return;
}

/**
 *******************************************************************************
 *
 * \brief This function prints the Link status
 *
 *        prints the Link status, such as
 *        - FPS
 *        - Callback Intervals
 *        - Input DropCount
 *        - etc
 *
 * \param   pObj        [IN] Sgx3Dsrv Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_drvPrintStatistics(srv3dinfoadaslink_Obj *pObj)
{
    OSA_printLinkStatistics(&pObj->linkStats, "SRV3DINFOADAS", TRUE);

    OSA_printLatency("SRV3DINFOADAS",
                       &pObj->linkLatency,
                       &pObj->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function de-queue frames/meta data from prvious link output Qs
 *
 *        Function perform the following operations
 *        - De-queue frames/meta data from prvious link output Qs
 *        - Put them in an internal Qs
 *
 * \param   pObj     [IN] Sgx3Dsrv Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_getInputFrameData(srv3dinfoadaslink_Obj * pObj)
{
    srv3dinfoadaslink_InputQueId      inputQId;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       bufId;
    System_BufferList            inputBufList;
    System_BufferList            inputBufListReturn;
    System_Buffer              * pSysBufferInput;
    srv3dinfoadaslink_CreateParams  * pPrm;
    System_VideoFrameCompositeBuffer *pVideoCompositeFrame;
    UInt32 i;

    pPrm = &pObj->createArgs;

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_MULTIVIEW and queue them up locally.
     */
    inputQId = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;

    System_getLinksFullBuffers(
        pPrm->inQueParams[inputQId].prevLinkId,
        pPrm->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysBufferInput = inputBufList.buffers[bufId];

            /*TBD: Check for parameter correctness. If in error, return input*/

            if (pSysBufferInput != NULL)
            {
            struct scene_buffer_object *bufObject = OSA_memAlloc(sizeof(struct scene_buffer_object));
            pVideoCompositeFrame = (System_VideoFrameCompositeBuffer *)
                                       (pSysBufferInput->payload);

            memcpy(bufObject, &pObj->multiView, sizeof(struct scene_buffer_object));

                        bufObject->num_buffers = 4;
                        bufObject->sbo_priv     = (Void*)pSysBufferInput;

            for (i = 0; i < 4; i++)
            {
                  bufObject->buffers[i].paddrs[0] = 
                                (void *)OSA_memVirt2Phys((UInt32)pVideoCompositeFrame->bufAddr[0][i], OSA_MEM_REGION_TYPE_AUTO);
            }            
        
                      //POST THE BUFFER_OBJECT HERE to UI_CLIENT
                        SCENE_post_new_buffer(bufObject, SNDR_3DLINK);
            OSA_memFree(bufObject);

                status = OSA_quePut(
                            &(pObj->localInputQ[inputQId].queHandle),
                            (Int32) pSysBufferInput,
                            OSA_TIMEOUT_NONE);
                OSA_assert(status == SYSTEM_LINK_STATUS_SOK);
            }
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_PALUT and queue them up locally.
     */
    inputQId = SRV3DINFOADAS_LINK_IPQID_PALUT;

    System_getLinksFullBuffers(
        pPrm->inQueParams[inputQId].prevLinkId,
        pPrm->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            pSysBufferInput = inputBufList.buffers[bufId];
            /*TBD: Check for parameter correctness. If in error, return input*/
            status = OSA_quePut(
                        &(pObj->localInputQ[inputQId].queHandle),
                        (Int32) pSysBufferInput,
                        OSA_TIMEOUT_NONE);
            UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
            pObj->receivedFirstPALUTFlag = TRUE;
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = ALGLINK_SYNTHESIS_IPQID_GALUT and store latest copy locally.
     */
    inputQId = SRV3DINFOADAS_LINK_IPQID_GALUT;
    System_getLinksFullBuffers(
        pPrm->inQueParams[inputQId].prevLinkId,
        pPrm->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            /*
             * At any point in time, Synthesis link will hold only one GA LUT.
             * So whenever GA LUT is received, the previously received one
             * will be released and the newly received one will be archived.
             */
            if(pObj->receivedGALUTFlag == TRUE)
            {
                inputBufListReturn.numBuf     = 1;
                inputBufListReturn.buffers[0] = pObj->sysBufferGALUT;

                if(inputBufListReturn.numBuf)
                {
                    System_putLinksEmptyBuffers(pPrm->inQueParams[inputQId].prevLinkId,
                                                pPrm->inQueParams[inputQId].prevLinkQueId,
                                                &inputBufListReturn);
                }

                pObj->receivedGALUTFlag = FALSE;
            }

            pObj->sysBufferGALUT = inputBufList.buffers[bufId];
            /*TBD: Check for parameter correctness. If in error, return input*/
            pObj->receivedGALUTFlag = TRUE;
#ifdef SYSTEM_DEBUG_DISPLAY
            Vps_printf (" SRV3DINFOADAS Link - GA LUT received !!! \n");
#endif
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_BLENDLUT and store latest copy locally.
     */
    inputQId = SRV3DINFOADAS_LINK_IPQID_BLENDLUT;
    System_getLinksFullBuffers(
        pPrm->inQueParams[inputQId].prevLinkId,
        pPrm->inQueParams[inputQId].prevLinkQueId,
        &inputBufList);

    if(inputBufList.numBuf)
    {
        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
            /*
             * At any point in time, Synthesis link will hold only one GA LUT.
             * So whenever GA LUT is received, the previously received one
             * will be released and the newly received one will be archived.
             */
            if(pObj->receivedBlendLUTFlag == TRUE)
            {
                inputBufListReturn.numBuf     = 1;
                inputBufListReturn.buffers[0] = pObj->sysBufferBlendLUT;

                if(inputBufListReturn.numBuf)
                {
                    System_putLinksEmptyBuffers(pPrm->inQueParams[inputQId].prevLinkId,
                                                pPrm->inQueParams[inputQId].prevLinkQueId,
                                                &inputBufListReturn);
                }

                pObj->receivedBlendLUTFlag = FALSE;
            }

            pObj->sysBufferBlendLUT = inputBufList.buffers[bufId];
            /*TBD: Check for parameter correctness. If in error, return input*/
            pObj->receivedBlendLUTFlag = TRUE;
#ifdef SYSTEM_DEBUG_DISPLAY
            Vps_printf (" SRV3DINFOADAS Link - Blending LUT received !!! \n");
#endif
        }
    }

    pObj->linkStats.newDataCmdCount++;

    return status;
}

Void srv3dInfoAdasLink_putEmptyBuffers(Void *buf)
{
   Int32 inQue = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;
   System_Buffer *pBuf;
   System_BufferList freeBufList;
   srv3dinfoadaslink_CreateParams *pCreateArgs;
   srv3dinfoadaslink_Obj *pObj = &gsrv3dInfoAdasLink_obj[0];
   struct scene_buffer_object *buffer = buf;

   if(pObj == NULL)
       return ;

   pCreateArgs = &pObj->createArgs;

   OSA_mutexLock(&(pObj->lock));

   inQue = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;


   pBuf = (System_Buffer*)buffer->sbo_priv;
   OSA_assert(pBuf != NULL);


   freeBufList.numBuf = 1;
   freeBufList.buffers[0] = pBuf;
   System_putLinksEmptyBuffers(pCreateArgs->inQueParams[inQue].
                                       prevLinkId,
                                       pCreateArgs->inQueParams[inQue].
                                       prevLinkQueId,
                                       &freeBufList);


   OSA_mutexUnlock(&(pObj->lock));

   return;
}


/**
 *******************************************************************************
 *
 * \brief This function de-queue and process/sgx3Dsrv the input frames
 *
 *        Function perform the following operations
 *        - De-queue the frames from the previous link output Q
 *        - Put them in intenal link queues
 *        - Get/De-queue the valid frames from the internal queue
 *        - Populate/Map OpenGL/Drm frame structure from system buffer
 *        - Call the OpenGL/DRM process function
 *        - Free-up these frames by send back to previous link after display
 *        - Immediately Free-up the frames which are not part of the display
 *
 * \param   pObj     [IN] Sgx3Dsrv Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */

static int init_gl(System_EglObj *pEglObj, void *disp, void *surf)
{
    EGLint num_configs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint w, h;
    int ret;

    const EGLint attribs[] = {
       EGL_RED_SIZE, 1,
       EGL_GREEN_SIZE, 1,
       EGL_BLUE_SIZE, 1,
       EGL_ALPHA_SIZE, 0,
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
       EGL_DEPTH_SIZE, 8,
       EGL_NONE
    };
    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    memset(pEglObj, 0, sizeof(*pEglObj));

    pEglObj->display = eglGetDisplay((EGLNativeDisplayType)disp);
    System_eglCheckEglError("eglGetDisplay", EGL_TRUE);
    if (pEglObj->display == EGL_NO_DISPLAY) {
       Vps_printf(" EGL: ERROR: eglGetDisplay() returned EGL_NO_DISPLAY !!!\n");
       return -1;
    }

    ret = eglInitialize(pEglObj->display, &majorVersion, &minorVersion);
    System_eglCheckEglError("eglInitialize", ret);
    Vps_printf(" EGL: version %d.%d\n", majorVersion, minorVersion);
    if (ret != EGL_TRUE) {
       Vps_printf(" EGL: eglInitialize() failed !!!\n");
       return -1;
    }

    if (!eglChooseConfig(pEglObj->display, attribs, &pEglObj->config, 1, &num_configs))
    {
       Vps_printf(" EGL: ERROR: eglChooseConfig() failed. Couldn't get an EGL visual config !!!\n");
       return -1;
    }

    pEglObj->surface = eglCreateWindowSurface(
                pEglObj->display,
                pEglObj->config,
                (EGLNativeWindowType)surf,
                NULL);
    System_eglCheckEglError("eglCreateWindowSurface", EGL_TRUE);
    if (pEglObj->surface == EGL_NO_SURFACE)
    {
       Vps_printf(" EGL: eglCreateWindowSurface() failed!!!\n");
       return -1;
    }

    pEglObj->context = eglCreateContext(pEglObj->display, pEglObj->config, EGL_NO_CONTEXT, context_attribs);
    System_eglCheckEglError("eglCreateContext", EGL_TRUE);
    if (pEglObj->context == EGL_NO_CONTEXT) {
       Vps_printf(" EGL: eglCreateContext() failed !!!\n");
       return -1;
    }

    ret = eglMakeCurrent(pEglObj->display, pEglObj->surface, pEglObj->surface, pEglObj->context);
    System_eglCheckEglError("eglMakeCurrent", ret);
    if (ret != EGL_TRUE) {
       Vps_printf(" EGL: eglMakeCurrent() failed !!!\n");
       return -1;
    }

    eglQuerySurface(pEglObj->display, pEglObj->surface, EGL_WIDTH, &w);
    System_eglCheckEglError("eglQuerySurface", EGL_TRUE);
    eglQuerySurface(pEglObj->display, pEglObj->surface, EGL_HEIGHT, &h);
    System_eglCheckEglError("eglQuerySurface", EGL_TRUE);

    pEglObj->width = w;
    pEglObj->height = h;

    Vps_printf(" EGL: Window dimensions: %d x %d\n", w, h);

    System_eglPrintGLString("Version", GL_VERSION);
    System_eglPrintGLString("Vendor", GL_VENDOR);
    System_eglPrintGLString("Renderer", GL_RENDERER);
    System_eglPrintGLString("Extensions", GL_EXTENSIONS);

    glViewport(0, 0, w, h);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(pEglObj->display, pEglObj->surface);

    return 0;
}

volatile static unsigned int gDebug_IsCalibDone = 0;
Int32 srv3dinfoadaslink_drvDoProcessFrames(srv3dinfoadaslink_Obj *pObj)
{
    srv3dinfoadaslink_CreateParams  * pPrm;
    srv3dinfoadaslink_InputQueId      inputQId;
    UInt32                       channelId = 0;
    Int32                        status = SYSTEM_LINK_STATUS_SOK;
    System_BufferList            inputBufListReturn;
    System_Buffer              * pSystemBufferMultiview;
    System_Buffer              * pSystemBufferPALUT;
    Bool                         isProcessCallDoneFlag;
    System_MetaDataBuffer      * pPALUTBuffer;
    System_MetaDataBuffer      * pGALUTBuffer;
    System_MetaDataBuffer      * pBlendLUTBuffer;
    System_VideoFrameCompositeBuffer *pVideoCompositeFrame;
    GLuint                       texYuv[4] = {0};
    System_EglTexProperty        texProp;

    pPrm = &pObj->createArgs;

    srv3dinfoadaslink_getInputFrameData(pObj);

    /*
     * Continous loop to perform synthesis as long as
     * input buffers are available.
     */
    if(!pObj->glInitDone) {
        pObj->render3DSRVObj.screen_width = pObj->width3d;
            pObj->render3DSRVObj.screen_height = pObj->height3d;
            init_gl(&pObj->eglObj, pObj->display3d, pObj->surface3d);
            status = SgxRender3DSRV_setup(&pObj->render3DSRVObj);
            OSA_assert(status==SYSTEM_LINK_STATUS_SOK);
        pObj->glInitDone = 1;
    }
    while(1)
    {
      isProcessCallDoneFlag = FALSE;
      /* Checking if all the inputs are available */

      if(pObj->receivedGALUTFlag == TRUE
              &&
         pObj->receivedBlendLUTFlag == TRUE
              &&
         pObj->receivedFirstPALUTFlag == TRUE
              &&
         OSA_queGetQueuedCount(
              &(pObj->localInputQ[SRV3DINFOADAS_LINK_IPQID_MULTIVIEW].queHandle))>0
       )
       {

        pSystemBufferPALUT = NULL;
        status = OSA_queGet(
                    &(pObj->localInputQ[SRV3DINFOADAS_LINK_IPQID_PALUT].
                        queHandle),
                    (Int32 *) &pSystemBufferPALUT,
                    OSA_TIMEOUT_NONE);

        if (pSystemBufferPALUT != NULL)
        {
            pObj->sysBufferPALUT = pSystemBufferPALUT;
        }

        pPALUTBuffer = (System_MetaDataBuffer *)pObj->sysBufferPALUT->payload;
        OSA_assert (pPALUTBuffer != NULL);

        pGALUTBuffer = (System_MetaDataBuffer *)pObj->sysBufferGALUT->payload;
        OSA_assert (pGALUTBuffer != NULL);

        pBlendLUTBuffer = (System_MetaDataBuffer *)pObj->sysBufferBlendLUT->payload;
        OSA_assert (pBlendLUTBuffer != NULL);

        if(pObj->isFirstFrameRecv==FALSE)
        {
            pObj->isFirstFrameRecv = TRUE;

            OSA_resetLinkStatistics(
                    &pObj->linkStats,
                    pObj->numInputChannels,
                    1);

            OSA_resetLatency(&pObj->linkLatency);
            OSA_resetLatency(&pObj->srcToLinkLatency);
        }

        /*
         * Reaching here means output buffers are available.
         * Hence getting inputs from local Queus
         */
        pSystemBufferMultiview = NULL;
        status = OSA_queGet(
                  &(pObj->localInputQ[SRV3DINFOADAS_LINK_IPQID_MULTIVIEW].
                      queHandle),
                  (Int32 *) &pSystemBufferMultiview,
                  OSA_TIMEOUT_NONE);

        /* Submit the SRV frames to SGX processing & DRM display */
        if (pSystemBufferMultiview != NULL && status == SYSTEM_LINK_STATUS_SOK)
        {
            channelId = pSystemBufferMultiview->chNum;
            if(channelId < pObj->linkStats.numCh)
            {
                pObj->linkStats.chStats[channelId].inBufRecvCount++;
                pObj->linkStats.chStats[channelId].inBufProcessCount++;
                pObj->linkStats.chStats[channelId].outBufCount[0]++;
            }

            pSystemBufferMultiview->linkLocalTimestamp = OSA_getCurGlobalTimeInUsec();

            inputQId = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;

            texProp.width      = pObj->inQueInfo[inputQId].chInfo[channelId].width;
            texProp.height     = pObj->inQueInfo[inputQId].chInfo[channelId].height;
            texProp.pitch[0]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[0];
            texProp.pitch[1]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[1];
            texProp.dataFormat = pObj->inDataFormat;

            // Do we need to specify the stitched frame resolution ?
            // Do we need to set any other parameters in render3DSRVObj ?
            pObj->render3DSRVObj.LUT3D = (void *) pGALUTBuffer->bufAddr[0];
            pObj->render3DSRVObj.blendLUT3D = (void *) pBlendLUTBuffer->bufAddr[0];
            pObj->render3DSRVObj.PALUT3D = (void *) pPALUTBuffer->bufAddr[0];

            if(pSystemBufferMultiview->bufType==SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER)
            {
                pVideoCompositeFrame = (System_VideoFrameCompositeBuffer *)
                                       (pSystemBufferMultiview->payload);
                OSA_assert(pVideoCompositeFrame != NULL);

                /* pick CH0 by default */
                // TODO: check that the A15 mapped address is not NULL
                texYuv[0] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][0]);
                texYuv[1] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][1]);
                texYuv[2] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][2]);
                texYuv[3] = System_eglGetTexYuv(&pObj->eglObj, &texProp, pVideoCompositeFrame->bufAddr[0][3]);
            }
            else
            {
                Vps_printf(" SRV3DINFOADAS Link: ERROR: Recevied invalid buffer type !!!\n");
                OSA_assert(0);
            }
        //OSA_assert(texYuv != NULL);
        //OSA_assert(texProp.width != 0);
            render3dFrame(
                            &pObj->render3DSRVObj,
                &pObj->eglObj,
                            texYuv
                            );
            OSA_updateLatency(&pObj->linkLatency,
                              pSystemBufferMultiview->linkLocalTimestamp);
            OSA_updateLatency(&pObj->srcToLinkLatency,
                              pSystemBufferMultiview->srcTimestamp);
        }


        if(status!=SYSTEM_LINK_STATUS_SOK)
        {
            Vps_printf(" SRV3DINFOADAS Link: System_drmEglSwapBuffers() failed !!!\n");
        }

        isProcessCallDoneFlag = TRUE;

        /*
         * Releasing (Free'ing) Input buffers, since algorithm does not need
         * it for any future usage.
         */

        if (pSystemBufferPALUT != NULL)
        {
          inputQId                      = SRV3DINFOADAS_LINK_IPQID_PALUT;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSystemBufferPALUT;
          if(inputBufListReturn.numBuf)
          {
              System_putLinksEmptyBuffers(pPrm->inQueParams[inputQId].prevLinkId,
                                          pPrm->inQueParams[inputQId].prevLinkQueId,
                                          &inputBufListReturn);
          }
        }
      }

      if(isProcessCallDoneFlag == FALSE)
      {
          /* TBD - the drop frame staus update is kept here temperally.
           * This should done if output buffer is not available, and this
           * will be done when DRM is removed out from this link */
          pObj->linkStats.inBufErrorCount++;
          break;
      }
    } /* End of while(1) */

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function delete the sgx3Dsrv link & driver
 *
 *        De-queue any frames which are held inside the driver, then
 *        - Delete the simply driver
 *        - delete the semaphore and other link data structures
 *        - delete the link periodic object
 *
 * \param   pObj     [IN] Sgx3Dsrv Link Instance handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_drvDelete(srv3dinfoadaslink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    srv3dinfoadaslink_InputQueId inputQId;
    struct link_cb cb;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Delete in progress !!!\n");
#endif

    /*
     * Deletion of local input Qs for SGX3DSRV_LINK_IPQID_MULTIVIEW and
     * SGX3DSRV_LINK_IPQID_PALUT.
     * For ALGLINK_SYNTHESIS_IPQID_GALUT, always just one entry is kept.
     */
    inputQId = SRV3DINFOADAS_LINK_IPQID_MULTIVIEW;
    status = OSA_queDelete(&(pObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SRV3DINFOADAS_LINK_IPQID_PALUT;
    status = OSA_queDelete(&(pObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);
  
    cb.recipient = RCVR_3DLINK;
    LINK_unregister_cb(&cb);
    OSA_mutexDelete(&pObj->lock);

    /*
    status = System_drmClose(&pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);
    */

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Delete Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief This function start the simply driver
 *
 *        Primming of a few frames are required to start the Sgx3Dsrv driver.
 *        Use blank buffers to prime and start the simply driver even
 *        before the actual frames are received by the sgx3Dsrv link. This
 *        primming is done while sgx3Dsrv link create. Start shall be called
 *        only after the link create function
 *
 * \param   pObj     [IN] Sgx3Dsrv Link Instance handle
 *
 * \return  status
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_drvStart(srv3dinfoadaslink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Start in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Start Done !!!\n");
#endif

    return status;
}

/**
 *******************************************************************************
 *
 * \brief This function stop the simply driver
 *
 *        When ever the driver is stopped, enable the sgx3Dsrv link periodic
 *        call back function. This will initiate to free-up the input frames
 *        in STOP state. The driver call back will be stopped when sgx3Dsrv
 *        driver stop is done
 *
 * \param   pObj     [IN] Sgx3Dsrv Link Instance handle
 *
 * \return  status
 *
 *******************************************************************************
 */
Int32 srv3dinfoadaslink_drvStop(srv3dinfoadaslink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Stop in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SRV3DINFOADAS Link: Stop Done !!!\n");
#endif

    return status;
}

/* Nothing beyond this point */

